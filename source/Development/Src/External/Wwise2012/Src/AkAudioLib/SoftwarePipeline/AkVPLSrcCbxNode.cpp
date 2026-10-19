/*=============================================================================
	AkVPLSrcCbxNode.cpp - CAkVPLSrcCbxNode (akvplsrccbxnode.cpp, 2012 0x9256d0..0x927580), CAkVPLPitchNode
	(akvplpitchnode.cpp, 0x926970, 0x941620..0x941da0), CAkVPLLPFNode (akvpllpfnode.cpp, 0x93a4c0..0x93a510);
	2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * GetVolumes, 3D branch (IH3 seam: CAkPBI::Get3DPositioningParams + CAkListener::Get3DVolumes): a 3D voice is
	    mixed as the 2D branch mixes it (its base pan / center %, its effective volume, no distance attenuation, cone
	    or obstruction); the voice LPF is the effective LPF and the obstruction LPF 0.
	  * GetVolumes: no feedback (motion) volumes - no feedback device exists; the AK_SpeakerVolumeMatrix callback
	    (CAkListener::DoSpeakerVolumeMatrixCallback) is IH3's and not called.
	  * AddPipeline / RefreshBypassFx: GetFX (the sound's insert effects) is IH4's; no effect is inserted, the four
	    filter slots stay empty and the refresh has nothing to refresh.
	  * StartRun's virtual voice: CAkListener::IsInitiallyUnderThreshold (AddSrc) is IH3's positioning; a voice is
	    never initially under the threshold, so AddSrc always fetches the stream.
=============================================================================*/
#include "AkVPLSrcCbxNode.h"
#include "AkLEngine.h"
#include "AkCommon.h"
#include "AkSpeakerPan.h"
#include "AkMath.h"
#include "AkPositionRepository.h"
#include "AkPBI.h"

AkReal32 g_fVolumeThreshold = 1.5258789e-05f;
AkReal32 g_fVolumeThresholdDB = -96.3f;

// ------------------------------------------------------------------------------------------------------------
// CAkVPLPitchNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x941a60. */
void CAkVPLPitchNode::Init( AkAudioFormat * io_pFormat, CAkPBI * in_pPBI, AkUInt32 in_usSampleRate )
{
	m_pPBI = in_pPBI;
	m_bLast = false;
	m_bPadFrameOffset = true;
	m_Pitch.Init( io_pFormat, in_usSampleRate );
}

/** 2012 0x941a90. */
AKRESULT CAkVPLPitchNode::Term()
{
	if( m_BufferOut.pData )
	{
		m_BufferOut.FreeMarkers();
		m_BufferOut.ReleaseCachedBuffer();
	}
	m_BufferIn.FreeMarkers();
	m_pPBI = NULL;
	return AK_Success;
}

/** 2012 0x941720. */
void CAkVPLPitchNode::ReleaseBuffer()
{
	if( !m_BufferOut.pData )
	{
		return;
	}
	m_BufferOut.ReleaseCachedBuffer();
	m_BufferOut.uValidFrames = 0;
	m_BufferOut.pMarkers = NULL;
	m_BufferOut.pData = NULL;
	m_BufferOut.uMaxFrames = 0;
	m_BufferOut.eState = AK_DataNeeded;
	m_BufferOut.uNumMarkers = 0;
	m_BufferOut.posInfo.Clear();
	m_Pitch.SetOutputFrameOffset( 0 );
}

/** 2012 0x941780: the source's buffer goes back and the input buffer is emptied. */
void CAkVPLPitchNode::ReleaseInputBuffer( AkPipelineBuffer & io_buffer )
{
	m_pInput->ReleaseBuffer();
	m_BufferIn.FreeMarkers();
	m_BufferIn.pData = NULL;
	m_BufferIn.uValidFrames = 0;
	m_BufferIn.uMaxFrames = 0;
	m_BufferIn.eState = AK_DataNeeded;
	m_BufferIn.posInfo.Clear();
	io_buffer.pData = NULL;
}

/** 2012 0x941620: the next source of a sample-accurate sequence takes over when its offset has run out and its format
    matches; until then the current voice ends this frame (AK_NoMoreData). */
AKRESULT CAkVPLPitchNode::SwitchToNextSrc()
{
	CAkVPLSrcNode * pNextSrc = m_pCbx->m_pSources[ 1 ];
	CAkPBI * pNextCtx = pNextSrc->m_pCtx;
	if( pNextCtx->m_iFrameOffset > 0 )
	{
		AkInt32 iFrames = m_BufferOut.uMaxFrames - m_BufferOut.uValidFrames;
		if( iFrames >= pNextCtx->m_iFrameOffset )
		{
			iFrames = pNextCtx->m_iFrameOffset;
		}
		pNextCtx->m_iFrameOffset -= iFrames;
		return AK_NoMoreData;
	}
	const AKRESULT eResult = pNextSrc->FetchStreamedData();
	if( eResult == AK_FormatNotReady )
	{
		return AK_NoMoreData;
	}
	if( eResult != AK_Success )
	{
		return AK_Fail;
	}
	const AkAudioFormat fmtNew = pNextCtx->m_sMediaFormat;
	if( fmtNew.uChannelMask != m_pPBI->m_sMediaFormat.uChannelMask )
	{
		return AK_NoMoreData;
	}
	m_pCbx->SwitchToNextSrc();
	m_pPBI = pNextCtx;
	m_pInput = m_pCbx->m_pSources[ 0 ];
	pNextCtx->CalcEffectiveParams();
	m_Pitch.SwitchTo( fmtNew, pNextCtx->m_EffectiveParams.Pitch, &m_BufferOut, m_pCbx->m_uSampleRate );
	m_bLast = false;
	return m_BufferOut.uValidFrames == m_Pitch.m_InternalPitchState.uRequestedFrames ? AK_DataReady : AK_DataNeeded;
}

