// DishonoredGame/src/diswhaleoilbattery.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x8d9060  public: static void __cdecl ADisWhaleOilBattery::InitializePrivateStaticClassADisWhaleOilBattery(void)
//   0x8d9080  public: static void __cdecl UDisTweaks_WhaleOilBattery::InitializePrivateStaticClassUDisTweaks_WhaleOilBattery(void)
//   0x8d90a0  public: static void __cdecl UDisSeqEvent_WhaleOilBattery::InitializePrivateStaticClassUDisSeqEvent_WhaleOilBattery(void)
//   0x8d90c0  public: static void __cdecl UDisSeqAct_PlugWhaleOilBattery::InitializePrivateStaticClassUDisSeqAct_PlugWhaleOilBattery(void)
//   0x8d90e0  public: static void __cdecl UDisSeqAct_RefillWhaleOilBattery::InitializePrivateStaticClassUDisSeqAct_RefillWhaleOilBattery(void)
//   0x8d9100  protected: virtual void __thiscall ADisWhaleOilBattery::OnActorTerminated(void)
//   0x8d9110  public: virtual unsigned int __thiscall ADisWhaleOilBattery::IgnoreBlockingBy(class AActor const *)const
//   0x8d9140  protected: virtual unsigned int __thiscall ADisWhaleOilBattery::ShouldBlockPlayer_Override(void)const
//   0x8d9150  public: virtual void __thiscall ADisWhaleOilBattery::OnMovableGrab_End(class ADishonoredPawn *)
//   0x8d9180  public: unsigned int __thiscall ADisWhaleOilBattery::IsThrown(void)const
//   0x8d9190  public: virtual void __thiscall ADisWhaleOilBattery::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x8d91c0  protected: virtual class UDisTweaksBase * __thiscall ADisWhaleOilBattery::GetTweaks_Derived(void)
//   0x8d91d0  public: virtual void __thiscall ADishonoredMovable::DisableSoulRendering(void)
//   0x8da220  public: unsigned int __thiscall ADisWhaleOilBattery::OnPlugged(class ADisWhaleOilReceptacle *, unsigned int)
//   0x8da530  public: virtual void __thiscall ADisWhaleOilBattery::OnRigidBodyStatusChange(void)
//   0x8da560  public: virtual unsigned int __thiscall ADisWhaleOilBattery::Tick(float, enum ELevelTick)
//   0x8da6c0  public: virtual void __thiscall ADisWhaleOilBattery::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8da790  protected: virtual void __thiscall ADisWhaleOilBattery::SetTweaks_Derived(class UDisTweaksBase *)
//   0x8f0210  public: static class UClass * __cdecl UDisTweaks_WhaleOilBattery::GetPrivateStaticClassUDisTweaks_WhaleOilBattery(wchar_t const *)
//   0x8f02a0  public: static class UClass * __cdecl UDisSeqEvent_WhaleOilBattery::GetPrivateStaticClassUDisSeqEvent_WhaleOilBattery(wchar_t const *)
//   0x8f0330  public: static class UClass * __cdecl UDisSeqAct_PlugWhaleOilBattery::GetPrivateStaticClassUDisSeqAct_PlugWhaleOilBattery(wchar_t const *)
//   0x8f03c0  public: static class UClass * __cdecl UDisSeqAct_RefillWhaleOilBattery::GetPrivateStaticClassUDisSeqAct_RefillWhaleOilBattery(wchar_t const *)
//   0x8f0af0  public: static class UClass * __cdecl ADisWhaleOilBattery::GetPrivateStaticClassADisWhaleOilBattery(wchar_t const *)
//   0x8f0b80  public: static class UClass * __cdecl UDisTweaks_WhaleOilBattery::StaticClassNoInline(void)
//   0x8f0bb0  public: static class UClass * __cdecl UDisSeqEvent_WhaleOilBattery::StaticClassNoInline(void)
//   0x8f0be0  public: static class UClass * __cdecl UDisSeqAct_PlugWhaleOilBattery::StaticClassNoInline(void)
//   0x8f0c10  public: static class UClass * __cdecl UDisSeqAct_RefillWhaleOilBattery::StaticClassNoInline(void)
//   0x8f2020  public: static class UClass * __cdecl ADisWhaleOilBattery::StaticClassNoInline(void)
//   0x8f3df0  public: virtual void __thiscall ADisWhaleOilBattery::PostScriptDestroyed(void)
//   0x8f3e40  private: int __thiscall ADisWhaleOilBattery::GetChargesFromType(enum EWhaleOilChargeCost)const
//   0x8f3eb0  public: virtual unsigned int __thiscall ADisWhaleOilBattery::HasSoul(int)const
//   0x8f3f10  public: virtual void __thiscall ADisWhaleOilBattery::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x8f3f80  public: virtual class FString const & __thiscall ADisWhaleOilBattery::GetInteractableName(void)const
//   0x8f3fc0  public: virtual unsigned int __thiscall ADisWhaleOilBattery::CanInteract(struct FCanInteractParams const &)const
//   0x8f4030  private: unsigned int __thiscall ADisWhaleOilBattery::IsShielded(class FVector const &, class AActor *)const
//   0x8f5a10  public: virtual class URB_BodyInstance * __thiscall ADisWhaleOilBattery::OnMovableGrab_Start(class ADishonoredPawn *)
//   0x8f5a50  public: void __thiscall ADisWhaleOilBattery::Unplug(unsigned int)
//   0x8f5c10  public: virtual void __thiscall ADisWhaleOilBattery::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x8f5f00  public: virtual void __thiscall ADisWhaleOilBattery::OnBroken(int)
//   0x8f6040  public: virtual void __thiscall UDisSeqAct_PlugWhaleOilBattery::Activated(void)
//   0x8f6950  public: void __thiscall ADisWhaleOilBattery::UpdateChargeCount(int, unsigned int)
//   0x8f6c60  public: virtual void __thiscall ADisWhaleOilBattery::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8f6d50  public: virtual void __thiscall UDisSeqAct_RefillWhaleOilBattery::Activated(void)
//   0x8f71e0  public: virtual void __thiscall ADisWhaleOilBattery::PreBeginPlay(void)
