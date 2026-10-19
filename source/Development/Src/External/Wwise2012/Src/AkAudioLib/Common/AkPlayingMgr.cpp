/*=============================================================================
	AkPlayingMgr.cpp - CAkPlayingMgr (akplayingmgr.cpp, 2012 0x927a10..0x9286e0; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * The music notifications (NotifyMusic, NotifyMusicUserCues, NotifyMusicPlayStarted, PrepareMusicNotification),
	    NotifySpeakerVolumeMatrix and NotifyEndOfDynamicSequenceItem are IH4's / unused by the glue (no caller
	    registers AK_Music* / AK_SpeakerVolumeMatrix / dynamic sequences) and are not ported.
	  * -akdiag logs every playing id's end (event, game object, the callback flags) - the IH2 acceptance measures
	    play time from it.
=============================================================================*/
#include "AkPlayingMgr.h"
#include "AkPBI.h"
#include "AkPositionRepository.h"

CAkPlayingMgr * g_pPlayingMgr = NULL;

/** 2012 0x927a10: the callback event starts signaled (no callback in flight). */
AKRESULT CAkPlayingMgr::Init()
{
	m_CallbackEvent.m_Event = CreateEventW( NULL, TRUE, FALSE, NULL );
	if( !m_CallbackEvent.m_Event )
	{
		return AK_Fail;
	}
	SetEvent( m_CallbackEvent.m_Event );
	m_PlayingMap.Init();
	return AK_Success;
}

/** Inlined in AK::SoundEngine::Term (2012 0x91a090): the map is intrusive, every item was released with its count. */
void CAkPlayingMgr::Term()
{
	m_PlayingMap.Term();
}

/** 2012 0x927a50: one item per post, with one active action (the event itself) until ProcessMsgQueue has run it. */
AKRESULT CAkPlayingMgr::AddPlayingID( AkQueuedMsg_EventBase & in_event, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_uiRegisteredNotif, AkUniqueID in_id )
{
	AkAutoLock<CAkLock> lock( m_csMapLock );
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( PlayingMgrItem ) );
	if( !pMem )
	{
		return AK_Fail;
	}
	PlayingMgrItem * pItem = AkPlacementNew( pMem ) PlayingMgrItem;
	pItem->cPBI = 0;
	pItem->cAction = 1;
	pItem->eventID = in_id;
	pItem->GameObj = in_event.GameObjID;
	pItem->userParam.Init( in_event.PlayingID, in_event.CustomParam );
	pItem->pfnCallback = in_pfnCallback;
	pItem->pCookie = in_pCookie;
	if( !in_pfnCallback )
	{
		in_uiRegisteredNotif &= ~AK_CallbackBits | AK_MusicSyncUserCue | AK_Marker;
	}
	pItem->uiRegisteredNotif = in_uiRegisteredNotif;
	m_PlayingMap.Set( pItem );
	return AK_Success;
}

/** 2012 0x9283f0. */
AKRESULT CAkPlayingMgr::SetPBI( AkPlayingID in_PlayingID, CAkTransportAware * in_pPBI, AkUInt32 * out_puRegisteredNotif )
{
	AkAutoLock<CAkLock> lock( m_csMapLock );
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( pItem )
	{
		++pItem->cPBI;
		*out_puRegisteredNotif = pItem->uiRegisteredNotif;
	}
	return AK_Success;
}

/** 2012 0x928670. */
void CAkPlayingMgr::Remove( AkPlayingID in_PlayingID, CAkTransportAware * in_pPBI )
{
	m_csMapLock.Lock();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( !pItem )
	{
		m_csMapLock.Unlock();
		return;
	}
	--pItem->cPBI;
	CheckRemovePlayingID( in_PlayingID, pItem );
}

/** 2012 0x928470. */
void CAkPlayingMgr::AddItemActiveCount( AkPlayingID in_PlayingID )
{
	if( !in_PlayingID )
	{
		return;
	}
	AkAutoLock<CAkLock> lock( m_csMapLock );
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( pItem )
	{
		++pItem->cAction;
	}
}

/** 2012 0x9286e0. */
void CAkPlayingMgr::RemoveItemActiveCount( AkPlayingID in_PlayingID )
{
	m_csMapLock.Lock();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( !pItem )
	{
		m_csMapLock.Unlock();
		return;
	}
	--pItem->cAction;
	CheckRemovePlayingID( in_PlayingID, pItem );
}

