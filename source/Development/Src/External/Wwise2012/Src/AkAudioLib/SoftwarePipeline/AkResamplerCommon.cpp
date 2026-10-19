/*=============================================================================
	AkResamplerCommon.cpp - port of akresamplercommon.cpp (2012 PDB, d:\branches\wwise_v2012.1\): the N-channel pitch
	routines and the native-float (de)interleavers.

	DISHONORED(port): agent IH2 (wave 26). 16-bit input is interleaved in WAV channel order; the output channel of input
	channel i moves the LFE to the last plane (retail's inline remap, PipelineChannelIndex below). The 16-bit interpolation
	is retail's integer form ((prev << 16) + frac * (next - prev)) * 2^-31, done in unsigned 32-bit arithmetic so the
	intermediate product wraps as retail's imul does. Retail evaluates the float routines on the x87 stack; the
	intermediates are kept in double here.
=============================================================================*/
#include "AkResamplerCommon.h"
#include <string.h>

/** 2012 0x957f40; 2013 0x9547e0. */
AKRESULT Bypass_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFramesToCopy = uOutBufferFrames < uInBufferFrames ? uOutBufferFrames : uInBufferFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pInBuffer->uChannelMask );
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		AkReal32 * pIn = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset + i * io_pInBuffer->uMaxFrames;
		memcpy( (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset + i * io_pOutBuffer->uMaxFrames, pIn, uFramesToCopy * sizeof( AkReal32 ) );
		io_pPitchState->fLastValue[ i ] = pIn[ uFramesToCopy - 1 ];
	}
	io_pPitchState->uFloatIndex = FPMUL;
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesToCopy, uInBufferFrames, uFramesToCopy, uOutBufferFrames );
}

/** 2012 0x958030; 2013 0x9548d0. */
AKRESULT Fixed_I16_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - io_pPitchState->uFloatIndex + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkUInt32 uChannelMask = io_pInBuffer->uChannelMask;
	const AkUInt32 uNumChannels = AkGetNumChannels( uChannelMask );
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + uNumChannels * ( io_pPitchState->uInFrameOffset - 1 );
	const AkUInt32 uStartIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uIndexFP = uStartIndexFP;
	AkUInt32 uNumIterThisFrame = 0;
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		uIndexFP = uStartIndexFP;
		const AkInt16 * pInBufChan = pInBuf + i;
		AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset + PipelineChannelIndex( i, uChannelMask, uNumChannels ) * io_pOutBuffer->uMaxFrames;
		const AkInt32 iPreviousFrame = io_pPitchState->iLastValue[ i ];
		for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
		{
			*pfOutBuf++ = InterpolateI16( iPreviousFrame, pInBufChan[ uNumChannels ], uIndexFP & FPMASK );
			uIndexFP += uFrameSkipFP;
		}
		uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
		if( uOutBufferFrames - uNumIterPreviousFrame < uNumIterThisFrame )
		{
			uNumIterThisFrame = uOutBufferFrames - uNumIterPreviousFrame;
		}
		for( AkUInt32 n = uNumIterThisFrame; n; --n )
		{
			const AkUInt32 uIndex = uIndexFP >> FPBITS;
			*pfOutBuf++ = InterpolateI16( pInBufChan[ uIndex * uNumChannels ], pInBufChan[ uIndex * uNumChannels + uNumChannels ], uIndexFP & FPMASK );
			uIndexFP += uFrameSkipFP;
		}
	}
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed && uNumChannels )
	{
		memcpy( io_pPitchState->iLastValue, pInBuf + uFramesConsumed * uNumChannels, uNumChannels * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterThisFrame + uNumIterPreviousFrame, uOutBufferFrames );
}

/** 2012 0x958320; 2013 0x954bc0. */
AKRESULT Fixed_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - io_pPitchState->uFloatIndex + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pInBuffer->uChannelMask );
	const AkUInt32 uStartIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uIndexFP = uStartIndexFP;
	AkUInt32 uNumIterThisFrame = 0;
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		uIndexFP = uStartIndexFP;
		const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1 + i * io_pInBuffer->uMaxFrames;
		AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset + i * io_pOutBuffer->uMaxFrames;
		const AkReal32 fPreviousFrame = io_pPitchState->fLastValue[ i ];
		for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
		{
			*pfOutBuf++ = InterpolateNative( fPreviousFrame, pInBuf[ 1 ], uIndexFP & FPMASK );
			uIndexFP += uFrameSkipFP;
		}
		uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
		if( uOutBufferFrames - uNumIterPreviousFrame < uNumIterThisFrame )
		{
			uNumIterThisFrame = uOutBufferFrames - uNumIterPreviousFrame;
		}
		for( AkUInt32 n = uNumIterThisFrame; n; --n )
		{
			const AkUInt32 uIndex = uIndexFP >> FPBITS;
			*pfOutBuf++ = InterpolateNative( pInBuf[ uIndex ], pInBuf[ uIndex + 1 ], uIndexFP & FPMASK );
			uIndexFP += uFrameSkipFP;
		}
	}
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pPitchState->fLastValue[ i ] = ( (AkReal32 *)io_pInBuffer->pData )[ uFramesConsumed + io_pPitchState->uInFrameOffset + i * io_pInBuffer->uMaxFrames - 1 ];
		}
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterPreviousFrame + uNumIterThisFrame, uOutBufferFrames );
}

