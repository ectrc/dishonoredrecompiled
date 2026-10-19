/*=============================================================================
	AkActionPlayAndContinue.cpp - CAkActionPlayAndContinue (akactionplayandcontinue.cpp, 2012 0x9493f0..0x949b30) and
	CAkActionPlay::GetHistArray (akactionplay.cpp); 2013 matched by bytes (ih1_inventory.csv). Resume, UnsetFadeBack and
	NeedNotifyDelay stay with the audio manager (AkAudioMgr.cpp, IH2).

	DISHONORED(port): agent IH3, fork C (wave 26).
	Deviation (named): the MONITOR_* notification Execute sends on a cross-fade is compiled out (no profiler).
=============================================================================*/
#include "AkActionPlayAndContinue.h"
#include "AkContinuousPBI.h"
#include "AkLEngineCmds.h"
#include "AkPath.h"

/** 2012 0x949600. */
CAkActionPlayAndContinue::CAkActionPlayAndContinue( AkActionType in_eActionType, AkUniqueID in_ulID, CAkContinuationList *& in_spContList )
	: CAkActionPlay( in_eActionType, in_ulID )
	, m_spContList( in_spContList )
	, m_pPreviousPBI( NULL )
	, m_FadeOutTimeForLast( 0 )
	, m_InitialPlaybackState( PB_Playing )
	, m_SA_PBIID( 0 )
	, m_bNeedNotifyDelay( true )
	, m_bIsfirstPlay( false )
	, m_pTransitionOwner( NULL )
	, m_pInstigator( NULL )
	, m_ulPauseCount( 0 )
	, m_pPathOwner( NULL )
{
	if( m_spContList )
	{
		m_spContList->AddRef();
	}
	m_PlayHistory.arrayIsContinuous.m_i32BitArray = 0;
	m_PlayHistory.HistArray.uiArraySize = 0;
	m_PathInfo.pPBPath = NULL;
	m_PathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
}

/** 2012 0x949690. */
CAkActionPlayAndContinue::~CAkActionPlayAndContinue()
{
	if( m_PBTrans.pvPSTrans && m_pTransitionOwner )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, m_pTransitionOwner );
	}
	m_PBTrans.bIsPSTransFading = 0;
	m_PBTrans.pvPSTrans = NULL;
	if( m_PBTrans.pvPRTrans && m_pTransitionOwner )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, m_pTransitionOwner );
	}
	m_PBTrans.bIsPRTransFading = 0;
	m_PBTrans.pvPRTrans = NULL;
	if( m_PathInfo.pPBPath )
	{
		g_pPathManager->RemovePotentialUser( m_PathInfo.pPBPath );
	}
	if( m_pPathOwner )
	{
		m_pPathOwner->Release();
		m_pPathOwner = NULL;
	}
	if( m_pInstigator )
	{
		m_pInstigator->Release();
	}
	if( m_spContList )
	{
		m_spContList->Release();
		m_spContList = NULL;
	}
}

/** 2012 0x949740: indexed like any action so the pending-action lists can AddRef / Release it. */
CAkActionPlayAndContinue * CAkActionPlayAndContinue::Create( AkActionType in_eActionType, AkUniqueID in_ulID, CAkContinuationList *& in_spContList )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkActionPlayAndContinue ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkActionPlayAndContinue * pAction = AkPlacementNew( pMem ) CAkActionPlayAndContinue( in_eActionType, in_ulID, in_spContList );
	pAction->AddToIndex();
	return pAction;
}

/** 2012 0x9493f0 (CAkActionPlay::GetHistArray's override; monitoring only). */
void CAkActionPlayAndContinue::GetHistArray( AkCntrHistArray & out_rHistArray )
{
	out_rHistArray = m_PlayHistory.HistArray;
}

/** 2012 0x949410. */
void CAkActionPlayAndContinue::SetHistory( PlayHistory & in_rPlayHistory )
{
	m_PlayHistory = in_rPlayHistory;
}

/** 2012 0x949430. */
void CAkActionPlayAndContinue::SetInitialPlaybackState( AkPlaybackState in_eInitialPlaybackState )
{
	m_InitialPlaybackState = in_eInitialPlaybackState;
}

/** 2012 0x949440. */
void CAkActionPlayAndContinue::SetInstigator( CAkPBIAware * in_pInstigator )
{
	m_pInstigator = in_pInstigator;
	in_pInstigator->AddRef();
}

/** 2012 0x949470. */
void CAkActionPlayAndContinue::SetIsFirstPlay( bool in_bIsFirstPlay )
{
	m_bIsfirstPlay = in_bIsFirstPlay;
}

