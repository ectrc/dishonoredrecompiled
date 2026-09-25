// DishonoredGame/src/disratswarm.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (64):
//   0x8b0930  public: static void __cdecl UDisTweaks_RatSwarm::InitializePrivateStaticClassUDisTweaks_RatSwarm(void)
//   0x8b0950  public: __thiscall FDisRatSwarmCustomBehavior::FDisRatSwarmCustomBehavior(void)
//   0x8b0970  public: unsigned int __thiscall FDisRatSwarmCustomBehavior::IsAllowedToIdle(void)const
//   0x8b0990  public: unsigned int __thiscall FDisRatSwarmCustomBehavior::IsAttackGroupAllowedOnTarget(class IDisRatTargetInterface *)const
//   0x8b0a20  public: unsigned int __thiscall ADisRatSwarm::IsLODed(void)const
//   0x8b0a30  public: unsigned int __thiscall ADisRatSwarm::IsAlive(void)const
//   0x8b0a40  public: unsigned int __thiscall ADisRatSwarm::IsIdle(void)const
//   0x8b0a50  public: unsigned int __thiscall ADisRatSwarm::HasAggressiveBehavior(void)const
//   0x8b0a80  public: unsigned int __thiscall ADisRatSwarm::IsSpawningRats(void)const
//   0x8b0ab0  public: class TArray<class AGameCrowdAgent *, class FDefaultAllocator> const & __thiscall ADisRatSwarm::GetRatList(void)const
//   0x8b0ac0  public: class FVector __thiscall ADisRatSwarm::GetVisibilityTestOffset(void)const
//   0x8b0af0  public: void __thiscall ADisRatSwarm::SetDuration(float)
//   0x8b0b10  public: void __thiscall ADisRatSwarm::SetFleeing(void)
//   0x8b0b50  public: unsigned int __thiscall ADisRatSwarm::IsDead(void)const
//   0x8b0b60  public: virtual int __thiscall UInterpTrackFaceTo::GetNumKeyframes(void)const
//   0x8b0b70  private: void __thiscall ADisRatSwarm::UpdateActiveState(float)
//   0x8b6180  public: void __thiscall FDisRatSwarmCustomBehavior::Stop(void)
//   0x8b61d0  public: void __thiscall FDisRatSwarmCustomBehavior::SetForcedActionFinished(void)
//   0x8b6220  public: void __thiscall FDisRatSwarmCustomBehavior::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8b6300  public: void __thiscall FDisRatSwarmCustomBehavior::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8b63d0  public: void __thiscall FDisRatSwarmCustomBehavior::OnDestinationReached(void)
//   0x8b63e0  public: void __thiscall ADisRatSwarm::OnRatPlayedDeath(class ADisGameCrowdAgentSkeletalRat *, class AActor *)
//   0x8b6470  public: unsigned int __thiscall ADisRatSwarm::IsDevouringSwarm(void)const
//   0x8b6490  public: void __thiscall ADisRatSwarm::SetDefaultBehavior(void)
//   0x8b6500  public: virtual void __thiscall ADisRatSwarm::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8ba670  public: void __thiscall FDisRatSwarmCustomBehavior::Start(class ADisRatSwarm *)
//   0x8ba6d0  public: int __thiscall ADisRatSwarm::GetAliveRatCount(void)const
//   0x8ba790  public: unsigned int __thiscall ADisRatSwarm::IsTargetAttackable(class IDisRatTargetInterface *)const
//   0x8ba890  public: unsigned int __thiscall ADisRatSwarm::IsTargetSighted(class IDisRatTargetInterface const *)const
//   0x8ba920  public: struct FDisRatGroup * __thiscall ADisRatSwarm::GetGroupByTarget(class IDisRatTargetInterface const *)const
//   0x8ba9d0  public: int __thiscall ADisRatSwarm::GetGroupIdxByTarget(class IDisRatTargetInterface const *)const
//   0x8baa50  public: unsigned int __thiscall ADisRatSwarm::IsEngagingTarget(class IDisRatTargetInterface const *)const
//   0x8baa70  public: unsigned int __thiscall ADisRatSwarm::IsAttackingTarget(class IDisRatTargetInterface const *)const
//   0x8baaa0  public: int __thiscall ADisRatSwarm::GetEngagedAllyCount(class ADishonoredPawn *)const
//   0x8bab70  public: void __thiscall ADisRatSwarm::SetCustomBehavior(struct FDisRatSwarmCustomBehavior const &)
//   0x8babf0  private: void __thiscall ADisRatSwarm::UpdateLocation(void)
//   0x8bae80  private: void __thiscall ADisRatSwarm::RegisterUnreachableTarget(class IDisRatTargetInterface *, unsigned int)
//   0x8bede0  public: unsigned int __thiscall ADisRatSwarm::HasEnoughRats(void)const
//   0x8bee30  public: unsigned int __thiscall ADisRatSwarm::IsHostileToTarget(class IDisRatTargetInterface const *)const
//   0x8beea0  private: void __thiscall ADisRatSwarm::UpdateTargetSightInfo(class IDisRatTargetInterface *)
//   0x8bf0c0  private: unsigned int __thiscall ADisRatSwarm::CheckTargetReachability(class IDisRatTargetInterface *)
//   0x8c3900  public: virtual void __thiscall ADisRatSwarm::PostBeginPlay(void)
//   0x8c3990  public: void __thiscall ADisRatSwarm::GetAliveRatList(struct TMemStackArray<class ADisGameCrowdAgentSkeletalRat *> &)const
//   0x8c6e70  public: void __thiscall ADisRatSwarm::OnTargetDestroyed(class FArkGameEvent const &)
//   0x8c6f20  private: void __thiscall ADisRatSwarm::UpdateDetectedTargets(void)
//   0x8c8960  public: void __thiscall ADisRatSwarm::Initialize(class AActor *, struct FGameCrowdSpawnerSettings const *)
//   0x8c8b50  public: virtual void __thiscall ADisRatSwarm::BeginDestroy(void)
//   0x8c8c10  public: class ADisRatSwarm * __thiscall ADisRatSwarm::DuplicateRatSwarm(class FVector)
//   0x8c8d30  public: virtual void __thiscall ADisRatSwarm::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8ca260  public: static class UClass * __cdecl ADisRatSwarm::GetPrivateStaticClassADisRatSwarm(wchar_t const *)
//   0x8ca2f0  public: void __thiscall ADisRatSwarm::OnNavMeshPathClosed(void)
//   0x8cc300  public: static void __cdecl ADisRatSwarm::InitializePrivateStaticClassADisRatSwarm(void)
//   0x8cd050  public: static class UClass * __cdecl ADisRatSwarm::StaticClassNoInline(void)
//   0x8cd080  public: static class UClass * __cdecl UDisTweaks_RatSwarm::GetPrivateStaticClassUDisTweaks_RatSwarm(wchar_t const *)
//   0x8cf700  public: static class UClass * __cdecl UDisTweaks_RatSwarm::StaticClassNoInline(void)
//   0x8cf730  public: void __thiscall ADisRatSwarm::CleanUpRat(class ADisGameCrowdAgentSkeletalRat *, unsigned int)
//   0x8d12e0  public: void __thiscall ADisRatSwarm::CleanUp(void)
//   0x8d15b0  public: void __thiscall ADisRatSwarm::Reset(class AActor *)
//   0x8d15e0  public: virtual void __thiscall ADisRatSwarm::PostScriptDestroyed(void)
//   0x8d1600  public: virtual unsigned int __thiscall ADisRatSwarm::Tick(float, enum ELevelTick)
//   0x8d1700  public: void __thiscall ADisRatSwarm::OnRatKilled(class ADisGameCrowdAgentSkeletalRat *)
//   0x8d1720  public: void __thiscall ADisRatSwarm::OnRatDestroyed(class ADisGameCrowdAgentSkeletalRat *)
//   0x8d33a0  public: void __thiscall ADisRatSwarm::OnRatSpawned(class ADisGameCrowdAgentSkeletalRat *)
//   0xbaace0  _dynamic_initializer_for__s_StateMethods__
