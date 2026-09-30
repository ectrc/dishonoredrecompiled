// DishonoredGame/src/dishonoredplayerinput.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x6d1a40  public: static void __cdecl UDishonoredPlayerInput::InitializePrivateStaticClassUDishonoredPlayerInput(void)
//   0x6d1a60  public: static void __cdecl UDisTweaks_PlayerInput::InitializePrivateStaticClassUDisTweaks_PlayerInput(void)
//   0x6d1a80  public: virtual void __thiscall UDishonoredPlayerInput::Dis_Jump_ButtonDown(void)
//   0x6d1a90  public: virtual void __thiscall UDishonoredPlayerInput::PlayerInput(float)
//   0x6d1ae0  public: struct FVector2D __thiscall UDishonoredPlayerInput::GetMouseDirection(unsigned int)
//   0x6e4100  public: struct FKeyBind const * __thiscall UDishonoredPlayerInput::GetKeyBindFromAction(class FName, unsigned int, unsigned int)const
//   0x6e41a0  public: virtual void __thiscall UDishonoredPlayerInput::Dis_PlayerChoice_RequestSkip(void)
//   0x6e4200  public: virtual void __thiscall UDishonoredPlayerInput::AdjustMouseSensitivity(float)
//   0x6ef8f0  protected: void __thiscall UDishonoredPlayerInput::ReadPCBindingsFromProfile(void)
//   0x6efbe0  protected: void __thiscall UDishonoredPlayerInput::AddBindingSet(class TArrayNoInit<struct FKeyBind> const &)
//   0x6f1750  protected: void __thiscall UDishonoredPlayerInput::TranslateBaseBindings(void)
//   0x6f3fe0  protected: void __thiscall UDishonoredPlayerInput::InitGameActionBindings(void)
//   0x6f53e0  public: static class UClass * __cdecl UDisTweaks_PlayerInput::GetPrivateStaticClassUDisTweaks_PlayerInput(wchar_t const *)
//   0x6f69a0  public: static class UClass * __cdecl UDisTweaks_PlayerInput::StaticClassNoInline(void)
//   0x6f69d0  protected: unsigned int __thiscall UDishonoredPlayerInput::HandleMultiPressCommand(wchar_t const *, wchar_t const *, wchar_t *, int, class FOutputDevice &)
//   0x6f8db0  public: int __thiscall UDishonoredPlayerInput::TranslateBindingSet(int)const
//   0x6f8e10  protected: virtual void __thiscall UDishonoredPlayerInput::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x6f90b0  public: void __thiscall UDishonoredPlayerInput::OnInit(class ADishonoredPlayerController *)
//   0x6f9170  public: virtual void __thiscall UDishonoredPlayerInput::Dis_SneakOrSlide(unsigned int)
//   0x6f92e0  public: virtual void __thiscall UDishonoredPlayerInput::Dis_PlayerChoice_RequestSkip_Released(void)
//   0x6f9380  public: virtual float __thiscall UDishonoredPlayerInput::GetFOVScale(unsigned int)
//   0x6f9460  protected: void __thiscall UDishonoredPlayerInput::HandleCutSceneSkipping(void)
//   0x6f9620  protected: unsigned int __thiscall UDishonoredPlayerInput::HandleAxisCommand(wchar_t const *, wchar_t const *, wchar_t *, int, class FOutputDevice &)
//   0x6f98c0  public: virtual unsigned int __thiscall UDishonoredPlayerInput::Exec(wchar_t const *, class FOutputDevice &)
//   0x6fa1b0  public: static class UClass * __cdecl UDishonoredPlayerInput::GetPrivateStaticClassUDishonoredPlayerInput(wchar_t const *)
//   0x6fa240  public: virtual void __thiscall UDishonoredPlayerInput::PreProcessInput(float)
//   0x6fa8b0  public: static class UClass * __cdecl UDishonoredPlayerInput::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x6afeb0 (2012 0x6f90b0). 2013 reads UDishonoredEngine::m_bUsingGamepad at its 2013 bit (0x400).
void UDishonoredPlayerInput::OnInit( ADishonoredPlayerController* Owner )
{
	m_pPlayerController = Cast<ADishonoredPlayerController>( Owner );
	m_pTweaks = (UDisTweaks_PlayerInput*)UObject::StaticLoadObject( UDisTweaks_PlayerInput::StaticClass(), NULL, *m_TweakName, NULL, LOAD_None, NULL, TRUE );
	UDishonoredEngine* DishonoredEngine = Cast<UDishonoredEngine>( GEngine );
	if( DishonoredEngine )
	{
		bUsingGamepad = DishonoredEngine->m_bUsingGamepad;
	}
	// DISHONORED(bringup): ArkSettings::ApplyCurrentSettings(IArkSettingsListenerInterface) (Engine, 2013 rva 0x53b790) is not in our
	// Engine yet, so the sensitivity/invert options stay at their defaults and the binding half of ApplyGameSettings is called here
	BuildBindings();
}

