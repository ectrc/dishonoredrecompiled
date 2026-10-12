// DishonoredGame/src/dishonoredcamera_playercontrol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x721e00  public: static void __cdecl UDishonoredCamera_PlayerControl::InitializePrivateStaticClassUDishonoredCamera_PlayerControl(void)
//   0x721e20  protected: virtual void __thiscall UDishonoredCamera_PlayerControl::PostBeginPlay_Derived(void)
//   0x721e30  protected: virtual void __thiscall UDishonoredCamera_PlayerControl::OnActivate_Derived(class APawn *)
//   0x721e40  public: virtual void __thiscall UDishonoredCamera_PlayerControl::OnTeleport(class USeqAct_Teleport *)
//   0x721e50  public: void __thiscall UDishonoredCamera_PlayerControl::GoToTargetOnNextControllerReset(class FRotator const &)
//   0x729ef0  public: static class UClass * __cdecl UDishonoredCamera_PlayerControl::GetPrivateStaticClassUDishonoredCamera_PlayerControl(wchar_t const *)
//   0x729f80  protected: float __thiscall UDishonoredCamera_PlayerControl::CalcItemSpecificOffset(class APawn const *, float, enum EDisEquipUsage)const
//   0x72a0f0  protected: virtual unsigned int __thiscall UDishonoredCamera_PlayerControl::GatherInfluence_Derived(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)
//   0x72d0b0  public: static class UClass * __cdecl UDishonoredCamera_PlayerControl::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the first-person influence the camera's core group holds at weight 1 outside a soiree.
	GatherInfluence_Derived 2012 0x72a0f0 / 2013 0x6cba40 (named in both, same 558 B), CalcItemSpecificOffset 0x729f80 /
	2013 0x6cb8d0 (same 363 B), GoToTargetOnNextControllerReset 0x721e50 / 2013 0x6c1660, OnActivate_Derived 0x721e30,
	PostBeginPlay_Derived 0x721e20, OnTeleport 0x721e40 (the last three inline in the CppText; unnamed in 2013).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_CameraPlayerControl()
{
}

// the first activation after PostBeginPlay (m_bFirstRotation) does not reset the controller's rotation; every later one does
void UDishonoredCamera_PlayerControl::OnActivate_Derived( APawn* Pawn )
{
	if( !m_bFirstRotation )
	{
		m_bResetControllerRot = TRUE;
	}
}

// while inactive, remember the rotation the controller is to be given when this influence takes over again
void UDishonoredCamera_PlayerControl::GoToTargetOnNextControllerReset( const FRotator& ResetTarget )
{
	if( !m_bActive )
	{
		m_TargetOnControllerReset = ResetTarget;
		m_bGoToTargetOnControllerReset = TRUE;
	}
}

/**
 * The item in the given hand contributes an arm pitch offset read off its player-specific tweaks' m_ArmPitch_OffsetKeys
 * (sorted by pitch, in degrees): the key below the current pitch, cosine-eased toward the key above it.
 * The item's tweaks come from its IDisTweaksInterface, falling back to UDisTweaks_InventoryItem's class default.
 */
FLOAT UDishonoredCamera_PlayerControl::CalcItemSpecificOffset( const APawn* Pawn, FLOAT CurPitch, BYTE Usage ) const
{
	const ADishonoredPawn* DisPawn = ConstCast<ADishonoredPawn>( Pawn );
	if( !DisPawn || !DisPawn->m_pInventory )
	{
		return 0.f;
	}
	UDishonoredInventoryItem* Item = DisPawn->m_pInventory->GetEquippedItem( Usage );
	if( !Item )
	{
		return 0.f;
	}
	const UDisTweaks_InventoryItem* ItemTweaks = Cast<UDisTweaks_InventoryItem>( Item->GetTweaks() );
	if( !ItemTweaks )
	{
		ItemTweaks = (UDisTweaks_InventoryItem*)UDisTweaks_InventoryItem::StaticClass()->GetDefaultObject();
	}
	const UDisTweaks_InventoryItem_PlayerSpecific* PlayerTweaks = ItemTweaks ? ItemTweaks->m_pPlayerSpecificTweaks : NULL;
	if( !PlayerTweaks )
	{
		return 0.f;
	}
	const TArrayNoInit<FDisItemArmPitchDif>& Keys = PlayerTweaks->m_ArmPitch_OffsetKeys;
	const FDisItemArmPitchDif* StartKey = NULL;
	const FDisItemArmPitchDif* EndKey = NULL;
	for( INT Index = 0; Index < Keys.Num(); Index++ )
	{
		EndKey = &Keys(Index);
		if( EndKey->m_fArmPitch > CurPitch )
		{
			break;
		}
		StartKey = EndKey;
	}
	if( !StartKey )
	{
		StartKey = EndKey;
	}
	if( !EndKey || StartKey == EndKey )
	{
		return StartKey ? StartKey->m_fArmPitch_Difference : 0.f;
	}
	const FLOAT Weight = Clamp( ( CurPitch - StartKey->m_fArmPitch ) / ( EndKey->m_fArmPitch - StartKey->m_fArmPitch ), 0.f, 1.f );
	return StartKey->m_fArmPitch_Difference + ( 1.f - appCos( Weight * PI ) ) * 0.5f * ( EndKey->m_fArmPitch_Difference - StartKey->m_fArmPitch_Difference );
}

