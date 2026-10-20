/*=============================================================================
	AkTimeStretchFX.h - CAkTimeStretchFX (Wwise 2012.1, plugins/effects/aktimestretch/aktimestretchfx.h). Agent IH4 (fork F2).
	An out-of-place effect: AkTimeStretchFXInfo 612, CAkTimeStretchFX 628 bytes as the 2012 PDB.
=============================================================================*/
#ifndef _AK_TIMESTRETCHFX_H_
#define _AK_TIMESTRETCHFX_H_

#include "AkTimeStretchFXParams.h"
#include "AkPhaseVocoder.h"

struct AkTimeStretchFXInfo
{
	AkTimeStretchFXParams						Params;
	AkTimeStretchFXParams						PrevParams;
	AkUInt32									uNumChannels;
	AkUInt32									uSampleRate;
	AkReal32									fTSRandomOffset;
	DSP::AkFFTAllButterflies::CAkPhaseVocoder	PhaseVocoder;
	AkUInt8										uNoTSModeCount;
	bool										bCanEnterNoTSMode;
};

class CAkTimeStretchFX : public AK::IAkOutOfPlaceEffectPlugin
{
public:
	CAkTimeStretchFX();
	~CAkTimeStretchFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * in_pBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * out_pBuffer );

private:
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames );
	void Bypass( AkAudioBuffer * in_pBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * out_pBuffer );

	CAkTimeStretchFXParams *	m_pSharedParams;
	AK::IAkPluginMemAlloc *		m_pAllocator;
	AkTimeStretchFXInfo			m_FXInfo;
	AkReal32					m_fSkippedFrames;
};

static_assert( sizeof( AkTimeStretchFXInfo ) == 612, "AkTimeStretchFXInfo: 2012 PDB sizeof 612" );
static_assert( sizeof( CAkTimeStretchFX ) == 628, "CAkTimeStretchFX: 2012 PDB sizeof 628" );

#endif
