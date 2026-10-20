/*=============================================================================
	ak_fft.h - Audiokinetic's kiss-FFT derivative (wwise\sdk\source\soundengine\akaudiolib\common\ak_fft), Wwise 2012.1.

	Agent IH4 (fork F2): the layouts are the 2012 PDB's (ak_fft_cpx 8 bytes, ak_fft_state 80 bytes aligned 8 with the
	twiddles pointer at +72 and the twiddle table right after the state). The whole unit sits in namespace
	DSP::AkFFTAllButterflies as in the PDB. The allocator never mallocs: the caller hands the memory in (the plug-ins
	carve it out of their own allocations), as retail ak_fft_alloc 0x995e10 does.
=============================================================================*/
#ifndef _AK_FFT_H_
#define _AK_FFT_H_

#include <xmmintrin.h>

/** 8 bytes. */
struct ak_fft_cpx
{
	float r;
	float i;
};

/** 80 bytes: MAXFACTORS 8. */
struct __declspec( align( 8 ) ) ak_fft_state
{
	int nfft;
	int inverse;
	int factors[ 16 ];
	ak_fft_cpx * twiddles;
};

static_assert( sizeof( ak_fft_cpx ) == 8, "ak_fft_cpx: 2012 PDB sizeof 8" );
static_assert( sizeof( ak_fft_state ) == 80, "ak_fft_state: 2012 PDB sizeof 80" );

namespace DSP
{
	namespace AkFFTAllButterflies
	{
		ak_fft_state * ak_fft_alloc( int nfft, int inverse_fft, void * mem, unsigned int * lenmem );
		void ak_fft( ak_fft_state * cfg, const ak_fft_cpx * fin, ak_fft_cpx * fout );
		void ak_fft_stride( ak_fft_state * st, const ak_fft_cpx * fin, ak_fft_cpx * fout, int in_stride );
		int ak_fft_next_fast_size( int n );
	}
}

#endif
