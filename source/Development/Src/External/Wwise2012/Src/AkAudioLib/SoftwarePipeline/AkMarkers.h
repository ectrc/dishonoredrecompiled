/*=============================================================================
	AkMarkers.h - CAkMarkers (akmarkers.cpp): the cue points of a media file and the labels the 'labl' chunks give them.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's: { AkMarkersHeader, AkAudioMarker * }.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

struct AkMarkersHeader
{
	AkUInt32 uNumMarkers;
};

class CAkMarkers
{
public:
	CAkMarkers();

	AKRESULT Allocate( AkUInt32 in_uNumMarkers );
	void Free();
	AKRESULT SetLabel( AkUInt32 in_idx, char * in_psLabel, AkUInt32 in_uStrSize );
	void CopyRelevantMarkers( CAkPBI * in_pCtx, AkPipelineBuffer & io_buffer, AkUInt32 in_ulBufferStartPos );
	void NotifyRelevantMarkers( CAkPBI * in_pCtx, AkUInt32 in_uStartSample, AkUInt32 in_uStopSample );
	void TimeSkipMarkers( CAkPBI * in_pCtx, AkUInt32 in_ulCurrSampleOffset, AkUInt32 in_uSkippedSamples, AkUInt32 in_uFileEnd );
	const AkAudioMarker * GetClosestMarker( AkUInt32 in_uPosition ) const;

	AkMarkersHeader m_hdrMarkers;
	AkAudioMarker * m_pMarkers;
};
