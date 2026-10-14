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
	bWaitToBlendOneFrame argument through), GetCameraPos 2012 0x70bdf0 (22 B; unnamed in 2013). GetViewRotation and
	GetPawnViewLocation are not ported.

	Agent GF (PHASE19 GF): OnCameraUpdate_Derived 2012 0x71e3b0 / 2013 0x6abdd0 (1227 / 1242 B) - the arm placement: the player
	skeletal component's offset is the arm-follow-weighted difference between the final view position and the non-additive camera
	position; the three single-bone skel controls named by the camera tweaks (right hand, left hand, camera) get the view pitch over
	the non-additive pitch plus the controller offset pitch, plus the arm-follow offset rotation eased by (1 - cos(w * PI)) / 2, scaled by
	the arm-follow rotation weight and relative to the pawn's pitch; the arms' FOV is m_fCurFOV_Arms blended into the view FOV by
	m_MeshSpecificFOVWeight (DisableFOV when they coincide). On pass 1 the upper FSM's logical state and the active powers get the
	view (UStatePlayerUpperBase::OnCameraUpdate and the power components have no bodies here - named once).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "gfarmsdiag.h"

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

void ADishonoredPlayerPawn::OnCameraUpdate_Derived( const FDishonoredViewTarget& _VT, INT _PassCount )
{
	UDishonoredPlayerSkeletalComponent* PlayerMesh = Cast<UDishonoredPlayerSkeletalComponent>( Mesh );
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( PlayerMesh )
	{
		const FDishonoredVTSettings& S = _VT.m_DishonoredSettings;
		PlayerMesh->SetOffset( ( _VT.m_VT.POV.Location - S.m_NonAdditive_Pos ) * S.m_ArmFollowWeight );
		const UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( GetPawnTweaks() );
		if( !Tweaks )
		{
			Tweaks = (UDisTweaks_PlayerPawn*)UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject();
		}
		const UDisTweaks_PlayerPawn_Camera* CameraTweaks = Tweaks ? Tweaks->m_pCameraTweaks : NULL;
		if( CameraTweaks )
		{
			if( !m_pLookAtControl_RightHand )
			{
				m_pLookAtControl_RightHand = Cast<USkelControlSingleBone>( Mesh->FindSkelControl( CameraTweaks->m_LookAtControlName_RightHand ) );
			}
			if( !m_pLookAtControl_LeftHand )
			{
				m_pLookAtControl_LeftHand = Cast<USkelControlSingleBone>( Mesh->FindSkelControl( CameraTweaks->m_LookAtControlName_LeftHand ) );
			}
			if( !m_pLookAtControl_Camera )
			{
				m_pLookAtControl_Camera = Cast<USkelControlSingleBone>( Mesh->FindSkelControl( CameraTweaks->m_LookAtControlName_Camera ) );
			}
		}
		const INT ViewPitch = _VT.m_VT.POV.Rotation.Pitch - S.m_NonAdditive_Rot.Pitch + S.m_ControllerOffset_Rot.Pitch;
		struct FArmControl
		{
			static void Apply( USkelControlSingleBone* Control, INT InViewPitch, const FRotator& OffsetRot, FLOAT OffsetWeight, FLOAT RotWeight, INT PawnPitch )
			{
				if( !Control )
				{
					return;
				}
				Control->BoneRotation = FRotator( InViewPitch, 0, 0 );
				Control->BoneRotation.MakeShortestRoute();
				Control->BoneRotation += OffsetRot * ( ( 1.f - appCos( OffsetWeight * PI ) ) * 0.5f );
				Control->BoneRotation.MakeShortestRoute();
				Control->BoneRotation.Pitch = appTrunc( (FLOAT)Control->BoneRotation.Pitch * RotWeight ) - PawnPitch;
				Control->BoneRotation.MakeShortestRoute();
			}
		};
		FArmControl::Apply( m_pLookAtControl_RightHand, ViewPitch, S.m_ArmFollowOffset_Rot_Primary, S.m_ArmFollowOffset_Weight_Primary, S.m_ArmFollowWeight_Rot_Primary, Rotation.Pitch );
		FArmControl::Apply( m_pLookAtControl_LeftHand, ViewPitch, S.m_ArmFollowOffset_Rot_Secondary, S.m_ArmFollowOffset_Weight_Secondary, S.m_ArmFollowWeight_Rot_Secondary, Rotation.Pitch );
		FArmControl::Apply( m_pLookAtControl_Camera, ViewPitch, FRotator( 0, 0, 0 ), Max( S.m_ArmFollowOffset_Weight_Primary, S.m_ArmFollowOffset_Weight_Secondary ),
			Max( S.m_ArmFollowWeight_Rot_Primary, S.m_ArmFollowWeight_Rot_Secondary ), Rotation.Pitch );
		const FLOAT ViewFOV = _VT.m_VT.POV.FOV;
		const FLOAT ArmsFOV = Camera ? ( Camera->m_fCurFOV_Arms - ViewFOV ) * S.m_MeshSpecificFOVWeight + ViewFOV : ViewFOV;
		if( Abs( ArmsFOV - ViewFOV ) <= 1e-8f )
		{
			PlayerMesh->DisableFOV();
		}
		else
		{
			PlayerMesh->SetFOV( ArmsFOV );
		}
		Mesh->BeginDeferredUpdateTransform();
	}
	if( _PassCount == 1 )
	{
		GFDIAG_ONCE( TEXT("OnCameraUpdate_Derived pass 1: UStatePlayerUpperBase::OnCameraUpdate and the active powers' OnCameraUpdate are not ported") );
	}
}
