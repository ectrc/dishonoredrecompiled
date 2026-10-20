/*=============================================================================
	AkPhaseVocoder.h - DSP::AkFFTAllButterflies::CAkPhaseVocoder (Wwise 2012.1, plugins/common/akphasevocoder.h).
	Agent IH4 (fork F2). 564 bytes as the 2012 PDB: per channel an input accumulation ring, an OLA output ring, two
	analysis spectra (alternating, m_uFreqWindowIndex) and the synthesis (vocoder) spectrum; a 1/4-window hop.
=============================================================================*/
#ifndef _AK_PHASEVOCODER_H_
#define _AK_PHASEVOCODER_H_

#include "CircularBuffer.h"
#include "OLACircularBuffer.h"
#include "TimeWindow.h"
#include "FreqWindow.h"

namespace DSP
{
	namespace AkFFTAllButterflies
	{
		class CAkPhaseVocoder
		{
		public:
			CAkPhaseVocoder();
			~CAkPhaseVocoder();

			AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate, AkUInt32 in_uFFTSize, bool in_bUseInputBuffer );
			void Term( AK::IAkPluginMemAlloc * in_pAllocator );
			void Reset();
			void Execute( AkAudioBuffer * io_pInBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * io_pOutBuffer, AkReal32 in_fTSFactor, bool in_bEnterNoTSMode, AkReal32 * in_pfTempStorage );
			AkUInt32 GetFFTSize() const { return m_uFFTSize; }

		protected:
			CAkCircularBuffer		m_InputAccumBuf[ 6 ];
			CAkOLACircularBuffer	m_OLAOutCircBuf[ 6 ];
			CAkTimeWindow			m_TimeWindow;
			CAkFreqWindow			m_FreqWindow[ 2 ][ 6 ];
			CAkFreqWindow			m_VocoderWindow[ 6 ];
			AkReal32 *				m_pfPrevSynthesisPhase[ 6 ];
			AkUInt8					m_uFreqWindowIndex[ 6 ];
			ak_fftr_state *			m_pFFTState;
			ak_fftr_state *			m_pIFFTState;
			AkUInt32				m_uFFTSpaceRequirements;
			AkUInt32				m_uIFFTSpaceRequirements;
			AkUInt32				m_uNumChannels;
			AkUInt32				m_uSampleRate;
			AkUInt32				m_uFFTSize;
			AkReal32				m_fInterpPos;
			AkUInt32				m_uInputFramesToDiscard;
			bool					m_bInitPhases;
			bool					m_bInputStartFill;
			bool					m_bUseInputBuffer;
		};
	}
}

static_assert( sizeof( DSP::AkFFTAllButterflies::CAkPhaseVocoder ) == 564, "CAkPhaseVocoder: 2012 PDB sizeof 564" );

#endif
