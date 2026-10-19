// DishonoredGame/src/dishonorednpcpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (162):
//   0x7ab1d0  public: static void __cdecl UDisDamageType_Impact::InitializePrivateStaticClassUDisDamageType_Impact(void)
//   0x7ab1f0  void __cdecl EnableParticleEffect(class UParticleSystemComponent *, class UParticleSystem *, class FName const &, class USkeletalMeshComponent *)
//   0x7ab250  protected: virtual void __thiscall ADishonoredNPCPawn::PreBeginPlay(void)
//   0x7ab270  public: unsigned int __thiscall ADishonoredNPCPawn::IsBeingCarried(void)const
//   0x7ab2a0  public: unsigned int __thiscall ADishonoredNPCPawn::ShouldUseSimplifiedPhysWalk(void)const
//   0x7ab2b0  public: virtual void __thiscall ADishonoredPlayerPawn::DisplayDebug_Native(class AHUD *, float &, float &)
//   0x7ab2c0  public: virtual void __thiscall ADishonoredNPCPawn::OnAdjustedBendTime(unsigned int, unsigned int)
//   0x7ab2f0  public: virtual void __thiscall ADishonoredNPCPawn::OnNPCMarkForVanish(class UDisSeqAct_NPCMarkForVanish *)
//   0x7ab330  public: void __thiscall ADishonoredNPCPawn::SetTeleportSpellActionStatus(enum ENPCDoTeleportSpellOutputEnum)
//   0x7ab360  public: float __thiscall ADishonoredNPCPawn::GetLastTeleportTime(void)const
//   0x7ab3a0  private: virtual void __thiscall ADishonoredNPCPawn::SetupPathfindingParams(struct FNavMeshPathParams &)const
//   0x7ab470  public: virtual void __thiscall ADishonoredNPCPawn::NativePostRenderFor(class APlayerController *, class UCanvas *, class FVector, class FVector)
//   0x7ab4a0  protected: void __thiscall ADishonoredNPCPawn::StealableInteract(class ADishonoredPlayerPawn *)
//   0x7ab500  public: virtual int __thiscall ADishonoredNPCPawn::InteractDTraceFlag(void)const
//   0x7ab520  public: virtual void __thiscall ADishonoredNPCPawn::PostProcessPhysics(float, class FVector const &)
//   0x7ab570  public: unsigned int __thiscall ADishonoredNPCPawn::ShouldAIBeNotifiedOfRelationshipChange(void)const
//   0x7ab580  public: void __thiscall ADishonoredNPCPawn::AcknowledgeRelationshipChangeHandledByAI(void)
//   0x7ab590  int __cdecl GetGameSpecificLocomotionModifier(enum KeyLocomotionModifier)
//   0x7ab5b0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_BlendStop(void)
//   0x7ab5c0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SetForceInventory(unsigned int)
//   0x7ab5e0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_Attach(class AActor * const, struct FAttachmentInfos const &)
//   0x7ab690  public: virtual void __thiscall ADishonoredNPCPawn::MAT_Detach(void)
//   0x7ab6e0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SetMeshTranslationMode(enum EMatMeshTranslationMode)
//   0x7ab710  public: virtual unsigned int __thiscall ADishonoredNPCPawn::MAT_PermitLODPause(void)const
//   0x7ab720  public: virtual void __thiscall ADishonoredNPCPawn::MAT_DisableMeshTranslation(unsigned int)
//   0x7ab750  public: void __thiscall ADishonoredNPCPawn::OverrideDisableHitReactSoiree(unsigned int, unsigned int, unsigned int)
//   0x7ab7a0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SetCollision(unsigned int, unsigned int)
//   0x7ab8a0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::MAT_IsCollisionDisable(void)const
//   0x7ab8b0  public: unsigned int __thiscall ADishonoredNPCPawn::IsAwareOfDistraction(void)const
//   0x7ab8c0  public: class FArkComponentLocomotion * __thiscall ADishonoredNPCPawn::GetComponentLocomotion(void)const
//   0x7ab8d0  public: class FArkComponentFaceTo * __thiscall ADishonoredNPCPawn::GetComponentFaceTo(void)const
//   0x7ab8e0  public: class FArkComponentLookat * __thiscall ADishonoredNPCPawn::GetComponentLookat(void)const
//   0x7ab8f0  public: class FDisComponentAnimPlayer * __thiscall ADishonoredNPCPawn::GetComponentAnimPlayer(void)const
//   0x7ab900  public: class FDisComponentLODManager * __thiscall ADishonoredNPCPawn::GetComponentLODManager(void)const
//   0x7ab910  private: struct FRBCollisionChannelContainer __thiscall ADishonoredNPCPawn::GetRagdollCollidesWithChannels(void)const
//   0x7ab960  public: virtual void __thiscall ADishonoredNPCPawn::OnTriggerNotify(struct FTriggerNotifyInfo const *, int)
//   0x7ab9a0  public: static void __cdecl UDisSeqAct_SpawnStealable::InitializePrivateStaticClassUDisSeqAct_SpawnStealable(void)
//   0x7ab9c0  public: unsigned int __thiscall ADishonoredNPCPawn::AreReactionsDisabled(void)const
//   0x7ad1c0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAsleep(void)const
//   0x7ad200  public: virtual void __thiscall ADishonoredNPCPawn::PostInitAnimTree(class USkeletalMeshComponent *)
//   0x7ad240  public: virtual void __thiscall ADishonoredNPCPawn::physWalking(float, int)
//   0x7ad2b0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanInteract(struct FCanInteractParams const &)const
//   0x7ad2d0  public: virtual void __thiscall ADishonoredNPCPawn::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x7ad3a0  protected: virtual void __thiscall ADishonoredNPCPawn::UpdatePushBody(void)
//   0x7ad4c0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::HasSoul(int)const
//   0x7ad510  public: virtual unsigned int __thiscall ADishonoredNPCPawn::WantUsableObjectNPCAnims(void)const
//   0x7ad570  public: virtual int __thiscall ADishonoredNPCPawn::MAT_PausedAIGroup(class UInterpGroupInst const *)
//   0x7ad5c0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_ResumedAIGroup(class UInterpGroupInst const *, int)
//   0x7ad5e0  public: virtual void __thiscall ADishonoredNPCPawn::MAT_BlendOut(class UInterpGroupInst *)
//   0x7ad6b0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::MAT_IsUnequipped(void)const
//   0x7ad720  public: virtual unsigned int __thiscall ADishonoredNPCPawn::MAT_ShouldForceInventory(void)const
//   0x7ad740  public: virtual void __thiscall ADishonoredNPCPawn::MAT_GetBodyIntention(unsigned char &, class UClass * &, class UClass * &)const
//   0x7ad790  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SendAimRequest(class UInterpTrackAIControlNPCAimKeyProperties const *, class AActor *)
//   0x7ad810  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SendAimStopRequest(void)
//   0x7ad840  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SendFireRequest(class UInterpTrackAIControlNPCFireKeyProperties const *, class AActor *)
//   0x7ad8d0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::RequestNPCMasterAimState(void)
//   0x7ad920  public: virtual unsigned int __thiscall ADishonoredNPCPawn::RequestNPCMasterAimGrenadeState(class UDisItemContext_ProjectileAttack * const)
//   0x7ad950  public: virtual int __thiscall ADishonoredNPCPawn::GetMeshCount(void)const
//   0x7ad960  public: virtual class AActor * __thiscall ADishonoredNPCPawn::GetHighlightActor(void)const
//   0x7ad980  public: virtual unsigned int __thiscall ADishonoredNPCPawn::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x7ada50  public: void __thiscall ADishonoredNPCPawn::SetIgnoreRigidBodyDamages(unsigned int)
//   0x7b09d0  public: static class UClass * __cdecl UDisDamageType_Impact::GetPrivateStaticClassUDisDamageType_Impact(wchar_t const *)
//   0x7b0a60  protected: virtual void __thiscall ADishonoredNPCPawn::Spawned(void)
//   0x7b0c10  protected: virtual void __thiscall ADishonoredNPCPawn::PreBeginPlay_NativeComponents(void)
//   0x7b0d10  public: virtual void __thiscall ADishonoredNPCPawn::AnimState_SyncEnd(enum eAnimState_Picker)
//   0x7b0d40  public: void __thiscall ADishonoredNPCPawn::SeverLimb_Deferred(struct FDisSeveredLimbRequest const &)
//   0x7b0db0  public: void __thiscall ADishonoredNPCPawn::AddImpulse_Deferred(struct FDisRagdollImpulseDeferred const &)
//   0x7b0dc0  public: void __thiscall ADishonoredNPCPawn::ExplodeLimbs(class ADishonoredPawn *, enum eDisSeverLimbImpulse, float)
//   0x7b0f00  public: unsigned int __thiscall ADishonoredNPCPawn::IsBoneCutFromBody(class FName)const
//   0x7b0fc0  public: virtual void __thiscall ADishonoredNPCPawn::PostGameLoad(enum ESaveLoadLocation)
//   0x7b1110  public: virtual class FVector __thiscall ADishonoredNPCPawn::GetPawnViewLocation(void)
//   0x7b1180  public: unsigned int __thiscall ADishonoredNPCPawn::CanBeCarriedBy(class ADishonoredPawn const * const)const
//   0x7b1320  public: virtual unsigned int __thiscall ADishonoredNPCPawn::EndInteract_Derived(struct FEndInteractParams const &)
//   0x7b13e0  public: virtual class FString const & __thiscall ADishonoredNPCPawn::GetUseMessage(void)const
//   0x7b1430  public: virtual class FString const & __thiscall ADishonoredNPCPawn::GetCannotUseMessage(void)const
//   0x7b1480  public: virtual class UDisTweaks_InteractableInterface const * __thiscall ADishonoredNPCPawn::GetInteractableTweaks_Derived(void)const
//   0x7b14c0  public: virtual void __thiscall ADishonoredNPCPawn::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x7b1850  public: virtual void __thiscall ADishonoredNPCPawn::DisableSoulRendering(void)
//   0x7b19a0  public: void __thiscall ADishonoredNPCPawn::MatineeControllerAdd(class USeqAct_Interp * const, enum ESoireeAIPriority, unsigned int)
//   0x7b1a60  public: int __thiscall ADishonoredNPCPawn::GetMatineeControllerCount(enum ESoireeAIPriority, unsigned int)const
//   0x7b1af0  public: virtual class FName const __thiscall ADishonoredNPCPawn::GetSpineBenderNodeName(void)const
//   0x7b1b50  public: virtual class UObject * __thiscall ADishonoredNPCPawn::GetStateFiringObject(class UAnimTree *)const
//   0x7b1b80  public: unsigned int __thiscall ADishonoredNPCPawn::IsNPCInBodyAction(void)const
//   0x7b1bb0  public: virtual void __thiscall ADishonoredNPCPawn::OnMeleeStateChanged(class UDisItemContext_MeleeAttack *, enum eDisWeaponMeleeState)
//   0x7b1c90  public: virtual class USkeletalMeshComponent * __thiscall ADishonoredNPCPawn::GetMeshByIndex(int)const
//   0x7b1cf0  public: virtual void __thiscall ADishonoredNPCPawn::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x7b1de0  public: virtual void __thiscall ADishonoredNPCPawn::UnTouch(class AActor *)
//   0x7b1e30  public: void __thiscall ADishonoredNPCPawn::SetupWalkUpperBodyAnim(unsigned char, int, int)
//   0x7b1ec0  public: void __thiscall ADishonoredNPCPawn::ClearWalkUpperBodyAnim(void)
//   0x7b1f00  public: virtual class FString const & __thiscall ADishonoredNPCPawn::GetCrosshairFocusText(void)const
//   0x7b1f60  public: void __thiscall ADishonoredNPCPawn::SyncPhysicsSensor(void)
//   0x7b2190  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsOverlapping(class AActor *, struct FCheckResult *, class UPrimitiveComponent *, class UPrimitiveComponent *)
//   0x7b21e0  public: virtual void __thiscall ADishonoredNPCPawn::SyncActorToRBPhysics(void)
//   0x7b6260  public: static class UClass * __cdecl UDisDamageType_Impact::StaticClassNoInline(void)
//   0x7b6290  public: virtual void __thiscall ADishonoredNPCPawn::OnSeverLimb(class UDisSeqAct_SeverLimb *)
//   0x7b6470  public: unsigned int __thiscall ADishonoredNPCPawn::SeverLimbsUponDeath(class FVector const &, struct FDisSeverLimbsDamageParams const *, class ADishonoredPlayerPawn *)
//   0x7b6830  public: unsigned int __thiscall ADishonoredNPCPawn::SeverLimbForPinning(class FVector const &, struct FDisSeverLimbsDamageParams const *, class ADishonoredPlayerPawn *, class ADisProjectile_Arrow *)
//   0x7b6b40  public: void __thiscall ADishonoredNPCPawn::StopMatinees(struct TMemStackArray<class USeqAct_Interp *> const &, class FString const *)
//   0x7b6cd0  public: virtual void __thiscall ADishonoredNPCPawn::OnSpawnStealable(class UDisSeqAct_SpawnStealable *)
//   0x7b6d70  public: unsigned int __thiscall ADishonoredNPCPawn::WillLocationCauseTethering(class FVector const &)const
//   0x7b9a10  public: void __thiscall ADishonoredNPCPawn::HideLimb(int, unsigned int, int, class UDisSkeletalMeshComponent * &)
//   0x7b9cf0  protected: unsigned int __thiscall ADishonoredNPCPawn::OnLooted(class ADishonoredPlayerPawn *)
//   0x7b9ef0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::AttemptAltInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x7ba010  protected: virtual void __thiscall ADishonoredNPCPawn::OnRigidBodyCollision(struct FRigidBodyCollisionInfo const &, struct FRigidBodyCollisionInfo const &, struct FCollisionImpactData const &)
//   0x7ba3d0  MakePushActor
//   0x7ba780  public: virtual void __thiscall ADishonoredNPCPawn::SetPushesRigidBodies(unsigned int)
//   0x7baa20  public: virtual unsigned int __thiscall ADishonoredNPCPawn::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x7bac70  protected: virtual void __thiscall ADishonoredNPCPawn::UpdateNodeSlot(void)
//   0x7baea0  public: void __thiscall ADishonoredNPCPawn::MatineeControllerRemove(class USeqAct_Interp * const, enum ESoireeAIPriority)
//   0x7baf10  public: void __thiscall ADishonoredNPCPawn::StartPhysicsSensor(unsigned int)
//   0x7bb9b0  public: void __thiscall ADishonoredNPCPawn::StopPhysicsSensor(void)
//   0x7bba20  public: virtual void __thiscall ADishonoredNPCPawn::TermRBPhys(class FRBPhysScene *)
//   0x7bba40  public: void __thiscall ADishonoredNPCPawn::OnPossessed(void)
//   0x7bbaf0  protected: virtual void __thiscall ADishonoredNPCPawn::OnOtherActorTerminated(class AActor const &)
//   0x7bbbc0  public: virtual void __thiscall ADishonoredNPCPawn::OnNPCSetMaterials(class UDisSeqAct_NPCSetMaterials *)
//   0x7bdb70  public: virtual void __thiscall ADishonoredNPCPawn::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7bde30  public: virtual void __thiscall ADishonoredNPCPawn::ExitFromMatinees(enum ESoireeAIPriority, class FString const *)
//   0x7bdfd0  public: virtual void __thiscall ADishonoredNPCPawn::ExitFromMatinees(class FGuid, class FString const *)
//   0x7be9c0  protected: virtual void __thiscall ADishonoredNPCPawn::ApplyTweakChanges_Derived(void)
//   0x7bfce0  protected: void __thiscall ADishonoredNPCPawn::OnLODChanged(class FArkGameEvent const &)
//   0x7c0030  public: class UDishonoredAIBrain * __thiscall ADishonoredNPCPawn::GetAIBrain(void)
//   0x7c0080  public: virtual class UDishonoredAIBrain const * __thiscall ADishonoredNPCPawn::GetAttnTargetBrain(void)const
//   0x7c00d0  private: enum eCrossHairStatus __thiscall ADishonoredNPCPawn::GetCrosshairStatus_Helper(class ADishonoredPawn *)const
//   0x7c02b0  public: virtual enum eCrossHairStatus __thiscall ADishonoredNPCPawn::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x7c0380  protected: void __thiscall ADishonoredNPCPawn::OnGave(void)
//   0x7c0450  public: void __thiscall ADishonoredNPCPawn::UpdateAvoidableCollisionGroupFlags(void)
//   0x7c04e0  protected: virtual void __thiscall ADishonoredNPCPawn::OnRelationshipChange(void)
//   0x7c0570  public: virtual void __thiscall ADishonoredNPCPawn::MAT_BeginAIGroup(class UInterpGroupInstAI *)
//   0x7c0830  public: virtual void __thiscall ADishonoredNPCPawn::MAT_FinishAIGroup(class UInterpGroupInstAI *)
//   0x7c0a10  public: virtual void __thiscall ADishonoredNPCPawn::MAT_SendBodyIntentionRequest(class UInterpTrackAIControlBodyIntentionKeyProperties const *)
//   0x7c0c20  public: virtual unsigned int __thiscall ADishonoredNPCPawn::MAT_IsAllowed(class USeqAct_Interp *, class UInterpGroupAI *, enum ESoireeAIPriority, unsigned int, class FString *)
//   0x7c0e50  public: class FDisComponentVisionNPC * __thiscall ADishonoredNPCPawn::GetComponentVision(void)const
//   0x7c0e80  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAttacking(class ADishonoredPawn const &)const
//   0x7c0ef0  private: void __thiscall ADishonoredNPCPawn::UpdateTetherVolume(void)
//   0x7c1010  public: virtual void __thiscall ADishonoredNPCPawn::OnSetIgnoreDeath(class UDisSeqAct_SetIgnoreDeath *)
//   0x7c1d20  protected: virtual void __thiscall ADishonoredNPCPawn::ShutPawnDown_Derived(void)
//   0x7c1e30  public: virtual void __thiscall ADishonoredNPCPawn::OnNPCDoTeleportSpell(class UDisSeqAct_NPCDoTeleportSpell *)
//   0x7c2000  public: enum EAIAwareness __thiscall ADishonoredNPCPawn::GetAwarenessLevel(void)const
//   0x7c2040  public: virtual unsigned int __thiscall ADishonoredNPCPawn::SetupPathGoalsAndConstraints(class FVector const &, unsigned int)
//   0x7c21f0  public: unsigned int __thiscall ADishonoredNPCPawn::CanBeChokedBy(class ADishonoredPlayerPawn const * const)const
//   0x7c2530  public: virtual float __thiscall ADishonoredNPCPawn::GetAltInteractTime(void)const
//   0x7c25c0  public: virtual void __thiscall ADishonoredNPCPawn::FillUIAltInteraction(struct FDisUIInteractionContext &)const
//   0x7c2640  public: unsigned int __thiscall ADishonoredNPCPawn::SafeToVanish(unsigned int, float)
//   0x7c26b0  public: void __thiscall ADishonoredNPCPawn::OnUnpossessed(void)
//   0x7c26d0  public: static class UClass * __cdecl UDisSeqAct_SpawnStealable::GetPrivateStaticClassUDisSeqAct_SpawnStealable(wchar_t const *)
//   0x7c2760  public: void __thiscall ADishonoredNPCPawn::OnEnterTetherVolume(class ADisTetherVolume * const)
//   0x7c2780  public: void __thiscall ADishonoredNPCPawn::OnExitAllTetherVolumes(void)
//   0x7c2790  public: void __thiscall ADishonoredNPCPawn::BecomeOfficiallyDead_NPC(void)
//   0x7c3db0  public: unsigned int __thiscall ADishonoredNPCPawn::MoveAtSafeLocation(class FVector const &)
//   0x7c4170  protected: void __thiscall ADishonoredNPCPawn::CheckForVanish(float)
//   0x7c4230  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanAltInteract(class ADishonoredPawn const * const)const
//   0x7c4290  public: static class UClass * __cdecl UDisSeqAct_SpawnStealable::StaticClassNoInline(void)
//   0x7c5c60  public: class ADisMovableLimb * __thiscall ADishonoredNPCPawn::SpawnLimb(int, unsigned int, int, int, struct FDisSeveredLimb const *, class FName const &, class UDisSkeletalMeshComponent * &)
//   0x7c7400  public: static class UClass * __cdecl ADishonoredNPCPawn::GetPrivateStaticClassADishonoredNPCPawn(wchar_t const *)
//   0x7c7490  public: class ADisMovableLimb * __thiscall ADishonoredNPCPawn::SeverLimb(struct FDisSeveredLimbRequest const &)
//   0x7c7c40  public: virtual void __thiscall ADishonoredNPCPawn::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x7c8070  public: static void __cdecl ADishonoredNPCPawn::InitializePrivateStaticClassADishonoredNPCPawn(void)
//   0x7c8090  private: virtual unsigned int __thiscall ADishonoredNPCPawn::Tick(float, enum ELevelTick)
//   0x7c84d0  public: static class UClass * __cdecl ADishonoredNPCPawn::StaticClassNoInline(void)
//   0x7c8770  private: virtual void __thiscall ADishonoredNPCPawn::PostBeginPlay(void)
//   0x7c9230  protected: void __thiscall ADishonoredNPCPawn::OnStolen(void)
//   0x7c93b0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)

