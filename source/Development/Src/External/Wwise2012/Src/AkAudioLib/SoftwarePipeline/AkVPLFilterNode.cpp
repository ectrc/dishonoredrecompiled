/*=============================================================================
	AkVPLFilterNode.cpp - CAkVPLFilterNodeBase (2012 0x94b370..0x94b460), CAkVPLFilterNode (0x94b4c0..0x94b6f0),
	CAkVPLFilterNodeOutOfPlace (0x94b730..0x94bc80); 2013 matched by bytes (0x947c40..0x948530).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Retail behaviour; the plug-in instance comes from the voice's
	FX share set / custom through CAkEffectsMgr (AddPipeline / UpdateFx), its parameter object is a clone of the FX
	object's on AkFXMemAlloc::m_instanceLower, subscribed to the FX object's RTPC curves on the voice's game object.
	A bypassed effect (its own bit or the voice's m_bBypassAllFX) is reset once and passes the buffer through.
=============================================================================*/
#include "AkVPLFilterNode.h"
#include "AkHierarchy.h"
#include "AkRTPCMgr.h"
#include "AkPositionRepository.h"
#include "AkPBI.h"

// ------------------------------------------------------------------------------------------------------------
// CAkVPLFilterNodeBase
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94b370: from the beginning, the effect will be fed again. */
void CAkVPLFilterNodeBase::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	if( eBehavior == AkVirtualQueueBehavior_FromBeginning )
	{
		m_bLast = false;
	}
	if( !m_bLast )
	{
		m_pInput->VirtualOn( eBehavior );
	}
}

/** 2012 0x94b3a0. */
AKRESULT CAkVPLFilterNodeBase::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	if( m_bLast )
	{
		return AK_Success;
	}
	return m_pInput->VirtualOff( eBehavior, in_bUseSourceOffset );
}

/** 2012 0x94b3c0: the voice's own clone of the effect's parameter object, the insert context, the RTPC subscription. */
AKRESULT CAkVPLFilterNodeBase::Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format )
{
	m_uFXIndex = in_uFXIndex;
	m_pCtx = in_pCtx;
	m_pInsertFXContext = NULL;
	m_bLast = false;
	m_bBypassed = false;
	m_LastBypassed = false;
	m_pParam = in_fxDesc.pFx->m_pParam ? in_fxDesc.pFx->m_pParam->Clone( &AkFXMemAlloc::m_instanceLower ) : NULL;
	if( !m_pParam )
	{
		return AK_Fail;
	}
	m_FXID = in_fxDesc.pFx->m_FXID;
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkInsertFXContext ) );
	m_pInsertFXContext = pMem ? AkPlacementNew( pMem ) CAkInsertFXContext( in_pCtx, in_uFXIndex ) : NULL;
	if( !m_pInsertFXContext )
	{
		return AK_Fail;
	}
	in_fxDesc.pFx->SubscribeRTPC( m_pParam, in_pCtx->GetGameObjectPtr() );
	return AK_Success;
}

/** 2012 0x94b460. */
void CAkVPLFilterNodeBase::Term()
{
	if( m_pParam )
	{
		g_pRTPCMgr->UnSubscribeRTPC( m_pParam );
		m_pParam->Term( &AkFXMemAlloc::m_instanceLower );
		m_pParam = NULL;
	}
	if( m_pInsertFXContext )
	{
		m_pInsertFXContext->~CAkInsertFXContext();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pInsertFXContext );
		m_pInsertFXContext = NULL;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkVPLFilterNode (in place)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94b4c0: once the input ended, the effect is pulled for its tail. */
void CAkVPLFilterNode::GetBuffer( AkVPLState & io_state )
{
	if( m_bLast )
	{
		io_state.result = AK_NoMoreData;
		ConsumeBuffer( io_state );
	}
}

/** 2012 0x94b4f0: a tail (no input buffer) gets a buffer of its own. */
void CAkVPLFilterNode::ConsumeBuffer( AkVPLState & io_state )
{
	if( m_bBypassed || m_pCtx->GetBypassAllFX() )
	{
		if( !m_LastBypassed )
		{
			m_pEffect->Reset();
		}
		m_LastBypassed = true;
		return;
	}
	m_LastBypassed = false;
	if( io_state.result == AK_NoMoreData )
	{
		m_bLast = true;
	}
	if( !io_state.buffer.pData )
	{
		m_pAllocatedBuffer = (AkUInt8 *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, io_state.buffer.NumChannels() * io_state.buffer.uMaxFrames * sizeof( AkReal32 ) );
		if( !m_pAllocatedBuffer )
		{
			io_state.result = AK_Fail;
			return;
		}
		io_state.buffer.pData = m_pAllocatedBuffer;
		io_state.buffer.uValidFrames = 0;
	}
	io_state.buffer.eState = io_state.result;
	m_pEffect->Execute( &io_state.buffer );
	io_state.result = io_state.buffer.eState;
}

/** 2012 0x94b5b0. */
void CAkVPLFilterNode::ReleaseBuffer()
{
	if( m_pAllocatedBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pAllocatedBuffer );
		m_pAllocatedBuffer = NULL;
	}
	else
	{
		m_pInput->ReleaseBuffer();
	}
}

