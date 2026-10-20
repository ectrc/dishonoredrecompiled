/*=============================================================================
	AkDelayFXParams.cpp - CAkDelayFXParams, Wwise 2012.1 (samples/plugins/akdelay/sources/audioenginefx/akdelayfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Bank parameter block (18 bytes):
	delay time s, feedback %, wet/dry %, output level dB, feedback enabled, process LFE. As retail, the wet/dry and output
	level RTPC paths do not raise bHasChanged (Process reads them every buffer), and an unknown id is AK_InvalidParameter.
=============================================================================*/
#include "AkDelayFXParams.h"
#include <math.h>

/** 2012 0x97c410, 2013 0x978c10. */
AK::IAkPluginParam * CreateDelayFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkDelayFXParams() );
}

/** Inlined into Clone 0x97c2f0. */
CAkDelayFXParams::CAkDelayFXParams( const CAkDelayFXParams & in_rCopy )
{
	RTPC = in_rCopy.RTPC;
	RTPC.bHasChanged = true;
	NonRTPC = in_rCopy.NonRTPC;
	NonRTPC.bHasChanged = true;
}

/** 2012 0x97c2f0, 2013 0x978af0. */
AK::IAkPluginParam * CAkDelayFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkDelayFXParams( *this ) );
}

/** 2012 0x97c190, 2013 0x978990. */
AKRESULT CAkDelayFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	NonRTPC.fDelayTime = 0.5f;
	RTPC.fFeedback = 0.f;
	RTPC.fWetDryMix = 0.5f;
	RTPC.fOutputLevel = (AkReal32)pow( 10.0, 0.0 );
	RTPC.bFeedbackEnabled = false;
	RTPC.bHasChanged = true;
	NonRTPC.bProcessLFE = true;
	NonRTPC.bHasChanged = true;
	return AK_Success;
}

/** 2012 0x97c340, 2013 0x978b40. */
AKRESULT CAkDelayFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x97c370, 2013 0x978b70. */
AKRESULT CAkDelayFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	NonRTPC.fDelayTime = *(const AkReal32 *)( pBlock + 0 );
	const AkReal32 fFeedback = *(const AkReal32 *)( pBlock + 4 );
	const AkReal32 fWetDryMix = *(const AkReal32 *)( pBlock + 8 );
	RTPC.fOutputLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 12 ) * 0.050000001f );
	RTPC.bFeedbackEnabled = *( pBlock + 16 ) != 0;
	NonRTPC.bProcessLFE = *( pBlock + 17 ) != 0;
	RTPC.fFeedback = fFeedback * ONEOVER_DELAYFXPARAM_PERCENT_MAX;
	RTPC.fWetDryMix = fWetDryMix * ONEOVER_DELAYFXPARAM_PERCENT_MAX;
	RTPC.bHasChanged = true;
	NonRTPC.bHasChanged = true;
	return AK_Success;
}

/** 2012 0x97c1f0, 2013 0x9789f0. */
AKRESULT CAkDelayFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	switch( in_ParamID )
	{
	case AK_DELAYFXPARAM_DELAYTIME_ID:
		NonRTPC.fDelayTime = fValue;
		NonRTPC.bHasChanged = true;
		break;
	case AK_DELAYFXPARAM_FEEDBACK_ID:
		RTPC.fFeedback = fValue * ONEOVER_DELAYFXPARAM_PERCENT_MAX;
		RTPC.bHasChanged = true;
		break;
	case AK_DELAYFXPARAM_WETDRYMIX_ID:
		RTPC.fWetDryMix = fValue * ONEOVER_DELAYFXPARAM_PERCENT_MAX;
		break;
	case AK_DELAYFXPARAM_OUTPUTLEVEL_ID:
		RTPC.fOutputLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		break;
	case AK_DELAYFXPARAM_FEEDBACKENABLED_ID:
		RTPC.bFeedbackEnabled = fValue != 0.f;
		RTPC.bHasChanged = true;
		break;
	case AK_DELAYFXPARAM_PROCESSLFE_ID:
		NonRTPC.bProcessLFE = *(const AkUInt8 *)in_pValue != 0;
		NonRTPC.bHasChanged = true;
		break;
	default:
		return AK_InvalidParameter;
	}
	return AK_Success;
}
