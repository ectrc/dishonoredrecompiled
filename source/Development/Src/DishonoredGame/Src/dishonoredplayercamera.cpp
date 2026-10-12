// DishonoredGame/src/dishonoredplayercamera.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (39):
//   0x6fbf20  public: static void __cdecl ADishonoredPlayerCamera::InitializePrivateStaticClassADishonoredPlayerCamera(void)
//   0x6fbf40  public: virtual void __thiscall ADishonoredPlayerCamera::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x6fbf80  public: class UDishonoredCamera_PhysicalReact * __thiscall ADishonoredPlayerCamera::GetCamera_PhysicalReact(void)const
//   0x6fbf90  public: class UDishonoredCamera_HitReact * __thiscall ADishonoredPlayerCamera::GetCamera_HitReact(void)const
//   0x6fbfa0  public: class UDishonoredCamera_DisableArmFollow * __thiscall ADishonoredPlayerCamera::GetCamera_DisableArmFollow(enum EDisEquipUsage)const
//   0x6fbfd0  public: class UDishonoredCamera_BumpSmoother * __thiscall ADishonoredPlayerCamera::GetCamera_BumpSmoother(void)const
//   0x6fbfe0  public: class UDishonoredCamera_Lean * __thiscall ADishonoredPlayerCamera::GetCamera_Lean(void)const
//   0x6fbff0  public: class UDisCamera_Look * __thiscall ADishonoredPlayerCamera::GetCamera_Look(void)const
//   0x6fc000  public: class UDisCamera_Aim * __thiscall ADishonoredPlayerCamera::GetCamera_Aim(void)const
//   0x6fc010  public: class UDishonoredCamera_PlayerControl * __thiscall ADishonoredPlayerCamera::GetCamera_PlayerControl(void)const
//   0x6fc020  public: class UDishonoredCamera_AnimDriven * __thiscall ADishonoredPlayerCamera::GetCamera_AnimDriven(void)const
//   0x6fc030  public: class UDisCamera_UnpossessDeath * __thiscall ADishonoredPlayerCamera::GetCamera_UnpossessDeath(void)const
//   0x6fc040  public: class UDisCamera_Rumble * __thiscall ADishonoredPlayerCamera::GetCamera_Rumble(void)const
//   0x6fc050  public: class UDishonoredCamera_CrouchMantleOffset * __thiscall ADishonoredPlayerCamera::GetCamera_CrouchMantleOffset(void)const
//   0x6fc060  public: class UDisCamera_Possess * __thiscall ADishonoredPlayerCamera::GetCamera_Possess(void)const
//   0x6fc070  unsigned int __cdecl DisSpeedBlendVector(class FVector &, class FVector const &, class FVector const &, float, float, float)
//   0x6fc260  public: void __thiscall ADishonoredPlayerCamera::EnableCollision(unsigned int)
//   0x6fc280  public: void __thiscall ADishonoredPlayerCamera::SetCameraCollisionSize(float, float)
//   0x6fc2b0  public: void __thiscall ADishonoredPlayerCamera::SetRainEmitter(class AEmitter *, class FVector const &, class UDisSeqAct_SetRainEmitter *)
//   0x700120  private: unsigned int __thiscall ADishonoredPlayerCamera::CamMoveSmooth(class FVector const &, class FVector &)
//   0x709a80  private: void __thiscall ADishonoredPlayerCamera::TickFOV(float)
//   0x709ba0  private: void __thiscall ADishonoredPlayerCamera::TickPostProcess(float)
//   0x709c60  public: virtual unsigned int __thiscall ADishonoredPlayerCamera::IgnoreBlockingBy(class AActor const *)const
//   0x709cf0  private: class UDishonoredCameraInfluence * __thiscall ADishonoredPlayerCamera::GetCamera_Influence(class UClass *)const
//   0x709de0  public: void __thiscall ADishonoredPlayerCamera::OnTeleport(class USeqAct_Teleport *)
//   0x709e60  private: void __thiscall ADishonoredPlayerCamera::HandleCollision(class FVector const &)
//   0x709f40  public: void __thiscall ADishonoredPlayerCamera::SetFOVTarget(enum eDisCamFOVPriority, float, float, unsigned int)
//   0x709fb0  public: void __thiscall ADishonoredPlayerCamera::ClearFOVTarget(enum eDisCamFOVPriority)
//   0x70a010  public: void __thiscall ADishonoredPlayerCamera::SetPostProcessTarget(enum eDisCamPostProcessPriority, struct FArkPpConfig const &, float, int)
//   0x70a120  public: void __thiscall ADishonoredPlayerCamera::ClearPostProcessTarget(enum eDisCamPostProcessPriority)
//   0x70a1c0  public: void __thiscall ADishonoredPlayerCamera::ApplyCameraPostProcess(struct FArkPpConfig &)
//   0x70e6e0  public: virtual unsigned int __thiscall ADishonoredPlayerCamera::Tick(float, enum ELevelTick)
//   0x70e720  public: class UDisCamera_FollowProjectile * __thiscall ADishonoredPlayerCamera::GetCamera_FollowProjectile(void)const
//   0x7135c0  public: virtual void __thiscall ADishonoredPlayerCamera::UpdateViewTarget(struct FTViewTarget &, float)
//   0x718790  public: static class UClass * __cdecl ADishonoredPlayerCamera::GetPrivateStaticClassADishonoredPlayerCamera(wchar_t const *)
//   0x719040  public: static class UClass * __cdecl ADishonoredPlayerCamera::StaticClassNoInline(void)
//   0x71bbc0  public: virtual void __thiscall ADishonoredPlayerCamera::PostBeginPlay(void)
//   0x71bfd0  public: void __thiscall ADishonoredPlayerCamera::UseDefaultCamCollisionSize(void)
//   0x721020  public: virtual void __thiscall ADishonoredPlayerCamera::ApplyDebugCam_Native(class APawn *, float, struct FTViewTarget &)

