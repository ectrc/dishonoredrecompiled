/*=============================================================================
	AkSrcBase.h - CAkSrcBaseEx (aksrcbase.cpp/.h): the base of every Wwise-file source (PCM, ADPCM, Vorbis, bank or
	streamed): the sample position, the loop region and count, the markers.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's (52 bytes); OnLoopComplete is the one virtual
	it adds after CAkVPLSrcNode's.
=============================================================================*/
#pragma once

#include "AkVPLNode.h"
#include "AkMarkers.h"

class CAkSrcBaseEx : public CAkVPLSrcNode
{
public:
	explicit CAkSrcBaseEx( CAkPBI * in_pCtx );
	virtual ~CAkSrcBaseEx();

	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	virtual void StopStream();
	virtual AkReal32 GetDuration() const;
	virtual AKRESULT StopLooping();

	AkUInt32 m_uTotalSamples;
	AkUInt32 m_uCurSample;
	AkUInt32 m_uDataSize;
	AkUInt32 m_uDataOffset;
	AkUInt32 m_uPCMLoopStart;
	AkUInt32 m_uPCMLoopEnd;
	CAkMarkers m_markers;
	AkUInt16 m_uLoopCnt;

protected:
	virtual AKRESULT OnLoopComplete( bool in_bEndOfFile );

	void SubmitBufferAndUpdate( void * in_pData, AkUInt16 in_uNumSamplesProduced, AkUInt32 in_uSampleRate, AkChannelMask in_channelMask, AkVPLState & io_state );
	AkUInt32 GetSourceOffset();

private:
	void TimeSkipMarkersAndPosition( AkUInt32 in_ulCurrSampleOffset, AkUInt32 in_uSkippedSamples, AkUInt32 in_uFileEnd );
};
