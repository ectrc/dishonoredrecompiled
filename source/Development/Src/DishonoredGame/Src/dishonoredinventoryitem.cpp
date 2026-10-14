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

// Agent GF (PHASE19 GF): ShouldItemTick (2012 0x853da0) and OnCameraUpdate (0x859da0). The rest of this unit stays a skeleton; the
// item contexts (UDisItemContext, slot 77 of its vtable) and UpdateMeleeExtents (dishonoredinventoryitem_collision.cpp) have no bodies.

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gjequipdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGF_InventoryItem()
{
}

// the tweaks' tick mode: 0 when equipped (a usage), 1 when socketed, 2 when both, 3 always, 4 never
UBOOL UDishonoredInventoryItem::ShouldItemTick() const
{
	const UDisTweaks_InventoryItem* Tweaks = Cast<UDisTweaks_InventoryItem>( GetTweaks() );
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_InventoryItem*)UDisTweaks_InventoryItem::StaticClass()->GetDefaultObject();
	}
	switch( Tweaks ? Tweaks->m_ItemTickMode : 4 )
	{
	case 0: return m_EquipUsage != 0;
	case 1: return m_CurSocket != 0;
	case 2: return m_CurSocket != 0 && m_EquipUsage != 0;
	case 3: return TRUE;
	default: return FALSE;
	}
}

void UDishonoredInventoryItem::OnCameraUpdate( const FDishonoredViewTarget& _rVT )
{
	OnCameraUpdate_Derived( _rVT );
}

/*-----------------------------------------------------------------------------
	Agent GJ (PHASE20 GJ): the item's half of the equip path (rvas in CppText/UDishonoredInventoryItem.h, decompiles in
	build/agentGJ/decomp2012 and decomp2013). The player's half of the item (zoom, the anim-state pickers, the pawn sync)
	stays named: no item zooms or plays an equip animation on this route.
-----------------------------------------------------------------------------*/

// retail's IDisTweaksInterface::GetTweaks<UDisTweaks_InventoryItem> (2012 0x6b5fa0): the item's own tweaks, the class default
// object of UDisTweaks_InventoryItem when it has none
UDisTweaks_InventoryItem* UDishonoredInventoryItem::GetItemTweaks() const
{
	UDisTweaks_InventoryItem* Tweaks = Cast<UDisTweaks_InventoryItem>( GetTweaks() );
	return Tweaks ? Tweaks : UDisTweaks_InventoryItem::StaticClass()->GetDefaultObject<UDisTweaks_InventoryItem>();
}

// 2012 0x8580f0 / 2013 0x80f9f0 (208 / 196 B, same): the tweaks set and applied when they change (ApplyTweakChanges ->
// ApplyTweakChanges_Derived builds the meshes), the attributes constructed from m_pAttributeTweak[1], OnDifficultyChange
// registered to DisGameEventType_DifficultyChange, both meshes told their item, no owning inventory yet, m_bNeedToTick
void UDishonoredInventoryItem::InitItem( UDisTweaks_InventoryItem* _pItemTweaks )
{
	if( GetTweaks() != _pItemTweaks )
	{
		SetTweaks_Derived( _pItemTweaks );
		ApplyTweakChanges();
	}
	const UDisTweaks_InventoryItem* Tweaks = GetItemTweaks();
	m_pAttributes = Tweaks->m_pAttributeTweak[1] ? Tweaks->m_pAttributeTweak[1]->ConstructAttributes( this ) : NULL;
	FArkGameEventDispatcher::GetInstance()->RegisterToEvent( (INT)DisGameEventType_DifficultyChange, this, &UDishonoredInventoryItem::OnDifficultyChange );
	if( m_pPlayerMesh )
	{
		m_pPlayerMesh->SetInventoryItem( this );
	}
	if( m_pMesh )
	{
		m_pMesh->SetInventoryItem( this );
	}
	m_pOwningInventory = NULL;
	m_bNeedToTick = ShouldItemTick() ? 1 : 0;
}

// 2012 0x852db0 (the difficulty handler re-applies the attribute tweaks; UDisAttributes has no difficulty pass here)
void UDishonoredInventoryItem::OnDifficultyChange( const FArkGameEvent& _rEvent )
{
	GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::OnDifficultyChange (2012 0x852db0): the attribute re-application on a difficulty change is not ported") );
}

