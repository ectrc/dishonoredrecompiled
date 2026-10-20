/*=============================================================================
	AkDelayFXParams.h - CAkDelayFXParams (Wwise 2012.1, samples/plugins/akdelay/sources/audioenginefx/akdelayfxparams.h).
	Agent IH4 (fork F2). Layouts are the 2012 PDB's: AkDelayRTPCParams 16, AkDelayNonRTPCParams 8, AkDelayFXParams 24,
	CAkDelayFXParams 28 bytes. The params class inherits the struct (CAkDelayFXParams : IAkPluginParam, AkDelayFXParams).
=============================================================================*/
#ifndef _AK_DELAYFXPARAMS_H_
#define _AK_DELAYFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkDelayFXFactory.h>

static const AkPluginParamID AK_DELAYFXPARAM_DELAYTIME_ID = 0;
static const AkPluginParamID AK_DELAYFXPARAM_FEEDBACK_ID = 1;
static const AkPluginParamID AK_DELAYFXPARAM_WETDRYMIX_ID = 2;
static const AkPluginParamID AK_DELAYFXPARAM_OUTPUTLEVEL_ID = 3;
static const AkPluginParamID AK_DELAYFXPARAM_FEEDBACKENABLED_ID = 4;
static const AkPluginParamID AK_DELAYFXPARAM_PROCESSLFE_ID = 5;

/** 2012 .rdata 0xda2764. */
static const AkReal32 ONEOVER_DELAYFXPARAM_PERCENT_MAX = 0.0099999998f;

struct AkDelayRTPCParams
{
	AkReal32	fFeedback;
	AkReal32	fWetDryMix;
	AkReal32	fOutputLevel;
	bool		bFeedbackEnabled;
	bool		bHasChanged;
};

struct AkDelayNonRTPCParams
{
	AkReal32	fDelayTime;
	bool		bProcessLFE;
	bool		bHasChanged;
};

struct AkDelayFXParams
{
	AkDelayRTPCParams		RTPC;
	AkDelayNonRTPCParams	NonRTPC;
};

class CAkDelayFXParams : public AK::IAkPluginParam, public AkDelayFXParams
{
public:
	CAkDelayFXParams() {}
	CAkDelayFXParams( const CAkDelayFXParams & in_rCopy );
	~CAkDelayFXParams() {}

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
};

static_assert( sizeof( AkDelayRTPCParams ) == 16, "AkDelayRTPCParams: 2012 PDB sizeof 16" );
static_assert( sizeof( AkDelayNonRTPCParams ) == 8, "AkDelayNonRTPCParams: 2012 PDB sizeof 8" );
static_assert( sizeof( AkDelayFXParams ) == 24, "AkDelayFXParams: 2012 PDB sizeof 24" );
static_assert( sizeof( CAkDelayFXParams ) == 28, "CAkDelayFXParams: 2012 PDB sizeof 28" );

#endif
