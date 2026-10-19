/*=============================================================================
	AkPBI.cpp - CAkPBI (akpbi.cpp, 2012 0x93efb0..0x9411b0; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named, every one an IH3 seam unless stated):
	Agent IH3 (wave 26, fork P): Init / Term / _InitPlay / _Stop / _Play / _Pause / _Resume create, start, pause, resume
	and release the user-defined 3D path (CAkPathManager); the positioning half (Get3DPositioningParams 0x9411b0,
	PositioningChangeNotification 0x941030, Scale3DUserDefRTPCValue 0x940fc0, Subscribe / UnsubscribeAttenuationRTPC
	0x93ffe0 / 0x940040) is in AkPositioning.cpp.
	Agent IH3 (fork N): RefreshParameters runs the full GetAudioParameters (states, RTPC, SIS, ducking, layers) and logs
	the voice's volume split under -akdiag (AkDiagVoiceParameters); ParamNotification 0x9402e0 and UpdateFx 0x93f500 are
	in AkParameterNode.cpp. No feedback (motion) volume.
	  * GetFeedbackParameters / InvalidateFeedbackParameters: no feedback device exists (CAkFeedbackDeviceMgr is never
	    created by the glue), so retail's own first test returns NULL - ported as that test.
	  * The MONITOR_* notifications (MonitorFade) are compiled out.
=============================================================================*/
#include "AkActivityChunk.h"
#include "AkURenderer.h"
#include "AkLEngineCmds.h"
#include "AkPlayingMgr.h"
#include "AkBusCtx.h"
#include "AkPath.h"
#include "AkBankMgr.h"
#include "AkVPLSrcCbxNode.h"

AkUInt32 CAkPBI::ms_PBIIDGenerator = 0;

/** 2012 0x940760. */
CAkPBI::CAkPBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, UserParams & in_rUserparams, PlayHistory & in_rPlayHistory, AkUniqueID in_SeqID, const PriorityInfoCurrent & in_rPriority, bool in_bTargetFeedback, AkUInt32 in_uSourceOffset, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter )
	: pNextItem( NULL )
	, pNextLightItem( NULL )
	, m_pUsageSlot( NULL )
	, m_p3DSound( NULL )
	, m_UserParams( in_rUserparams )
	, m_SeqID( in_SeqID )
	, m_pSound( in_pSound )
	, m_pSource( in_pSource )
	, m_pGameObj( in_pGameObj )
	, m_pCbx( NULL )
	, m_Volume( 0.f )
	, m_LPF( 0.f )
	, m_LPFAutomationOffset( 0.f )
	, m_fPlayStopFadeRatio( 1.f )
	, m_fPauseResumeFadeRatio( 1.f )
	, m_LoopCount( 1 )
	, m_PriorityInfoCurrent( in_rPriority )
	, m_ulPauseCount( 0 )
	, m_iFrameOffset( 0 )
	, m_pDataPtr( NULL )
	, m_uDataSize( 0 )
	, m_fMaxDistance( 0.f )
	, m_pFeedbackInfo( NULL )
	, m_pAMLimiter( in_pAMLimiter )
	, m_pBusLimiter( in_pBusLimiter )
{
	m_Ranges.VolumeOffset = 0.f;
	m_Ranges.PitchOffset = 0.f;
	m_Ranges.LPFOffset = 0.f;
	m_uSeekPosition = in_uSourceOffset;
	m_eInitialState = 0;
	m_State = 0;
	m_eCachedVirtualQueueBehavior = 0;
	m_eCachedBelowThresholdBehavior = 0;
	m_bVirtualBehaviorCached = 0;
	m_bAreParametersValid = 0;
	m_b3DPositionDone = 0;
	m_bGetAudioParamsCalled = 0;
	m_bNeedNotifyEndReached = 0;
	m_bIsNotifyEndReachedContinuous = 0;
	m_bTerminatedByStop = 0;
	m_bPlayFailed = 0;
	m_bWasStopped = 0;
	m_bWasPreStopped = 0;
	m_bWasPaused = 0;
	m_bInitPlayWasCalled = 0;
	m_bWasKicked = 0;
	m_eWasKickedForMemory = 0;
	m_bWasPlayCountDecremented = 0;
	m_bRequiresPreBuffering = 1;
	m_bFeedbackParametersValid = 0;
	m_bTargetIsFeedback = in_bTargetFeedback;
	m_bBypassAllFX = 0;
	m_bSeekDirty = in_uSourceOffset != 0;
	m_bSeekRelativeToDuration = 0;
	m_bSnapSeekToMarker = 0;
	m_bIsForcedToVirtualize = 0;
	m_bWasForcedToVirtualize = 0;
	m_bIsVirtual = 0;
	m_bNeedsFadeIn = 0;
	m_EffectiveParams.Volume = 0.f;
	m_EffectiveParams.Pitch = 0.f;
	m_EffectiveParams.LPF = 0.f;
	m_PriorityInfoCurrent.currentPriority.PBIID = ms_PBIIDGenerator++;
	m_PathInfo.pPBPath = NULL;
	m_PathInfo.PathOwnerID = 0;
	m_BasePosParams.m_fPAN_RL = 0.f;
	m_BasePosParams.m_fPAN_FR = 0.f;
	m_BasePosParams.m_fCenterPCT = 0.f;
	m_BasePosParams.bIsPannerEnabled = false;
	m_Prev2DParams.prev2DParams.m_fPAN_RL = 101.f;
	m_Prev2DParams.prevVolume = 0.f;
	m_Prev2DParams.prevDryLevel = 0.f;
	m_Prev2DParams.prevMotionVol = 0.f;
	memset( &m_sMediaFormat, 0, sizeof( m_sMediaFormat ) );
	m_sMediaFormat.uChannelMask = AK_SPEAKER_SETUP_MONO;
	++m_pGameObj->m_refCount;
	m_pSound->AddRef();
	m_pSound->m_pActivityChunk->m_listPBI.AddFirst( this );
	m_CntrHistArray = in_rPlayHistory.HistArray;
}