/** 2012 0x94b5e0. */
AKRESULT CAkVPLFilterNode::Seek()
{
	m_pEffect->Reset();
	m_bLast = false;
	return m_pInput->Seek();
}

/** 2012 0x94b600. */
void CAkVPLFilterNode::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	if( eBehavior != AkVirtualQueueBehavior_Resume )
	{
		m_pEffect->Reset();
	}
	CAkVPLFilterNodeBase::VirtualOn( eBehavior );
}

/** 2012 0x94b630. */
AKRESULT CAkVPLFilterNode::Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format )
{
	m_pEffect = static_cast<AK::IAkInPlaceEffectPlugin *>( in_pPlugin );
	m_pAllocatedBuffer = NULL;
	AKRESULT eResult = CAkVPLFilterNodeBase::Init( in_pPlugin, in_fxDesc, in_uFXIndex, in_pCtx, io_format );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = m_pEffect->Init( &AkFXMemAlloc::m_instanceLower, m_pInsertFXContext, m_pParam, io_format );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return m_pEffect->Reset();
}

/** 2012 0x94b690. */
void CAkVPLFilterNode::Term()
{
	ReleaseMemory();
	CAkVPLFilterNodeBase::Term();
}

/** 2012 0x94b6b0. */
void CAkVPLFilterNode::ReleaseMemory()
{
	if( m_pEffect )
	{
		m_pEffect->Term( &AkFXMemAlloc::m_instanceLower );
		m_pEffect = NULL;
	}
	if( m_pAllocatedBuffer )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pAllocatedBuffer );
		m_pAllocatedBuffer = NULL;
	}
}

/** 2012 0x94b6f0: AK_NoMoreData (17) once the input ended. */
AKRESULT CAkVPLFilterNode::TimeSkip( AkUInt32 & io_uFrames )
{
	if( m_bLast )
	{
		return AK_NoMoreData;
	}
	if( m_pEffect )
	{
		m_pEffect->TimeSkip( io_uFrames );
	}
	return m_pInput->TimeSkip( io_uFrames );
}

// ------------------------------------------------------------------------------------------------------------
// CAkVPLFilterNodeOutOfPlace
// ------------------------------------------------------------------------------------------------------------

static void AkResetPipelineBuffer( AkPipelineBuffer & io_buffer )
{
	io_buffer.pData = NULL;
	io_buffer.eState = AK_DataNeeded;
	io_buffer.uValidFrames = 0;
	io_buffer.uMaxFrames = 0;
	io_buffer.posInfo.Clear();
}

/** 2012 0x94b730: remembers the request size; consumes what is left of its input (or its tail). */
void CAkVPLFilterNodeOutOfPlace::GetBuffer( AkVPLState & io_state )
{
	m_usRequestedFrames = io_state.buffer.uMaxFrames;
	if( m_bLast )
	{
		io_state.result = AK_NoMoreData;
		ConsumeBuffer( io_state );
	}
	else if( m_BufferIn.uValidFrames )
	{
		ConsumeBuffer( io_state );
	}
}

