/*=============================================================================
	AkURenderer.cpp - CAkURenderer (akurenderer.cpp, 2012 0x91ee90..0x920630; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26).
	Deviations (named):
	  * Play: external sources (codec 8, ResolveExternalSource 0x91f2e0) are not resolved - the glue never posts
	    external sources, and such a play is refused as retail refuses an unresolvable one (AK_Fail, play count undone).
	  * The monitoring notifications (MONITOR_OBJECTNOTIF: play failed / continue aborted) are compiled out.
	  * GetMaxRadius (Query::GetMaxRadiusAudio) and InvalidateAllMotionPBIs are not ported: the first is IH3's
	    positioning (IH1's Query answers GetMaxRadius from the node data), the second is the motion (feedback) pipeline.
=============================================================================*/
#include "AkURenderer.h"
#include "AkLEngine.h"
#include "AkLEngineCmds.h"
#include "AkListener.h"
#include "AkPlayingMgr.h"
#include "AkMath.h"

AkListBare<CAkPBI>														CAkURenderer::m_listCtxs;
CAkList2<CAkURenderer::ContextNotif, const CAkURenderer::ContextNotif &, 1, ArrayPoolDefault>	CAkURenderer::m_CtxNotifQueue;
AkListBareLight<CAkLimiter>												CAkURenderer::m_BusLimiters;
AkListBareLight<CAkLimiter>												CAkURenderer::m_AMLimiters;
CAkLimiter																CAkURenderer::m_GlobalLimiter( 0, true, true );
AkUInt32																CAkURenderer::m_uNumVirtualizedSounds = 0;

/** 2012 0x91efc0: a pool used past its ratio threshold. */
static bool CheckOverRatio( AkReal32 in_fLimitRatio, AkMemPoolId in_poolID )
{
	if( in_fLimitRatio >= 1.f )
	{
		return false;
	}
	AK::MemoryMgr::PoolMemInfo info;
	AK::MemoryMgr::GetPoolMemoryUsed( in_poolID, info );
	return info.uReserved && (double)info.uUsed / (double)info.uReserved > in_fLimitRatio;
}

