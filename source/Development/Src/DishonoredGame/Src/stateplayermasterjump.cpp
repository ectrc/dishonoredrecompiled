// DishonoredGame/src/stateplayermasterjump.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (16):
//   0x693020  public: virtual void __thiscall UStatePlayerMasterJump::OnExitState(class UDishonoredNativeState *)
//   0x693090  protected: void __thiscall UStatePlayerMasterJump::DoNormalJump_Fixed(void)
//   0x6930e0  protected: unsigned int __thiscall UStatePlayerMasterJump::HasPowerJump(void)const
//   0x693150  public: void __thiscall UStatePlayerMasterJump::DisablePowerJump(void)
//   0x69b980  public: static class UClass * __cdecl UStatePlayerMasterJump::GetPrivateStaticClassUStatePlayerMasterJump(wchar_t const *)
//   0x69ba10  protected: void __thiscall UStatePlayerMasterJump::PlayJumpInActions(void)
//   0x6a0570  public: static void __cdecl UStatePlayerMasterJump::InitializePrivateStaticClassUStatePlayerMasterJump(void)
//   0x6a1f20  public: static class UClass * __cdecl UStatePlayerMasterJump::StaticClassNoInline(void)
//   0x6a4190  public: virtual unsigned int __thiscall UStatePlayerMasterJump::CanTransitionExternal(class UClass const *)const
//   0x6a6460  protected: void __thiscall UStatePlayerMasterJump::DoPowerJump_Fixed(void)
//   0x6a64e0  protected: void __thiscall UStatePlayerMasterJump::TickJumpLogic_FixedPower(float)
//   0x6a65d0  protected: void __thiscall UStatePlayerMasterJump::TickJumpLogic_HeldPower_FullStop(float)
//   0x6a66b0  protected: void __thiscall UStatePlayerMasterJump::TickJumpLogic_ContinuousPower(float)
//   0x6a6800  protected: void __thiscall UStatePlayerMasterJump::TickJumpLogic_NoPower(float)
//   0x6a79a0  public: virtual void __thiscall UStatePlayerMasterJump::OnEnterState(class UDishonoredNativeState *)
//   0x6a7bc0  public: virtual void __thiscall UStatePlayerMasterJump::TickState(float)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): UStatePlayerMasterJump, the master FSM's jump. ADishonoredPlayerPawn::DoJump requests it; it gives the
	pawn its jump speed (the JumpZ attribute; a power jump in the styles that have one), switches it to PHYS_Falling - its own
	CanTransitionExternal refuses the Falling request that physics change raises - and demands Falling once the pawn stops
	rising. The 2013 bodies are ported (2013 rvas in Inc/CppText/UStatePlayerMasterJump.h; 2013 reads AreIncomingAttacksAllowed
	through the vtable and inlines EnableControllerInput where 2012 called them).
	DISHONORED(bringup), named and not called inside: the active projectiles' cancel (UDisGlobalProjectileManager, as in
	UStatePlayerMasterBase::OnEnterState), the camera bump smoother's weight (ADishonoredPlayerCamera::GetCamera_BumpSmoother,
	UDishonoredCameraInfluence::SetWeight), the climbable's collision-restore timer (ADisClimbable::StartCollisionRestoreTimer).
	(UpdateWalkNavAnimState is agent IA's port.) The jump-in actions go through RequestAction, refused while the upper FSM is
	not initialised (dishonoredplayerpawn_action.cpp).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_UStatePlayerMasterJump()
{
}

// 2012 0x693cc0 / 2013 0x66dc80 is FStatePlayerMasterSwim_Param's constructor, which /OPT:ICF shares with the jump and falling ones
FStatePlayerMasterJump_Param::FStatePlayerMasterJump_Param()
	: FStatePlayerMasterBase_Param( UStatePlayerMasterJump::StaticClass() )
{
}

static FLOAT DisIFJumpZ( ADishonoredPlayerPawn* Pawn )
{
	static const FName NAME_JumpZ( TEXT("Attribute_JumpZ") );
	static const FName NAME_JumpZCarryingCorpse( TEXT("Attribute_JumpZCarryingCorpse") );
	return Pawn->GetAttributeValue( Pawn->IsCarryingCorpse() ? NAME_JumpZCarryingCorpse : NAME_JumpZ );
}

/**
 * 2013 0x675ee0. The action blend, the projectiles, every master input but sneak/zoom (0x8200) enabled, the jump speed by style
 * (fixed and continuous: the JumpZ attribute; held: the full-stop power jump when available; none: the normal jump), the upward
 * speed of a rising base added, PHYS_Falling, the start height kept, and the jump-in actions.
 */
void UStatePlayerMasterJump::OnEnterState( UDishonoredNativeState* LastState )
{
	static const FName NAME_PowerJumpFullStop( TEXT("Attribute_PowerJumpFullStop_PROTOTYPE") );
	UStatePlayerMasterAction* Action = LastState ? Cast<UStatePlayerMasterAction>( LastState ) : NULL;
	if( Action )
	{
		m_bBlendingFromLastState = TRUE;
		m_fBlendTimeFromLastState = Action->m_fTimeToGo;
	}
	if( !AreIncomingAttacksAllowed() )
	{
		IFDIAG_ONCE( TEXT("UStatePlayerMasterJump::OnEnterState: UDisGlobalProjectileManager::CancelActiveProjectiles is not ported") );
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->EnableControllerInput( 0x8200, FALSE, DisInputMaskLevel_MasterAction );
	m_fHeldButtonTime = 0.f;
	m_JumpState = 0;
	if( m_JumpStyle == 0 || m_JumpStyle == 2 )
	{
		m_pPlayerPawn->Velocity.Z = DisIFJumpZ( m_pPlayerPawn );
	}
	else if( m_JumpStyle == 1 && HasPowerJump() )
	{
		m_pPlayerPawn->Velocity.Z = m_pPlayerPawn->GetAttributeValue( NAME_PowerJumpFullStop );
		m_JumpState = 1;
	}
	else
	{
		DoNormalJump_Fixed();
	}
	AActor* Base = m_pPlayerPawn->Base;
	if( Base )
	{
		while( Base->bHardAttach && Base->Base && !Base->Base->bWorldGeometry )
		{
			Base = Base->Base;
		}
		if( !Base->bWorldGeometry && Base->Velocity.Z > 0.f )
		{
			m_pPlayerPawn->Velocity.Z += m_pPlayerPawn->Base->Velocity.Z;
		}
	}
	m_pPlayerPawn->setPhysics( PHYS_Falling, NULL, FVector( 0.f, 0.f, 1.f ) );
	UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	m_fStartJumpHeight = m_pPlayerPawn->Location.Z;
	m_fMaxJumpHeight = 0.f;
	IFDIAG_ONCE( TEXT("UStatePlayerMasterJump: the camera bump smoother weight (GetCamera_BumpSmoother / SetWeight) is not ported") );
	m_bJumpingFromChain = m_pPlayerPawn->m_pClimbable != NULL;
	IFDIAG( TEXT("jump enter: style %d, Velocity.Z %.1f, start Z %.1f"), (INT)m_JumpStyle, m_pPlayerPawn->Velocity.Z, m_fStartJumpHeight );
	PlayJumpInActions();
	m_bPowerJumpAllowed = TRUE;
}

// 2013 0x65ffa0: the blend flag, the bump smoother back to its default, the sneak/zoom input back, and a chain jump's climbable released
void UStatePlayerMasterJump::OnExitState( UDishonoredNativeState* NextState )
{
	m_bBlendingFromLastState = FALSE;
	m_pPlayerPawn->EnableControllerInput( 0x8200, TRUE, DisInputMaskLevel_MasterAction );
	if( m_bJumpingFromChain && m_pPlayerPawn->m_pClimbable )
	{
		IFDIAG_ONCE( TEXT("UStatePlayerMasterJump::OnExitState: ADisClimbable::StartCollisionRestoreTimer is not ported") );
		m_pPlayerPawn->m_pClimbable = NULL;
	}
	IFDIAG( TEXT("jump exit: peak %.1f above the start"), m_fMaxJumpHeight );
}

// 2013 0x679150: the highest point so far, the style's logic, and a ranged secondary at full dispersion while airborne
void UStatePlayerMasterJump::TickState( FLOAT DeltaSeconds )
{
	static const FName NAME_DispersionMax( TEXT("Attribute_DispersionMax") );
	m_fMaxJumpHeight = Max( m_fMaxJumpHeight, m_pPlayerPawn->Location.Z - m_fStartJumpHeight );
	if( m_JumpStyle == 0 )
	{
		TickJumpLogic_FixedPower( DeltaSeconds );
	}
	else if( m_JumpStyle == 1 )
	{
		TickJumpLogic_HeldPower_FullStop( DeltaSeconds );
	}
	else if( m_JumpStyle == 2 )
	{
		TickJumpLogic_ContinuousPower( DeltaSeconds );
	}
	else
	{
		TickJumpLogic_NoPower( DeltaSeconds );
	}
	UDishonoredWeapon_Ranged* Ranged = Cast<UDishonoredWeapon_Ranged>( m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Secondary ) );
	if( Ranged )
	{
		UDisTweaks_WeaponRanged* Tweaks = Cast<UDisTweaks_WeaponRanged>( Ranged->GetTweaks_Derived() );
		if( Tweaks == NULL )
		{
			Tweaks = UDisTweaks_WeaponRanged::StaticClass()->GetDefaultObject<UDisTweaks_WeaponRanged>();
		}
		if( Tweaks->m_bMaxDispersionOnJump )
		{
			Ranged->m_fDispersion = Ranged->GetAttributeValue( NAME_DispersionMax );
		}
	}
}

// 2013 0x66eb00 (47 B): the jump refuses a Falling request (its own PHYS_Falling raises one); it demands Falling itself
UBOOL UStatePlayerMasterJump::CanTransitionExternal( const UClass* StateID ) const
{
	return StateID != UStatePlayerMasterFalling::StaticClass();
}

// 2013 0x660010 (74 B)
void UStatePlayerMasterJump::DoNormalJump_Fixed()
{
	m_pPlayerPawn->Velocity.Z = DisIFJumpZ( m_pPlayerPawn );
}

// 2013 0x671370 (115 B): the power jump speed, the jump FOV target, the power jump sound
void UStatePlayerMasterJump::DoPowerJump_Fixed()
{
	static const FName NAME_JumpZ_PowerJump( TEXT("Attribute_JumpZ_PowerJump") );
	m_pPlayerPawn->Velocity.Z = m_pPlayerPawn->GetAttributeValue( NAME_JumpZ_PowerJump );
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	UStatePlayerMasterJump* Default = UStatePlayerMasterJump::StaticClass()->GetDefaultObject<UStatePlayerMasterJump>();
	if( Camera )
	{
		Camera->SetFOVTarget( eDisCamFOVPriority_Locomotion_Jump, Default->m_fPowerJumpFOV, Default->m_fPowerJumpFOVBlendSpeed, TRUE );
	}
	if( m_pPowerJumpSoundEvent )
	{
		m_pPlayerPawn->PostAkEvent( m_pPowerJumpSoundEvent );
	}
}

// 2013 0x660060 (102 B): the style's power attribute is set
UBOOL UStatePlayerMasterJump::HasPowerJump() const
{
	static const FName NAME_JumpZ_PowerJump( TEXT("Attribute_JumpZ_PowerJump") );
	static const FName NAME_HeldPowerJumpButtonTime( TEXT("Attribute_HeldPowerJumpButtonTime_PROTOTYPE") );
	static const FName NAME_PowerJumpFullStop( TEXT("Attribute_PowerJumpFullStop_PROTOTYPE") );
	if( m_JumpStyle == 0 )
	{
		return m_pPlayerPawn->GetAttributeValue( NAME_JumpZ_PowerJump ) > 0.0001f;
	}
	if( m_JumpStyle == 1 )
	{
		return m_pPlayerPawn->GetAttributeValue( NAME_HeldPowerJumpButtonTime ) > 0.0001f;
	}
	if( m_JumpStyle == 2 )
	{
		return m_pPlayerPawn->GetAttributeValue( NAME_PowerJumpFullStop ) > 0.0001f;
	}
	return FALSE;
}

// 2013 0x672ed0 (335 B): the jump-in action for an armed primary hand (the sneak variant while sneaking) and for an armed left hand
void UStatePlayerMasterJump::PlayJumpInActions()
{
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( m_pPlayerPawn->GetTweaks_Derived() );
	if( Tweaks == NULL )
	{
		Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
	}
	UDisTweaks_PlayerPawn_NavigationActions* Actions = ((UDisTweaks_PlayerPawn_Actions*)Tweaks->m_pActionTweaks)->m_pNavigationActionTweaks;
	UDishonoredInventoryItem* Primary = m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Primary );
	if( Primary && !Primary->IsA( UDishonoredItemEmpty::StaticClass() ) )
	{
		FStatePlayerAction_Param ActionParam( m_pPlayerPawn->IsSneaking() ? Actions->m_Action_JumpIn_Sneak : Actions->m_Action_JumpIn );
		m_pPlayerPawn->RequestAction( eDisPlayerActionUsage_Upperbody, ActionParam, NULL, FALSE );
	}
	UDishonoredInventoryItem* Secondary = m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Secondary );
	if( Secondary && !Secondary->IsA( UDishonoredItemEmpty::StaticClass() ) )
	{
		FStatePlayerAction_Param ActionParam( Actions->m_Action_JumpIn_LeftHand );
		m_pPlayerPawn->RequestAction( eDisPlayerActionUsage_LeftHand, ActionParam, NULL, FALSE );
	}
}

