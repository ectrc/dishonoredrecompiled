// DishonoredGame/src/dishonoredpawn_inventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x7aab20  public: class UDishonoredInventoryItem * __thiscall ADishonoredPawn::GetEquippedItem(enum EDisEquipUsage)const
//   0x7aab30  public: unsigned int __thiscall ADishonoredPawn::PrimaryHandAppearEmpty(void)const
//   0x7aab60  public: void __thiscall ADishonoredPawn::ShowEquippedInventoryItem(unsigned int, enum EDisEquipUsage)
//   0x7aab80  public: virtual void __thiscall ADishonoredPawn::EquipNextItem(void)
//   0x7aaba0  public: virtual void __thiscall ADishonoredPawn::EquipPrevItem(void)
//   0x7aabc0  public: virtual unsigned int __thiscall ADishonoredPawn::DirectUseEquippedItem(enum EDisEquipUsage, class UClass *, struct FDisItemContextParams const *)
//   0x7aac00  public: virtual void __thiscall ADishonoredPawn::DropEquippedItem(enum EDisEquipUsage, class FVector const &, class FVector const &)
//   0x7aac20  public: virtual unsigned int __thiscall ADishonoredPawn::AttachEquippedItem(enum EItemSocket, enum EDisEquipUsage)
//   0x7aac60  public: unsigned int __thiscall ADishonoredPawn::IsPossessing(void)const
//   0x7aac70  public: virtual void __thiscall ADishonoredPawn::OnAddAbstractItem(class UDisSeqAct_AddAbstractItem *)
//   0x7aaca0  public: virtual void __thiscall ADishonoredPawn::OnRemoveAbstractItem(class UDisSeqAct_RemoveAbstractItem *)
//   0x7aace0  public: virtual void __thiscall ADishonoredPawn::OnGetAbstractItemQuantity(class UDisSeqAct_GetAbstractItemQuantity *)
//   0x7aad20  public: virtual void __thiscall ADishonoredPawn::OnModifyElixirCount(class UDisSeqAct_ModifyElixirCount *)
//   0x7acbb0  public: unsigned int __thiscall ADishonoredPawn::HasNothingEquipped(void)const
//   0x7acc50  public: unsigned int __thiscall ADishonoredPawn::HandsAppearEmpty(void)const
//   0x7acca0  public: virtual unsigned int __thiscall ADishonoredPawn::ReloadEquippedItem(enum EDisEquipUsage, enum eDisAmmoType)
//   0x7afac0  public: virtual void __thiscall ADishonoredPawn::Tick_Inventory(float, enum ELevelTick)
//   0x7afbe0  public: unsigned int __thiscall ADishonoredPawn::EquipItemByType_KeepTrying(class UClass *, enum EDisEquipUsage, enum ADishonoredPawn::EEquipItemFlags)
//   0x7afd10  public: virtual unsigned int __thiscall ADishonoredPawn::AddInventoryItem(class ADishonoredInventoryPickup *)
//   0x7afe30  public: virtual unsigned int __thiscall ADishonoredPawn::IsHoldingMovableObject(void)const
//   0x7afe60  public: virtual class UDisMovableComponent * __thiscall ADishonoredPawn::GetHeldMovableObject(void)
//   0x7afe90  public: void __thiscall ADishonoredPawn::SpawnInventoryLoadout_Ammo(class TArray<struct FDisInventoryAmmoEntry, class FDefaultAllocator> const &, unsigned int)
//   0x7aff20  public: virtual void __thiscall ADishonoredPawn::OnAddInventoryItem(class UDisSeqAct_AddInventoryItem *)
//   0x7b0050  public: virtual void __thiscall ADishonoredPawn::OnEquipItemType(class UDisSeqAct_EquipItemType *)
//   0x7b0100  public: virtual void __thiscall ADishonoredPawn::OnModifyAmmo(class UDisSeqAct_ModifyAmmo *)
//   0x7c38a0  public: virtual void __thiscall ADishonoredPawn::PreBeginPlay_Inventory(void)

