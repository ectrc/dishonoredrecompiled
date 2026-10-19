/*=============================================================================
	AkLEngine.cpp - CAkLEngine (aklengine.cpp Win32 0x9212e0..0x921b20, aklengine_softwarepipeline.cpp 0x91c040..
	0x91dc80), AkAudioLibSettings::SetSampleFrequency (0x941e10); 2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * No feedback (motion) pipeline: CAkFeedbackDeviceMgr is never created (no motion device is registered by the
	    game), so every feedback branch is the retail branch for m_pDeviceMgr == NULL.
	  * RegisterDeviceChange / InternalWindowProc / CAkMMNotificationClient (the default-device-changed reset) are not
	    ported: the sink is not recreated when the Windows default output device changes mid-game.
	    m_bResetAudioDevice therefore only turns true when the sink itself reports a failure.
	  * Environmental sends (GetEnvironmentalBus, CreateEnvMixBus, MergeLastAndCurrentValues and the per-environment
	    mix in RunVPL) are the IH4 seam: an environment bus exists to run its reverb effect, which is IH4. A voice on
	    an aux-sending bus mixes its dry path (scaled by the game object dry level) and its obstruction LPF only.
	  * -akwav (SetWavCapture): diagnostic, not retail. When set, every refill handed to the sink (the 16-bit mixed
	    output, or silence) is appended to a WAV file on the audio thread.
=============================================================================*/
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <windows.h>
#include <stdio.h>

#include "AkLEngine.h"
#include "AkSink.h"
#include "AkPBI.h"
#include "AkListener.h"
#include "AkResampler.h"
#include "AkSpeakerPan.h"
#include "AkSinkConvert.h"
#include "AkAudioMgr.h"
#include "AkAudioLibGlobals.h"
#include "AkMath.h"
#include "AkPositionRepository.h"
#include "AkPlayingMgr.h"

AkMemPoolId g_LEngineDefaultPoolId = AK_INVALID_POOL_ID;

namespace AkAudioLibSettings
{
	AkUInt32 g_pcWaitTime = 10;
	AkUInt32 g_uLpfUpdatePeriod = 127;

	/** 2012 0x941e10. */
	void SetSampleFrequency( AkUInt32 in_uSampleFrequency )
	{
		const double fFrequency = (double)in_uSampleFrequency;
		g_pipelineCoreFrequency = in_uSampleFrequency;
		g_msPerBufferTick = (AkInt32)( 1024.0 / ( 0.001 * fFrequency ) );
		g_pcWaitTime = (AkUInt32)( 1024000.0 / fFrequency * 0.5 );
		g_uLpfUpdatePeriod = (AkUInt32)( fFrequency * 0.000020833333 * 128.0 );
	}
}

AkPanningRule CAkLEngine::m_panningRule = AkPanningRule_Speakers;
AkPipelineBufferBase CAkLEngine::m_MasterOut;
CAkVPLFinalMixNode * CAkLEngine::m_pFinalMixNode = NULL;
AkArray<AkVPL *, AkVPL *, ArrayPoolLEngineDefault, 8> CAkLEngine::m_arrayVPLs;
AkUInt32 CAkLEngine::m_uLastStarvationTime = 0;
bool CAkLEngine::m_bResetAudioDevice = false;
bool CAkLEngine::m_bCoInitializeSucceeded = false;
CAkLEngine::CachedBuffers CAkLEngine::m_CachedAudioBuffers[ 24 ];

static AkUInt32 s_uNumRetries = 0;
static FILE * s_pWavFile = NULL;
static AkUInt32 s_uWavDataBytes = 0;
static AkUInt16 s_uWavChannels = 0;

/** 2012 0x9218e0: pools, COM, the sink (DirectSound, else the dummy), listeners, commands, panning tables, the final
    mix in the sink speaker configuration and the master output buffer; the sink is started. */
