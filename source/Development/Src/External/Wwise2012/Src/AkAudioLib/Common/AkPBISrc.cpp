/*=============================================================================
	AkPBISrc.cpp - the akpbi.h members of CAkPBI that the sources call and retail compiles out of line:
	GetSourceOffsetRemainder (2012 0x941600; 2013 0x93df30), SetMediaFormat (0x955900; 0x9521a0),
	GetSeekPosition (0x956b10; 0x9533b0). All three matched by bytes.

	DISHONORED(port): agent IH2 fork S (wave 26). InvalidateFeedbackParameters (0x93f5f0) is akpbi.cpp's and defined
	with the rest of CAkPBI in AkPBI.cpp.
=============================================================================*/
#include "AkPBI.h"

/** 2012 0x941600; 2013 0x93df30: the seek left for the decoder to skip once the source has seeked. */
AkUInt32 CAkPBI::GetSourceOffsetRemainder()
{
	return m_bSeekDirty ? 0 : m_uSeekPosition;
}

/** 2012 0x955900; 2013 0x9521a0. */
void CAkPBI::SetMediaFormat( const AkAudioFormat & in_rFormat )
{
	m_sMediaFormat = in_rFormat;
	InvalidateFeedbackParameters();
}

/** 2012 0x956b10; 2013 0x9533b0: a seek by percentage, in samples of a source lasting in_fSourceDuration ms. */
AkUInt32 CAkPBI::GetSeekPosition( AkReal32 in_fSourceDuration, bool & out_bSnapToMarker )
{
	out_bSnapToMarker = m_bSnapSeekToMarker != 0;
	return (AkUInt32)(AkInt64)( (AkReal64)m_sMediaFormat.uSampleRate * m_fSeekPercent * in_fSourceDuration * 0.001 );
}
