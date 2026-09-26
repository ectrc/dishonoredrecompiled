// DishonoredGame/src/diselixir.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x66a820  protected: virtual class UDisTweaksBase * __thiscall ADisElixirMana::GetTweaks_Derived(void)
//   0x66c6e0  private: virtual unsigned int __thiscall ADisElixirHealth::DoInteract_Impl(class ADishonoredPlayerPawn *)
//   0x66c710  private: virtual unsigned int __thiscall ADisElixirMana::DoInteract_Impl(class ADishonoredPlayerPawn *)
//   0x6704f0  public: virtual unsigned int __thiscall ADisElixirHealth::CanBePickedUp(class ADishonoredPlayerPawn *)const
//   0x670550  public: virtual unsigned int __thiscall ADisElixirMana::CanBePickedUp(class ADishonoredPlayerPawn *)const
//   0x678980  public: static class UClass * __cdecl ADisElixirHealth::GetPrivateStaticClassADisElixirHealth(wchar_t const *)
//   0x678a10  public: static class UClass * __cdecl ADisElixirMana::GetPrivateStaticClassADisElixirMana(wchar_t const *)
//   0x67ae60  public: static class UClass * __cdecl UDisTweaks_ElixirHealth::GetPrivateStaticClassUDisTweaks_ElixirHealth(wchar_t const *)
//   0x67aef0  public: static class UClass * __cdecl UDisTweaks_ElixirMana::GetPrivateStaticClassUDisTweaks_ElixirMana(wchar_t const *)
//   0x67bd90  public: static void __cdecl ADisElixirHealth::InitializePrivateStaticClassADisElixirHealth(void)
//   0x67bdb0  public: static void __cdecl ADisElixirMana::InitializePrivateStaticClassADisElixirMana(void)
//   0x67c640  public: static class UClass * __cdecl ADisElixirHealth::StaticClassNoInline(void)
//   0x67c670  public: static void __cdecl UDisTweaks_ElixirHealth::InitializePrivateStaticClassUDisTweaks_ElixirHealth(void)
//   0x67c690  public: static class UClass * __cdecl ADisElixirMana::StaticClassNoInline(void)
//   0x67c6c0  public: static void __cdecl UDisTweaks_ElixirMana::InitializePrivateStaticClassUDisTweaks_ElixirMana(void)
//   0x67ff90  public: static class UClass * __cdecl UDisTweaks_ElixirHealth::StaticClassNoInline(void)
//   0x67ffc0  public: static class UClass * __cdecl UDisTweaks_ElixirMana::StaticClassNoInline(void)
// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): ADisElixirHealth's IDisTweaksInterface slots (SetTweaks_Derived 2013 rva 0x61d5c0, 2012 0x88b260: a single
// store into m_pPickupTweaks; the getter is folded).
UDisTweaksBase* ADisElixirHealth::GetTweaks_Derived()
{
	return m_pPickupTweaks;
}

void ADisElixirHealth::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pPickupTweaks = (UDisTweaks_ElixirHealth*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x622160 (2012 0x6704f0): one more may be taken only below the UDisTweaks_PlayerPawn cap.
UBOOL ADisElixirHealth::CanBePickedUp( ADishonoredPlayerPawn* PlayerPawn ) const
{
	if( !PlayerPawn || !PlayerPawn->m_pInventory )
	{
		return FALSE;
	}
	UDisTweaksBase* PawnTweaks = PlayerPawn->GetTweaks_Derived();
	if( !PawnTweaks )
	{
		PawnTweaks = (UDisTweaksBase*)UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject();
	}
	return PlayerPawn->m_pInventory->m_ElixirCounts[0] < ((UDisTweaks_PlayerPawn*)PawnTweaks)->m_nMaxHealthElixir;
}

// DISHONORED(written): 2013 rva 0x61d560 (2012 0x66c6e0): one elixir into the inventory.
// DISHONORED(bringup): the HUD notification that follows it (UDisGFxMoviePlayerHUD, 2013 rva 0x795090) is not ported.
UBOOL ADisElixirHealth::DoInteract_Impl( ADishonoredPlayerPawn* PlayerPawn )
{
	if( PlayerPawn && PlayerPawn->m_pInventory )
	{
		PlayerPawn->m_pInventory->AddElixir( 0, 1 );
	}
	return TRUE;
}

// DISHONORED(written): ADisElixirMana's IDisTweaksInterface slots (SetTweaks_Derived 2013 rva 0x61d5c0, 2012 0x88b260: a single
// store into m_pPickupTweaks; the getter is folded).
UDisTweaksBase* ADisElixirMana::GetTweaks_Derived()
{
	return m_pPickupTweaks;
}

void ADisElixirMana::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pPickupTweaks = (UDisTweaks_ElixirMana*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x6222c0 (2012 0x670550): one more may be taken only below the UDisTweaks_PlayerPawn cap.
UBOOL ADisElixirMana::CanBePickedUp( ADishonoredPlayerPawn* PlayerPawn ) const
{
	if( !PlayerPawn || !PlayerPawn->m_pInventory )
	{
		return FALSE;
	}
	UDisTweaksBase* PawnTweaks = PlayerPawn->GetTweaks_Derived();
	if( !PawnTweaks )
	{
		PawnTweaks = (UDisTweaksBase*)UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject();
	}
	return PlayerPawn->m_pInventory->m_ElixirCounts[1] < ((UDisTweaks_PlayerPawn*)PawnTweaks)->m_nMaxManaElixir;
}

// DISHONORED(written): 2013 rva 0x61d590 (2012 0x66c710): one elixir into the inventory.
// DISHONORED(bringup): the HUD notification that follows it (UDisGFxMoviePlayerHUD, 2013 rva 0x795090) is not ported.
UBOOL ADisElixirMana::DoInteract_Impl( ADishonoredPlayerPawn* PlayerPawn )
{
	if( PlayerPawn && PlayerPawn->m_pInventory )
	{
		PlayerPawn->m_pInventory->AddElixir( 1, 1 );
	}
	return TRUE;
}
