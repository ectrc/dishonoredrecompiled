/*=============================================================================
	AkHarmonizerFXParams.cpp - CAkHarmonizerFXParams, Wwise 2012.1 (plugins/effects/akharmonizer/akharmonizerfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (no 2013 twins: the 2013 exe does not link this plug-in).
	Bank parameter block (packed): per voice enable (u8), pitch cents, gain dB, filter type, filter gain, frequency, Q
	(25 bytes); then input type, dry dB, wet dB, window size, process LFE (u8), sync dry (u8). As retail, every SetParam
	marks its id in the change handler, even an unknown id.
=============================================================================*/
#include "AkHarmonizerFXParams.h"
#include <math.h>
#include <string.h>

/** 2012 0x990cf0. */
AK::IAkPluginParam * CreateHarmonizerFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkHarmonizerFXParams() );
}

/** 2012 0x990ba0. */
CAkHarmonizerFXParams::CAkHarmonizerFXParams()
{
	m_ParamChangeHandler.ResetAllParamChanges();
}

/** 2012 0x990820. */
CAkHarmonizerFXParams::~CAkHarmonizerFXParams()
{
}

/** 2012 0x990c20. */
CAkHarmonizerFXParams::CAkHarmonizerFXParams( const CAkHarmonizerFXParams & in_rCopy )
{
	m_ParamChangeHandler.ResetAllParamChanges();
	m_Params = in_rCopy.m_Params;
	m_ParamChangeHandler.SetAllParamChanges();
}

/** 2012 0x990cc0. */
AK::IAkPluginParam * CAkHarmonizerFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkHarmonizerFXParams( *this ) );
}

/** 2012 0x990830. */
AKRESULT CAkHarmonizerFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	return AK_Success;
}

/** 2012 0x990870. */
AKRESULT CAkHarmonizerFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x9908a0. */
AKRESULT CAkHarmonizerFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	AkReal32 fValue;
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		AkHarmonizerVoiceParams & rVoice = m_Params.Voice[ i ];
		rVoice.bEnable = pBlock[ 0 ] != 0;
		memcpy( &fValue, pBlock + 1, sizeof( fValue ) );
		rVoice.fPitchFactor = (AkReal32)pow( 2.0, fValue * 0.00083333335f );
		memcpy( &fValue, pBlock + 5, sizeof( fValue ) );
		rVoice.fGain = (AkReal32)pow( 10.0, fValue * 0.050000001f );
		memcpy( &rVoice.Filter.eFilterType, pBlock + 9, sizeof( AkUInt32 ) );
		memcpy( &rVoice.Filter.fFilterGain, pBlock + 13, sizeof( AkReal32 ) );
		memcpy( &rVoice.Filter.fFilterFrequency, pBlock + 17, sizeof( AkReal32 ) );
		memcpy( &rVoice.Filter.fFilterQFactor, pBlock + 21, sizeof( AkReal32 ) );
		pBlock += 25;
	}
	AkUInt32 uValue;
	memcpy( &uValue, pBlock, sizeof( uValue ) );
	m_Params.eInputType = (AkInputType)uValue;
	memcpy( &fValue, pBlock + 4, sizeof( fValue ) );
	m_Params.fDryLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
	memcpy( &fValue, pBlock + 8, sizeof( fValue ) );
	m_Params.fWetLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f );
	memcpy( &m_Params.uWindowSize, pBlock + 12, sizeof( AkUInt32 ) );
	m_Params.bProcessLFE = pBlock[ 16 ] != 0;
	m_Params.bSyncDry = pBlock[ 17 ] != 0;
	m_ParamChangeHandler.SetAllParamChanges();
	return AK_Success;
}

/** 2012 0x990990. */
AKRESULT CAkHarmonizerFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	if( !in_pValue )
	{
		return AK_InvalidParameter;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	const bool bValue = *(const AkUInt8 *)in_pValue != 0;
	switch( in_ParamID )
	{
	case AKHARMONIZERPARAMID_INPUT:					m_Params.eInputType = (AkInputType)*(const AkUInt32 *)in_pValue; break;
	case AKHARMONIZERPARAMID_PROCESSLFE:			m_Params.bProcessLFE = bValue; break;
	case AKHARMONIZERPARAMID_SYNCDRY:				m_Params.bSyncDry = bValue; break;
	case AKHARMONIZERPARAMID_DRYLEVEL:				m_Params.fDryLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKHARMONIZERPARAMID_WETLEVEL:				m_Params.fWetLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKHARMONIZERPARAMID_WINDOWSIZE:			m_Params.uWindowSize = *(const AkUInt32 *)in_pValue; break;
	case AKHARMONIZERPARAMID_VOICE1ENABLE:			m_Params.Voice[ 0 ].bEnable = bValue; break;
	case AKHARMONIZERPARAMID_VOICE1PITCH:			m_Params.Voice[ 0 ].fPitchFactor = (AkReal32)pow( 2.0, fValue * 0.00083333335f ); break;
	case AKHARMONIZERPARAMID_VOICE1GAIN:			m_Params.Voice[ 0 ].fGain = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKHARMONIZERPARAMID_VOICE1FILTERTYPE:		m_Params.Voice[ 0 ].Filter.eFilterType = (AkUInt32)(AkInt64)fValue; break;
	case AKHARMONIZERPARAMID_VOICE1FILTERGAIN:		m_Params.Voice[ 0 ].Filter.fFilterGain = fValue; break;
	case AKHARMONIZERPARAMID_VOICE1FILTERFREQUENCY:	m_Params.Voice[ 0 ].Filter.fFilterFrequency = fValue; break;
	case AKHARMONIZERPARAMID_VOICE1FILTERQFACTOR:	m_Params.Voice[ 0 ].Filter.fFilterQFactor = fValue; break;
	case AKHARMONIZERPARAMID_VOICE2ENABLE:			m_Params.Voice[ 1 ].bEnable = bValue; break;
	case AKHARMONIZERPARAMID_VOICE2PITCH:			m_Params.Voice[ 1 ].fPitchFactor = (AkReal32)pow( 2.0, fValue * 0.00083333335f ); break;
	case AKHARMONIZERPARAMID_VOICE2GAIN:			m_Params.Voice[ 1 ].fGain = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKHARMONIZERPARAMID_VOICE2FILTERTYPE:		m_Params.Voice[ 1 ].Filter.eFilterType = (AkUInt32)(AkInt64)fValue; break;
	case AKHARMONIZERPARAMID_VOICE2FILTERGAIN:		m_Params.Voice[ 1 ].Filter.fFilterGain = fValue; break;
	case AKHARMONIZERPARAMID_VOICE2FILTERFREQUENCY:	m_Params.Voice[ 1 ].Filter.fFilterFrequency = fValue; break;
	case AKHARMONIZERPARAMID_VOICE2FILTERQFACTOR:	m_Params.Voice[ 1 ].Filter.fFilterQFactor = fValue; break;
	default:
		break;
	}
	m_ParamChangeHandler.SetParamChange( in_ParamID );
	return AK_Success;
}

/** 2012 0x990850. */
void CAkHarmonizerFXParams::GetParams( AkHarmonizerFXParams * out_pParams )
{
	*out_pParams = m_Params;
}
