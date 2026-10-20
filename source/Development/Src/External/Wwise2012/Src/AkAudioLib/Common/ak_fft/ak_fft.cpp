/*=============================================================================
	ak_fft.cpp - Audiokinetic's kiss-FFT derivative, Wwise 2012.1 (akaudiolib\common\ak_fft\ak_fft.cpp).

	Agent IH4 (fork F2): every function is ported from the 2012 decompile (rvas on each), 2013 twins matched by bytes.
	Retail's radix-4 butterfly processes two outputs per SSE register when m is even (an odd-m radix-4 stage takes the
	generic butterfly, as kf_work 0x995bc0 decides) and keeps the scalar loop for m < 2; the other radices are scalar.
	The generic butterfly uses a fixed 31-entry stack scratch (no allocation), as retail.
	Deviation (named): the twiddle tables are computed with the CRT's double cos/sin like retail; x87 (retail) vs SSE2
	(ours) double evaluation can differ in the last float ulp.
=============================================================================*/
#include "ak_fft.h"
#include <math.h>

namespace DSP
{
namespace AkFFTAllButterflies
{

/** The SIMD complex-multiply sign, 2012 rva 0xda9740 = (1, -1, 1, -1). */
static const __m128 vSign = { 1.f, -1.f, 1.f, -1.f };

/** 2012 0x995000, 2013 0x991c60. */
static inline __m128 AKSIMD_LOADUNALIGNED_DUAL( const ak_fft_cpx * in_dAddress1, const ak_fft_cpx * in_dAddress2 )
{
	return _mm_movelh_ps( _mm_loadl_pi( _mm_setzero_ps(), (const __m64 *)in_dAddress1 ), _mm_loadl_pi( _mm_setzero_ps(), (const __m64 *)in_dAddress2 ) );
}

/** Two complex products at once, the sequence retail inlines in kf_bfly4 and ak_fftr. */
static inline __m128 ComplexMul2( __m128 in_v, __m128 in_tw )
{
	const __m128 vIm = _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( in_v, in_v, 245 ), vSign ), in_tw );
	return _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps( in_v, in_v, 160 ), in_tw ), _mm_shuffle_ps( vIm, vIm, 177 ) );
}

/** 2012 0x994f60, 2013 0x991bc0. */
static void kf_bfly2( ak_fft_cpx * Fout, ak_fft_state * const st, unsigned int fstride, int m )
{
	ak_fft_cpx * tw1 = st->twiddles;
	ak_fft_cpx * Fout2 = Fout + m;
	do
	{
		const float tr = ( tw1->r * Fout2->r ) - ( tw1->i * Fout2->i );
		const float ti = ( Fout2->i * tw1->r ) + ( tw1->i * Fout2->r );
		Fout2->r = Fout->r - tr;
		Fout2->i = Fout->i - ti;
		Fout->r = Fout->r + tr;
		Fout->i = Fout->i + ti;
		tw1 += fstride;
		++Fout2;
		++Fout;
	}
	while( --m );
}

