/*=============================================================================
	AkHarmonizerFX.h - CAkHarmonizerFX (Wwise 2012.1, plugins/effects/akharmonizer/akharmonizerfx.h). Agent IH4 (fork F2).
	AkHarmonizerFXInfo 1948, CAkHarmonizerFX 1960 bytes as the 2012 PDB: two resampling phase-vocoder voices, each
	with its biquad, and a sync delay per channel for the dry path.
=============================================================================*/
#ifndef _AK_HARMONIZERFX_H_
#define _AK_HARMONIZERFX_H_

#include "AkHarmonizerFXParams.h"
#include "AkResamplingPhaseVocoder.h"
#include "MultiChannelBiquadFilter.h"
#include "DelayLines.h"
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>

struct AkHarmonizerFXInfo
{
	DSP::AkFFTAllButterflies::CAkResamplingPhaseVocoder	PhaseVocoder[ 2 ];
	AK::DSP::MultiChannelBiquadFilter<6>				Filter[ 2 ];
	DSP::CDelayLight									DryDelay[ 6 ];
	AkHarmonizerFXParams								Params;
	AkHarmonizerFXParams								PrevParams;
	AkFXTailHandler										FXTailHandler;
	AkUInt32											uNumProcessedChannels;
	AkChannelMask										eProcessChannelMask;
	AkUInt32											uTotalNumChannels;
	AkUInt32											uSampleRate;
	bool												bWetPathEnabled;
	bool												bSendMode;
};

void AkHarmonizerDSPProcessVoice( AkReal32 * in_pfCurChan, AkHarmonizerFXInfo & io_FXInfo, AkUInt32 in_uChannelIndex, AkUInt32 in_uVoiceIndex,
	AkReal32 * in_pfMonoPitchedVoice, AkReal32 * in_pfWet, AkUInt32 in_uNumFrames, bool in_bNoMoreData, AkReal32 in_fResamplingFactor, AkReal32 * in_pfPVTDWindow );
void AkHarmonizerDSPProcess( AkAudioBuffer * io_pBuffer, AkHarmonizerFXInfo & io_FXInfo, AkReal32 * in_pfTempStorage );

class CAkHarmonizerFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkHarmonizerFX();
	~CAkHarmonizerFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	void ComputeNumProcessedChannels( AkChannelMask in_eChannelMask );
	void ComputeWetPathEnabled( AkChannelMask in_eChannelMask );
	AKRESULT InitPitchVoices();
	void TermPitchVoices();
	void ResetPitchVoices();
	AKRESULT InitDryDelay();
	void TermDryDelay();
	void ResetDryDelay();

	CAkHarmonizerFXParams *	m_pParams;
	AK::IAkPluginMemAlloc *	m_pAllocator;
	AkHarmonizerFXInfo		m_FXInfo;
};

static_assert( sizeof( AkHarmonizerFXInfo ) == 1948, "AkHarmonizerFXInfo: 2012 PDB sizeof 1948" );
static_assert( sizeof( CAkHarmonizerFX ) == 1960, "CAkHarmonizerFX: 2012 PDB sizeof 1960" );

#endif
