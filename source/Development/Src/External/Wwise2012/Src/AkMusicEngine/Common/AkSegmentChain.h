/*=============================================================================
	AkSegmentChain.h - AkScheduledChain and CAkChainCtx (aksegmentchain.cpp): a context that plays a chain of scheduled
	items (segments, or a silent "nothing" item) one after the other: the single segment of a CAkSequencableSegmentCtx,
	the playlist of a CAkSequenceCtx, or the silence of a CAkNothingCtx. m_uItemsTimeOffset is the chain time of the
	context's start (the destination entry position).

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB): CAkMatrixAwareCtx (80) + m_chain (+80) +
	m_uItemsTimeOffset (+92) = 96. Retail virtual slots 22..25 (IsValid pure, Grow, UpdateChainLength,
	HandleFatalError) follow the matrix-aware ones; Grow returns false, UpdateChainLength does nothing and
	HandleFatalError returns the last item in this base (retail's folded bodies at 0x9dee90 / 0xa26ea0 / 0x964ef0).
=============================================================================*/
#pragma once

#include "AkMatrixAwareCtx.h"

class CAkScheduledItem;
class CAkSegmentCtx;
class CAkChainCtx;

class AkScheduledChain : public AkListBare<CAkScheduledItem>
{
public:
	/** An iterator that knows its chain context (for the chain <-> segment time conversions). */
	struct SelfContainedIter
	{
		SelfContainedIter() : pItem( NULL ), pCtx( NULL ) {}

		AkInt64 GetScheduledItemTime() const;
		AkInt64 CtxTimeToSegmentPosition( AkInt64 in_iTime ) const;
		AkInt64 SegmentPositionToCtxTime( AkInt64 in_iSegmentPosition ) const;

		CAkScheduledItem *	pItem;
		CAkChainCtx *		pCtx;
	};
};

class CAkChainCtx : public CAkMatrixAwareCtx
{
public:
	explicit CAkChainCtx( CAkMusicCtx * in_parent );
	virtual ~CAkChainCtx();

	virtual void OnStopped();
	virtual void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo );
	virtual AkInt32 Prepare( const AkMusicTransDestRule * in_pRule, AkInt32 in_iMinStartPosition, AkUInt32 & out_uSelectedCue, AkUInt32 in_uCueHashForMatchSrc );
	virtual void GetNextScheduleWindow( CAkScheduleWindow & io_window, bool in_bDoNotGrow );
	virtual void RefreshWindow( CAkScheduleWindow & io_window );
	virtual void QueryLookAheadInfo( AkInt64 & out_iPlayTime, AkInt64 & out_iPlayTimeAudible );
	virtual bool CanRestartPlaying();
	virtual void CancelPlayback( AkInt64 in_iCurrentTime );

	virtual bool IsValid() = 0;
	virtual bool Grow() { return false; }
	virtual void UpdateChainLength( AkInt64 in_iDesiredPosition ) {}
	virtual CAkScheduledItem * HandleFatalError() { return m_chain.Last(); }

	AkInt32 GetSegmentPosition( CAkScheduledItem * in_pItem );
	CAkScheduledItem * EnqueueItem( AkUInt64 in_uTimeOffset, CAkSegmentCtx * in_pSegment );
	void Flush();

	AkScheduledChain	m_chain;
	AkUInt32			m_uItemsTimeOffset;

private:
	void ClearChain();
};

static_assert( sizeof( CAkChainCtx ) == 96, "CAkChainCtx: 2012 PDB 96" );
