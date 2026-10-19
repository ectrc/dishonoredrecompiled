/*=============================================================================
	AkEnvironmentsMgr.cpp - CAkEnvironmentsMgr (akenvironmentsmgr.cpp, 2012 0x933ec0..0x9340f0; 2013 0x9307c0..0x9309f0,
	matched by bytes).

	DISHONORED(port): agent IH3 (wave 26), fork R. Rva on each definition.
	Deviation (named): BypassEnvironment stores the share set's flag but does not call CAkLEngine::BypassEnvFx
	(2012 0x91c440): the environmental FX busses of the lower engine are IH4's (no env bus is ever created).
=============================================================================*/
#include "AkHierarchy.h"
#include "AkEnvironmentsMgr.h"

/** 2012 0x933ec0. */
AKRESULT CAkEnvironmentsMgr::Init()
{
	m_bEnvironmentalBypass = false;
	memset( m_bCurveEnabled, 0, sizeof( m_bCurveEnabled ) );
	return AK_Success;
}

/** 2012 0x933ed0. */
AKRESULT CAkEnvironmentsMgr::Term()
{
	for( AkUInt32 x = 0; x < MAX_CURVE_X_TYPES; ++x )
	{
		for( AkUInt32 y = 0; y < MAX_CURVE_Y_TYPES; ++y )
		{
			ConversionTable[ x ][ y ].Unset();
		}
	}
	return AK_Success;
}

/** 2012 0x933f20. */
AKRESULT CAkEnvironmentsMgr::SetObsOccCurve( eCurveXType in_x, eCurveYType in_y, AkUInt32 in_ulNbPoints, AkRTPCGraphPoint * in_paPoints, AkCurveScaling in_eScaling )
{
	return ConversionTable[ in_x ][ in_y ].Set( in_paPoints, in_ulNbPoints, in_eScaling );
}

/** 2012 0x933f50: a disabled curve answers 0. */
AkReal32 CAkEnvironmentsMgr::GetCurveValue( eCurveXType in_x, eCurveYType in_y, AkReal32 in_value )
{
	if( !m_bCurveEnabled[ in_x ][ in_y ] )
	{
		return 0.f;
	}
	AkUInt32 uIndex = 0;
	return ConversionTable[ in_x ][ in_y ].ConvertInternal( in_value, uIndex );
}

/** 2012 0x933f90. */
AKRESULT CAkEnvironmentsMgr::SetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID, const AkSpeakerVolumes & in_sVolumes )
{
	CAkFxShareSet * pFx = g_pIndex->m_idxFxShareSets.GetPtrAndAddRef( in_FXParameterSetID );
	if( !pFx )
	{
		return AK_Fail;
	}
	pFx->m_sVolumes = in_sVolumes;
	pFx->Release();
	return AK_Success;
}

/** 2012 0x933ff0: an unknown share set answers 0 on every speaker (the folded .rdata zero the decompile names EmptyArrayData). */
AkSpeakerVolumes CAkEnvironmentsMgr::GetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID )
{
	AkSpeakerVolumes volumes;
	CAkFxShareSet * pFx = g_pIndex->m_idxFxShareSets.GetPtrAndAddRef( in_FXParameterSetID );
	if( pFx )
	{
		volumes = pFx->m_sVolumes;
		pFx->Release();
		return volumes;
	}
	volumes.fFrontLeft = volumes.fFrontRight = volumes.fCenter = volumes.fRearLeft = volumes.fRearRight = volumes.fLfe = 0.f;
	return volumes;
}

/** 2012 0x9340a0. */
AKRESULT CAkEnvironmentsMgr::BypassEnvironment( AkAuxBusID in_FXParameterSetID, bool in_bIsBypassed )
{
	CAkFxShareSet * pFx = g_pIndex->m_idxFxShareSets.GetPtrAndAddRef( in_FXParameterSetID );
	if( !pFx )
	{
		return AK_Fail;
	}
	pFx->m_bBypassed = in_bIsBypassed;
	pFx->Release();
	return AK_Success;
}

/** 2012 0x9340f0: the global bypass, an unknown share set, or the share set's own flag. */
bool CAkEnvironmentsMgr::IsBypassed( AkAuxBusID in_FXParameterSetID )
{
	if( m_bEnvironmentalBypass )
	{
		return true;
	}
	CAkFxShareSet * pFx = g_pIndex->m_idxFxShareSets.GetPtrAndAddRef( in_FXParameterSetID );
	if( !pFx )
	{
		return true;
	}
	const bool bBypassed = pFx->m_bBypassed;
	pFx->Release();
	return bBypassed;
}