// 2012 0x858390 (711 B; 2013 unnamed, slot 80): the owning inventory, every melee extent cache's context list emptied, then for a
// player owner the player-specific tweaks (the pawn item anim sets added to the player mesh when the pawn has one, the player item
// mesh's light environment and shadow parent from the pawn, the five player context slots) and for an NPC the NPC-specific
// tweaks (the NPC mesh's light environment and shadow parent, the five NPC slots); the unattached pawn sync cleared
void UDishonoredInventoryItem::OnAddToInventory( UDishonoredInventory* _pInventory )
{
	m_pOwningInventory = _pInventory;
	const UDisTweaks_InventoryItem* Tweaks = GetItemTweaks();
	for( INT Idx = 0; Idx < m_MeleeExtentCache.Num(); Idx++ )
	{
		m_MeleeExtentCache(Idx).m_MeleeExtentItemContexts.Empty();
	}
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	if( Owner )
	{
		ADishonoredPlayerPawn* Player = Cast<ADishonoredPlayerPawn>( Owner );
		if( Player )
		{
			const UDisTweaks_InventoryItem_PawnSpecific* PlayerTweaks = Tweaks->m_pPlayerSpecificTweaks;
			if( PlayerTweaks )
			{
				if( Player->Mesh )
				{
					for( INT Idx = 0; Idx < PlayerTweaks->m_PawnItemAnimSets.Num(); Idx++ )
					{
						Player->Mesh->AnimSets.AddUniqueItem( PlayerTweaks->m_PawnItemAnimSets(Idx) );
					}
				}
				if( m_pPlayerMesh && Player->Mesh )
				{
					m_pPlayerMesh->SetLightEnvironment( Player->m_pLightEnvironment );
					m_pPlayerMesh->SetShadowParent( Player->Mesh );
				}
				InitItemContextSlot( PlayerTweaks, m_ContextSlots_Primary_Player, PlayerTweaks->m_ContextSlots_Primary, eDisItemContextSlot_PrimaryUse );
				InitItemContextSlot( PlayerTweaks, m_ContextSlots_Alternate_Player, PlayerTweaks->m_ContextSlots_Bash, eDisItemContextSlot_AlternateUse );
				InitItemContextSlot( PlayerTweaks, m_ContextSlots_Parry_Player, PlayerTweaks->m_ContextSlots_Parry, eDisItemContextSlot_Parry );
				InitItemContextSlot( PlayerTweaks, m_ContextSlots_Reload_Player, PlayerTweaks->m_ContextSlots_Reload, eDisItemContextSlot_Reload );
				InitItemContextSlot( PlayerTweaks, m_ContextSlots_Direct_Player, PlayerTweaks->m_ContextSlots_Direct, eDisItemContextSlot_Direct );
			}
		}
		else
		{
			const UDisTweaks_InventoryItem_PawnSpecific* NPCTweaks = Tweaks->m_pNPCSpecificTweaks;
			if( m_pMesh && Owner->Mesh )
			{
				m_pMesh->SetLightEnvironment( Owner->m_pLightEnvironment );
				m_pMesh->SetShadowParent( Owner->Mesh );
			}
			if( NPCTweaks )
			{
				InitItemContextSlot( NPCTweaks, m_ContextSlots_Primary_NPC, NPCTweaks->m_ContextSlots_Primary, eDisItemContextSlot_PrimaryUse );
				InitItemContextSlot( NPCTweaks, m_ContextSlots_Alternate_NPC, NPCTweaks->m_ContextSlots_Bash, eDisItemContextSlot_AlternateUse );
				InitItemContextSlot( NPCTweaks, m_ContextSlots_Parry_NPC, NPCTweaks->m_ContextSlots_Parry, eDisItemContextSlot_Parry );
				InitItemContextSlot( NPCTweaks, m_ContextSlots_Reload_NPC, NPCTweaks->m_ContextSlots_Reload, eDisItemContextSlot_Reload );
				InitItemContextSlot( NPCTweaks, m_ContextSlots_Direct_NPC, NPCTweaks->m_ContextSlots_Direct, eDisItemContextSlot_Direct );
			}
		}
	}
	m_UnattachedPawnSync.m_StateName = NAME_None;
}

