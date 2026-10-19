/*=============================================================================
	AkAudioMgr.cpp - CAkAudioMgr (akaudiomgr.cpp, 2012 0x921ce0..0x925300), CAkAudioThread (akaudiothread.cpp,
	0x949ba0..0x949c80), AkPendingAction, AkExternalSourceArray (akaudiolib.cpp 0x918520 / 0x9198f0); 2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * ProcessMsgQueue runs the message types AK::SoundEngine (AkSoundEngine.cpp) enqueues. The game-sync messages
	    (RTPC 2/3/5/6, State 7, Switch 8, Trigger 10) are never enqueued before IH3 replaces the AkGameSyncs data
	    stores (the API records them on the game thread, see AkSoundEngine.cpp); the environment volume / bypass
	    messages (20/21) need IH3's CAkEnvironmentsMgr; dynamic sequences (24/25), motion devices (29/30/31), source
	    plug-ins (38) and SetEffect (40) are not reachable from the glue. Seek (37) releases its event without seeking:
	    CAkSound::SeekSound is IH3's.
	  * The monitoring hooks (GetHistArray before a delayed play's notification, MONITOR_*) are compiled out.
	  * -akdiag: every event the queue executes (event, playing id, game object, action count, buffer tick).
=============================================================================*/
#include "AkSink.h"
#include "AkAudioMgr.h"
#include "AkPlayingMgr.h"
#include "AkActionPlayAndContinue.h"
#include "AkRegistryMgr.h"
#include "AkURenderer.h"
#include "AkLEngine.h"
#include "AkLEngineCmds.h"
#include "AkListener.h"
#include "AkBankMgr.h"
#include <time.h>
#include <stdlib.h>

CAkAudioMgr * g_pAudioMgr = NULL;
AkThread CAkAudioThread::m_hEventMgrThread = NULL;
AkThreadID CAkAudioThread::m_hEventMgrThreadID = 0;

extern AkArray<AkGlobalCallbackFunc, AkGlobalCallbackFunc, ArrayPoolDefault, 4> g_aBehavioralExtensions;

// ------------------------------------------------------------------------------------------------------------
// AkExternalSourceArray / AkPendingAction
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9198f0: one allocation, the file names copied. */
AkExternalSourceArray * AkExternalSourceArray::Create( AkUInt32 in_nCount, AkExternalSourceInfo * in_pSrcs )
{
	const AkUInt32 uSize = sizeof( AkExternalSourceArray ) + ( in_nCount - 1 ) * sizeof( AkExternalSourceInfo );
	AkExternalSourceArray * pArray = (AkExternalSourceArray *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uSize );
	if( !pArray )
	{
		return NULL;
	}
	pArray->m_cRefCount = 1;
	pArray->m_nCount = in_nCount;
	memcpy( pArray->m_pSrcs, in_pSrcs, in_nCount * sizeof( AkExternalSourceInfo ) );
	for( AkUInt32 i = 0; i < in_nCount; ++i )
	{
		if( in_pSrcs[ i ].szFile )
		{
			const size_t uLength = wcslen( in_pSrcs[ i ].szFile ) + 1;
			pArray->m_pSrcs[ i ].szFile = (AkOSChar *)AK::MemoryMgr::Malloc( g_DefaultPoolId, (AkUInt32)( uLength * sizeof( AkOSChar ) ) );
			if( pArray->m_pSrcs[ i ].szFile )
			{
				memcpy( pArray->m_pSrcs[ i ].szFile, in_pSrcs[ i ].szFile, uLength * sizeof( AkOSChar ) );
			}
		}
	}
	return pArray;
}

/** 2012 0x918520. */
void AkExternalSourceArray::Release()
{
	if( m_cRefCount-- != 1 )
	{
		return;
	}
	for( AkUInt32 i = 0; i < m_nCount; ++i )
	{
		if( m_pSrcs[ i ].szFile )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pSrcs[ i ].szFile );
		}
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x921d20. */
AkPendingAction::AkPendingAction( CAkRegisteredObj * in_pGameObj )
	: pAction( NULL )
	, LaunchTick( 0 )
	, LaunchFrameOffset( 0 )
	, PausedTick( 0 )
	, ulPauseCount( 0 )
	, TargetPlayingID( AK_INVALID_PLAYING_ID )
	, pGameObj( in_pGameObj )
{
	if( in_pGameObj )
	{
		++in_pGameObj->m_refCount;
	}
}

/** 2012 0x922110. */
AkPendingAction::~AkPendingAction()
{
	if( pGameObj && --pGameObj->m_refCount <= 0 )
	{
		pGameObj->~CAkRegisteredObj();
		AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
	}
}

/** 2012 0x923460: a play-and-continue action's fades (its play/stop and pause/resume transitions). */
void AkPendingAction::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	if( pAction->m_eActionType != AkActionType_PlayAndContinue )
	{
		return;
	}
	CAkActionPlayAndContinue * pPlayAndContinue = static_cast<CAkActionPlayAndContinue *>( pAction );
	switch( in_eTarget )
	{
	case TransTarget_Play:
	case TransTarget_Stop:
		if( in_bIsTerminated )
		{
			pPlayAndContinue->m_PBTrans.bIsPSTransFading = 0;
			pPlayAndContinue->m_PBTrans.pvPSTrans = NULL;
			if( in_eTarget == TransTarget_Stop )
			{
				g_pAudioMgr->StopPending( this );
			}
		}
		break;
	case TransTarget_Pause:
	case TransTarget_Resume:
		if( in_bIsTerminated )
		{
			pPlayAndContinue->m_PBTrans.bIsPRTransFading = 0;
			pPlayAndContinue->m_PBTrans.pvPRTrans = NULL;
			if( in_eTarget == TransTarget_Pause )
			{
				g_pAudioMgr->PausePending( this );
			}
			pPlayAndContinue->m_ulPauseCount = 0;
		}
		break;
	default:
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkActionPlayAndContinue (the members a pending action needs; see the header)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9494e0. */
void CAkActionPlayAndContinue::Resume()
{
	if( m_PBTrans.bIsPRTransFading && m_PBTrans.pvPRTrans )
	{
		if( m_pTransitionOwner )
		{
			g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, m_pTransitionOwner );
		}
		m_PBTrans.bIsPRTransFading = 0;
		m_PBTrans.pvPRTrans = NULL;
	}
}

/** 2012 0x9494b0. */
void CAkActionPlayAndContinue::UnsetFadeBack( CAkContinuousPBI * in_pPBIToCheck )
{
	if( in_pPBIToCheck == m_pPreviousPBI )
	{
		m_pPreviousPBI = NULL;
	}
}

