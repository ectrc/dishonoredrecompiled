// DishonoredGame/src/disprojectile.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (49):
//   0x8873f0  public: static void __cdecl ADisProjectile::InitializePrivateStaticClassADisProjectile(void)
//   0x887410  public: static void __cdecl UDisTweaks_Projectile::InitializePrivateStaticClassUDisTweaks_Projectile(void)
//   0x887430  public: class AActor const * __thiscall ADisProjectile::GetSourceActor(void)const
//   0x887440  public: virtual unsigned int __thiscall ADisProjectile::Tick(float, enum ELevelTick)
//   0x887490  public: virtual unsigned int __thiscall ADisProjectile::TraceForHit(struct FDisLineProbeResult &, class FVector const &, class FVector const &, class FVector const &, struct TMemStackArray<class AActor const *> &)const
//   0x8874e0  protected: virtual void __thiscall ADisProjectile::PostScriptDestroyed(void)
//   0x887520  protected: virtual void __thiscall ADisProjectile::ClearComponents(void)
//   0x887560  public: void __thiscall ADisProjectile::HomeToTarget(class AActor *, float, float, float, class FVector const *)
//   0x8875e0  public: void __thiscall ADisProjectile::HomeToPosition(class FVector const &, float, float)
//   0x887630  public: virtual class ADishonoredAudioVolume * __thiscall ADisProjectile::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x887650  public: virtual unsigned int __thiscall FDataBaseConnection::Execute(wchar_t const *, class FDataBaseRecordSet * &)
//   0x887670  public: void __thiscall ADisProjectile::DestroyProjectile(void)
//   0x887690  private: virtual class FVector __thiscall ADisProjectile::GetDamageSourceLocation(void)const
//   0x8876b0  public: virtual void __thiscall ADisProjectile::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x8877a0  public: virtual void __thiscall ADisProjectile::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x88a5b0  void __cdecl DisProjectile::FindProjectileTargetPos(enum EDisComputeHitAccuracy, enum EDisComputeHitTargetRegion, class AActor const *, class FVector &, unsigned int &, class FVector const * const)
//   0x88a720  public: unsigned int __thiscall UDisTweaks_Projectile::ComputeDirectionForShot(enum EDisComputeHitAccuracy, enum EDisComputeHitTargetRegion, class FVector const &, class AActor const *, float, float, class FVector const * const, class FVector &)const
//   0x88a8b0  public: unsigned int __thiscall UDisTweaks_Projectile::ComputeDirectionForShot_WithViewOffset(enum EDisComputeHitAccuracy, enum EDisComputeHitTargetRegion, class FVector const &, float, class AActor const *, float, float, class FVector const * const, class FVector &, class FVector *)const
//   0x88abb0  protected: virtual void __thiscall ADisProjectile_Arrow_Explosive::SetTweaks_Derived(class UDisTweaksBase *)
//   0x88abc0  public: virtual void __thiscall ADisProjectile::OnHitBody(struct FDisLineProbeResult &, class UClass *, class UClass const *, unsigned int)
//   0x88abe0  public: virtual void __thiscall ADisProjectile::OnHitActor(struct FDisLineProbeResult &, class UClass *, class UClass const *)
//   0x88ac00  public: void __thiscall ADisProjectile::OnPawnShutDown(class ADishonoredPawn const &)
//   0x88ac30  public: virtual class ADishonoredPawn * __thiscall ADisProjectile::GetNoiseMakerPawn_Derived(void)const
//   0x88ac40  public: virtual void __thiscall ADisProjectile::PostGameLoad(enum ESaveLoadLocation)
//   0x88e6c0  public: virtual void __thiscall ADisProjectile::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8927e0  private: void __thiscall ADisProjectile::AdjustLocation(class FVector const &)
//   0x895920  void __cdecl DisProjectile::AddAttachmentsToArray(class ADishonoredPawn * const, struct TMemStackArray<class AActor const *> &)
//   0x895a30  protected: virtual void __thiscall UDisTweaks_Projectile::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x8996c0  public: static class UClass * __cdecl ADisProjectile::GetPrivateStaticClassADisProjectile(wchar_t const *)
//   0x89c310  public: static class UClass * __cdecl ADisProjectile::StaticClassNoInline(void)
//   0x89e2c0  public: static class UClass * __cdecl UDisTweaks_Projectile::GetPrivateStaticClassUDisTweaks_Projectile(wchar_t const *)
//   0x89e350  public: virtual unsigned int __thiscall ADisProjectile::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x89f5d0  public: static class UClass * __cdecl UDisTweaks_Projectile::StaticClassNoInline(void)
//   0x8a27e0  public: void __thiscall ADisProjectile::SetSourceActor(class AActor *)
//   0x8a28f0  private: void __thiscall ADisProjectile::CommonInit(void)
//   0x8a2990  protected: void __thiscall ADisProjectile::GetProjectileExtents(class FVector &, class FVector &)const
//   0x8a2c30  protected: void __thiscall ADisProjectile::UpdateHoming(float)
//   0x8a3010  public: virtual float __thiscall ADisProjectile::CalculateProjectileDamage(struct FDisLineProbeResult const &, class UClass const *, class UClass * &, class FVector, class FVector, class FVector)const
//   0x8a3130  protected: void __thiscall ADisProjectile::InitWindblast_PushToTarget(class FVector const &, float, class AActor const *)
//   0x8a3350  protected: void __thiscall ADisProjectile::InitWindblast_Homing(struct AActor::WindBlastParams const &)
//   0x8a3780  protected: void __thiscall ADisProjectile::PlayPickupSound(unsigned int, unsigned int)
//   0x8a3830  private: unsigned int __thiscall ADisProjectile::HandleProjectileHit(class FVector, class FVector, class FVector const &, struct FDisLineProbeResult &, struct TMemStackArray<class AActor const *> &, int)
//   0x8a3f00  private: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisProjectile::GetInteractableTweaks_Derived(void)const
//   0x8a3f30  public: virtual void __thiscall ADisProjectile::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8a40f0  public: unsigned int __thiscall ADisProjectile::TraceForCameraFollow(float)const
//   0x8a6400  public: virtual void __thiscall ADisProjectile::InitProjectile(float, class FVector const &, class FVector const &, unsigned int, int, float)
//   0x8a6a00  public: virtual void __thiscall ADisProjectile::physProjectile(float, int)
//   0x8a6f70  public: virtual unsigned int __thiscall ADisProjectile::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x8a9430  protected: virtual unsigned int __thiscall UDisTweaks_Projectile::FixupDefaults_Derived(void)
