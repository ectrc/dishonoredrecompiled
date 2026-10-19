/*=============================================================================
	AkCommon.h - the buffer helpers of akcommon.h that retail compiles out of line: zero padding of a voice buffer and the
	absolute -> relative source offset of a looping source.

	DISHONORED(port): agent IH2 (wave 26). AkPipelineBuffer::FreeMarkers / AkPipelineBufferBase::GetCachedBuffer /
	ReleaseCachedBuffer (akcommon.cpp) are declared with the structs in AkLEngineStructs.h and defined in AkCommon.cpp.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

void ZeroPadBuffer( AkAudioBuffer * io_pAudioBuffer );
void ZeroPrePadBufferInterleaved( AkAudioBuffer * io_pAudioBuffer, AkUInt32 in_ulNumFramestoZero );
void ZeroPrePadBuffer( AkAudioBuffer * io_pAudioBuffer, AkUInt32 in_ulNumFramestoZero );
void AbsoluteToRelativeSourceOffset( AkUInt32 in_uAbsoluteSourcePosition, AkUInt32 in_uLoopStart, AkUInt32 in_uLoopEnd, AkUInt16 in_usLoopCount, AkUInt32 & out_uRelativeSourceOffset, AkUInt16 & out_uRemainingLoops );