AKRESULT CAkLEngine::Init()
{
	m_panningRule = AkPanningRule_Speakers;
	if( !AK::MemoryMgr::IsInitialized() || !AK::IAkStreamMgr::Get() )
	{
		return AK_Fail;
	}
	CAkResampler::InitDSPFunctTable();
	AkAudioLibSettings::SetSampleFrequency( g_PDSettings.eAudioQuality == AkSoundQuality_Low ? 24000 : 48000 );
	AKRESULT eResult = CreateLEnginePools();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_bCoInitializeSucceeded = CoInitializeEx( NULL, COINIT_MULTITHREADED ) >= 0;
	if( !g_pAkSink )
	{
		g_pAkSink = CAkSink::Create( g_PDSettings );
		if( !g_pAkSink )
		{
			g_pAkSink = CAkSink::CreateDummy();
			if( !g_pAkSink )
			{
				return AK_Fail;
			}
		}
	}
	eResult = CAkListener::Init();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = CAkLEngineCmds::Init();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	CAkSpeakerPan::Init();
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkVPLFinalMixNode ) );
	if( !pMem )
	{
		return AK_InsufficientMemory;
	}
	m_pFinalMixNode = AkPlacementNew( pMem ) CAkVPLFinalMixNode();
	const AkChannelMask uSpeakersConfig = g_pAkSink->m_SpeakersConfig;
	eResult = m_pFinalMixNode->Init( uSpeakersConfig );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 uSize = AkGetNumChannels( uSpeakersConfig ) * LE_MAX_FRAMES_PER_BUFFER * sizeof( AkReal32 );
	void * pData = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uSize );
	if( !pData )
	{
		return AK_InsufficientMemory;
	}
	memset( pData, 0, uSize );
	m_MasterOut.uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
	m_MasterOut.uChannelMask = uSpeakersConfig;
	m_MasterOut.pData = pData;
	m_MasterOut.uValidFrames = 0;
	m_MasterOut.eState = AK_DataNeeded;
	return g_pAkSink->Start();
}

/** 2012 0x921b20. */
void CAkLEngine::Term()
{
	CAkLEngineCmds::Term();
	if( m_arrayVPLs.m_pItems )
	{
		m_arrayVPLs.Term();
	}
	if( m_MasterOut.pData )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_MasterOut.pData );
		m_MasterOut.pData = NULL;
	}
	if( m_pFinalMixNode )
	{
		m_pFinalMixNode->Term();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_pFinalMixNode );
		m_pFinalMixNode = NULL;
	}
	for( AkUInt32 i = 0; i < 24; ++i )
	{
		for( AkUInt32 j = 0; j < m_CachedAudioBuffers[ i ].m_cItems; ++j )
		{
			AK::MemoryMgr::Free( g_LEngineDefaultPoolId, m_CachedAudioBuffers[ i ].m_Items[ j ] );
		}
		m_CachedAudioBuffers[ i ].m_cItems = 0;
	}
	CAkEffectsMgr::Term();
	if( g_pAkSink )
	{
		g_pAkSink->Term();
		g_pAkSink->~CAkSink();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, g_pAkSink );
		g_pAkSink = NULL;
	}
	if( m_bCoInitializeSucceeded )
	{
		CoUninitialize();
		m_bCoInitializeSucceeded = false;
	}
	DestroyLEnginePools();
	SetWavCapture( NULL );
}

/** 2012 0x91cbd0. */
void CAkLEngine::Stop()
{
	DestroyAllVPLMixBusses();
	if( m_pFinalMixNode )
	{
		m_pFinalMixNode->DropFx();
	}
	CAkLEngineCmds::DestroyDisconnectedSources();
}

/** 2012 0x91db30. */
void CAkLEngine::ApplyGlobalSettings( AkPlatformInitSettings * io_pPDSettings )
{
	if( io_pPDSettings )
	{
		g_PDSettings = *io_pPDSettings;
		if( g_PDSettings.uNumRefillsInVoice < 2 )
		{
			g_PDSettings.uNumRefillsInVoice = 4;
		}
		*io_pPDSettings = g_PDSettings;
	}
	else
	{
		AK::SoundEngine::GetDefaultPlatformInitSettings( g_PDSettings );
	}
}

/** 2012 0x91dab0: 16 MB unless the settings ask for more than 64 bytes. */
AKRESULT CAkLEngine::CreateLEnginePools()
{
	if( g_LEngineDefaultPoolId != AK_INVALID_POOL_ID )
	{
		return AK_Success;
	}
	const AkUInt32 uSize = g_PDSettings.uLEngineDefaultPoolSize > 0x40 ? g_PDSettings.uLEngineDefaultPoolSize : 0x1000000;
	g_LEngineDefaultPoolId = AK::MemoryMgr::CreatePool( NULL, uSize, 0x40, AkMalloc, 0x10 );
	return g_LEngineDefaultPoolId != AK_INVALID_POOL_ID ? AK_Success : AK_InsufficientMemory;
}

/** 2012 0x91db00. */
void CAkLEngine::DestroyLEnginePools()
{
	if( AK::MemoryMgr::IsInitialized() && g_LEngineDefaultPoolId != AK_INVALID_POOL_ID )
	{
		AK::MemoryMgr::DestroyPool( g_LEngineDefaultPoolId );
		g_LEngineDefaultPoolId = AK_INVALID_POOL_ID;
	}
}

