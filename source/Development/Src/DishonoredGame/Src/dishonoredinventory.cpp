// DishonoredGame/src/dishonoredinventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (60):
//   0x849b70  public: static void __cdecl UDishonoredInventory::InitializePrivateStaticClassUDishonoredInventory(void)
//   0x849b90  public: void __thiscall UDishonoredInventory::DropEquippedItem(enum EDisEquipUsage, class FVector const &, class FVector const &)
//   0x851d60  public: void __thiscall UDishonoredInventory::AddInventorySlot(class UClass *, enum EDisEquipUsage)
//   0x851db0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::GetEquippedItem(enum EDisEquipUsage)const
//   0x851e10  public: enum eDisTransitionItemResult __thiscall UDishonoredInventory::PlayerTransitionItemIn(enum EDisEquipUsage, enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x851e60  protected: int __thiscall UDishonoredInventory::GetRequiredSlot(class UClass *, unsigned int, enum EDisEquipUsage, struct FPawnInventorySlot * *)
//   0x851f10  protected: int __thiscall UDishonoredInventory::GetNextEmptySlot(int, class UDishonoredInventoryItem *)
//   0x852060  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindEquippableItem(class UClass const * const, enum EDisEquipUsage)const
//   0x852180  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItemByClass(class UClass const * const, unsigned int, unsigned int, unsigned int, enum EDisEquipUsage)const
//   0x8522d0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItemBySelectionType(enum eDisUISelectionType)const
//   0x852340  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::FindItem(class UDisTweaks_InventoryItem const *)const
//   0x8523f0  public: void __thiscall UDishonoredInventory::AddElixir(enum EElixirType, int)
//   0x8524a0  public: void __thiscall UDishonoredInventory::SetElixirCount(enum EElixirType, int)
//   0x8525a0  public: unsigned int __thiscall UDishonoredInventory::ConsumeElixir(enum EElixirType)
//   0x8527c0  public: unsigned int __thiscall UDishonoredInventory::HasAmmo(enum eDisAmmoType, int)const
//   0x852820  public: unsigned int __thiscall UDishonoredInventory::ConsumeAmmo(enum eDisAmmoType, unsigned int)
//   0x852880  public: int __thiscall UDishonoredInventory::AddAmmo(enum eDisAmmoType, unsigned int)
//   0x852920  public: void __thiscall UDishonoredInventory::SetAmmo(enum eDisAmmoType, unsigned int, unsigned int)
//   0x852990  public: struct FDisAmmoInfo const * __thiscall UDishonoredInventory::GetAmmoInfo(enum eDisAmmoType)const
//   0x8529e0  public: unsigned int __thiscall UDishonoredInventory::ConsumeStatPickup(class ADisStatPickup *, int *)
//   0x852a90  public: void __thiscall UDishonoredInventory::SetReequipItem(enum EDisEquipUsage, class UClass *)
//   0x852b70  public: class UClass * __thiscall UDishonoredInventory::GetReequipItem(enum EDisEquipUsage, unsigned int)const
//   0x852bf0  public: void __thiscall UDishonoredInventory::RegisterActiveContext(class UDisItemContext *)
//   0x852c10  public: virtual unsigned int __thiscall UDishonoredInventory::FillUIInteractions(struct FDisUIInteractionContext &)const
//   0x8559f0  public: int __thiscall UDishonoredInventory::GetRequiredSlot(class UDishonoredInventoryItem *)
//   0x855a50  public: unsigned int __thiscall UDishonoredInventory::AttachItemToSocket(class UDishonoredInventoryItem * const, enum EItemSocket)
//   0x855bc0  public: unsigned int __thiscall UDishonoredInventory::HasItemByClass(class UClass const *, unsigned int)const
//   0x855be0  public: unsigned int __thiscall UDishonoredInventory::HasItem(class UDisTweaks_InventoryItem const *)const
//   0x855c00  public: void __thiscall UDishonoredInventory::DamageInventoryItems(int &, class AController *, class FVector, class FVector &, class UClass * &, struct FTraceHitInfo, class AActor *)
//   0x855ce0  public: void __thiscall UDishonoredInventory::AddAbstractItem(class UDisAbstractItem *, int, unsigned int)
//   0x855d70  public: void __thiscall UDishonoredInventory::HideAbstractItem(class UDisAbstractItem *)
//   0x855db0  public: int __thiscall UDishonoredInventory::GetAbstractItemQuantity(class UDisAbstractItem *)const
//   0x855df0  protected: unsigned int __thiscall UDishonoredInventory::CanEquipItemType(class UClass *, enum EDisEquipUsage, int &)
//   0x857f00  public: unsigned int __thiscall UDishonoredInventory::CanEquipItemType(class UClass *, enum EDisEquipUsage, class UDisTweaks_InventoryItem const * *)
//   0x857fb0  protected: unsigned int __thiscall UDishonoredInventory::ShowInventoryItem(class UDishonoredInventoryItem *, unsigned int, enum EDisEquipUsage)
//   0x858040  public: unsigned int __thiscall UDishonoredInventory::ShowEquippedItem(unsigned int, enum EDisEquipUsage)
//   0x858080  public: void __thiscall UDishonoredInventory::RemoveAbstractItem(class UDisAbstractItem *, int)
//   0x859440  protected: void __thiscall UDishonoredInventory::BackupIntoLoadout(struct FDisInventoryLoadout &)
//   0x8596d0  public: virtual void __thiscall UDishonoredInventory::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8598f0  public: virtual void __thiscall UDishonoredInventory::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x85a580  public: static class UClass * __cdecl UDishonoredInventory::GetPrivateStaticClassUDishonoredInventory(wchar_t const *)
//   0x85a610  public: void __thiscall UDishonoredInventory::TerminateInventory(void)
//   0x85a730  public: void __thiscall UDishonoredInventory::OnCameraUpdate(struct FDishonoredViewTarget const &)
//   0x85a7b0  protected: void __thiscall UDishonoredInventory::DoEquip(int, enum EDisEquipUsage, unsigned int)
//   0x85a940  public: unsigned int __thiscall UDishonoredInventory::EquipItemType(class UClass *, enum EDisEquipUsage, unsigned int)
//   0x85a9b0  public: void __thiscall UDishonoredInventory::RemoveInventoryItem(class UDishonoredInventoryItem *, unsigned int)
//   0x85aab0  public: void __thiscall UDishonoredInventory::RemoveItemsByClass(class UClass const *, unsigned int)
//   0x85b760  public: static class UClass * __cdecl UDishonoredInventory::StaticClassNoInline(void)
//   0x85b790  public: void __thiscall UDishonoredInventory::InitInventory(class ADishonoredPawn *, class UDisTweaks_InventoryItem *)
//   0x85b930  public: void __thiscall UDishonoredInventory::GiveAllItems(class ADishonoredPawn *)
//   0x85bb60  protected: void __thiscall UDishonoredInventory::DropItemsInSlot(struct FPawnInventorySlot &, class FVector const &, class FVector const &)
//   0x85bc60  public: class UClass * __thiscall UDishonoredInventory::EquipNextItem(unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85bd60  public: class UClass * __thiscall UDishonoredInventory::EquipPrevItem(unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85be60  public: unsigned int __thiscall UDishonoredInventory::AddInventoryItem(class UDishonoredInventoryItem * const, unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85c760  public: void __thiscall UDishonoredInventory::TickInventory(float)
//   0x85c940  public: void __thiscall UDishonoredInventory::DropAttachedItems(class FVector const &, class FVector const &)
//   0x85c9c0  public: class UDishonoredInventoryItem * __thiscall UDishonoredInventory::SpawnAndAddInventoryItem(class UDisTweaks_InventoryItem * const, unsigned int, unsigned int, enum EDisEquipUsage)
//   0x85ca60  public: void __thiscall UDishonoredInventory::RestoreFromBackup(void)
//   0x861a90  public: void __thiscall UDishonoredInventory::ClearInventory(void)
//   0x861c70  public: void __thiscall UDishonoredInventory::BackupAndClear(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gjequipdiag.h"

