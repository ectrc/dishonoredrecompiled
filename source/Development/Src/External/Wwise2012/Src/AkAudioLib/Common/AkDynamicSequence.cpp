/*=============================================================================
	AkDynamicSequence.cpp - CAkDynamicSequence (akdynamicsequence.cpp, 2012 0x928b50..0x9296f0), CAkDynamicSequencePBI
	(akdynamicsequencepbi.cpp, 0x94cf60..0x94d1f0) and PlaylistItem::operator= (0x928c40); 2013 matched by bytes
	(ih1_inventory.csv). The API (AK::SoundEngine::DynamicSequence) is in AkSilentSoundEngine.cpp next to PostEvent (it
	shares the playing-id counter), its two messages in CAkAudioMgr::ProcessMsgQueue.

	DISHONORED(port): agent IH3, fork C (wave 26). Not reached by Dishonored (no caller of the API in the glue).
	Deviations (named):
	  * CAkDynamicSequencePBI's ctor reads the sequence's queued item (audio node, custom info) by name; the 2012
	    decompile shows the reads at instigator +112 / +116 through a CAkIndexable-typed pointer.
	  * The audio-thread message UnlockPlaylist posts (type 25, command 6 = ResumeWaiting) is built with the port's
	    AkQueuedMsg_DynamicSequenceCmd record and its SizeofBody (retail: a folded Sizeof helper).
=============================================================================*/
#include "AkDynamicSequence.h"
#include "AkPlayingMgr.h"
#include "AkLEngineCmds.h"

AK::SoundEngine::DynamicSequence::PlaylistItem & AK::SoundEngine::DynamicSequence::PlaylistItem::operator=( const PlaylistItem & in_rCopy )
{
	if( pExternalSrcs )
	{
		pExternalSrcs->Release();
	}
	audioNodeID = in_rCopy.audioNodeID;
	msDelay = in_rCopy.msDelay;
	pCustomInfo = in_rCopy.pCustomInfo;
	pExternalSrcs = in_rCopy.pExternalSrcs;
	if( pExternalSrcs )
	{
		pExternalSrcs->AddRef();
	}
	return *this;
}

// ------------------------------------------------------------------------------------------------------------
// CAkDynamicSequence
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x928dc0. */
CAkDynamicSequence::CAkDynamicSequence( AkPlayingID in_playingID, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType )
	: CAkPBIAware( in_playingID )
	, m_eState( State_Stopped )
	, m_pGameObj( NULL )
	, m_bClosed( false )
	, m_eDynamicSequenceType( in_eDynamicSequenceType )
	, m_ulPauseCount( 0 )
	, m_uSequenceID( 0 )
{
	m_userParams.m_PlayingID = in_playingID;
}

/** 2012 0x928f70. */
CAkDynamicSequence::~CAkDynamicSequence()
{
	m_playList.Term();
	if( m_pGameObj && --m_pGameObj->m_refCount <= 0 )
	{
		m_pGameObj->~CAkRegisteredObj();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pGameObj );
	}
}

/** 2012 0x928c80. */
AkUInt32 CAkDynamicSequence::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxDynamicSequences.m_IndexLock );
	return ++m_lRef;
}

/** 2012 0x929010. */
AkUInt32 CAkDynamicSequence::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxDynamicSequences.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxDynamicSequences.RemoveID( key );
		this->~CAkDynamicSequence();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x9290c0. */
AKRESULT CAkDynamicSequence::Init()
{
	g_pIndex->m_idxDynamicSequences.SetIDToPtr( this );
	return AK_Success;
}

/** 2012 0x929470. */
CAkDynamicSequence * CAkDynamicSequence::Create( AkPlayingID in_PlayingID, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkDynamicSequence ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkDynamicSequence * pSequence = AkPlacementNew( pMem ) CAkDynamicSequence( in_PlayingID, in_eDynamicSequenceType );
	if( pSequence->Init() != AK_Success )
	{
		pSequence->Release();
		return NULL;
	}
	return pSequence;
}

/** 2012 0x928b50. */
CAkPBI * CAkDynamicSequence::CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkDynamicSequencePBI ) );
	if( !pMem )
	{
		return NULL;
	}
	return AkPlacementNew( pMem ) CAkDynamicSequencePBI( in_pSound, in_pSource, in_rPBIParams.pGameObj, *in_rPBIParams.pContinuousParams, in_rPBIParams.userParams, in_rPBIParams.playHistory, in_rPBIParams.bIsFirst, in_rPBIParams.sequenceID, in_rPBIParams.pInstigator, in_rPriority, m_eDynamicSequenceType, in_pAMLimiter, in_pBusLimiter );
}

