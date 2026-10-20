/*=============================================================================
	AkResamplingPhaseVocoder.h - DSP::AkFFTAllButterflies::CAkResamplingPhaseVocoder (Wwise 2012.1,
	plugins/common/akresamplingphasevocoder.h). Agent IH4 (fork F2). 732 bytes as the 2012 PDB: the phase vocoder with a
	resampling input ring per channel (pitch shift = resample + time stretch by the inverse factor).
=============================================================================*/
#ifndef _AK_RESAMPLINGPHASEVOCODER_H_
#define _AK_RESAMPLINGPHASEVOCODER_H_

#include "AkPhaseVocoder.h"
#include "ResamplingCircularBuffer.h"

namespace DSP
{
	namespace AkFFTAllButterflies
	{
		class CAkResamplingPhaseVocoder : public CAkPhaseVocoder
		{
		public:
			AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate, AkUInt32 in_uFFTSize, bool in_bUseInputBuffer );
			void Term( AK::IAkPluginMemAlloc * in_pAllocator );
			void Reset();
			void ResetInputFill();
			AKRESULT ProcessPitchChannel( AkReal32 * in_pfInBuf, AkUInt32 in_uNumFrames, bool in_bNoMoreData, AkUInt32 i, AkReal32 * io_pfOutBuf, AkReal32 in_fResamplingFactor, AkReal32 * in_pfTempStorage );

		protected:
			CAkResamplingCircularBuffer m_ResamplingInputAccumBuf[ 6 ];
		};
	}
}

static_assert( sizeof( DSP::AkFFTAllButterflies::CAkResamplingPhaseVocoder ) == 732, "CAkResamplingPhaseVocoder: 2012 PDB sizeof 732" );

#endif