/** 2012 0x949480: Execute fades this PBI out over the cross-fade. */
void CAkActionPlayAndContinue::SetFadeBack( CAkContinuousPBI * in_pPBIToNotify, AkTimeMs in_CrossFadeTime )
{
	m_pPreviousPBI = in_pPBIToNotify;
	m_FadeOutTimeForLast = in_CrossFadeTime;
	m_bNeedNotifyDelay = false;
}

/** 2012 0x9494a0. */
void CAkActionPlayAndContinue::SetSAInfo( AkUniqueID in_seqID )
{
	m_SA_PBIID = in_seqID;
}

/** 2012 0x9494d0. */
void CAkActionPlayAndContinue::StartAsPaused()
{
	m_InitialPlaybackState = PB_Paused;
}

/** 2012 0x949520: the pending action becomes the user of the inherited play / stop fade. */
AKRESULT CAkActionPlayAndContinue::SetPlayStopTransition( CAkTransition * in_pTransition, bool in_bTransitionFading, AkPendingAction * in_pTransitionOwner )
{
	m_pTransitionOwner = in_pTransitionOwner;
	if( m_PBTrans.pvPSTrans && in_pTransitionOwner )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, in_pTransitionOwner );
	}
	m_PBTrans.bIsPSTransFading = 0;
	m_PBTrans.pvPSTrans = NULL;
	if( in_pTransition )
	{
		const AKRESULT eResult = g_pTransitionManager->AddTransitionUser( in_pTransition, m_pTransitionOwner );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	m_PBTrans.pvPSTrans = in_pTransition;
	m_PBTrans.bIsPSTransFading = in_bTransitionFading;
	return AK_Success;
}

/** 2012 0x949590. */
AKRESULT CAkActionPlayAndContinue::SetPauseResumeTransition( CAkTransition * in_pTransition, bool in_bTransitionFading, AkPendingAction * in_pTransitionOwner )
{
	m_pTransitionOwner = in_pTransitionOwner;
	if( m_PBTrans.pvPRTrans && in_pTransitionOwner )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, in_pTransitionOwner );
	}
	m_PBTrans.bIsPRTransFading = 0;
	m_PBTrans.pvPRTrans = NULL;
	if( in_pTransition )
	{
		const AKRESULT eResult = g_pTransitionManager->AddTransitionUser( in_pTransition, m_pTransitionOwner );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	m_PBTrans.pvPRTrans = in_pTransition;
	m_PBTrans.bIsPRTransFading = in_bTransitionFading;
	return AK_Success;
}

/** 2012 0x949b30: the path keeps one more potential user and its owner node stays referenced. */
void CAkActionPlayAndContinue::SetPathInfo( AkPathInfo * in_pPathInfo )
{
	m_PathInfo.pPBPath = NULL;
	m_PathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
	if( in_pPathInfo->pPBPath )
	{
		g_pPathManager->AddPotentialUser( in_pPathInfo->pPBPath );
	}
	m_PathInfo = *in_pPathInfo;
	if( m_pPathOwner )
	{
		m_pPathOwner->Release();
		m_pPathOwner = NULL;
	}
	m_pPathOwner = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_pPathInfo->PathOwnerID );
}

/** 2012 0x949790: a Break targeting in_nodeID - the continuation items at and under it are dropped, and the action
    switches to the next item of the first container above it (false: keep the action), else it is broken (true). */