/** 2012 0x958670; 2013 0x954f10. */
AKRESULT Interpolating_I16_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uChannelMask = io_pInBuffer->uChannelMask;
	const AkUInt32 uNumChannels = AkGetNumChannels( uChannelMask );
	const AkUInt32 uStartRampCount = io_pPitchState->uInterpolationRampCount;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + uNumChannels * ( io_pPitchState->uInFrameOffset - 1 );
	const AkUInt32 uStartIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uIndexFP = uStartIndexFP;
	AkUInt32 uRampCount = uStartRampCount;
	AkUInt32 uFramesProduced = 0;
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		uIndexFP = uStartIndexFP;
		uRampCount = uStartRampCount;
		const AkInt16 * pInBufChan = pInBuf + i;
		AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset + PipelineChannelIndex( i, uChannelMask, uNumChannels ) * io_pOutBuffer->uMaxFrames;
		AkReal32 * pfOutBufStart = pfOutBuf;
		AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
		const AkInt32 iPreviousFrame = io_pPitchState->iLastValue[ i ];
		AkUInt32 uMaxNumIter = uOutBufferFrames;
		if( uMaxNumIter >= ( PITCHRAMPLENGTH - uRampCount ) / uRampInc )
		{
			uMaxNumIter = ( PITCHRAMPLENGTH - uRampCount ) / uRampInc;
		}
		if( !( uIndexFP >> FPBITS ) )
		{
			AkUInt32 uFrameSkip = uScaledStartFrameSkip + uRampCount * uFrameSkipDiff;
			while( uMaxNumIter )
			{
				uFrameSkip += uRampInc * uFrameSkipDiff;
				const AkUInt32 uFrac = uIndexFP & FPMASK;
				uIndexFP += uFrameSkip >> 10;
				*pfOutBuf++ = InterpolateI16( iPreviousFrame, pInBufChan[ uNumChannels ], uFrac );
				uRampCount += uRampInc;
				if( uIndexFP >> FPBITS )
				{
					break;
				}
				--uMaxNumIter;
			}
		}
		uMaxNumIter = (AkUInt32)( pfOutBufEnd - pfOutBuf );
		if( uMaxNumIter >= ( PITCHRAMPLENGTH - uRampCount ) / uRampInc )
		{
			uMaxNumIter = ( PITCHRAMPLENGTH - uRampCount ) / uRampInc;
		}
		if( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 )
		{
			AkUInt32 uFrameSkip = uScaledStartFrameSkip + uRampCount * uFrameSkipDiff;
			do
			{
				if( !uMaxNumIter-- )
				{
					break;
				}
				uRampCount += uRampInc;
				const AkUInt32 uIndex = uIndexFP >> FPBITS;
				*pfOutBuf++ = InterpolateI16( pInBufChan[ uIndex * uNumChannels ], pInBufChan[ uIndex * uNumChannels + uNumChannels ], uIndexFP & FPMASK );
				uFrameSkip += uRampInc * uFrameSkipDiff;
				uIndexFP += uFrameSkip >> 10;
			}
			while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
		}
		uFramesProduced = (AkUInt32)( pfOutBuf - pfOutBufStart );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed && uNumChannels )
	{
		memcpy( io_pPitchState->iLastValue, pInBuf + uFramesConsumed * uNumChannels, uNumChannels * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uFramesProduced, uOutBufferFrames );
}

