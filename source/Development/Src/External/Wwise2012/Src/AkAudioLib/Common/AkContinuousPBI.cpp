/*=============================================================================
	AkContinuousPBI.cpp - CAkContinuousPBI (akcontinuouspbi.cpp, 2012 0x94bf30..0x94cdd0); 2013 matched by bytes
	(ih1_inventory.csv).

	DISHONORED(port): agent IH3, fork C (wave 26). Every transition mode schedules the next item the way retail does:
	  none / delay       - Term (the item ended) posts the PlayAndContinue, with the delay for Transition_Delay;
	  cross-fades        - SetEstimatedLength (the source's length is known) posts it length - fade early with a fade-back
	                       on this PBI (CAkActionPlayAndContinue::Execute stops it with the fade);
	  trigger rate       - SetEstimatedLength posts it after the transition time;
	  sample accurate    - PrepareSampleAccurateTransition (ProcessContextNotif) posts it at once with this PBI's
	                       sequence id, so the lower engine chains the source in the same voice.
	Deviations (named): the MonitorFade calls are compiled out (no profiler); AkTimeConv::MillisecondsToSamples is the
	file-static MillisecondsToSamples; SetEstimatedLength reads the estimated length once (the 2012 decompile's
	__userpurge prototype shows it as two stack arguments, the same value).
=============================================================================*/
#include "AkContinuousPBI.h"
#include "AkActionPlayAndContinue.h"
#include "AkTransitionManager.h"

/** 2012 .data 0xe37300 = 1. */
AkUniqueID CAkContinuousPBI::m_CalSeqID = 1;

static AkInt32 MillisecondsToSamples( AkReal32 in_fMs )
{
	return (AkInt32)( in_fMs * (AkReal32)AkAudioLibSettings::g_pipelineCoreFrequency / 1000.f );
}

static void FreePendingAction( AkPendingAction * in_pPendingAction )
{
	in_pPendingAction->~AkPendingAction();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pPendingAction );
}

static AkPendingAction * NewPendingAction( CAkRegisteredObj * in_pGameObj )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPendingAction ) );
	return pMem ? AkPlacementNew( pMem ) AkPendingAction( in_pGameObj ) : NULL;
}

/** 2012 0x94c220: the continuous params' transitions move into this PBI; the instigator is kept referenced. */
CAkContinuousPBI::CAkContinuousPBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, ContParams & in_Cparameters, UserParams & in_rUserParams, PlayHistory & in_rPlayHistory, bool in_bIsFirst, AkUniqueID in_SeqID, CAkPBIAware * in_pInstigator, const PriorityInfoCurrent & in_rPriority, bool in_bTargetFeedback, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter )
	: CAkPBI( in_pSound, in_pSource, in_pGameObj, in_rUserParams, in_rPlayHistory, in_SeqID, in_rPriority, in_bTargetFeedback, 0, in_pAMLimiter, in_pBusLimiter )
	, m_spContList( in_Cparameters.spContList )
	, m_pInstigator( in_pInstigator )
	, m_fTransitionTime( 0.f )
	, m_ulNextElementToPlay( AK_INVALID_UNIQUE_ID )
	, m_eTransitionMode( 0 )
	, m_bIsFirstPlay( in_bIsFirst )
	, m_bIsContinuousPaused( 0 )
	, m_bIsNextPrepared( 0 )
{
	if( m_spContList )
	{
		m_spContList->AddRef();
	}
	m_PlayHistoryForNextToPlay.arrayIsContinuous.m_i32BitArray = 0;
	in_pInstigator->AddRef();
	if( !m_SeqID )
	{
		m_SeqID = m_CalSeqID++;
	}
	m_ulPauseCount = in_Cparameters.ulPauseCount;
	if( !m_PBTrans.pvPSTrans )
	{
		m_PBTrans.pvPSTrans = in_Cparameters.pPlayStopTransition;
		in_Cparameters.pPlayStopTransition = NULL;
		m_PBTrans.bIsPSTransFading = in_Cparameters.bIsPlayStopTransitionFading;
	}
	if( !m_PBTrans.pvPRTrans )
	{
		m_PBTrans.pvPRTrans = in_Cparameters.pPauseResumeTransition;
		in_Cparameters.pPauseResumeTransition = NULL;
		m_PBTrans.bIsPRTransFading = in_Cparameters.bIsPauseResumeTransitionFading;
	}
	m_PlayHistoryForNextToPlay.HistArray.uiArraySize = 0;
	PrepareNextPlayHistory( in_rPlayHistory );
}

