/*=============================================================================
	AkResamplerSIMD.cpp - port of akresamplersimd.cpp (2012 PDB, d:\branches\wwise_v2012.1\): the 1- and 2-channel pitch
	routines and the vectorised 16-bit bypass.

	DISHONORED(port): agent IH2 (wave 26). Deviations, all without a change of result:
	  * Bypass_I16_NChanVec (MMX) and Bypass_I16_2Chan (MMX + SSE) are written scalar: x * 2^-15 of a 16-bit integer is
	    exact in float, so every output sample is bit-identical to the MMX path. The SSE2 twins keep the intrinsics.
	  * The 2-channel float routines read the right channel at the OUTPUT buffer's uMaxFrames stride, as retail does
	    (both are LE_MAX_FRAMES_PER_BUFFER in the pipeline).
	Bypass_I16_NChanVec(SSE2) leaves the output INTERLEAVED (one float per input sample); CAkVPLPitchNode deinterleaves
	afterwards when CAkResampler::IsPostDeInterleaveRequired.
=============================================================================*/
#include "AkResamplerCommon.h"
#include <emmintrin.h>
#include <string.h>

namespace
{
	const AkReal32 SHORT_TO_FLOAT = 0.000030517578f;
}

/** 2012 0x959080; 2013 0x955920. */
AKRESULT Bypass_I16_NChanVec( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFramesToCopy = uOutBufferFrames < uInBufferFrames ? uOutBufferFrames : uInBufferFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pInBuffer->uChannelMask );
	const AkInt16 * pIn = (AkInt16 *)io_pInBuffer->pData + uNumChannels * io_pPitchState->uInFrameOffset;
	AkReal32 * pOut = (AkReal32 *)io_pOutBuffer->pData + uNumChannels * io_pPitchState->uOutFrameOffset;
	for( AkUInt32 n = uFramesToCopy * uNumChannels; n; --n )
	{
		*pOut++ = (AkReal32)*pIn++ * SHORT_TO_FLOAT;
	}
	if( uNumChannels )
	{
		memcpy( io_pPitchState->iLastValue, pIn - uNumChannels, uNumChannels * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = FPMUL;
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesToCopy, uInBufferFrames, uFramesToCopy, uOutBufferFrames );
}

/** 2012 0x959260; 2013 0x955b00. */
AKRESULT Bypass_I16_NChanVecSSE2( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFramesToCopy = uOutBufferFrames < uInBufferFrames ? uOutBufferFrames : uInBufferFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pInBuffer->uChannelMask );
	const AkInt16 * pIn = (AkInt16 *)io_pInBuffer->pData + uNumChannels * io_pPitchState->uInFrameOffset;
	AkReal32 * pOut = (AkReal32 *)io_pOutBuffer->pData + uNumChannels * io_pPitchState->uOutFrameOffset;
	const AkUInt32 uNumSamples = uFramesToCopy * uNumChannels;
	const AkUInt32 uNumBlocks = uNumSamples >> 4;
	const __m128 vScale = _mm_set1_ps( SHORT_TO_FLOAT );
	const __m128i vZero = _mm_setzero_si128();
	for( AkUInt32 b = 0; b < uNumBlocks; ++b )
	{
		const __m128i vIn0 = _mm_loadu_si128( (const __m128i *)pIn );
		const __m128i vIn1 = _mm_loadu_si128( (const __m128i *)pIn + 1 );
		const __m128i vSign0 = _mm_cmpgt_epi16( vZero, vIn0 );
		const __m128i vSign1 = _mm_cmpgt_epi16( vZero, vIn1 );
		_mm_store_ps( pOut, _mm_mul_ps( _mm_cvtepi32_ps( _mm_unpacklo_epi16( vIn0, vSign0 ) ), vScale ) );
		_mm_store_ps( pOut + 4, _mm_mul_ps( _mm_cvtepi32_ps( _mm_unpackhi_epi16( vIn0, vSign0 ) ), vScale ) );
		_mm_store_ps( pOut + 8, _mm_mul_ps( _mm_cvtepi32_ps( _mm_unpacklo_epi16( vIn1, vSign1 ) ), vScale ) );
		_mm_store_ps( pOut + 12, _mm_mul_ps( _mm_cvtepi32_ps( _mm_unpackhi_epi16( vIn1, vSign1 ) ), vScale ) );
		pIn += 16;
		pOut += 16;
	}
	for( AkUInt32 n = uNumSamples - 16 * uNumBlocks; n; --n )
	{
		*pOut++ = (AkReal32)*pIn++ * SHORT_TO_FLOAT;
	}
	if( uNumChannels )
	{
		memcpy( io_pPitchState->iLastValue, pIn - uNumChannels, uNumChannels * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = FPMUL;
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesToCopy, uInBufferFrames, uFramesToCopy, uOutBufferFrames );
}

