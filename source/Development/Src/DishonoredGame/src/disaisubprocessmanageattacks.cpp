// DishonoredGame/src/disaisubprocessmanageattacks.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x765f70  void __cdecl SubtractRadiusFromAttackLine(class FVector &, class FVector &, float &, unsigned int)
//   0x7660a0  public: void __thiscall UDisAISubProcessManageAttacks::SetEnemyProxy(struct FDisAttentionProxy const &)
//   0x7660d0  public: unsigned int __thiscall UDisAISubProcessManageAttacks::HasPrimaryLineOfAttack(void)const
//   0x7660e0  public: unsigned int __thiscall UDisAISubProcessManageAttacks::HasSecondaryLineOfAttack(void)const
//   0x7660f0  public: unsigned int __thiscall UDisAISubProcessManageAttacks::IsObstructedByGeometry(void)const
//   0x766100  private: virtual unsigned char const * __thiscall UDisAISubProcessManageAttacks::BuildFilterStimMask(void)const
//   0x766140  private: void __thiscall UDisAISubProcessManageAttacks::ResetBackStep(class UDisTweaks_AISubProcess_ManageAttacks const *)
//   0x766170  private: void __thiscall UDisAISubProcessManageAttacks::ResetSideStep(class UDisTweaks_AISubProcess_ManageAttacks const *)
//   0x766190  private: unsigned int __thiscall UDisAISubProcessManageAttacks::WantsToBackStep(class UDisTweaks_AISubProcess_ManageAttacks const *)const
//   0x7661c0  private: unsigned int __thiscall UDisAISubProcessManageAttacks::WantsToSideStep(class UDisTweaks_AISubProcess_ManageAttacks const *)const
//   0x766200  private: unsigned int __thiscall UDisAISubProcessManageAttacks::IdleForTooLong(void)const
//   0x769b90  private: unsigned int __thiscall UDisAISubProcessManageAttacks::IsAllyOrNeutralInTheWay(class FVector, class FVector, class FVector)const
//   0x769c70  private: unsigned int __thiscall UDisAISubProcessManageAttacks::IsEnemyAimingAtMe(class UDisTweaks_AISubProcess_ManageAttacks const *)const
//   0x7716f0  private: unsigned int __thiscall UDisAISubProcessManageAttacks::IsLineOfAttackClear(unsigned int, class FVector, class FVector, struct FDisManageAttacksUsageInfo const &, unsigned int &, unsigned int &)const
//   0x774b20  public: static class UClass * __cdecl UDisAISubProcessManageAttacks::GetPrivateStaticClassUDisAISubProcessManageAttacks(wchar_t const *)
//   0x776630  public: static void __cdecl UDisAISubProcessManageAttacks::InitializePrivateStaticClassUDisAISubProcessManageAttacks(void)
//   0x778220  public: static class UClass * __cdecl UDisAISubProcessManageAttacks::StaticClassNoInline(void)
//   0x784290  public: static class UClass * __cdecl UDisTweaks_AISubProcess_ManageAttacks::GetPrivateStaticClassUDisTweaks_AISubProcess_ManageAttacks(wchar_t const *)
//   0x784900  public: static void __cdecl UDisTweaks_AISubProcess_ManageAttacks::InitializePrivateStaticClassUDisTweaks_AISubProcess_ManageAttacks(void)
//   0x785270  public: static class UClass * __cdecl UDisTweaks_AISubProcess_ManageAttacks::StaticClassNoInline(void)
//   0x789cd0  public: class FVector __thiscall UDisAISubProcessManageAttacks::GetShootExtent(void)const
//   0x789d20  public: float __thiscall UDisAISubProcessManageAttacks::GetShootHeight(void)const
//   0x789da0  private: void __thiscall UDisAISubProcessManageAttacks::RefreshAttackStatuses(float)
//   0x789e70  public: unsigned int __thiscall UDisAISubProcessManageAttacks::ConfirmAllyAndNeutralSafety(unsigned int)const
//   0x789f80  public: void __thiscall UDisAISubProcessManageAttacks::RefreshLinesOfAttack(void)
//   0x78a310  private: unsigned int __thiscall UDisAISubProcessManageAttacks::FilterEvadedMelee_Outgoing(struct FAIStimStruct_EvadedMelee_Outgoing const &)
//   0x78a3d0  private: float __thiscall UDisAISubProcessManageAttacks::CalculateCooldownMultiplier(void)const
//   0x78a410  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::FindOrExecuteItemContext(class UClass *, unsigned int)const
//   0x78a5e0  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::CheckForDanger(float, class UDisTweaks_AISubProcess_ManageAttacks const *)
//   0x78a740  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::CheckForBackStep(float, class UDisTweaks_AISubProcess_ManageAttacks const *)
//   0x78a860  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::CheckForSideStep(float, class UDisTweaks_AISubProcess_ManageAttacks const *)
//   0x78bb10  private: virtual void __thiscall UDisAISubProcessManageAttacks::BeginSubProcess_Derived(void)
//   0x78bc80  private: virtual void __thiscall UDisAISubProcessManageAttacks::TickSubProcess_Derived(float)
//   0x78bce0  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::AttemptBlockBreaker(void)const
//   0x78bd20  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::AttemptPrimaryUsage(void)const
//   0x78bdb0  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::AttemptSecondaryUsage(void)const
//   0x78be60  private: class UDisItemContext * __thiscall UDisAISubProcessManageAttacks::AttemptShortRangeAttack(void)const
//   0x78bfd0  private: unsigned int __thiscall UDisAISubProcessManageAttacks::FilterEvadedMelee_Incoming(struct FAIStimStruct_EvadedMelee_Incoming const &)
//   0x78c020  private: unsigned int __thiscall UDisAISubProcessManageAttacks::FilterAttackedByEnemy(struct FAIStimStruct_AttackedByEnemy const &)
//   0x78c0d0  private: void __thiscall UDisAISubProcessManageAttacks::RefreshCombatRange(void)
//   0x78ccf0  private: virtual void __thiscall UDisAISubProcessManageAttacks::RefreshSubProcess_Derived(float)
//   0x78d0a0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAISubProcessManageAttacks::GetFilterStimDelegate_SubProcess(enum EAIStimID)
