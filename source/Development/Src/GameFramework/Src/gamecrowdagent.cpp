// GameFramework/src/gamecrowdagent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x599f30  public: static void __cdecl AGameCrowdAgent::InitializePrivateStaticClassAGameCrowdAgent(void)
//   0x599f50  public: void __thiscall FSingleAgentAttractor::ClearAttractionForce(void)
//   0x599f60  public: class FVector __thiscall FSingleAgentAttractor::CalculateAttractionForce(class AGameCrowdAgent *)const
//   0x59a1a0  public: void __thiscall FSingleAgentAttractor::TickAttractionForce(float)
//   0x59a1c0  public: virtual void __thiscall AGameCrowdAgent::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x59a220  public: virtual void __thiscall AGameCrowdAgent::PlayDeath(class AController *, class FVector, class UClass *, class AActor *)
//   0x59a2a0  public: void __thiscall AGameCrowdAgent::ComputeAvoidOtherRadius(void)
//   0x59a2e0  public: virtual unsigned int __thiscall AController::IsPlayerOwned(void)
//   0x59a2f0  public: virtual void __thiscall AGameCrowdAgent::VolumeBasedDestroy(class APhysicsVolume *)
//   0x59a300  protected: struct FNavMeshPolyBase * __thiscall AGameCrowdAgent::FindNewPoly(class FVector, class FVector &)
//   0x59a370  public: void __thiscall AGameCrowdAgent::SetCurrentDestination(class AGameCrowdDestination *)
//   0x59a410  public: virtual void __thiscall AGameCrowdAgent::SetupPathfindingParams(struct FNavMeshPathParams &)const
//   0x59a480  public: virtual class FVector __thiscall AGameCrowdAgent::GetEdgeZAdjust(struct FNavMeshEdgeBase *)const
//   0x59a4a0  public: unsigned int __thiscall AGameCrowdAgent::ShouldPerformCrowdSimulation(float)
//   0x59a4f0  public: unsigned int __thiscall AGameCrowdAgent::IsAlive(void)const
//   0x59e1c0  public: void __thiscall FSingleAgentAttractor::SetAttractionForce(class FVector const &, float, float, float)
//   0x59e210  public: virtual void __thiscall AGameCrowdAgent::PostBeginPlay(void)
//   0x59e420  public: virtual void __thiscall AGameCrowdAgent::ClampVelocity(float, class FVector const &, class FVector const &, class FVector const &)
//   0x59e600  public: virtual void __thiscall AGameCrowdAgent::ExactVelocity(float)
//   0x59e8b0  protected: void __thiscall AGameCrowdAgent::UpdateRotation(float)
//   0x59eb50  protected: class FVector __thiscall AGameCrowdAgent::ComputeWallForce(class FVector const &)
//   0x59eff0  public: unsigned int __thiscall AGameCrowdAgent::ReachedIntermediatePoint(void)
//   0x59f0d0  public: unsigned int __thiscall AGameCrowdAgent::CurrentDestinationMovedTooMuch(void)
//   0x59f300  public: virtual void __thiscall AGameCrowdAgent::KillAgent(void)
//   0x59f390  public: class FVector __thiscall AGameCrowdAgent::GeneratePathToActor(class AActor *, float, unsigned int)
//   0x59f4f0  public: unsigned int __thiscall AGameCrowdAgent::UpdateAgentDeath(float)
//   0x59f6b0  public: virtual unsigned int __thiscall AGameCrowdAgent::Tick(float, enum ELevelTick)
//   0x5a0c80  public: virtual void __thiscall AGameCrowdAgent::PreBeginPlay(void)
//   0x5a0cb0  public: class FVector __thiscall AGameCrowdAgent::CalcPathForce(void)
//   0x5a0ec0  protected: unsigned int __thiscall AGameCrowdAgent::MovementCheck(struct AGameCrowdAgent::FMovementCheckResult &, struct FNavMeshPolyBase *, class FVector, class FVector, struct FNavMeshPolyBase * &)
//   0x5a19c0  public: void __thiscall AGameCrowdAgent::UpdateIntermediatePoint(class AActor *)
//   0x5a1c10  public: virtual void __thiscall AGameCrowdAgent::TickSpecial(float)
//   0x5a1ce0  public: virtual void __thiscall AGameCrowdAgent::SetLightEnvironment(class ULightEnvironmentComponent *)
//   0x5a1db0  public: virtual class FString __thiscall AGameCrowdAgent::GetDetailedInfoInternal(void)const
//   0x5a69c0  public: virtual void __thiscall AGameCrowdAgent::PostScriptDestroyed(void)
//   0x5a6a30  public: class FVector __thiscall AGameCrowdAgent::CheckRelevantAttractors(void)
//   0x5a6b00  protected: struct FNavMeshPolyBase * __thiscall AGameCrowdAgent::FindNextPoly(class FVector, class FVector &, unsigned int &, class FVector &)
//   0x5a8c30  public: unsigned int __thiscall AGameCrowdAgent::UpdateLocation(class FVector const &, unsigned int)
//   0x5aa1f0  public: virtual void __thiscall AGameCrowdAgent::performPhysics(float)
//   0x5abcc0  public: static class UClass * __cdecl AGameCrowdAgent::GetPrivateStaticClassAGameCrowdAgent(wchar_t const *)
//   0x5ac590  public: static class UClass * __cdecl AGameCrowdAgent::StaticClassNoInline(void)
//   0x5ad500  public: virtual void __thiscall AGameCrowdAgent::AddedToPool(void)
//   0x5ad6c0  public: virtual void __thiscall AGameCrowdAgent::ResetPooledAgent(void)
//   0x5af1e0  public: virtual void __thiscall AGameCrowdAgent::InitializeAgent(class AActor *, class AGameCrowdAgent *, float, unsigned int, unsigned int)
