// DishonoredGame/src/disbehavioroverseercombat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x745ae0  public: virtual unsigned char const * __thiscall UDisBehaviorOverseerCombat::BuildFilterStimMask(void)const
//   0x745b30  private: unsigned int __thiscall UDisBehaviorOverseerCombat::FilterImpendingExplosion(struct FAIStimStruct_ImpendingExplosion const &)
//   0x7470e0  public: virtual void __thiscall UDisBehaviorOverseerCombat::OnExitCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x747120  private: virtual unsigned int __thiscall UDisBehaviorOverseerCombat::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x7492e0  private: unsigned int __thiscall UDisBehaviorOverseerCombat::IsChargeCoordinationAllowed(void)const
//   0x74d2c0  public: virtual unsigned char const * __thiscall UDisBehaviorOverseerCombat::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74d330  private: unsigned int __thiscall UDisBehaviorOverseerCombat::WillGrenadeHurtAllies(struct FDisAttentionProxy const &, class UDisWepGrenade * const, float *)const
//   0x74f9a0  private: unsigned int __thiscall UDisBehaviorOverseerCombat::IsCoordinatedChargeUnderWay(void)const
//   0x751ec0  public: virtual void __thiscall UDisBehaviorOverseerCombat::InitBehavior(class UDishonoredAIBrain * const)
//   0x751fe0  private: unsigned int __thiscall UDisBehaviorOverseerCombat::UnregisterPawnFromCharge(class ADishonoredNPCPawn *, class ADishonoredNPCPawn *)
//   0x7520c0  private: void __thiscall UDisBehaviorOverseerCombat::OnAnyPawnDeath(class FArkGameEvent const &)
//   0x757390  public: static class UClass * __cdecl UDisBehaviorOverseerCombat::GetPrivateStaticClassUDisBehaviorOverseerCombat(wchar_t const *)
//   0x757420  private: unsigned int __thiscall UDisBehaviorOverseerCombat::MessageChargeParticipants(enum eDisCoordinatedAttackStage, class ADishonoredPawn * const)
//   0x75b230  public: static void __cdecl UDisBehaviorOverseerCombat::InitializePrivateStaticClassUDisBehaviorOverseerCombat(void)
//   0x75b250  public: static class UClass * __cdecl UDisTweaks_AIBehavior_OverseerCombat::GetPrivateStaticClassUDisTweaks_AIBehavior_OverseerCombat(wchar_t const *)
//   0x75bd50  public: static class UClass * __cdecl UDisBehaviorOverseerCombat::StaticClassNoInline(void)
//   0x75c230  private: unsigned int __thiscall UDisBehaviorOverseerCombat::UnregisterFromCharge(void)
//   0x75d850  public: static void __cdecl UDisTweaks_AIBehavior_OverseerCombat::InitializePrivateStaticClassUDisTweaks_AIBehavior_OverseerCombat(void)
//   0x75d870  public: virtual void __thiscall UDisBehaviorOverseerCombat::OnBehaviorResume(void)
//   0x75ff20  public: static class UClass * __cdecl UDisTweaks_AIBehavior_OverseerCombat::StaticClassNoInline(void)
//   0x762ee0  public: virtual void __thiscall UDisBehaviorOverseerCombat::OnBehaviorStart(void)
//   0x762f40  private: void __thiscall UDisBehaviorOverseerCombat::DoJumpback(void)
//   0x762f70  private: void __thiscall UDisBehaviorOverseerCombat::StartChargeCoordination(void)
//   0x7631f0  private: void __thiscall UDisBehaviorOverseerCombat::StartMeleeRushStance(unsigned int)
//   0x7632b0  private: void __thiscall UDisBehaviorOverseerCombat::StopMeleeRushStance(void)
//   0x763370  public: virtual void __thiscall UDisBehaviorOverseerCombat::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7633c0  private: virtual unsigned int __thiscall UDisBehaviorOverseerCombat::FilterCombatEngageRejected(struct FAIStimStruct_CombatEngageRejected const &)
//   0x7633f0  private: void __thiscall UDisBehaviorOverseerCombat::OnGrenadeArmedStartCharge(class FArkGameEvent const &)
//   0x763c60  private: void __thiscall UDisBehaviorOverseerCombat::ManageMeleeRush(float)
//   0x763d20  private: void __thiscall UDisBehaviorOverseerCombat::TriggerCharge(class ADishonoredPawn * const)
//   0x763e10  private: void __thiscall UDisBehaviorOverseerCombat::CancelCharge(unsigned int)
//   0x763ed0  private: void __thiscall UDisBehaviorOverseerCombat::DoRushReady(void)
//   0x763f80  public: virtual void __thiscall UDisBehaviorOverseerCombat::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x763f90  public: virtual void __thiscall UDisBehaviorOverseerCombat::RefreshCallback_MeleeChase(class UDisAISubState *, float)
//   0x763fc0  private: unsigned int __thiscall UDisBehaviorOverseerCombat::FilterItemContext_Start(struct FAIStimStruct_ItemContext_Start const &)
//   0x764270  public: virtual void __thiscall UDisBehaviorOverseerCombat::OnBehaviorPause(unsigned int)
//   0x7642e0  private: void __thiscall UDisBehaviorOverseerCombat::ManageChargeTriggering(float)
//   0x764450  private: void __thiscall UDisBehaviorOverseerCombat::DoPrepareRush(unsigned int)
//   0x7644f0  private: unsigned int __thiscall UDisBehaviorOverseerCombat::FilterItemContext_End(struct FAIStimStruct_ItemContext_End const &)
//   0x764690  private: unsigned int __thiscall UDisBehaviorOverseerCombat::FilterCoordinatedAttackRequest(struct FAIStimStruct_CoordinatedAttackRequest const &)
//   0x7649b0  public: virtual void __thiscall UDisBehaviorOverseerCombat::TickBehavior(float)
//   0x764a90  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorOverseerCombat::GetFilterStimDelegate(enum EAIStimID)
