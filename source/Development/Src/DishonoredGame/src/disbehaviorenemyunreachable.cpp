// DishonoredGame/src/disbehaviorenemyunreachable.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (39):
//   0x722b70  public: static void __cdecl UDisTweaks_AIBehavior_EnemyUnreachable::InitializePrivateStaticClassUDisTweaks_AIBehavior_EnemyUnreachable(void)
//   0x722b90  public: virtual enum EModuleType __thiscall UParticleModuleRequired::GetModuleType(void)const
//   0x722ba0  public: virtual unsigned char const * __thiscall UDisBehaviorEnemyUnreachable::BuildEvaluateStimMask(void)const
//   0x722be0  public: virtual unsigned char const * __thiscall UDisBehaviorEnemyUnreachable::BuildFilterStimMask(void)const
//   0x722c50  public: unsigned int __thiscall UDisBehaviorEnemyUnreachable::EvaluateReachabilityChange(struct FAIStimStruct_ReachabilityChange const &)const
//   0x722c70  public: void __thiscall UDisBehaviorEnemyUnreachable::SetupFromReachabilityChange(struct FAIStimStruct_ReachabilityChange const &)
//   0x722c90  public: unsigned int __thiscall UDisBehaviorEnemyUnreachable::FilterReachabilityChange(struct FAIStimStruct_ReachabilityChange const &)
//   0x724a60  private: virtual unsigned int __thiscall UDisBehaviorShoot::IsBehaviorFinished(void)const
//   0x727f80  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::RefreshThoughts(float)
//   0x727fd0  public: virtual unsigned char const * __thiscall UDisBehaviorEnemyUnreachable::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728040  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::OnExitCallback_Menace(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7280c0  private: virtual unsigned int __thiscall UDisBehaviorCombatRatSwarm::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x72b4c0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorEnemyUnreachable::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x72b4f0  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorEnemyUnreachable::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72b520  public: class ADisHideoutAccessPoint const * __thiscall UDisBehaviorEnemyUnreachable::GetCurrentAccessPoint(void)const
//   0x72dcd0  public: unsigned int __thiscall UDisBehaviorEnemyUnreachable::FilterCombatEnd(struct FAIStimStruct_CombatEnd const &)
//   0x72de20  public: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorEnemyUnreachable::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x730220  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::OnBehaviorStart(void)
//   0x730260  public: void __thiscall UDisBehaviorEnemyUnreachable::ReleaseCurrentAccessPoint(void)const
//   0x7321c0  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::InitBehavior(class UDishonoredAIBrain * const)
//   0x732370  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::OnBehaviorStop(unsigned int)
//   0x732390  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorEnemyUnreachable::GetFilterStimDelegate(enum EAIStimID)
//   0x736130  public: static class UClass * __cdecl UDisBehaviorEnemyUnreachable::GetPrivateStaticClassUDisBehaviorEnemyUnreachable(wchar_t const *)
//   0x738960  public: static void __cdecl UDisBehaviorEnemyUnreachable::InitializePrivateStaticClassUDisBehaviorEnemyUnreachable(void)
//   0x739b40  public: static class UClass * __cdecl UDisBehaviorEnemyUnreachable::StaticClassNoInline(void)
//   0x739b70  public: static class UClass * __cdecl UDisTweaks_AIBehavior_EnemyUnreachable::GetPrivateStaticClassUDisTweaks_AIBehavior_EnemyUnreachable(wchar_t const *)
//   0x73ac50  public: static class UClass * __cdecl UDisTweaks_AIBehavior_EnemyUnreachable::StaticClassNoInline(void)
//   0x73ca90  private: unsigned int __thiscall UDisBehaviorEnemyUnreachable::MayInteractWithHideout(class UDisHideoutComponent const *)const
//   0x73cae0  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::OnEnterCallback_Menace(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7407e0  public: class ADisHideoutAccessPoint * __thiscall UDisBehaviorEnemyUnreachable::FindAvailableEntrance(class UDisHideoutComponent const &)
//   0x740990  public: void __thiscall UDisBehaviorEnemyUnreachable::DoGetToEntrance(class ADisHideoutAccessPoint const *)
//   0x740a10  public: unsigned int __thiscall UDisBehaviorEnemyUnreachable::TryBackOut(void)
//   0x740b30  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::RefreshCallback_Menace(class UDisAISubState *, float)
//   0x740b60  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x7427d0  public: void __thiscall UDisBehaviorEnemyUnreachable::DoSelectEntrance(void)
//   0x742850  public: void __thiscall UDisBehaviorEnemyUnreachable::DoAttackAtEntrance(void)
//   0x7428f0  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::RequestStateExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x7429e0  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::RequestStateExitCallback_DoWeaponManoeuver(class UDishonoredNativeState *)
//   0x7434b0  public: virtual void __thiscall UDisBehaviorEnemyUnreachable::OnBehaviorResume(void)
