/*=============================================================================
	FreqWindow.h - DSP::AkFFTAllButterflies::CAkFreqWindow and DSP::AkPolar (Wwise 2012.1, plugins/common/freqwindow.h).
	Agent IH4 (fork F2). CAkFreqWindow is 12 bytes (2012 PDB): the half spectrum (FFT size / 2 + 1 bins), in cartesian
	or polar form (m_bPolar).
=============================================================================*/
#ifndef _AK_FREQWINDOW_H_
#define _AK_FREQWINDOW_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include "ak_fft/tools/ak_fftr.h"

namespace DSP
{
	/** 8 bytes. */
	struct AkPolar
	{
		AkReal32 fMag;
		AkReal32 fPhase;
	};

	namespace AkFFTAllButterflies
	{
		class CAkFreqWindow
		{
		public:
			CAkFreqWindow() : m_pfFreqData( NULL ), m_uFFTSize( 0 ), m_bReady( false ), m_bPolar( false ) {}

			AKRESULT Alloc( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uFFTSize );
			void Free( AK::IAkPluginMemAlloc * in_pAllocator );
			void CartToPol();
			void PolToCart();
			void Compute( AkReal32 * in_pfTimeDomainWindow, AkUInt32 in_uNumFrames, ak_fftr_state * in_pFFTState );
			void ConvertToTimeDomain( AkReal32 * out_pfTimeDomainBuffer, AkUInt32 in_uNumFrames, ak_fftr_state * in_pIFFTState );
			void ComputeVocoderSpectrum( AkPolar * in_pPreviousFrame, AkPolar * in_pNextFrame, AkReal32 * io_pPreviousSynthPhases, AkUInt32 in_uHopSize, AkReal32 in_fInterpLoc, bool in_bInitPhases );
			ak_fft_cpx * Get() { return m_pfFreqData; }
			bool IsReady() const { return m_bReady; }
			void SetReady( bool in_bReady ) { m_bReady = in_bReady; }

		protected:
			void CartToPol( ak_fft_cpx * io_pfFreqData );
			void PolToCart( AkPolar * io_pfFreqData );
			void Compute( AkReal32 * in_pfTimeDomainWindow, AkUInt32 in_uNumFrames, ak_fftr_state * in_pFFTState, ak_fft_cpx * io_pfFreqData );
			void ConvertToTimeDomain( AkReal32 * out_pfTimeDomainBuffer, AkUInt32 in_uNumFrames, ak_fftr_state * in_pIFFTState, ak_fft_cpx * io_pfFreqData );
			void ComputeVocoderSpectrum( AkPolar * in_pPreviousFrame, AkPolar * in_pNextFrame, AkReal32 * io_pPreviousSynthPhases, AkUInt32 in_uHopSize, AkReal32 in_fInterpLoc, bool in_bInitPhases, AkPolar * io_pfFreqData );

			ak_fft_cpx *	m_pfFreqData;
			AkUInt32		m_uFFTSize;
			bool			m_bReady;
			bool			m_bPolar;
		};
	}
}

static_assert( sizeof( DSP::AkPolar ) == 8, "DSP::AkPolar: 2012 PDB sizeof 8" );
static_assert( sizeof( DSP::AkFFTAllButterflies::CAkFreqWindow ) == 12, "CAkFreqWindow: 2012 PDB sizeof 12" );

#endif