/** 2012 0x949790. DISHONORED(IH3 seam): the continuation-list walk; without a list retail breaks (true). */
bool CAkActionPlayAndContinue::BreakToNode( AkUniqueID in_nodeID, CAkRegisteredObj * in_pGameObj, AkPendingAction * in_pPendingAction )
{
	return true;
}

// ------------------------------------------------------------------------------------------------------------
// CAkAudioThread
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x949bc0: the voice starts, then one Perform per signal or per sink wait time until Stop. */
DWORD WINAPI CAkAudioThread::EventMgrThreadFunc( LPVOID lpParameter )
{
	CAkAudioThread * pThread = (CAkAudioThread *)lpParameter;
	m_hEventMgrThreadID = GetCurrentThreadId();
	srand( (unsigned int)time( NULL ) );
	CAkLEngine::StartVoice();
	DWORD dwWaitResult = WAIT_TIMEOUT;
	do
	{
		if( dwWaitResult == WAIT_OBJECT_0 || dwWaitResult == WAIT_TIMEOUT )
		{
			g_pAudioMgr->Perform();
		}
		dwWaitResult = WaitForSingleObject( pThread->m_eventProcess, g_pAkSink->GetThreadWaitTime() );
	}
	while( !pThread->m_bStopThread );
	return 0;
}

/** 2012 0x949c30: "AK::EventManager" with g_PDSettings.threadLEngine. */
AKRESULT CAkAudioThread::Start()
{
	m_eventProcess = CreateEventW( NULL, FALSE, FALSE, NULL );
	m_bStopThread = false;
	if( !m_eventProcess )
	{
		return AK_Fail;
	}
	AKPLATFORM::AkCreateThread( EventMgrThreadFunc, this, g_PDSettings.threadLEngine, &m_hEventMgrThread, "AK::EventManager" );
	return m_hEventMgrThread ? AK_Success : AK_Fail;
}

/** 2012 0x949c80. */
void CAkAudioThread::Stop()
{
	m_bStopThread = true;
	if( m_hEventMgrThread )
	{
		SetEvent( m_eventProcess );
		WaitForSingleObject( m_hEventMgrThread, INFINITE );
		CloseHandle( m_hEventMgrThread );
		m_hEventMgrThread = NULL;
	}
	if( m_eventProcess )
	{
		CloseHandle( m_eventProcess );
	}
	m_eventProcess = NULL;
}

/** 2012 0x949bb0. */
void CAkAudioThread::WakeupEventsConsumer()
{
	SetEvent( m_eventProcess );
}

// ------------------------------------------------------------------------------------------------------------
// CAkAudioMgr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9222d0. */
CAkAudioMgr::CAkAudioMgr()
	: m_bDrainMsgQueue( false )
	, m_MsgQueueActualSize( 0 )
	, m_uBufferTick( 0 )
	, m_ulWriterFlag( 0 )
	, m_ulReaderFlag( 0 )
{
}

/** 2012 0x922000. */
CAkAudioMgr::~CAkAudioMgr()
{
}

