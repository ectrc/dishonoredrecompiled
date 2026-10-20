/*=============================================================================
	AkHarmonizerDSPProcess.cpp - AkHarmonizerDSPProcessVoice / AkHarmonizerDSPProcess, Wwise 2012.1
	(plugins/effects/akharmonizer/akharmonizerdspprocess.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (2012 0x992d10 / 0x992dd0, no 2013 twins). Channels are walked in
	speaker-bit order (the LFE is the buffer's last channel). Each processed channel's enabled voices are pitched by their
	resampling vocoder (and biquad) and summed with their ramped gains into the wet buffer, which is then cross-faded with
	the (optionally sync-delayed) dry signal. The tail is 12 windows. Kept as retail: with input "left only", the right
	channel also gets the wet buffer left over from the left channel.
=============================================================================*/
#include "AkHarmonizerFX.h"
#include "Mix2Interp.h"
#include "AkApplyGain.h"
#include <string.h>

/** 2012 0x992d10. */
void AkHarmonizerDSPProcessVoice( AkReal32 * in_pfCurChan, AkHarmonizerFXInfo & io_FXInfo, AkUInt32 in_uChannelIndex, AkUInt32 in_uVoiceIndex,
	AkReal32 * in_pfMonoPitchedVoice, AkReal32 * in_pfWet, AkUInt32 in_uNumFrames, bool in_bNoMoreData, AkReal32 in_fResamplingFactor, AkReal32 * in_pfPVTDWindow )
{
	const AkHarmonizerVoiceParams & rVoice = io_FXInfo.Params.Voice[ in_uVoiceIndex ];
	if( !rVoice.bEnable )
	{
		return;
	}
	io_FXInfo.PhaseVocoder[ in_uVoiceIndex ].ProcessPitchChannel( in_pfCurChan, in_uNumFrames, in_bNoMoreData, in_uChannelIndex, in_pfMonoPitchedVoice, in_fResamplingFactor, in_pfPVTDWindow );
	if( rVoice.Filter.eFilterType != AKFILTERTYPE_NONE )
	{
		io_FXInfo.Filter[ in_uVoiceIndex ].ProcessChannel( in_pfMonoPitchedVoice, in_uNumFrames, in_uChannelIndex );
	}
	DSP::Mix2Interp( in_pfWet, in_pfMonoPitchedVoice, 1.f, 1.f, io_FXInfo.PrevParams.Voice[ in_uVoiceIndex ].fGain, rVoice.fGain, in_uNumFrames );
}

/** 2012 0x992dd0. */
void AkHarmonizerDSPProcess( AkAudioBuffer * io_pBuffer, AkHarmonizerFXInfo & io_FXInfo, AkReal32 * in_pfTempStorage )
{
	io_FXInfo.FXTailHandler.HandleTail( io_pBuffer, 12 * io_FXInfo.Params.uWindowSize );
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	if( !uNumFrames )
	{
		return;
	}
	AkUInt32 uDryChannelIndex = 0;
	AkUInt32 uWetProcessedChan = 0;
	AkReal32 * pfWet = in_pfTempStorage + uNumFrames;
	const AkChannelMask eBufferChannelMask = io_pBuffer->uChannelMask;
	DSP::CDelayLight * pDryDelay = io_FXInfo.DryDelay;
	AkUInt32 uChannelBit = 1;
	for( AkUInt32 i = 0; i < 6; ++i, uChannelBit <<= 1 )
	{
		if( !( uChannelBit & eBufferChannelMask ) )
		{
			continue;
		}
		AkReal32 * pfChannel;
		if( uChannelBit & AK_SPEAKER_LOW_FREQUENCY )
		{
			pfChannel = io_pBuffer->GetLFE();
		}
		else
		{
			pfChannel = io_pBuffer->GetChannel( uDryChannelIndex );
			++uDryChannelIndex;
		}
		const bool bWet = io_FXInfo.bWetPathEnabled && ( uChannelBit & io_FXInfo.eProcessChannelMask );
		if( bWet )
		{
			memset( pfWet, 0, sizeof( AkReal32 ) * uNumFrames );
			const bool bNoMoreData = io_pBuffer->eState == AK_NoMoreData;
			AkHarmonizerDSPProcessVoice( pfChannel, io_FXInfo, uWetProcessedChan, 0, in_pfTempStorage, pfWet, uNumFrames, bNoMoreData, io_FXInfo.Params.Voice[ 0 ].fPitchFactor, in_pfTempStorage + 2 * uNumFrames );
			AkHarmonizerDSPProcessVoice( pfChannel, io_FXInfo, uWetProcessedChan++, 1, in_pfTempStorage, pfWet, uNumFrames, bNoMoreData, io_FXInfo.Params.Voice[ 1 ].fPitchFactor, in_pfTempStorage + 2 * uNumFrames );
		}
		if( io_FXInfo.Params.bSyncDry )
		{
			( pDryDelay++ )->ProcessBuffer( pfChannel, uNumFrames );
		}
		if( bWet || ( io_FXInfo.Params.eInputType == AKINPUTTYPE_LEFTONLY && uChannelBit == AK_SPEAKER_FRONT_RIGHT && ( eBufferChannelMask & AK_SPEAKER_FRONT_RIGHT ) ) )
		{
			DSP::Mix2Interp( pfChannel, pfWet, io_FXInfo.PrevParams.fDryLevel, io_FXInfo.Params.fDryLevel, io_FXInfo.PrevParams.fWetLevel, io_FXInfo.Params.fWetLevel, uNumFrames );
		}
		else
		{
			const AkReal32 fGain = io_FXInfo.PrevParams.fDryLevel;
			if( io_FXInfo.Params.fDryLevel == fGain )
			{
				AK::DSP::ApplyGain( pfChannel, uNumFrames, fGain );
			}
			else
			{
				AK::DSP::ApplyGainRamp( pfChannel, uNumFrames, fGain, io_FXInfo.Params.fDryLevel );
			}
		}
	}
	io_FXInfo.PrevParams = io_FXInfo.Params;
}