// 2012 0x8581c0 (365 B): the slot array sized to the tweaks' list; each missing entry gets a context constructed from the context
// tweak's spawned class inside the item, its slot and tweaks written, then InitContext (UDisItemContext slot 75 - the contexts
// have no bodies here, so the call is named once and the context stays Idle). GetItemContextTweak 0x8703e0 -> 0x86d590 /
// 2013 0x80d480 and FindContextTweakByName 0x8657b0 / 0x806d00 are inlined here: the info's own tweak, or the one named by
// m_RefOfContextWithName in the Primary, Bash, Direct, Parry, Reload lists in that order.
void UDishonoredInventoryItem::InitItemContextSlot( const UDisTweaks_InventoryItem_PawnSpecific* _pPawnSpecificTweaks, TArrayNoInit<UDisItemContext*>& _rSlot, const TArrayNoInit<FDisItemContextInfo>& _rInfo, BYTE _SlotEnum )
{
	_rSlot.Empty( _rInfo.Num() );
	_rSlot.AddZeroed( _rInfo.Num() );
	for( INT Idx = 0; Idx < _rInfo.Num(); Idx++ )
	{
		const FDisItemContextInfo& Info = _rInfo(Idx);
		if( !_rSlot(Idx) )
		{
			UDisTweaks_ItemContext* ContextTweak = Info.m_pItemContext;
			if( Info.m_RefOfContextWithName != NAME_None )
			{
				const TArrayNoInit<FDisItemContextInfo>* Lists[] = { &_pPawnSpecificTweaks->m_ContextSlots_Primary, &_pPawnSpecificTweaks->m_ContextSlots_Bash, &_pPawnSpecificTweaks->m_ContextSlots_Direct, &_pPawnSpecificTweaks->m_ContextSlots_Parry, &_pPawnSpecificTweaks->m_ContextSlots_Reload };
				ContextTweak = NULL;
				for( INT List = 0; List < ARRAY_COUNT( Lists ) && !ContextTweak; List++ )
				{
					for( INT Entry = 0; Entry < Lists[List]->Num(); Entry++ )
					{
						if( (*Lists[List])(Entry).m_ContextName == Info.m_RefOfContextWithName )
						{
							ContextTweak = (*Lists[List])(Entry).m_pItemContext;
							break;
						}
					}
				}
			}
			UClass* ContextClass = ContextTweak ? ContextTweak->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame ) : NULL;
			if( !ContextClass || ( ContextClass->ClassFlags & CLASS_Abstract ) )
			{
				continue;
			}
			UDisItemContext* Context = ConstructObject<UDisItemContext>( ContextClass, this );
			_rSlot(Idx) = Context;
			Context->m_OwnerContextSlot = _SlotEnum;
			Context->m_pContextTweaks = ContextTweak;
		}
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::InitItemContextSlot: UDisItemContext::InitContext (2012 0x861ee0, slot 75) is not ported; the contexts are built and stay Idle") );
	}
}

// 2012 0x856240 / 2013 0x80feb0 (303 B, same): the usage, m_bNeedToTick, the mesh hidden when the tweaks say
// m_bHideWhenEquipped, for a player owner the stance (ModifyPlayerStance over the hand's current stance -> m_DesiredPlayerStance
// and SetPlayerStance) and, unless temporary, the controller's OnEquipmentChange with the item's UI selection (GFx, not ported);
// the post-update tick component attached to the owner, then the derived hook
void UDishonoredInventoryItem::OnEquipItem( BYTE _EquipUsage, UBOOL _bTemporary )
{
	m_EquipUsage = _EquipUsage;
	m_bNeedToTick = ShouldItemTick() ? 1 : 0;
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	if( GetItemTweaks()->m_bHideWhenEquipped )
	{
		UDishonoredItemSkeletalComponent* ItemMesh = Owner && Owner->IsA( ADishonoredPlayerPawn::StaticClass() ) ? m_pPlayerMesh : m_pMesh;
		if( ItemMesh )
		{
			ItemMesh->SetHiddenGame( TRUE );
		}
	}
	ADishonoredPlayerPawn* Player = Cast<ADishonoredPlayerPawn>( Owner );
	if( Player )
	{
		BYTE Stance = Player->m_PlayerStance[m_EquipUsage];
		if( ModifyPlayerStance( Stance ) )
		{
			Player->m_DesiredPlayerStance[m_EquipUsage] = Stance;
			Player->SetPlayerStance( m_EquipUsage, Stance );
		}
		if( !_bTemporary && m_eUISelection != 0 )
		{
			GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::OnEquipItem: ADishonoredPlayerController::OnEquipmentChange (2012 0x6d12d0) is GFx and not ported") );
		}
	}
	if( Owner && GetPostUpdateTickComponent() )
	{
		Owner->AttachComponent( GetPostUpdateTickComponent() );
	}
	OnEquipItem_Derived();
}

