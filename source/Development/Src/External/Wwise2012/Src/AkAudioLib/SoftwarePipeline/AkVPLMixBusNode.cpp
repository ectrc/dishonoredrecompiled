/*=============================================================================
	AkVPLMixBusNode.cpp - CAkBusFX (0x93ac30..0x93b430), CAkVPLMixBusNode (0x93acc0..0x93b660), CAkVPLFinalMixNode
	(0x93e000..0x93e320); 2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26). Deviations named in AkVPLMixBusNode.h (effect slots: IH4; environment
	volumes: IH3/IH4).
=============================================================================*/
#include "AkVPLMixBusNode.h"
#include "AkLEngine.h"
#include "AkCommon.h"
#include "AkMath.h"

/** The slots' reset, inlined in both Init (0x93adc0, 0x93e000). */
void CAkBusFX::InitFX()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_aFX[ i ].id = AK_INVALID_UNIQUE_ID;
		m_aFX[ i ].pParam = NULL;
		m_aFX[ i ].pEffect = NULL;
		m_aFX[ i ].pBusFXContext = NULL;
		m_aFX[ i ].bBypass = 0;
		m_aFX[ i ].bLastBypass = 0;
	}
	m_bBypassAllFX = 0;
	m_bLastBypassAllFX = 0;
}

/** 2012 0x93ac30: bits 0..3 the slots, bit 4 the whole bus. */
void CAkBusFX::SetInsertFxBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( in_uTargetMask & ( 1 << i ) )
		{
			m_aFX[ i ].bBypass = ( in_bitsFXBypass >> i ) & 1;
		}
	}
	if( in_uTargetMask & 0x10 )
	{
		m_bBypassAllFX = ( in_bitsFXBypass >> 4 ) & 1;
	}
}

/** 2012 0x93b430. DISHONORED(IH4 seam): CAkBusCtx::GetFX finds no effect plug-in, so the slot is dropped and left empty
    (the retail path for a bus without an effect in that slot). */
AKRESULT CAkBusFX::SetInsertFx( const AK::CAkBusCtx & in_BusContext, AkUInt32 in_uFXIndex, AkUniqueID in_EnvID )
{
	DropFx( in_uFXIndex );
	return AK_Success;
}

/** 2012 0x93ad30: nothing to terminate in an empty slot. */
void CAkBusFX::DropFx( AkUInt32 in_uFXIndex )
{
	FX & rFX = m_aFX[ in_uFXIndex ];
	rFX.pEffect = NULL;
	rFX.pBusFXContext = NULL;
	rFX.id = AK_INVALID_UNIQUE_ID;
	rFX.pParam = NULL;
}

/** 2012 0x93b380. */
void CAkBusFX::DropFx()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		DropFx( i );
	}
}

/** 2012 0x93adc0: the bus (and its output chain) counts this node as activity; the output buffer is zeroed. */
AKRESULT CAkVPLMixBusNode::Init( AkUInt32 in_uChannelMask, AkUInt16 in_uMaxFrames, AK::CAkBusCtx in_busContext )
{
	m_Mixer.Init( in_uMaxFrames );
	m_BusContext = in_busContext;
	if( in_busContext.m_pBus )
	{
		in_busContext.m_pBus->AddRef();
		if( !m_BusContext.m_pBus->IncrementActivityCount( 3 ) )
		{
			return AK_Fail;
		}
	}
	InitFX();
	m_fPreviousVolume = 1.f;
	m_fNextVolume = 1.f;
	m_sPreviousEnvVolumes.fFrontLeft = m_sPreviousEnvVolumes.fFrontRight = m_sPreviousEnvVolumes.fCenter = 1.f;
	m_sPreviousEnvVolumes.fRearLeft = m_sPreviousEnvVolumes.fRearRight = m_sPreviousEnvVolumes.fLfe = 1.f;
	m_eState = NodeStateIdle;
	m_fNextVolumedB = 0.f;
	m_EnvID = AK_INVALID_UNIQUE_ID;
	m_uConnectCount = 0;
	m_ulBufferOutSize = AkGetNumChannels( in_uChannelMask ) * in_uMaxFrames * sizeof( AkReal32 );
	m_BufferOut.pData = NULL;
	m_BufferOut.uValidFrames = 0;
	m_BufferOut.uMaxFrames = 0;
	m_BufferOut.eState = AK_NoMoreData;
	void * pData = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, m_ulBufferOutSize );
	if( !pData )
	{
		return AK_InsufficientMemory;
	}
	memset( pData, 0, m_ulBufferOutSize );
	m_BufferOut.pData = pData;
	m_BufferOut.uMaxFrames = in_uMaxFrames;
	m_BufferOut.uValidFrames = 0;
	m_BufferOut.uChannelMask = in_uChannelMask;
	memset( &m_BufferOut.AudioMix, 0, sizeof( m_BufferOut.AudioMix ) );
	return AK_Success;
}

