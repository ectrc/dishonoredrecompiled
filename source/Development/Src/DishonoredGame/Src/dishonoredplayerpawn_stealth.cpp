// DishonoredGame/src/dishonoredplayerpawn_stealth.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (15):
//   0x6fb890  class ADisStealthVolume * & __cdecl DishonoredPlayerPawn_Stealth::GetNextStealthVolume(class ADisStealthVolume * const)
//   0x6fb8a0  public: virtual float __thiscall ADishonoredPlayerPawn::GetSpeedFraction(void)const
//   0x6fec60  public: virtual void __thiscall ADishonoredPlayerPawn::RefreshSearchProxy(void)
//   0x6fec70  public: unsigned int __thiscall ADishonoredPlayerPawn::IsSliding(void)const
//   0x6feca0  protected: unsigned int __thiscall ADishonoredPlayerPawn::AttemptChangeCollisionHeight(float)
//   0x707310  public: virtual void __thiscall ADishonoredPlayerPawn::OnEnterStealthVolume(class ADisStealthVolume *)
//   0x707380  public: virtual void __thiscall ADishonoredPlayerPawn::OnExitStealthVolume(class ADisStealthVolume *)
//   0x707400  public: void __thiscall ADishonoredPlayerPawn::ClearVisAndMantleSettings(void)
//   0x707470  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsSneaking(void)const
//   0x7074c0  protected: unsigned int __thiscall ADishonoredPlayerPawn::AttemptChangeToSneakHeight(void)
//   0x707510  protected: void __thiscall ADishonoredPlayerPawn::GameSave_Stealth(class FArchive &, enum ESaveLoadLocation)
//   0x70d440  public: virtual void __thiscall ADishonoredPlayerPawn::ExtractSearchLocation(class ADishonoredNPCPawn * const, class FVector &)
//   0x70d450  public: void __thiscall ADishonoredPlayerPawn::StartSneaking(unsigned int)
//   0x70d570  public: void __thiscall ADishonoredPlayerPawn::StopSneaking(unsigned int)
//   0x70d6a0  protected: void __thiscall ADishonoredPlayerPawn::GameLoad_Stealth(class FArchive &, enum ESaveLoadLocation)

/*-----------------------------------------------------------------------------
	The sneak toggle (crouch in the key bindings): agents FZ (PHASE18, StopSneaking's bits) and IF (PHASE23, the rest).

	2012 / 2013 rvas, the 2013 bodies ported where the two differ:
	  StartSneaking                 0x70d450 / 0x6d3860 (286 -> 225 B: 2013 calls RequestAction where 2012 inlined it)
	  StopSneaking                  0x70d570 / 0x6cd960 (295 -> 242 B, the same)
	  IsSneaking                    0x707470 / 0x6c8280 (65 -> 109 B: 2013 also answers TRUE for a mantle that requires sneaking)
	  AttemptChangeToSneakHeight    0x7074c0 / 0x6cda60
	  AttemptChangeCollisionHeight  0x6feca0 / 0x6c3430 (618 -> 626 B: 2013's head-room trace has extent Z 1.0, 2012's 0)
	  GetSpeedFraction              0x6fb8a0 / 0x6c33b0
	Sneaking is not the engine's crouch: it changes only the collision height (m_fSneakHeight of the pawn tweaks), raises
	m_bSneakModeToggled and asks the upper FSM for the go-to-crouch action; bIsCrouched stays as it was (retail sets it only
	through APawn::Crouch, which the swim state uses) and only StopSneaking clears it.
	DISHONORED(bringup): the go-to-crouch / leave-crouch actions go through ADishonoredPlayerPawn::RequestAction, which refuses
	while the upper FSM is not initialised (dishonoredplayerpawn_action.cpp): the height changes, the animation does not.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PlayerPawnStealth()
{
}

static UDisTweaks_PlayerPawn* DisIFPlayerTweaks( ADishonoredPlayerPawn* Pawn )
{
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( Pawn->GetTweaks_Derived() );
	return Tweaks ? Tweaks : UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
}

static UDisTweaks_PlayerPawn_NavigationActions* DisIFNavigationActions( ADishonoredPlayerPawn* Pawn )
{
	return ((UDisTweaks_PlayerPawn_Actions*)DisIFPlayerTweaks( Pawn )->m_pActionTweaks)->m_pNavigationActionTweaks;
}

// 2013 0x6c8280: the toggle, a choke begun while sneaking, or a mantle that requires sneaking
UBOOL ADishonoredPlayerPawn::IsSneaking() const
{
	UBOOL bChokeSneak = FALSE;
	UBOOL bMantleSneak = FALSE;
	UDishonoredNativeState* State = m_pPlayerMasterFSM ? m_pPlayerMasterFSM->GetCurrentlyActiveState() : NULL;
	UStatePlayerMasterChoke* Choke = Cast<UStatePlayerMasterChoke>( State );
	if( Choke )
	{
		bChokeSneak = Choke->m_bWasSneaking;
	}
	else
	{
		UStatePlayerMasterMantle* Mantle = Cast<UStatePlayerMasterMantle>( State );
		if( Mantle )
		{
			bMantleSneak = Mantle->m_bSneakRequired;
		}
	}
	return m_bSneakModeToggled || bChokeSneak || bMantleSneak;
}

// 2013 0x6cda60: the pawn tweaks' sneak height
UBOOL ADishonoredPlayerPawn::AttemptChangeToSneakHeight()
{
	return AttemptChangeCollisionHeight( DisIFPlayerTweaks( this )->m_fSneakHeight );
}

/**
 * 2013 0x6c3430. Shrinking always succeeds: the cylinder takes the new height and the StartCrouch event moves the pawn down by
 * the difference. Growing moves the pawn up by the difference, provided a line trace of the old radius from the location to the
 * new top is clear (skipped for bNoEncroachCheck) and FarMoveActor accepts the new location; otherwise the old size is restored.
 */
