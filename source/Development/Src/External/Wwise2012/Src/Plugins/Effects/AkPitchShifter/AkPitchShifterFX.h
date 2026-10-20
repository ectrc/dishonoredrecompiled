/*=============================================================================
	AkPitchShifterFX.h - CAkPitchShifterFX (Wwise 2012.1, plugins/effects/akpitchshifter/akpitchshifterfx.h). Agent IH4 (fork F2).
	AkPitchShifterFXInfo is 432 bytes, CAkPitchShifterFX 444 bytes as the 2012 PDB.
=============================================================================*/
#ifndef _AK_PITCHSHIFTERFX_H_
#define _AK_PITCHSHIFTERFX_H_

#include "AkPitchShifterFXParams.h"
#include "AkDelayPitchShift.h"
#include "MultiChannelBiquadFilter.h"
#include "DelayLines.h"
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>

struct AkPitchShifterFXInfo
{
	AK::DSP::AkDelayPitchShift				PitchShifter;
	AK::DSP::MultiChannelBiquadFilter<6>	Filter;
	DSP::CDelayLight						DryDelay[ 6 ];
	AkFXTailHandler							FXTailHandler;
	AkPitchShifterFXParams					Params;
	AkPitchShifterFXParams					PrevParams;
	AkUInt32								uNumProcessedChannels;
	AkChannelMask							eProcessChannelMask;
	AkUInt32								uTotalNumChannels;
	AkUInt32								uSampleRate;
	AkUInt32								uTailLength;
	bool									bSendMode;
};

void AkPitchShifterDSPProcess( AkAudioBuffer * io_pBuffer, AkPitchShifterFXInfo & io_FXInfo, AkReal32 * in_pfBufferStorage, void * in_pScratch );

class CAkPitchShifterFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkPitchShifterFX();
	~CAkPitchShifterFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	void ComputeNumProcessedChannels( AkChannelMask in_eChannelMask );
	void ComputeTailLength();
	AKRESULT InitPitchVoice();
	void TermPitchVoice();
	void ResetPitchVoice();
	AKRESULT InitDryDelay();
	void TermDryDelay();
	void ResetDryDelay();

	CAkPitchShifterFXParams *	m_pParams;
	AK::IAkPluginMemAlloc *		m_pAllocator;
	AkPitchShifterFXInfo		m_FXInfo;
};

static_assert( sizeof( AkPitchShifterFXInfo ) == 432, "AkPitchShifterFXInfo: 2012 PDB sizeof 432" );
static_assert( sizeof( CAkPitchShifterFX ) == 444, "CAkPitchShifterFX: 2012 PDB sizeof 444" );

#endif
