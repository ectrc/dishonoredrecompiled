/*=============================================================================
	AkPitchShifterFXParams.h - CAkPitchShifterFXParams (Wwise 2012.1, plugins/effects/akpitchshifter/akpitchshifterfxparams.h).
	Agent IH4 (fork F2). Layouts are the 2012 PDB's: AkVoiceFilterParams 16, AkPitchVoiceParams 20, AkPitchShifterFXParams
	40, CAkPitchShifterFXParams 48 bytes (an 11-parameter change handler, 2 bytes, then the parameters).
	The voice filter type is 0 for none, else MultiChannelBiquadFilter<6>::FilterType + 1 (InitPitchVoice 0x996130).
=============================================================================*/
#ifndef _AK_PITCHSHIFTERFXPARAMS_H_
#define _AK_PITCHSHIFTERFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkPitchShifterFXFactory.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

#ifndef AK_PITCHVOICE_TYPES_DEFINED
#define AK_PITCHVOICE_TYPES_DEFINED
/** Shared with the harmonizer's parameters (the SDK declares them in both plug-ins' headers). */
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

static const AkPluginParamID AKPITCHSHIFTERPARAMID_INPUT = 0;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_PROCESSLFE = 1;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_SYNCDRY = 2;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_DRYLEVEL = 3;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_WETLEVEL = 4;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_DELAYTIME = 5;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_PITCH = 6;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_FILTERTYPE = 7;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_FILTERGAIN = 8;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_FILTERFREQUENCY = 9;
static const AkPluginParamID AKPITCHSHIFTERPARAMID_FILTERQFACTOR = 10;
static const AkUInt32 AKPITCHSHIFTERPARAMID_NUM = 11;

struct AkPitchShifterFXParams
{
	AkPitchVoiceParams	Voice;
	AkInputType			eInputType;
	AkReal32			fDryLevel;
	AkReal32			fWetLevel;
	AkReal32			fDelayTime;
	bool				bProcessLFE;
	bool				bSyncDry;

	AkPitchShifterFXParams()
	{
		Voice.Filter.eFilterType = AKFILTERTYPE_NONE;
		Voice.Filter.fFilterGain = 1.f;
		Voice.Filter.fFilterFrequency = 1000.f;
		Voice.Filter.fFilterQFactor = 1.f;
		Voice.fPitchFactor = 1.f;
		eInputType = AKINPUTTYPE_ASINPUT;
		fDryLevel = 1.f;
		fWetLevel = 1.f;
		fDelayTime = 50.f;
		bProcessLFE = false;
		bSyncDry = false;
	}
};

class CAkPitchShifterFXParams : public AK::IAkPluginParam
{
public:
	friend class CAkPitchShifterFX;

	CAkPitchShifterFXParams();
	CAkPitchShifterFXParams( const CAkPitchShifterFXParams & in_rCopy );
	~CAkPitchShifterFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );

	void GetParams( AkPitchShifterFXParams * out_pParams );

	AK::AkFXParameterChangeHandler<AKPITCHSHIFTERPARAMID_NUM> m_ParamChangeHandler;

private:
	AkPitchShifterFXParams m_Params;
};

static_assert( sizeof( AkVoiceFilterParams ) == 16, "AkVoiceFilterParams: 2012 PDB sizeof 16" );
static_assert( sizeof( AkPitchVoiceParams ) == 20, "AkPitchVoiceParams: 2012 PDB sizeof 20" );
static_assert( sizeof( AkPitchShifterFXParams ) == 40, "AkPitchShifterFXParams: 2012 PDB sizeof 40" );
static_assert( sizeof( CAkPitchShifterFXParams ) == 48, "CAkPitchShifterFXParams: 2012 PDB sizeof 48" );

#endif
