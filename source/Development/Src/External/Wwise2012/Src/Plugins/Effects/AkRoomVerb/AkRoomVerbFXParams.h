/*=============================================================================
	AkRoomVerbFXParams.h - CAkRoomVerbFXParams (plugins\effects\akroomverb\akroomverbfxparams.cpp/.h, Wwise 2012.1): the
	RoomVerb's parameter object, its bank block layout and the RTPC'able parameter ids.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's: RTPCParams 84 bytes, InvariantParams 64,
	AlgorithmTunings 44, CAkRoomVerbFXParams 196 (Clone / CreateRoomVerbFXParams allocate 196). Levels are kept linear
	(the block holds dB; SetParamsBlock / SetParam convert with 10^(dB/20), the reverb level 3 dB lower).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

enum FilterInsertType
{
	FILTERINSERTTYPE_OFF			= 0,
	FILTERINSERTTYPE_ERONLY			= 1,
	FILTERINSERTTYPE_REVERBONLY		= 2,
	FILTERINSERTTYPE_ERANDREVERB	= 3
};

enum FilterCurveType
{
	FILTERCURVETYPE_LOWSHELF	= 0,
	FILTERCURVETYPE_PEAKING		= 1,
	FILTERCURVETYPE_HIGHSHELF	= 2
};

/** SetParam's ids (2012 0x98ca20): 0..4 ER pattern / reverb delay / room size / ER front-back delay / ER enable;
    10, 11 decay / HF damping; 12..14 density / room shape / units; 15, 16 diffusion / stereo width; 20 tone controls;
    21..25, 26..30, 31..35 filter 1..3 position, curve, gain, frequency, Q; 40, 41 input center / LFE (dB); 50..53 front /
    rear / center / LFE levels (dB); 60..62 dry / ER / reverb levels (dB, reverb -3 dB); 100..110 the algorithm tunings. */

struct RTPCParams
{
	AkReal32	fDecayTime;
	AkReal32	fHFDamping;
	AkReal32	fDiffusion;
	AkReal32	fStereoWidth;
	AkReal32	fFilter1Gain;
	AkReal32	fFilter1Freq;
	AkReal32	fFilter1Q;
	AkReal32	fFilter2Gain;
	AkReal32	fFilter2Freq;
	AkReal32	fFilter2Q;
	AkReal32	fFilter3Gain;
	AkReal32	fFilter3Freq;
	AkReal32	fFilter3Q;
	AkReal32	fFrontLevel;
	AkReal32	fRearLevel;
	AkReal32	fCenterLevel;
	AkReal32	fLFELevel;
	AkReal32	fDryLevel;
	AkReal32	fERLevel;
	AkReal32	fReverbLevel;
	bool		bDirty;
};

struct InvariantParams
{
	AkUInt32			uERPattern;
	AkReal32			fReverbDelay;
	AkReal32			fRoomSize;
	AkReal32			fERFrontBackDelay;
	AkReal32			fDensity;
	AkReal32			fRoomShape;
	AkUInt32			uNumReverbUnits;
	AkReal32			fInputCenterLevel;
	AkReal32			fInputLFELevel;
	FilterInsertType	eFilter1Pos;
	FilterCurveType		eFilter1Curve;
	FilterInsertType	eFilter2Pos;
	FilterCurveType		eFilter2Curve;
	FilterInsertType	eFilter3Pos;
	FilterCurveType		eFilter3Curve;
	bool				bEnableToneControls;
	bool				bEnableEarlyReflections;
	bool				bDirty;
};

struct AlgorithmTunings
{
	AkReal32 fDensityDelayMin;
	AkReal32 fDensityDelayMax;
	AkReal32 fDensityDelayRdmPerc;
	AkReal32 fRoomShapeMin;
	AkReal32 fRoomShapeMax;
	AkReal32 fDiffusionDelayScalePerc;
	AkReal32 fDiffusionDelayMax;
	AkReal32 fDiffusionDelayRdmPerc;
	AkReal32 fDCFilterCutFreq;
	AkReal32 fReverbUnitInputDelay;
	AkReal32 fReverbUnitInputDelayRmdPerc;
};

struct AkRoomVerbFXParams
{
	RTPCParams			sRTPCParams;
	InvariantParams		sInvariantParams;
	AlgorithmTunings	sAlgoTunings;
};

class CAkRoomVerbFXParams : public AK::IAkPluginParam, public AkRoomVerbFXParams
{
public:
	CAkRoomVerbFXParams() {}
	CAkRoomVerbFXParams( const CAkRoomVerbFXParams & in_rCopy );
	virtual ~CAkRoomVerbFXParams() {}

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_uParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize );
};

/** The algorithm tunings a parameter object without a bank block starts with (2012 .data 0xe3ef40 g_AlgoTunings). */
extern const AlgorithmTunings g_AlgoTunings;

static_assert( sizeof( RTPCParams ) == 84, "RTPCParams: 2012 PDB sizeof 84" );
static_assert( sizeof( InvariantParams ) == 64, "InvariantParams: 2012 PDB sizeof 64" );
static_assert( sizeof( AlgorithmTunings ) == 44, "AlgorithmTunings: 2012 PDB sizeof 44" );
static_assert( sizeof( CAkRoomVerbFXParams ) == 196, "CAkRoomVerbFXParams: 2012 sizeof 196 (Clone)" );