/** 2012 0x93f770. */
CAkPBI::~CAkPBI()
{
	m_LastComputedPosition.Term();
}

/** 2012 0x940b80: limiters, the playing id's PBI count, the 3D parameters and attenuation, then the media lock. */
AKRESULT CAkPBI::Init( AkPathInfo * in_pPathInfo )
{
	if( m_pAMLimiter )
	{
		m_pAMLimiter->Add( this, AKVoiceLimiter_AM );
	}
	if( m_pBusLimiter )
	{
		m_pBusLimiter->Add( this, AKVoiceLimiter_Bus );
	}
	CAkURenderer::m_GlobalLimiter.Add( this, AKVoiceLimiter_Global );
	if( !m_UserParams.m_PlayingID )
	{
		return AK_Fail;
	}
	AKRESULT eResult = g_pPlayingMgr->SetPBI( m_UserParams.m_PlayingID, this, &m_uRegisteredNotif );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_bBypassAllFX = m_pSound->GetBypassAllFX( m_pGameObj );
	m_p3DSound = NULL;
	m_pSound->Get3DParams( m_p3DSound, m_pGameObj, false, &m_BasePosParams );
	if( m_p3DSound )
	{
		CAkAttenuation * pAttenuation = m_p3DSound->m_Params.GetAttenuation();
		if( pAttenuation )
		{
			m_p3DSound->m_Params.m_fConeOutsideVolume = pAttenuation->m_ConeParams.fOutsideVolume;
			m_p3DSound->m_Params.m_fConeLoPass = pAttenuation->m_ConeParams.LoPass;
			eResult = SubscribeAttenuationRTPC( pAttenuation );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
		else if( m_p3DSound->m_Params.m_uAttenuationID )
		{
			return AK_Fail;
		}
		if( m_p3DSound->m_Params.m_eType == Ak3DUserDef && m_p3DSound->m_Params.m_pArrayPlaylist )
		{
			AkUniqueID pathOwnerID = m_p3DSound->m_Params.m_ID;
			if( in_pPathInfo->pPBPath && in_pPathInfo->PathOwnerID == pathOwnerID )
			{
				m_PathInfo = *in_pPathInfo;
			}
			if( !m_PathInfo.pPBPath || ( m_p3DSound->m_Params.m_ePathMode & AkPathPickNewPath ) )
			{
				m_PathInfo.pPBPath = g_pPathManager->AddPathToList();
				if( m_PathInfo.pPBPath )
				{
					eResult = m_p3DSound->SetPathPlayList( m_PathInfo.pPBPath, m_pSound->GetPathState() );
					if( eResult != AK_Success )
					{
						g_pPathManager->RemovePathFromList( m_PathInfo.pPBPath );
						pathOwnerID = 0;
						m_PathInfo.pPBPath = NULL;
					}
					m_PathInfo.PathOwnerID = pathOwnerID;
				}
			}
			if( !m_p3DSound->m_Params.m_bFollowOrientation && m_PathInfo.pPBPath )
			{
				m_PathInfo.pPBPath->InitRotationMatricesForNoFollowMode( m_pGameObj->m_PosKeep.m_uListenerMask );
			}
		}
	}
	if( m_PathInfo.pPBPath )
	{
		if( g_pPathManager->AddPathUser( m_PathInfo.pPBPath, this ) == AK_Fail )
		{
			m_PathInfo.pPBPath = NULL;
		}
		else
		{
			m_PathInfo.pPBPath->SetSoundUniqueID( m_pSound->key );
			m_PathInfo.pPBPath->SetPlayingID( m_UserParams.m_PlayingID );
		}
	}
	void * pDataPtr = NULL;
	m_pSource->LockDataPtr( pDataPtr, m_uDataSize, this );
	m_pDataPtr = (AkUInt8 *)pDataPtr;
	return eResult;
}

/** 2012 0x940dc0: undo everything Init and the constructor did; the last PBI of a sound may free its activity chunk. */
void CAkPBI::Term( bool in_bFailedToInit )
{
	DecrementPlayCount();
	if( m_PathInfo.pPBPath )
	{
		if( m_PathInfo.pPBPath->m_PathMode & AkPathContinuous )
		{
			AkPathState * pPathState = m_pSound->GetPathState();
			pPathState->pbPlayed = NULL;
			pPathState->ulCurrentListIndex = 0;
		}
		g_pPathManager->RemovePathUser( m_PathInfo.pPBPath, this );
		m_PathInfo.pPBPath = NULL;
		m_PathInfo.PathOwnerID = 0;
	}
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, this );
	}
	if( m_PBTrans.pvPRTrans )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, this );
	}
	m_bNeedNotifyEndReached = 0;
	if( m_UserParams.m_PlayingID )
	{
		g_pPlayingMgr->Remove( m_UserParams.m_PlayingID, this );
	}
	if( m_pGameObj && --m_pGameObj->m_refCount <= 0 )
	{
		m_pGameObj->~CAkRegisteredObj();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pGameObj );
	}
	if( m_p3DSound )
	{
		CAkAttenuation * pAttenuation = m_p3DSound->m_Params.GetAttenuation();
		if( pAttenuation )
		{
			UnsubscribeAttenuationRTPC( pAttenuation );
		}
		m_p3DSound->~CAkGen3DParams();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_p3DSound );
		m_p3DSound = NULL;
	}
	m_mapMutedNodes.Term();
	m_pSound->m_pActivityChunk->m_listPBI.Remove( this );
	if( m_pSound->m_pActivityChunk->ChunkIsUseless() )
	{
		m_pSound->DeleteActivityChunk();
	}
	if( m_pDataPtr )
	{
		m_pSource->UnLockDataPtr();
	}
	if( m_pUsageSlot )
	{
		m_pUsageSlot->Release( false );
		m_pUsageSlot = NULL;
	}
	m_pSound->Release();
}