// 2012 0x859e40 / 2013 0x816ed0 (75 B, same)
void UDishonoredInventoryItem::OnUnEquipItem()
{
	m_EquipUsage = EDisEquipUsage_None;
	m_bNeedToTick = ShouldItemTick() ? 1 : 0;
	StopZoom();
	CancelActiveContexts();
	UActorComponent* TickComponent = GetPostUpdateTickComponent();
	if( TickComponent && TickComponent->GetOwner() )
	{
		TickComponent->GetOwner()->DetachComponent( TickComponent );
	}
	OnUnEquipItem_Derived();
}

// 2012 0x853cb0 / 2013 0x80c680 (225 B, same): the tweaks' required usage must match (or be none); a player may equip it when
// m_bIsEquipable_Player and either the hand is the primary, the left hand is not disabled or the item is the empty hand; any
// other pawn needs m_bIsEquipable_NPC; then the derived answer
UBOOL UDishonoredInventoryItem::IsEquippable( BYTE _EquipUsage, const ADishonoredPawn* _pPotentialOwner ) const
{
	const UDisTweaks_InventoryItem* Tweaks = GetItemTweaks();
	if( Tweaks->m_RequiredEquipUsage != 0 && Tweaks->m_RequiredEquipUsage != _EquipUsage )
	{
		return FALSE;
	}
	const ADishonoredPlayerPawn* Player = ConstCast<ADishonoredPlayerPawn>( _pPotentialOwner );
	if( Player )
	{
		const UBOOL bHandAllowed = _EquipUsage == EDisEquipUsage_Primary || !Player->IsLeftHandDisabled() || GetClass() == UDishonoredItemEmpty::StaticClass();
		if( !Tweaks->m_bIsEquipable_Player || !bHandAllowed )
		{
			return FALSE;
		}
	}
	else if( !Tweaks->m_bIsEquipable_NPC )
	{
		return FALSE;
	}
	return IsEquippable_Derived( _pPotentialOwner );
}

