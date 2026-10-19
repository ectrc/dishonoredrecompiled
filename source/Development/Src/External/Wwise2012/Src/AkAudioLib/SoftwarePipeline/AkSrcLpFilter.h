/*=============================================================================
	AkSrcLpFilter.h - CAkSrcLpFilter (aksrclpfilter.cpp): the per-voice low-pass filter CAkVPLLPFNode and the source
	combiner's obstruction LPF run; SetLPFPar is the inline retail's CAkVPLSrcCbxNode::GetVolumes expands
	(clamp 0..100, mark dirty when the target changes).

	DISHONORED(port): agent IH2 (wave 26). 44 bytes as the 2012 PDB: vptr (virtual dtor only), AkInternalLPFState,
	m_pfFiltMem (4 floats per channel in g_LEngineDefaultPoolId), m_DSPFunctionIndex (0 mono, 1 stereo, 2 N channels).
=============================================================================*/
#pragma once

#include "AkLPFCommon.h"

class CAkSrcLpFilter
{
public:
	CAkSrcLpFilter();
	virtual ~CAkSrcLpFilter();

	AKRESULT Init( AkChannelMask in_uChannelMask, bool in_bComputeCoefsForFeedback );
	void Term();
	void ResetRamp();
	void Execute( AkAudioBuffer * io_pBuffer );

	void SetLPFPar( AkReal32 in_fTargetLPFPar )
	{
		if( in_fTargetLPFPar < 0.f )
		{
			in_fTargetLPFPar = 0.f;
		}
		else if( in_fTargetLPFPar > 100.f )
		{
			in_fTargetLPFPar = 100.f;
		}
		m_InternalLPFState.bTargetDirty |= in_fTargetLPFPar != m_InternalLPFState.fTargetLPFPar;
		m_InternalLPFState.fTargetLPFPar = in_fTargetLPFPar;
	}
	AkReal32 GetLPFPar() const { return m_InternalLPFState.fTargetLPFPar; }
	bool IsInitialized() const { return m_pfFiltMem != NULL; }

	AkInternalLPFState	m_InternalLPFState;
	AkReal32 *			m_pfFiltMem;
	AkUInt8				m_DSPFunctionIndex;
};
static_assert( sizeof( CAkSrcLpFilter ) == 44, "CAkSrcLpFilter: 2012 PDB sizeof 44" );

void ManageLPFChange( AkInternalLPFState & io_state );
