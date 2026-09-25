// DishonoredGame/src/dishonoredspawner.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x691500  public: static void __cdecl ADishonoredSpawner::InitializePrivateStaticClassADishonoredSpawner(void)
//   0x691520  public: void __thiscall FDisAlarmSpawnInfo::SetAlarmInfo(class ADisAlarmBell *, class FVector const &, int, class AActor *)
//   0x691550  public: void __thiscall FDisAlarmSpawnInfo::SendAlarmToSpawnedPawn(class ADishonoredNPCPawn *)const
//   0x691570  public: virtual void __thiscall ADishonoredSpawner::PostLoad(void)
//   0x691600  public: virtual void __thiscall ADishonoredSpawner::PostEditMove(unsigned int)
//   0x691670  private: virtual class UDisTweaksBase * __thiscall ADishonoredSpawner::GetTweaks_Derived(void)
//   0x691680  private: virtual class ADishonoredAudioVolume * __thiscall ADishonoredSpawner::GetListenerAudioCell(void)const
//   0x694020  public: __thiscall FDisSpawnInfo::FDisSpawnInfo(void)
//   0x694080  public: virtual void __thiscall ADishonoredSpawner::PostEditImport(void)
//   0x6940d0  private: virtual void __thiscall ADishonoredSpawner::SetTweaks_Derived(class UDisTweaksBase *)
//   0x6940e0  public: virtual void __thiscall ADishonoredSpawner::HandleSquadNameChange(class FName const &, class FName const &)
//   0x694120  private: virtual class FVector __thiscall ADishonoredSpawner::GetEarLocation(void)const
//   0x697170  public: virtual void __thiscall ADishonoredSpawner::MAT_StartPreview(void)
//   0x6971e0  public: virtual void __thiscall ADishonoredSpawner::MAT_EndPreview(void)
//   0x697250  protected: void __thiscall ADishonoredSpawner::GetDialogTrees(class UDisDialogTree * &, class UDisDialogTree * &)
//   0x697300  public: virtual void __thiscall ADishonoredSpawner::AddDialogOneShot(class UDisDialogVoiceData *, unsigned int)
//   0x697340  public: virtual class IDisConvSpeakerInterface * __thiscall ADishonoredSpawner::GetSpeakerFromOneShotOwner(class IDisConvSpeakerInterface *)
//   0x6973d0  private: unsigned int __thiscall ADishonoredSpawner::IsMinDelaySinceLastSpawnElapsed(float * const)const
//   0x697450  private: void __thiscall ADishonoredSpawner::CopyEventsToSpawned(class ADishonoredNPCPawn *)
//   0x697700  public: virtual class UDisTweaks_Vision const * __thiscall ADishonoredSpawner::GetDrawVisionTweaks(void)const
//   0x697740  public: virtual unsigned int __thiscall ADishonoredSpawner::GetDrawVisionLocationAndRotation(class FVector &, class FRotator &)const
//   0x69c270  public: virtual class FString const __thiscall ADisPickup_Base::GetDisplayName(void)const
//   0x69c2a0  public: virtual class APawn * __thiscall ADishonoredSpawner::MAT_PreviewForceSpawn(class FString &)
//   0x69c510  public: virtual void __thiscall ADishonoredSpawner::DisOnDialogSelChange(class TArray<class UDisConv_Node *, class FDefaultAllocator> &)
//   0x69f750  public: virtual void __thiscall ADishonoredSpawner::RemoveDialogOneShot(class UDisDialogVoiceData *)
//   0x69f880  private: void __thiscall ADishonoredSpawner::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x6a0a60  public: unsigned int __thiscall ADishonoredSpawner::SpawnOnePawn(struct FDisSpawnInfo *)
//   0x6a0b50  public: void __thiscall ADishonoredSpawner::ClearPendingSpawns(void)
//   0x6a0c60  public: virtual unsigned int __thiscall ADishonoredSpawner::Tick(float, enum ELevelTick)
//   0x6a0f20  public: virtual void __thiscall ADishonoredSpawner::OnActorTerminated(void)
//   0x6a0f80  void __cdecl DisDebugPrintSquadSpawners(struct FDisSquadInfo const *)
//   0x6a1010  protected: virtual void __thiscall ADishonoredSpawner::OnSpawned(class ADishonoredNPCPawn *)
//   0x6a1290  public: virtual void __thiscall ADishonoredSpawner::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x6a1350  public: virtual void __thiscall ADishonoredSpawner::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6a1480  private: virtual void __thiscall ADishonoredSpawner::HandleNoiseHeard(struct FDisAINoiseInfo const &)
//   0x6a35d0  public: virtual void __thiscall ADishonoredSpawner::PostBeginPlay(void)
//   0x6a36e0  public: virtual void __thiscall ADishonoredSpawner::OnStartSpawn(class UDisSeqAct_StartSpawn *)
//   0x6a4a60  public: static class UClass * __cdecl ADishonoredSpawner::GetPrivateStaticClassADishonoredSpawner(wchar_t const *)
//   0x6a6c90  public: static class UClass * __cdecl ADishonoredSpawner::StaticClassNoInline(void)
//   0x6a8e00  public: void __thiscall ADishonoredSpawner::OnPawnDestroyed(class FArkGameEvent const &)
//   0x6aa3b0  public: virtual void __thiscall ADishonoredSpawner::ClearComponents(void)
//   0x6aa460  public: virtual enum eDisSpawnNowResult __thiscall ADishonoredSpawner::DoSpawnNow(struct FDisSpawnInfo const &)