/** 2012 0x941da0. */
void CAkVPLPitchNode::GetBuffer( AkVPLState & io_state )
{
	m_Pitch.SetRequestedFrames( io_state.buffer.uMaxFrames );
	m_bStartPosInfoUpdated = false;
	m_Pitch.SetPitch( m_pCbx->GetContext()->m_EffectiveParams.Pitch );
	if( m_BufferIn.uValidFrames )
	{
		ConsumeBuffer( io_state );
	}
	else if( m_bLast )
	{
		io_state.result = AK_NoMoreData;
	}
}

/** 2012 0x941b30: resample the source buffer into the output (pre-padded by the frame offset), apply the seek offset. */
void CAkVPLPitchNode::ConsumeBuffer( AkVPLState & io_state )
{
	if( io_state.result == AK_NoMoreData )
	{
		m_bLast = true;
	}
	if( !m_BufferIn.uValidFrames )
	{
		if( !io_state.buffer.uValidFrames && io_state.result == AK_DataReady )
		{
			io_state.result = AK_DataNeeded;
			return;
		}
		m_BufferIn = io_state.buffer;
	}
	if( !m_BufferOut.pData )
	{
		if( m_BufferOut.GetCachedBuffer( (AkUInt16)m_Pitch.m_InternalPitchState.uRequestedFrames, m_BufferIn.uChannelMask ) != AK_Success )
		{
			io_state.result = AK_Fail;
			return;
		}
		if( m_bPadFrameOffset )
		{
			const AkInt32 iPad = m_pPBI->m_iFrameOffset + ( m_pPBI->m_bTargetIsFeedback ? (AkUInt16)( 384000 / AkAudioLibSettings::g_pipelineCoreFrequency ) : AK_NUM_VOICE_REFILL_FRAMES );
			if( iPad > 0 )
			{
				if( m_Pitch.IsPostDeInterleaveRequired() )
				{
					ZeroPrePadBufferInterleaved( &m_BufferOut, iPad );
				}
				else
				{
					ZeroPrePadBuffer( &m_BufferOut, iPad );
				}
				m_Pitch.SetOutputFrameOffset( iPad );
			}
			m_bPadFrameOffset = false;
		}
	}
	if( !m_pPBI->m_bSeekDirty && m_pPBI->m_uSeekPosition )
	{
		const AkUInt32 uSeek = m_pPBI->m_uSeekPosition;
		if( m_BufferIn.uValidFrames <= uSeek )
		{
			m_pPBI->m_bSeekDirty = 0;
			m_pPBI->m_bSeekRelativeToDuration = 0;
			m_pPBI->m_bSnapSeekToMarker = 0;
			m_pPBI->m_uSeekPosition = uSeek - m_BufferIn.uValidFrames;
			m_BufferIn.uValidFrames = 0;
			io_state.buffer.uValidFrames = 0;
			ReleaseInputBuffer( io_state.buffer );
			io_state.result = m_bLast ? AK_NoMoreData : AK_DataNeeded;
			return;
		}
		m_Pitch.SetInputFrameOffset( uSeek );
		m_BufferIn.uValidFrames = (AkUInt16)( m_BufferIn.uValidFrames - uSeek );
		m_pPBI->m_bSeekDirty = 0;
		m_pPBI->m_bSeekRelativeToDuration = 0;
		m_pPBI->m_bSnapSeekToMarker = 0;
		m_pPBI->m_uSeekPosition = 0;
	}
	const AkUInt32 uInFrameOffset = m_Pitch.GetInputFrameOffset();
	const AkUInt16 uValidFramesBefore = m_BufferIn.uValidFrames;
	const AKRESULT eExecuteResult = m_Pitch.Execute( &m_BufferIn, &m_BufferOut );
	CopyRelevantMarkers( &m_BufferIn, &m_BufferOut, uInFrameOffset, (AkUInt16)( uValidFramesBefore - m_BufferIn.uValidFrames ) );
	if( m_BufferIn.posInfo.uStartPos != (AkUInt32)-1 && !m_bStartPosInfoUpdated )
	{
		m_BufferOut.posInfo.fLastRate = m_BufferIn.posInfo.fLastRate;
		m_BufferOut.posInfo.uFileEnd = m_BufferIn.posInfo.uFileEnd;
		m_BufferOut.posInfo.uSampleRate = m_BufferIn.posInfo.uSampleRate;
		m_BufferOut.posInfo.uStartPos = uInFrameOffset + m_BufferIn.posInfo.uStartPos;
		m_bStartPosInfoUpdated = true;
	}
	m_BufferOut.posInfo.fLastRate = m_Pitch.GetLastRate();
	AKRESULT eResult = eExecuteResult;
	if( !m_BufferIn.uValidFrames )
	{
		ReleaseInputBuffer( io_state.buffer );
		if( m_bLast )
		{
			eResult = m_pCbx->m_pSources[ 1 ] ? SwitchToNextSrc() : AK_NoMoreData;
		}
	}
	if( eResult == AK_DataReady || eResult == AK_NoMoreData )
	{
		if( m_Pitch.IsPostDeInterleaveRequired() )
		{
			m_Pitch.DeinterleaveAndSwapOutput( &m_BufferOut );
		}
		static_cast<AkPipelineBuffer &>( io_state.buffer ) = m_BufferOut;
	}
	io_state.result = eResult;
}