/** 2012 0x9593f0; 2013 0x955c90. */
AKRESULT Bypass_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFramesToCopy = uOutBufferFrames < uInBufferFrames ? uOutBufferFrames : uInBufferFrames;
	const AkInt16 * pIn = (AkInt16 *)io_pInBuffer->pData + 2 * io_pPitchState->uInFrameOffset;
	AkReal32 * pOutL = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	AkReal32 * pOutR = pOutL + io_pOutBuffer->uMaxFrames;
	memcpy( io_pPitchState->iLastValue, pIn + 2 * ( uFramesToCopy - 1 ), 2 * sizeof( AkInt16 ) );
	for( AkUInt32 n = uFramesToCopy; n; --n )
	{
		*pOutL++ = (AkReal32)pIn[ 0 ] * SHORT_TO_FLOAT;
		*pOutR++ = (AkReal32)pIn[ 1 ] * SHORT_TO_FLOAT;
		pIn += 2;
	}
	io_pPitchState->uFloatIndex = FPMUL;
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesToCopy, uInBufferFrames, uFramesToCopy, uOutBufferFrames );
}

/** 2012 0x9596a0; 2013 0x955f40. */
AKRESULT Bypass_I16_2ChanSSE2( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFramesToCopy = uOutBufferFrames < uInBufferFrames ? uOutBufferFrames : uInBufferFrames;
	const AkInt16 * pIn = (AkInt16 *)io_pInBuffer->pData + 2 * io_pPitchState->uInFrameOffset;
	AkReal32 * pOutL = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	const AkUInt32 uMaxFrames = io_pOutBuffer->uMaxFrames;
	memcpy( io_pPitchState->iLastValue, pIn + 2 * ( uFramesToCopy - 1 ), 2 * sizeof( AkInt16 ) );
	const AkUInt32 uNumBlocks = uFramesToCopy >> 4;
	const __m128 vScale = _mm_set1_ps( SHORT_TO_FLOAT );
	for( AkUInt32 b = 0; b < uNumBlocks; ++b )
	{
		for( AkUInt32 q = 0; q < 4; ++q )
		{
			const __m128i vIn = _mm_loadu_si128( (const __m128i *)pIn + q );
			_mm_storeu_ps( pOutL + 4 * q, _mm_mul_ps( _mm_cvtepi32_ps( _mm_srai_epi32( _mm_slli_epi32( vIn, 16 ), 16 ) ), vScale ) );
			_mm_storeu_ps( pOutL + 4 * q + uMaxFrames, _mm_mul_ps( _mm_cvtepi32_ps( _mm_srai_epi32( vIn, 16 ) ), vScale ) );
		}
		pIn += 32;
		pOutL += 16;
	}
	for( AkUInt32 n = uFramesToCopy - 16 * uNumBlocks; n; --n )
	{
		pOutL[ 0 ] = (AkReal32)pIn[ 0 ] * SHORT_TO_FLOAT;
		pOutL[ uMaxFrames ] = (AkReal32)pIn[ 1 ] * SHORT_TO_FLOAT;
		++pOutL;
		pIn += 2;
	}
	io_pPitchState->uFloatIndex = FPMUL;
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesToCopy, uInBufferFrames, uFramesToCopy, uOutBufferFrames );
}

/** 2012 0x9598a0; 2013 0x956140. */
AKRESULT Fixed_I16_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - uIndexFP + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	const AkInt32 iPreviousFrame = io_pPitchState->iLastValue[ 0 ];
	for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
	{
		*pfOutBuf++ = InterpolateI16( iPreviousFrame, pInBuf[ 1 ], uIndexFP & FPMASK );
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
	if( uOutBufferFrames - uNumIterPreviousFrame < uNumIterThisFrame )
	{
		uNumIterThisFrame = uOutBufferFrames - uNumIterPreviousFrame;
	}
	for( AkUInt32 n = uNumIterThisFrame; n; --n )
	{
		const AkUInt32 uIndex = uIndexFP >> FPBITS;
		*pfOutBuf++ = InterpolateI16( pInBuf[ uIndex ], pInBuf[ uIndex + 1 ], uIndexFP & FPMASK );
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->iLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterPreviousFrame + uNumIterThisFrame, uOutBufferFrames );
}

