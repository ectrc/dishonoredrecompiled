/*=============================================================================
	AkLPFCommon.cpp - port of aklpfcommon.cpp and aklpfcommon.h's LPF_APPLY_CHANNEL (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). LPF_APPLY_CHANNEL is unrolled by 4 in retail with the same per-sample
	arithmetic; it is the plain loop here. The *Interp routines inline EvalLPFCutoff in retail; they call it here.
	When the filter comes out of bypass (fCurrentLPFPar <= 0.1) the first two frames of every channel seed the filter
	memory unfiltered, as retail does.
=============================================================================*/
#include "AkLPFCommon.h"
#include <math.h>

LPFDSPFuncPtr LPFDSPFuncTable[ 6 ] =
{
	Perform1Chan,
	Perform2Chan,
	PerformNChan,
	Perform1ChanInterp,
	Perform2ChanInterp,
	PerformNChanInterp
};

/** 2012 0x95ab40; 2013 0x9573e0. y = b0 (x + x[n-2]) + b1 x[n-1] + a2 y[n-2] + a1 y[n-1]; advances pfBuf. */
void LPF_APPLY_CHANNEL( AkReal32 * pFiltMem, AkReal32 * pfCoefs, AkReal32 *& pfBuf, AkUInt32 uFramesInBlock )
{
	const AkReal32 fB0 = pfCoefs[ 0 ];
	const AkReal32 fB1 = pfCoefs[ 1 ];
	const AkReal32 fA1 = pfCoefs[ 2 ];
	const AkReal32 fA2 = pfCoefs[ 3 ];
	AkReal32 fXn1 = pFiltMem[ 0 ];
	AkReal32 fXn2 = pFiltMem[ 1 ];
	AkReal32 fYn1 = pFiltMem[ 2 ];
	AkReal32 fYn2 = pFiltMem[ 3 ];
	AkReal32 * pfEnd = pfBuf + uFramesInBlock;
	while( pfBuf < pfEnd )
	{
		const AkReal32 fIn = *pfBuf;
		const AkReal32 fOut = ( fA2 * fYn2 + ( ( fIn + fXn2 ) * fB0 + fB1 * fXn1 ) ) + fA1 * fYn1;
		*pfBuf++ = fOut;
		fXn2 = fXn1;
		fXn1 = fIn;
		fYn2 = fYn1;
		fYn1 = fOut;
	}
	pFiltMem[ 0 ] = fXn1;
	pFiltMem[ 1 ] = fXn2;
	pFiltMem[ 2 ] = fYn1;
	pFiltMem[ 3 ] = fYn2;
}

/** 2012 0x95ada0; 2013 0x957640. LPF parameter (0..100) to cutoff in Hz. */
AkReal32 EvalLPFCutoff( AkReal32 in_fCurrentLPFPar, bool in_bIsForFeedbackPipeline )
{
	if( in_bIsForFeedbackPipeline )
	{
		return 187.5f - in_fCurrentLPFPar * 1.865f;
	}
	if( in_fCurrentLPFPar >= 30.f )
	{
		return (AkReal32)( pow( 1.090000033378601, 100.0 - in_fCurrentLPFPar ) * 16.797443f );
	}
	return ( 30.f - in_fCurrentLPFPar ) * 433.33334f + 7000.f;
}

