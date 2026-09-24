// DishonoredGame/src/disaisubstatefirepistol.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (32):
//   0x765410  public: void __thiscall UDisAISubStateFirePistol::ForceStateExit(void)
//   0x765420  private: unsigned int __thiscall UDisAISubStateFirePistol::UsingLastSeenLocation(void)const
//   0x765450  private: virtual struct FDisLocoRequest * __thiscall UDisAISubStateFirePistol::GetDesiresLocoRequest(void)
//   0x765460  private: virtual struct FDisLookAtRequest * __thiscall UDisAISubStateFirePistol::GetDesiresLookAtRequest(void)
//   0x765470  private: virtual struct FDisBodyIntentionRequest * __thiscall UDisAISubStateFirePistol::GetDesiresBodyIntentionRequest(void)
//   0x768a50  protected: virtual void __thiscall UDisAISubStateFirePistol::RequestStateExit_Derived(void)
//   0x768aa0  private: virtual unsigned int __thiscall UDisAISubStateFirePistol::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x770dd0  private: unsigned int __thiscall UDisAISubStateFirePistol::ConfirmAllyAndNeutralSafety(void)const
//   0x7736c0  private: class FVector __thiscall UDisAISubStateFirePistol::GetTargetLocation(void)const
//   0x775470  public: static class UClass * __cdecl UDisAISubStateFirePistol::GetPrivateStaticClassUDisAISubStateFirePistol(wchar_t const *)
//   0x775500  private: float __thiscall UDisAISubStateFirePistol::GetAimDiscrepancy(void)const
//   0x775660  private: unsigned int __thiscall UDisAISubStateFirePistol::PullTheTrigger(void)
//   0x775730  private: void __thiscall UDisAISubStateFirePistol::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x7774b0  public: virtual void __thiscall FDisAISubStateFirePistolControlled_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x777580  public: virtual void __thiscall FDisAISubStateFirePistolCombat_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x777670  private: virtual void __thiscall UDisAISubStateFirePistol::BeginDestroy(void)
//   0x77bb50  public: static void __cdecl UDisAISubStateFirePistol::InitializePrivateStaticClassUDisAISubStateFirePistol(void)
//   0x780230  public: static class UClass * __cdecl UDisAISubStateFirePistol::StaticClassNoInline(void)
//   0x780260  public: virtual void __thiscall UDisAISubStateFirePistol::EndSubState_Derived(unsigned int)
//   0x781bb0  public: __thiscall FDisAISubStateFirePistolControlled_Param::FDisAISubStateFirePistolControlled_Param(class AActor *, unsigned int, int)
//   0x781c10  public: __thiscall FDisAISubStateFirePistolCombat_Param::FDisAISubStateFirePistolCombat_Param(struct FDisAttentionProxy const &, unsigned int)
//   0x783230  public: static class UClass * __cdecl UDisTweaks_AISubState_FirePistol::GetPrivateStaticClassUDisTweaks_AISubState_FirePistol(wchar_t const *)
//   0x784580  public: static void __cdecl UDisTweaks_AISubState_FirePistol::InitializePrivateStaticClassUDisTweaks_AISubState_FirePistol(void)
//   0x784d50  public: static class UClass * __cdecl UDisTweaks_AISubState_FirePistol::StaticClassNoInline(void)
//   0x787220  public: virtual void __thiscall UDisAISubStateFirePistol::BeginSubState_Derived(void)
//   0x787430  public: virtual void __thiscall UDisAISubStateFirePistol::RefreshSubState(float)
//   0x787500  private: void __thiscall UDisAISubStateFirePistol::ChangePistolState(enum DisAIPistolState)
//   0x78adf0  private: void __thiscall UDisAISubStateFirePistol::UpdateState_Relaxed(float, class UDishonoredWeapon_Ranged * const)
//   0x78b060  private: void __thiscall UDisAISubStateFirePistol::UpdateState_Aiming(float, class UDishonoredWeapon_Ranged * const)
//   0x78b300  private: void __thiscall UDisAISubStateFirePistol::UpdateState_Recovering(float, class UDishonoredWeapon_Ranged * const)
//   0x78b3a0  private: void __thiscall UDisAISubStateFirePistol::UpdateState_Reloading(float, class UDishonoredWeapon_Ranged * const)
//   0x78c700  public: virtual void __thiscall UDisAISubStateFirePistol::TickState(float)