/** 2012 0x928bc0. */
void CAkDynamicSequence::SetGameObject( CAkRegisteredObj * in_pGameObj )
{
	m_pGameObj = in_pGameObj;
	if( in_pGameObj )
	{
		++in_pGameObj->m_refCount;
	}
}

/** 2012 0x928be0: a sequence waiting for items resumes on the audio thread. */
void CAkDynamicSequence::UnlockPlaylist()
{
	const bool bWaiting = m_eState == State_Waiting;
	m_lockPlaylist.Unlock();
	if( bWaiting )
	{
		AkQueuedMsg Item;
		Item.type = QueuedMsgType_DynamicSequenceCmd;
		AddRef();
		Item.dynamicsequencecmd.pDynamicSequence = this;
		Item.dynamicsequencecmd.eCommand = AkQueuedMsg_DynamicSequenceCmd::ResumeWaiting;
		Item.dynamicsequencecmd.uTransitionDuration = 0;
		Item.dynamicsequencecmd.eFadeCurve = AkCurveInterpolation_Log3;
		g_pAudioMgr->Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_DynamicSequenceCmd>() );
	}
}

/** 2012 0x928e40: the next playlist item (items with no node are skipped and reported ended); none -> waiting. */
AkUniqueID CAkDynamicSequence::_GetNextToPlay( AkTimeMs & out_delay, void *& out_rpCustomParam )
{
	if( !m_pGameObj )
	{
		return AK_INVALID_UNIQUE_ID;
	}
	m_queuedItem.audioNodeID = AK_INVALID_UNIQUE_ID;
	m_lockPlaylist.Lock();
	if( m_eState != State_Stopped )
	{
		while( !m_playList.IsEmpty() )
		{
			AK::SoundEngine::DynamicSequence::PlaylistItem & rItem = m_playList[ 0 ];
			out_rpCustomParam = rItem.pCustomInfo;
			if( rItem.audioNodeID )
			{
				m_queuedItem = rItem;
				out_delay = rItem.msDelay;
				m_playList.Erase( 0 );
				break;
			}
			m_playList.Erase( 0 );
			g_pPlayingMgr->NotifyEndOfDynamicSequenceItem( m_userParams.m_PlayingID, AK_INVALID_UNIQUE_ID, out_rpCustomParam );
		}
		if( !m_queuedItem.audioNodeID )
		{
			m_eState = State_Waiting;
		}
	}
	const AkUniqueID audioNodeID = m_queuedItem.audioNodeID;
	m_lockPlaylist.Unlock();
	return audioNodeID;
}

/** 2012 0x928f00: the queued item becomes the playing one; the caller's user params take the next item's sources. */
AkUniqueID CAkDynamicSequence::GetNextToPlay( AkTimeMs & out_delay, void *& out_rpCustomParam, UserParams & out_rUserParam )
{
	m_playingItem = m_queuedItem;
	const AkUniqueID nextID = _GetNextToPlay( out_delay, out_rpCustomParam );
	AkExternalSourceArray * pExternalSrcs = m_queuedItem.pExternalSrcs;
	if( out_rUserParam.m_CustomParam.pExternalSrcs )
	{
		out_rUserParam.m_CustomParam.pExternalSrcs->Release();
	}
	if( pExternalSrcs )
	{
		pExternalSrcs->AddRef();
	}
	out_rUserParam.m_CustomParam.pExternalSrcs = pExternalSrcs;
	return nextID;
}

/** 2012 0x929110. */
AKRESULT CAkDynamicSequence::_Stop( AkDynamicSequenceStopMode in_eStopMode, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	m_eState = State_Stopped;
	m_ulPauseCount = 0;
	g_pAudioMgr->ClearPendingItems( m_userParams.m_PlayingID );
	if( m_queuedItem.audioNodeID && in_eStopMode == AK_StopImmediate )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_queuedItem.audioNodeID );
		if( pNode )
		{
			pNode->Stop( m_pGameObj, m_userParams.m_PlayingID, in_uTransitionDuration, in_eFadeCurve );
			pNode->Release();
		}
		m_queuedItem.audioNodeID = AK_INVALID_UNIQUE_ID;
	}
	if( m_playingItem.audioNodeID )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_playingItem.audioNodeID );
		if( pNode )
		{
			if( in_eStopMode == AK_StopImmediate )
			{
				pNode->Stop( m_pGameObj, m_userParams.m_PlayingID, in_uTransitionDuration, in_eFadeCurve );
			}
			else
			{
				pNode->PlayToEnd( m_pGameObj, m_playingItem.audioNodeID, m_userParams.m_PlayingID );
			}
			pNode->Release();
		}
	}
	return AK_Success;
}

