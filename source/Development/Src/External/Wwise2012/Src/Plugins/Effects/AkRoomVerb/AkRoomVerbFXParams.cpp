/*=============================================================================
	AkRoomVerbFXParams.cpp - CAkRoomVerbFXParams (2012 0x98c850..0x98d350), CreateRoomVerbFXParams 0x98d350; 2013 matched
	by bytes. Agent IH4 fork F1 (wave 26). Retail behaviour; the defaults are the .rdata AK_ROOMVERBFXPARAM_*_DEF values
	(0xda9604..0xda966c) and g_AlgoTunings (0xe3ef40), read out of the 2012 exe.
=============================================================================*/
#include "AkRoomVerbFXParams.h"
#include <math.h>
#include <string.h>

const AlgorithmTunings g_AlgoTunings = { 8.f, 50.f, 2.f, 0.1f, 0.8f, 66.f, 15.f, 5.f, 40.f, 0.5f, 50.f };

static inline AkReal32 AkDBToLin( AkReal32 in_fDB )
{
	return (AkReal32)pow( 10.0, in_fDB * 0.050000001f );
}

/** 2012 0x98d350. */
AK::IAkPluginParam * CreateRoomVerbFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkRoomVerbFXParams() );
}

/** 2012 0x98cf60: the copy is dirty in both halves. */
CAkRoomVerbFXParams::CAkRoomVerbFXParams( const CAkRoomVerbFXParams & in_rCopy )
{
	sRTPCParams = in_rCopy.sRTPCParams;
	sRTPCParams.bDirty = true;
	sInvariantParams = in_rCopy.sInvariantParams;
	sInvariantParams.bDirty = true;
	sAlgoTunings = in_rCopy.sAlgoTunings;
}

/** 2012 0x98cfb0. */
AK::IAkPluginParam * CAkRoomVerbFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkRoomVerbFXParams( *this ) );
}

/** 2012 0x98c860: the bank block, else the defaults (decay 1.2 s, HF damping 2.25, diffusion 100, width 180, filters
    100 / 1000 / 10000 Hz Q 1 flat, levels 0 dB but LFE -96, ER -20, reverb -23; delays 25 / 40 ms, density 80, shape
    100, pattern 23, 8 units, ER on, tone controls off). */
AKRESULT CAkRoomVerbFXParams::Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
{
	if( in_uBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_uBlockSize );
	}
	sRTPCParams.fDecayTime = 1.2f;
	sRTPCParams.fHFDamping = 2.25f;
	sRTPCParams.fStereoWidth = 180.f;
	sRTPCParams.fDiffusion = 100.f;
	sRTPCParams.fFilter1Gain = 0.f;
	sRTPCParams.fFilter1Freq = 100.f;
	sRTPCParams.fFilter2Gain = 0.f;
	sRTPCParams.fFilter3Gain = 0.f;
	sRTPCParams.fFilter1Q = 1.f;
	sRTPCParams.fFilter2Freq = 1000.f;
	sRTPCParams.fFilter2Q = 1.f;
	sRTPCParams.fFilter3Freq = 10000.f;
	sRTPCParams.fFilter3Q = 1.f;
	const AkReal32 fUnity = (AkReal32)pow( 10.0, 0.0 );
	sRTPCParams.fFrontLevel = fUnity;
	sRTPCParams.fRearLevel = fUnity;
	sRTPCParams.fCenterLevel = fUnity;
	const AkReal32 fLFE = (AkReal32)pow( 10.0, -4.800000071525574 );
	sRTPCParams.fLFELevel = fLFE;
	sRTPCParams.fDryLevel = fUnity;
	sRTPCParams.fERLevel = (AkReal32)pow( 10.0, -1.000000014901161 );
	sRTPCParams.fReverbLevel = (AkReal32)pow( 10.0, -1.150000017136335 );
	sInvariantParams.fReverbDelay = 25.f;
	sInvariantParams.fInputLFELevel = fLFE;
	sInvariantParams.fRoomSize = 0.f;
	sInvariantParams.fERFrontBackDelay = 40.f;
	sInvariantParams.eFilter1Pos = FILTERINSERTTYPE_ERANDREVERB;
	sInvariantParams.eFilter2Pos = FILTERINSERTTYPE_ERANDREVERB;
	sInvariantParams.eFilter3Pos = FILTERINSERTTYPE_ERANDREVERB;
	sInvariantParams.fDensity = 80.f;
	sAlgoTunings = g_AlgoTunings;
	sInvariantParams.fRoomShape = 100.f;
	sInvariantParams.uERPattern = 23;
	sInvariantParams.bEnableEarlyReflections = true;
	sInvariantParams.uNumReverbUnits = 8;
	sInvariantParams.fInputCenterLevel = fUnity;
	sInvariantParams.eFilter1Curve = FILTERCURVETYPE_LOWSHELF;
	sInvariantParams.eFilter2Curve = FILTERCURVETYPE_PEAKING;
	sInvariantParams.eFilter3Curve = FILTERCURVETYPE_HIGHSHELF;
	sInvariantParams.bEnableToneControls = false;
	sRTPCParams.bDirty = true;
	sInvariantParams.bDirty = true;
	return AK_Success;
}