/** 2012 0x94c340. */
CAkContinuousPBI::~CAkContinuousPBI()
{
	m_pInstigator->Release();
	if( m_spContList )
	{
		m_spContList->Release();
		m_spContList = NULL;
	}
}

/** 2012 0x94bf30: becomes a user of the inherited transitions; a finished fade-out starts the PBI stopped
    (m_eInitialState 2), a finished pause fade or a pending pause count starts it paused (1). */
AKRESULT CAkContinuousPBI::Init( AkPathInfo * in_pPathInfo )
{
	const AKRESULT eResult = CAkPBI::Init( in_pPathInfo );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( m_PBTrans.pvPSTrans )
	{
		const AKRESULT eAdd = g_pTransitionManager->AddTransitionUser( m_PBTrans.pvPSTrans, this );
		if( eAdd == AK_Success )
		{
			if( g_pTransitionManager->IsTerminated( m_PBTrans.pvPSTrans ) )
			{
				g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, this );
				m_PBTrans.pvPSTrans = NULL;
				if( m_PBTrans.bIsPSTransFading )
				{
					m_eInitialState = 2;
					m_PBTrans.bIsPSTransFading = 0;
				}
			}
		}
		else
		{
			if( eAdd != AK_UserAlreadyInList )
			{
				m_PBTrans.pvPSTrans = NULL;
			}
			if( eAdd == AK_TransitionNotFound && m_PBTrans.bIsPSTransFading )
			{
				m_eInitialState = 2;
				m_PBTrans.bIsPSTransFading = 0;
			}
		}
	}
	if( m_PBTrans.pvPRTrans )
	{
		const AKRESULT eAdd = g_pTransitionManager->AddTransitionUser( m_PBTrans.pvPRTrans, this );
		bool bEnded = false;
		if( eAdd == AK_Success )
		{
			if( g_pTransitionManager->IsTerminated( m_PBTrans.pvPRTrans ) )
			{
				g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, this );
				m_PBTrans.pvPRTrans = NULL;
				bEnded = true;
			}
		}
		else
		{
			if( eAdd != AK_UserAlreadyInList )
			{
				m_PBTrans.pvPRTrans = NULL;
			}
			bEnded = eAdd == AK_TransitionNotFound;
		}
		if( bEnded && m_PBTrans.bIsPRTransFading )
		{
			if( !m_eInitialState )
			{
				m_eInitialState = 1;
			}
			m_PBTrans.bIsPRTransFading = 0;
		}
	}
	if( !m_eInitialState && m_ulPauseCount && !m_PBTrans.pvPRTrans )
	{
		m_eInitialState = 1;
	}
	return AK_Success;
}

/** 2012 0x94c0a0: no seeking inside a cross-fade / trigger-rate chain, nor in a sample-accurate one past its first item. */
bool CAkContinuousPBI::CanSeek()
{
	bool bSampleAccurate = false;
	for( CAkParameterNodeBase * pNode = m_pSound->m_pParentNode; pNode; pNode = pNode->m_pParentNode )
	{
		if( pNode->NodeCategory() != AkNodeCategory_RanSeqCntr )
		{
			continue;
		}
		const AkTransitionMode eMode = static_cast<CAkRanSeqCntr *>( pNode )->TransitionMode();
		if( eMode == Transition_CrossFadeAmp || eMode == Transition_CrossFadePower || eMode == Transition_TriggerRate )
		{
			return false;
		}
		if( eMode == Transition_SampleAccurate )
		{
			bSampleAccurate = true;
		}
	}
	return !bSampleAccurate || m_bIsFirstPlay;
}

/** 2012 0x94c120: the history up to the innermost continuous container. */
void CAkContinuousPBI::PrepareNextPlayHistory( PlayHistory & in_rPlayHistory )
{
	m_PlayHistoryForNextToPlay = in_rPlayHistory;
	while( m_PlayHistoryForNextToPlay.HistArray.uiArraySize )
	{
		const AkUInt32 uLast = m_PlayHistoryForNextToPlay.HistArray.uiArraySize - 1;
		if( uLast < 32 && m_PlayHistoryForNextToPlay.arrayIsContinuous.IsSet( uLast ) )
		{
			break;
		}
		m_PlayHistoryForNextToPlay.HistArray.uiArraySize = uLast;
	}
}

