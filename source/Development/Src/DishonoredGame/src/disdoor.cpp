// DishonoredGame/src/disdoor.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (75):
//   0x66af80  public: static void __cdecl UDisSeqEvent_KeyHoleUsed::InitializePrivateStaticClassUDisSeqEvent_KeyHoleUsed(void)
//   0x66afa0  public: virtual struct FUsableObjectStage const & __thiscall UDisTweaks_Door::GetUsableStage(int)const
//   0x66aff0  public: enum EDisDoorState __thiscall ADisDoor::GetDoorState(void)const
//   0x66b000  public: unsigned int __thiscall ADisDoor::IsTotallyDestroyed(void)const
//   0x66b010  private: void __thiscall ADisDoor::ShutdownSweepBody(void)
//   0x66b050  public: virtual class FGuid * __thiscall ADisDoor::GetGuid(void)
//   0x66b060  private: enum EDisDoorOpenDirection __thiscall ADisDoor::GetOpenDir(class FVector const &, class AActor *)const
//   0x66b110  public: virtual unsigned int __thiscall ADisDoor::CanAltInteract(class ADishonoredPawn const * const)const
//   0x66b1a0  public: unsigned int __thiscall ADisDoor::IsEdgeBlocked(struct FNavMeshPathObjectEdge *)const
//   0x66d260  PathEdgesMatch
//   0x66d490  private: class FVector __thiscall ADisDoor::GetDoorFacing_Body(void)const
//   0x66d590  private: void __thiscall ADisDoor::UpdateSweepBody(void)
//   0x66d720  GetFinalBodyBox
//   0x66d800  public: unsigned int __thiscall ADisDoor::GetKeyHole(class FRotator, class FVector &, class FRotator &)const
//   0x670c50  public: virtual void __thiscall UDisTweaks_Door::PostLoad(void)
//   0x670d30  private: void __thiscall ADisDoor::ForceNPCsMovingThruToRepath(void)
//   0x670df0  protected: virtual void __thiscall ADisDoor::OnTransitioning(float)
//   0x670f80  public: virtual void __thiscall ADisDoor::GetActorReferences(class TArray<struct FActorReference *, class FDefaultAllocator> &, unsigned int)
//   0x670ff0  public: virtual void __thiscall ADisDoor::ClearCrossLevelReferences(void)
//   0x6710c0  public: void __thiscall ADisDoor::OnNavMeshPathClosed(void)
//   0x671370  public: virtual void __thiscall ADisDoor::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x6713e0  public: virtual int __thiscall ADisDoor::CostFor(struct FNavMeshPathParams const &, class FVector const &, class FVector &, struct FNavMeshPathObjectEdge *, struct FNavMeshPolyBase *)
//   0x6714a0  public: virtual void __thiscall ADisDoor::AddActorMovingThruEdge(struct FNavMeshPathObjectEdge const * const, class AActor * const)
//   0x673100  private: unsigned int __thiscall ADisDoor::ShouldOpenDoorClockwise(class ADishonoredPawn const * const)
//   0x673250  public: virtual void __thiscall ADisDoor::InitGuid(class TArray<class FGuid, class FDefaultAllocator> &)
//   0x673300  private: void __thiscall ADisDoor::ClearDoorPathEdgesForPoly(struct FNavMeshPolyBase *)
//   0x673430  public: virtual unsigned int __thiscall ADisDoor::DrawEdge(class FDebugRenderSceneProxy *, class FColor, class FVector, struct FNavMeshPathObjectEdge *)
//   0x674320  public: virtual void __thiscall ADisDoor::RemoveActorMovingThruEdge(struct FNavMeshPathObjectEdge const * const, class AActor * const)
//   0x679040  public: static class UClass * __cdecl ADisDoor::GetPrivateStaticClassADisDoor(wchar_t const *)
//   0x67b550  public: static class UClass * __cdecl UDisTweaks_Door::GetPrivateStaticClassUDisTweaks_Door(wchar_t const *)
//   0x67bf70  public: static void __cdecl ADisDoor::InitializePrivateStaticClassADisDoor(void)
//   0x67c920  public: static class UClass * __cdecl ADisDoor::StaticClassNoInline(void)
//   0x67c950  public: static class UClass * __cdecl UDisDoorBreakSteps::GetPrivateStaticClassUDisDoorBreakSteps(wchar_t const *)
//   0x67c9e0  public: static class UClass * __cdecl UDisSeqEvent_KeyHoleUsed::GetPrivateStaticClassUDisSeqEvent_KeyHoleUsed(wchar_t const *)
//   0x681230  public: static void __cdecl UDisDoorBreakSteps::InitializePrivateStaticClassUDisDoorBreakSteps(void)
//   0x681250  public: static void __cdecl UDisTweaks_Door::InitializePrivateStaticClassUDisTweaks_Door(void)
//   0x681270  public: static class UClass * __cdecl UDisSeqEvent_KeyHoleUsed::StaticClassNoInline(void)
//   0x6812a0  public: virtual class FString const & __thiscall ADisDoor::GetInteractableName(void)const
//   0x683220  public: static class UClass * __cdecl UDisDoorBreakSteps::StaticClassNoInline(void)
//   0x683250  public: static class UClass * __cdecl UDisTweaks_Door::StaticClassNoInline(void)
//   0x683280  public: virtual unsigned int __thiscall ADisDoor::AttemptAltInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x683410  public: virtual unsigned int __thiscall ADisDoor::EndInteract_Derived(struct FEndInteractParams const &)
//   0x688640  protected: virtual class UDisTweaksBase * __thiscall ADisDoor::GetTweaks_Derived(void)
//   0x688670  protected: virtual unsigned int __thiscall ADisDoor::OnUseObject(void)
//   0x688aa0  protected: virtual void __thiscall ADisDoor::OnTransitionCompleted(void)
//   0x68b8b0  public: virtual void __thiscall ADisDoor::PostLoad(void)
//   0x68b910  public: virtual unsigned int __thiscall ADisDoor::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x68b9b0  private: void __thiscall ADisDoor::UpdateAudioPortalOcclusion(void)
//   0x68bb20  public: void __thiscall ADisDoor::RattleDoor(enum EDisDoorOpenDirection)
//   0x68bbc0  private: enum EDisDoorPathEdgeType __thiscall ADisDoor::GetDoorPathEdgeType(class FVector const &, class FVector const &, struct ADisDoor::FDoorShapeDetails const &)const
//   0x68be10  protected: void __thiscall ADisDoor::EnableAvoidable(unsigned int)
//   0x68c060  private: void __thiscall ADisDoor::SetupSweepBody(void)
//   0x68ca80  public: unsigned int __thiscall ADisDoor::CanSlamOpen(struct AActor::WindBlastParams const &)const
//   0x68cbb0  public: virtual void __thiscall ADisDoor::OnTakeHit(class FVector const &, class FVector const &, class AActor *, class UClass const *)
//   0x68ccc0  public: virtual enum eCrossHairStatus __thiscall ADisDoor::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x68cd20  public: virtual float __thiscall ADisDoor::GetAltInteractTime(void)const
//   0x68cd50  private: void __thiscall ADisDoor::ApplyInitialState(void)
//   0x68cda0  private: void __thiscall ADisDoor::ReplaceDoorPathEdgesForPoly(struct FNavMeshPolyBase *, struct ADisDoor::FDoorShapeDetails const &)
//   0x68cfc0  protected: virtual void __thiscall ADisDoor::DestroySkeletalBreakable(void)
//   0x68e290  public: virtual void __thiscall ADisDoor::PostBeginPlay(void)
//   0x68e4a0  public: virtual void __thiscall ADisDoor::MarkComponentsAsPendingKill(unsigned int)
//   0x68e520  public: virtual void __thiscall ADisDoor::BeginDestroy(void)
//   0x68e610  protected: virtual void __thiscall ADisDoor::OnTransitionStart(void)
//   0x68eb50  public: virtual void __thiscall ADisDoor::Lock(void)
//   0x68eb70  public: virtual void __thiscall ADisDoor::Unlock(void)
//   0x68eba0  public: virtual void __thiscall ADisDoor::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x68ec60  private: unsigned int __thiscall ADisDoor::GetDoorShapeDetails(struct ADisDoor::FDoorShapeDetails &)
//   0x68f4f0  public: virtual void __thiscall ADisDoor::DisGetMeshSplittingPolys(class TArray<struct UNavigationMeshBase::FMeshSplitingShape, class FDefaultAllocator> &)
//   0x68f740  public: virtual void __thiscall ADisDoor::CreateEdgesForPathObject(class APylon *)
//   0x690090  public: void __thiscall ADisDoor::OpenDoor(enum EDisDoorOpenDirection)
//   0x6900f0  public: void __thiscall ADisDoor::SlamOpenDoor(enum EDisDoorOpenDirection)
//   0x6901e0  public: void __thiscall ADisDoor::CloseDoor(void)
//   0x690230  public: virtual unsigned int __thiscall ADisDoor::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x6903c0  public: virtual void __thiscall ADisDoor::OnBroken(int, int, class FVector const &, class FVector const &, class AActor *, class UClass const *)
//   0x690b10  public: virtual unsigned int __thiscall ADisDoor::Tick(float, enum ELevelTick)
