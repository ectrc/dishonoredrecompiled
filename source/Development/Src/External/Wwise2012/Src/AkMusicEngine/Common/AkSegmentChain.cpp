/*=============================================================================
	AkSegmentChain.cpp - AkScheduledChain::SelfContainedIter and CAkChainCtx (aksegmentchain.cpp, 2012
	0x964e20..0x9655b0).

	DISHONORED(port): agent IH4 (wave 26).
=============================================================================*/
#include "AkSegmentChain.h"
#include "AkMatrixSequencer.h"
#include "AkScheduleWindow.h"
#include "AkSegmentCtx.h"
#include "AkMusicSegment.h"

// ------------------------------------------------------------------------------------------------------------
// AkScheduledChain::SelfContainedIter
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x964e20, 2013 0x961660: the item's start in the context's time (items before the context's start clamp to 0). */
AkInt64 AkScheduledChain::SelfContainedIter::GetScheduledItemTime() const
{
	const AkInt64 iTime = (AkInt64)pItem->m_iLocalTime - (AkInt64)pCtx->m_uItemsTimeOffset;
	return iTime < 0 ? 0 : iTime;
}

/** 2012 0x964e50, 2013 0x961690. */
AkInt64 AkScheduledChain::SelfContainedIter::CtxTimeToSegmentPosition( AkInt64 in_iTime ) const
{
	return in_iTime + (AkInt64)pCtx->m_uItemsTimeOffset - (AkInt64)pItem->m_iLocalTime;
}

/** 2012 0x964e70, 2013 0x9616b0. */
AkInt64 AkScheduledChain::SelfContainedIter::SegmentPositionToCtxTime( AkInt64 in_iSegmentPosition ) const
{
	return in_iSegmentPosition + (AkInt64)pItem->m_iLocalTime - (AkInt64)pCtx->m_uItemsTimeOffset;
}

// ------------------------------------------------------------------------------------------------------------
// CAkChainCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x964ea0, 2013 0x9616e0. */
CAkChainCtx::CAkChainCtx( CAkMusicCtx * in_parent )
	: CAkMatrixAwareCtx( in_parent )
	, m_uItemsTimeOffset( 0 )
{
}

/** 2012 0x9654a0, 2013 0x961cd0. */
CAkChainCtx::~CAkChainCtx()
{
	ClearChain();
	m_chain.RemoveAll();
}

/** 2012 0x964f00, 2013 0x961730. */
void CAkChainCtx::ClearChain()
{
	while( CAkScheduledItem * pItem = m_chain.RemoveFirst() )
	{
		pItem->Destroy();
	}
}

/** 2012 0x964f40, 2013 0x961770: appended at the end of the chain. */
CAkScheduledItem * CAkChainCtx::EnqueueItem( AkUInt64 in_uTimeOffset, CAkSegmentCtx * in_pSegment )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkScheduledItem ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkScheduledItem * pItem = AkPlacementNew( pMem ) CAkScheduledItem( in_uTimeOffset, in_pSegment );
	m_chain.AddLast( pItem );
	return pItem;
}

/**
 * 2012 0x964fa0: the next item of the chain (the first one: the one playing at the sequencer's time, unless the window
 * asks for the very first item); the window lasts until the following item (growing the chain if needed), else forever.
 */
