/*=============================================================================
	ConstantPowerChannelMixdown.cpp - see ConstantPowerChannelMixdown.h. Agent IH4 fork F1 (wave 26).
=============================================================================*/
#include "ConstantPowerChannelMixdown.h"
#include <math.h>
#include <string.h>

namespace DSP
{
	/** 2012 0x98f2e0: g = sqrt(1 / (Nfull + c^2 + lfe^2)); every present channel adds g (the center g c) times its samples;
	    the LFE, skipped while other channels remain, is added last with g lfe (an LFE-only buffer is the LFE). */
	void ConstantPowerChannelMixdown( AkAudioBuffer * in_pAudioBufferIn, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset, AkReal32 * out_pfBufferOut,
		AkUInt32 in_uChannelMask, AkReal32 in_fInCenterGain, AkReal32 in_fInLFEGain )
	{
		if( in_fInLFEGain > 1.f )
		{
			in_fInLFEGain = 1.f;
		}
		if( in_fInCenterGain > 1.f )
		{
			in_fInCenterGain = 1.f;
		}
		AkReal32 fUnscaled = 0.f;
		AkReal32 fCenter = 0.f;
		AkReal32 fLFE = 0.f;
		if( in_uChannelMask & AK_SPEAKER_FRONT_LEFT )
		{
			fUnscaled = 1.f;
		}
		if( in_uChannelMask & AK_SPEAKER_FRONT_RIGHT )
		{
			fUnscaled = fUnscaled + 1.f;
		}
		if( in_uChannelMask & AK_SPEAKER_BACK_LEFT )
		{
			fUnscaled = fUnscaled + 1.f;
		}
		if( in_uChannelMask & AK_SPEAKER_BACK_RIGHT )
		{
			fUnscaled = fUnscaled + 1.f;
		}
		if( in_uChannelMask & AK_SPEAKER_FRONT_CENTER )
		{
			fCenter = in_fInCenterGain * in_fInCenterGain;
		}
		if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
		{
			fLFE = in_fInLFEGain * in_fInLFEGain;
		}
		const AkReal32 fChanGain = (AkReal32)sqrt( 1.0 / ( fLFE + fCenter + fUnscaled ) );
		memset( out_pfBufferOut, 0, sizeof( AkReal32 ) * in_uNumFrames );
		AkUInt32 uMask = in_uChannelMask;
		AkUInt32 uChannel = 0;
		for( AkUInt32 uBit = 1; uMask; uBit <<= 1 )
		{
			if( !( uBit & uMask ) && uMask != AK_SPEAKER_LOW_FREQUENCY )
			{
				continue;
			}
			AkReal32 fGain = fChanGain;
			if( uBit & AK_SPEAKER_FRONT_CENTER )
			{
				fGain = fChanGain * in_fInCenterGain;
			}
			if( uMask == AK_SPEAKER_LOW_FREQUENCY )
			{
				uMask = 0;
				fGain = fGain * in_fInLFEGain;
			}
			else if( uBit & AK_SPEAKER_LOW_FREQUENCY )
			{
				continue;
			}
			const AkReal32 * pfIn = (const AkReal32 *)in_pAudioBufferIn->pData + in_uFrameOffset + uChannel * in_pAudioBufferIn->uMaxFrames;
			for( AkUInt32 i = 0; i < in_uNumFrames; ++i )
			{
				out_pfBufferOut[ i ] = pfIn[ i ] * fGain + out_pfBufferOut[ i ];
			}
			uMask &= ~uBit;
			++uChannel;
		}
	}
}