/** 2012 0x93efb0. */
void CAkPBI::_InitPlay()
{
	m_LoopCount = m_pSound->Loop();
	if( !m_bInitPlayWasCalled )
	{
		m_bInitPlayWasCalled = 1;
		if( m_PathInfo.pPBPath )
		{
			g_pPathManager->Start( m_PathInfo.pPBPath, m_pSound->GetPathState() );
		}
	}
}

/** 2012 0x93f7c0: a fade-in transition, then the play (or play-paused) command to the lower engine. */
void CAkPBI::_Play( TransParams & in_transParams, bool in_bPaused, bool in_bForceIgnoreSync )
{
	if( in_transParams.TransitionTime )
	{
		m_fPlayStopFadeRatio = 0.f;
		if( m_PBTrans.pvPSTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPSTrans, TransTarget_Play, 0.f, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		}
		else
		{
			TransitionParameters params;
			params.pUser = this;
			params.eTarget = TransTarget_Play;
			params.fStartValue = -96.3f;
			params.fTargetValue = 0.f;
			params.lDuration = in_transParams.TransitionTime;
			params.eFadeCurve = in_transParams.eFadeCurve;
			params.bdBs = true;
			params.bUseReciprocalCurve = true;
			m_PBTrans.pvPSTrans = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
			m_PBTrans.bIsPSTransFading = 0;
			m_bNeedsFadeIn = 1;
			if( !m_PBTrans.pvPSTrans )
			{
				TransUpdateValue( params.eTarget, params.fTargetValue, true );
			}
		}
	}
	if( in_bPaused || m_eInitialState == 1 )
	{
		m_bWasPaused = 1;
		CAkLEngineCmds::EnqueueAction( LEStatePlayPause, this );
		if( m_PBTrans.pvPSTrans )
		{
			g_pTransitionManager->Pause( m_PBTrans.pvPSTrans );
		}
		if( m_PathInfo.pPBPath )
		{
			g_pPathManager->Pause( m_PathInfo.pPBPath );
		}
	}
	else
	{
		CAkLEngineCmds::EnqueueAction( LEStatePlay, this );
	}
	if( m_eInitialState == 2 )
	{
		_Stop( AkPBIStopMode_Normal, false, false );
	}
	if( in_bForceIgnoreSync )
	{
		++CAkLEngineCmds::m_ulPlayEventID;
	}
}

