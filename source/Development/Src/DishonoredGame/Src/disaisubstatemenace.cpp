// DishonoredGame/src/disaisubstatemenace.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateMenace and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateMenace_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781f10
FDisAISubStateMenace_Param::FDisAISubStateMenace_Param( const FDisAttentionProxy& _rEnemyProxy )
	: FDisAISubState_Param( UDisAISubStateMenace::StaticClass() )
	, m_EnemyProxy( _rEnemyProxy )
{
}

// DISHONORED(port): folded with FDisAISubStateStareAtUnreachable_Param::OnPending (2012 rva 0x769490): both write one
// proxy at sub-state offset 208.
void FDisAISubStateMenace_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
	( (UDisAISubStateMenace*)PendingState )->m_EnemyProxy = m_EnemyProxy;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateMenace
-----------------------------------------------------------------------------*/

/** DISHONORED(port): the taunt interval retail rolls in two places (BeginSubState_Derived and RefreshSubState, 2012 rvas
    0x788240 and 0x788300), with the same expression both times. */
static FLOAT DisRollTauntTimer( const UDisTweaks_AISubState_Menace& _rTweaks )
{
	return _rTweaks.m_fTauntMinTime + appFrand() * ( _rTweaks.m_fTauntMaxTime - _rTweaks.m_fTauntMinTime );
}

// DISHONORED(port): 2012 rva 0x788240. Taunting is standing off, so the brain is told to disengage from combat, and the
// first taunt is either immediate or one interval away.
// DISHONORED(bringup): UDishonoredAIBrain::CombatDisengage_Brain (2012 rva 0x75c4d0) needs UDisBehaviorCombat, the three
// UDisSteeringInfluence_* classes and UDisGlobalCombatManager, none of them ported (agentCG.md: the steering influences
// are generated with their members and no bodies). The taunt clock and the facing are faithful.
void UDisAISubStateMenace::BeginSubState_Derived()
{
	const UDisTweaks_AISubState_Menace* Tweaks = Cast<UDisTweaks_AISubState_Menace>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Menace*)UDisTweaks_AISubState_Menace::StaticClass()->GetDefaultObject();
	}
	m_fTauntTimer = Tweaks->m_bImmediateFirstTaunt ? 0.f : DisRollTauntTimer( *Tweaks );

	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredAIBrain::CombatDisengage_Brain is not ported; a menacing NPC does not release its combat slot") );
	}

	SetFaceToProxyDesire( m_EnemyProxy, -100.f, FALSE );
}

// DISHONORED(port): 2012 rva 0x788300. The enemy becomes this sub-state's action target - which is what the behaviour's
// callbacks and the attention system read back - and the taunt clock is run down on the thought clock rather than the
// frame clock, so a distant NPC taunts less often.
// DISHONORED(bringup): the taunt itself is a state change on the pawn's own FSM with an FStateNPCPlayAnim_Param (agent
// AV's anim states), so the clock rolls over and no animation plays.
void UDisAISubStateMenace::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	SetActionTargetProxy( m_EnemyProxy );

	const UDisTweaks_AISubState_Menace* Tweaks = Cast<UDisTweaks_AISubState_Menace>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Menace*)UDisTweaks_AISubState_Menace::StaticClass()->GetDefaultObject();
	}

	m_fTauntTimer -= TimeSinceLastThought;
	if( m_fTauntTimer >= 0.f )
	{
		return;
	}
	// DISHONORED(bringup): retail also refuses to taunt while the pawn is in a body action
	// (ADishonoredNPCPawn::IsNPCInBodyAction, 2012 rva 0x7b1b80, which needs UStateSharedActionBase - agent AV's package).
	m_fTauntTimer = DisRollTauntTimer( *Tweaks );
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FStateNPCPlayAnim_Param is not ported; a menacing NPC rolls its taunt clock but plays no taunt") );
	}
}

// DISHONORED(port): 2012 rva 0x788470: a menacing NPC draws its sword when it comes back to slot 0, but only if the slot's
// tweaks said to draw a weapon at all.
UBOOL UDisAISubStateMenace::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	const UDisTweaks_AISubState_Menace* Tweaks = Cast<UDisTweaks_AISubState_Menace>( const_cast<UDisAISubStateMenace*>( this )->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_Menace*)UDisTweaks_AISubState_Menace::StaticClass()->GetDefaultObject();
	}
	if( !Tweaks->m_bDrawWeapon )
	{
		return FALSE;
	}
	// DISHONORED(bringup): retail names UDisWepMelee as the primary item class; the weapon classes are generated with their
	// members and have no bodies, so the class is named here and the equip half (UDisItemContext, agent AJ's third root)
	// is what would act on it.
	_rResumingBodyIntention.m_IntendedBodyStance = eDisNPCBodyStance_Equipped;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = UDisWepMelee::StaticClass();
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	return TRUE;
}
