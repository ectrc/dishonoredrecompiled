// DishonoredGame/src/stateplayermasterswim.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x6af7c0  public: static void __cdecl UStatePlayerMasterSwim::InitializePrivateStaticClassUStatePlayerMasterSwim(void)
//   0x6af7e0  public: unsigned int __thiscall UStatePlayerMasterSwim::IsOnSurface(void)const
//   0x6af800  public: void __thiscall UStatePlayerMasterSwim::Stroke(void)
//   0x6b3e20  public: virtual void __thiscall UStatePlayerMasterSwim::OnExitState(class UDishonoredNativeState *)
//   0x6b3ef0  public: virtual unsigned int __thiscall UStatePlayerMasterSwim::CanTransitionExternal(class UClass const *)const
//   0x6b3f20  public: virtual void __thiscall UStatePlayerMasterSwim::OnRejectStateRequest(class UClass const *)
//   0x6b3f50  public: float __thiscall UStatePlayerMasterSwim::GetSwimStrokeAccel(void)const
//   0x6b78b0  public: static class UClass * __cdecl UStatePlayerMasterSwim::GetPrivateStaticClassUStatePlayerMasterSwim(wchar_t const *)
//   0x6b7940  public: virtual void __thiscall UStatePlayerMasterSwim::OnEnterState(class UDishonoredNativeState *)
//   0x6beab0  public: static class UClass * __cdecl UStatePlayerMasterSwim::StaticClassNoInline(void)
//   0x6c3bb0  public: virtual void __thiscall UStatePlayerMasterSwim::TickState(float)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): UStatePlayerMasterSwim. ADishonoredPlayerPawn::setPhysics requests it for PHYS_Swimming (the engine's
	zone change into a water volume starts swimming); leaving the water (PHYS_Falling / PHYS_Walking) requests the next state.
	ADishonoredPlayerController::CalcPlayerSwimAccelRate, which the script's swimming move takes its acceleration from, asks it
	for the stroke acceleration. The 2013 bodies are ported (rvas in Inc/CppText/UStatePlayerMasterSwim.h).
	At the surface (no active water volume - and m_pActiveWaterVolume stays NULL in this tree, see dishonoredwatervolume.cpp)
	the pawn strokes on its own every m_fSurfaceSwimDelayBetweenStrokes; a stroke gives m_fMaxSwimAccel for m_fStrokeTime, between
	strokes m_fMinSwimAccel below m_fMaxSpeedNoStroke and nothing above it.
	DISHONORED(bringup), named and not called inside: the swim anim state (ADishonoredPlayerPawn::SetAnimState with Nav_Swim; the
	anim-state system is not ported), the fish attractor (UDishonoredMapInfo::CreateAttractor / DestroyAttractor, unported - the
	pawn's m_pFishAttractor stays NULL), the anim-state component's picker lock while the hands are empty, the projectiles' cancel.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_UStatePlayerMasterSwim()
{
}

// 2012 0x693cc0 / 2013 0x66dc80 (42 B)
FStatePlayerMasterSwim_Param::FStatePlayerMasterSwim_Param()
	: FStatePlayerMasterBase_Param( UStatePlayerMasterSwim::StaticClass() )
{
}

/**
 * 2013 0x673190 (729 B). The action blend and projectiles; only the swim inputs (0x108FB) enabled; both hands emptied and hidden
 * (a power in the left hand stays), the push box's channels 16 and 9 raised around the equip and lowered after; no sneaking, the
 * engine's crouch (the swimming height), the arms following again, the locomotion FOV target cleared.
 */
void UStatePlayerMasterSwim::OnEnterState( UDishonoredNativeState* LastState )
{
	UStatePlayerMasterAction* Action = LastState ? Cast<UStatePlayerMasterAction>( LastState ) : NULL;
	if( Action )
	{
		m_bBlendingFromLastState = TRUE;
		m_fBlendTimeFromLastState = Action->m_fTimeToGo;
	}
	if( !AreIncomingAttacksAllowed() )
	{
		IFDIAG_ONCE( TEXT("UStatePlayerMasterSwim::OnEnterState: UDisGlobalProjectileManager::CancelActiveProjectiles is not ported") );
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, FALSE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->EnableControllerInput( 0x108FB, TRUE, DisInputMaskLevel_MasterAction );
	IFDIAG_ONCE( TEXT("UStatePlayerMasterSwim::OnEnterState: SetAnimState( Nav_Swim ) is not ported (the anim-state system)") );
	m_fFallingTimer = 0.f;
	UDishonoredInventory* Inventory = m_pPlayerPawn->m_pInventory;
	Inventory->ShowEquippedItem( FALSE, EDisEquipUsage_Primary );
	Inventory->ShowEquippedItem( FALSE, EDisEquipUsage_Secondary );
	Inventory->EquipItemType( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Primary, TRUE );
	UDishonoredInventoryItem* Secondary = m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Secondary );
	if( Secondary == NULL || Secondary->GetClass() != UDisItemPowers::StaticClass() )
	{
		Inventory->EquipItemType( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Secondary, TRUE );
	}
	m_pPlayerPawn->PushBoxCollisionChannel.SetChannel( (ERBCollisionChannel)16, TRUE );
	m_pPlayerPawn->PushBoxCollisionChannel.SetChannel( (ERBCollisionChannel)9, TRUE );
	if( m_pPlayerPawn->IsSneaking() )
	{
		m_pPlayerPawn->StopSneaking( FALSE );
	}
	if( !m_pPlayerPawn->bIsCrouched )
	{
		m_pPlayerPawn->Crouch( FALSE );
	}
	m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, FALSE, -1.f, FALSE );
	m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, FALSE, -1.f, FALSE );
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	if( Camera )
	{
		Camera->ClearFOVTarget( eDisCamFOVPriority_Locomotion );
	}
	IFDIAG_ONCE( TEXT("UStatePlayerMasterSwim: the fish attractor (UDishonoredMapInfo::CreateAttractor / DestroyAttractor) is not ported") );
	m_pPlayerPawn->PushBoxCollisionChannel.SetChannel( (ERBCollisionChannel)16, FALSE );
	m_pPlayerPawn->PushBoxCollisionChannel.SetChannel( (ERBCollisionChannel)9, FALSE );
	IFDIAG( TEXT("swim enter: height %.2f crouched %d, physics %d"), m_pPlayerPawn->CylinderComponent->CollisionHeight, (INT)m_pPlayerPawn->bIsCrouched, (INT)m_pPlayerPawn->Physics );
}

