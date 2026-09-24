// DishonoredGame/src/disaisubstateinvestigate.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x765530  private: virtual unsigned char const * __thiscall UDisAISubStateInvestigate::BuildFilterStimMask(void)const
//   0x765570  private: unsigned int __thiscall UDisAISubStateInvestigate::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x765590  private: unsigned int __thiscall UDisAISubStateInvestigate::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x7655d0  public: unsigned int __thiscall UDisAISubStateInvestigate::ReachedProxy(void)const
//   0x7655e0  public: unsigned int __thiscall UDisAISubStateInvestigate::IsRunningInvestigate(void)const
//   0x7655f0  public: unsigned int __thiscall UDisAISubStateInvestigate::WantsStareAtUnreachable(void)const
//   0x765600  public: class IDisCorpseInterface * __thiscall UDisAISubStateInvestigate::GetCorpseBeingInvestigated(void)const
//   0x765610  public: class FArkComponentPostAsyncWorkPolicy & __thiscall FArkComponentManager::GetPolicy<class FArkComponentPostAsyncWorkPolicy>(void)
//   0x765620  private: virtual struct FDisLocoRequest * __thiscall UDisAISubStateInvestigate::GetDesiresLocoRequest(void)
//   0x765630  private: virtual struct FDisLookAtRequest * __thiscall UDisAISubStateInvestigate::GetDesiresLookAtRequest(void)
//   0x765b10  public: enum EDisAttentionChangeReasonType __thiscall UDisAISubStateInvestigate::GetCurrentInvestigateReason(void)const
//   0x768e70  public: virtual void __thiscall FDisAISubStateInvestigate_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x768f10  private: void __thiscall UDisAISubStateInvestigate::RefreshInvestigation(void)
//   0x769010  private: void __thiscall UDisAISubStateInvestigate::CheckDoBark(float)
//   0x7690a0  private: unsigned int __thiscall UDisAISubStateInvestigate::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x770e60  private: virtual void __thiscall UDisAISubStateInvestigate::ResumeSubState_Derived(void)
//   0x770f20  private: virtual void __thiscall UDisAISubStateInvestigate::PauseSubState_Derived(unsigned int)
//   0x770f60  private: virtual void __thiscall UDisAISubStateInvestigate::RefreshSubState(float)
//   0x770fc0  private: unsigned int __thiscall UDisAISubStateInvestigate::FilterSearchReachedProxy(struct FAIStimStruct_SearchReachedProxy const &)
//   0x7758b0  public: static class UClass * __cdecl UDisAISubStateInvestigate::GetPrivateStaticClassUDisAISubStateInvestigate(wchar_t const *)
//   0x775940  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAISubStateInvestigate::GetFilterStimDelegate_SubState(enum EAIStimID)
//   0x77bca0  public: static void __cdecl UDisAISubStateInvestigate::InitializePrivateStaticClassUDisAISubStateInvestigate(void)
//   0x780310  public: static class UClass * __cdecl UDisAISubStateInvestigate::StaticClassNoInline(void)
//   0x781d60  public: __thiscall FDisAISubStateInvestigate_Param::FDisAISubStateInvestigate_Param(struct FDisAttentionProxy const &, unsigned int, struct FDisAttentionChangeReason, enum eDisHookInvestigate, float, float)
//   0x783520  public: static class UClass * __cdecl UDisTweaks_AISubState_Investigate::GetPrivateStaticClassUDisTweaks_AISubState_Investigate(wchar_t const *)
//   0x784620  public: static void __cdecl UDisTweaks_AISubState_Investigate::InitializePrivateStaticClassUDisTweaks_AISubState_Investigate(void)
//   0x784ed0  public: static class UClass * __cdecl UDisTweaks_AISubState_Investigate::StaticClassNoInline(void)
//   0x787f20  private: float __thiscall UDisAISubStateInvestigate::GetProximityThreshold(void)const
//   0x78b700  private: virtual void __thiscall UDisAISubStateInvestigate::TickState(float)
