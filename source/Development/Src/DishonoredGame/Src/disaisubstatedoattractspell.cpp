// DishonoredGame/src/disaisubstatedoattractspell.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateDoAttractSpell and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateDoAttractSpell_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781a30
FDisAISubStateDoAttractSpell_Param::FDisAISubStateDoAttractSpell_Param( const FDisAttentionProxy& _rTargetProxy )
	: FDisAISubState_Param( UDisAISubStateDoAttractSpell::StaticClass() )
	, m_TargetProxy( _rTargetProxy )
{
}

// DISHONORED(port): folded with FDisAISubStateMenace_Param::OnPending - one proxy at sub-state offset 208.
void FDisAISubStateDoAttractSpell_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
	( (UDisAISubStateDoAttractSpell*)PendingState )->m_TargetProxy = m_TargetProxy;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateDoAttractSpell
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7652d0
const BYTE* UDisAISubStateDoAttractSpell::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_AttackedByEnemy, EAIStimID_EvadedMelee_Incoming, EAIStimID_TouchedEnemy };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x770ad0: all three ids share one filter, which is the shape retail's switch has.
FDisStimPredicateDelegate UDisAISubStateDoAttractSpell::GetFilterStimDelegate_SubState( BYTE StimID )
{
	if( StimID == EAIStimID_AttackedByEnemy || StimID == EAIStimID_EvadedMelee_Incoming || StimID == EAIStimID_TouchedEnemy )
	{
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateDoAttractSpell, FAIStimStruct, FilterAttackedByEnemy );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x765310: being hit while casting means give up after this cast rather than chain into the
// coup de grace.
UBOOL UDisAISubStateDoAttractSpell::FilterAttackedByEnemy( const FAIStimStruct& _rStim )
{
	m_bLastResort = TRUE;
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x780060. The cast is a direct use of the secondary item with an FDisItemContextParam_UseNPC
// naming the target; if it is accepted the NPC faces the target and its attack sub-process is suspended so it does not
// also lunge.
// DISHONORED(bringup): ADishonoredPawn::DirectUseEquippedItem and UDisItemContext_NPCAttractSpell are agent AJ's third
// dependency root (UDisItemContext), unported, so the cast is always refused - which is exactly the branch retail takes
// when the NPC has no spell, and TickState then leaves the state at once.
void UDisAISubStateDoAttractSpell::BeginSubState_Derived()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisItemContext_NPCAttractSpell is not ported; an assassin's pull spell is always refused") );
	}
	m_bAttractBegun = FALSE;
	m_bLastResort = FALSE;
}

// DISHONORED(port): 2012 rva 0x780130: a cast that is still running when the state ends is cancelled, and the attack
// sub-process is put back the way it was.
void UDisAISubStateDoAttractSpell::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	if( !bIsBeingTerminated && m_bAttractBegun )
	{
		// DISHONORED(bringup): UDishonoredInventoryItem::FindItemContext / UDisItemContext::CancelContext, unported.
	}
}

// DISHONORED(port): 2012 rva 0x7751b0: a cast that was never accepted leaves at once; one that was runs until the coup de
// grace is accepted or the NPC is interrupted.
void UDisAISubStateDoAttractSpell::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );
	if( !m_bAttractBegun )
	{
		RequestStateExit();
	}
}