/** 2012 0x922020: the queue holds uCommandQueueSize bytes in 2 KB chunks; 32 pending items of each list pre-allocated. */
AKRESULT CAkAudioMgr::Init()
{
	m_ulWriterFlag = 0;
	m_ulReaderFlag = 0;
	AKRESULT eResult = m_MsgQueue.Init( g_settings.uCommandQueueSize / AkSparseChunkRing::GetChunkSize() );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( m_mmapPending.Init( 32, (AkUInt32)-1 ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	if( m_mmapPausedPending.Init( 32, (AkUInt32)-1 ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	return AK_Success;
}

/** 2012 0x923070. */
void CAkAudioMgr::Term()
{
	m_audioThread.Stop();
	RemoveAllPreallocAndReferences();
	RemoveAllPausedPendingAction();
	RemoveAllPendingAction();
	m_MsgQueue.Term();
	m_mmapPending.Term();
	m_mmapPausedPending.Term();
}

/** 2012 0x921ce0. */
AKRESULT CAkAudioMgr::Start()
{
	m_hEventMgrThreadDrainEvent.m_Event = CreateEventW( NULL, TRUE, FALSE, NULL );
	if( !m_hEventMgrThreadDrainEvent.m_Event )
	{
		return AK_Fail;
	}
	return m_audioThread.Start();
}

/** 2012 0x921d10. */
void CAkAudioMgr::Stop()
{
	m_audioThread.Stop();
}

/** 2012 0x925300. */
AKRESULT CAkAudioMgr::Enqueue( AkQueuedMsg & in_rItem, AkUInt32 in_uSize )
{
	AkAutoLock<CAkLock> lock( m_queueLock );
	return LockedEnqueue( in_rItem, in_uSize );
}

/** 2012 0x9251f0: a full queue makes the writer wait for the audio thread to drain it. */
AKRESULT CAkAudioMgr::LockedEnqueue( AkQueuedMsg & in_rItem, AkUInt32 in_uSize )
{
	in_rItem.size = (AkUInt16)in_uSize;
	AKRESULT eResult = m_MsgQueue.Write( &in_rItem, in_uSize );
	while( eResult == AK_InsufficientMemory )
	{
		if( in_uSize > AkSparseChunkRing::GetChunkSize() )
		{
			return AK_InvalidParameter;
		}
		ResetEvent( m_hEventMgrThreadDrainEvent.m_Event );
		m_bDrainMsgQueue = true;
		m_queueLock.Unlock();
		m_audioThread.WakeupEventsConsumer();
		WaitForSingleObject( m_hEventMgrThreadDrainEvent.m_Event, INFINITE );
		m_queueLock.Lock();
		eResult = m_MsgQueue.Write( &in_rItem, in_uSize );
	}
	return eResult;
}

/** 2012 0x925280: an end-of-list marker closes the batch the game wrote since the last call, and the thread wakes. */
AKRESULT CAkAudioMgr::RenderAudio()
{
	bool bWakeup;
	{
		AkAutoLock<CAkLock> lock( m_queueLock );
		bWakeup = !m_MsgQueue.IsEmpty();
		if( bWakeup )
		{
			AkQueuedMsg endOfList;
			endOfList.type = QueuedMsgType_EndOfList;
			LockedEnqueue( endOfList, AkQueuedMsg::Sizeof_EndOfList() );
			++m_ulWriterFlag;
		}
	}
	if( bWakeup )
	{
		m_audioThread.WakeupEventsConsumer();
	}
	return AK_Success;
}

/** 2012 0x925130: the commands, then one audio frame per buffer the sink wants (the last pass only processes commands). */
void CAkAudioMgr::Perform()
{
	AkAutoLock<CAkLock> gate( g_csMain );
	for( AkUInt32 uBuffersNeeded = CAkLEngine::GetNumBufferNeededAndSubmit(); ; --uBuffersNeeded )
	{
		ProcessMsgQueue();
		ProcessPendingList();
		if( CAkLEngineCmds::m_bProcessPlayCmdsNeeded )
		{
			CAkLEngineCmds::ProcessPlayCommands();
		}
		if( !uBuffersNeeded )
		{
			break;
		}
		for( AkInt32 i = (AkInt32)g_aBehavioralExtensions.Length() - 1; i >= 0; --i )
		{
			g_aBehavioralExtensions[ i ]( false );
		}
		const AkUInt32 uNextTick = m_uBufferTick + 1;
		g_pTransitionManager->ProcessTransitionsList( uNextTick );
		CAkURenderer::ProcessLimiters();
		CAkLEngine::Perform();
		CAkURenderer::PerformContextNotif();
		++m_uBufferTick;
	}
}

/** 2012 0x924240: drains the queue up to the end-of-list marker of a complete batch (or all of it when draining). */
AKRESULT CAkAudioMgr::ProcessMsgQueue()
{
	if( !m_bDrainMsgQueue && m_ulWriterFlag == m_ulReaderFlag )
	{
		return AK_Success;
	}
	bool bReachedEnd = false;
	m_queueLock.Lock();
	for( ;; )
	{
		if( m_MsgQueue.IsEmpty() )
		{
			break;
		}
		AkUInt32 uSizeAvail = 0;
		AkUInt32 uSizeRead = 0;
		AkUInt8 * pData = (AkUInt8 *)m_MsgQueue.BeginReadEx( uSizeAvail );
		m_queueLock.Unlock();
		do
		{
			AkQueuedMsg & item = *(AkQueuedMsg *)pData;
			switch( item.type )
			{
			case QueuedMsgType_EndOfList:
				++m_ulReaderFlag;
				bReachedEnd = m_ulReaderFlag == m_ulWriterFlag && !m_bDrainMsgQueue;
				break;

			case QueuedMsgType_Event:
			{
				CAkEvent * pEvent = item.event.Event;
				CAkRegisteredObj * pGameObj = g_pRegistryMgr->GetObjAndAddref( item.event.GameObjID );
				AkUInt32 uNumActions = 0;
				for( CAkAction * pAction = pEvent->m_actions.First(); pAction; pAction = pAction->pNextLightItem )
				{
					if( ( pAction->m_eActionType & 1 ) && !pGameObj )
					{
						continue;
					}
					void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPendingAction ) );
					if( !pMem )
					{
						continue;
					}
					AkPendingAction * pThisAction = AkPlacementNew( pMem ) AkPendingAction( ( pAction->m_eActionType & 1 ) ? pGameObj : NULL );
					pThisAction->TargetPlayingID = item.event.TargetPlayingID;
					pThisAction->pAction = pAction;
					pThisAction->UserParam.Init( item.event.PlayingID, item.event.CustomParam );
					EnqueueOrExecuteAction( pThisAction );
					++uNumActions;
				}
				AKDIAG( L"akdiag: event 0x%08x executed: playing id %u, game object %u, %u action(s), buffer tick %u", pEvent->key,
					item.event.PlayingID, (AkUInt32)item.event.GameObjID, uNumActions, m_uBufferTick );
				if( pGameObj && --pGameObj->m_refCount <= 0 )
				{
					pGameObj->~CAkRegisteredObj();
					AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
				}
				g_pPlayingMgr->RemoveItemActiveCount( item.event.PlayingID );
				pEvent->Release();
				if( item.event.CustomParam.pExternalSrcs )
				{
					item.event.CustomParam.pExternalSrcs->Release();
				}
				break;
			}

			case QueuedMsgType_RegisterGameObj:
				g_pRegistryMgr->RegisterObject( item.reggameobj.GameObjID, item.reggameobj.pMonitorData );
				break;

			case QueuedMsgType_UnregisterGameObj:
				if( item.unreggameobj.GameObjID == AK_INVALID_GAME_OBJECT )
				{
					g_pRegistryMgr->UnregisterAll();
				}
				else
				{
					g_pRegistryMgr->UnregisterObject( item.unreggameobj.GameObjID );
				}
				break;

			case QueuedMsgType_GameObjPosition:
				g_pRegistryMgr->SetPosition( item.gameobjpos.GameObjID, &item.gameobjpos.Position, 1, AK::SoundEngine::MultiPositionType_SingleSource, item.gameobjpos.uListenerIndex );
				break;

			case QueuedMsgType_GameObjActiveListeners:
				g_pRegistryMgr->SetActiveListeners( item.gameobjactlist.GameObjID, item.gameobjactlist.uListenerMask );
				break;

			case QueuedMsgType_ListenerPosition:
				CAkListener::SetListenerPosition( item.listpos.uListenerIndex, item.listpos.Position );
				break;

			case QueuedMsgType_ListenerSpatialization:
				CAkListener::SetListenerSpatialization( item.listspat.uListenerIndex, item.listspat.bSpatialized, item.listspat.bSetVolumes ? &item.listspat.Volumes : NULL );
				break;

			case QueuedMsgType_GameObjEnvValues:
				g_pRegistryMgr->SetGameObjectEnvironmentsValues( item.gameobjenvvalues.GameObjID, item.gameobjenvvalues.EnvValues, item.gameobjenvvalues.uNumValues );
				break;

			case QueuedMsgType_GameObjDryLevel:
				g_pRegistryMgr->SetGameObjectDryLevelValue( item.gameobjdrylevel.GameObjID, item.gameobjdrylevel.fValue );
				break;

			case QueuedMsgType_GameObjObstruction:
				g_pRegistryMgr->SetObjectObstructionAndOcclusion( item.gameobjobstr.GameObjID, item.gameobjobstr.uListenerIndex, item.gameobjobstr.fObstructionLevel, item.gameobjobstr.fOcclusionLevel );
				break;

			case QueuedMsgType_KillBank:
				CAkURenderer::StopAllPBIs( item.killbank.pUsageSlot );
				item.killbank.pUsageSlot->Release( false );
				break;

			case QueuedMsgType_StopAll:
			{
				CAkAction * pStopAction = CAkAction::Create( item.stopAll.GameObjID == AK_INVALID_GAME_OBJECT ? 0x104 : 0x105, 0 );
				if( pStopAction )
				{
					CAkRegisteredObj * pGameObj = NULL;
					if( item.stopAll.GameObjID != AK_INVALID_GAME_OBJECT )
					{
						pGameObj = g_pRegistryMgr->GetObjAndAddref( item.stopAll.GameObjID );
						if( !pGameObj )
						{
							pStopAction->Release();
							break;
						}
					}
					{
						AkPendingAction pendingAction( pGameObj );
						pStopAction->Execute( &pendingAction );
					}
					if( pGameObj && --pGameObj->m_refCount <= 0 )
					{
						pGameObj->~CAkRegisteredObj();
						AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
					}
					pStopAction->Release();
				}
				break;
			}

			case QueuedMsgType_ListenerPipeline:
				CAkListener::SetListenerPipeline( item.listpipe.uListenerIndex, item.listpipe.bAudio, item.listpipe.bFeedback );
				break;

			case QueuedMsgType_StopPlayingID:
			{
				ClearPendingItems( item.stopEvent.playingID );
				CAkBus * pMasterBus = CAkBus::GetMasterBusAndAddRef();
				if( pMasterBus )
				{
					ActionParams params;
					params.eType = ActionParamType_Stop;
					params.pGameObj = NULL;
					params.playingID = item.stopEvent.playingID;
					params.transParams.TransitionTime = item.stopEvent.uTransitionDuration;
					params.transParams.eFadeCurve = item.stopEvent.eFadeCurve;
					params.bIsFromBus = false;
					params.bIsMasterCall = false;
					params.bIsMasterResume = false;
					params.targetNodeID = 0;
					pMasterBus->ExecuteAction( params );
					pMasterBus->Release();
				}
				break;
			}

			case QueuedMsgType_EventAction:
			{
				CAkRegisteredObj * pGameObj = g_pRegistryMgr->GetObjAndAddref( item.eventAction.GameObjID );
				if( pGameObj || item.eventAction.GameObjID == AK_INVALID_GAME_OBJECT )
				{
					for( CAkAction * pAction = item.eventAction.Event->m_actions.First(); pAction; pAction = pAction->pNextLightItem )
					{
						if( pAction->m_eActionType == AkActionType_Play )
						{
							ProcessCustomAction( pAction->m_ulElementID, pGameObj, item.eventAction.eActionToExecute, item.eventAction.uTransitionDuration, item.eventAction.eFadeCurve, item.eventAction.TargetPlayingID );
							continue;
						}
						switch( item.eventAction.eActionToExecute )
						{
						case AK::SoundEngine::AkActionOnEventType_Stop:
						case AK::SoundEngine::AkActionOnEventType_Break:
							StopAction( pAction->key, item.eventAction.TargetPlayingID );
							break;
						case AK::SoundEngine::AkActionOnEventType_Pause:
							PauseAction( pAction->key, item.eventAction.TargetPlayingID );
							break;
						case AK::SoundEngine::AkActionOnEventType_Resume:
							ResumeAction( pAction->key, item.eventAction.TargetPlayingID );
							break;
						default:
							break;
						}
					}
					if( pGameObj && --pGameObj->m_refCount <= 0 )
					{
						pGameObj->~CAkRegisteredObj();
						AK::MemoryMgr::Free( g_DefaultPoolId, pGameObj );
					}
				}
				item.eventAction.Event->Release();
				break;
			}

			case QueuedMsgType_GameObjScalingFactor:
				g_pRegistryMgr->SetGameObjectScalingFactor( item.gameobjscalingfactor.GameObjID, item.gameobjscalingfactor.fValue );
				break;

			case QueuedMsgType_GameObjMultiPosition:
				g_pRegistryMgr->SetPosition( item.gameObjMultiPos.GameObjID, item.gameObjMultiPos.aMultiPosition, item.gameObjMultiPos.uNumPositions, item.gameObjMultiPos.eMultiPositionType, 0xFFFFFFFF );
				break;

			case QueuedMsgType_ListenerScalingFactor:
				CAkListener::SetScalingFactor( item.listenerscalingfactor.uListenerIndex, item.listenerscalingfactor.fValue );
				break;

			case QueuedMsgType_Seek:
				item.seek.Event->Release();
				break;

			case QueuedMsgType_SetPanningRule:
				CAkLEngine::SetPanningRule( item.setPanningRule.panRule );
				break;

			default:
				break;
			}
			pData += item.size;
			uSizeRead += item.size;
			++CAkLEngineCmds::m_ulPlayEventID;
		}
		while( uSizeRead < uSizeAvail && !bReachedEnd );
		m_queueLock.Lock();
		m_MsgQueue.EndRead( uSizeRead );
		if( bReachedEnd )
		{
			break;
		}
	}
	m_bDrainMsgQueue = false;
	m_queueLock.Unlock();
	SetEvent( m_hEventMgrThreadDrainEvent.m_Event );
	return AK_Success;
}

/** Retail's AkDelete of a pending action (inlined at every exit of the pending lists). */
void CAkAudioMgr::ReleasePendingAction( AkPendingAction * in_pPA )
{
	in_pPA->pAction->Release();
	in_pPA->~AkPendingAction();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pPA );
}

/** 2012 0x921d50. */
void CAkAudioMgr::ProcessAction( AkPendingAction * in_pAction )
{
	in_pAction->pAction->Execute( in_pAction );
	if( in_pAction->UserParam.m_PlayingID )
	{
		g_pPlayingMgr->RemoveItemActiveCount( in_pAction->UserParam.m_PlayingID );
	}
	ReleasePendingAction( in_pAction );
}

/** 2012 0x922430: delays are counted in whole buffers plus a frame offset; a play-and-continue keeps its look-ahead. */
void CAkAudioMgr::EnqueueOrExecuteAction( AkPendingAction * in_pActionItem )
{
	g_pPlayingMgr->AddItemActiveCount( in_pActionItem->UserParam.m_PlayingID );
	in_pActionItem->pAction->AddRef();
	const AkUInt32 uDelay = (AkUInt32)in_pActionItem->pAction->GetDelayTime();
	AkUInt32 uDelayTicks = uDelay / AK_NUM_VOICE_REFILL_FRAMES;
	in_pActionItem->LaunchTick = m_uBufferTick;
	in_pActionItem->LaunchFrameOffset = uDelay - uDelayTicks * AK_NUM_VOICE_REFILL_FRAMES;
	if( uDelayTicks && in_pActionItem->pAction->m_eActionType == AkActionType_PlayAndContinue )
	{
		const AkUInt32 uLookAhead = uDelayTicks < g_settings.uContinuousPlaybackLookAhead ? uDelayTicks : g_settings.uContinuousPlaybackLookAhead;
		uDelayTicks -= uLookAhead;
		in_pActionItem->LaunchFrameOffset = uDelay - uDelayTicks * AK_NUM_VOICE_REFILL_FRAMES;
	}
	if( !uDelayTicks )
	{
		ProcessAction( in_pActionItem );
		return;
	}
	in_pActionItem->LaunchTick += uDelayTicks;
	if( m_mmapPending.Insert( in_pActionItem->LaunchTick, in_pActionItem ) == AK_Success )
	{
		NotifyDelayStarted( in_pActionItem );
	}
	else
	{
		ReleasePendingAction( in_pActionItem );
	}
}

/** 2012 0x922340: every pending action whose launch tick has come. */
void CAkAudioMgr::ProcessPendingList()
{
	while( m_mmapPending.m_pFirst && m_mmapPending.m_pFirst->Item.key <= m_uBufferTick )
	{
		AkPendingAction * pPendingAction = m_mmapPending.m_pFirst->Item.item;
		m_mmapPending.RemoveItem( m_mmapPending.m_pFirst, NULL );
		pPendingAction->pAction->Execute( pPendingAction );
		if( pPendingAction->UserParam.m_PlayingID )
		{
			g_pPlayingMgr->RemoveItemActiveCount( pPendingAction->UserParam.m_PlayingID );
		}
		ReleasePendingAction( pPendingAction );
		++CAkLEngineCmds::m_ulPlayEventID;
	}
}

/** 2012 0x921ec0: only the monitoring notification (compiled out). */
void CAkAudioMgr::NotifyDelayStarted( AkPendingAction * in_pPending )
{
}

/** 2012 0x921f10. */
void CAkAudioMgr::NotifyDelayAborted( AkPendingAction * in_pPending, bool in_bWasPaused )
{
	g_pPlayingMgr->RemoveItemActiveCount( in_pPending->UserParam.m_PlayingID );
}

/** 2012 0x922220 (FlushPendingItem<CAkMultiKeyList>): unlink, free, and the next item. */
AkMultimapPending::ListItem * CAkAudioMgr::FlushPendingItem( AkPendingAction * in_pPA, AkMultimapPending & in_List, AkMultimapPending::ListItem * in_pItem, AkMultimapPending::ListItem * in_pPrevItem )
{
	AkMultimapPending::ListItem * pNext = in_pItem->pNextListItem;
	in_List.RemoveItem( in_pItem, in_pPrevItem );
	ReleasePendingAction( in_pPA );
	return pNext;
}

/** 2012 0x921da0: whatever is still queued at Term gives back its references. */
void CAkAudioMgr::RemoveAllPreallocAndReferences()
{
	AkAutoLock<CAkLock> lock( m_queueLock );
	while( !m_MsgQueue.IsEmpty() )
	{
		AkQueuedMsg & item = *(AkQueuedMsg *)m_MsgQueue.BeginRead();
		switch( item.type )
		{
		case QueuedMsgType_Event:
			g_pPlayingMgr->RemoveItemActiveCount( item.event.PlayingID );
			item.event.Event->Release();
			if( item.event.CustomParam.pExternalSrcs )
			{
				item.event.CustomParam.pExternalSrcs->Release();
			}
			break;
		case QueuedMsgType_EventAction:
			item.eventAction.Event->Release();
			break;
		case QueuedMsgType_Seek:
			item.seek.Event->Release();
			break;
		default:
			break;
		}
		m_MsgQueue.EndRead( item.size );
	}
}

/** 2012 0x9226e0. */
void CAkAudioMgr::RemoveAllPausedPendingAction()
{
	while( m_mmapPausedPending.m_pFirst )
	{
		AkPendingAction * pPA = m_mmapPausedPending.m_pFirst->Item.item;
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		FlushPendingItem( pPA, m_mmapPausedPending, m_mmapPausedPending.m_pFirst, NULL );
	}
}

/** 2012 0x9227c0. */
void CAkAudioMgr::RemoveAllPendingAction()
{
	while( m_mmapPending.m_pFirst )
	{
		AkPendingAction * pPA = m_mmapPending.m_pFirst->Item.item;
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		FlushPendingItem( pPA, m_mmapPending, m_mmapPending.m_pFirst, NULL );
	}
}

/** 2012 0x922c40: StopPlayingID drops the playing id's delayed and paused actions. */
void CAkAudioMgr::ClearPendingItems( AkPlayingID in_PlayingID )
{
	AkMultimapPending * aLists[ 2 ] = { &m_mmapPending, &m_mmapPausedPending };
	for( AkUInt32 iList = 0; iList < 2; ++iList )
	{
		AkMultimapPending::ListItem * pPrev = NULL;
		AkMultimapPending::ListItem * pItem = aLists[ iList ]->m_pFirst;
		while( pItem )
		{
			AkPendingAction * pPA = pItem->Item.item;
			if( pPA->UserParam.m_PlayingID != in_PlayingID )
			{
				pPrev = pItem;
				pItem = pItem->pNextListItem;
				continue;
			}
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, *aLists[ iList ], pItem, pPrev );
		}
	}
}

