// DishonoredGame/src/disaisubstatemaintaindistance.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateMaintainDistance and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateMaintainDistance_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781e80
FDisAISubStateMaintainDistance_Param::FDisAISubStateMaintainDistance_Param( FLOAT _fIdealDistance, const FDisAttentionProxy& _rFocus )
	: FDisAISubState_Param( UDisAISubStateMaintainDistance::StaticClass() )
	, m_Focus( _rFocus )
	, m_fIdealDistance( _fIdealDistance )
{
}

// DISHONORED(port): 2012 rva 0x7693b0
void FDisAISubStateMaintainDistance_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateMaintainDistance* MaintainDistance = (UDisAISubStateMaintainDistance*)PendingState;
	MaintainDistance->m_fIdealDistance = m_fIdealDistance;
	MaintainDistance->m_Focus = m_Focus;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateMaintainDistance

	Two stages (m_eCurrentStage): 0 = the focus is a confirmed target, so hold the ring around it; 1 = it is not, so walk
	to where it was believed to be and wait for it to be confirmed again. That is the difference between keeping your
	distance from someone you can see and closing on a noise.
-----------------------------------------------------------------------------*/

enum { DIS_MAINTAINDIST_HOLDING = 0, DIS_MAINTAINDIST_APPROACHING = 1 };

/**
 * DISHONORED(bringup): retail asks UDisBehaviorCombatMelee::ComputeNewMeleePosition (2012 rva 0x771290) where to stand
 * and where to look, which folds in the attack pattern, the formation slot and the other NPCs already engaged. That
 * behaviour is not ported, so the fallback used here is the one it degenerates to with no formation: stand where the
 * proxy is believed to be and look at it. Every caller in this package goes through this one function, so the whole
 * melee family becomes exact the moment UDisBehaviorCombatMelee lands.
 */
FVector DisComputeMeleePosition( const FDisAttentionProxy& _rProxy, FVector& _rOutLookPosition )
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisBehaviorCombatMelee::ComputeNewMeleePosition is not ported; melee sub-states walk straight at their target with no formation") );
	}
	_rOutLookPosition = _rProxy.GetBestTargetLocation();
	return _rProxy.GetProxyFeetLocation();
}

// DISHONORED(port): 2012 rva 0x769400
void UDisAISubStateMaintainDistance::SetupDesires()
{
	FVector LookPosition;
	const FVector GoToPosition = DisComputeMeleePosition( m_Focus, LookPosition );
	SetLocoLocationDesire( GoToPosition, ETransitSpeed_Walk, -1.f, 1.f, FALSE, FALSE );
	if( m_Focus.IsVerifiedAsTarget() )
	{
		SetLookAtLocationDesire( LookPosition, FDisLookAtInfluence::TorsoSpeedIndependent, -1.f );
	}
}

// DISHONORED(port): 2012 rva 0x771100
void UDisAISubStateMaintainDistance::MaintainDistance()
{
	SetupDesires();
	// DISHONORED(bringup): UDishonoredAIBrain::SetCombatRange is part of the combat manager plumbing, unported.
	m_eCurrentStage = DIS_MAINTAINDIST_HOLDING;
}

// DISHONORED(port): 2012 rva 0x7740e0.
// DISHONORED(bringup): UDishonoredAIBrain::CombatEngage_Brain and SetCombatRange belong to the combat manager
// (UDisGlobalCombatManager, unported), so the NPC holds its ring without claiming a combat slot.
void UDisAISubStateMaintainDistance::BeginSubState_Derived()
{
	SetupDesires();
	m_eCurrentStage = DIS_MAINTAINDIST_HOLDING;
}

// DISHONORED(port): 2012 rva 0x774120. While the focus is confirmed the loco desire is toggled on and off around the ideal
// distance - that on/off is the whole "maintain distance" behaviour. When it stops being confirmed the sub-state switches
// to approaching and waits for it to be confirmed again.
void UDisAISubStateMaintainDistance::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( m_eCurrentStage == DIS_MAINTAINDIST_APPROACHING )
	{
		if( m_Focus.IsValid() && m_Focus.IsVerifiedAsTarget() )
		{
			MaintainDistance();
		}
		return;
	}

	if( !m_Focus.IsValid() )
	{
		return;
	}
	if( !m_Focus.IsVerifiedAsTarget() )
	{
		SetupDesires();
		m_eCurrentStage = DIS_MAINTAINDIST_APPROACHING;
		return;
	}

	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}
	const FVector ToFocus = Pawn->Location - m_Focus.GetProxyLocation();
	FDisLocoRequest* LocoRequest = GetDesiresLocoRequest();
	const UBOOL bHasLocoDesire = LocoRequest && LocoRequest->m_bDesired;
	if( ( m_fIdealDistance * m_fIdealDistance ) <= ToFocus.SizeSquared() )
	{
		if( !bHasLocoDesire )
		{
			SetLocoProxyDesire( m_Focus, ETransitSpeed_Run, -1.f, 1.f, FALSE, FALSE );
		}
	}
	else if( bHasLocoDesire )
	{
		ClearLocoDesire();
	}
}

// DISHONORED(port): 2012 rva 0x77be20: leaving for another real sub-state hands the combat engagement on; leaving for the
// idle state does not, because the behaviour itself is stopping.
void UDisAISubStateMaintainDistance::OnExitState( UDishonoredNativeState* NextState )
{
	Super::OnExitState( NextState );
	if( m_Focus.IsValid() && NextState && !NextState->IsA( UDisAISubStateInit::StaticClass() ) )
	{
		// DISHONORED(bringup): UDishonoredAIBrain::CombatEngage_Brain, unported (see BeginSubState_Derived).
	}
}