/**
 * 2013 0x676110 (233 B). Rising: the held-button time grows while the button is held (until the power jump fired). At the top:
 * a press held longer than 50 ms fires the power jump once, if allowed and available; otherwise the jump becomes a fall.
 */
void UStatePlayerMasterJump::TickJumpLogic_FixedPower( FLOAT DeltaSeconds )
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	if( PC == NULL )
	{
		return;
	}
	if( m_pPlayerPawn->Velocity.Z <= 0.0001f )
	{
		if( !m_bPowerJumpAllowed || m_fHeldButtonTime <= 0.05f || m_JumpState == 1 || !HasPowerJump() )
		{
			FStatePlayerMasterFalling_Param FallingParam;
			DemandStateChange( FallingParam );
			return;
		}
		DoPowerJump_Fixed();
		m_JumpState = 1;
	}
	if( m_JumpState != 1 )
	{
		m_fHeldButtonTime = PC->m_bJumpButton ? m_fHeldButtonTime + DeltaSeconds : 0.f;
	}
	UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
}

// 2013 0x676200 (212 B): releasing the button during a power jump caps the rise at the extra stop speed; at the top, a fall
void UStatePlayerMasterJump::TickJumpLogic_HeldPower_FullStop( FLOAT DeltaSeconds )
{
	static const FName NAME_FullStopExtraStopVel( TEXT("Attribute_FullStop_ExtraStopVel_PROTOTYPE") );
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	if( PC == NULL )
	{
		return;
	}
	if( m_JumpState == 1 && !PC->m_bJumpButton )
	{
		m_pPlayerPawn->Velocity.Z = Min( m_pPlayerPawn->Velocity.Z, m_pPlayerPawn->GetAttributeValue( NAME_FullStopExtraStopVel ) );
	}
	if( m_pPlayerPawn->Velocity.Z > 0.0001f )
	{
		UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
		return;
	}
	FStatePlayerMasterFalling_Param FallingParam;
	DemandStateChange( FallingParam );
}