/** 2012 0x94c170. */
void CAkContinuousPBI::SetPauseStateForContinuous( bool in_bIsPaused )
{
	m_bIsContinuousPaused = in_bIsPaused;
}

/** 2012 0x94c190. */
void CAkContinuousPBI::SeekTimeAbsolute( AkTimeMs in_iPosition, bool in_bSnapToMarker )
{
	if( CanSeek() )
	{
		CAkPBI::SeekTimeAbsolute( in_iPosition, in_bSnapToMarker );
	}
}

/** 2012 0x94c1c0. */
void CAkContinuousPBI::SeekPercent( AkReal32 in_fPercent, bool in_bSnapToMarker )
{
	if( CanSeek() )
	{
		CAkPBI::SeekPercent( in_fPercent, in_bSnapToMarker );
	}
}

/** 2012 0x94c380: walks the continuation list from its innermost item for the next element (and the innermost
    container's transition); multi-play items hand the walk back to the caller's list. Preliminary calls (before the
    end) do not pop a multi-play item unless the container under it triggers at a rate. */
void CAkContinuousPBI::PrepareNextToPlay( bool in_bIsPreliminary )
{
	if( m_bIsNextPrepared || !m_spContList )
	{
		m_bIsNextPrepared = 1;
		return;
	}
	while( !m_spContList->m_listItems.IsEmpty() )
	{
		CAkContinueListItem & rItem = *( m_spContList->m_listItems.m_pItemsEnd - 1 );
		if( !rItem.m_pMultiPlayNode )
		{
			AkUInt16 wPositionSelected;
			AkUniqueID uSelectedNodeID;
			CAkParameterNodeBase * pNextToPlay = rItem.m_pContainer->GetNextToPlayContinuous( m_pGameObj, wPositionSelected, uSelectedNodeID, rItem.m_pContainerInfo, rItem.m_LoopingInfo );
			if( pNextToPlay )
			{
				m_PlayHistoryForNextToPlay.HistArray.aCntrHist[ m_PlayHistoryForNextToPlay.HistArray.uiArraySize - 1 ] = wPositionSelected;
				m_ulNextElementToPlay = pNextToPlay->key;
				m_eTransitionMode = rItem.m_pContainer->TransitionMode();
				const AkTransitionMode eMode = TransitionMode();
				if( eMode == Transition_CrossFadeAmp || eMode == Transition_CrossFadePower || eMode == Transition_Delay || eMode == Transition_TriggerRate )
				{
					m_fTransitionTime = rItem.m_pContainer->TransitionTime( m_pGameObj );
				}
				else
				{
					m_fTransitionTime = 0.f;
				}
				m_bIsNextPrepared = 1;
				pNextToPlay->Release();
				return;
			}
			if( --m_PlayHistoryForNextToPlay.HistArray.uiArraySize )
			{
				do
				{
					const AkUInt32 uLast = m_PlayHistoryForNextToPlay.HistArray.uiArraySize - 1;
					if( uLast < 32 && m_PlayHistoryForNextToPlay.arrayIsContinuous.IsSet( uLast ) )
					{
						break;
					}
					--m_PlayHistoryForNextToPlay.HistArray.uiArraySize;
				}
				while( m_PlayHistoryForNextToPlay.HistArray.uiArraySize );
			}
			m_spContList->RemoveLast();
			continue;
		}
		if( in_bIsPreliminary )
		{
			CAkContinuationList * pAlternate = rItem.m_pAlternateContList;
			if( pAlternate->m_listItems.IsEmpty() )
			{
				return;
			}
			CAkContinueListItem * pDeep = pAlternate->m_listItems.m_pItemsEnd - 1;
			while( pDeep->m_pMultiPlayNode )
			{
				CAkContinuationList * pNextAlternate = pDeep->m_pAlternateContList;
				if( pNextAlternate->m_listItems.IsEmpty() )
				{
					return;
				}
				pDeep = pNextAlternate->m_listItems.m_pItemsEnd - 1;
			}
			if( pDeep->m_pContainer->TransitionMode() != Transition_TriggerRate )
			{
				return;
			}
		}
		CAkContinuationList * l_spContList = NULL;
		rItem.m_pMultiPlayNode->ContGetList( rItem.m_pAlternateContList, l_spContList );
		m_spContList->RemoveLast();
		if( !l_spContList )
		{
			CAkContinuationList * pOld = m_spContList;
			m_spContList = NULL;
			pOld->Release();
			return;
		}
		CAkContinuationList * pOld = m_spContList;
		m_spContList = l_spContList;
		pOld->Release();
	}
	CAkContinuationList * pOld = m_spContList;
	m_spContList = NULL;
	pOld->Release();
	m_bIsNextPrepared = 1;
}