/**
 * A pending controller reset gives the controller the pawn's yaw (and, when asked by GoToTargetOnNextControllerReset, the
 * remembered target, through the controller's StartTrackingFixedAngles when the transition is finite). Then the view: the
 * pawn's camera position (ADishonoredPawn::GetCameraPos, the player's camera bone) and view rotation, the controller's
 * rotation as the controller offset, every arm-follow weight at 1, and each hand's item pitch offset.
 * DISHONORED(bringup): ADishonoredPlayerController::StartTrackingFixedAngles (2012 0x6f8b90) has no body in this tree, so
 * the remembered target is applied at once rather than tracked over the transition.
 */
UBOOL UDishonoredCamera_PlayerControl::GatherInfluence_Derived( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	if( !Pawn )
	{
		return FALSE;
	}
	m_bFirstRotation = FALSE;
	if( m_bResetControllerRot )
	{
		FRotator PawnRot = Pawn->Rotation;
		PawnRot.MakeShortestRoute();
		PawnRot.Pitch = 0;
		PawnRot.Roll = 0;
		if( Pawn->Controller )
		{
			Pawn->Controller->SetRotation( PawnRot );
		}
		if( m_bGoToTargetOnControllerReset )
		{
			ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Pawn->Controller );
			if( PC )
			{
				FRotator FinalRot( m_TargetOnControllerReset.Pitch, m_TargetOnControllerReset.Yaw, PC->Rotation.Roll );
				FinalRot.MakeShortestRoute();
				PC->SetRotation( FinalRot );
				Pawn->eventUpdatePawnRotation( FinalRot );
			}
			m_bGoToTargetOnControllerReset = FALSE;
		}
		m_bResetControllerRot = FALSE;
	}
	const ADishonoredPawn* DisPawn = ConstCast<ADishonoredPawn>( Pawn );
	OutVT.m_VT.POV.Location = DisPawn ? DisPawn->GetCameraPos() : Pawn->Location;
	OutVT.m_VT.POV.Rotation = Pawn->GetViewRotation();
	if( Pawn->Controller )
	{
		OutVT.m_DishonoredSettings.m_ControllerOffset_Rot = Pawn->Controller->Rotation;
		OutVT.m_DishonoredSettings.m_ControllerOffset_Rot.MakeShortestRoute();
	}
	OutVT.m_VT.POV.Rotation.MakeShortestRoute();
	const FLOAT CurPitch = (FLOAT)OutVT.m_VT.POV.Rotation.Pitch * ( 360.f / 65536.f );
	FDishonoredVTSettings& Settings = OutVT.m_DishonoredSettings;
	Settings.m_ArmFollowWeight = 1.f;
	Settings.m_ArmFollowWeight_Rot_Primary = 1.f;
	Settings.m_ArmFollowWeight_Rot_Secondary = 1.f;
	Settings.m_MeshSpecificFOVWeight = 1.f;
	Settings.m_ArmFollowOffset_Weight_Primary = 1.f;
	Settings.m_ArmFollowOffset_Weight_Secondary = 1.f;
	Settings.m_ArmFollowOffset_Rot_Primary.Pitch = appTrunc( CalcItemSpecificOffset( Pawn, CurPitch, EDisEquipUsage_Primary ) * ( 65536.f / 360.f ) );
	Settings.m_ArmFollowOffset_Rot_Secondary.Pitch = appTrunc( CalcItemSpecificOffset( Pawn, CurPitch, EDisEquipUsage_Secondary ) * ( 65536.f / 360.f ) );
	m_Debug_POV_Rotator = OutVT.m_VT.POV.Rotation;
	m_Debug_POV_Location = OutVT.m_VT.POV.Location;
	return TRUE;
}
