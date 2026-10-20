/*=============================================================================
	AkScheduleWindow.cpp - CAkScheduleWindow (akschedulewindow.cpp, 2012 0x965740..0x965ce0).

	DISHONORED(port): agent IH4 (wave 26). Times: "chain time" is the time inside the chain context of the window's
	item; ChainCtxTimeRelativeToLevel adds the local times of the switch branches from a level down to the chain, so
	StartTime() is relative to the window's creator context.
=============================================================================*/
#include "AkScheduleWindow.h"
#include "AkMatrixSequencer.h"
#include "AkMusicSwitchCtx.h"
#include "AkMusicSegment.h"
#include "AkSegmentCtx.h"

static const AkInt64 AK_INT64_MAX = 0x7FFFFFFFFFFFFFFFLL;

/** 2012 0x965740, 2013 0x961f70. */
CAkScheduleWindow::CAkScheduleWindow( bool in_bFirstItemOnly )
	: m_uLevel( 0 )
	, m_uWindowDuration( 0 )
	, m_bIsDurationInfinite( true )
	, m_bInvalidChain( false )
	, m_bFirstItemOnly( in_bFirstItemOnly )
{
	m_arBranchStack.m_uSize = 0;
}

/** 2012 0x965790, 2013 0x961fc0: positioned on the creator's first (or current) item. */
CAkScheduleWindow::CAkScheduleWindow( CAkMatrixAwareCtx * in_pCreatorCtx, bool in_bFirstItemOnly )
	: m_uLevel( 0 )
	, m_uWindowDuration( 0 )
	, m_bIsDurationInfinite( true )
	, m_bInvalidChain( false )
	, m_bFirstItemOnly( in_bFirstItemOnly )
{
	m_arBranchStack.m_uSize = 0;
	in_pCreatorCtx->GetNextScheduleWindow( *this, false );
}

/** 2012 0x9657f0, 2013 0x962020: an item of a chain that lost its playlist marks the window invalid. */
void CAkScheduleWindow::SetScheduledItem( const AkScheduledChain::SelfContainedIter & in_item )
{
	m_bInvalidChain = !in_item.pCtx->IsValid();
	m_itScheduledItem = in_item;
}

/** 2012 0x965830, 2013 0x962060: the branch chosen at the current level, or none yet. */
CAkMusicSwitchTransition * CAkScheduleWindow::GetBranch() const
{
	const AkUInt32 uIndex = m_uLevel - 1;
	if( uIndex == m_arBranchStack.m_uSize )
	{
		return NULL;
	}
	return m_arBranchStack.m_arBranches[ uIndex ];
}

/** 2012 0x965860, 2013 0x962090. */
void CAkScheduleWindow::SetDuration( AkUInt64 in_uDuration, bool in_bInfinite )
{
	m_uWindowDuration = in_uDuration;
	m_bIsDurationInfinite = in_bInfinite;
}

/** 2012 0x965890, 2013 0x9620c0: the item's segment; a silent item reports its chain context's node (or that context's parent's). */
CAkMusicSegment * CAkScheduleWindow::GetNode( CAkMusicNode ** out_ppParentNode ) const
{
	CAkSegmentCtx * pSegment = m_itScheduledItem.pItem->m_pSegment;
	if( pSegment )
	{
		CAkMusicSegment * pNode = pSegment->m_pSegmentNode;
		if( out_ppParentNode )
		{
			*out_ppParentNode = static_cast<CAkMusicNode *>( pNode->m_pParentNode );
		}
		return pNode;
	}
	if( out_ppParentNode )
	{
		*out_ppParentNode = m_itScheduledItem.pCtx->Node();
		if( !*out_ppParentNode )
		{
			*out_ppParentNode = static_cast<CAkMatrixAwareCtx *>( m_itScheduledItem.pCtx->m_pParentCtx )->Node();
		}
	}
	return NULL;
}

/** 2012 0x9658f0, 2013 0x962120: selecting another branch at a level drops the deeper branches and the item. */
void CAkScheduleWindow::SetBranch( CAkMusicSwitchTransition * in_itTrans )
{
	const AkUInt32 uIndex = m_uLevel - 1;
	if( uIndex == m_arBranchStack.m_uSize )
	{
		m_itScheduledItem.pCtx = NULL;
		m_itScheduledItem.pItem = NULL;
		m_arBranchStack.m_arBranches[ m_arBranchStack.m_uSize++ ] = in_itTrans;
	}
	else if( m_arBranchStack.m_arBranches[ uIndex ] != in_itTrans )
	{
		m_itScheduledItem.pCtx = NULL;
		m_itScheduledItem.pItem = NULL;
		if( (AkInt32)( m_arBranchStack.m_uSize - 1 ) >= (AkInt32)uIndex )
		{
			m_arBranchStack.m_uSize = uIndex;
		}
		m_arBranchStack.m_arBranches[ m_arBranchStack.m_uSize++ ] = in_itTrans;
	}
}

/** 2012 0x965980, 2013 0x9621b0: the sum of the branch destinations' local times from in_uLevel down. */
AkInt64 CAkScheduleWindow::ChainCtxTimeRelativeToLevel( AkUInt32 in_uLevel ) const
{
	AkInt64 iTime = 0;
	for( AkUInt32 i = in_uLevel; i < m_arBranchStack.m_uSize; ++i )
	{
		iTime += m_arBranchStack.m_arBranches[ i ]->m_pDestCtx->m_iLocalTime;
	}
	return iTime;
}