/** 2012 0x91dc30. */
AKRESULT CAkLEngine::SetPanningRule( AkPanningRule in_panningRule )
{
	m_panningRule = in_panningRule;
	CAkListener::ResetListenerData();
	return AK_Success;
}

/** 2012 0x921350. */
void CAkLEngine::StartVoice()
{
	g_pAkSink->Play();
}

/** 2012 0x9213e0: a failed device falls back to the dummy sink, and the real one is retried for the next 10 frames. */
AkUInt32 CAkLEngine::GetNumBufferNeededAndSubmit()
{
	AkUInt32 uBuffersNeeded = 0;
	if( s_uNumRetries )
	{
		--s_uNumRetries;
		CAkSink * pSink = CAkSink::Create( g_PDSettings );
		if( pSink )
		{
			if( pSink->Start() == AK_Success && pSink->Play() == AK_Success )
			{
				g_pAkSink->Term();
				g_pAkSink->~CAkSink();
				AK::MemoryMgr::Free( g_LEngineDefaultPoolId, g_pAkSink );
				g_pAkSink = pSink;
				s_uNumRetries = 0;
			}
			else
			{
				pSink->Term();
				pSink->~CAkSink();
				AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pSink );
			}
		}
	}
	if( g_pAkSink->IsDataNeeded( uBuffersNeeded ) != AK_Success || m_bResetAudioDevice )
	{
		m_bResetAudioDevice = false;
		g_pAkSink->Term();
		g_pAkSink->~CAkSink();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, g_pAkSink );
		g_pAkSink = CAkSink::CreateDummy();
		g_pAkSink->Start();
		g_pAkSink->Play();
		s_uNumRetries = 10;
	}
	return uBuffersNeeded;
}

/** 2012 0x91c670. */
void CAkLEngine::HandleStarvation()
{
	if( g_pAkSink->IsStarved() )
	{
		g_pAkSink->ResetStarved();
		if( !m_uLastStarvationTime || g_pAudioMgr->m_uBufferTick - m_uLastStarvationTime > 8 )
		{
			m_uLastStarvationTime = g_pAudioMgr->m_uBufferTick;
		}
	}
}

/** 2012 0x921530: one 1024-frame buffer through every VPL into the sink (silence when nothing played), flush-to-zero
    on, then the idle mix busses are removed. */
void CAkLEngine::Perform()
{
	CAkLEngineCmds::ProcessAllCommands();
	HandleStarvation();
	_mm_setcsr( _mm_getcsr() | 0x8000 );
	const AkUInt32 uFlushToZero = _mm_getcsr() & 0x8000;
	m_MasterOut.uValidFrames = 0;
	GetBuffer();
	if( m_MasterOut.uValidFrames )
	{
		WriteWav( &m_MasterOut );
		g_pAkSink->PassData( m_MasterOut );
		m_pFinalMixNode->ReleaseBuffer();
	}
	else
	{
		WriteWav( NULL );
		g_pAkSink->PassSilence( LE_MAX_FRAMES_PER_BUFFER );
	}
	_mm_setcsr( uFlushToZero | ( _mm_getcsr() & ~0x8000u ) );
	RemoveMixBusses();
}

/** 2012 0x91d510: the VPLs run last-created first (children before their parent bus), each voice into its bus, each
    bus into its parent or the final mix; the final mix interleaves into m_MasterOut. */