/**
 * 2013 0x673470 (244 B). Mantling disallowed, every input back, the arms-only body; unless the pawn died, the hands get their
 * re-equip items back (a left-hand power stays).
 */
void UStatePlayerMasterSwim::OnExitState( UDishonoredNativeState* NextState )
{
	m_pPlayerPawn->m_bMantleAllowed = FALSE;
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->SetBodyMode( BODY_MODE_ARMS_ONLY );
	if( NextState && !NextState->IsA( UStatePlayerMasterDead::StaticClass() ) )
	{
		UClass* Reequip = m_pPlayerPawn->m_pInventory->GetReequipItem( EDisEquipUsage_Primary, FALSE );
		if( Reequip )
		{
			m_pPlayerPawn->EquipItemByType_KeepTrying( Reequip, EDisEquipUsage_Primary, ADishonoredPawn::EEquipItemFlags_FastEquip );
		}
		UDishonoredInventoryItem* Secondary = m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Secondary );
		if( Secondary == NULL || Secondary->GetClass() != UDisItemPowers::StaticClass() )
		{
			UClass* SecondaryReequip = m_pPlayerPawn->m_pInventory->GetReequipItem( EDisEquipUsage_Secondary, FALSE );
			if( SecondaryReequip )
			{
				m_pPlayerPawn->EquipItemByType_KeepTrying( SecondaryReequip, EDisEquipUsage_Secondary, ADishonoredPawn::EEquipItemFlags_FastEquip );
			}
		}
	}
	IFDIAG( TEXT("swim exit to %s"), NextState ? *NextState->GetClass()->GetName() : TEXT("none") );
}

/**
 * 2013 0x676710 (614 B). Empty hands show the arms only (hidden with the left hand disabled). At the surface the pawn strokes on
 * its own (the stroke time extended by the upgrade attribute unless the surface speed is constant) with the surface swim sound,
 * every m_fSurfaceSwimDelayBetweenStrokes. A power left in the left hand while another item waits to be re-equipped there is
 * swapped for empty hands. The stroke runs out; mantling is allowed; a refused fall that persists for a second becomes a fall.
 */
