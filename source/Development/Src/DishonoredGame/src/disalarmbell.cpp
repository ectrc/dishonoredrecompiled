// DishonoredGame/src/disalarmbell.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (47):
//   0x66ab50  public: static void __cdecl UDisSeqEvent_AlarmEnded::InitializePrivateStaticClassUDisSeqEvent_AlarmEnded(void)
//   0x66ab70  public: virtual void __thiscall ADisAlarmBell::PostEditMove(unsigned int)
//   0x66ab90  public: void __thiscall ADisAlarmBell::RemovePendingRinger(class UDisBehaviorTriggerAlarm *)
//   0x66abc0  public: class UDisBehaviorTriggerAlarm * __thiscall ADisAlarmBell::GetPendingRinger(void)const
//   0x66abd0  public: class UDisBehaviorTriggerAlarm * __thiscall ADisAlarmBell::GetPreviousPendingRinger(void)const
//   0x66abe0  public: void __thiscall ADisAlarmBell::ClearPendingRinger(void)
//   0x66abf0  public: void __thiscall ADisAlarmBell::AcquireAlarm(class ADishonoredNPCPawn *)
//   0x66ac00  public: void __thiscall ADisAlarmBell::ReleaseAlarm(class ADishonoredNPCPawn *)
//   0x66ac10  private: virtual void __thiscall ADisAlarmBell::OnPawnSpawned(class ADishonoredNPCPawn *)
//   0x66ac20  public: unsigned int __thiscall ADisAlarmBell::IsRinging(void)const
//   0x66ac30  public: virtual class ADishonoredAudioVolume * __thiscall ADisAlarmBell::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x66ac50  public: class FVector __thiscall ADisAlarmBell::GetPreviousValidNPCPosition(void)const
//   0x66cdd0  public: virtual void __thiscall ADisAlarmBell::PostEditSelectChange(void)
//   0x66cde0  public: unsigned int __thiscall ADisAlarmBell::TrySetPendingRinger(class UDisBehaviorTriggerAlarm *, float)
//   0x66cee0  public: void __thiscall ADisAlarmBell::SendAlarmToSpawnedPawn(class FVector const &, int, class AActor *, class ADishonoredNPCPawn *)
//   0x66d250  protected: virtual void __thiscall ADisDoor::SetTweaks_Derived(class UDisTweaksBase *)
//   0x670950  public: virtual unsigned int __thiscall ADisAlarmBell::OnIsUsable(void)const
//   0x670980  public: unsigned int __thiscall ADisAlarmBell::ReadyToBeUsedByNPC(class ADishonoredNPCPawn *)const
//   0x6709d0  private: virtual void __thiscall ADisAlarmBell::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x674240  public: void __thiscall ADisAlarmBell::SetRingerEnemy(struct FDisAttentionProxy const &)
//   0x677840  public: virtual void __thiscall ADisAlarmBell::PostScriptDestroyed(void)
//   0x678d90  public: static class UClass * __cdecl ADisAlarmBell::GetPrivateStaticClassADisAlarmBell(wchar_t const *)
//   0x67b370  public: static class UClass * __cdecl UDisTweaks_Interactable_AlarmBell::GetPrivateStaticClassUDisTweaks_Interactable_AlarmBell(wchar_t const *)
//   0x67b8e0  public: virtual int __thiscall UDisConv_PlayerChoice::GetNumOptionalChoices(void)const
//   0x67bee0  public: static void __cdecl ADisAlarmBell::InitializePrivateStaticClassADisAlarmBell(void)
//   0x67bf00  public: static void __cdecl UDisTweaks_Interactable_AlarmBell::InitializePrivateStaticClassUDisTweaks_Interactable_AlarmBell(void)
//   0x67c800  public: static class UClass * __cdecl ADisAlarmBell::StaticClassNoInline(void)
//   0x67c830  public: static class UClass * __cdecl UDisSeqEvent_AlarmEnded::GetPrivateStaticClassUDisSeqEvent_AlarmEnded(wchar_t const *)
//   0x67c8c0  public: static class UClass * __cdecl UDisTweaks_Interactable_AlarmBell::StaticClassNoInline(void)
//   0x680b80  public: static class UClass * __cdecl UDisSeqEvent_AlarmEnded::StaticClassNoInline(void)
//   0x682f90  void __cdecl DisAlarmBell::CheckSendAlarmEndKismetEvent(class ADisAlarmBell * const)
//   0x6877a0  private: void __thiscall ADisAlarmBell::SoundAlarm(void)
//   0x687ba0  public: float __thiscall ADisAlarmBell::GetNPCClosenessGoToAlarm(void)const
//   0x687be0  private: void __thiscall ADisAlarmBell::GetValidNPCPositions(class ADishonoredNPCPawn const *, class FVector * const)const
//   0x687eb0  private: void __thiscall ADisAlarmBell::SetAlarmIsRingingCommon(unsigned int)
//   0x687f40  private: void __thiscall ADisAlarmBell::SetAlarmIsRinging(unsigned int, unsigned int)
//   0x688010  private: void __thiscall ADisAlarmBell::SetAlarmIsReady(unsigned int)
//   0x6880a0  private: virtual void __thiscall ADisAlarmBell::OnOtherActorTerminated(class AActor const &)
//   0x68b030  public: virtual unsigned int __thiscall ADisAlarmBell::OnUseObject(void)
//   0x68b1e0  public: virtual void __thiscall ADisAlarmBell::OnTransitionStart(void)
//   0x68b220  public: class FVector __thiscall ADisAlarmBell::GetNearestValidNPCPosition(class ADishonoredNPCPawn const *)const
//   0x68b3e0  private: virtual void __thiscall ADisAlarmBell::PostGameLoad(enum ESaveLoadLocation)
//   0x68b470  public: virtual void __thiscall ADisAlarmBell::OnActorTerminated(void)
//   0x68b590  private: virtual void __thiscall ADisAlarmBell::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x68dd20  public: virtual void __thiscall ADisAlarmBell::PostBeginPlay(void)
//   0x68dfe0  private: virtual void __thiscall ADisAlarmBell::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6907f0  public: virtual unsigned int __thiscall ADisAlarmBell::Tick(float, enum ELevelTick)
