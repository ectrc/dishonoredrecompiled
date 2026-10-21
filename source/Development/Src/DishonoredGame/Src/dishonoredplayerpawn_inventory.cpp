// DishonoredGame/src/dishonoredplayerpawn_inventory.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x6fb600  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::ShouldEquipNewPickup_Derived(enum EDisEquipUsage)
//   0x6fb650  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::AddInventoryItem(class ADishonoredInventoryPickup *)
//   0x6fb6e0  public: void __thiscall ADishonoredPlayerPawn::EquipNextItem(unsigned int)
//   0x6fb720  public: void __thiscall ADishonoredPlayerPawn::EquipPrevItem(unsigned int)
//   0x6fb760  public: enum eDisTransitionItemResult __thiscall ADishonoredPlayerPawn::TransitionItemIn(enum EDisEquipUsage, enum eDisPlayerActionUsage, unsigned int, unsigned int)
//   0x6fb7a0  public: virtual void __thiscall ADishonoredPlayerPawn::DropCorpse(unsigned int)
//   0x6fb7d0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DisIsPossessing(void)const
//   0x6feb60  public: virtual void __thiscall ADishonoredPlayerPawn::EquipNextItem(void)
//   0x6feb90  public: virtual void __thiscall ADishonoredPlayerPawn::EquipPrevItem(void)
//   0x6febc0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsCarryingCorpse(void)const
//   0x706710  protected: void __thiscall ADishonoredPlayerPawn::GameSave_Inventory(class FArchive &, enum ESaveLoadLocation)
//   0x7067c0  protected: void __thiscall ADishonoredPlayerPawn::GameLoad_Inventory(class FArchive &, enum ESaveLoadLocation)
//   0x7068b0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::CanUseItemContext(class UDisItemContext const *)const
//   0x706a70  public: virtual int __thiscall ADishonoredPlayerPawn::CanUseObjectTypes(void)const
//   0x706b90  public: struct FPawnAction const * __thiscall ADishonoredPlayerPawn::GetUnequipAction(enum EDisEquipUsage, unsigned int)const
//   0x706cd0  public: unsigned int __thiscall ADishonoredPlayerPawn::UseEquippedItem(enum EDisEquipUsage)
//   0x706d20  public: unsigned int __thiscall ADishonoredPlayerPawn::AlternateUseEquippedItem(enum EDisEquipUsage, unsigned int)
//   0x706d80  public: virtual class UClass * __thiscall ADishonoredPlayerPawn::GetItemClassBeingEquipped(enum EDisEquipUsage)const
//   0x706dd0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DropMovableObject(unsigned int)
//   0x706e10  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::CanDropCorpse(void)const
//   0x706e60  public: virtual class IDisCorpseInterface * __thiscall ADishonoredPlayerPawn::GetCarriedCorpseOrBodyPart(void)const
//   0x70ce90  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::EquipItemByType(class UClass *, enum EDisEquipUsage, enum ADishonoredPawn::EEquipItemFlags)
//   0x70d140  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::GrabMovableObject(class UDisMovableComponent *)
//   0x70d2d0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::GrabCorpse(class ADishonoredNPCPawn *)
//   0x70d370  public: virtual void __thiscall ADishonoredPlayerPawn::UseKey(class FString const &)
//   0x70fef0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::HasKey(class FString const &)const
//   0x7128d0  public: void __thiscall ADishonoredPlayerPawn::RestoreUpgradesFromBackup(void)
//   0x716e10  public: void __thiscall ADishonoredPlayerPawn::BackupAndClearUpgrades(void)
//   0x718360  public: virtual void __thiscall ADishonoredPlayerPawn::PostBeginPlay_Inventory(void)