void CAkChainCtx::GetNextScheduleWindow( CAkScheduleWindow & io_window, bool in_bDoNotGrow )
{
	AkScheduledChain::SelfContainedIter itNext;
	itNext.pCtx = io_window.m_itScheduledItem.pCtx;
	if( io_window.m_itScheduledItem.pItem )
	{
		itNext.pItem = io_window.m_itScheduledItem.pItem->pNextItem;
		if( !itNext.pItem )
		{
			io_window.m_itScheduledItem.pItem = NULL;
			return;
		}
	}
	else
	{
		itNext.pItem = m_chain.First();
		itNext.pCtx = this;
		if( !itNext.pItem )
		{
			return;
		}
		if( !io_window.m_bFirstItemOnly )
		{
			AkInt64 iCurrentTime = m_iLocalTime;
			if( m_pParentCtx )
			{
				iCurrentTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
			}
			const AkInt64 iChainTime = (AkInt64)m_pSequencer->m_uTime - iCurrentTime;
			AkScheduledChain::SelfContainedIter it = itNext;
			for( CAkScheduledItem * pItem = itNext.pItem->pNextItem; pItem; pItem = pItem->pNextItem )
			{
				if( (AkInt64)pItem->m_iLocalTime - (AkInt64)m_uItemsTimeOffset >= iChainTime )
				{
					break;
				}
				it.pItem = pItem;
			}
			itNext = it;
		}
	}
	AkUInt64 uDuration = 0;
	bool bInfinite = false;
	CAkScheduledItem * pFollowing = itNext.pItem->pNextItem;
	if( !pFollowing && ( in_bDoNotGrow || !Grow() ) )
	{
		bInfinite = true;
	}
	else
	{
		pFollowing = itNext.pItem->pNextItem;
		AkInt64 iStart = (AkInt64)itNext.pItem->m_iLocalTime - (AkInt64)m_uItemsTimeOffset;
		if( iStart < 0 )
		{
			iStart = 0;
		}
		AkInt64 iEnd = (AkInt64)pFollowing->m_iLocalTime - (AkInt64)m_uItemsTimeOffset;
		if( iEnd < 0 )
		{
			iEnd = 0;
		}
		uDuration = (AkUInt64)( iEnd - iStart );
	}
	io_window.SetScheduledItem( itNext );
	io_window.SetDuration( uDuration, bInfinite );
}

/** 2012 0x965140, 2013 0x961970: the window's duration after the chain may have grown. */
void CAkChainCtx::RefreshWindow( CAkScheduleWindow & io_window )
{
	CAkScheduledItem * pItem = io_window.m_itScheduledItem.pItem;
	if( !pItem->pNextItem && !Grow() )
	{
		io_window.SetDuration( 0, true );
		return;
	}
	AkInt64 iStart = (AkInt64)pItem->m_iLocalTime - (AkInt64)m_uItemsTimeOffset;
	if( iStart < 0 )
	{
		iStart = 0;
	}
	AkInt64 iEnd = (AkInt64)pItem->pNextItem->m_iLocalTime - (AkInt64)m_uItemsTimeOffset;
	if( iEnd < 0 )
	{
		iEnd = 0;
	}
	io_window.SetDuration( (AkUInt64)( iEnd - iStart ), false );
}

/** 2012 0x9651e0, 2013 0x961a10. */
void CAkChainCtx::Flush()
{
	while( CAkScheduledItem * pItem = m_chain.First() )
	{
		pItem->OnStopped();
		m_chain.RemoveFirst();
		pItem->Destroy();
	}
}

/**
 * 2012 0x965230: the first item is prepared at its entry position (by the transition's destination rule, plus the
 * pre-entry if the rule plays it); returns the longest look-ahead any item needs before the context's start.
 */
