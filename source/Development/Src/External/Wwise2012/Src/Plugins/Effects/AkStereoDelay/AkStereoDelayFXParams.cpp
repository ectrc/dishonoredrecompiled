/*=============================================================================
	AkStereoDelayFXParams.cpp - CAkStereoDelayFXParams, Wwise 2012.1 (plugins/effects/akstereodelay/akstereodelayfxparams.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (no 2013 twins: the 2013 exe does not link this plug-in).
	Bank parameter block (62 bytes): per side (left, right) input type, delay time s, feedback dB, cross-feed dB; the
	feedback filter (type, gain, frequency, Q, copied raw); dry dB, wet dB, front/rear balance, enable feedback, enable
	cross-feed. Levels are stored linear.
=============================================================================*/
#include "AkStereoDelayFXParams.h"
#include <math.h>
#include <string.h>

/** 2012 0x998f60. */
AK::IAkPluginParam * CreateStereoDelayFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkStereoDelayFXParams() );
}

/** 2012 0x998e00. */
CAkStereoDelayFXParams::CAkStereoDelayFXParams()
{
	m_ParamChangeHandler.ResetAllParamChanges();
}

/** 2012 0x998a00. */
CAkStereoDelayFXParams::~CAkStereoDelayFXParams()
{
}

/** 2012 0x998e80. */
CAkStereoDelayFXParams::CAkStereoDelayFXParams( const CAkStereoDelayFXParams & in_rCopy )
{
	m_ParamChangeHandler.ResetAllParamChanges();
	m_Params = in_rCopy.m_Params;
	m_ParamChangeHandler.SetAllParamChanges();
}

/** 2012 0x998f30. */
AK::IAkPluginParam * CAkStereoDelayFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkStereoDelayFXParams( *this ) );
}

/** 2012 0x998a10. */
AKRESULT CAkStereoDelayFXParams::Init( AK::IAkPluginMemAlloc * /*in_pAllocator*/, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize )
{
	if( in_ulBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
	}
	return AK_Success;
}

/** 2012 0x998a50. */
AKRESULT CAkStereoDelayFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x998a80. */
AKRESULT CAkStereoDelayFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 /*in_ulBlockSize*/ )
{
	const AkUInt8 * pBlock = (const AkUInt8 *)in_pParamsBlock;
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		const AkUInt8 * pSide = pBlock + 16 * i;
		m_Params.eInputType[ i ] = (AkInputChannelType)*(const AkUInt32 *)( pSide + 0 );
		m_Params.StereoDelayParams[ i ].fDelayTime = *(const AkReal32 *)( pSide + 4 );
		m_Params.StereoDelayParams[ i ].fFeedback = (AkReal32)pow( 10.0, *(const AkReal32 *)( pSide + 8 ) * 0.050000001f );
		m_Params.StereoDelayParams[ i ].fCrossFeed = (AkReal32)pow( 10.0, *(const AkReal32 *)( pSide + 12 ) * 0.050000001f );
	}
	memcpy( &m_Params.FilterParams, pBlock + 32, sizeof( m_Params.FilterParams ) );
	m_Params.fDryLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 48 ) * 0.050000001f );
	m_Params.fWetLevel = (AkReal32)pow( 10.0, *(const AkReal32 *)( pBlock + 52 ) * 0.050000001f );
	m_Params.fFrontRearBalance = *(const AkReal32 *)( pBlock + 56 );
	m_Params.bEnableFeedback = pBlock[ 60 ] != 0;
	m_Params.bEnableCrossFeed = pBlock[ 61 ] != 0;
	m_ParamChangeHandler.SetAllParamChanges();
	return AK_Success;
}

/** 2012 0x998bb0. */
AKRESULT CAkStereoDelayFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 /*in_ulParamSize*/ )
{
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	switch( in_ParamID )
	{
	case AKSTEREODELAYPARAMID_ENABLECROSSFEED:		m_Params.bEnableCrossFeed = fValue != 0.f; break;
	case AKSTEREODELAYPARAMID_ENABLEFEEDBACK:		m_Params.bEnableFeedback = fValue != 0.f; break;
	case AKSTEREODELAYPARAMID_DRYLEVEL:				m_Params.fDryLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKSTEREODELAYPARAMID_WETLEVEL:				m_Params.fWetLevel = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKSTEREODELAYPARAMID_FRONTREARBALANCE:		m_Params.fFrontRearBalance = fValue; break;
	case AKSTEREODELAYPARAMID_FILTERTYPE:			m_Params.FilterParams.eFilterType = (AkUInt32)(AkInt64)fValue; break;
	case AKSTEREODELAYPARAMID_FILTERGAIN:			m_Params.FilterParams.fFilterGain = fValue; break;
	case AKSTEREODELAYPARAMID_FILTERFREQUENCY:		m_Params.FilterParams.fFilterFrequency = fValue; break;
	case AKSTEREODELAYPARAMID_FILTERQFACTOR:		m_Params.FilterParams.fFilterQFactor = fValue; break;
	case AKSTEREODELAYPARAMID_LEFTINPUTTYPE:		m_Params.eInputType[ 0 ] = (AkInputChannelType)*(const AkUInt32 *)in_pValue; break;
	case AKSTEREODELAYPARAMID_LEFTDELAYTIME:		m_Params.StereoDelayParams[ 0 ].fDelayTime = fValue; break;
	case AKSTEREODELAYPARAMID_LEFTFEEDBACK:			m_Params.StereoDelayParams[ 0 ].fFeedback = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKSTEREODELAYPARAMID_LEFTCROSSFEED:		m_Params.StereoDelayParams[ 0 ].fCrossFeed = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKSTEREODELAYPARAMID_RIGHTINPUTTYPE:		m_Params.eInputType[ 1 ] = (AkInputChannelType)*(const AkUInt32 *)in_pValue; break;
	case AKSTEREODELAYPARAMID_RIGHTDELAYTIME:		m_Params.StereoDelayParams[ 1 ].fDelayTime = fValue; break;
	case AKSTEREODELAYPARAMID_RIGHTFEEDBACK:		m_Params.StereoDelayParams[ 1 ].fFeedback = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	case AKSTEREODELAYPARAMID_RIGHTCROSSFEED:		m_Params.StereoDelayParams[ 1 ].fCrossFeed = (AkReal32)pow( 10.0, fValue * 0.050000001f ); break;
	default:
		return AK_InvalidParameter;
	}
	m_ParamChangeHandler.SetParamChange( in_ParamID );
	return AK_Success;
}

/** 2012 0x998a30. */
void CAkStereoDelayFXParams::GetParams( AkStereoDelayFXParams * out_pParams )
{
	*out_pParams = m_Params;
}
