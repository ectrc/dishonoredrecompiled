/*=============================================================================
	AkTimeStretchFXParams.h - CAkTimeStretchFXParams (Wwise 2012.1, plugins/effects/aktimestretch/aktimestretchfxparams.h).
	Agent IH4 (fork F2). AkTimeStretchFXParams 16, CAkTimeStretchFXParams 20 bytes as the 2012 PDB.
=============================================================================*/
#ifndef _AK_TIMESTRETCHFXPARAMS_H_
#define _AK_TIMESTRETCHFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkTimeStretchFXFactory.h>

static const AkPluginParamID AK_TIMESTRETCHFXPARAM_WINDOWSIZE_ID = 0;
static const AkPluginParamID AK_TIMESTRETCHFXPARAM_TIMESTRETCH_ID = 1;
static const AkPluginParamID AK_TIMESTRETCHFXPARAM_OUTPUTGAIN_ID = 2;
static const AkPluginParamID AK_TIMESTRETCHFXPARAM_TIMESTRETCHRANDOM_ID = 3;

struct AkTimeStretchFXParams
{
	AkUInt32	uWindowSize;
	AkReal32	fTimeStretch;
	AkReal32	fTimeStretchRandom;
	AkReal32	fOutputGain;

	AkTimeStretchFXParams() : uWindowSize( 2048 ), fTimeStretch( 0.f ), fTimeStretchRandom( 0.f ), fOutputGain( 1.f ) {}
};

class CAkTimeStretchFXParams : public AK::IAkPluginParam
{
public:
	CAkTimeStretchFXParams();
	CAkTimeStretchFXParams( const CAkTimeStretchFXParams & in_rCopy );
	~CAkTimeStretchFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );

	void GetParams( AkTimeStretchFXParams * out_pParams );

private:
	AkTimeStretchFXParams m_Params;
};

static_assert( sizeof( AkTimeStretchFXParams ) == 16, "AkTimeStretchFXParams: 2012 PDB sizeof 16" );
static_assert( sizeof( CAkTimeStretchFXParams ) == 20, "CAkTimeStretchFXParams: 2012 PDB sizeof 20" );

#endif
