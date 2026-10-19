/*=============================================================================
	AkEnvironmentsMgr.h - CAkEnvironmentsMgr (akenvironmentsmgr.cpp): the obstruction / occlusion curves (ENVS chunk of
	Init.bnk) and the environmental (aux) share sets' volumes and bypass.

	DISHONORED(port): agent IH3 (wave 26), fork R; replaces IH1's AkGameSyncs data store. Layout is the 2012 PDB's.
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

class CAkEnvironmentsMgr
{
public:
	enum eCurveXType { CurveObs = 0, CurveOcc = 1, MAX_CURVE_X_TYPES = 2 };
	enum eCurveYType { CurveVol = 0, CurveLPF = 1, MAX_CURVE_Y_TYPES = 2 };

	AKRESULT Init();
	AKRESULT Term();

	AKRESULT SetObsOccCurve( eCurveXType in_x, eCurveYType in_y, AkUInt32 in_ulNbPoints, AkRTPCGraphPoint * in_paPoints, AkCurveScaling in_eScaling );
	AkReal32 GetCurveValue( eCurveXType in_x, eCurveYType in_y, AkReal32 in_value );
	bool IsCurveEnabled( eCurveXType in_x, eCurveYType in_y ) const { return m_bCurveEnabled[ in_x ][ in_y ]; }

	AKRESULT SetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID, const AkSpeakerVolumes & in_sVolumes );
	AkSpeakerVolumes GetEnvironmentVolumes( AkAuxBusID in_FXParameterSetID );
	AKRESULT BypassEnvironment( AkAuxBusID in_FXParameterSetID, bool in_bIsBypassed );
	bool IsBypassed( AkAuxBusID in_FXParameterSetID );

	bool											m_bEnvironmentalBypass;
	bool											m_bCurveEnabled[ MAX_CURVE_X_TYPES ][ MAX_CURVE_Y_TYPES ];
	CAkConversionTable<AkRTPCGraphPoint, AkReal32>	ConversionTable[ MAX_CURVE_X_TYPES ][ MAX_CURVE_Y_TYPES ];
};