/*-----------------------------------------------------------------------------
	Agent GJ (PHASE20 GJ): the player's half of the equip path (decompiles in build/agentGJ/decomp2012 and decomp2013).
	2012 / 2013 rvas: PostBeginPlay_Inventory 0x718360 / 0x6ad200, EquipItemByType 0x70ce90 / 0x6b3210, TransitionItemIn
	0x6fb760 / 0x6a2590, GetItemClassBeingEquipped 0x706d80, GetDesiredPlayerStance 0x6fb250 / 0x6a2120, GetActualPlayerStance
	0x6fb220 / 0x6a20f0, SetPlayerStance 0x6fb200, IsLeftHandDisabled 0x6fb110, IsSneaking 0x707470.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gjequipdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGJ_PlayerPawnInventory()
{
}

// 2012 0x718360 / 2013 0x6ad200 (157 B, same): the base, the powers item (UDisTweaks_PlayerPawn::m_pPowersItem) spawned into the
// inventory, then - unless a save is about to restore the pawn - the map's own player loadout (UDishonoredMapInfo::
// m_pPlayerMapSpecificLoadout) or the tweaks' default one applied
void ADishonoredPlayerPawn::PostBeginPlay_Inventory()
{
	Super::PostBeginPlay_Inventory();
	UDishonoredMapInfo* MapInfo = DishonoredGetMapInfo();
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
	}
	const UBOOL bRestoring = DisIsObjectStateGoingToBeRestored( this );
	if( m_pInventory )
	{
		m_pInventory->SpawnAndAddInventoryItem( Tweaks->m_pPowersItem, FALSE, FALSE, EDisEquipUsage_None );
	}
	if( bRestoring )
	{
		return;
	}
	const UDisTweaks_PlayerLoadout* MapLoadout = MapInfo ? MapInfo->GetMapSpecificPlayerLoadout() : NULL;
	if( MapLoadout )
	{
		MapLoadout->ApplyPlayerLoadoutToPlayer( this );
	}
	else if( Tweaks->m_pDefaultPlayerLoadout )
	{
		Tweaks->m_pDefaultPlayerLoadout->ApplyPlayerLoadoutToPlayer( this );
	}
	for( INT Pass = 0; Pass < 2; Pass++ )
	{
		const UDisTweaks_PlayerLoadout* Loadout = Pass == 0 ? MapLoadout : Tweaks->m_pDefaultPlayerLoadout;
		for( INT Idx = 0; Loadout && Idx < Loadout->m_InventoryLoadout.m_Items.Num(); Idx++ )
		{
			const UDisTweaks_InventoryItem* ItemTweaks = Loadout->m_InventoryLoadout.m_Items(Idx);
			debugf( TEXT("DISHONORED(bringup): gj loadout %s item %d: %s -> %s"), Pass == 0 ? TEXT("map") : TEXT("default"), Idx, ItemTweaks ? *ItemTweaks->GetPathName() : TEXT("none"),
				ItemTweaks && ItemTweaks->GetSpawnedObjectClass( 0 ) ? *ItemTweaks->GetSpawnedObjectClass( 0 )->GetName() : TEXT("no class") );
		}
		if( Loadout )
		{
			debugf( TEXT("DISHONORED(bringup): gj loadout %s %s: defaults primary %s secondary %s"), Pass == 0 ? TEXT("map") : TEXT("default"), *Loadout->GetPathName(),
				Loadout->m_InventoryLoadout.m_pDefaultEquipType_Primary ? *Loadout->m_InventoryLoadout.m_pDefaultEquipType_Primary->GetName() : TEXT("none"),
				Loadout->m_InventoryLoadout.m_pDefaultEquipType_Secondary ? *Loadout->m_InventoryLoadout.m_pDefaultEquipType_Secondary->GetName() : TEXT("none") );
		}
	}
	debugf( TEXT("DISHONORED(bringup): gj PostBeginPlay_Inventory: powers item %s, loadout %s (%s), slots %d, primary %s, secondary %s"),
		Tweaks->m_pPowersItem ? *Tweaks->m_pPowersItem->GetName() : TEXT("none"),
		MapLoadout ? *MapLoadout->GetPathName() : Tweaks->m_pDefaultPlayerLoadout ? *Tweaks->m_pDefaultPlayerLoadout->GetPathName() : TEXT("none"),
		MapLoadout ? TEXT("map") : TEXT("default"), m_pInventory ? m_pInventory->m_Slots.Num() : -1,
		GetEquippedItem( EDisEquipUsage_Primary ) ? *GetEquippedItem( EDisEquipUsage_Primary )->GetClass()->GetName() : TEXT("none"),
		GetEquippedItem( EDisEquipUsage_Secondary ) ? *GetEquippedItem( EDisEquipUsage_Secondary )->GetClass()->GetName() : TEXT("none") );
}

// 2012 0x70ce90 / 2013 0x6b3210 (672 / 687 B, same): a left-hand equip is refused while the left hand is disabled unless it is the
// empty hand; nothing happens when the hand already holds the class or its item has an active context, or the inventory cannot
// equip the class. Retail then asks the hand's FSM for an equip-change action (FStatePlayerEquipChange_Param through RequestAction -
// UStatePlayerEquipChange plays the unequip and equip animations and calls the inventory at the right frames); the instant flag (or
// the powers item in a possession) takes the direct path - ADishonoredPawn::EquipItemByType, TransitionItemIn, m_bHandleInstantEquipChange,
// and for the left hand the left-arm FSM reset to idle then nav. UStatePlayerEquipChange is a skeleton here, so every request is
// taken the instant way (named once); the animated draw is what the stand-in leaves out. A non-temporary success remembers the
// class as the hand's re-equip item.
UBOOL ADishonoredPlayerPawn::EquipItemByType( UClass* _pType, BYTE _EquipUsage, INT _Flags )
{
	const UBOOL bTemporary = ( _Flags & EEquipItemFlags_Temporary ) != 0;
	const UBOOL bHandAllowed = _EquipUsage == EDisEquipUsage_Primary || !m_bLeftHandDisabled || _pType == UDishonoredItemEmpty::StaticClass();
	if( !m_pInventory || !bHandAllowed )
	{
		return FALSE;
	}
	UBOOL bEquipped = FALSE;
	UDishonoredInventoryItem* Equipped = m_pInventory->GetEquippedItem( _EquipUsage );
	const UBOOL bAlreadyHeld = Equipped && Equipped->GetClass()->IsChildOf( _pType );
	if( !bAlreadyHeld && !( Equipped && Equipped->HasActiveContexts() ) )
	{
		const UDisTweaks_InventoryItem* ItemThatWillEquip = NULL;
		if( m_pInventory->CanEquipItemType( _pType, _EquipUsage, &ItemThatWillEquip ) )
		{
			if( !( _Flags & EEquipItemFlags_InstantEquip ) )
			{
				GJDIAG_ONCE( TEXT("ADishonoredPlayerPawn::EquipItemByType: the equip-change action (UStatePlayerEquipChange through RequestAction) is a skeleton; the equip is taken the instant way, without the draw animation") );
			}
			bEquipped = ADishonoredPawn::EquipItemByType( _pType, _EquipUsage, _Flags );
			TransitionItemIn( _EquipUsage, _EquipUsage == EDisEquipUsage_Primary ? 1 : 2, FALSE, FALSE );
			m_bHandleInstantEquipChange = TRUE;
			if( _EquipUsage == EDisEquipUsage_Secondary && m_pPlayerLeftArmFSM )
			{
				GJDIAG_ONCE( TEXT("ADishonoredPlayerPawn::EquipItemByType: the left-arm FSM's idle-then-nav reset (UStatePlayerUpperIdle / UStatePlayerActionNav) is a skeleton") );
			}
		}
	}
	if( !bTemporary && bEquipped )
	{
		m_pInventory->SetReequipItem( _EquipUsage, _pType );
	}
	return bEquipped;
}

// 2012 0x6fb760 / 2013 0x6a2590 (53 B, same): the hand's desired stance refreshed, then the inventory's transition
BYTE ADishonoredPlayerPawn::TransitionItemIn( BYTE _EquipUsage, BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip )
{
	m_DesiredPlayerStance[_EquipUsage] = GetDesiredPlayerStance( _EquipUsage );
	return m_pInventory ? m_pInventory->PlayerTransitionItemIn( _EquipUsage, _ActionUsage, _bFromOffscreen, _bUseFastEquip ) : eDisTransitionItemResult_NothingDone;
}

// 2012 0x706d80 (70 B): the class the hand's FSM (upper for the primary, left arm for the secondary) is equipping while its logical
// state is a UStatePlayerEquipChange - a skeleton here, so no hand is ever mid-equip
UClass* ADishonoredPlayerPawn::GetItemClassBeingEquipped( BYTE _EquipUsage ) const
{
	return NULL;
}

// 2012 0x6fb250 / 2013 0x6a2120 (144 / 156 B, same): the hand's stance through the item, then: the primary goes Blocking when the
// item left it at MAX (3), NotReady while a corpse is carried, Ready while sneaking and not ready; the secondary is Ready; the item
// modifies the result again
BYTE ADishonoredPlayerPawn::GetDesiredPlayerStance( BYTE _EquipUsage ) const
{
	BYTE Current = m_PlayerStance[_EquipUsage];
	UDishonoredInventoryItem* Item = GetEquippedItem( _EquipUsage );
	if( Item )
	{
		Item->ModifyPlayerStance( Current );
	}
	BYTE Stance = eDisPlayerStance_NotReady;
	if( _EquipUsage != EDisEquipUsage_Primary )
	{
		Stance = eDisPlayerStance_Ready;
	}
	else
	{
		if( Current == eDisPlayerStance_Blocking )
		{
			Stance = eDisPlayerStance_Blocking;
		}
		if( IsCarryingCorpse() )
		{
			Stance = eDisPlayerStance_NotReady;
		}
		else if( IsSneaking() && Stance == eDisPlayerStance_NotReady )
		{
			Stance = eDisPlayerStance_Ready;
		}
	}
	if( Item )
	{
		Item->ModifyPlayerStance( Stance );
	}
	return Stance;
}

// 2012 0x6fb220 / 2013 0x6a20f0 (45 B, same)
BYTE ADishonoredPlayerPawn::GetActualPlayerStance( BYTE _EquipUsage ) const
{
	BYTE Stance = m_PlayerStance[_EquipUsage];
	UDishonoredInventoryItem* Item = GetEquippedItem( _EquipUsage );
	if( Item )
	{
		Item->ModifyPlayerStance( Stance );
	}
	return Stance;
}

// DISHONORED(port): agent IF, 2012 0x6febc0 / 2013 0x6a5220 (153 B): the upper FSM's logical state is carrying or grabbing a
// corpse. The upper machine is not initialised in this tree (agent FZ), so this answers FALSE; retail does not test the pointer.
UBOOL ADishonoredPlayerPawn::IsCarryingCorpse() const
{
	UDishonoredNativeState* LogicalState = m_pPlayerUpperFSM ? m_pPlayerUpperFSM->GetLogicalState() : NULL;
	return LogicalState != NULL && ( LogicalState->IsA( UStatePlayerCarryCorpseIdle::StaticClass() ) || LogicalState->IsA( UStatePlayerGrabCorpse::StaticClass() ) );
}

// DISHONORED(port): agent HE, 2012 0x6fb7d0 (86 B): the possession states of the player's master FSM
UBOOL ADishonoredPlayerPawn::DisIsPossessing() const
{
	return m_pPlayerMasterFSM != NULL
		&& ( m_pPlayerMasterFSM->IsCurState( UStatePlayerMasterPossess::StaticClass(), TRUE )
		|| m_pPlayerMasterFSM->IsCurState( UStatePlayerMasterPrePossess::StaticClass(), TRUE ) );
}

// DISHONORED(port): agent HE, 2012 0x6fec70 (38 B)
UBOOL ADishonoredPlayerPawn::IsSliding() const
{
	return m_pPlayerMasterFSM != NULL && m_pPlayerMasterFSM->IsCurState( UStatePlayerMasterSlide::StaticClass(), TRUE );
}

// DISHONORED(port): agent JH (PHASE24 JH). 2012 0x70fef0 (32 B): the key ring holds the key. Retail reads the ring without a test;
// this tree tests it.
UBOOL ADishonoredPlayerPawn::HasKey( const FString& _rKey ) const
{
	return m_pKeyRing && m_pKeyRing->FindKey( _rKey ) != INDEX_NONE;
}
