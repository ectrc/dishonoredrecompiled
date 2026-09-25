// DishonoredGame/src/disbehaviorcombatmelee.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x745a00  public: virtual void __thiscall UDisBehaviorCombatMelee::RefreshCallback_MeleeChase(class UDisAISubState *, float)
//   0x745a20  public: virtual void __thiscall UDisBehaviorCombatMelee::RefreshCallback_Stand(class UDisAISubState *, float)
//   0x749030  public: static class FVector __cdecl UDisBehaviorCombatMelee::ComputeNewMeleePosition(class UDishonoredAIBrain const *, struct FDisAttentionProxy const &, class FVector &)
//   0x7491d0  private: virtual unsigned int __thiscall UDisBehaviorCombatMelee::CanLoseBustedTarget(void)const
//   0x74cab0  protected: virtual unsigned char const * __thiscall UDisBehaviorCombatMelee::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74cb20  protected: void __thiscall UDisBehaviorCombatMelee::AllowMeleeAttacks(void)
//   0x74cb60  public: virtual void __thiscall UDisBehaviorCombatEliteGuard::RefreshThoughts(float)
//   0x74cba0  protected: virtual void __thiscall UDisBehaviorCombatMelee::SetupEnemy_Derived(struct FDisAttentionProxy const &)
//   0x74cbd0  public: virtual unsigned int __thiscall UDisBehaviorCombatMelee::IsWillingToStartRangedAction(void)const
//   0x74cc50  public: virtual unsigned int __thiscall UDisBehaviorCombatMelee::ConfirmAllyAndNeutralSafety(void)const
//   0x751930  public: virtual void __thiscall UDisBehaviorCombatMelee::InitBehavior(class UDishonoredAIBrain * const)
//   0x757030  public: static class UClass * __cdecl UDisBehaviorCombatMelee::GetPrivateStaticClassUDisBehaviorCombatMelee(wchar_t const *)
//   0x759d60  public: static void __cdecl UDisBehaviorCombatMelee::InitializePrivateStaticClassUDisBehaviorCombatMelee(void)
//   0x75ad50  public: static class UClass * __cdecl UDisBehaviorCombatMelee::StaticClassNoInline(void)
//   0x75ad80  public: static class UClass * __cdecl UDisTweaks_AIBehavior_CombatMelee::GetPrivateStaticClassUDisTweaks_AIBehavior_CombatMelee(wchar_t const *)
//   0x75c710  public: static void __cdecl UDisTweaks_AIBehavior_CombatMelee::InitializePrivateStaticClassUDisTweaks_AIBehavior_CombatMelee(void)
//   0x75c730  public: virtual void __thiscall UDisBehaviorCombatMelee::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x75c770  protected: virtual void __thiscall UDisBehaviorCombatMelee::OnBehaviorPause(unsigned int)
//   0x75d760  public: static class UClass * __cdecl UDisTweaks_AIBehavior_CombatMelee::StaticClassNoInline(void)
//   0x7626f0  protected: void __thiscall UDisBehaviorCombatMelee::EnsureEngageSubState(class UDisAISubState *)
//   0x762750  protected: void __thiscall UDisBehaviorCombatMelee::EnsureChaseSubState(void)
//   0x762790  protected: void __thiscall UDisBehaviorCombatMelee::EnsureFindShootPositionSubState(void)
//   0x762810  protected: void __thiscall UDisBehaviorCombatMelee::EnsureStandSubState(void)
//   0x762860  public: virtual void __thiscall UDisBehaviorCombatMelee::RequestStateExitCallback_FindShootingPosition(class UDishonoredNativeState *)
//   0x762900  protected: virtual unsigned int __thiscall UDisBehaviorCombatMelee::FilterCombatEngageRejected(struct FAIStimStruct_CombatEngageRejected const &)
//   0x762910  protected: virtual unsigned int __thiscall UDisBehaviorCombatMelee::FilterReachabilityChange(struct FAIStimStruct_ReachabilityChange const &)
//   0x762940  protected: virtual unsigned int __thiscall UDisBehaviorCombatMelee::FilterDifficultyChanged(struct FAIStimStruct_DifficultyChanged const &)
//   0x762950  protected: virtual void __thiscall UDisBehaviorCombatMelee::OnBehaviorStart(void)
//   0x763840  protected: virtual void __thiscall UDisBehaviorCombatMelee::EnsureProperSubState(class UDisAISubState *)