/** 2012 0x94b770: markers and the played position follow the consumed input (averaged rate over 8 buffers). */
void CAkVPLFilterNodeOutOfPlace::ProcessDone( AkVPLState & io_state )
{
	const AkUInt32 uConsumed = m_InputFramesBeforeExec - m_BufferIn.uValidFrames;
	CopyRelevantMarkers( &m_BufferIn, &m_BufferOut, m_uInOffset, uConsumed );
	if( m_pCtx->GetRegisteredNotif() & AK_EnableGetSourcePlayPosition )
	{
		m_uConsumedSinceLastOutput += uConsumed;
		if( m_BufferOut.eState == AK_DataReady || m_BufferOut.eState == AK_NoMoreData )
		{
			const AkUInt32 uStartPos = m_uInOffset + m_BufferIn.posInfo.uStartPos;
			m_fAveragedInput = ( m_fAveragedInput * 7.f + (AkReal32)m_uConsumedSinceLastOutput ) * 0.125f;
			m_fAveragedOutput = ( (AkReal32)m_BufferOut.uValidFrames + m_fAveragedOutput * 7.f ) * 0.125f;
			m_uConsumedSinceLastOutput = 0;
			AkBufferPosInformation bufferPosInfo;
			bufferPosInfo.uSampleRate = m_pCtx->GetMediaFormat().uSampleRate;
			bufferPosInfo.uStartPos = uStartPos;
			bufferPosInfo.uFileEnd = (AkUInt32)-1;
			bufferPosInfo.fLastRate = m_fAveragedInput / m_fAveragedOutput;
			g_pPositionRepository->UpdatePositionInfo( m_pCtx->GetPlayingID(), &bufferPosInfo, this );
		}
	}
	m_uRequestedInputFrames += uConsumed;
	m_uInOffset += uConsumed;
	m_uConsumedInputFrames = m_uRequestedInputFrames;
	if( !m_BufferIn.uValidFrames )
	{
		m_pInput->ReleaseBuffer();
		m_BufferIn.FreeMarkers();
		AkResetPipelineBuffer( m_BufferIn );
		io_state.buffer.pData = NULL;
		io_state.buffer.uValidFrames = 0;
	}
	if( m_BufferOut.eState == AK_DataReady || m_BufferOut.eState == AK_NoMoreData )
	{
		m_BufferOut.posInfo = io_state.buffer.posInfo;
		*static_cast<AkPipelineBuffer *>( &io_state.buffer ) = m_BufferOut;
		io_state.result = m_BufferOut.eState;
	}
	else
	{
		io_state.result = m_BufferOut.eState;
	}
}

/** 2012 0x94b930. */
void CAkVPLFilterNodeOutOfPlace::ReleaseBuffer()
{
	if( m_BufferOut.pData )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_BufferOut.pData );
		m_BufferOut.pMarkers = NULL;
		m_BufferOut.uNumMarkers = 0;
		AkResetPipelineBuffer( m_BufferOut );
	}
}

/** 2012 0x94b990. */
AKRESULT CAkVPLFilterNodeOutOfPlace::Seek()
{
	m_pEffect->Reset();
	m_pInput->ReleaseBuffer();
	m_BufferIn.FreeMarkers();
	AkResetPipelineBuffer( m_BufferIn );
	m_bLast = false;
	return m_pInput->Seek();
}

/** 2012 0x94ba00. */
void CAkVPLFilterNodeOutOfPlace::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	if( eBehavior != AkVirtualQueueBehavior_Resume )
	{
		m_pEffect->Reset();
		ReleaseInputBuffer();
	}
	CAkVPLFilterNodeBase::VirtualOn( eBehavior );
}

/** 2012 0x94ba30: the output takes the channel mask the effect sets in the format. */
AKRESULT CAkVPLFilterNodeOutOfPlace::Init( AK::IAkPlugin * in_pPlugin, const AkFXDesc & in_fxDesc, AkUInt32 in_uFXIndex, CAkPBI * in_pCtx, AkAudioFormat & io_format )
{
	m_pEffect = static_cast<AK::IAkOutOfPlaceEffectPlugin *>( in_pPlugin );
	m_BufferIn.pMarkers = NULL;
	m_BufferIn.uNumMarkers = 0;
	AkResetPipelineBuffer( m_BufferIn );
	m_BufferOut.pMarkers = NULL;
	m_BufferOut.uNumMarkers = 0;
	AkResetPipelineBuffer( m_BufferOut );
	m_uRequestedInputFrames = 0;
	m_uConsumedInputFrames = 0;
	m_uConsumedSinceLastOutput = 0;
	m_fAveragedInput = 1.f;
	m_fAveragedOutput = 1.f;
	AKRESULT eResult = CAkVPLFilterNodeBase::Init( in_pPlugin, in_fxDesc, in_uFXIndex, in_pCtx, io_format );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = m_pEffect->Init( &AkFXMemAlloc::m_instanceLower, m_pInsertFXContext, m_pParam, io_format );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_BufferOut.uChannelMask = io_format.uChannelMask;
	return m_pEffect->Reset();
}

/** 2012 0x94bb20. */
void CAkVPLFilterNodeOutOfPlace::Term()
{
	ReleaseMemory();
	CAkVPLFilterNodeBase::Term();
	if( m_BufferOut.pData )
	{
		m_BufferOut.FreeMarkers();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_BufferOut.pData );
		m_BufferOut.pData = NULL;
		m_BufferOut.uMaxFrames = 0;
		m_BufferOut.uValidFrames = 0;
	}
	m_BufferIn.FreeMarkers();
}

/** 2012 0x94bb70. */
void CAkVPLFilterNodeOutOfPlace::ReleaseMemory()
{
	if( m_pEffect )
	{
		m_pEffect->Term( &AkFXMemAlloc::m_instanceLower );
		m_pEffect = NULL;
	}
}