// ---- agent CG ports (PHASE9 CG): the accessors the brain reads every frame ----

#include "DishonoredGame.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

// DISHONORED(port): 2013 rva 0x7704b0 (2012 0x7ae420) / 0x7704d0 (0x7ae440) / 0x7704f0 (0x7ae460): the live entry of
// m_BodyIntention for each of the three aspects, selected by its own priority byte.
BYTE ADishonoredNPCPawn::GetBodyStance() const
{
	return m_BodyIntention[m_CurrentBodyStancePriority].m_IntendedBodyStance;
}

UClass* ADishonoredNPCPawn::GetDesiredPrimaryItem() const
{
	return m_BodyIntention[m_CurrentPrimaryItemPriority].m_pDesiredPrimaryItemClass;
}

UClass* ADishonoredNPCPawn::GetDesiredSecondaryItem() const
{
	return m_BodyIntention[m_CurrentSecondaryItemPriority].m_pDesiredSecondaryItemClass;
}

// DISHONORED(port): 2013 rva 0x76e8c0 (2012 0x7c9560): one slot of m_BodyIntention is written and the three "current"
// priority bytes are recomputed as the highest priority that has a non-default intent for each aspect.
// DISHONORED(bringup): retail also asks the item contexts to equip or unequip towards the new intent
// (UDisItemContext, agentAJ.md's third root, unported), and pushes the stance into the anim state component. The
// intention itself is stored and read back correctly, which is what the AI and the census depend on.
void ADishonoredNPCPawn::SetBodyIntention( BYTE _Priority, BYTE _BodyStance, UClass* _pPrimaryItemClass, UClass* _pSecondaryItemClass )
{
	if( _Priority >= ARRAY_COUNT(m_BodyIntention) )
	{
		return;
	}
	m_BodyIntention[_Priority].m_IntendedBodyStance = _BodyStance;
	m_BodyIntention[_Priority].m_pDesiredPrimaryItemClass = _pPrimaryItemClass;
	m_BodyIntention[_Priority].m_pDesiredSecondaryItemClass = _pSecondaryItemClass;

	m_CurrentBodyStancePriority = 0;
	m_CurrentPrimaryItemPriority = 0;
	m_CurrentSecondaryItemPriority = 0;
	for( INT Priority = ARRAY_COUNT(m_BodyIntention) - 1; Priority > 0; Priority-- )
	{
		if( m_CurrentBodyStancePriority == 0 && m_BodyIntention[Priority].m_IntendedBodyStance != 0 )
		{
			m_CurrentBodyStancePriority = Priority;
		}
		if( m_CurrentPrimaryItemPriority == 0 && m_BodyIntention[Priority].m_pDesiredPrimaryItemClass )
		{
			m_CurrentPrimaryItemPriority = Priority;
		}
		if( m_CurrentSecondaryItemPriority == 0 && m_BodyIntention[Priority].m_pDesiredSecondaryItemClass )
		{
			m_CurrentSecondaryItemPriority = Priority;
		}
	}
}

