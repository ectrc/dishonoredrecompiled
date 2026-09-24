// DishonoredGame/src/dishonoredaibehavior.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (43):
//   0x745360  public: static void __cdecl UDishonoredAIBehavior::InitializePrivateStaticClassUDishonoredAIBehavior(void)
//   0x745380  public: int __thiscall UDishonoredAIBehavior::GetLogicalSubStateIndex(void)const
//   0x745390  public: int __thiscall UDishonoredAIBehavior::GetActiveSubStateIndex(void)const
//   0x7453a0  protected: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDishonoredAIBehavior::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x7453c0  protected: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDishonoredAIBehavior::GetSetupFromStimDelegate(enum EAIStimID)
//   0x7453e0  protected: virtual enum eDisDialogAvailability __thiscall UDishonoredAIBehavior::GetDialogAvailability(enum eDisConvPriority, int)const
//   0x745410  public: virtual unsigned int __thiscall IDisRatTargetInterface::IsRatSocketValid(class FName)const
//   0x745420  public: class UDisAISubState * __thiscall UDishonoredAIBehavior::GetCurrentSubState(void)const
//   0x745430  public: class ADishonoredNPCPawn * __thiscall UDishonoredAIBehavior::GetOwningPawn(void)const
//   0x745450  public: virtual unsigned int __thiscall UDishonoredAIBehavior::CanBlockSoiree(class FGuid, enum ESoireeAIPriority)const
//   0x746af0  public: virtual void __thiscall UDishonoredAIBehavior::PostGameLoad(enum ESaveLoadLocation)
//   0x746b80  public: virtual unsigned int __thiscall UDishonoredAIBehavior::IsPlayerAllowedToPushMe(void)const
//   0x748600  private: virtual void __thiscall UDishonoredAIBehavior::SetTweaks_Derived(class UDisTweaksBase *)
//   0x7487b0  public: void __thiscall UDishonoredAIBehavior::CallRefreshThoughts(float)
//   0x7488a0  public: void __thiscall UDishonoredAIBehavior::CallTickBehavior(float)
//   0x7489c0  public: static void __cdecl UDishonoredAIBehavior::GetDefaultPathGoals(class FVector const &, class TArray<class UNavMeshPathGoalEvaluator *, class FDefaultAllocator> &)
//   0x748a30  public: unsigned int __thiscall UDishonoredAIBehavior::CallGetPathGoals(class FVector const &, class TArray<class UNavMeshPathGoalEvaluator *, class FDefaultAllocator> &)const
//   0x748aa0  public: static void __cdecl UDishonoredAIBehavior::GetDefaultPathConstraints(class FVector const &, class TArray<class UNavMeshPathConstraint *, class FDefaultAllocator> &)
//   0x748b00  public: unsigned int __thiscall UDishonoredAIBehavior::CallGetPathConstraints(class FVector const &, unsigned int, class TArray<class UNavMeshPathConstraint *, class FDefaultAllocator> &)const
//   0x748b70  public: unsigned int __thiscall UDishonoredAIBehavior::ShouldLowerAlertness(void)const
//   0x748bd0  protected: virtual enum EAIAwareness __thiscall UDishonoredAIBehavior::GetAwarenessLevel(void)const
//   0x748c50  protected: virtual class ADishonoredPawn * __thiscall UDishonoredAIBehavior::GetCurrentEnemy(void)const
//   0x748c90  private: class UDisAISubProcess * __thiscall UDishonoredAIBehavior::GetSubProcess(class UClass * const)const
//   0x748d30  private: void __thiscall UDishonoredAIBehavior::CollectActionTargetSettings(class AActor * &, struct FDisAttentionProxy &)const
//   0x74c450  protected: void __thiscall UDishonoredAIBehavior::BuildInternalFilterStimMasks(unsigned char *, unsigned char *)const
//   0x74c560  private: void __thiscall UDishonoredAIBehavior::RegisterCallbacks(class UDisAISubState * const, unsigned long)
//   0x74c690  public: unsigned int __thiscall UDishonoredAIBehavior::GetBehaviorActionTargetLocation(class FVector &)const
//   0x74c750  public: class AActor * __thiscall UDishonoredAIBehavior::GetBehaviorActionTargetActor(void)const
//   0x74f1d0  public: void __thiscall UDishonoredAIBehavior::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x74f2c0  public: unsigned int __thiscall UDishonoredAIBehavior::CallFilterAIStim(struct FAIStimStruct const &)
//   0x74f420  public: unsigned int __thiscall UDishonoredAIBehavior::CallShouldFinishWhileDormant(struct FAIStimStruct const &)
//   0x74f480  public: void __thiscall UDishonoredAIBehavior::OnBecomeDormant(void)
//   0x74f550  public: unsigned int __thiscall UDishonoredAIBehavior::FireDialogHook(enum eDisDialogHook, class IDisConvSpeakerInterface *)
//   0x74f690  protected: void __thiscall UDishonoredAIBehavior::SetActionTargetProxy(struct FDisAttentionProxy)
//   0x74f760  protected: void __thiscall UDishonoredAIBehavior::SetActionTargetActor(class AActor *)
//   0x74f820  protected: void __thiscall UDishonoredAIBehavior::ClearActionTarget(void)
//   0x74f8e0  public: virtual void __thiscall UDishonoredAIBehavior::BeginDestroy(void)
//   0x750de0  public: virtual void __thiscall UDishonoredAIBehavior::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x750e90  public: void __thiscall UDishonoredAIBehavior::CallInitBehavior(class UDishonoredAIBrain * const, class UDisTweaks_AIBehavior * const)
//   0x751410  public: void __thiscall UDishonoredAIBehavior::CallOnBehaviorResume(struct FDisBodyIntention const &)
//   0x756df0  public: static class UClass * __cdecl UDishonoredAIBehavior::GetPrivateStaticClassUDishonoredAIBehavior(wchar_t const *)
//   0x759c10  public: static class UClass * __cdecl UDishonoredAIBehavior::StaticClassNoInline(void)
//   0x759c40  public: void __thiscall UDishonoredAIBehavior::CallOnBehaviorPause(unsigned int)
