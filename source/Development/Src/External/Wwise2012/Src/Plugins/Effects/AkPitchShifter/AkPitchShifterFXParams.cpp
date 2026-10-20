/*=============================================================================
	AkPitchShifterFXParams.cpp - CAkPitchShifterFXParams, Wwise 2012.1 (plugins/effects/akpitchshifter/akpitchshifterfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Bank parameter block (38 bytes,
	packed): input type, dry dB, wet dB, delay time ms, process LFE (u8), sync dry (u8), pitch cents, filter type,
	filter gain, filter frequency, filter Q. The pitch is stored as the factor 2^(cents / 1200). Every SetParam marks its
	id in the change handler (even an unknown id, as retail); a block marks them all.
=============================================================================*/
#include "AkPitchShifterFXParams.h"
#include <math.h>
#include <string.h>

/** 2012 0x996d60, 2013 0x98d1f0. */
AK::IAkPluginParam * CreatePitchShifterFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkPitchShifterFXParams() );
}

/** 2012 0x996bf0, 2013 0x98d080. */
CAkPitchShifterFXParams::CAkPitchShifterFXParams()
{
	m_ParamChangeHandler.ResetAllParamChanges();
}

/** 2012 0x996980, 2013 0x98ce10. */
CAkPitchShifterFXParams::~CAkPitchShifterFXParams()
{
}

/** 2012 0x996c50, 2013 0x98d0e0. */
CAkPitchShifterFXParams::CAkPitchShifterFXParams( const CAkPitchShifterFXParams & in_rCopy )
{
	m_ParamChangeHandler.ResetAllParamChanges();
	m_Params = in_rCopy.m_Params;
	m_ParamChangeHandler.SetAllParamChanges();
}

/** 2012 0x996cd0, 2013 0x98d160. */
AK::IAkPluginParam * CAkPitchShifterFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkPitchShifterFXParams( *this ) );
}

/** 2012 0x996990, 2013 0x98ce20. */
AKRESULT CAkPitchShifterFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	return AK_Success;
}

/** 2012 0x9969d0, 2013 0x98ce60. */
AKRESULT CAkPitchShifterFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x996a00, 2013 0x98ce90. */
AKRESULT CAkPitchShifterFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	AkReal32 fValue;
	m_Params.eInputType = (AkInputType)*(const AkUInt32 *)( pBlock + 0 );
	m_Params.fDryLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 4 ) * 0.050000001f );
	m_Params.fWetLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 8 ) * 0.050000001f );
	m_Params.fDelayTime = *(const AkReal32 *)( pBlock + 12 );
	m_Params.bProcessLFE = pBlock[ 16 ] != 0;
	m_Params.bSyncDry = pBlock[ 17 ] != 0;
	memcpy( &fValue, pBlock + 18, sizeof( fValue ) );
	m_Params.Voice.fPitchFactor = (AkReal32)pow( 2.0, fValue * 0.00083333335f );
	memcpy( &m_Params.Voice.Filter.eFilterType, pBlock + 22, sizeof( AkUInt32 ) );
	memcpy( &m_Params.Voice.Filter.fFilterGain, pBlock + 26, sizeof( AkReal32 ) );
	memcpy( &m_Params.Voice.Filter.fFilterFrequency, pBlock + 30, sizeof( AkReal32 ) );
	memcpy( &m_Params.Voice.Filter.fFilterQFactor, pBlock + 34, sizeof( AkReal32 ) );
	m_ParamChangeHandler.SetAllParamChanges();
	return AK_Success;
}

/** 2012 0x996ab0, 2013 0x98cf40. */
AKRESULT CAkPitchShifterFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	if( !in_pValue )
	{
		return AK_InvalidParameter;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	switch( in_ParamID )
	{
	case AKPITCHSHIFTERPARAMID_INPUT:
		m_Params.eInputType = (AkInputType)*(const AkUInt32 *)in_pValue;
		break;
	case AKPITCHSHIFTERPARAMID_PROCESSLFE:
		m_Params.bProcessLFE = *(const AkUInt8 *)in_pValue != 0;
		break;
	case AKPITCHSHIFTERPARAMID_SYNCDRY:
		m_Params.bSyncDry = *(const AkUInt8 *)in_pValue != 0;
		break;
	case AKPITCHSHIFTERPARAMID_DRYLEVEL:
		m_Params.fDryLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		break;
	case AKPITCHSHIFTERPARAMID_WETLEVEL:
		m_Params.fWetLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		break;
	case AKPITCHSHIFTERPARAMID_DELAYTIME:
		m_Params.fDelayTime = fValue;
		break;
	case AKPITCHSHIFTERPARAMID_PITCH:
		m_Params.Voice.fPitchFactor = (AkReal32)pow( 2.0, fValue * 0.00083333335f );
		break;
	case AKPITCHSHIFTERPARAMID_FILTERTYPE:
		m_Params.Voice.Filter.eFilterType = (AkUInt32)(AkInt64)fValue;
		break;
	case AKPITCHSHIFTERPARAMID_FILTERGAIN:
		m_Params.Voice.Filter.fFilterGain = fValue;
		break;
	case AKPITCHSHIFTERPARAMID_FILTERFREQUENCY:
		m_Params.Voice.Filter.fFilterFrequency = fValue;
		break;
	case AKPITCHSHIFTERPARAMID_FILTERQFACTOR:
		m_Params.Voice.Filter.fFilterQFactor = fValue;
		break;
	default:
		break;
	}
	m_ParamChangeHandler.SetParamChange( in_ParamID );
	return AK_Success;
}

/** 2012 0x9969b0, 2013 0x98ce40. */
void CAkPitchShifterFXParams::GetParams( AkPitchShifterFXParams * out_pParams )
{
	*out_pParams = m_Params;
}
