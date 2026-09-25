// DishonoredGame/src/disitemcontext_meleeattackplayer.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (34):
//   0x862400  protected: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::EndItemContext_Derived(enum eDisItemContextStatus)
//   0x862410  public: void __thiscall UDisItemContext_MeleeAttackPlayer::ChainInputZone_Enter(void)
//   0x862440  public: void __thiscall UDisItemContext_MeleeAttackPlayer::ChainInputZone_Exit(void)
//   0x862470  public: unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::IsChaining(void)const
//   0x862480  public: unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::HasQueuedChain(void)const
//   0x8640e0  public: virtual unsigned int __thiscall UDisTweaks_MeleeAttackPlayer::EditConditionAskObj_IsConditionMet(class FEditPropertyChain *, wchar_t const *)
//   0x864160  public: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::CanDoContext_GatherInfo_Derived(void)
//   0x864170  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::CalculateDodge(class ADishonoredPawn * const)const
//   0x868570  public: virtual float __thiscall UDisItemContext_MeleeAttackPlayer::GetMaxContextRange(void)const
//   0x868620  protected: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::ApplyHitContact_Derived(class UClass const *, class UClass const *, int, unsigned int)
//   0x868780  protected: virtual class FVector const __thiscall UDisItemContext_MeleeAttackPlayer::GetHitMomentum(unsigned int)const
//   0x8688a0  public: virtual enum eDisFilterResult __thiscall UDisItemContext_MeleeAttackPlayer::FilterUseAttempt(enum eDisItemContextSlot, struct FDisItemContextParams const *)
//   0x868a10  protected: struct FItemLinkedAction const * __thiscall UDisItemContext_MeleeAttackPlayer::CheckSynchedKillAttack(struct FItemLinkedAction const &, class ADishonoredNPCPawn *)
//   0x868aa0  protected: void __thiscall UDisItemContext_MeleeAttackPlayer::ChooseMeleeActions(class ADishonoredNPCPawn *, struct FPawnAction const * &, struct FDisItemAction const * &, struct FItemLinkedAction const * &)
//   0x868db0  protected: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::SetMeleeState_Derived(enum eDisWeaponMeleeState, class UDisItemContext_MeleeAttack *)
//   0x868fc0  public: void __thiscall UDisItemContext_MeleeAttackPlayer::PromoteQueuedChain(void)
//   0x869040  protected: virtual class UClass * __thiscall UDisItemContext_MeleeAttackPlayer::FindHitContactTypeBetterResult(struct FDisLineProbeResult const &, struct FDisLineProbeResult &, class UClass *, enum eDisPawnHitReactionType)
//   0x869190  public: virtual class UClass * __thiscall UDisItemContext_MeleeAttackPlayer::ChooseDamageType(void)const
//   0x871ab0  protected: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::FilterCollisionResults(struct TMemStackArray<struct FDisLineProbeResult> &)
//   0x875df0  protected: virtual enum eDisItemContextStatus __thiscall UDisItemContext_MeleeAttackPlayer::DoContext_Derived_UsePlayer(struct FDisItemContextParam_UsePlayer const &)
//   0x8760e0  public: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::TickContext(float)
//   0x876160  protected: unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::ChooseSweepingTarget(struct TMemStackArray<struct FDisLineProbeResult> *, struct FDisLineProbeResult &, int, int)const
//   0x876900  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::CheckForCollision(struct TMemStackArray<struct FDisLineProbeResult> &)const
//   0x876a60  public: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::OnSuccessfulHit(struct FImpactInfo &)
//   0x876b80  protected: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::DoHitRebound(unsigned int)
//   0x87c150  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttackPlayer::UpdatePawnInfo_Melee(struct FDisMeleeInfo const &)
//   0x87d760  public: static class UClass * __cdecl UDisItemContext_MeleeAttackPlayer::GetPrivateStaticClassUDisItemContext_MeleeAttackPlayer(wchar_t const *)
//   0x87d7f0  public: static void __cdecl UDisItemContext_MeleeAttackPlayer::InitializePrivateStaticClassUDisItemContext_MeleeAttackPlayer(void)
//   0x87fdb0  public: static class UClass * __cdecl UDisItemContext_MeleeAttackPlayer::StaticClassNoInline(void)
//   0x880b80  public: static class UClass * __cdecl UDisTweaks_MeleeAttackPlayer::GetPrivateStaticClassUDisTweaks_MeleeAttackPlayer(wchar_t const *)
//   0x883d30  public: virtual void __thiscall UDisItemContext_MeleeAttackPlayer::InitContext(class UDishonoredInventoryItem *)
//   0x885630  public: static void __cdecl UDisTweaks_MeleeAttackPlayer::InitializePrivateStaticClassUDisTweaks_MeleeAttackPlayer(void)
//   0x885980  public: static class UClass * __cdecl UDisTweaks_MeleeAttackPlayer::StaticClassNoInline(void)
//   0x8859b0  public: virtual float __thiscall UDisItemContext_MeleeAttackPlayer::CalculateMeleeDamage(class AActor *, struct FImpactInfo const *)const
