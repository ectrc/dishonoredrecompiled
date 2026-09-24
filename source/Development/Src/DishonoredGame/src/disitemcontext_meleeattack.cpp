// DishonoredGame/src/disitemcontext_meleeattack.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (41):
//   0x8620a0  public: virtual enum eDisFilterResult __thiscall UDisItemContext_MeleeAttack::FilterUseAttempt(enum eDisItemContextSlot, struct FDisItemContextParams const *)
//   0x8620e0  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttack::CanDoDamage(void)const
//   0x862140  public: enum eDisWeaponMeleeState __thiscall UDisItemContext_MeleeAttack::GetMeleeState(void)const
//   0x862150  public: unsigned int __thiscall UDisItemContext_MeleeAttack::InDamageZone(void)const
//   0x862180  protected: unsigned int __thiscall UDisItemContext_MeleeAttack::RequestMeleeAction_CheckOnly(class ADishonoredPawn const *, struct FDisItemAction const *, struct FStateSharedActionBase_Param &)const
//   0x8621c0  public: void __thiscall UDisItemContext_MeleeAttack::MakeInterruptable(void)
//   0x8621d0  public: void __thiscall UDisItemContext_MeleeAttack::StopInterruptable(void)
//   0x863b60  protected: virtual class UClass * __thiscall UDisItemContext_MeleeAttack::ChooseDamageType(void)const
//   0x866350  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_Begin(void)
//   0x8663a0  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_AttackZone(enum EDisSwingDirection, float)
//   0x866450  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_Parryable(void)
//   0x8664c0  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_Versusable(void)
//   0x866560  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_End(void)
//   0x8665e0  protected: virtual class FVector const __thiscall UDisItemContext_MeleeAttack::GetHitMomentum(unsigned int)const
//   0x866670  protected: virtual float __thiscall UDisItemContext_MeleeAttack::CalculateMeleeDamage(class AActor *, struct FImpactInfo const *)const
//   0x866720  protected: int __thiscall UDisItemContext_MeleeAttack::DealHitDamage(class AActor *, int, class UClass *, class FVector const &, struct FDisLineProbeResult const &)
//   0x866830  public: void __thiscall UDisItemContext_MeleeAttack::SetMeleeState(enum eDisWeaponMeleeState, class UDisItemContext_MeleeAttack *)
//   0x866890  protected: class UClass * __thiscall UDisItemContext_MeleeAttack::RefineExistingHitContactType(struct FDisLineProbeResult const &, struct FDisLineProbeResult &, class UClass *, enum eDisPawnHitReactionType)
//   0x866cb0  protected: unsigned int __thiscall UDisItemContext_MeleeAttack::RequestMeleeAction(class ADishonoredPawn *, struct FDisItemAction const *, struct FStateSharedActionBase_Param &)
//   0x866d10  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttack::IsParryable(class ADishonoredPawn const *)const
//   0x866da0  public: unsigned int __thiscall UDisItemContext_MeleeAttack::IsDodgeable(class ADishonoredPawn const *)const
//   0x866e30  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttack::CalculateDodge(class ADishonoredPawn * const)const
//   0x866ec0  public: enum EDisSwingDirection __thiscall UDisItemContext_MeleeAttack::CalculateSwingDirection(void)const
//   0x86d730  protected: virtual enum eDisItemContextStatus __thiscall UDisItemContext_MeleeAttack::StartMeleeAttack(void)
//   0x86d7c0  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_Dodgeable(void)
//   0x86d820  protected: virtual class UClass * __thiscall UDisItemContext_MeleeAttack::FindHitContactTypeBetterResult(struct FDisLineProbeResult const &, struct FDisLineProbeResult &, class UClass *, enum eDisPawnHitReactionType)
//   0x870a60  public: static void __cdecl UDisItemContext_MeleeAttack::InitializePrivateStaticClassUDisItemContext_MeleeAttack(void)
//   0x870a80  public: virtual void __thiscall UDisItemContext_MeleeAttack::DoHit(struct FDisLineProbeResult &, unsigned int)
//   0x870d90  public: virtual unsigned int __thiscall UDisItemContext_MeleeAttack::UpdatePawnInfo_Melee(struct FDisMeleeInfo const &)
//   0x874bb0  public: virtual void __thiscall UDisItemContext_MeleeAttack::TickContext(float)
//   0x874c30  public: virtual void __thiscall UDisItemContext_MeleeAttack::OnMeleeZone_Damaging(void)
//   0x874cb0  protected: virtual void __thiscall UDisItemContext_MeleeAttack::DoHitRebound(unsigned int)
//   0x874d20  public: virtual void __thiscall UDisItemContext_MeleeAttack::ProcessMeleeCollision(void)
//   0x874fd0  protected: virtual void __thiscall UDisItemContext_MeleeAttack::OnSuccessfulHit(struct FImpactInfo &)
//   0x87c090  public: static class UClass * __cdecl UDisItemContext_MeleeAttack::GetPrivateStaticClassUDisItemContext_MeleeAttack(wchar_t const *)
//   0x87d580  public: static class UClass * __cdecl UDisItemContext_MeleeAttack::StaticClassNoInline(void)
//   0x880840  public: static class UClass * __cdecl UDisTweaks_MeleeAttack::GetPrivateStaticClassUDisTweaks_MeleeAttack(wchar_t const *)
//   0x883890  public: virtual void __thiscall UDisItemContext_MeleeAttack::InitContext(class UDishonoredInventoryItem *)
//   0x883a10  protected: virtual void __thiscall UDisItemContext_MeleeAttack::ItemActionEnded_Derived(class UStateSharedActionBase *)
//   0x884d30  public: static void __cdecl UDisTweaks_MeleeAttack::InitializePrivateStaticClassUDisTweaks_MeleeAttack(void)
//   0x885570  public: static class UClass * __cdecl UDisTweaks_MeleeAttack::StaticClassNoInline(void)