/**
 * 2013 0x6762e0 (335 B). Holding the button accelerates the rise until the held time passes the power-jump time, which fires the
 * jump FOV and sound once; releasing ends the boost. At the top, a fall.
 */
void UStatePlayerMasterJump::TickJumpLogic_ContinuousPower( FLOAT DeltaSeconds )
{
	static const FName NAME_HeldPowerJumpButtonTime( TEXT("Attribute_HeldPowerJumpButtonTime_PROTOTYPE") );
	static const FName NAME_HeldPowerJumpAccel( TEXT("Attribute_HeldPowerJumpAccel_PROTOTYPE") );
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	if( PC == NULL )
	{
		return;
	}
	if( m_JumpState == 0 && HasPowerJump() )
	{
		if( PC->m_bJumpButton )
		{
			m_fHeldButtonTime += DeltaSeconds;
			if( m_fHeldButtonTime > m_pPlayerPawn->GetAttributeValue( NAME_HeldPowerJumpButtonTime ) )
			{
				m_JumpState = 1;
				ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
				UStatePlayerMasterJump* Default = UStatePlayerMasterJump::StaticClass()->GetDefaultObject<UStatePlayerMasterJump>();
				if( Camera )
				{
					Camera->SetFOVTarget( eDisCamFOVPriority_Locomotion_Jump, Default->m_fPowerJumpFOV, Default->m_fPowerJumpFOVBlendSpeed, TRUE );
				}
				if( m_pPowerJumpSoundEvent )
				{
					m_pPlayerPawn->PostAkEvent( m_pPowerJumpSoundEvent );
				}
			}
		}
		else
		{
			m_JumpState = 1;
		}
		if( m_JumpState == 0 )
		{
			m_pPlayerPawn->Velocity.Z += m_pPlayerPawn->GetAttributeValue( NAME_HeldPowerJumpAccel ) * DeltaSeconds;
		}
	}
	if( m_pPlayerPawn->Velocity.Z > 0.0001f )
	{
		UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
		return;
	}
	FStatePlayerMasterFalling_Param FallingParam;
	DemandStateChange( FallingParam );
}

// 2013 0x676430 (139 B): at the top, a fall
void UStatePlayerMasterJump::TickJumpLogic_NoPower( FLOAT DeltaSeconds )
{
	if( Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller ) == NULL )
	{
		return;
	}
	if( m_pPlayerPawn->Velocity.Z > 0.0001f )
	{
		UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
		return;
	}
	FStatePlayerMasterFalling_Param FallingParam;
	DemandStateChange( FallingParam );
}