// 2012 0x85cc80 / 2013 0x819ff0 (890 B, same): the melee extent caches rebuilt from the tweaks' m_MeleeCollisionInfo (one per entry,
// its tweak remembered); the NPC mesh takes the NPC-specific item anim sets, physics asset and skeletal mesh (through the setters
// when attached in game, straight into the fields otherwise), loses its anim tree and plays a transient UAnimNodeSequence when it
// has anim sets or is forced to the ref pose (and skips updates) when it has none; the player mesh takes the player-specific
// anim sets, physics asset, skeletal mesh and anim tree template the same two ways. Retail's AddExtAnimSetUnique (0x33a0c0) and
// SetAnimationsSeq (0x330b50) are its engine's; the reference engine's AnimSets.AddUniqueItem and the Animations root stand for them.
void UDishonoredInventoryItem::ApplyTweakChanges_Derived()
{
	const UDisTweaks_InventoryItem* Tweaks = GetItemTweaks();
	m_MeleeExtentCache.Empty( Tweaks->m_MeleeCollisionInfo.Num() );
	m_MeleeExtentCache.AddZeroed( Tweaks->m_MeleeCollisionInfo.Num() );
	for( INT Idx = 0; Idx < Tweaks->m_MeleeCollisionInfo.Num(); Idx++ )
	{
		m_MeleeExtentCache(Idx).m_pMeleeExtentTweak = (FPointer)&Tweaks->m_MeleeCollisionInfo(Idx);
	}
	if( m_pMesh )
	{
		const UDisTweaks_InventoryItem_PawnSpecific* NPCTweaks = Tweaks->m_pNPCSpecificTweaks;
		const INT NumAnimSets = NPCTweaks ? NPCTweaks->m_ItemAnimSets.Num() : 0;
		for( INT Idx = 0; Idx < NumAnimSets; Idx++ )
		{
			m_pMesh->AnimSets.AddUniqueItem( NPCTweaks->m_ItemAnimSets(Idx) );
		}
		if( GIsGame && m_pMesh->IsAttached() )
		{
			m_pMesh->SetPhysicsAsset( NPCTweaks ? NPCTweaks->m_pEquippedPhysicsAsset : NULL );
			m_pMesh->SetSkeletalMesh( NPCTweaks ? NPCTweaks->m_pEquippedMesh : NULL );
		}
		else
		{
			m_pMesh->PhysicsAsset = NPCTweaks ? NPCTweaks->m_pEquippedPhysicsAsset : NULL;
			m_pMesh->SkeletalMesh = NPCTweaks ? NPCTweaks->m_pEquippedMesh : NULL;
		}
		m_pMesh->DeleteAnimTree();
		if( NumAnimSets > 0 )
		{
			UAnimNodeSequence* Sequence = ConstructObject<UAnimNodeSequence>( UAnimNodeSequence::StaticClass(), UObject::GetTransientPackage() );
			m_pMesh->Animations = Sequence;
			m_pMesh->InitAnimTree( TRUE );
		}
		else
		{
			m_pMesh->SetForceRefPose( TRUE );
			m_pMesh->m_bSkipUpdate = TRUE;
		}
	}
	if( m_pPlayerMesh )
	{
		const UDisTweaks_InventoryItem_PawnSpecific* PlayerTweaks = Tweaks->m_pPlayerSpecificTweaks;
		const INT NumAnimSets = PlayerTweaks ? PlayerTweaks->m_ItemAnimSets.Num() : 0;
		for( INT Idx = 0; Idx < NumAnimSets; Idx++ )
		{
			m_pPlayerMesh->AnimSets.AddUniqueItem( PlayerTweaks->m_ItemAnimSets(Idx) );
		}
		if( GIsGame && m_pPlayerMesh->IsAttached() )
		{
			m_pPlayerMesh->SetPhysicsAsset( PlayerTweaks ? PlayerTweaks->m_pEquippedPhysicsAsset : NULL );
			m_pPlayerMesh->SetSkeletalMesh( PlayerTweaks ? PlayerTweaks->m_pEquippedMesh : NULL );
			m_pPlayerMesh->SetAnimTreeTemplate( PlayerTweaks ? PlayerTweaks->m_pItemAnimTree : NULL );
		}
		else
		{
			m_pPlayerMesh->PhysicsAsset = PlayerTweaks ? PlayerTweaks->m_pEquippedPhysicsAsset : NULL;
			m_pPlayerMesh->SkeletalMesh = PlayerTweaks ? PlayerTweaks->m_pEquippedMesh : NULL;
			m_pPlayerMesh->AnimTreeTemplate = PlayerTweaks ? PlayerTweaks->m_pItemAnimTree : NULL;
		}
	}
}

// 2012 0x852e00 / 2013 0x80b990 (272 / 260 B): the attributes tick; for a player owner the item aim eases towards
// GetItemAimForAnims_Derived at the tweaks' m_fItemAimSpeed per second; then the derived tick
void UDishonoredInventoryItem::TickInventoryItem( FLOAT _fDeltaTime )
{
	if( m_pAttributes )
	{
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::TickInventoryItem: UDisAttributes::TickAttributes (2012 0x8ef260) is not ported") );
	}
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	if( Owner && Owner->IsA( ADishonoredPlayerPawn::StaticClass() ) )
	{
		const FVector2D TargetAim = GetItemAimForAnims_Derived();
		const FLOAT Alpha = Clamp<FLOAT>( GWorld->GetDeltaSeconds() * GetItemTweaks()->m_fItemAimSpeed, 0.f, 1.f );
		m_CurItemAim.X += ( TargetAim.X - m_CurItemAim.X ) * Alpha;
		m_CurItemAim.Y += ( TargetAim.Y - m_CurItemAim.Y ) * Alpha;
	}
	TickInventoryItem_Derived( _fDeltaTime );
}

// 2012 0x854070 (126 B): for a player controller the yaw the view leads the pawn by, in degrees, as X; nothing otherwise
const FVector2D UDishonoredInventoryItem::GetItemAimForAnims_Derived() const
{
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	APlayerController* PC = Owner ? Cast<APlayerController>( Owner->Controller ) : NULL;
	if( !PC )
	{
		return FVector2D( 0.f, 0.f );
	}
	INT YawDelta = ( PC->Rotation.Yaw - Owner->Rotation.Yaw ) & 0xFFFF;
	if( YawDelta > 0x7FFF )
	{
		YawDelta -= 0x10000;
	}
	return FVector2D( (FLOAT)YawDelta * ( 360.f / 65536.f ), 0.f );
}

