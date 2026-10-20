/*=============================================================================
	AkStereoDelayDSPProcess.cpp - AkStereoDelayDSPProcess, Wwise 2012.1 (plugins/effects/akstereodelay/akstereodelaydspprocess.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (2012 0x999820, no 2013 twin). The front pair's input is built per
	side from its input type (the side's channel, the center, the side + center downmix at 0.707 each, or silence), runs
	through the front delay line and is mixed into L / R with the front share of the front/rear balance; with both rears
	present the rear pair does the same through the rear line. The center (when not the only channel) and the LFE get the
	ramped dry level; in send mode the LFE is silenced first. Kept as retail: the wet ramps start at prev wet x current
	balance gain and end at prev balance gain x current wet; a center-only buffer mixes C + both wet sides.
=============================================================================*/
#include "AkStereoDelayFX.h"
#include "Mix2Interp.h"
#include "AkApplyGain.h"
#include <math.h>
#include <string.h>

/** 2012 0x999820. */
void AkStereoDelayDSPProcess( AkAudioBuffer * io_pBuffer, AkStereoDelayFXInfo & io_FXInfo, AkReal32 * in_pfStereoBufferStorage )
{
	io_FXInfo.FXTailHandler.HandleTail( io_pBuffer, io_FXInfo.uTailLength );
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	if( !uNumFrames )
	{
		return;
	}
	if( io_pBuffer->uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
	{
		AkReal32 * pfLFE = io_pBuffer->GetChannel( io_pBuffer->NumChannels() - 1 );
		if( pfLFE && io_FXInfo.bSendMode )
		{
			memset( pfLFE, 0, sizeof( AkReal32 ) * uNumFrames );
		}
	}
	if( io_pBuffer->uChannelMask == AK_SPEAKER_LOW_FREQUENCY )
	{
		return;
	}
	const AkChannelMask InputChannelMask = io_pBuffer->uChannelMask & ~AK_SPEAKER_LOW_FREQUENCY;
	AkAudioBuffer StereoInputBuffer;
	AkAudioBuffer StereoOutputBuffer;
	StereoInputBuffer.pData = in_pfStereoBufferStorage;
	StereoInputBuffer.uMaxFrames = (AkUInt16)uNumFrames;
	StereoInputBuffer.uValidFrames = (AkUInt16)uNumFrames;
	StereoInputBuffer.uChannelMask = AK_SPEAKER_SETUP_STEREO;
	StereoInputBuffer.eState = AK_DataReady;
	StereoOutputBuffer.pData = in_pfStereoBufferStorage + 2 * uNumFrames;
	StereoOutputBuffer.uMaxFrames = (AkUInt16)uNumFrames;
	StereoOutputBuffer.uValidFrames = (AkUInt16)uNumFrames;
	StereoOutputBuffer.uChannelMask = AK_SPEAKER_SETUP_STEREO;
	StereoOutputBuffer.eState = AK_DataReady;
	const AkUInt32 uNumInputChannels = AkGetNumChannels( InputChannelMask );
	const AkReal32 fDownmix = 0.70700002f;
	const AkUInt32 uBytes = sizeof( AkReal32 ) * uNumFrames;
	const bool bLeftAndCenter = ( InputChannelMask & AK_SPEAKER_FRONT_LEFT ) && ( InputChannelMask & AK_SPEAKER_FRONT_CENTER );
	const bool bRightAndCenter = ( InputChannelMask & AK_SPEAKER_FRONT_RIGHT ) && ( InputChannelMask & AK_SPEAKER_FRONT_CENTER );

	AkReal32 * pfInputL = StereoInputBuffer.GetChannel( 0 );
	switch( io_FXInfo.Params.eInputType[ 0 ] )
	{
	case AKINPUTCHANNELTYPE_LEFT_OR_RIGHT:
		memcpy( pfInputL, io_pBuffer->GetChannel( 0 ), uBytes );
		break;
	case AKINPUTCHANNELTYPE_CENTER:
		memcpy( pfInputL, io_pBuffer->GetChannel( bLeftAndCenter ? 2 : 0 ), uBytes );
		break;
	case AKINPUTCHANNELTYPE_DOWNMIX:
		if( bLeftAndCenter )
		{
			DSP::Mix2Interp( io_pBuffer->GetChannel( 0 ), io_pBuffer->GetChannel( 2 ), pfInputL, fDownmix, fDownmix, fDownmix, fDownmix, uNumFrames );
		}
		else
		{
			memcpy( pfInputL, io_pBuffer->GetChannel( 0 ), uBytes );
		}
		break;
	case AKINPUTCHANNELTYPE_NONE:
		memset( pfInputL, 0, uBytes );
		break;
	default:
		break;
	}
	AkReal32 * pfInputR = StereoInputBuffer.GetChannel( 1 );
	switch( io_FXInfo.Params.eInputType[ 1 ] )
	{
	case AKINPUTCHANNELTYPE_LEFT_OR_RIGHT:
		memcpy( pfInputR, io_pBuffer->GetChannel( ( InputChannelMask & AK_SPEAKER_FRONT_RIGHT ) ? 1 : 0 ), uBytes );
		break;
	case AKINPUTCHANNELTYPE_CENTER:
		{
			AkUInt32 uChannel = 2;
			if( !bRightAndCenter )
			{
				uChannel = uNumInputChannels - 1;
				if( uChannel > 1 )
				{
					uChannel = 1;
				}
			}
			memcpy( pfInputR, io_pBuffer->GetChannel( uChannel ), uBytes );
		}
		break;
	case AKINPUTCHANNELTYPE_DOWNMIX:
		if( bRightAndCenter )
		{
			DSP::Mix2Interp( io_pBuffer->GetChannel( 1 ), io_pBuffer->GetChannel( 2 ), pfInputR, fDownmix, fDownmix, fDownmix, fDownmix, uNumFrames );
		}
		else
		{
			memcpy( pfInputR, io_pBuffer->GetChannel( ( InputChannelMask & AK_SPEAKER_FRONT_RIGHT ) ? 1 : 0 ), uBytes );
		}
		break;
	case AKINPUTCHANNELTYPE_NONE:
		memset( pfInputR, 0, uBytes );
		break;
	default:
		break;
	}

	io_FXInfo.StereoDelay[ 0 ].ProcessBuffer( &StereoInputBuffer, &StereoOutputBuffer, io_FXInfo.PrevParams.StereoDelayParams, io_FXInfo.Params.StereoDelayParams,
		io_FXInfo.Params.FilterParams, io_FXInfo.bRecomputeFilterCoefs );

	AkReal32 fCurFrontGain = 1.f, fCurRearGain = 1.f, fPrevFrontGain = 1.f, fPrevRearGain = 1.f;
	if( InputChannelMask & AK_SPEAKER_BACK_LEFT )
	{
		const AkReal32 fBalance = ( io_FXInfo.Params.fFrontRearBalance + 100.f ) * 0.005f;
		fCurRearGain = sqrtf( fBalance );
		fCurFrontGain = sqrtf( 1.f - fBalance );
		const AkReal32 fPrevBalance = ( io_FXInfo.PrevParams.fFrontRearBalance + 100.f ) * 0.005f;
		fPrevRearGain = sqrtf( fPrevBalance );
		fPrevFrontGain = sqrtf( 1.f - fPrevBalance );
	}
	AkReal32 * pfOutputL = StereoOutputBuffer.GetChannel( 0 );
	AkReal32 * pfOutputR = StereoOutputBuffer.GetChannel( 1 );
	const AkReal32 fPrevDry = io_FXInfo.PrevParams.fDryLevel;
	const AkReal32 fDry = io_FXInfo.Params.fDryLevel;
	const AkReal32 fPrevWet = io_FXInfo.PrevParams.fWetLevel;
	const AkReal32 fWet = io_FXInfo.Params.fWetLevel;
	if( InputChannelMask == AK_SPEAKER_FRONT_CENTER )
	{
		DSP::Mix3Interp( io_pBuffer->GetChannel( 0 ), pfOutputL, pfOutputR, fPrevDry, fDry, fPrevWet, fWet, fPrevWet, fWet, uNumFrames );
	}
	else if( InputChannelMask & AK_SPEAKER_FRONT_LEFT )
	{
		DSP::Mix2Interp( io_pBuffer->GetChannel( 0 ), pfOutputL, fPrevDry, fDry, fPrevWet * fCurFrontGain, fPrevFrontGain * fWet, uNumFrames );
		DSP::Mix2Interp( io_pBuffer->GetChannel( 1 ), pfOutputR, fPrevDry, fDry, fPrevWet * fCurFrontGain, fPrevFrontGain * fWet, uNumFrames );
	}

	if( ( InputChannelMask & AK_SPEAKER_BACK_LEFT ) && ( InputChannelMask & AK_SPEAKER_BACK_RIGHT ) )
	{
		const AkUInt32 uRearLeft = ( ( InputChannelMask & AK_SPEAKER_FRONT_CENTER ) != 0 ) | 2;
		switch( io_FXInfo.Params.eInputType[ 0 ] )
		{
		case AKINPUTCHANNELTYPE_LEFT_OR_RIGHT:
		case AKINPUTCHANNELTYPE_DOWNMIX:
			memcpy( pfInputL, io_pBuffer->GetChannel( uRearLeft ), uBytes );
			break;
		case AKINPUTCHANNELTYPE_CENTER:
		case AKINPUTCHANNELTYPE_NONE:
			memset( pfInputL, 0, uBytes );
			break;
		default:
			break;
		}
		switch( io_FXInfo.Params.eInputType[ 1 ] )
		{
		case AKINPUTCHANNELTYPE_LEFT_OR_RIGHT:
		case AKINPUTCHANNELTYPE_DOWNMIX:
			memcpy( pfInputR, io_pBuffer->GetChannel( uRearLeft + 1 ), uBytes );
			break;
		case AKINPUTCHANNELTYPE_CENTER:
		case AKINPUTCHANNELTYPE_NONE:
			memset( pfInputR, 0, uBytes );
			break;
		default:
			break;
		}
		io_FXInfo.StereoDelay[ 1 ].ProcessBuffer( &StereoInputBuffer, &StereoOutputBuffer, io_FXInfo.PrevParams.StereoDelayParams, io_FXInfo.Params.StereoDelayParams,
			io_FXInfo.Params.FilterParams, io_FXInfo.bRecomputeFilterCoefs );
		DSP::Mix2Interp( io_pBuffer->GetChannel( uRearLeft ), pfOutputL, fPrevDry, fDry, fPrevWet * fCurRearGain, fPrevRearGain * fWet, uNumFrames );
		DSP::Mix2Interp( io_pBuffer->GetChannel( uRearLeft + 1 ), pfOutputR, fPrevDry, fDry, fPrevWet * fCurRearGain, fPrevRearGain * fWet, uNumFrames );
	}
	if( ( InputChannelMask & AK_SPEAKER_FRONT_CENTER ) && InputChannelMask != AK_SPEAKER_FRONT_CENTER )
	{
		AK::DSP::ApplyGain( io_pBuffer->GetChannel( 2 ), uNumFrames, fPrevDry, fDry );
	}
	if( io_pBuffer->uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
	{
		AkReal32 * pfLFE = io_pBuffer->GetChannel( io_pBuffer->NumChannels() - 1 );
		if( pfLFE )
		{
			AK::DSP::ApplyGain( pfLFE, uNumFrames, fPrevDry, fDry );
		}
	}
	io_FXInfo.PrevParams = io_FXInfo.Params;
}