/** 2012 0x959a60; 2013 0x956300. */
AKRESULT Fixed_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - uIndexFP + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkUInt32 uMaxFrames = io_pOutBuffer->uMaxFrames;
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + 2 * io_pPitchState->uInFrameOffset - 2;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	const AkInt32 iPreviousFrameL = io_pPitchState->iLastValue[ 0 ];
	const AkInt32 iPreviousFrameR = io_pPitchState->iLastValue[ 1 ];
	for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
	{
		const AkUInt32 uFrac = uIndexFP & FPMASK;
		pfOutBuf[ 0 ] = InterpolateI16( iPreviousFrameL, pInBuf[ 2 ], uFrac );
		pfOutBuf[ uMaxFrames ] = InterpolateI16( iPreviousFrameR, pInBuf[ 3 ], uFrac );
		++pfOutBuf;
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
	if( uOutBufferFrames - uNumIterPreviousFrame < uNumIterThisFrame )
	{
		uNumIterThisFrame = uOutBufferFrames - uNumIterPreviousFrame;
	}
	for( AkUInt32 n = uNumIterThisFrame; n; --n )
	{
		const AkUInt32 uIndex = 2 * ( uIndexFP >> FPBITS );
		const AkUInt32 uFrac = uIndexFP & FPMASK;
		pfOutBuf[ 0 ] = InterpolateI16( pInBuf[ uIndex ], pInBuf[ uIndex + 2 ], uFrac );
		pfOutBuf[ uMaxFrames ] = InterpolateI16( pInBuf[ uIndex + 1 ], pInBuf[ uIndex + 3 ], uFrac );
		++pfOutBuf;
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		memcpy( io_pPitchState->iLastValue, pInBuf + 2 * uFramesConsumed, 2 * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterPreviousFrame + uNumIterThisFrame, uOutBufferFrames );
}

/** 2012 0x959ca0; 2013 0x956540. */
AKRESULT Fixed_Native_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - uIndexFP + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	const AkReal32 fLeftSample = io_pPitchState->fLastValue[ 0 ];
	for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
	{
		*pfOutBuf++ = InterpolateNative( fLeftSample, pInBuf[ 1 ], uIndexFP & FPMASK );
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
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
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->fLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterPreviousFrame + uNumIterThisFrame, uOutBufferFrames );
}

/** 2012 0x959e70; 2013 0x956710. */
AKRESULT Fixed_Native_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uFrameSkipFP = io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	AkUInt32 uNumIterPreviousFrame = ( uFrameSkipFP - uIndexFP + FPMASK ) / uFrameSkipFP;
	if( uOutBufferFrames < uNumIterPreviousFrame )
	{
		uNumIterPreviousFrame = uOutBufferFrames;
	}
	const AkUInt32 uMaxFrames = io_pOutBuffer->uMaxFrames;
	const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	const AkReal32 fPreviousFrameL = io_pPitchState->fLastValue[ 0 ];
	const AkReal32 fPreviousFrameR = io_pPitchState->fLastValue[ 1 ];
	for( AkUInt32 n = uNumIterPreviousFrame; n; --n )
	{
		const AkUInt32 uFrac = uIndexFP & FPMASK;
		pfOutBuf[ 0 ] = InterpolateNative( fPreviousFrameL, pInBuf[ 1 ], uFrac );
		pfOutBuf[ uMaxFrames ] = InterpolateNative( fPreviousFrameR, pInBuf[ uMaxFrames + 1 ], uFrac );
		++pfOutBuf;
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uNumIterThisFrame = ( ( uInBufferFrames << FPBITS ) - uIndexFP + uFrameSkipFP - 1 ) / uFrameSkipFP;
	if( uOutBufferFrames - uNumIterPreviousFrame < uNumIterThisFrame )
	{
		uNumIterThisFrame = uOutBufferFrames - uNumIterPreviousFrame;
	}
	for( AkUInt32 n = uNumIterThisFrame; n; --n )
	{
		const AkUInt32 uIndex = uIndexFP >> FPBITS;
		const AkReal32 fFrac = (AkReal32)( uIndexFP & FPMASK );
		const AkReal32 fL0 = pInBuf[ uIndex ];
		const AkReal32 fR0 = pInBuf[ uIndex + uMaxFrames ];
		pfOutBuf[ 0 ] = ( ( pInBuf[ uIndex + 1 ] - fL0 ) * fFrac ) * 0.000015258789f + fL0;
		pfOutBuf[ uMaxFrames ] = ( ( pInBuf[ uIndex + uMaxFrames + 1 ] - fR0 ) * fFrac ) * 0.000015258789f + fR0;
		++pfOutBuf;
		uIndexFP += uFrameSkipFP;
	}
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->fLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
		io_pPitchState->fLastValue[ 1 ] = pInBuf[ uFramesConsumed + uMaxFrames ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, uNumIterPreviousFrame + uNumIterThisFrame, uOutBufferFrames );
}

