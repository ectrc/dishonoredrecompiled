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