/** 2012 0x94bb90: the effect says how much input the skipped output needs; the input skips it in whole buffers. */
AKRESULT CAkVPLFilterNodeOutOfPlace::TimeSkip( AkUInt32 & io_uFrames )
{
	if( m_bLast )
	{
		return AK_NoMoreData;
	}
	AkUInt32 uFrames = io_uFrames;
	AKRESULT eResult = m_pEffect->TimeSkip( uFrames );
	m_uRequestedInputFrames += uFrames;
	AkUInt32 uToSkip = m_uRequestedInputFrames - m_uConsumedInputFrames;
	while( uToSkip >= LE_MAX_FRAMES_PER_BUFFER && eResult == AK_DataReady )
	{
		AkUInt32 uFramesToProduce = LE_MAX_FRAMES_PER_BUFFER;
		eResult = m_pInput->TimeSkip( uFramesToProduce );
		m_uConsumedInputFrames += uFramesToProduce;
		uToSkip -= uFramesToProduce;
		m_bLast = eResult == AK_NoMoreData;
	}
	return eResult;
}

/** 2012 0x94bc30. */
bool CAkVPLFilterNodeOutOfPlace::ReleaseInputBuffer()
{
	m_pInput->ReleaseBuffer();
	m_BufferIn.FreeMarkers();
	m_BufferIn.uNumMarkers = 0;
	m_BufferIn.pMarkers = NULL;
	AkResetPipelineBuffer( m_BufferIn );
	return true;
}

/** 2012 0x94bc80: the input buffer is kept until the effect consumed it all; bypassed, the input is copied channel by
    channel into the output layout (missing channels zeroed). */
void CAkVPLFilterNodeOutOfPlace::ConsumeBuffer( AkVPLState & io_state )
{
	if( io_state.result == AK_NoMoreData )
	{
		m_bLast = true;
	}
	if( !m_BufferIn.uValidFrames )
	{
		m_uInOffset = 0;
		m_BufferIn = io_state.buffer;
		CopyRelevantMarkers( &io_state.buffer, &m_BufferIn, 0, io_state.buffer.uValidFrames );
	}
	m_BufferIn.eState = io_state.result;
	if( !m_BufferOut.pData )
	{
		void * pData = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, m_BufferOut.NumChannels() * m_usRequestedFrames * sizeof( AkReal32 ) );
		if( !pData )
		{
			io_state.result = AK_Fail;
			return;
		}
		m_BufferOut.uMaxFrames = m_usRequestedFrames;
		m_BufferOut.pData = pData;
		m_BufferOut.uValidFrames = 0;
	}
	m_InputFramesBeforeExec = m_BufferIn.uValidFrames;
	if( m_bBypassed || m_pCtx->GetBypassAllFX() )
	{
		if( !m_LastBypassed )
		{
			m_pEffect->Reset();
		}
		m_LastBypassed = true;
		const AkUInt32 uChannelMaskIn = m_BufferIn.uChannelMask;
		const AkUInt32 uChannelMaskOut = m_BufferOut.uChannelMask;
		const AkUInt32 uNumFrames = m_BufferIn.uValidFrames < m_BufferOut.uMaxFrames ? m_BufferIn.uValidFrames : m_BufferOut.uMaxFrames;
		AkUInt32 uChannelIdxIn = 0;
		AkUInt32 uChannelIdxOut = 0;
		for( AkUInt32 uBit = 1; uBit <= AK_SPEAKER_BACK_RIGHT; uBit <<= 1 )
		{
			if( uChannelMaskOut & uBit )
			{
				AkReal32 * pOut = (AkReal32 *)m_BufferOut.pData + uChannelIdxOut * m_BufferOut.uMaxFrames;
				if( uChannelMaskIn & uBit )
				{
					memcpy( pOut, (AkReal32 *)m_BufferIn.pData + uChannelIdxIn * m_BufferIn.uMaxFrames, uNumFrames * sizeof( AkReal32 ) );
				}
				else
				{
					memset( pOut, 0, uNumFrames * sizeof( AkReal32 ) );
				}
			}
			if( uChannelMaskIn & uBit )
			{
				++uChannelIdxIn;
			}
			if( uChannelMaskOut & uBit )
			{
				++uChannelIdxOut;
			}
		}
		m_BufferOut.uValidFrames = m_BufferIn.uValidFrames;
		m_BufferIn.uValidFrames = 0;
		if( m_bLast )
		{
			m_BufferOut.eState = AK_NoMoreData;
		}
		else
		{
			m_BufferOut.eState = m_BufferOut.uValidFrames == m_BufferOut.uMaxFrames ? AK_DataReady : AK_DataNeeded;
		}
	}
	else
	{
		m_LastBypassed = false;
		m_pEffect->Execute( &m_BufferIn, m_uInOffset, &m_BufferOut );
	}
	ProcessDone( io_state );
}