// DISHONORED(written): 2013 rva 0x6a12b0 (2012 0x6d1a80, same bytes)
void UDishonoredPlayerInput::Dis_Jump_ButtonDown()
{
	m_pPlayerController->bPressedJump = TRUE;
}

// DISHONORED(written): 2013 rva 0x6a12c0 (2012 0x6d1a90, identical bytes), retail vtable +348: after the base tick the four axes are
// kept for the next frame (m_fOld_aLookUp <- aLookUp, m_fOld_aTurn <- aTurn, m_fOld_aForward <- aForward, m_fOld_aStrafe <- aStrafe).
void UDishonoredPlayerInput::PlayerInput( FLOAT DeltaTime )
{
	UPlayerInput::PlayerInput( DeltaTime );
	m_fOld_aLookUp = aLookUp;
	m_fOld_aTurn = aTurn;
	m_fOld_aForward = aForward;
	m_fOld_aStrafe = aStrafe;
}

// DISHONORED(written): 2013 rva 0x6b84f0 (2012 0x6efbe0): merge one binding set into Bindings. A set entry whose Name and modifier
// bits match an existing binding appends its command behind a '|'; one whose modifiers are a strict superset replaces the existing
// entry; anything else is added. Retail compares the ten FKeyBind bits of the 2013 struct (Control..m_bIgnoreRMouse).
void UDishonoredPlayerInput::AddBindingSet( const TArrayNoInit<FKeyBind>& Set )
{
	for( INT SetIndex = 0; SetIndex < Set.Num(); SetIndex++ )
	{
		const FKeyBind& New = Set(SetIndex);
		UBOOL bHandled = FALSE;
		for( INT BindIndex = 0; BindIndex < Bindings.Num(); BindIndex++ )
		{
			FKeyBind& Bind = Bindings(BindIndex);
			if( Bind.Name != New.Name )
			{
				continue;
			}
			const UBOOL bSameModifiers =
				(UBOOL)Bind.Control == (UBOOL)New.Control && (UBOOL)Bind.Shift == (UBOOL)New.Shift &&
				(UBOOL)Bind.Alt == (UBOOL)New.Alt && (UBOOL)Bind.m_bLMouseHeld == (UBOOL)New.m_bLMouseHeld &&
				(UBOOL)Bind.m_bRMouseHeld == (UBOOL)New.m_bRMouseHeld && (UBOOL)Bind.bIgnoreCtrl == (UBOOL)New.bIgnoreCtrl &&
				(UBOOL)Bind.bIgnoreShift == (UBOOL)New.bIgnoreShift && (UBOOL)Bind.bIgnoreAlt == (UBOOL)New.bIgnoreAlt &&
				(UBOOL)Bind.m_bIgnoreLMouse == (UBOOL)New.m_bIgnoreLMouse && (UBOOL)Bind.m_bIgnoreRMouse == (UBOOL)New.m_bIgnoreRMouse;
			if( bSameModifiers )
			{
				Bind.Command += TEXT("|");
				Bind.Command += New.Command;
				bHandled = TRUE;
				break;
			}
			const UBOOL bNewCovers =
				( New.Control || !Bind.Control ) && ( New.Shift || !Bind.Shift ) && ( New.Alt || !Bind.Alt ) &&
				( New.m_bLMouseHeld || !Bind.m_bLMouseHeld ) && ( New.m_bRMouseHeld || !Bind.m_bRMouseHeld );
			if( bNewCovers )
			{
				Bind = New;
				bHandled = TRUE;
				break;
			}
		}
		if( !bHandled )
		{
			Bindings.AddItem( New );
		}
	}
}

