/*=============================================================================
	AkStereoDelayLine.h - AK::DSP::CStereoDelayLine (Wwise 2012.1, plugins/effects/akstereodelay/akstereodelayline.h).
	Agent IH4 (fork F2). 108 bytes as the 2012 PDB: two single-channel delay lines (left, right) with a biquad on each
	feedback path.
=============================================================================*/
#ifndef _AK_STEREODELAYLINE_H_
#define _AK_STEREODELAYLINE_H_

#include "AkStereoDelayFXParams.h"
#include <AK/DSP/AkDelayLineMemory.h>
#include "BiquadFilter.h"

namespace AK
{
	namespace DSP
	{
		class CStereoDelayLine
		{
		public:
			CStereoDelayLine() : m_uSampleRate( 0 ) {}

			AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 * const in_fDelayTimes, AkUInt32 in_uSampleRate );
			void Term( AK::IAkPluginMemAlloc * in_pAllocator );
			void Reset();
			void ProcessBuffer( AkAudioBuffer * in_pBuffer, AkAudioBuffer * out_pBuffer, AkStereoDelayChannelParams * const in_PrevStereoDelayParams,
				AkStereoDelayChannelParams * const in_CurStereoDelayParams, AkStereoDelayFilterParams & in_FilterParams, bool in_bRecomputeFilterParams );

		protected:
			CAkDelayLineMemory<AkReal32, 1>	m_DelayLines[ 2 ];
			::DSP::BiquadFilter				m_FeedbackFilters[ 2 ];
			AkUInt32						m_uSampleRate;
		};
	}
}

static_assert( sizeof( AK::DSP::CStereoDelayLine ) == 108, "CStereoDelayLine: 2012 PDB sizeof 108" );

#endif
