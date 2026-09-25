// DishonoredGame/src/dishonoredutilities_physics.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (19):
//   0x824520  public: static enum ERBCollisionChannel __cdecl FDisPhysicsUtil::DisCreatePawnChannel(void)
//   0x824590  public: static void __cdecl FDisPhysicsUtil::DisReleasePawnChannel(enum ERBCollisionChannel)
//   0x828250  public: static float __cdecl FDisPhysicsUtil::CalculateImpulseForce(class FVector const &, class FVector const &, class FVector const &, float, float)
//   0x828390  public: static unsigned int __cdecl FDisPhysicsUtil::PredictProjectileDirectionToHit(class FVector const &, class FVector const &, float, float, class FVector &, float &)
//   0x828530  public: static void __cdecl FDisPhysicsUtil::AdjustMomentumForCoolness(class FVector *, float, float)
//   0x8286d0  public: static unsigned int __cdecl FDisPhysicsUtil::PhysObjectShouldTraceCommon(class AActor const *, unsigned long)
//   0x82e7e0  public: static void __cdecl FDisPhysicsUtil::DisBodyAddForceAtPoint(class URB_BodyInstance *, class FVector, class FVector, unsigned int)
//   0x82e960  public: static void __cdecl FDisPhysicsUtil::DisBodyAddImpulseAtPoint(class URB_BodyInstance *, class FVector, class FVector, unsigned int)
//   0x82eae0  public: static void __cdecl FDisPhysicsUtil::DisBodySetDamping(class URB_BodyInstance *, float, float)
//   0x82ec10  public: static class ARB_ConstraintActor * __cdecl FDisPhysicsUtil::DisNailPawn(class ADishonoredPawn *, class FName, class FVector const &, class AActor *, class FName, class FVector const &, class FRotator const &, float, float, float)
//   0x82f0c0  public: static void __cdecl FDisPhysicsUtil::DisCleanUpRagdoll(class ADishonoredNPCPawn *)
//   0x832b40  public: static class FBox __cdecl FDisPhysicsUtil::DisCalcAABBFromKinematics(class USkeletalMeshComponent *)
//   0x832f90  public: static void __cdecl FDisPhysicsUtil::ImpulseActorBonesAtPos(class FVector const &, class FVector const &, class FVector const &, class UPrimitiveComponent *, float, float, float)
//   0x833150  public: static void __cdecl FDisPhysicsUtil::ImpulseActorBones(class FVector const &, class FVector const &, class UPrimitiveComponent *, float, float, float)
//   0x833340  public: static void __cdecl FDisPhysicsUtil::UniformImpulseActorBones(class FVector const &, class UPrimitiveComponent *, float)
//   0x8334e0  public: static float __cdecl FDisPhysicsUtil::GetComponentMass(class UPrimitiveComponent const *)
//   0x8335d0  unsigned int __cdecl DishonoredUtilities_Physics::EncroachingAnyGeometry(class FVector const &, class FVector const &, class TArray<class AActor const *, class FDefaultAllocator> const *)
//   0x83d890  unsigned int __cdecl DishonoredUtilities_Physics::DisFindSpotLineCheck(struct FDisLineProbeResult &, class FVector const &, class FVector const &, class FVector const &, class TArray<class AActor const *, class FDefaultAllocator> const * const)
//   0x83d930  public: static unsigned int __cdecl FDisPhysicsUtil::DisFindSpot(class FVector const &, class FVector const &, class FVector const &, class FVector &, class TArray<class AActor const *, class FDefaultAllocator> const *)
