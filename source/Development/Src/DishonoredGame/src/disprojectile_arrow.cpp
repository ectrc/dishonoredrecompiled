// DishonoredGame/src/disprojectile_arrow.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (46):
//   0x8877d0  private: void __thiscall ADisProjectile_Arrow::OnWorldStopBendTime(class FArkGameEvent const &)
//   0x8877f0  protected: virtual unsigned int __thiscall ADisProjectile_Arrow::AttachToActor(struct FDisLineProbeResult &, class AActor * const)
//   0x887810  public: virtual unsigned int __thiscall ADisProjectile_Arrow::IgnoreBlockingBy(class AActor const *)const
//   0x887840  public: virtual void __thiscall ADisProjectile_Arrow::HideHighlight(void)
//   0x887870  protected: virtual unsigned int __thiscall ADisProjectile_Arrow::IsAllowedToPassThrough(void)const
//   0x887930  public: virtual void __thiscall ADisProjectile_GrenadeBase::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x887950  public: virtual void __thiscall ADisProjectile_Arrow::DisableSoulRendering(void)
//   0x88ac70  public: virtual void __thiscall ADisProjectile_Arrow::ShowHighlight(class UMaterialInterface *)
//   0x88e880  protected: virtual unsigned int __thiscall ADisProjectile_Arrow::CanStickIntoBody(class AActor const &, enum eDisHitRegion)const
//   0x88e8e0  public: virtual unsigned int __thiscall ADisProjectile_Arrow::Tick(float, enum ELevelTick)
//   0x88e980  private: void __thiscall ADisProjectile_Arrow::MoveLimb(class FVector const &, class FRotator const &)
//   0x88f1c0  private: void __thiscall ADisProjectile_Arrow::StopLimb(struct FDisLineProbeResult const &)
//   0x88f260  private: void __thiscall ADisProjectile_Arrow::ReleaseLimb(void)
//   0x88f410  public: virtual void __thiscall ADisProjectile_Arrow::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8928f0  public: virtual void __thiscall ADisProjectile_Arrow::MoveProjectileChildren(class FVector const &, unsigned int)
//   0x892930  private: void __thiscall ADisProjectile_Arrow::GrabLimb(class ADisMovableLimb *)
//   0x8941f0  public: void __thiscall ADisProjectile_Arrow::OnLimbSevered(class ADisMovableLimb *)
//   0x895a90  public: virtual void __thiscall UDisTweaks_Arrow::GetGenericThumbnailPreviewAsset(struct TMemStackArray<struct FDisTweaksGenericThumbnailItem> &)const
//   0x895ad0  private: virtual unsigned int __thiscall ADisProjectile_Arrow::TraceForHit(struct FDisLineProbeResult &, class FVector const &, class FVector const &, class FVector const &, struct TMemStackArray<class AActor const *> &)const
//   0x899750  public: static class UClass * __cdecl ADisProjectile_Arrow::GetPrivateStaticClassADisProjectile_Arrow(wchar_t const *)
//   0x89c340  public: static void __cdecl ADisProjectile_Arrow::InitializePrivateStaticClassADisProjectile_Arrow(void)
//   0x89e3a0  public: static class UClass * __cdecl ADisProjectile_Arrow::StaticClassNoInline(void)
//   0x89e3d0  public: static class UClass * __cdecl UDisTweaks_Arrow::GetPrivateStaticClassUDisTweaks_Arrow(wchar_t const *)
//   0x89f600  public: static void __cdecl UDisTweaks_Arrow::InitializePrivateStaticClassUDisTweaks_Arrow(void)
//   0x8a03f0  public: static class UClass * __cdecl UDisTweaks_Arrow::StaticClassNoInline(void)
//   0x8a42f0  public: virtual unsigned int __thiscall ADisBullet::AttemptCannotUseInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x8a43a0  public: virtual void __thiscall ADisProjectile_Arrow::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8a7080  protected: virtual void __thiscall ADisProjectile_Arrow::ApplyTweakChanges_Derived(void)
//   0x8a7130  private: void __thiscall ADisProjectile_Arrow::HandleTimeBent(void)
//   0x8a7230  private: void __thiscall ADisProjectile_Arrow::OnWorldStartBendTime(class FArkGameEvent const &)
//   0x8a7240  protected: virtual void __thiscall ADisProjectile_Arrow::HaltArrow(unsigned int, struct FDisLineProbeResult const &)
//   0x8a7410  public: virtual void __thiscall ADisProjectile_Arrow::OnHitBody(struct FDisLineProbeResult &, class UClass *, class UClass const *, unsigned int)
//   0x8a76c0  protected: virtual unsigned int __thiscall ADisProjectile_Arrow::ShouldBreak(void)const
//   0x8a7770  public: virtual void __thiscall ADisProjectile_Arrow::OnHitActor(struct FDisLineProbeResult &, class UClass *, class UClass const *)
//   0x8a78e0  public: virtual float __thiscall ADisProjectile_Arrow::CalculateProjectileDamage(struct FDisLineProbeResult const &, class UClass const *, class UClass * &, class FVector, class FVector, class FVector)const
//   0x8a7aa0  public: virtual void __thiscall ADisProjectile_Arrow::BaseChange(void)
//   0x8a7bb0  protected: unsigned int __thiscall ADisProjectile_Arrow::CanBePickedUp(class ADishonoredPawn const *)const
//   0x8a7c30  private: unsigned int __thiscall ADisProjectile_Arrow::CanInteract_Internal(void)const
//   0x8a7d10  public: virtual enum eCrossHairStatus __thiscall ADisProjectile_Arrow::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x8a7d50  public: virtual unsigned int __thiscall ADisProjectile_Arrow::CanInteract(struct FCanInteractParams const &)const
//   0x8a7d60  public: virtual unsigned int __thiscall ADisProjectile_Arrow::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x8a7e30  private: void __thiscall ADisProjectile_Arrow::UnregisterEvents(void)
//   0x8a7e80  public: virtual void __thiscall ADisProjectile_Arrow::BeginDestroy(void)
//   0x8a7e90  public: virtual void __thiscall ADisProjectile_Arrow::PostScriptDestroyed(void)
//   0x8a7f00  public: virtual void __thiscall ADisProjectile_Arrow::PostGameLoad(enum ESaveLoadLocation)
//   0x8a9460  public: virtual void __thiscall ADisProjectile_Arrow::InitProjectile(float, class FVector const &, class FVector const &, unsigned int, int, float)