/** 2012 0x94c5f0: once the source length is known a cross-fade (the item is long enough) or a trigger-rate container
    schedules its next item now. */
void CAkContinuousPBI::SetEstimatedLength( AkReal32 in_fEstimatedLength )
{
	if( m_bWasStopped || m_bWasPreStopped )
	{
		return;
	}
	PrepareNextToPlay( true );
	const AkTransitionMode eMode = TransitionMode();
	const bool bCrossFade = eMode == Transition_CrossFadeAmp || eMode == Transition_CrossFadePower;
	if( bCrossFade )
	{
		if( in_fEstimatedLength < 50.f )
		{
			return;
		}
	}
	else if( eMode != Transition_TriggerRate )
	{
		return;
	}
	if( !m_ulNextElementToPlay )
	{
		return;
	}
	CAkActionPlayAndContinue * pAction = CAkActionPlayAndContinue::Create( (AkActionType)AkActionType_PlayAndContinue, 0, m_spContList );
	if( pAction )
	{
		AkPendingAction * pPendingAction = NewPendingAction( m_pGameObj );
		if( pPendingAction )
		{
			pAction->m_ulPauseCount = m_ulPauseCount;
			pAction->SetHistory( m_PlayHistoryForNextToPlay );
			pAction->SetElementID( m_ulNextElementToPlay );
			pAction->SetInstigator( m_pInstigator );
			pAction->SetPauseResumeTransition( m_PBTrans.pvPRTrans, m_PBTrans.bIsPRTransFading != 0, pPendingAction );
			pAction->SetPathInfo( &m_PathInfo );
			AkInt32 iDelaySamples;
			if( bCrossFade )
			{
				const AkReal32 fTransitionTime = in_fEstimatedLength * 0.5f <= m_fTransitionTime ? in_fEstimatedLength * 0.5f : m_fTransitionTime;
				iDelaySamples = MillisecondsToSamples( in_fEstimatedLength - fTransitionTime );
				pAction->SetFadeBack( this, (AkTimeMs)fTransitionTime );
			}
			else
			{
				iDelaySamples = MillisecondsToSamples( m_fTransitionTime );
				const AkInt32 iMinimalDelay = m_bTargetIsFeedback ? (AkUInt16)( 384000 / AkAudioLibSettings::g_pipelineCoreFrequency ) : AK_NUM_VOICE_REFILL_FRAMES;
				if( iDelaySamples < iMinimalDelay )
				{
					iDelaySamples = iMinimalDelay;
				}
				const AkInt32 iFrameOffset = m_iFrameOffset + iMinimalDelay;
				if( iFrameOffset > 0 )
				{
					iDelaySamples += iFrameOffset;
				}
				pAction->SetPlayStopTransition( m_PBTrans.pvPSTrans, m_PBTrans.bIsPSTransFading != 0, pPendingAction );
			}
			if( pAction->SetAkProp( AkPropID_DelayTime, iDelaySamples, 0, 0 ) == AK_Success )
			{
				pPendingAction->pAction = pAction;
				pPendingAction->UserParam = m_UserParams;
				AKDIAG( L"akdiag: continuous PBI %u: next 0x%08x scheduled at %d samples (%s)", GetPBIID(), m_ulNextElementToPlay, iDelaySamples, bCrossFade ? L"cross-fade" : L"trigger rate" );
				g_pAudioMgr->EnqueueOrExecuteAction( pPendingAction );
				if( iDelaySamples >= AK_NUM_VOICE_REFILL_FRAMES && m_ulPauseCount )
				{
					g_pAudioMgr->PausePending( pPendingAction );
				}
			}
			else
			{
				FreePendingAction( pPendingAction );
			}
		}
		pAction->Release();
		m_bIsNotifyEndReachedContinuous = 1;
	}
	if( m_spContList )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	m_ulNextElementToPlay = AK_INVALID_UNIQUE_ID;
}