/** 2012 0x928590: entered with m_csMapLock held, leaves it released; AK_EndOfEvent goes out after the item is freed. */
void CAkPlayingMgr::CheckRemovePlayingID( AkPlayingID in_PlayingID, PlayingMgrItem * in_pItem )
{
	if( in_pItem->cPBI || in_pItem->cAction )
	{
		m_csMapLock.Unlock();
		return;
	}
	if( in_pItem->uiRegisteredNotif & AK_EnableGetSourcePlayPosition )
	{
		g_pPositionRepository->RemovePlayingID( in_PlayingID );
	}
	AkEventCallbackInfo info;
	info.pCookie = in_pItem->pCookie;
	info.gameObjID = in_pItem->GameObj;
	info.playingID = in_PlayingID;
	info.eventID = in_pItem->eventID;
	const AkCallbackFunc pfnCallback = in_pItem->pfnCallback;
	const AkUInt32 uiRegisteredNotif = in_pItem->uiRegisteredNotif;
	m_PlayingMap.Unset( in_PlayingID );
	in_pItem->~PlayingMgrItem();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pItem );
	AKDIAG( L"akdiag: playing id %u ended (event 0x%08x, game object %u, notif 0x%x, buffer tick %u)", in_PlayingID, info.eventID,
		(AkUInt32)info.gameObjID, uiRegisteredNotif, g_pAudioMgr ? g_pAudioMgr->GetBufferTick() : 0 );
	if( !( uiRegisteredNotif & AK_EndOfEvent ) )
	{
		m_csMapLock.Unlock();
		return;
	}
	ResetEvent( m_CallbackEvent.m_Event );
	m_csMapLock.Unlock();
	pfnCallback( AK_EndOfEvent, &info );
	SetEvent( m_CallbackEvent.m_Event );
}

/** 2012 0x927b50: from another thread, waits until a callback in flight has returned. */
void CAkPlayingMgr::CancelCallbackCookie( void * in_pCookie )
{
	{
		AkAutoLock<CAkLock> lock( m_csMapLock );
		for( AkHashListBare<AkPlayingID, PlayingMgrItem, 31, PlayingMgrItemPolicy>::Iterator it = m_PlayingMap.Begin(); it != m_PlayingMap.End(); ++it )
		{
			PlayingMgrItem * pItem = *it;
			if( pItem->pCookie == in_pCookie )
			{
				pItem->pfnCallback = NULL;
				pItem->uiRegisteredNotif &= ~AK_CallbackBits;
			}
		}
	}
	if( CAkAudioThread::m_hEventMgrThreadID != GetCurrentThreadId() )
	{
		WaitForSingleObject( m_CallbackEvent.m_Event, INFINITE );
	}
}

/** 2012 0x927d70. */
void CAkPlayingMgr::CancelCallback( AkPlayingID in_playingID )
{
	{
		AkAutoLock<CAkLock> lock( m_csMapLock );
		PlayingMgrItem * pItem = m_PlayingMap.Exists( in_playingID );
		if( pItem )
		{
			pItem->pfnCallback = NULL;
			pItem->uiRegisteredNotif &= ~AK_CallbackBits;
		}
	}
	if( CAkAudioThread::m_hEventMgrThreadID != GetCurrentThreadId() )
	{
		WaitForSingleObject( m_CallbackEvent.m_Event, INFINITE );
	}
}

/** 2012 0x927bf0: io_ruNumIDs 0 counts only. */
AKRESULT CAkPlayingMgr::GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs )
{
	AkUInt32 uMaxIDs = (AkUInt32)-1;
	AkPlayingID * pOut = NULL;
	if( io_ruNumIDs )
	{
		if( !out_aPlayingIDs )
		{
			return AK_InvalidParameter;
		}
		pOut = out_aPlayingIDs;
		uMaxIDs = io_ruNumIDs;
	}
	io_ruNumIDs = 0;
	AkAutoLock<CAkLock> lock( m_csMapLock );
	for( AkHashListBare<AkPlayingID, PlayingMgrItem, 31, PlayingMgrItemPolicy>::Iterator it = m_PlayingMap.Begin(); it != m_PlayingMap.End(); ++it )
	{
		PlayingMgrItem * pItem = *it;
		if( pItem->GameObj == in_GameObjId )
		{
			--uMaxIDs;
			if( pOut )
			{
				pOut[ io_ruNumIDs ] = pItem->userParam.m_PlayingID;
			}
			++io_ruNumIDs;
		}
		if( !uMaxIDs )
		{
			break;
		}
	}
	return AK_Success;
}

/** 2012 0x9284b0. */
AkUniqueID CAkPlayingMgr::GetEventIDFromPlayingID( AkPlayingID in_playingID )
{
	AkAutoLock<CAkLock> lock( m_csMapLock );
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_playingID );
	return pItem ? pItem->eventID : AK_INVALID_UNIQUE_ID;
}

