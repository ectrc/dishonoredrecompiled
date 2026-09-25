// DishonoredGame/src/disitemcontext_npcteleportspell.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x886570  public: static void __cdecl UDisItemContext_NPCTeleportSpell::InitializePrivateStaticClassUDisItemContext_NPCTeleportSpell(void)
//   0x886590  public: static void __cdecl UDisTweaks_NPCTeleportSpell::InitializePrivateStaticClassUDisTweaks_NPCTeleportSpell(void)
//   0x8865b0  public: virtual unsigned int __thiscall UDisItemContext_NPCTeleportSpell::CancelContext(void)
//   0x8865c0  public: void __thiscall UDisItemContext_NPCTeleportSpell::PrepareManualTeleport(enum EDisTeleportIntention, float, class FVector const &)
//   0x886600  public: void __thiscall UDisItemContext_NPCTeleportSpell::CancelManualTeleport(void)
//   0x886610  public: virtual enum eDisFilterResult __thiscall UDisItemContext_NPCTeleportSpell::FilterUseAttempt(enum eDisItemContextSlot, struct FDisItemContextParams const *)
//   0x886680  protected: virtual void __thiscall UDisItemContext_NPCAttractSpell::ItemActionEnded_Derived(class UStateSharedActionBase *)
//   0x888db0  public: virtual void __thiscall UDisItemContext_NPCTeleportSpell::InitContext(class UDishonoredInventoryItem *)
//   0x888de0  class FVector __cdecl CalculateRandomTeleportTarget(class FVector, class FVector, class AActor *, float, float)
//   0x888ec0  protected: class FVector __thiscall UDisItemContext_NPCTeleportSpell::GetPerceivedTargetLocation(class AActor *)const
//   0x888f80  public: virtual void __thiscall UDisItemContext_NPCTeleportSpell::PreGameSave(void)
//   0x888fa0  protected: void __thiscall UDisItemContext_NPCTeleportSpell::BecomeUpright(void)
//   0x88c160  protected: virtual unsigned int __thiscall UDisItemContext_NPCTeleportSpell::IsInhibitedByCooldown(struct FDisItemContextParams const *)const
//   0x88c210  protected: class FVector __thiscall UDisItemContext_NPCTeleportSpell::CalculateTeleportSpot(class FVector, float)const
//   0x88c6b0  protected: void __thiscall UDisItemContext_NPCTeleportSpell::ShowAttachments(void)
//   0x891940  public: virtual float __thiscall UDisItemContext_NPCTeleportSpell::CalculateContextCooldown(struct FDisItemContextParams const *)const
//   0x8919d0  protected: void __thiscall UDisItemContext_NPCTeleportSpell::EnterBrimstoneDimension(void)
//   0x891c30  protected: void __thiscall UDisItemContext_NPCTeleportSpell::HideAttachments(void)
//   0x891d20  protected: void __thiscall UDisItemContext_NPCTeleportSpell::StartReappearingState(void)
//   0x893bf0  protected: virtual enum eDisItemContextStatus __thiscall UDisItemContext_NPCTeleportSpell::DoContext_Derived_UseNPC(struct FDisItemContextParam_UseNPC const &)
//   0x893de0  protected: void __thiscall UDisItemContext_NPCTeleportSpell::StartTesseractState(void)
//   0x894ea0  public: virtual void __thiscall UDisItemContext_NPCTeleportSpell::TickContext(float)
//   0x895010  protected: void __thiscall UDisItemContext_NPCTeleportSpell::ExitBrimstoneDimension(void)
//   0x898250  public: static class UClass * __cdecl UDisItemContext_NPCTeleportSpell::GetPrivateStaticClassUDisItemContext_NPCTeleportSpell(wchar_t const *)
//   0x8982e0  protected: virtual void __thiscall UDisItemContext_NPCTeleportSpell::EndItemContext_Derived(enum eDisItemContextStatus)
//   0x899120  public: static class UClass * __cdecl UDisItemContext_NPCTeleportSpell::StaticClassNoInline(void)
//   0x89cde0  public: static class UClass * __cdecl UDisTweaks_NPCTeleportSpell::GetPrivateStaticClassUDisTweaks_NPCTeleportSpell(wchar_t const *)
//   0x89ce70  protected: virtual unsigned int __thiscall UDisItemContext_NPCTeleportSpell::CanDoContext_Derived_UseNPC(struct FDisItemContextParam_UseNPC const &)const
//   0x89f0c0  public: static class UClass * __cdecl UDisTweaks_NPCTeleportSpell::StaticClassNoInline(void)
