/*=============================================================================
	stateplayermasterbase.cpp: agent FZ (PHASE18 FZ) - UStatePlayerMasterBase, the base of the player master FSM's
	26 states. Retail 2012 rvas from the Shipping PDB (this unit's banner): OnEnterState 0x6a72c0 (2013 0x6729d0,
	matched by order 0.889), OnExitState 0x692d20 (2013 0x65fe00), GetBlendInTime 0x692d60, UpdateWalkNavAnimState
	0x69ab60 (2013 0x675450, 757 B), the const getters 0x692d30..0x692da0 (inline in the CppText).
=============================================================================*/
#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_UStatePlayerMasterBase()
{
}

/**
 * DISHONORED(port): 2012 0x6a72c0. Coming out of an action state the blend time is the action's remaining time
 * (UStatePlayerMasterAction::m_fTimeToGo, +72) and m_bBlendingFromLastState is raised; unless the state allows
 * incoming attacks every active projectile is cancelled. The 2013 body reads AreIncomingAttacksAllowed through the
 * vtable (slot 95) where 2012 read the bit; the value is the same bit.
 * DISHONORED(bringup): UDisGlobalProjectileManager::CancelActiveProjectiles (2012 0x8b1050) and
 * DisGetGlobalProjectileManager (0x82dc30) have no bodies in this tree; that call is the one thing left out.
 */
void UStatePlayerMasterBase::OnEnterState( UDishonoredNativeState* LastState )
{
	if( LastState )
	{
		UStatePlayerMasterAction* Action = Cast<UStatePlayerMasterAction>( LastState );
		if( Action )
		{
			m_bBlendingFromLastState = TRUE;
			m_fBlendTimeFromLastState = Action->m_fTimeToGo;
		}
	}
	if( !AreIncomingAttacksAllowed() )
	{
		static UBOOL bSaid = FALSE;
		if( !bSaid )
		{
			bSaid = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): UStatePlayerMasterBase::OnEnterState: UDisGlobalProjectileManager::CancelActiveProjectiles is not ported; active projectiles are not cancelled on the state change") );
		}
	}
}

// DISHONORED(port): 2012 0x692d20 (7 B)
void UStatePlayerMasterBase::OnExitState( UDishonoredNativeState* NextState )
{
	m_bBlendingFromLastState = FALSE;
}

// DISHONORED(port): 2012 0x692d60
FLOAT UStatePlayerMasterBase::GetBlendInTime() const
{
	return m_bBlendingFromLastState ? m_fBlendTimeFromLastState : m_fBlendInTime;
}

/**
 * DISHONORED(bringup): 2012 0x69ab60, 749 B, read and NOT ported. Retail picks the player's full-body navigation anim
 * state (Nav_Default / _Crouched / _Sprinting / _Ready / Zoom) from the desired and actual stances through
 * ADishonoredPlayerPawn::SetAnimState / RequestAction / GetActualPlayerStance / GetDesiredPlayerStance / IsSneaking,
 * sets the arm-follow influence per hand and the crouch FOV target (UStatePlayerMasterWalk::m_fCrouchedFOV through
 * ADishonoredPlayerCamera::SetFOVTarget( eDisCamFOVPriority_Locomotion )). None of the stance and anim-state functions
 * exist in this tree (dishonoredplayerpawn_combat.cpp and displayerpawn_animstates.cpp are skeletons), so the walk
 * state enters without them, exactly as the pawn stood before this package. It clears m_bBlendingFromLastState as
 * retail does at its end, so the blend-in time is consumed once.
 */
void UStatePlayerMasterBase::UpdateWalkNavAnimState( ADishonoredPlayerPawn* Pawn, UBOOL bHandleCrouch )
{
	static UBOOL bSaid = FALSE;
	if( !bSaid )
	{
		bSaid = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UStatePlayerMasterBase::UpdateWalkNavAnimState (2012 0x69ab60) is not ported: the player's navigation anim state, stance arm-follow and crouch FOV are not driven by the master FSM") );
	}
	m_bBlendingFromLastState = FALSE;
}