/** 2012 0x9417e0: a virtual voice's time passes through the resampler's rate without producing samples. */
AKRESULT CAkVPLPitchNode::TimeSkip( AkUInt32 & io_uFrames )
{
	m_Pitch.SetPitchForTimeSkip( m_pCbx->GetContext()->m_EffectiveParams.Pitch );
	AkUInt32 uFramesToProduce = io_uFrames;
	m_Pitch.TimeOutputToInput( uFramesToProduce );
	AkUInt32 uProducedFrames = 0;
	AKRESULT eResult = AK_DataReady;
	while( uFramesToProduce )
	{
		if( !m_BufferIn.uValidFrames && !m_bLast )
		{
			AkUInt32 uSrcFrames = io_uFrames;
			const AKRESULT eSrcResult = m_pInput->TimeSkip( uSrcFrames );
			if( eSrcResult != AK_DataReady )
			{
				if( eSrcResult != AK_NoMoreData )
				{
					return eSrcResult;
				}
				m_bLast = true;
			}
			AkUInt32 uSeek = m_pPBI->m_bSeekDirty ? 0 : m_pPBI->m_uSeekPosition;
			AkUInt32 uSeekLeft = 0;
			if( uSeek > uSrcFrames )
			{
				uSeekLeft = uSeek - uSrcFrames;
				uSeek = uSrcFrames;
			}
			uSrcFrames -= uSeek;
			m_pPBI->m_bSeekDirty = 0;
			m_pPBI->m_bSeekRelativeToDuration = 0;
			m_pPBI->m_bSnapSeekToMarker = 0;
			m_pPBI->m_uSeekPosition = uSeekLeft;
			m_BufferIn.uValidFrames = (AkUInt16)uSrcFrames;
		}
		const AkUInt32 uFrames = uFramesToProduce < m_BufferIn.uValidFrames ? uFramesToProduce : m_BufferIn.uValidFrames;
		uProducedFrames += uFrames;
		m_BufferIn.uValidFrames = (AkUInt16)( m_BufferIn.uValidFrames - uFrames );
		uFramesToProduce -= uFrames;
		if( !m_BufferIn.uValidFrames && m_bLast )
		{
			eResult = AK_NoMoreData;
			break;
		}
	}
	m_Pitch.TimeInputToOutput( uProducedFrames );
	io_uFrames = uProducedFrames;
	return eResult;
}

/** 2012 0x941920. */
void CAkVPLPitchNode::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	if( eBehavior != AkVirtualQueueBehavior_Resume )
	{
		if( m_BufferIn.pData )
		{
			m_pInput->ReleaseBuffer();
		}
		m_BufferIn.FreeMarkers();
		m_BufferIn.uValidFrames = 0;
		m_BufferIn.uMaxFrames = 0;
		m_BufferIn.pData = NULL;
		m_BufferIn.eState = AK_DataNeeded;
		m_BufferIn.posInfo.Clear();
		m_Pitch.ResetOffsets();
	}
	if( eBehavior == AkVirtualQueueBehavior_FromBeginning )
	{
		m_bLast = false;
	}
	if( !m_bLast )
	{
		m_pInput->VirtualOn( eBehavior );
	}
}

/** 2012 0x9419b0. */
AKRESULT CAkVPLPitchNode::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	if( eBehavior == AkVirtualQueueBehavior_FromElapsedTime )
	{
		m_BufferIn.uValidFrames = 0;
	}
	return m_bLast ? AK_Success : m_pInput->VirtualOff( eBehavior, in_bUseSourceOffset );
}

/** 2012 0x9419f0. */
AKRESULT CAkVPLPitchNode::Seek()
{
	m_pInput->ReleaseBuffer();
	m_BufferIn.FreeMarkers();
	m_BufferIn.uValidFrames = 0;
	m_BufferIn.uMaxFrames = 0;
	m_BufferIn.pData = NULL;
	m_BufferIn.eState = AK_DataNeeded;
	m_BufferIn.posInfo.Clear();
	m_Pitch.ResetOffsets();
	m_bLast = false;
	return m_pInput->Seek();
}

// ------------------------------------------------------------------------------------------------------------
// CAkVPLLPFNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x93a4c0. */
void CAkVPLLPFNode::ConsumeBuffer( AkVPLState & io_state )
{
	if( io_state.buffer.pData )
	{
		m_LPF.Execute( &io_state.buffer );
	}
}