/** 2012 0x93f000: the stop proper (once); transitions and paths released unless a continuation takes over. */
void CAkPBI::_Stop( AkPBIStopMode in_eStopMode, bool in_bIsFromTransition, bool in_bHasNotStarted )
{
	if( m_bWasStopped )
	{
		return;
	}
	m_bWasStopped = 1;
	if( !in_bIsFromTransition )
	{
		CAkLEngineCmds::EnqueueActionStop( this );
	}
	if( in_eStopMode == AkPBIStopMode_Normal || in_eStopMode == AkPBIStopMode_StopAndContinueSequel )
	{
		if( m_PBTrans.pvPSTrans )
		{
			g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, this );
			m_PBTrans.pvPSTrans = NULL;
		}
		if( m_PBTrans.pvPRTrans )
		{
			g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, this );
			m_PBTrans.pvPRTrans = NULL;
		}
		if( m_PathInfo.pPBPath )
		{
			if( m_PathInfo.pPBPath->m_PathMode & AkPathContinuous )
			{
				AkPathState * pPathState = m_pSound->GetPathState();
				pPathState->pbPlayed = NULL;
				pPathState->ulCurrentListIndex = 0;
			}
			g_pPathManager->RemovePathUser( m_PathInfo.pPBPath, this );
			m_PathInfo.pPBPath = NULL;
			m_PathInfo.PathOwnerID = 0;
		}
		m_bNeedNotifyEndReached = 0;
		m_bTerminatedByStop = 1;
	}
}

/** 2012 0x93f920: a stop with a fade-out (or the minimal fade when the voice is already playing). */
void CAkPBI::_Stop( const TransParams & in_transParams, bool in_bUseMinTransTime )
{
	if( m_bWasPaused || ( m_PBTrans.pvPRTrans && m_PBTrans.bIsPRTransFading ) )
	{
		_Stop( AkPBIStopMode_Normal, false, false );
		return;
	}
	m_bWasPreStopped = 1;
	if( in_transParams.TransitionTime )
	{
		if( m_PBTrans.pvPSTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPSTrans, TransTarget_Stop, -96.3f, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		}
		else
		{
			TransitionParameters params;
			params.pUser = this;
			params.eTarget = TransTarget_Stop;
			params.fStartValue = m_fPlayStopFadeRatio * 96.3f - 96.3f;
			params.fTargetValue = -96.3f;
			params.lDuration = in_transParams.TransitionTime;
			params.eFadeCurve = in_transParams.eFadeCurve;
			params.bdBs = true;
			params.bUseReciprocalCurve = true;
			m_PBTrans.pvPSTrans = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
			m_PBTrans.bIsPSTransFading = 1;
			m_bNeedsFadeIn = 0;
			if( !m_PBTrans.pvPSTrans )
			{
				TransUpdateValue( params.eTarget, params.fTargetValue, true );
			}
		}
	}
	else if( m_State )
	{
		if( m_PBTrans.pvPSTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPSTrans, TransTarget_Stop, -96.3f, 0, AkCurveInterpolation_Linear, AkValueMeaning_Default );
		}
		else if( in_bUseMinTransTime )
		{
			StopWithMinTransTime();
		}
	}
	else
	{
		_Stop( AkPBIStopMode_Normal, false, true );
	}
}

/** 2012 0x93f0d0. */
void CAkPBI::StopWithMinTransTime()
{
	if( !m_bTargetIsFeedback )
	{
		m_fPlayStopFadeRatio = 0.f;
		m_EffectiveParams.Volume = -96.3f;
	}
	_Stop( AkPBIStopMode_Normal, true, false );
}

/** 2012 0x93fab0. */
void CAkPBI::_Pause( bool in_bIsFromTransition )
{
	if( m_bWasStopped || m_bWasPaused )
	{
		return;
	}
	m_bWasPaused = 1;
	if( !m_bTargetIsFeedback )
	{
		m_fPauseResumeFadeRatio = 0.f;
		m_EffectiveParams.Volume = -96.3f;
	}
	if( !in_bIsFromTransition )
	{
		CAkLEngineCmds::EnqueueAction( LEStatePause, this );
	}
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->Pause( m_PBTrans.pvPSTrans );
	}
	if( m_PathInfo.pPBPath )
	{
		g_pPathManager->Pause( m_PathInfo.pPBPath );
	}
}

/** 2012 0x93fb30. */
void CAkPBI::_Pause( TransParams & in_transParams )
{
	++m_ulPauseCount;
	if( in_transParams.TransitionTime )
	{
		if( m_PBTrans.pvPRTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Pause, -96.3f, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		}
		else
		{
			TransitionParameters params;
			params.pUser = this;
			params.eTarget = TransTarget_Pause;
			params.fStartValue = m_fPauseResumeFadeRatio * 96.3f - 96.3f;
			params.fTargetValue = -96.3f;
			params.lDuration = in_transParams.TransitionTime;
			params.eFadeCurve = in_transParams.eFadeCurve;
			params.bdBs = true;
			params.bUseReciprocalCurve = true;
			m_PBTrans.pvPRTrans = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
			m_PBTrans.bIsPRTransFading = 1;
			if( !m_PBTrans.pvPRTrans )
			{
				TransUpdateValue( params.eTarget, params.fTargetValue, true );
			}
		}
	}
	else if( m_State )
	{
		if( m_PBTrans.pvPRTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Pause, -96.3f, 0, AkCurveInterpolation_Linear, AkValueMeaning_Default );
		}
		else
		{
			_Pause( true );
		}
	}
	else
	{
		_Pause( false );
	}
}

