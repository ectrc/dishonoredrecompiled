// DishonoredGame/src/disconversationcomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (50):
//   0x8d9890  public: static void __cdecl UDisConversationComponent::InitializePrivateStaticClassUDisConversationComponent(void)
//   0x8d98b0  public: int __thiscall UDisConversationComponent::GetRunningInstID(class UDisConversationComponent::FDisConvComp_NPCOnlyKey const &)const
//   0x8d98c0  public: unsigned int __thiscall UDisConversationComponent::SupportsDialogHook(enum eDisDialogHook)const
//   0x8d98f0  public: void __thiscall UDisConversationComponent::SetLookTarget(class AActor *, enum eDisConvoLookType, unsigned int)
//   0x8d9930  public: void __thiscall UDisConversationComponent::Shutdown(void)
//   0x8d9940  ConvPrepareEventCB
//   0x8d9960  protected: void __thiscall UDisConversationComponent::UnprepareAkEvent(void)
//   0x8d99b0  public: virtual void __thiscall UDisConversationComponent::BeginDestroy(void)
//   0x8daaf0  public: void __thiscall UDisConversationComponent::SerializeForGameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8dab60  public: void __thiscall UDisConversationComponent::SerializeForGameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8dabd0  public: unsigned int __thiscall UDisConversationComponent::ShouldLowerAlertness(void)const
//   0x8dabf0  public: void __thiscall UDisConversationComponent::StreamDialogBlurb(struct FDisVoiceTextBlurb *)
//   0x8dac70  public: class UDisAkComponent * __thiscall UDisConversationComponent::StopAkEvents(class AActor *)
//   0x8de710  public: virtual void __thiscall UDisConversationComponent::Detach(unsigned int)
//   0x8de770  public: virtual void __thiscall UDisConversationComponent::BeginPlay(void)
//   0x8de7f0  public: class FString const __thiscall UDisConversationComponent::ExtractDialogHookText(enum eDisDialogHook, class IDisConvSpeakerInterface *)
//   0x8dea10  public: unsigned int __thiscall UDisConversationComponent::FireDialogHookNow(enum eDisDialogHook, int, class IDisConvSpeakerInterface *, enum eDisDialogCheckOnly *)
//   0x8debb0  public: enum eDisConvPriority __thiscall UDisConversationComponent::InConversation(unsigned int &, unsigned int *, int const *)const
//   0x8dec70  protected: unsigned int __thiscall UDisConversationComponent::SupportsDialogHook_Slow(enum eDisDialogHook)const
//   0x8ded80  public: void __thiscall UDisConversationComponent::BreakOutOfConversation(enum DisConvoEndReasonEnum, enum eDisDialogHookFireGroup)
//   0x8deed0  public: struct FDisVoiceTextBlurb * __thiscall UDisConversationComponent::FindTextBlurb(class UDisDialogTree const *, class FGuid const &, class UDisDialogVoiceData * &)
//   0x8df040  public: class AActor const * __thiscall UDisConversationComponent::GetLookTarget(enum eDisConvoLookType &)const
//   0x8df100  public: unsigned int __thiscall UDisConversationComponent::IsReadyToSpeak(void)const
//   0x8df160  public: unsigned int __thiscall UDisConversationComponent::InFallbackChain(class UDisDialogTree const *)const
//   0x8df270  public: virtual void __thiscall UDisConversationComponent::OnOtherActorTerminated(class AActor const &)
//   0x8e1a80  public: virtual void __thiscall UDisConversationComponent::Attach(void)
//   0x8e1b20  public: enum eDisDialogCheckOnly __thiscall UDisConversationComponent::CheckDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *)const
//   0x8e1ca0  protected: void __thiscall UDisConversationComponent::BuildDialogHookSupportedCache(void)
//   0x8e1d00  public: void __thiscall UDisConversationComponent::StartDialog(struct FDisDialogRunningInstance *, unsigned int, int)
//   0x8e3920  public: virtual void __thiscall UDisConversationComponent::Tick(float)
//   0x8e3c50  public: void __thiscall UDisConversationComponent::RemoveOneShot(struct FDisSpeakerOneShotInfo const &)
//   0x8e3d00  protected: class UDisDialogTree_InGameBind * __thiscall UDisConversationComponent::InstanceAndBindDialogTree(class UDisDialogVoiceData *)
//   0x8e5820  public: void __thiscall UDisConversationComponent::AddOneShotAfterSpawned(struct FDisSpeakerOneShotInfo &)
//   0x8e5890  public: unsigned int __thiscall UDisConversationComponent::IsAnyAkEventPlaying(void)const
//   0x8e8670  public: void __thiscall UDisConversationComponent::GetPsychicConvoPartners(struct TMemStackArray<class IDisConvSpeakerInterface *> &)const
//   0x8e8730  public: unsigned int __thiscall UDisConversationComponent::IsDonePlayingBlurb(void)const
//   0x8e88d0  public: enum eDisQueuedFiringGroupType __thiscall UDisConversationComponent::ClassifyGlobalFireGroup(class UDisConversationComponent::FDisConvComp_GlobalManKey const &)const
//   0x8ea8e0  public: void __thiscall UDisConversationComponent::PostSpawned(struct TMemStackArray<class UDisDialogVoiceData *> &, class TArray<struct FDisSpeakerOneShotInfo, class FDefaultAllocator> const &)
//   0x8ef700  public: void __thiscall UDisConversationComponent::StopDialogBlurb(void)
//   0x8f0850  public: void __thiscall UDisConversationComponent::EndDialog(struct FDisDialogRunningInstance *, enum DisConvoEndReasonEnum)
//   0x8f2210  public: struct FDisConvSaveData_DialogTree * __thiscall FDisConvSaveData::FindDialogData(class UDisDialogTree const *, unsigned int)
//   0x8f2390  public: struct FDisConvSaveData_Conversation * __thiscall FDisConvSaveData::FindConvData(class UDisDialogTree const *, class UDisConversation const *, unsigned int)
//   0x8f2520  public: struct FDisConvSaveData_Label * __thiscall FDisConvSaveData::FindLabelData(class UDisDialogTree const *, class UDisConversation const *, class FName const &, unsigned int)
//   0x8f52e0  public: void __thiscall UDisConversationComponent::FireDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *, unsigned int)
//   0x8f6ec0  public: static class UClass * __cdecl UDisConversationComponent::GetPrivateStaticClassUDisConversationComponent(wchar_t const *)
//   0x8f7270  public: static class UClass * __cdecl UDisConversationComponent::StaticClassNoInline(void)
//   0x8f7840  void __cdecl DisConvAkEventCB(enum AkCallbackType, struct AkCallbackInfo *)
//   0x8f7910  public: void __thiscall UDisConversationComponent::PlayDialogBlurb(class UDisDialogVoiceData *, struct FDisVoiceTextBlurb *, wchar_t const *, float, unsigned int, float, unsigned int, float)
//   0x8f7e30  void __cdecl DisFaceFxStartAudio(class UActorComponent *, class USkeletalMeshComponent *, float)
//   0xbaae50  _dynamic_initializer_for__UDisConversationComponent::ms_VoiceBoneName__