AkInt32 CAkChainCtx::Prepare( const AkMusicTransDestRule * in_pRule, AkInt32 in_iMinStartPosition, AkUInt32 & out_uSelectedCue, AkUInt32 in_uCueHashForMatchSrc )
{
	CAkScheduledItem * pFirst = m_chain.First();
	AkUInt32 uSyncTime = 0;
	AkInt32 iLookAhead;
	if( in_pRule )
	{
		AkInt32 iEntryPosition = 0;
		if( pFirst->m_pSegment )
		{
			CAkMusicSegment * pFirstSegmentNode = pFirst->m_pSegment->m_pSegmentNode;
			pFirstSegmentNode->GetEntrySyncPos( *in_pRule, in_iMinStartPosition, in_uCueHashForMatchSrc, out_uSelectedCue, uSyncTime );
			iEntryPosition = (AkInt32)uSyncTime;
			if( in_pRule->bPlayPreEntry && !uSyncTime && in_pRule->eEntryType != EntryTypeSameTime )
			{
				iEntryPosition -= pFirstSegmentNode->PreEntryDuration();
			}
		}
		UpdateChainLength( uSyncTime );
		iLookAhead = pFirst->Prepare( uSyncTime, iEntryPosition, in_pRule->fadeParams );
	}
	else
	{
		uSyncTime = in_iMinStartPosition <= 0 ? 0 : in_iMinStartPosition;
		UpdateChainLength( uSyncTime );
		AkMusicFade fadeParams;
		fadeParams.transitionTime = 0;
		fadeParams.eFadeCurve = AkCurveInterpolation_Linear;
		fadeParams.iFadeOffset = 0;
		iLookAhead = pFirst->Prepare( uSyncTime, in_iMinStartPosition, fadeParams );
	}
	m_uItemsTimeOffset = uSyncTime;
	for( CAkScheduledItem * pItem = pFirst->pNextItem; pItem; pItem = pItem->pNextItem )
	{
		const AkInt32 iItemLookAhead = (AkInt32)uSyncTime - pItem->m_cmdPlay.iRelativeTime - (AkInt32)pItem->m_iLocalTime;
		if( iItemLookAhead > iLookAhead )
		{
			iLookAhead = iItemLookAhead;
		}
	}
	return iLookAhead;
}

/** 2012 0x965330, 2013 0x961b60: the earliest play command and the earliest audible start among the items, relative to the start. */
void CAkChainCtx::QueryLookAheadInfo( AkInt64 & out_iPlayTime, AkInt64 & out_iPlayTimeAudible )
{
	CAkScheduledItem * pFirst = m_chain.First();
	if( !pFirst->m_pSegment )
	{
		out_iPlayTime = 0;
		out_iPlayTimeAudible = 0;
		return;
	}
	AkInt64 iEarliestPlayTime = (AkInt64)pFirst->m_iLocalTime + pFirst->m_cmdPlay.iRelativeTime;
	AkInt64 iEarliestAudibleTime = (AkInt64)pFirst->m_iLocalTime + pFirst->m_pSegment->m_iAudibleTime;
	for( CAkScheduledItem * pItem = pFirst->pNextItem; pItem && pItem->m_pSegment; pItem = pItem->pNextItem )
	{
		const AkInt64 iPlayTime = (AkInt64)pItem->m_iLocalTime + pItem->m_cmdPlay.iRelativeTime;
		if( iPlayTime < iEarliestPlayTime )
		{
			iEarliestPlayTime = iPlayTime;
		}
		const AkInt64 iAudibleTime = (AkInt64)pItem->m_iLocalTime + pItem->m_pSegment->m_iAudibleTime;
		if( iAudibleTime < iEarliestAudibleTime )
		{
			iEarliestAudibleTime = iAudibleTime;
		}
	}
	out_iPlayTime = iEarliestPlayTime - m_uItemsTimeOffset;
	out_iPlayTimeAudible = iEarliestAudibleTime - m_uItemsTimeOffset;
}

