/*=============================================================================
	mdct.cpp - the inverse MDCT and the overlap-add of Wwise's Vorbis decoder: Tremor lowmem mdct.c in float, with
	Audiokinetic's SSE butterflies.

	DISHONORED(port): agent IH2 (wave 26), transcribed from the 2012 decompile (the SSE stages shuffle for shuffle, the
	scalar stages in retail's operation order). Tremor's MULT31 / MULT32 / >>1 become float products and * 0.5f, the
	output scale >>9 becomes * (1 / 512). Retail behaviour kept: mdct_step8's interpolated case (step 0, n = 8192) writes
	its last pair to x[5], x[6] (Tremor's lowmem x+5 / x+6 slip; retail stores exactly that); unreachable for the
	256 / 2048 blocks Dishonored's media use.
	Deviation (named): the SSE loads and stores are unaligned (_mm_loadu_ps / _mm_storeu_ps) where retail uses movaps on
	16-byte aligned work vectors; the results are identical, only the alignment requirement is dropped.
=============================================================================*/
#include "codec_internal.h"
#include <xmmintrin.h>

static const AkUInt32 _mask_NegFloat_ = 0x80000000;

static inline float NegFloat( float in_f )
{
	AkVorbisFloatBits bits;
	bits.f = in_f;
	bits.u ^= _mask_NegFloat_;
	return bits.f;
}

static inline __m128 Ld( const float * in_p )
{
	return _mm_loadu_ps( in_p );
}

static inline void St( float * in_p, __m128 in_v )
{
	_mm_storeu_ps( in_p, in_v );
}

#define SHUF( _a_, _b_, _imm_ ) _mm_shuffle_ps( ( _a_ ), ( _b_ ), ( _imm_ ) )

static void presymmetry( float * in, int n2, int step )
{
	const int n4 = n2 >> 1;
	float * aX = in + n2 - 3;
	const float * T = fsincos_lookup0;
	do
	{
		const float s0 = aX[ 0 ];
		const float s2 = aX[ 2 ];
		aX[ 0 ] = ( T[ 1 ] * s2 ) + ( T[ 0 ] * s0 );
		aX[ 2 ] = ( T[ 0 ] * s2 ) - ( T[ 1 ] * s0 );
		aX -= 4;
		T += step;
	}
	while( aX >= in + n4 );
	do
	{
		const float s0 = aX[ 0 ];
		const float s2 = aX[ 2 ];
		aX[ 0 ] = ( T[ 0 ] * s2 ) + ( T[ 1 ] * s0 );
		aX[ 2 ] = ( T[ 1 ] * s2 ) - ( T[ 0 ] * s0 );
		aX -= 4;
		T -= step;
	}
	while( aX >= in );

	aX = in + n2 - 4;
	float * bX = in;
	T = fsincos_lookup0;
	do
	{
		const float ro0 = bX[ 0 ];
		const float ro2 = bX[ 2 ];
		const float ri0 = aX[ 0 ];
		const float ri2 = aX[ 2 ];
		const float t0 = T[ 0 ];
		const float t1 = T[ 1 ];
		aX[ 2 ] = ( t0 * ro2 ) + ( t1 * ro0 );
		T += step;
		aX[ 0 ] = ( t1 * ro2 ) - ( t0 * ro0 );
		const float b2 = ( T[ 1 ] * ri2 ) + ( T[ 0 ] * ri0 );
		bX[ 0 ] = ( T[ 0 ] * ri2 ) - ( T[ 1 ] * ri0 );
		bX[ 2 ] = b2;
		aX -= 4;
		bX += 4;
	}
	while( aX >= bX );
}

