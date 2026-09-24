// DishonoredGame/src/disbehaviorinteract.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (39):
//   0x7233c0  public: static void __cdecl UDisTweaks_AIBehavior_Interact::InitializePrivateStaticClassUDisTweaks_AIBehavior_Interact(void)
//   0x7233e0  private: virtual void __thiscall UDisBehaviorInteract::OnPostGameLoad(unsigned int, unsigned int)
//   0x723400  private: virtual unsigned char const * __thiscall UDisBehaviorInteract::BuildFilterStimMask(void)const
//   0x7234c0  private: unsigned int __thiscall UDisBehaviorInteract::FilterEndDistracted(struct FAIStimStruct_EndDistracted const &)
//   0x7234e0  private: unsigned int __thiscall UDisBehaviorInteract::FilterInterruptingStim(struct FAIStimStruct const &)
//   0x723500  private: unsigned int __thiscall UDisBehaviorInteract::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x723540  private: unsigned int __thiscall UDisBehaviorInteract::FilterTopAttnProxyUpdated(struct FAIStimStruct_TopAttnProxyUpdated const &)
//   0x723570  private: virtual unsigned char const * __thiscall UDisBehaviorInteract::BuildEvaluateStimMask(void)const
//   0x7235c0  private: unsigned int __thiscall UDisBehaviorInteract::EvaluateDistracted_Anim(struct FAIStimStruct_Distracted_Anim const &)const
//   0x723620  private: virtual enum EAIAwareness __thiscall UDisBehaviorInteract::GetAwarenessLevel(void)const
//   0x723640  private: virtual struct FDisFaceToRequest * __thiscall UDisBehaviorInteract::GetDesiresFaceToRequest(void)
//   0x723650  private: virtual struct FDisLookAtRequest * __thiscall UDisBehaviorSearch::GetDesiresLookAtRequest(void)
//   0x723f60  private: unsigned int __thiscall UDisBehaviorRatStomp::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x724180  private: unsigned int __thiscall UDisBehaviorMagicResponse::FilterSoiree(struct FAIStimStruct_Soiree const &)
//   0x7285d0  private: void __thiscall UDisBehaviorInteract::EndCurrentInteraction(void)
//   0x728630  private: virtual void __thiscall UDisBehaviorInteract::OnBehaviorStop(unsigned int)
//   0x728640  private: virtual void __thiscall UDisBehaviorInteract::OnBehaviorPause(unsigned int)
//   0x7286a0  private: virtual unsigned char const * __thiscall UDisBehaviorInteract::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728710  public: virtual void __thiscall UDisBehaviorInteract::OnEnterCallback_TakePosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x728760  public: virtual void __thiscall UDisBehaviorInteract::OnExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x7287c0  public: virtual void __thiscall UDisBehaviorInteract::OnExitCallback_Stand(class UDishonoredNativeState *)
//   0x72b9d0  public: void __thiscall FDisInteractInfo::SetupFromDistractedStim(struct FAIStimStruct_Distracted_Anim const &, class UDishonoredAIBrain *)
//   0x72ba60  private: unsigned int __thiscall UDisBehaviorInteract::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x72bab0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorInteract::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x72bae0  private: void __thiscall UDisBehaviorInteract::SetupFromDistracted_Anim(struct FAIStimStruct_Distracted_Anim const &)
//   0x72bb00  private: virtual unsigned int __thiscall UDisBehaviorInteract::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x72e100  private: virtual void __thiscall UDisBehaviorInteract::InitBehavior(class UDishonoredAIBrain * const)
//   0x730a50  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorInteract::GetSetupFromStimDelegate(enum EAIStimID)
//   0x7365e0  public: static class UClass * __cdecl UDisBehaviorInteract::GetPrivateStaticClassUDisBehaviorInteract(wchar_t const *)
//   0x738a60  public: static void __cdecl UDisBehaviorInteract::InitializePrivateStaticClassUDisBehaviorInteract(void)
//   0x73a050  public: static class UClass * __cdecl UDisBehaviorInteract::StaticClassNoInline(void)
//   0x73a080  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Interact::GetPrivateStaticClassUDisTweaks_AIBehavior_Interact(wchar_t const *)
//   0x73ada0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Interact::StaticClassNoInline(void)
//   0x741110  private: void __thiscall UDisBehaviorInteract::StartNewInteraction(void)
//   0x7411d0  private: virtual void __thiscall UDisBehaviorInteract::OnBehaviorStart(void)
//   0x741210  private: unsigned int __thiscall UDisBehaviorInteract::FilterInteractBegin(struct FAIStimStruct_InteractBegin const &)
//   0x7412c0  public: virtual void __thiscall UDisBehaviorInteract::RequestStateExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x741340  public: virtual void __thiscall UDisBehaviorInteract::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x7436e0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorInteract::GetFilterStimDelegate(enum EAIStimID)
