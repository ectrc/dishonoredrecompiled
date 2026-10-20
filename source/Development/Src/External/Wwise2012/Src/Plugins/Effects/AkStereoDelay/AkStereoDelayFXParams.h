/*=============================================================================
	AkStereoDelayFXParams.h - CAkStereoDelayFXParams (Wwise 2012.1, plugins/effects/akstereodelay/akstereodelayfxparams.h).
	Agent IH4 (fork F2). Layouts are the 2012 PDB's: AkStereoDelayChannelParams 12, AkStereoDelayFilterParams 16,
	AkStereoDelayFXParams 64, CAkStereoDelayFXParams 72 bytes (a 17-parameter change handler, 3 bytes).
	The feedback filter type is 0 for none, else DSP::BiquadFilter::FilterType + 1.
=============================================================================*/
#ifndef _AK_STEREODELAYFXPARAMS_H_
#define _AK_STEREODELAYFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkStereoDelayFXFactory.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

enum AkInputChannelType
{
	AKINPUTCHANNELTYPE_LEFT_OR_RIGHT = 0,
	AKINPUTCHANNELTYPE_CENTER = 1,
	AKINPUTCHANNELTYPE_DOWNMIX = 2,
	AKINPUTCHANNELTYPE_NONE = 3
};

struct AkStereoDelayChannelParams
{
	AkReal32 fDelayTime;
	AkReal32 fFeedback;
	AkReal32 fCrossFeed;
};

struct AkStereoDelayFilterParams
{
	AkUInt32	eFilterType;
	AkReal32	fFilterGain;
	AkReal32	fFilterFrequency;
	AkReal32	fFilterQFactor;
};

static const AkPluginParamID AKSTEREODELAYPARAMID_ENABLECROSSFEED = 0;
static const AkPluginParamID AKSTEREODELAYPARAMID_ENABLEFEEDBACK = 1;
static const AkPluginParamID AKSTEREODELAYPARAMID_DRYLEVEL = 2;
static const AkPluginParamID AKSTEREODELAYPARAMID_WETLEVEL = 3;
static const AkPluginParamID AKSTEREODELAYPARAMID_FRONTREARBALANCE = 4;
static const AkPluginParamID AKSTEREODELAYPARAMID_FILTERTYPE = 5;
static const AkPluginParamID AKSTEREODELAYPARAMID_FILTERGAIN = 6;
static const AkPluginParamID AKSTEREODELAYPARAMID_FILTERFREQUENCY = 7;
static const AkPluginParamID AKSTEREODELAYPARAMID_FILTERQFACTOR = 8;
static const AkPluginParamID AKSTEREODELAYPARAMID_LEFTINPUTTYPE = 9;
static const AkPluginParamID AKSTEREODELAYPARAMID_LEFTDELAYTIME = 10;
static const AkPluginParamID AKSTEREODELAYPARAMID_LEFTFEEDBACK = 11;
static const AkPluginParamID AKSTEREODELAYPARAMID_LEFTCROSSFEED = 12;
static const AkPluginParamID AKSTEREODELAYPARAMID_RIGHTINPUTTYPE = 13;
static const AkPluginParamID AKSTEREODELAYPARAMID_RIGHTDELAYTIME = 14;
static const AkPluginParamID AKSTEREODELAYPARAMID_RIGHTFEEDBACK = 15;
static const AkPluginParamID AKSTEREODELAYPARAMID_RIGHTCROSSFEED = 16;
static const AkUInt32 AKSTEREODELAYPARAMID_NUM = 17;

struct AkStereoDelayFXParams
{
	AkStereoDelayChannelParams	StereoDelayParams[ 2 ];
	AkInputChannelType			eInputType[ 2 ];
	AkStereoDelayFilterParams	FilterParams;
	AkReal32					fDryLevel;
	AkReal32					fWetLevel;
	AkReal32					fFrontRearBalance;
	bool						bEnableFeedback;
	bool						bEnableCrossFeed;

	AkStereoDelayFXParams()
	{
		for( AkUInt32 i = 0; i < 2; ++i )
		{
			StereoDelayParams[ i ].fDelayTime = 0.5f;
			StereoDelayParams[ i ].fFeedback = 0.25f;
			StereoDelayParams[ i ].fCrossFeed = 0.25f;
			eInputType[ i ] = AKINPUTCHANNELTYPE_LEFT_OR_RIGHT;
		}
		FilterParams.eFilterType = 0;
		FilterParams.fFilterGain = 1.f;
		FilterParams.fFilterFrequency = 1000.f;
		FilterParams.fFilterQFactor = 1.f;
		fDryLevel = 1.f;
		fWetLevel = 1.f;
		fFrontRearBalance = 0.f;
		bEnableFeedback = false;
		bEnableCrossFeed = false;
	}
};

class CAkStereoDelayFXParams : public AK::IAkPluginParam
{
public:
	CAkStereoDelayFXParams();
	CAkStereoDelayFXParams( const CAkStereoDelayFXParams & in_rCopy );
	~CAkStereoDelayFXParams();

	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_ulParamSize );
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_ulBlockSize );

	void GetParams( AkStereoDelayFXParams * out_pParams );

	AK::AkFXParameterChangeHandler<AKSTEREODELAYPARAMID_NUM> m_ParamChangeHandler;

private:
	AkStereoDelayFXParams m_Params;
};

static_assert( sizeof( AkStereoDelayChannelParams ) == 12, "AkStereoDelayChannelParams: 2012 PDB sizeof 12" );
static_assert( sizeof( AkStereoDelayFilterParams ) == 16, "AkStereoDelayFilterParams: 2012 PDB sizeof 16" );
static_assert( sizeof( AkStereoDelayFXParams ) == 64, "AkStereoDelayFXParams: 2012 PDB sizeof 64" );
static_assert( sizeof( CAkStereoDelayFXParams ) == 72, "CAkStereoDelayFXParams: 2012 PDB sizeof 72" );

#endif