// DISHONORED(written): the binding half of 2013 UDishonoredPlayerInput::ApplyGameSettings (0x6bd610): Bindings = BaseBindings, then
// AddBindingSet(m_PCBindings) and AddBindingSet(m_PadBindingSet[m_PadBindingSetMap[0]]) (TranslateBindingSet 0x6afe50 maps the
// gamepad-scheme setting through the tweak). Without it Bindings stays empty and no key reaches the player controller.
// DISHONORED(bringup): ReadPCBindingsFromProfile (0x6b8240, the user's rebinds), TranslateBaseBindings (0x6baa80, the non-US
// keyboard-layout remap) and InitGameActionBindings (0x6bac10, the UI glyph table) are not run: there is no profile yet and the
// settings interface ArkSettings::ApplyCurrentSettings (Engine, 0x53b790) is not in our Engine.
void UDishonoredPlayerInput::BuildBindings()
{
	Bindings = BaseBindings;
	AddBindingSet( m_PCBindings );
	UDisTweaks_PlayerInput* Tweaks = m_pTweaks ? m_pTweaks : (UDisTweaks_PlayerInput*)UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject();
	const INT PadSet = Tweaks ? Tweaks->m_PadBindingSetMap[0] : 0;
	switch( PadSet )
	{
	case 1:		AddBindingSet( m_PadBindingSet2 ); break;
	case 2:		AddBindingSet( m_PadBindingSet3 ); break;
	case 3:		AddBindingSet( m_PadBindingSet4 ); break;
	default:	AddBindingSet( m_PadBindingSet1 ); break;
	}
	debugf( TEXT("DISHONORED(bringup): player input bindings: %d base + %d pc + pad set %d -> %d bindings"),
		BaseBindings.Num(), m_PCBindings.Num(), PadSet, Bindings.Num() );
}

// DISHONORED(written): 2013 rva 0x5efd00 (vtable +360)
void UDishonoredPlayerInput::execDis_Jump_ButtonDown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	Dis_Jump_ButtonDown();
}

// DISHONORED(port): agent EQ, 2013 rva 0x6bd610 (2012 0x6f8e10, byte-identical - this body did not change
// between the builds). Two halves. The first is the binding half and retail runs it only for
// ASLI_ApplyCurrentValues and ASLI_ValidatedByUser (retail's test is `if (!Reason || Reason == 3)`): it
// translates the gamepad scheme setting, rebuilds Bindings out of BaseBindings and the two binding sets, and
// then hands the five movement/use keys to the GFx engine so the menus can draw their glyphs. The second half
// is unconditional and is the eleven values the input code reads - the two pad sensitivities, the three
// invert/friction/auto-aim bits per device, the two friction strengths, the mouse sensitivity modifier, and
// UPlayerInput::bEnableMouseSmoothing, which is bit 11 of the bitfield at +284.
// DISHONORED(bringup): the binding half is BuildBindings(), the stand-in agent DO wrote for retail's
// ReadPCBindingsFromProfile (0x6b8240) / TranslateBaseBindings (0x6baa80) / InitGameActionBindings (0x6bac10)
// chain, plus retail's FGFxEngine call for the five glyph keys, which this tree's GFxUI has no equivalent for.
void UDishonoredPlayerInput::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	if( Reason == ASLI_ApplyCurrentValues || Reason == ASLI_ValidatedByUser )
	{
		BuildBindings();
	}

	m_fGamepadLookXSensitivitySettings = (FLOAT)Parameters->m_GamepadLookXSensitivity * 0.01f;
	m_fGamepadLookYSensitivitySettings = (FLOAT)Parameters->m_GamepadLookYSensitivity * 0.01f;
	m_bGamepadInvertYAxisSettings = Parameters->m_bGamepadInvertY ? TRUE : FALSE;
	m_bGamepadUseFrictionSettings = Parameters->m_bGamepadFriction ? TRUE : FALSE;
	m_bGamepadUseAutoAimSettings = Parameters->m_bGamepadAutoAim ? TRUE : FALSE;
	m_fGamepadFrictionStrengthSettings = (FLOAT)Parameters->m_GamepadFrictionStrength * 0.01f;
	m_fMouseSensitivityModifierSetting = Parameters->m_fMouseSensitivity;
	m_bMouseInvertYAxisSettings = Parameters->m_bMouseInvertY ? TRUE : FALSE;
	m_bMouseUseFrictionSettings = Parameters->m_bMouseFriction ? TRUE : FALSE;
	m_fMouseFrictionStrengthSettings = (FLOAT)Parameters->m_MouseFrictionStrength * 0.01f;
	m_bMouseUseAutoAimSettings = Parameters->m_bMouseAutoAim ? TRUE : FALSE;
	bEnableMouseSmoothing = Parameters->m_bMouseSmoothing ? TRUE : FALSE;
}
