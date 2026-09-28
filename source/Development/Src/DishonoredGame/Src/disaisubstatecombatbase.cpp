// DishonoredGame/src/disaisubstatecombatbase.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateCombatBase and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateCombatBase_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7744e0. Abstract: it names no state class of its own, so only its three subclasses
// (MeleeChase, MeleeEngage, the two wolfhound distances) are ever constructed.
FDisAISubStateCombatBase_Param::FDisAISubStateCombatBase_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eBodyStance )
	: FDisAISubState_Param( NULL )
	, m_EnemyProxy( _rEnemyProxy )
	, m_eOverriddenBodyStance( _eBodyStance )
{
}

// DISHONORED(port): 2012 rva 0x7696d0
void FDisAISubStateCombatBase_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisAISubStateCombatBase* Combat = (UDisAISubStateCombatBase*)PendingState;
	Combat->m_EnemyProxy = m_EnemyProxy;
	Combat->m_eCombatBodyStance = m_eOverriddenBodyStance;
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
}

/*-----------------------------------------------------------------------------
	UDisAISubStateCombatBase
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x766e00.
// DISHONORED(bringup): retail asks UDisAISubProcessManageAttacks (through the brain's behaviour) which attack pattern this
// NPC has been allotted against this enemy, which is how a group takes turns. That sub-process is one of the four small
// subsystems agent CG costed and is not ported, so every NPC gets pattern 0 - they would all lunge together.
BYTE UDisAISubStateCombatBase::CombatEngageAttackPattern() const
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisAISubProcessManageAttacks is not ported; every melee NPC uses attack pattern 0") );
	}
	return 0;
}