void UStatePlayerMasterSwim::TickState( FLOAT DeltaSeconds )
{
	static const FName NAME_PlayerSwimStrokeTimeExtension( TEXT("Attribute_PlayerSwimStrokeTimeExtension") );
	if( m_pPlayerPawn->HandsAppearEmpty() )
	{
		IFDIAG_ONCE( TEXT("UStatePlayerMasterSwim::TickState: UDisAnimStateComponent::LockPickerChildren is not ported") );
		m_pPlayerPawn->SetBodyMode( m_pPlayerPawn->IsLeftHandDisabled() ? BODY_MODE_HIDDEN : BODY_MODE_ARMS_ONLY );
	}
	UBOOL bResetStrokeDelay = TRUE;
	if( m_pPlayerPawn->m_pActiveWaterVolume == NULL )
	{
		m_fSurfaceSwimDelayBetweenStrokes -= DeltaSeconds;
		if( m_fSurfaceSwimDelayBetweenStrokes > 0.f )
		{
			bResetStrokeDelay = FALSE;
		}
		else
		{
			if( !m_bConstantSpeedAtSurface )
			{
				m_fCurStrokeTime = m_pPlayerPawn->GetAttributeValue( NAME_PlayerSwimStrokeTimeExtension ) + m_fStrokeTime;
			}
			UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( m_pPlayerPawn->GetTweaks_Derived() );
			if( Tweaks == NULL )
			{
				Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
			}
			if( Tweaks->m_pSurfaceSwimSound )
			{
				m_pPlayerPawn->PostAkEvent( Tweaks->m_pSurfaceSwimSound );
			}
		}
	}
	if( bResetStrokeDelay )
	{
		m_fSurfaceSwimDelayBetweenStrokes = GetClass()->GetDefaultObject<UStatePlayerMasterSwim>()->m_fSurfaceSwimDelayBetweenStrokes;
	}
	UClass* Reequip = m_pPlayerPawn->m_pInventory->GetReequipItem( EDisEquipUsage_Secondary, FALSE );
	UDishonoredInventoryItem* Secondary = m_pPlayerPawn->GetEquippedItem( EDisEquipUsage_Secondary );
	if( Secondary && Secondary->IsA( UDisItemPowers::StaticClass() ) && Reequip && !Reequip->IsChildOf( UDisItemPowers::StaticClass() ) )
	{
		m_pPlayerPawn->m_pInventory->EquipItemType( UDishonoredItemEmpty::StaticClass(), EDisEquipUsage_Secondary, FALSE );
		m_pPlayerPawn->m_pInventory->SetReequipItem( EDisEquipUsage_Secondary, Reequip );
	}
	if( m_fCurStrokeTime > 0.f )
	{
		m_fCurStrokeTime -= DeltaSeconds;
	}
	m_pPlayerPawn->m_bMantleAllowed = TRUE;
	if( m_bRejectedFalling )
	{
		m_fFallingTimer += DeltaSeconds;
		if( m_fFallingTimer > 1.f )
		{
			FStatePlayerMasterFalling_Param FallingParam;
			DemandStateChange( FallingParam );
		}
		m_bRejectedFalling = FALSE;
	}
	else
	{
		m_fFallingTimer = 0.f;
	}
}

// 2013 0x66f990 (47 B): a fall is refused (the swim state demands one itself after a second of refused requests)
UBOOL UStatePlayerMasterSwim::CanTransitionExternal( const UClass* StateID ) const
{
	return StateID != UStatePlayerMasterFalling::StaticClass();
}

// 2013 0x66f9c0 (39 B)
void UStatePlayerMasterSwim::OnRejectStateRequest( const UClass* StateID )
{
	if( StateID == UStatePlayerMasterFalling::StaticClass() )
	{
		m_bRejectedFalling = TRUE;
	}
}

/**
 * 2013 0x660690 (275 B). During a stroke m_fMaxSwimAccel; between strokes m_fMinSwimAccel while slower than m_fMaxSpeedNoStroke,
 * else nothing. In a DLC game (2013 0x7bf1f0: the game info's class is none of ADishonoredGameInfo and two DLC info classes whose
 * StaticClass thunks, 0xcaabf0 / 0xcc3ca0, are not identified) the three are scaled by 1 + their upgrade attributes; the campaign's
 * game info is ADishonoredGameInfo itself, which is the only case written here.
 */
FLOAT UStatePlayerMasterSwim::GetSwimStrokeAccel() const
{
	if( m_fCurStrokeTime > 0.f )
	{
		return m_fMaxSwimAccel;
	}
	if( m_fMaxSpeedNoStroke * m_fMaxSpeedNoStroke > m_pPlayerPawn->Velocity.SizeSquared() )
	{
		return m_fMinSwimAccel;
	}
	return 0.f;
}

// 2013 0x660650 (17 B): at the surface while no water volume holds the camera
UBOOL UStatePlayerMasterSwim::IsOnSurface() const
{
	return m_pPlayerPawn->m_pActiveWaterVolume == NULL;
}