/** 2012 0x93b3a0. */
AKRESULT CAkVPLMixBusNode::Term()
{
	if( m_BusContext.m_pBus )
	{
		m_BusContext.m_pBus->DecrementActivityCount( 3 );
		m_BusContext.m_pBus->Release();
	}
	DropFx();
	if( m_BufferOut.pData )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_BufferOut.pData );
		m_BufferOut.pData = NULL;
	}
	return AK_Success;
}

/** 2012 0x93afb0: a node that mixed nothing this frame goes idle. */
void CAkVPLMixBusNode::ReleaseBuffer()
{
	if( m_BufferOut.pData )
	{
		m_eState = m_BufferOut.eState != AK_NoMoreData ? NodeStatePlay : NodeStateIdle;
		m_BufferOut.eState = AK_NoMoreData;
		m_BufferOut.uValidFrames = 0;
		memset( m_BufferOut.pData, 0, m_ulBufferOutSize );
	}
}

/** 2012 0x93aff0: an idle bus starts its ramp at its current volume. */
void CAkVPLMixBusNode::Connect()
{
	if( m_eState != NodeStatePlay )
	{
		m_fNextVolumedB = m_BusContext.GetVolume( BusVolumeType_ToNextBusWithEffect );
		m_fNextVolume = AkMath::dBToLin( m_fNextVolumedB );
		m_fPreviousVolume = m_fNextVolume;
		m_sPreviousEnvVolumes.fFrontLeft = m_sPreviousEnvVolumes.fFrontRight = m_sPreviousEnvVolumes.fCenter = 1.f;
		m_sPreviousEnvVolumes.fRearLeft = m_sPreviousEnvVolumes.fRearRight = m_sPreviousEnvVolumes.fLfe = 1.f;
	}
	++m_uConnectCount;
}

/** 2012 0x93b0f0: a child bus's buffer. */
void CAkVPLMixBusNode::ConsumeBuffer( AkAudioBufferBus & io_rBuffer )
{
	if( !io_rBuffer.uValidFrames )
	{
		return;
	}
	m_BufferOut.eState = AK_DataReady;
	if( m_eState == NodeStateIdle )
	{
		m_eState = NodeStatePlay;
	}
	ZeroPadBuffer( &io_rBuffer );
	io_rBuffer.uValidFrames = io_rBuffer.uMaxFrames;
	if( io_rBuffer.uChannelMask == AK_SPEAKER_SETUP_STEREO )
	{
		m_Mixer.MixStereo( &io_rBuffer, &m_BufferOut );
	}
	else if( io_rBuffer.uChannelMask == AK_SPEAKER_SETUP_5POINT1 )
	{
		m_Mixer.Mix51( &io_rBuffer, &m_BufferOut );
	}
}