static void mdct_butterfly_generic( float * x, int points, int step )
{
	const float * T = fsincos_lookup0;
	float * x1 = x + points - 4;
	float * x2 = x + ( points >> 1 ) - 4;
	do
	{
		const __m128 a = Ld( x1 );
		const __m128 b = Ld( x2 );
		const __m128 t = Ld( T );
		const __m128 r = _mm_sub_ps( SHUF( a, b, 220 ), SHUF( a, b, 137 ) );
		const __m128 c = SHUF( b, a, 141 );
		const __m128 d = SHUF( b, a, 216 );
		St( x1, _mm_add_ps( SHUF( d, d, 114 ), SHUF( c, c, 114 ) ) );
		St( x2, _mm_add_ps( _mm_mul_ps( _mm_mul_ps( SHUF( t, t, 85 ), SHUF( r, r, 156 ) ), vfsign1 ), _mm_mul_ps( SHUF( t, t, 0 ), SHUF( r, r, 201 ) ) ) );
		T += step;
		x1 -= 4;
		x2 -= 4;
	}
	while( T < fsincos_lookup0 + 1024 );
	do
	{
		const __m128 a = Ld( x1 );
		const __m128 b = Ld( x2 );
		const __m128 t = Ld( T );
		const __m128 r = _mm_sub_ps( SHUF( a, b, 200 ), SHUF( a, b, 157 ) );
		const __m128 c = SHUF( b, a, 141 );
		const __m128 d = SHUF( b, a, 216 );
		St( x1, _mm_add_ps( SHUF( d, d, 114 ), SHUF( c, c, 114 ) ) );
		St( x2, _mm_add_ps( _mm_mul_ps( _mm_mul_ps( SHUF( t, t, 85 ), SHUF( r, r, 201 ) ), vfsign2 ), _mm_mul_ps( SHUF( t, t, 0 ), SHUF( r, r, 156 ) ) ) );
		T -= step;
		x1 -= 4;
		x2 -= 4;
	}
	while( T > fsincos_lookup0 );
}

