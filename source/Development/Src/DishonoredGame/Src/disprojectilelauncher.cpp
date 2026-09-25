// DishonoredGame/src/disprojectilelauncher.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x669bb0  public: virtual class UDisTweaksBase * __thiscall ADisProjectileLauncher::GetTweaks_Derived(void)
//   0x669bc0  public: virtual enum eCrossHairStatus __thiscall ADisProjectileLauncher::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x669be0  public: virtual unsigned int __thiscall ADisProjectileLauncher::CanInteract(struct FCanInteractParams const &)const
//   0x669bf0  public: virtual unsigned int __thiscall ADisProjectileLauncher::HasSoul(int)const
//   0x669c00  public: virtual void __thiscall ADisProjectileLauncher::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x669c20  public: virtual void __thiscall ADisProjectileLauncher::DisableSoulRendering(void)
//   0x669c30  public: virtual void __thiscall ADisProjectileLauncher::HideHighlight(void)
//   0x669c50  public: virtual void __thiscall ADisProjectileLauncher::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x66be30  private: virtual void __thiscall ADishonoredUsableObject::SetTweaks_Derived(class UDisTweaksBase *)
//   0x66be40  public: virtual void __thiscall ADisProjectileLauncher::ShowHighlight(class UMaterialInterface *)
//   0x66bf20  public: virtual void __thiscall ADisProjectileLauncher::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66fc10  public: virtual void __thiscall ADisProjectileLauncher::OnToggle_Native(class USeqAct_Toggle *)
//   0x674580  protected: virtual void __thiscall UDisTweaks_ProjectileLauncher::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x6783e0  public: static class UClass * __cdecl ADisProjectileLauncher::GetPrivateStaticClassADisProjectileLauncher(wchar_t const *)
//   0x67a990  public: static class UClass * __cdecl UDisTweaks_ProjectileLauncher::GetPrivateStaticClassUDisTweaks_ProjectileLauncher(wchar_t const *)
//   0x67aa20  public: static void __cdecl ADisProjectileLauncher::InitializePrivateStaticClassADisProjectileLauncher(void)
//   0x67bb90  public: static class UClass * __cdecl ADisProjectileLauncher::StaticClassNoInline(void)
//   0x67c420  public: static void __cdecl UDisTweaks_ProjectileLauncher::InitializePrivateStaticClassUDisTweaks_ProjectileLauncher(void)
//   0x67d2e0  public: static class UClass * __cdecl UDisTweaks_ProjectileLauncher::StaticClassNoInline(void)
//   0x684710  protected: virtual unsigned int __thiscall UDisTweaks_ProjectileLauncher::FixupDefaults_Derived(void)
//   0x684740  public: virtual void __thiscall ADisProjectileLauncher::ApplyTweakChanges_Derived(void)
//   0x684790  public: void __thiscall ADisProjectileLauncher::Activate(class APawn *)
//   0x684aa0  public: void __thiscall ADisProjectileLauncher::Disarm(void)
//   0x684b40  public: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisProjectileLauncher::GetInteractableTweaks_Derived(void)const
//   0x684b70  public: virtual void __thiscall ADisProjectileLauncher::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x689ab0  public: virtual unsigned int __thiscall ADisProjectileLauncher::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x689c30  public: virtual void __thiscall ADisProjectileLauncher::BaseChange(void)
//   0x68dca0  public: virtual void __thiscall ADisProjectileLauncher::OnBroken(int, int, class FVector const &, class FVector const &, class AActor *, class UClass const *)