UBOOL ADishonoredPlayerPawn::AttemptChangeCollisionHeight( FLOAT NewHeight )
{
	const FLOAT OldHeight = CylinderComponent->CollisionHeight;
	const FLOAT OldRadius = CylinderComponent->CollisionRadius;
	CylinderComponent->SetCylinderSize( OldRadius, NewHeight );
	CylinderComponent->UpdateBounds();
	const FLOAT HeightAdjust = NewHeight - OldHeight;
	if( NewHeight <= OldHeight )
	{
		SetCollisionSize( OldRadius, NewHeight );
		bForceFloorCheck = TRUE;
		APawn::eventStartCrouch( -HeightAdjust );
		ForceUpdateComponents( FALSE, TRUE );
		IFDIAG( TEXT("height %.2f -> %.2f (down), pawn Z %.1f"), OldHeight, NewHeight, Location.Z );
		return TRUE;
	}
	const FVector NewLocation( Location.X, Location.Y, Location.Z + HeightAdjust );
	UBOOL bBlocked = FALSE;
	if( !bNoEncroachCheck )
	{
		FCheckResult Hit( 1.f );
		bBlocked = !GWorld->SingleLineCheck( Hit, this, FVector( Location.X, Location.Y, NewLocation.Z + NewHeight ), Location, 0x20D3,
			FVector( OldRadius, OldRadius, 1.f ) );
	}
	const UBOOL bWasJustTeleported = bJustTeleported;
	if( !bBlocked )
	{
		bBlocked = !GWorld->FarMoveActor( this, NewLocation, FALSE, FALSE, TRUE );
	}
	bJustTeleported = bWasJustTeleported;
	if( bBlocked )
	{
		CylinderComponent->SetCylinderSize( OldRadius, OldHeight );
		CylinderComponent->UpdateBounds();
		IFDIAG( TEXT("height %.2f -> %.2f (up) blocked, stays %.2f"), OldHeight, NewHeight, OldHeight );
		return FALSE;
	}
	APawn::eventEndCrouch( HeightAdjust );
	ForceUpdateComponents( FALSE, TRUE );
	IFDIAG( TEXT("height %.2f -> %.2f (up), pawn Z %.1f"), OldHeight, NewHeight, Location.Z );
	return TRUE;
}

/**
 * 2013 0x6d3860. Already crouched, or the sneak height fits: unless asked to skip the transition, carrying a corpse or holding
 * a movable, and while the master state allows a crouch action, the go-to-crouch action (the ready variant for a ready hand) is
 * requested of the upper body; the toggle is raised either way.
 */
void ADishonoredPlayerPawn::StartSneaking( UBOOL bSkipTransitions )
{
	if( !bIsCrouched && !AttemptChangeToSneakHeight() )
	{
		return;
	}
	if( !bSkipTransitions && !IsCarryingCorpse() && !IsHoldingMovableObject() && m_pPlayerMasterFSM->GetLogicalState()->AllowsCrouchAction() )
	{
		UDisTweaks_PlayerPawn_NavigationActions* Actions = DisIFNavigationActions( this );
		FStatePlayerAction_Param ActionParam( GetActualPlayerStance( EDisEquipUsage_Primary ) == eDisPlayerStance_NotReady
			? Actions->m_Action_GoToCrouch : Actions->m_Action_GoToCrouch_Ready );
		RequestAction( eDisPlayerActionUsage_Upperbody, ActionParam, NULL, FALSE );
	}
	m_bSneakModeToggled = TRUE;
}

/**
 * 2013 0x6cd960. Only while sneaking: back to the class default's collision height; when that fits the toggle and bIsCrouched are
 * cleared and, unless holding a movable, carrying a corpse or asked to skip it, the leave-crouch action is requested.
 */
void ADishonoredPlayerPawn::StopSneaking( UBOOL bSkipTransitions )
{
	if( !m_bSneakModeToggled )
	{
		return;
	}
	const FLOAT StandingHeight = GetClass()->GetDefaultObject<ADishonoredPlayerPawn>()->CylinderComponent->CollisionHeight;
	if( !AttemptChangeCollisionHeight( StandingHeight ) )
	{
		return;
	}
	m_bSneakModeToggled = FALSE;
	bIsCrouched = FALSE;
	if( !IsHoldingMovableObject() && !IsCarryingCorpse() && !bSkipTransitions )
	{
		UDisTweaks_PlayerPawn_NavigationActions* Actions = DisIFNavigationActions( this );
		FStatePlayerAction_Param ActionParam( GetActualPlayerStance( EDisEquipUsage_Primary ) == eDisPlayerStance_NotReady
			? Actions->m_Action_LeaveCrouch : Actions->m_Action_LeaveCrouch_Ready );
		RequestAction( eDisPlayerActionUsage_Upperbody, ActionParam, NULL, FALSE );
	}
}

// 2013 0x6c33b0: the 2D speed as a fraction of the sprint speed attribute
FLOAT ADishonoredPlayerPawn::GetSpeedFraction() const
{
	static const FName NAME_GroundSpeedSprint( TEXT("Attribute_GroundSpeedSprint") );
	const FLOAT SprintSpeed = GetAttributeValue( NAME_GroundSpeedSprint );
	return SprintSpeed > 0.f ? Velocity.Size2D() / SprintSpeed : 0.f;
}
