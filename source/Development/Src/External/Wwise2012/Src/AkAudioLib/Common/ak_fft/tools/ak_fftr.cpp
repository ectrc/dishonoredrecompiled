/*=============================================================================
	ak_fftr.cpp - the real FFT over ak_fft, Wwise 2012.1 (akaudiolib\common\ak_fft\tools\ak_fftr.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (rvas on each function). As retail, the state, the half-size
	complex state, the work buffer and the super twiddles live in one caller block (no malloc), and the split step
	processes two bins per SSE register: (k, k+1) against (N-k, N-k-1).
	Deviation (named): retail multiplies the forward split by the 0.5 constant it shares with ArkNormalize4_PS
	(2012 rva 0xda9730); it is a local constant here.
=============================================================================*/
#include "ak_fftr.h"
#include <math.h>

namespace DSP
{
namespace AkFFTAllButterflies
{

static const __m128 vSign = { 1.f, -1.f, 1.f, -1.f };
static const __m128 sOneHalf = { 0.5f, 0.5f, 0.5f, 0.5f };

/** 2012 0x994580, 2013 0x991870. */
ak_fftr_state * ak_fftr_alloc( int nfft, int inverse_fft, void * mem, unsigned int * lenmem )
{
	if( nfft & 1 )
	{
		return 0;
	}
	nfft >>= 1;
	unsigned int subsize = 0;
	ak_fft_alloc( nfft, inverse_fft, 0, &subsize );
	const unsigned int memneeded = sizeof( ak_fftr_state ) + subsize + sizeof( ak_fft_cpx ) * ( nfft * 3 / 2 );
	ak_fftr_state * st = 0;
	if( *lenmem >= memneeded )
	{
		st = (ak_fftr_state *)mem;
	}
	*lenmem = memneeded;
	if( !st )
	{
		return 0;
	}
	st->substate = (ak_fft_state *)( st + 1 );
	st->tmpbuf = (ak_fft_cpx *)( (char *)st->substate + subsize );
	st->super_twiddles = st->tmpbuf + nfft;
	ak_fft_alloc( nfft, inverse_fft, st->substate, &subsize );
	for( int i = 0; i < nfft / 2; ++i )
	{
		double phase = -3.141592653589793 * ( (double)( i + 1 ) / nfft + 0.5 );
		if( inverse_fft )
		{
			phase *= -1;
		}
		st->super_twiddles[ i ].r = (float)cos( phase );
		st->super_twiddles[ i ].i = (float)sin( phase );
	}
	return st;
}

/** 2012 0x9946a0, 2013 0x991990. */
void ak_fftr( ak_fftr_state * st, const float * timedata, ak_fft_cpx * freqdata )
{
	const int ncfft = st->substate->nfft;
	ak_fft( st->substate, (const ak_fft_cpx *)timedata, st->tmpbuf );
	ak_fft_cpx * tmpbuf = st->tmpbuf;
	if( ncfft / 2 )
	{
		const ak_fft_cpx * pTwiddles = st->super_twiddles;
		ak_fft_cpx * pLow = freqdata + 1;
		ak_fft_cpx * pHigh = freqdata + ncfft - 2;
		const ak_fft_cpx * pTmpLow = tmpbuf + 1;
		const ak_fft_cpx * pTmpHigh = tmpbuf + ncfft - 2;
		unsigned int k = (unsigned int)( ncfft / 2 + 1 ) >> 1;
		do
		{
			const __m128 fpk = _mm_loadu_ps( &pTmpLow->r );
			const __m128 vHigh = _mm_loadu_ps( &pTmpHigh->r );
			const __m128 fpnk = _mm_mul_ps( _mm_shuffle_ps( vHigh, vHigh, 78 ), vSign );
			const __m128 f1k = _mm_add_ps( fpnk, fpk );
			const __m128 f2k = _mm_sub_ps( fpk, fpnk );
			const __m128 vTw = _mm_loadu_ps( &pTwiddles->r );
			const __m128 vIm = _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( f2k, f2k, 245 ), vSign ), vTw );
			const __m128 tw = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps( f2k, f2k, 160 ), vTw ), _mm_shuffle_ps( vIm, vIm, 177 ) );
			_mm_storeu_ps( &pLow->r, _mm_mul_ps( _mm_add_ps( tw, f1k ), sOneHalf ) );
			const __m128 vOut = _mm_mul_ps( _mm_mul_ps( _mm_sub_ps( f1k, tw ), vSign ), sOneHalf );
			_mm_storeu_ps( &pHigh->r, _mm_shuffle_ps( vOut, vOut, 78 ) );
			pTwiddles += 2;
			pLow += 2;
			pHigh -= 2;
			pTmpLow += 2;
			pTmpHigh -= 2;
		}
		while( --k );
	}
	const float tdc_i = tmpbuf->i;
	const float tdc_r = tmpbuf->r;
	freqdata[ 0 ].r = tdc_i + tmpbuf->r;
	freqdata[ ncfft ].r = tdc_r - tdc_i;
	freqdata[ 0 ].i = 0.f;
	freqdata[ ncfft ].i = 0.f;
}

/** 2012 0x9947c0, 2013 0x991ab0. */
void ak_fftri( ak_fftr_state * st, const ak_fft_cpx * freqdata, float * timedata )
{
	ak_fft_state * substate = st->substate;
	const ak_fft_cpx * pTwiddles = st->super_twiddles;
	ak_fft_cpx * tmpbuf = st->tmpbuf;
	const int ncfft = substate->nfft;
	const float dc = freqdata[ 0 ].r;
	const float nyq = freqdata[ ncfft ].r;
	tmpbuf[ 0 ].r = dc + nyq;
	tmpbuf[ 0 ].i = dc - nyq;
	if( ncfft / 2 )
	{
		ak_fft_cpx * pHigh = tmpbuf + ncfft - 2;
		ak_fft_cpx * pLow = tmpbuf + 1;
		const ak_fft_cpx * pInLow = freqdata + 1;
		const ak_fft_cpx * pInHigh = freqdata + ncfft - 2;
		unsigned int k = (unsigned int)( ncfft / 2 + 1 ) >> 1;
		do
		{
			const __m128 fk = _mm_loadu_ps( &pInLow->r );
			const __m128 vHigh = _mm_loadu_ps( &pInHigh->r );
			const __m128 fnkc = _mm_mul_ps( _mm_shuffle_ps( vHigh, vHigh, 78 ), vSign );
			const __m128 fek = _mm_add_ps( fnkc, fk );
			const __m128 tmp = _mm_sub_ps( fk, fnkc );
			const __m128 vTw = _mm_loadu_ps( &pTwiddles->r );
			const __m128 vIm = _mm_mul_ps( _mm_mul_ps( _mm_shuffle_ps( tmp, tmp, 245 ), vSign ), vTw );
			const __m128 fok = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps( tmp, tmp, 160 ), vTw ), _mm_shuffle_ps( vIm, vIm, 177 ) );
			_mm_storeu_ps( &pLow->r, _mm_add_ps( fok, fek ) );
			const __m128 vOut = _mm_mul_ps( _mm_sub_ps( fek, fok ), vSign );
			_mm_storeu_ps( &pHigh->r, _mm_shuffle_ps( vOut, vOut, 78 ) );
			pTwiddles += 2;
			pLow += 2;
			pHigh -= 2;
			pInLow += 2;
			pInHigh -= 2;
		}
		while( --k );
	}
	ak_fft( substate, tmpbuf, (ak_fft_cpx *)timedata );
}

}
}
