// DishonoredGame/src/disseqact_togglehudelement.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x7f3b50  public: static void __cdecl UDisSeqAct_ToggleHUDElement::InitializePrivateStaticClassUDisSeqAct_ToggleHUDElement(void)
//   0x7f75b0  public: virtual void __thiscall UDisSeqAct_ToggleHUDElement::Activated(void)
//   0x81b140  public: static class UClass * __cdecl UDisSeqAct_ToggleHUDElement::GetPrivateStaticClassUDisSeqAct_ToggleHUDElement(wchar_t const *)
//   0x81cfa0  public: static class UClass * __cdecl UDisSeqAct_ToggleHUDElement::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(port): agent HE, 2013 rva 0x791940 (384 B; 2012 0x7f75b0, ratio 0.964). Input 0 enables, input 1 disables, and
// only on the designer-forced mask level (4). The element maps to a HUD bit through retail's local table - crosshair 0x1,
// health 0x2, mana 0x4, equipment 0x8, stealth gem 0x100 - except the stealth shroud, which is the player controller's
// m_bDisableStealthShroud (this+1552 bit 0x8000) instead. Retail reads the controller and its HUD without a NULL test.
void UDisSeqAct_ToggleHUDElement::Activated()
{
	static const INT ElementBits[DHE_StealthShroud] = { 0x1, 0x2, 0x4, 0x8, 0x100 };
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	const UBOOL bEnable = InputLinks(0).bHasImpulse;
	const UBOOL bDisable = !bEnable && InputLinks(1).bHasImpulse;
	if( m_HudElement == DHE_StealthShroud )
	{
		if( bEnable )
		{
			PC->m_bDisableStealthShroud = FALSE;
		}
		else if( bDisable )
		{
			PC->m_bDisableStealthShroud = TRUE;
		}
	}
	else
	{
		ADishonoredHUD* HUD = (ADishonoredHUD*)PC->myHUD;
		if( bEnable )
		{
			HUD->EnableHUDElements( eDisHUDMaskLevel_DesignerForced, ElementBits[m_HudElement] );
		}
		else if( bDisable )
		{
			HUD->DisableHUDElements( eDisHUDMaskLevel_DesignerForced, ElementBits[m_HudElement] );
		}
	}
	if( UDisGFxMoviePlayerHUD::IsDiagEnabled() )
	{
		ADishonoredHUD* HUD = Cast<ADishonoredHUD>( PC->myHUD );
		debugf( TEXT("DISHONORED(bringup): phuddiag ToggleHUDElement %s element %d %s level 4 -> %04X t=%.2f"), *GetPathName(),
			(INT)m_HudElement, bEnable ? TEXT("enable") : ( bDisable ? TEXT("disable") : TEXT("no input") ),
			HUD ? HUD->m_ShowFlags[eDisHUDMaskLevel_DesignerForced] : -1, GWorld ? GWorld->GetTimeSeconds() : 0.f );
	}
	Super::Activated();
}

// DISHONORED(bringup, agent HE): the unit's only function is a virtual, so nothing references this object and MSVC would drop it
// out of the static library (agents EN, FI). Named from DishonoredGameRegistrants.cpp.
void DishonoredLinkDisSeqActToggleHUDElement()
{
}
