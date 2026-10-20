/*=============================================================================
	AkStereoDelayFX.h - CAkStereoDelayFX (Wwise 2012.1, plugins/effects/akstereodelay/akstereodelayfx.h). Agent IH4 (fork F2).
	AkStereoDelayFXInfo 364, CAkStereoDelayFX 376 bytes as the 2012 PDB: a front and a rear stereo delay line.
=============================================================================*/
#ifndef _AK_STEREODELAYFX_H_
#define _AK_STEREODELAYFX_H_

#include "AkStereoDelayLine.h"
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>

struct AkStereoDelayFXInfo
{
	AK::DSP::CStereoDelayLine	StereoDelay[ 2 ];
	AkStereoDelayFXParams		Params;
	AkStereoDelayFXParams		PrevParams;
	AkFXTailHandler				FXTailHandler;
	AkUInt32					uTailLength;
	AkUInt32					uSampleRate;
	bool						bRecomputeFilterCoefs;
	bool						bSendMode;
};

void AkStereoDelayDSPProcess( AkAudioBuffer * io_pBuffer, AkStereoDelayFXInfo & io_FXInfo, AkReal32 * in_pfStereoBufferStorage );

class CAkStereoDelayFX : public AK::IAkInPlaceEffectPlugin
{
public:
	CAkStereoDelayFX();
	~CAkStereoDelayFX();

	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat );
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );
	virtual AKRESULT Reset();
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );
	virtual void Execute( AkAudioBuffer * io_pBuffer );
	virtual AKRESULT TimeSkip( AkUInt32 in_uFrames );

private:
	void ComputeTailLength();
	void SanitizeParams( AkStereoDelayFXParams & io_FXParams );

	CAkStereoDelayFXParams *	m_pParams;
	AK::IAkPluginMemAlloc *		m_pAllocator;
	AkStereoDelayFXInfo			m_FXInfo;
};

static_assert( sizeof( AkStereoDelayFXInfo ) == 364, "AkStereoDelayFXInfo: 2012 PDB sizeof 364" );
static_assert( sizeof( CAkStereoDelayFX ) == 376, "CAkStereoDelayFX: 2012 PDB sizeof 376" );

#endif
