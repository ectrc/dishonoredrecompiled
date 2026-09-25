// DishonoredGame/src/disaibrainprocessattention.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (88):
//   0x78d830  private: enum UDisAIBrainProcessAttention::EDisAttentionTweakCategory __thiscall UDisAIBrainProcessAttention::FindAttentionTweakCategory(enum ERelationship)const
//   0x78d890  private: virtual unsigned char const * __thiscall UDisAIBrainProcessAttention::BuildFilterStimMask(void)const
//   0x78d9b0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterSuspicionLevelChanged(struct FAIStimStruct_SuspicionLevelChanged const &)
//   0x78d9c0  AttentionLevelEquals
//   0x78fe30  struct FDisAttentionChangeReason __cdecl DisAttention::MakeAttentionChangeReason(enum EDisAttentionChangeReasonType, class AActor * const, int, int)
//   0x78fe60  float __cdecl DisAttention::FindAttentionHeight(class FVector const &, class FVector const &, class FVector const &)
//   0x78fea0  public: __thiscall FDisAttentionIncreaseInfo::FDisAttentionIncreaseInfo(class FVector const &)
//   0x78ff00  public: __thiscall FDisAttentionIncreaseInfo::FDisAttentionIncreaseInfo(struct FDisAttentionProxyInfo const &)
//   0x78ff40  private: unsigned int __thiscall UDisAIBrainProcessAttention::CanHearAuralRequest(class TScriptInterface<class IDisAttentionTargetInterface> const &)const
//   0x792e10  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_Kismet(class AActor *, enum EDisAttentionLevel, enum EDisAttentionLevel)
//   0x792f10  private: virtual void __thiscall UDisAIBrainProcessAttention::OnDifficultyChange_AIBrainProcess_Derived(void)
//   0x792f70  private: virtual void __thiscall UDisAIBrainProcessAttention::PostGameLoad_BrainProcess_Derived(void)
//   0x792fe0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterClearAttention(struct FAIStimStruct_ClearAttention const &)
//   0x793090  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterPsychicAttentionDisabled_AllTargets(struct FAIStimStruct_PsychicAttentionDisabled_AllTargets const &)
//   0x793130  private: class UDisAttentionInfo_Base * __thiscall UDisAIBrainProcessAttention::FindAttentionInfo(class IDisAttentionTargetInterface const *, int *)
//   0x7931c0  private: void __thiscall UDisAIBrainProcessAttention::DoAttentionReactions(class IDisAttentionTargetInterface *, struct FDisAttentionReaction const &)
//   0x793230  private: void __thiscall UDisAIBrainProcessAttention::ClearAllMinAttention(enum EDisMinAttentionLevelType)
//   0x7932a0  public: enum EDisAttentionLevel __thiscall UDisAIBrainProcessAttention::GetAttentionLevel(class IDisAttentionTargetInterface const *)const
//   0x7932d0  public: unsigned int __thiscall UDisAIBrainProcessAttention::ShouldKeepAlarmRinging(class IDisAttentionTargetInterface const *)const
//   0x798860  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_Messages(class IDisAttentionTargetInterface *, struct TMemStackArray<struct DisAttention::FDisAttentionChangeMessage> const &)
//   0x798970  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_UpdateLatestAttentionTag(struct FDisAttentionSlot const &)
//   0x7989d0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterTargetUnsighted(struct FAIStimStruct_TargetUnsighted const &)
//   0x798a10  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterPsychicAttentionDisabled(struct FAIStimStruct_PsychicAttentionDisabled const &)
//   0x798a70  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterClearMinAttentionForTarget(struct FAIStimStruct_ClearMinAttentionForTarget const &)
//   0x798ab0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterClearAllMinAttention(struct FAIStimStruct_ClearAllMinAttention const &)
//   0x798ad0  public: void __thiscall UDisAIBrainProcessAttention::GetAttentionProxyInfo(class IDisAttentionTargetInterface const *, struct FDisAttentionProxyInfo &)const
//   0x79b0d0  public: static void __cdecl UDisAIBrainProcessAttention::InitializePrivateStaticClassUDisAIBrainProcessAttention(void)
//   0x79caa0  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_ReactToCorpses(struct FDisAttentionSlot const &, enum UDisAttentionInfo_Base::EDisWitnessDeathResult)
//   0x79cce0  public: void __thiscall UDisAIBrainProcessAttention::GetTargetsAtAttentionLevel(enum EDisAttentionLevel, struct TMemStackArray<class IDisAttentionTargetInterface *> &, enum EDisLifeStateFilter, enum ERelationship, enum EDisAttentionProxyStatus)const
//   0x7a06a0  public: static class UClass * __cdecl UDisAIBrainProcessAttention::GetPrivateStaticClassUDisAIBrainProcessAttention(wchar_t const *)
//   0x7a0730  private: void __thiscall UDisAIBrainProcessAttention::RemoveAttentionInfoIdx(int)
//   0x7a1f50  public: static class UClass * __cdecl UDisAIBrainProcessAttention::StaticClassNoInline(void)
//   0x7a1f80  private: virtual void __thiscall UDisAIBrainProcessAttention::TermBrainProcess_Derived(void)
//   0x7a1fb0  private: virtual void __thiscall UDisAIBrainProcessAttention::OnOtherActorTerminated_AIBrainProcess_Derived(class AActor const &)
//   0x7a6460  private: void __thiscall UDisAIBrainProcessAttention::BuildAttnCategoryArray(struct FAttentionTweakCategoryInfo * const)const
//   0x7a6500  private: virtual void __thiscall UDisAIBrainProcessAttention::InitBrainProcess_Derived(void)
//   0x7a6570  public: class UDisTweaks_PawnAttention * __thiscall UDisAIBrainProcessAttention::FindAttentionTweaks(class IDisAttentionTargetInterface const * const)const
//   0x7a65e0  public: unsigned int __thiscall UDisAIBrainProcessAttention::ShouldIgnoreDeathOf(class IDisCorpseInterface const * const)const
//   0x7a6660  public: unsigned int __thiscall UDisAIBrainProcessAttention::ShouldSeeBodyAsHandled(class IDisCorpseInterface const * const)const
//   0x7a66a0  public: unsigned int __thiscall UDisAIBrainProcessAttention::ShouldSeeBodyAsHandled(class IDisAttentionTargetInterface const * const)const
//   0x7a6710  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_CheckForCarryingCorpse(class IDisAttentionTargetInterface const *)
//   0x7a67a0  private: void __thiscall UDisAIBrainProcessAttention::HandleTopAttnTarget(void)
//   0x7a6c70  private: void __thiscall UDisAIBrainProcessAttention::MarkActiveReactionContexts(struct FDisAttentionSlot &, struct TMemStackArray<struct FDisAppliedAttentionChangeInfo> const &, struct TMemStackArray<class IDisAttentionTargetInterface *> &)
//   0x7a6fe0  private: void __thiscall UDisAIBrainProcessAttention::CheckAttentionReactions(struct TMemStackArray<class IDisAttentionTargetInterface *> const &)
//   0x7a77a0  public: class UDisTweaks_SimpleAttention * __thiscall UDisAIBrainProcessAttention::FindSimpleAttentionTweaks(class IDisAttentionTargetInterface const * const)const
//   0x7a7810  public: unsigned int __thiscall UDisAIBrainProcessAttention::ShouldIgnoreDeathOf(class IDisAttentionTargetInterface const * const, unsigned int *)const
//   0x7a7880  private: class UDisAttentionInfo_Base & __thiscall UDisAIBrainProcessAttention::GetAttentionInfo(class IDisAttentionTargetInterface *)
//   0x7a7ce0  private: void __thiscall UDisAIBrainProcessAttention::UpdateSoireeAttention(void)
//   0x7a7e30  private: void __thiscall UDisAIBrainProcessAttention::BumpAttention(class IDisAttentionTargetInterface *, struct FDisAttentionIncrease const &, struct FDisAttentionChangeReason const &, struct FDisAttentionIncreaseInfo const &)
//   0x7a7ea0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterTargetSighted(struct FAIStimStruct_TargetSighted const &)
//   0x7a7ed0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterTargetTouched(struct FAIStimStruct_TargetTouched const &)
//   0x7a7f20  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterIncomingDamage(struct FAIStimStruct_IncomingDamage const &)
//   0x7a8070  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterMaxOutAttention(struct FAIStimStruct_MaxOutAttention const &)
//   0x7a8130  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterPsychicAttentionEnabled(struct FAIStimStruct_PsychicAttentionEnabled const &)
//   0x7a8190  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterSetMinAttentionForTarget(struct FAIStimStruct_SetMinAttentionForTarget const &)
//   0x7a81d0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterNoticeRequest(struct FAIStimStruct_NoticeRequest const &)
//   0x7a82a0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterEvadedMelee_Incoming(struct FAIStimStruct_EvadedMelee_Incoming const &)
//   0x7a8380  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterForceRingAlarm(struct FAIStimStruct_ForceRingAlarm const &)
//   0x7a8490  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterEscapedBeingChoked(struct FAIStimStruct_EscapedBeingChoked const &)
//   0x7a8570  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterPlayerHideoutTransition(struct FAIStimStruct_PlayerHideoutTransition const &)
//   0x7a8820  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterDialogAttentionChange(struct FAIStimStruct_DialogAttentionChange const &)
//   0x7a8900  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterDoorUsedByPlayerWhileWary(struct FAIStimStruct_DoorUsedByPlayerWhileWary const &)
//   0x7a89d0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterHeardCorpseSplat(struct FAIStimStruct_HeardCorpseSplat const &)
//   0x7a8af0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterTraveled(struct FAIStimStruct_Traveled const &)
//   0x7a8bb0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterHelpRequest(class ADishonoredNPCPawn *, class ADishonoredPawn *, struct FDisAttentionIncreaseInfo const &)
//   0x7a8c80  private: void __thiscall UDisAIBrainProcessAttention::HandleDiscoveredCorpse(class IDisCorpseInterface *, class FVector const &, unsigned int)
//   0x7a8e90  private: void __thiscall UDisAIBrainProcessAttention::HandleAttackedEnemy(class ADishonoredPawn *)
//   0x7a8f70  private: void __thiscall UDisAIBrainProcessAttention::HandleSearchRequest(class ADishonoredPawn *, enum EDisSearchRequestStimCause, enum ETransitSpeed, struct FDisAttentionIncreaseInfo const &)
//   0x7a8ff0  private: void __thiscall UDisAIBrainProcessAttention::HandleHearSomething(struct FDisAINoiseParameters const &)
//   0x7a93a0  private: void __thiscall UDisAIBrainProcessAttention::HandleHearAlarm(class IDisAttentionTargetInterface *, class ADisAlarmBell &, int)
//   0x7a9490  private: void __thiscall UDisAIBrainProcessAttention::HandleDeathByWoL(class FVector, class ADishonoredPawn *)
//   0x7a9550  public: unsigned int __thiscall UDisAIBrainProcessAttention::HandleWitnessedMagic(class IDisAttentionTargetInterface *)
//   0x7a9620  public: void __thiscall UDisAIBrainProcessAttention::TransferAttentionForTarget(class UDishonoredAIBrain *, class IDisAttentionTargetInterface * const)
//   0x7a9900  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_ReactToAlly(class IDisAttentionTargetInterface const *)
//   0x7a9a60  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_CheckForMurder(class IDisAttentionTargetInterface const *)
//   0x7a9bd0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterSearchRequest(struct FAIStimStruct_SearchRequest const &)
//   0x7a9c50  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterHelp(struct FAIStimStruct_Help const &)
//   0x7a9cb0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterHeardSomething(struct FAIStimStruct_HeardSomething const &)
//   0x7a9cd0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterAlarm(struct FAIStimStruct_Alarm const &)
//   0x7a9d00  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterDiscoveredCorpse(struct FAIStimStruct_DiscoveredCorpse const &)
//   0x7a9d40  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterWitnessDeath(struct FAIStimStruct_WitnessDeath const &)
//   0x7a9db0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterMeleeConnected_Outgoing(struct FAIStimStruct_MeleeConnected_Outgoing const &)
//   0x7a9dd0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterEvadedMelee_Outgoing(struct FAIStimStruct_EvadedMelee_Outgoing const &)
//   0x7a9df0  private: unsigned int __thiscall UDisAIBrainProcessAttention::FilterDeathByWoL(struct FAIStimStruct_DeathByWoL const &)
//   0x7a9f50  private: void __thiscall UDisAIBrainProcessAttention::TickAttention_Empathy(struct TMemStackArray<class IDisAttentionTargetInterface *> &)
//   0x7aa040  private: void __thiscall UDisAIBrainProcessAttention::TickAttention(float)
//   0x7aa490  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisAIBrainProcessAttention::GetFilterStimDelegate_BrainProcess(enum EAIStimID)
//   0x7aa770  private: virtual void __thiscall UDisAIBrainProcessAttention::TickBrainProcess_Derived(float)