/** 2012 0x94c8a0: a sample-accurate container's next item is posted now, chained to this PBI's sequence id. */
void CAkContinuousPBI::PrepareSampleAccurateTransition()
{
	if( m_bWasStopped )
	{
		return;
	}
	PrepareNextToPlay( true );
	if( TransitionMode() != Transition_SampleAccurate || !m_ulNextElementToPlay )
	{
		return;
	}
	CAkActionPlayAndContinue * pAction = CAkActionPlayAndContinue::Create( (AkActionType)AkActionType_PlayAndContinue, 0, m_spContList );
	if( pAction )
	{
		AkPendingAction * pPendingAction = NewPendingAction( m_pGameObj );
		if( pPendingAction )
		{
			pAction->m_ulPauseCount = m_ulPauseCount;
			pAction->SetHistory( m_PlayHistoryForNextToPlay );
			pAction->SetElementID( m_ulNextElementToPlay );
			pAction->SetInstigator( m_pInstigator );
			if( pAction->SetPlayStopTransition( m_PBTrans.pvPSTrans, m_PBTrans.bIsPSTransFading != 0, pPendingAction ) == AK_Success
				&& pAction->SetPauseResumeTransition( m_PBTrans.pvPRTrans, m_PBTrans.bIsPRTransFading != 0, pPendingAction ) == AK_Success )
			{
				pAction->SetPathInfo( &m_PathInfo );
				if( m_bWasPaused )
				{
					pAction->StartAsPaused();
				}
				pAction->SetSAInfo( m_SeqID );
				pPendingAction->pAction = pAction;
				pPendingAction->UserParam = m_UserParams;
				AKDIAG( L"akdiag: continuous PBI %u: next 0x%08x sample-accurate (sequence %u)", GetPBIID(), m_ulNextElementToPlay, m_SeqID );
				g_pAudioMgr->EnqueueOrExecuteAction( pPendingAction );
			}
			else
			{
				FreePendingAction( pPendingAction );
			}
		}
		pAction->Release();
		m_bIsNotifyEndReachedContinuous = 1;
	}
	if( m_spContList )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	m_ulNextElementToPlay = AK_INVALID_UNIQUE_ID;
}

/** 2012 0x94ca30: a stopped item ends its pass. */
void CAkContinuousPBI::_Stop( AkPBIStopMode in_eStopMode, bool in_bIsFromTransition, bool in_bHasNotStarted )
{
	m_bIsNextPrepared = 1;
	m_ulNextElementToPlay = AK_INVALID_UNIQUE_ID;
	if( m_spContList )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	CAkPBI::_Stop( in_eStopMode, in_bIsFromTransition, in_bHasNotStarted );
}

/** 2012 0x94cae0: the item ended - none / delay / sample-accurate (when not posted before) / failed plays post the next
    item here (a failed or never-started item waits at least the look-ahead plus 10 frames). */