// DISHONORED(port): 2013 rva 0x74b010 (2012 0x7ab570) / 0x74b020 (0x7ab580): the one-shot request the relationship
// system raises and the brain consumes, so a relationship change is acted on exactly once.
UBOOL ADishonoredNPCPawn::ShouldAIBeNotifiedOfRelationshipChange() const
{
	return m_bNotifyAIOfRelationshipChange != 0;
}

void ADishonoredNPCPawn::AcknowledgeRelationshipChangeHandledByAI()
{
	m_bNotifyAIOfRelationshipChange = FALSE;
}

// DISHONORED(port): 2013 rva 0x7599b0 (2012 0x7c0030): the pawn only admits to having a brain when it is alive, its
// controller is an NPC controller and that brain has finished initialising. Every caller relies on that, which is why
// a dying NPC stops answering AI queries one frame before its controller stops ticking.
UDishonoredAIBrain* ADishonoredNPCPawn::GetAIBrain()
{
	ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Controller );
	if( DisIsPawnDead( this ) || !NPCController || !NPCController->m_pAIBrain )
	{
		return NULL;
	}
	if( !NPCController->m_pAIBrain->IsBrainInitialized() )
	{
		return NULL;
	}
	return NPCController->m_pAIBrain;
}

// DISHONORED(port): the predicate ADishonoredNPCController::OnAISetSuspicionLevel reads to choose the unsuspecting
// stance. Retail's own body tests the pawn's NPC tweaks; an NPC that has a melee or ranged loadout is a fighter.
UBOOL ADishonoredNPCPawn::IsAFighter() const
{
	UDisTweaks_NPCPawn* NPCTweaks = Cast<UDisTweaks_NPCPawn>( ( (ADishonoredNPCPawn*)this )->GetTweaks_Derived() );
	if( !NPCTweaks )
	{
		NPCTweaks = (UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	return NPCTweaks->m_pContentInventory_DefaultEquipType_Primary != NULL;
}

// ---- agent CG ports (PHASE9 CG): the damage entry point ----

// DISHONORED(port): 2013 rva 0x783680 (2012 0x7c6d50): the NPC's own damage pass. Its whole observable job is to record
// the shot in m_LastNPCDamageInfo, which is what the death chain, the kill-cam and the severed-limb code all read back;
// the actual health change is APawn::TakeDamage's, below.
// DISHONORED(bringup): four refinements of retail's body are not ported and are named rather than skipped silently:
//   UDishonoredInventory::DamageInventoryItems (2013 rva 0x8064d0) - the equipped item takes its share of the damage;
//   UDisTweaks_Vulnerability::IsImmuneToDamageType / IDisVulnerabilityInterface::IsObjectVulnerableToInstaDeath - the
//     per-NPC vulnerability table, so every NPC is currently vulnerable to everything;
//   IDisRatTargetInterface::DamageAttachedRats - a swarm riding the NPC takes the damage first;
//   UDishonoredAIBrain::HandleIncomingDamage (2013 rva 0x723780) - the brain's reaction, which needs the attention
//     process this package could not reach (see agentCG.md).
// ADishonoredPawn::GetBoneRegionInfo (0x74d6e0) is the fifth: without it m_pDamageRegion stays NULL and a headshot
// counts as a body shot.
void ADishonoredNPCPawn::TakeDamage_Native( INT& _rDamage, AController* _pInstigatedBy, FVector _HitLocation, FVector& _rMomentum, UClass* _pDamageType, FTraceHitInfo _HitInfo, AActor* _pDamageCauser )
{
	m_LastNPCDamageInfo.m_pDamageCausingActor = _pDamageCauser;
	m_LastNPCDamageInfo.m_pDamageType = _pDamageType;
	m_LastNPCDamageInfo.m_DamageHitLocation = _HitLocation;
	m_LastNPCDamageInfo.m_pDamageInstigator = DisGetPawnInstigator( _pInstigatedBy );
	m_LastNPCDamageInfo.m_DamageHitInfo = _HitInfo;
	m_LastNPCDamageInfo.m_DamageMomentum = _rMomentum;
	m_LastNPCDamageInfo.m_iHealthBeforeDamage = Health;
	m_LastNPCDamageInfo.m_iDamage = _rDamage;
}

// DISHONORED(port): 2013 rva 0x74a340's shape (2012 ADishonoredPawn::TakeDamage 0x7acae0): the script-facing native
// reads its seven parameters, hands them to the virtual TakeDamage_Native so the game-specific pass can adjust the
// damage and the momentum in place, and then lets APawn::TakeDamage apply what is left.
void ADishonoredNPCPawn::execTakeDamage( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(Damage);
	P_GET_OBJECT(AController, InstigatedBy);
	P_GET_STRUCT(FVector, HitLocation);
	P_GET_STRUCT(FVector, Momentum);
	P_GET_OBJECT(UClass, DamageType);
	P_GET_STRUCT_OPTX(FTraceHitInfo, HitInfo, FTraceHitInfo(EC_EventParm));
	P_GET_OBJECT_OPTX(AActor, DamageCauser, NULL);
	P_FINISH;

	TakeDamage_Native( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	APawn::TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
}

// ---- agent DI ports (PHASE10 DI): the NPC's PostBeginPlay ----

// DISHONORED(port): 2013 rva 0x76a610 (2012 0x7c8770). After the base pass has built the head mesh, the head is bound
// to the body as its animation parent - that is what makes the two meshes move as one character - and the three
// per-spawn counters are cleared.
// DISHONORED(bringup): retail's remaining work needs subsystems that are not ported, each named here:
//   FDisPhysicsUtil::DisCreatePawnChannel plus SetRBChannel / SetRBCollidesWithChannel and
//     UPhysicsAssetInstance::SetFullAnimWeightBonesFixed - the pawn's rigid-body channel, which needs the Dishonored
//     collision-channel allocator;
//   the ammo copy out of the tweaks' m_Ammo and SpawnInventoryLoadout_Items / EquipItemByType / AttachEquippedItem -
//     UDishonoredInventory and UDisItemContext, agent AJ's third root (the same gap CG recorded for the melee draw);
//   FDisComponentPlague, added when the tweaks' m_bHasPlague bit is set;
//   ADishonoredNPCPawn::RegisterAvoidable (2013 rva 0x7796b0) and the FArkGameEventDispatcher registration of OnLODChanged - the
//     dispatcher has no instance in this tree (agent CG hand-over 4);
//   FDisComponentLODManager::StartLOD (2013 rva 0x870a80), so the NPC never enters the LOD state machine.
void ADishonoredNPCPawn::PostBeginPlay()
{
	Super::PostBeginPlay();
	if( m_pHeadMesh )
	{
		m_pHeadMesh->SetParentAnimComponent( Mesh );
	}
	DesiredSpeed = 0.f;
	m_iNumReactionDisableRequestsFromSoiree = 0;
	m_iNumFakeDeathRequestsFromSoiree = 0;
}

/*-----------------------------------------------------------------------------
	Agent FS (PHASE17 package FS): the matinee contract, the AI-group half.

	UInterpGroupInstAI::InitPawn (Engine) calls MAT_SetForceInventory and MAT_BeginAIGroup when a scene
	binds this pawn, UInterpGroupAI::UpdateGroup calls MAT_SetCollision / MAT_SetMeshTranslationMode
	every frame with what the StretchAnimControl keys asked for, and TermGroupInst calls MAT_FinishAIGroup.

	MAT_BeginAIGroup (2012 rva 0x7c0570, 696 B; 2013 0xb65080, 720 B) and MAT_FinishAIGroup (0x7c0830,
	478 B; 2013 0xb65350, 521 B) follow the 2013 bodies. In: the mesh-offset snap (agent GA), the master FSM
	exit request, UpdateNodeSlot, the pose blend in (m_pBlendPoseNode->BlendNow( 0.2, TRUE ) when this is the
	pawn's first matinee controller) and MatineeControllerAdd / Remove, the reaction-disable count, the jiggle
	bones (bDisableJiggleBones -> Mesh->m_bDontUpdateKinematic), the locomotion switches, the look-forward
	request on LockRequests and its StopAllLookAtFromAsker / the m_iMatineeLookAtRequestID stop, MAT_BlendOut
	(the blend out, 2012 0x7ad5e0 / 2013 0xb4ec80), the collision and physics choice, the body-intention reset
	at Soiree priority (agents FS, GA, HB), the FAIStimStruct_Soiree start / stop stim (agent IC; 2013 0xb19470 + 0xb5af50). Still missing, named:
	  - FDisComponentLODManager::StartLOD( group, name, eLRP_Soiree, request, -1 ) / StopAllLODByAsker
	    (the LOD manager is not ported);
	  - 2013's bShouldFakeDeath (sub_B5A0B0 / m_iNumFakeDeathRequestsFromSoiree) and the +1288 bit 0x2000;
	  - ApplyLocomotionModifier() on finish, replaced by m_bLocoModifierNeedsApplication = TRUE.
-----------------------------------------------------------------------------*/

#include "arkcomponentlocomotion.h"
#include "arkcomponentmeshoffset.h"
#include "arkcomponentlookat.h"
#include "dishonoredutilities_ai.h"
#include "fkmatineediag.h"
#include "icdiag.h"
#include "aistimstruct.h"

// 2012 rva 0x7bac70 (554 B): SlotNodes emptied and re-collected from the mesh's tree - every
// UArkAnimNodeStack whose CanBeUsedByMatinee is set, an unnamed one taking the editor ini's
// MatineePreview/DefaultAnimSlotName. A mesh without a tree gets its AnimTreeTemplate instanced first
// (or a bare UAnimNodeSequence when it has none).
void ADishonoredNPCPawn::UpdateNodeSlot()
{
	if( Mesh == NULL )
	{
		return;
	}
	SlotNodes.Empty();
	if( Mesh->Animations == NULL )
	{
		if( Mesh->AnimTreeTemplate != NULL )
		{
			Mesh->SetAnimTreeTemplate( Mesh->AnimTreeTemplate );
		}
		else
		{
			Mesh->DeleteAnimTree();
			Mesh->Animations = ConstructObject<UAnimNodeSequence>( UAnimNodeSequence::StaticClass() );
			if( Mesh->Animations != NULL )
			{
				Mesh->InitAnimTree( TRUE );
			}
		}
	}
	if( Mesh->AnimTreeTemplate == NULL || Mesh->Animations == NULL )
	{
		return;
	}
	TArray<UAnimNode*> Nodes;
	Mesh->Animations->GetNodesByClass( Nodes, UArkAnimNodeStack::StaticClass() );
	for( INT NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++ )
	{
		UArkAnimNodeStack* Stack = (UArkAnimNodeStack*)Nodes(NodeIndex);
		if( !Stack->CanBeUsedByMatinee )
		{
			continue;
		}
		SlotNodes.AddUniqueItem( Stack );
		if( Stack->NodeName == NAME_None )
		{
			Stack->NodeName = FName( *GConfig->GetStr( TEXT("MatineePreview"), TEXT("DefaultAnimSlotName"), GEditorIni ) );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7b19a0 (180 B) / 2013 0xb53500 (106 B): one entry per binding, the matinee, its AI priority, blocking
void ADishonoredNPCPawn::MatineeControllerAdd( USeqAct_Interp* const Matinee, BYTE SoireePriority, UBOOL bIsBlocking )
{
	FMatineeControllerInfos& Info = m_MatineeControllersInfos( m_MatineeControllersInfos.AddZeroed() );
	Info.m_pMatinee = Matinee;
	Info.m_SoireePriority = SoireePriority;
	Info.m_bIsBlocking = bIsBlocking ? 1 : 0;
	if( DisICDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): icdiag controller %s add %s prio %d blocking %d count %d world %.2f"), *GetName(),
			Matinee ? *Matinee->GetName() : TEXT("none"), SoireePriority, bIsBlocking ? 1 : 0, m_MatineeControllersInfos.Num(), GWorld->GetTimeSeconds() );
	}
}

// DISHONORED(port): 2012 rva 0x7baea0 (111 B) / 2013 0xb5f760: every entry of the matinee, whatever the priority argument
void ADishonoredNPCPawn::MatineeControllerRemove( USeqAct_Interp* const Matinee, BYTE SoireePriority )
{
	for( INT Index = m_MatineeControllersInfos.Num() - 1; Index >= 0; Index-- )
	{
		if( m_MatineeControllersInfos(Index).m_pMatinee == Matinee )
		{
			m_MatineeControllersInfos.Remove( Index );
		}
	}
	if( DisICDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): icdiag controller %s remove %s count %d world %.2f"), *GetName(),
			Matinee ? *Matinee->GetName() : TEXT("none"), m_MatineeControllersInfos.Num(), GWorld->GetTimeSeconds() );
	}
}

// DISHONORED(port): 2012 rva 0x7b1a60 (138 B) / 2013 0xb53570: the entries at this priority and blocking state
INT ADishonoredNPCPawn::GetMatineeControllerCount( BYTE SoireePriority, UBOOL bIsBlocking ) const
{
	INT Count = 0;
	for( INT Index = 0; Index < m_MatineeControllersInfos.Num(); Index++ )
	{
		const FMatineeControllerInfos& Info = m_MatineeControllersInfos(Index);
		if( Info.m_SoireePriority == SoireePriority && Info.m_bIsBlocking == ( bIsBlocking ? 1 : 0 ) )
		{
			Count++;
		}
	}
	return Count;
}

// DISHONORED(port): 2012 rva 0x7bdfd0 (422 B) / 2013 0xb65940 (vtable +1236): the matinees of that guid. Retail collects them in a
// TMemStackArray on GMainThreadMemStack; a TArray here (named).
void ADishonoredNPCPawn::ExitFromMatinees( FGuid MatineeGuid, const FString* Reason )
{
	if( !GIsGame )
	{
		return;
	}
	TArray<USeqAct_Interp*> MatineesToStop;
	for( INT Index = 0; Index < m_MatineeControllersInfos.Num(); Index++ )
	{
		USeqAct_Interp* Matinee = m_MatineeControllersInfos(Index).m_pMatinee;
		if( Matinee && Matinee->m_MatineeGUID == MatineeGuid )
		{
			MatineesToStop.AddItem( Matinee );
		}
	}
	StopMatinees( MatineesToStop, Reason );
}

// DISHONORED(port): 2012 rva 0x7bde30 (404 B) / 2013 0xb657a0 (vtable +1240): the matinees at that priority, ESAP_MAX = all
void ADishonoredNPCPawn::ExitFromMatinees( BYTE SoireePriority, const FString* Reason )
{
	if( !GIsGame )
	{
		return;
	}
	TArray<USeqAct_Interp*> MatineesToStop;
	for( INT Index = 0; Index < m_MatineeControllersInfos.Num(); Index++ )
	{
		const FMatineeControllerInfos& Info = m_MatineeControllersInfos(Index);
		if( SoireePriority == ESAP_MAX || Info.m_SoireePriority == SoireePriority )
		{
			MatineesToStop.AddItem( Info.m_pMatinee );
		}
	}
	StopMatinees( MatineesToStop, Reason );
}

// DISHONORED(port): 2012 rva 0x7b6b40 (387 B) / 2013 0xb5a120 (410 B). The pawn's own AI group is terminated in every matinee whose
// group may not interrupt it (CanInterruptMatinee clear), detached when the group asks for it (2013: bShouldBeDetached ->
// MAT_Detach, vtable +796); a matinee none of whose groups refused is stopped as a whole (USeqAct_Interp::StopFromNPC, 2013
// 0x612a60: Stop() unless m_bShouldNotBeInterruptedByDialog). Reason is retail's log text and unused in Shipping.
// DISHONORED(written): a pawn group that is not a UInterpGroupAI is skipped (retail reads the flags through NULL).
void ADishonoredNPCPawn::StopMatinees( const TArray<USeqAct_Interp*>& MatineesToStop, const FString* Reason )
{
	for( INT MatIdx = 0; MatIdx < MatineesToStop.Num(); MatIdx++ )
	{
		USeqAct_Interp* Matinee = MatineesToStop(MatIdx);
		UBOOL bInterruptionIsAllowed = TRUE;
		for( INT GroupIdx = Matinee->GroupInst.Num() - 1; GroupIdx >= 0; GroupIdx-- )
		{
			UInterpGroupInstAI* GroupInst = Cast<UInterpGroupInstAI>( Matinee->GroupInst(GroupIdx) );
			if( !GroupInst || GroupInst->GetGroupActor() != this )
			{
				continue;
			}
			UInterpGroupAI* AIGroup = Cast<UInterpGroupAI>( GroupInst->Group );
			if( !AIGroup )
			{
				continue;
			}
			if( !AIGroup->CanInterruptMatinee )
			{
				GroupInst->TermGroupInst( TRUE );
				bInterruptionIsAllowed = FALSE;
			}
			if( AIGroup->bShouldBeDetached )
			{
				MAT_Detach();
			}
		}
		if( bInterruptionIsAllowed && !Matinee->m_bShouldNotBeInterruptedByDialog )
		{
			Matinee->Stop();
		}
		if( DisICDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): icdiag exitmatinee %s %s interrupt %d world %.2f"), *GetName(), *Matinee->GetName(),
				bInterruptionIsAllowed ? 1 : 0, GWorld->GetTimeSeconds() );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7ad720 (30 B): the force-inventory bit counts only while a matinee holds the pawn
UBOOL ADishonoredNPCPawn::MAT_ShouldForceInventory() const
{
	return m_MatineeControllersInfos.Num() > 0 && m_bShouldForceBodyIntention;
}

// DISHONORED(port): 2012 rva 0x7ad740 (72 B): empty hands, no stance
void ADishonoredNPCPawn::MAT_GetBodyIntention( BYTE& BodyStance, UClass*& PrimaryItem, UClass*& SecondaryItem ) const
{
	BodyStance = 0;
	PrimaryItem = UDishonoredItemEmpty::StaticClass();
	SecondaryItem = UDishonoredItemEmpty::StaticClass();
}

// DISHONORED(port): 2013 rva 0xb4ec80 (209 B; 2012 0x7ad5e0, 194 B). A group with a blend-out time blends the pawn out of the
// matinee pose through the pose blender unless the pawn is being thrown, assassinated or choked, or is a ragdoll; 2013 adds
// the else branch, two frames of m_iPreventRBVelFromAnimFrameCount.
void ADishonoredNPCPawn::MAT_BlendOut( UInterpGroupInst* GroupInst )
{
	const FLOAT BlendOutTime = GroupInst->GetBlendOutTime();
	if( !( BlendOutTime > 0.f ) )
	{
		return;
	}
	UDishonoredNativeState* LogicalState = ( m_pNPCMasterFSM != NULL ) ? m_pNPCMasterFSM->GetLogicalState() : NULL;
	const UBOOL bBeingHandled = LogicalState != NULL
		&& ( LogicalState->IsA( UStateNPCMasterThrown::StaticClass() ) || LogicalState->IsA( UStateNPCMasterBeingAssassinated::StaticClass() )
			|| LogicalState->IsA( UStateNPCMasterBeingChoked::StaticClass() ) );
	if( m_pBlendPoseNode == NULL || Physics == PHYS_RigidBody || bBeingHandled )
	{
		m_iPreventRBVelFromAnimFrameCount = 2;
	}
	else
	{
		m_pBlendPoseNode->BlendNow( BlendOutTime, TRUE );
	}
	if( DisFKDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag blendpose %s MAT_BlendOut group '%s' blendOut %.2f node %s physics %d handled %d world %.2f"),
			*GetName(), GroupInst->Group ? *GroupInst->Group->GroupName.ToString() : TEXT("?"), BlendOutTime,
			m_pBlendPoseNode ? *m_pBlendPoseNode->GetName() : TEXT("none"), (INT)Physics, bBeingHandled, GWorld->GetTimeSeconds() );
	}
}

// Agent IC: retail lists the pawn's matinees (MatineeControllerAdd) and hands the brain the FAIStimStruct_Soiree start stim in
// the same breath; UDisAIBrainProcessAttention::UpdateSoireeAttention then holds every other pawn of those matinees at
// DMALT_Soiree / DAL_Busted (retail's 2013 0xb3f120 does the same) and UDisBehaviorSoiree keeps a non-blocking soiree's brain quiet.
// DISHONORED(bringup): both stay OFF unless -hbcontrollers (agent HB's switch). Measured (agentIC.md): with the list filled the
// render thread dies in FParticleSystemSceneProxy::ReleaseRenderThreadResources 0.1-1 s after the boat matinee binds in every
// run with the soiree stim and behaviour ported (IC1a, IC1b, IC1e, IC1f; IC1d with -icnostim too), and in none with
// -icnosoireeattn (the co-actors not held at Busted: IC1c, IC1g); the boat is a blocking matinee, which UDisBehaviorSoiree never
// takes (EvaluateSoiree, retail's),
// so the stim cannot be what keeps retail alive. -icnostim sends no soiree stim with the list on; -icpsysdiag names the particle
// components touched after the blocking soiree starts.
static UBOOL DisHBControllersEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( appStrfind( appCmdLine(), TEXT("-hbcontrollers") ) != NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

static UBOOL DisICSoireeStimEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		Enabled = ( DisHBControllersEnabled() && appStrfind( appCmdLine(), TEXT("-icnostim") ) == NULL ) ? 1 : 0;
	}
	return Enabled != 0;
}

// Engine/Src/UnParticleComponents.cpp: -icpsysdiag logs particle component lifecycle calls once this is raised
extern UBOOL GDisICParticleWatch;

static void DisSendSoireeStim( ADishonoredNPCPawn* _pPawn, const FGuid& _MatineeGuid, BYTE _Event, BYTE _Priority, UBOOL _bIsBlocking )
{
	if( !_pPawn->Controller || !DisICSoireeStimEnabled() )
	{
		return;
	}
	UDishonoredAIBrain* Brain = _pPawn->GetAIBrain();
	if( !Brain )
	{
		return;
	}
	FAIStimStruct_Soiree Stim = DisMakeStim< FAIStimStruct_Soiree >( EAIStimID_Soiree );
	Stim.m_SoireeGUID = _MatineeGuid;
	Stim.m_SoireeEvent = _Event;
	Stim.m_SoireePriority = _Priority;
	Stim.m_bIsBlocking = _bIsBlocking ? 1 : 0;
	DisHandleAIStim( Brain, Stim, _pPawn );
	if( _bIsBlocking && _Event == ESE_Start )
	{
		GDisICParticleWatch = TRUE;
	}
	if( DisICDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): icdiag soireestim %s send %s prio %d blocking %d guid %08X world %.2f"), *_pPawn->GetName(),
			_Event == ESE_Start ? TEXT("Start") : ( _Event == ESE_Stop ? TEXT("Stop") : TEXT("Break") ), _Priority, _bIsBlocking ? 1 : 0,
			_MatineeGuid.A, GWorld->GetTimeSeconds() );
	}
}