/** 2012 0x93a4e0. */
void CAkVPLLPFNode::ReleaseBuffer()
{
	if( m_pInput )
	{
		m_pInput->ReleaseBuffer();
	}
}

/** 2012 0x93a500. */
AKRESULT CAkVPLLPFNode::TimeSkip( AkUInt32 & io_uFrames )
{
	return m_pInput->TimeSkip( io_uFrames );
}

/** 2012 0x93a510. */
void CAkVPLLPFNode::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	m_LPF.ResetRamp();
	if( m_pInput )
	{
		m_pInput->VirtualOn( eBehavior );
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkVPLSrcCbxNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9256d0. */
CAkVPLSrcCbxNodeBase::CAkVPLSrcCbxNodeBase()
	: pNextItem( NULL )
	, m_eState( NodeStateInit )
	, m_eVirtualBehavior( AkVirtualQueueBehavior_FromBeginning )
	, m_eBelowThresholdBehavior( AkBelowThresholdBehavior_ContinueToPlay )
	, m_bAudible( true )
	, m_bPreviousSilent( true )
	, m_bFirstBufferProcessed( false )
{
	m_pSources[ 0 ] = NULL;
	m_pSources[ 1 ] = NULL;
	memset( m_PreviousDirect, 0, sizeof( m_PreviousDirect ) );
	memset( m_PreviousEnv, 0, sizeof( m_PreviousEnv ) );
	memset( m_LastEnvironmentValues, 0, sizeof( m_LastEnvironmentValues ) );
}

/** 2012 0x926ac0. */
CAkVPLSrcCbxNode::CAkVPLSrcCbxNode()
	: m_uSampleRate( AK_CORE_SAMPLERATE )
	, m_uChannelMask( AK_SPEAKER_SETUP_MONO )
	, m_cbxRec( this )
{
}

/** 2012 0x926a50. */
void AkVPLSrcCbxRec::ClearVPL()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( m_pFilter[ i ] )
		{
			m_pFilter[ i ]->Term();
			m_pFilter[ i ]->~CAkVPLFilterNodeBase();
			AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pFilter[ i ] );
			m_pFilter[ i ] = NULL;
		}
	}
	m_Pitch.Term();
	m_LPF.m_LPF.Term();
}

/** 2012 0x926e00. */
void CAkVPLSrcCbxNode::Term()
{
	RemovePipeline( CtxDestroyReasonFinished );
	if( m_pSources[ 1 ] )
	{
		m_pSources[ 1 ]->Term( CtxDestroyReasonFinished );
		m_pSources[ 1 ]->~CAkVPLSrcNode();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pSources[ 1 ] );
		m_pSources[ 1 ] = NULL;
	}
}

/** 2012 0x926b40. */
void CAkVPLSrcCbxNode::Start()
{
	if( m_eState == NodeStateInit )
	{
		m_pSources[ 0 ]->Start();
		m_eState = NodeStatePlay;
	}
	else if( m_eState != NodeStateStop )
	{
		Stop();
	}
}

/** 2012 0x925760. */
void CAkVPLSrcCbxNode::Stop()
{
	if( m_pSources[ 0 ] )
	{
		m_pSources[ 0 ]->Stop();
	}
	m_eState = NodeStateStop;
}

/** 2012 0x925780: only a playing voice pauses; anything else stops. */
void CAkVPLSrcCbxNode::Pause()
{
	if( m_eState == NodeStatePlay )
	{
		m_pSources[ 0 ]->Pause();
		m_eState = NodeStatePause;
	}
	else
	{
		Stop();
	}
}

/** 2012 0x9257b0. */
void CAkVPLSrcCbxNode::Resume()
{
	if( m_eState == NodeStatePlay )
	{
		return;
	}
	if( m_eState == NodeStatePause )
	{
		m_pSources[ 0 ]->Resume( m_cbxRec.m_Pitch.m_Pitch.GetLastRate() );
		m_eState = NodeStatePlay;
	}
	else
	{
		Stop();
	}
}

/** 2012 0x926d00. */
void CAkVPLSrcCbxNode::StopLooping( CAkPBI * in_pCtx )
{
	if( m_pSources[ 0 ] && in_pCtx == m_pSources[ 0 ]->m_pCtx )
	{
		if( !m_pSources[ 0 ]->m_bIOReady || m_pSources[ 0 ]->StopLooping() != AK_Success )
		{
			Stop();
		}
	}
	else if( m_pSources[ 1 ] && in_pCtx == m_pSources[ 1 ]->m_pCtx )
	{
		m_pSources[ 1 ]->Term( CtxDestroyReasonFinished );
		m_pSources[ 1 ]->~CAkVPLSrcNode();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pSources[ 1 ] );
		m_pSources[ 1 ] = NULL;
	}
}

