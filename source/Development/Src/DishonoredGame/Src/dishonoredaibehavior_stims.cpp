// DishonoredGame/src/dishonoredaibehavior_stims.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (377):
//   0x764e00  public: virtual float __thiscall FAIStimStruct_SearchRequest::GetDelay(class UDishonoredAIBrain *)const
//   0x764e10  public: virtual float __thiscall FAIStimStruct_Stolen::GetDelay(class UDishonoredAIBrain *)const
//   0x764e20  public: virtual float __thiscall FAIStimStruct_TetherRequest::GetDelay(class UDishonoredAIBrain *)const
//   0x764e40  public: virtual float __thiscall FAIStimStruct_WitnessMagic::GetDelay(class UDishonoredAIBrain *)const
//   0x767570  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_TouchedEnemy::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x7675a0  public: virtual void __thiscall FAIStimStruct_Help::Serialize(class FArchive &)
//   0x767650  public: virtual void __thiscall FAIStimStruct_HeardSomething::Serialize(class FArchive &)
//   0x7676a0  public: virtual void __thiscall FAIStimStruct_SearchReachedProxy::Serialize(class FArchive &)
//   0x7676e0  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_SearchRequest::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x767710  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_Alarm::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x767740  public: virtual void __thiscall FAIStimStruct_Intimidated::Serialize(class FArchive &)
//   0x767780  public: virtual void __thiscall FAIStimStruct_DestinationReached::Serialize(class FArchive &)
//   0x7677c0  public: virtual void __thiscall FAIStimStruct_RotationReached::Serialize(class FArchive &)
//   0x767800  public: virtual void __thiscall FAIStimStruct_TetherRequest::Serialize(class FArchive &)
//   0x767840  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_TetherRequest::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x767870  public: virtual float __thiscall FAIStimStruct_ReachabilityChange::GetDelay(class UDishonoredAIBrain *)const
//   0x7678c0  public: virtual void __thiscall FAIStimStruct_EnemyBusted::Serialize(class FArchive &)
//   0x767900  public: virtual void __thiscall FAIStimStruct_SearchBegin::Serialize(class FArchive &)
//   0x767940  public: virtual void __thiscall FAIStimStruct_CombatBegin::Serialize(class FArchive &)
//   0x767980  public: virtual void __thiscall FAIStimStruct_AmbushRequest::Serialize(class FArchive &)
//   0x7679e0  public: virtual void __thiscall FAIStimStruct_CombatToSearch::Serialize(class FArchive &)
//   0x767a20  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_WitnessMagic::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x767a50  public: virtual void __thiscall FAIStimStruct_TopAttnProxyUpdated::Serialize(class FArchive &)
//   0x76f2c0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_BrainInit::GetScriptStruct(void)const
//   0x76f2d0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_AttackedByEnemy::GetScriptStruct(void)const
//   0x76f2e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_BattleVictory::GetScriptStruct(void)const
//   0x76f2f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_MeleeConnected_Outgoing::GetScriptStruct(void)const
//   0x76f300  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TouchedEnemy::GetScriptStruct(void)const
//   0x76f310  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TouchedAlly::GetScriptStruct(void)const
//   0x76f320  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TouchedNeutral::GetScriptStruct(void)const
//   0x76f330  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Traveled::GetScriptStruct(void)const
//   0x76f340  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EvadedMelee_Incoming::GetScriptStruct(void)const
//   0x76f350  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EvadedMelee_Outgoing::GetScriptStruct(void)const
//   0x76f360  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_HeardSomething::GetScriptStruct(void)const
//   0x76f370  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_HeardCorpseSplat::GetScriptStruct(void)const
//   0x76f380  public: virtual void __thiscall FAIStimStruct_DialogAttentionChange::Serialize(class FArchive &)
//   0x76f3d0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_HeadTrackBegin::GetScriptStruct(void)const
//   0x76f3e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_HeadTrackEnd::GetScriptStruct(void)const
//   0x76f3f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SearchRequest::GetScriptStruct(void)const
//   0x76f400  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SetMinAttentionForTarget::GetScriptStruct(void)const
//   0x76f410  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Help::GetScriptStruct(void)const
//   0x76f420  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_Help::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x76f470  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_HelpFromRats::GetScriptStruct(void)const
//   0x76f480  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Alarm::GetScriptStruct(void)const
//   0x76f490  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_AllyBusted::GetScriptStruct(void)const
//   0x76f4a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DiscoveredCorpse::GetScriptStruct(void)const
//   0x76f4b0  public: virtual void __thiscall FAIStimStruct_WitnessDeath::Serialize(class FArchive &)
//   0x76f510  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_GoToRequest::GetScriptStruct(void)const
//   0x76f520  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_FollowRequest::GetScriptStruct(void)const
//   0x76f530  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ForceRingAlarm::GetScriptStruct(void)const
//   0x76f540  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_BehaviorAbort::GetScriptStruct(void)const
//   0x76f550  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_GuardRequest::GetScriptStruct(void)const
//   0x76f560  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ShootRequest::GetScriptStruct(void)const
//   0x76f570  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PatrolRequest::GetScriptStruct(void)const
//   0x76f580  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PatrolSearchRequest::GetScriptStruct(void)const
//   0x76f590  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_IdleRequest::GetScriptStruct(void)const
//   0x76f5a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_BlockingDialogRequest::GetScriptStruct(void)const
//   0x76f5b0  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_BlockingDialogRequest::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x76f5e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_BlockingDialogEnd::GetScriptStruct(void)const
//   0x76f5f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Distracted_Anim::GetScriptStruct(void)const
//   0x76f600  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Distracted_HeadLook::GetScriptStruct(void)const
//   0x76f610  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DocileRatIsNear::GetScriptStruct(void)const
//   0x76f620  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DoorUsedByPlayer::GetScriptStruct(void)const
//   0x76f630  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DoorUsedByPlayerWhileWary::GetScriptStruct(void)const
//   0x76f640  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EndDistracted::GetScriptStruct(void)const
//   0x76f650  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EndPossession::GetScriptStruct(void)const
//   0x76f660  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_InteractBegin::GetScriptStruct(void)const
//   0x76f670  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_InteractEnd::GetScriptStruct(void)const
//   0x76f680  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PathingSuccess::GetScriptStruct(void)const
//   0x76f690  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PathingFail::GetScriptStruct(void)const
//   0x76f6a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DestinationReached::GetScriptStruct(void)const
//   0x76f6b0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DifficultyChanged::GetScriptStruct(void)const
//   0x76f6c0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DialogAttentionChange::GetScriptStruct(void)const
//   0x76f6d0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_RotationReached::GetScriptStruct(void)const
//   0x76f6e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Stolen::GetScriptStruct(void)const
//   0x76f6f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SuspicionLevelChanged::GetScriptStruct(void)const
//   0x76f700  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TargetSighted::GetScriptStruct(void)const
//   0x76f710  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TargetTouched::GetScriptStruct(void)const
//   0x76f720  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TargetUnsighted::GetScriptStruct(void)const
//   0x76f730  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Panicked::GetScriptStruct(void)const
//   0x76f740  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Intimidated::GetScriptStruct(void)const
//   0x76f750  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SightedRatSwarm::GetScriptStruct(void)const
//   0x76f760  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_RelationshipChanged::GetScriptStruct(void)const
//   0x76f770  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ReturnTravelRequest::GetScriptStruct(void)const
//   0x76f780  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PlayerUsed::GetScriptStruct(void)const
//   0x76f790  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ProjectileLaunched::GetScriptStruct(void)const
//   0x76f7a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PsychicAttentionEnabled::GetScriptStruct(void)const
//   0x76f7b0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PsychicAttentionDisabled::GetScriptStruct(void)const
//   0x76f7c0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PsychicAttentionDisabled_AllTargets::GetScriptStruct(void)const
//   0x76f7d0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Soiree::GetScriptStruct(void)const
//   0x76f7e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SoireeRejected::GetScriptStruct(void)const
//   0x76f7f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TetherRequest::GetScriptStruct(void)const
//   0x76f800  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ReachabilityChange::GetScriptStruct(void)const
//   0x76f810  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_ReachabilityChange::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x76f840  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ShownRangedThreat::GetScriptStruct(void)const
//   0x76f850  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EnemyBusted::GetScriptStruct(void)const
//   0x76f860  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EscapedBeingChoked::GetScriptStruct(void)const
//   0x76f870  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_EscapedBeingPossessed::GetScriptStruct(void)const
//   0x76f880  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_NoticeBegin::GetScriptStruct(void)const
//   0x76f890  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_NoticeEnd::GetScriptStruct(void)const
//   0x76f8a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_NoticeRequest::GetScriptStruct(void)const
//   0x76f8b0  public: virtual void __thiscall FAIStimStruct_CarryCorpseOrBodyPart::Serialize(class FArchive &)
//   0x76f910  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_NPCReachAttentionLevel::GetScriptStruct(void)const
//   0x76f920  public: virtual void __thiscall FAIStimStruct_NPCReachAttentionLevel::Serialize(class FArchive &)
//   0x76f970  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ScrambleRequest::GetScriptStruct(void)const
//   0x76f980  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SearchBegin::GetScriptStruct(void)const
//   0x76f990  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SearchEnd::GetScriptStruct(void)const
//   0x76f9a0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_SearchReachedProxy::GetScriptStruct(void)const
//   0x76f9b0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CombatBegin::GetScriptStruct(void)const
//   0x76f9c0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CombatEnd::GetScriptStruct(void)const
//   0x76f9d0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CombatEngageRejected::GetScriptStruct(void)const
//   0x76f9e0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_AmbushRequest::GetScriptStruct(void)const
//   0x76f9f0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_AmbushAbortRequest::GetScriptStruct(void)const
//   0x76fa00  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PlagueZone::GetScriptStruct(void)const
//   0x76fa10  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ItemContext_End::GetScriptStruct(void)const
//   0x76fa20  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ItemContext_Start::GetScriptStruct(void)const
//   0x76fa30  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_LocoPushedByPlayer::GetScriptStruct(void)const
//   0x76fa40  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_MaxOutAttention::GetScriptStruct(void)const
//   0x76fa50  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ImpendingExplosion::GetScriptStruct(void)const
//   0x76fa60  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_ImpendingExplosion::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x76fa90  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_IncomingDamage::GetScriptStruct(void)const
//   0x76faa0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Explosion::GetScriptStruct(void)const
//   0x76fab0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_DeathByWoL::GetScriptStruct(void)const
//   0x76fac0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_WitnessDeath::GetScriptStruct(void)const
//   0x76fad0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_WitnessMagic::GetScriptStruct(void)const
//   0x76fae0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TopAttnProxyReplaced::GetScriptStruct(void)const
//   0x76faf0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_TopAttnProxyUpdated::GetScriptStruct(void)const
//   0x76fb00  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CombatToSearch::GetScriptStruct(void)const
//   0x76fb10  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PlayerMessingWithActor::GetScriptStruct(void)const
//   0x76fb20  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ActivateAlarm::GetScriptStruct(void)const
//   0x76fb30  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ActorTamperedWith::GetScriptStruct(void)const
//   0x76fb40  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ReactionRequest::GetScriptStruct(void)const
//   0x76fb50  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CarryCorpseOrBodyPart::GetScriptStruct(void)const
//   0x76fb60  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ClearAllMinAttention::GetScriptStruct(void)const
//   0x76fb70  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ClearAttention::GetScriptStruct(void)const
//   0x76fb80  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_ClearMinAttentionForTarget::GetScriptStruct(void)const
//   0x76fb90  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_CoordinatedAttackRequest::GetScriptStruct(void)const
//   0x76fba0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_NoMoreWolfHoundInFight::GetScriptStruct(void)const
//   0x76fbb0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_PlayerHideoutTransition::GetScriptStruct(void)const
//   0x76fbc0  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_PlayerHideoutTransition::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x76fbf0  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_WitnessPickpocket::GetScriptStruct(void)const
//   0x76fc00  public: virtual class UScriptStruct const * __thiscall FAIStimStruct_Teleported::GetScriptStruct(void)const
//   0x772fa0  public: virtual class DisDelegate<enum FAIStimStruct::EDisPendingStimFilterResult, struct FAIStimStruct> __thiscall FAIStimStruct_DiscoveredCorpse::GetPendingStimFilterDelegate(enum EAIStimID)const
//   0x7783b0  public: __thiscall FAIStimStruct_BrainInit::FAIStimStruct_BrainInit(void)
//   0x7783e0  public: __thiscall FAIStimStruct_AttackedByEnemy::FAIStimStruct_AttackedByEnemy(class ADishonoredPawn *)
//   0x778420  public: __thiscall FAIStimStruct_BattleVictory::FAIStimStruct_BattleVictory(enum FAIStimStruct::FAIStimTypeInfo::EAIStimStructEmptyConstructorEnum)
//   0x7784a0  public: __thiscall FAIStimStruct_BattleVictory::FAIStimStruct_BattleVictory(struct FDisAttentionProxy const &)
//   0x778510  public: __thiscall FAIStimStruct_MeleeConnected_Outgoing::FAIStimStruct_MeleeConnected_Outgoing(class ADishonoredPawn *)
//   0x778550  public: __thiscall FAIStimStruct_TouchedEnemy::FAIStimStruct_TouchedEnemy(class ADishonoredPawn *, unsigned int)
//   0x7785a0  public: __thiscall FAIStimStruct_TouchedAlly::FAIStimStruct_TouchedAlly(class ADishonoredPawn *)
//   0x7785e0  public: __thiscall FAIStimStruct_TouchedNeutral::FAIStimStruct_TouchedNeutral(class ADishonoredPawn *)
//   0x778620  public: __thiscall FAIStimStruct_Traveled::FAIStimStruct_Traveled(void)
//   0x778650  public: __thiscall FAIStimStruct_EvadedMelee_Incoming::FAIStimStruct_EvadedMelee_Incoming(class UDisItemContext_MeleeAttack *, enum eDisMeleeEvasionType)
//   0x7786a0  public: __thiscall FAIStimStruct_EvadedMelee_Outgoing::FAIStimStruct_EvadedMelee_Outgoing(class UDisItemContext_MeleeAttack *, class UDisItemContext_MeleeBlock *, enum eDisMeleeEvasionType)
//   0x7786f0  public: __thiscall FAIStimStruct_HeardSomething::FAIStimStruct_HeardSomething(enum FAIStimStruct::FAIStimTypeInfo::EAIStimStructEmptyConstructorEnum)
//   0x778760  public: __thiscall FAIStimStruct_HeardSomething::FAIStimStruct_HeardSomething(struct FDisAINoiseParameters const &)
//   0x7787b0  public: __thiscall FAIStimStruct_HeardCorpseSplat::FAIStimStruct_HeardCorpseSplat(class IDisCorpseInterface *)
//   0x778870  public: __thiscall FAIStimStruct_HeadTrackBegin::FAIStimStruct_HeadTrackBegin(enum FAIStimStruct::FAIStimTypeInfo::EAIStimStructEmptyConstructorEnum)
//   0x7788f0  public: __thiscall FAIStimStruct_HeadTrackBegin::FAIStimStruct_HeadTrackBegin(struct FDisAttentionProxy const &)
//   0x778960  public: __thiscall FAIStimStruct_HeadTrackEnd::FAIStimStruct_HeadTrackEnd(void)
//   0x778990  public: __thiscall FAIStimStruct_SearchRequest::FAIStimStruct_SearchRequest(enum FAIStimStruct::FAIStimTypeInfo::EAIStimStructEmptyConstructorEnum)
//   0x778a50  public: __thiscall FAIStimStruct_SearchRequest::FAIStimStruct_SearchRequest(class ADishonoredNPCPawn *, class ADishonoredPawn *, enum EDisSearchRequestStimCause, enum ETransitSpeed, struct FDisAttentionIncreaseInfo const &)
//   0x778ad0  public: __thiscall FAIStimStruct_SetMinAttentionForTarget::FAIStimStruct_SetMinAttentionForTarget(class IDisAttentionTargetInterface *, enum EDisAttentionLevel, enum EDisMinAttentionLevelType)
//   0x778ba0  public: __thiscall FAIStimStruct_Help::FAIStimStruct_Help(enum FAIStimStruct::FAIStimTypeInfo::EAIStimStructEmptyConstructorEnum)
//   0x778c20  public: __thiscall FAIStimStruct_Help::FAIStimStruct_Help(class ADishonoredNPCPawn *, class ADishonoredPawn *, int)
//   0x778d00  public: __thiscall FAIStimStruct_HelpFromRats::FAIStimStruct_HelpFromRats(class ADishonoredNPCPawn *, class ADisRatSwarm *)
//   0x778d50  public: __thiscall FAIStimStruct_Alarm::FAIStimStruct_Alarm(class IDisAttentionTargetInterface *, class ADisAlarmBell *, int)
//   0x778e20  public: __thiscall FAIStimStruct_AllyBusted::FAIStimStruct_AllyBusted(class ADishonoredPawn *)
//   0x778e60  public: __thiscall FAIStimStruct_DiscoveredCorpse::FAIStimStruct_DiscoveredCorpse(class IDisCorpseInterface *, class ADishonoredPawn *, class FVector const &)
//   0x778f30  public: __thiscall FAIStimStruct_GoToRequest::FAIStimStruct_GoToRequest(class UDisSeqAct_AIGoToActor *)
//   0x778f90  public: __thiscall FAIStimStruct_FollowRequest::FAIStimStruct_FollowRequest(class ADishonoredPawn * const, unsigned int, struct FFollowParameters * const)
//   0x779010  public: __thiscall FAIStimStruct_ForceRingAlarm::FAIStimStruct_ForceRingAlarm(class ADishonoredPawn *, unsigned int, class FVector const &)
//   0x779070  public: __thiscall FAIStimStruct_BehaviorAbort::FAIStimStruct_BehaviorAbort(class UClass *)
//   0x7790b0  public: __thiscall FAIStimStruct_GuardRequest::FAIStimStruct_GuardRequest(class AActor *, float)
//   0x779100  public: __thiscall FAIStimStruct_ShootRequest::FAIStimStruct_ShootRequest(class AActor *, unsigned int, int, unsigned int, float)
//   0x779170  public: __thiscall FAIStimStruct_PatrolRequest::FAIStimStruct_PatrolRequest(class AActor *)
//   0x7791b0  public: __thiscall FAIStimStruct_PatrolSearchRequest::FAIStimStruct_PatrolSearchRequest(class AActor *)
//   0x7791f0  public: __thiscall FAIStimStruct_IdleRequest::FAIStimStruct_IdleRequest(void)
//   0x779220  public: __thiscall FAIStimStruct_BlockingDialogRequest::FAIStimStruct_BlockingDialogRequest(int, unsigned int)
//   0x779270  public: __thiscall FAIStimStruct_BlockingDialogEnd::FAIStimStruct_BlockingDialogEnd(int)
//   0x7792b0  public: __thiscall FAIStimStruct_Distracted_Anim::FAIStimStruct_Distracted_Anim(class UDisNPCDistractionComponent *)
//   0x7792f0  public: __thiscall FAIStimStruct_Distracted_HeadLook::FAIStimStruct_Distracted_HeadLook(class UDisNPCDistractionComponent *)
//   0x779330  public: __thiscall FAIStimStruct_DocileRatIsNear::FAIStimStruct_DocileRatIsNear(class ADisGameCrowdAgentSkeletalRat *, unsigned int, float)
//   0x779390  public: __thiscall FAIStimStruct_DoorUsedByPlayer::FAIStimStruct_DoorUsedByPlayer(class ADisDoor *)
//   0x7793d0  public: __thiscall FAIStimStruct_DoorUsedByPlayerWhileWary::FAIStimStruct_DoorUsedByPlayerWhileWary(class ADisDoor *)
//   0x779410  public: __thiscall FAIStimStruct_EndDistracted::FAIStimStruct_EndDistracted(class UDisNPCDistractionComponent *)
//   0x779450  public: __thiscall FAIStimStruct_EndPossession::FAIStimStruct_EndPossession(void)
//   0x779480  public: __thiscall FAIStimStruct_InteractBegin::FAIStimStruct_InteractBegin(unsigned int, class FVector const &)
//   0x7794e0  public: __thiscall FAIStimStruct_InteractEnd::FAIStimStruct_InteractEnd(void)
//   0x779510  public: __thiscall FAIStimStruct_PathingSuccess::FAIStimStruct_PathingSuccess(class UObject *)
//   0x779550  public: __thiscall FAIStimStruct_PathingFail::FAIStimStruct_PathingFail(class UObject *)
//   0x779590  public: __thiscall FAIStimStruct_DestinationReached::FAIStimStruct_DestinationReached(class FVector const &, unsigned int, class UObject *)
//   0x7795f0  public: __thiscall FAIStimStruct_DifficultyChanged::FAIStimStruct_DifficultyChanged(void)
//   0x779620  public: __thiscall FAIStimStruct_DialogAttentionChange::FAIStimStruct_DialogAttentionChange(class IDisAttentionTargetInterface *, enum EDisAttentionLevel)
//   0x7796f0  public: __thiscall FAIStimStruct_RotationReached::FAIStimStruct_RotationReached(class FVector const &, class UObject *)
//   0x779740  public: __thiscall FAIStimStruct_Stolen::FAIStimStruct_Stolen(float)
//   0x779780  public: __thiscall FAIStimStruct_SuspicionLevelChanged::FAIStimStruct_SuspicionLevelChanged(enum EDisAISuspicionLevel)
//   0x7797c0  public: __thiscall FAIStimStruct_TargetSighted::FAIStimStruct_TargetSighted(class IDisAttentionTargetInterface * const)
//   0x779880  public: __thiscall FAIStimStruct_TargetTouched::FAIStimStruct_TargetTouched(class IDisAttentionTargetInterface * const)
//   0x779940  public: __thiscall FAIStimStruct_TargetUnsighted::FAIStimStruct_TargetUnsighted(class IDisAttentionTargetInterface * const)
//   ... 177 more, see resources/docs/symbols/functions.csv
