/*=============================================================================
	AkMath.h - AkMath (akmath.cpp): vectors, dB conversion and the fast log10 the speaker panner and the volume code use.

	DISHONORED(port): agent IH2 (wave 26). Every function is a port of its 2012 body (rva on the definition).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AkMath
{
	AkReal32 ToRadians( AkReal32 in_fDegrees );
	AkReal32 Magnitude( AkVector & in_rVector );
	AkReal32 DotProduct( const AkVector & Vector1, const AkVector & Vector2 );
	AkVector CrossProduct( AkVector & Vector1, AkVector & Vector2 );
	AkReal32 Distance( const AkVector & Position1, const AkVector & Position2 );
	AkReal32 Interpolate( AkReal32 in_fLowerX, AkReal32 in_fLowerY, AkReal32 in_fUpperX, AkReal32 in_fUpperY, AkReal32 in_fX );
	void MatrixMul3by3( AkReal32 * pA, AkReal32 * pB, AkReal32 * pC );
	AkReal32 dBToLin( AkReal32 in_fdB );
	AkReal32 FastLog10( AkReal32 fX );
	void Normalise( AkVector & io_Vector );
}