/** 2012 0x93fc80. */
void CAkPBI::_Resume()
{
	if( m_bWasStopped || !m_bWasPaused )
	{
		return;
	}
	if( m_PathInfo.pPBPath )
	{
		g_pPathManager->Resume( m_PathInfo.pPBPath );
	}
	m_bWasPaused = 0;
	CAkLEngineCmds::EnqueueAction( LEStateResume, this );
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->Resume( m_PBTrans.pvPSTrans );
	}
}

/** 2012 0x940180: a master resume clears every pause; otherwise one pause count per resume. */
void CAkPBI::_Resume( TransParams & in_transParams, bool in_bIsMasterResume )
{
	if( !in_bIsMasterResume && m_ulPauseCount > 1 )
	{
		--m_ulPauseCount;
		return;
	}
	m_ulPauseCount = 0;
	_Resume();
	if( in_transParams.TransitionTime )
	{
		if( m_PBTrans.pvPRTrans )
		{
			g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Resume, 0.f, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		}
		else
		{
			TransitionParameters params;
			params.pUser = this;
			params.eTarget = TransTarget_Resume;
			params.fStartValue = m_fPauseResumeFadeRatio * 96.3f - 96.3f;
			params.fTargetValue = 0.f;
			params.lDuration = in_transParams.TransitionTime;
			params.eFadeCurve = in_transParams.eFadeCurve;
			params.bdBs = true;
			params.bUseReciprocalCurve = true;
			m_PBTrans.pvPRTrans = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
			m_PBTrans.bIsPRTransFading = 0;
			if( !m_PBTrans.pvPRTrans )
			{
				TransUpdateValue( params.eTarget, params.fTargetValue, true );
			}
		}
	}
	else if( m_PBTrans.pvPRTrans )
	{
		g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Resume, 0.f, 0, AkCurveInterpolation_Linear, AkValueMeaning_Default );
	}
	else
	{
		m_fPauseResumeFadeRatio = 1.f;
		CalculateMutedEffectiveVolume();
	}
}

/** 2012 0x93f100. */
void CAkPBI::PlayToEnd( CAkParameterNodeBase * in_pNode )
{
	m_LoopCount = 1;
	CAkLEngineCmds::EnqueueAction( LEStateStopLooping, this );
}

/** 2012 0x93fce0: milliseconds to samples at the pipeline rate. */
void CAkPBI::SeekTimeAbsolute( AkTimeMs in_iPosition, bool in_bSnapToMarker )
{
	m_uSeekPosition = in_iPosition * ( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	m_bSeekRelativeToDuration = 0;
	m_bSnapSeekToMarker = in_bSnapToMarker;
	m_bSeekDirty = 1;
	CAkLEngineCmds::EnqueueAction( LEStateSeek, this );
}

/** 2012 0x93fd30. */
void CAkPBI::SeekPercent( AkReal32 in_fPercent, bool in_bSnapToMarker )
{
	m_fSeekPercent = in_fPercent;
	m_bSnapSeekToMarker = in_bSnapToMarker;
	m_bSeekRelativeToDuration = 1;
	m_bSeekDirty = 1;
	CAkLEngineCmds::EnqueueAction( LEStateSeek, this );
}

/** akpbi.h: only music PBIs handle a virtual voice coming back themselves. */
AkCtxVirtualHandlingResult CAkPBI::NotifyVirtualOff( AkVirtualQueueBehavior in_eBehavior )
{
	return VirtualHandling_NotHandled;
}

/** 2012 0x93fd70: fades and mutes scale the linear distance above -96.3 dB. */
void CAkPBI::CalculateMutedEffectiveVolume()
{
	AkReal32 fRatio = 1.f;
	for( MapStruct<AkMutedMapItem, AkReal32> * p = m_mapMutedNodes.m_pItems; p != m_mapMutedNodes.m_pItemsEnd; ++p )
	{
		fRatio *= p->item;
	}
	fRatio = fRatio * m_fPlayStopFadeRatio * m_fPauseResumeFadeRatio;
	if( fRatio <= 0.f )
	{
		fRatio = 0.f;
	}
	AkReal32 fVolume = m_Volume + m_Ranges.VolumeOffset;
	if( fVolume >= 0.f )
	{
		fVolume = 0.f;
	}
	m_EffectiveParams.Volume = ( fVolume + 96.3f ) * fRatio - 96.3f;
}

/** 2012 0x93f120. */
void CAkPBI::CalculateEffectiveLPF()
{
	m_EffectiveParams.LPF = m_LPFAutomationOffset + m_LPF;
}

/** 2012 0x93fdf0: the play/stop and pause/resume fades drive their ratio; a finished stop / pause takes effect. */
void CAkPBI::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	switch( in_eTarget )
	{
	case TransTarget_Play:
	case TransTarget_Stop:
		if( in_bIsTerminated )
		{
			m_PBTrans.pvPSTrans = NULL;
			if( in_eTarget == TransTarget_Stop )
			{
				_Stop( AkPBIStopMode_Normal, true, false );
			}
		}
		m_fPlayStopFadeRatio = ( in_fValue + 96.3f ) * 0.010384216f;
		break;
	case TransTarget_Pause:
	case TransTarget_Resume:
		if( in_bIsTerminated )
		{
			m_PBTrans.pvPRTrans = NULL;
			if( in_eTarget == TransTarget_Pause )
			{
				_Pause( true );
			}
		}
		m_fPauseResumeFadeRatio = ( in_fValue + 96.3f ) * 0.010384216f;
		break;
	default:
		break;
	}
	CalculateMutedEffectiveVolume();
}