// DISHONORED(bringup): agent HB. -hbnolockreq: no look-forward request on LockRequests (an A/B switch).
static UBOOL DisHBLockRequestsDisabled()
{
	static INT Disabled = -1;
	if( Disabled < 0 )
	{
		Disabled = ( appStrfind( appCmdLine(), TEXT("-hbnolockreq") ) != NULL ) ? 1 : 0;
	}
	return Disabled != 0;
}

// 2012 rva 0x7c0570 / 2013 0xb65080
void ADishonoredNPCPawn::MAT_BeginAIGroup( UInterpGroupInstAI* GroupInst )
{
	UInterpGroupAI* AIGroup = ( GroupInst != NULL ) ? GroupInst->AIGroup : NULL;
	USeqAct_Interp* Seq = ( GroupInst != NULL ) ? Cast<USeqAct_Interp>( GroupInst->GetOuter() ) : NULL;

	// agent GA: retail snaps the mesh offset here when the LOD manager reports it off (FDisComponentLODManager::
	// MeshOffestEnabled); with no LOD manager in this tree the started component is snapped whenever there is one -
	// Tick( 0 ) on an enabled offset is the same snap.
	FArkComponentMeshOffset* MeshOffset = FArkComponentMeshOffset::GetMeshOffsetComponent( this, TRUE );
	if( MeshOffset != NULL )
	{
		MeshOffset->ForceInstantSnapMesh();
	}

	if( !DisIsPawnDead( this ) && m_pNPCMasterFSM != NULL )
	{
		UDishonoredNativeState* Pending = m_pNPCMasterFSM->GetPendingState();
		UDishonoredNativeState* Current = m_pNPCMasterFSM->GetCurrentlyActiveState();
		if( Pending != NULL && Pending != Current )
		{
			Pending->RequestStateExit();
		}
		if( Current != NULL )
		{
			Current->RequestStateExit();
		}
	}

	// DISHONORED(port): agent HB. The slots, then the blend in from the pose the pawn holds now - only for its first matinee.
	UpdateNodeSlot();
	if( m_MatineeControllersInfos.Num() == 0 && m_pBlendPoseNode != NULL )
	{
		m_pBlendPoseNode->BlendNow( 0.2f, TRUE );
	}
	if( Seq != NULL && DisHBControllersEnabled() )
	{
		MatineeControllerAdd( Seq, Seq->m_AIBehaviorPriority, Seq->m_bIsBlocking );
	}
	if( Seq != NULL )
	{
		DisSendSoireeStim( this, Seq->m_MatineeGUID, ESE_Start, Seq->m_AIBehaviorPriority, Seq->m_bIsBlocking );
	}

	if( AIGroup != NULL && AIGroup->bDisableHitReact )
	{
		m_iNumReactionDisableRequestsFromSoiree++;
	}
	// DISHONORED(port): agent HB. 2013 0xb65080 tests the AI group's bDisableJiggleBones (+120 bit 0x20) here.
	if( AIGroup != NULL && AIGroup->bDisableJiggleBones && Mesh != NULL )
	{
		Mesh->m_bDontUpdateKinematic = 1;
	}

	FArkComponentLocomotion* Loco = GetComponentLocomotion();
	if( Loco != NULL )
	{
		Loco->DisableSteering();
		Loco->AllowToFall( TRUE );
		Loco->SetModifier( 0 );
	}

	// DISHONORED(port): agent HB. LockRequests: a look-forward request at the group's priority, asker the AI group.
	FArkComponentLookat* Lookat = ( AIGroup != NULL && AIGroup->LockRequests && !DisHBLockRequestsDisabled() ) ? FArkComponentLookat::GetLookatComponent( this ) : NULL;
	if( Lookat != NULL )
	{
		const INT RequestID = Lookat->StartLookForward( AIGroup, AIGroup->GetFName(), UInterpTrackLookAt::GetGameLookatPriority( AIGroup->LookatRequestsPriority ), 0.5f, -1.f );
		if( DisFKDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): fkdiag lookattrack %s LockRequests forward prio %d req %d pawn %s"),
				*AIGroup->GroupName.ToString(), UInterpTrackLookAt::GetGameLookatPriority( AIGroup->LookatRequestsPriority ), RequestID, *GetName() );
		}
	}
}