static void mdct_butterfly_32( float * x )
{
	float * p = x + 12;
#define P( _i_ ) ( p + 4 * ( _i_ ) )
	const __m128 v41 = Ld( P( -3 ) );
	const __m128 p1 = Ld( P( 1 ) );
	const __m128 v42 = SHUF( p1, v41, 216 );
	const __m128 v43 = SHUF( p1, v41, 141 );
	const __m128 v44 = _mm_sub_ps( v42, v43 );
	const __m128 v45 = _mm_add_ps( SHUF( v43, v43, 216 ), SHUF( v42, v42, 216 ) );
	const __m128 v46 = Ld( P( -2 ) );
	St( P( -3 ), _mm_add_ps( _mm_mul_ps( SHUF( v44, v44, 216 ), vfcPI_3131 ), _mm_mul_ps( SHUF( v44, v44, 141 ), vfcPI_m131m3 ) ) );
	const __m128 p2 = Ld( P( 2 ) );
	const __m128 v47 = SHUF( p2, v46, 216 );
	const __m128 v48 = SHUF( p2, v46, 141 );
	const __m128 v49 = _mm_sub_ps( v47, v48 );
	St( P( 1 ), v45 );
	const __m128 v51 = Ld( P( -1 ) );
	const __m128 v52 = _mm_add_ps( SHUF( v48, v48, 216 ), SHUF( v47, v47, 216 ) );
	St( P( -2 ), _mm_mul_ps( _mm_add_ps( _mm_mul_ps( SHUF( v49, v49, 153 ), vfsign3 ), SHUF( v49, v49, 204 ) ), vfcPI2_8 ) );
	const __m128 p3 = Ld( P( 3 ) );
	const __m128 v53 = SHUF( p3, v51, 216 );
	const __m128 v54 = SHUF( p3, v51, 141 );
	St( P( 2 ), v52 );
	const __m128 v56 = _mm_sub_ps( v53, v54 );
	const __m128 v57 = _mm_add_ps( SHUF( v54, v54, 216 ), SHUF( v53, v53, 216 ) );
	const __m128 v58 = _mm_add_ps( _mm_mul_ps( SHUF( v56, v56, 216 ), vfcPI_1313 ), _mm_mul_ps( SHUF( v56, v56, 141 ), vfcPI_m313m1 ) );
	St( P( 3 ), v57 );
	const __m128 v59 = Ld( P( 0 ) );
	St( P( -1 ), v58 );
	const __m128 p4 = Ld( P( 4 ) );
	const __m128 v60 = _mm_sub_ps( SHUF( p4, v59, 200 ), SHUF( p4, v59, 157 ) );
	const __m128 v61 = SHUF( p4, v59, 216 );
	const __m128 v62 = SHUF( p4, v59, 141 );
	St( P( 4 ), _mm_add_ps( SHUF( v62, v62, 216 ), SHUF( v61, v61, 216 ) ) );
	const __m128 v64 = v58;
	const __m128 v65 = Ld( P( -3 ) );
	const __m128 v66 = SHUF( v64, v65, 216 );
	const __m128 v67 = SHUF( v64, v65, 141 );
	const __m128 v68 = _mm_sub_ps( v66, v67 );
	const __m128 v69 = SHUF( v60, v60, 156 );
	const __m128 v71 = Ld( P( -2 ) );
	St( P( -1 ), _mm_add_ps( SHUF( v67, v67, 216 ), SHUF( v66, v66, 216 ) ) );
	const __m128 v77 = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( SHUF( v68, v68, 157 ), vfsign3 ), SHUF( v68, v68, 200 ) ), vfcPI2_8 );
	const __m128 v72 = _mm_sub_ps( SHUF( v71, v69, 140 ), SHUF( v71, v69, 217 ) );
	const __m128 v74 = SHUF( v69, v71, 141 );
	const __m128 v75 = SHUF( v69, v71, 216 );
	const __m128 v76 = _mm_add_ps( SHUF( v74, v74, 216 ), SHUF( v75, v75, 216 ) );
	const __m128 v78 = SHUF( v72, v72, 54 );
	const __m128 v79 = SHUF( v77, v78, 136 );
	const __m128 v80 = SHUF( v77, v78, 221 );
	const __m128 v82 = _mm_sub_ps( v79, v80 );
	const __m128 v83 = _mm_add_ps( v79, v80 );
	St( P( -3 ), _mm_add_ps( _mm_mul_ps( SHUF( v82, v82, 17 ), vfsign4 ), SHUF( v82, v82, 238 ) ) );
	St( P( -2 ), _mm_add_ps( _mm_mul_ps( SHUF( v83, v83, 68 ), vfsign2 ), SHUF( v83, v83, 238 ) ) );
	St( P( 0 ), v76 );
	const __m128 pm1 = Ld( P( -1 ) );
	const __m128 v84 = SHUF( pm1, v76, 221 );
	const __m128 v85 = SHUF( pm1, v76, 136 );
	const __m128 v86 = _mm_add_ps( v85, v84 );
	const __m128 v87 = _mm_sub_ps( v85, v84 );
	St( P( 0 ), _mm_add_ps( _mm_mul_ps( SHUF( v86, v86, 68 ), vfsign2 ), SHUF( v86, v86, 238 ) ) );
	St( P( -1 ), _mm_add_ps( _mm_mul_ps( SHUF( v87, v87, 17 ), vfsign4 ), SHUF( v87, v87, 238 ) ) );
	const __m128 v88 = Ld( P( 1 ) );
	const __m128 v89 = Ld( P( 3 ) );
	const __m128 v90 = SHUF( v89, v88, 141 );
	const __m128 v91 = SHUF( v89, v88, 216 );
	const __m128 v93 = _mm_sub_ps( v91, v90 );
	const __m128 v94 = _mm_add_ps( SHUF( v90, v90, 216 ), SHUF( v91, v91, 216 ) );
	const __m128 v102 = _mm_mul_ps( _mm_add_ps( _mm_mul_ps( SHUF( v93, v93, 157 ), vfsign3 ), SHUF( v93, v93, 200 ) ), vfcPI2_8 );
	const __m128 v95 = Ld( P( 4 ) );
	St( P( 3 ), v94 );
	const __m128 v96 = Ld( P( 2 ) );
	const __m128 v97 = _mm_sub_ps( SHUF( v96, v95, 140 ), SHUF( v96, v95, 217 ) );
	const __m128 v99 = SHUF( v95, v96, 141 );
	const __m128 v100 = SHUF( v95, v96, 216 );
	const __m128 v101 = _mm_add_ps( SHUF( v99, v99, 216 ), SHUF( v100, v100, 216 ) );
	const __m128 v103 = SHUF( v97, v97, 54 );
	const __m128 v104 = SHUF( v102, v103, 136 );
	const __m128 v105 = SHUF( v102, v103, 221 );
	const __m128 v107 = _mm_sub_ps( v104, v105 );
	const __m128 v108 = _mm_add_ps( v104, v105 );
	St( P( 1 ), _mm_add_ps( _mm_mul_ps( SHUF( v107, v107, 17 ), vfsign4 ), SHUF( v107, v107, 238 ) ) );
	St( P( 2 ), _mm_add_ps( _mm_mul_ps( SHUF( v108, v108, 68 ), vfsign2 ), SHUF( v108, v108, 238 ) ) );
	const __m128 v110 = SHUF( v94, v101, 221 );
	const __m128 v111 = SHUF( v94, v101, 136 );
	const __m128 v112 = _mm_sub_ps( v111, v110 );
	const __m128 v113 = _mm_add_ps( v111, v110 );
	St( P( 3 ), _mm_add_ps( _mm_mul_ps( SHUF( v112, v112, 17 ), vfsign4 ), SHUF( v112, v112, 238 ) ) );
	St( P( 4 ), _mm_add_ps( _mm_mul_ps( SHUF( v113, v113, 68 ), vfsign2 ), SHUF( v113, v113, 238 ) ) );