/** 2012 0x93f2a0. */
void CAkPBI::Play( AkReal32 in_fDuration )
{
	m_State = CtxStatePlay;
	CAkURenderer::EnqueueContextNotif( this, CtxStatePlay, CtxDestroyReasonFinished, in_fDuration );
}

/** 2012 0x93f2d0. */
void CAkPBI::Stop()
{
	m_State = CtxStateStop;
}

/** 2012 0x93f2e0. */
void CAkPBI::Pause()
{
	m_State = CtxStatePause;
	CAkURenderer::EnqueueContextNotif( this, CtxStatePause, CtxDestroyReasonFinished, 0.f );
}

/** 2012 0x93f310. */
void CAkPBI::Resume()
{
	m_State = CtxStatePlay;
	CAkURenderer::EnqueueContextNotif( this, CtxStateResume, CtxDestroyReasonFinished, 0.f );
}

/** 2012 0x93f340. */
void CAkPBI::NotifAddedAsSA()
{
	m_State = CtxStatePlay;
}

/** 2012 0x93f280: the voice is gone; the renderer frees the PBI after this frame. */
void CAkPBI::Destroy( AkCtxDestroyReason in_eReason )
{
	m_pCbx = NULL;
	CAkURenderer::EnqueueContextNotif( this, CtxStateToDestroy, in_eReason, 0.f );
}

/** 2012 0x93f360. */
void CAkPBI::ProcessContextNotif( AkCtxState in_eState, AkCtxDestroyReason in_eDestroyReason, AkReal32 in_fEstimatedLength )
{
	switch( in_eState )
	{
	case CtxStatePlay:
		m_bNeedNotifyEndReached = 1;
		PrepareSampleAccurateTransition();
		SetEstimatedLength( in_fEstimatedLength );
		break;
	case CtxStateToDestroy:
		if( in_eDestroyReason == CtxDestroyReasonPlayFailed )
		{
			m_bNeedNotifyEndReached = 0;
			m_bPlayFailed = 1;
		}
		break;
	default:
		break;
	}
}

/** 2012 0x940070. */
void CAkPBI::Kick( KickFrom in_eIsForMemoryThreshold )
{
	m_bWasKicked = 1;
	m_eWasKickedForMemory = in_eIsForMemoryThreshold;
	TransParams transParams;
	transParams.TransitionTime = 0;
	transParams.eFadeCurve = AkCurveInterpolation_Linear;
	_Stop( transParams, true );
}

/** 2012 0x9400c0. */
void CAkPBI::ForceVirtualize( KickFrom in_eReason )
{
	AkVirtualQueueBehavior _unused;
	switch( GetVirtualBehavior( _unused ) )
	{
	case AkBelowThresholdBehavior_KillVoice:
		Kick( in_eReason );
		break;
	case AkBelowThresholdBehavior_SetAsVirtualVoice:
		m_bIsForcedToVirtualize = 1;
		break;
	default:
		break;
	}
}

/** 2012 0x93f640. */
void CAkPBI::Virtualize()
{
	if( m_bIsVirtual )
	{
		return;
	}
	m_bIsVirtual = 1;
	++CAkURenderer::m_uNumVirtualizedSounds;
	CounterParameters counterParams;
	counterParams.pGameObj = m_pGameObj;
	m_pSound->IncrementVirtualCount( counterParams );
}

/** 2012 0x93f6b0. */
void CAkPBI::Devirtualize( bool in_bAllowKick )
{
	if( !m_bIsVirtual )
	{
		return;
	}
	m_bIsVirtual = 0;
	CAkURenderer::DecrementVirtualCount( in_bAllowKick );
	CounterParameters counterParams;
	counterParams.pGameObj = m_pGameObj;
	counterParams.bAllowKick = in_bAllowKick;
	m_pSound->DecrementVirtualCount( counterParams );
}

