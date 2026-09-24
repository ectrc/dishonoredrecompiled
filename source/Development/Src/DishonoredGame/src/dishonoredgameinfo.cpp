// DishonoredGame/src/dishonoredgameinfo.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (67):
//   0x62f7a0  public: static void __cdecl ADishonoredGameInfo::InitializePrivateStaticClassADishonoredGameInfo(void)
//   0x62f7c0  public: static void __cdecl UDisGameplayEventsWriter::InitializePrivateStaticClassUDisGameplayEventsWriter(void)
//   0x62f7e0  public: static void __cdecl UDisTweaks_ChapterInfo::InitializePrivateStaticClassUDisTweaks_ChapterInfo(void)
//   0x62f800  public: static void __cdecl UDisTweaks_ChapterInfoList::InitializePrivateStaticClassUDisTweaks_ChapterInfoList(void)
//   0x62f820  public: static void __cdecl UDisTweaks_ChapterTarget::InitializePrivateStaticClassUDisTweaks_ChapterTarget(void)
//   0x62f840  public: virtual void __thiscall ADishonoredGameInfo::OnOtherActorTerminated(class AActor const &)
//   0x62f8b0  public: void __thiscall ADishonoredGameInfo::InitGlobalManagers(void)
//   0x62f950  public: void __thiscall ADishonoredGameInfo::TermGlobalManagers(void)
//   0x62fa30  public: virtual void __thiscall ADishonoredGameInfo::BeginDestroy(void)
//   0x62fa40  public: virtual void __thiscall ADishonoredGameInfo::SetupPathfindingParams(struct FNavMeshPathParams &)const
//   0x62fa60  public: void __thiscall ADishonoredGameInfo::GetBendTime(enum EBendTimeChannel, float &, float &, float &)
//   0x62fa90  public: void __thiscall ADishonoredGameInfo::SetBendTimeChannelExclusive(enum EBendTimeChannel, unsigned int)
//   0x62fab0  public: float __thiscall ADishonoredGameInfo::GetBendTimePowerRealTimeSeconds(void)const
//   0x62fac0  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeDilation(unsigned int)const
//   0x62fb00  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeInputDilation(void)const
//   0x62fb10  public: virtual unsigned int __thiscall ADishonoredGameInfo::IsBendTimeOn(void)const
//   0x62fb40  public: virtual unsigned int __thiscall ADishonoredGameInfo::IsBendTimeFrozen(void)const
//   0x62fb70  public: virtual unsigned int __thiscall ADishonoredGameInfo::CanStartMatch(void)
//   0x62fb90  public: class UDisGameplayEventsWriter * __thiscall ADishonoredGameInfo::GetGameplayEventsWriter(void)const
//   0x62fba0  public: void __thiscall ADishonoredGameInfo::SetDifficulty(enum EDifficulty)
//   0x62fbf0  public: enum EDifficulty __thiscall ADishonoredGameInfo::GetDifficulty(void)const
//   0x62fc00  public: void __thiscall ADishonoredGameInfo::EnableGore(unsigned int)
//   0x62fc20  public: unsigned int __thiscall ADishonoredGameInfo::IsGoreEnabled(void)const
//   0x62fc30  public: unsigned int __thiscall ADishonoredGameInfo::IsGameOverPending(void)const
//   0x62fc40  public: virtual class UGameCrowdPopulationManager * __thiscall ADishonoredGameInfo::GetCrowdPopulationManager(void)
//   0x62fc50  public: virtual void __thiscall ADishonoredGameInfo::FlushCrowdAgents(void)
//   0x63fab0  private: void __thiscall ADishonoredGameInfo::Tick_PendingGameOver(float)
//   0x63fb20  public: virtual void __thiscall ADishonoredGameInfo::GameEnding(void)
//   0x63fc00  public: virtual void __thiscall ADishonoredGameInfo::OnCleanupWorld(void)
//   0x63fc10  public: virtual void __thiscall ADishonoredGameInfo::PostBeginPlay(void)
//   0x63fcf0  public: void __thiscall ADishonoredGameInfo::BendTime(enum EBendTimeChannel, float, float, float, float, enum EBendTimeEffectType)
//   0x63fe30  public: virtual void __thiscall ADishonoredGameInfo::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x640020  public: virtual void __thiscall ADishonoredGameInfo::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x640240  public: struct FDisChapterTarget const * __thiscall ADishonoredGameInfo::GetCurrentChapterTarget(int)const
//   0x640270  public: void __thiscall ADishonoredGameInfo::GameOver(class FString const &, float)
//   0x642b50  public: class UDisTweaks_PlayerPawn * __thiscall ADishonoredGameInfo::LoadDefaultPlayerTweaks(unsigned int)const
//   0x642c40  public: virtual class APawn * __thiscall ADishonoredGameInfo::SpawnPlayer(class UClass *, class FVector, class FRotator)
//   0x642c90  public: void __thiscall ADishonoredGameInfo::TickBendTimeParticleEffects(float, float)
//   0x642db0  public: virtual void __thiscall ADishonoredGameInfo::TickBendTime(float)
//   0x6432d0  public: class UDisTweaks_ChapterInfo const * __thiscall ADishonoredGameInfo::GetCurrentChapter(void)const
//   0x6483f0  public: void __thiscall ADishonoredGameInfo::Tick_PendingDestroyList(void)
//   0x6484a0  public: void __thiscall ADishonoredGameInfo::AddToPendingDestroyList(class AActor *)
//   0x6484c0  public: void __thiscall ADishonoredGameInfo::SetCurrentChapter(class FName)
//   0x6486f0  public: void __thiscall ADishonoredGameInfo::OnTargetNotification(class UDisSeqAct_ShowTargetNotification *)
//   0x64b480  public: virtual unsigned int __thiscall ADishonoredGameInfo::Tick(float, enum ELevelTick)
//   0x64fee0  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeDuration(class AActor const *)const
//   0x6534a0  public: virtual void __thiscall ADishonoredGameInfo::CancelAutoReturnToBendTime(class AActor *)
//   0x6534b0  public: void __thiscall ADishonoredGameInfo::ReallowAutoReturnToBendTime(class AActor *)
//   0x654c60  public: static class UClass * __cdecl UDisGameplayEventsWriter::GetPrivateStaticClassUDisGameplayEventsWriter(wchar_t const *)
//   0x654cf0  public: static class UClass * __cdecl UDisTweaks_ChapterInfo::GetPrivateStaticClassUDisTweaks_ChapterInfo(wchar_t const *)
//   0x654d80  public: static class UClass * __cdecl UDisTweaks_ChapterInfoList::GetPrivateStaticClassUDisTweaks_ChapterInfoList(wchar_t const *)
//   0x654e10  public: static class UClass * __cdecl UDisTweaks_ChapterTarget::GetPrivateStaticClassUDisTweaks_ChapterTarget(wchar_t const *)
//   0x655ae0  public: static class UClass * __cdecl UDisGameplayEventsWriter::StaticClassNoInline(void)
//   0x655b10  public: static class UClass * __cdecl UDisTweaks_ChapterInfo::StaticClassNoInline(void)
//   0x655b40  public: static class UClass * __cdecl UDisTweaks_ChapterInfoList::StaticClassNoInline(void)
//   0x655b70  public: static class UClass * __cdecl UDisTweaks_ChapterTarget::StaticClassNoInline(void)
//   0x655ba0  public: virtual void __thiscall ADishonoredGameInfo::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x655ca0  public: virtual void __thiscall ADishonoredGameInfo::Serialize(class FArchive &)
//   0x655cd0  public: virtual void __thiscall ADishonoredGameInfo::PostTickBendTime(float)
//   0x655e00  public: virtual void __thiscall ADishonoredGameInfo::AutoReturnToBendTime(class AActor *, float, unsigned int)
//   0x655ec0  public: void __thiscall ADishonoredGameInfo::DisallowAutoReturnToBendTime(class AActor *)
//   0x658510  public: static class UClass * __cdecl ADishonoredGameInfo::GetPrivateStaticClassADishonoredGameInfo(wchar_t const *)
//   0x65d860  public: static class UClass * __cdecl ADishonoredGameInfo::StaticClassNoInline(void)
//   0x65f400  public: virtual void __thiscall ADishonoredGameInfo::PostCommitMapChange(void)
//   0x65f500  public: virtual void __thiscall ADishonoredGameInfo::PostGameLoad(enum ESaveLoadLocation)
//   0x65f570  public: void __thiscall ADishonoredGameInfo::SetCurrentMission(int)
//   0x661260  public: virtual void __thiscall ADishonoredGameInfo::PreCommitMapChange(class FString const &, class FString const &)