/** 2012 0x995010, 2013 0x991c70. */
static void kf_bfly4( ak_fft_cpx * Fout, unsigned int m, unsigned int fstride, ak_fft_state * const st )
{
	ak_fft_cpx * tw1 = st->twiddles;
	ak_fft_cpx * tw2 = st->twiddles;
	ak_fft_cpx * tw3 = st->twiddles;
	if( m < 2 )
	{
		const int inverse = st->inverse;
		unsigned int k = m;
		do
		{
			const float s0i = ( tw1->i * Fout[ m ].r ) + ( Fout[ m ].i * tw1->r );
			const float s0r = ( tw1->r * Fout[ m ].r ) - ( tw1->i * Fout[ m ].i );
			const float s2i = ( tw3->i * Fout[ 3 * m ].r ) + ( Fout[ 3 * m ].i * tw3->r );
			const float s2r = ( tw3->r * Fout[ 3 * m ].r ) - ( tw3->i * Fout[ 3 * m ].i );
			const float s3i = s2i + s0i;
			const float s4i = s0i - s2i;
			const float s3r = s2r + s0r;
			const float s4r = s0r - s2r;
			const float s1i = ( tw2->i * Fout[ 2 * m ].r ) + ( Fout[ 2 * m ].i * tw2->r );
			const float s1r = ( tw2->r * Fout[ 2 * m ].r ) - ( tw2->i * Fout[ 2 * m ].i );
			const float f0r = Fout->r;
			const float f0i = Fout->i;
			Fout->r = f0r + s1r;
			const float s5i = f0i - s1i;
			Fout->i = f0i + s1i;
			Fout[ 2 * m ].r = ( f0r + s1r ) - s3r;
			Fout[ 2 * m ].i = Fout->i - s3i;
			Fout->r = Fout->r + s3r;
			const float s5r = f0r - s1r;
			Fout->i = Fout->i + s3i;
			float f1i, f3r, f3i;
			if( inverse )
			{
				Fout[ m ].r = s5r - s4i;
				f1i = s5i + s4r;
				f3r = s5r + s4i;
				f3i = s5i - s4r;
			}
			else
			{
				Fout[ m ].r = s5r + s4i;
				f1i = s5i - s4r;
				f3r = s5r - s4i;
				f3i = s5i + s4r;
			}
			tw1 += fstride;
			tw2 += 2 * fstride;
			tw3 += 3 * fstride;
			Fout[ m ].i = f1i;
			Fout[ 3 * m ].i = f3i;
			Fout[ 3 * m ].r = f3r;
			++Fout;
		}
		while( --k );
	}
	else
	{
		unsigned int k = m >> 1;
		const __m128 vPlusMinus = { 1.f, -1.f, 1.f, -1.f };
		const __m128 vMinusPlus = { -1.f, 1.f, -1.f, 1.f };
		__m128 vSign1, vSign3;
		if( st->inverse )
		{
			vSign1 = vMinusPlus;
			vSign3 = vPlusMinus;
		}
		else
		{
			vSign1 = vPlusMinus;
			vSign3 = vMinusPlus;
		}
		do
		{
			const __m128 s0 = ComplexMul2( _mm_loadu_ps( &Fout[ m ].r ), AKSIMD_LOADUNALIGNED_DUAL( tw1, tw1 + fstride ) );
			const __m128 s2 = ComplexMul2( _mm_loadu_ps( &Fout[ 3 * m ].r ), AKSIMD_LOADUNALIGNED_DUAL( tw3, tw3 + 3 * fstride ) );
			tw1 += 2 * fstride;
			tw3 += 6 * fstride;
			const __m128 s3 = _mm_add_ps( s2, s0 );
			const __m128 s4 = _mm_sub_ps( s0, s2 );
			const __m128 s1 = ComplexMul2( _mm_loadu_ps( &Fout[ 2 * m ].r ), AKSIMD_LOADUNALIGNED_DUAL( tw2, tw2 + 2 * fstride ) );
			tw2 += 4 * fstride;
			const __m128 f0 = _mm_loadu_ps( &Fout->r );
			const __m128 s5 = _mm_sub_ps( f0, s1 );
			const __m128 f0s1 = _mm_add_ps( f0, s1 );
			_mm_storeu_ps( &Fout[ 2 * m ].r, _mm_sub_ps( f0s1, s3 ) );
			_mm_storeu_ps( &Fout->r, _mm_add_ps( f0s1, s3 ) );
			const __m128 s4swap = _mm_shuffle_ps( s4, s4, 177 );
			_mm_storeu_ps( &Fout[ m ].r, _mm_add_ps( _mm_mul_ps( s4swap, vSign1 ), s5 ) );
			_mm_storeu_ps( &Fout[ 3 * m ].r, _mm_add_ps( _mm_mul_ps( s4swap, vSign3 ), s5 ) );
			Fout += 2;
		}
		while( --k );
	}
}

/** 2012 0x9953b0, 2013 0x992010. */
static void kf_bfly3( ak_fft_cpx * Fout, unsigned int m, unsigned int fstride, ak_fft_state * const st )
{
	ak_fft_cpx * tw1 = st->twiddles;
	ak_fft_cpx * tw2 = st->twiddles;
	const float epi3_i = st->twiddles[ m * fstride ].i;
	unsigned int k = m;
	do
	{
		const float s1i = ( Fout[ m ].i * tw1->r ) + ( Fout[ m ].r * tw1->i );
		const float s1r = ( Fout[ m ].r * tw1->r ) - ( tw1->i * Fout[ m ].i );
		const float s2r = ( Fout[ 2 * m ].r * tw2->r ) - ( tw2->i * Fout[ 2 * m ].i );
		const float s2i = ( Fout[ 2 * m ].r * tw2->i ) + ( Fout[ 2 * m ].i * tw2->r );
		const float s3i = s2i + s1i;
		const float s0i = s1i - s2i;
		Fout[ m ].r = Fout->r - ( ( s2r + s1r ) * 0.5f );
		Fout[ m ].i = Fout->i - ( s3i * 0.5f );
		const float s0r = epi3_i * ( s1r - s2r );
		Fout->r = Fout->r + ( s2r + s1r );
		Fout->i = Fout->i + s3i;
		Fout[ 2 * m ].r = Fout[ m ].r + ( epi3_i * s0i );
		Fout[ 2 * m ].i = Fout[ m ].i - s0r;
		const float f1i = s0r + Fout[ m ].i;
		Fout[ m ].r = Fout[ m ].r - ( epi3_i * s0i );
		Fout[ m ].i = f1i;
		tw1 += fstride;
		tw2 += 2 * fstride;
		++Fout;
	}
	while( --k );
}

