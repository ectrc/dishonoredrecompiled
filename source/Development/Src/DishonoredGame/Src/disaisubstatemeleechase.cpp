// DishonoredGame/src/disaisubstatemeleechase.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateMeleeChase and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateMeleeChase_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x782400: the base carries the enemy and the stance; this only names the class.
FDisAISubStateMeleeChase_Param::FDisAISubStateMeleeChase_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eOverriddenBodyStance )
	: FDisAISubStateCombatBase_Param( _rEnemyProxy, _eOverriddenBodyStance )
{
	m_pStateClass = UDisAISubStateMeleeChase::StaticClass();
}

/*-----------------------------------------------------------------------------
	UDisAISubStateMeleeChase
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x771300
void UDisAISubStateMeleeChase::BeginSubState_Derived()
{
	CombatEngageAttackPattern();
}

// DISHONORED(port): 2012 rva 0x783af0. The chase is one loco desire and one look-at, both re-stated every thought so the
// NPC keeps following a moving enemy; and if the brain has lost its combat engagement the sub-state says so with a
// CombatEngageRejected stim rather than carrying on.
void UDisAISubStateMeleeChase::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	FVector LookPosition;
	const FVector ChasePosition = DisComputeMeleePosition( m_EnemyProxy, LookPosition );
	SetLocoLocationDesire( ChasePosition, ETransitSpeed_Run, -1.f, 1.f, FALSE, FALSE );
	SetLookAtLocationDesire( LookPosition, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );

	// DISHONORED(bringup): UDishonoredAIBrain::IsCombatEngaged is part of the combat manager plumbing and is not ported, so
	// the rejection stim below is never raised. Retail raises FAIStimStruct_CombatEngageRejected here, which is how a
	// second attacker is told to fall back to MaintainDistance instead of crowding the first.
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDishonoredAIBrain::IsCombatEngaged is not ported; a chasing NPC never raises CombatEngageRejected") );
	}
}

// DISHONORED(port): 2012 rva 0x769730: a chasing NPC holds the stance its parameter asked for with its melee weapon drawn.
UBOOL UDisAISubStateMeleeChase::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	_rResumingBodyIntention.m_IntendedBodyStance = m_eCombatBodyStance;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = UDisWepMelee::StaticClass();
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	return TRUE;
}
