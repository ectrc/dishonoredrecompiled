/*=============================================================================
	AkSrcPhysModel.cpp - CAkSrcPhysModel (aksrcphysmodel.cpp, 2012 0x956b70..0x956f00; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26).
	Deviation (named, IH4 seam): source plug-ins do not exist yet - no AK::IAkSourcePlugin / IAkPluginParam interface,
	no CAkSourceFXContext, no CAkEffectsMgr::Alloc (plug-in registration only stores the factories). StartStream
	therefore takes retail's own failure path right where retail calls CAkEffectsMgr::Alloc: the FX custom object is
	looked up and released, the node stops and AK_Fail goes back to FetchStreamedData, which fails the PBI exactly as a
	missing plug-in does in retail. m_pEffect / m_pParam / m_pSourceFXContext therefore stay NULL, and every member
	below is retail's code for that state; the branches that call into a live plug-in (Execute, Reset, TimeSkip,
	GetDuration, StopLooping, Seek, Term, the RTPC unsubscription) arrive with IH4.
=============================================================================*/
#include "AkSrcPhysModel.h"
#include "AkLEngine.h"
#include "AkAudioLibIndex.h"
#include "AkAudioLibGlobals.h"
#include "AkHierarchy.h"

/** 2012 0x956e70. */
CAkSrcPhysModel::CAkSrcPhysModel( CAkPBI * in_pCtx )
	: CAkVPLSrcNode( in_pCtx )
	, m_FXID( AK_INVALID_PLUGINID )
	, m_pParam( NULL )
	, m_pEffect( NULL )
	, m_pSourceFXContext( NULL )
{
}

/** 2012 0x956f00. */
AKRESULT CAkSrcPhysModel::StartStream()
{
	CAkFxCustom * pFx = g_pIndex->m_idxFxCustom.GetPtrAndAddRef( m_pCtx->m_pSource->m_sSrcTypeInfo.mediaInfo.sourceID );
	if( pFx )
	{
		m_FXID = pFx->m_FXID;
	}
	StopStream();
	if( pFx )
	{
		pFx->Release();
	}
	return AK_Fail;
}

/** 2012 0x956df0. */
void CAkSrcPhysModel::StopStream()
{
	ReleaseBuffer();
}

/** 2012 0x956b70: without a plug-in the buffer is cleared and the source fails. */
void CAkSrcPhysModel::GetBuffer( AkVPLState & io_state )
{
	io_state.buffer.uMaxFrames = 0;
	io_state.buffer.pMarkers = NULL;
	io_state.buffer.pData = NULL;
	io_state.buffer.uValidFrames = 0;
	io_state.buffer.eState = AK_DataNeeded;
	io_state.buffer.uNumMarkers = 0;
	io_state.buffer.posInfo.Clear();
	io_state.result = AK_Fail;
}

/** 2012 0x956c90. */
void CAkSrcPhysModel::ReleaseBuffer()
{
	if( !m_pluginBuffer.pData )
	{
		return;
	}
	if( m_AudioFormat.uInterleaveID == AK_NONINTERLEAVED )
	{
		m_pluginBuffer.ReleaseCachedBuffer();
	}
	else
	{
		CAkLEngine::ReleaseCachedAudioBuffer( m_AudioFormat.uBlockAlign * LE_MAX_FRAMES_PER_BUFFER, m_pluginBuffer.pData );
		m_pluginBuffer.pData = NULL;
	}
}

/** 2012 0x956ce0. */
void CAkSrcPhysModel::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
}

/** 2012 0x956d00. */
AKRESULT CAkSrcPhysModel::TimeSkip( AkUInt32 & io_uFrames )
{
	return AK_DataReady;
}

/** 2012 0x956d30. */
AkReal32 CAkSrcPhysModel::GetDuration() const
{
	return 0.f;
}

/** 2012 0x956d50. */
AKRESULT CAkSrcPhysModel::StopLooping()
{
	return AK_Fail;
}

/** 2012 0x956d70: the seek is consumed; without a plug-in there is nothing to seek. */
AKRESULT CAkSrcPhysModel::ChangeSourcePosition()
{
	m_pCtx->SetSourceOffsetRemainder( 0 );
	return AK_Fail;
}