/** 2012 0x93ff20: once, out of every limiter and down the play counts. */
void CAkPBI::DecrementPlayCount()
{
	if( m_bIsVirtual )
	{
		Devirtualize( false );
	}
	if( m_bWasPlayCountDecremented )
	{
		return;
	}
	m_bWasPlayCountDecremented = 1;
	if( m_pAMLimiter )
	{
		m_pAMLimiter->Remove( this, AKVoiceLimiter_AM );
		m_pAMLimiter = NULL;
	}
	if( m_pBusLimiter )
	{
		m_pBusLimiter->Remove( this, AKVoiceLimiter_Bus );
		m_pBusLimiter = NULL;
	}
	CAkURenderer::m_GlobalLimiter.Remove( this, AKVoiceLimiter_Global );
	CounterParameters counterParams;
	counterParams.pGameObj = m_pGameObj;
	m_pSound->DecrementPlayCount( counterParams );
}

/** 2012 0x93f210: cached after the first query. */
AkBelowThresholdBehavior CAkPBI::GetVirtualBehavior( AkVirtualQueueBehavior & out_Behavior )
{
	if( m_bVirtualBehaviorCached )
	{
		out_Behavior = (AkVirtualQueueBehavior)m_eCachedVirtualQueueBehavior;
		return (AkBelowThresholdBehavior)m_eCachedBelowThresholdBehavior;
	}
	m_bVirtualBehaviorCached = 1;
	const AkBelowThresholdBehavior eBehavior = m_pSound->GetVirtualBehavior( out_Behavior );
	m_eCachedVirtualQueueBehavior = out_Behavior;
	m_eCachedBelowThresholdBehavior = eBehavior;
	return eBehavior;
}

/** 2012 0x93f420. */
void CAkPBI::UpdatePriority( AkReal32 in_NewPriority )
{
	if( m_PriorityInfoCurrent.currentPriority.priority == in_NewPriority )
	{
		return;
	}
	if( m_pAMLimiter )
	{
		m_pAMLimiter->Update( in_NewPriority, this );
	}
	if( m_pBusLimiter )
	{
		m_pBusLimiter->Update( in_NewPriority, this );
	}
	CAkURenderer::m_GlobalLimiter.Update( in_NewPriority, this );
	m_PriorityInfoCurrent.currentPriority.priority = in_NewPriority;
}

/** 2012 0x93f140. */
void CAkPBI::RecalcNotification()
{
	m_bAreParametersValid = 0;
	m_bFeedbackParametersValid = 0;
}

/** 2012 0x940700. */
void CAkPBI::CalcEffectiveParams()
{
	if( !m_bAreParametersValid )
	{
		RefreshParameters();
	}
}

/** 2012 0x940490: only the persistent mute entries survive a refresh. */
void CAkPBI::RemoveAllVolatileMuteItems()
{
	MapStruct<AkMutedMapItem, AkReal32> * p = m_mapMutedNodes.m_pItems;
	while( p != m_mapMutedNodes.m_pItemsEnd )
	{
		if( p->key.m_bIsPersistent )
		{
			++p;
			continue;
		}
		if( m_mapMutedNodes.Length() > 1 )
		{
			*p = *( m_mapMutedNodes.m_pItemsEnd - 1 );
		}
		--m_mapMutedNodes.m_pItemsEnd;
	}
}

/** 2012 0x9404f0: the node hierarchy's parameters (GetAudioParameters, IH3 seam), ranges, fades and priority. */
void CAkPBI::RefreshParameters()
{
	m_EffectiveParams.Volume = 0.f;
	m_EffectiveParams.Pitch = 0.f;
	m_EffectiveParams.LPF = 0.f;
	RemoveAllVolatileMuteItems();
	const bool bPositioningFromRTPC = m_pSound->UpdateBaseParams( m_pGameObj, &m_BasePosParams );
	if( m_pCbx )
	{
		m_pCbx->RefreshBypassFx();
	}
	m_bBypassAllFX = m_pSound->GetBypassAllFX( m_pGameObj );
	if( m_p3DSound )
	{
		m_p3DSound->m_Params.m_bIsPanningFromRTPC = bPositioningFromRTPC;
	}
	const bool bFirstRefresh = !m_bGetAudioParamsCalled;
	m_pSound->GetAudioParameters( m_EffectiveParams, 0xFFFFFFFF, m_mapMutedNodes, m_pGameObj, !m_bGetAudioParamsCalled, m_Ranges, true );
	m_bGetAudioParamsCalled = 1;
	m_LPF = m_Ranges.LPFOffset + m_EffectiveParams.LPF;
	m_EffectiveParams.Pitch += m_Ranges.PitchOffset;
	m_EffectiveParams.LPF = m_LPFAutomationOffset + m_LPF;
	m_Volume = m_EffectiveParams.Volume;
	CalculateMutedEffectiveVolume();
	if( DishonoredWwise::IsDiagnostics() )
	{
		AkDiagVoiceParameters( this, bFirstRefresh );
	}
	const PriorityInfo priorityInfo = m_pSound->GetPriority( m_pGameObj );
	if( priorityInfo.priority != m_PriorityInfoCurrent.priorityInfo.priority || priorityInfo.distanceOffset != m_PriorityInfoCurrent.priorityInfo.distanceOffset )
	{
		m_PriorityInfoCurrent.priorityInfo = priorityInfo;
		UpdatePriority( priorityInfo.priority );
	}
	m_bAreParametersValid = 1;
}