#undef P
}

static void mdct_butterflies( float * x, int points, int shift )
{
	int stages = 8 - shift;
	for( int i = 0; --stages > 0; ++i )
	{
		for( int j = 0; j < ( 1 << i ); ++j )
		{
			mdct_butterfly_generic( x + ( points >> i ) * j, points >> i, 4 << ( i + shift ) );
		}
	}
	for( int j = 0; j < points; j += 32 )
	{
		mdct_butterfly_32( x + j );
	}
}

static int bitrev12( int x )
{
	return bitrev[ x >> 8 ] | ( ( ( bitrev[ x & 0x0f ] << 4 ) | bitrev[ ( x & 0xf0 ) >> 4 ] ) << 4 );
}

static void mdct_bitreverse( float * x, int n, int shift )
{
	int bit = 0;
	float * w = x + ( n >> 1 );
	do
	{
		float * xx = x + ( bitrev12( bit++ ) >> shift );
		w -= 2;
		if( w > xx )
		{
			float r = xx[ 0 ];
			xx[ 0 ] = w[ 0 ];
			w[ 0 ] = r;
			r = xx[ 1 ];
			xx[ 1 ] = w[ 1 ];
			w[ 1 ] = r;
		}
	}
	while( w > x );
}

static void mdct_step7( float * x, int n, int step )
{
	float * w0 = x;
	float * w1 = x + ( n >> 1 );
	const float * T = ( step >= 4 ) ? ( fsincos_lookup0 + ( step >> 1 ) ) : fsincos_lookup1;
	const float * Ttop = T + 1024;
	do
	{
		const float w1_0 = w1[ -2 ];
		const float w0_1 = w0[ 1 ];
		const float w1_1 = w1[ -1 ];
		w1 -= 2;
		const float r1 = w1_1 - w0_1;
		const float r0 = w0[ 0 ] + w1_0;
		const float d = w0[ 0 ] - w1[ 0 ];
		const float r2 = ( ( T[ 0 ] * r1 ) + ( T[ 1 ] * r0 ) ) * 0.5f;
		const float s0 = ( w0_1 + w1[ 1 ] ) * 0.5f;
		const float r3 = ( ( T[ 1 ] * r1 ) - ( T[ 0 ] * r0 ) ) * 0.5f;
		const float s1 = d * 0.5f;
		w0[ 0 ] = r2 + s0;
		w0[ 1 ] = r3 + s1;
		T += step;
		w0 += 2;
		w1[ 0 ] = s0 - r2;
		w1[ 1 ] = r3 - s1;
	}
	while( T < Ttop );
	do
	{
		const float w0_1 = w0[ 1 ];
		const float w1_0 = w1[ -2 ];
		const float w1_1 = w1[ -1 ];
		w1 -= 2;
		const float r1 = w1_1 - w0_1;
		T -= step;
		const float r0 = w0[ 0 ] + w1_0;
		const float d = w0[ 0 ] - w1[ 0 ];
		const float r2 = ( ( r1 * T[ 1 ] ) + ( T[ 0 ] * r0 ) ) * 0.5f;
		const float s0 = ( w0_1 + w1[ 1 ] ) * 0.5f;
		w0[ 0 ] = r2 + s0;
		const float r3 = ( ( T[ 0 ] * r1 ) - ( r0 * T[ 1 ] ) ) * 0.5f;
		const float s1 = d * 0.5f;
		w0[ 1 ] = r3 + s1;
		w0 += 2;
		w1[ 0 ] = s0 - r2;
		w1[ 1 ] = r3 - s1;
	}
	while( w0 < w1 );
}