/** 2012 0x921f60. */
void CAkAudioMgr::ClearCrossFadeOccurence( CAkContinuousPBI * in_pPBIToCheck )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.item->pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			static_cast<CAkActionPlayAndContinue *>( pItem->Item.item->pAction )->UnsetFadeBack( in_pPBIToCheck );
		}
	}
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.item->pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			static_cast<CAkActionPlayAndContinue *>( pItem->Item.item->pAction )->UnsetFadeBack( in_pPBIToCheck );
		}
	}
}

/** 2012 0x9231b0: a paused playing id's delayed actions move to the paused list (duck actions excepted). */
void CAkAudioMgr::PausePendingItems( AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.item->UserParam.m_PlayingID == in_PlayingID && pItem->Item.item->pAction->m_eActionType != AkActionType_Duck )
		{
			++pItem->Item.item->ulPauseCount;
		}
	}
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( pPA->UserParam.m_PlayingID != in_PlayingID || pPA->pAction->m_eActionType == AkActionType_Duck )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		pPA->PausedTick = m_uBufferTick;
		pPA->ulPauseCount = 0;
		if( m_mmapPausedPending.Insert( pPA->pAction->m_ulElementID, pPA ) != AK_Success )
		{
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, m_mmapPending, pItem, pPrev );
			continue;
		}
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
}

