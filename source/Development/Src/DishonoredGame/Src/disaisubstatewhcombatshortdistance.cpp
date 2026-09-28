// DishonoredGame/src/disaisubstatewhcombatshortdistance.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateWHCombatShortDistance and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateWHCombatShortDistance_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x782920. A wolfhound has one stance, so the parameter forces Equipped rather than taking it.
FDisAISubStateWHCombatShortDistance_Param::FDisAISubStateWHCombatShortDistance_Param( const FDisAttentionProxy& _rEnemyProxy )
	: FDisAISubStateCombatBase_Param( _rEnemyProxy, eDisNPCBodyStance_Equipped )
{
	m_pStateClass = UDisAISubStateWHCombatShortDistance::StaticClass();
}

/*-----------------------------------------------------------------------------
	UDisAISubStateWHCombatShortDistance
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x771670: the attack pattern is claimed with the "in formation" flag, which for a wolfhound
// means "I am the one biting".
void UDisAISubStateWHCombatShortDistance::BeginSubState_Derived()
{
	m_fRepositionTimer = -1.f;
	CombatEngageAttackPattern();
}

// DISHONORED(port): 2012 rva 0x789190: re-claim the pattern and hold the bite position.
// DISHONORED(bringup): the pattern and the engagement come from the combat manager (UDisGlobalCombatManager,
// UDisBehaviorCombat), unported, so the hound closes on its enemy with no formation and never reports a rejected
// engagement.
void UDisAISubStateWHCombatShortDistance::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	FVector LookPosition;
	const FVector BitePosition = DisComputeMeleePosition( m_EnemyProxy, LookPosition );
	SetLocoLocationDesire( BitePosition, ETransitSpeed_Run, -1.f, 1.f, FALSE, FALSE );
	SetLookAtLocationDesire( LookPosition, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );
}