static void mdct_step8( float * x, int n, int step )
{
	float * const iX = x + ( n >> 1 );
	step >>= 2;
	if( step >= 2 )
	{
		const float * T = ( step >= 4 ) ? ( fsincos_lookup0 + ( step >> 1 ) ) : fsincos_lookup1;
		do
		{
			const float r1 = NegFloat( x[ 1 ] );
			const float x1 = ( T[ 0 ] * r1 ) - ( T[ 1 ] * x[ 0 ] );
			x[ 0 ] = ( T[ 1 ] * r1 ) + ( T[ 0 ] * x[ 0 ] );
			x[ 1 ] = x1;
			x += 2;
			T += step;
		}
		while( x < iX );
	}
	else if( step == 1 )
	{
		const float * V = fsincos_lookup1;
		const float * T = fsincos_lookup0 + 2;
		float t0 = fsincos_lookup0[ 0 ] * 0.5f;
		float t1 = fsincos_lookup0[ 1 ] * 0.5f;
		x += 2;
		for( ;; )
		{
			const float r1 = NegFloat( x[ -1 ] );
			const float v0 = V[ 0 ] * 0.5f;
			const float a0 = v0 + t0;
			const float v1 = V[ 1 ] * 0.5f;
			const float a1 = v1 + t1;
			const float r0 = x[ -2 ];
			x[ -2 ] = ( r1 * a1 ) + ( r0 * a0 );
			const float r2 = x[ 0 ];
			x[ -1 ] = ( r1 * a0 ) - ( r0 * a1 );
			const float r3 = NegFloat( x[ 1 ] );
			const float nt1 = T[ 1 ] * 0.5f;
			const float b1 = v1 + nt1;
			t0 = T[ 0 ] * 0.5f;
			const float b0 = v0 + t0;
			x[ 0 ] = ( b1 * r3 ) + ( b0 * r2 );
			x[ 1 ] = ( b0 * r3 ) - ( b1 * r2 );
			x += 4;
			V += 2;
			T += 2;
			if( x - 2 >= iX )
			{
				break;
			}
			t1 = nt1;
		}
	}
	else
	{
		float t0 = fsincos_lookup0[ 0 ];
		float t1 = fsincos_lookup0[ 1 ];
		const float * V = fsincos_lookup1;
		const float * T = fsincos_lookup0 + 2;
		x += 2;
		do
		{
			const float v0 = V[ 0 ];
			const float v1 = V[ 1 ];
			const float q0 = ( v0 - t0 ) * 0.25f;
			const float a0 = q0 + t0;
			const float q1 = ( v1 - t1 ) * 0.25f;
			const float r0 = x[ -2 ];
			const float a1 = q1 + t1;
			const float r1 = NegFloat( x[ -1 ] );
			x[ -1 ] = ( r1 * a0 ) - ( r0 * a1 );
			const float r3 = NegFloat( x[ 1 ] );
			x[ -2 ] = ( r1 * a1 ) + ( r0 * a0 );
			const float r2 = x[ 0 ];
			x[ 1 ] = ( r3 * ( v0 - q0 ) ) - ( x[ 0 ] * ( v1 - q1 ) );
			x[ 0 ] = ( r3 * ( v1 - q1 ) ) + ( r2 * ( v0 - q0 ) );
			t0 = T[ 0 ];
			t1 = T[ 1 ];
			const float c4 = ( NegFloat( x[ 3 ] ) * ( ( ( t1 - v1 ) * 0.25f ) + v1 ) ) + ( x[ 2 ] * ( ( ( t0 - v0 ) * 0.25f ) + v0 ) );
			const float b0 = t0 - ( ( t0 - v0 ) * 0.25f );
			x[ 2 ] = c4;
			V += 2;
			const float b1 = t1 - ( ( t1 - v1 ) * 0.25f );
			T += 2;
			const float r7 = NegFloat( x[ 5 ] );
			const float c5 = ( r7 * b1 ) + ( x[ 4 ] * b0 );
			const float c6 = ( r7 * b0 ) - ( x[ 4 ] * b1 );
			x[ 3 ] = c5;
			x[ 4 ] = c6;
			x += 8;
		}
		while( x - 2 < iX );
	}
}