/** 2012 0x965a30, 2013 0x962260. */
AkInt64 CAkScheduleWindow::StartTime() const
{
	if( !m_itScheduledItem.pItem )
	{
		return AK_INT64_MAX;
	}
	return m_itScheduledItem.GetScheduledItemTime() + ChainCtxTimeRelativeToLevel( 0 );
}

/**
 * 2012 0x965a70: the first sync point of the requested type at or after in_iMinTime (and the window start) in the
 * window's segment; a silent item syncs immediately (if allowed); AK_Fail past the window's end.
 */
AKRESULT CAkScheduleWindow::FindSyncPoint( AkInt64 in_iMinTime, AkSyncType in_eSyncType, AkUInt32 & io_uCueFilter, bool in_bDoSkipEntryCue, bool in_bSucceedOnNothing, AkInt64 & out_iSyncTime ) const
{
	AkInt64 iWindowStart = AK_INT64_MAX;
	if( m_itScheduledItem.pItem )
	{
		iWindowStart = m_itScheduledItem.GetScheduledItemTime() + ChainCtxTimeRelativeToLevel( 0 );
	}
	const AkInt64 iMinTime = in_iMinTime > iWindowStart ? in_iMinTime : iWindowStart;
	AkInt64 iSegmentPosition = m_itScheduledItem.CtxTimeToSegmentPosition( iMinTime - ChainCtxTimeRelativeToLevel( 0 ) );
	AKRESULT eResult;
	if( !m_itScheduledItem.pItem->m_pSegment || m_bInvalidChain )
	{
		io_uCueFilter = 0;
		if( !in_bSucceedOnNothing && in_eSyncType != SyncTypeImmediate )
		{
			return AK_Fail;
		}
		eResult = AK_Success;
	}
	else
	{
		AkUInt32 uExitSyncPos;
		eResult = m_itScheduledItem.pItem->m_pSegment->m_pSegmentNode->GetExitSyncPos( (AkUInt32)iSegmentPosition, in_eSyncType, io_uCueFilter, in_bDoSkipEntryCue, uExitSyncPos );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		iSegmentPosition = uExitSyncPos;
	}
	out_iSyncTime = m_itScheduledItem.SegmentPositionToCtxTime( iSegmentPosition ) + ChainCtxTimeRelativeToLevel( 0 );
	if( m_bIsDurationInfinite || out_iSyncTime <= iWindowStart + (AkInt64)m_uWindowDuration )
	{
		return eResult;
	}
	return AK_Fail;
}

/** 2012 0x965bd0, 2013 0x962400. */
bool CAkScheduleWindow::IsAtExitCue( AkUInt64 in_uTime ) const
{
	if( !m_itScheduledItem.pItem->m_pSegment )
	{
		return false;
	}
	const AkInt64 iSegmentPosition = m_itScheduledItem.CtxTimeToSegmentPosition( (AkInt64)in_uTime - ChainCtxTimeRelativeToLevel( 0 ) );
	CAkSegmentCtx * pSegment = m_itScheduledItem.pItem->m_pSegment;
	CAkMusicSegment * pNode = pSegment ? pSegment->m_pSegmentNode : NULL;
	return iSegmentPosition == (AkInt64)pNode->ActiveDuration();
}

/** 2012 0x965c30, 2013 0x962460: the item's actions after the sync point are taken off it, to be rescheduled. */
void CAkScheduleWindow::CancelActionsAfterTransitionSyncPoint( AkListBareLight<AkAssociatedAction> & io_listCancelledActions, AkInt64 in_iSyncTime )
{
	AkInt64 iWindowStart = AK_INT64_MAX;
	if( m_itScheduledItem.pItem )
	{
		iWindowStart = m_itScheduledItem.GetScheduledItemTime() + ChainCtxTimeRelativeToLevel( 0 );
	}
	m_itScheduledItem.pItem->PopAssociatedActionsToRescheduleAfterTransitionSyncPoint( io_listCancelledActions, in_iSyncTime - iWindowStart );
}

/** 2012 0x965c90, 2013 0x9624c0. */
void CAkScheduleWindow::NotifyMusicCallbacks( AkInt64 in_iCurrentTime, AkUInt32 in_uFrameDuration, AkUInt32 in_uMusicSyncFlags, AkPlayingID in_playingID ) const
{
	if( !m_itScheduledItem.pItem->m_pSegment )
	{
		return;
	}
	const AkInt64 iSegmentPosition = m_itScheduledItem.CtxTimeToSegmentPosition( in_iCurrentTime - ChainCtxTimeRelativeToLevel( 0 ) );
	m_itScheduledItem.pItem->NotifyMusicCallbacks( (AkInt32)iSegmentPosition, in_uFrameDuration, in_uMusicSyncFlags, in_playingID );
}

/** 2012 0x965ce0, 2013 0x962510. */
AkInt64 CAkScheduleWindow::StartTimeRelativeToCurrentLevel() const
{
	if( !m_itScheduledItem.pItem )
	{
		return AK_INT64_MAX;
	}
	return m_itScheduledItem.GetScheduledItemTime() + ChainCtxTimeRelativeToLevel( m_uLevel - 1 );
}
