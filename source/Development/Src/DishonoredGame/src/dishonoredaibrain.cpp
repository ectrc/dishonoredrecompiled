// DishonoredGame/src/dishonoredaibrain.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x745eb0  public: static void __cdecl UDishonoredAIBrain::InitializePrivateStaticClassUDishonoredAIBrain(void)
//   0x745ed0  public: virtual class UDisTweaksBase * __thiscall UDishonoredAIBrain::GetTweaks_Derived(void)
//   0x745ee0  public: unsigned int __thiscall UDishonoredAIBrain::IsBrainInitialized(void)const
//   0x745ef0  public: void __thiscall UDishonoredAIBrain::MarkForRefreshBrainThoughts(void)
//   0x745f00  public: unsigned int __thiscall UDishonoredAIBrain::IsCurrentBehavior(class UClass * const)const
//   0x745f40  public: unsigned int __thiscall UDishonoredAIBrain::IsIgnoringTechnologyDanger(void)const
//   0x745f60  public: unsigned int __thiscall UDishonoredAIBrain::ShouldKeepAlarmRinging(class IDisAttentionTargetInterface const *)const
//   0x745f80  public: unsigned int __thiscall UDishonoredAIBrain::IsFlagSet(enum EDisAIBrainFlags)const
//   0x745fa0  public: void __thiscall UDishonoredAIBrain::SetFlagTo(enum EDisAIBrainFlags, unsigned int)
//   0x747720  public: virtual void __thiscall UDishonoredAIBrain::SetTweaks_Derived(class UDisTweaksBase *)
//   0x747730  public: unsigned int __thiscall UDishonoredAIBrain::IsBrainInhibited(void)const
//   0x747740  public: unsigned int __thiscall UDishonoredAIBrain::IsBehaviorOnStack(class UClass * const)const
//   0x7477b0  public: unsigned int __thiscall UDishonoredAIBrain::AllowPlayerUse(enum eCrossHairStatus * const)const
//   0x747850  public: unsigned int __thiscall UDishonoredAIBrain::IsAttentiveToward(class ADishonoredPawn const *, enum ERelationship, enum EDisAttentionLevel)const
//   0x7496a0  public: class UDisTweaks_AIBehavior * __thiscall UDishonoredAIBrain::GetAIBehaviorTweakForSlot(int)const
//   0x749780  private: void __thiscall UDishonoredAIBrain::InitBrain_Processes(void)
//   0x749910  private: void __thiscall UDishonoredAIBrain::RefreshBrain_Processes(float)
//   0x749980  private: void __thiscall UDishonoredAIBrain::TickBrain_Stims(float)
//   0x7499f0  public: virtual void __thiscall UDishonoredAIBrain::PostGameLoad(enum ESaveLoadLocation)
//   0x749a70  public: unsigned int __thiscall UDishonoredAIBrain::SupportsBehavior(class UClass const *, unsigned int)const
//   0x749b20  public: unsigned int __thiscall UDishonoredAIBrain::GetPathGoalsAndConstraintsFromBehavior(class FVector const &, unsigned int, class TArray<class UNavMeshPathGoalEvaluator *, class FDefaultAllocator> &, class TArray<class UNavMeshPathConstraint *, class FDefaultAllocator> &)const
//   0x749b60  public: class UDisAIBrainProcess * __thiscall UDishonoredAIBrain::GetBrainProcess(class UClass *)
//   0x749bd0  public: unsigned int __thiscall UDishonoredAIBrain::IsAvailableForSoiree_Brain(class FGuid, enum ESoireeAIPriority, unsigned int, class UDishonoredAIBehavior * &)const
//   0x74d710  public: void __thiscall UDishonoredAIBrain::AddBrainInhibitor(class UObject *)
//   0x74d790  public: virtual void __thiscall UDishonoredAIBrain::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x74fc20  public: void __thiscall UDishonoredAIBrain::RemoveBrainInhibitor(class UObject *)
//   0x74fc90  public: class FDisAIKnowledgeComponent & __thiscall UDishonoredAIBrain::GetKnowledge(void)const
//   0x752290  public: virtual void __thiscall UDishonoredAIBrain::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x752490  private: void __thiscall UDishonoredAIBrain::EnqueueStim(struct FAIStimStruct *)
//   0x757920  public: static class UClass * __cdecl UDishonoredAIBrain::GetPrivateStaticClassUDishonoredAIBrain(wchar_t const *)
//   0x7579b0  public: void __thiscall UDishonoredAIBrain::OnDifficultyChange(class FArkGameEvent const &)
//   0x757b50  public: void __thiscall UDishonoredAIBrain::OnPushedByAvoidable(class FArkGameEvent const &)
//   0x759e30  public: static class UClass * __cdecl UDishonoredAIBrain::StaticClassNoInline(void)
//   0x759e60  private: void __thiscall UDishonoredAIBrain::ProcessOneStim(struct FAIStimStruct const &, unsigned int * const, unsigned int * const)
//   0x75b630  private: void __thiscall UDishonoredAIBrain::FlushStimQueue(unsigned int * const, unsigned int * const)
//   0x75b830  private: void __thiscall UDishonoredAIBrain::ProcessAllStims(void)
//   0x75bdf0  private: void __thiscall UDishonoredAIBrain::TickBrain_Processes(float)
//   0x75be60  public: void __thiscall UDishonoredAIBrain::RefreshBrainThoughts(void)
//   0x75c390  public: virtual void __thiscall UDishonoredAIBrain::BeginDestroy(void)
//   0x75c3f0  public: void __thiscall UDishonoredAIBrain::TickBrain(float)
//   0x75c8d0  public: void __thiscall UDishonoredAIBrain::InitBrain(class ADishonoredNPCPawn * const, class UDisTweaks_AIBrain * const, enum EDisAISuspicionLevel)
//   0x75ccc0  public: void __thiscall UDishonoredAIBrain::TerminateBrain(void)
//   0x75cf80  public: void __thiscall UDishonoredAIBrain::Forget(void)
//   0x75dd20  public: void __thiscall UDishonoredAIBrain::OnOtherActorTerminated_AIBrain(class AActor const &)
