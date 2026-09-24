// DishonoredGame/src/disbehaviorgohome.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (40):
//   0x722fe0  public: static void __cdecl UDisTweaks_AIBehavior_GoHome::InitializePrivateStaticClassUDisTweaks_AIBehavior_GoHome(void)
//   0x723000  public: virtual void __thiscall UDisBehaviorGoHome::OnBehaviorPause(unsigned int)
//   0x723010  public: virtual unsigned char const * __thiscall UDisBehaviorGoHome::BuildEvaluateStimMask(void)const
//   0x723050  public: virtual unsigned char const * __thiscall UDisBehaviorGoHome::BuildFilterStimMask(void)const
//   0x723120  private: unsigned int __thiscall UDisBehaviorGoHome::FilterTetherRequest(struct FAIStimStruct_TetherRequest const &)
//   0x723150  private: unsigned int __thiscall UDisBehaviorGoHome::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x723180  private: unsigned int __thiscall UDisBehaviorGoHome::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x7231a0  private: unsigned int __thiscall UDisBehaviorGoHome::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x7231c0  private: virtual struct FDisBodyIntentionRequest * __thiscall UDisBehaviorGoHome::GetDesiresBodyIntentionRequest(void)
//   0x7231d0  public: virtual unsigned int __thiscall FD3D9DynamicRHI::GetTextureMemoryVisualizeData(class FColor *, int, int, int, int)
//   0x7231e0  public: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorGoHome::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x723660  public: virtual unsigned int __thiscall FFileManagerError::MakeDirectory(wchar_t const *, unsigned int)
//   0x723c00  private: unsigned int __thiscall UDisBehaviorGoHome::EvaluateTetherRequest(struct FAIStimStruct_TetherRequest const &)const
//   0x723c70  public: virtual struct FDynamicSpriteEmitterReplayDataBase const * __thiscall FDynamicSubUVEmitterData::GetSourceData(void)const
//   0x7282e0  public: virtual void __thiscall UDisBehaviorGoHome::RefreshThoughts(float)
//   0x728390  public: virtual enum EAIAwareness __thiscall UDisBehaviorGoHome::GetAwarenessLevel(void)const
//   0x7283e0  public: virtual unsigned char const * __thiscall UDisBehaviorGoHome::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x72b7e0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoHome::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x72df50  public: virtual void __thiscall UDisBehaviorGoHome::InitBehavior(class UDishonoredAIBrain * const)
//   0x72dfe0  private: void __thiscall UDisBehaviorGoHome::StartFocusingOnEnemy(void)
//   0x730800  public: virtual void __thiscall UDisBehaviorGoHome::OnEnterCallback_TakePosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7308d0  protected: void __thiscall UDisBehaviorGoHome::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x730920  protected: virtual void __thiscall UDisBehaviorGoHome::BeginDestroy(void)
//   0x730970  private: void __thiscall UDisBehaviorGoHome::RefreshEnemyFocus(void)
//   0x7326b0  private: void __thiscall UDisBehaviorGoHome::SetupFromTetherRequest(struct FAIStimStruct_TetherRequest const &)
//   0x732780  public: virtual void __thiscall UDisBehaviorGoHome::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7363d0  public: static class UClass * __cdecl UDisBehaviorGoHome::GetPrivateStaticClassUDisBehaviorGoHome(wchar_t const *)
//   0x736460  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorGoHome::GetSetupFromStimDelegate(enum EAIStimID)
//   0x738a00  public: static void __cdecl UDisBehaviorGoHome::InitializePrivateStaticClassUDisBehaviorGoHome(void)
//   0x739e10  public: static class UClass * __cdecl UDisBehaviorGoHome::StaticClassNoInline(void)
//   0x739e40  public: static class UClass * __cdecl UDisTweaks_AIBehavior_GoHome::GetPrivateStaticClassUDisTweaks_AIBehavior_GoHome(wchar_t const *)
//   0x73ad10  public: static class UClass * __cdecl UDisTweaks_AIBehavior_GoHome::StaticClassNoInline(void)
//   0x73d040  public: virtual void __thiscall UDisBehaviorGoHome::OnBehaviorStart(void)
//   0x73d170  public: virtual void __thiscall UDisBehaviorGoHome::RefreshCallback_TakePosition(class UDisAISubState *, float)
//   0x73d310  public: virtual void __thiscall UDisBehaviorGoHome::RequestStateExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x73d370  public: virtual void __thiscall UDisBehaviorGoHome::RefreshCallback_Menace(class UDisAISubState *, float)
//   0x73d4c0  public: virtual void __thiscall UDisBehaviorGoHome::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x73d540  private: unsigned int __thiscall UDisBehaviorGoHome::FilterCombatBegin(struct FAIStimStruct_CombatBegin const &)
//   0x73d6a0  private: virtual unsigned int __thiscall UDisBehaviorGoHome::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x742b20  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoHome::GetFilterStimDelegate(enum EAIStimID)
