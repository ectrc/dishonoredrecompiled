// DishonoredGame/src/dishonoredaibrain_senses.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (77):
//   0x746190  public: struct FDisAttentionProxy const & __thiscall UDishonoredAIBrain::GetTopEnemyProxy(void)const
//   0x7461a0  public: struct FDisAttentionProxy const & __thiscall UDishonoredAIBrain::GetTopEnemyProxy(enum EDisAttentionLevel &)const
//   0x7461c0  public: void __thiscall UDishonoredAIBrain::ClearNotedMagic(class ADishonoredPawn *)
//   0x7461f0  public: unsigned int __thiscall UDishonoredAIBrain::CanLoseBustedTarget(void)const
//   0x746200  public: unsigned int __thiscall UDishonoredAIBrain::CanEmpathize(void)const
//   0x746230  public: void __thiscall UDishonoredAIBrain::GetTargetsAtAttentionLevel(enum EDisAttentionLevel, struct TMemStackArray<class IDisAttentionTargetInterface *> &, enum EDisLifeStateFilter, enum ERelationship, enum EDisAttentionProxyStatus)const
//   0x746250  public: enum EDisAttentionLevel __thiscall UDishonoredAIBrain::GetAttentionLevel(class IDisAttentionTargetInterface const *)const
//   0x746270  public: void __thiscall UDishonoredAIBrain::GetAttentionProxyInfo(class IDisAttentionTargetInterface const *, struct FDisAttentionProxyInfo &)const
//   0x746290  public: void __thiscall UDishonoredAIBrain::OverrideProtectNeutrals(unsigned int)
//   0x7462b0  public: unsigned int __thiscall UDishonoredAIBrain::IsProtectingNeutrals(void)const
//   0x7462c0  public: void __thiscall UDishonoredAIBrain::TransferAttentionForTarget(class UDishonoredAIBrain *, class IDisAttentionTargetInterface * const)
//   0x7462e0  public: unsigned int __thiscall UDishonoredAIBrain::ShouldCareAboutThisCrime(class IDisRelationshipInterface const *, class IDisRelationshipInterface const *)const
//   0x746350  public: unsigned int __thiscall UDishonoredAIBrain::ShouldAlwaysWitnessDeath(void)const
//   0x746360  public: void __thiscall UDishonoredAIBrain::HandleHearSomething(class FName const &)
//   0x7478e0  public: unsigned int __thiscall UDishonoredAIBrain::HasDumbFlag(enum EDisSenseMaskType)const
//   0x747900  public: unsigned int __thiscall UDishonoredAIBrain::HasDeafFlag(enum EDisSenseMaskType)const
//   0x747920  public: unsigned int __thiscall UDishonoredAIBrain::HasBlindFlag(enum EDisSenseMaskType)const
//   0x747940  public: unsigned int __thiscall UDishonoredAIBrain::HasNumbFlag(enum EDisSenseMaskType)const
//   0x747960  private: void __thiscall UDishonoredAIBrain::BuildFinalSenseMask(void)
//   0x7479f0  private: unsigned int __thiscall UDishonoredAIBrain::IsSenseIntercepted(struct FAIStimStruct const &)
//   0x747a60  public: __thiscall FDisArchiveCheckStimReference::FDisArchiveCheckStimReference(class AActor const &, struct FAIStimStruct const * const)
//   0x747ae0  private: virtual class FArchive & __thiscall FDisArchiveCheckStimReference::operator<<(class UObject * &)
//   0x747b00  public: void __thiscall UDishonoredAIBrain::FireKismetEvent_Touched(class AActor * const)
//   0x747b40  public: void __thiscall UDishonoredAIBrain::FireKismetEvent_PlayerHeard(class AActor * const)
//   0x747b80  public: void __thiscall UDishonoredAIBrain::OnTouchedEnemyStimAccepted(struct FAIStimStruct_TouchedEnemy const &)
//   0x747bd0  public: void __thiscall UDishonoredAIBrain::HandleWitnessedInteraction(class AActor * const, class ADishonoredPawn * const)
//   0x749dc0  class IDisCorpseInterface * __cdecl DisAttention::GetCorpseFromReason(struct FDisAttentionChangeReason const &)
//   0x749e10  public: void __thiscall UDishonoredAIBrain::SetDumbToNonPlayer(void)
//   0x749e20  public: unsigned int __thiscall UDishonoredAIBrain::CanReactToNoise(struct FDisAINoiseInfo const &)
//   0x749f80  public: void __thiscall UDishonoredAIBrain::DoOneAttentionReaction(class IDisAttentionTargetInterface *, enum EDisAttentionReactionType)
//   0x74a0d0  public: class ADishonoredPawn * __thiscall UDishonoredAIBrain::GetTopEnemy(void)const
//   0x74a100  public: class ADishonoredPawn * __thiscall UDishonoredAIBrain::GetTopBustedEnemy(void)const
//   0x74a140  public: unsigned int __thiscall UDishonoredAIBrain::ShouldMagicDrainAttn(class AActor *, unsigned int &)const
//   0x74a1d0  private: void __thiscall UDishonoredAIBrain::HandleEnemyBusted(class ADishonoredPawn *)
//   0x74a240  private: void __thiscall UDishonoredAIBrain::HandleNeutralBusted(class ADishonoredPawn *)
//   0x74a280  private: void __thiscall UDishonoredAIBrain::HandleAllyBusted(class ADishonoredPawn *, class FVector const &)
//   0x74a2c0  public: void __thiscall UDishonoredAIBrain::HandleObjectTouched(class AActor *, class FVector const &)
//   0x74a300  public: class FVector __thiscall UDishonoredAIBrain::GetPerceivedLocation(class AActor const *)const
//   0x74a410  private: void __thiscall UDishonoredAIBrain::CheckForImportantKismetEvents(void)
//   0x74a580  public: unsigned int __thiscall UDishonoredAIBrain::ShouldLowerAlertness(void)const
//   0x74d8c0  public: void __thiscall UDishonoredAIBrain::SetDumbFlag(enum EDisSenseMaskType, unsigned int)
//   0x74d8f0  public: void __thiscall UDishonoredAIBrain::SetDeafFlag(enum EDisSenseMaskType, unsigned int)
//   0x74d920  public: void __thiscall UDishonoredAIBrain::SetBlindFlag(enum EDisSenseMaskType, unsigned int)
//   0x74d950  public: void __thiscall UDishonoredAIBrain::SetNumbFlag(enum EDisSenseMaskType, unsigned int)
//   0x74d980  public: float __thiscall UDishonoredAIBrain::CalcPlayerAttnScale(void)const
//   0x74db60  public: void __thiscall UDishonoredAIBrain::InhibitRatSwarmTracking(float)
//   0x74dba0  public: class IDisAttentionTargetInterface * __thiscall UDishonoredAIBrain::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x752750  public: void __thiscall UDishonoredAIBrain::GetPawnsAtAttentionLevel(enum EDisAttentionLevel, struct TMemStackArray<class ADishonoredPawn *> &, enum EDisLifeStateFilter, enum ERelationship, enum EDisAttentionProxyStatus)const
//   0x752890  public: unsigned int __thiscall UDishonoredAIBrain::CanSeeAnyBustedEnemy(void)const
//   0x7562d0  private: void __thiscall UDishonoredAIBrain::TickBrain_Senses(float)
//   0x757c30  private: void __thiscall UDishonoredAIBrain::OnAttentionLevelChanged_Stims(class AActor *, enum EDisAttentionLevel, enum EDisAttentionLevel, struct FDisAttentionChangeReason const &)
//   0x757d30  private: void __thiscall UDishonoredAIBrain::OnDecreaseTopAttnTargetAttentionLevel(unsigned int)
//   0x757eb0  private: void __thiscall UDishonoredAIBrain::OnIncreaseTopAttnTargetAttentionLevel(struct FDisAttentionChangeReason const &)
//   0x758070  public: void __thiscall UDishonoredAIBrain::HandleTargetSighted(class IDisAttentionTargetInterface *)
//   0x758100  public: void __thiscall UDishonoredAIBrain::HandleTargetUnsighted(class IDisAttentionTargetInterface *)
//   0x758190  private: void __thiscall UDishonoredAIBrain::HandleTargetUnbusted(class AActor *)
//   0x758290  private: void __thiscall UDishonoredAIBrain::HandlePawnBusted_SendStims(class ADishonoredPawn *, struct FDisAttentionChangeReason const &)
//   0x7583e0  public: void __thiscall UDishonoredAIBrain::HandleRatSwarmSighted(class ADisRatSwarm * const)
//   0x758470  public: void __thiscall UDishonoredAIBrain::HandlePawnTouched(class ADishonoredPawn *)
//   0x758500  public: void __thiscall UDishonoredAIBrain::HandleUsedByOther(class ADishonoredPawn * const, enum eCrossHairStatus)
//   0x758640  public: void __thiscall UDishonoredAIBrain::HandleIncomingDamage(class AActor *, class UClass *, int, class AActor *)
//   0x7588e0  private: void __thiscall UDishonoredAIBrain::RefreshRelationshipStatus(void)
//   0x758970  public: void __thiscall UDishonoredAIBrain::ClearAttention(class IDisAttentionTargetInterface const *, float)
//   0x758a00  public: void __thiscall UDishonoredAIBrain::MaxOutAttention(class IDisAttentionTargetInterface *)
//   0x758a90  public: void __thiscall UDishonoredAIBrain::EnablePsychicAttention(class IDisAttentionTargetInterface *, enum EDisPsychicAttentionType)
//   0x758b20  public: void __thiscall UDishonoredAIBrain::DisablePsychicAttention(class IDisAttentionTargetInterface const *, enum EDisPsychicAttentionType)
//   0x758bb0  public: void __thiscall UDishonoredAIBrain::DisableAllPsychicAttention(enum EDisPsychicAttentionType)
//   0x758c40  public: unsigned int __thiscall UDishonoredAIBrain::HandleWitnessedMagic(class UClass * const, class ADishonoredPawn * const, enum eDisMagicWitnessingMode)
//   0x758de0  public: void __thiscall UDishonoredAIBrain::SetMinAttentionForTarget(class IDisAttentionTargetInterface *, enum EDisAttentionLevel, enum EDisMinAttentionLevelType)
//   0x758e70  public: void __thiscall UDishonoredAIBrain::ClearMinAttentionForTarget(class IDisAttentionTargetInterface *, enum EDisMinAttentionLevelType)
//   0x758f00  public: void __thiscall UDishonoredAIBrain::ClearAllMinAttention(enum EDisMinAttentionLevelType)
//   0x75a200  public: void __thiscall UDishonoredAIBrain::SetTopEnemyAttnTarget(struct FDisAttentionProxy const &, enum EDisAttentionLevel)
//   0x75a420  private: void __thiscall UDishonoredAIBrain::ChangeTopAttnTargetAttentionLevel(enum EDisAttentionLevel, unsigned int, struct FDisAttentionChangeReason const &, class UDisTweaks_PawnAttention const * const)
//   0x75a580  private: void __thiscall UDishonoredAIBrain::HandleTargetBusted(class AActor *, class FVector const &, struct FDisAttentionChangeReason const &)
//   0x75ba20  public: void __thiscall UDishonoredAIBrain::SetTopAttnTarget(struct FDisAttentionProxy const &, enum ERelationship, unsigned int, enum EDisAttentionLevel, struct FDisAttentionChangeReason const &, class UDisTweaks_PawnAttention const * const)
//   0x75bf80  private: void __thiscall UDishonoredAIBrain::OnOtherActorTerminated_AIBrain_Senses(class AActor const &)
//   0x75c540  public: void __thiscall UDishonoredAIBrain::SendAttentionLevelChanged(class AActor *, enum EDisAttentionLevel, enum EDisAttentionLevel, struct FDisAttentionChangeReason const &, class FVector const &)
