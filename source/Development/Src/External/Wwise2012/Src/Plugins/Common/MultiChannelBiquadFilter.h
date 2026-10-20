/*=============================================================================
	MultiChannelBiquadFilter.h - AK::DSP::MultiChannelBiquadFilter<N> (Wwise 2012.1, plugins/common/multichannelbiquadfilter.h).
	Agent IH4 (fork F2). One coefficient set shared by N channels' memories; <6> is 116 bytes as the 2012 PDB. The
	filter types are the order ComputeCoefs 0x991960 switches on (RBJ shelves / peaking / band-pass / notch, Butterworth
	low- and high-pass); A1 and A2 are stored negated so the recursion only adds.
=============================================================================*/
#ifndef _AK_MULTICHANNELBIQUADFILTER_H_
#define _AK_MULTICHANNELBIQUADFILTER_H_

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK
{
	namespace DSP
	{
		template < AkUInt32 MAXCHANNELS >
		class MultiChannelBiquadFilter
		{
		public:
			enum FilterType
			{
				FilterType_LowShelf = 0,
				FilterType_Peaking,
				FilterType_HighShelf,
				FilterType_LowPass,
				FilterType_HighPass,
				FilterType_BandPass,
				FilterType_Notch
			};

			struct Coefficients
			{
				AkReal32 fB0;
				AkReal32 fB1;
				AkReal32 fB2;
				AkReal32 fA1;
				AkReal32 fA2;
				Coefficients() : fB0( 0.f ), fB1( 0.f ), fB2( 0.f ), fA1( 0.f ), fA2( 0.f ) {}
			};

			struct Memories
			{
				AkReal32 fFFwd1;
				AkReal32 fFFwd2;
				AkReal32 fFFbk1;
				AkReal32 fFFbk2;
				Memories();
			};

			void ComputeCoefs( FilterType in_eFilterType, AkUInt32 in_uSampleRate, AkReal32 in_fFreq, AkReal32 in_fGain, AkReal32 in_fQ );
			void Reset();
			void ProcessChannel( AkReal32 * io_pfBuffer, AkUInt32 in_uNumframes, AkUInt32 in_uChannelIndex );

		protected:
			Coefficients	m_Coefficients;
			Memories		m_Memories[ MAXCHANNELS ];
		};
	}
}

static_assert( sizeof( AK::DSP::MultiChannelBiquadFilter<6> ) == 116, "MultiChannelBiquadFilter<6>: 2012 PDB sizeof 116" );

#endif