/** 2012 0x95ae20; 2013 0x9576c0. */
void ComputeLPFCoefs( AkReal32 in_fCutFreq, AkReal32 * out_pfFiltCoefs )
{
	AkReal32 fNormFreq = (AkReal32)( in_fCutFreq / (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency );
	if( fNormFreq > 0.44999999f )
	{
		fNormFreq = 0.44999999f;
	}
	const AkReal32 fIntVal = (AkReal32)( 1.0 / tan( fNormFreq * 3.1415927f ) );
	const AkReal32 fNorm = 1.f / ( fIntVal * fIntVal + fIntVal * 1.4142135f + 1.f );
	out_pfFiltCoefs[ 1 ] = fNorm * 2.f;
	out_pfFiltCoefs[ 0 ] = fNorm;
	out_pfFiltCoefs[ 2 ] = ( 1.f - fIntVal * fIntVal ) * fNorm * -2.f;
	out_pfFiltCoefs[ 3 ] = -( ( ( 1.f - fIntVal * 1.4142135f ) + fIntVal * fIntVal ) * fNorm );
}

namespace
{
	inline void StepLPFInterpolation( AkInternalLPFState * io_pLPFState, AkReal32 in_fLPFParamStart, AkReal32 in_fLPFParamDiff )
	{
		if( io_pLPFState->uNumInterBlocks < NUMBLOCKTOREACHTARGET )
		{
			++io_pLPFState->uNumInterBlocks;
			io_pLPFState->fCurrentLPFPar = (AkReal32)io_pLPFState->uNumInterBlocks * in_fLPFParamDiff * 0.125f + in_fLPFParamStart;
			ComputeLPFCoefs( EvalLPFCutoff( io_pLPFState->fCurrentLPFPar, io_pLPFState->bComputeCoefsForFeedback ), io_pLPFState->fFiltCoefs );
		}
	}

	inline void EndLPFInterpolation( AkInternalLPFState * io_pLPFState )
	{
		if( io_pLPFState->uNumInterBlocks >= NUMBLOCKTOREACHTARGET )
		{
			io_pLPFState->bBypassFilter = io_pLPFState->fTargetLPFPar <= BYPASSMAXVAL;
		}
	}
}

/** 2012 0x95af00; 2013 0x9577a0. */
void Perform1ChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	AkUInt32 uFramesLeft = io_pBuffer->uValidFrames;
	AkReal32 * pfBuf = (AkReal32 *)io_pBuffer->pData;
	const AkReal32 fLPFParamStart = io_pLPFState->fCurrentLPFPar;
	const AkReal32 fLPFParamDiff = io_pLPFState->fTargetLPFPar - fLPFParamStart;
	if( fLPFParamStart <= BYPASSMAXVAL )
	{
		if( uFramesLeft < 2 )
		{
			return;
		}
		io_pFiltMem[ 0 ] = pfBuf[ 1 ];
		io_pFiltMem[ 1 ] = pfBuf[ 0 ];
		io_pFiltMem[ 2 ] = pfBuf[ 1 ];
		io_pFiltMem[ 3 ] = pfBuf[ 0 ];
		pfBuf += 2;
		uFramesLeft -= 2;
	}
	while( uFramesLeft )
	{
		const AkUInt32 uFramesInBlock = AkAudioLibSettings::g_uLpfUpdatePeriod < uFramesLeft ? AkAudioLibSettings::g_uLpfUpdatePeriod : uFramesLeft;
		StepLPFInterpolation( io_pLPFState, fLPFParamStart, fLPFParamDiff );
		LPF_APPLY_CHANNEL( io_pFiltMem, io_pLPFState->fFiltCoefs, pfBuf, uFramesInBlock );
		uFramesLeft -= uFramesInBlock;
	}
	EndLPFInterpolation( io_pLPFState );
}

/** 2012 0x95b0a0; 2013 0x957940. */
void Perform2ChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	AkUInt32 uFramesLeft = io_pBuffer->uValidFrames;
	AkReal32 * pfBufL = (AkReal32 *)io_pBuffer->pData;
	AkReal32 * pfBufR = pfBufL + io_pBuffer->uMaxFrames;
	const AkReal32 fLPFParamStart = io_pLPFState->fCurrentLPFPar;
	const AkReal32 fLPFParamDiff = io_pLPFState->fTargetLPFPar - fLPFParamStart;
	if( fLPFParamStart <= BYPASSMAXVAL )
	{
		if( uFramesLeft < 2 )
		{
			return;
		}
		io_pFiltMem[ 0 ] = pfBufL[ 1 ];
		io_pFiltMem[ 1 ] = pfBufL[ 0 ];
		io_pFiltMem[ 2 ] = pfBufL[ 1 ];
		io_pFiltMem[ 3 ] = pfBufL[ 0 ];
		io_pFiltMem[ 4 ] = pfBufR[ 1 ];
		io_pFiltMem[ 5 ] = pfBufR[ 0 ];
		io_pFiltMem[ 6 ] = pfBufR[ 1 ];
		io_pFiltMem[ 7 ] = pfBufR[ 0 ];
		pfBufL += 2;
		pfBufR += 2;
		uFramesLeft -= 2;
	}
	while( uFramesLeft )
	{
		const AkUInt32 uFramesInBlock = AkAudioLibSettings::g_uLpfUpdatePeriod < uFramesLeft ? AkAudioLibSettings::g_uLpfUpdatePeriod : uFramesLeft;
		StepLPFInterpolation( io_pLPFState, fLPFParamStart, fLPFParamDiff );
		LPF_APPLY_CHANNEL( io_pFiltMem, io_pLPFState->fFiltCoefs, pfBufL, uFramesInBlock );
		LPF_APPLY_CHANNEL( io_pFiltMem + 4, io_pLPFState->fFiltCoefs, pfBufR, uFramesInBlock );
		uFramesLeft -= uFramesInBlock;
	}
	EndLPFInterpolation( io_pLPFState );
}

