// DishonoredGame/src/disaisubstatemeleeengage.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateMeleeEngage and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateMeleeEngage_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x782450
FDisAISubStateMeleeEngage_Param::FDisAISubStateMeleeEngage_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eBodyStance )
	: FDisAISubStateCombatBase_Param( _rEnemyProxy, _eBodyStance )
{
	m_pStateClass = UDisAISubStateMeleeEngage::StaticClass();
}

/*-----------------------------------------------------------------------------
	UDisAISubStateMeleeEngage
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765b40. The formation position and the speed to reach it: an attack pattern that wants the
// NPC to walk rather than run says so with its own flag, which is how a circling attacker reads as deliberate.
// DISHONORED(bringup): UDishonoredAIBrain::GetCurrentAttackPattern belongs to the combat manager, unported, so the speed
// is always Run.
void UDisAISubStateMeleeEngage::SetFormationPosition()
{
	FVector LookPosition;
	const FVector EnemyLocation = DisComputeMeleePosition( m_EnemyProxy, LookPosition );
	SetLocoLocationDesire( EnemyLocation, ETransitSpeed_Run, -1.f, 1.f, FALSE, FALSE );
	SetLookAtLocationDesire( LookPosition, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );
}

// DISHONORED(port): 2012 rva 0x771310
void UDisAISubStateMeleeEngage::BeginSubState_Derived()
{
	CombatEngageAttackPattern();
	m_fRepositionTimer = -1.f;
	SetFormationPosition();
}

// DISHONORED(port): 2012 rva 0x788e50. Every thought: re-claim the attack pattern when the brain is receptive to a new one,
// and re-state the formation position.
// DISHONORED(bringup): the receptiveness test is UDisBehaviorCombat::ReceptiveToNewAttackPattern and the engagement test is
// UDishonoredAIBrain::IsCombatEngaged, both combat-manager plumbing and unported, so the pattern is claimed once and the
// CombatEngageRejected stim is never raised.
void UDisAISubStateMeleeEngage::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
	SetFormationPosition();
}