/** 2012 0x98d020. */
AKRESULT CAkRoomVerbFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

static inline AkReal32 AkReadFloat( const AkUInt8 * in_pBlock, AkUInt32 in_uOffset )
{
	AkReal32 fValue;
	memcpy( &fValue, in_pBlock + in_uOffset, sizeof( fValue ) );
	return fValue;
}

static inline AkUInt32 AkReadU32( const AkUInt8 * in_pBlock, AkUInt32 in_uOffset )
{
	AkUInt32 uValue;
	memcpy( &uValue, in_pBlock + in_uOffset, sizeof( uValue ) );
	return uValue;
}

/** 2012 0x98d050: 186 bytes, packed: 20 floats (13 RTPC + 7 levels in dB), ER enable, pattern, 6 invariants, tone
    controls enable, 3 x (position, curve), input center / LFE (dB), 11 algorithm tunings. */
AKRESULT CAkRoomVerbFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
{
	const AkUInt8 * p = (const AkUInt8 *)in_pParamsBlock;
	sRTPCParams.fDecayTime = AkReadFloat( p, 0 );
	sRTPCParams.fHFDamping = AkReadFloat( p, 4 );
	sRTPCParams.fDiffusion = AkReadFloat( p, 8 );
	sRTPCParams.fStereoWidth = AkReadFloat( p, 12 );
	sRTPCParams.fFilter1Gain = AkReadFloat( p, 16 );
	sRTPCParams.fFilter1Freq = AkReadFloat( p, 20 );
	sRTPCParams.fFilter1Q = AkReadFloat( p, 24 );
	sRTPCParams.fFilter2Gain = AkReadFloat( p, 28 );
	sRTPCParams.fFilter2Freq = AkReadFloat( p, 32 );
	sRTPCParams.fFilter2Q = AkReadFloat( p, 36 );
	sRTPCParams.fFilter3Gain = AkReadFloat( p, 40 );
	sRTPCParams.fFilter3Freq = AkReadFloat( p, 44 );
	sRTPCParams.fFilter3Q = AkReadFloat( p, 48 );
	sRTPCParams.fFrontLevel = AkDBToLin( AkReadFloat( p, 52 ) );
	sRTPCParams.fRearLevel = AkDBToLin( AkReadFloat( p, 56 ) );
	sRTPCParams.fCenterLevel = AkDBToLin( AkReadFloat( p, 60 ) );
	sRTPCParams.fLFELevel = AkDBToLin( AkReadFloat( p, 64 ) );
	sRTPCParams.fDryLevel = AkDBToLin( AkReadFloat( p, 68 ) );
	sRTPCParams.fERLevel = AkDBToLin( AkReadFloat( p, 72 ) );
	sRTPCParams.fReverbLevel = (AkReal32)pow( 10.0, ( AkReadFloat( p, 76 ) - 3.0 ) * 0.050000001f );
	sInvariantParams.bEnableEarlyReflections = p[ 80 ] != 0;
	sInvariantParams.uERPattern = AkReadU32( p, 81 );
	sInvariantParams.fReverbDelay = AkReadFloat( p, 85 );
	sInvariantParams.fRoomSize = AkReadFloat( p, 89 );
	sInvariantParams.fERFrontBackDelay = AkReadFloat( p, 93 );
	sInvariantParams.fDensity = AkReadFloat( p, 97 );
	sInvariantParams.fRoomShape = AkReadFloat( p, 101 );
	sInvariantParams.uNumReverbUnits = AkReadU32( p, 105 );
	sInvariantParams.bEnableToneControls = p[ 109 ] != 0;
	sInvariantParams.eFilter1Pos = (FilterInsertType)AkReadU32( p, 110 );
	sInvariantParams.eFilter1Curve = (FilterCurveType)AkReadU32( p, 114 );
	sInvariantParams.eFilter2Pos = (FilterInsertType)AkReadU32( p, 118 );
	sInvariantParams.eFilter2Curve = (FilterCurveType)AkReadU32( p, 122 );
	sInvariantParams.eFilter3Pos = (FilterInsertType)AkReadU32( p, 126 );
	sInvariantParams.eFilter3Curve = (FilterCurveType)AkReadU32( p, 130 );
	sInvariantParams.fInputCenterLevel = AkDBToLin( AkReadFloat( p, 134 ) );
	sInvariantParams.fInputLFELevel = AkDBToLin( AkReadFloat( p, 138 ) );
	sAlgoTunings.fDensityDelayMin = AkReadFloat( p, 142 );
	sAlgoTunings.fDensityDelayMax = AkReadFloat( p, 146 );
	sAlgoTunings.fDensityDelayRdmPerc = AkReadFloat( p, 150 );
	sAlgoTunings.fRoomShapeMin = AkReadFloat( p, 154 );
	sAlgoTunings.fRoomShapeMax = AkReadFloat( p, 158 );
	sAlgoTunings.fDiffusionDelayScalePerc = AkReadFloat( p, 162 );
	sAlgoTunings.fDiffusionDelayMax = AkReadFloat( p, 166 );
	sAlgoTunings.fDiffusionDelayRdmPerc = AkReadFloat( p, 170 );
	sAlgoTunings.fDCFilterCutFreq = AkReadFloat( p, 174 );
	sAlgoTunings.fReverbUnitInputDelay = AkReadFloat( p, 178 );
	sRTPCParams.bDirty = true;
	sAlgoTunings.fReverbUnitInputDelayRmdPerc = AkReadFloat( p, 182 );
	sInvariantParams.bDirty = true;
	return AK_Success;
}