/** 2012 0x977020; 2013 0x973800. presymmetry / butterflies / bitreverse / step7 / step8 are inlined in retail. */
void mdct_backward( int n, float * in )
{
	int shift;
	for( shift = 4; !( n & ( 1 << shift ) ); ++shift )
	{
	}
	shift = 13 - shift;
	const int step = 2 << shift;
	presymmetry( in, n >> 1, step );
	mdct_butterflies( in, n >> 1, shift );
	mdct_bitreverse( in, n, shift );
	mdct_step7( in, n, step );
	mdct_step8( in, n, step );
}

/** 2012 0x977bf0; 2013 0x9743d0. */
void mdct_shift_right( int n, long * in, long * right )
{
	for( int i = 0; i < n >> 2; ++i )
	{
		right[ i ] = in[ i * 2 + 1 ];
	}
}

static inline short ClipTo15( float in_f )
{
	const int x = (int)in_f;
	return (short)( x - ( ( x >= -32768 ) ? 0 : x + 32768 ) - ( ( x - 32767 ) & ( ( x <= 32767 ) - 1 ) ) );
}

/** 2012 0x977c20; 2013 0x974400. */
void mdct_unroll_lap( int n0, int n1, int lW, int W, float * in, float * right, const float * w0, const float * w1, short * out, int step, int start, int end )
{
	float * l = in + ( ( W && lW ) ? n1 >> 1 : n0 >> 1 );
	float * r = right + ( lW ? n1 >> 2 : n0 >> 2 );
	const float * wR = ( W && lW ) ? w1 + ( n1 >> 1 ) : w0 + ( n0 >> 1 );
	const float * wL = ( W && lW ) ? w1 : w0;
	const int preLap = ( lW && !W ) ? ( n1 >> 2 ) - ( n0 >> 2 ) : 0;
	const int halfLap = ( lW && W ) ? ( n1 >> 2 ) : ( n0 >> 2 );
	const int postLap = ( !lW && W ) ? ( n1 >> 2 ) - ( n0 >> 2 ) : 0;
	float * post;
	int n;
	int off;

	if( preLap )
	{
		n = ( end < preLap ) ? end : preLap;
		off = ( start < preLap ) ? start : preLap;
		post = r - n;
		r -= off;
		start -= off;
		end -= n;
		while( r > post )
		{
			*out = ClipTo15( *--r * 0.001953125f );
			out += step;
		}
	}

	n = ( end < halfLap ) ? end : halfLap;
	off = ( start < halfLap ) ? start : halfLap;
	post = r - n;
	r -= off;
	l -= off * 2;
	start -= off;
	wR -= off;
	wL += off;
	end -= n;
	while( r > post )
	{
		l -= 2;
		const float a = l[ 0 ] * *wL++;
		--wR;
		--r;
		*out = ClipTo15( ( a + ( *wR * *r ) ) * 0.001953125f );
		out += step;
	}

	n = ( end < halfLap ) ? end : halfLap;
	off = ( start < halfLap ) ? start : halfLap;
	post = r + n;
	r += off;
	l += off * 2;
	start -= off;
	end -= n;
	wR -= off;
	wL += off;
	while( r < post )
	{
		const float a = *--wR * *r++;
		const float b = *l * *wL++;
		*out = ClipTo15( ( a - b ) * 0.001953125f );
		out += step;
		l += 2;
	}

	if( postLap )
	{
		n = ( end < postLap ) ? end : postLap;
		off = ( start < postLap ) ? start : postLap;
		post = l + n * 2;
		l += off * 2;
		while( l < post )
		{
			*out = ClipTo15( *l * -0.001953125f );
			out += step;
			l += 2;
		}
	}
}
