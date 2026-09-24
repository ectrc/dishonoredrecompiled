// DishonoredGame/src/disgamecrowdagentskeletalrat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (63):
//   0x8b0260  public: static void __cdecl ADisGameCrowdAgentSkeletalRat::InitializePrivateStaticClassADisGameCrowdAgentSkeletalRat(void)
//   0x8b0280  public: static void __cdecl UDisTweaks_GameCrowdAgentSkeletalRat::InitializePrivateStaticClassUDisTweaks_GameCrowdAgentSkeletalRat(void)
//   0x8b02a0  public: static void __cdecl UDisContactType_Rat::InitializePrivateStaticClassUDisContactType_Rat(void)
//   0x8b02c0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::PostBeginPlay(void)
//   0x8b0320  private: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::OnAnimEnd(class UAnimNodeSequence *, float, float)
//   0x8b0350  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x8b0380  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::ResetPooledAgent(void)
//   0x8b03a0  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::ShouldPerformCrowdSimulation(float)
//   0x8b03d0  public: virtual float __thiscall ADisGameCrowdAgentSkeletalRat::GetAttractionFactor(struct FGameCrowdAttractor const &)const
//   0x8b04c0  public: void __thiscall ADisGameCrowdAgentSkeletalRat::InhibitMovement(float)
//   0x8b04e0  private: class FVector __thiscall ADisGameCrowdAgentSkeletalRat::GetMeshLoc(void)const
//   0x8b0550  public: virtual enum IDisPossessableInterface::EPossessionAvailability __thiscall ADisGameCrowdAgentSkeletalRat::GetPossessionAvailability(void)const
//   0x8b0560  public: virtual class USkeletalMeshComponent * __thiscall ADisGameCrowdAgentSkeletalRat::GetPossessableSkelMesh(void)
//   0x8b0570  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x8b0590  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::DisableSoulRendering(void)
//   0x8b05a0  public: virtual class FVector __thiscall ADisGameCrowdAgentSkeletalRat::GetDamageSourceLocation(void)const
//   0x8b05d0  private: void __thiscall ADisGameCrowdAgentSkeletalRat::DetachFromTarget(void)
//   0x8b0680  public: void __thiscall ADisGameCrowdAgentSkeletalRat::SetFatalAttraction(class FVector const &, float, float, float)
//   0x8b06b0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x8b57c0  public: virtual class FVector __thiscall ADisGameCrowdAgentSkeletalRat::GetPrepossessCameraFocus(void)const
//   0x8b5800  public: virtual class ADisPossessablePawn * __thiscall ADisGameCrowdAgentSkeletalRat::GetPossessablePawn(void)
//   0x8b5830  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::TickPossessionOnPossessable(void)
//   0x8b5870  private: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::CanAttack(void)const
//   0x8b5980  private: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::CanPerformAttachedAttack(void)const
//   0x8b59c0  private: void __thiscall ADisGameCrowdAgentSkeletalRat::UpdateLandingLocation(void)
//   0x8b5c30  public: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::CanReach(class ADisGameCrowdAgentSkeletalRat *)
//   0x8ba3e0  private: class FName __thiscall ADisGameCrowdAgentSkeletalRat::PickRandomTargetSocket(class IDisRatTargetInterface *, unsigned int, class FVector const *)
//   0x8c3250  public: static class UClass * __cdecl UDisContactType_Rat::GetPrivateStaticClassUDisContactType_Rat(wchar_t const *)
//   0x8c32e0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::GetPossessTargetLocations(struct TMemStackArray<class FVector> &)const
//   0x8c3310  private: void __thiscall ADisGameCrowdAgentSkeletalRat::AttachToTarget(class IDisRatTargetInterface *, class FName)
//   0x8c6de0  public: static class UClass * __cdecl UDisContactType_Rat::StaticClassNoInline(void)
//   0x8c6e10  protected: virtual void __thiscall UDisTweaks_GameCrowdAgentSkeletalRat::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x8c86f0  public: void __thiscall ADisGameCrowdAgentSkeletalRat::Explode(class UClass *, class AActor *, class FVector *)
//   0x8c8840  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::TakeDamage_Native(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x8ca0b0  public: static class UClass * __cdecl ADisGameCrowdAgentSkeletalRat::GetPrivateStaticClassADisGameCrowdAgentSkeletalRat(wchar_t const *)
//   0x8cc030  public: static class UClass * __cdecl ADisGameCrowdAgentSkeletalRat::StaticClassNoInline(void)
//   0x8cceb0  public: static class UClass * __cdecl UDisTweaks_GameCrowdAgentSkeletalRat::GetPrivateStaticClassUDisTweaks_GameCrowdAgentSkeletalRat(wchar_t const *)
//   0x8cf5e0  public: static class UClass * __cdecl UDisTweaks_GameCrowdAgentSkeletalRat::StaticClassNoInline(void)
//   0x8cf610  private: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::StopRagdolling(void)
//   0x8d2640  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::InitializeAgent(class AActor *, class AGameCrowdAgent *, float, unsigned int, unsigned int)
//   0x8d27b0  public: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::IsWhite(void)const
//   0x8d2820  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::CanInteract(struct FCanInteractParams const &)const
//   0x8d2860  public: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisGameCrowdAgentSkeletalRat::GetInteractableTweaks_Derived(void)const
//   0x8d2890  public: virtual class UDisTweaks_Possessable * __thiscall ADisGameCrowdAgentSkeletalRat::GetPossessableTweaks_Derived(void)const
//   0x8d28c0  private: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::UpdateImpactLocation(unsigned int)
//   0x8d2f10  private: void __thiscall ADisGameCrowdAgentSkeletalRat::CleanUpAttack(void)
//   0x8d30d0  private: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x8d3150  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::PlayParticleEffect(class UAnimNotify_PlayParticleEffect const *)
//   0x8d41f0  private: void __thiscall ADisGameCrowdAgentSkeletalRat::SetRatPhysics(enum EDisRatPhysics, unsigned int)
//   0x8d43d0  private: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::InitAttack(enum EDisRatAttackType, class IDisRatTargetInterface *)
//   0x8d4510  private: void __thiscall ADisGameCrowdAgentSkeletalRat::PerformAttackPhysics(float)
//   0x8d4c80  private: void __thiscall ADisGameCrowdAgentSkeletalRat::Fly(class FVector const &)
//   0x8d4df0  private: void __thiscall ADisGameCrowdAgentSkeletalRat::PerformFlyPhysics(float)
//   0x8d58a0  public: unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::StartRagdolling(class FVector const &)
//   0x8d5c10  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::AddedToPool(void)
//   0x8d5d10  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::Tick(float, enum ELevelTick)
//   0x8d5da0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::performPhysics(float)
//   0x8d5f90  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::TickSpecial(float)
//   0x8d62d0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::PlayDeath(class AController *, class FVector, class UClass *, class AActor *)
//   0x8d66d0  public: virtual unsigned int __thiscall ADisGameCrowdAgentSkeletalRat::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x8d68b0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::InitPossessionOnPossessable(void)
//   0x8d68d0  public: virtual void __thiscall ADisGameCrowdAgentSkeletalRat::TermPossessionOnPossessable(void)
//   0x8d69f0  protected: virtual unsigned int __thiscall UDisTweaks_GameCrowdAgentSkeletalRat::FixupDefaults_Derived(void)
