/*=============================================================================
	AkMusicRenderer.h - CAkMusicRenderer (akmusicrenderer.cpp): the interactive-music singleton. It owns the top-level
	music contexts (each with its matrix sequencer), runs them once per audio frame as a behavioural extension of the
	audio manager, creates the music PBIs, routes stop / pause / resume / seek of music nodes to their contexts, keeps the
	segment info of playing ids, and is the sound engine's external state handler: a state change some playing segment
	syncs to is delayed to that sync point (AkStateChangeRecord queue) and posted from there.

	DISHONORED(port): agent IH4 (wave 26). All data is static as retail's (the object itself is a 1-byte allocation).
=============================================================================*/
#pragma once

#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include "AkSegmentInfoRepository.h"
#include "AkMatrixAwareCtx.h"
#include <AkBankMgr.h>

class CAkMusicCtx;
class CAkMusicNode;
class CAkMusicPBI;
class CAkSoundBase;
class CAkSource;
class CAkRegisteredObj;
struct AkTrackSrc;

namespace AK
{
	namespace SoundEngine
	{
		/** 2012 0x917320: the queued SetState with its two flags (the music engine posts delayed states with skip extension). */
		AKRESULT SetState( AkStateGroupID in_stateGroup, AkStateID in_state, bool in_bSkipTransition, bool in_bSkipExtension );
		/** 2012 0x918260. */
		void AddExternalBankHandler( AkExternalBankHandlerCallback in_pCallback );
	}
}

class CAkMusicRenderer
{
public:
	struct AkStateChangeRecord
	{
		AkStateGroupID	stateGroupID;
		AkStateID		stateID;
		AkUInt32		bWasPosted : 1;
		AkUInt32		bIsReferenced : 1;
	};

	struct AkListBareNextTopLevelCtxSibling
	{
		static CAkMatrixAwareCtx *& Get( CAkMatrixAwareCtx * in_pItem ) { return in_pItem->pNextTopLevelSibling; }
	};

	typedef CAkList2<AkStateChangeRecord, const AkStateChangeRecord &, 1, ArrayPoolDefault> PendingStateChanges;

	static CAkMusicRenderer * Create( AkMusicSettings * in_pSettings );
	void Destroy();

	static AKRESULT Play( CAkMusicCtx * io_pParentCtx, CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, UserParams & in_rUserparams, const AkTrackSrc * in_pSrcInfo, AkUInt32 in_uSourceOffset, AkUInt32 in_uFrameOffset, CAkMusicPBI *& out_pPBI );

	void Stop( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, AkPlayingID in_playingID );
	void Pause( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, AkPlayingID in_playingID );
	void Resume( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, TransParams & in_transParams, bool in_bMasterResume, AkPlayingID in_playingID );
	void SeekTimeAbsolute( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, AkTimeMs in_iSeekTime, bool in_bSnapToCue );
	void SeekPercent( CAkMusicNode * in_pNode, CAkRegisteredObj * in_pGameObj, AkReal32 in_fSeekPercent, bool in_bSnapToCue );

	AKRESULT AddChild( CAkMatrixAwareCtx * in_pMusicCtx, UserParams & in_rUserparams, CAkRegisteredObj * in_pGameObj );
	void RemoveChild( CAkMatrixAwareCtx * in_pMusicCtx );

	static void PerformNextFrameBehavior( bool in_bLastCall );
	static bool SetState( AkStateGroupID in_stateGroupID, AkStateID in_stateID );
	void PerformDelayedStateChange( void * in_pCookie );
	void RescheduleDelayedStateChange( void * in_pCookie );

	static CAkMusicRenderer *		m_pMusicRenderer;
	static AkMusicSettings			m_musicSettings;
	static AkListBare<CAkMatrixAwareCtx, AkListBareNextTopLevelCtxSibling>	m_listCtx;
	static CAkSegmentInfoRepository	m_segmentInfoRepository;
	static PendingStateChanges		m_queuePendingStateChanges;
	static HANDLE					m_hTermEvent;

private:
	AKRESULT Init( AkMusicSettings * in_pSettings );
	static void Play( CAkMusicPBI * in_pContext, TransParams & in_transParams );
	static AkInt64 GetDelayedStateChangeData( AkStateGroupID in_stateGroupID, CAkMatrixAwareCtx *& out_pChosenCtx, AkInt64 & out_iChosenRelativeSyncTime, AkUInt32 & out_uChosenSegmentLookAhead );
	static void FindPendingStateChange( void * in_pCookie, PendingStateChanges::ListItem *& out_pItem, PendingStateChanges::ListItem *& out_pPrevItem );
	static void InvalidateOlderPendingStateChanges( PendingStateChanges::ListItem * in_pItem, AkStateGroupID in_stateGroupID );
	static void CancelDelayedStateChange( AkStateGroupID in_stateGroupID, PendingStateChanges::ListItem * in_pItem );
	static void CleanPendingStateChanges();
	static AkStateChangeRecord * AddFirstPendingStateChange();
};

namespace AkMusicBank
{
	AKRESULT LoadBankItem( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot, AkUInt32 in_dwBankID );
}
