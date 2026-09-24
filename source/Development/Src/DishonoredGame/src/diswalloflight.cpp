// DishonoredGame/src/diswalloflight.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (64):
//   0x66a2b0  public: static void __cdecl ADisWallOfLight::InitializePrivateStaticClassADisWallOfLight(void)
//   0x66a2d0  public: static void __cdecl UDisTweaks_WallOfLight::InitializePrivateStaticClassUDisTweaks_WallOfLight(void)
//   0x66a2f0  public: static void __cdecl UDisSeqEvent_WallOfLight::InitializePrivateStaticClassUDisSeqEvent_WallOfLight(void)
//   0x66a310  public: static void __cdecl UDisSeqAct_WallofLightControl::InitializePrivateStaticClassUDisSeqAct_WallofLightControl(void)
//   0x66a330  public: static void __cdecl UDisDamageType_Energy::InitializePrivateStaticClassUDisDamageType_Energy(void)
//   0x66a350  public: virtual class FGuid * __thiscall ADisWallOfLight::GetGuid(void)
//   0x66a360  public: virtual void __thiscall ADisWallOfLight::PostLoad(void)
//   0x66a380  public: virtual void __thiscall ADisWallOfLight::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66a3e0  public: virtual class GFxStateBag * __thiscall GFxFontManagerStates::GetStateBagImpl(void)const
//   0x66a3f0  public: float __thiscall ADisWallOfLight::GetActivationTimer(void)const
//   0x66a400  public: unsigned int __thiscall ADisWallOfLight::IsReversed(void)const
//   0x66a410  public: virtual void __thiscall ADisWallOfLight::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x66a440  public: virtual void __thiscall ADisWallOfLight::DisableSoulRendering(void)
//   0x66c480  public: virtual void __thiscall ADisWallOfLight::PostConstructed(void)
//   0x670030  public: static class UClass * __cdecl UDisDamageType_Energy::GetPrivateStaticClassUDisDamageType_Energy(wchar_t const *)
//   0x6700c0  public: static class UClass * __cdecl UDisDamageType_WallOfLight::GetPrivateStaticClassUDisDamageType_WallOfLight(wchar_t const *)
//   0x670150  public: virtual unsigned int __thiscall ADisWallOfLight::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x672d80  public: static class UClass * __cdecl UDisDamageType_Energy::StaticClassNoInline(void)
//   0x672db0  public: static void __cdecl UDisDamageType_WallOfLight::InitializePrivateStaticClassUDisDamageType_WallOfLight(void)
//   0x672dd0  protected: void __thiscall ADisWallOfLight::SetParticleSystemTarget(class UParticleSystemComponent *, class FVector const &, unsigned int)
//   0x672ed0  private: unsigned int __thiscall ADisWallOfLight::ComputeShape(class TArray<class FVector, class FDefaultAllocator> &)const
//   0x672ff0  public: virtual void __thiscall ADisWallOfLight::CreateEdgesForPathObject(class APylon *)
//   0x674060  public: static class UClass * __cdecl UDisDamageType_WallOfLight::StaticClassNoInline(void)
//   0x6746f0  public: virtual void __thiscall UDisTweaks_WallOfLight::GetGenericThumbnailPreviewAsset(struct TMemStackArray<struct FDisTweaksGenericThumbnailItem> &)const
//   0x6786b0  public: static class UClass * __cdecl ADisWallOfLight::GetPrivateStaticClassADisWallOfLight(wchar_t const *)
//   0x678740  public: virtual void __thiscall ADisWallOfLight::DisGetMeshSplittingPolys(class TArray<struct UNavigationMeshBase::FMeshSplitingShape, class FDefaultAllocator> &)
//   0x67a2d0  public: static class UClass * __cdecl ADisWallOfLight::StaticClassNoInline(void)
//   0x67acb0  public: static class UClass * __cdecl UDisTweaks_WallOfLight::GetPrivateStaticClassUDisTweaks_WallOfLight(wchar_t const *)
//   0x67bce0  public: static class UClass * __cdecl UDisTweaks_WallOfLight::StaticClassNoInline(void)
//   0x67c460  public: static class UClass * __cdecl UDisSeqEvent_WallOfLight::GetPrivateStaticClassUDisSeqEvent_WallOfLight(wchar_t const *)
//   0x67c4f0  public: static class UClass * __cdecl UDisSeqAct_WallofLightControl::GetPrivateStaticClassUDisSeqAct_WallofLightControl(wchar_t const *)
//   0x67dc20  public: static class UClass * __cdecl UDisSeqEvent_WallOfLight::StaticClassNoInline(void)
//   0x67dc50  public: static class UClass * __cdecl UDisSeqAct_WallofLightControl::StaticClassNoInline(void)
//   0x67dc80  protected: unsigned int __thiscall ADisWallOfLight::ZapPawn(class APawn *, class FVector const &, class FVector const &, class AActor *)
//   0x67e010  protected: unsigned int __thiscall ADisWallOfLight::HasWhaleOil(void)const
//   0x67e070  public: virtual void __thiscall ADisWallOfLight::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x67e220  protected: virtual void __thiscall ADisWallOfLight::ApplyTweakChanges_Derived(void)
//   0x67e280  public: unsigned int __thiscall ADisWallOfLight::CheckPawnFactionClearance(class ADishonoredPawn *)const
//   0x67e350  public: unsigned int __thiscall ADisWallOfLight::IsWoLPassageAllowedFor(class ADishonoredNPCPawn const *)const
//   0x67e490  protected: unsigned int __thiscall ADisWallOfLight::HasBeenTamperedWith(void)const
//   0x67e500  protected: void __thiscall ADisWallOfLight::OnSeenByWhileTampered(class FArkGameEvent const &)
//   0x67e5a0  public: virtual unsigned int __thiscall ADisWallOfLight::HasSoul(int)const
//   0x681b70  public: virtual void __thiscall ADisWallOfLight::PostBeginPlay(void)
//   0x681d00  public: virtual void __thiscall ADisWallOfLight::BeginDestroy(void)
//   0x681d40  protected: void __thiscall ADisWallOfLight::SetBeamTarget(int, class FVector const &, unsigned int)
//   0x681e20  protected: void __thiscall ADisWallOfLight::DeactivateKillEffect(void)
//   0x681fc0  public: void __thiscall ADisWallOfLight::ReversePolarity(unsigned int)
//   0x682080  public: virtual int __thiscall ADisWallOfLight::CostFor(struct FNavMeshPathParams const &, class FVector const &, class FVector &, struct FNavMeshPathObjectEdge *, struct FNavMeshPolyBase *)
//   0x6820f0  protected: void __thiscall ADisWallOfLight::CheckTamper(void)
//   0x685620  protected: unsigned int __thiscall ADisWallOfLight::ActivateKillEffect(class FVector const &, class AActor *, unsigned int)
//   0x6858e0  protected: void __thiscall ADisWallOfLight::Activate(void)
//   0x685a90  protected: void __thiscall ADisWallOfLight::Deactivate(void)
//   0x685c40  public: virtual void __thiscall ADisWallOfLight::OnBatteryUnplugged(void)
//   0x685c60  public: void __thiscall ADisWallOfLight::SetEnabled(unsigned int)
//   0x685cd0  private: void __thiscall UDisSeqAct_WallofLightControl::DoWallofLightAction(class ADisWallOfLight *, enum EDisWallofLightControlKismetAction)
//   0x689cc0  public: virtual void __thiscall ADisWallOfLight::PreBeginPlay(void)
//   0x689cf0  public: virtual void __thiscall ADisWallOfLight::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x689f50  public: void __thiscall ADisWallOfLight::HandleCrowdAgentCollision(class ADisGameCrowdAgentSkeletalRat *, class FVector const &, class FVector const &)
//   0x68a0e0  protected: unsigned int __thiscall ADisWallOfLight::ZapActor(class AActor *, class FVector const &)
//   0x68a370  public: virtual void __thiscall ADisWallOfLight::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x68a3b0  public: virtual unsigned int __thiscall ADisWallOfLight::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x68a4c0  public: virtual void __thiscall ADisWallOfLight::OnBatteryPlugged(void)
//   0x68a4e0  public: virtual unsigned int __thiscall ADisWallOfLight::Tick(float, enum ELevelTick)
//   0x68ad60  public: virtual void __thiscall UDisSeqAct_WallofLightControl::Activated(void)
