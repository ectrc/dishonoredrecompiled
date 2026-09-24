// DishonoredGame/src/disaisubstatefindshootingposition.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x765380  public: virtual unsigned char const * __thiscall UDisAISubStateFindShootingPosition::BuildFilterStimMask(void)const
//   0x7653b0  private: unsigned int __thiscall UDisAISubStateFindShootingPosition::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x7653e0  private: unsigned int __thiscall UDisAISubStateFindShootingPosition::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x765400  private: virtual struct FDisLocoRequest * __thiscall UDisAISubStateFindShootingPosition::GetDesiresLocoRequest(void)
//   0x7687e0  public: virtual void __thiscall FDisAISubStateFindShootingPosition_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x7688a0  private: static unsigned int __cdecl UDisAISubStateFindShootingPosition::CanShootFrom(class AActor *, class FVector, class FVector, float, float, float, float, float, float, class FVector, unsigned int &)
//   0x770b50  public: static int __cdecl CompareDisFindShootingPosition_SortDistFromIdealFVectorConstRef::Compare(class FVector const &, class FVector const &)
//   0x770bf0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAISubStateFindShootingPosition::GetFilterStimDelegate_SubState(enum EAIStimID)
//   0x770c30  public: virtual unsigned int __thiscall UDisAISubStateFindShootingPosition::GetPathGoals(class FVector const &, class TArray<class UNavMeshPathGoalEvaluator *, class FDefaultAllocator> &)const
//   0x770cb0  private: unsigned int __thiscall UDisAISubStateFindShootingPosition::EvaluatePosition(class FVector, float, float, float, float, float, unsigned int &)const
//   0x775280  public: virtual unsigned int __thiscall UDisAISubStateFindShootingPosition::GetPathConstraints(class FVector const &, unsigned int, class TArray<class UNavMeshPathConstraint *, class FDefaultAllocator> &)const
//   0x775350  private: unsigned int __thiscall UDisAISubStateFindShootingPosition::CheckAgainstHackedWallOfLight(class FVector)const
//   0x777420  public: static class UClass * __cdecl UDisAISubStateFindShootingPosition::GetPrivateStaticClassUDisAISubStateFindShootingPosition(wchar_t const *)
//   0x77ba30  public: static void __cdecl UDisAISubStateFindShootingPosition::InitializePrivateStaticClassUDisAISubStateFindShootingPosition(void)
//   0x77ba50  private: void __thiscall UDisAISubStateFindShootingPosition::FindShootPositionCandidates(void)
//   0x780200  public: static class UClass * __cdecl UDisAISubStateFindShootingPosition::StaticClassNoInline(void)
//   0x781af0  public: __thiscall FDisAISubStateFindShootingPosition_Param::FDisAISubStateFindShootingPosition_Param(struct FDisAttentionProxy const &, class FVector, float, unsigned int, unsigned int, unsigned int)
//   0x7831a0  public: static class UClass * __cdecl UDisTweaks_AISubState_FindShootingPosition::GetPrivateStaticClassUDisTweaks_AISubState_FindShootingPosition(wchar_t const *)
//   0x784560  public: static void __cdecl UDisTweaks_AISubState_FindShootingPosition::InitializePrivateStaticClassUDisTweaks_AISubState_FindShootingPosition(void)
//   0x784d20  public: static class UClass * __cdecl UDisTweaks_AISubState_FindShootingPosition::StaticClassNoInline(void)
//   0x786a40  private: void __thiscall UDisAISubStateFindShootingPosition::JustStartWalkingSomewhere(void)
//   0x786c20  private: void __thiscall UDisAISubStateFindShootingPosition::CheckForBlockedMovement(float)
//   0x786cf0  public: unsigned int __thiscall UDisAISubStateFindShootingPosition::IsWithinLegalShootRange(struct FDisAttentionProxy const &)const
//   0x786eb0  private: void __thiscall UDisAISubStateFindShootingPosition::CalculateShootDistances(void)
//   0x786f90  private: unsigned int __thiscall UDisAISubStateFindShootingPosition::EvaluateCandidates(int &, class TArray<class FVector, class FDefaultAllocator> &)
//   0x78ac60  public: virtual void __thiscall UDisAISubStateFindShootingPosition::BeginSubState_Derived(void)
//   0x78acb0  public: virtual void __thiscall UDisAISubStateFindShootingPosition::RefreshSubState(float)