/** 2012 0x9291f0. */
AKRESULT CAkDynamicSequence::Pause( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	if( m_eState == State_Stopped )
	{
		return AK_Success;
	}
	if( ++m_ulPauseCount == 1 )
	{
		g_pAudioMgr->PausePendingItems( m_userParams.m_PlayingID );
		const AkUniqueID aNodes[ 2 ] = { m_playingItem.audioNodeID, m_queuedItem.audioNodeID };
		for( AkUInt32 i = 0; i < 2; ++i )
		{
			CAkParameterNodeBase * pNode = aNodes[ i ] ? g_pIndex->m_idxAudioNode.GetPtrAndAddRef( aNodes[ i ] ) : NULL;
			if( pNode )
			{
				pNode->Pause( m_pGameObj, m_userParams.m_PlayingID, in_uTransitionDuration, in_eFadeCurve );
				pNode->Release();
			}
		}
	}
	return AK_Success;
}

/** 2012 0x9292a0: one item played as a dynamic-sequence PBI (sample-accurate sequences share one sequence id). */
AKRESULT CAkDynamicSequence::_PlayNode( AkUniqueID in_nodeID, AkTimeMs in_delay, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	if( !m_pGameObj )
	{
		return AK_Fail;
	}
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_nodeID );
	if( !pNode )
	{
		return AK_IDNotFound;
	}
	TransParams Tparameters;
	Tparameters.TransitionTime = in_uTransitionDuration;
	Tparameters.eFadeCurve = in_eFadeCurve;
	AkPathInfo PathInfo;
	PathInfo.pPBPath = NULL;
	PathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
	ContParams continuousParams;
	continuousParams.pPlayStopTransition = NULL;
	continuousParams.pPauseResumeTransition = NULL;
	continuousParams.pPathInfo = &PathInfo;
	continuousParams.bIsPlayStopTransitionFading = false;
	continuousParams.bIsPauseResumeTransitionFading = false;
	continuousParams.ulPauseCount = 0;
	continuousParams.spContList = CAkContinuationList::Create();
	if( !continuousParams.spContList )
	{
		pNode->Release();
		return AK_Fail;
	}
	AkPBIParams pbiParams;
	pbiParams.eType = AkPBIParams::DynamicSequencePBI;
	pbiParams.pInstigator = this;
	pbiParams.userParams = m_userParams;
	pbiParams.playHistory.HistArray.uiArraySize = 0;
	pbiParams.playHistory.arrayIsContinuous.m_i32BitArray = 0;
	pbiParams.bTargetFeedback = false;
	pbiParams.uFrameOffset = in_delay * ( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	pbiParams.ePlaybackState = PB_Playing;
	pbiParams.bIsFirst = true;
	pbiParams.pGameObj = m_pGameObj;
	pbiParams.pTransitionParameters = &Tparameters;
	pbiParams.pContinuousParams = &continuousParams;
	if( m_eDynamicSequenceType == AK::SoundEngine::DynamicSequence::DynamicSequenceType_SampleAccurate && !m_uSequenceID )
	{
		do
		{
			m_uSequenceID = CAkContinuousPBI::m_CalSeqID++;
		}
		while( !m_uSequenceID );
	}
	pbiParams.sequenceID = m_uSequenceID;
	AKDIAG( L"akdiag: dynamic sequence %u plays node 0x%08x (delay %d ms)", m_userParams.m_PlayingID, in_nodeID, in_delay );
	const AKRESULT eResult = pNode->Play( pbiParams );
	pNode->Release();
	continuousParams.spContList->Release();
	return eResult;
}

/** 2012 0x9294c0: plays the next item; an item that fails is reported ended and the next one tried. */
AKRESULT CAkDynamicSequence::Play( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	if( ( m_eState != State_Stopped && m_eState != State_Waiting ) || m_ulPauseCount )
	{
		return AK_Success;
	}
	m_eState = State_Playing;
	AkTimeMs delay = 0;
	for( ;; )
	{
		void * pCustomInfo = NULL;
		const AkUniqueID nextID = _GetNextToPlay( delay, pCustomInfo );
		AkExternalSourceArray * pExternalSrcs = m_queuedItem.pExternalSrcs;
		if( m_userParams.m_CustomParam.pExternalSrcs )
		{
			m_userParams.m_CustomParam.pExternalSrcs->Release();
		}
		if( pExternalSrcs )
		{
			pExternalSrcs->AddRef();
		}
		m_userParams.m_CustomParam.pExternalSrcs = pExternalSrcs;
		if( !nextID || _PlayNode( nextID, delay, in_uTransitionDuration, in_eFadeCurve ) == AK_Success )
		{
			break;
		}
		g_pPlayingMgr->NotifyEndOfDynamicSequenceItem( m_userParams.m_PlayingID, nextID, pCustomInfo );
	}
	return AK_Success;
}