void CAkLEngine::GetBuffer()
{
	if( !g_pPositionRepository->m_mapPosInfo.IsEmpty() )
	{
		g_pPositionRepository->UpdateTime();
	}
	CAkLEngineCmds::ProcessDisconnectedSources( LE_MAX_FRAMES_PER_BUFFER );
	for( AkInt32 iVPL = (AkInt32)m_arrayVPLs.Length() - 1; iVPL >= 0; --iVPL )
	{
		AkVPL * pVPL = m_arrayVPLs[ iVPL ];
		const bool bIsEnv = pVPL->m_MixBus.m_BusContext.IsEnvironmental();
		AkListBare<CAkVPLSrcCbxNode>::IteratorEx it = pVPL->m_listVPLSrcs.BeginEx();
		while( it != pVPL->m_listVPLSrcs.EndEx() )
		{
			CAkVPLSrcCbxNode * pCbx = *it;
			AkRunningVPL runningVPL;
			runningVPL.state.buffer.uNumMarkers = 0;
			runningVPL.state.bPause = false;
			runningVPL.state.bStop = false;
			runningVPL.state.buffer.pData = NULL;
			runningVPL.state.buffer.pMarkers = NULL;
			runningVPL.state.buffer.uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
			runningVPL.state.bIsEnvironmentalBus = bIsEnv;
			runningVPL.state.result = AK_DataNeeded;
			runningVPL.bFeedbackVPL = pVPL->m_bFeedback;
			runningVPL.pFeedbackData = NULL;
			if( pCbx->StartRun( runningVPL.state ) )
			{
				runningVPL.pCbx = pCbx;
				runningVPL.pBus = pVPL;
				runningVPL.state.buffer.uValidFrames = 0;
				runningVPL.state.buffer.uNumMarkers = 0;
				runningVPL.state.buffer.pMarkers = NULL;
				runningVPL.state.buffer.posInfo.uStartPos = (AkUInt32)-1;
				RunVPL( runningVPL );
				if( runningVPL.state.result == AK_NoDataReady )
				{
					pCbx->RestorePreviousVolumes( &runningVPL.state.buffer );
				}
				else
				{
					pCbx->ReleaseBuffer();
				}
			}
			FinishRun( pCbx, runningVPL.state );
			if( pCbx->m_eState == NodeStateStop )
			{
				pVPL->m_MixBus.Disconnect();
				CAkLEngineCmds::DeleteAllCommandsForSource( pCbx );
				it = pVPL->m_listVPLSrcs.Erase( it );
				VPLDestroySource( pCbx );
			}
			else
			{
				++it;
			}
		}
		if( !pVPL->m_bFeedback )
		{
			AkAudioBufferBus * pBuffer;
			pVPL->m_MixBus.GetResultingBuffer( pBuffer );
			if( pVPL->m_pParent )
			{
				pVPL->m_pParent->m_MixBus.ConsumeBuffer( *pBuffer );
			}
			else
			{
				m_pFinalMixNode->ConsumeBuffer( pBuffer );
			}
			pVPL->m_MixBus.ReleaseBuffer();
		}
	}
	m_pFinalMixNode->GetResultingBuffer( m_MasterOut );
}

/** 2012 0x91cf80: pull the voice buffer through source -> pitch -> (effects) -> LPF, re-pulling upstream while a node
    needs data, then mix it into its bus. */
void CAkLEngine::RunVPL( AkRunningVPL & io_runningVPL )
{
	CAkVPLSrcCbxNode * pCbx = io_runningVPL.pCbx;
	AkVPLState & rState = io_runningVPL.state;
	pCbx->m_cbxRec.m_Pitch.GetBuffer( rState );
	while( rState.result == AK_DataNeeded )
	{
		rState.buffer.uMaxFrames = LE_MAX_FRAMES_PER_BUFFER;
		pCbx->m_pSources[ 0 ]->GetBuffer( rState );
		if( rState.result != AK_DataReady && rState.result != AK_NoMoreData )
		{
			return;
		}
		pCbx->m_cbxRec.m_Pitch.ConsumeBuffer( rState );
	}
	if( rState.result != AK_DataReady && rState.result != AK_NoMoreData )
	{
		return;
	}
	pCbx->m_cbxRec.m_LPF.ConsumeBuffer( rState );
	pCbx->ConsumeBuffer( rState );
	if( rState.result != AK_DataReady && rState.result != AK_NoMoreData )
	{
		return;
	}
	g_pPlayingMgr->NotifyMarkers( rState.buffer );
	if( !rState.bAudible )
	{
		return;
	}
	if( rState.bIsEnvironmentalBus )
	{
		pCbx->m_ObstructionLPF.ConsumeBuffer( rState );
	}
	if( !io_runningVPL.bFeedbackVPL )
	{
		io_runningVPL.pBus->m_MixBus.ConsumeBuffer( rState, -1 );
	}
}

/** 2012 0x91c120: a source that ran out switches to its queued sample-accurate successor, else the voice stops. */
void CAkLEngine::FinishRun( CAkVPLSrcCbxNode * in_pCbx, AkVPLState & io_state )
{
	if( io_state.result == AK_NoMoreData && !io_state.bStop )
	{
		CAkVPLSrcNode * pNext = in_pCbx->m_pSources[ 1 ];
		if( pNext )
		{
			in_pCbx->m_pSources[ 1 ] = NULL;
			in_pCbx->RemovePipeline( CtxDestroyReasonFinished );
			if( in_pCbx->AddSrc( pNext, true ) == AK_Success && in_pCbx->AddPipeline( io_state.bIsEnvironmentalBus ) == AK_Success )
			{
				pNext->Start();
				return;
			}
		}
		in_pCbx->Stop();
	}
	else if( io_state.result == AK_Fail || io_state.bStop )
	{
		in_pCbx->Stop();
	}
	else if( io_state.bPause )
	{
		in_pCbx->Pause();
	}
}