/** 2012 0x926810: a virtual voice that restarts from the beginning only forgets its seek. */
void CAkVPLSrcCbxNode::SeekSource()
{
	if( m_eVirtualBehavior == AkVirtualQueueBehavior_FromBeginning && m_eBelowThresholdBehavior == AkBelowThresholdBehavior_SetAsVirtualVoice )
	{
		CAkPBI * pCtx = m_pSources[ 0 ] ? m_pSources[ 0 ]->m_pCtx : NULL;
		if( pCtx )
		{
			pCtx->m_bSeekDirty = 0;
			pCtx->m_bSeekRelativeToDuration = 0;
			pCtx->m_bSnapSeekToMarker = 0;
			pCtx->m_uSeekPosition = 0;
		}
		return;
	}
	if( m_pSources[ 0 ] && m_pSources[ 0 ]->m_bIOReady )
	{
		m_cbxRec.m_LPF.ReleaseBuffer();
		if( m_cbxRec.m_LPF.Seek() != AK_Success )
		{
			Stop();
		}
	}
}

/** 2012 0x925800: position info to the repository, an in-buffer stop offset, and the next source's stream. */
void CAkVPLSrcCbxNode::ConsumeBuffer( AkVPLState & io_state )
{
	if( io_state.buffer.posInfo.uStartPos != (AkUInt32)-1 )
	{
		g_pPositionRepository->UpdatePositionInfo( m_pSources[ 0 ]->m_pCtx->GetPlayingID(), &io_state.buffer.posInfo, m_pSources[ 0 ] );
	}
	const AkUInt32 uStopOffset = m_pSources[ 0 ]->m_pCtx->GetAndClearStopOffset();
	if( uStopOffset != (AkUInt32)-1 )
	{
		if( uStopOffset < io_state.buffer.uValidFrames )
		{
			io_state.buffer.uValidFrames = (AkUInt16)uStopOffset;
		}
		io_state.bStop = true;
	}
	if( m_pSources[ 1 ] && m_pSources[ 1 ]->FetchStreamedData() == AK_Fail )
	{
		io_state.result = AK_Fail;
	}
}

/** 2012 0x925870. */
void CAkVPLSrcCbxNode::ReleaseBuffer()
{
	m_cbxRec.m_LPF.ReleaseBuffer();
}

/** 2012 0x925910: a stream that is ready but whose frame offset lies beyond the look-ahead waits (AK_FormatNotReady 63). */
AKRESULT CAkVPLSrcCbxNode::FetchStreamedData( CAkPBI * in_pCtx )
{
	const AKRESULT eResult = m_pSources[ 0 ]->FetchStreamedData();
	if( eResult == AK_FormatNotReady )
	{
		return eResult;
	}
	if( eResult != AK_Success )
	{
		return AK_Fail;
	}
	const AkInt32 iFrames = in_pCtx->m_bTargetIsFeedback ? (AkUInt16)( 384000 / AkAudioLibSettings::g_pipelineCoreFrequency ) : AK_NUM_VOICE_REFILL_FRAMES;
	return in_pCtx->m_iFrameOffset >= (AkInt32)( iFrames * ( g_settings.uContinuousPlaybackLookAhead + 1 ) ) ? AK_FormatNotReady : AK_Success;
}

/** 2012 0x926740: a voice that produced nothing this frame keeps last frame's volumes for the next ramp. */
void CAkVPLSrcCbxNode::RestorePreviousVolumes( AkAudioBufferCbx * io_pBuffer )
{
	CAkPBI * pCtx = GetContext();
	pCtx->m_Prev2DParams.prev2DParams.m_fPAN_RL = 101.f;
	const AkUInt32 uNumChannels = io_pBuffer->NumChannels();
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		m_PreviousDirect[ i ] = io_pBuffer->AudioMix[ i ].Previous.volumes;
		m_PreviousEnv[ i ] = io_pBuffer->EnvMix[ i ].Previous.volumes;
	}
}

/** 2012 0x9268b0. */
void CAkVPLSrcCbxNode::SetLastEnvironmentalValues( const AkEnvironmentValue * in_pValues )
{
	if( in_pValues )
	{
		memcpy( m_LastEnvironmentValues, in_pValues, sizeof( m_LastEnvironmentValues ) );
	}
}

/** 2012 0x9268d0: only the insert effects' media reference a bank slot (none before IH4). */
bool CAkVPLSrcCbxNode::IsUsingThisSlot( const CAkUsageSlot * in_pUsageSlot )
{
	return false;
}

/** 2012 0x925890. */
void CAkVPLSrcCbxNode::SetFxBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( m_cbxRec.m_pFilter[ i ] && ( in_uTargetMask & ( 1 << i ) ) )
		{
			m_cbxRec.m_pFilter[ i ]->m_bBypassed = ( in_bitsFXBypass & ( 1 << i ) ) != 0;
		}
	}
}

/** 2012 0x927210. DISHONORED(IH4 seam): GetFX is IH4's; with no filter inserted there is nothing to refresh. */
void CAkVPLSrcCbxNode::RefreshBypassFx()
{
}

/** 2012 0x926920: becoming inaudible virtualizes the PBI, becoming audible again devirtualizes it. */
void CAkVPLSrcCbxNode::SetAudible( bool in_bAudible )
{
	if( m_pSources[ 0 ] )
	{
		CAkPBI * pCtx = m_pSources[ 0 ]->m_pCtx;
		if( m_bAudible && !in_bAudible )
		{
			pCtx->Virtualize();
		}
		else if( !m_bAudible && in_bAudible )
		{
			pCtx->Devirtualize( true );
		}
	}
	m_bAudible = in_bAudible;
}

