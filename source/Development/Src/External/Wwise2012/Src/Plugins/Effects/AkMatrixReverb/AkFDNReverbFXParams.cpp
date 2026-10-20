/*=============================================================================
	AkFDNReverbFXParams.cpp - CAkFDNReverbFXParams, Wwise 2012.1 (plugins/effects/akfdnreverb/akfdnreverbfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Bank parameter block: reverb time
	s, HF ratio, number of delays (u32), dry dB, wet dB, pre-delay s, process LFE (u8), delay lengths mode (u32, packed
	at offset 25) and, in mode 1, one delay time (ms) per delay. Kept as retail: SetParam id 7 stores the float bits of
	the value into the integer mode field, and ids >= 8 write the delay times by offset from fReverbTime.
=============================================================================*/
#include "AkFDNReverbFXParams.h"
#include <math.h>
#include <string.h>

/** 2012 0x988680, 2013 0x984e90. */
AK::IAkPluginParam * CreateMatrixReverbFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFDNReverbFXParams() );
}

/** 2012 0x988670, 2013 0x984e80. */
CAkFDNReverbFXParams::CAkFDNReverbFXParams()
{
}

/** 2012 0x988350, 2013 0x984b60. */
CAkFDNReverbFXParams::~CAkFDNReverbFXParams()
{
}

/** 2012 0x988500, 2013 0x984d10. */
CAkFDNReverbFXParams::CAkFDNReverbFXParams( const CAkFDNReverbFXParams & in_rCopy )
{
	RTPC = in_rCopy.RTPC;
	NonRTPC = in_rCopy.NonRTPC;
	NonRTPC.bDirty = true;
}

/** 2012 0x988540, 2013 0x984d50. */
AK::IAkPluginParam * CAkFDNReverbFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFDNReverbFXParams( *this ) );
}

/** 2012 0x988360, 2013 0x984b70. */
AKRESULT CAkFDNReverbFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	RTPC.fReverbTime = 4.f;
	RTPC.fHFRatio = 2.f;
	RTPC.fDryLevel = (AkReal32)pow( 10.0, -0.1500000059604645 );
	RTPC.fWetLevel = (AkReal32)pow( 10.0, -0.5 );
	NonRTPC.uNumberOfDelays = 8;
	NonRTPC.fPreDelay = 0.f;
	NonRTPC.uProcessLFE = 1;
	NonRTPC.uDelayLengthsMode = 0;
	NonRTPC.bDirty = true;
	return AK_Success;
}

/** 2012 0x988590, 2013 0x984da0. */
AKRESULT CAkFDNReverbFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x9885c0, 2013 0x984dd0. */
AKRESULT CAkFDNReverbFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	RTPC.fReverbTime = *(const AkReal32 *)( pBlock + 0 );
	RTPC.fHFRatio = *(const AkReal32 *)( pBlock + 4 );
	const AkUInt32 uNumberOfDelays = *(const AkUInt32 *)( pBlock + 8 );
	NonRTPC.uNumberOfDelays = uNumberOfDelays;
	RTPC.fDryLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 12 ) * 0.050000001f );
	RTPC.fWetLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 16 ) * 0.050000001f );
	NonRTPC.fPreDelay = *(const AkReal32 *)( pBlock + 20 );
	NonRTPC.uProcessLFE = pBlock[ 24 ] != 0;
	AkUInt32 uMode;
	memcpy( &uMode, pBlock + 25, sizeof( uMode ) );
	NonRTPC.uDelayLengthsMode = uMode;
	if( uMode == 1 )
	{
		const AkUInt8 * pDelay = pBlock + 29;
		for( AkUInt32 i = 0; i < NonRTPC.uNumberOfDelays; ++i )
		{
			memcpy( &NonRTPC.fDelayTime[ i ], pDelay, sizeof( AkReal32 ) );
			pDelay += sizeof( AkReal32 );
		}
	}
	NonRTPC.bDirty = true;
	return AK_Success;
}

/** 2012 0x9883f0, 2013 0x984c00. */
AKRESULT CAkFDNReverbFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	switch( in_ParamID )
	{
	case AK_FDNREVERBFXPARAM_REVERBTIME_ID:
		RTPC.fReverbTime = fValue;
		break;
	case AK_FDNREVERBFXPARAM_HFRATIO_ID:
		RTPC.fHFRatio = fValue;
		break;
	case AK_FDNREVERBFXPARAM_NUMBEROFDELAYS_ID:
		NonRTPC.uNumberOfDelays = *(const AkUInt32 *)in_pValue;
		NonRTPC.bDirty = true;
		break;
	case AK_FDNREVERBFXPARAM_DRYLEVEL_ID:
		RTPC.fDryLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		break;
	case AK_FDNREVERBFXPARAM_WETLEVEL_ID:
		RTPC.fWetLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		break;
	case AK_FDNREVERBFXPARAM_PREDELAY_ID:
		NonRTPC.bDirty = true;
		NonRTPC.fPreDelay = fValue;
		break;
	case AK_FDNREVERBFXPARAM_PROCESSLFE_ID:
		NonRTPC.bDirty = true;
		NonRTPC.uProcessLFE = *(const AkUInt8 *)in_pValue != 0;
		break;
	case AK_FDNREVERBFXPARAM_DELAYLENGTHSMODE_ID:
		NonRTPC.bDirty = true;
		memcpy( &NonRTPC.uDelayLengthsMode, &fValue, sizeof( AkReal32 ) );
		break;
	default:
		( &RTPC.fReverbTime )[ in_ParamID ] = fValue;
		NonRTPC.bDirty = true;
		break;
	}
	return AK_Success;
}
