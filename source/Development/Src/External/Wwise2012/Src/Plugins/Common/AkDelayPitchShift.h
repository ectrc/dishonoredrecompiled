/*=============================================================================
	AkDelayPitchShift.h - AK::DSP::AkDelayPitchShift (Wwise 2012.1, plugins/common/akdelaypitchshift.h). Agent IH4 (fork F2).
	132 bytes as the 2012 PDB: the classic two-tap variable-delay pitch shifter (taps half a delay line apart,
	triangular crossfade), one delay line per channel.
=============================================================================*/
#ifndef _AK_DELAYPITCHSHIFT_H_
#define _AK_DELAYPITCHSHIFT_H_

#include <AK/DSP/AkDelayLineMemory.h>

namespace AK
{
	namespace DSP
	{
		class AkDelayPitchShift
		{
		public:
			AkDelayPitchShift();

			AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 in_MaxDelayTime, AkUInt32 in_uNumChannels, AkUInt32 in_uSampleRate );
			void Term( AK::IAkPluginMemAlloc * in_pAllocator );
			void Reset();
			void SetPitchFactor( AkReal32 in_fPitchFactor );
			void ProcessChannel( AkReal32 * in_pfInBuf, AkReal32 * out_pfOutBuf, AkUInt32 in_uNumFrames, AkUInt32 in_uChanIndex );

		protected:
			CAkDelayLineMemory<AkReal32, 1>	m_DelayLines[ 6 ];
			AkReal32						m_fFractDelay[ 6 ];
			AkReal32						m_fReadWriteRateDelta;
			AkUInt32						m_uNumChannels;
			AkUInt32						m_uDelayLength;
		};
	}
}

static_assert( sizeof( AK::DSP::AkDelayPitchShift ) == 132, "AkDelayPitchShift: 2012 PDB sizeof 132" );

#endif