// Agent FZ (PHASE18 FZ): HasNothingEquipped, 2012 0x7acbb0 (160 B): both hands' equipped items are UDishonoredItemEmpty
// (or absent). The rest of this unit stays a skeleton; agents EF (SpawnInventoryLoadout_Ammo) and the abstract-item five
// put their bodies in dissavegame.cpp and dishonoredpawn.cpp before this unit was compiled.

#include "DishonoredGame.h"
#include "gjequipdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PawnInventory()
{
}

UBOOL ADishonoredPawn::HasNothingEquipped() const
{
	if( !m_pInventory )
	{
		return TRUE;
	}
	UDishonoredInventoryItem* Primary = m_pInventory->GetEquippedItem( EDisEquipUsage_Primary );
	UDishonoredInventoryItem* Secondary = m_pInventory->GetEquippedItem( EDisEquipUsage_Secondary );
	const UBOOL bPrimaryEmpty = !Primary || Primary->IsA( UDishonoredItemEmpty::StaticClass() );
	const UBOOL bSecondaryEmpty = !Secondary || Secondary->IsA( UDishonoredItemEmpty::StaticClass() );
	return bPrimaryEmpty && bSecondaryEmpty;
}

/*-----------------------------------------------------------------------------
	Agent GJ (PHASE20 GJ): the pawn's half of the equip path (decompiles in build/agentGJ/decomp2012 and decomp2013).
	2012 / 2013 rvas: GetEquippedItem 0x7aab20, ShowEquippedInventoryItem 0x7aab60, AttachEquippedItem 0x7aac20 / 0x74a510,
	EquipItemByType 0x7c8500 / 0x75d400, EquipItemByType_KeepTrying 0x7afbe0 / 0x751790, PreBeginPlay_Inventory 0x7c38a0 /
	0x758860, Tick_Inventory 0x7afac0, SpawnInventoryLoadout_Items 0x7c8690 / 0x75d590, PrimaryHandAppearEmpty 0x7aab30,
	HandsAppearEmpty 0x7acc50.
-----------------------------------------------------------------------------*/

UDishonoredInventoryItem* ADishonoredPawn::GetEquippedItem( BYTE _EquipUsage ) const
{
	return m_pInventory ? m_pInventory->GetEquippedItem( _EquipUsage ) : NULL;
}

void ADishonoredPawn::ShowEquippedInventoryItem( UBOOL _bShow, BYTE _EquipUsage )
{
	if( m_pInventory )
	{
		m_pInventory->ShowEquippedItem( _bShow, _EquipUsage );
	}
}

// the hand's item's mesh is absent or hidden in game
UBOOL ADishonoredPawn::PrimaryHandAppearEmpty() const
{
	UDishonoredInventoryItem* Item = GetEquippedItem( EDisEquipUsage_Primary );
	return !Item || !Item->m_pMesh || Item->m_pMesh->HiddenGame;
}

UBOOL ADishonoredPawn::HandsAppearEmpty() const
{
	if( !PrimaryHandAppearEmpty() )
	{
		return FALSE;
	}
	UDishonoredInventoryItem* Item = GetEquippedItem( EDisEquipUsage_Secondary );
	return !Item || !Item->m_pMesh || Item->m_pMesh->HiddenGame;
}

// the base body; ADishonoredNPCPawn overrides it (2012 0x7b7950, 565 B: the matinee-driven detach check against the intended
// item, not ported - the NPCs equip nothing on this route)
UBOOL ADishonoredPawn::AttachEquippedItem( BYTE _ItemSocket, BYTE _EquipUsage )
{
	UDishonoredInventoryItem* Item = GetEquippedItem( _EquipUsage );
	return Item && m_pInventory ? m_pInventory->AttachItemToSocket( Item, _ItemSocket ) : FALSE;
}

