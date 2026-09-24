// DishonoredGame/src/dishonoredactivepowercomponent_possess.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (43):
//   0x849000  public: static void __cdecl UDishonoredActivePowerComponent_Possess::InitializePrivateStaticClassUDishonoredActivePowerComponent_Possess(void)
//   0x849020  private: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Possess::CanBeCanceled_Derived(void)const
//   0x849030  private: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Possess::IsAvailable_Derived(void)const
//   0x849070  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Possess::IsActive(void)const
//   0x849080  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Possess::IsStillBusy(void)const
//   0x849090  public: virtual unsigned int __thiscall UDishonoredActivePowerComponent_Possess::Execute(void)
//   0x8490b0  public: void __thiscall UDishonoredActivePowerComponent_Possess::FirePossession(void)
//   0x8490d0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::OnButtonReleased(void)
//   0x8490f0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::ForceCancel(void)
//   0x849100  public: float __thiscall UDishonoredActivePowerComponent_Possess::GetPossessionTimeLeft(void)const
//   0x849110  public: class TScriptInterface<class IDisPossessableInterface> const & __thiscall UDishonoredActivePowerComponent_Possess::GetPossessee(void)const
//   0x849120  public: class ADishonoredPawn * __thiscall UDishonoredActivePowerComponent_Possess::GetPossessedPawn(void)const
//   0x849130  public: void __thiscall UDishonoredActivePowerComponent_Possess::SetDieOnEndPossess(void)
//   0x84aa80  public: void __thiscall UDishonoredActivePowerComponent_Possess::PlayPossessionActionFailSound(void)
//   0x84aae0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::FillUIInteractions(struct FDisUIInteractionContext &)const
//   0x84f4e0  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_Cooldown(float)
//   0x84f590  private: void __thiscall UDishonoredActivePowerComponent_Possess::ClearHighlightedTarget(void)
//   0x855350  public: static class UClass * __cdecl UDishonoredActivePowerComponent_Possess::GetPrivateStaticClassUDishonoredActivePowerComponent_Possess(wchar_t const *)
//   0x8553e0  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_Effects(void)
//   0x856ef0  public: static class UClass * __cdecl UDishonoredActivePowerComponent_Possess::StaticClassNoInline(void)
//   0x856f20  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8570f0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x85e050  private: struct FDisPossessLevel const __thiscall UDishonoredActivePowerComponent_Possess::GetTweaksLevel(void)const
//   0x85e0e0  private: void __thiscall UDishonoredActivePowerComponent_Possess::CollectAllPossessables(struct TMemStackArray<class IDisPossessableInterface *> &)const
//   0x85e1e0  private: unsigned int __thiscall UDishonoredActivePowerComponent_Possess::TargetCanBePossessed(class IDisPossessableInterface *, float *, float *, unsigned int *, unsigned int *)const
//   0x85e6f0  private: void __thiscall UDishonoredActivePowerComponent_Possess::StartIntro(void)
//   0x85e800  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::ApplyTweakChanges_Derived(void)
//   0x85e840  private: void __thiscall UDishonoredActivePowerComponent_Possess::SetStatusMessage(enum EDisPossessStatusMessage)
//   0x85fb30  private: virtual void __thiscall UDishonoredActivePowerComponent_Possess::Detach(unsigned int)
//   0x85fb80  private: void __thiscall UDishonoredActivePowerComponent_Possess::CollectValidTargets(struct TMemStackArray<struct FPossessTargetEntry> &)const
//   0x85fdb0  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_PossessionInactive(float)
//   0x85fe20  private: void __thiscall UDishonoredActivePowerComponent_Possess::TryStopPossessing(unsigned int)
//   0x85ff60  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::OnOtherActorTerminated(class AActor const &)
//   0x8602c0  private: class IDisPossessableInterface * __thiscall UDishonoredActivePowerComponent_Possess::FindBestValidTarget(unsigned int &, unsigned int &)const
//   0x860510  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_PossessionActive(float)
//   0x8606a0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::Cancel(void)
//   0x860730  private: void __thiscall UDishonoredActivePowerComponent_Possess::TryStartPossessing(unsigned int)
//   0x860980  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::PostGameLoad(enum ESaveLoadLocation)
//   0x860d10  private: struct FDisPossessTarget __thiscall UDishonoredActivePowerComponent_Possess::FindIntendedTarget(void)const
//   0x861140  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_Targeting(void)
//   0x8612e0  private: void __thiscall UDishonoredActivePowerComponent_Possess::Tick_PrePossess(float)
//   0x861800  private: virtual void __thiscall UDishonoredActivePowerComponent_Possess::Attach(void)
//   0x8618e0  public: virtual void __thiscall UDishonoredActivePowerComponent_Possess::Tick(float)