// DISHONORED(written): 2013 rva 0x804bd0 (2012 0x851db0): only an initialised inventory answers; the usage's equipped slot
// index (-1 when nothing is equipped) selects the slot whose item is returned
UDishonoredInventoryItem* UDishonoredInventory::GetEquippedItem( BYTE Usage ) const
{
	if( !m_bInitialized )
	{
		return NULL;
	}
	const INT SlotIndex = m_EquipUsageInfo[Usage].m_iEquippedSlot;
	if( SlotIndex == INDEX_NONE )
	{
		return NULL;
	}
	return m_Slots(SlotIndex).m_pItem;
}

// DISHONORED(written): 2013 rva 0x80b780 (2012 0x855ce0): a known item takes the quantity (never negative), a new one is
// inserted at the front or appended, and the GFx HUD is told either way.
// DISHONORED(bringup): the HUD notification is the UDisGFxMoviePlayerHUD vtable slot +500 (item added), which is not ported.
void UDishonoredInventory::AddAbstractItem( UDisAbstractItem* Item, INT Quantity, UBOOL bAddToFront )
{
	if( !Item || Quantity <= 0 )
	{
		return;
	}
	for( INT Index = 0; Index < m_AbstractItem.Num(); Index++ )
	{
		if( m_AbstractItem(Index).m_pItem == Item )
		{
			m_AbstractItem(Index).m_Quantity += Quantity;
			return;
		}
	}
	const INT NewIndex = bAddToFront ? 0 : m_AbstractItem.Num();
	m_AbstractItem.InsertZeroed( NewIndex, 1 );
	m_AbstractItem(NewIndex).m_pItem = Item;
	m_AbstractItem(NewIndex).m_Quantity = Quantity;
	m_AbstractItem(NewIndex).m_bHidden = FALSE;
}

// DISHONORED(written): 2013 rva 0x80f8b0 (2012 0x858080): the quantity never goes below zero, and an emptied item whose
// m_bRemoveAtZeroQuantity is set loses its entry
void UDishonoredInventory::RemoveAbstractItem( UDisAbstractItem* Item, INT Quantity )
{
	if( !Item )
	{
		return;
	}
	for( INT Index = 0; Index < m_AbstractItem.Num(); Index++ )
	{
		if( m_AbstractItem(Index).m_pItem != Item )
		{
			continue;
		}
		const INT Left = Max<INT>( m_AbstractItem(Index).m_Quantity - Max<INT>( Quantity, 0 ), 0 );
		m_AbstractItem(Index).m_Quantity = Left;
		if( Left == 0 && Item->m_bRemoveAtZeroQuantity )
		{
			m_AbstractItem.Remove( Index, 1 );
		}
		return;
	}
}

// DISHONORED(written): 2013 rva 0x80b850 (2012 0x855d70, same bytes)
void UDishonoredInventory::HideAbstractItem( UDisAbstractItem* Item )
{
	if( !Item )
	{
		return;
	}
	for( INT Index = 0; Index < m_AbstractItem.Num(); Index++ )
	{
		if( m_AbstractItem(Index).m_pItem == Item )
		{
			m_AbstractItem(Index).m_bHidden = TRUE;
			return;
		}
	}
}

// DISHONORED(written): 2013 rva 0x80b890 (2012 0x855db0): an unknown item has quantity 0 and is not hidden
INT UDishonoredInventory::GetAbstractItemQuantity( UDisAbstractItem* Item, UBOOL* bOutHidden ) const
{
	if( bOutHidden )
	{
		*bOutHidden = FALSE;
	}
	for( INT Index = 0; Index < m_AbstractItem.Num(); Index++ )
	{
		if( m_AbstractItem(Index).m_pItem == Item )
		{
			if( bOutHidden )
			{
				*bOutHidden = m_AbstractItem(Index).m_bHidden ? TRUE : FALSE;
			}
			return m_AbstractItem(Index).m_Quantity;
		}
	}
	return 0;
}

// DISHONORED(written): 2013 rva 0x8053f0 (2012 0x852990): the ammo type indexes m_AmmoInfo directly, as retail does
const FDisAmmoInfo* UDishonoredInventory::GetAmmoInfo( BYTE AmmoType ) const
{
	return &m_AmmoInfo(AmmoType);
}

// DISHONORED(written): 2013 rva 0x805200 (2012 0x8527c0)
UBOOL UDishonoredInventory::HasAmmo( BYTE AmmoType, INT Count ) const
{
	return m_AmmoInfo(AmmoType).m_AmmoCount >= Count;
}