/** 2012 0x95a0d0; 2013 0x956970. */
AKRESULT Interpolating_I16_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uRampCount = io_pPitchState->uInterpolationRampCount;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	AkReal32 * pfOutBufStart = pfOutBuf;
	AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
	const AkInt32 iPreviousFrame = io_pPitchState->iLastValue[ 0 ];
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
			*pfOutBuf++ = InterpolateI16( iPreviousFrame, pInBuf[ 1 ], uFrac );
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
			uRampCount += uRampInc;
			const AkUInt32 uIndex = uIndexFP >> FPBITS;
			*pfOutBuf++ = InterpolateI16( pInBuf[ uIndex ], pInBuf[ uIndex + 1 ], uIndexFP & FPMASK );
			uFrameSkip += uRampInc * uFrameSkipDiff;
			uIndexFP += uFrameSkip >> 10;
		}
		while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->iLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, (AkUInt32)( pfOutBuf - pfOutBufStart ), uOutBufferFrames );
}

/** 2012 0x95a320; 2013 0x956bc0. */
AKRESULT Interpolating_I16_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uMaxFrames = io_pOutBuffer->uMaxFrames;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uRampCount = io_pPitchState->uInterpolationRampCount;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	const AkInt16 * pInBuf = (AkInt16 *)io_pInBuffer->pData + 2 * io_pPitchState->uInFrameOffset - 2;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	AkReal32 * pfOutBufStart = pfOutBuf;
	AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
	const AkInt32 iPreviousFrameL = io_pPitchState->iLastValue[ 0 ];
	const AkInt32 iPreviousFrameR = io_pPitchState->iLastValue[ 1 ];
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
			uRampCount += uRampInc;
			const AkUInt32 uFrac = uIndexFP & FPMASK;
			pfOutBuf[ 0 ] = InterpolateI16( iPreviousFrameL, pInBuf[ 2 ], uFrac );
			pfOutBuf[ uMaxFrames ] = InterpolateI16( iPreviousFrameR, pInBuf[ 3 ], uFrac );
			++pfOutBuf;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			uIndexFP += uFrameSkip >> 10;
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
			const AkUInt32 uIndex = 2 * ( uIndexFP >> FPBITS );
			const AkUInt32 uFrac = uIndexFP & FPMASK;
			uRampCount += uRampInc;
			pfOutBuf[ 0 ] = InterpolateI16( pInBuf[ uIndex ], pInBuf[ uIndex + 2 ], uFrac );
			pfOutBuf[ uMaxFrames ] = InterpolateI16( pInBuf[ uIndex + 1 ], pInBuf[ uIndex + 3 ], uFrac );
			++pfOutBuf;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			uIndexFP += uFrameSkip >> 10;
		}
		while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		memcpy( io_pPitchState->iLastValue, pInBuf + 2 * uFramesConsumed, 2 * sizeof( AkInt16 ) );
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, (AkUInt32)( pfOutBuf - pfOutBufStart ), uOutBufferFrames );
}

/** 2012 0x95a5f0; 2013 0x956e90. */
AKRESULT Interpolating_Native_1Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uRampCount = io_pPitchState->uInterpolationRampCount;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	AkReal32 * pfOutBufStart = pfOutBuf;
	AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
	const AkReal32 fLeftSample = io_pPitchState->fLastValue[ 0 ];
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
			uRampCount += uRampInc;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			const AkUInt32 uFrac = uIndexFP & FPMASK;
			uIndexFP += uFrameSkip >> 10;
			*pfOutBuf++ = InterpolateNative( fLeftSample, pInBuf[ 1 ], uFrac );
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
			uRampCount += uRampInc;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			const AkUInt32 uIndex = uIndexFP >> FPBITS;
			const AkUInt32 uFrac = uIndexFP & FPMASK;
			uIndexFP += uFrameSkip >> 10;
			*pfOutBuf++ = InterpolateNative( pInBuf[ uIndex ], pInBuf[ uIndex + 1 ], uFrac );
		}
		while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->fLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, (AkUInt32)( pfOutBuf - pfOutBufStart ), uOutBufferFrames );
}