// 2012 rva 0x7c0830 / 2013 0xb65350
void ADishonoredNPCPawn::MAT_FinishAIGroup( UInterpGroupInstAI* GroupInst )
{
	UInterpGroupAI* AIGroup = ( GroupInst != NULL ) ? GroupInst->AIGroup : NULL;
	USeqAct_Interp* Seq = ( GroupInst != NULL ) ? Cast<USeqAct_Interp>( GroupInst->GetOuter() ) : NULL;

	// DISHONORED(port): agent HB. The LockRequests look-forward goes first, then the blend out.
	FArkComponentLookat* Lookat = GetComponentLookat();
	if( Lookat != NULL && AIGroup != NULL )
	{
		Lookat->StopAllLookAtFromAsker( AIGroup );
	}
	if( GroupInst != NULL )
	{
		MAT_BlendOut( GroupInst );
	}

	MAT_SetCollision( FALSE, FALSE );
	MAT_SetRootMotionMode( MRMM_Disable );
	MAT_EnableMeshTranslation();
	m_bOverrideReactionDisableFromSoiree = 0;
	SetBodyIntention( DisBodyIntentionPriority_Soiree, eDisNPCBodyStance_None, NULL, NULL );

	FArkComponentLocomotion* Loco = GetComponentLocomotion();
	if( Loco != NULL )
	{
		Loco->EnableSteering();
		Loco->AllowToFall( FALSE );
		m_bLocoModifierNeedsApplication = 1;
	}

	if( Seq != NULL )
	{
		DisSendSoireeStim( this, Seq->m_MatineeGUID, ESE_Stop, ESAP_MAX, FALSE );
		MatineeControllerRemove( Seq, Seq->m_AIBehaviorPriority );
	}
	if( AIGroup != NULL && AIGroup->bDisableJiggleBones && Mesh != NULL )
	{
		Mesh->m_bDontUpdateKinematic = 0;
	}
	if( AIGroup != NULL && AIGroup->bDisableHitReact )
	{
		m_iNumReactionDisableRequestsFromSoiree--;
	}
	MAT_StopRootMotion();
	m_bDisableCollisionForSoiree = 0;
	if( m_iMatineeLookAtRequestID != INDEX_NONE && Lookat != NULL )
	{
		Lookat->StopLookAt( m_iMatineeLookAtRequestID );
	}
}

