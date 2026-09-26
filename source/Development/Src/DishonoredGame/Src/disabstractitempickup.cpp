// DishonoredGame/src/disabstractitempickup.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x887ba0  public: static void __cdecl ADisAbstractItemPickup::InitializePrivateStaticClassADisAbstractItemPickup(void)
//   0x887bc0  public: static void __cdecl UDisTweaks_AbstractItemPickup::InitializePrivateStaticClassUDisTweaks_AbstractItemPickup(void)
//   0x88b260  protected: virtual void __thiscall ADisElixirHealth::SetTweaks_Derived(class UDisTweaksBase *)
//   0x88fc00  public: virtual unsigned int __thiscall ADisAbstractItemPickup::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x899c60  public: static class UClass * __cdecl ADisAbstractItemPickup::GetPrivateStaticClassADisAbstractItemPickup(wchar_t const *)
//   0x89c3f0  public: static class UClass * __cdecl ADisAbstractItemPickup::StaticClassNoInline(void)
//   0x89ebf0  public: static class UClass * __cdecl UDisTweaks_AbstractItemPickup::GetPrivateStaticClassUDisTweaks_AbstractItemPickup(wchar_t const *)
//   0x89f7c0  public: static class UClass * __cdecl UDisTweaks_AbstractItemPickup::StaticClassNoInline(void)
//   0x8a4b90  public: virtual unsigned int __thiscall ADisAbstractItemPickup::DoInteract_Impl(class ADishonoredPlayerPawn *)
//   0x8a4cd0  protected: virtual class FString const & __thiscall ADisAbstractItemPickup::GetUseMessage(void)const
//   0x8a4d20  protected: virtual void __thiscall ADisAbstractItemPickup::FormatText(class FString &)const
//   0x8a4e80  public: virtual void __thiscall ADisAbstractItemPickup::Steal(class ADishonoredPlayerPawn *)
//   0x8a4ef0  public: virtual unsigned int __thiscall ADisAbstractItemPickup::CanBePickedUp(class ADishonoredPlayerPawn *)const

// DISHONORED(written): ADisAbstractItemPickup's IDisTweaksInterface slots. GetTweaks_Derived / SetTweaks_Derived (identical-COMDAT folded with the other pickups' pair in the 2013 exe).
// Without these every pickup reads the class default of its tweaks class, so its item, ammo and elixir amounts are all
// zero - which is exactly what the first -dispickupprobe run measured before they were added.

// ---- agent AU ports (PHASE7 AU): the abstract-item (gold, runes, bone charms, notes) pickup ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): the GetTweaks_Derived-or-class-default shape of every ADisAbstractItemPickup body
UDisTweaks_AbstractItemPickup* ADisAbstractItemPickup::GetAbstractItemTweaks() const
{
	UDisTweaksBase* Tweaks = const_cast<ADisAbstractItemPickup*>( this )->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaksBase*)UDisTweaks_AbstractItemPickup::StaticClass()->GetDefaultObject();
	}
	return (UDisTweaks_AbstractItemPickup*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x84c100 (2012 0x8a4ef0): a maximum quantity caps how much of the item the player may
// hold; without one the base answer stands.
UBOOL ADisAbstractItemPickup::CanBePickedUp( ADishonoredPlayerPawn* PlayerPawn ) const
{
	const UBOOL bBase = ADisPickup_Base::CanBePickedUp( PlayerPawn );
	UDisTweaks_AbstractItemPickup* Tweaks = GetAbstractItemTweaks();
	if( !bBase || !Tweaks->m_pAbstractItem || Tweaks->m_MaximumQuantity <= 0 )
	{
		return bBase;
	}
	UDishonoredInventory* Inventory = PlayerPawn ? PlayerPawn->m_pInventory : NULL;
	const INT Held = Inventory ? Inventory->GetAbstractItemQuantity( Tweaks->m_pAbstractItem ) : 0;
	return ( Tweaks->m_Quantity + Held ) <= Tweaks->m_MaximumQuantity;
}

// DISHONORED(written): 2013 rva 0x84bd60 (2012 0x8a4b90): the item and its quantity go into the inventory.
// DISHONORED(bringup): the precious-item value bonus (UDisTweaks_PlayerPawn's precious-item tweak class +
// Attribute_PreciousItemsValueBonus), the ePlayerStat_GoldFound / ePlayerStat_RuneFound stats and the HUD's
// OnAbstractItemPickedUp notification all need unported code (IDisAttributesInterface::GetAttributeValue,
// ADishonoredPlayerPawn::IncrementStat, UDisGFxMoviePlayerHUD).
UBOOL ADisAbstractItemPickup::DoInteract_Impl( ADishonoredPlayerPawn* PlayerPawn )
{
	ADisPickup_Base::DoInteract_Impl( PlayerPawn );
	UDisTweaks_AbstractItemPickup* Tweaks = GetAbstractItemTweaks();
	if( Tweaks->m_pAbstractItem && PlayerPawn && PlayerPawn->m_pInventory )
	{
		PlayerPawn->m_pInventory->AddAbstractItem( Tweaks->m_pAbstractItem, Tweaks->m_Quantity, FALSE );
	}
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x84bee0 (2012 0x8a4cd0): one of the item is its own message
const FString& ADisAbstractItemPickup::GetUseMessage() const
{
	UDisTweaks_AbstractItemPickup* Tweaks = GetAbstractItemTweaks();
	if( Tweaks->m_Quantity <= 1 )
	{
		return Tweaks->m_SingleItemPickupMessage;
	}
	return IDisInteractableInterface::GetUseMessage();
}

// DISHONORED(written): 2013 rva 0x84bf30 (2012 0x8a4d20): the quantity replaces "`n" and the item's own name "`~"
// DISHONORED(bringup): retail reads the display name off the UDisAbstractItem's localised strings, which are not
// ported; the interactable tweaks' name is used for "`~" as the base does.
void ADisAbstractItemPickup::FormatText( FString& Text ) const
{
	IDisInteractableInterface::FormatText( Text );
	Text = Text.Replace( TEXT("`n"), *FString::Printf( TEXT("%i"), GetAbstractItemTweaks()->m_Quantity ) );
}

// DISHONORED(written): 2013 rva 0x8452f0 (2012 0x88fc00, same bytes)
UBOOL ADisAbstractItemPickup::ShouldTrace( UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags )
{
	// DISHONORED(retail): an abstract item is never a melee or projectile target, only a crosshair / move one.
	if( ( TraceFlags & ( TRACE_DisGameplay_Melee | TRACE_DisGameplay_Projectile ) ) != 0 )
	{
		return FALSE;
	}
	return ADisPickup_Base::ShouldTrace( Primitive, SourceActor, TraceFlags );
}

// DISHONORED(written): 2013 rva 0x84c090 (2012 0x8a4e80, same bytes)
void ADisAbstractItemPickup::Steal( ADishonoredPlayerPawn* PlayerPawn )
{
	ADisPickup_Base::Steal( PlayerPawn );
}

// DISHONORED(written): ADisAbstractItemPickup's IDisTweaksInterface slots. Both slots are identical-COMDAT folded in the 2013 exe.
UDisTweaksBase* ADisAbstractItemPickup::GetTweaks_Derived()
{
	return m_pTweaks;
}

void ADisAbstractItemPickup::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pTweaks = (UDisTweaks_AbstractItemPickup*)Tweaks;
}