/** 2012 0x926e50. */
AKRESULT CAkVPLSrcCbxNode::AddSrc( CAkPBI * in_pCtx, bool in_bActive )
{
	CAkVPLSrcNode * pSrc = CAkVPLSrcNode::Create( in_pCtx );
	return pSrc ? AddSrc( pSrc, in_bActive ) : AK_Fail;
}

/** 2012 0x926b80: the source fetches its stream (or the voice starts virtual); a failed fetch destroys it. */
AKRESULT CAkVPLSrcCbxNode::AddSrc( CAkVPLSrcNode * in_pSrc, bool in_bActive )
{
	CAkPBI * pCtx = in_pSrc->m_pCtx;
	pCtx->m_pCbx = this;
	if( in_bActive )
	{
		m_eBelowThresholdBehavior = pCtx->GetVirtualBehavior( m_eVirtualBehavior );
	}
	const AKRESULT eResult = in_pSrc->FetchStreamedData();
	if( eResult != AK_Success && eResult != AK_FormatNotReady )
	{
		in_pSrc->Term( CtxDestroyReasonPlayFailed );
		in_pSrc->~CAkVPLSrcNode();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, in_pSrc );
		return eResult;
	}
	if( in_bActive )
	{
		m_pSources[ 0 ] = in_pSrc;
		m_bPreviousSilent = pCtx->m_bNeedsFadeIn;
	}
	else
	{
		m_pSources[ 1 ] = in_pSrc;
	}
	return eResult;
}

/** 2012 0x926c60. */
void CAkVPLSrcCbxNode::SwitchToNextSrc()
{
	m_pSources[ 0 ]->Term( CtxDestroyReasonFinished );
	m_pSources[ 0 ]->~CAkVPLSrcNode();
	AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pSources[ 0 ] );
	m_pSources[ 0 ] = m_pSources[ 1 ];
	m_pSources[ 1 ] = NULL;
	m_pSources[ 0 ]->Start();
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( m_cbxRec.m_pFilter[ i ] )
		{
			m_cbxRec.m_pFilter[ i ]->m_pCtx = m_pSources[ 0 ]->m_pCtx;
		}
	}
}