/** 2012 0x9225c0. */
void CAkAudioMgr::ResumePausedPendingItems( AkPlayingID in_playingID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( pPA->UserParam.m_PlayingID != in_playingID )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		if( pPA->ulPauseCount )
		{
			--pPA->ulPauseCount;
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		pPA->LaunchTick += m_uBufferTick - pPA->PausedTick;
		if( m_mmapPending.Insert( pPA->LaunchTick, pPA ) != AK_Success )
		{
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, m_mmapPausedPending, pItem, pPrev );
			continue;
		}
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPausedPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
}

/** 2012 0x9228a0: every delayed or paused instance of the action (of the playing id). */
AKRESULT CAkAudioMgr::StopAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID )
{
	AkMultimapPending * aLists[ 2 ] = { &m_mmapPending, &m_mmapPausedPending };
	for( AkUInt32 iList = 0; iList < 2; ++iList )
	{
		AkMultimapPending::ListItem * pPrev = NULL;
		AkMultimapPending::ListItem * pItem = aLists[ iList ]->m_pFirst;
		while( pItem )
		{
			AkPendingAction * pPA = pItem->Item.item;
			if( pPA->pAction->key != in_ActionID || ( in_PlayingID && pPA->UserParam.m_PlayingID != in_PlayingID ) )
			{
				pPrev = pItem;
				pItem = pItem->pNextListItem;
				continue;
			}
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, *aLists[ iList ], pItem, pPrev );
		}
	}
	return AK_Success;
}

