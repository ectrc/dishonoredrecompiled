/*=============================================================================
	AkTremoloFXParams.cpp - CAkTremoloFXParams, Wwise 2012.1 (plugins/effects/aktremolo/aktremolofxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Bank parameter block (38 bytes):
	depth %, frequency Hz, waveform, smoothing %, PWM %, phase offset (deg), phase mode, phase spread (deg), output
	level dB, process center, process LFE. Percentages are stored / 100, the output level as a linear gain.
	The phase offset / mode / spread RTPC paths do not set bHasChanged (retail: they only take effect at the next
	LFO setup).
=============================================================================*/
#include "AkTremoloFXParams.h"
#include <math.h>

/** 2012 0x99b570, 2013 0x994270. */
AK::IAkPluginParam * CreateTremoloFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTremoloFXParams() );
}

/** 2012 0x99b560, 2013 0x994260. */
CAkTremoloFXParams::CAkTremoloFXParams()
{
}

/** 2012 0x99b190, 2013 0x993e90. */
CAkTremoloFXParams::~CAkTremoloFXParams()
{
}

/** 2012 0x99b3d0, 2013 0x9940d0. */
CAkTremoloFXParams::CAkTremoloFXParams( const CAkTremoloFXParams & in_rCopy )
{
	m_Params = in_rCopy.m_Params;
	m_Params.NonRTPC.bHasChanged = true;
	m_Params.RTPC.bHasChanged = true;
}

/** 2012 0x99b400, 2013 0x994100. */
AK::IAkPluginParam * CAkTremoloFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTremoloFXParams( *this ) );
}

/** 2012 0x99b1b0, 2013 0x993eb0. */
AKRESULT CAkTremoloFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	m_Params.RTPC.fModDepth = AK_TREMOLOFXPARAM_MODDEPTH_DEF;
	m_Params.RTPC.modParams.lfoParams.eWaveform = DSP::LFO::WAVEFORM_SINE;
	m_Params.RTPC.modParams.lfoParams.fFrequency = AK_TREMOLOFXPARAM_MODFREQUENCY_DEF;
	m_Params.RTPC.modParams.lfoParams.fSmooth = AK_TREMOLOFXPARAM_MODSMOOTH_DEF;
	m_Params.RTPC.modParams.lfoParams.fPWM = AK_TREMOLOFXPARAM_MODPWM_DEF;
	m_Params.RTPC.modParams.phaseParams.fPhaseOffset = AK_TREMOLOFXPARAM_MODPHASEOFFSET_DEF;
	m_Params.RTPC.modParams.phaseParams.ePhaseMode = DSP::LFO::MultiChannel::PHASE_MODE_LEFT_RIGHT;
	m_Params.RTPC.modParams.phaseParams.fPhaseSpread = AK_TREMOLOFXPARAM_MODPHASESPREAD_DEF;
	m_Params.RTPC.fOutputGain = AK_TREMOLOFXPARAM_OUTPUTLEVEL_DEF;
	m_Params.NonRTPC.bProcessCenter = true;
	m_Params.NonRTPC.bProcessLFE = true;
	SetDirty( true );
	return AK_Success;
}

/** 2012 0x99b440, 2013 0x994140. */
AKRESULT CAkTremoloFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x99b470, 2013 0x994170. */
AKRESULT CAkTremoloFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	const AkReal32 fModDepth = *(const AkReal32 *)( pBlock + 0 );
	m_Params.RTPC.modParams.lfoParams.fFrequency = *(const AkReal32 *)( pBlock + 4 );
	m_Params.RTPC.modParams.lfoParams.eWaveform = (DSP::LFO::Waveform)*(const AkUInt32 *)( pBlock + 8 );
	const AkReal32 fSmooth = *(const AkReal32 *)( pBlock + 12 );
	const AkReal32 fPWM = *(const AkReal32 *)( pBlock + 16 );
	m_Params.RTPC.modParams.phaseParams.fPhaseOffset = *(const AkReal32 *)( pBlock + 20 );
	m_Params.RTPC.modParams.phaseParams.ePhaseMode = (DSP::LFO::MultiChannel::PhaseMode)*(const AkUInt32 *)( pBlock + 24 );
	m_Params.RTPC.modParams.phaseParams.fPhaseSpread = *(const AkReal32 *)( pBlock + 28 );
	m_Params.RTPC.fOutputGain = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 32 ) * 0.050000001f );
	m_Params.NonRTPC.bProcessCenter = *( pBlock + 36 ) != 0;
	m_Params.NonRTPC.bProcessLFE = *( pBlock + 37 ) != 0;
	m_Params.RTPC.fModDepth = fModDepth * 0.0099999998f;
	m_Params.RTPC.modParams.lfoParams.fSmooth = fSmooth * 0.0099999998f;
	m_Params.RTPC.modParams.lfoParams.fPWM = fPWM * 0.0099999998f;
	SetDirty( true );
	return AK_Success;
}

/** 2012 0x99b220, 2013 0x993f20. */
AKRESULT CAkTremoloFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	if( !in_pValue )
	{
		return AK_InvalidParameter;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	switch( in_ParamID )
	{
	case AK_TREMOLOFXPARAM_MODDEPTH_ID:
		m_Params.RTPC.fModDepth = fValue * 0.0099999998f;
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_MODFREQUENCY_ID:
		m_Params.RTPC.modParams.lfoParams.fFrequency = fValue;
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_MODWAVEFORM_ID:
		m_Params.RTPC.modParams.lfoParams.eWaveform = (DSP::LFO::Waveform)(AkInt64)fValue;
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_MODSMOOTH_ID:
		m_Params.RTPC.modParams.lfoParams.fSmooth = fValue * 0.0099999998f;
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_MODPWM_ID:
		m_Params.RTPC.modParams.lfoParams.fPWM = fValue * 0.0099999998f;
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_MODPHASEOFFSET_ID:
		m_Params.RTPC.modParams.phaseParams.fPhaseOffset = fValue;
		break;
	case AK_TREMOLOFXPARAM_MODPHASEMODE_ID:
		m_Params.RTPC.modParams.phaseParams.ePhaseMode = (DSP::LFO::MultiChannel::PhaseMode)(AkInt64)fValue;
		break;
	case AK_TREMOLOFXPARAM_MODPHASESPREAD_ID:
		m_Params.RTPC.modParams.phaseParams.fPhaseSpread = fValue;
		break;
	case AK_TREMOLOFXPARAM_OUTPUTLEVEL_ID:
		m_Params.RTPC.fOutputGain = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		m_Params.RTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_PROCESSCENTER_ID:
		m_Params.NonRTPC.bProcessCenter = *(const AkUInt8 *)in_pValue != 0;
		m_Params.NonRTPC.bHasChanged = true;
		break;
	case AK_TREMOLOFXPARAM_PROCESSLFE_ID:
		m_Params.NonRTPC.bProcessLFE = *(const AkUInt8 *)in_pValue != 0;
		m_Params.NonRTPC.bHasChanged = true;
		break;
	default:
		break;
	}
	return AK_Success;
}

/** 2012 0x99b1a0, 2013 0x993ea0. */
void CAkTremoloFXParams::SetDirty( bool in_bDirty )
{
	m_Params.NonRTPC.bHasChanged = in_bDirty;
	m_Params.RTPC.bHasChanged = in_bDirty;
}
