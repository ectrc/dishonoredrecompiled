/*=============================================================================
	AkScheduleWindow.h - CAkScheduleWindow (akschedulewindow.cpp): a window on what a music context will play: one
	scheduled item of some chain deep inside the context tree, reached through a stack of switch-transition branches
	(one per switch level), with the item's duration until the next item (or infinite). Contexts advance it with
	GetNextScheduleWindow; transitions and stingers search sync points in it.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, align 8, 160 bytes): BranchStack (32 iterators + size, 132)
	+ m_itScheduledItem (+132) + m_uLevel (+140) + m_uWindowDuration (+144) + m_bIsDurationInfinite (+152) +
	m_bInvalidChain (+153) + m_bFirstItemOnly (+154).
=============================================================================*/
#pragma once

#include "AkSegmentChain.h"

class CAkMusicSwitchTransition;
class CAkMusicSegment;
class CAkMusicNode;
struct AkAssociatedAction;

class CAkScheduleWindow
{
public:
	struct BranchStack
	{
		CAkMusicSwitchTransition *	m_arBranches[ 32 ];
		AkUInt32					m_uSize;
	};

	/** RAII level guard (akschedulewindow.h AutoBranchLevel): GetNextScheduleWindow / RefreshWindow of a switch level. */
	class AutoBranchLevel
	{
	public:
		explicit AutoBranchLevel( CAkScheduleWindow & io_window ) : m_wnd( &io_window ) { ++m_wnd->m_uLevel; }
		~AutoBranchLevel() { --m_wnd->m_uLevel; }
		CAkScheduleWindow * m_wnd;
	};

	explicit CAkScheduleWindow( bool in_bFirstItemOnly = false );
	CAkScheduleWindow( CAkMatrixAwareCtx * in_pCreatorCtx, bool in_bFirstItemOnly = false );

	void SetScheduledItem( const AkScheduledChain::SelfContainedIter & in_item );
	CAkMusicSwitchTransition * GetBranch() const;
	void SetBranch( CAkMusicSwitchTransition * in_itTrans );
	void SetDuration( AkUInt64 in_uDuration, bool in_bInfinite );
	CAkMusicSegment * GetNode( CAkMusicNode ** out_ppParentNode ) const;
	AkInt64 StartTime() const;
	AkInt64 StartTimeRelativeToCurrentLevel() const;
	AKRESULT FindSyncPoint( AkInt64 in_iMinTime, AkSyncType in_eSyncType, AkUInt32 & io_uCueFilter, bool in_bDoSkipEntryCue, bool in_bSucceedOnNothing, AkInt64 & out_iSyncTime ) const;
	bool IsAtExitCue( AkUInt64 in_uTime ) const;
	void CancelActionsAfterTransitionSyncPoint( AkListBareLight<AkAssociatedAction> & io_listCancelledActions, AkInt64 in_iSyncTime );
	void NotifyMusicCallbacks( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration, AkUInt32 in_uMusicSyncFlags, AkPlayingID in_playingID ) const;
	AkInt64 ChainCtxTimeRelativeToLevel( AkUInt32 in_uLevel ) const;

	BranchStack								m_arBranchStack;
	AkScheduledChain::SelfContainedIter		m_itScheduledItem;
	AkUInt32								m_uLevel;
	AkUInt64								m_uWindowDuration;
	bool									m_bIsDurationInfinite;
	bool									m_bInvalidChain;
	bool									m_bFirstItemOnly;
};

static_assert( sizeof( CAkScheduleWindow ) == 160, "CAkScheduleWindow: 2012 PDB 160 (align 8)" );