// 2012 0x7c8500 / 2013 0x75d400 (389 / 388 B, same): an instant equip first puts the hand's current, attached item away - to the
// holstered socket on an NPC whose item tweaks name one, off the mesh otherwise; the inventory equips the type (temporary when
// flag 1); on success an instant equip attaches the new item to the equipped socket and shows it unless its tweaks say
// m_bHideWhenEquipped; on failure an instant equip puts the previous item back on the equipped socket
UBOOL ADishonoredPawn::EquipItemByType( UClass* _pType, BYTE _EquipUsage, INT _Flags )
{
	UDishonoredInventoryItem* Previous = GetEquippedItem( _EquipUsage );
	const UDisTweaks_InventoryItem* PreviousTweaks = Previous ? Previous->GetItemTweaks() : UDisTweaks_InventoryItem::StaticClass()->GetDefaultObject<UDisTweaks_InventoryItem>();
	const UBOOL bInstant = ( _Flags & EEquipItemFlags_InstantEquip ) != 0;
	const UBOOL bWasAttached = Previous && Previous->m_CurSocket != ItemSocket_None;
	if( bWasAttached && bInstant )
	{
		const UBOOL bHolster = PreviousTweaks->m_Sockets[ItemSocket_Holstered] != NAME_None && IsA( ADishonoredNPCPawn::StaticClass() );
		AttachEquippedItem( bHolster ? ItemSocket_Holstered : ItemSocket_None, _EquipUsage );
	}
	if( m_pInventory && m_pInventory->EquipItemType( _pType, _EquipUsage, ( _Flags & EEquipItemFlags_Temporary ) != 0 ) )
	{
		if( bInstant )
		{
			AttachEquippedItem( ItemSocket_Equipped, _EquipUsage );
			UDishonoredInventoryItem* Equipped = GetEquippedItem( _EquipUsage );
			if( Equipped && !Equipped->GetItemTweaks()->m_bHideWhenEquipped )
			{
				m_pInventory->ShowEquippedItem( TRUE, _EquipUsage );
			}
		}
		return TRUE;
	}
	if( bInstant && bWasAttached && !PreviousTweaks->m_bHideWhenEquipped )
	{
		AttachEquippedItem( ItemSocket_Equipped, _EquipUsage );
	}
	return FALSE;
}

// 2012 0x7afbe0 / 2013 0x751790 (292 B, same): an equippable item of the class in the inventory is asked for through EquipItemByType;
// unless the hand now holds (or is in the middle of equipping) that class the request is remembered in m_DesiredEquippedItems and
// Tick_Inventory keeps asking
UBOOL ADishonoredPawn::EquipItemByType_KeepTrying( UClass* _pItemClass, BYTE _EquipUsage, INT _Flags )
{
	if( !m_pInventory )
	{
		return FALSE;
	}
	UDishonoredInventoryItem* Item = m_pInventory->FindItemByClass( _pItemClass, FALSE, FALSE, FALSE, _EquipUsage );
	if( !Item || !Item->IsEquippable( _EquipUsage, this ) )
	{
		return FALSE;
	}
	EquipItemByType( _pItemClass, _EquipUsage, _Flags );
	if( _EquipUsage >= m_DesiredEquippedItems.Num() )
	{
		return FALSE;
	}
	FDisDeferredItemEquip& Desired = m_DesiredEquippedItems(_EquipUsage);
	const UBOOL bAlreadyDesired = Desired.m_pItemClass != NULL;
	UClass* BeingEquipped = GetItemClassBeingEquipped( _EquipUsage );
	UDishonoredInventoryItem* Equipped = GetEquippedItem( _EquipUsage );
	if( Equipped && Equipped->IsA( _pItemClass ) && ( !BeingEquipped || BeingEquipped->IsChildOf( _pItemClass ) ) )
	{
		Desired.m_pItemClass = NULL;
		return FALSE;
	}
	Desired.m_pItemClass = _pItemClass;
	Desired.m_Flags = _Flags;
	m_pInventory->SetReequipItem( _EquipUsage, _pItemClass );
	if( !bAlreadyDesired )
	{
		m_nPendingEquip++;
	}
	return TRUE;
}