/** 2012 0x929570. */
AKRESULT CAkDynamicSequence::Stop( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	m_uSequenceID = 0;
	return _Stop( AK_StopImmediate, in_uTransitionDuration, in_eFadeCurve );
}

/** 2012 0x929590: the playing item finishes, nothing follows. */
AKRESULT CAkDynamicSequence::Break()
{
	const bool bStopped = m_eState == State_Stopped;
	m_uSequenceID = 0;
	if( !bStopped )
	{
		m_eState = State_Stopped;
		m_ulPauseCount = 0;
		g_pAudioMgr->ClearPendingItems( m_userParams.m_PlayingID );
		if( m_playingItem.audioNodeID )
		{
			CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_playingItem.audioNodeID );
			if( pNode )
			{
				pNode->PlayToEnd( m_pGameObj, m_playingItem.audioNodeID, m_userParams.m_PlayingID );
				pNode->Release();
			}
		}
	}
	return AK_Success;
}

/** 2012 0x929610. */
AKRESULT CAkDynamicSequence::Resume( AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	if( !m_ulPauseCount )
	{
		return AK_Success;
	}
	if( m_ulPauseCount-- == 1 )
	{
		if( m_eState != State_Playing )
		{
			Play( in_uTransitionDuration, in_eFadeCurve );
		}
		g_pAudioMgr->ResumePausedPendingItems( m_userParams.m_PlayingID );
		const AkUniqueID aNodes[ 2 ] = { m_playingItem.audioNodeID, m_queuedItem.audioNodeID };
		for( AkUInt32 i = 0; i < 2; ++i )
		{
			CAkParameterNodeBase * pNode = aNodes[ i ] ? g_pIndex->m_idxAudioNode.GetPtrAndAddRef( aNodes[ i ] ) : NULL;
			if( pNode )
			{
				pNode->Resume( m_pGameObj, m_userParams.m_PlayingID, in_uTransitionDuration, in_eFadeCurve );
				pNode->Release();
			}
		}
	}
	return AK_Success;
}

/** 2012 0x9296d0. */
void CAkDynamicSequence::ResumeWaiting()
{
	if( m_eState == State_Waiting && !m_playList.IsEmpty() )
	{
		Play( 0, AkCurveInterpolation_Linear );
	}
}

