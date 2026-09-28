// DishonoredGame/src/disaisubstateinvestigate.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateInvestigate and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateInvestigate_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781d60, SizeOf 0x781e10 = 68 bytes - the largest of the 25 parameters, because it carries a
// whole FDisAttentionChangeReason: the investigation has to know WHY it started so the behaviour can bark the right line.
FDisAISubStateInvestigate_Param::FDisAISubStateInvestigate_Param( const FDisAttentionProxy& _rAttentionProxy, UBOOL _bRunningInvestigate, FDisAttentionChangeReason _InvestigateStartReason, BYTE _InvestigateHookOutput, FLOAT _fReactionDelayTime, FLOAT _fMaxFunnelRadiusMultiplier )
	: FDisAISubState_Param( UDisAISubStateInvestigate::StaticClass() )
	, m_InvestigateTargetProxy( _rAttentionProxy )
	, m_InvestigateStartReason( _InvestigateStartReason )
	, m_InvestigateHookOutput( _InvestigateHookOutput )
	, m_bRunningInvestigate( _bRunningInvestigate )
	, m_fReactionDelayTime( _fReactionDelayTime )
	, m_fMaxFunnelRadiusMultiplier( _fMaxFunnelRadiusMultiplier )
{
}

// DISHONORED(port): 2012 rva 0x768e70
void FDisAISubStateInvestigate_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateInvestigate* Investigate = (UDisAISubStateInvestigate*)PendingState;
	Investigate->m_InvestigateTargetProxy = m_InvestigateTargetProxy;
	Investigate->m_bRunningInvestigate = m_bRunningInvestigate ? 1 : 0;
	Investigate->m_InvestigateStartReason = m_InvestigateStartReason;
	Investigate->m_InvestigateHookOutput = m_InvestigateHookOutput;
	Investigate->m_fReactionDelayTimer = m_fReactionDelayTime;
	Investigate->m_fMaxFunnelRadiusMultiplier = m_fMaxFunnelRadiusMultiplier;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateInvestigate
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765530. Four stims; four bits in the 2012 mask body.
const BYTE* UDisAISubStateInvestigate::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_PathingFail, EAIStimID_RotationReached, EAIStimID_SearchReachedProxy };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

FDisStimPredicateDelegate UDisAISubStateInvestigate::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateInvestigate, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateInvestigate, FAIStimStruct_PathingFail, FilterPathingFail );
	case EAIStimID_RotationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateInvestigate, FAIStimStruct_RotationReached, FilterRotationReached );
	case EAIStimID_SearchReachedProxy:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateInvestigate, FAIStimStruct, FilterSearchReachedProxy );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x768f10. The one function that does the work: the destination follows the NPC's BELIEF about
// where the thing is (the proxy's location, not the target's), and it is only re-stated when that belief has actually moved
// - which is why a guard walking to a noise does not re-path every thought.
void UDisAISubStateInvestigate::RefreshInvestigation()
{
	const FVector ProxyLocation = m_InvestigateTargetProxy.GetProxyLocation();
	if( ProxyLocation.Equals( m_CachedProxyLoc, KINDA_SMALL_NUMBER ) )
	{
		return;
	}
	m_CachedProxyLoc = ProxyLocation;
	m_InvestigateMoveDest = ProxyLocation;
	SetLocoLocationDesire( m_InvestigateMoveDest, m_SearchTransitSpeed, -1.f, m_fMaxFunnelRadiusMultiplier, FALSE, FALSE );
	SetLookAtLocationDesire( m_InvestigateMoveDest, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );
	ClearFaceToDesire();
	m_bReachedProxy = FALSE;
	m_bFacingProxy = FALSE;
	m_fFocusTimer = -1.f;
}

// DISHONORED(port): 2012 rva 0x770e60. A running investigation walks; a cautious one strolls - that is the whole meaning of
// m_bRunningInvestigate. The Kismet event fires so a level can react to the NPC noticing something.
// DISHONORED(bringup): UDishonoredAIBrain::SetMinAttentionForTarget is part of UDisAIBrainProcessAttention (agentCG.md's
// highest-leverage remaining piece), so the investigation does not raise the NPC's own attention on its target.
void UDisAISubStateInvestigate::ResumeSubState_Derived()
{
	m_SearchTransitSpeed = m_bRunningInvestigate ? ETransitSpeed_Run : ETransitSpeed_Walk;

	IDisAttentionTargetInterface* Target = m_InvestigateTargetProxy.GetProxyAttnTarget();
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( Target && Pawn )
	{
		AActor* TargetActor = Target->GetAttnTargetActor();
		if( Cast<ADishonoredPawn>( TargetActor ) )
		{
			DisFireKismetEvent( Pawn, UDisSeqEvent_Investigate::StaticClass(), TargetActor, Pawn, 0, FALSE );
		}
	}

	m_bInvestigationFinished = FALSE;
	m_bReachedProxy = FALSE;
	m_bFacingProxy = FALSE;
	m_bWantsStareAtUnreachable = FALSE;
	m_PathingFailCount = 0;
	m_CachedProxyLoc = FVector( 0.f, 0.f, 0.f );
	RefreshInvestigation();

	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredAIBrain::SetMinAttentionForTarget needs UDisAIBrainProcessAttention; an investigating NPC does not raise its own attention") );
	}
}