// 2012 0x849c30 / 2013 0x7fecd0 (71 / 78 B): a physically equipped item answers through its derived hook; one that is not forces
// NotReady and says so when that changed the stance
UBOOL UDishonoredInventoryItem::ModifyPlayerStance( BYTE& _rStance ) const
{
	if( ItemIsPhysicallyEquipped() )
	{
		return ModifyPlayerStance_Derived( _rStance );
	}
	if( _rStance == eDisPlayerStance_NotReady )
	{
		return FALSE;
	}
	_rStance = eDisPlayerStance_NotReady;
	return TRUE;
}

// 2012 0x84dbb0 / 2013 0x801bb0 (240 / 242 B): the player or NPC list of the slot, by the owner's class
TArrayNoInit<UDisItemContext*>* UDishonoredInventoryItem::GetContextsForSlot( BYTE _ContextSlot )
{
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	const UBOOL bPlayer = Owner && Owner->IsA( ADishonoredPlayerPawn::StaticClass() );
	switch( _ContextSlot )
	{
	case eDisItemContextSlot_PrimaryUse: return bPlayer ? &m_ContextSlots_Primary_Player : &m_ContextSlots_Primary_NPC;
	case eDisItemContextSlot_AlternateUse: return bPlayer ? &m_ContextSlots_Alternate_Player : &m_ContextSlots_Alternate_NPC;
	case eDisItemContextSlot_Parry: return bPlayer ? &m_ContextSlots_Parry_Player : &m_ContextSlots_Parry_NPC;
	case eDisItemContextSlot_Reload: return bPlayer ? &m_ContextSlots_Reload_Player : &m_ContextSlots_Reload_NPC;
	default: return bPlayer ? &m_ContextSlots_Direct_Player : &m_ContextSlots_Direct_NPC;
	}
}

// 2012 0x84dcc0 (25 B): a context InProgress whose owner is this item
UBOOL UDishonoredInventoryItem::IsItemContextActive( const UDisItemContext* _pContext ) const
{
	return _pContext && _pContext->m_ContextStatus == eDisItemContextStatus_InProgress && _pContext->m_pOwner == this;
}