/** 2012 0x95a840; 2013 0x9570e0. */
AKRESULT Interpolating_Native_2Chan( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer, AkUInt32 uRequestedSize, AkInternalPitchState * io_pPitchState )
{
	const AkUInt32 uInBufferFrames = io_pInBuffer->uValidFrames;
	const AkUInt32 uOutBufferFrames = uRequestedSize - io_pPitchState->uOutFrameOffset;
	const AkUInt32 uMaxFrames = io_pOutBuffer->uMaxFrames;
	const AkUInt32 uRampInc = io_pPitchState->uInterpolationRampInc;
	const AkUInt32 uScaledStartFrameSkip = io_pPitchState->uCurrentFrameSkip << 10;
	const AkUInt32 uFrameSkipDiff = io_pPitchState->uTargetFrameSkip - io_pPitchState->uCurrentFrameSkip;
	AkUInt32 uRampCount = io_pPitchState->uInterpolationRampCount;
	AkUInt32 uIndexFP = io_pPitchState->uFloatIndex;
	const AkReal32 * pInBuf = (AkReal32 *)io_pInBuffer->pData + io_pPitchState->uInFrameOffset - 1;
	AkReal32 * pfOutBuf = (AkReal32 *)io_pOutBuffer->pData + io_pPitchState->uOutFrameOffset;
	AkReal32 * pfOutBufStart = pfOutBuf;
	AkReal32 * pfOutBufEnd = pfOutBuf + uOutBufferFrames;
	const AkReal32 fPreviousFrameL = io_pPitchState->fLastValue[ 0 ];
	const AkReal32 fPreviousFrameR = io_pPitchState->fLastValue[ 1 ];
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
			uRampCount += uRampInc;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			const AkUInt32 uFrac = uIndexFP & FPMASK;
			uIndexFP += uFrameSkip >> 10;
			pfOutBuf[ 0 ] = InterpolateNative( fPreviousFrameL, pInBuf[ 1 ], uFrac );
			pfOutBuf[ uMaxFrames ] = InterpolateNative( fPreviousFrameR, pInBuf[ uMaxFrames + 1 ], uFrac );
			++pfOutBuf;
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
			uRampCount += uRampInc;
			uFrameSkip += uRampInc * uFrameSkipDiff;
			const AkUInt32 uIndex = uIndexFP >> FPBITS;
			const AkReal32 fFrac = (AkReal32)( uIndexFP & FPMASK );
			uIndexFP += uFrameSkip >> 10;
			const AkReal32 fL0 = pInBuf[ uIndex ];
			const AkReal32 fR0 = pInBuf[ uIndex + uMaxFrames ];
			pfOutBuf[ 0 ] = ( ( pInBuf[ uIndex + 1 ] - fL0 ) * fFrac ) * 0.000015258789f + fL0;
			pfOutBuf[ uMaxFrames ] = ( ( pInBuf[ uIndex + uMaxFrames + 1 ] - fR0 ) * fFrac ) * 0.000015258789f + fR0;
			++pfOutBuf;
		}
		while( ( uIndexFP >> FPBITS ) <= uInBufferFrames - 1 );
	}
	io_pPitchState->uInterpolationRampCount = uRampCount;
	AkUInt32 uFramesConsumed = uIndexFP >> FPBITS;
	if( uFramesConsumed >= uInBufferFrames )
	{
		uFramesConsumed = uInBufferFrames;
	}
	if( uFramesConsumed )
	{
		io_pPitchState->fLastValue[ 0 ] = pInBuf[ uFramesConsumed ];
		io_pPitchState->fLastValue[ 1 ] = pInBuf[ uFramesConsumed + uMaxFrames ];
	}
	io_pPitchState->uFloatIndex = uIndexFP - ( uFramesConsumed << FPBITS );
	return FinishPitch( io_pInBuffer, io_pOutBuffer, io_pPitchState, uFramesConsumed, uInBufferFrames, (AkUInt32)( pfOutBuf - pfOutBufStart ), uOutBufferFrames );
}