// 2012 rva 0x7ab5c0 (30 B): bit 21 of the flag word at +2180 is m_bShouldForceBodyIntention (2012 PDB)
void ADishonoredNPCPawn::MAT_SetForceInventory( UBOOL bShouldForceEmptyHand )
{
	m_bShouldForceBodyIntention = bShouldForceEmptyHand ? 1 : 0;
}

// 2012 rva 0x7ab7a0 (252 B). A pawn the retargeting component carries counts as collision-disabled and stays in PHYS_None
// (agent HC: the in-game retargeting test). PHYS_RigidBody is left alone: a ragdolled pawn stays one.
void ADishonoredNPCPawn::MAT_SetCollision( UBOOL bDisableCollisionForSoiree, UBOOL bHaveMoveTrack )
{
	const UBOOL bRetargeting = GIsGame && IsRetargeting();
	m_bDisableCollisionForSoiree = ( bDisableCollisionForSoiree || bRetargeting ) ? 1 : 0;
	if( m_bDisableCollisionForSoiree || bRetargeting )
	{
		if( Physics != PHYS_None )
		{
			setPhysics( PHYS_None );
		}
	}
	else if( bHaveMoveTrack )
	{
		if( Physics != PHYS_Interpolating )
		{
			setPhysics( PHYS_Interpolating );
		}
	}
	else if( Physics != PHYS_RigidBody )
	{
		setPhysics( PHYS_Walking );
	}
}