/** 2012 0x927290: source -> pitch -> (insert effects: IH4) -> voice LPF, plus the obstruction LPF on an aux-sending bus. */
AKRESULT CAkVPLSrcCbxNode::AddPipeline( bool in_bIsEnv )
{
	CAkPBI * pCtx = m_pSources[ 0 ]->m_pCtx;
	CAkVPLNode * pNodes[ 7 ];
	AkUInt32 uNumNodes = 0;
	pNodes[ uNumNodes++ ] = m_pSources[ 0 ];
	AkAudioFormat format = pCtx->m_sMediaFormat;
	m_cbxRec.m_Pitch.Init( &format, pCtx, m_uSampleRate );
	format.uSampleRate = m_uSampleRate;
	pNodes[ uNumNodes++ ] = &m_cbxRec.m_Pitch;
	m_uChannelMask = format.uChannelMask;
	AKRESULT eResult = m_cbxRec.m_LPF.m_LPF.Init( m_uChannelMask, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	pNodes[ uNumNodes++ ] = &m_cbxRec.m_LPF;
	if( in_bIsEnv )
	{
		eResult = m_ObstructionLPF.m_LPF.Init( m_uChannelMask, false );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	for( AkUInt32 i = uNumNodes - 1; i > 0; --i )
	{
		pNodes[ i ]->Connect( pNodes[ i - 1 ] );
	}
	RefreshBypassFx();
	return AK_Success;
}

/** 2012 0x926d80. */
void CAkVPLSrcCbxNode::RemovePipeline( AkCtxDestroyReason in_eReason )
{
	if( m_pSources[ 0 ] )
	{
		m_pSources[ 0 ]->Term( in_eReason );
		m_pSources[ 0 ]->~CAkVPLSrcNode();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pSources[ 0 ] );
		m_pSources[ 0 ] = NULL;
	}
	m_cbxRec.ClearVPL();
	m_ObstructionLPF.m_LPF.Term();
	m_bAudible = true;
}

/** 2012 0x926890. */
AKRESULT CAkVPLSrcCbxNode::SourceTimeSkip( AkUInt32 in_uMaxFrames )
{
	return m_cbxRec.m_LPF.TimeSkip( in_uMaxFrames );
}

/** 2012 0x925970: the voice's per-channel mix for this frame (Next), last frame's (Previous) for the ramp, and whether
    the voice is below the volume threshold. See the banner for the IH3 seams (3D, feedback, volume matrix callback). */
void CAkVPLSrcCbxNode::GetVolumes( bool in_bEnvBus, CAkPBI * in_pContext, AkAudioBufferCbx * io_pBuffer, bool & out_bNextSilent )
{
	in_pContext->CalcEffectiveParams();
	const AkUInt32 uNumChannels = io_pBuffer->NumChannels();
	if( !in_pContext->m_bIsForcedToVirtualize )
	{
		AkReal32 fLPF = in_pContext->m_EffectiveParams.LPF;
		m_cbxRec.m_LPF.m_LPF.SetLPFPar( fLPF );
		if( in_pContext->m_p3DSound )
		{
			m_ObstructionLPF.m_LPF.SetLPFPar( 0.f );
		}
		const AkReal32 fDryLevel = in_bEnvBus ? in_pContext->m_pGameObj->m_fDryLevelValue : 0.f;
		const AkReal32 fVolumedB = in_pContext->m_EffectiveParams.Volume;
		out_bNextSilent = g_fVolumeThresholdDB >= fVolumedB;
		Prev2DParams & rPrev = in_pContext->m_Prev2DParams;
		const BaseGenParams & rBase = in_pContext->m_BasePosParams;
		if( rPrev.prev2DParams.m_fPAN_RL == rBase.m_fPAN_RL && rPrev.prev2DParams.m_fPAN_FR == rBase.m_fPAN_FR && rPrev.prev2DParams.m_fCenterPCT == rBase.m_fCenterPCT
			&& rPrev.prev2DParams.bIsPannerEnabled == rBase.bIsPannerEnabled && rPrev.prevVolume == fVolumedB && rPrev.prevDryLevel == fDryLevel )
		{
			for( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				io_pBuffer->AudioMix[ i ].Next.Zero();
				io_pBuffer->AudioMix[ i ].Next.volumes = m_PreviousDirect[ i ];
				if( in_bEnvBus )
				{
					io_pBuffer->EnvMix[ i ].Next.Zero();
					io_pBuffer->EnvMix[ i ].Next.volumes = m_PreviousEnv[ i ];
				}
			}
		}
		else
		{
			rPrev.prev2DParams = rBase;
			rPrev.prevVolume = fVolumedB;
			rPrev.prevDryLevel = fDryLevel;
			const AkReal32 fVolume = AkMath::dBToLin( fVolumedB );
			AkSIMDSpeakerVolumes volumes[ AK_VOICE_MAX_NUM_CHANNELS ];
			CAkSpeakerPan::GetSpeakerVolumes2DPan( ( rBase.m_fPAN_RL + 100.f ) * 0.005f, ( rBase.m_fPAN_FR + 100.f ) * 0.005f, rBase.m_fCenterPCT * 0.01f, rBase.bIsPannerEnabled, io_pBuffer->uChannelMask, volumes );
			const AkUInt32 uNumFullBand = AkGetNumChannels( io_pBuffer->uChannelMask & ~AK_SPEAKER_LOW_FREQUENCY );
			for( AkUInt32 i = 0; i < uNumFullBand; ++i )
			{
				io_pBuffer->AudioMix[ i ].Next.Copy( volumes[ i ] );
				io_pBuffer->AudioMix[ i ].Next.Mul( fVolume );
			}
			if( uNumFullBand != uNumChannels )
			{
				AkSIMDSpeakerVolumes & rLFE = io_pBuffer->AudioMix[ uNumChannels - 1 ].Next;
				rLFE.Zero();
				rLFE.volumes.fLfe = fVolume;
			}
			if( in_bEnvBus )
			{
				for( AkUInt32 i = 0; i < uNumChannels; ++i )
				{
					io_pBuffer->EnvMix[ i ].Next.Copy( io_pBuffer->AudioMix[ i ].Next );
					io_pBuffer->AudioMix[ i ].Next.Mul( fDryLevel );
				}
			}
		}
	}
	else
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pBuffer->AudioMix[ i ].Next.Zero();
			if( in_bEnvBus )
			{
				io_pBuffer->EnvMix[ i ].Next.Zero();
			}
		}
		in_pContext->m_Prev2DParams.prev2DParams.m_fPAN_RL = 101.f;
		out_bNextSilent = true;
	}
	if( out_bNextSilent )
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pBuffer->AudioMix[ i ].Next.Zero();
			io_pBuffer->EnvMix[ i ].Next.Zero();
		}
	}
	if( m_bPreviousSilent )
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pBuffer->AudioMix[ i ].Previous.Zero();
			io_pBuffer->EnvMix[ i ].Previous.Zero();
		}
	}
	else if( m_bFirstBufferProcessed )
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pBuffer->AudioMix[ i ].Previous.Zero();
			io_pBuffer->AudioMix[ i ].Previous.volumes = m_PreviousDirect[ i ];
			io_pBuffer->EnvMix[ i ].Previous.Zero();
			io_pBuffer->EnvMix[ i ].Previous.volumes = m_PreviousEnv[ i ];
		}
	}
	else
	{
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			io_pBuffer->AudioMix[ i ].Previous.Copy( io_pBuffer->AudioMix[ i ].Next );
			io_pBuffer->EnvMix[ i ].Previous.Copy( io_pBuffer->EnvMix[ i ].Next );
		}
		m_bPreviousSilent = out_bNextSilent;
	}
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		m_PreviousDirect[ i ] = io_pBuffer->AudioMix[ i ].Next.volumes;
		m_PreviousEnv[ i ] = io_pBuffer->EnvMix[ i ].Next.volumes;
	}
}

/** 2012 0x927580: stop offset, volumes and audibility, the virtual-voice behaviours, the frame offset, and the pipeline
    built on the first audible frame; true when the voice runs this frame. */
