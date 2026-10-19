/*=============================================================================
	stateplayermasterbase.cpp: agent FZ (PHASE18 FZ) - UStatePlayerMasterBase, the base of the player master FSM's
	26 states. Retail 2012 rvas from the Shipping PDB (this unit's banner): OnEnterState 0x6a72c0 (2013 0x6729d0,
	matched by order 0.889), OnExitState 0x692d20 (2013 0x65fe00), GetBlendInTime 0x692d60, UpdateWalkNavAnimState
	0x69ab60 (2013 0x675450, 757 B), the const getters 0x692d30..0x692da0 (inline in the CppText).
=============================================================================*/
#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

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
 * DISHONORED(port): agent IA (PHASE23 IA), 2012 0x69ab60 (749 B) / 2013 0x675450 (757 B, the same flow). The blend time is the
 * state's blend-in time (GetBlendInTime) unless the pawn asked for an instant equip change (m_bHandleInstantEquipChange, cleared
 * here); the primary hand's desired stance is stored in m_DesiredPlayerStance[1]; the secondary hand's arm follow is disabled while
 * it is not ready; a not-ready primary picks Nav_Crouched (sneaking) or Nav_Sprinting / Nav_Default with its arm follow disabled,
 * a ready one Nav_Crouched_Ready / Nav_Sprinting_Ready / Nav_Default_Ready with its arm follow on; then the
 * crouch FOV: UStatePlayerMasterWalk's class default m_fCrouchedFOV at m_fCrouchFOV_BlendSpeed, arms locked, in the Locomotion
 * slot when the crouched anim state was chosen, else that slot is cleared.
 * agent IM (PHASE23 IM): the two halves agent IA named are retail's now - a desired stance the hand does not have is requested of
 * the upper body (the item actions' m_Actions_ExitReady for NotReady, m_Actions_GoToReady otherwise, through
 * FStatePlayerChangeReadyStance_Param), and the chosen full-body nav state is set (ADishonoredPlayerPawn::SetAnimState, not attached).
 */
void UStatePlayerMasterBase::UpdateWalkNavAnimState( ADishonoredPlayerPawn* Pawn, UBOOL bHandleCrouch )
{
	const FLOAT BlendTime = Pawn->m_bHandleInstantEquipChange ? 0.f : GetBlendInTime();
	UBOOL bDidCrouch = FALSE;
	Pawn->m_bHandleInstantEquipChange = FALSE;
	const BYTE DesiredStance = Pawn->GetDesiredPlayerStance( EDisEquipUsage_Primary );
	Pawn->m_DesiredPlayerStance[EDisEquipUsage_Primary] = DesiredStance;
	if( DesiredStance != Pawn->GetActualPlayerStance( EDisEquipUsage_Primary ) )
	{
		UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( Pawn->GetTweaks_Derived() );
		if( Tweaks == NULL )
		{
			Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
		}
		UDisTweaks_PlayerPawn_ItemActions* ItemActions = ((UDisTweaks_PlayerPawn_Actions*)Tweaks->m_pActionTweaks)->m_pItemActionTweaks;
		FStatePlayerChangeReadyStance_Param StanceParam( DesiredStance == eDisPlayerStance_NotReady ? ItemActions->m_Actions_ExitReady : ItemActions->m_Actions_GoToReady, DesiredStance );
		Pawn->RequestAction( eDisPlayerActionUsage_Upperbody, StanceParam, NULL, FALSE );
	}
	const BYTE ActualStance = Pawn->GetActualPlayerStance( EDisEquipUsage_Primary );
	Pawn->DisableArmFollow( EDisEquipUsage_Secondary, Pawn->GetActualPlayerStance( EDisEquipUsage_Secondary ) == eDisPlayerStance_NotReady, BlendTime, FALSE );
	const UBOOL bCrouch = Pawn->IsSneaking() && bHandleCrouch;
	const TCHAR* AnimState = NULL;
	if( ActualStance == eDisPlayerStance_NotReady )
	{
		AnimState = bCrouch ? TEXT("Nav_Crouched") : ( Pawn->m_bSprinting ? TEXT("Nav_Sprinting") : TEXT("Nav_Default") );
		bDidCrouch = bCrouch;
		Pawn->DisableArmFollow( EDisEquipUsage_Primary, TRUE, BlendTime, FALSE );
	}
	else
	{
		// retail asks the secondary item through a direct call that the linker folded onto a `return FALSE` body (2013 0x722980,
		// `xor eax, eax / retn`; 2012 0xd9220 the same), so the zoom variants are never chosen in either build
		const UBOOL bZoomItem = FALSE;
		if( bCrouch )
		{
			AnimState = bZoomItem ? TEXT("Nav_Zoom_Default") : TEXT("Nav_Crouched_Ready");
			bDidCrouch = TRUE;
		}
		else if( Pawn->m_bSprinting )
		{
			AnimState = TEXT("Nav_Sprinting_Ready");
		}
		else
		{
			AnimState = bZoomItem ? TEXT("Nav_Zoom_Default") : TEXT("Nav_Default_Ready");
		}
		Pawn->DisableArmFollow( EDisEquipUsage_Primary, FALSE, BlendTime, FALSE );
	}
	Pawn->SetAnimState( FName( AnimState ), ANIMSTATE_FULL_BODY, NULL, BlendTime, NULL );
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	const UStatePlayerMasterWalk* WalkDefaults = UStatePlayerMasterWalk::StaticClass()->GetDefaultObject<UStatePlayerMasterWalk>();
	if( Camera )
	{
		if( bDidCrouch && WalkDefaults->m_fCrouchedFOV > 0.0001f )
		{
			Camera->SetFOVTarget( eDisCamFOVPriority_Locomotion, WalkDefaults->m_fCrouchedFOV, WalkDefaults->m_fCrouchFOV_BlendSpeed, TRUE );
		}
		else
		{
			Camera->ClearFOVTarget( eDisCamFOVPriority_Locomotion );
		}
	}
	m_bBlendingFromLastState = FALSE;
}
