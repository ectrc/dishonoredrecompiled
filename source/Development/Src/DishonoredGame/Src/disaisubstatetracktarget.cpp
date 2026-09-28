// DishonoredGame/src/disaisubstatetracktarget.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateTrackTarget and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateTrackTarget_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x782380
FDisAISubStateTrackTarget_Param::FDisAISubStateTrackTarget_Param( const FDisAttentionProxy& _rTrackTargetProxy, FLOAT _fMaxFunnelRadiusMultiplier )
	: FDisAISubState_Param( UDisAISubStateTrackTarget::StaticClass() )
	, m_TrackTargetProxy( _rTrackTargetProxy )
	, m_fMaxFunnelRadiusMultiplier( _fMaxFunnelRadiusMultiplier )
{
}

// DISHONORED(port): 2012 rva 0x769650
void FDisAISubStateTrackTarget_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateTrackTarget* TrackTarget = (UDisAISubStateTrackTarget*)PendingState;
	TrackTarget->m_TrackTargetProxy = m_TrackTargetProxy;
	TrackTarget->m_fMaxFunnelRadiusMultiplier = m_fMaxFunnelRadiusMultiplier;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateTrackTarget
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765a30
const BYTE* UDisAISubStateTrackTarget::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_PathingFail, EAIStimID_SearchRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x78cc60
FDisStimPredicateDelegate UDisAISubStateTrackTarget::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateTrackTarget, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateTrackTarget, FAIStimStruct_PathingFail, FilterPathingFail );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x7888e0. Tracking a target is an engagement: the global AI manager is told, the behaviour
// fires its "I am tracking" dialog hook so the NPC barks, and the head starts the drawn-sword search sweep.
// DISHONORED(bringup): UDishonoredGlobalAIManager::Engage / Disengage and UDishonoredAIBehavior::FireDialogHook belong to
// the combat manager and the conversation system, neither ported (FireDialogHook is 5 of the natives agent CG's
// classification lists as blocked). The look-at sweep and the destination are this sub-state's own work and are faithful.
void UDisAISubStateTrackTarget::BeginSubState_Derived()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredGlobalAIManager::Engage and UDishonoredAIBehavior::FireDialogHook are not ported; a tracking NPC neither engages nor barks") );
	}
	m_bCloseEnough = FALSE;
	SetLookAtProceduralDesire( DisLookAtProceduralPattern_SearchSword, FDisLookAtInfluence::Torso, -1.f );
	UpdateCurrentSearchDest();
}

// DISHONORED(port): 2012 rva 0x765a10
void UDisAISubStateTrackTarget::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	// DISHONORED(bringup): UDishonoredGlobalAIManager::Disengage, unported (see BeginSubState_Derived).
}

/** DISHONORED(port): the destination half of 2012 rva 0x788a70's first branch: the proxy's own believed location. */
void UDisAISubStateTrackTarget::UpdateCurrentSearchDest()
{
	m_CurrentSearchDest = m_TrackTargetProxy.GetProxyFeetLocation();
	SetLocoLocationDesire( m_CurrentSearchDest, ETransitSpeed_Walk, -1.f, m_fMaxFunnelRadiusMultiplier, FALSE, FALSE );
}

// DISHONORED(port): 2012 rva 0x765a70: an inexact arrival is "close enough", and the loco desire is dropped so the NPC
// stops pressing forward while its behaviour decides what to do next.
UBOOL UDisAISubStateTrackTarget::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine && !_rStim.m_bExact )
	{
		m_bCloseEnough = TRUE;
		ClearLocoDesire();
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x788a70.
// DISHONORED(bringup): retail answers a pathing failure by sweeping eight directions for a reachable point with a trace
// and DisComputeNearestNavMeshLocFromLocation - the nav-mesh query agent CG's hand-over 3 names as one of the four the AI
// needs. Without it there is nowhere else to try, so the failure is recorded and the behaviour's RefreshCallback is what
// gives up.
UBOOL UDisAISubStateTrackTarget::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		m_bDebug_PathFailWandering = TRUE;
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): DisComputeNearestNavMeshLocFromLocation is not ported; a tracking NPC that cannot path does not wander") );
		}
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x788ba0-region: the destination follows the proxy as the NPC's belief about it moves.
void UDisAISubStateTrackTarget::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
	if( !m_bCloseEnough )
	{
		UpdateCurrentSearchDest();
	}
}