/** 2012 0x922a60: paused instances count one more pause; delayed ones move to the paused list. */
AKRESULT CAkAudioMgr::PauseAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( pPA->pAction->key == in_ActionID && ( !in_PlayingID || pPA->UserParam.m_PlayingID == in_PlayingID ) )
		{
			++pPA->ulPauseCount;
		}
	}
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( pPA->pAction->key != in_ActionID || ( in_PlayingID && pPA->UserParam.m_PlayingID != in_PlayingID ) )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		pPA->PausedTick = m_uBufferTick;
		pPA->ulPauseCount = 0;
		if( m_mmapPausedPending.Insert( pPA->pAction->m_ulElementID, pPA ) != AK_Success )
		{
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, m_mmapPending, pItem, pPrev );
			continue;
		}
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
	return AK_Success;
}

/** 2012 0x922bc0. */
AKRESULT CAkAudioMgr::ResumeAction( AkUniqueID in_ActionID, AkPlayingID in_PlayingID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( pPA->pAction->key == in_ActionID && ( !in_PlayingID || pPA->UserParam.m_PlayingID == in_PlayingID ) )
		{
			if( !pPA->ulPauseCount )
			{
				TransferToPending( pPA );
				AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
				m_mmapPausedPending.RemoveItem( pItem, pPrev );
				pItem = pNext;
				continue;
			}
			--pPA->ulPauseCount;
		}
		pPrev = pItem;
		pItem = pItem->pNextListItem;
	}
	return AK_Success;
}

/** 2012 0x922500. */
void CAkAudioMgr::InsertAsPaused( AkUniqueID in_ElementID, AkPendingAction * in_pPendingAction, AkUInt32 in_ulPauseCount )
{
	in_pPendingAction->PausedTick = m_uBufferTick;
	in_pPendingAction->ulPauseCount = in_ulPauseCount;
	if( m_mmapPausedPending.Insert( in_ElementID, in_pPendingAction ) != AK_Success )
	{
		NotifyDelayAborted( in_pPendingAction, false );
		ReleasePendingAction( in_pPendingAction );
	}
}

/** 2012 0x922560. */
void CAkAudioMgr::TransferToPending( AkPendingAction * in_pPendingAction )
{
	in_pPendingAction->LaunchTick += m_uBufferTick - in_pPendingAction->PausedTick;
	if( m_mmapPending.Insert( in_pPendingAction->LaunchTick, in_pPendingAction ) != AK_Success )
	{
		NotifyDelayAborted( in_pPendingAction, true );
		ReleasePendingAction( in_pPendingAction );
	}
}

/** 2012 0x9233d0: is in_IDToCheck the target, or under it (actor-mixer parents, then output buses)? */
bool CAkAudioMgr::IsElementOf( AkUniqueID in_TargetID, AkUniqueID in_IDToCheck )
{
	if( !in_TargetID || !in_IDToCheck )
	{
		return false;
	}
	if( in_TargetID == in_IDToCheck )
	{
		return true;
	}
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_IDToCheck );
	if( !pNode )
	{
		return false;
	}
	bool bFound = false;
	CAkParameterNodeBase * pBus = pNode->m_pBusOutputNode;
	for( CAkParameterNodeBase * pParent = pNode->m_pParentNode; pParent; pParent = pParent->m_pParentNode )
	{
		if( pParent->key == in_TargetID )
		{
			bFound = true;
			break;
		}
		if( !pBus )
		{
			pBus = pParent->m_pBusOutputNode;
		}
	}
	for( ; !bFound && pBus; pBus = pBus->m_pBusOutputNode )
	{
		bFound = pBus->key == in_TargetID;
	}
	pNode->Release();
	return bFound;
}

/** 2012 0x923300: is the action's element (or a parent, or an output bus) in the exception list? */
bool CAkAudioMgr::IsAnException( CAkAction * in_pAction, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList )
{
	if( !in_pExceptionList || !in_pAction->m_ulElementID )
	{
		return false;
	}
	CAkParameterNodeBase * pNodeInitial = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_pAction->m_ulElementID );
	if( !pNodeInitial )
	{
		return false;
	}
	bool bFound = false;
	CAkParameterNodeBase * pBus = NULL;
	for( CAkParameterNodeBase * pNode = pNodeInitial; pNode && !bFound; pNode = pNode->m_pParentNode )
	{
		bFound = in_pExceptionList->Exists( pNode->key ) != NULL;
		if( !pBus )
		{
			pBus = pNode->m_pBusOutputNode;
		}
	}
	for( ; !bFound && pBus; pBus = pBus->m_pBusOutputNode )
	{
		bFound = in_pExceptionList->Exists( pBus->key ) != NULL;
	}
	pNodeInitial->Release();
	return bFound;
}

/** 2012 0x923850: delayed and paused actions under the target (of the object / playing id); duck actions are kept. */
AKRESULT CAkAudioMgr::StopPendingAction( AkUniqueID in_TargetID, CAkRegisteredObj * in_GameObj, AkPlayingID in_PlayingID )
{
	AkMultimapPending * aLists[ 2 ] = { &m_mmapPending, &m_mmapPausedPending };
	for( AkUInt32 iList = 0; iList < 2; ++iList )
	{
		AkMultimapPending::ListItem * pPrev = NULL;
		AkMultimapPending::ListItem * pItem = aLists[ iList ]->m_pFirst;
		while( pItem )
		{
			AkPendingAction * pPA = pItem->Item.item;
			CAkAction * pAction = pPA->pAction;
			if( ( in_TargetID && !IsElementOf( in_TargetID, pAction->m_ulElementID ) ) || ( in_GameObj && in_GameObj != pPA->pGameObj )
				|| ( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID ) || pAction->m_eActionType == AkActionType_Duck )
			{
				pPrev = pItem;
				pItem = pItem->pNextListItem;
				continue;
			}
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, *aLists[ iList ], pItem, pPrev );
		}
	}
	return AK_Success;
}