/** 2012 0x91d8a0: a sample-accurate successor joins its predecessor voice; otherwise a new voice on its mixing bus VPL. */
AKRESULT CAkLEngine::AddSound( AkLECmd & io_cmd )
{
	CAkPBI * pCtx = io_cmd.m_pCtx;
	CAkVPLSrcCbxNode * pExisting = FindExistingVPLSrc( pCtx );
	if( pExisting )
	{
		pExisting->AddSrc( pCtx, false );
		pCtx->NotifAddedAsSA();
		return AK_PartialSuccess;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkVPLSrcCbxNode ) );
	if( !pMem )
	{
		pCtx->Destroy( CtxDestroyReasonPlayFailed );
		return AK_Fail;
	}
	CAkVPLSrcCbxNode * pCbx = AkPlacementNew( pMem ) CAkVPLSrcCbxNode();
	pCbx->Init( pCtx->m_bTargetIsFeedback ? 375 : AkAudioLibSettings::g_pipelineCoreFrequency );
	AKRESULT eResult = pCbx->AddSrc( pCtx, true );
	if( eResult == AK_FormatNotReady )
	{
		CAkLEngineCmds::AddDisconnectedSource( pCbx );
		return AK_Success;
	}
	if( eResult == AK_Success )
	{
		AkVPL * pVPL = GetVPLMixBus( pCtx );
		if( pVPL && ( !pCbx->m_bAudible || pCbx->AddPipeline( pVPL->m_MixBus.m_BusContext.IsEnvironmental() ) == AK_Success ) )
		{
			pVPL->m_listVPLSrcs.AddLast( pCbx );
			pVPL->m_MixBus.Connect();
			io_cmd.m_bSourceConnected = true;
			return AK_Success;
		}
		eResult = AK_Fail;
	}
	VPLDestroySource( pCbx );
	return eResult;
}

/** 2012 0x91d9c0: a disconnected source whose stream is now ready joins its bus. */
AKRESULT CAkLEngine::VPLTryConnectSource( CAkPBI * in_pContext, CAkVPLSrcCbxNode * in_pCbx )
{
	const AKRESULT eResult = in_pCbx->FetchStreamedData( in_pContext );
	if( eResult == AK_FormatNotReady )
	{
		return eResult;
	}
	CAkLEngineCmds::m_listSrcsNotConnected.Remove( in_pCbx );
	if( eResult == AK_Success )
	{
		AkVPL * pVPL = GetVPLMixBus( in_pContext );
		if( pVPL && in_pCbx->AddPipeline( pVPL->m_MixBus.m_BusContext.IsEnvironmental() ) == AK_Success )
		{
			pVPL->m_listVPLSrcs.AddLast( in_pCbx );
			pVPL->m_MixBus.Connect();
			return AK_Success;
		}
	}
	VPLDestroySource( in_pCbx );
	return AK_Fail;
}

/** 2012 0x91c7b0: the voice of a command (a stop-looping also matches the queued successor). */
CAkVPLSrcCbxNode * CAkLEngine::ResolveCommandVPL( AkLECmd & io_cmd )
{
	CAkPBI * pCtx = io_cmd.m_pCtx;
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		for( CAkVPLSrcCbxNode * pCbx = m_arrayVPLs[ i ]->m_listVPLSrcs.m_pFirst; pCbx; pCbx = pCbx->pNextItem )
		{
			if( ( pCbx->m_pSources[ 0 ] && pCbx->m_pSources[ 0 ]->m_pCtx == pCtx )
				|| ( io_cmd.m_eState == LEStateStopLooping && pCbx->m_pSources[ 1 ] && pCbx->m_pSources[ 1 ]->m_pCtx == pCtx ) )
			{
				if( pCbx->m_eState != NodeStateInit )
				{
					io_cmd.m_bSourceConnected = true;
				}
				return pCbx;
			}
		}
	}
	CAkVPLSrcCbxNode * pCbx = pCtx->m_pCbx;
	if( !pCbx )
	{
		return NULL;
	}
	if( ( !pCbx->m_pSources[ 0 ] || pCbx->m_pSources[ 0 ]->m_pCtx != pCtx ) && io_cmd.m_eState != LEStateStopLooping )
	{
		return NULL;
	}
	return pCbx;
}

