// DishonoredGame/src/disbehaviorguard.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x723240  public: static void __cdecl UDisTweaks_AIBehavior_Guard::InitializePrivateStaticClassUDisTweaks_AIBehavior_Guard(void)
//   0x723260  public: virtual unsigned char const * __thiscall UDisBehaviorGuard::BuildEvaluateStimMask(void)const
//   0x7232a0  public: virtual unsigned char const * __thiscall UDisBehaviorGuard::BuildFilterStimMask(void)const
//   0x723300  private: unsigned int __thiscall UDisBehaviorGuard::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x728450  public: virtual unsigned char const * __thiscall UDisBehaviorGuard::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x7284c0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGuard::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x72b810  public: virtual void __thiscall UDisBehaviorGuard::OnBehaviorPause(unsigned int)
//   0x72b840  private: void __thiscall UDisBehaviorGuard::EnableAmbientActions(void)
//   0x72b8b0  private: void __thiscall UDisBehaviorGuard::DisableAmbientActions(void)
//   0x72e040  public: virtual void __thiscall UDisBehaviorGuard::InitBehavior(class UDishonoredAIBrain * const)
//   0x72e0c0  public: virtual void __thiscall UDisBehaviorGuard::OnExitCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7309c0  protected: void __thiscall UDisBehaviorGuard::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x730a10  protected: virtual void __thiscall UDisBehaviorGuard::BeginDestroy(void)
//   0x7327d0  private: void __thiscall UDisBehaviorGuard::SetupFromGuardRequest(struct FAIStimStruct_GuardRequest const &)
//   0x736490  public: static class UClass * __cdecl UDisBehaviorGuard::GetPrivateStaticClassUDisBehaviorGuard(wchar_t const *)
//   0x736520  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorGuard::GetSetupFromStimDelegate(enum EAIStimID)
//   0x738a20  public: static void __cdecl UDisBehaviorGuard::InitializePrivateStaticClassUDisBehaviorGuard(void)
//   0x739ed0  public: static class UClass * __cdecl UDisBehaviorGuard::StaticClassNoInline(void)
//   0x739f00  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Guard::GetPrivateStaticClassUDisTweaks_AIBehavior_Guard(wchar_t const *)
//   0x73ad40  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Guard::StaticClassNoInline(void)
//   0x73d720  public: virtual void __thiscall UDisBehaviorGuard::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73d8f0  public: virtual void __thiscall UDisBehaviorGuard::RefreshCallback_Stand(class UDisAISubState *, float)
//   0x740f70  private: void __thiscall UDisBehaviorGuard::EnsureProperLocation(void)
//   0x740fc0  public: virtual void __thiscall UDisBehaviorGuard::OnBehaviorResume(void)
//   0x741020  private: unsigned int __thiscall UDisBehaviorGuard::FilterEndPossession(struct FAIStimStruct_EndPossession const &)
//   0x741030  public: virtual void __thiscall UDisBehaviorGuard::RequestStateExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x743660  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGuard::GetFilterStimDelegate(enum EAIStimID)