/** 2012 0x923b50. */
AKRESULT CAkAudioMgr::StopPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, AkPlayingID in_PlayingID )
{
	AkMultimapPending * aLists[ 2 ] = { &m_mmapPending, &m_mmapPausedPending };
	for( AkUInt32 iList = 0; iList < 2; ++iList )
	{
		AkMultimapPending::ListItem * pPrev = NULL;
		AkMultimapPending::ListItem * pItem = aLists[ iList ]->m_pFirst;
		while( pItem )
		{
			AkPendingAction * pPA = pItem->Item.item;
			if( ( in_GameObj && in_GameObj != pPA->pGameObj ) || ( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID )
				|| IsAnException( pPA->pAction, in_pExceptionList ) || pPA->pAction->m_eActionType == AkActionType_Duck )
			{
				pPrev = pItem;
				pItem = pItem->pNextListItem;
				continue;
			}
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, *aLists[ iList ], pItem, pPrev );
		}
	}
	return AK_Success;
}

/** 2012 0x9234f0: a resume action only pauses with a master pause. */
void CAkAudioMgr::PausePendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( !in_ulElementID || IsElementOf( in_ulElementID, pAction->m_ulElementID ) ) && ( ( pAction->m_eActionType & 0xFF00 ) != 0x300 || in_bIsMasterOnResume )
			&& ( !in_GameObj || in_GameObj == pPA->pGameObj ) && ( !in_PlayingID || in_PlayingID == pPA->UserParam.m_PlayingID ) && pAction->m_eActionType != AkActionType_Duck )
		{
			++pPA->ulPauseCount;
		}
	}
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( in_ulElementID && !IsElementOf( in_ulElementID, pAction->m_ulElementID ) ) || ( ( pAction->m_eActionType & 0xFF00 ) == 0x300 && !in_bIsMasterOnResume )
			|| ( in_GameObj && in_GameObj != pPA->pGameObj ) || ( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID ) || pAction->m_eActionType == AkActionType_Duck )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		InsertAsPaused( pAction->m_ulElementID, pPA, 0 );
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
}

/** 2012 0x923a10. */
void CAkAudioMgr::PausePendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( ( pAction->m_eActionType & 0xFF00 ) != 0x300 || in_bIsMasterOnResume ) && ( !in_GameObj || in_GameObj == pPA->pGameObj )
			&& ( !in_PlayingID || in_PlayingID == pPA->UserParam.m_PlayingID ) && !IsAnException( pAction, in_pExceptionList ) && pAction->m_eActionType != AkActionType_Duck )
		{
			++pPA->ulPauseCount;
		}
	}
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( ( pAction->m_eActionType & 0xFF00 ) == 0x300 && !in_bIsMasterOnResume ) || ( in_GameObj && in_GameObj != pPA->pGameObj )
			|| ( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID ) || IsAnException( pAction, in_pExceptionList ) || pAction->m_eActionType == AkActionType_Duck )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		InsertAsPaused( pAction->m_ulElementID, pPA, 0 );
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
}

/** 2012 0x923f50: paused instances under the element come back (a non-master resume undoes one pause at a time). */
void CAkAudioMgr::ResumePausedPendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( ( in_ulElementID && !IsElementOf( in_ulElementID, pItem->Item.key ) ) || ( in_GameObj && in_GameObj != pPA->pGameObj )
			|| ( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID ) )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		if( !in_bIsMasterOnResume && pPA->ulPauseCount )
		{
			--pPA->ulPauseCount;
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		pPA->LaunchTick += m_uBufferTick - pPA->PausedTick;
		if( m_mmapPending.Insert( pPA->LaunchTick, pPA ) != AK_Success )
		{
			g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
			pItem = FlushPendingItem( pPA, m_mmapPausedPending, pItem, pPrev );
			continue;
		}
		AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
		m_mmapPausedPending.RemoveItem( pItem, pPrev );
		pItem = pNext;
	}
	ResumeNotPausedPendingAction( in_ulElementID, in_GameObj, in_bIsMasterOnResume, in_PlayingID );
}

/** 2012 0x9240b0. */
void CAkAudioMgr::ResumePausedPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( ( !in_GameObj || in_GameObj == pPA->pGameObj ) && ( !in_PlayingID || in_PlayingID == pPA->UserParam.m_PlayingID ) && !IsAnException( pPA->pAction, in_pExceptionList ) )
		{
			if( in_bIsMasterOnResume || !pPA->ulPauseCount )
			{
				TransferToPending( pPA );
				AkMultimapPending::ListItem * pNext = pItem->pNextListItem;
				m_mmapPausedPending.RemoveItem( pItem, pPrev );
				pItem = pNext;
				continue;
			}
			--pPA->ulPauseCount;
		}
		pPrev = pItem;
		pItem = pItem->pNextListItem;
	}
	ResumeNotPausedPendingActionAllExcept( in_GameObj, in_pExceptionList, in_bIsMasterOnResume, in_PlayingID );
}

/** 2012 0x923ce0: a play-and-continue still delayed drops its pause fade. */
void CAkAudioMgr::ResumeNotPausedPendingAction( AkUniqueID in_ulElementID, CAkRegisteredObj * in_GameObj, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( !in_ulElementID || IsElementOf( in_ulElementID, pAction->m_ulElementID ) ) && ( !in_GameObj || in_GameObj == pPA->pGameObj )
			&& ( !in_PlayingID || in_PlayingID == pPA->UserParam.m_PlayingID ) && pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			static_cast<CAkActionPlayAndContinue *>( pAction )->Resume();
		}
	}
}

/** 2012 0x923d50. */
void CAkAudioMgr::ResumeNotPausedPendingActionAllExcept( CAkRegisteredObj * in_GameObj, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> * in_pExceptionList, bool in_bIsMasterOnResume, AkPlayingID in_PlayingID )
{
	for( AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		if( ( !in_GameObj || in_GameObj == pPA->pGameObj ) && ( !in_PlayingID || in_PlayingID == pPA->UserParam.m_PlayingID )
			&& !IsAnException( pAction, in_pExceptionList ) && pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			static_cast<CAkActionPlayAndContinue *>( pAction )->Resume();
		}
	}
}

