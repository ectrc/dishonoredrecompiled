/*=============================================================================
	AkResamplerCommon.h - the inline pieces shared by the pitch routines of akresamplercommon.cpp and akresamplersimd.cpp
	(retail expands them in every routine): the LFE-last channel remap of interleaved input, the 16-bit and float linear
	interpolation, and the offsets / AKRESULT bookkeeping every routine ends with.

	DISHONORED(port): agent IH2 (wave 26).
=============================================================================*/
#pragma once

#include "AkResampler.h"

namespace
{
	const AkReal32 INT_TO_FLOAT_31 = 4.6566129e-10f;
	const AkReal64 FRAC_TO_FLOAT = 0.000015258789;

	inline AkUInt32 PipelineChannelIndex( AkUInt32 in_uChannel, AkUInt32 in_uChannelMask, AkUInt32 in_uNumChannels )
	{
		if( in_uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
		{
			const AkUInt32 uNumFrontChannels = AkGetNumChannels( in_uChannelMask & ( AK_SPEAKER_FRONT_LEFT | AK_SPEAKER_FRONT_RIGHT | AK_SPEAKER_FRONT_CENTER ) );
			if( in_uChannel == uNumFrontChannels )
			{
				return in_uNumChannels - 1;
			}
			if( in_uChannel > uNumFrontChannels )
			{
				return in_uChannel - 1;
			}
		}
		return in_uChannel;
	}

	inline AkReal32 InterpolateI16( AkInt32 in_iPrevious, AkInt32 in_iNext, AkUInt32 in_uFrac )
	{
		return (AkReal32)(AkInt32)( (AkUInt32)( in_iPrevious << 16 ) + in_uFrac * (AkUInt32)( in_iNext - in_iPrevious ) ) * INT_TO_FLOAT_31;
	}

	inline AkReal32 InterpolateNative( AkReal32 in_fPrevious, AkReal32 in_fNext, AkUInt32 in_uFrac )
	{
		return (AkReal32)( (AkReal64)in_uFrac * (AkReal64)( in_fNext - in_fPrevious ) * FRAC_TO_FLOAT + in_fPrevious );
	}

	/** The common tail of every pitch routine: consumed input, produced output, offsets and the AKRESULT. */
	inline AKRESULT FinishPitch( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkInternalPitchState * io_pPitchState, AkUInt32 in_uFramesConsumed, AkUInt32 in_uInBufferFrames, AkUInt32 in_uFramesProduced, AkUInt32 in_uOutBufferFrames )
	{
		io_pInBuffer->uValidFrames = (AkUInt16)( io_pInBuffer->uValidFrames - in_uFramesConsumed );
		io_pOutBuffer->uValidFrames = (AkUInt16)( in_uFramesProduced + (AkUInt16)io_pPitchState->uOutFrameOffset );
		if( in_uFramesConsumed == in_uInBufferFrames )
		{
			io_pPitchState->uInFrameOffset = 0;
		}
		else
		{
			io_pPitchState->uInFrameOffset += in_uFramesConsumed;
		}
		if( in_uFramesProduced == in_uOutBufferFrames )
		{
			return AK_DataReady;
		}
		io_pPitchState->uOutFrameOffset += in_uFramesProduced;
		return AK_DataNeeded;
	}
}
