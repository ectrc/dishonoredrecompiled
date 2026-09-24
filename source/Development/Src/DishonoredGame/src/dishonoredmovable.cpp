// DishonoredGame/src/dishonoredmovable.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (35):
//   0x66b630  public: static void __cdecl ADishonoredMovable::InitializePrivateStaticClassADishonoredMovable(void)
//   0x66b650  public: static void __cdecl UDisTweaks_Movable::InitializePrivateStaticClassUDisTweaks_Movable(void)
//   0x66b670  protected: virtual class UDisTweaksBase * __thiscall ADishonoredMovable::GetTweaks_Derived(void)
//   0x66b680  public: virtual class UObject * __thiscall ADishonoredMovable::GetUObjectInterfaceDisMovableInterface(void)
//   0x66b690  public: virtual class UPrimitiveComponent * __thiscall ADishonoredMovable::GetMovablePrimitiveComponent(void)const
//   0x66b6a0  public: virtual struct FBoxSphereBounds __thiscall ADishonoredMovable::GetMovableBodyBounds(void)const
//   0x66b6f0  protected: virtual int __thiscall ADishonoredMovable::GetMaxDamage(void)const
//   0x66b700  protected: virtual void __thiscall ADishonoredMovable::SetMaxDamage(int)
//   0x66b710  public: virtual void __thiscall ADishonoredMovable::HideHighlight(void)
//   0x66b730  public: virtual void __thiscall ADishonoredMovable::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x66de20  protected: virtual void __thiscall ADishonoredMovable::SetTweaks_Derived(class UDisTweaksBase *)
//   0x66de30  public: virtual unsigned int __thiscall ADishonoredMovable::CanSplash(void)
//   0x66de50  public: virtual void __thiscall ADishonoredMovable::OnBroken(int)
//   0x66de90  public: virtual unsigned int __thiscall ADishonoredMovable::WasJustThrownBy(class ADishonoredPawn const *)const
//   0x66dec0  public: virtual void __thiscall ADishonoredMovable::OnMovableGrab_Complete(class ADishonoredPawn *)
//   0x66dee0  public: virtual void __thiscall ADishonoredMovable::OnMovableGrab_End(class ADishonoredPawn *)
//   0x66dfb0  public: virtual unsigned int __thiscall ADishonoredMovable::Tick(float, enum ELevelTick)
//   0x66e720  public: virtual void __thiscall ADishonoredMovable::ShowHighlight(class UMaterialInterface *)
//   0x66e7f0  public: unsigned int __thiscall ADishonoredMovable::IsBeingHeld(void)const
//   0x671760  protected: virtual class UDisTweaks_InteractableInterface const * __thiscall ADishonoredMovable::GetInteractableTweaks_Derived(void)const
//   0x6717a0  public: virtual void __thiscall ADishonoredMovable::OnSleepRBPhysics_Native(void)
//   0x67cb50  public: static class UClass * __cdecl UDisTweaks_Movable::GetPrivateStaticClassUDisTweaks_Movable(wchar_t const *)
//   0x681530  public: static class UClass * __cdecl ADishonoredMovable::GetPrivateStaticClassADishonoredMovable(wchar_t const *)
//   0x6815c0  public: static class UClass * __cdecl UDisTweaks_Movable::StaticClassNoInline(void)
//   0x6846e0  public: static class UClass * __cdecl ADishonoredMovable::StaticClassNoInline(void)
//   0x689630  public: virtual unsigned int __thiscall ADishonoredMovable::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x689720  public: unsigned int __thiscall ADishonoredMovable::ShouldBlockPlayer(void)const
//   0x689790  private: unsigned int __thiscall ADishonoredMovable::CanInteract_Internal(void)const
//   0x6897e0  public: virtual enum eCrossHairStatus __thiscall ADishonoredMovable::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x689800  public: virtual unsigned int __thiscall ADishonoredMovable::CanInteract(struct FCanInteractParams const &)const
//   0x689830  public: virtual unsigned int __thiscall ADishonoredMovable::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x689960  public: virtual enum EMovableWeightClass __thiscall ADishonoredMovable::GetMovableWeightClass(void)const
//   0x6899a0  public: virtual void __thiscall ADishonoredMovable::OnThrow(class FVector const &, class FVector const &)
//   0x689a40  public: virtual unsigned int __thiscall ADishonoredMovable::HasSoul(int)const
//   0x68dc40  public: virtual unsigned int __thiscall ADishonoredMovable::IgnoreBlockingBy(class AActor const *)const