/** 2012 0x91c830. */
CAkVPLSrcCbxNode * CAkLEngine::FindExistingVPLSrc( CAkPBI * in_pCtx )
{
	const AkUInt32 uSeqID = in_pCtx->m_SeqID;
	if( !uSeqID )
	{
		return NULL;
	}
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		for( CAkVPLSrcCbxNode * pCbx = m_arrayVPLs[ i ]->m_listVPLSrcs.m_pFirst; pCbx; pCbx = pCbx->pNextItem )
		{
			if( pCbx->GetContext()->m_SeqID == uSeqID )
			{
				return pCbx;
			}
		}
	}
	return NULL;
}

/** 2012 0x91d4d0. */
AkVPL * CAkLEngine::GetVPLMixBus( CAkPBI * in_pCtx )
{
	AK::CAkBusCtx ctxBus = in_pCtx->GetBusContext();
	return GetVPLMixBus( ctxBus, in_pCtx->m_bTargetIsFeedback );
}

/** 2012 0x91d3b0: the VPL of a mixing bus, created with its parent chain on first use (channel mask narrowed by the
    bus, its parent and the master output). */
AkVPL * CAkLEngine::GetVPLMixBus( AK::CAkBusCtx & in_ctxBus, bool in_bForFeedback )
{
	const AkUniqueID busID = in_ctxBus.ID();
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		AkVPL * pVPL = m_arrayVPLs[ i ];
		if( pVPL->m_MixBus.m_BusContext.ID() == busID && pVPL->m_MixBus.m_eState != NodeStateStop )
		{
			return pVPL;
		}
	}
	AkVPL * pParentVPL = NULL;
	AK::CAkBusCtx parentCtx = in_ctxBus.GetParentCtx();
	if( parentCtx.m_pBus )
	{
		pParentVPL = GetVPLMixBus( parentCtx, in_bForFeedback );
		if( !pParentVPL )
		{
			return NULL;
		}
	}
	AkVPL * pVPL;
	if( in_bForFeedback )
	{
		pVPL = CreateVPLMixBus( in_ctxBus, AK_SPEAKER_FRONT_CENTER, 1 );
	}
	else
	{
		AkChannelMask uChannelMask = in_ctxBus.GetChannelConfig() & m_MasterOut.uChannelMask;
		if( pParentVPL )
		{
			uChannelMask &= pParentVPL->m_MixBus.m_BufferOut.uChannelMask;
		}
		pVPL = CreateVPLMixBus( in_ctxBus, uChannelMask, LE_MAX_FRAMES_PER_BUFFER );
	}
	if( !pVPL )
	{
		return NULL;
	}
	pVPL->m_bFeedback = in_bForFeedback;
	if( !in_bForFeedback )
	{
		if( pParentVPL )
		{
			pParentVPL->m_MixBus.Connect();
			pVPL->m_pParent = pParentVPL;
		}
		else
		{
			m_pFinalMixNode->Connect( &pVPL->m_MixBus );
		}
	}
	if( !in_ctxBus.IsEnvironmental() && !in_ctxBus.IsMasterBus() )
	{
		pVPL->m_MixBus.SetAllInsertFx( AK_INVALID_UNIQUE_ID );
	}
	return pVPL;
}

/** 2012 0x91d310. */
AkVPL * CAkLEngine::CreateVPLMixBus( AK::CAkBusCtx in_BusCtx, AkChannelMask in_uChannelMask, AkUInt16 in_usMaxFrames )
{
	void * pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( AkVPL ) );
	if( !pMem )
	{
		return NULL;
	}
	AkVPL * pVPL = AkPlacementNew( pMem ) AkVPL();
	pVPL->m_MixBus.m_BufferOut.pData = NULL;
	pVPL->m_MixBus.m_BusContext.m_pBus = NULL;
	pVPL->m_pParent = NULL;
	if( pVPL->m_MixBus.Init( in_uChannelMask, in_usMaxFrames, in_BusCtx ) != AK_Success || !m_arrayVPLs.AddLast( pVPL ) )
	{
		pVPL->m_listVPLSrcs.RemoveAll();
		pVPL->m_MixBus.Term();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pVPL );
		return NULL;
	}
	return pVPL;
}