// DISHONORED(written): 2013 rva 0x8052e0 (2012 0x852880): the added amount is clamped to the remaining capacity and the
// equipped items of both player usages hear about it.
// DISHONORED(bringup): the item notification is the UDishonoredInventoryItem vtable slot +396 (ammo changed), not ported.
INT UDishonoredInventory::AddAmmo( BYTE AmmoType, INT Count )
{
	if( Count <= 0 )
	{
		return 0;
	}
	FDisAmmoInfo& Info = m_AmmoInfo(AmmoType);
	const INT Added = Min<INT>( Count, Info.m_AmmoCapacity - Info.m_AmmoCount );
	if( Added <= 0 )
	{
		return Added;
	}
	Info.m_AmmoCount += Added;
	return Added;
}

// DISHONORED(written): 2013 rva 0x805380 (2012 0x852920): without bIgnoreCapacity the count is capped at the capacity
void UDishonoredInventory::SetAmmo( BYTE AmmoType, INT Count, UBOOL bIgnoreCapacity )
{
	FDisAmmoInfo& Info = m_AmmoInfo(AmmoType);
	Info.m_AmmoCount = bIgnoreCapacity ? Count : Min<INT>( Count, Info.m_AmmoCapacity );
}

// DISHONORED(written): 2013 rva 0x805260 (2012 0x852820): the infinite-ammo cheat (game-info vtable slot +1060) consumes
// nothing and always succeeds.
// DISHONORED(bringup): that cheat virtual is not ported (retail's shipping build keeps it), so only the ammo count decides.
UBOOL UDishonoredInventory::ConsumeAmmo( BYTE AmmoType, INT Count )
{
	FDisAmmoInfo& Info = m_AmmoInfo(AmmoType);
	if( Info.m_AmmoCount < Count )
	{
		return FALSE;
	}
	Info.m_AmmoCount -= Count;
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x804e10 (2012 0x8523f0): elixirs exist on the player pawn only (retail tests the owner's
// m_ActorTypeFlags == 36); the new count is clamped to the player tweaks' maximum, and a non-player owner is reset to 0.
// DISHONORED(bringup): the two notifications retail then makes (the player pawn's elixir-changed handler and the HUD update)
// are not ported.
void UDishonoredInventory::AddElixir( BYTE ElixirType, INT Count )
{
	if( !m_pOwner || m_pOwner->m_ActorTypeFlags != 36 )
	{
		m_ElixirCounts[ElixirType] = 0;
		return;
	}
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( m_pOwner->GetTweaks() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
	}
	const INT MaxCount = ElixirType == ElixirType_Health ? Tweaks->m_nMaxHealthElixir : ElixirType == ElixirType_Mana ? Tweaks->m_nMaxManaElixir : 0;
	m_ElixirCounts[ElixirType] = Clamp<INT>( m_ElixirCounts[ElixirType] + Count, 0, MaxCount );
}

// DISHONORED(written): 2013 rva 0x804ec0 (2012 0x8524a0): the same clamp, on a player pawn only (retail tests the owner's
// class here instead of the actor-type flag)
void UDishonoredInventory::SetElixirCount( BYTE ElixirType, INT Count )
{
	if( !m_pOwner || !m_pOwner->IsA( ADishonoredPlayerPawn::StaticClass() ) )
	{
		m_ElixirCounts[ElixirType] = 0;
		return;
	}
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( m_pOwner->GetTweaks() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
	}
	const INT MaxCount = ElixirType == ElixirType_Health ? Tweaks->m_nMaxHealthElixir : ElixirType == ElixirType_Mana ? Tweaks->m_nMaxManaElixir : 0;
	m_ElixirCounts[ElixirType] = Clamp<INT>( Count, 0, MaxCount );
}

// ---- agent AU ports (PHASE7 AU) ----

// DISHONORED(written): 2013 rva 0x805440 (2012 0x8529e0): every ammo type the pickup carries is offered to AddAmmo, the
// pickup keeps what did not fit, and the total taken is reported back so ADisStatPickup's use message can name it.
// DISHONORED(bringup): the Attribute_StatPickupCapacityBonusChance roll (one extra round at a chance the player's
// attributes carry) needs IDisAttributesInterface::GetAttributeValue / UDisAttributes, which are not ported, so the
// chance is 0; UDisGFxMoviePlayerHUD::OnAmmoPickedUp is GFx and not ported.
UBOOL UDishonoredInventory::ConsumeStatPickup( class ADisStatPickup* Pickup, INT* OutConsumedAmmoCount )
{
	INT ConsumedAmmoCount = 0;
	if( Pickup )
	{
		for( INT Type = 0; Type < eDisAmmoType_MAX && Type < m_AmmoInfo.Num(); Type++ )
		{
			if( Pickup->m_CurAmmo[Type] == 0 )
			{
				continue;
			}
			const INT Added = AddAmmo( Type, Pickup->m_CurAmmo[Type] );
			Pickup->m_CurAmmo[Type] -= Added;
			ConsumedAmmoCount += Added;
		}
	}
	if( OutConsumedAmmoCount )
	{
		*OutConsumedAmmoCount = ConsumedAmmoCount;
	}
	return ConsumedAmmoCount > 0;
}

// ---- agent FV (PHASE18 FV): the lookups the NPC's look-at choice and the go-to's resume read ----

// DISHONORED(port): 2013 rva 0x80b4a0 (2012 0x852180, 332 B): the first slot whose item is of the class (exactly, or derived),
// skipping empty hands and non-removable items when asked, and matching the slot's required usage when one is given.
UDishonoredInventoryItem* UDishonoredInventory::FindItemByClass( const UClass* const _pItemClass, UBOOL _bExactMatch, UBOOL _bSkipEmptyHands, UBOOL _bSkipNonRemovable, BYTE _EquipUsage ) const
{
	for( INT Idx = 0; Idx < m_Slots.Num(); Idx++ )
	{
		UDishonoredInventoryItem* Item = m_Slots(Idx).m_pItem;
		if( !Item )
		{
			continue;
		}
		if( _bExactMatch ? Item->GetClass() != _pItemClass : ( _pItemClass && !Item->GetClass()->IsChildOf( _pItemClass ) ) )
		{
			continue;
		}
		if( _bSkipEmptyHands && Item->IsA( UDishonoredItemEmpty::StaticClass() ) )
		{
			continue;
		}
		if( _bSkipNonRemovable )
		{
			UDisTweaks_InventoryItem* Tweaks = Cast<UDisTweaks_InventoryItem>( Item->GetTweaks_Derived() );
			if( !Tweaks )
			{
				Tweaks = UDisTweaks_InventoryItem::StaticClass()->GetDefaultObject<UDisTweaks_InventoryItem>();
			}
			if( Tweaks->m_bNeverRemove )
			{
				continue;
			}
		}
		if( m_Slots(Idx).m_RequiredUsage == 0 || m_Slots(Idx).m_RequiredUsage == _EquipUsage )
		{
			return Item;
		}
	}
	return NULL;
}

// DISHONORED(port): 2013 rva 0x80f870 (2012 0x855bc0)
UBOOL UDishonoredInventory::HasItemByClass( const UClass* _pItemClass, UBOOL _bExactMatch ) const
{
	return FindItemByClass( _pItemClass, _bExactMatch, FALSE, FALSE, 0 ) != NULL;
}

// DISHONORED(port): 2013 rva 0x804c80 (2012 0x852060, 283 B): an item of the class that could go into the hand asked for -
// never the one the other hand holds, never empty hands; the one already equipped (in either hand) is preferred, otherwise
// the first found.
UDishonoredInventoryItem* UDishonoredInventory::FindEquippableItem( const UClass* const _pItemClass, BYTE _EquipUsage ) const
{
	INT PrimaryUsage = 1;
	INT SecondaryUsage = 2;
	INT ExcludedUsage = 0;
	if( _EquipUsage == EDisEquipUsage_Primary )
	{
		SecondaryUsage = 0;
		ExcludedUsage = 2;
	}
	else if( _EquipUsage == EDisEquipUsage_Secondary )
	{
		PrimaryUsage = 0;
		ExcludedUsage = 1;
	}
	const INT PrimarySlot = m_EquipUsageInfo[PrimaryUsage].m_iEquippedSlot;
	const INT SecondarySlot = m_EquipUsageInfo[SecondaryUsage].m_iEquippedSlot;
	const INT ExcludedSlot = m_EquipUsageInfo[ExcludedUsage].m_iEquippedSlot;
	UDishonoredInventoryItem* Found = NULL;
	for( INT Idx = 0; Idx < m_Slots.Num(); Idx++ )
	{
		if( Idx == ExcludedSlot )
		{
			continue;
		}
		UDishonoredInventoryItem* Item = m_Slots(Idx).m_pItem;
		if( !Item )
		{
			continue;
		}
		if( m_Slots(Idx).m_RequiredUsage != 0 && m_Slots(Idx).m_RequiredUsage != _EquipUsage )
		{
			continue;
		}
		if( _pItemClass && !Item->GetClass()->IsChildOf( _pItemClass ) )
		{
			continue;
		}
		if( Item->IsA( UDishonoredItemEmpty::StaticClass() ) )
		{
			continue;
		}
		if( Idx == PrimarySlot || Idx == SecondarySlot )
		{
			return Item;
		}
		if( !Found )
		{
			Found = Item;
		}
	}
	return Found;
}

// ---- agent GF (PHASE19 GF) ----

// DISHONORED(port): 2012 0x85a730 / 2013 0x817c90 (118 B, the same body)
void UDishonoredInventory::OnCameraUpdate( const FDishonoredViewTarget& _rVT )
{
	for( INT Index = 0; Index < m_Slots.Num(); Index++ )
	{
		UDishonoredInventoryItem* Item = m_Slots(Index).m_pItem;
		if( Item && Item->ShouldItemTick() )
		{
			Item->OnCameraUpdate( _rVT );
		}
	}
}

/*-----------------------------------------------------------------------------
	Agent GJ (PHASE20 GJ): the equip path. Every body below was decompiled from the 2012 database at the rva in its
	banner and checked against the 2013 one (build/agentGJ/decomp2012, decomp2013); the two differ where said.
	The Array.h index asserts retail compiles in are the (i) accessor's own here.
-----------------------------------------------------------------------------*/

// 2012 0x85b790 / 2013 0x818c80 (413 / 464 B): the owner, two reserved slots, then for each hand a slot that requires
// UDishonoredItemEmpty, an empty-hands item spawned from the pawn tweaks' m_pEmptyHandsItem (RF_Transactional in the
// transient package, as retail), InitItem, OnAddToInventory, and the hand equipped with it. The 2013 body adds the
// player's item-acquired notice (0xba1a30, the tutorial trackers) after OnAddToInventory - not ported, the trackers are not.
void UDishonoredInventory::InitInventory( ADishonoredPawn* _pOwner, UDisTweaks_InventoryItem* _pEmptyHandsType )
{
	m_bInitialized = TRUE;
	m_pOwner = _pOwner;
	m_Slots.Reserve( 2 );
	for( INT Usage = EDisEquipUsage_None; Usage < EDisEquipUsage_MAX; Usage++ )
	{
		m_EquipUsageInfo[Usage].m_iEquippedSlot = INDEX_NONE;
		m_EquipUsageInfo[Usage].m_iReequipSlot = INDEX_NONE;
		m_EquipUsageInfo[Usage].m_iReequipSlotNonEmpty = INDEX_NONE;
		m_EquipUsageInfo[Usage].m_iDropSlot_NextFrame = INDEX_NONE;
		if( Usage == EDisEquipUsage_None )
		{
			continue;
		}
		const INT SlotIndex = m_Slots.AddZeroed( 1 );
		m_Slots(SlotIndex).m_pRequiredType = UDishonoredItemEmpty::StaticClass();
		m_Slots(SlotIndex).m_RequiredUsage = Usage;
		if( _pEmptyHandsType )
		{
			UClass* ItemClass = _pEmptyHandsType->GetSpawnedObjectClass( 0 );
			UDishonoredInventoryItem* Item = ItemClass ? ConstructObject<UDishonoredInventoryItem>( ItemClass, UObject::GetTransientPackage(), NAME_None, RF_Transactional ) : NULL;
			if( Item )
			{
				Item->InitItem( _pEmptyHandsType );
				if( !m_Slots(SlotIndex).m_pItem )
				{
					m_Slots(SlotIndex).m_pItem = Item;
					Item->OnAddToInventory( this );
				}
			}
		}
		EquipItemType( UDishonoredItemEmpty::StaticClass(), Usage, FALSE );
	}
}

// 2012 0x851d60 / 2013 0x804b80 (69 B, same)
void UDishonoredInventory::AddInventorySlot( UClass* _pRequiredClass, BYTE _RequiredUsage )
{
	const INT SlotIndex = m_Slots.AddZeroed( 1 );
	m_Slots(SlotIndex).m_pRequiredType = _pRequiredClass;
	m_Slots(SlotIndex).m_RequiredUsage = _RequiredUsage;
}

// 2012 0x851e60 / 2013 0x80b2a0 (175 B, same): the first slot whose required type the class derives from and whose required
// usage is none or the one asked for. _bOwnerSpecific is not read by either body.
INT UDishonoredInventory::GetRequiredSlot( UClass* _pRequiredClass, UBOOL _bOwnerSpecific, BYTE _EquipUsage, FPawnInventorySlot** _ppOutSlot )
{
	if( _ppOutSlot )
	{
		*_ppOutSlot = NULL;
	}
	for( INT Idx = 0; Idx < m_Slots.Num(); Idx++ )
	{
		FPawnInventorySlot& Slot = m_Slots(Idx);
		if( !Slot.m_pRequiredType || ( Slot.m_RequiredUsage != 0 && Slot.m_RequiredUsage != _EquipUsage ) )
		{
			continue;
		}
		if( _pRequiredClass && _pRequiredClass->IsChildOf( Slot.m_pRequiredType ) )
		{
			if( _ppOutSlot )
			{
				*_ppOutSlot = &Slot;
			}
			return Idx;
		}
	}
	return INDEX_NONE;
}

// 2012 0x8559f0 / 2013 0x80f680 (90 B, same): the item's class against its tweaks' required usage; owner-specific when the
// tweaks name m_pOnlyHeldByPawnType
INT UDishonoredInventory::GetRequiredSlot( UDishonoredInventoryItem* _pItem )
{
	const UDisTweaks_InventoryItem* Tweaks = _pItem->GetItemTweaks();
	return GetRequiredSlot( _pItem->GetClass(), Tweaks->m_pOnlyHeldByPawnType != NULL, Tweaks->m_RequiredEquipUsage, NULL );
}

// 2012 0x851f10 / 2013 0x80b350 (322 B, same): from _iCur (or the start) round the slots once for an empty, usage-less slot
// whose required type (if any) the item's class derives from
INT UDishonoredInventory::GetNextEmptySlot( INT _iCur, UDishonoredInventoryItem* _pForItem )
{
	const INT NumSlots = m_Slots.Num();
	if( NumSlots == 0 )
	{
		return INDEX_NONE;
	}
	INT Idx = ( _iCur == INDEX_NONE || _iCur >= NumSlots ) ? 0 : _iCur;
	const INT Start = Idx;
	UClass* ItemClass = _pForItem->GetClass();
	for( ;; )
	{
		const FPawnInventorySlot& Slot = m_Slots(Idx);
		const UBOOL bFits = !Slot.m_pItem && Slot.m_RequiredUsage == 0 && ( !Slot.m_pRequiredType || ItemClass->IsChildOf( Slot.m_pRequiredType ) );
		if( bFits )
		{
			return Idx;
		}
		if( ++Idx >= NumSlots )
		{
			Idx = 0;
		}
		if( Idx == Start )
		{
			return INDEX_NONE;
		}
	}
}

// 2012 0x855df0 / 2013 0x80f920 (151 B, same): the required slot of the class for that usage must hold an item the owner can
// equip; the owner-specific pass is tried when the generic one has no equippable item
UBOOL UDishonoredInventory::CanEquipItemType( UClass* _pType, BYTE _EquipUsage, INT& _iSlot )
{
	FPawnInventorySlot* Slot = NULL;
	_iSlot = GetRequiredSlot( _pType, FALSE, _EquipUsage, &Slot );
	if( _iSlot == INDEX_NONE || !Slot->m_pItem || !Slot->m_pItem->IsEquippable( _EquipUsage, m_pOwner ) )
	{
		_iSlot = GetRequiredSlot( _pType, TRUE, _EquipUsage, &Slot );
	}
	return _iSlot != INDEX_NONE && Slot->m_pItem && Slot->m_pItem->IsEquippable( _EquipUsage, m_pOwner );
}

// 2012 0x857f00 / 2013 0x811b50 (163 B, same): the public form hands back the tweaks of the item that would equip
UBOOL UDishonoredInventory::CanEquipItemType( UClass* _pType, BYTE _EquipUsage, const UDisTweaks_InventoryItem** _ppItemThatWillEquip )
{
	INT SlotIndex = INDEX_NONE;
	const UBOOL bCanEquip = CanEquipItemType( _pType, _EquipUsage, SlotIndex );
	if( bCanEquip && _ppItemThatWillEquip )
	{
		*_ppItemThatWillEquip = m_Slots(SlotIndex).m_pItem->GetItemTweaks();
	}
	return bCanEquip;
}

// 2012 0x85a7b0 / 2013 0x817d10 (399 / 403 B): the hand's current item is unequipped (OnUnEquipItem), the slot recorded, the
// re-equip memory set unless temporary, an unattached item with a player anim-state component is attached to the equipped
// socket and its mesh hidden (SetHiddenGame TRUE - the equip animation's DisNotify_AttachItemInSlot shows it), then
// OnEquipItem. Both bodies then read the item tweaks for a player owner and do nothing with them (a compiled-out trace).
void UDishonoredInventory::DoEquip( INT _iSlot, BYTE _EquipUsage, UBOOL _bTemporary )
{
	FDisEquipUsageInfo& Usage = m_EquipUsageInfo[_EquipUsage];
	if( Usage.m_iEquippedSlot != INDEX_NONE )
	{
		UDishonoredInventoryItem* Previous = m_Slots(Usage.m_iEquippedSlot).m_pItem;
		if( Previous )
		{
			Previous->OnUnEquipItem();
		}
	}
	Usage.m_iEquippedSlot = _iSlot;
	UDishonoredInventoryItem* Item = m_Slots(_iSlot).m_pItem;
	if( !_bTemporary )
	{
		SetReequipItem( _EquipUsage, Item->GetClass() );
	}
	if( Item && Item->m_CurSocket == ItemSocket_None && Item->AnimState_GetAnimStateComp() )
	{
		AttachItemToSocket( Item, ItemSocket_Equipped );
		USkeletalMeshComponent* SkelComp = Item->AnimState_GetSkeletalComponent();
		if( SkelComp )
		{
			SkelComp->SetHiddenGame( TRUE );
		}
	}
	Item->OnEquipItem( _EquipUsage, _bTemporary );
}

// 2012 0x85a940 / 2013 0x817eb0 (110 B, same): CanEquipItemType, DoEquip, then DisGameEventType_ItemEquipped with the owner
// as instigator
UBOOL UDishonoredInventory::EquipItemType( UClass* _pType, BYTE _EquipUsage, UBOOL _bTemporary )
{
	INT SlotIndex = _EquipUsage;
	if( !CanEquipItemType( _pType, _EquipUsage, SlotIndex ) )
	{
		return FALSE;
	}
	DoEquip( SlotIndex, _EquipUsage, _bTemporary );
	FDisGameEventItemEquipped_Params Params;
	Params.m_pEquippingPawn = m_pOwner;
	Params.m_pItemClass = _pType;
	FArkGameEventDispatcher::GetInstance()->ProcessEvent( FArkGameEvent( DisGameEventType_ItemEquipped, &Params, m_pOwner ) );
	return TRUE;
}

// the 2013 body's helper 0xbc8920: the depth priority group onto the component and, through a skeletal mesh component's
// attachments, onto every attached component (2012 sets it on the item mesh alone)
static void DisSetItemDepthPriorityGroup( UActorComponent* Component, ESceneDepthPriorityGroup Group )
{
	UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>( Component );
	if( !Primitive )
	{
		return;
	}
	Primitive->SetDepthPriorityGroup( Group );
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( Primitive );
	if( SkelComp )
	{
		for( INT Idx = 0; Idx < SkelComp->Attachments.Num(); Idx++ )
		{
			UActorComponent* Attached = SkelComp->Attachments(Idx).Component;
			if( Attached && Attached->IsAttached() )
			{
				DisSetItemDepthPriorityGroup( Attached, Group );
			}
		}
	}
}

// 2012 0x855a50 / 2013 0x80f6e0 (364 / 378 B): the item's mesh (the player mesh for a player owner) is detached from whatever
// skeletal component holds it and, for a socket other than None, attached to the owner mesh's socket named by the item tweaks'
// m_Sockets[socket], put in the owner mesh's depth priority group when the owner is the player, and told OnAttached; None
// detaches and tells OnDetached. m_bNeedToTick follows ShouldItemTick. The 2013 body also calls a new item virtual (slot 80)
// before a detach - unnamed in both databases, not ported.
UBOOL UDishonoredInventory::AttachItemToSocket( UDishonoredInventoryItem* const _pItem, BYTE _ItemSocket )
{
	USkeletalMeshComponent* OwnerMesh = m_pOwner ? m_pOwner->Mesh : NULL;
	const UDisTweaks_InventoryItem* Tweaks = _pItem->GetItemTweaks();
	UDishonoredItemSkeletalComponent* ItemMesh = m_pOwner && m_pOwner->IsA( ADishonoredPlayerPawn::StaticClass() ) ? _pItem->m_pPlayerMesh : _pItem->m_pMesh;
	if( !ItemMesh || !OwnerMesh || _ItemSocket == _pItem->m_CurSocket )
	{
		return FALSE;
	}
	const FName SocketName = Tweaks->m_Sockets[_ItemSocket];
	if( ItemMesh->AttachedToSkelComponent )
	{
		ItemMesh->AttachedToSkelComponent->DetachComponent( ItemMesh );
	}
	if( _ItemSocket != ItemSocket_None )
	{
		OwnerMesh->AttachComponentToSocket( ItemMesh, SocketName );
		if( m_pOwner == ADishonoredPlayerPawn::s_pInstance )
		{
			DisSetItemDepthPriorityGroup( ItemMesh, (ESceneDepthPriorityGroup)OwnerMesh->DepthPriorityGroup );
		}
		_pItem->m_CurSocket = _ItemSocket;
		_pItem->OnAttached();
	}
	else
	{
		_pItem->m_CurSocket = ItemSocket_None;
		_pItem->OnDetached();
	}
	_pItem->m_bNeedToTick = _pItem->ShouldItemTick() ? 1 : 0;
	return TRUE;
}

// 2012 0x857fb0 / 2013 0x811c00 (141 B, same): hiding detaches; showing unhides the mesh and attaches it to the equipped socket
// when the item is the hand's equipped one, to the holstered socket otherwise
UBOOL UDishonoredInventory::ShowInventoryItem( UDishonoredInventoryItem* _pItem, UBOOL _bShow, BYTE _EquipUsage )
{
	if( !_bShow )
	{
		return AttachItemToSocket( _pItem, ItemSocket_None );
	}
	const BYTE Socket = GetEquippedItem( _EquipUsage ) == _pItem ? ItemSocket_Equipped : ItemSocket_Holstered;
	UDishonoredItemSkeletalComponent* ItemMesh = m_pOwner && m_pOwner->IsA( ADishonoredPlayerPawn::StaticClass() ) ? _pItem->m_pPlayerMesh : _pItem->m_pMesh;
	if( ItemMesh )
	{
		ItemMesh->SetHiddenGame( FALSE );
	}
	return AttachItemToSocket( _pItem, Socket );
}

// 2012 0x858040 / 2013 0x811c90 (52 B, same)
UBOOL UDishonoredInventory::ShowEquippedItem( UBOOL _bShow, BYTE _EquipUsage )
{
	UDishonoredInventoryItem* Item = GetEquippedItem( _EquipUsage );
	return Item ? ShowInventoryItem( Item, _bShow, _EquipUsage ) : FALSE;
}

// 2012 0x852a90 / 2013 0x805500 (219 B, same): the first slot of that usage (or none) holding an item of exactly that class
// becomes the hand's re-equip slot, and its non-empty re-equip slot when the class is not an empty hand; NULL clears both
void UDishonoredInventory::SetReequipItem( BYTE _EquipUsage, UClass* _pType )
{
	FDisEquipUsageInfo& Usage = m_EquipUsageInfo[_EquipUsage];
	if( !_pType )
	{
		Usage.m_iReequipSlot = INDEX_NONE;
		Usage.m_iReequipSlotNonEmpty = INDEX_NONE;
		return;
	}
	for( INT Idx = 0; Idx < m_Slots.Num(); Idx++ )
	{
		const FPawnInventorySlot& Slot = m_Slots(Idx);
		if( ( Slot.m_RequiredUsage == 0 || Slot.m_RequiredUsage == _EquipUsage ) && Slot.m_pItem && Slot.m_pItem->GetClass() == _pType )
		{
			Usage.m_iReequipSlot = Idx;
			if( !Slot.m_pItem->GetClass()->IsChildOf( UDishonoredItemEmpty::StaticClass() ) )
			{
				Usage.m_iReequipSlotNonEmpty = Idx;
			}
			return;
		}
	}
}

// 2012 0x852b70 / 2013 0x8055e0 (119 B, same)
UClass* UDishonoredInventory::GetReequipItem( BYTE _EquipUsage, UBOOL _bNonEmptyRequired ) const
{
	const INT SlotIndex = _bNonEmptyRequired ? m_EquipUsageInfo[_EquipUsage].m_iReequipSlotNonEmpty : m_EquipUsageInfo[_EquipUsage].m_iReequipSlot;
	if( SlotIndex < 0 || SlotIndex >= m_Slots.Num() || !m_Slots(SlotIndex).m_pItem )
	{
		return NULL;
	}
	return m_Slots(SlotIndex).m_pItem->GetClass();
}

// 2012 0x85be60 / 2013 0x819380 (451 / 522 B): an item whose tweaks name m_pOnlyHeldByPawnType needs an owner with those tweaks;
// the required slot (or the next empty one) takes it unless an unequipped occupant stays (dropped when _bDropIfNeeded; the 2013
// body adds a _bRemoveIfNeeded argument between the two that removes it instead - not taken here); OnAddToInventory, the
// player's wheel tracker (UDisWheelTracker::OnItemAdded, not ported - the wheel is GFx) and the 2013 item-acquired notice
// (not ported), then ShowInventoryItem when asked.
UBOOL UDishonoredInventory::AddInventoryItem( UDishonoredInventoryItem* const _pItem, UBOOL _bDropIfNeeded, UBOOL _bShowIfNeeded, BYTE _EquipUsage )
{
	INT SlotIndex = GetRequiredSlot( _pItem );
	const UDisTweaks_InventoryItem* Tweaks = _pItem->GetItemTweaks();
	if( Tweaks->m_pOnlyHeldByPawnType && !( m_pOwner && m_pOwner->HasTweaks( *Tweaks->m_pOnlyHeldByPawnType ) ) )
	{
		return FALSE;
	}
	if( SlotIndex == INDEX_NONE )
	{
		SlotIndex = GetNextEmptySlot( INDEX_NONE, _pItem );
	}
	else if( m_Slots(SlotIndex).m_pItem )
	{
		UBOOL bOccupantEquipped = FALSE;
		for( INT Usage = 0; Usage < EDisEquipUsage_MAX; Usage++ )
		{
			bOccupantEquipped |= m_EquipUsageInfo[Usage].m_iEquippedSlot == SlotIndex;
		}
		if( !_bDropIfNeeded || bOccupantEquipped )
		{
			return FALSE;
		}
		GJDIAG_ONCE( TEXT("UDishonoredInventory::AddInventoryItem: DropItemsInSlot (2012 0x85bb60) is not ported; the occupied slot keeps its item") );
		return FALSE;
	}
	if( SlotIndex == INDEX_NONE )
	{
		return FALSE;
	}
	if( !m_Slots(SlotIndex).m_pItem )
	{
		m_Slots(SlotIndex).m_pItem = _pItem;
		_pItem->OnAddToInventory( this );
	}
	if( _bShowIfNeeded )
	{
		ShowInventoryItem( _pItem, TRUE, _EquipUsage );
	}
	return TRUE;
}

// 2012 0x85c9c0 / 2013 0x819cd0 (147 / 151 B): the tweaks' spawned class constructed RF_Transactional in the transient package,
// InitItem, AddInventoryItem; NULL when the tweaks are missing, the construction fails or the inventory refuses it
UDishonoredInventoryItem* UDishonoredInventory::SpawnAndAddInventoryItem( UDisTweaks_InventoryItem* const _pItemTweaks, UBOOL _bDropIfNeeded, UBOOL _bShowIfNeeded, BYTE _EquipUsage )
{
	if( !_pItemTweaks )
	{
		return NULL;
	}
	UClass* ItemClass = _pItemTweaks->GetSpawnedObjectClass( 0 );
	UDishonoredInventoryItem* Item = ItemClass ? ConstructObject<UDishonoredInventoryItem>( ItemClass, UObject::GetTransientPackage(), NAME_None, RF_Transactional ) : NULL;
	if( !Item )
	{
		return NULL;
	}
	Item->InitItem( _pItemTweaks );
	return AddInventoryItem( Item, _bDropIfNeeded, _bShowIfNeeded, _EquipUsage ) ? Item : NULL;
}

// 2012 0x85c760 (490 B; 2013 unnamed, reached from Tick_Inventory): the deferred drops (m_bNeedToDropAnItem - DropItemsInSlot not
// ported, the request is cleared), TickInventoryItem on every slot item that needs to tick, then the active contexts: the ones
// no longer InProgress leave the list, the rest of a ticking item get their tick (UDisItemContext slot 76 - the contexts have no
// bodies here, named once)
void UDishonoredInventory::TickInventory( FLOAT _fDeltaTime )
{
	if( m_bNeedToDropAnItem )
	{
		for( INT Usage = 0; Usage < EDisEquipUsage_MAX; Usage++ )
		{
			if( m_EquipUsageInfo[Usage].m_iDropSlot_NextFrame != INDEX_NONE )
			{
				GJDIAG_ONCE( TEXT("UDishonoredInventory::TickInventory: a deferred drop was requested; DropItemsInSlot (2012 0x85bb60) is not ported") );
				m_EquipUsageInfo[Usage].m_iDropSlot_NextFrame = INDEX_NONE;
			}
		}
		m_bNeedToDropAnItem = FALSE;
	}
	for( INT Idx = 0; Idx < m_Slots.Num(); Idx++ )
	{
		UDishonoredInventoryItem* Item = m_Slots(Idx).m_pItem;
		if( Item && Item->m_bNeedToTick )
		{
			Item->TickInventoryItem( _fDeltaTime );
		}
	}
	for( INT Idx = 0; Idx < m_ActiveItemContexts.Num(); )
	{
		UDisItemContext* Context = m_ActiveItemContexts(Idx);
		if( Context && Context->m_ContextStatus == eDisItemContextStatus_InProgress )
		{
			if( Context->m_pOwner && Context->m_pOwner->m_bNeedToTick )
			{
				GJDIAG_ONCE( TEXT("UDishonoredInventory::TickInventory: an active item context wants its tick (UDisItemContext vtable slot 76); the contexts have no bodies") );
			}
			Idx++;
		}
		else
		{
			m_ActiveItemContexts.Remove( Idx, 1 );
		}
	}
}

// 2012 0x861a90 / 2013 0x8180a0 (480 / 391 B): every item removed, each hand that still names a slot detached and given empty hands
// again, every ammo count zeroed (a negative count is kept - the infinite marker), every abstract item removed, both elixirs zero
void UDishonoredInventory::ClearInventory()
{
	RemoveItemsByClass( UDishonoredInventoryItem::StaticClass(), FALSE );
	for( INT Usage = EDisEquipUsage_Primary; Usage < EDisEquipUsage_MAX; Usage++ )
	{
		const INT SlotIndex = m_EquipUsageInfo[Usage].m_iEquippedSlot;
		if( SlotIndex == INDEX_NONE )
		{
			continue;
		}
		if( m_Slots(SlotIndex).m_pItem )
		{
			AttachItemToSocket( m_Slots(SlotIndex).m_pItem, ItemSocket_None );
		}
		EquipItemType( UDishonoredItemEmpty::StaticClass(), Usage, FALSE );
	}
	for( INT Type = 0; Type < 8 && Type < m_AmmoInfo.Num(); Type++ )
	{
		m_AmmoInfo(Type).m_AmmoCount = Min<INT>( m_AmmoInfo(Type).m_AmmoCount, 0 );
	}
	for( INT Idx = 0; Idx < m_AbstractItem.Num(); )
	{
		const INT NumBefore = m_AbstractItem.Num();
		RemoveAbstractItem( m_AbstractItem(Idx).m_pItem, m_AbstractItem(Idx).m_Quantity );
		if( m_AbstractItem.Num() == NumBefore )
		{
			Idx++;
		}
	}
	for( INT Elixir = 0; Elixir < 2; Elixir++ )
	{
		SetElixirCount( Elixir, 0 );
	}
}

// 2012 0x85aab0 (474 B; 2013 unnamed): every removable item of the class (empty hands and m_bNeverRemove items skipped) leaves its
// slot: a hand holding it is given empty hands (DoEquip + the equipped event), the re-equip memories that named the slot are
// cleared, the mesh detached. The item object itself is left to the garbage collector, as retail leaves it.
void UDishonoredInventory::RemoveItemsByClass( const UClass* _pClass, UBOOL _bExactMatch )
{
	for( UDishonoredInventoryItem* Item = FindItemByClass( _pClass, _bExactMatch, TRUE, TRUE, EDisEquipUsage_None ); Item; Item = FindItemByClass( _pClass, _bExactMatch, TRUE, TRUE, EDisEquipUsage_None ) )
	{
		const UDisTweaks_InventoryItem* Tweaks = Item->GetItemTweaks();
		if( Tweaks->m_bNeverRemove )
		{
			break;
		}
		const INT SlotIndex = GetRequiredSlot( Item->GetClass(), Tweaks->m_pOnlyHeldByPawnType != NULL, Tweaks->m_RequiredEquipUsage, NULL );
		if( SlotIndex == INDEX_NONE )
		{
			break;
		}
		for( INT Usage = EDisEquipUsage_None; Usage < EDisEquipUsage_MAX; Usage++ )
		{
			FDisEquipUsageInfo& Info = m_EquipUsageInfo[Usage];
			if( Info.m_iEquippedSlot == SlotIndex )
			{
				INT EmptySlot = INDEX_NONE;
				if( CanEquipItemType( UDishonoredItemEmpty::StaticClass(), Usage, EmptySlot ) )
				{
					DoEquip( EmptySlot, Usage, FALSE );
					FDisGameEventItemEquipped_Params Params;
					Params.m_pEquippingPawn = m_pOwner;
					Params.m_pItemClass = UDishonoredItemEmpty::StaticClass();
					FArkGameEventDispatcher::GetInstance()->ProcessEvent( FArkGameEvent( DisGameEventType_ItemEquipped, &Params, m_pOwner ) );
				}
			}
			if( Info.m_iReequipSlot == SlotIndex )
			{
				Info.m_iReequipSlot = INDEX_NONE;
			}
			if( Info.m_iReequipSlotNonEmpty == SlotIndex )
			{
				Info.m_iReequipSlotNonEmpty = INDEX_NONE;
			}
		}
		AttachItemToSocket( Item, ItemSocket_None );
		m_Slots(SlotIndex).m_pItem = NULL;
	}
}

// 2012 0x851e10 / 2013 0x804c30 (79 B, same): NothingDone without a physically equipped item in the hand, else the item's own
// transition - TransitionFailure when it did nothing, TransitionSuccess otherwise
BYTE UDishonoredInventory::PlayerTransitionItemIn( BYTE _EquipUsage, BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip )
{
	UDishonoredInventoryItem* Item = GetEquippedItem( _EquipUsage );
	if( !Item || !Item->ItemIsPhysicallyEquipped() )
	{
		return eDisTransitionItemResult_NothingDone;
	}
	return Item->PlayerTransitionItemIn_Derived( _ActionUsage, _bFromOffscreen, _bUseFastEquip ) != eDisTransitionItemResult_NothingDone ? eDisTransitionItemResult_TransitionSuccess : eDisTransitionItemResult_TransitionFailure;
}
