/*=============================================================================
	AkDelayFXDSP.h - CAkDelayFXDSP (Wwise 2012.1, samples/plugins/akdelay/sources/audioenginefx/akdelayfxdsp.h).
	Agent IH4 (fork F2). 84 bytes as the 2012 PDB: the per-channel delay memory, the tail handler, the previous buffer's
	parameters (the ramps' start points), the processed channel count and the tail length.
=============================================================================*/
#ifndef _AK_DELAYFXDSP_H_
#define _AK_DELAYFXDSP_H_

#include "AkDelayFXParams.h"
#include <AK/DSP/AkDelayLineMemory.h>
#include <AK/Plugin/PluginServices/AkFXTailHandler.h>

class CAkDelayFXDSP
{
public:
	CAkDelayFXDSP();

	void Setup( AkDelayFXParams * pInitialParams, bool in_bIsSendMode, AkUInt32 in_uSampleRate );
	AKRESULT InitDelay( AK::IAkPluginMemAlloc * in_pAllocator, AkDelayFXParams * pParams, AkChannelMask in_uChannelMask );
	void ResetDelay();
	void TermDelay( AK::IAkPluginMemAlloc * in_pAllocator );
	void ComputeTailLength( bool in_bFeedbackEnabled, AkReal32 in_fFeedbackValue );
	void Process( AkAudioBuffer * io_pBuffer, AkDelayFXParams * pCurrentParams );

private:
	AK::DSP::CAkDelayLineMemoryStream<AkReal32, 6>	m_DelayMem;
	AkFXTailHandler									m_FXTailHandler;
	AkDelayFXParams									m_PreviousParams;
	AkUInt32										m_uNumProcessedChannels;
	AkUInt32										m_uSampleRate;
	AkUInt32										m_uTailLength;
	bool											m_bSendMode;
	bool											m_bProcessLFE;
};

static_assert( sizeof( CAkDelayFXDSP ) == 84, "CAkDelayFXDSP: 2012 PDB sizeof 84" );

#endif