/** 2012 0x91c970. */
void CAkLEngine::DestroyVPLMixBus( AkVPL * in_pMixBus )
{
	in_pMixBus->m_MixBus.Term();
	while( CAkVPLSrcCbxNode * pCbx = in_pMixBus->m_listVPLSrcs.RemoveFirst() )
	{
		pCbx->Stop();
		VPLDestroySource( pCbx );
	}
	in_pMixBus->m_listVPLSrcs.RemoveAll();
	if( in_pMixBus->m_pParent )
	{
		in_pMixBus->m_pParent->m_MixBus.Disconnect();
	}
	AK::MemoryMgr::Free( g_LEngineDefaultPoolId, in_pMixBus );
}

/** 2012 0x91c9e0. */
void CAkLEngine::DestroyAllVPLMixBusses()
{
	for( AkInt32 i = (AkInt32)m_arrayVPLs.Length() - 1; i >= 0; --i )
	{
		DestroyVPLMixBus( m_arrayVPLs[ i ] );
	}
	m_arrayVPLs.RemoveAll();
}

/** 2012 0x91cac0: a bus with nothing mixed and nothing connected goes away (children are removed first). */
void CAkLEngine::RemoveMixBusses()
{
	for( AkInt32 i = (AkInt32)m_arrayVPLs.Length() - 1; i >= 0; --i )
	{
		AkVPL * pVPL = m_arrayVPLs[ i ];
		if( pVPL->m_MixBus.m_eState != NodeStatePlay && !pVPL->m_MixBus.m_uConnectCount )
		{
			DestroyVPLMixBus( pVPL );
			m_arrayVPLs.Erase( i );
		}
	}
}

/** 2012 0x91dc80. */
void CAkLEngine::VPLDestroySource( CAkVPLSrcCbxNode * in_pCbx )
{
	in_pCbx->Term();
	in_pCbx->~CAkVPLSrcCbxNode();
	AK::MemoryMgr::Free( g_LEngineDefaultPoolId, in_pCbx );
}

/** 2012 0x91c480: only bus insert effects reference bank media, and none exist before IH4. */
void CAkLEngine::StopMixBussesUsingThisSlot( const CAkUsageSlot * in_pSlot )
{
}

/** 2012 0x91c2f0. */
void CAkLEngine::SetBusVolume( AkUniqueID in_MixBusID, AkReal32 in_VolumeOffset )
{
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		CAkVPLMixBusNode & rMixBus = m_arrayVPLs[ i ]->m_MixBus;
		if( rMixBus.m_BusContext.ID() == in_MixBusID )
		{
			rMixBus.m_fNextVolumedB += in_VolumeOffset;
			rMixBus.m_fNextVolume = AkMath::dBToLin( rMixBus.m_fNextVolumedB );
		}
	}
}

/** 2012 0x91c240. */
void CAkLEngine::ForceBusVolume( AkUniqueID in_MixBusID, AkReal32 in_Volume )
{
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		CAkVPLMixBusNode & rMixBus = m_arrayVPLs[ i ]->m_MixBus;
		if( rMixBus.m_BusContext.ID() == in_MixBusID )
		{
			rMixBus.m_fNextVolumedB = in_Volume;
			rMixBus.m_fNextVolume = AkMath::dBToLin( in_Volume );
		}
	}
}

/** 2012 0x91c070. */
void CAkLEngine::SetMasterBusVolume( AkReal32 in_VolumeOffset )
{
	m_pFinalMixNode->m_fNextVolumedB += in_VolumeOffset;
	m_pFinalMixNode->m_fNextVolume = AkMath::dBToLin( m_pFinalMixNode->m_fNextVolumedB );
}

/** 2012 0x91c040. */
void CAkLEngine::ForceMasterBusVolume( AkReal32 in_Volume )
{
	m_pFinalMixNode->m_fNextVolumedB = in_Volume;
	m_pFinalMixNode->m_fNextVolume = AkMath::dBToLin( in_Volume );
}

/** 2012 0x91c3b0. */
void CAkLEngine::BypassBusFx( AkUniqueID in_MixBusID, AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	for( AkUInt32 i = 0; i < m_arrayVPLs.Length(); ++i )
	{
		if( m_arrayVPLs[ i ]->m_MixBus.m_BusContext.ID() == in_MixBusID )
		{
			m_arrayVPLs[ i ]->m_MixBus.SetInsertFxBypass( in_bitsFXBypass, in_uTargetMask );
		}
	}
}

/** 2012 0x91c0b0. */
void CAkLEngine::BypassMasterBusFx( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	if( m_pFinalMixNode )
	{
		m_pFinalMixNode->SetInsertFxBypass( in_bitsFXBypass, in_uTargetMask );
	}
}

