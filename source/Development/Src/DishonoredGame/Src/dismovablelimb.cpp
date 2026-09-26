// DishonoredGame/src/dismovablelimb.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (57):
//   0x691070  public: static void __cdecl ADisMovableLimb::InitializePrivateStaticClassADisMovableLimb(void)
//   0x691090  public: static void __cdecl UDisTweaks_MovableLimb::InitializePrivateStaticClassUDisTweaks_MovableLimb(void)
//   0x6910b0  public: static void __cdecl UDisContactType_Limb::InitializePrivateStaticClassUDisContactType_Limb(void)
//   0x6910d0  public: virtual void __thiscall ADisMovableLimb::Serialize(class FArchive &)
//   0x691100  public: void __thiscall ADisMovableLimb::SetFormerOwningPawn(class ADishonoredNPCPawn *)
//   0x691140  private: virtual void __thiscall ADisMovableLimb::PreBeginPlay(void)
//   0x691160  private: virtual void __thiscall ADisMovableLimb::OnActorTerminated(void)
//   0x691190  public: virtual class UPrimitiveComponent * __thiscall ADisWhiskeyBottle::GetMovablePrimitiveComponent(void)const
//   0x6911a0  private: virtual enum EMovableWeightClass __thiscall ADisMovableLimb::GetMovableWeightClass(void)const
//   0x6911b0  private: virtual unsigned int __thiscall ADisMovableLimb::WasJustThrownBy(class ADishonoredPawn const *)const
//   0x6911e0  public: virtual unsigned int __thiscall ADisWhiskeyBottle::CanInteract(struct FCanInteractParams const &)const
//   0x691200  private: virtual class USkeletalMeshComponent * __thiscall ADisMovableLimb::GetRatTargetMesh(void)
//   0x691210  private: virtual class FVector const & __thiscall ADisMovableLimb::GetRatTargetLocation(void)const
//   0x691220  private: virtual struct FDisRatTargetContext & __thiscall ADisMovableLimb::GetRatTargetContext(void)
//   0x691230  private: virtual void __thiscall ADisMovableLimb::OnCorpseConsumed(float)
//   0x691260  private: virtual unsigned char __thiscall ADisMovableLimb::GetRatTargetPriority(void)const
//   0x691270  private: virtual void __thiscall ADisMovableLimb::PostScriptDestroyed(void)
//   0x6912b0  private: virtual void __thiscall ADisMovableLimb::TakeDamage_Impl(int, class AController * const, class FVector const &, class FVector const &, class UClass * const, struct FTraceHitInfo const &, class AActor * const)
//   0x691350  public: void __thiscall ADisMovableLimb::SetupCollision(void)
//   0x6913b0  private: virtual class FVector __thiscall ADisMovableLimb::GetAttnTargetExtent(void)const
//   0x6913d0  private: virtual enum ERelationship __thiscall ADisMovableLimb::GetAttnTargetIncomingRelationship(class IDisRelationshipInterface const *)const
//   0x691400  private: virtual class IDisRelationshipInterface * __thiscall ADisMovableLimb::GetRelationshipInterface(void)
//   0x691420  private: virtual class UDisTweaks_Faction * __thiscall ADisMovableLimb::GetFactionTweak(void)const
//   0x691430  private: virtual struct FDisRelationshipOverrideInfo * __thiscall ADisMovableLimb::GetRelationshipOverrideInfo(void)
//   0x691440  private: virtual void __thiscall ADisMovableLimb::OnOtherActorTerminated(class AActor const &)
//   0x691480  public: virtual void __thiscall ADisMovableLimb::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x6914a0  public: virtual void __thiscall ADisMovableLimb::DisableSoulRendering(void)
//   0x6914b0  private: virtual void __thiscall ADisMovableLimb::OnEnterStealthVolume(class ADisStealthVolume *)
//   0x6914c0  private: virtual class FBox __thiscall ADisMovableLimb::GetBoundsForStealthVolumes(void)const
//   0x6914f0  private: virtual struct FDisVisSettingsUserCache & __thiscall ADisMovableLimb::GetVisSettingsUserCache(void)
//   0x693d90  public: virtual void __thiscall ADisMovableLimb::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x693db0  private: virtual void __thiscall ADisMovableLimb::PostBeginPlay(void)
//   0x693e30  private: virtual unsigned int __thiscall ADisMovableLimb::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x693eb0  public: virtual unsigned int __thiscall ADisMovableLimb::CanSplash(void)
//   0x693ed0  private: virtual struct FBoxSphereBounds __thiscall ADisMovableLimb::GetMovableBodyBounds(void)const
//   0x693f30  private: virtual class FQuat __thiscall ADisMovableLimb::GetMovableOrientation(void)
//   0x693f60  private: virtual void __thiscall ADisMovableLimb::OnThrow(class FVector const &, class FVector const &)
//   0x693fb0  private: virtual void __thiscall ADisMovableLimb::SetTweaks_Derived(class UDisTweaksBase *)
//   0x693fc0  private: virtual class FVector __thiscall ADisMovableLimb::GetAttnTargetLocation(void)const
//   0x693ff0  private: virtual class FVector __thiscall ADisMovableLimb::GetAttnTargetVelocity(void)const
//   0x697030  private: virtual void __thiscall ADisMovableLimb::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x69e740  public: static class UClass * __cdecl UDisContactType_Limb::GetPrivateStaticClassUDisContactType_Limb(wchar_t const *)
//   0x69e7d0  private: virtual class URB_BodyInstance * __thiscall ADisMovableLimb::OnMovableGrab_Start(class ADishonoredPawn *)
//   0x69e870  private: virtual void __thiscall ADisMovableLimb::OnMovableGrab_End(class ADishonoredPawn *)
//   0x69e8a0  private: virtual void __thiscall ADisMovableLimb::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x69efe0  private: virtual void __thiscall ADisMovableLimb::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x69f740  private: virtual class FDisComponentObservable const * __thiscall ADisMovableLimb::GetObservableComponent(void)const
//   0x6a0800  public: static class UClass * __cdecl UDisContactType_Limb::StaticClassNoInline(void)
//   0x6a0830  private: virtual unsigned int __thiscall ADisMovableLimb::Tick(float, enum ELevelTick)
//   0x6a08f0  private: virtual void __thiscall ADisMovableLimb::CollectAttnTargetParams(struct FDisAttnTargetParams &)
//   0x6a49b0  private: virtual void __thiscall ADisMovableLimb::OnMovableGrab_Complete(class ADishonoredPawn *)
//   0x6a7f00  public: static class UClass * __cdecl UDisTweaks_MovableLimb::GetPrivateStaticClassUDisTweaks_MovableLimb(wchar_t const *)
//   0x6a84d0  public: static class UClass * __cdecl UDisTweaks_MovableLimb::StaticClassNoInline(void)
//   0x6aa050  private: virtual class AActor * __thiscall ADisMovableLimb::GetRatTargetActor(void)
//   0x6aa330  private: virtual unsigned int __thiscall ADisMovableLimb::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x6ac700  public: static class UClass * __cdecl ADisMovableLimb::GetPrivateStaticClassADisMovableLimb(wchar_t const *)
//   0x6ad8a0  public: static class UClass * __cdecl ADisMovableLimb::StaticClassNoInline(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x642c30 (2012 0x6910d0, same shape): AActor::Serialize, then the relationship overrides
void ADisMovableLimb::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	DisSerializeRelationshipOverrideInfo( m_RelationshipOverrideInfo, Ar );
}
