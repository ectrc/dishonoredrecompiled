// DishonoredGame/src/disbehaviortriggeralarm.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (56):
//   0x7451d0  public: static void __cdecl UDisBehaviorTriggerAlarm::InitializePrivateStaticClassUDisBehaviorTriggerAlarm(void)
//   0x7451f0  private: virtual unsigned int __thiscall UDisBehaviorTriggerAlarm::IsBehaviorFinished(void)const
//   0x745200  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x745220  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::FilterActivateAlarm(struct FAIStimStruct_ActivateAlarm const &)
//   0x745240  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnExitCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x745260  public: virtual void __thiscall UDisBehaviorTriggerAlarm::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x745270  private: virtual unsigned int __thiscall UDisBehaviorBattleVictory::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x7452a0  private: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorTriggerAlarm::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x745750  public: virtual class FVertexFactory const * __thiscall TFoliageRenderData<class TFoliageFactoryRenderDataPolicy<struct FDirectionalFoliageInstance>>::GetVertexFactory(void)
//   0x746910  private: virtual unsigned char const * __thiscall UDisBehaviorTriggerAlarm::BuildFilterStimMask(void)const
//   0x746990  protected: unsigned int __thiscall UDisBehaviorCombat::FilterDiscoveredCorpse(struct FAIStimStruct_DiscoveredCorpse const &)
//   0x7469c0  private: virtual unsigned char const * __thiscall UDisBehaviorTriggerAlarm::BuildEvaluateStimMask(void)const
//   0x748450  private: virtual unsigned int __thiscall UDisBehaviorTriggerAlarm::GetPathConstraints(class FVector const &, unsigned int, class TArray<class UNavMeshPathConstraint *, class FDefaultAllocator> &)const
//   0x748500  private: virtual void __thiscall UDisBehaviorTriggerAlarm::OnPostGameLoad(unsigned int, unsigned int)
//   0x74ece0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::InitBehavior(class UDishonoredAIBrain * const)
//   0x74eea0  private: virtual unsigned char const * __thiscall UDisBehaviorTriggerAlarm::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74ef10  private: void __thiscall UDisBehaviorTriggerAlarm::EndTriggerAlarm(void)
//   0x74efb0  private: void __thiscall UDisBehaviorTriggerAlarm::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x7501d0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::OnBehaviorStop(unsigned int)
//   0x7501f0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::OnBehaviorResume(void)
//   0x750230  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x750260  private: void __thiscall UDisBehaviorTriggerAlarm::SetupFromEnemyBusted(struct FAIStimStruct_EnemyBusted const &)
//   0x750310  private: void __thiscall UDisBehaviorTriggerAlarm::SetupFromDiscoveredCorpse(struct FAIStimStruct_DiscoveredCorpse const &)
//   0x7503c0  private: void __thiscall UDisBehaviorTriggerAlarm::SetupFromWitnessDeath(struct FAIStimStruct_WitnessDeath const &)
//   0x750490  private: void __thiscall UDisBehaviorTriggerAlarm::SetupFromHelp(struct FAIStimStruct_Help const &)
//   0x750540  private: void __thiscall UDisBehaviorTriggerAlarm::SetupFromForceRingAlarm(struct FAIStimStruct_ForceRingAlarm const &)
//   0x7505f0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::BeginDestroy(void)
//   0x750640  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x750650  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnExitCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x756bf0  public: static class UClass * __cdecl UDisBehaviorTriggerAlarm::GetPrivateStaticClassUDisBehaviorTriggerAlarm(wchar_t const *)
//   0x756c80  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorTriggerAlarm::GetSetupFromStimDelegate(enum EAIStimID)
//   0x759af0  public: static class UClass * __cdecl UDisBehaviorTriggerAlarm::StaticClassNoInline(void)
//   0x759b20  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnEnterCallback_TakePosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x759bb0  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnExitCallback_TakePosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x75aaf0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TriggerAlarm::GetPrivateStaticClassUDisTweaks_AIBehavior_TriggerAlarm(wchar_t const *)
//   0x75c0e0  public: static void __cdecl UDisTweaks_AIBehavior_TriggerAlarm::InitializePrivateStaticClassUDisTweaks_AIBehavior_TriggerAlarm(void)
//   0x75c670  public: static class UClass * __cdecl UDisTweaks_AIBehavior_TriggerAlarm::StaticClassNoInline(void)
//   0x75eb50  private: void __thiscall UDisBehaviorTriggerAlarm::CheckEnemyCloseness(class FVector const &)
//   0x75ec60  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::FilterAttackedByEnemy(struct FAIStimStruct_AttackedByEnemy const &)
//   0x75ec80  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x75ecf0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::TickBehavior(float)
//   0x75edc0  private: void __thiscall UDisBehaviorTriggerAlarm::ChooseDestination(unsigned int)
//   0x75ee80  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::ShouldTriggerAlarm(float, class ADishonoredPawn * const, unsigned int)const
//   0x75efb0  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::OwnerIsAlone(void)const
//   0x75f140  public: virtual void __thiscall UDisBehaviorTriggerAlarm::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x75f230  public: virtual void __thiscall UDisBehaviorTriggerAlarm::RefreshCallback_TakePosition(class UDisAISubState *, float)
//   0x75f240  public: virtual void __thiscall UDisBehaviorTriggerAlarm::RequestStateExitCallback_TakePosition(class UDishonoredNativeState *)
//   0x760650  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::EvaluateEnemyBusted(struct FAIStimStruct_EnemyBusted const &)const
//   0x760700  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::EvaluateDiscoveredCorpse(struct FAIStimStruct_DiscoveredCorpse const &)const
//   0x760790  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::EvaluateWitnessDeath(struct FAIStimStruct_WitnessDeath const &)const
//   0x760830  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::EvaluateHelp(struct FAIStimStruct_Help const &)const
//   0x760880  private: unsigned int __thiscall UDisBehaviorTriggerAlarm::EvaluateForceRingAlarm(struct FAIStimStruct_ForceRingAlarm const &)const
//   0x7608d0  public: virtual void __thiscall UDisBehaviorTriggerAlarm::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x7620d0  private: virtual void __thiscall UDisBehaviorTriggerAlarm::OnBehaviorStart(void)
//   0x762200  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorTriggerAlarm::GetFilterStimDelegate(enum EAIStimID)
//   0x763570  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorTriggerAlarm::GetEvaluateStimDelegate(enum EAIStimID)const
