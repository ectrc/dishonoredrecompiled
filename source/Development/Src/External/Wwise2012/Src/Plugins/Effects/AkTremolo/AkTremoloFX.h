/*=============================================================================
	AkTremoloFX.h - CAkTremoloFX (Wwise 2012.1, plugins/effects/aktremolo/aktremolofx.h). Agent IH4 (fork F2).
	252 bytes as the 2012 PDB (AkTremoloFXInfo 92 + the multichannel LFO 148).
=============================================================================*/
#ifndef _AK_TREMOLOFX_H_
#define _AK_TREMOLOFX_H_

#include "AkTremoloFXParams.h"
#include "LFOMultiChannel.h"

struct AkTremoloFXInfo
{
	AkTremoloFXParams	Params;
	AkTremoloFXParams	PrevParams;
	AkUInt32			uSampleRate;
};

class CAkTremoloFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkTremoloFX();
	~CAkTremoloFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	void SetupLFO( AkUInt32 in_uChannelMask );
	void LiveParametersUpdate( AkAudioBuffer * io_pBuffer );
	void RTPCParametersUpdate();

	CAkTremoloFXParams *				m_pSharedParams;
	AK::IAkPluginMemAlloc *				m_pAllocator;
	AkTremoloFXInfo						m_FXInfo;
	DSP::MultiChannelLFO<DSP::Unipolar>	m_lfo;
};

static_assert( sizeof( AkTremoloFXInfo ) == 92, "AkTremoloFXInfo: 2012 PDB sizeof 92" );
static_assert( sizeof( CAkTremoloFX ) == 252, "CAkTremoloFX: 2012 PDB sizeof 252" );

#endif
