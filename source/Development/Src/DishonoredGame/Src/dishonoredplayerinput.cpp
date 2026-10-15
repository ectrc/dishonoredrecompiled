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
	// DISHONORED(port): agent HJ. Retail's last statement (2013 0x6afeb0 -> ArkSettings::ApplyCurrentSettings 0x53b790): the current
	// settings reach ApplyGameSettings with ASLI_ApplyCurrentValues, which builds the bindings out of the profile.
	ArkSettings::ApplyCurrentSettings( this );
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

// DISHONORED(port): 2013 rva 0x6afe50 (2012 0x6f8db0): the gamepad-scheme setting through the tweak's m_PadBindingSetMap, the class
// default's when the tweak is not loaded.
INT UDishonoredPlayerInput::TranslateBindingSet( INT _BindingSet ) const
{
	const UDisTweaks_PlayerInput* Tweaks = m_pTweaks ? m_pTweaks : (UDisTweaks_PlayerInput*)UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject();
	return Tweaks->m_PadBindingSetMap[_BindingSet];
}

// DISHONORED(port): agent HJ, 2013 rva 0x6b8240 (2012 0x6ef8f0; 2013 reads ids 29..64, 2012 29..62). The profile owns the PC keys:
// every m_PCBindings command part (trimmed in place, as FString::Trim/TrimTrailing do) that names a bindable action - the
// EProfileSettingID names 29..64 without their PSI_ prefix, UArkProfileSettings::FillGBAList 2013 0x537610 - takes the key the
// profile stores for that action (GetProfileSettingValueInt(29 + action), UArkProfileSettings::GetKeyName 2013 0x533560 =
// m_BindableKeyMapping[key]) as the binding's key. Each action is consumed by the first binding that names it, so a second binding
// of the same action keeps its ini key. A setting the profile does not hold leaves the binding's ini key.
void UDishonoredPlayerInput::ReadPCBindingsFromProfile()
{
	ADishonoredPlayerController* Controller = Cast<ADishonoredPlayerController>( GetOuter() );
	// DISHONORED(bringup): retail calls the controller's GetProfileSettings (vtable +1424) unguarded; this tree can run the input
	// before the controller's online data store is registered.
	if( Controller == NULL || Controller->OnlinePlayerData == NULL || Controller->OnlinePlayerData->ProfileProvider == NULL )
	{
		return;
	}
	UArkProfileSettings* Profile = Controller->GetProfileSettings();
	if( Profile == NULL )
	{
		return;
	}
	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("puidiag") );
	TArray<FString> GBAs;
	UArkProfileSettings::FillGBAList( GBAs );
	for( INT BindingIdx = 0; BindingIdx < m_PCBindings.Num(); BindingIdx++ )
	{
		FKeyBind& KeyBind = m_PCBindings(BindingIdx);
		TArray<FString> Commands;
		KeyBind.Command.ParseIntoArray( &Commands, TEXT("|"), TRUE );
		for( INT CommandIdx = 0; CommandIdx < Commands.Num(); CommandIdx++ )
		{
			FString& Command = Commands(CommandIdx);
			Command.Trim();
			Command.TrimTrailing();
			if( !Command.StartsWith( TEXT("GBA_") ) )
			{
				continue;
			}
			const INT GBAIndex = GBAs.FindItemIndex( Command );
			if( GBAIndex == INDEX_NONE )
			{
				continue;
			}
			INT BindableKey = -1;
			// DISHONORED(bringup): the key index is range-checked; retail indexes m_BindableKeyMapping with whatever the profile holds.
			if( Profile->GetProfileSettingValueInt( GBAIndex + 29, BindableKey ) && BindableKey >= 0 && BindableKey < ARRAY_COUNT(Profile->m_BindableKeyMapping) )
			{
				const FName OldKey = KeyBind.Name;
				KeyBind.Name = Profile->GetKeyName( BindableKey );
				if( bDiag )
				{
					debugf( TEXT("DISHONORED(bringup): puidiag profile key %s: id %d value %d, %s -> %s"), *Command, GBAIndex + 29, BindableKey,
						*OldKey.ToString(), *KeyBind.Name.ToString() );
				}
			}
			else if( bDiag )
			{
				debugf( TEXT("DISHONORED(bringup): puidiag profile key %s: id %d not held, keeps %s"), *Command, GBAIndex + 29, *KeyBind.Name.ToString() );
			}
			GBAs(GBAIndex).Empty();
		}
	}
}

