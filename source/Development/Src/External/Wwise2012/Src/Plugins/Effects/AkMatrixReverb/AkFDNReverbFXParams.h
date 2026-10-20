/*=============================================================================
	AkFDNReverbFXParams.h - CAkFDNReverbFXParams (Wwise 2012.1, plugins/effects/akfdnreverb/akfdnreverbfxparams.h).
	Agent IH4 (fork F2). Layouts are the 2012 PDB's: AkFDNReverbRTPCParams 16, AkFDNReverbNonRTPCParams 84,
	AkFDNReverbFXParams 100, CAkFDNReverbFXParams 104 bytes (the params class inherits the struct).
=============================================================================*/
#ifndef _AK_FDNREVERBFXPARAMS_H_
#define _AK_FDNREVERBFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkMatrixReverbFXFactory.h>

static const AkPluginParamID AK_FDNREVERBFXPARAM_REVERBTIME_ID = 0;
static const AkPluginParamID AK_FDNREVERBFXPARAM_HFRATIO_ID = 1;
static const AkPluginParamID AK_FDNREVERBFXPARAM_NUMBEROFDELAYS_ID = 2;
static const AkPluginParamID AK_FDNREVERBFXPARAM_DRYLEVEL_ID = 3;
static const AkPluginParamID AK_FDNREVERBFXPARAM_WETLEVEL_ID = 4;
static const AkPluginParamID AK_FDNREVERBFXPARAM_PREDELAY_ID = 5;
static const AkPluginParamID AK_FDNREVERBFXPARAM_PROCESSLFE_ID = 6;
static const AkPluginParamID AK_FDNREVERBFXPARAM_DELAYLENGTHSMODE_ID = 7;
static const AkPluginParamID AK_FDNREVERBFXPARAM_FIRSTDELAYTIME_ID = 8;

struct AkFDNReverbRTPCParams
{
	AkReal32 fReverbTime;
	AkReal32 fHFRatio;
	AkReal32 fDryLevel;
	AkReal32 fWetLevel;
};

struct AkFDNReverbNonRTPCParams
{
	AkUInt32	uNumberOfDelays;
	AkReal32	fPreDelay;
	AkUInt32	uProcessLFE;
	AkUInt32	uDelayLengthsMode;
	AkReal32	fDelayTime[ 16 ];
	bool		bDirty;
};

struct AkFDNReverbFXParams
{
	AkFDNReverbRTPCParams		RTPC;
	AkFDNReverbNonRTPCParams	NonRTPC;
};

class CAkFDNReverbFXParams : public AK::IAkPluginParam, public AkFDNReverbFXParams
{
public:
	CAkFDNReverbFXParams();
	CAkFDNReverbFXParams( const CAkFDNReverbFXParams & in_rCopy );
	~CAkFDNReverbFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
};

static_assert( sizeof( AkFDNReverbRTPCParams ) == 16, "AkFDNReverbRTPCParams: 2012 PDB sizeof 16" );
static_assert( sizeof( AkFDNReverbNonRTPCParams ) == 84, "AkFDNReverbNonRTPCParams: 2012 PDB sizeof 84" );
static_assert( sizeof( AkFDNReverbFXParams ) == 100, "AkFDNReverbFXParams: 2012 PDB sizeof 100" );
static_assert( sizeof( CAkFDNReverbFXParams ) == 104, "CAkFDNReverbFXParams: 2012 PDB sizeof 104" );

#endif
