/*=============================================================================
	AkPlayingMgr.h - CAkPlayingMgr (akplayingmgr.cpp): one item per playing id, counting the PBIs and pending actions
	that keep it alive, and the callbacks (AK_EndOfEvent when both counts reach zero, AK_Duration, AK_Marker).

	DISHONORED(port): agent IH2 (wave 26). PlayingMgrItem is the PDB's 0x38 bytes; the map is an
	AkHashListBare<AkPlayingID, PlayingMgrItem, 31> keyed by the item's userParam.m_PlayingID.
	Callbacks run on the thread that drops the last count (the audio thread, as retail) with m_CallbackEvent reset,
	so CancelCallback / CancelCallbackCookie from another thread wait for a callback in flight to return.
=============================================================================*/
#pragma once

#include "AkAudioMgr.h"
#include <AK/SoundEngine/Common/AkCallback.h>

class CAkPBI;
class CAkTransportAware;
class AkPipelineBuffer;
struct AkAudioMarker;

class CAkPlayingMgr
{
public:
	struct PlayingMgrItem
	{
		AkInt32			cPBI;
		AkInt32			cAction;
		AkUniqueID		eventID;
		AkGameObjectID	GameObj;
		UserParams		userParam;
		AkCallbackFunc	pfnCallback;
		void *			pCookie;
		AkUInt32		uiRegisteredNotif;
		PlayingMgrItem *	pNextItem;
	};

	struct PlayingMgrItemPolicy
	{
		static AkPlayingID Key( const PlayingMgrItem * in_pItem ) { return in_pItem->userParam.m_PlayingID; }
	};

	AKRESULT Init();
	void Term();

	AKRESULT AddPlayingID( AkQueuedMsg_EventBase & in_event, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_uiRegisteredNotif, AkUniqueID in_id );
	AKRESULT SetPBI( AkPlayingID in_PlayingID, CAkTransportAware * in_pPBI, AkUInt32 * out_puRegisteredNotif );
	void Remove( AkPlayingID in_PlayingID, CAkTransportAware * in_pPBI );
	void AddItemActiveCount( AkPlayingID in_PlayingID );
	void RemoveItemActiveCount( AkPlayingID in_PlayingID );
	void CancelCallbackCookie( void * in_pCookie );
	void CancelCallback( AkPlayingID in_playingID );
	AKRESULT GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs );
	AkUniqueID GetEventIDFromPlayingID( AkPlayingID in_playingID );
	AkGameObjectID GetGameObjectFromPlayingID( AkPlayingID in_playingID );
	void NotifyDuration( AkPlayingID in_PlayingID, AkReal32 in_fDuration, AkReal32 in_fEstimatedDuration, AkUniqueID in_idAudioNode );
	void NotifyMarker( CAkPBI * in_pPBI, AkAudioMarker * in_pMarker );
	void NotifyMarkers( AkPipelineBuffer & io_CurrMarkerBuffer );

	AkHashListBare<AkPlayingID, PlayingMgrItem, 31, PlayingMgrItemPolicy>	m_PlayingMap;
	CAkLock			m_csMapLock;
	AkManualEvent	m_CallbackEvent;

private:
	void CheckRemovePlayingID( AkPlayingID in_PlayingID, PlayingMgrItem * in_pItem );
};

extern CAkPlayingMgr * g_pPlayingMgr;
