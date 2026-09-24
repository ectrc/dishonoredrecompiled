// DishonoredGame/src/dishonoredactivepowercomponent_blink.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (38):
//   0x824a60  protected: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Blink::IsAvailable_Derived(void)const
//   0x824aa0  public: unsigned int __thiscall UDishonoredActivePowerComponent_Blink::IsTravelling(void)const
//   0x824ab0  protected: unsigned int __thiscall UDishonoredActivePowerComponent_Blink::OwnerCanBlink(void)
//   0x824c70  protected: void __thiscall UDishonoredActivePowerComponent_Blink::ConfigureBasedOnOwner(void)
//   0x824db0  protected: void __thiscall UDishonoredActivePowerComponent_Blink::TryCrouch(void)
//   0x824e50  protected: void __thiscall UDishonoredActivePowerComponent_Blink::ConditionalUncrouch(void)
//   0x824e90  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Blink::AllowBump(class ADishonoredNPCPawn *)const
//   0x824f00  protected: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Blink::CanBeCanceled_Derived(void)const
//   0x824f10  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::OnPhysModeChanged(enum EPhysics)
//   0x8288b0  private: float __thiscall UDishonoredActivePowerComponent_Blink::GetBackupDistance(class FVector, class FVector)const
//   0x828a60  protected: void __thiscall UDishonoredActivePowerComponent_Blink::DisableEffects(void)
//   0x839500  public: static void __cdecl UDishonoredActivePowerComponent_Blink::InitializePrivateStaticClassUDishonoredActivePowerComponent_Blink(void)
//   0x83e0a0  public: static class UClass * __cdecl UDishonoredActivePowerComponent_Blink::GetPrivateStaticClassUDishonoredActivePowerComponent_Blink(wchar_t const *)
//   0x83e130  protected: unsigned int __thiscall UDishonoredActivePowerComponent_Blink::MoveActorToPos(class FVector const &)
//   0x83e7f0  protected: void __thiscall UDishonoredActivePowerComponent_Blink::EndBlink(void)
//   0x83f430  public: static class UClass * __cdecl UDishonoredActivePowerComponent_Blink::StaticClassNoInline(void)
//   0x83f460  protected: void __thiscall UDishonoredActivePowerComponent_Blink::Tick_Cooldown(float)
//   0x840720  public: static class UClass * __cdecl UDisTweaks_Blink::GetPrivateStaticClassUDisTweaks_Blink(wchar_t const *)
//   0x840e30  public: static void __cdecl UDisTweaks_Blink::InitializePrivateStaticClassUDisTweaks_Blink(void)
//   0x841360  public: static class UClass * __cdecl UDisTweaks_Blink::StaticClassNoInline(void)
//   0x844d20  public: class FVector __thiscall UDishonoredActivePowerComponent_Blink::LimitDirection(class FVector const &, class FVector const &, class AActor *)const
//   0x844f60  protected: void __thiscall UDishonoredActivePowerComponent_Blink::SetupTargetFromPlayer(class ADishonoredPlayerPawn *)
//   0x845970  protected: void __thiscall UDishonoredActivePowerComponent_Blink::BeginCooldown(unsigned int)
//   0x845c00  protected: void __thiscall UDishonoredActivePowerComponent_Blink::TouchedPawn(class AActor *)
//   0x845ee0  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::OnTouch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x845ef0  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::OnBump(class AActor *, class UPrimitiveComponent *, class FVector const &)
//   0x845f00  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::Cancel(void)
//   0x845f10  protected: struct FPowerAttributes_Blink const * __thiscall UDishonoredActivePowerComponent_Blink::GetPowerAttributes(void)const
//   0x845f50  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Blink::IgnoreBlockingBy(class AActor const *)const
//   0x846fa0  protected: float __thiscall UDishonoredActivePowerComponent_Blink::Tick_Move(float)
//   0x8473c0  public: unsigned int __thiscall UDishonoredActivePowerComponent_Blink::GetSettings(void)
//   0x847470  protected: void __thiscall UDishonoredActivePowerComponent_Blink::BeginWarmup(void)
//   0x8474d0  protected: void __thiscall UDishonoredActivePowerComponent_Blink::BeginMove(void)
//   0x847640  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::OnButtonReleased(void)
//   0x847f90  protected: void __thiscall UDishonoredActivePowerComponent_Blink::Tick_Warmup(float)
//   0x8487c0  public: virtual void __thiscall UDishonoredActivePowerComponent_Blink::Tick(float)
//   0x848b90  protected: void __thiscall UDishonoredActivePowerComponent_Blink::Blink(void)
//   0x848e20  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Blink::Execute(void)