// 2012 0x859a70 / 2013 0x816d10 (121 B, same): the first active context of any slot (TDisActiveItemContextIterator<UDisItemContext>)
UBOOL UDishonoredInventoryItem::HasActiveContexts() const
{
	for( INT Slot = 0; Slot < eDisItemContextSlot_MAX; Slot++ )
	{
		const TArrayNoInit<UDisItemContext*>* Contexts = const_cast<UDishonoredInventoryItem*>( this )->GetContextsForSlot( Slot );
		for( INT Idx = 0; Contexts && Idx < Contexts->Num(); Idx++ )
		{
			if( IsItemContextActive( (*Contexts)(Idx) ) )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

// 2012 0x859af0 (131 B) / 2013 0x816d90 (138 B): every active context of every slot is told to cancel (UDisItemContext slot 73
// CancelContext in 2012, slot 74 with the forced flag in 2013) and the answer is whether any did. The contexts have no bodies
// here: an active one is counted and named once. This is what EnterSoireeMode calls on the equipped items - a cancel of the
// swing in progress, not a hide: the hiding is MAT_SetForceInventory's empty hands.
UBOOL UDishonoredInventoryItem::CancelActiveContexts( UBOOL _bForced )
{
	UBOOL bCanceled = FALSE;
	for( INT Slot = 0; Slot < eDisItemContextSlot_MAX; Slot++ )
	{
		TArrayNoInit<UDisItemContext*>* Contexts = GetContextsForSlot( Slot );
		for( INT Idx = 0; Contexts && Idx < Contexts->Num(); Idx++ )
		{
			if( IsItemContextActive( (*Contexts)(Idx) ) )
			{
				GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::CancelActiveContexts: UDisItemContext::CancelContext has no body; the active context is left as it is") );
				bCanceled = TRUE;
			}
		}
	}
	return bCanceled;
}

// 2012 0x856370 (264 B): nothing for an item that is not zoomed; the zoomed half (the camera's FOV target and aim influence, the
// zoom-out sound, the listener override, the controller's target tracking, OnZoomOut, the post-process effect) is not ported -
// no item zooms on this route
void UDishonoredInventoryItem::StopZoom()
{
	if( m_ZoomLevel == 0 )
	{
		return;
	}
	GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::StopZoom: an item was zoomed; the zoom-out (camera FOV target, aim influence, tracking) is not ported") );
	m_ZoomLevel = 0;
}

// 2012 0x84dce0 (85 B): the player mesh for a player owner, the NPC mesh otherwise
USkeletalMeshComponent* UDishonoredInventoryItem::AnimState_GetSkeletalComponent()
{
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	return Owner && Owner->IsA( ADishonoredPlayerPawn::StaticClass() ) ? m_pPlayerMesh : m_pMesh;
}

// 2012 0x84dd30 (73 B): the player anim-state component for a player owner, none otherwise
UDisAnimStateComponent* UDishonoredInventoryItem::AnimState_GetAnimStateComp()
{
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	return Owner && Owner->IsA( ADishonoredPlayerPawn::StaticClass() ) ? m_pAnimStateCompPlayer : NULL;
}

// 2012 0x85c030 / 2013 0x8196a0 (198 B, same): a player owner's item with an anim-state component and no pickers yet binds the
// component to itself and finds the pickers on the player mesh; a pending pawn sync is replayed. Both halves are the anim-state
// system (UDisAnimStateComponent::BindToAnimStateOwner 0x6b1ab0, FindAnimStatePickers 0x861370, SyncPawnAnimState 0x85ad40),
// which has no bodies here - named once each.
void UDishonoredInventoryItem::OnAttached()
{
	ADishonoredPawn* Owner = m_pOwningInventory ? m_pOwningInventory->m_pOwner : NULL;
	if( !Owner )
	{
		return;
	}
	if( m_pAnimStateCompPlayer && Owner->IsA( ADishonoredPlayerPawn::StaticClass() ) && m_AnimStatePickerInfoPlayer.Num() == 0 )
	{
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::OnAttached: UDisAnimStateComponent::BindToAnimStateOwner and FindAnimStatePickers are not ported; the item plays no anim states") );
	}
	if( m_UnattachedPawnSync.m_StateName != NAME_None )
	{
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::OnAttached: SyncPawnAnimState (2012 0x85ad40) is not ported; the pending pawn sync is dropped") );
		m_UnattachedPawnSync.m_StateName = NAME_None;
	}
}

// 2012 0x849ca0 / 2013 0x7fed20 (24 B, same): the player anim-state component's state cleared (ClearAnimState 0x6c0c20, not ported)
void UDishonoredInventoryItem::OnDetached()
{
	if( m_pAnimStateCompPlayer )
	{
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::OnDetached: UDisAnimStateComponent::ClearAnimState (2012 0x6c0c20) is not ported") );
	}
}

// 2012 0x858660 / 2013 0x812110 (388 B, same): from offscreen the player's upper FSM is asked for the item-in transition (the
// sneaking / fast / ready-stance variants of UDisTweaks_PlayerPawn's action table) or, when the hand's desired stance differs from
// its actual one, for the ready-stance change - the two action states (UStatePlayerTransitionItemIn, UStatePlayerChangeReadyStance)
// are skeletons here, so that branch is named and answers TransitionFailure; on screen the equipped item is simply shown
BYTE UDishonoredInventoryItem::PlayerTransitionItemIn_Derived( BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip )
{
	if( _bFromOffscreen )
	{
		GJDIAG_ONCE( TEXT("UDishonoredInventoryItem::PlayerTransitionItemIn_Derived: the item-in action (UStatePlayerTransitionItemIn / UStatePlayerChangeReadyStance through RequestAction) is not ported; the item appears without its draw animation") );
		return eDisTransitionItemResult_TransitionFailure;
	}
	UDishonoredInventoryItem* Equipped = m_pOwningInventory ? m_pOwningInventory->GetEquippedItem( m_EquipUsage ) : NULL;
	if( Equipped && m_pOwningInventory )
	{
		m_pOwningInventory->ShowEquippedItem( TRUE, m_EquipUsage );
	}
	return eDisTransitionItemResult_TransitionSuccess;
}
