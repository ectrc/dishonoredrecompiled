// DishonoredGame/src/disitemcontext.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (39):
//   0x861ec0  public: static void __cdecl UDisItemContext::InitializePrivateStaticClassUDisItemContext(void)
//   0x861ee0  public: virtual void __thiscall UDisItemContext::InitContext(class UDishonoredInventoryItem *)
//   0x861f10  public: virtual void __thiscall UDisItemContext::CanDoContext_GatherInfo(struct FDisItemContextParams const *)
//   0x861f30  protected: void __thiscall UDisItemContext::ObliterateLastUseTime(void)
//   0x861f40  public: void __thiscall UDisItemContext::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x861f60  public: class ADishonoredPawn * __thiscall UDisItemContext::GetOwningPawn(void)
//   0x861f70  public: int __thiscall UDisItemContext::GetOwnerTickTag(void)const
//   0x861fa0  protected: void __thiscall UDisItemContext::DEBUG_ConfirmNPCResult(unsigned int, unsigned long)const
//   0x861fe0  public: int __thiscall FItemLinkedAction::GetSyncFlags(void)const
//   0x863010  public: __thiscall FDisMeleeInfo::FDisMeleeInfo(void)
//   0x863070  protected: void __thiscall UDisItemContext::ResetElapsedTimeSinceLastUse(void)
//   0x863090  public: void __thiscall UDisItemContext::ItemActionEnded(class UStateSharedActionBase *)
//   0x865830  public: class ADishonoredPawn * __thiscall FDisMeleeInfo::GetCombatTarget(void)const
//   0x865840  public: void __thiscall UDisItemContext::TerminateContext(void)
//   0x865870  public: virtual unsigned int __thiscall UDisItemContext::IsInhibitedByCooldown(struct FDisItemContextParams const *)const
//   0x8658c0  protected: struct FDisItemContextOverride const * __thiscall UDisItemContext::FindContextOverride(void)const
//   0x865990  public: unsigned int __thiscall UDisItemContext::IsContextDisabled(void)const
//   0x8659c0  public: unsigned int __thiscall UDisItemContext::CanDoContext(struct FDisItemContextParams const *)const
//   0x865a70  public: virtual float __thiscall UDisItemContext::CalculateContextCooldown(struct FDisItemContextParams const *)const
//   0x865aa0  protected: unsigned int __thiscall UDisItemContext::CheckLinkedItemAction(struct FItemLinkedAction const &, class ADishonoredPawn *, struct FStatePlayerMasterAction_Param &, struct FStateNPCMasterAction_Param &)const
//   0x865b10  public: virtual unsigned int __thiscall UDisItemContext::IsActorInRange(class AActor const *)const
//   0x865c40  public: virtual unsigned int __thiscall UDisItemContext::IsPawnRegionInRange(class ADishonoredPawn const *, enum eDisHitRegion, enum eDisRegionFocusType, enum eDisHitRegion, enum eDisRegionFocusType, float)const
//   0x865d60  public: virtual float __thiscall UDisItemContext::GetMinContextRange(void)const
//   0x865d70  public: virtual float __thiscall UDisItemContext::GetMaxContextRange(void)const
//   0x865dc0  public: virtual float __thiscall UDisItemContext::GetIdealContextRange(void)const
//   0x865dd0  public: struct FItemLinkedAction const * __thiscall FItemLinkedAction::EnsureActionsSupported(class ADishonoredPlayerPawn *, class ADishonoredNPCPawn *)const
//   0x86d610  public: static class UClass * __cdecl UDisItemContext::GetPrivateStaticClassUDisItemContext(wchar_t const *)
//   0x8703f0  public: static class UClass * __cdecl UDisItemContext::StaticClassNoInline(void)
//   0x870420  public: void __thiscall UDisItemContext::SetPawnInfo_Melee_ImpactInfo(struct FImpactInfo const &)
//   0x8704a0  public: void __thiscall UDisItemContext::SetPawnInfo_Melee_HitActor(class AActor *)
//   0x8747d0  public: virtual void __thiscall UDisItemContext::BeginDestroy(void)
//   0x874810  public: void __thiscall UDisItemContext::SetPawnInfo_Melee(struct FDisMeleeInfo const &)
//   0x87c070  public: virtual unsigned int __thiscall UDisItemContext::UpdatePawnInfo_Melee(struct FDisMeleeInfo const &)
//   0x8807b0  public: static class UClass * __cdecl UDisTweaks_ItemContext::GetPrivateStaticClassUDisTweaks_ItemContext(wchar_t const *)
//   0x883720  public: static void __cdecl UDisTweaks_ItemContext::InitializePrivateStaticClassUDisTweaks_ItemContext(void)
//   0x883740  public: void __thiscall UDisItemContext::EndItemContext(enum eDisItemContextStatus)
//   0x884be0  public: static class UClass * __cdecl UDisTweaks_ItemContext::StaticClassNoInline(void)
//   0x884c10  public: enum eDisItemContextStatus __thiscall UDisItemContext::DoContext(struct FDisItemContextParams const *)
//   0x884f60  public: virtual unsigned int __thiscall UDisItemContext_NPCAttractSpell::CancelContext(void)