// 2012 0x7c38a0 / 2013 0x758860 (254 / 294 B, same): the inventory initialised with the pawn tweaks' empty-hands item, the three
// deferred-equip entries, then the tweaks' m_InventorySlots added
void ADishonoredPawn::PreBeginPlay_Inventory()
{
	UDisTweaks_Pawn* Tweaks = GetPawnTweaks();
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_Pawn::StaticClass()->GetDefaultObject<UDisTweaks_Pawn>();
	}
	if( !m_pInventory )
	{
		GJDIAG_ONCE( TEXT("ADishonoredPawn::PreBeginPlay_Inventory: a pawn without its pInventory subobject") );
		return;
	}
	m_pInventory->InitInventory( this, Tweaks->m_pEmptyHandsItem );
	m_nPendingEquip = 0;
	m_DesiredEquippedItems.Empty( EDisEquipUsage_MAX );
	m_DesiredEquippedItems.AddZeroed( EDisEquipUsage_MAX );
	m_pInventory->m_Slots.Reserve( m_pInventory->m_Slots.Num() + Tweaks->m_InventorySlots.Num() );
	for( INT Idx = 0; Idx < Tweaks->m_InventorySlots.Num(); Idx++ )
	{
		m_pInventory->AddInventorySlot( Tweaks->m_InventorySlots(Idx).m_pRequiredClass, Tweaks->m_InventorySlots(Idx).m_RequiredUsage );
	}
}

// 2012 0x7afac0 (288 B; 2013 unnamed): the inventory's tick, then while equips are pending each hand's desired class is retried
// (the current item's contexts cancelled first) until the hand holds it or is equipping it. ADishonoredNPCPawn's override
// (0x7be2c0: the deferred armor drops) is not ported.
void ADishonoredPawn::Tick_Inventory( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	if( !m_pInventory )
	{
		return;
	}
	m_pInventory->TickInventory( _fDeltaTime );
	DisGJReportInventory( this );
	if( m_nPendingEquip <= 0 )
	{
		return;
	}
	for( INT Usage = 0; Usage < m_DesiredEquippedItems.Num(); Usage++ )
	{
		FDisDeferredItemEquip& Desired = m_DesiredEquippedItems(Usage);
		if( !Desired.m_pItemClass )
		{
			continue;
		}
		UClass* BeingEquipped = GetItemClassBeingEquipped( Usage );
		UDishonoredInventoryItem* Equipped = m_pInventory->GetEquippedItem( Usage );
		if( Equipped && Equipped->GetClass()->IsChildOf( Desired.m_pItemClass ) )
		{
			if( !BeingEquipped || BeingEquipped->IsChildOf( Desired.m_pItemClass ) )
			{
				Desired.m_pItemClass = NULL;
				m_nPendingEquip--;
			}
			continue;
		}
		if( Equipped )
		{
			Equipped->CancelActiveContexts();
		}
		EquipItemByType( Desired.m_pItemClass, Usage, Desired.m_Flags );
	}
}

// 2012 0x7c8690 / 2013 0x75d590 (205 / 214 B, same): both re-equip memories cleared, then every tweak of the list spawned into the
// inventory with its own required usage - shown at once on an NPC, left for the equip on the player
void ADishonoredPawn::SpawnInventoryLoadout_Items( const TArray<UDisTweaks_InventoryItem*>& _rLoadout )
{
	if( !m_pInventory )
	{
		return;
	}
	m_pInventory->SetReequipItem( EDisEquipUsage_Primary, NULL );
	m_pInventory->SetReequipItem( EDisEquipUsage_Secondary, NULL );
	const UBOOL bShow = IsA( ADishonoredNPCPawn::StaticClass() );
	for( INT Idx = 0; Idx < _rLoadout.Num(); Idx++ )
	{
		UDisTweaks_InventoryItem* ItemTweaks = _rLoadout(Idx);
		if( ItemTweaks )
		{
			m_pInventory->SpawnAndAddInventoryItem( ItemTweaks, FALSE, bShow, ItemTweaks->m_RequiredEquipUsage );
		}
	}
}
