// DishonoredGame/src/dishonoredinventoryitem.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (81):
//   0x849be0  public: static void __cdecl UDishonoredInventoryItem::InitializePrivateStaticClassUDishonoredInventoryItem(void)
//   0x849c00  public: unsigned int __thiscall UDishonoredInventoryItem::IsValid_InventoryItem(void)const
//   0x849c20  public: class ADishonoredPawn const * __thiscall UDishonoredInventoryItem::GetOwningPawn(void)const
//   0x849c30  public: unsigned int __thiscall UDishonoredInventoryItem::ModifyPlayerStance(enum eDisPlayerStance &)const
//   0x849c80  protected: virtual unsigned int __thiscall UDishonoredInventoryItem::ModifyPlayerStance_Derived(enum eDisPlayerStance &)const
//   0x849ca0  public: virtual void __thiscall UDishonoredInventoryItem::OnDetached(void)
//   0x849cc0  public: unsigned int __thiscall UDishonoredInventoryItem::IsItemZoomed(void)const
//   0x849cd0  public: virtual void __thiscall UDishonoredInventoryItem::TakeDamage(int &, class AController *, class FVector, class FVector &, class UClass * &, struct FTraceHitInfo, class AActor *)
//   0x849ce0  public: virtual unsigned int __thiscall FDisItemContextParam_None::CanDoContext(class UDisItemContext const *)const
//   0x849d00  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_None::DoContext(class UDisItemContext *)const
//   0x849d20  public: virtual unsigned int __thiscall FDisItemContextParam_UsePlayer::CanDoContext(class UDisItemContext const *)const
//   0x849d40  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_UsePlayer::DoContext(class UDisItemContext *)const
//   0x849d60  public: virtual unsigned int __thiscall FDisItemContextParam_UseNPC::CanDoContext(class UDisItemContext const *)const
//   0x849d80  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_UseNPC::DoContext(class UDisItemContext *)const
//   0x849da0  public: virtual void __thiscall FDisItemContextParam_None::CanDoContext_GatherInfo(class UDisItemContext *)const
//   0x849dc0  public: virtual unsigned int __thiscall FDisItemContextParam_ParryNPC::CanDoContext(class UDisItemContext const *)const
//   0x849de0  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_ParryNPC::DoContext(class UDisItemContext *)const
//   0x849e00  public: virtual unsigned int __thiscall FDisItemContextParam_ParryPlayer::CanDoContext(class UDisItemContext const *)const
//   0x849e20  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_ParryPlayer::DoContext(class UDisItemContext *)const
//   0x849e40  public: virtual unsigned int __thiscall FDisItemContextParam_Reload::CanDoContext(class UDisItemContext const *)const
//   0x849e60  public: virtual enum eDisItemContextStatus __thiscall FDisItemContextParam_Reload::DoContext(class UDisItemContext *)const
//   0x849e80  public: struct FVector2D const __thiscall UDishonoredInventoryItem::GetItemAimForAnims(void)const
//   0x849ea0  public: virtual class FName __thiscall UDishonoredInventoryItem::GetNavAnimState(void)const
//   0x84dbb0  protected: class TArrayNoInit<class UDisItemContext *> * __thiscall UDishonoredInventoryItem::GetContextsForSlot(enum eDisItemContextSlot)
//   0x84dcc0  public: unsigned int __thiscall UDishonoredInventoryItem::IsItemContextActive(class UDisItemContext const *)const
//   0x84dce0  public: virtual class USkeletalMeshComponent * __thiscall UDishonoredInventoryItem::AnimState_GetSkeletalComponent(void)
//   0x84dd30  public: virtual class UDisAnimStateComponent * __thiscall UDishonoredInventoryItem::AnimState_GetAnimStateComp(void)
//   0x84dd80  public: virtual enum EDisCrosshairState __thiscall UDishonoredInventoryItem::GetCrosshairState(void)const
//   0x852c90  public: virtual void __thiscall UDishonoredInventoryItem::TrailNotify(class UAnimNodeSequence *)
//   0x852cf0  public: virtual void __thiscall UDishonoredInventoryItem::TrailNotifyTick(class UAnimNodeSequence *, float, float, float)
//   0x852d50  public: virtual void __thiscall UDishonoredInventoryItem::TrailNotifyEnd(class UAnimNodeSequence *, float)
//   0x852db0  protected: void __thiscall UDishonoredInventoryItem::OnDifficultyChange(class FArkGameEvent const &)
//   0x852e00  public: void __thiscall UDishonoredInventoryItem::TickInventoryItem(float)
//   0x852f10  public: void __thiscall UDishonoredInventoryItem::GetWorldLocationAndRotation(class FVector &, class FRotator &)const
//   0x853070  protected: virtual void __thiscall UDishonoredInventoryItem::GetItemVelocity(class FVector &, class FVector &)const
//   0x853190  protected: unsigned int __thiscall UDishonoredInventoryItem::SpawnDropPickup(class FVector const &, class FRotator const &, class FVector const *, class FVector const *)
//   0x853410  public: unsigned int __thiscall UDishonoredInventoryItem::IsActorInContextRange(class AActor const *, enum eDisItemContextSlot, class UClass *)const
//   0x8534b0  public: float __thiscall UDishonoredInventoryItem::GetLongestContextRange(enum eDisItemContextSlot, class UClass *)const
//   0x853560  public: class UDisItemContext const * __thiscall UDishonoredInventoryItem::GetLongestContext(enum eDisItemContextSlot, class UClass *)const
//   0x853630  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::FindItemContext(enum eDisItemContextSlot, class UClass *)
//   0x8536b0  public: class FName __thiscall UDishonoredInventoryItem::GetItemPrefix(void)const
//   0x8537e0  public: void __thiscall UDishonoredInventoryItem::OnChangeZoomLevel(unsigned int)
//   0x8538f0  protected: void __thiscall UDishonoredInventoryItem::FindAndZoomToTarget(class ADishonoredPlayerPawn *, class ADishonoredPlayerCamera *)
//   0x853cb0  public: unsigned int __thiscall UDishonoredInventoryItem::IsEquippable(enum EDisEquipUsage, class ADishonoredPawn const *)const
//   0x853da0  public: unsigned int __thiscall UDishonoredInventoryItem::ShouldItemTick(void)const
//   0x853e30  private: void __thiscall UDishonoredInventoryItem::AttachUpgrade(class USkeletalMesh *, class UAnimSet *)
//   0x853f80  public: void __thiscall UDishonoredInventoryItem::RevertUpgrade(class UDisTweaks_Upgrade const *)
//   0x854070  protected: virtual struct FVector2D const __thiscall UDishonoredInventoryItem::GetItemAimForAnims_Derived(void)const
//   0x854100  public: void __thiscall UDishonoredInventoryItem::PlayAnimOnAttachment(class FName, class USkeletalMesh const *)const
//   0x855e90  public: virtual void __thiscall UDishonoredInventoryItem::OnDrop(class FVector const &, class FVector const &)
//   0x856210  public: unsigned int __thiscall UDishonoredInventoryItem::IsActorInRange(class AActor const *, enum eDisItemContextSlot)const
//   0x856240  public: void __thiscall UDishonoredInventoryItem::OnEquipItem(enum EDisEquipUsage, unsigned int)
//   0x856370  public: void __thiscall UDishonoredInventoryItem::StopZoom(void)
//   0x856480  public: void __thiscall UDishonoredInventoryItem::Zoom(unsigned int)
//   0x856600  protected: unsigned int __thiscall UDishonoredInventoryItem::ApplyUpgrade_Internal(class UDisTweaks_Upgrade const *)
//   0x8580f0  public: virtual void __thiscall UDishonoredInventoryItem::InitItem(class UDisTweaks_InventoryItem *)
//   0x8581c0  protected: void __thiscall UDishonoredInventoryItem::InitItemContextSlot(class UDisTweaks_InventoryItem_PawnSpecific const *, class TArrayNoInit<class UDisItemContext *> &, class TArrayNoInit<struct FDisItemContextInfo> const &, enum eDisItemContextSlot)
//   0x858330  private: void __thiscall UDishonoredInventoryItem::TerminateAllContextsForSlot(enum eDisItemContextSlot)
//   0x858390  public: virtual void __thiscall UDishonoredInventoryItem::OnAddToInventory(class UDishonoredInventory *)
//   0x858660  protected: virtual enum eDisTransitionItemResult __thiscall UDishonoredInventoryItem::PlayerTransitionItemIn_Derived(enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x8587f0  public: virtual void __thiscall UDishonoredInventoryItem::BeginDestroy(void)
//   0x858830  public: unsigned int __thiscall UDishonoredInventoryItem::ApplyUpgrade(class UDisTweaks_Upgrade const *)
//   0x859a70  public: unsigned int __thiscall UDishonoredInventoryItem::HasActiveContexts(void)const
//   0x859af0  public: unsigned int __thiscall UDishonoredInventoryItem::CancelActiveContexts(void)
//   0x859b80  protected: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse(enum eDisItemContextSlot, class UClass *, struct FDisItemContextParams const *)
//   0x859da0  public: void __thiscall UDishonoredInventoryItem::OnCameraUpdate(struct FDishonoredViewTarget const &)
//   0x859e40  public: void __thiscall UDishonoredInventoryItem::OnUnEquipItem(void)
//   0x859e90  public: enum eSetAnimStateStatus __thiscall UDishonoredInventoryItem::SetAnimState(class FName const &, class UDishonoredNativeState *, float, class UObject *)
//   0x85ac80  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_PrimaryUsePlayer(struct FDisItemContextParam_UsePlayer const &)
//   0x85aca0  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_AIAttackAutoSelect(struct FDisItemContextParam_UseNPC const &, class UClass *)
//   0x85acc0  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_AlternateUsePlayer(struct FDisItemContextParam_UsePlayer const &)
//   0x85ace0  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_ParryNPC(struct FDisItemContextParam_ParryNPC const &)
//   0x85ad00  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_Reload(struct FDisItemContextParam_Reload const &)
//   0x85ad20  public: class UDisItemContext * __thiscall UDishonoredInventoryItem::AttemptUse_DirectUse(class UClass *, struct FDisItemContextParams const *)
//   0x85ad40  public: unsigned int __thiscall UDishonoredInventoryItem::SyncPawnAnimState(class FName const &, class UDishonoredNativeState *, float, class UObject *)
//   0x85ade0  public: virtual void __thiscall UDishonoredInventoryItem::OnSyncPawnAnimStateFail(class FName const &, class UDishonoredNativeState *, float, class UObject *)
//   0x85c030  public: virtual void __thiscall UDishonoredInventoryItem::OnAttached(void)
//   0x85cc80  protected: virtual void __thiscall UDishonoredInventoryItem::ApplyTweakChanges_Derived(void)
//   0x861370  public: virtual void __thiscall UDishonoredInventoryItem::FindAnimStatePickers(class UDisAnimStateComponent *, class USkeletalMeshComponent *, class TArrayNoInit<struct FDisItemAnimState_PickerInfo> &, unsigned int)
//   0x8619d0  public: static class UClass * __cdecl UDishonoredInventoryItem::GetPrivateStaticClassUDishonoredInventoryItem(wchar_t const *)
//   0x861a60  public: static class UClass * __cdecl UDishonoredInventoryItem::StaticClassNoInline(void)
