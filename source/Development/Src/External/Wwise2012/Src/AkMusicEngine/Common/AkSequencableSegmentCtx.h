/*=============================================================================
	AkSequencableSegmentCtx.h - CAkSequencableSegmentCtx (aksequencablesegmentctx.cpp): a chain of one segment (plus a
	silent item at its exit cue) - what a music segment plays as on its own, as a switch container's child, or as a
	stinger. It can seek (absolute or percent, optionally snapped to the nearest cue).

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, 0x68): CAkChainCtx (96) + m_pSegmentNode (+96).
=============================================================================*/
#pragma once

#include "AkSegmentChain.h"

class CAkMusicSegment;

class CAkSequencableSegmentCtx : public CAkChainCtx
{
public:
	CAkSequencableSegmentCtx( CAkMusicSegment * in_pSegmentNode, CAkMusicCtx * in_pParentCtx );
	virtual ~CAkSequencableSegmentCtx();

	AKRESULT Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	virtual CAkMusicNode * Node();
	virtual bool IsValid() { return true; }
	virtual AKRESULT SeekTimeAbsolute( AkTimeMs & io_position, bool in_bSnapToCue );
	virtual AKRESULT SeekPercent( AkReal32 & io_fPercent, bool in_bSnapToCue );
	virtual AKRESULT GetPlayingSegmentInfo( AkSegmentInfo & out_segmentInfo );

	CAkMusicSegment * m_pSegmentNode;

private:
	AKRESULT SetupSegmentChain( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams, AkInt32 in_iStartPos, AkInt32 & out_iLookAheadDuration );
	AKRESULT Seek( AkInt32 in_iSeekPosition );
};

static_assert( sizeof( CAkSequencableSegmentCtx ) == 0x68, "CAkSequencableSegmentCtx: 2012 PDB 0x68 (CreateSegmentContext mallocs 0x68)" );
