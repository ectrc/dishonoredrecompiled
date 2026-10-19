/*=============================================================================
	AkSrcLpFilter.cpp - port of aksrclpfilter.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26).
=============================================================================*/
#include "AkSrcLpFilter.h"
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <string.h>

/** 2012 0x942860; 2013 0x93f1a0. */
CAkSrcLpFilter::CAkSrcLpFilter()
	: m_pfFiltMem( NULL )
{
	m_InternalLPFState.fCurrentLPFPar = 0.f;
	m_InternalLPFState.fTargetLPFPar = 0.f;
}

/** 2012 0x942880; 2013 0x93f1c0. */
CAkSrcLpFilter::~CAkSrcLpFilter()
{
}

/** 2012 0x942890; 2013 0x93f1d0. */
void CAkSrcLpFilter::Term()
{
	if( m_pfFiltMem )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pfFiltMem );
		m_pfFiltMem = NULL;
	}
}

/** 2012 0x9428c0; 2013 0x93f200. */
void CAkSrcLpFilter::ResetRamp()
{
	m_InternalLPFState.bFirstSetLPF = true;
	if( m_pfFiltMem )
	{
		memset( m_pfFiltMem, 0, AkGetNumChannels( m_InternalLPFState.uChannelMask ) * 4 * sizeof( AkReal32 ) );
	}
}

/** 2012 0x9428f0 (_ManageLPFChange); 2013 0x93f230. */
void ManageLPFChange( AkInternalLPFState & io_state )
{
	if( !io_state.bTargetDirty )
	{
		return;
	}
	io_state.bTargetDirty = false;
	if( io_state.bFirstSetLPF )
	{
		io_state.bFirstSetLPF = false;
		io_state.fCurrentLPFPar = io_state.fTargetLPFPar;
		io_state.uNumInterBlocks = NUMBLOCKTOREACHTARGET;
		io_state.bBypassFilter = io_state.fTargetLPFPar <= BYPASSMAXVAL;
		if( io_state.fTargetLPFPar > BYPASSMAXVAL )
		{
			ComputeLPFCoefs( EvalLPFCutoff( io_state.fTargetLPFPar, io_state.bComputeCoefsForFeedback ), io_state.fFiltCoefs );
		}
	}
	else if( io_state.fCurrentLPFPar > BYPASSMAXVAL || io_state.fTargetLPFPar > BYPASSMAXVAL )
	{
		io_state.bBypassFilter = false;
		io_state.uNumInterBlocks = 0;
	}
	else
	{
		io_state.bBypassFilter = true;
		io_state.uNumInterBlocks = NUMBLOCKTOREACHTARGET;
	}
}

/** 2012 0x9429a0; 2013 0x93f2e0. */
void CAkSrcLpFilter::Execute( AkAudioBuffer * io_pBuffer )
{
	ManageLPFChange( m_InternalLPFState );
	if( !m_InternalLPFState.bBypassFilter )
	{
		LPFDSPFuncTable[ m_DSPFunctionIndex + ( m_InternalLPFState.uNumInterBlocks < NUMBLOCKTOREACHTARGET ? 3 : 0 ) ]( io_pBuffer, &m_InternalLPFState, m_pfFiltMem );
	}
}

/** 2012 0x942a00; 2013 0x93f340. */
AKRESULT CAkSrcLpFilter::Init( AkChannelMask in_uChannelMask, bool in_bComputeCoefsForFeedback )
{
	switch( AkGetNumChannels( in_uChannelMask ) )
	{
	case 1:
		m_DSPFunctionIndex = 0;
		break;
	case 2:
		m_DSPFunctionIndex = 1;
		break;
	default:
		m_DSPFunctionIndex = 2;
		break;
	}
	m_InternalLPFState.uChannelMask = (AkUInt16)in_uChannelMask;
	m_InternalLPFState.bComputeCoefsForFeedback = in_bComputeCoefsForFeedback;
	m_InternalLPFState.uNumInterBlocks = NUMBLOCKTOREACHTARGET;
	m_InternalLPFState.bBypassFilter = true;
	m_InternalLPFState.bTargetDirty = true;
	m_InternalLPFState.bFirstSetLPF = true;
	const AkUInt32 uMemSize = AkGetNumChannels( (AkUInt16)in_uChannelMask ) * 4 * sizeof( AkReal32 );
	m_pfFiltMem = (AkReal32 *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uMemSize );
	if( !m_pfFiltMem )
	{
		return AK_InsufficientMemory;
	}
	memset( m_pfFiltMem, 0, uMemSize );
	m_InternalLPFState.fFiltCoefs[ 0 ] = 1.f;
	m_InternalLPFState.fFiltCoefs[ 1 ] = 0.f;
	m_InternalLPFState.fFiltCoefs[ 2 ] = 0.f;
	m_InternalLPFState.fFiltCoefs[ 3 ] = 0.f;
	return AK_Success;
}
