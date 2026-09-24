// DishonoredGame/src/dishonorednpcpawn_combat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (70):
//   0x7abc60  public: class AController * __thiscall FDisNPCDamageInfo::GetDamageEventInstigator(void)const
//   0x7abc70  public: class FVector __thiscall FDisNPCDamageInfo::GetDamageMomentum(void)const
//   0x7abc90  public: class AActor * __thiscall FDisNPCDamageInfo::GetDamageCausingActor(void)const
//   0x7abca0  public: class FVector __thiscall FDisNPCDamageInfo::GetDamageSourceOrigin(void)const
//   0x7abce0  public: void __thiscall FDisNPCDeathInfo::MarkIgnoreDeath(void)
//   0x7abcf0  public: unsigned int __thiscall FDisNPCDeathInfo::ShouldIgnoreDeath(void)const
//   0x7abd00  public: class FName __thiscall FDisNPCDeathInfo::GetDeadNPCName(void)const
//   0x7abd20  public: unsigned int __thiscall FDisNPCDeathInfo::IsCorpseHandled(void)const
//   0x7abd30  public: void __thiscall FDisNPCDeathInfo::MarkCorpseHandled(void)
//   0x7abd40  public: void __thiscall FDisNPCDeathInfo::ClearCorpseHandled(class ADishonoredPawn *)
//   0x7abd60  public: void __thiscall FDisNPCDeathInfo::MarkIgnoreCorpseCleanup(void)
//   0x7abd70  public: void __thiscall FDisNPCDeathInfo::MarkSpawnedDead(void)
//   0x7abd80  public: unsigned int __thiscall FDisNPCDeathInfo::ShouldIgnoreCorpseCleanup(void)const
//   0x7abd90  public: void __thiscall FDisNPCDeathInfo::OnOtherActorTerminated(class AActor const &)
//   0x7abdb0  public: void __thiscall FDisNPCDeathInfo::AccountForStats(class ADishonoredPlayerPawn *)
//   0x7abdd0  public: enum EAIAwareness __thiscall FDisNPCDeathInfo::GetAwareness(void)const
//   0x7abde0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::ArkIsIncapacitated(void)const
//   0x7abe10  public: virtual unsigned int __thiscall ADishonoredNPCPawn::ArkIsDeadOrDestroyed(void)const
//   0x7abe40  protected: virtual unsigned int __thiscall ADishonoredNPCPawn::IsVulnerable_Derived(enum eDisVulnerabilityType, unsigned int)const
//   0x7abeb0  public: virtual void __thiscall ADishonoredNPCPawn::OnStunned(float)
//   0x7abf10  public: virtual void __thiscall ADishonoredNPCPawn::OnMeleeIncoming_End(class UDisItemContext_MeleeAttack *)
//   0x7abf40  public: void __thiscall ADishonoredNPCPawn::AcknowledgeHostility(class ADishonoredPawn * const)
//   0x7abf80  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanMoveInAimStance(void)const
//   0x7abfe0  protected: unsigned int __thiscall ADishonoredNPCPawn::IsHittingLegs(struct FTraceHitInfo const &)const
//   0x7ac020  public: unsigned int __thiscall ADishonoredNPCPawn::TryToHandleDramaAssassination(void)
//   0x7ae6b0  public: void __thiscall FDisNPCDamageInfo::SetDamageInfo_WithKismet(class ADishonoredPawn *, class ADishonoredPawn *, class AActor *, class UClass *, class FVector, class FVector, int, int)
//   0x7ae7b0  class FArchive & __cdecl operator<<(class FArchive &, struct FDisNPCDeathInfo &)
//   0x7ae850  public: virtual unsigned int __thiscall ADishonoredNPCPawn::ParryEquippedItem(enum EDisEquipUsage, class UDisItemContext_MeleeAttack *)
//   0x7ae8a0  public: void __thiscall ADishonoredNPCPawn::RequestStateChangeWithDamage(struct FStateNPCMasterAction_Param &, class FVector const &, class ADishonoredPawn *, class UClass *, class AActor *)
//   0x7ae930  public: virtual void __thiscall ADishonoredNPCPawn::DoPostDeathHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class UClass *, class AController *)
//   0x7ae980  public: virtual void __thiscall ADishonoredNPCPawn::OnMeleeOutgoing_AttackZone(class UDisItemContext_MeleeAttack *)
//   0x7ae990  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsFalling(void)const
//   0x7ae9c0  public: unsigned int __thiscall ADishonoredNPCPawn::IsAiming(void)const
//   0x7aea10  public: unsigned int __thiscall ADishonoredNPCPawn::IsNavigationallyObstructed(enum ECardinalDirection, float)const
//   0x7aeb50  public: unsigned int __thiscall ADishonoredNPCPawn::IsPhysicallyObstructed(enum ECardinalDirection, float, unsigned int)const
//   0x7aedc0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanObstructAllyAttacks(void)const
//   0x7b3130  public: unsigned int __thiscall ADishonoredNPCPawn::RollAimToHit(class UDisItemContext const *, class AActor * const, class FVector const &)const
//   0x7b3240  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsWounded(void)const
//   0x7b32a0  public: virtual void __thiscall ADishonoredNPCPawn::DoWeakHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class UClass *)
//   0x7b3380  public: virtual void __thiscall ADishonoredNPCPawn::DoCustomDeathHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor *, class ADishonoredPawn *, class UClass *)
//   0x7b3410  public: virtual void __thiscall ADishonoredNPCPawn::DoStrongHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x7b34a0  public: virtual void __thiscall ADishonoredNPCPawn::DoKnockdownHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x7b3590  public: virtual unsigned int __thiscall ADishonoredNPCPawn::DoesActorCauseHitRebound(int, class UClass *)const
//   0x7b3690  public: virtual enum eDisMeleeResponse __thiscall ADishonoredNPCPawn::OnMeleeIncoming_Versusable(class UDisItemContext_MeleeAttack *)
//   0x7b36f0  public: void __thiscall ADishonoredNPCPawn::DodgeIncomingAttack(class UDisItemContext_MeleeAttack *)
//   0x7b3800  public: virtual void __thiscall ADishonoredNPCPawn::OnMeleeOutgoing_Versusable(class UDisItemContext_MeleeAttack *)
//   0x7b3820  public: virtual void __thiscall ADishonoredNPCPawn::OnMeleeOutgoing_End(class UDisItemContext_MeleeAttack *)
//   0x7b3840  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsStunned(unsigned int &)const
//   0x7b38a0  public: virtual void __thiscall ADishonoredNPCPawn::NotifyStopStun(void)
//   0x7b38c0  protected: void __thiscall ADishonoredNPCPawn::UpdateSleepStats(class ADishonoredPawn * const, class ADishonoredPlayerPawn * const, class UDisTweaks_NPCPawn const *)const
//   0x7b3960  protected: void __thiscall ADishonoredNPCPawn::UpdateDeathStats(class ADishonoredPawn * const, class ADishonoredPlayerPawn * const, class UDisTweaks_NPCPawn const *)const
//   0x7b3ae0  public: unsigned int __thiscall ADishonoredNPCPawn::ShouldDeathDamageSeverLimbs(class ADishonoredPawn *, class UClass *, unsigned int)const
//   0x7b3c50  protected: virtual void __thiscall ADishonoredNPCPawn::PlayDying_Native_Derived(class AController *, class UClass *)
//   0x7b3e70  public: void __thiscall ADishonoredNPCPawn::SetupDeathInfo(void)
//   0x7b3f50  public: virtual void __thiscall ADishonoredNPCPawn::DoProjectileHitReaction(class FVector const &, class FVector const &, struct FTraceHitInfo const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x7b41b0  public: unsigned int __thiscall ADishonoredNPCPawn::IsDramaAssassinationTarget(void)const
//   0x7b41f0  protected: void __thiscall ADishonoredNPCPawn::SendCustomDamageToKismet(class AActor *, class UClass *, int)
//   0x7b7900  protected: void __thiscall ADishonoredNPCPawn::CheckDeathDamageForLimbSevering(class ADishonoredPawn *, class UClass *, class FVector const &)
//   0x7bc580  public: virtual void __thiscall ADishonoredNPCPawn::DoThrowHitReaction(class FVector const &, class FVector const &, class AActor * const, class ADishonoredPawn * const, class UClass *)
//   0x7c14a0  protected: virtual unsigned int __thiscall ADishonoredNPCPawn::CanDodge(class ADishonoredPawn const * const, class UDisItemContext_MeleeAttack const * const)const
//   0x7c1590  public: virtual void __thiscall ADishonoredNPCPawn::GetMeleeCombatInfo(struct FDisMeleeInfo &, class UDisItemContext const *)const
//   0x7c1620  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsInCombat(void)const
//   0x7c1680  public: virtual enum eDisMeleeResponse __thiscall ADishonoredNPCPawn::OnMeleeIncoming_Parryable(class UDisItemContext_MeleeAttack *)
//   0x7c1750  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAFighter(void)const
//   0x7c27e0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::ChooseAndTriggerDeathEvent_Native(class UClass *)
//   0x7c2b40  public: virtual void __thiscall ADishonoredNPCPawn::PlayDying_Native(class AController *, class UClass *, class FVector)
//   0x7c43a0  protected: virtual enum eDisPawnHitReactionType __thiscall ADishonoredNPCPawn::DetermineHitReactionType_Derived(class UClass *, int, class FVector const &, struct FTraceHitInfo const *, class AActor *, class ADishonoredPawn *)const
//   0x7c4690  public: void __thiscall ADishonoredNPCPawn::DoubleTapToEnsureDeath(void)
//   0x7c47a0  public: virtual enum eDisMeleeResponse __thiscall ADishonoredNPCPawn::OnMeleeIncoming_Dodgeable(class UDisItemContext_MeleeAttack *)
//   0x7c6d50  public: virtual void __thiscall ADishonoredNPCPawn::TakeDamage_Native(int &, class AController *, class FVector, class FVector &, class UClass *, struct FTraceHitInfo, class AActor *)