bool CAkVPLSrcCbxNode::StartRun( AkVPLState & io_state )
{
	if( m_eState != NodeStatePlay )
	{
		return false;
	}
	CAkPBI * pCtx = m_pSources[ 0 ]->m_pCtx;
	const AkUInt32 uStopOffset = pCtx->GetStopOffset();
	if( uStopOffset != (AkUInt32)-1 )
	{
		io_state.bStop = true;
		if( !uStopOffset )
		{
			Stop();
			return false;
		}
	}
	io_state.buffer.uChannelMask = m_uChannelMask;
	bool bNextSilent;
	GetVolumes( io_state.bIsEnvironmentalBus, pCtx, &io_state.buffer, bNextSilent );
	const bool bAudibleThisFrame = !m_bPreviousSilent || !bNextSilent;
	bool bAudible = bAudibleThisFrame;
	if( bNextSilent || pCtx->m_bTargetIsFeedback )
	{
		if( pCtx->m_bWasStopped )
		{
			io_state.bStop = true;
		}
		else if( pCtx->m_bWasPaused )
		{
			io_state.bPause = true;
		}
	}
	io_state.bAudible = bAudibleThisFrame;
	bool bRun = false;
	if( !bAudibleThisFrame )
	{
		if( m_eBelowThresholdBehavior == AkBelowThresholdBehavior_SetAsVirtualVoice )
		{
			if( m_bAudible )
			{
				m_cbxRec.m_LPF.VirtualOn( m_eVirtualBehavior );
			}
			if( m_eVirtualBehavior == AkVirtualQueueBehavior_FromElapsedTime )
			{
				m_cbxRec.m_LPF.ReleaseBuffer();
				if( pCtx->m_iFrameOffset < io_state.buffer.uMaxFrames )
				{
					io_state.result = SourceTimeSkip( io_state.buffer.uMaxFrames );
				}
				if( pCtx->m_iFrameOffset >= 0 )
				{
					pCtx->m_iFrameOffset -= io_state.buffer.uMaxFrames;
				}
			}
			else if( m_eVirtualBehavior == AkVirtualQueueBehavior_FromBeginning )
			{
				m_cbxRec.m_LPF.ReleaseBuffer();
			}
		}
		else if( m_eBelowThresholdBehavior == AkBelowThresholdBehavior_KillVoice )
		{
			Stop();
		}
		else
		{
			bAudible = true;
			bRun = pCtx->m_iFrameOffset < io_state.buffer.uMaxFrames;
		}
	}
	else
	{
		CAkVPLSrcNode * pSrc = m_pSources[ 0 ];
		if( ( m_bAudible && pSrc->m_bIOReady ) || m_eBelowThresholdBehavior != AkBelowThresholdBehavior_SetAsVirtualVoice )
		{
			bRun = pCtx->m_iFrameOffset < io_state.buffer.uMaxFrames;
		}
		else if( pSrc->m_bIOReady )
		{
			bool bUseSourceOffset = false;
			const AkCtxVirtualHandlingResult eHandling = pCtx->NotifyVirtualOff( m_eVirtualBehavior );
			if( eHandling == VirtualHandling_ShouldStop )
			{
				Stop();
				return false;
			}
			if( eHandling == VirtualHandling_RequiresSeeking )
			{
				m_cbxRec.m_Pitch.ResetPadFrameOffset();
				bUseSourceOffset = true;
			}
			if( m_cbxRec.m_LPF.VirtualOff( m_eVirtualBehavior, bUseSourceOffset ) != AK_Success )
			{
				Stop();
				return false;
			}
			bRun = pCtx->m_iFrameOffset < io_state.buffer.uMaxFrames;
		}
		else
		{
			const AKRESULT eResult = pSrc->FetchStreamedData();
			if( eResult == AK_FormatNotReady )
			{
				bAudible = false;
			}
			else if( eResult != AK_Success )
			{
				Stop();
				bAudible = false;
			}
			else
			{
				bRun = pCtx->m_iFrameOffset < io_state.buffer.uMaxFrames;
			}
		}
	}
	if( pCtx->m_iFrameOffset >= 0 )
	{
		pCtx->m_iFrameOffset -= io_state.buffer.uMaxFrames;
	}
	SetAudible( bAudible );
	m_bFirstBufferProcessed = true;
	if( !bRun )
	{
		return false;
	}
	m_bPreviousSilent = bNextSilent;
	if( m_cbxRec.m_Pitch.m_pPBI )
	{
		return true;
	}
	if( AddPipeline( io_state.bIsEnvironmentalBus ) != AK_Success )
	{
		Stop();
		return false;
	}
	pCtx->m_Prev2DParams.prev2DParams.m_fPAN_RL = 101.f;
	io_state.buffer.uChannelMask = m_uChannelMask;
	GetVolumes( io_state.bIsEnvironmentalBus, pCtx, &io_state.buffer, bNextSilent );
	for( AkUInt32 i = 0; i < AK_VOICE_MAX_NUM_CHANNELS; ++i )
	{
		io_state.buffer.AudioMix[ i ].Previous.Zero();
		io_state.buffer.EnvMix[ i ].Previous.Zero();
	}
	return true;
}