/** 2012 0x91c580: 24 size classes of 1 KB, two blocks cached per class. */
void * CAkLEngine::GetCachedAudioBuffer( AkUInt32 in_uSize )
{
	const AkUInt32 uSize = in_uSize < 0x400 ? 0x400 : in_uSize;
	CachedBuffers & rCache = m_CachedAudioBuffers[ ( uSize - 1 ) >> 10 ];
	if( !rCache.m_cItems )
	{
		return AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uSize );
	}
	return rCache.m_Items[ --rCache.m_cItems ];
}

/** 2012 0x91c5d0. */
void CAkLEngine::ReleaseCachedAudioBuffer( AkUInt32 in_uSize, void * in_pvBuffer )
{
	const AkUInt32 uSize = in_uSize < 0x400 ? 0x400 : in_uSize;
	CachedBuffers & rCache = m_CachedAudioBuffers[ ( uSize - 1 ) >> 10 ];
	if( rCache.m_cItems >= 2 )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, in_pvBuffer );
		return;
	}
	rCache.m_Items[ rCache.m_cItems++ ] = in_pvBuffer;
}

void CAkLEngine::SetWavCapture( const wchar_t * in_pszPath )
{
	AkAutoLock<CAkLock> gate( g_csMain );
	if( s_pWavFile )
	{
		const AkUInt32 uRiffSize = 36 + s_uWavDataBytes;
		fseek( s_pWavFile, 4, SEEK_SET );
		fwrite( &uRiffSize, 4, 1, s_pWavFile );
		fseek( s_pWavFile, 40, SEEK_SET );
		fwrite( &s_uWavDataBytes, 4, 1, s_pWavFile );
		fclose( s_pWavFile );
		s_pWavFile = NULL;
	}
	if( !in_pszPath || _wfopen_s( &s_pWavFile, in_pszPath, L"wb" ) != 0 )
	{
		s_pWavFile = NULL;
		return;
	}
	s_uWavDataBytes = 0;
	s_uWavChannels = 0;
}

void CAkLEngine::WriteWav( const AkPipelineBufferBase * in_pBuffer )
{
	if( !s_pWavFile )
	{
		return;
	}
	if( !s_uWavChannels )
	{
		s_uWavChannels = (AkUInt16)AkGetNumChannels( g_pAkSink->m_SpeakersConfig );
		const AkUInt32 uRate = AkAudioLibSettings::g_pipelineCoreFrequency;
		const AkUInt16 uBlockAlign = 2 * s_uWavChannels;
		const AkUInt32 uByteRate = uRate * uBlockAlign;
		const AkUInt32 uZero = 0;
		const AkUInt32 uFmtSize = 16;
		const AkUInt16 uPCM = 1;
		const AkUInt16 uBits = 16;
		fwrite( "RIFF", 4, 1, s_pWavFile );
		fwrite( &uZero, 4, 1, s_pWavFile );
		fwrite( "WAVEfmt ", 8, 1, s_pWavFile );
		fwrite( &uFmtSize, 4, 1, s_pWavFile );
		fwrite( &uPCM, 2, 1, s_pWavFile );
		fwrite( &s_uWavChannels, 2, 1, s_pWavFile );
		fwrite( &uRate, 4, 1, s_pWavFile );
		fwrite( &uByteRate, 4, 1, s_pWavFile );
		fwrite( &uBlockAlign, 2, 1, s_pWavFile );
		fwrite( &uBits, 2, 1, s_pWavFile );
		fwrite( "data", 4, 1, s_pWavFile );
		fwrite( &uZero, 4, 1, s_pWavFile );
	}
	AkInt16 aSamples[ LE_MAX_FRAMES_PER_BUFFER * AK_VOICE_MAX_NUM_CHANNELS ];
	const AkUInt32 uNumSamples = LE_MAX_FRAMES_PER_BUFFER * s_uWavChannels;
	if( in_pBuffer && in_pBuffer->uChannelMask == g_pAkSink->m_SpeakersConfig )
	{
		FloatsToShortsSSE2( aSamples, (const AkReal32 *)in_pBuffer->pData, uNumSamples );
	}
	else
	{
		memset( aSamples, 0, uNumSamples * sizeof( AkInt16 ) );
	}
	fwrite( aSamples, sizeof( AkInt16 ), uNumSamples, s_pWavFile );
	s_uWavDataBytes += uNumSamples * sizeof( AkInt16 );
}
