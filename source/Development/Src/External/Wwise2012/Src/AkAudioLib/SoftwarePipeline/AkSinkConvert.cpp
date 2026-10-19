/*=============================================================================
	AkSinkConvert.cpp - port of aksink.h's FloatsToShortsSSE2 / FloatsToShortsMMX (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). Retail passes the pointers in registers (edx/esi/eax); here they are ordinary
	cdecl parameters. The MMX twin keeps movntq stores and emms.
=============================================================================*/
#include "AkSinkConvert.h"
#include <emmintrin.h>

/** 2012 0x95b5b0; 2013 0x957e50. */
void FloatsToShortsSSE2( AkInt16 * dst, const AkReal32 * src, AkUInt32 nFloats )
{
	if( !nFloats )
	{
		return;
	}
	const __m128 vScale = _mm_set1_ps( 32767.f );
	__m128i * pDst = (__m128i *)dst;
	for( AkUInt32 n = ( ( 4 * nFloats - 1 ) >> 6 ) + 1; n; --n )
	{
		const __m128i v0 = _mm_cvtps_epi32( _mm_mul_ps( _mm_load_ps( src ), vScale ) );
		const __m128i v1 = _mm_cvtps_epi32( _mm_mul_ps( _mm_load_ps( src + 4 ), vScale ) );
		const __m128i v2 = _mm_cvtps_epi32( _mm_mul_ps( _mm_load_ps( src + 8 ), vScale ) );
		const __m128i v3 = _mm_cvtps_epi32( _mm_mul_ps( _mm_load_ps( src + 12 ), vScale ) );
		_mm_storeu_si128( pDst, _mm_packs_epi32( v0, v1 ) );
		_mm_storeu_si128( pDst + 1, _mm_packs_epi32( v2, v3 ) );
		src += 16;
		pDst += 2;
	}
}

/** 2012 0x95b620; 2013 0x957ec0. */
void FloatsToShortsMMX( AkInt16 * dst, const AkReal32 * src, AkUInt32 nFloats )
{
	if( nFloats )
	{
		const __m128 vScale = _mm_set1_ps( 32767.f );
		__m64 * pDst = (__m64 *)dst;
		for( AkUInt32 n = ( ( 4 * nFloats - 1 ) >> 6 ) + 1; n; --n )
		{
			for( AkUInt32 q = 0; q < 4; ++q )
			{
				const __m128 v = _mm_mul_ps( _mm_load_ps( src + 4 * q ), vScale );
				_mm_stream_pi( pDst + q, _mm_packs_pi32( _mm_cvtps_pi32( v ), _mm_cvtps_pi32( _mm_movehl_ps( v, v ) ) ) );
			}
			src += 16;
			pDst += 4;
		}
	}
	_mm_empty();
}
