/*=============================================================================
	AkSegmentCtx.h - CAkSegmentCtx (aksegmentctx.cpp): the playing instance of one music segment, owned by a scheduled
	item of a chain. When played it schedules its tracks' clips (the sub-track each track chose) as play actions, each
	early by its streaming look-ahead; each audio frame it runs the actions that fall in the frame (creating the music
	PBIs through the music renderer, and their stop actions) and applies the clip automation curves.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, 0x74): CAkMusicCtx (60) + m_sequencer (+60) +
	m_pSegmentNode (+76) + m_pOwner (+80) + m_iAudibleTime (+84) + m_arTracks (+88) + m_arTrackRS (+100) +
	m_listAutomation (+112).
=============================================================================*/
#pragma once

#include "AkMusicCtx.h"
#include "AkContextualMusicSequencer.h"

class CAkMusicSegment;
class CAkMusicTrack;
class CAkMusicPBI;
class CAkScheduledItem;
class CAkPBI;

class CAkSegmentCtx : public CAkMusicCtx
{
public:
	CAkSegmentCtx( CAkMusicSegment * in_pSegmentNode, CAkMusicCtx * in_pParentCtx );
	virtual ~CAkSegmentCtx();

	AKRESULT Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );
	AkInt32 Prepare( AkInt32 in_iSourceOffset );
	void Process( AkInt32 in_iTime, AkUInt32 in_uNumSamples );
	void RemoveAllReferences( CAkPBI * in_pCtx );
	bool GetSourceInfoForPlaybackRestart( const CAkMusicPBI * in_pCtx, AkInt32 & out_iLookAhead, AkInt32 & out_iSourceOffset );

	virtual void OnPaused();
	virtual void OnPlayed();
	virtual void OnStopped();

	CAkContextualMusicSequencer								m_sequencer;
	CAkMusicSegment *										m_pSegmentNode;
	CAkScheduledItem *										m_pOwner;
	AkInt32													m_iAudibleTime;
	AkArray<CAkMusicTrack *, CAkMusicTrack *, ArrayPoolDefault, 8>	m_arTracks;
	AkArray<AkUInt16, AkUInt16, ArrayPoolDefault, 16>		m_arTrackRS;
	AkListBareLight<AkMusicAutomation>						m_listAutomation;

private:
	AkInt32 ComputeMinSrcLookAhead( AkInt32 in_iPosition );
	void ScheduleAudioClips();
	void ExecuteScheduledCmds( AkInt32 in_iTime, AkUInt32 in_uNumSamples );
	void Flush();
	void ApplyAutomation( AkMusicAutomation * in_pAutomation, AkInt32 in_iSegmentFrameEnd );
};

static_assert( sizeof( CAkSegmentCtx ) == 0x74, "CAkSegmentCtx: 2012 PDB 0x74 (CreateLowLevelSegmentCtxAndAddRef mallocs 0x74)" );