// 2012 rva 0x7ab6e0 (46 B) and 0x7ab720 (33 B): both yield to the retargeting component, which owns the mesh translation
// while it carries the pawn (agent HC); the first only in game
void ADishonoredNPCPawn::MAT_SetMeshTranslationMode( BYTE MatMeshTranslationMode )
{
	if( !GIsGame || !IsRetargeting() )
	{
		ADishonoredPawn::MAT_SetMeshTranslationMode( MatMeshTranslationMode );
	}
}

void ADishonoredNPCPawn::MAT_DisableMeshTranslation( UBOOL bReset )
{
	if( !IsRetargeting() )
	{
		ADishonoredPawn::MAT_DisableMeshTranslation( bReset );
	}
}

// 2012 rvas 0x7ab8a0 (13 B), 0x7ab710 (15 B)
UBOOL ADishonoredNPCPawn::MAT_IsCollisionDisable() const
{
	return m_bDisableCollisionForSoiree;
}

UBOOL ADishonoredNPCPawn::MAT_PermitLODPause() const
{
	return !m_bRenderedByPlayer;
}

// ---- agent FV (PHASE18 FV): the NPC tick and the relationship change ----

#include "arkcomponentlookat.h"
#include "fvdiag.h"
#include "ibdiag.h"
#include "discomponentretargeting.h"

