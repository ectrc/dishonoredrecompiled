// DishonoredGame/src/disaiblackboardrecords_global.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (52):
//   0x78d630  public: virtual void __thiscall FAIOccupyAccessPointTaskRecord::Clear_Derived(void)
//   0x78d640  public: virtual unsigned int __thiscall FAIOccupyAccessPointTaskRecord::CheckActorTermination(class AActor const *)
//   0x78d660  public: unsigned int __thiscall FAITaskRecord_SummonWolfHound::IsRelevant(float)const
//   0x78d690  public: virtual void __thiscall FAISeekAllyTaskRecord::Clear_Derived(void)
//   0x78d6a0  public: virtual unsigned int __thiscall FAIRecordGrenadeThrow::CheckActorTermination(class AActor const *)
//   0x78d6c0  public: virtual void __thiscall FAIRecord_CountWolfHound::Clear_Derived(void)
//   0x78d6d0  public: virtual void __thiscall FAIRecord_RegisteredHideout::Clear_Derived(void)
//   0x78d6e0  public: virtual unsigned int __thiscall FAITaskRecord_Weeping::CheckActorTermination(class AActor const *)
//   0x78d700  public: virtual void __thiscall FAIKnowledgeRecord_Hideout::Clear_Derived(void)
//   0x78d710  public: virtual unsigned int __thiscall FAIRecord_ShootingPosition::CheckActorTermination(class AActor const *)
//   0x78faa0  public: virtual void __thiscall FAISeekAllyTaskRecord::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fac0  public: virtual unsigned int __thiscall FAISeekAllyTaskRecord::Matches_Derived(struct FAIBlackboardRecord const &)const
//   0x78faf0  public: virtual void __thiscall FAIOccupyAccessPointTaskRecord::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fb10  public: virtual unsigned int __thiscall FAIOccupyAccessPointTaskRecord::Matches_Derived(struct FAIBlackboardRecord const &)const
//   0x78fb50  public: virtual unsigned int __thiscall FAISeekCombatGroupTaskRecord::Matches_Derived(struct FAIBlackboardRecord const &)const
//   0x78fb80  public: virtual void __thiscall FAIRecord_CountWolfHound::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fba0  public: virtual void __thiscall FAIRecordGrenadeThrow::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fbc0  public: virtual unsigned long __thiscall FAITaskRecord_CoordinatedAttack::GetAllocatedSize(void)
//   0x78fbd0  public: virtual void __thiscall FAIRecord_RegisteredHideout::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fc00  public: virtual void __thiscall FAITaskRecord_Weeping::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fc20  public: virtual unsigned int __thiscall FAIRecord_ShootingPosition::Matches_Derived(struct FAIBlackboardRecord const &)const
//   0x78fc50  public: virtual void __thiscall FAIRecord_ShootingPosition::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x78fd00  public: virtual void __thiscall FAIKnowledgeRecord_Hideout::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x792db0  public: virtual unsigned int __thiscall FAIRecord_RegisteredHideout::CheckActorTermination(class AActor const *)
//   0x792dd0  public: virtual class FString __thiscall FAIRecord_ShootingPosition::ToString_Derived(class UEnum *)const
//   0x797690  public: virtual class FString __thiscall FAISeekAllyTaskRecord::ToString_Derived(class UEnum *)const
//   0x797830  public: virtual class FString __thiscall FAIOccupyAccessPointTaskRecord::ToString_Derived(class UEnum *)const
//   0x797a50  public: virtual class FString __thiscall FAISeekCombatGroupTaskRecord::ToString_Derived(class UEnum *)const
//   0x797b80  public: virtual class FString __thiscall FAITaskRecord_SummonWolfHound::ToString_Derived(class UEnum *)const
//   0x797cc0  public: virtual class FString __thiscall FAIRecord_CountWolfHound::ToString_Derived(class UEnum *)const
//   0x797d90  public: virtual class FString __thiscall FAIRecordGrenadeThrow::ToString_Derived(class UEnum *)const
//   0x797f20  public: virtual class FString __thiscall FAITaskRecord_CoordinatedAttack::ToString_Derived(class UEnum *)const
//   0x798060  public: virtual class FString __thiscall FAIRecord_PanickingNPCs::ToString_Derived(class UEnum *)const
//   0x798110  public: virtual class FString __thiscall FAIRecord_RegisteredHideout::ToString_Derived(class UEnum *)const
//   0x7981d0  public: virtual class FString __thiscall FAITaskRecord_Weeping::ToString_Derived(class UEnum *)const
//   0x79af70  public: virtual void __thiscall FAISeekCombatGroupTaskRecord::Clear_Derived(void)
//   0x79afa0  public: virtual void __thiscall FAITaskRecord_CoordinatedAttack::Clear_Derived(void)
//   0x79afe0  public: virtual unsigned int __thiscall FAITaskRecord_CoordinatedAttack::CheckActorTermination(class AActor const *)
//   0x79c960  public: virtual void __thiscall FAISeekCombatGroupTaskRecord::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x79ca50  public: virtual void __thiscall FAITaskRecord_CoordinatedAttack::UpdateRecord_Derived(struct FAIBlackboardRecord const &)
//   0x79e690  public: __thiscall FAIOccupyAccessPointTaskRecord::FAIOccupyAccessPointTaskRecord(void)
//   0x79e6e0  public: __thiscall FAIOccupyAccessPointTaskRecord::FAIOccupyAccessPointTaskRecord(class ADisHideoutAccessPoint *, class UDisHideoutComponent *)
//   0x79e730  public: __thiscall FAISeekCombatGroupTaskRecord::FAISeekCombatGroupTaskRecord(void)
//   0x79e7c0  public: __thiscall FAISeekCombatGroupTaskRecord::FAISeekCombatGroupTaskRecord(class ADishonoredPawn *, class ADisRatSwarm *)
//   0x79e820  public: __thiscall FAITaskRecord_SummonWolfHound::FAITaskRecord_SummonWolfHound(void)
//   0x79e870  public: __thiscall FAIRecord_CountWolfHound::FAIRecord_CountWolfHound(void)
//   0x79e8d0  public: __thiscall FAIRecordGrenadeThrow::FAIRecordGrenadeThrow(void)
//   0x79e920  public: __thiscall FAITaskRecord_CoordinatedAttack::FAITaskRecord_CoordinatedAttack(void)
//   0x79e9e0  public: __thiscall FAIRecord_PanickingNPCs::FAIRecord_PanickingNPCs(void)
//   0x79ea30  public: __thiscall FAITaskRecord_Weeping::FAITaskRecord_Weeping(void)
//   0x79ea90  public: __thiscall FAIRecord_ShootingPosition::FAIRecord_ShootingPosition(void)
//   0x79eae0  public: __thiscall FAIRecord_ShootingPosition::FAIRecord_ShootingPosition(class ADishonoredNPCPawn *, class FVector const &)
