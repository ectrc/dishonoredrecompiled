/*=============================================================================
	AkLPFCommon.h - the voice low-pass filter's state and DSP routines (aklpfcommon.h / aklpfcommon.cpp): a 2nd-order
	Butterworth biquad per channel whose cutoff follows the Wwise LPF parameter (0..100), interpolated over 8 blocks of
	AkAudioLibSettings::g_uLpfUpdatePeriod frames when it changes.

	DISHONORED(port): agent IH2 (wave 26). AkInternalLPFState is the 2012 PDB's (28 bytes). Filter memory is 4 floats per
	channel: x[n-1], x[n-2], y[n-1], y[n-2]. LPFDSPFuncTable (2012 .data, indexed by CAkSrcLpFilter::m_DSPFunctionIndex,
	+3 while interpolating) is Perform1Chan, Perform2Chan, PerformNChan, Perform1ChanInterp, Perform2ChanInterp,
	PerformNChanInterp.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

namespace AkAudioLibSettings
{
	extern AkUInt32 g_pipelineCoreFrequency;
	extern AkUInt32 g_uLpfUpdatePeriod;
}

#define NUMBLOCKTOREACHTARGET	8
#define BYPASSMAXVAL			0.1f

struct AkInternalLPFState
{
	AkReal32	fFiltCoefs[ 4 ];
	AkReal32	fCurrentLPFPar;
	AkReal32	fTargetLPFPar;
	AkUInt16	uNumInterBlocks;
	AkUInt16	uChannelMask;
	bool		bComputeCoefsForFeedback;
	bool		bBypassFilter;
	bool		bTargetDirty;
	bool		bFirstSetLPF;
};
static_assert( sizeof( AkInternalLPFState ) == 32, "AkInternalLPFState: 2012 PDB sizeof 32" );

typedef void ( *LPFDSPFuncPtr )( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );

extern LPFDSPFuncPtr LPFDSPFuncTable[ 6 ];

void LPF_APPLY_CHANNEL( AkReal32 * pFiltMem, AkReal32 * pfCoefs, AkReal32 *& pfBuf, AkUInt32 uFramesInBlock );
AkReal32 EvalLPFCutoff( AkReal32 in_fCurrentLPFPar, bool in_bIsForFeedbackPipeline );
void ComputeLPFCoefs( AkReal32 in_fCutFreq, AkReal32 * out_pfFiltCoefs );
void Perform1ChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
void Perform2ChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
void Perform1Chan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
void Perform2Chan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
void PerformNChanInterp( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
void PerformNChan( AkAudioBuffer * io_pBuffer, AkInternalLPFState * io_pLPFState, AkReal32 * io_pFiltMem );