/** 2012 0x995520, 2013 0x992180. */
static void kf_bfly5( ak_fft_cpx * Fout, const unsigned int fstride, ak_fft_state * const st, int m )
{
	ak_fft_cpx * twiddles = st->twiddles;
	const ak_fft_cpx ya = twiddles[ m * fstride ];
	const ak_fft_cpx yb = twiddles[ 2 * m * fstride ];
	ak_fft_cpx * Fout0 = Fout;
	ak_fft_cpx * Fout1 = Fout0 + m;
	ak_fft_cpx * Fout2 = Fout0 + 2 * m;
	ak_fft_cpx * Fout3 = Fout0 + 3 * m;
	ak_fft_cpx * Fout4 = Fout0 + 4 * m;
	for( int u = 0; u < m; ++u )
	{
		const ak_fft_cpx s0 = *Fout0;
		const ak_fft_cpx & t1 = twiddles[ u * fstride ];
		const ak_fft_cpx & t2 = twiddles[ 2 * u * fstride ];
		const ak_fft_cpx & t3 = twiddles[ 3 * u * fstride ];
		const ak_fft_cpx & t4 = twiddles[ 4 * u * fstride ];
		ak_fft_cpx s1, s2, s3, s4, s7, s8, s9, s10;
		s1.r = ( Fout1->r * t1.r ) - ( Fout1->i * t1.i );
		s1.i = ( Fout1->r * t1.i ) + ( Fout1->i * t1.r );
		s2.r = ( Fout2->r * t2.r ) - ( Fout2->i * t2.i );
		s2.i = ( Fout2->i * t2.r ) + ( Fout2->r * t2.i );
		s3.r = ( Fout3->r * t3.r ) - ( Fout3->i * t3.i );
		s3.i = ( Fout3->r * t3.i ) + ( Fout3->i * t3.r );
		s4.r = ( Fout4->r * t4.r ) - ( Fout4->i * t4.i );
		s4.i = ( Fout4->i * t4.r ) + ( Fout4->r * t4.i );
		s7.r = s4.r + s1.r;
		s10.r = s1.r - s4.r;
		s8.i = s3.i + s2.i;
		s8.r = s3.r + s2.r;
		s7.i = s4.i + s1.i;
		s9.r = s2.r - s3.r;
		s9.i = s2.i - s3.i;
		s10.i = s1.i - s4.i;
		Fout0->r = ( Fout0->r + s8.r ) + s7.r;
		Fout0->i = ( Fout0->i + s8.i ) + s7.i;
		const float s5r = ( ( yb.r * s8.r ) + ( ya.r * s7.r ) ) + s0.r;
		const float s5i = ( ( yb.r * s8.i ) + ( ya.r * s7.i ) ) + s0.i;
		const float s6r = ( yb.i * s9.i ) + ( ya.i * s10.i );
		const float s6i = -( ya.i * s10.r ) - ( yb.i * s9.r );
		Fout1->r = s5r - s6r;
		Fout1->i = s5i - s6i;
		Fout4->r = s6r + s5r;
		Fout4->i = s6i + s5i;
		const float s11r = ( ( ya.r * s8.r ) + ( yb.r * s7.r ) ) + s0.r;
		const float s11i = ( ( ya.r * s8.i ) + ( yb.r * s7.i ) ) + s0.i;
		const float s12r = ( ya.i * s9.i ) - ( yb.i * s10.i );
		const float s12i = ( yb.i * s10.r ) - ( ya.i * s9.r );
		Fout2->r = s12r + s11r;
		Fout2->i = s12i + s11i;
		Fout3->r = s11r - s12r;
		Fout3->i = s11i - s12i;
		++Fout0;
		++Fout1;
		++Fout2;
		++Fout3;
		++Fout4;
	}
}

/** 2012 0x995900, 2013 0x992560. */
static void kf_bfly_generic( ak_fft_cpx * Fout, ak_fft_state * const st, unsigned int fstride, int m, int p )
{
	const ak_fft_cpx * twiddles = st->twiddles;
	const int Norig = st->nfft;
	ak_fft_cpx scratchbuf[ 31 ];
	for( int u = 0; u < m; ++u )
	{
		int k = u;
		for( int q1 = 0; q1 < p; ++q1 )
		{
			scratchbuf[ q1 ] = Fout[ k ];
			k += m;
		}
		k = u;
		for( int q1 = 0; q1 < p; ++q1 )
		{
			int twidx = 0;
			Fout[ k ] = scratchbuf[ 0 ];
			for( int q = 1; q < p; ++q )
			{
				twidx += fstride * k;
				if( twidx >= Norig )
				{
					twidx -= Norig;
				}
				const ak_fft_cpx t = twiddles[ twidx ];
				Fout[ k ].r = ( ( scratchbuf[ q ].r * t.r ) - ( scratchbuf[ q ].i * t.i ) ) + Fout[ k ].r;
				Fout[ k ].i = Fout[ k ].i + ( ( t.r * scratchbuf[ q ].i ) + ( scratchbuf[ q ].r * t.i ) );
			}
			k += m;
		}
	}
}

