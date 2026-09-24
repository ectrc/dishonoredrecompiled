// DishonoredGame/src/disriverkrust.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (78):
//   0x6921b0  public: static void __cdecl ADisRiverKrust::InitializePrivateStaticClassADisRiverKrust(void)
//   0x6921d0  protected: virtual class FVector __thiscall ADisRiverKrust::GetPrepossessCameraFocus(void)const
//   0x6921f0  protected: virtual enum IDisPossessableInterface::EPossessionAvailability __thiscall ADisRiverKrust::GetPossessionAvailability(void)const
//   0x692220  protected: virtual class USkeletalMeshComponent * __thiscall ADisRiverKrust::GetPossessableSkelMesh(void)
//   0x692230  protected: virtual class FVector __thiscall ADisRiverKrust::GetPossessableLocation(void)const
//   0x692250  protected: virtual void __thiscall ADisRiverKrust::OnOtherActorTerminated(class AActor const &)
//   0x692260  public: virtual struct FDisRelationshipOverrideInfo * __thiscall ADisRiverKrust::GetRelationshipOverrideInfo(void)
//   0x692270  public: class FVector const & __thiscall ADisRiverKrust::GetVisionLocation(void)const
//   0x692280  public: class FRotator const & __thiscall ADisRiverKrust::GetVisionRotation(void)const
//   0x692290  public: virtual void __thiscall ADisRiverKrust::PostLoad(void)
//   0x6922b0  public: virtual void __thiscall ADisRiverKrust::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x6922d0  public: virtual void __thiscall ADisRiverKrust::Serialize(class FArchive &)
//   0x692300  public: virtual void __thiscall ADisRiverKrust::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x6923f0  private: void __thiscall ADisRiverKrust::SetLootCollision(class ADisPickup_Base * const)const
//   0x692460  public: class USkeletalMeshComponent * __thiscall ADisRiverKrust::GetSkeletalMeshComponent(void)const
//   0x692470  public: virtual void __thiscall ADisRiverKrust::OnRiverKrustDisable(class UDisSeqAct_RiverKrustDisable *)
//   0x6924d0  public: virtual void __thiscall ADisRiverKrust::EnableSoulRendering(struct IDisSoulRenderInterface::FSoulRenderParams const &)
//   0x692500  public: virtual void __thiscall ADisRiverKrust::DisableSoulRendering(void)
//   0x692510  public: virtual enum eCrossHairStatus __thiscall ADisRiverKrust::GetCrosshairStatus(class ADishonoredPawn *)const
//   0x692580  public: virtual int __thiscall ADisRiverKrust::InteractDTraceFlag(void)const
//   0x6954f0  protected: virtual void __thiscall ADisRiverKrust::SetTweaks_Derived(class UDisTweaksBase *)
//   0x695500  protected: virtual class ADisPossessablePawn * __thiscall ADisRiverKrust::GetPossessablePawn(void)
//   0x695520  protected: virtual void __thiscall ADisRiverKrust::TickPossessionOnPossessable(void)
//   0x695580  public: virtual void __thiscall ADisRiverKrust::PostConstructed(void)
//   0x695590  public: virtual unsigned int __thiscall ADisRiverKrust::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x695640  public: virtual void __thiscall ADisRiverKrust::DoKismetAttachment(class AActor *, struct FAttachmentInfos)
//   0x695880  public: virtual unsigned int __thiscall ADisRiverKrust::CanInteract(struct FCanInteractParams const &)const
//   0x6958b0  public: virtual void __thiscall ADisRiverKrust::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x6958f0  public: virtual class AActor * __thiscall ADisRiverKrust::GetHighlightActor(void)const
//   0x699930  protected: virtual void __thiscall ADisRiverKrust::TermPossessionOnPossessable(void)
//   0x699a00  public: void __thiscall ADisRiverKrust::VisionStatusChanged(unsigned int, class AActor *)
//   0x699b40  public: void __thiscall ADisRiverKrust::VisionLOSUpdated(unsigned int, class AActor *, class FVector)
//   0x699d70  public: virtual void __thiscall ADisRiverKrust::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x69dde0  private: void __thiscall ADisRiverKrust::FireDyingKismetEvents(void)
//   0x6a0120  public: virtual void __thiscall ADisRiverKrust::OnActorTerminated(void)
//   0x6a0280  public: virtual unsigned int __thiscall ADisRiverKrust::AttemptInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x6a1d60  public: virtual void __thiscall ADisRiverKrust::GetSpringRazorTriggerTestPoints(struct TMemStackArray<class FVector> &)const
//   0x6aa9c0  protected: virtual void __thiscall ADisRiverKrust::ApplyTweakChanges_Derived(void)
//   0x6aaac0  protected: virtual class UDisTweaks_Possessable * __thiscall ADisRiverKrust::GetPossessableTweaks_Derived(void)const
//   0x6aaaf0  public: virtual class UDisTweaks_Faction * __thiscall ADisRiverKrust::GetFactionTweak(void)const
//   0x6aab20  public: virtual unsigned int __thiscall ADisRiverKrust::OnWindblast(struct AActor::WindBlastParams const &, float &)
//   0x6aad60  public: class UDisTweaks_Vision const * __thiscall ADisRiverKrust::GetVisionTweaks(void)const
//   0x6aadc0  private: void __thiscall ADisRiverKrust::ExitState(enum ERiverKrustState)
//   0x6aaeb0  private: void __thiscall ADisRiverKrust::EnterAnimState(enum ERiverKrustAnimState)
//   0x6aaf40  private: void __thiscall ADisRiverKrust::CalculateProjectileLaunchLocationAndRotation(class FVector &, class FRotator &)const
//   0x6aafe0  private: void __thiscall ADisRiverKrust::CalculateVisionLocationAndRotation(class FVector &, class FRotator &)const
//   0x6ab0a0  private: void __thiscall ADisRiverKrust::CreateBodyParts(void)
//   0x6ab400  private: void __thiscall ADisRiverKrust::CreateLoot(void)
//   0x6ab4b0  private: void __thiscall ADisRiverKrust::OnLooted(void)
//   0x6ab570  public: virtual class UDisTweaks_Vision const * __thiscall ADisRiverKrust::GetDrawVisionTweaks(void)const
//   0x6ab5b0  public: virtual unsigned int __thiscall ADisRiverKrust::GetDrawVisionLocationAndRotation(class FVector &, class FRotator &)const
//   0x6ab5d0  public: virtual unsigned int __thiscall ADisRiverKrust::FireProjectile(void)
//   0x6abba0  public: virtual unsigned int __thiscall ADisRiverKrust::DoesActorCauseHitRebound(int, class UClass *)const
//   0x6abc40  public: virtual unsigned int __thiscall ADisRiverKrust::IsCapableOfTriggeringSpringRazors(void)const
//   0x6abcb0  private: int __thiscall ADisRiverKrust::GetModifiedIncomingDamage(int, class UClass *)const
//   0x6abe00  private: void __thiscall ADisRiverKrust::InitRagdoll(void)
//   0x6abfa0  private: void __thiscall ADisRiverKrust::FireInstantRagdollAkEvents(void)
//   0x6ac1f0  public: void __thiscall ADisRiverKrust::ScriptedSpitAtTarget(class AActor * const)
//   0x6ac270  public: virtual void __thiscall ADisRiverKrust::OnRiverKrustSpitAtTarget(class UDisSeqAct_RiverKrustSpitAtTarget *)
//   0x6ac290  public: virtual class UClass * __thiscall ADisRiverKrust::GetContactTypeOverride(void)const
//   0x6ac320  public: virtual class UClass * __thiscall ADisRiverKrust::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x6ac3b0  public: virtual class UDisTweaks_InteractableInterface const * __thiscall ADisRiverKrust::GetInteractableTweaks_Derived(void)const
//   0x6ac7c0  public: static class UClass * __cdecl ADisRiverKrust::GetPrivateStaticClassADisRiverKrust(wchar_t const *)
//   0x6ac850  public: virtual void __thiscall ADisRiverKrust::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6acdc0  private: void __thiscall ADisRiverKrust::ChangeAnimState(enum ERiverKrustAnimState)
//   0x6acf00  public: virtual void __thiscall ADisRiverKrust::TakeDamage_Native(int &, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x6ad8d0  public: static class UClass * __cdecl ADisRiverKrust::StaticClassNoInline(void)
//   0x6ad900  private: void __thiscall ADisRiverKrust::EnterState(enum ERiverKrustState)
//   0x6adcb0  private: enum ERiverKrustState const __thiscall ADisRiverKrust::UpdateState(enum ERiverKrustState)
//   0x6adf40  private: void __thiscall ADisRiverKrust::ChangeState(enum ERiverKrustState)
//   0x6adf70  public: virtual void __thiscall ADisRiverKrust::OnAnimEnd(class UAnimNodeSequence *, float, float)
//   0x6adff0  public: void __thiscall ADisRiverKrust::StartRagdolling(void)
//   0x6ae240  protected: virtual void __thiscall ADisRiverKrust::InitPossessionOnPossessable(void)
//   0x6ae320  public: unsigned int __thiscall ADisRiverKrust::ShouldPermanentlyIgnoreActor(class AActor const * const)const
//   0x6ae380  public: virtual void __thiscall ADisRiverKrust::PostGameLoad(enum ESaveLoadLocation)
//   0x6ae3e0  public: virtual void __thiscall ADisRiverKrust::PostBeginPlay(void)
//   0x6ae4d0  public: virtual unsigned int __thiscall ADisRiverKrust::Tick(float, enum ELevelTick)
//   0x6aef90  public: virtual void __thiscall ADisRiverKrust::OnShreddedBySpringRazor(class AActor * const, class FVector const &, class FVector const &, class UClass * const, float, float, int, int)