// DISHONORED(port): 2013 rva 0x76da40 (2012 0x7c8090, 1085 B). In retail's order: m_bNoTick marks the actor ticked and leaves;
// in game, Tick_Rotation, CheckForVanish, the rigid-body contact end, UpdateMeshOriginOffset; ADisPossessablePawn::Tick
// (ADishonoredPawn::Tick here - the possessable half is not ported); then, for a live, non-deleted pawn in game: the
// master FSM ticks, a live NPC updates its equipment and tether volume, counts its distraction cooldown down and, when
// the sleep timer it was given runs out, takes the sleep damage; the deferred gore and ragdoll impulses are flushed;
// UpdateLocomotion; and a live NPC's look-at component is told the mode ChooseLookAt picks - the line agent FT's
// FArkComponentLookat was waiting for. Finally m_bLastRenderedByPlayer follows m_bRenderedByPlayer and a pawn with no
// physics drops bJustTeleported.
// Agent HC: Tick_Rotation (2012 0x7d4d40) is ported and called in retail's place.
// DISHONORED(bringup): CheckForVanish (0x7c4170), ADishonoredKActor::CollideContactSystemEnd
// (0x6698c0), UpdateMeshOriginOffset (0x7abb00), UpdateEquipment (0x7b7b90), UpdateTetherVolume (0x7c0ef0), SeverLimb
// (0x7c7490) with UDishonoredInventory::DropAttachedItems (0x85c940) and the ragdoll AddImpulse flush are not ported; the
// two deferred arrays are emptied as retail empties them so they cannot grow.
UBOOL ADishonoredNPCPawn::Tick( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	if( m_bNoTick )
	{
		bTicked = GWorld->Ticked;
		return TRUE;
	}
	if( GIsGame )
	{
		Tick_Rotation( _fDeltaTime );
	}
	const UBOOL bResult = Super::Tick( _fDeltaTime, _TickType );
	if( !bDeleteMe && !IsPendingKill() && ( !GIsEditor || GIsGame ) )
	{
		if( m_pNPCMasterFSM )
		{
			m_pNPCMasterFSM->TickStateMachine( _fDeltaTime );
		}
		if( !DisIsPawnDead( this ) )
		{
			m_fDistractionCooldownTime_Countdown -= _fDeltaTime;
			if( m_fTimeBeforeSleep > 0.f )
			{
				m_fTimeBeforeSleep -= _fDeltaTime;
				if( m_fTimeBeforeSleep <= 0.f )
				{
					m_LastSleepDamageInfo.m_bAlreadyDeferred = TRUE;
					TakeDamage( m_LastSleepDamageInfo.m_DamageAmount, m_LastSleepDamageInfo.m_pInstigator, Location, FVector( 0.f, 0.f, 0.f ),
						m_LastSleepDamageInfo.m_pDamageType, FTraceHitInfo( EC_EventParm ), NULL );
				}
			}
		}
		if( m_GoreRequestDeferred.Num() > 0 )
		{
			static UBOOL bNoted = FALSE;
			if( !bNoted )
			{
				bNoted = TRUE;
				debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredNPCPawn::Tick: %i deferred gore request(s) dropped on %s; SeverLimb (2012 rva 0x7c7490) is not ported"), m_GoreRequestDeferred.Num(), *GetName() );
			}
			m_GoreRequestDeferred.Empty();
		}
		if( m_RagdollImpulseDeferred.Num() > 0 )
		{
			m_RagdollImpulseDeferred.Empty();
		}
		UpdateLocomotion();
		DisIBDiagReport( this );
		if( !DisIsPawnDead( this ) )
		{
			FArkComponentLookat* Lookat = GetComponentLookat();
			const BYTE LookAtMode = ChooseLookAt();
			if( Lookat )
			{
				Lookat->SetLookAtMode( LookAtMode );
			}
			if( DisFVDiagEnabled() )
			{
				static FLOAT s_fNextReport = 0.f;
				const FLOAT Now = GWorld && GWorld->GetWorldInfo() ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;
				if( Now >= s_fNextReport )
				{
					s_fNextReport = Now + 5.f;
					for( FActorIterator It; It; ++It )
					{
						ADishonoredNPCPawn* NPC = Cast<ADishonoredNPCPawn>( *It );
						if( NPC && !NPC->bDeleteMe )
						{
							FVDIAG( "base %s on %s at %.0f/%.0f/%.0f vel %.0f/%.0f/%.0f physics %i floor %.2f", *NPC->GetName(), NPC->Base ? *NPC->Base->GetName() : TEXT("none"),
								NPC->Location.X, NPC->Location.Y, NPC->Location.Z, NPC->Velocity.X, NPC->Velocity.Y, NPC->Velocity.Z, (INT)NPC->Physics, NPC->Floor.Z );
						}
					}
				}
			}
		}
	}
	m_bLastRenderedByPlayer = m_bRenderedByPlayer;
	if( Physics == PHYS_None )
	{
		bJustTeleported = FALSE;
	}
	return bResult;
}

// DISHONORED(port): 2013 rva 0x759de0 (2012 0x7c04e0, 136 B): the AI is told on its next thought (m_bNotifyAIOfRelationshipChange,
// read by ShouldAIBeNotifiedOfRelationshipChange), the avoidance group flags follow the new relations, and an NPC whose
// plot settings say the game is over once he is hostile to the player ends it when DetermineRelationship( player ) says Enemy.
// DISHONORED(bringup): UpdateAvoidableCollisionGroupFlags (2012 0x7c0450: UArkAvoidable::SetGroupFlags, whose class has no
// ported methods - agent DN) and the game-over check (UDisTweaks_NPCPawn::GetPlotSettings 0x7ece50 / DisGameOver 0x82d5d0,
// neither ported) are named and not run; the relationship itself is Neutral until the faction tables land
// (disglobalfactionmanager.cpp), so the check could not fire.
void ADishonoredNPCPawn::OnRelationshipChange()
{
	m_bNotifyAIOfRelationshipChange = TRUE;
	FVDIAG( "relationship changed on %s (%i individual, %i faction override(s))", *GetName(), m_PersonalRelationships.m_IndividualOverrideMap.Num(), m_PersonalRelationships.m_FactionOverrideMap.Num() );
}

// DISHONORED(port): agent GB, completed by agent HC. 2012 0x7ab5e0 / 2013 0xb4b080 (162 B): retail does not base the NPC on
// the boat - it stops a running retargeting, starts one on the key's bone or socket anchored at the pawn's 'anchor_jnt'
// (rotation allowed, kept closer, no blend time, volume collisions managed) and clears the pawn's base.
void ADishonoredNPCPawn::MAT_Attach( AActor* const InBase, const FAttachmentInfos& AttachInfos )
{
	FDisComponentRetargeting* Retargeting = GetComponentRetargeting();
	if( Retargeting )
	{
		if( Retargeting->m_bIsRetargeting )
		{
			Retargeting->StopRetargeting();
		}
		Retargeting->StartRetargeting( InBase, AttachInfos.BoneName, FName( TEXT("anchor_jnt") ), TRUE, TRUE, 0.f, TRUE );
	}
	SetBase( NULL, FVector( 0.f, 0.f, 1.f ), 1, NULL, NAME_None );
}

// DISHONORED(port): agent HC. 2012 0x7ab690 (65 B): StopRetargeting with m_bDisableCollisionForSoiree cleared around the call
// and restored after it.
void ADishonoredNPCPawn::MAT_Detach()
{
	const UBOOL bDisableCollisionForSoiree = m_bDisableCollisionForSoiree;
	m_bDisableCollisionForSoiree = 0;
	if( FDisComponentRetargeting* Retargeting = GetComponentRetargeting() )
	{
		Retargeting->StopRetargeting();
	}
	m_bDisableCollisionForSoiree = bDisableCollisionForSoiree;
}
