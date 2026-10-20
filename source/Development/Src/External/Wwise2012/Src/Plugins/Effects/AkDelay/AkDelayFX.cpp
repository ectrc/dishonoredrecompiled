/*=============================================================================
	AkDelayFX.cpp - CAkDelayFX, Wwise 2012.1 (samples/plugins/akdelay/sources/audioenginefx/akdelayfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. The delay memory is re-created
	when a non-RTPC parameter (delay time, LFE processing) changes, and the tail length is recomputed when the
	feedback changes, both on the audio thread in Execute as retail. TimeSkip is retail's folded "return AK_DataReady"
	(2012 0x98c570).
=============================================================================*/
#include "AkDelayFX.h"

/** 2012 0x97c150, 2013 0x978950. */
AK::IAkPlugin * CreateDelayFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkDelayFX() );
}

/** Inlined into CreateDelayFX 0x97c150. */
CAkDelayFX::CAkDelayFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
{
}

/** 2012 0x97bf70, 2013 0x978770. */
AKRESULT CAkDelayFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pParams = static_cast<CAkDelayFXParams *>( in_pParams );
	m_pAllocator = in_pAllocator;
	m_FXState.Setup( m_pParams, in_pFXCtx->IsSendModeEffect(), in_rFormat.uSampleRate );
	const AKRESULT eResult = m_FXState.InitDelay( in_pAllocator, m_pParams, in_rFormat.uChannelMask );
	m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
	m_pParams->NonRTPC.bHasChanged = false;
	m_pParams->RTPC.bHasChanged = false;
	return eResult;
}

/** 2012 0x97c110, 2013 0x978910. */
AKRESULT CAkDelayFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_FXState.TermDelay( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x97c010, 2013 0x978810. */
AKRESULT CAkDelayFX::Reset()
{
	m_FXState.ResetDelay();
	return AK_Success;
}

/** 2012 0x97c020, 2013 0x978820. */
AKRESULT CAkDelayFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded). */
AKRESULT CAkDelayFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x97c040, 2013 0x978840. */
void CAkDelayFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if( m_pParams->NonRTPC.bHasChanged )
	{
		if( m_FXState.InitDelay( m_pAllocator, m_pParams, io_pBuffer->uChannelMask ) != AK_Success )
		{
			return;
		}
		m_FXState.ResetDelay();
		m_pParams->NonRTPC.bHasChanged = false;
	}
	if( m_pParams->RTPC.bHasChanged )
	{
		m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
		m_pParams->RTPC.bHasChanged = false;
	}
	m_FXState.Process( io_pBuffer, m_pParams );
}
