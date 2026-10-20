/*=============================================================================
	AkTimeStretchFXParams.cpp - CAkTimeStretchFXParams, Wwise 2012.1 (plugins/effects/aktimestretch/aktimestretchfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Bank parameter block (16 bytes):
	window size, time stretch %, random stretch %, output gain dB. No bank of Dishonored uses this plug-in (the glue
	registers it).
=============================================================================*/
#include "AkTimeStretchFXParams.h"
#include <math.h>

/** 2012 0x9982d0, 2013 0x98f700. */
AK::IAkPluginParam * CreateTimeStretchFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTimeStretchFXParams() );
}

/** 2012 0x9982a0, 2013 0x98f6d0. */
CAkTimeStretchFXParams::CAkTimeStretchFXParams()
{
}

/** 2012 0x9980a0, 2013 0x98f4d0. */
CAkTimeStretchFXParams::~CAkTimeStretchFXParams()
{
}

/** 2012 0x998170, 2013 0x98f5a0. */
CAkTimeStretchFXParams::CAkTimeStretchFXParams( const CAkTimeStretchFXParams & in_rCopy )
{
	m_Params = in_rCopy.m_Params;
}

/** 2012 0x9981c0, 2013 0x98f5f0. */
AK::IAkPluginParam * CAkTimeStretchFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTimeStretchFXParams( *this ) );
}

/** 2012 0x9980b0, 2013 0x98f4e0. */
AKRESULT CAkTimeStretchFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	return AK_Success;
}

/** 2012 0x998220, 2013 0x98f650. */
AKRESULT CAkTimeStretchFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x998250, 2013 0x98f680. */
AKRESULT CAkTimeStretchFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	m_Params.uWindowSize = *(const AkUInt32 *)( pBlock + 0 );
	m_Params.fTimeStretch = *(const AkReal32 *)( pBlock + 4 );
	m_Params.fTimeStretchRandom = *(const AkReal32 *)( pBlock + 8 );
	m_Params.fOutputGain = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 12 ) * 0.050000001f );
	return AK_Success;
}

/** 2012 0x9980d0, 2013 0x98f500. */
AKRESULT CAkTimeStretchFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	if( !in_pValue )
	{
		return AK_InvalidParameter;
	}
	switch( in_ParamID )
	{
	case AK_TIMESTRETCHFXPARAM_WINDOWSIZE_ID:
		m_Params.uWindowSize = *(const AkUInt32 *)in_pValue;
		break;
	case AK_TIMESTRETCHFXPARAM_TIMESTRETCH_ID:
		m_Params.fTimeStretch = *(const AkReal32 *)in_pValue;
		break;
	case AK_TIMESTRETCHFXPARAM_OUTPUTGAIN_ID:
		m_Params.fOutputGain = (AkReal32)pow( 10.0, *(const AkReal32 *)in_pValue * 0.050000001f );
		break;
	case AK_TIMESTRETCHFXPARAM_TIMESTRETCHRANDOM_ID:
		m_Params.fTimeStretchRandom = *(const AkReal32 *)in_pValue;
		break;
	default:
		break;
	}
	return AK_Success;
}

/** 2012 0x998150, 2013 0x98f580. */
void CAkTimeStretchFXParams::GetParams( AkTimeStretchFXParams * out_pParams )
{
	*out_pParams = m_Params;
}
