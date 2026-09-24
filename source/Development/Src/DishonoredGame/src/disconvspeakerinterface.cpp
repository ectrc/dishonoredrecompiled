// DishonoredGame/src/disconvspeakerinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (33):
//   0x8f8670  public: static void __cdecl UDisConvSpeakerInterface::InitializePrivateStaticClassUDisConvSpeakerInterface(void)
//   0x8f8690  public: struct FDisHookParameters * __thiscall IDisConvSpeakerInterface::GetHookParameters(void)
//   0x8f93a0  public: class UDisConversationComponent * __thiscall IDisConvSpeakerInterface::GetConversationComponent(unsigned int)
//   0x8f93c0  public: void __thiscall IDisConvSpeakerInterface::GetPsychicConvoPartners(struct TMemStackArray<class IDisConvSpeakerInterface *> &)const
//   0x8f93e0  public: virtual enum eDisDialogAvailability __thiscall IDisConvSpeakerInterface::IsAvailableForDialog(enum eDisConvPriority, int)const
//   0x8f9450  public: virtual void __thiscall IDisConvSpeakerInterface::StartDialog(struct FDisDialogRunningInstance *, unsigned int, int, int)
//   0x8f9480  public: virtual void __thiscall IDisConvSpeakerInterface::EndDialog(struct FDisDialogRunningInstance *, enum DisConvoEndReasonEnum)
//   0x8f94a0  public: virtual struct FDisConvSaveData * __thiscall IDisConvSpeakerInterface::GetDialogSaveData(void)
//   0x8f94c0  public: virtual void __thiscall IDisConvSpeakerInterface::PostSpawned_Dialog(struct TMemStackArray<class UDisDialogVoiceData *> &, class TArray<struct FDisSpeakerOneShotInfo, class FDefaultAllocator> const &)
//   0x8f94e0  public: virtual void __thiscall IDisConvSpeakerInterface::RemoveOneShot(struct FDisSpeakerOneShotInfo const &)
//   0x8f9500  protected: virtual struct FDisHookParameters * __thiscall IDisConvSpeakerInterface::GetHookParameters_Derived(void)
//   0x8f9520  protected: virtual class FString * __thiscall IDisConvSpeakerInterface::GetDebugSpeechText_Derived(void)
//   0x8f9540  public: virtual void __thiscall IDisConvSpeakerInterface::FireDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *)
//   0x8f9570  public: virtual enum eDisDialogCheckOnly __thiscall IDisConvSpeakerInterface::CheckDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *)const
//   0x8f95a0  public: virtual void __thiscall IDisConvSpeakerInterface::FireDialogHookForAudioLog(enum eDisDialogHook, class IDisConvSpeakerInterface *)
//   0x8f95d0  public: virtual unsigned int __thiscall IDisConvSpeakerInterface::SupportsDialogHook(enum eDisDialogHook)const
//   0x8f9600  public: virtual enum eDisConvPriority __thiscall IDisConvSpeakerInterface::InConversation(unsigned int &, int const *)const
//   0x8f9640  public: virtual void __thiscall IDisConvSpeakerInterface::BreakOutOfConversation(enum DisConvoEndReasonEnum, enum eDisDialogHookFireGroup)
//   0x8f9660  public: virtual void __thiscall IDisConvSpeakerInterface::StreamDialogBlurb(struct FDisVoiceTextBlurb *)
//   0x8f9680  public: virtual void __thiscall IDisConvSpeakerInterface::PlayDialogBlurb(class UDisDialogVoiceData *, struct FDisVoiceTextBlurb *, wchar_t const *, float, unsigned int, float, unsigned int, float)
//   0x8f96d0  public: virtual void __thiscall IDisConvSpeakerInterface::StopDialogBlurb(void)
//   0x8f96f0  public: virtual struct FDisVoiceTextBlurb * __thiscall IDisConvSpeakerInterface::FindTextBlurb(class UDisDialogTree const *, class FGuid const &, class UDisDialogVoiceData * &)
//   0x8f9720  public: virtual unsigned int __thiscall IDisConvSpeakerInterface::SupportsDialogTree(class UDisDialogTree const *)const
//   0x8f9750  public: virtual class AActor const * __thiscall IDisConvSpeakerInterface::GetConvLookTarget(enum eDisConvoLookType &)const
//   0x8f9790  public: virtual void __thiscall IDisConvSpeakerInterface::SetLookTarget(class AActor *, enum eDisConvoLookType, unsigned int)
//   0x8f97b0  public: virtual unsigned int __thiscall IDisConvSpeakerInterface::IsReadyToSpeak(void)const
//   0x8f97d0  public: virtual unsigned int __thiscall IDisConvSpeakerInterface::IsDonePlayingBlurb(void)const
//   0x8faa90  public: static class UClass * __cdecl UDisConvSpeakerInterface::GetPrivateStaticClassUDisConvSpeakerInterface(wchar_t const *)
//   0x8fab20  public: void __thiscall IDisConvSpeakerInterface::PostShutdown_Dialog(void)
//   0x8fab70  public: void __thiscall IDisConvSpeakerInterface::GameSave_Dialog(class FArchive &, enum ESaveLoadLocation)
//   0x8fabf0  public: void __thiscall IDisConvSpeakerInterface::GameLoad_Dialog(class FArchive &, enum ESaveLoadLocation)
//   0x8fd300  public: static class UClass * __cdecl UDisConvSpeakerInterface::StaticClassNoInline(void)
//   0x8fd330  public: class FString const __thiscall IDisConvSpeakerInterface::ExtractDialogHookText(enum eDisDialogHook, class IDisConvSpeakerInterface *)