/** 2012 0x91f4c0: 64 context notifications pre-allocated, then the lower engine. */
AKRESULT CAkURenderer::Init()
{
	m_uNumVirtualizedSounds = 0;
	if( m_CtxNotifQueue.Init( 64, (AkUInt32)-1 ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	return CAkLEngine::Init();
}

/** 2012 0x920270. */
void CAkURenderer::Term()
{
	CAkLEngine::Term();
	DestroyAllPBIs();
	m_listCtxs.RemoveAll();
	m_CtxNotifQueue.Term();
	m_GlobalLimiter.Term();
	m_AMLimiters.m_pFirst = NULL;
	m_BusLimiters.m_pFirst = NULL;
}

/** 2012 0x91f1f0: the node's priority, offset by the distance to the nearest listener within the max radius. */
PriorityInfoCurrent CAkURenderer::_CalcInitialPriority( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, AkReal32 & out_fMaxRadius )
{
	out_fMaxRadius = 0.f;
	PriorityInfoCurrent priority;
	priority.priorityInfo = in_pSound->GetPriority( in_pGameObj );
	priority.currentPriority.priority = priority.priorityInfo.priority;
	priority.currentPriority.PBIID = 0;
	if( in_pSound->GetMaxRadius( out_fMaxRadius ) )
	{
		out_fMaxRadius *= in_pGameObj->m_fScalingFactor;
		if( priority.priorityInfo.distanceOffset != 0.f )
		{
			const AkReal32 fMinDistance = GetMinDistance( in_pGameObj->m_PosKeep );
			AkReal32 fPriority;
			if( out_fMaxRadius <= fMinDistance )
			{
				fPriority = priority.priorityInfo.priority + priority.priorityInfo.distanceOffset;
			}
			else
			{
				fPriority = ( fMinDistance / out_fMaxRadius ) * priority.priorityInfo.distanceOffset + priority.priorityInfo.priority;
			}
			if( fPriority < 0.f )
			{
				fPriority = 0.f;
			}
			else if( fPriority > 100.f )
			{
				fPriority = 100.f;
			}
			priority.currentPriority.priority = fPriority;
		}
	}
	return priority;
}

/** 2012 0x91ee90: the scaled distance of the nearest position to any active listener (0 for the object's own listener). */
AkReal32 CAkURenderer::GetMinDistance( const AkPositionKeeper & in_rPosKeep )
{
	AkReal32 fMinDistance = 1.0e10f;
	AkUInt32 uMask = in_rPosKeep.m_uListenerMask;
	for( AkUInt32 uListener = 0; uMask; ++uListener, uMask >>= 1 )
	{
		if( !( uMask & 1 ) )
		{
			continue;
		}
		if( in_rPosKeep.m_iListenerIdx == (AkInt32)uListener )
		{
			return 0.f;
		}
		const AkVector & listenerPos = CAkListener::m_listeners[ uListener ].data.Pos.Position;
		if( in_rPosKeep.m_uNumPos )
		{
			for( AkUInt32 i = 0; i < in_rPosKeep.m_uNumPos; ++i )
			{
				const AkReal32 fDistance = AkMath::Distance( listenerPos, in_rPosKeep.m_aPos[ i ].Position ) / CAkListener::GetScalingFactor( uListener );
				if( fDistance <= fMinDistance )
				{
					fMinDistance = fDistance;
				}
			}
		}
		else
		{
			const AkReal32 fDistance = AkMath::Distance( listenerPos, g_DefaultSoundPosition.Position ) / CAkListener::GetScalingFactor( uListener );
			if( fDistance <= fMinDistance )
			{
				fMinDistance = fDistance;
			}
		}
	}
	return fMinDistance;
}

/** 2012 0x9202c0: limits, play count up the hierarchy, then the instigator's PBI, Init and the play itself. */
AKRESULT CAkURenderer::Play( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams )
{
	AkReal32 fMaxRadius;
	const PriorityInfoCurrent priority = _CalcInitialPriority( in_pSound, in_rPBIParams.pGameObj, fMaxRadius );
	if( !ValidateMemoryLimit( priority.currentPriority.priority ) )
	{
		return AK_PartialSuccess;
	}
	const AKRESULT eValidateLimitsResult = ValidateMaximumNumberVoiceLimit( priority.currentPriority.priority );
	if( eValidateLimitsResult == AK_Fail )
	{
		return AK_PartialSuccess;
	}
	CounterParameters counterParams;
	counterParams.fPriority = priority.currentPriority.priority;
	counterParams.pGameObj = in_rPBIParams.pGameObj;
	const AKRESULT eIncrementResult = in_pSound->IncrementPlayCount( counterParams );
	bool bAllowedToPlay = eIncrementResult != AK_Fail;
	if( eIncrementResult == AK_MustBeVirtualized || eValidateLimitsResult == AK_MustBeVirtualized )
	{
		AkVirtualQueueBehavior _unused;
		bAllowedToPlay = in_pSound->GetVirtualBehavior( _unused ) != AkBelowThresholdBehavior_KillVoice;
	}
	CounterParameters decrementParams;
	decrementParams.pGameObj = in_rPBIParams.pGameObj;
	if( !bAllowedToPlay )
	{
		in_pSound->DecrementPlayCount( decrementParams );
		return AK_PartialSuccess;
	}
	if( CODECID_FROM_PLUGINID( in_pSource->m_sSrcTypeInfo.dwID ) == AKCODECID_EXTERNAL_SOURCE )
	{
		in_pSound->DecrementPlayCount( decrementParams );
		return AK_Fail;
	}
	CAkPBI * pContext = in_rPBIParams.pInstigator->CreatePBI( in_pSound, in_pSource, in_rPBIParams, priority, counterParams.pAMLimiter, counterParams.pBusLimiter );
	if( !pContext )
	{
		in_pSound->DecrementPlayCount( decrementParams );
		return AK_Fail;
	}
	AkPathInfo pathInfo;
	pathInfo.pPBPath = NULL;
	pathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
	const AKRESULT eInitResult = pContext->Init( in_rPBIParams.eType != AkPBIParams::PBI ? in_rPBIParams.pContinuousParams->pPathInfo : &pathInfo );
	if( eInitResult != AK_Success )
	{
		pContext->Term( true );
		pContext->~CAkPBI();
		AK::MemoryMgr::Free( g_DefaultPoolId, pContext );
		return AK_Fail;
	}
	pContext->m_fMaxDistance = fMaxRadius;
	pContext->m_iFrameOffset = in_rPBIParams.uFrameOffset;
	Play( pContext, *in_rPBIParams.pTransitionParameters, in_rPBIParams.ePlaybackState );
	return AK_Success;
}

/** 2012 0x91f980: the memory limit fails outright, then the voice limit decides. */
AKRESULT CAkURenderer::ValidateLimits( AkReal32 in_fPriority )
{
	if( !ValidateMemoryLimit( in_fPriority ) )
	{
		return AK_Fail;
	}
	return ValidateMaximumNumberVoiceLimit( in_fPriority );
}

/** 2012 0x91fc60. */
void CAkURenderer::Play( CAkPBI * in_pContext, TransParams & in_rTparameters, AkPlaybackState in_ePlaybackState )
{
	m_listCtxs.AddLast( in_pContext );
	in_pContext->_InitPlay();
	in_pContext->_Play( in_rTparameters, in_ePlaybackState == PB_Paused, false );
}

/** 2012 0x91f550. */
AKRESULT CAkURenderer::ProcessCommand( ActionParamType in_eCommand, CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID, TransParams & in_rTparameters, bool in_bIsMasterResume )
{
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		if( pPBI->m_pSound != in_pSound || ( in_pGameObj && pPBI->m_pGameObj != in_pGameObj ) || ( in_PlayingID && pPBI->GetPlayingID() != in_PlayingID ) )
		{
			continue;
		}
		switch( in_eCommand )
		{
		case ActionParamType_Stop:
			pPBI->_Stop( in_rTparameters, true );
			break;
		case ActionParamType_Pause:
			pPBI->_Pause( in_rTparameters );
			break;
		case ActionParamType_Resume:
			pPBI->_Resume( in_rTparameters, in_bIsMasterResume );
			break;
		default:
			break;
		}
	}
	return AK_Success;
}

/** 2012 0x91fcc0. */
AKRESULT CAkURenderer::Stop( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, AkPlayingID in_PlayingID )
{
	return ProcessCommand( ActionParamType_Stop, in_pSound, in_pGameObj, in_PlayingID, in_rTparameters, false );
}

/** 2012 0x91fcf0. */
AKRESULT CAkURenderer::Pause( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, AkPlayingID in_PlayingID )
{
	return ProcessCommand( ActionParamType_Pause, in_pSound, in_pGameObj, in_PlayingID, in_rTparameters, false );
}

/** 2012 0x91fd50. */
AKRESULT CAkURenderer::Resume( CAkSoundBase * in_pSound, CAkRegisteredObj * in_pGameObj, TransParams & in_rTparameters, bool in_bIsMasterResume, AkPlayingID in_PlayingID )
{
	return ProcessCommand( ActionParamType_Resume, in_pSound, in_pGameObj, in_PlayingID, in_rTparameters, in_bIsMasterResume );
}

/** 2012 0x91fa40: a bank going away stops every PBI and mix bus using its data. */
void CAkURenderer::StopAllPBIs( const CAkUsageSlot * in_pUsageSlot )
{
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		if( pPBI->IsUsingThisSlot( in_pUsageSlot ) )
		{
			pPBI->_Stop( AkPBIStopMode_Normal, false, false );
			g_pAudioMgr->StopPendingAction( pPBI->m_pSound->key, NULL, AK_INVALID_PLAYING_ID );
		}
	}
	CAkLEngine::StopMixBussesUsingThisSlot( in_pUsageSlot );
}

