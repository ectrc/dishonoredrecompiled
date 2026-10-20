/*=============================================================================
	AkHarmonizerFXParams.h - CAkHarmonizerFXParams (Wwise 2012.1, plugins/effects/akharmonizer/akharmonizerfxparams.h).
	Agent IH4 (fork F2). Layouts are the 2012 PDB's: two 28-byte voices (the PDB types them as a byte array; the
	offsets are SetParam 0x990990's: filter type / gain / frequency / Q, pitch factor, gain, enable),
	AkHarmonizerFXParams 76, CAkHarmonizerFXParams 84 bytes (a 20-parameter change handler, 3 bytes).
=============================================================================*/
#ifndef _AK_HARMONIZERFXPARAMS_H_
#define _AK_HARMONIZERFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkHarmonizerFXFactory.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

#ifndef AK_PITCHVOICE_TYPES_DEFINED
#define AK_PITCHVOICE_TYPES_DEFINED
/** Shared with the pitch shifter's parameters (the SDK declares them in both plug-ins' headers). */
enum AkInputType
{
	AKINPUTTYPE_ASINPUT = 0,
	AKINPUTTYPE_CENTER = 1,
	AKINPUTTYPE_STEREO = 2,
	AKINPUTCHANNELTYPE_3POINT0 = 3,
	AKINPUTTYPE_4POINT0 = 4,
	AKINPUTTYPE_5POINT0 = 5,
	AKINPUTTYPE_LEFTONLY = 6
};

enum AkFilterType
{
	AKFILTERTYPE_NONE = 0
};

struct AkVoiceFilterParams
{
	AkUInt32	eFilterType;
	AkReal32	fFilterGain;
	AkReal32	fFilterFrequency;
	AkReal32	fFilterQFactor;
};

struct AkPitchVoiceParams
{
	AkVoiceFilterParams	Filter;
	AkReal32			fPitchFactor;
};
#endif

struct AkHarmonizerVoiceParams
{
	AkVoiceFilterParams	Filter;
	AkReal32			fPitchFactor;
	AkReal32			fGain;
	bool				bEnable;
};

static const AkPluginParamID AKHARMONIZERPARAMID_INPUT = 0;
static const AkPluginParamID AKHARMONIZERPARAMID_PROCESSLFE = 1;
static const AkPluginParamID AKHARMONIZERPARAMID_SYNCDRY = 2;
static const AkPluginParamID AKHARMONIZERPARAMID_DRYLEVEL = 3;
static const AkPluginParamID AKHARMONIZERPARAMID_WETLEVEL = 4;
static const AkPluginParamID AKHARMONIZERPARAMID_WINDOWSIZE = 5;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1ENABLE = 6;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1PITCH = 7;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1GAIN = 8;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1FILTERTYPE = 9;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1FILTERGAIN = 10;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1FILTERFREQUENCY = 11;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE1FILTERQFACTOR = 12;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2ENABLE = 13;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2PITCH = 14;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2GAIN = 15;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2FILTERTYPE = 16;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2FILTERGAIN = 17;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2FILTERFREQUENCY = 18;
static const AkPluginParamID AKHARMONIZERPARAMID_VOICE2FILTERQFACTOR = 19;
static const AkUInt32 AKHARMONIZERPARAMID_NUM = 20;

struct AkHarmonizerFXParams
{
	AkHarmonizerVoiceParams	Voice[ 2 ];
	AkInputType				eInputType;
	AkReal32				fDryLevel;
	AkReal32				fWetLevel;
	AkUInt32				uWindowSize;
	bool					bProcessLFE;
	bool					bSyncDry;

	AkHarmonizerFXParams()
	{
		for( AkUInt32 i = 0; i < 2; ++i )
		{
			Voice[ i ].Filter.eFilterType = AKFILTERTYPE_NONE;
			Voice[ i ].Filter.fFilterGain = 1.f;
			Voice[ i ].Filter.fFilterFrequency = 1000.f;
			Voice[ i ].Filter.fFilterQFactor = 1.f;
			Voice[ i ].fPitchFactor = 1.f;
			Voice[ i ].fGain = 1.f;
			Voice[ i ].bEnable = false;
		}
		eInputType = AKINPUTTYPE_ASINPUT;
		fDryLevel = 1.f;
		fWetLevel = 1.f;
		uWindowSize = 1024;
		bProcessLFE = false;
		bSyncDry = false;
	}
};

class CAkHarmonizerFXParams : public AK::IAkPluginParam
{
public:
	CAkHarmonizerFXParams();
	CAkHarmonizerFXParams( const CAkHarmonizerFXParams & in_rCopy );
	~CAkHarmonizerFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );

	void GetParams( AkHarmonizerFXParams * out_pParams );

	AK::AkFXParameterChangeHandler<AKHARMONIZERPARAMID_NUM> m_ParamChangeHandler;

private:
	AkHarmonizerFXParams m_Params;
};

static_assert( sizeof( AkHarmonizerVoiceParams ) == 28, "AkHarmonizerVoiceParams: 2012 PDB 28-byte voice" );
static_assert( sizeof( AkHarmonizerFXParams ) == 76, "AkHarmonizerFXParams: 2012 PDB sizeof 76" );
static_assert( sizeof( CAkHarmonizerFXParams ) == 84, "CAkHarmonizerFXParams: 2012 PDB sizeof 84" );

#endif