void CAkContinuousPBI::Term( bool in_bFailedToInit )
{
	DecrementPlayCount();
	PrepareNextToPlay( false );
	if( !m_ulNextElementToPlay && g_pAudioMgr )
	{
		g_pAudioMgr->ClearCrossFadeOccurence( this );
	}
	const AkTransitionMode eMode = TransitionMode();
	if( m_ulNextElementToPlay && !m_bTerminatedByStop && ( !m_bWasPreStopped || ( eMode != Transition_CrossFadeAmp && eMode != Transition_CrossFadePower ) ) )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulNextElementToPlay );
		if( pNode )
		{
			CAkActionPlayAndContinue * pAction = CAkActionPlayAndContinue::Create( (AkActionType)AkActionType_PlayAndContinue, 0, m_spContList );
			if( pAction )
			{
				pAction->m_ulPauseCount = m_ulPauseCount;
				pAction->SetHistory( m_PlayHistoryForNextToPlay );
				pAction->SetElementID( pNode->key );
				pAction->SetInstigator( m_pInstigator );
				AkPendingAction * pPendingAction = NewPendingAction( m_pGameObj );
				if( pPendingAction )
				{
					AKRESULT eResult = AK_Fail;
					if( pAction->SetPlayStopTransition( m_PBTrans.pvPSTrans, m_PBTrans.bIsPSTransFading != 0, pPendingAction ) == AK_Success
						&& pAction->SetPauseResumeTransition( m_PBTrans.pvPRTrans, m_PBTrans.bIsPRTransFading != 0, pPendingAction ) == AK_Success )
					{
						pAction->SetPathInfo( &m_PathInfo );
						if( m_bPlayFailed || in_bFailedToInit )
						{
							pAction->SetIsFirstPlay( m_bIsFirstPlay );
							const AkInt32 iMinDelay = ( g_settings.uContinuousPlaybackLookAhead + 10 ) * AK_NUM_VOICE_REFILL_FRAMES;
							AkInt32 iDelay = MillisecondsToSamples( m_fTransitionTime );
							if( eMode != Transition_Delay || iDelay < iMinDelay )
							{
								iDelay = iMinDelay;
							}
							eResult = pAction->SetAkProp( AkPropID_DelayTime, iDelay, 0, 0 );
						}
						else if( eMode == Transition_Delay )
						{
							eResult = pAction->SetAkProp( AkPropID_DelayTime, MillisecondsToSamples( m_fTransitionTime ), 0, 0 );
						}
						else
						{
							if( eMode == Transition_SampleAccurate )
							{
								pAction->SetSAInfo( m_SeqID );
							}
							eResult = AK_Success;
						}
					}
					if( eResult == AK_Success )
					{
						pPendingAction->pAction = pAction;
						pPendingAction->UserParam = m_UserParams;
						AKDIAG( L"akdiag: continuous PBI %u ended: next 0x%08x posted (transition mode %d, %.0f ms)", GetPBIID(), pNode->key, (AkInt32)eMode, m_fTransitionTime );
						g_pAudioMgr->EnqueueOrExecuteAction( pPendingAction );
						if( m_fTransitionTime != 0.f && m_ulPauseCount && !m_PBTrans.pvPRTrans )
						{
							g_pAudioMgr->PausePending( pPendingAction );
						}
					}
					else
					{
						FreePendingAction( pPendingAction );
					}
				}
				pAction->Release();
			}
			if( m_bNeedNotifyEndReached )
			{
				m_bIsNotifyEndReachedContinuous = 1;
			}
			pNode->Release();
		}
	}
	if( m_spContList )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	if( m_bNeedNotifyEndReached && m_bIsNotifyEndReachedContinuous )
	{
		m_bNeedNotifyEndReached = 0;
	}
	CAkPBI::Term( in_bFailedToInit );
}

/** 2012 0x94cdd0: a play-to-end of an ancestor drops the continuation items at and under that node. */
void CAkContinuousPBI::PlayToEnd( CAkParameterNodeBase * in_pNode )
{
	CAkParameterNode * pNextToPlay = m_ulNextElementToPlay ? static_cast<CAkParameterNode *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulNextElementToPlay ) ) : NULL;
	if( !m_bIsNextPrepared || !m_ulNextElementToPlay || pNextToPlay->IsOrIsChildOf( in_pNode->key, false ) )
	{
		m_bIsNextPrepared = 0;
		m_ulNextElementToPlay = AK_INVALID_UNIQUE_ID;
		if( m_spContList )
		{
			while( !m_spContList->m_listItems.IsEmpty() )
			{
				CAkContinueListItem & rItem = *( m_spContList->m_listItems.m_pItemsEnd - 1 );
				if( rItem.m_pMultiPlayNode )
				{
					m_spContList->m_listItems.RemoveAll();
				}
				else
				{
					if( !rItem.m_pContainer->IsOrIsChildOf( in_pNode->key, false ) )
					{
						break;
					}
					if( --m_PlayHistoryForNextToPlay.HistArray.uiArraySize )
					{
						do
						{
							const AkUInt32 uLast = m_PlayHistoryForNextToPlay.HistArray.uiArraySize - 1;
							if( uLast < 32 && m_PlayHistoryForNextToPlay.arrayIsContinuous.IsSet( uLast ) )
							{
								break;
							}
							--m_PlayHistoryForNextToPlay.HistArray.uiArraySize;
						}
						while( m_PlayHistoryForNextToPlay.HistArray.uiArraySize );
					}
					m_spContList->RemoveLast();
				}
			}
			if( m_spContList->m_listItems.IsEmpty() )
			{
				CAkContinuationList * pOld = m_spContList;
				m_spContList = NULL;
				pOld->Release();
			}
		}
	}
	if( pNextToPlay )
	{
		pNextToPlay->Release();
	}
	CAkPBI::PlayToEnd( in_pNode );
}