// agentDO:camerapp
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "arkpp.h"

/**
 * DISHONORED(port): 2013 rva 0x6ca780 (2012 0x70a1c0) - the camera's own post-process targets. Eight entries per
 * target, each an FArkPpConfig with a current weight; every entry with weight above 0.0001 is blended into the
 * config the view will carry.
 *
 * The SetDefaultOnNoOverride / ForceDefault pair before each blend is retail's and it is per entry, not per target:
 * whichever of the two runs depends on the DESTINATION's override bit, so the first entry to blend decides how the
 * rest of the group sees the neutral value. Ported as retail has it.
 */
void ADishonoredPlayerCamera::ApplyCameraPostProcess( FArkPpConfig& Config )
{
	for( INT TargetIndex = 0; TargetIndex < m_PostProcessTargets.Num(); TargetIndex++ )
	{
		FDisCamPostProcessTarget& Target = m_PostProcessTargets(TargetIndex);
		for( INT EntryIndex = 0; EntryIndex < ARRAY_COUNT(Target.m_Entries); EntryIndex++ )
		{
			const FDisCamPostProcessEntry& Entry = Target.m_Entries[EntryIndex];
			const FLOAT Weight = Entry.m_fCurWeight;
			if( Weight <= 0.0001f )
			{
				continue;
			}
			if( Config.m_bOverrideUberPpParameters )
			{
				ArkUberPpSetDefaultOnNoOverride( Config.m_UberPpParameters );
			}
			else
			{
				ArkUberPpForceDefault( Config.m_UberPpParameters );
			}
			if( Entry.m_PpSettings.m_bOverrideUberPpParameters )
			{
				ArkUberPpApplyTo( Entry.m_PpSettings.m_UberPpParameters, Config.m_UberPpParameters, Weight, FALSE );
				Config.m_bOverrideUberPpParameters = TRUE;
			}
			// DISHONORED(retail): as in UpdatePostProcessSettings and FArkPpConfig::ApplyTo, the bloom group is only
			// ever marked overridden - no bloom value is copied with it.
			if( Entry.m_PpSettings.m_bOverrideBloomPpParameters )
			{
				Config.m_bOverrideBloomPpParameters = TRUE;
			}
		}
	}
}