bool CAkActionPlayAndContinue::BreakToNode( AkUniqueID in_nodeID, CAkRegisteredObj * in_pGameObj, AkPendingAction * )
{
	if( !m_spContList )
	{
		return true;
	}
	while( !m_spContList->m_listItems.IsEmpty() )
	{
		CAkContinueListItem & rItem = *( m_spContList->m_listItems.m_pItemsEnd - 1 );
		if( rItem.m_pMultiPlayNode )
		{
			CAkContinuationList * l_spContList = NULL;
			rItem.m_pMultiPlayNode->ContGetList( rItem.m_pAlternateContList, l_spContList );
			m_spContList->RemoveLast();
			CAkContinuationList * pOld = m_spContList;
			m_spContList = l_spContList;
			if( pOld )
			{
				pOld->Release();
			}
			if( !m_spContList )
			{
				return true;
			}
			continue;
		}
		if( !rItem.m_pContainer->IsOrIsChildOf( in_nodeID, false ) )
		{
			AkUInt16 wPositionSelected;
			AkUniqueID uSelectedNodeID;
			CAkParameterNodeBase * pNextToPlay = rItem.m_pContainer->GetNextToPlayContinuous( in_pGameObj, wPositionSelected, uSelectedNodeID, rItem.m_pContainerInfo, rItem.m_LoopingInfo );
			if( !pNextToPlay )
			{
				break;
			}
			m_PlayHistory.HistArray.aCntrHist[ m_PlayHistory.HistArray.uiArraySize - 1 ] = wPositionSelected;
			m_ulElementID = pNextToPlay->key;
			pNextToPlay->Release();
			return false;
		}
		if( --m_PlayHistory.HistArray.uiArraySize )
		{
			do
			{
				const AkUInt32 uLast = m_PlayHistory.HistArray.uiArraySize - 1;
				if( uLast < 32 && m_PlayHistory.arrayIsContinuous.IsSet( uLast ) )
				{
					break;
				}
				--m_PlayHistory.HistArray.uiArraySize;
			}
			while( m_PlayHistory.HistArray.uiArraySize );
		}
		m_spContList->RemoveLast();
	}
	if( m_spContList->m_listItems.IsEmpty() )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	return true;
}

/** 2012 0x949930: a cross-fade first fades the previous item out (sine for the power curve), then the element plays as
    a continuous PBI carrying the continuation list, transitions, path and sample-accurate sequence id. */
AKRESULT CAkActionPlayAndContinue::Execute( AkPendingAction * in_pAction )
{
	TransParams transParams;
	transParams.TransitionTime = m_FadeOutTimeForLast;
	transParams.eFadeCurve = AkCurveInterpolation_Linear;
	if( m_pPreviousPBI )
	{
		if( m_pPreviousPBI->TransitionMode() == Transition_CrossFadePower )
		{
			transParams.eFadeCurve = AkCurveInterpolation_Sine;
		}
		m_pPreviousPBI->_Stop( transParams, true );
	}
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( !pNode )
	{
		return AK_IDNotFound;
	}
	ContParams Cparameters;
	Cparameters.pPlayStopTransition = m_PBTrans.pvPSTrans;
	Cparameters.pPauseResumeTransition = m_PBTrans.pvPRTrans;
	Cparameters.pPathInfo = &m_PathInfo;
	Cparameters.bIsPlayStopTransitionFading = m_PBTrans.bIsPSTransFading != 0;
	Cparameters.bIsPauseResumeTransitionFading = m_PBTrans.bIsPRTransFading != 0;
	Cparameters.ulPauseCount = m_ulPauseCount;
	Cparameters.spContList = m_spContList;
	if( Cparameters.spContList )
	{
		Cparameters.spContList->AddRef();
	}
	AkPBIParams pbiParams;
	pbiParams.eType = AkPBIParams::ContinuousPBI;
	pbiParams.pInstigator = m_pInstigator;
	pbiParams.pGameObj = in_pAction->pGameObj;
	pbiParams.pTransitionParameters = &transParams;
	pbiParams.userParams = in_pAction->UserParam;
	pbiParams.playHistory = m_PlayHistory;
	pbiParams.ePlaybackState = m_InitialPlaybackState;
	pbiParams.uFrameOffset = in_pAction->LaunchFrameOffset;
	pbiParams.pContinuousParams = &Cparameters;
	pbiParams.sequenceID = m_SA_PBIID;
	pbiParams.bTargetFeedback = false;
	pbiParams.bIsFirst = m_bIsfirstPlay;
	AKDIAG( L"akdiag: PlayAndContinue 0x%08x on game object %u (sequence %u, %s)", m_ulElementID, in_pAction->pGameObj ? (AkUInt32)in_pAction->pGameObj->m_GameObjID : 0, m_SA_PBIID, m_pPreviousPBI ? L"cross-fade" : L"next" );
	const AKRESULT eResult = pNode->Play( pbiParams );
	++CAkLEngineCmds::m_ulPlayEventID;
	if( m_spContList )
	{
		CAkContinuationList * pOld = m_spContList;
		m_spContList = NULL;
		pOld->Release();
	}
	if( m_PathInfo.pPBPath )
	{
		g_pPathManager->RemovePotentialUser( m_PathInfo.pPBPath );
		m_PathInfo.pPBPath = NULL;
		m_PathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
	}
	if( m_pPathOwner )
	{
		m_pPathOwner->Release();
		m_pPathOwner = NULL;
	}
	pNode->Release();
	if( Cparameters.spContList )
	{
		Cparameters.spContList->Release();
	}
	return eResult;
}
