// DishonoredGame/src/distweaks_playerloadout.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6fbd00  public: static void __cdecl UDisTweaks_PlayerLoadout::InitializePrivateStaticClassUDisTweaks_PlayerLoadout(void)
//   0x717260  public: void __thiscall UDisTweaks_PlayerLoadout::ApplyPlayerLoadoutToPlayer(class ADishonoredPlayerPawn *)const
//   0x719e60  public: static class UClass * __cdecl UDisTweaks_PlayerLoadout::GetPrivateStaticClassUDisTweaks_PlayerLoadout(wchar_t const *)
//   0x71ba90  public: static class UClass * __cdecl UDisTweaks_PlayerLoadout::StaticClassNoInline(void)

// Agent GJ (PHASE20 GJ): the loadout applied to the player.

#include "DishonoredGame.h"
#include "gjequipdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGJ_TweaksPlayerLoadout()
{
}

// 2012 0x717260 / 2013 0x6cf770 (734 / 462 B): the inventory cleared and the loadout spawned (SpawnInventoryLoadout with the ammo
// capacity respected), every upgrade of the player tweaks reverted and the loadout's starting upgrades applied (UDisTweaks_Upgrade::
// Revert / Apply - the 2013 body goes through OnLoseUpgrade / OnReceiveUpgrade), the bone charms replaced by m_NumStartingWhaleBoneCharms
// random ones, the powers component minimised and the starting powers set. The upgrades, charms and powers have no bodies here -
// named once each when the loadout asks for any.
void UDisTweaks_PlayerLoadout::ApplyPlayerLoadoutToPlayer( ADishonoredPlayerPawn* _pPlayer ) const
{
	if( !_pPlayer || !_pPlayer->m_pInventory )
	{
		return;
	}
	_pPlayer->m_pInventory->ClearInventory();
	_pPlayer->SpawnInventoryLoadout( m_InventoryLoadout, FALSE );
	if( m_StartingUpgrades.Num() > 0 || _pPlayer->m_Upgrades.Num() > 0 )
	{
		GJDIAG_ONCE( TEXT("UDisTweaks_PlayerLoadout::ApplyPlayerLoadoutToPlayer: UDisTweaks_Upgrade::Revert / Apply (2012 0x6739f0 / 0x673930) are not ported") );
	}
	if( m_NumStartingWhaleBoneCharms > 0 || _pPlayer->m_WhaleBoneCharms.Num() > 0 )
	{
		GJDIAG_ONCE( TEXT("UDisTweaks_PlayerLoadout::ApplyPlayerLoadoutToPlayer: RemoveBoneCharm / GiveRandomBoneCharm (2012 0x711940 / 0x716a40) are not ported") );
	}
	if( _pPlayer->m_pPowersComponent && m_StartingPowers.Num() > 0 )
	{
		GJDIAG_ONCE( TEXT("UDisTweaks_PlayerLoadout::ApplyPlayerLoadoutToPlayer: UDishonoredPowersComponent::MinOutPowers / SetPowerLevel (2012 0x713290 / 0x712bc0) are not ported") );
	}
}