// DISHONORED(port): agent EQ, 2013 rva 0x6c01f0 (2012 0x6fbf40). Four writes and no reason test: the field of
// view setting becomes both the camera's own default and ACamera::DefaultFOV, and the head-bob setting becomes
// both the bob and the roll amplitude. Retail's `this` is the IArkSettingsListenerInterface subobject at +1040,
// which is why the decompile writes +340 (m_fDefaultFOVSettings @1380) and -444 (ACamera::DefaultFOV @596).
void ADishonoredPlayerCamera::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	const FLOAT FieldOfView = (FLOAT)Parameters->m_FOV;
	m_fDefaultFOVSettings = FieldOfView;
	DefaultFOV = FieldOfView;
	m_RollAmount = Parameters->m_fHeadBobAmount;
	m_BobAmount = Parameters->m_fHeadBobAmount;
}

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the influence pipeline - what makes the view the player's camera bone during a soiree.

	Both databases (2013 rva = VA - 0x400000): PostBeginPlay 2012 0x71bbc0 / 2013 0x6d42b0 (named in both, 1030 -> 1046 B),
	Tick 0x70e6e0 / 2013 0x6d0290 (61 B), UpdateViewTarget 0x7135c0 / 2013 0x6d80a0 (5871 / 5883 B; the first instructions are
	the same in both: PCOwner->Pawn, ViewTarget.Target == Pawn, CameraStyle == 'FirstPerson' (name index 0x4B9 in both),
	FOV = m_fCurFOV, the three influence groups), TickFOV 0x709a80 (282 B, unnamed in 2013), SetFOVTarget 0x709f40 / 2013
	0x6ca500, ClearFOVTarget 0x709fb0 / 2013 0x6ca570, GetCamera_Influence 0x709cf0 / 2013 0x6ca280, OnTeleport 0x709de0.

	Of UpdateViewTarget, what is ported is the whole influence branch: FOV from m_fCurFOV, the bob and reaction groups'
	weights from m_BobAmount / m_fReactionWeight, every group gathered in turn over a capped 1/15 s delta (the uncapped one
	passed beside it), the no-collision placement of the camera actor at the blended non-additive position and rotation
	(FarMoveActor), and the fallbacks to ACamera::UpdateViewTarget for every other style. Not ported, each named: the
	frame-step pause copy (m_bSingleStep / m_bSingleStepPaused, +752 in 2012), the teleport detection off the mesh's tick
	tag, the collision branch (m_bCollisionEnabled: the cylinder resize through every active influence's
	AdjustCameraCollision, CamMoveSmooth 0x700120, HandleCollision 0x709e60, the sudden-collision smoothing), the per-pass
	OnCameraUpdate on the controller (0x6fa840: the arm placement and the crosshair, both unported), the rain box and its
	impacts, the FollowProjectile pass count, ApplyDebugCam_Native. The camera here keeps m_PassCount at 0.
-----------------------------------------------------------------------------*/

#include "fzsoireediag.h"

/**
 * The FOV from the settings when they carry one, the current FOVs started there, every influence group's PostBeginPlay,
 * six post-process target slots, the cached influence pointers (one per class, found through the groups), the aim
 * influence put to sleep, the collision size remembered, a teleport, and the settings republished.
 */
