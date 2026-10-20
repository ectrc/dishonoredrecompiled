/*=============================================================================
	AkParametricEQFX.h - CAkParametricEQFX and CAkParameterEQFXParams (plugins\effects\akparametriceq: akparametriceqfx.cpp
	(win32), akparametriceqfxparams.cpp, akparametriceqfxcommon.h), Wwise 2012.1: three biquad bands (RBJ low / high
	pass, band pass, notch, low / high shelf, peaking) and an output gain ramp, in place on every channel but the LFE
	unless bProcessLFE.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (CAkParametricEQFX 84 bytes:
	CreateParametricEQFX; CAkParameterEQFXParams 76: Clone; EQModuleParams 17 bytes packed in the bank block).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

enum AkFilterType
{
	AKFILTERTYPE_LOWPASS	= 0,
	AKFILTERTYPE_HIPASS		= 1,
	AKFILTERTYPE_BANDPASS	= 2,
	AKFILTERTYPE_NOTCH		= 3,
	AKFILTERTYPE_LOWSHELF	= 4,
	AKFILTERTYPE_HISHELF	= 5,
	AKFILTERTYPE_PEAKINGEQ	= 6
};

enum AkBandNumber
{
	BAND1 = 0,
	BAND2 = 1,
	BAND3 = 2
};

struct EQModuleParams
{
	AkFilterType	eFilterType;
	AkReal32		fGain;
	AkReal32		fFrequency;
	AkReal32		fQFactor;
	bool			bOnOff;
};

struct AkParametricEQFXParams
{
	EQModuleParams	Band[ 3 ];
	AkReal32		fOutputLevel;
	bool			bProcessLFE;
};

/** SetParam ids: band b (0..2) at 5b + { 0 type, 1 gain (dB), 2 frequency, 3 Q, 4 on/off }; 15 output level (dB);
    16 process LFE. */
class CAkParameterEQFXParams : public AK::IAkPluginParam
{
public:
	CAkParameterEQFXParams();
	CAkParameterEQFXParams( const CAkParameterEQFXParams & in_rCopy );
	virtual ~CAkParameterEQFXParams() {}

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_uParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize );

	AkParametricEQFXParams	m_Params;
	bool					m_bBandDirty[ 3 ];
};

class CAkParametricEQFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkParametricEQFX() : m_pSharedParams( NULL ), m_pfAllocatedMem( NULL ) {}
	virtual ~CAkParametricEQFX() {}

	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames ) { return AK_DataReady; }

	void ComputeBiquadCoefs( AkBandNumber in_eBandNum, EQModuleParams * in_sModuleParams );

	AkReal32					m_pfFiltCoefs[ 3 ][ 5 ];
	CAkParameterEQFXParams *	m_pSharedParams;
	AkUInt32					m_uNumProcessedChannels;
	AkUInt32					m_uSampleRate;
	AkReal32 *					m_pfAllocatedMem;
	AkReal32					m_fCurrentGain;
};

static_assert( sizeof( EQModuleParams ) == 20, "EQModuleParams: 2012 PDB sizeof 20" );
static_assert( sizeof( AkParametricEQFXParams ) == 68, "AkParametricEQFXParams: 2012 PDB sizeof 68" );
static_assert( sizeof( CAkParameterEQFXParams ) == 76, "CAkParameterEQFXParams: 2012 sizeof 76 (Clone)" );
static_assert( sizeof( CAkParametricEQFX ) == 84, "CAkParametricEQFX: 2012 sizeof 84 (CreateParametricEQFX)" );
