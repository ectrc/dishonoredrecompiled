// DishonoredGame/src/dispickup_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x66a830  public: virtual void __thiscall ADisPickup_Base::PostLoad(void)
//   0x66a870  protected: virtual void __thiscall ADisPickup_Base::TakeDamage_Impl(int, class AController * const, class FVector const &, class FVector const &, class UClass * const, struct FTraceHitInfo const &, class AActor * const)
//   0x66a8e0  public: virtual void __thiscall ADisPickup_Base::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x66a910  public: virtual void __thiscall ADisPickup_Base::DisableSoulRendering(void)
//   0x66a930  public: virtual class UPrimitiveComponent * __thiscall ADisPickup_Base::GetMovablePrimitiveComponent(void)const
//   0x66a940  public: virtual struct FBoxSphereBounds __thiscall ADisPickup_Base::GetMovableBodyBounds(void)const
//   0x66a990  protected: virtual void __thiscall ADisPickup_Base::HideHighlight(void)
//   0x66a9b0  protected: virtual unsigned int __thiscall ADisPickup_Base::WantsTick_Derived(void)const
//   0x66c7b0  public: virtual void __thiscall ADisPickup_Base::OnRigidBodyStatusChange(void)
//   0x66c820  public: virtual unsigned int __thiscall ADisPickup_Base::CanSplash(void)
//   0x66c840  public: void __thiscall ADisPickup_Base::ConsumePickup(void)
//   0x66c870  public: virtual void __thiscall ADisPickup_Base::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x66c8d0  public: virtual unsigned int __thiscall ADisPickup_Base::WasJustThrownBy(class ADishonoredPawn const *)const
//   0x66c900  public: virtual unsigned int __thiscall ADisPickup_Base::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x66c9a0  protected: virtual void __thiscall ADisPickup_Base::ShowHighlight(class UMaterialInterface *)
//   0x66ca70  private: void __thiscall ADisPickup_Base::AdjustDetachedPosition(void)
//   0x6705b0  public: virtual unsigned int __thiscall ADisPickup_Base::IgnoreBlockingBy(class AActor const *)const
//   0x6705f0  public: void __thiscall ADisPickup_Base::Detach(void)
//   0x670680  public: virtual void __thiscall ADisPickup_Base::ClearComponents(void)
//   0x6706b0  public: virtual void __thiscall ADisPickup_Base::BaseChange(void)
//   0x676490  public: virtual class AActor * __thiscall ADisPickup_Base::GetMovableActor(void)
//   0x678bc0  public: static class UClass * __cdecl ADisPickup_Base::GetPrivateStaticClassADisPickup_Base(wchar_t const *)
//   0x67a360  public: static void __cdecl ADisPickup_Base::InitializePrivateStaticClassADisPickup_Base(void)
//   0x67b0a0  public: static class UClass * __cdecl ADisPickup_Base::StaticClassNoInline(void)
//   0x680050  protected: virtual void __thiscall ADisPickup_Base::ApplyTweakChanges_Derived(void)
//   0x6800d0  public: void __thiscall ADisPickup_Base::StartPickupTravel(class ADishonoredPawn *)
//   0x680260  protected: virtual void __thiscall ADisPickup_Base::OnActorTerminated(void)
//   0x6802c0  private: unsigned int __thiscall ADisPickup_Base::IsUseBlockedByPossession(class ADishonoredPlayerPawn const &)const
//   0x680320  protected: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisPickup_Base::GetInteractableTweaks_Derived(void)const
//   0x680350  protected: virtual unsigned int __thiscall ADisPickup_Base::AttemptCannotUseInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x6803c0  public: virtual void __thiscall ADisPickup_Base::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x680490  public: virtual void __thiscall ADisPickup_Base::PostGameLoad(enum ESaveLoadLocation)
//   0x6804b0  public: virtual unsigned int __thiscall ADisPickup_Base::HasSoul(int)const
//   0x680500  private: class ADishonoredPlayerPawn * __thiscall ADisPickup_Base::GetPlayerPawnFromUser(class ADishonoredPawn *)const
//   0x680580  public: virtual enum EMovableWeightClass __thiscall ADisPickup_Base::GetMovableWeightClass(void)const
//   0x6805c0  public: void __thiscall ADisPickup_Base::Attach(class AActor *, class USkeletalMeshComponent *)
//   0x680650  public: virtual void __thiscall ADisPickup_Base::Steal(class ADishonoredPlayerPawn *)
//   0x6806b0  public: virtual unsigned int __thiscall ADisPickup_Base::Tick(float, enum ELevelTick)
//   0x680980  public: virtual void __thiscall ADisPickup_Base::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x680a00  public: virtual void __thiscall ADisPickup_Base::PostBeginPlay(void)
//   0x682c50  protected: virtual enum eCrossHairStatus __thiscall ADisPickup_Base::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x682ca0  protected: virtual unsigned int __thiscall ADisPickup_Base::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
