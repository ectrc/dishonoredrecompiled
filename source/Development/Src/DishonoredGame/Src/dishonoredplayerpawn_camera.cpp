// DishonoredGame/src/dishonoredplayerpawn_camera.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x705540  public: void __thiscall ADishonoredPlayerPawn::GetCameraBone(class FVector *, class FRotator *)const
//   0x705600  public: virtual class FRotator __thiscall ADishonoredPlayerPawn::GetViewRotation(void)
//   0x70be10  public: void __thiscall ADishonoredPlayerPawn::EnableCoreCamInfluence(class UClass const *, float)
//   0x71a6c0  public: virtual class FVector __thiscall ADishonoredPlayerPawn::GetPawnViewLocation(void)
//   0x71e3b0  public: virtual void __thiscall ADishonoredPlayerPawn::OnCameraUpdate_Derived(struct FDishonoredViewTarget const &, int)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the player pawn's three camera reads the soiree needs. GetCameraBone 2012 0x705540 / 2013
	0x6aba60 (named in both), EnableCoreCamInfluence 2012 0x70be10 / 2013 0x6aba00 (callee 0.688; the 2013 body passes the
	bWaitToBlendOneFrame argument through), GetCameraPos 2012 0x70bdf0 (22 B; unnamed in 2013). GetViewRotation,
	GetPawnViewLocation and OnCameraUpdate_Derived (the arm placement, 1227 B) are not ported.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PlayerPawnCamera()
{
}

/**
 * DISHONORED(port): 2012 0x705540. The bone named by the player tweaks' camera block (UDisTweaks_PlayerPawn::
 * m_pCameraTweaks->m_CameraBoneName), through ADishonoredPawn::GetBone_ByName; while ghosting with the chest view
 * (m_Debug_Player +12 bits 1 and 2: m_bIsGhosting, m_bGhostChestView) the pawn's own location and rotation instead. The tweaks fall back to the
 * class default object when the pawn has none, as every retail tweak read does.
 */
void ADishonoredPlayerPawn::GetCameraBone( FVector* OutLocation, FRotator* OutRotation ) const
{
	const UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( GetPawnTweaks() );
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_PlayerPawn*)UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject();
	}
	const UDisTweaks_PlayerPawn_Camera* CameraTweaks = Tweaks ? Tweaks->m_pCameraTweaks : NULL;
	if( m_Debug_Player.m_bIsGhosting && m_Debug_Player.m_bGhostChestView )
	{
		if( OutLocation )
		{
			*OutLocation = Location;
		}
		if( OutRotation )
		{
			*OutRotation = Rotation;
		}
		return;
	}
	GetBone_ByName( CameraTweaks ? CameraTweaks->m_CameraBoneName : NAME_None, OutLocation, OutRotation );
}

// DISHONORED(port): 2012 0x70bdf0: the camera bone's location is the player's camera position
FVector ADishonoredPlayerPawn::GetCameraPos() const
{
	FVector Result = Location;
	GetCameraBone( &Result, NULL );
	return Result;
}

// DISHONORED(port): 2013 0x6aba00: the camera's first (core, non-additive) influence group blends to the given influence class with weight 1
void ADishonoredPlayerPawn::EnableCoreCamInfluence( const UClass* InfluenceClass, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame )
{
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( Camera && Camera->InfluenceGroups().Num() > 0 && Camera->InfluenceGroups()(0) )
	{
		Camera->InfluenceGroups()(0)->SetWeight( 1.f, InfluenceClass, BlendTime, bWaitToBlendOneFrame );
	}
}