/** 2012 0x91f9c0. */
void CAkURenderer::DestroyAllPBIs()
{
	while( CAkPBI * pPBI = m_listCtxs.RemoveFirst() )
	{
		pPBI->_Stop( AkPBIStopMode_Normal, true, false );
		CAkLEngineCmds::DequeuePBI( pPBI );
		pPBI->Term( false );
		pPBI->~CAkPBI();
		AK::MemoryMgr::Free( g_DefaultPoolId, pPBI );
	}
}

/** 2012 0x91f5e0. */
void CAkURenderer::EnqueueContext( CAkPBI * in_pContext )
{
	m_listCtxs.AddLast( in_pContext );
}

/** 2012 0x920630: a full queue is flushed first. */
void CAkURenderer::EnqueueContextNotif( CAkPBI * in_pPBI, AkCtxState in_eState, AkCtxDestroyReason in_eDestroyReason, AkReal32 in_fEstimatedTime )
{
	ContextNotif * pNotif = m_CtxNotifQueue.AddLast();
	if( !pNotif )
	{
		PerformContextNotif();
		pNotif = m_CtxNotifQueue.AddLast();
	}
	pNotif->pPBI = in_pPBI;
	pNotif->state = in_eState;
	pNotif->DestroyReason = in_eDestroyReason;
	pNotif->fEstimatedLength = in_fEstimatedTime;
}

