/*=============================================================================
	FreqWindow.cpp - DSP::AkFFTAllButterflies::CAkFreqWindow, Wwise 2012.1 (plugins/common/freqwindow.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. As retail, the bins between DC and
	Nyquist store the negated atan2 phase (and PolToCart negates it back); DC and Nyquist keep the plain phase. The phase
	vocoder's synthesis phase update keeps retail's float arithmetic ((dPhi - expected) + expected) and its float pi
	constants. The inverse FFT is normalised by 1 / FFT size with AK::DSP::ApplyGain (retail's ApplyGain_4 copy 0x993640).
=============================================================================*/
#include "FreqWindow.h"
#include "AkApplyGain.h"
#include <math.h>

namespace DSP
{
namespace AkFFTAllButterflies
{

static const AkReal32 PI = 3.1415927f;
static const AkReal32 TWOPI = 6.2831855f;

/** 2012 0x993740, 2013 0x990a30. */
AKRESULT CAkFreqWindow::Alloc( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uFFTSize )
{
	m_uFFTSize = in_uFFTSize;
	m_pfFreqData = (ak_fft_cpx *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( ak_fft_cpx ) * ( in_uFFTSize / 2 + 1 ) );
	return m_pfFreqData ? AK_Success : AK_InsufficientMemory;
}

/** 2012 0x993770, 2013 0x990a60. */
void CAkFreqWindow::Free( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pfFreqData )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pfFreqData );
		m_pfFreqData = NULL;
	}
	m_bReady = false;
}

/** 2012 0x9937a0, 2013 0x990a90. */
void CAkFreqWindow::CartToPol( ak_fft_cpx * io_pfFreqData )
{
	const AkUInt32 uNyquist = m_uFFTSize / 2;
	for( AkUInt32 i = 1; i < uNyquist; ++i )
	{
		const AkReal32 fReal = io_pfFreqData[ i ].r;
		const AkReal32 fImag = io_pfFreqData[ i ].i;
		io_pfFreqData[ i ].r = sqrtf( fImag * fImag + fReal * fReal );
		io_pfFreqData[ i ].i = -atan2f( fImag, fReal );
	}
	m_bPolar = true;
	const AkReal32 fDCReal = io_pfFreqData[ 0 ].r;
	const AkReal32 fDCImag = io_pfFreqData[ 0 ].i;
	io_pfFreqData[ 0 ].r = sqrtf( fDCImag * fDCImag + fDCReal * fDCReal );
	io_pfFreqData[ 0 ].i = atan2f( fDCImag, fDCReal );
	const AkReal32 fNyqReal = io_pfFreqData[ uNyquist ].r;
	const AkReal32 fNyqImag = io_pfFreqData[ uNyquist ].i;
	io_pfFreqData[ uNyquist ].r = sqrtf( fNyqImag * fNyqImag + fNyqReal * fNyqReal );
	io_pfFreqData[ uNyquist ].i = atan2f( fNyqImag, fNyqReal );
}

/** 2012 0x993860, 2013 0x990b50. */
void CAkFreqWindow::PolToCart( AkPolar * io_pfFreqData )
{
	const AkUInt32 uNyquist = m_uFFTSize / 2;
	for( AkUInt32 i = 1; i < uNyquist; ++i )
	{
		const AkReal32 fMag = io_pfFreqData[ i ].fMag;
		const AkReal32 fPhase = io_pfFreqData[ i ].fPhase * -1.f;
		io_pfFreqData[ i ].fMag = cosf( fPhase ) * fMag;
		io_pfFreqData[ i ].fPhase = fMag * sinf( fPhase );
	}
	m_bPolar = false;
	const AkReal32 fDCMag = io_pfFreqData[ 0 ].fMag;
	const AkReal32 fDCPhase = io_pfFreqData[ 0 ].fPhase;
	io_pfFreqData[ 0 ].fMag = cosf( fDCPhase ) * fDCMag;
	io_pfFreqData[ 0 ].fPhase = fDCMag * sinf( fDCPhase );
	const AkReal32 fNyqMag = io_pfFreqData[ uNyquist ].fMag;
	const AkReal32 fNyqPhase = io_pfFreqData[ uNyquist ].fPhase;
	io_pfFreqData[ uNyquist ].fMag = cosf( fNyqPhase ) * fNyqMag;
	io_pfFreqData[ uNyquist ].fPhase = fNyqMag * sinf( fNyqPhase );
}

/** 2012 0x993920, 2013 0x990c10. */
void CAkFreqWindow::Compute( AkReal32 * in_pfTimeDomainWindow, AkUInt32 /*in_uNumFrames*/, ak_fftr_state * in_pFFTState, ak_fft_cpx * io_pfFreqData )
{
	ak_fftr( in_pFFTState, in_pfTimeDomainWindow, io_pfFreqData );
	m_bReady = true;
	m_bPolar = false;
}