// DISHONORED(port): 2012 rva 0x770f20: being pushed off slot 0 mid-investigation still fires the bark, so the NPC does not
// go silent because something more urgent came up.
void UDisAISubStateInvestigate::PauseSubState_Derived( UBOOL bIsBeingTerminated )
{
	if( !bIsBeingTerminated && m_InvestigateTargetProxy.IsValid() )
	{
		// DISHONORED(bringup): CheckDoBark goes through UDishonoredAIBehavior::FireDialogHook (the conversation system).
	}
}

// DISHONORED(port): 2012 rva 0x787f20: the distance that counts as "close enough to have looked" depends on WHY the
// investigation started - a corpse has to be stood over, a noise only walked towards.
FLOAT UDisAISubStateInvestigate::GetProximityThreshold() const
{
	const UDisTweaks_AISubState_Investigate* Tweaks = Cast<UDisTweaks_AISubState_Investigate>( const_cast<UDisAISubStateInvestigate*>( this )->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Investigate*)UDisTweaks_AISubState_Investigate::StaticClass()->GetDefaultObject();
	}
	const FDisAttentionChangeReason ChangeReason = m_InvestigateTargetProxy.GetProxyChangeReason();
	if( ChangeReason.m_ReasonType < ARRAY_COUNT(Tweaks->m_fProximityThresholds) )
	{
		return Tweaks->m_fProximityThresholds[ ChangeReason.m_ReasonType ];
	}
	return 0.f;
}

// DISHONORED(port): 2012 rvas 0x7655d0 / 0x7655f0 / 0x7655e0 / 0x765600 / 0x765b10: the five readers the behaviour's
// callbacks ask. Agent CG's classification lists ReachedProxy, WantsStareAtUnreachable and GetCorpseBeingInvestigated as
// blockers of UDisBehaviorSearch's own natives.
UBOOL UDisAISubStateInvestigate::ReachedProxy() const
{
	return m_bReachedProxy != 0;
}

UBOOL UDisAISubStateInvestigate::WantsStareAtUnreachable() const
{
	return m_bWantsStareAtUnreachable != 0;
}

UBOOL UDisAISubStateInvestigate::IsRunningInvestigate() const
{
	return m_bRunningInvestigate != 0;
}

// DISHONORED(bringup): 2012 rva 0x765600 reads the corpse out of the change reason through DisAttention::GetCorpseFromReason,
// which belongs to the attention package (UDisAIBrainProcessAttention). The reason itself is carried faithfully, so this
// becomes exact the moment that helper lands.
AActor* UDisAISubStateInvestigate::GetCorpseBeingInvestigated() const
{
	return NULL;
}

FDisAttentionChangeReason UDisAISubStateInvestigate::GetCurrentInvestigateReason() const
{
	return m_InvestigateStartReason;
}

// DISHONORED(port): 2012 rva 0x765570
UBOOL UDisAISubStateInvestigate::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		m_bReachedProxy = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x765590: having turned to face it, the facing desire is dropped so the head can go back to
// looking around.
UBOOL UDisAISubStateInvestigate::FilterRotationReached( const FAIStimStruct_RotationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		ClearFaceToDesire();
		m_bFacingProxy = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x7690a0. Two attempts at a nearby reachable point, then give up and ask to stare instead -
// which is what makes a guard stand and glare at you on a rooftop rather than walking into the wall below it.
// DISHONORED(bringup): DisComputeNearestNavMeshLocFromLocation is one of the four nav-mesh queries agent CG's hand-over 3
// names, so the retry cannot be made and the sub-state goes straight to the stare.
UBOOL UDisAISubStateInvestigate::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine && !m_bInvestigationFinished )
	{
		if( ++m_PathingFailCount < 2 )
		{
			static UBOOL bNoted = FALSE;
			if( !bNoted )
			{
				bNoted = TRUE;
				debugf( NAME_Warning, TEXT("DISHONORED(bringup): DisComputeNearestNavMeshLocFromLocation is not ported; an investigating NPC cannot retry a failed path") );
			}
		}
		m_bWantsStareAtUnreachable = TRUE;
		m_bInvestigationFinished = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x770fc0: another NPC has already reached this proxy, so this one stops investigating it -
// which is how a group does not all crowd the same noise.
UBOOL UDisAISubStateInvestigate::FilterSearchReachedProxy( const FAIStimStruct& _rStim )
{
	if( m_bInvestigationFinished )
	{
		return TRUE;
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x788ee0-region: the reaction delay first (an NPC does not turn the instant it notices), then
// the destination follows the belief, then arriving and facing finish the investigation.
void UDisAISubStateInvestigate::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	if( m_fReactionDelayTimer > 0.f )
	{
		m_fReactionDelayTimer -= TimeSinceLastThought;
		return;
	}
	if( !m_bInvestigationFinished )
	{
		RefreshInvestigation();
	}
}

// DISHONORED(port): once the NPC is standing on what it came to look at, it turns to face it; once it has, the
// investigation is finished and the behaviour's RefreshCallback decides what to do about it.
void UDisAISubStateInvestigate::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( m_bInvestigationFinished || !m_bReachedProxy )
	{
		return;
	}
	if( !m_bFacingProxy )
	{
		SetFaceToProxyDesire( m_InvestigateTargetProxy, -100.f, FALSE );
		return;
	}
	m_bInvestigationFinished = TRUE;
}