void ADishonoredPlayerCamera::PostBeginPlay()
{
	Super::PostBeginPlay();
	if( m_fDefaultFOVSettings > 0.0001f )
	{
		DefaultFOV = m_fDefaultFOVSettings;
	}
	m_fCurFOV = DefaultFOV;
	m_fCurFOV_Arms = DefaultFOV;
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& Groups = InfluenceGroups();
	for( INT Index = 0; Index < Groups.Num(); Index++ )
	{
		if( Groups(Index) )
		{
			Groups(Index)->PostBeginPlay( this );
		}
	}
	m_PostProcessTargets.AddZeroed( 6 );
	m_pPhysicalReact_Influence = (UDishonoredCamera_PhysicalReact*)GetCamera_Influence( UDishonoredCamera_PhysicalReact::StaticClass() );
	m_pHitReact_Influence = (UDishonoredCamera_HitReact*)GetCamera_Influence( UDishonoredCamera_HitReact::StaticClass() );
	m_pDisableArmFollow_Primary_Influence = (UDishonoredCamera_DisableArmFollow*)GetCamera_Influence( UDishonoredCamera_DisableArmFollow::StaticClass() );
	m_pDisableArmOffset_Primary_Influence = (UDishonoredCamera_DisableArmOffset*)GetCamera_Influence( UDishonoredCamera_DisableArmOffset::StaticClass() );
	m_pDisableArmFollow_Secondary_Influence = (UDishonoredCamera_DisableArmFollow*)GetCamera_Influence( UDisCamera_DisableArmFollow_Secondary::StaticClass() );
	m_pDisableArmOffset_Secondary_Influence = (UDishonoredCamera_DisableArmOffset*)GetCamera_Influence( UDisCamera_DisableArmOffset_Secondary::StaticClass() );
	m_pBumpSmoother_Influence = (UDishonoredCamera_BumpSmoother*)GetCamera_Influence( UDishonoredCamera_BumpSmoother::StaticClass() );
	m_pLean_Influence = (UDishonoredCamera_Lean*)GetCamera_Influence( UDishonoredCamera_Lean::StaticClass() );
	m_pLook_Influence = (UDisCamera_Look*)GetCamera_Influence( UDisCamera_Look::StaticClass() );
	m_pAim_Influence = (UDisCamera_Aim*)GetCamera_Influence( UDisCamera_Aim::StaticClass() );
	m_pAnimDrive_Influence = (UDishonoredCamera_AnimDriven*)GetCamera_Influence( UDishonoredCamera_AnimDriven::StaticClass() );
	m_pUnpossessDeath_Influence = (UDisCamera_UnpossessDeath*)GetCamera_Influence( UDisCamera_UnpossessDeath::StaticClass() );
	m_pPlayerControl_Influence = (UDishonoredCamera_PlayerControl*)GetCamera_Influence( UDishonoredCamera_PlayerControl::StaticClass() );
	m_pShake_Influence = (UDishonoredCamera_Shake*)GetCamera_Influence( UDishonoredCamera_Shake::StaticClass() );
	m_pRumble_Influence = (UDisCamera_Rumble*)GetCamera_Influence( UDisCamera_Rumble::StaticClass() );
	m_pRecoil_Influence = (UDishonoredCamera_Recoil*)GetCamera_Influence( UDishonoredCamera_Recoil::StaticClass() );
	m_pCrouchMantleOffset_Influence = (UDishonoredCamera_CrouchMantleOffset*)GetCamera_Influence( UDishonoredCamera_CrouchMantleOffset::StaticClass() );
	m_pStepUpMantleOffset_Influence = (UDisCamera_StepUpMantleOffset*)GetCamera_Influence( UDisCamera_StepUpMantleOffset::StaticClass() );
	m_pPossess_Influence = (UDisCamera_Possess*)GetCamera_Influence( UDisCamera_Possess::StaticClass() );
	if( m_pAim_Influence )
	{
		m_pAim_Influence->PutToSleep();
	}
	UCylinderComponent* Cylinder = Cast<UCylinderComponent>( CollisionComponent );
	if( Cylinder )
	{
		m_fCurCollisionRadius = Cylinder->CollisionRadius;
		m_fCurCollisionHeight = Cylinder->CollisionHeight;
	}
	OnTeleport( NULL );
	ArkSettings::ApplyCurrentSettings( this );

	INT NumInfluences = 0;
	for( INT Index = 0; Index < Groups.Num(); Index++ )
	{
		NumInfluences += Groups(Index) ? Groups(Index)->m_Influences.Num() : 0;
	}
	debugf( TEXT("DISHONORED(bringup): fz camera %s PostBeginPlay: %d influence groups, %d influences, PlayerControl %s, AnimDriven %s, DefaultFOV %.2f, style %s"),
		*GetName(), Groups.Num(), NumInfluences, m_pPlayerControl_Influence ? TEXT("yes") : TEXT("NONE"), m_pAnimDrive_Influence ? TEXT("yes") : TEXT("NONE"), DefaultFOV, *CameraStyle.ToString() );
}

// DISHONORED(port): 2012 0x70e6e0: the actor tick, the FOV tick; TickPostProcess (0x709ba0) is not ported
UBOOL ADishonoredPlayerCamera::Tick( FLOAT DeltaTime, ELevelTick TickType )
{
	const UBOOL bResult = Super::Tick( DeltaTime, TickType );
	TickFOV( DeltaTime );
	return bResult;
}

/**
 * The highest-priority FOV target with a value moves the current FOV toward it at its blend speed; none moves it back
 * to DefaultFOV at m_fDefaultFOVBlendSpeed. The arms' FOV follows the same target unless that target locks the arms.
 */
void ADishonoredPlayerCamera::TickFOV( FLOAT DeltaTime )
{
	FLOAT FOVTarget = DefaultFOV;
	FLOAT FOVTarget_Arms = DefaultFOV;
	FLOAT BlendSpeed = m_fDefaultFOVBlendSpeed;
	for( INT Index = m_FOVTargets.Num() - 1; Index >= 0; Index-- )
	{
		const FDisCamFOVTarget& Target = m_FOVTargets(Index);
		if( Target.m_fTarget > 0.0001f )
		{
			FOVTarget = Target.m_fTarget;
			BlendSpeed = Target.m_fBlendSpeed;
			if( !Target.m_bLockArms )
			{
				FOVTarget_Arms = Target.m_fTarget;
			}
			break;
		}
	}
	m_fCurFOV = DisInterpolateBySpeedWithDif( m_fCurFOV, FOVTarget, BlendSpeed, 0.1f, DeltaTime );
	m_fCurFOV_Arms = DisInterpolateBySpeedWithDif( m_fCurFOV_Arms, FOVTarget_Arms, BlendSpeed, 0.1f, DeltaTime );
}

