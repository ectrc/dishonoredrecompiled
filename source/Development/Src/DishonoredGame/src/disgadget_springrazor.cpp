// DishonoredGame/src/disgadget_springrazor.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (48):
//   0x886ea0  public: static void __cdecl ADisGadget_SpringRazorPlaced::InitializePrivateStaticClassADisGadget_SpringRazorPlaced(void)
//   0x886ec0  public: static void __cdecl UDisTweaks_SpringRazor::InitializePrivateStaticClassUDisTweaks_SpringRazor(void)
//   0x886ee0  public: static void __cdecl UDisTweaks_SpringRazorPlaced::InitializePrivateStaticClassUDisTweaks_SpringRazorPlaced(void)
//   0x886f00  public: virtual unsigned int __thiscall UDisGadget_SpringRazor::ItemIsPhysicallyEquipped(void)const
//   0x886f20  public: virtual class UDisTweaksBase * __thiscall UDisGadget_SpringRazor::GetTweaks_Derived(void)
//   0x886f30  protected: virtual unsigned int __thiscall UDisGadget_SpringRazor::IsEquippable_Derived(class ADishonoredPawn const *)const
//   0x886f50  public: virtual enum eCrossHairStatus __thiscall ADisGadget_SpringRazorPlaced::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x886f80  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x887070  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::HideHighlight(void)
//   0x887090  public: virtual unsigned int __thiscall ADisGadget_SpringRazorPlaced::CanInteract(struct FCanInteractParams const &)const
//   0x8870a0  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x8870c0  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::DisableSoulRendering(void)
//   0x889a50  protected: virtual void __thiscall UDisGadget_SpringRazor::OnAddAmmo_Derived(enum eDisAmmoType)
//   0x889a90  protected: class FVector __thiscall ADisGadget_SpringRazorPlaced::GetForward(void)const
//   0x889c00  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::SetTweaks_Derived(class UDisTweaksBase *)
//   0x889c10  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::ShowHighlight(class UMaterialInterface *)
//   0x889cf0  public: void __thiscall ADisGadget_SpringRazorPlaced::SerializeSaveLoadCommon(class FArchive &)
//   0x889d40  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x889da0  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x889e90  protected: virtual void __thiscall UDisItem_WolfhoundBark::SetTweaks_Derived(class UDisTweaksBase *)
//   0x88e1e0  public: virtual enum EDisCrosshairState __thiscall UDisGadget_SpringRazor::GetCrosshairState(void)const
//   0x8924d0  protected: unsigned int __thiscall ADisGadget_SpringRazorPlaced::FindTraceablePointInCylinder(class AActor const * const, struct TMemStackArray<class FVector> const &, class FVector const &, float, class FVector const &, class FVector &)
//   0x8956c0  protected: virtual void __thiscall UDisTweaks_SpringRazorPlaced::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x899480  public: static class UClass * __cdecl ADisGadget_SpringRazorPlaced::GetPrivateStaticClassADisGadget_SpringRazorPlaced(wchar_t const *)
//   0x89c2b0  public: static class UClass * __cdecl ADisGadget_SpringRazorPlaced::StaticClassNoInline(void)
//   0x89deb0  public: static class UClass * __cdecl UDisTweaks_SpringRazor::GetPrivateStaticClassUDisTweaks_SpringRazor(wchar_t const *)
//   0x89df40  public: static class UClass * __cdecl UDisTweaks_SpringRazorPlaced::GetPrivateStaticClassUDisTweaks_SpringRazorPlaced(wchar_t const *)
//   0x89f4f0  public: static class UClass * __cdecl UDisTweaks_SpringRazor::StaticClassNoInline(void)
//   0x89f520  public: static class UClass * __cdecl UDisTweaks_SpringRazorPlaced::StaticClassNoInline(void)
//   0x8a1240  public: virtual unsigned int __thiscall ADisGadget_SpringRazorPlaced::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x8a12c0  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::PostBeginPlay(void)
//   0x8a1390  protected: class FVector __thiscall ADisGadget_SpringRazorPlaced::GetDetectionCylinderTop(void)const
//   0x8a1440  protected: class FVector __thiscall ADisGadget_SpringRazorPlaced::GetDetonationCylinderTop(void)const
//   0x8a14f0  protected: class FVector __thiscall ADisGadget_SpringRazorPlaced::GetSensorLocation(void)const
//   0x8a15a0  protected: void __thiscall ADisGadget_SpringRazorPlaced::PreDetonate(class AActor *, unsigned int, unsigned int)
//   0x8a1640  protected: void __thiscall ADisGadget_SpringRazorPlaced::HarmActor(class AActor * const, class FVector const &, class UClass * const, float, float, int, int, class UClass *)
//   0x8a1c30  protected: void __thiscall ADisGadget_SpringRazorPlaced::Detonate(void)
//   0x8a1d10  protected: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisGadget_SpringRazorPlaced::GetInteractableTweaks_Derived(void)const
//   0x8a5c20  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::BaseChange(void)
//   0x8a5c60  public: virtual void __thiscall ADisGadget_SpringRazorPlaced::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x8a8950  protected: virtual unsigned int __thiscall UDisTweaks_SpringRazorPlaced::FixupDefaults_Derived(void)
//   0x8ad480  public: static class UClass * __cdecl UDisGadget_SpringRazor::GetPrivateStaticClassUDisGadget_SpringRazor(wchar_t const *)
//   0x8ad9c0  public: static void __cdecl UDisGadget_SpringRazor::InitializePrivateStaticClassUDisGadget_SpringRazor(void)
//   0x8adc50  public: static class UClass * __cdecl UDisGadget_SpringRazor::StaticClassNoInline(void)
//   0x8ae1a0  protected: float __thiscall ADisGadget_SpringRazorPlaced::GetRadiusBonus(void)const
//   0x8ae230  protected: float __thiscall ADisGadget_SpringRazorPlaced::GetDetectionCylinderRadius(void)const
//   0x8ae280  protected: void __thiscall ADisGadget_SpringRazorPlaced::EmitDamage(void)
//   0x8ae870  public: virtual unsigned int __thiscall ADisGadget_SpringRazorPlaced::Tick(float, enum ELevelTick)