/** 2012 0x93b160: a voice's buffer, with its per-channel speaker volumes. */
void CAkVPLMixBusNode::ConsumeBuffer( AkVPLState & io_rVPLState, AkInt8 in_iVPLEnvironmentIndex )
{
	if( !io_rVPLState.buffer.uValidFrames )
	{
		return;
	}
	m_BufferOut.eState = AK_DataReady;
	if( m_eState == NodeStateIdle )
	{
		m_eState = NodeStatePlay;
	}
	ZeroPadBuffer( &io_rVPLState.buffer );
	io_rVPLState.buffer.uValidFrames = io_rVPLState.buffer.uMaxFrames;
	m_Mixer.Mix3D( &io_rVPLState.buffer, &m_BufferOut );
}

/** 2012 0x93b1b0: the insert effects of a playing bus (none before IH4, see SetInsertFx). */
void CAkVPLMixBusNode::ProcessAllFX()
{
}

/** 2012 0x93b220: the output ramp is the bus volume (times the environment volumes on an aux bus), clamped to 1. */
void CAkVPLMixBusNode::PostProcessFx( AkAudioBufferBus *& io_rpBuffer )
{
	if( m_eState == NodeStatePlay )
	{
		m_bLastBypassAllFX = m_bBypassAllFX;
	}
	AkSIMDSpeakerVolumes env;
	env.Set( 1.f );
	const __m128 vOne = _mm_set1_ps( 1.f );
	const __m128 vNext = _mm_set1_ps( m_fNextVolume );
	const __m128 vPrevious = _mm_set1_ps( m_fPreviousVolume );
	m_BufferOut.AudioMix.Next.vector[ 0 ] = _mm_min_ps( _mm_mul_ps( vNext, env.vector[ 0 ] ), vOne );
	m_BufferOut.AudioMix.Next.vector[ 1 ] = _mm_min_ps( _mm_mul_ps( env.vector[ 1 ], vNext ), vOne );
	m_BufferOut.AudioMix.Previous.vector[ 0 ] = _mm_min_ps( _mm_mul_ps( env.vector[ 0 ], vPrevious ), vOne );
	m_BufferOut.AudioMix.Previous.vector[ 1 ] = _mm_min_ps( _mm_mul_ps( env.vector[ 1 ], vPrevious ), vOne );
	m_fPreviousVolume = m_fNextVolume;
	m_sPreviousEnvVolumes = env.volumes;
	io_rpBuffer = &m_BufferOut;
}

/** 2012 0x93b410. */
void CAkVPLMixBusNode::GetResultingBuffer( AkAudioBufferBus *& io_rpBuffer )
{
	ProcessAllFX();
	PostProcessFx( io_rpBuffer );
}

/** 2012 0x93b660. */
AKRESULT CAkVPLMixBusNode::SetAllInsertFx( AkUniqueID in_EnvID )
{
	m_EnvID = in_EnvID;
	AKRESULT eResult = AK_Success;
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		eResult = SetInsertFx( m_BusContext, i, in_EnvID );
	}
	m_bBypassAllFX = m_BusContext.GetBypassAllFX();
	return eResult;
}

/** 2012 0x93e000: the final mix is always 1024 frames of the sink speaker configuration. */
AKRESULT CAkVPLFinalMixNode::Init( AkUInt32 in_uChannelMask )
{
	m_Mixer.Init( LE_MAX_FRAMES_PER_BUFFER );
	InitFX();
	m_fPreviousVolume = 1.f;
	m_fNextVolume = 1.f;
	m_fNextVolumedB = 0.f;
	m_eState = NodeStateStop;
	m_BufferOut.uValidFrames = 0;
	m_BufferOut.pData = NULL;
	m_BufferOut.uMaxFrames = 0;
	m_BufferOut.eState = AK_NoMoreData;
	m_ulBufferOutSize = AkGetNumChannels( in_uChannelMask ) * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkReal32 );
	void * pData = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, m_ulBufferOutSize );
	if( !pData )
	{
		return AK_InsufficientMemory;
	}
	memset( pData, 0, m_ulBufferOutSize );
	m_BufferOut.pData = pData;
	m_BufferOut.uValidFrames = 0;
	m_BufferOut.uChannelMask = in_uChannelMask;
	m_BufferOut.uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
	return AK_Success;
}