/** 2012 0x993950, 2013 0x990c40. */
void CAkFreqWindow::ConvertToTimeDomain( AkReal32 * out_pfTimeDomainBuffer, AkUInt32 /*in_uNumFrames*/, ak_fftr_state * in_pIFFTState, ak_fft_cpx * io_pfFreqData )
{
	if( m_bPolar )
	{
		PolToCart( (AkPolar *)io_pfFreqData );
	}
	ak_fftri( in_pIFFTState, io_pfFreqData, out_pfTimeDomainBuffer );
	AK::DSP::ApplyGain( out_pfTimeDomainBuffer, m_uFFTSize, (AkReal32)( 1.0 / (double)m_uFFTSize ) );
}

/**
 * 2012 0x9939b0, 2013 0x990ca0: the magnitudes interpolate between the two analysis frames at in_fInterpLoc; each bin's
 * output phase is the running synthesis phase, which then advances by the measured phase difference (the expected
 * advance for the hop is subtracted and added back, retail's float arithmetic) and wraps into [-pi, pi).
 */
void CAkFreqWindow::ComputeVocoderSpectrum( AkPolar * in_pPreviousFrame, AkPolar * in_pNextFrame, AkReal32 * io_pPreviousSynthPhases, AkUInt32 in_uHopSize, AkReal32 in_fInterpLoc, bool in_bInitPhases, AkPolar * io_pfFreqData )
{
	const AkUInt32 uNumBins = m_uFFTSize / 2 + 1;
	for( AkUInt32 i = 0; i < uNumBins; ++i )
	{
		io_pfFreqData[ i ].fMag = ( in_pPreviousFrame[ i ].fMag * ( 1.f - in_fInterpLoc ) ) + ( in_pNextFrame[ i ].fMag * in_fInterpLoc );
	}
	if( in_bInitPhases )
	{
		for( AkUInt32 i = 0; i < uNumBins; ++i )
		{
			io_pPreviousSynthPhases[ i ] = in_pPreviousFrame[ i ].fPhase;
		}
	}
	const AkReal32 fExpectedPhaseAdvanceInc = (AkReal32)( (double)in_uHopSize * 6.283185307179586 / (double)m_uFFTSize );
	AkReal32 fExpectedPhaseAdvance = 0.f;
	for( AkUInt32 i = 0; i < uNumBins; ++i )
	{
		const AkReal32 fPrevSynthPhase = io_pPreviousSynthPhases[ i ];
		io_pfFreqData[ i ].fPhase = fPrevSynthPhase;
		AkReal32 fSynthPhase = ( ( ( in_pNextFrame[ i ].fPhase - in_pPreviousFrame[ i ].fPhase ) - fExpectedPhaseAdvance ) + fExpectedPhaseAdvance ) + fPrevSynthPhase;
		io_pPreviousSynthPhases[ i ] = fSynthPhase;
		if( fSynthPhase >= PI )
		{
			io_pPreviousSynthPhases[ i ] = fSynthPhase - TWOPI;
		}
		if( io_pPreviousSynthPhases[ i ] < -PI )
		{
			io_pPreviousSynthPhases[ i ] = io_pPreviousSynthPhases[ i ] + TWOPI;
		}
		fExpectedPhaseAdvance = fExpectedPhaseAdvance + fExpectedPhaseAdvanceInc;
		if( fExpectedPhaseAdvance >= TWOPI )
		{
			fExpectedPhaseAdvance = fExpectedPhaseAdvance - TWOPI;
		}
	}
	m_bPolar = true;
	m_bReady = true;
}

/** 2012 0x993e20, 2013 0x991110. */
void CAkFreqWindow::CartToPol()
{
	CartToPol( m_pfFreqData );
}

/** 2012 0x993e30, 2013 0x991120. */
void CAkFreqWindow::PolToCart()
{
	PolToCart( (AkPolar *)m_pfFreqData );
}

/** 2012 0x993e40, 2013 0x991130. */
void CAkFreqWindow::Compute( AkReal32 * in_pfTimeDomainWindow, AkUInt32 in_uNumFrames, ak_fftr_state * in_pFFTState )
{
	Compute( in_pfTimeDomainWindow, in_uNumFrames, in_pFFTState, m_pfFreqData );
}

/** 2012 0x993e70, 2013 0x991160. */
void CAkFreqWindow::ConvertToTimeDomain( AkReal32 * out_pfTimeDomainBuffer, AkUInt32 in_uNumFrames, ak_fftr_state * in_pIFFTState )
{
	ConvertToTimeDomain( out_pfTimeDomainBuffer, in_uNumFrames, in_pIFFTState, m_pfFreqData );
}

/** 2012 0x993e90, 2013 0x991180. */
void CAkFreqWindow::ComputeVocoderSpectrum( AkPolar * in_pPreviousFrame, AkPolar * in_pNextFrame, AkReal32 * io_pPreviousSynthPhases, AkUInt32 in_uHopSize, AkReal32 in_fInterpLoc, bool in_bInitPhases )
{
	ComputeVocoderSpectrum( in_pPreviousFrame, in_pNextFrame, io_pPreviousSynthPhases, in_uHopSize, in_fInterpLoc, in_bInitPhases, (AkPolar *)m_pfFreqData );
}

}
}