/** 2012 0x923640: a delayed play under the target is dropped (a play-and-continue only if it cannot continue below it). */
AKRESULT CAkAudioMgr::BreakPendingAction( AkUniqueID in_TargetID, CAkRegisteredObj * in_GameObj, AkPlayingID in_PlayingID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		bool bBreak = !( in_TargetID && !IsElementOf( in_TargetID, pAction->m_ulElementID ) ) && !( in_GameObj && in_GameObj != pPA->pGameObj )
			&& !( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID );
		if( bBreak && pAction->m_eActionType != AkActionType_Play )
		{
			bBreak = pAction->m_eActionType == AkActionType_PlayAndContinue && static_cast<CAkActionPlayAndContinue *>( pAction )->BreakToNode( in_TargetID, pPA->pGameObj, pPA );
		}
		if( !bBreak )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		pItem = FlushPendingItem( pPA, m_mmapPending, pItem, pPrev );
	}
	pPrev = NULL;
	pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		CAkAction * pAction = pPA->pAction;
		const bool bBreak = !( in_TargetID && !IsElementOf( in_TargetID, pAction->m_ulElementID ) ) && !( in_GameObj && in_GameObj != pPA->pGameObj )
			&& !( in_PlayingID && in_PlayingID != pPA->UserParam.m_PlayingID ) && pAction->m_eActionType == AkActionType_PlayAndContinue
			&& static_cast<CAkActionPlayAndContinue *>( pAction )->BreakToNode( in_TargetID, pPA->pGameObj, pPA );
		if( !bBreak )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		pItem = FlushPendingItem( pPA, m_mmapPausedPending, pItem, pPrev );
	}
	return AK_Success;
}

/** 2012 0x923db0: a node going away takes its delayed actions with it. */
void CAkAudioMgr::RemovePendingAction( AkUniqueID in_ulElementID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( !IsElementOf( in_ulElementID, pPA->pAction->m_ulElementID ) || pPA->pAction->m_eActionType == AkActionType_Duck )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		pItem = FlushPendingItem( pPA, m_mmapPending, pItem, pPrev );
	}
}

/** 2012 0x923e70. */
void CAkAudioMgr::RemovePausedPendingAction( AkUniqueID in_ulElementID )
{
	AkMultimapPending::ListItem * pPrev = NULL;
	AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst;
	while( pItem )
	{
		AkPendingAction * pPA = pItem->Item.item;
		if( !IsElementOf( in_ulElementID, pItem->Item.key ) )
		{
			pPrev = pItem;
			pItem = pItem->pNextListItem;
			continue;
		}
		g_pPlayingMgr->RemoveItemActiveCount( pPA->UserParam.m_PlayingID );
		pItem = FlushPendingItem( pPA, m_mmapPausedPending, pItem, pPrev );
	}
}

/** 2012 0x922e30: a play-and-continue's finished pause fade parks its pending action. */
AKRESULT CAkAudioMgr::PausePending( AkPendingAction * in_pPA )
{
	if( !in_pPA )
	{
		return AK_Success;
	}
	AkMultimapPending::ListItem * pPrev = NULL;
	for( AkMultimapPending::ListItem * pItem = m_mmapPending.m_pFirst; pItem; pPrev = pItem, pItem = pItem->pNextListItem )
	{
		if( pItem->Item.item != in_pPA )
		{
			continue;
		}
		AkUInt32 ulPauseCount = 0;
		if( in_pPA->pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			CAkActionPlayAndContinue * pPlayAndContinue = static_cast<CAkActionPlayAndContinue *>( in_pPA->pAction );
			ulPauseCount = pPlayAndContinue->m_ulPauseCount - 1;
			pPlayAndContinue->m_ulPauseCount = 0;
		}
		InsertAsPaused( in_pPA->pAction->m_ulElementID, in_pPA, ulPauseCount );
		m_mmapPending.RemoveItem( pItem, pPrev );
		return AK_Success;
	}
	for( AkMultimapPending::ListItem * pItem = m_mmapPausedPending.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.item != in_pPA )
		{
			continue;
		}
		if( in_pPA->pAction->m_eActionType == AkActionType_PlayAndContinue )
		{
			in_pPA->ulPauseCount += static_cast<CAkActionPlayAndContinue *>( in_pPA->pAction )->m_ulPauseCount;
		}
		else
		{
			++in_pPA->ulPauseCount;
		}
		break;
	}
	return AK_Success;
}

/** 2012 0x922f20: a play-and-continue's finished stop fade drops its pending action. */
AKRESULT CAkAudioMgr::StopPending( AkPendingAction * in_pPA )
{
	if( !in_pPA )
	{
		return AK_Success;
	}
	AkMultimapPending * aLists[ 2 ] = { &m_mmapPausedPending, &m_mmapPending };
	for( AkUInt32 iList = 0; iList < 2; ++iList )
	{
		AkMultimapPending::ListItem * pPrev = NULL;
		for( AkMultimapPending::ListItem * pItem = aLists[ iList ]->m_pFirst; pItem; pPrev = pItem, pItem = pItem->pNextListItem )
		{
			if( pItem->Item.item == in_pPA )
			{
				g_pPlayingMgr->RemoveItemActiveCount( in_pPA->UserParam.m_PlayingID );
				FlushPendingItem( in_pPA, *aLists[ iList ], pItem, pPrev );
				return AK_Success;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x924150: ExecuteActionOnEvent on a play action acts on its target directly (and on its delayed plays). */
void CAkAudioMgr::ProcessCustomAction( AkUniqueID in_Target, CAkRegisteredObj * in_pGameObj, AK::SoundEngine::AkActionOnEventType in_ActionToExecute, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_Target );
	if( !pNode )
	{
		return;
	}
	ActionParams params;
	params.pGameObj = in_pGameObj;
	params.playingID = in_PlayingID;
	params.transParams.TransitionTime = in_uTransitionDuration;
	params.transParams.eFadeCurve = in_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = in_Target;
	switch( in_ActionToExecute )
	{
	case AK::SoundEngine::AkActionOnEventType_Stop:
		StopPendingAction( in_Target, in_pGameObj, in_PlayingID );
		params.eType = ActionParamType_Stop;
		break;
	case AK::SoundEngine::AkActionOnEventType_Pause:
		PausePendingAction( in_Target, in_pGameObj, true, in_PlayingID );
		params.eType = ActionParamType_Pause;
		break;
	case AK::SoundEngine::AkActionOnEventType_Resume:
		ResumePausedPendingAction( in_Target, in_pGameObj, false, in_PlayingID );
		params.eType = ActionParamType_Resume;
		break;
	case AK::SoundEngine::AkActionOnEventType_Break:
		BreakPendingAction( in_Target, in_pGameObj, in_PlayingID );
		params.eType = ActionParamType_Break;
		break;
	default:
		pNode->Release();
		return;
	}
	pNode->ExecuteAction( params );
	pNode->Release();
}
