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
