// DishonoredGame/src/disbehaviorcombat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x7457a0  public: static void __cdecl UDisBehaviorCombat::InitializePrivateStaticClassUDisBehaviorCombat(void)
//   0x7457c0  public: virtual void __thiscall UDisBehaviorCombat::TickBehavior(float)
//   0x7457e0  public: virtual unsigned char const * __thiscall UDisBehaviorCombat::BuildFilterStimMask(void)const
//   0x745870  protected: unsigned int __thiscall UDisBehaviorCombat::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x745890  protected: unsigned int __thiscall UDisBehaviorCombat::FilterWitnessMagic(struct FAIStimStruct_WitnessMagic const &)
//   0x7458b0  public: virtual unsigned char const * __thiscall UDisBehaviorCombat::BuildEvaluateStimMask(void)const
//   0x7458d0  public: virtual unsigned char const * __thiscall UDisBehaviorCombat::BuildShouldFinishWhileDormantStimMask(void)const
//   0x745900  protected: unsigned int __thiscall UDisBehaviorCombat::IsPawnReachable(class ADishonoredPawn * const)const
//   0x745920  public: virtual unsigned int __thiscall UDisBehaviorCombat::IsAttacking(class ADishonoredPawn const &)const
//   0x745960  public: unsigned int __thiscall UDisBehaviorCombat::ReceptiveToNewAttackPattern(void)const
//   0x745980  public: unsigned int __thiscall UDisBehaviorCombat::IsUsingOuterSkirmishAttackPattern(void)const
//   0x745990  public: void __thiscall UDisBehaviorCombat::SetCurrentAttackPattern(class UDisTweaks_AIAttackPattern *, unsigned int)
//   0x7459c0  public: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorCombat::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x746e30  public: virtual void __thiscall UDisBehaviorCombat::RefreshThoughts(float)
//   0x746ed0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorCombat::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x746f00  protected: virtual unsigned int __thiscall UDisBehaviorCombat::IsEnemyViable(class ADishonoredPawn * const, unsigned int, unsigned int)const
//   0x746fc0  private: virtual unsigned int __thiscall UDisBehaviorAmbush::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x74ca10  public: virtual unsigned char const * __thiscall UDisBehaviorCombat::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74ca80  public: unsigned int __thiscall UDisBehaviorCombat::PresenceIsHonorablyConveyed(void)const
//   0x74f940  protected: unsigned int __thiscall UDisBehaviorCombat::FilterShownRangedThreat(struct FAIStimStruct_ShownRangedThreat const &)
//   0x751790  public: virtual void __thiscall UDisBehaviorCombat::InitBehavior(class UDishonoredAIBrain * const)
//   0x7517c0  public: virtual void __thiscall UDisBehaviorCombat::OnBehaviorStart(void)
//   0x751870  protected: void __thiscall UDisBehaviorCombat::SetupEnemy(struct FDisAttentionProxy const &)
//   0x755a70  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorCombat::GetFilterStimDelegate(enum EAIStimID)
//   0x756fa0  public: static class UClass * __cdecl UDisBehaviorCombat::GetPrivateStaticClassUDisBehaviorCombat(wchar_t const *)
//   0x759d30  public: static class UClass * __cdecl UDisBehaviorCombat::StaticClassNoInline(void)
//   0x75c6d0  public: virtual void __thiscall UDisBehaviorCombat::OnBehaviorPause(unsigned int)
//   0x75d3f0  public: virtual void __thiscall UDisBehaviorCombat::OnBehaviorStop(unsigned int)
//   0x75d550  public: virtual void __thiscall UDisBehaviorCombat::OnBehaviorResume(void)
//   0x75d6b0  private: void __thiscall UDisBehaviorCombat::SetupFromCombatBegin(struct FAIStimStruct_CombatBegin const &)
//   0x7610e0  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorCombat::GetSetupFromStimDelegate(enum EAIStimID)
