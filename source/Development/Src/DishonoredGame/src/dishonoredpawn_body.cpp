// DishonoredGame/src/dishonoredpawn_body.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (49):
//   0x78e900  public: static void __cdecl UDisSkeletalMeshComponent::InitializePrivateStaticClassUDisSkeletalMeshComponent(void)
//   0x78e920  public: void __thiscall ADishonoredPawn::LockMeshTranslation(unsigned int, enum ADishonoredPawn::eDisMeshOffsetDisableFlag)
//   0x78e960  public: void __thiscall ADishonoredPawn::SetMeshTranslation(class FVector const &, float)
//   0x78e9c0  public: void __thiscall ADishonoredPawn::ImpulsePawn(class FVector const &)
//   0x78ea60  public: virtual class FVector __thiscall ADishonoredPawn::GetCameraPos(void)const
//   0x78ea90  public: class FVector __thiscall ADishonoredPawn::GetPredictedLocation(float)const
//   0x7911f0  protected: virtual void __thiscall UDisTweaks_Pawn_Body::ApplyFallbackChain_Derived(void)
//   0x791280  public: unsigned int __thiscall UDisSkeletalMeshComponent::DisLegLineCheck(class FVector const &, class FVector const &, class FVector &, class FVector &, class FVector const &, struct FCheckResult &)
//   0x791380  public: virtual void __thiscall ADishonoredPawn::SetBase(class AActor *, class FVector, int, class USkeletalMeshComponent *, class FName)
//   0x791470  protected: void __thiscall ADishonoredPawn::Tick_Body_CachedPositions(float)
//   0x791600  public: virtual unsigned int __thiscall ADishonoredPawn::ResolveAttachedMoveEncroachment(class AActor *, struct FCheckResult const &)
//   0x791870  public: virtual class UClass * __thiscall ADishonoredPawn::GetContactTypeOverride(void)const
//   0x7918e0  public: virtual int __thiscall ADishonoredPawn::TakeFallingDamage_Native(class FVector, class AActor *)
//   0x794d20  public: void __thiscall FDisRegionCached::Set(struct FDisRegionInfo const *, class USkeletalMeshComponent *)
//   0x794dd0  public: virtual unsigned int __thiscall UDisSkeletalMeshComponent::LegLineCheck(class FVector const &, class FVector const &, class FVector &, class FVector &, class FVector const &)
//   0x794e50  public: void __thiscall UDisSkeletalMeshComponent::UpdateChildForTickEnd(void)
//   0x794f10  public: void __thiscall FDisFootCache::Init(struct FDisFootInfo const &, class USkeletalMeshComponent const *)
//   0x794fd0  public: void __thiscall ADishonoredPawn::ApplyTweakChanges_Body(void)
//   0x795080  public: void __thiscall ADishonoredPawn::PerformAngleOffsetFixup(class FMatrix &, unsigned int)const
//   0x795630  public: void __thiscall ADishonoredPawn::GetBone_ByName(class FName, class FMatrix *)const
//   0x7956a0  public: void __thiscall ADishonoredPawn::GetBone_ByName(class FName, class FVector *, class FRotator *)const
//   0x7957d0  protected: void __thiscall ADishonoredPawn::GetFocusBoneHelper(class FName const &, class FVector const &, class FVector *, class FRotator *)const
//   0x7959c0  public: virtual void __thiscall ADishonoredPawn::Tick_Body(float, enum ELevelTick)
//   0x7959e0  protected: virtual unsigned int __thiscall ADishonoredPawn::IsAutoFootfallEnabled(void)const
//   0x795a30  protected: virtual class UClass * __thiscall ADishonoredPawn::ChooseFootfallContactType(void)const
//   0x795a80  public: void __thiscall ADishonoredPawn::DoFootLock_Notify(unsigned int, int)
//   0x795b80  public: unsigned int __thiscall FDisFootCache::UpdateFootfallCache(struct FDisFootInfo const &, class USkeletalMeshComponent *, float, class FVector *)
//   0x795f70  public: virtual void __thiscall ADishonoredPawn::CrushedBy_Native(class APawn *)
//   0x799d10  public: virtual void __thiscall UDisSkeletalMeshComponent::TickEnd(float)
//   0x799d60  public: virtual unsigned int __thiscall ADishonoredPawn::OnTeleport_Native(class USeqAct_Teleport *)
//   0x79d690  public: void __thiscall UDisSkeletalMeshComponent::SetupUsedMaterials(void)
//   0x79d7c0  public: void __thiscall ADishonoredPawn::DoFootfallDamage(class UClass *, class FVector const &, float, int)
//   0x79ee50  public: enum eDisHitRegion __thiscall ADishonoredPawn::GetBoneRegion(class FName)const
//   0x79eea0  public: struct FDisRegionInfo const * __thiscall ADishonoredPawn::GetBoneRegionInfo(class FName)const
//   0x79ef60  public: struct FDisRegionInfo const * __thiscall ADishonoredPawn::GetHitRegionInfo(enum eDisHitRegion)const
//   0x79efa0  public: unsigned int __thiscall ADishonoredPawn::IsBoneInRegion(class FName, enum eDisHitRegion)const
//   0x79efd0  public: void __thiscall ADishonoredPawn::DoFootfallContact(int, class FVector const *)
//   0x79f560  public: void __thiscall ADishonoredPawn::DoFootfallContact_Notify(int)
//   0x79f640  public: virtual class UClass * __thiscall ADishonoredPawn::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x7a0bd0  public: void __thiscall ADishonoredPawn::GetFocusBone_ByRegion(enum eDisHitRegion, enum eDisRegionFocusType, class FVector *, class FRotator *)const
//   0x7a0c90  public: void __thiscall ADishonoredPawn::GetNearestFocusBoneToCam(enum eDisHitRegion, enum eDisRegionFocusType, class FVector const &, class FVector const &, class FVector *, class FRotator *)const
//   0x7a1250  public: unsigned int __thiscall ADishonoredPawn::GetBBox_ForRegion(enum eDisHitRegion, struct FBoxSphereBounds &)const
//   0x7a13e0  protected: virtual void __thiscall ADishonoredPawn::OnMeshPrecompose(void)
//   0x7a1560  public: class FVector __thiscall ADishonoredPawn::GetSpineBendRefPoint(void)const
//   0x7a4150  public: static class UClass * __cdecl UDisSkeletalMeshComponent::GetPrivateStaticClassUDisSkeletalMeshComponent(wchar_t const *)
//   0x7a4940  public: static class UClass * __cdecl UDisSkeletalMeshComponent::StaticClassNoInline(void)
//   0x7a4970  public: void __thiscall ADishonoredPawn::SetupHitRegions(void)
//   0x7a4b70  public: virtual void __thiscall ADishonoredPawn::PostBeginPlay_Body(void)
//   0x7a6360  public: virtual void __thiscall UDisSkeletalMeshComponent::PreComposeSkeleton(void)
