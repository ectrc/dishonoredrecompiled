/*=============================================================================
	AkMatrixSequencer.h - CAkMatrixSequencer and CAkScheduledItem (akmatrixsequencer.cpp).

	CAkMatrixSequencer is the clock of one top-level music context: it advances its time by the audio frame, has its
	owner process the frame, runs the stingers (triggers) and the music callbacks (beat / bar / grid / entry / exit /
	user cue / sync point), and finds the next sync point for a delayed state change.
	CAkScheduledItem is one entry of a context's segment chain: a segment context, its local time in the chain, its
	pending play and stop commands (relative to that time), and the actions attached to it (stingers and delayed state
	changes that fire when the chain reaches them).

	DISHONORED(port): agent IH4 (wave 26). Layouts (2012 PDB): CAkScheduledItem 0x38, AkAssociatedAction 0x18,
	AkStingerRecord 0x28, CAkMatrixSequencer 0x40 (IAkTriggerAware vtable, m_uTime +8, m_pOwner +16, m_UserParams +24,
	m_pGameObj +48, m_listPendingStingers +52, m_uCurTimeWindowSize +56). CAkSmartPtr members are raw pointers with
	retail's AddRef / Release discipline.
=============================================================================*/
#pragma once

#include "AkMatrixAwareCtx.h"
#include <AkStateMgr.h>
#include <AkAudioMgr.h>

class CAkSegmentCtx;
class CAkStinger;

struct AkStingerRecord
{
	AkStingerRecord *		pNextLightItem;
	CAkMatrixAwareCtx *		pStingerCtx;
	AkTriggerID				triggerID;
	AkUniqueID				segmentID;
	AkInt64					iSyncTime;
	AkUInt32				uDontRepeatTime;
	AkInt32					iLookAheadDuration;
	AkInt32					iRelativeStopTime;
	bool					bCanBeRescheduled;
};

struct AkAssociatedAction
{
	AkInt64					iRelativeTime;
	AkAssociatedActionType	eActionType;
	union
	{
		AkStingerRecord *	pStingerRecord;
		void *				pStateChangeCookie;
	};
	AkAssociatedAction *	pNextLightItem;
};

struct AkSegmentPlayCmd
{
	AkInt32		iRelativeTime;
	AkMusicFade	fadeParams;
};

struct AkSegmentStopCmd
{
	AkInt32		iRelativeTime;
	TransParams	transParams;
};

class CAkScheduledItem
{
public:
	CAkScheduledItem( AkUInt64 in_iLocalTime, CAkSegmentCtx * in_pSegment );

	void Destroy();
	AkInt32 Prepare( AkUInt32 in_uSyncPosition, AkInt32 in_iStartPosition, const AkMusicFade & in_fade );
	void SetFadeIn( AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, AkInt32 in_iFadeOffset );
	void AttachStopCmd( AkTimeMs in_iTransDuration, AkCurveInterpolation in_eFadeCurve, AkInt32 in_iRelativeTime );
	void ForcePostExit( bool in_bPlayPostExit );
	void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, bool in_bSkipPlay );
	void CancelPlayback( AkInt64 in_iCurrentTime );
	void OnStopped();
	void OnPaused();
	bool CanDestroy();
	AKRESULT GetInfo( AkInt32 in_iSegmentPosition, AkSegmentInfo & out_uSegmentInfo );
	void NotifyMusicCallbacks( AkInt32 in_iSegmentPosition, AkUInt32 in_uFrameDuration, AkUInt32 in_uNotifFlags, AkPlayingID in_playingID ) const;
	void PopAssociatedActionsToRescheduleAfterTransitionSyncPoint( AkListBareLight<AkAssociatedAction> & io_listCancelledActions, AkInt64 in_iMinActionTime );

	CAkScheduledItem *						pNextItem;
	AkUInt64								m_iLocalTime;
	AkListBareLight<AkAssociatedAction>		m_listAssociatedActions;
	CAkSegmentCtx *							m_pSegment;
	AkSegmentPlayCmd						m_cmdPlay;
	AkSegmentStopCmd						m_cmdStop;
	AkUInt16								m_bCmdPlayPending : 1;
	AkUInt16								m_bCmdStopPending : 1;
	AkUInt16								m_bWasPlaybackSkipped : 1;

protected:
	void _CancelPlayback( AkInt32 in_iCurrentTime );
};

class CAkMatrixSequencer : public IAkTriggerAware
{
public:
	enum AkDelayedStateHandlingResult
	{
		AK_DelayedStateSyncFound	= 0,
		AK_DelayedStateCannotSync	= 1,
		AK_DelayedStateImmediate	= 2,
		AK_DelayedStateNotHandled	= 3
	};

	CAkMatrixSequencer( CAkMatrixAwareCtx * in_pOwner, UserParams & in_rUserparams, CAkRegisteredObj * in_pGameObj );
	~CAkMatrixSequencer();

	void Execute( AkUInt32 in_uNumFrames );
	void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples );
	void OnStopped();
	AkUInt32 GetMusicSyncFlags();
	AkInt64 QueryStateChangeDelay( AkStateGroupID in_stateGroupID, AkUInt32 & out_uSegmentLookAhead, AkInt64 & out_iSyncTime );
	AKRESULT ProcessDelayedStateChange( void * in_pCookie, AkUInt32 in_uSegmentLookAhead, AkInt64 in_iSyncTime );
	void RescheduleCancelledActions( AkListBareLight<AkAssociatedAction> & in_listActions );
	virtual void Trigger( AkTriggerID in_triggerID );

	AkUInt64							m_uTime;
	CAkMatrixAwareCtx *					m_pOwner;
	UserParams							m_UserParams;
	CAkRegisteredObj *					m_pGameObj;
	AkListBareLight<AkStingerRecord>	m_listPendingStingers;
	AkUInt32							m_uCurTimeWindowSize;

private:
	void HandleTrigger( AkTriggerID in_triggerID, bool in_bReschedule );
	bool CanPlayStinger( AkTriggerID in_triggerID );
	AKRESULT ScheduleStingerForPlayback( const CAkScheduleWindow & in_window, CAkMatrixAwareCtx * in_pStingerCtx, const CAkStinger * in_pStingerData, AkInt64 in_iSyncTime, AkInt32 in_iLookAheadDuration, bool in_bScheduledInCurrentSegment );
	CAkMatrixAwareCtx * CreateStingerCtx( AkUniqueID in_segmentID, AkInt32 & out_iLookAheadDuration );
	void ProcessStingers( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration );
	void RemoveAllPendingStingers();
	void ProcessMusicNotifications( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration );
	AkDelayedStateHandlingResult GetEarliestStateSyncTime( const CAkScheduleWindow & in_window, AkStateGroupID in_stateGroupID, AkInt64 & out_iSyncTime );
	static void FreeStingerRecord( AkStingerRecord * in_pRecord );
};

static_assert( sizeof( AkStingerRecord ) == 0x28, "AkStingerRecord: 2012 PDB 0x28" );
static_assert( sizeof( AkAssociatedAction ) == 0x18, "AkAssociatedAction: 2012 PDB 0x18" );
static_assert( sizeof( CAkScheduledItem ) == 0x38, "CAkScheduledItem: 2012 PDB 0x38 (EnqueueItem mallocs 0x38)" );
static_assert( sizeof( CAkMatrixSequencer ) == 0x40, "CAkMatrixSequencer: 2012 PDB 0x40 (AddChild mallocs 0x40)" );