/** 2012 0x91feb0: after each audio frame; a context to destroy leaves the renderer and is freed. */
void CAkURenderer::PerformContextNotif()
{
	while( m_CtxNotifQueue.Length() )
	{
		const ContextNotif notif = m_CtxNotifQueue.First();
		notif.pPBI->ProcessContextNotif( notif.state, notif.DestroyReason, notif.fEstimatedLength );
		if( notif.state == CtxStateToDestroy )
		{
			m_listCtxs.Remove( notif.pPBI );
			CAkLEngineCmds::DequeuePBI( notif.pPBI );
			notif.pPBI->Term( false );
			notif.pPBI->~CAkPBI();
			AK::MemoryMgr::Free( g_DefaultPoolId, notif.pPBI );
		}
		m_CtxNotifQueue.RemoveFirst();
	}
}

/** 2012 0x91f620: the weakest kickable PBI (optionally of one game object, compatible with the node's instance count). */
AKRESULT CAkURenderer::Kick( AkReal32 in_fPriority, CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, bool in_bKickNewest, bool in_bUseVirtualBehavior, CAkParameterNodeBase *& out_pKicked, KickFrom in_eReason )
{
	AkReal32 fPriorityWeakest = 101.f;
	CAkPBI * pWeakest = NULL;
	AkBelowThresholdBehavior eBehavior = AkBelowThresholdBehavior_SetAsVirtualVoice;
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		if( in_pGameObj && pPBI->m_pGameObj != in_pGameObj )
		{
			continue;
		}
		const AkReal32 fPriority = pPBI->m_PriorityInfoCurrent.currentPriority.priority;
		if( fPriorityWeakest <= fPriority && ( !in_bKickNewest || fPriority != fPriorityWeakest ) )
		{
			continue;
		}
		if( pPBI->m_bWasKicked || pPBI->m_bIsForcedToVirtualize || pPBI->m_bIsVirtual )
		{
			continue;
		}
		if( !pPBI->IsInstanceCountCompatible( in_NodeID ) )
		{
			continue;
		}
		if( in_bUseVirtualBehavior )
		{
			AkVirtualQueueBehavior _unused;
			const AkBelowThresholdBehavior eThisBehavior = pPBI->GetVirtualBehavior( _unused );
			if( eThisBehavior == AkBelowThresholdBehavior_ContinueToPlay )
			{
				continue;
			}
			eBehavior = eThisBehavior;
		}
		fPriorityWeakest = fPriority;
		pWeakest = pPBI;
	}
	if( fPriorityWeakest > in_fPriority || ( in_bKickNewest && in_fPriority == fPriorityWeakest ) || !pWeakest )
	{
		return in_bUseVirtualBehavior ? AK_MustBeVirtualized : AK_Fail;
	}
	out_pKicked = pWeakest->m_pSound;
	if( !in_bUseVirtualBehavior || eBehavior == AkBelowThresholdBehavior_KillVoice )
	{
		pWeakest->Kick( in_eReason );
	}
	return AK_Success;
}

/** 2012 0x91f760: over a pool ratio threshold, the weakest voice is kicked for a stronger one. */
bool CAkURenderer::ValidateMemoryLimit( AkReal32 in_fPriority )
{
	if( !CheckOverRatio( g_settings.fDefaultPoolRatioThreshold, g_DefaultPoolId ) && !CheckOverRatio( g_PDSettings.fLEngineDefaultPoolRatioThreshold, g_LEngineDefaultPoolId ) )
	{
		return true;
	}
	AkReal32 fPriorityWeakest = 101.f;
	CAkPBI * pWeakest = NULL;
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		const AkReal32 fPriority = pPBI->m_PriorityInfoCurrent.currentPriority.priority;
		if( fPriorityWeakest >= fPriority && !pPBI->m_bWasKicked && !pPBI->m_bIsForcedToVirtualize && !pPBI->m_bIsVirtual && pPBI->IsInstanceCountCompatible( 0 ) )
		{
			fPriorityWeakest = fPriority;
			pWeakest = pPBI;
		}
	}
	if( fPriorityWeakest >= in_fPriority || !pWeakest )
	{
		return false;
	}
	pWeakest->Kick( KickFrom_OverMemoryLimit );
	return true;
}

