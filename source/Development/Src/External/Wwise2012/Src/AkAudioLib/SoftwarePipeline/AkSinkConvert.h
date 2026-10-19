/*=============================================================================
	AkSinkConvert.h - the float -> 16-bit converters of aksink.h (FloatsToShortsSSE2 2012 0x95b5b0, FloatsToShortsMMX
	0x95b620) that CAkSinkDirectSound::DoRefill uses when the mix already has the sink's channel layout.

	DISHONORED(port): agent IH2 (wave 26). Both scale by 32767, round to nearest (cvtps2dq / cvtps2pi) and saturate
	(packssdw). They convert in blocks of 16 floats: nFloats is rounded UP to a multiple of 16 (retail's loop count
	((4 * nFloats - 1) >> 6) + 1), so src must be 16-byte aligned and dst large enough for that.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

void FloatsToShortsSSE2( AkInt16 * dst, const AkReal32 * src, AkUInt32 nFloats );
void FloatsToShortsMMX( AkInt16 * dst, const AkReal32 * src, AkUInt32 nFloats );
