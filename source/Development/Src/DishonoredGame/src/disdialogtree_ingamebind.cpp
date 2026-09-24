// DishonoredGame/src/disdialogtree_ingamebind.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x8f8750  public: static void __cdecl UDisDialogTree_InGameBind::InitializePrivateStaticClassUDisDialogTree_InGameBind(void)
//   0x8f8770  public: unsigned int __thiscall FDisDialogRunningInstance::HasDialogStarted(void)const
//   0x8fae70  public: void __thiscall FDisDialogRunningInstance::StartInstance(enum eDisDialogHook, class IDisConvSpeakerInterface *, int, unsigned int)
//   0x8faed0  protected: struct FDisDialogRunningObjs & __thiscall FDisDialogRunningInstance::GetRunningObjects(void)
//   0x8faf20  public: class UDisDialogTree const * __thiscall FDisDialogRunningInstance::GetDialogTree(void)const
//   0x8faf70  public: virtual void __thiscall UDisDialogTree_InGameBind::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8fd530  public: void __thiscall FDisDialogRunningInstance::NotifyWhenDialogStarts(class UDisConv_Node const *)
//   0x900b20  public: void __thiscall FDisDialogRunningInstance::StartDialog(class UDisConversation_InGameData *)
//   0x902b80  public: class UDisConv_Node_InGameData * __thiscall FDisDialogRunningInstance::GetInGameNode(class UDisConv_Node const *, enum eDisConvInGameDataType)
//   0x902d20  private: void __thiscall UDisDialogTree_InGameBind::ChangeAttentionForSpeaker(class IDisConvSpeakerInterface *, class IDisConvSpeakerInterface *, enum EDisAttentionLevel)
//   0x905e80  public: class UDisConversation_InGameData * __thiscall FDisDialogRunningInstance::GetInGameConv(class UDisConv_Node const *)
//   0x905f40  private: unsigned int __thiscall UDisDialogTree_InGameBind::PlayFromNode(class UDisConv_Node const *, struct FDisDialogRunningInstance *, enum DisConvoEndReasonEnum &, int)
//   0x90a010  void __cdecl BuildFallbackChain(class UDisDialogTree_InGameBind *, class UDisDialogTree const *, class TArrayNoInit<struct FDisDialogRunningObjs> &, class UClass *, enum eDisConvInGameDataType)
//   0x90a290  public: void __thiscall FDisDialogRunningInstance::StopInstance(enum DisConvoEndReasonEnum)
//   0x90a490  public: virtual void __thiscall UDisDialogTree_InGameBind::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x90a550  public: void __thiscall UDisDialogTree_InGameBind::OnOtherActorTerminated(class AActor const &)
//   0x90a5c0  private: unsigned int __thiscall UDisDialogTree_InGameBind::FireDialogHook_Helper(enum eDisDialogHook, int, struct FDisDialogRunningInstance *, unsigned int &, enum DisConvoEndReasonEnum &, unsigned int)
//   0x90a6a0  public: void __thiscall UDisDialogTree_InGameBind::TickDialogTree(float)
//   0x90b3e0  private: unsigned int __thiscall UDisDialogTree_InGameBind::ExtractTextStartingAtNode(class UDisConv_Node const *, struct FDisDialogRunningInstance *, enum DisConvoEndReasonEnum &, class FString &)
//   0x90d770  void __cdecl BuildSingletonData_Conv(class UDisDialogTree_InGameBind *, class UDisConversation const *)
//   0x90d8b0  void __cdecl BuildSingletonData_Tree(class UDisDialogTree_InGameBind *, class UDisDialogTree const *)
//   0x90d940  public: void __thiscall UDisDialogTree_InGameBind::InitAndBind(class UDisDialogTree const *, class IDisConvSpeakerInterface *)
//   0x90da50  private: unsigned int __thiscall UDisDialogTree_InGameBind::ExtractDialogHookText_Helper(enum eDisDialogHook, int, struct FDisDialogRunningInstance *, unsigned int &, class FString &, enum DisConvoEndReasonEnum &)
//   0x90f1c0  public: static class UClass * __cdecl UDisDialogTree_InGameBind::GetPrivateStaticClassUDisDialogTree_InGameBind(wchar_t const *)
//   0x914500  public: static class UClass * __cdecl UDisDialogTree_InGameBind::StaticClassNoInline(void)
//   0x914530  public: void __thiscall FDisDialogRunningInstance::FiredFromHookName(class FString &)const
//   0x9145a0  public: unsigned int __thiscall UDisDialogTree_InGameBind::FireDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *, int, unsigned int &, enum eDisDialogCheckOnly *)
//   0x914800  public: enum eDisDialogAvailability __thiscall UDisDialogTree_InGameBind::IsSpeakerAvailableForDialog(class IDisConvSpeakerInterface const *, enum eDisConvPriority, enum eDisDialogHook, int)const
//   0x914940  public: unsigned int __thiscall UDisDialogTree_InGameBind::EvaluateDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *, int &, unsigned int)
//   0x914b20  public: class FString const __thiscall UDisDialogTree_InGameBind::ExtractDialogHookText(enum eDisDialogHook, class IDisConvSpeakerInterface *)