/** 2012 0x940980. */
void CAkPBI::MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems )
{
	if( in_bPrioritizeGameObjectSpecificItems )
	{
		AkMutedMapItem searchItem = in_rMutedItem;
		searchItem.m_bIsGlobal = !in_rMutedItem.m_bIsGlobal;
		if( m_mapMutedNodes.Exists( searchItem ) )
		{
			if( !in_rMutedItem.m_bIsGlobal )
			{
				m_mapMutedNodes.Unset( searchItem );
			}
			else
			{
				return;
			}
		}
	}
	if( in_fMuteRatio == 1.f && ( !in_bPrioritizeGameObjectSpecificItems || in_rMutedItem.m_bIsGlobal ) )
	{
		m_mapMutedNodes.Unset( in_rMutedItem );
	}
	else
	{
		m_mapMutedNodes.Set( in_rMutedItem, in_fMuteRatio );
	}
	CalculateMutedEffectiveVolume();
}

/** 2012 0x940ab0. */
AKRESULT CAkPBI::SetMuteMapEntry( AkMutedMapItem & in_key, AkReal32 in_fFadeRatio )
{
	AKRESULT eResult = AK_Success;
	if( in_fFadeRatio == 1.f )
	{
		m_mapMutedNodes.Unset( in_key );
	}
	else if( !m_mapMutedNodes.Set( in_key, in_fFadeRatio ) )
	{
		eResult = AK_Fail;
	}
	CalculateMutedEffectiveVolume();
	return eResult;
}

/** 2012 0x93f150: target 0x10 is the "bypass all" bit. */
void CAkPBI::NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	if( m_pCbx && ( in_uTargetMask & ~0x10 ) )
	{
		m_pCbx->SetFxBypass( in_bitsFXBypass, in_uTargetMask );
	}
	if( in_uTargetMask & 0x10 )
	{
		m_bBypassAllFX = ( in_bitsFXBypass >> 4 ) & 1;
	}
}

/** 2012 0x93f1e0. */
bool CAkPBI::IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest )
{
	if( !in_NodeIDToTest )
	{
		return true;
	}
	return m_pSound ? m_pSound->IsInstanceCountCompatible( in_NodeIDToTest, false, false ) : false;
}

/** 2012 0x93f620. */
bool CAkPBI::IsUsingThisSlot( const CAkUsageSlot * in_pSlotToCheck )
{
	if( m_pUsageSlot == in_pSlotToCheck )
	{
		return true;
	}
	return m_pCbx ? m_pCbx->IsUsingThisSlot( in_pSlotToCheck ) : false;
}

/** 2012 0x93f3e0. */
AK::CAkBusCtx CAkPBI::GetBusContext()
{
	AK::CAkBusCtx busCtx;
	busCtx.m_pBus = m_pSound->GetMixingBus();
	return busCtx;
}

/** 2012 0x93f490: the object's sends, minus the bypassed environments. DISHONORED(IH3 seam): nothing is bypassed. */
void CAkPBI::GetEnvironmentValues( AkEnvironmentValue * io_paEnvVal )
{
	AkUInt32 uNumValues = 0;
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		const AkEnvironmentValue & rValue = m_pGameObj->m_EnvironmentValues[ i ];
		if( !rValue.EnvID )
		{
			break;
		}
		io_paEnvVal[ uNumValues++ ] = rValue;
	}
	if( uNumValues < 4 )
	{
		io_paEnvVal[ uNumValues ].EnvID = AK_INVALID_UNIQUE_ID;
	}
}

/** 2012 0x93f3d0. */
AkUniqueID CAkPBI::GetSoundID() const
{
	return m_pSound->key;
}

/** 2012 0x93f5f0: the feedback (motion) parameters are rebuilt on next use; no feedback device exists, so there is never
    an m_pFeedbackInfo to free. */
void CAkPBI::InvalidateFeedbackParameters()
{
	m_bAreParametersValid = 0;
	m_bFeedbackParametersValid = 0;
	m_pFeedbackInfo = NULL;
}

/** 2012 0x93f1a0 (IH3 fork R): an attenuation RTPC subscription of the PBI writes the cone outside volume / LPF of its
    3D parameters (the other attenuation parameters are read from the RTPC manager when the volumes are computed). */
AKRESULT CAkPBI::SetParam( AkInt16 in_paramID, const void * in_pParam, AkUInt32 )
{
	if( !m_p3DSound )
	{
		return AK_Success;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pParam;
	if( in_paramID == RTPC_Positioning_Cone_Attenuation )
	{
		m_p3DSound->m_Params.m_fConeOutsideVolume = fValue;
	}
	else if( in_paramID == RTPC_Positioning_Cone_LPF )
	{
		m_p3DSound->m_Params.m_fConeLoPass = fValue;
	}
	return AK_Success;
}
