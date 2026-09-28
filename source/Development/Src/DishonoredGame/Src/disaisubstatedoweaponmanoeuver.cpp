// DishonoredGame/src/disaisubstatedoweaponmanoeuver.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateDoWeaponManoeuver and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateDoWeaponManoeuver_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781ab0. The cooldown flag is deliberately NOT set by the constructor: every retail call
// site writes it after construction, which is how the same manoeuvre can be requested with and without the cooldown test.
FDisAISubStateDoWeaponManoeuver_Param::FDisAISubStateDoWeaponManoeuver_Param()
	: FDisAISubState_Param( UDisAISubStateDoWeaponManoeuver::StaticClass() )
	, m_bCheckCooldownOnPrecondition( FALSE )
{
}

// DISHONORED(port): 2012 rva 0x7686e0
void FDisAISubStateDoWeaponManoeuver_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
	( (UDisAISubStateDoWeaponManoeuver*)PendingState )->m_bPrecondition_CheckCooldown = m_bCheckCooldownOnPrecondition ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateDoWeaponManoeuver

	The whole sub-state is a wrapper around one item context: start it, hold the stance while it runs, leave when it raises
	ItemContext_End. The precondition is what stops a behaviour asking for a manoeuvre whose cooldown has not elapsed.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765320: one stim, the end of its own item context.
const BYTE* UDisAISubStateDoWeaponManoeuver::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_ItemContext_End };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

FDisStimPredicateDelegate UDisAISubStateDoWeaponManoeuver::GetFilterStimDelegate_SubState( BYTE StimID )
{
	if( StimID == EAIStimID_ItemContext_End )
	{
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateDoWeaponManoeuver, FAIStimStruct, FilterItemContext_End );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(bringup): the whole body of this sub-state is UDisItemContext - agent AJ's third dependency root. Retail asks
// the pawn's equipped weapon for the manoeuvre context, starts it, keeps m_pRunningContext, and waits for the
// ItemContext_End stim. None of that exists, so the manoeuvre is refused and the state leaves on its next tick; the
// precondition therefore also answers FALSE, which is retail's own answer for an NPC whose weapon has no manoeuvre.
UBOOL UDisAISubStateDoWeaponManoeuver::ArePreconditionsMet_Derived()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisItemContext is not ported; a weapon manoeuvre's preconditions are never met") );
	}
	return FALSE;
}

void UDisAISubStateDoWeaponManoeuver::BeginSubState_Derived()
{
	m_pRunningContext = NULL;
}

void UDisAISubStateDoWeaponManoeuver::EndSubState_Derived( UBOOL bIsBeingTerminated )
{
	m_pRunningContext = NULL;
}

void UDisAISubStateDoWeaponManoeuver::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );
	if( !m_pRunningContext )
	{
		RequestStateExit();
	}
}

void UDisAISubStateDoWeaponManoeuver::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
}

// DISHONORED(port): 2012 rva 0x765350-region: the manoeuvre's own context has ended, so the state is done.
UBOOL UDisAISubStateDoWeaponManoeuver::FilterItemContext_End( const FAIStimStruct& _rStim )
{
	m_pRunningContext = NULL;
	RequestStateExit();
	return FALSE;
}

// DISHONORED(port): a manoeuvring NPC keeps its weapon up while the context runs.
UBOOL UDisAISubStateDoWeaponManoeuver::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	_rResumingBodyIntention.m_IntendedBodyStance = eDisNPCBodyStance_Equipped;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = UDisWepMelee::StaticClass();
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	return TRUE;
}

void UDisAISubStateDoWeaponManoeuver::PostGameLoad_DoWeaponManoeuver()
{
	PostGameLoad_SubState();
	m_pRunningContext = NULL;
}