// DISHONORED(port): 2012 0x709f40: the slot of the given priority (eDisCamFOVPriority indexes m_FOVTargets)
void ADishonoredPlayerCamera::SetFOVTarget( BYTE FOVPriority, FLOAT FOV, FLOAT BlendSpeed, UBOOL bLockArms )
{
	if( FOVPriority >= m_FOVTargets.Num() )
	{
		return;
	}
	FDisCamFOVTarget& Target = m_FOVTargets(FOVPriority);
	Target.m_fTarget = FOV;
	Target.m_bLockArms = bLockArms ? TRUE : FALSE;
	Target.m_fBlendSpeed = BlendSpeed;
}

// DISHONORED(port): 2012 0x709fb0
void ADishonoredPlayerCamera::ClearFOVTarget( BYTE FOVPriority )
{
	if( FOVPriority < m_FOVTargets.Num() )
	{
		m_FOVTargets(FOVPriority).m_fTarget = 0.f;
	}
}

// DISHONORED(port): 2012 0x709cf0: the first influence of exactly that class across the groups
UDishonoredCameraInfluence* ADishonoredPlayerCamera::GetCamera_Influence( UClass* InfluenceType ) const
{
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& Groups = const_cast<ADishonoredPlayerCamera*>( this )->InfluenceGroups();
	for( INT GroupIndex = 0; GroupIndex < Groups.Num(); GroupIndex++ )
	{
		UDishonoredCameraInfluenceGroup* Group = Groups(GroupIndex);
		if( !Group )
		{
			continue;
		}
		for( INT Index = 0; Index < Group->m_Influences.Num(); Index++ )
		{
			UDishonoredCameraInfluence* Influence = Group->m_Influences(Index);
			if( Influence && Influence->GetClass() == InfluenceType )
			{
				return Influence;
			}
		}
	}
	return NULL;
}

// DISHONORED(port): 2012 0x709de0: m_bTeleported, then every group's influences
void ADishonoredPlayerCamera::OnTeleport( USeqAct_Teleport* Action )
{
	m_bTeleported = TRUE;
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& Groups = InfluenceGroups();
	for( INT Index = 0; Index < Groups.Num(); Index++ )
	{
		if( Groups(Index) )
		{
			Groups(Index)->OnTeleport( Action );
		}
	}
}

void ADishonoredPlayerCamera::UpdateViewTarget( FTViewTarget& OutVT, FLOAT DeltaTime )
{
	static const FName NAME_FirstPersonStyle( TEXT("FirstPerson") );
	if( !PCOwner )
	{
		return;
	}
	APawn* OwnerPawn = PCOwner->Pawn;
	const UBOOL bTargetIsPawn = OutVT.Target && OutVT.Target == OwnerPawn;
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& Groups = InfluenceGroups();
	if( OwnerPawn && CameraStyle == NAME_FirstPersonStyle && bTargetIsPawn && Groups.Num() >= 3 )
	{
		OutVT.POV.FOV = m_fCurFOV;
		if( Groups(1) )
		{
			Groups(1)->m_Weight = m_BobAmount;
		}
		if( Groups(2) )
		{
			Groups(2)->m_Weight = m_fReactionWeight;
		}
		const FLOAT UncappedDeltaTime = DeltaTime;
		const FLOAT CappedDeltaTime = Min( DeltaTime, 0.06666f );
		FDishonoredViewTarget VT;
		VT.m_VT = OutVT;
		VT.m_DishonoredSettings = m_DishonoredVTSettings;
		for( INT Index = 0; Index < Groups.Num(); Index++ )
		{
			UDishonoredCameraInfluenceGroup* Group = Groups(Index);
			if( Group && Group->m_Weight > 1e-8f )
			{
				Group->GatherInfluences( VT, OwnerPawn, CappedDeltaTime, UncappedDeltaTime );
			}
		}
		m_DishonoredVTSettings = VT.m_DishonoredSettings;
		OutVT = VT.m_VT;
		m_bTeleported = FALSE;
		m_bSmoothingSuddenCollision = FALSE;
		m_CurCollisionStatus = 0;
		Rotation = m_DishonoredVTSettings.m_NonAdditive_Rot;
		if( GWorld )
		{
			GWorld->FarMoveActor( this, m_DishonoredVTSettings.m_NonAdditive_Pos, FALSE, TRUE, FALSE );
		}
		m_PassCount = 0;
		DisFZReportCamera( this, OutVT, OwnerPawn, DeltaTime );
		return;
	}
	Super::UpdateViewTarget( OutVT, DeltaTime );
	DisFZReportCamera( this, OutVT, OwnerPawn, DeltaTime );
}