/** 2012 0x995bc0, 2013 0x992820. */
static void kf_work( ak_fft_cpx * Fout, const ak_fft_cpx * f, unsigned int fstride, int in_stride, int * factors, ak_fft_state * const st )
{
	ak_fft_cpx * Fout_beg = Fout;
	const int p = *factors++;
	const int m = *factors++;
	const ak_fft_cpx * Fout_end = Fout + p * m;
	if( m == 1 )
	{
		do
		{
			*Fout = *f;
			f += fstride * in_stride;
		}
		while( ++Fout != Fout_end );
	}
	else
	{
		do
		{
			kf_work( Fout, f, fstride * p, in_stride, factors, st );
			f += fstride * in_stride;
		}
		while( ( Fout += m ) != Fout_end );
	}
	Fout = Fout_beg;
	switch( p )
	{
	case 2:
		kf_bfly2( Fout, st, fstride, m );
		break;
	case 3:
		kf_bfly3( Fout, m, fstride, st );
		break;
	case 4:
		if( m % 2 == 0 )
		{
			kf_bfly4( Fout, m, fstride, st );
			break;
		}
		kf_bfly_generic( Fout, st, fstride, m, p );
		break;
	case 5:
		kf_bfly5( Fout, fstride, st, m );
		break;
	default:
		kf_bfly_generic( Fout, st, fstride, m, p );
		break;
	}
}

/** 2012 0x995d90, 2013 0x9929f0. */
static void kf_factor( int n, int * facbuf )
{
	int p = 4;
	const double floor_sqrt = floor( sqrt( (double)n ) );
	do
	{
		while( n % p )
		{
			switch( p )
			{
			case 4: p = 2; break;
			case 2: p = 3; break;
			default: p += 2; break;
			}
			if( p > floor_sqrt )
			{
				p = n;
			}
		}
		n /= p;
		*facbuf++ = p;
		*facbuf++ = n;
	}
	while( n > 1 );
}

/** 2012 0x995e10, 2013 0x992a70. */
ak_fft_state * ak_fft_alloc( int nfft, int inverse_fft, void * mem, unsigned int * lenmem )
{
	ak_fft_state * st = 0;
	const unsigned int memneeded = ( sizeof( ak_fft_state ) + sizeof( ak_fft_cpx ) * ( nfft - 1 ) + 15 ) & ~15u;
	if( mem && *lenmem >= memneeded )
	{
		st = (ak_fft_state *)mem;
	}
	*lenmem = memneeded;
	if( st )
	{
		st->nfft = nfft;
		st->inverse = inverse_fft;
		st->twiddles = (ak_fft_cpx *)( (char *)mem + sizeof( ak_fft_state ) );
		for( int i = 0; i < nfft; ++i )
		{
			const double phase = ( inverse_fft ? 6.283185307179586 : -6.283185307179586 ) * (double)i / (double)nfft;
			st->twiddles[ i ].r = (float)cos( phase );
			st->twiddles[ i ].i = (float)sin( phase );
		}
		kf_factor( nfft, st->factors );
	}
	return st;
}

/** 2012 0x995d40, 2013 0x9929a0. */
void ak_fft_stride( ak_fft_state * st, const ak_fft_cpx * fin, ak_fft_cpx * fout, int in_stride )
{
	kf_work( fout, fin, 1, in_stride, st->factors, st );
}

/** 2012 0x995d70, 2013 0x9929d0. */
void ak_fft( ak_fft_state * cfg, const ak_fft_cpx * fin, ak_fft_cpx * fout )
{
	kf_work( fout, fin, 1, 1, cfg->factors, cfg );
}

/** 2012 0x995ee0, 2013 0x992b40. */
int ak_fft_next_fast_size( int n )
{
	while( 1 )
	{
		int m = n;
		while( ( m % 2 ) == 0 )
		{
			m /= 2;
		}
		while( ( m % 3 ) == 0 )
		{
			m /= 3;
		}
		while( ( m % 5 ) == 0 )
		{
			m /= 5;
		}
		if( m <= 1 )
		{
			break;
		}
		n++;
	}
	return n;
}

}
}
