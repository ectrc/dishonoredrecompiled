/*=============================================================================
	ak_fftr.h - the real-input FFT over ak_fft (ak_fft\tools\ak_fftr.cpp), Wwise 2012.1. Agent IH4 (fork F2).
	ak_fftr_state is 16 bytes (aligned 8): the complex half-size state, the work buffer and the super twiddles, all
	inside the caller's memory block.
=============================================================================*/
#ifndef _AK_FFTR_H_
#define _AK_FFTR_H_

#include "../ak_fft.h"

struct __declspec( align( 8 ) ) ak_fftr_state
{
	ak_fft_state * substate;
	ak_fft_cpx * tmpbuf;
	ak_fft_cpx * super_twiddles;
};
static_assert( sizeof( ak_fftr_state ) == 16, "ak_fftr_state: 2012 PDB sizeof 16" );

namespace DSP
{
	namespace AkFFTAllButterflies
	{
		ak_fftr_state * ak_fftr_alloc( int nfft, int inverse_fft, void * mem, unsigned int * lenmem );
		void ak_fftr( ak_fftr_state * st, const float * timedata, ak_fft_cpx * freqdata );
		void ak_fftri( ak_fftr_state * st, const ak_fft_cpx * freqdata, float * timedata );
	}
}

#endif
