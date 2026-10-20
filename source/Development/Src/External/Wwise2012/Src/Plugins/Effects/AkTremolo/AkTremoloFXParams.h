/*=============================================================================
	AkTremoloFXParams.h - CAkTremoloFXParams (Wwise 2012.1, plugins/effects/aktremolo/aktremolofxparams.h). Agent IH4 (fork F2).
	Layouts are the 2012 PDB's: AkTremoloRTPCParams 40, AkTremoloNonRTPCParams 3, AkTremoloFXParams 44,
	CAkTremoloFXParams 48 bytes. Parameter ids 1..11 as SetParam 0x99b220 switches on.
=============================================================================*/
#ifndef _AK_TREMOLOFXPARAMS_H_
#define _AK_TREMOLOFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkTremoloFXFactory.h>
#include "LFO.h"

static const AkPluginParamID AK_TREMOLOFXPARAM_MODDEPTH_ID = 1;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODFREQUENCY_ID = 2;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODWAVEFORM_ID = 3;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODSMOOTH_ID = 4;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODPWM_ID = 5;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODPHASEOFFSET_ID = 6;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODPHASEMODE_ID = 7;
static const AkPluginParamID AK_TREMOLOFXPARAM_MODPHASESPREAD_ID = 8;
static const AkPluginParamID AK_TREMOLOFXPARAM_OUTPUTLEVEL_ID = 9;
static const AkPluginParamID AK_TREMOLOFXPARAM_PROCESSCENTER_ID = 10;
static const AkPluginParamID AK_TREMOLOFXPARAM_PROCESSLFE_ID = 11;

/** The defaults retail's Init stores (2012 .rdata 0xda9848..0xda985c). */
static const AkReal32 AK_TREMOLOFXPARAM_MODDEPTH_DEF = 1.f;
static const AkReal32 AK_TREMOLOFXPARAM_MODFREQUENCY_DEF = 1.f;
static const AkReal32 AK_TREMOLOFXPARAM_MODSMOOTH_DEF = 0.f;
static const AkReal32 AK_TREMOLOFXPARAM_MODPWM_DEF = 0.5f;
static const AkReal32 AK_TREMOLOFXPARAM_MODPHASEOFFSET_DEF = 0.f;
static const AkReal32 AK_TREMOLOFXPARAM_MODPHASESPREAD_DEF = 0.f;
static const AkReal32 AK_TREMOLOFXPARAM_OUTPUTLEVEL_DEF = 1.f;

struct AkTremoloRTPCParams
{
	AkReal32							fModDepth;
	DSP::LFO::MultiChannel::AllParams	modParams;
	AkReal32							fOutputGain;
	bool								bHasChanged;
};

struct AkTremoloNonRTPCParams
{
	bool bProcessCenter;
	bool bProcessLFE;
	bool bHasChanged;
};

struct AkTremoloFXParams
{
	AkTremoloRTPCParams		RTPC;
	AkTremoloNonRTPCParams	NonRTPC;
};

class CAkTremoloFXParams : public AK::IAkPluginParam
{
public:
	friend class CAkTremoloFX;

	CAkTremoloFXParams();
	CAkTremoloFXParams( const CAkTremoloFXParams & in_rCopy );
	~CAkTremoloFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );

private:
	void SetDirty( bool in_bDirty );

	AkTremoloFXParams m_Params;
};

static_assert( sizeof( AkTremoloRTPCParams ) == 40, "AkTremoloRTPCParams: 2012 PDB sizeof 40" );
static_assert( sizeof( AkTremoloNonRTPCParams ) == 3, "AkTremoloNonRTPCParams: 2012 PDB sizeof 3" );
static_assert( sizeof( AkTremoloFXParams ) == 44, "AkTremoloFXParams: 2012 PDB sizeof 44" );
static_assert( sizeof( CAkTremoloFXParams ) == 48, "CAkTremoloFXParams: 2012 PDB sizeof 48" );

#endif