/** 2012 0x965410, 2013 0x961c40: not stopping, and no item skipped its playback. */
bool CAkChainCtx::CanRestartPlaying()
{
	if( ( m_eStatus & CtxStatusPlaying ) || m_eStatus == CtxStatusIdle )
	{
		for( CAkScheduledItem * pItem = m_chain.First(); pItem; pItem = pItem->pNextItem )
		{
			if( pItem->m_bWasPlaybackSkipped )
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

/** 2012 0x965440, 2013 0x961c70: a playing chain fades its items out over the time they had played; an idle one just stops. */
void CAkChainCtx::CancelPlayback( AkInt64 in_iCurrentTime )
{
	AddRef();
	if( m_eStatus == CtxStatusIdle )
	{
		OnStopped();
		Release();
		return;
	}
	if( m_eStatus & CtxStatusPlaying )
	{
		const AkInt64 iChainTime = in_iCurrentTime + m_uItemsTimeOffset - m_iLocalTime;
		for( CAkScheduledItem * pItem = m_chain.First(); pItem; pItem = pItem->pNextItem )
		{
			pItem->CancelPlayback( iChainTime );
		}
	}
	Release();
}

/** 2012 0x9654d0, 2013 0x961d00: the item's segment position at the sequencer's time. */
AkInt32 CAkChainCtx::GetSegmentPosition( CAkScheduledItem * in_pItem )
{
	CAkScheduleWindow window( this, true );
	while( window.m_itScheduledItem.pItem != in_pItem )
	{
		GetNextScheduleWindow( window, false );
	}
	const AkInt64 iTime = (AkInt64)m_pSequencer->m_uTime;
	AkInt64 iLocalTime = m_iLocalTime;
	if( m_pParentCtx )
	{
		iLocalTime += static_cast<CAkMatrixAwareCtx *>( m_pParentCtx )->GetAbsoluteTimeOffset();
	}
	if( !window.m_itScheduledItem.pItem->m_pSegment )
	{
		return 0;
	}
	const AkInt64 iCtxTime = iTime - iLocalTime - window.ChainCtxTimeRelativeToLevel( 0 );
	return (AkInt32)( iCtxTime + window.m_itScheduledItem.pCtx->m_uItemsTimeOffset - (AkInt32)window.m_itScheduledItem.pItem->m_iLocalTime );
}

/** 2012 0x965590, 2013 0x961dc0. */
void CAkChainCtx::OnStopped()
{
	AddRef();
	Flush();
	CAkMatrixAwareCtx::OnStopped();
	Release();
}

/**
 * 2012 0x9655b0: every item processes the frame (items at or past an active cutoff skip their play); items that are
 * done (and all before them) are collected once the chain has moved past them.
 */
void CAkChainCtx::Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo )
{
	const AkInt64 iCurrentTime = in_iCurrentTime - m_iLocalTime;
	in_cutoffInfo.iCutoffTime -= m_iLocalTime;
	AddRef();
	if( ( m_eStatus & CtxStatusLastFrame ) && m_uNumLastSamples != (AkUInt32)-1 )
	{
		in_uNumSamples = m_uNumLastSamples;
	}
	const AkInt64 iChainTime = iCurrentTime + m_uItemsTimeOffset;
	const AkInt64 iMaxEnabledItemTime = in_cutoffInfo.iCutoffTime + m_uItemsTimeOffset;
	UpdateChainLength( iChainTime );
	CAkScheduledItem * pPrev = NULL;
	CAkScheduledItem * pItem = m_chain.First();
	bool bCanCollectItems = true;
	while( pItem )
	{
		pItem->Process( iChainTime, in_uNumSamples, in_cutoffInfo.bCutoff && (AkInt64)pItem->m_iLocalTime >= iMaxEnabledItemTime );
		if( bCanCollectItems )
		{
			CAkScheduledItem * pNext = pItem->pNextItem;
			if( pItem->CanDestroy() && pNext && iChainTime > (AkInt64)pNext->m_iLocalTime )
			{
				if( pItem == m_chain.m_pFirst )
				{
					m_chain.m_pFirst = pNext;
				}
				else
				{
					pPrev->pNextItem = pNext;
				}
				if( pItem == m_chain.m_pLast )
				{
					m_chain.m_pLast = pPrev;
				}
				--m_chain.m_ulNumListItems;
				pItem->Destroy();
				pItem = pNext;
				continue;
			}
			bCanCollectItems = false;
		}
		pPrev = pItem;
		pItem = pItem->pNextItem;
	}
	ProcessEpilogue( iCurrentTime, in_uNumSamples );
}
