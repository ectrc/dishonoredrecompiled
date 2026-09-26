// DishonoredGame/src/disskeletalbreakable.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (36):
//   0x669c90  public: static void __cdecl ADisSkeletalBreakable::InitializePrivateStaticClassADisSkeletalBreakable(void)
//   0x669cb0  public: static void __cdecl UDisTweaks_SkeletalBreakable::InitializePrivateStaticClassUDisTweaks_SkeletalBreakable(void)
//   0x669cd0  public: virtual void __thiscall UDisTweaks_SkeletalBreakable::Serialize(class FArchive &)
//   0x669d00  public: virtual void __thiscall ADisSkeletalBreakable::PostLoad(void)
//   0x669d20  public: virtual void __thiscall ADisSkeletalBreakable::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x669e10  protected: virtual void __thiscall ADisSkeletalBreakable::DestroySkeletalBreakable(void)
//   0x669e30  public: virtual void __thiscall ADisSkeletalBreakable::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x669ec0  public: virtual class ADishonoredPawn * __thiscall ADisSkeletalBreakable::GetNoiseMakerPawn_Derived(void)const
//   0x669ed0  private: void __thiscall ADisSkeletalBreakable::InitFullRagdoll(void)
//   0x66bf70  public: virtual void __thiscall ADisSkeletalBreakable::PostConstructed(void)
//   0x66bf80  public: virtual unsigned int __thiscall ADisSkeletalBreakable::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x66c120  public: virtual unsigned int __thiscall ADisSkeletalBreakable::ArkIsDeadOrDestroyed(void)const
//   0x66fcd0  public: virtual void __thiscall UDisSkeletalBreakSteps::FixAINoise(void)
//   0x6745e0  protected: virtual void __thiscall UDisTweaks_SkeletalBreakable::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x678470  public: static class UClass * __cdecl ADisSkeletalBreakable::GetPrivateStaticClassADisSkeletalBreakable(wchar_t const *)
//   0x67a220  public: static class UClass * __cdecl ADisSkeletalBreakable::StaticClassNoInline(void)
//   0x67aa40  public: static class UClass * __cdecl UDisSkeletalBreakStepsInterface::GetPrivateStaticClassUDisSkeletalBreakStepsInterface(wchar_t const *)
//   0x67aad0  public: static class UClass * __cdecl UDisTweaks_SkeletalBreakable::GetPrivateStaticClassUDisTweaks_SkeletalBreakable(wchar_t const *)
//   0x67bbc0  public: static class UClass * __cdecl UDisSkeletalBreakSteps::GetPrivateStaticClassUDisSkeletalBreakSteps(wchar_t const *)
//   0x67bc50  public: static class UClass * __cdecl UDisTweaks_SkeletalBreakable::StaticClassNoInline(void)
//   0x67c440  public: static void __cdecl UDisSkeletalBreakStepsInterface::InitializePrivateStaticClassUDisSkeletalBreakStepsInterface(void)
//   0x67d310  public: static class UClass * __cdecl UDisSkeletalBreakStepsInterface::StaticClassNoInline(void)
//   0x67d340  public: static void __cdecl UDisSkeletalBreakSteps::InitializePrivateStaticClassUDisSkeletalBreakSteps(void)
//   0x67d360  protected: virtual void __thiscall ADisSkeletalBreakable::ApplyTweakChanges_Derived(void)
//   0x67d430  public: virtual void __thiscall ADisSkeletalBreakable::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x67d540  public: virtual void __thiscall ADisSkeletalBreakable::OnTakeHit(class FVector const &, class FVector const &, class AActor *, class UClass const *)
//   0x67d610  public: virtual unsigned int __thiscall ADisSkeletalBreakable::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x67d6e0  private: unsigned int __thiscall ADisSkeletalBreakable::SwitchToBrokenMesh(void)
//   0x67d7c0  public: unsigned int __thiscall ADisSkeletalBreakable::ShouldAttractNPC(void)const
//   0x67d810  protected: virtual unsigned int __thiscall ADisSkeletalBreakable::DoesActorCauseHitRebound(int, class UClass *)const
//   0x681760  public: static class UClass * __cdecl UDisSkeletalBreakSteps::StaticClassNoInline(void)
//   0x681790  protected: unsigned int __thiscall ADisSkeletalBreakable::UpdateMesh(int, class FVector const &, class FVector const &, class USkeletalMeshComponent *)
//   0x6819e0  public: virtual void __thiscall ADisSkeletalBreakable::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x684bc0  protected: virtual unsigned int __thiscall UDisTweaks_SkeletalBreakable::FixupDefaults_Derived(void)
//   0x684bf0  public: void __thiscall ADisSkeletalBreakable::OnBrokenTransferSkeletalMesh(int, int, class FVector const &, class FVector const &, class AActor *, class USkeletalMeshComponent *)
//   0x689c90  public: virtual void __thiscall ADisSkeletalBreakable::OnBroken(int, int, class FVector const &, class FVector const &, class AActor *, class UClass const *)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x618c30 (2012 0x669cd0): licensee versions below 24 upgrade m_pBreakSteps through a
// UDisSkeletalBreakStepsInterface virtual (2013 slot +328). The retail cooked content is licensee 30 (UnObjVer.cpp), so the
// branch is never taken; the step-interface virtual is not ported.
void UDisTweaks_SkeletalBreakable::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( Ar.LicenseeVer() < 24 )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): %s: licensee version %d break-step upgrade not ported"), *GetPathName(), Ar.LicenseeVer() );
	}
}