/** 2012 0x91f880: over the global voice limit, the weakest virtualizable voice is kicked or the new one is virtual. */
AKRESULT CAkURenderer::ValidateMaximumNumberVoiceLimit( AkReal32 in_fPriority )
{
	if( m_listCtxs.Length() - m_uNumVirtualizedSounds + 1 <= m_GlobalLimiter.m_u16LimiterMax )
	{
		return AK_Success;
	}
	AkReal32 fPriorityWeakest = 101.f;
	CAkPBI * pWeakest = NULL;
	AkBelowThresholdBehavior eBehavior = AkBelowThresholdBehavior_SetAsVirtualVoice;
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		const AkReal32 fPriority = pPBI->m_PriorityInfoCurrent.currentPriority.priority;
		if( fPriorityWeakest >= fPriority && !pPBI->m_bWasKicked && !pPBI->m_bIsForcedToVirtualize && !pPBI->m_bIsVirtual && pPBI->IsInstanceCountCompatible( 0 ) )
		{
			AkVirtualQueueBehavior _unused;
			const AkBelowThresholdBehavior eThisBehavior = pPBI->GetVirtualBehavior( _unused );
			if( eThisBehavior != AkBelowThresholdBehavior_ContinueToPlay )
			{
				fPriorityWeakest = fPriority;
				eBehavior = eThisBehavior;
				pWeakest = pPBI;
			}
		}
	}
	if( fPriorityWeakest >= in_fPriority || !pWeakest )
	{
		return AK_MustBeVirtualized;
	}
	if( eBehavior == AkBelowThresholdBehavior_KillVoice )
	{
		pWeakest->Kick( KickFrom_OverGlobalLimit );
	}
	return AK_Success;
}

/** 2012 0x91fdb0: a voice coming back from virtual may push the weakest killable voice out. */
void CAkURenderer::DecrementVirtualCount( bool in_bAllowKick )
{
	--m_uNumVirtualizedSounds;
	if( !in_bAllowKick || m_listCtxs.Length() - m_uNumVirtualizedSounds <= m_GlobalLimiter.m_u16LimiterMax )
	{
		return;
	}
	AkReal32 fPriorityWeakest = 101.f;
	CAkPBI * pWeakest = NULL;
	AkBelowThresholdBehavior eBehavior = AkBelowThresholdBehavior_SetAsVirtualVoice;
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		const AkReal32 fPriority = pPBI->m_PriorityInfoCurrent.currentPriority.priority;
		if( fPriorityWeakest >= fPriority && !pPBI->m_bWasKicked && !pPBI->m_bIsForcedToVirtualize && !pPBI->m_bIsVirtual && pPBI->IsInstanceCountCompatible( 0 ) )
		{
			AkVirtualQueueBehavior _unused;
			const AkBelowThresholdBehavior eThisBehavior = pPBI->GetVirtualBehavior( _unused );
			if( eThisBehavior != AkBelowThresholdBehavior_ContinueToPlay )
			{
				fPriorityWeakest = fPriority;
				eBehavior = eThisBehavior;
				pWeakest = pPBI;
			}
		}
	}
	if( fPriorityWeakest < 101.f && pWeakest && eBehavior == AkBelowThresholdBehavior_KillVoice )
	{
		pWeakest->Kick( KickFrom_OverGlobalLimit );
	}
}

/** 2012 0x91fb90: this frame's forced-virtual flag becomes last frame's, then every limiter re-evaluates. */
void CAkURenderer::ProcessLimiters()
{
	for( CAkPBI * pPBI = m_listCtxs.First(); pPBI; pPBI = pPBI->pNextItem )
	{
		pPBI->m_bWasForcedToVirtualize = pPBI->m_bIsForcedToVirtualize;
		pPBI->m_bIsForcedToVirtualize = 0;
	}
	m_GlobalLimiter.UpdateFlags();
	for( CAkLimiter * pLimiter = m_BusLimiters.First(); pLimiter; pLimiter = pLimiter->pNextLightItem )
	{
		pLimiter->UpdateFlags();
	}
	for( CAkLimiter * pLimiter = m_AMLimiters.First(); pLimiter; pLimiter = pLimiter->pNextLightItem )
	{
		pLimiter->UpdateFlags();
	}
}