// DISHONORED(port): 2013 rva 0x6bac10 (2012 0x6f3fe0, same logic), which retail's ApplyGameSettings runs right after the two
// AddBindingSet calls (agent HF). Every binding named GBA_<action> becomes an action entry whose PC and pad keys are the first
// binding of each kind with a '|'-separated command part equal to that name; then each <action>_Gamepad entry hands its pad
// key to <action>. Each part is trimmed first: FString::Trim/TrimTrailing trim the string in place and return a copy, which
// is the copy retail discards (agent HJ; agent HF's reading compared "GBA_Use " and never matched F's "GBA_Use | ...").
void UDishonoredPlayerInput::InitGameActionBindings()
{
	m_GameActionBindings.Reset();
	for( INT BindIndex = 0; BindIndex < Bindings.Num(); BindIndex++ )
	{
		const FName ActionName = Bindings(BindIndex).Name;
		const FString GBAName = ActionName.ToString();
		if( !GBAName.StartsWith( TEXT("GBA_") ) )
		{
			continue;
		}
		FDisGameActionBinding GBA( EC_EventParm );
		GBA.m_ActionName = ActionName;
		for( INT Other = 0; Other < Bindings.Num(); Other++ )
		{
			const FKeyBind& Candidate = Bindings(Other);
			TArray<FString> Parts;
			Candidate.Command.ParseIntoArray( &Parts, TEXT("|"), TRUE );
			for( INT Part = 0; Part < Parts.Num(); Part++ )
			{
				Parts(Part).Trim();
				Parts(Part).TrimTrailing();
				if( Parts(Part) != GBAName )
				{
					continue;
				}
				const UBOOL bGamepadKey = IsGamepadKey( Candidate.Name );
				if( GBA.m_KeyBind_PC.Name == NAME_None && !bGamepadKey )
				{
					GBA.m_KeyBind_PC = Candidate;
					break;
				}
				if( GBA.m_KeyBind_Gamepad.Name == NAME_None && bGamepadKey )
				{
					GBA.m_KeyBind_Gamepad = Candidate;
					break;
				}
			}
			if( GBA.m_KeyBind_PC.Name != NAME_None && GBA.m_KeyBind_Gamepad.Name != NAME_None )
			{
				break;
			}
		}
		m_GameActionBindings.AddItem( GBA );
	}
	const INT NumActions = m_GameActionBindings.Num();
	for( INT ActionIndex = 0; ActionIndex < NumActions; ActionIndex++ )
	{
		const FString GBAName = m_GameActionBindings(ActionIndex).m_ActionName.ToString();
		if( !GBAName.EndsWith( TEXT("_Gamepad") ) )
		{
			continue;
		}
		const FName BaseName( *GBAName.Replace( TEXT("_Gamepad"), TEXT("") ) );
		for( INT Base = 0; Base < NumActions; Base++ )
		{
			if( m_GameActionBindings(Base).m_ActionName == BaseName )
			{
				m_GameActionBindings(Base).m_KeyBind_Gamepad = m_GameActionBindings(ActionIndex).m_KeyBind_Gamepad;
				break;
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x6a7870 (2012 0x6e4100). The pad key when the player is on a pad or the pad is forced, unless
// PC is forced.
const FKeyBind* UDishonoredPlayerInput::GetKeyBindFromAction( FName _ActionName, UBOOL _bForcePC, UBOOL _bForceGamepad ) const
{
	for( INT Index = 0; Index < m_GameActionBindings.Num(); Index++ )
	{
		const FDisGameActionBinding& Binding = m_GameActionBindings(Index);
		if( Binding.m_ActionName == _ActionName )
		{
			return ( ( bUsingGamepad || _bForceGamepad ) && !_bForcePC ) ? &Binding.m_KeyBind_Gamepad : &Binding.m_KeyBind_PC;
		}
	}
	return NULL;
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
// DISHONORED(port): agent HJ, the binding half in retail's order: TranslateBindingSet, ReadPCBindingsFromProfile, Bindings =
// BaseBindings, AddBindingSet(m_PCBindings), AddBindingSet(the translated pad set), InitGameActionBindings.
// DISHONORED(bringup): TranslateBaseBindings (0x6baa80, run when the flag at +108 is clear: BaseBindings' keys remapped from the
// US layout to the user's through MapVirtualKeyEx) and the FGFxEngine call that hands the five movement/use keys to the GFx
// engine (0x9a0f30) have no equivalent in this tree.
void UDishonoredPlayerInput::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	if( Reason == ASLI_ApplyCurrentValues || Reason == ASLI_ValidatedByUser )
	{
		const INT PadSet = TranslateBindingSet( Parameters->m_GamepadBindingSet );
		ReadPCBindingsFromProfile();
		Bindings = BaseBindings;
		AddBindingSet( m_PCBindings );
		switch( PadSet )
		{
		case 1:		AddBindingSet( m_PadBindingSet2 ); break;
		case 2:		AddBindingSet( m_PadBindingSet3 ); break;
		case 3:		AddBindingSet( m_PadBindingSet4 ); break;
		default:	AddBindingSet( m_PadBindingSet1 ); break;
		}
		InitGameActionBindings();
		debugf( TEXT("DISHONORED(bringup): player input bindings: %d base + %d pc + pad set %d -> %d bindings, %d game actions"),
			BaseBindings.Num(), m_PCBindings.Num(), PadSet, Bindings.Num(), m_GameActionBindings.Num() );
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