/** 2012 0x93e140. */
AKRESULT CAkVPLFinalMixNode::Term()
{
	if( m_BufferOut.pData )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_BufferOut.pData );
		m_BufferOut.pData = NULL;
	}
	DropFx();
	return AK_Success;
}

/** 2012 0x93e170: Play if something was mixed this frame, else Stop. */
AKRESULT CAkVPLFinalMixNode::ReleaseBuffer()
{
	m_BufferOut.uValidFrames = 0;
	m_eState = m_BufferOut.eState == AK_NoMoreData ? NodeStateStop : NodeStatePlay;
	m_BufferOut.eState = AK_NoMoreData;
	if( m_BufferOut.pData )
	{
		memset( m_BufferOut.pData, 0, m_ulBufferOutSize );
	}
	return AK_Success;
}

/** 2012 0x93e1b0: the master bus volume applies here only when the master bus has effects (else it is folded into
    each voice volume). */
void CAkVPLFinalMixNode::Connect( CAkVPLMixBusNode * in_pInput )
{
	if( m_eState == NodeStatePlay )
	{
		return;
	}
	if( g_MasterBusCtx.HasEffect() )
	{
		m_fNextVolumedB = g_MasterBusCtx.GetVolume( BusVolumeType_ToNextBusWithEffect );
		m_fNextVolume = AkMath::dBToLin( m_fNextVolumedB );
		m_fPreviousVolume = m_fNextVolume;
	}
	else
	{
		m_fPreviousVolume = 1.f;
		m_fNextVolume = 1.f;
		m_fNextVolumedB = 0.f;
	}
}

/** 2012 0x93e320: a stopped final mix re-reads the master bus effects on its first buffer. */
AKRESULT CAkVPLFinalMixNode::ConsumeBuffer( AkAudioBufferBus *& io_rpBuffer )
{
	if( !io_rpBuffer->uValidFrames )
	{
		return AK_Success;
	}
	m_BufferOut.eState = AK_DataReady;
	if( m_eState == NodeStateStop )
	{
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			SetInsertFx( g_MasterBusCtx, i, AK_INVALID_UNIQUE_ID );
		}
	}
	m_eState = NodeStatePlay;
	if( io_rpBuffer->uChannelMask == AK_SPEAKER_SETUP_STEREO )
	{
		m_Mixer.MixStereo( io_rpBuffer, &m_BufferOut );
		return AK_Success;
	}
	if( io_rpBuffer->uChannelMask != AK_SPEAKER_SETUP_5POINT1 )
	{
		return AK_InvalidParameter;
	}
	m_Mixer.Mix51( io_rpBuffer, &m_BufferOut );
	return AK_Success;
}

/** 2012 0x93e210: the master ramp (clamped to 1) and the interleave into the sink buffer. */
void CAkVPLFinalMixNode::GetResultingBuffer( AkPipelineBufferBase & io_rBuffer )
{
	if( m_eState == NodeStatePlay )
	{
		m_bLastBypassAllFX = m_bBypassAllFX;
		io_rBuffer.uValidFrames = m_BufferOut.uMaxFrames;
	}
	m_BufferOut.AudioMix.Next.Set( m_fNextVolume >= 1.f ? 1.f : m_fNextVolume );
	m_BufferOut.AudioMix.Previous.Set( m_fPreviousVolume >= 1.f ? 1.f : m_fPreviousVolume );
	if( m_BufferOut.uChannelMask == AK_SPEAKER_SETUP_STEREO )
	{
		m_Mixer.MixFinalStereo( &m_BufferOut, &io_rBuffer );
	}
	else if( m_BufferOut.uChannelMask == AK_SPEAKER_SETUP_5POINT1 )
	{
		m_Mixer.MixFinal51( &m_BufferOut, &io_rBuffer );
	}
	m_fPreviousVolume = m_fNextVolume;
}