/** 2012 0x98ca20: one parameter (an RTPC, or a live edit); the ids are listed in the header. */
AKRESULT CAkRoomVerbFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_uParamSize )
{
	if( !in_pValue )
	{
		return AK_InvalidParameter;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	const AkUInt32 uValue = *(const AkUInt32 *)in_pValue;
	const bool bValue = *(const AkUInt8 *)in_pValue != 0;
	switch( in_ParamID )
	{
	case 0:		sInvariantParams.uERPattern = uValue; sInvariantParams.bDirty = true; break;
	case 1:		sInvariantParams.fReverbDelay = fValue; sInvariantParams.bDirty = true; break;
	case 2:		sInvariantParams.fRoomSize = fValue; sInvariantParams.bDirty = true; break;
	case 3:		sInvariantParams.fERFrontBackDelay = fValue; sInvariantParams.bDirty = true; break;
	case 4:		sInvariantParams.bEnableEarlyReflections = bValue; sInvariantParams.bDirty = true; break;
	case 10:	sRTPCParams.fDecayTime = fValue; sRTPCParams.bDirty = true; break;
	case 11:	sRTPCParams.fHFDamping = fValue; sRTPCParams.bDirty = true; break;
	case 12:	sInvariantParams.fDensity = fValue; sInvariantParams.bDirty = true; break;
	case 13:	sInvariantParams.fRoomShape = fValue; sInvariantParams.bDirty = true; break;
	case 14:	sInvariantParams.uNumReverbUnits = uValue; sInvariantParams.bDirty = true; break;
	case 15:	sRTPCParams.fDiffusion = fValue; sRTPCParams.bDirty = true; break;
	case 16:	sRTPCParams.fStereoWidth = fValue; sRTPCParams.bDirty = true; break;
	case 20:	sInvariantParams.bEnableToneControls = bValue; sInvariantParams.bDirty = true; break;
	case 21:	sInvariantParams.eFilter1Pos = (FilterInsertType)uValue; sInvariantParams.bDirty = true; break;
	case 22:	sInvariantParams.eFilter1Curve = (FilterCurveType)uValue; sInvariantParams.bDirty = true; break;
	case 23:	sRTPCParams.fFilter1Gain = fValue; sRTPCParams.bDirty = true; break;
	case 24:	sRTPCParams.fFilter1Freq = fValue; sRTPCParams.bDirty = true; break;
	case 25:	sRTPCParams.fFilter1Q = fValue; sRTPCParams.bDirty = true; break;
	case 26:	sInvariantParams.eFilter2Pos = (FilterInsertType)uValue; sInvariantParams.bDirty = true; break;
	case 27:	sInvariantParams.eFilter2Curve = (FilterCurveType)uValue; sInvariantParams.bDirty = true; break;
	case 28:	sRTPCParams.fFilter2Gain = fValue; sRTPCParams.bDirty = true; break;
	case 29:	sRTPCParams.fFilter2Freq = fValue; sRTPCParams.bDirty = true; break;
	case 30:	sRTPCParams.fFilter2Q = fValue; sRTPCParams.bDirty = true; break;
	case 31:	sInvariantParams.eFilter3Pos = (FilterInsertType)uValue; sInvariantParams.bDirty = true; break;
	case 32:	sInvariantParams.eFilter3Curve = (FilterCurveType)uValue; sInvariantParams.bDirty = true; break;
	case 33:	sRTPCParams.fFilter3Gain = fValue; sRTPCParams.bDirty = true; break;
	case 34:	sRTPCParams.fFilter3Freq = fValue; sRTPCParams.bDirty = true; break;
	case 35:	sRTPCParams.fFilter3Q = fValue; sRTPCParams.bDirty = true; break;
	case 40:	sInvariantParams.fInputCenterLevel = AkDBToLin( fValue ); sInvariantParams.bDirty = true; break;
	case 41:	sInvariantParams.fInputLFELevel = AkDBToLin( fValue ); sInvariantParams.bDirty = true; break;
	case 50:	sRTPCParams.fFrontLevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 51:	sRTPCParams.fRearLevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 52:	sRTPCParams.fCenterLevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 53:	sRTPCParams.fLFELevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 60:	sRTPCParams.fDryLevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 61:	sRTPCParams.fERLevel = AkDBToLin( fValue ); sRTPCParams.bDirty = true; break;
	case 62:	sRTPCParams.fReverbLevel = (AkReal32)pow( 10.0, ( fValue - 3.0 ) * 0.050000001f ); sRTPCParams.bDirty = true; break;
	case 100:	sAlgoTunings.fDensityDelayMin = fValue; break;
	case 101:	sAlgoTunings.fDensityDelayMax = fValue; break;
	case 102:	sAlgoTunings.fDensityDelayRdmPerc = fValue; break;
	case 103:	sAlgoTunings.fRoomShapeMin = fValue; break;
	case 104:	sAlgoTunings.fRoomShapeMax = fValue; break;
	case 105:	sAlgoTunings.fDiffusionDelayScalePerc = fValue; break;
	case 106:	sAlgoTunings.fDiffusionDelayMax = fValue; break;
	case 107:	sAlgoTunings.fDiffusionDelayRdmPerc = fValue; break;
	case 108:	sAlgoTunings.fDCFilterCutFreq = fValue; break;
	case 109:	sAlgoTunings.fReverbUnitInputDelay = fValue; break;
	case 110:	sAlgoTunings.fReverbUnitInputDelayRmdPerc = fValue; break;
	default:	break;
	}
	return AK_Success;
}
