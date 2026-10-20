/*=============================================================================
	AkPitchShifterDSPProcess.cpp - AkPitchShifterDSPProcess, Wwise 2012.1 (plugins/effects/akpitchshifter/akpitchshifterdspprocess.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (2012 0x9975c0, 2013 0x98e530). The buffer's channels are walked
	in speaker-bit order (L, R, C, LFE, rear L, rear R); the LFE is the buffer's last channel. A processed channel goes
	through the pitch voice (and the biquad) into the scratch buffer, its dry part through the sync delay, then the
	two are cross-faded with ramped dry / wet gains; any other channel only gets the ramped dry gain.
=============================================================================*/
#include "AkPitchShifterFX.h"
#include "Mix2Interp.h"
#include "AkApplyGain.h"

/** 2012 0x9975c0, 2013 0x98e530. */
void AkPitchShifterDSPProcess( AkAudioBuffer * io_pBuffer, AkPitchShifterFXInfo & io_FXInfo, AkReal32 * in_pfBufferStorage, void * /*in_pScratch*/ )
{
	io_FXInfo.FXTailHandler.HandleTail( io_pBuffer, io_FXInfo.uTailLength );
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	if( !uNumFrames )
	{
		return;
	}
	const AkChannelMask eBufferChannelMask = io_pBuffer->uChannelMask;
	AkUInt32 uDryChannelIndex = 0;
	AkUInt32 uWetProcessedChan = 0;
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
		const bool bProcessed = io_FXInfo.uNumProcessedChannels && ( uChannelBit & io_FXInfo.eProcessChannelMask );
		if( bProcessed )
		{
			io_FXInfo.PitchShifter.ProcessChannel( pfChannel, in_pfBufferStorage, uNumFrames, uWetProcessedChan );
			if( io_FXInfo.Params.Voice.Filter.eFilterType != AKFILTERTYPE_NONE )
			{
				io_FXInfo.Filter.ProcessChannel( in_pfBufferStorage, uNumFrames, uWetProcessedChan );
			}
			++uWetProcessedChan;
		}
		if( io_FXInfo.Params.bSyncDry )
		{
			( pDryDelay++ )->ProcessBuffer( pfChannel, uNumFrames );
		}
		if( bProcessed )
		{
			DSP::Mix2Interp( pfChannel, in_pfBufferStorage, io_FXInfo.PrevParams.fDryLevel, io_FXInfo.Params.fDryLevel, io_FXInfo.PrevParams.fWetLevel, io_FXInfo.Params.fWetLevel, uNumFrames );
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