/** 2012 0x95b290; 2013 0x957b30. */
void Perform1Chan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	AkReal32 * pfBuf = (AkReal32 *)io_pBuffer->pData;
	LPF_APPLY_CHANNEL( io_pFiltMem, io_pLPFState->fFiltCoefs, pfBuf, io_pBuffer->uValidFrames );
}

/** 2012 0x95b2c0; 2013 0x957b60. */
void Perform2Chan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	AkReal32 * pfBufL = (AkReal32 *)io_pBuffer->pData;
	AkReal32 * pfBufR = pfBufL + io_pBuffer->uMaxFrames;
	LPF_APPLY_CHANNEL( io_pFiltMem, io_pLPFState->fFiltCoefs, pfBufL, io_pBuffer->uValidFrames );
	LPF_APPLY_CHANNEL( io_pFiltMem + 4, io_pLPFState->fFiltCoefs, pfBufR, io_pBuffer->uValidFrames );
}

/** 2012 0x95b310; 2013 0x957bb0. */
void PerformNChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pLPFState->uChannelMask );
	const AkReal32 fLPFParamStart = io_pLPFState->fCurrentLPFPar;
	const AkReal32 fLPFParamDiff = io_pLPFState->fTargetLPFPar - fLPFParamStart;
	AkUInt32 uFrameOffset = 0;
	if( fLPFParamStart <= BYPASSMAXVAL )
	{
		if( uNumFrames < 2 )
		{
			return;
		}
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			const AkReal32 * pfChannel = io_pBuffer->GetChannel( i );
			AkReal32 * pfMem = io_pFiltMem + 4 * i;
			pfMem[ 0 ] = pfChannel[ 1 ];
			pfMem[ 1 ] = pfChannel[ 0 ];
			pfMem[ 2 ] = pfChannel[ 1 ];
			pfMem[ 3 ] = pfChannel[ 0 ];
		}
		uFrameOffset = 2;
	}
	while( uFrameOffset < uNumFrames )
	{
		const AkUInt32 uFramesInBlock = AkAudioLibSettings::g_uLpfUpdatePeriod < uNumFrames - uFrameOffset ? AkAudioLibSettings::g_uLpfUpdatePeriod : uNumFrames - uFrameOffset;
		StepLPFInterpolation( io_pLPFState, fLPFParamStart, fLPFParamDiff );
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			AkReal32 * pfBuf = io_pBuffer->GetChannel( i ) + uFrameOffset;
			LPF_APPLY_CHANNEL( io_pFiltMem + 4 * i, io_pLPFState->fFiltCoefs, pfBuf, uFramesInBlock );
		}
		uFrameOffset += uFramesInBlock;
	}
	EndLPFInterpolation( io_pLPFState );
}

/** 2012 0x95b540; 2013 0x957de0. */
void PerformNChan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem )
{
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( io_pLPFState->uChannelMask );
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		AkReal32 * pfBuf = io_pBuffer->GetChannel( i );
		LPF_APPLY_CHANNEL( io_pFiltMem + 4 * i, io_pLPFState->fFiltCoefs, pfBuf, uNumFrames );
	}
}
