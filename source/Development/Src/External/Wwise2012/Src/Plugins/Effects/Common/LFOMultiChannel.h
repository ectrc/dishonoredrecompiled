/*=============================================================================
	LFOMultiChannel.h - DSP::MultiChannelLFO<POLARITY> (Wwise 2012.1, plugins/effects/common/lfomultichannel.h).
	Agent IH4 (fork F2). <Unipolar> is 148 bytes as the 2012 PDB (6 mono LFOs + the channel count). Retail instantiates
	it only for the tremolo: SetParams 2012 0x99af90 (2013 0x993c90), Setup 0x99b110 (0x993e10).
=============================================================================*/
#ifndef _AK_LFOMULTICHANNEL_H_
#define _AK_LFOMULTICHANNEL_H_

#include "LFOMono.h"
#include <malloc.h>

namespace DSP
{
	template < class POLARITY >
	class MultiChannelLFO
	{
	public:
		MultiChannelLFO() : m_uNumChannels( 0 ) {}

		/** 2012 0x99b110, 2013 0x993e10: one LFO per channel of the mask, each with its initial phase (degrees). */
		void Setup( AkUInt32 in_uChannelMask, AkUInt32 in_uSampleRate, const LFO::MultiChannel::AllParams & in_params )
		{
			AkUInt32 uNumChannels = 0;
			for( AkUInt32 uMask = in_uChannelMask; uMask; uMask &= uMask - 1 )
			{
				++uNumChannels;
			}
			m_uNumChannels = uNumChannels;
			if( uNumChannels )
			{
				AkReal32 * pfPhases = (AkReal32 *)_alloca( sizeof( AkReal32 ) * uNumChannels );
				LFO::MultiChannel::ComputeInitialPhase( in_uChannelMask, in_params.phaseParams, pfPhases );
				for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
				{
					m_arLfo[ i ].Setup( in_uSampleRate, in_params.lfoParams, pfPhases[ i ] );
				}
			}
		}

		/** 2012 0x99af90, 2013 0x993c90. */
		void SetParams( AkUInt32 in_uSampleRate, const LFO::Params & in_lfoParams )
		{
			AkReal32 fB0, fA1;
			MonoLFO<POLARITY>::ComputeFilterCoefsFromParams( in_uSampleRate, in_lfoParams, fB0, fA1 );
			for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
			{
				m_arLfo[ i ].SetParams( in_uSampleRate, in_lfoParams, fB0, fA1 );
			}
		}

		AkUInt32 GetNumChannels() const { return m_uNumChannels; }
		MonoLFO<POLARITY> & GetChannel( AkUInt32 in_uIndex ) { return m_arLfo[ in_uIndex ]; }

	protected:
		MonoLFO<POLARITY>	m_arLfo[ 6 ];
		AkUInt32			m_uNumChannels;
	};
}

static_assert( sizeof( DSP::MultiChannelLFO<DSP::Unipolar> ) == 148, "DSP::MultiChannelLFO<DSP::Unipolar>: 2012 PDB sizeof 148" );

#endif
