// DishonoredGame/src/dishonoredengine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (61):
//   0x62b950  public: static void __cdecl UDishonoredEngine::InitializePrivateStaticClassUDishonoredEngine(void)
//   0x62b970  public: void __thiscall FDisAsyncSaveGameLister::Wait(void)
//   0x62b9a0  public: unsigned int __thiscall UDishonoredEngine::IsSavingAllowed(void)const
//   0x62b9c0  public: unsigned int __thiscall UDishonoredEngine::IsSaving(void)const
//   0x62b9e0  public: virtual void __thiscall UDishonoredEngine::OpenControllerConnectionMenu(void)const
//   0x62ba10  public: void __thiscall UDishonoredEngine::CancelMissionStartSave(class UDisSeqAct_AutoSave *)
//   0x62ba40  public: void __thiscall UDishonoredEngine::ResetPeriodicAutosave(void)
//   0x62ba80  public: virtual void __thiscall UDishonoredEngine::PushIgnoreAutosave(unsigned char)
//   0x62baa0  public: virtual void __thiscall UDishonoredEngine::PopIgnoreAutosave(unsigned char)
//   0x62bac0  public: virtual unsigned int __thiscall UDishonoredEngine::Exec(wchar_t const *, class FOutputDevice &)
//   0x62bb00  public: void __thiscall UDishonoredEngine::RequestLevelStateSaving(unsigned int)
//   0x62bb30  public: void __thiscall UDishonoredEngine::SetSaveLoadEnabled(unsigned int)
//   0x62bb50  public: unsigned int __thiscall UDishonoredEngine::IsSaveLoadEnabled(void)const
//   0x62bb60  public: unsigned int __thiscall UDishonoredEngine::IsSaveGameListReady(void)const
//   0x62bb80  public: void __thiscall UDishonoredEngine::WaitSaveGameListReady(void)const
//   0x62bb90  public: virtual unsigned int __thiscall UDishonoredEngine::IsLoadingGame(void)const
//   0x62bbb0  public: virtual unsigned int __thiscall UDishonoredEngine::IsLoadingLevelState(void)const
//   0x62bbc0  public: virtual void __thiscall UDishonoredEngine::PreExit(void)
//   0x62bc30  protected: virtual void __thiscall UDishonoredEngine::StopMovie(unsigned int)
//   0x63ed20  GetSaveGameSlot
//   0x641fc0  public: __thiscall FDisAsyncSaveGameDeleter::FDisAsyncSaveGameDeleter(wchar_t const *)
//   0x642090  public: int __thiscall UDishonoredEngine::GetNextAutoSaveSlot(void)
//   0x6421c0  public: int __thiscall UDishonoredEngine::GetNextUserSaveSlot(void)const
//   0x6422b0  public: virtual void __thiscall UDishonoredEngine::OpenPauseMenu(void)
//   0x642490  public: virtual void __thiscall UDishonoredEngine::OnControllerDisconnected(int)
//   0x642540  public: unsigned int __thiscall UDishonoredEngine::HasSaveGame(int)const
//   0x6425f0  public: struct FDisSaveGame * __thiscall UDishonoredEngine::GetSaveGame(int)const
//   0x642660  public: void __thiscall UDishonoredEngine::DeleteSaveGame(int)
//   0x642750  public: virtual void __thiscall UDishonoredEngine::NotifyActorDestroyed(class AActor *)
//   0x6427d0  public: struct FMapConfig * __thiscall UDishonoredEngine::FindMapConfig(class FString const &)
//   0x642900  public: struct FMapConfig * __thiscall UDishonoredEngine::FindMapConfigFromFriendlyName(class FString const &)
//   0x648180  public: virtual void __thiscall FDisAsyncSaveGameDeleter::DoWork(void)
//   0x6482b0  public: virtual void __thiscall UDishonoredEngine::PopDisableSave(unsigned char, float)
//   0x648300  public: unsigned int __thiscall UDishonoredEngine::IsObjectPartOfSavedLevelState(class UObject *)const
//   0x648310  public: virtual unsigned int __thiscall UDishonoredEngine::PlayLoadMapMovie(class FString const &, class FString const &)
//   0x64cca0  public: void __thiscall UDishonoredEngine::CancelPendingAutosave(class UDisSeqAct_AutoSave *)
//   0x64ccf0  public: void __thiscall UDishonoredEngine::CancelAllPendingAutosaves(void)
//   0x64cd80  public: virtual void __thiscall UDishonoredEngine::Dis_Load(int)
//   0x64ce40  public: virtual void __thiscall UDishonoredEngine::PushDisableSave(unsigned char)
//   0x64cf10  public: virtual unsigned int __thiscall UDishonoredEngine::LoadMap(struct FURL const &, class UPendingLevel *, class FString &)
//   0x64f3b0  public: virtual void __thiscall FDisAsyncSaveGameLister::DoWork(void)
//   0x64f8c0  private: void __thiscall UDishonoredEngine::DoSaveGame(int, struct TMemStackArray<class UDisSeqAct_AutoSave *> const &)
//   0x64f960  private: void __thiscall UDishonoredEngine::TryDoMissionStartSave(void)
//   0x64fa60  public: void __thiscall UDishonoredEngine::QueueMissionStartSave(int, class UDisSeqAct_AutoSave *)
//   0x64fab0  public: void __thiscall UDishonoredEngine::RefreshSaveGameList(void)
//   0x650fb0  private: void __thiscall UDishonoredEngine::ProcessSaveComplete(unsigned int)
//   0x6510c0  public: virtual void __thiscall UDishonoredEngine::Dis_Save(int)
//   0x653070  private: void __thiscall UDishonoredEngine::TryDoAutosave(void)
//   0x654ac0  public: void __thiscall UDishonoredEngine::QueueAutosave(class UDisSeqAct_AutoSave *)
//   0x654b40  public: void __thiscall UDishonoredEngine::PeriodicAutosave(float)
//   0x657160  public: static class UClass * __cdecl UDishonoredEngine::GetPrivateStaticClassUDishonoredEngine(wchar_t const *)
//   0x6584e0  public: static class UClass * __cdecl UDishonoredEngine::StaticClassNoInline(void)
//   0x65c990  private: void __thiscall UDishonoredEngine::WriteSaveGame(void)
//   0x65cb10  private: void __thiscall UDishonoredEngine::LoadGame(void)
//   0x65d250  public: void __thiscall UDishonoredEngine::DiscardLevelState(class FName const &)
//   0x65d280  public: virtual void __thiscall UDishonoredEngine::Init(void)
//   0x65d5d0  public: virtual void __thiscall UDishonoredEngine::PreCommitMapChange(void)
//   0x65d6b0  public: virtual void __thiscall UDishonoredEngine::PostCommitMapChange(void)
//   0x65f100  private: void __thiscall UDishonoredEngine::ProcessSaveLoadCmd(float &)
//   0x661150  public: virtual void __thiscall UDishonoredEngine::Tick(float)
//   0xba34b0  _dynamic_initializer_for__gs_SaveFileError__
