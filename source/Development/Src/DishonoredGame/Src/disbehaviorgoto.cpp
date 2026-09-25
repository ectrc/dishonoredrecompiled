// DishonoredGame/src/disbehaviorgoto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x745480  private: virtual enum eDisDialogAvailability __thiscall UDisBehaviorGoTo::GetDialogAvailability(enum eDisConvPriority, int)const
//   0x7454a0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterInterruptStim(struct FAIStimStruct const &)
//   0x7454b0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterTouchedAlly(struct FAIStimStruct_TouchedAlly const &)
//   0x7454e0  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildShouldFinishWhileDormantStimMask(void)const
//   0x745510  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildEvaluateStimMask(void)const
//   0x746bc0  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildFilterStimMask(void)const
//   0x746cb0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterKismetAIAction(struct FAIStimStruct const &)
//   0x748e60  private: unsigned int __thiscall UDisBehaviorGoTo::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x748e90  private: virtual unsigned int __thiscall UDisBehaviorGoTo::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x74c7a0  private: virtual void __thiscall UDisBehaviorGoTo::InitBehavior(class UDishonoredAIBrain * const)
//   0x74c7d0  private: virtual void __thiscall UDisBehaviorGoTo::OnBehaviorStart(void)
//   0x74c850  private: virtual unsigned char const * __thiscall UDisBehaviorGoTo::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x756e80  public: static class UClass * __cdecl UDisBehaviorGoTo::GetPrivateStaticClassUDisBehaviorGoTo(wchar_t const *)
//   0x75aca0  public: static void __cdecl UDisBehaviorGoTo::InitializePrivateStaticClassUDisBehaviorGoTo(void)
//   0x75acc0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TakePosition::GetPrivateStaticClassUDisTweaks_AIBehavior_TakePosition(wchar_t const *)
//   0x75bc50  public: static class UClass * __cdecl UDisBehaviorGoTo::StaticClassNoInline(void)
//   0x75bc80  public: static void __cdecl UDisTweaks_AIBehavior_TakePosition::InitializePrivateStaticClassUDisTweaks_AIBehavior_TakePosition(void)
//   0x75c120  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TakePosition::StaticClassNoInline(void)
//   0x75c150  private: unsigned int __thiscall UDisBehaviorGoTo::FilterBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)
//   0x75c190  private: unsigned int __thiscall UDisBehaviorGoTo::ShouldFinishWhileDormantBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)const
//   0x75d0f0  private: unsigned int __thiscall UDisBehaviorGoTo::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x75d250  private: unsigned int __thiscall UDisBehaviorGoTo::ShouldFinishWhileDormantGoToRequest(struct FAIStimStruct_GoToRequest const &)const
//   0x75d2a0  private: unsigned int __thiscall UDisBehaviorGoTo::EvaluateGoToRequest(struct FAIStimStruct_GoToRequest const &)const
//   0x75d2f0  private: void __thiscall UDisBehaviorGoTo::SetupFromGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x760eb0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetFilterStimDelegate(enum EAIStimID)
//   0x761040  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x761080  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x7610b0  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorGoTo::GetSetupFromStimDelegate(enum EAIStimID)