/** 2012 0x928520. */
AkGameObjectID CAkPlayingMgr::GetGameObjectFromPlayingID( AkPlayingID in_playingID )
{
	AkAutoLock<CAkLock> lock( m_csMapLock );
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_playingID );
	return pItem ? pItem->GameObj : AK_INVALID_GAME_OBJECT;
}

/** 2012 0x927ed0. */
void CAkPlayingMgr::NotifyDuration( AkPlayingID in_PlayingID, AkReal32 in_fDuration, AkReal32 in_fEstimatedDuration, AkUniqueID in_idAudioNode )
{
	m_csMapLock.Lock();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	AKDIAG( L"akdiag: playing id %u duration %.1f ms (estimated %.1f ms) node 0x%08x", in_PlayingID, in_fDuration, in_fEstimatedDuration, in_idAudioNode );
	if( !pItem || !( pItem->uiRegisteredNotif & AK_Duration ) )
	{
		m_csMapLock.Unlock();
		return;
	}
	AkDurationCallbackInfo info;
	info.pCookie = pItem->pCookie;
	info.gameObjID = pItem->GameObj;
	info.playingID = in_PlayingID;
	info.eventID = pItem->eventID;
	info.fDuration = in_fDuration;
	info.fEstimatedDuration = in_fEstimatedDuration;
	info.audioNodeID = in_idAudioNode;
	const AkCallbackFunc pfnCallback = pItem->pfnCallback;
	ResetEvent( m_CallbackEvent.m_Event );
	m_csMapLock.Unlock();
	pfnCallback( AK_Duration, &info );
	SetEvent( m_CallbackEvent.m_Event );
}

/** 2012 0x928070. */
void CAkPlayingMgr::NotifyMarker( CAkPBI * in_pPBI, AkAudioMarker * in_pMarker )
{
	m_csMapLock.Lock();
	const AkPlayingID playingID = in_pPBI->GetPlayingID();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( playingID );
	if( !pItem || !( pItem->uiRegisteredNotif & AK_Marker ) || !pItem->pfnCallback )
	{
		m_csMapLock.Unlock();
		return;
	}
	AkMarkerCallbackInfo info;
	info.pCookie = pItem->pCookie;
	info.gameObjID = pItem->GameObj;
	info.playingID = playingID;
	info.eventID = pItem->eventID;
	info.uIdentifier = in_pMarker->dwIdentifier;
	info.uPosition = in_pMarker->dwPosition;
	info.strLabel = in_pMarker->strLabel;
	const AkCallbackFunc pfnCallback = pItem->pfnCallback;
	ResetEvent( m_CallbackEvent.m_Event );
	m_csMapLock.Unlock();
	pfnCallback( AK_Marker, &info );
	SetEvent( m_CallbackEvent.m_Event );
}

/** 2012 0x927e00 (agent IH3 fork C): AK_EndOfDynamicSequenceItem to a playing id that registered it. */
void CAkPlayingMgr::NotifyEndOfDynamicSequenceItem( AkPlayingID in_PlayingID, AkUniqueID in_audioNodeID, void * in_pCustomInfo )
{
	m_csMapLock.Lock();
	PlayingMgrItem * pItem = m_PlayingMap.Exists( in_PlayingID );
	if( !pItem || !( pItem->uiRegisteredNotif & AK_EndOfDynamicSequenceItem ) )
	{
		m_csMapLock.Unlock();
		return;
	}
	AkDynamicSequenceItemCallbackInfo info;
	info.pCookie = pItem->pCookie;
	info.gameObjID = pItem->GameObj;
	info.playingID = in_PlayingID;
	info.audioNodeID = in_audioNodeID;
	info.pCustomInfo = in_pCustomInfo;
	const AkCallbackFunc pfnCallback = pItem->pfnCallback;
	ResetEvent( m_CallbackEvent.m_Event );
	m_csMapLock.Unlock();
	pfnCallback( AK_EndOfDynamicSequenceItem, &info );
	SetEvent( m_CallbackEvent.m_Event );
}

/** 2012 0x928200. */
void CAkPlayingMgr::NotifyMarkers( AkPipelineBuffer & io_CurrMarkerBuffer )
{
	if( !io_CurrMarkerBuffer.pMarkers )
	{
		return;
	}
	AkBufferMarker * pMarker = io_CurrMarkerBuffer.pMarkers;
	for( AkUInt32 i = 0; i < io_CurrMarkerBuffer.uNumMarkers; ++i, ++pMarker )
	{
		NotifyMarker( pMarker->pContext, &pMarker->marker );
	}
	io_CurrMarkerBuffer.FreeMarkers();
}