/** 2012 0x958a30; 2013 0x9552d0. */
AKRESULT Interpolating_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pInBuffer->uChannelMask );
	const AkUInt32 uStartRampCount = io_pPitchState->uInterpolationRampCount;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	const AkUInt32 uStartIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uIndexFP = uStartIndexFP;
	AkUInt32 uRampCount = uStartRampCount;
	AkUInt32 uFramesProduced = 0;
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		uIndexFP = uStartIndexFP;
		uRampCount = uStartRampCount;
		const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1 + i * io_pInBuffer->uMaxFrames;
		AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset + i * io_pOutBuffer->uMaxFrames;
		AkReal32 * pfOutBufStart = pfOutBuf;
		AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
		const AkReal32 fPreviousFrame = io_pPitchState->fLastValue[ i ];
		AkUInt32 uMaxNumIter = uOutBufferFrames;
		if( uMaxNumIter >= ( PITCHRAMPLENGTH - uRampCount ) / uRampInc )
		{
			uMaxNumIter = ( PITCHRAMPLENGTH - uRampCount ) / uRampInc;
		}
		if( !( uIndexFP >> FPBITS ) )
		{
			AkUInt32 uFrameSkip = uScaledStartFrameSkip + uRampCount * uFrameSkipDiff;
			do
			{
				if( !uMaxNumIter-- )
				{
					break;
				}
				uFrameSkip += uRampInc * uFrameSkipDiff;
				uRampCount += uRampInc;
				const AkUInt32 uFrac = uIndexFP & FPMASK;
				uIndexFP += uFrameSkip >> 10;
				*pfOutBuf++ = InterpolateNative( fPreviousFrame, pInBuf[ 1 ], uFrac );
			}
			while( !( uIndexFP >> FPBITS ) );
		}
		uMaxNumIter = (AkUInt32)( pfOutBufEnd - pfOutBuf );
		if( uMaxNumIter >= ( PITCHRAMPLENGTH - uRampCount ) / uRampInc )
		{
			uMaxNumIter = ( PITCHRAMPLENGTH - uRampCount ) / uRampInc;
		}
		if( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 )
		{
			AkUInt32 uFrameSkip = uScaledStartFrameSkip + uRampCount * uFrameSkipDiff;
			do
			{
				if( !uMaxNumIter-- )
				{
					break;
				}
				uFrameSkip += uRampInc * uFrameSkipDiff;
				uRampCount += uRampInc;
				const AkUInt32 uIndex = uIndexFP >> FPBITS;
				const AkUInt32 uFrac = uIndexFP & FPMASK;
				uIndexFP += uFrameSkip >> 10;
				*pfOutBuf++ = InterpolateNative( pInBuf[ uIndex ], pInBuf[ uIndex + 1 ], uFrac );
			}
			while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
		}
		uFramesProduced = (AkUInt32)( pfOutBuf - pfOutBufStart );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pPitchState->fLastValue[ i ] = ( (AkReal32 *)io_pInBuffer->pData )[ uFramesConsumed + io_pPitchState->uInFrameOffset + i * io_pInBuffer->uMaxFrames - 1 ];
		}
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uFramesProduced, uOutBufferFrames );
}

/** 2012 0x958e00; 2013 0x9556a0. Interleaved floats (in) to planes (out), LFE to the last plane; out->uValidFrames frames. */
void Deinterleave_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer )
{
	const AkUInt32 uNumFrames = io_pOutBuffer->uValidFrames;
	const AkUInt32 uChannelMask = io_pInBuffer->uChannelMask;
	const AkUInt32 uNumChannels = AkGetNumChannels( uChannelMask );
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		const AkReal32 * pIn = (AkReal32 *)io_pInBuffer->pData + i;
		AkReal32 * pOut = (AkReal32 *)io_pOutBuffer->pData + PipelineChannelIndex( i, uChannelMask, uNumChannels ) * io_pOutBuffer->uMaxFrames;
		for( AkUInt32 n = 0; n < uNumFrames; ++n )
		{
			*pOut++ = *pIn;
			pIn += uNumChannels;
		}
	}
}

/** 2012 0x958f40; 2013 0x9557e0. Planes (in, LFE last) to interleaved floats (out); out->uValidFrames frames. */
void Interleave_Native_NChan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer )
{
	const AkUInt32 uNumFrames = io_pOutBuffer->uValidFrames;
	const AkUInt32 uChannelMask = io_pInBuffer->uChannelMask;
	const AkUInt32 uNumChannels = AkGetNumChannels( uChannelMask );
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		const AkReal32 * pIn = (AkReal32 *)io_pInBuffer->pData + PipelineChannelIndex( i, uChannelMask, uNumChannels ) * io_pInBuffer->uMaxFrames;
		AkReal32 * pOut = (AkReal32 *)io_pOutBuffer->pData + i;
		for( AkUInt32 n = 0; n < uNumFrames; ++n )
		{
			*pOut = *pIn++;
			pOut += uNumChannels;
		}
	}
}