/** 2012 0x9296f0: a stop / pause / resume of everything on the sequence's game object. */
void CAkDynamicSequence::AllExec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj )
{
	if( in_pGameObj && m_pGameObj != in_pGameObj )
	{
		return;
	}
	switch( in_eType )
	{
	case ActionParamType_Stop:
		m_eState = State_Stopped;
		m_ulPauseCount = 0;
		m_queuedItem.audioNodeID = AK_INVALID_UNIQUE_ID;
		break;
	case ActionParamType_Pause:
		if( m_eState != State_Stopped )
		{
			++m_ulPauseCount;
		}
		break;
	case ActionParamType_Resume:
		if( m_ulPauseCount && !--m_ulPauseCount && m_eState != State_Playing )
		{
			Play( 0, AkCurveInterpolation_Linear );
		}
		break;
	default:
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkDynamicSequencePBI
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94cf60. */
CAkDynamicSequencePBI::CAkDynamicSequencePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, ContParams & in_rCparameters, UserParams & in_rUserparams, PlayHistory & in_rPlayHistory, bool in_bIsFirst, AkUniqueID in_SeqID, CAkPBIAware * in_pInstigator, const PriorityInfoCurrent & in_rPriority, AK::SoundEngine::DynamicSequence::DynamicSequenceType in_eDynamicSequenceType, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter )
	: CAkContinuousPBI( in_pSound, in_pSource, in_pGameObj, in_rCparameters, in_rUserparams, in_rPlayHistory, in_bIsFirst, in_SeqID, in_pInstigator, in_rPriority, false, in_pAMLimiter, in_pBusLimiter )
	, m_bRequestNextFromDynSeq( true )
	, m_eDynamicSequenceType( in_eDynamicSequenceType )
{
	const CAkDynamicSequence * pSequence = static_cast<const CAkDynamicSequence *>( m_pInstigator );
	m_startingNode = pSequence->m_queuedItem.audioNodeID;
	m_pDynSecCustomInfo = pSequence->m_queuedItem.pCustomInfo;
}

/** 2012 0x94cff0. */
void CAkDynamicSequencePBI::Term( bool in_bFailedToInit )
{
	if( !in_bFailedToInit )
	{
		g_pPlayingMgr->NotifyEndOfDynamicSequenceItem( m_UserParams.m_PlayingID, m_startingNode, m_pDynSecCustomInfo );
	}
	CAkContinuousPBI::Term( in_bFailedToInit );
}

/** 2012 0x94d060. */
AKRESULT CAkDynamicSequencePBI::PlayNextElement( AkUniqueID in_nextElementID, AkTimeMs in_delay )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_nextElementID );
	if( !pNode )
	{
		return AK_Fail;
	}
	ContParams Cparameters;
	Cparameters.pPathInfo = &m_PathInfo;
	Cparameters.pPauseResumeTransition = m_PBTrans.pvPRTrans;
	Cparameters.pPlayStopTransition = m_PBTrans.pvPSTrans;
	Cparameters.ulPauseCount = m_ulPauseCount;
	Cparameters.bIsPlayStopTransitionFading = m_PBTrans.bIsPSTransFading != 0;
	Cparameters.bIsPauseResumeTransitionFading = m_PBTrans.bIsPRTransFading != 0;
	Cparameters.spContList = CAkContinuationList::Create();
	TransParams Tparameters;
	Tparameters.TransitionTime = 0;
	Tparameters.eFadeCurve = AkCurveInterpolation_Constant;
	AkPBIParams pbiParams;
	pbiParams.eType = AkPBIParams::DynamicSequencePBI;
	pbiParams.pInstigator = m_pInstigator;
	pbiParams.userParams = m_UserParams;
	pbiParams.playHistory.HistArray.uiArraySize = 0;
	pbiParams.playHistory.arrayIsContinuous.m_i32BitArray = 0;
	pbiParams.bTargetFeedback = false;
	pbiParams.uFrameOffset = in_delay * ( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	pbiParams.pGameObj = m_pGameObj;
	pbiParams.pContinuousParams = &Cparameters;
	pbiParams.sequenceID = m_SeqID;
	pbiParams.ePlaybackState = PB_Playing;
	pbiParams.bIsFirst = false;
	pbiParams.pTransitionParameters = &Tparameters;
	const AKRESULT eResult = pNode->Play( pbiParams );
	if( m_bNeedNotifyEndReached )
	{
		m_bIsNotifyEndReachedContinuous = 1;
	}
	pNode->Release();
	++CAkLEngineCmds::m_ulPlayEventID;
	if( Cparameters.spContList )
	{
		Cparameters.spContList->Release();
	}
	return eResult;
}

/** 2012 0x94d1f0: when nothing continues inside the item, the sequence's next item plays (a sample-accurate sequence
    asks already on the preliminary call). */
void CAkDynamicSequencePBI::PrepareNextToPlay( bool in_bIsPreliminary )
{
	CAkContinuousPBI::PrepareNextToPlay( in_bIsPreliminary );
	if( !m_bIsNextPrepared || m_bWasStopped )
	{
		return;
	}
	if( m_ulNextElementToPlay )
	{
		m_bRequestNextFromDynSeq = false;
		return;
	}
	if( !m_bRequestNextFromDynSeq || ( in_bIsPreliminary && m_eDynamicSequenceType != AK::SoundEngine::DynamicSequence::DynamicSequenceType_SampleAccurate ) )
	{
		return;
	}
	CAkDynamicSequence * pSequence = static_cast<CAkDynamicSequence *>( m_pInstigator );
	m_bRequestNextFromDynSeq = false;
	void * pCustomInfo = NULL;
	AkTimeMs delay = 0;
	for( AkUniqueID nextID = pSequence->GetNextToPlay( delay, pCustomInfo, m_UserParams ); nextID; nextID = pSequence->GetNextToPlay( delay, pCustomInfo, m_UserParams ) )
	{
		if( PlayNextElement( nextID, delay ) == AK_Success )
		{
			break;
		}
		g_pPlayingMgr->NotifyEndOfDynamicSequenceItem( m_UserParams.m_PlayingID, nextID, pCustomInfo );
		pCustomInfo = NULL;
		delay = 0;
	}
}
