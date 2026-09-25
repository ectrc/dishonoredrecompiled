// DishonoredGame/src/dispowermenu.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x6300a0  public: static void __cdecl UDisPowerMenu::InitializePrivateStaticClassUDisPowerMenu(void)
//   0x6407f0  public: void __thiscall UDisPowerMenu::HandleUp(void)
//   0x640810  public: void __thiscall UDisPowerMenu::HandleDown(void)
//   0x648bf0  public: void __thiscall UDisPowerMenu::Render(class UCanvas *)const
//   0x64b690  private: void __thiscall UDisPowerMenu::RegenerateItems(void)
//   0x64bb30  public: void __thiscall UDisPowerMenu::HandleEnter(void)
//   0x651660  public: static class UClass * __cdecl UDisPowerMenu::GetPrivateStaticClassUDisPowerMenu(wchar_t const *)
//   0x6536d0  public: static class UClass * __cdecl UDisPowerMenu::StaticClassNoInline(void)
//   0x65f5f0  public: void __thiscall UDisPowerMenu::ShowMenu(void)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x602280 (2012 0x648bf0): nothing is drawn while the menu is hidden
void UDisPowerMenu::Render( UCanvas* Canvas )
{
	if( !m_bVisible )
	{
		return;
	}
	// DISHONORED(bringup): the visible menu (dark tile, "Wealth" line, one line per power with its cost, 1.2 KB of canvas text) is not
	// ported; it is the debug power-purchase menu, opened only by its own input handling
}
