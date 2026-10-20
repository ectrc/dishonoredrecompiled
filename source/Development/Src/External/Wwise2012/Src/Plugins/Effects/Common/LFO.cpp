/*=============================================================================
	LFO.cpp - DSP::LFO::MultiChannel::ComputeInitialPhase, Wwise 2012.1 (plugins/effects/common/lfo.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (2012 0x99b770, 2013 0x994470). Channels are in the lower
	engine's deinterleaved order (L, R, C, rear L, rear R, LFE last). The random mode draws with the CRT rand() as
	retail; the LFE never gets a random offset.
=============================================================================*/
#include "LFO.h"
#include <stdlib.h>
#include <string.h>

namespace DSP
{
namespace LFO
{
namespace MultiChannel
{

/** 2012 0x99b770, 2013 0x994470. */
void ComputeInitialPhase( AkUInt32 in_uChannelMask, const PhaseParams & in_phaseParams, AkReal32 * const out_uChannelPhase )
{
	AkUInt32 uNumChannels = 0;
	for( AkUInt32 uMask = in_uChannelMask; uMask; uMask &= uMask - 1 )
	{
		++uNumChannels;
	}
	memset( out_uChannelPhase, 0, sizeof( AkReal32 ) * uNumChannels );
	switch( in_phaseParams.ePhaseMode )
	{
	case PHASE_MODE_LEFT_RIGHT:
		{
			AkUInt32 uChannel = 0;
			if( in_uChannelMask & AK_SPEAKER_FRONT_RIGHT )
			{
				uChannel = 2;
				out_uChannelPhase[ 1 ] = in_phaseParams.fPhaseSpread;
				if( in_uChannelMask & AK_SPEAKER_FRONT_CENTER )
				{
					out_uChannelPhase[ 2 ] = in_phaseParams.fPhaseSpread * 0.5f;
					uChannel = 3;
				}
			}
			if( in_uChannelMask & AK_SPEAKER_BACK_LEFT )
			{
				out_uChannelPhase[ uChannel ] = 0.f;
				out_uChannelPhase[ uChannel + 1 ] = in_phaseParams.fPhaseSpread;
			}
		}
		break;
	case PHASE_MODE_FRONT_REAR:
		if( in_uChannelMask & AK_SPEAKER_BACK_LEFT )
		{
			const AkUInt32 uChannel = ( ( in_uChannelMask & AK_SPEAKER_FRONT_CENTER ) | 8 ) >> 2;
			out_uChannelPhase[ uChannel ] = in_phaseParams.fPhaseSpread;
			out_uChannelPhase[ uChannel + 1 ] = in_phaseParams.fPhaseSpread;
		}
		break;
	case PHASE_MODE_CIRCULAR:
		if( in_uChannelMask & ( AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT ) )
		{
			out_uChannelPhase[ 1 ] = in_phaseParams.fPhaseSpread * 0.5f;
			AkUInt32 uChannel = 2;
			if( in_uChannelMask & AK_SPEAKER_FRONT_CENTER )
			{
				out_uChannelPhase[ 2 ] = in_phaseParams.fPhaseSpread * 0.25f;
				uChannel = 3;
			}
			out_uChannelPhase[ uChannel ] = in_phaseParams.fPhaseSpread * 0.5f;
			out_uChannelPhase[ uChannel + 1 ] = in_phaseParams.fPhaseSpread;
		}
		else if( in_uChannelMask & ( AK_SPEAKER_FRONT_LEFT | AK_SPEAKER_FRONT_RIGHT ) )
		{
			out_uChannelPhase[ 1 ] = in_phaseParams.fPhaseSpread;
			if( in_uChannelMask & AK_SPEAKER_FRONT_CENTER )
			{
				out_uChannelPhase[ 2 ] = in_phaseParams.fPhaseSpread * 0.5f;
			}
		}
		break;
	case PHASE_MODE_RANDOM:
		{
			AkUInt32 uNumNoLFE = 0;
			for( AkUInt32 uMask = in_uChannelMask & ~AK_SPEAKER_LOW_FREQUENCY; uMask; uMask &= uMask - 1 )
			{
				++uNumNoLFE;
			}
			for( AkUInt32 i = 1; i < uNumNoLFE; ++i )
			{
				out_uChannelPhase[ i ] = (AkReal32)( (double)rand() * 0.00003051850947599719 * in_phaseParams.fPhaseSpread );
			}
		}
		break;
	default:
		break;
	}
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		out_uChannelPhase[ i ] = out_uChannelPhase[ i ] + in_phaseParams.fPhaseOffset;
	}
}

}
}
}
