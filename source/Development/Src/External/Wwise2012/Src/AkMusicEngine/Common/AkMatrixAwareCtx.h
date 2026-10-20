/*=============================================================================
	AkMatrixAwareCtx.h - CAkMatrixAwareCtx (akmatrixawarectx.cpp): a music context the matrix sequencer drives - a
	switch context, a sequence (playlist) context, a single-segment chain or a "nothing" context. Its local time is its
	start relative to its parent (absolute for a top-level context, whose sequencer it owns through the renderer).
	The schedule interface (GetNextScheduleWindow / RefreshWindow / Prepare / QueryLookAheadInfo / Process) is what
	transitions and stingers use to look at what will play when.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, align 8, 80 bytes): CAkMusicCtx (60) + m_pSequencer (+60) +
	m_iLocalTime (+64) + pNextTopLevelSibling (+72). The virtual interface is retail's slots 9..21 in order.
=============================================================================*/
#pragma once

#include "AkMusicCtx.h"

class CAkMatrixSequencer;
class CAkScheduleWindow;
class CAkMusicNode;
class CAkMusicSegment;

struct AkCutoffInfo
{
	AkInt64	iCutoffTime;
	bool	bCutoff;
};

class CAkMatrixAwareCtx : public CAkMusicCtx
{
public:
	explicit CAkMatrixAwareCtx( CAkMusicCtx * in_parent );
	virtual ~CAkMatrixAwareCtx();

	AKRESULT Init( CAkRegisteredObj * in_pGameObj, UserParams & in_rUserparams );

	virtual void OnStopped();

	virtual void EndInit() {}
	virtual void Process( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples, AkCutoffInfo & in_cutoffInfo ) = 0;
	virtual AkInt32 Prepare( const AkMusicTransDestRule * in_pRule, AkInt32 in_iMinStartPosition, AkUInt32 & out_uSelectedCue, AkUInt32 in_uCueHashForMatchSrc ) = 0;
	virtual void GetNextScheduleWindow( CAkScheduleWindow & io_window, bool in_bDoNotGrow ) = 0;
	virtual void RefreshWindow( CAkScheduleWindow & io_window ) = 0;
	virtual void QueryLookAheadInfo( AkInt64 & out_iPlayTime, AkInt64 & out_iPlayTimeAudible ) = 0;
	virtual CAkMusicNode * Node() = 0;
	virtual bool CanRestartPlaying() = 0;
	virtual void CancelPlayback( AkInt64 in_iCurrentTime ) = 0;
	virtual void PerformDelayedSwitchChange() {}
	/** 2012 0x964820, 2013 0x961060 (both seeks, folded). */
	virtual AKRESULT SeekTimeAbsolute( AkTimeMs & io_position, bool in_bSnapToCue ) { return AK_Fail; }
	virtual AKRESULT SeekPercent( AkReal32 & io_fPercent, bool in_bSnapToCue ) { return AK_Fail; }
	virtual AKRESULT GetPlayingSegmentInfo( AkSegmentInfo & out_segmentInfo );

	AkUInt32 GetSilentDuration();
	CAkMusicSegment * GetFirstSegmentNode( CAkMusicNode ** out_ppParentNode = NULL );
	void SetSequencer( CAkMatrixSequencer * in_pSequencer );
	CAkMatrixSequencer * Sequencer() const { return m_pSequencer; }
	AkInt64 GetAbsoluteTimeOffset();

	CAkMatrixSequencer *	m_pSequencer;
	AkInt64					m_iLocalTime;
	CAkMatrixAwareCtx *		pNextTopLevelSibling;

protected:
	void ProcessEpilogue( AkInt64 in_iCurrentTime, AkUInt32 in_uNumSamples );
};

static_assert( sizeof( CAkMatrixAwareCtx ) == 80, "CAkMatrixAwareCtx: 2012 PDB 80" );
