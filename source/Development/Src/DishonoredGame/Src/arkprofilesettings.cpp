// DishonoredGame/src/arkprofilesettings.cpp
// Engine.ArkProfileSettings is an Engine-package shim declared by the generated DishonoredGameEngineShims.h; retail keeps its code
// in Engine/src/arksettings.cpp (2012 PDB).

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x1cd3a0 = UOnlinePlayerStorage::execSetToDefaults (one body in the retail native table for four
// classes); it reaches UArkProfileSettings::SetToDefaults below through the virtual.
void UArkProfileSettings::execSetToDefaults( FFrame& Stack, RESULT_DECL )
{
	UOnlinePlayerStorage::execSetToDefaults( Stack, Result );
}

// DISHONORED(port): agent HJ, 2013 rva 0x537610 (2012 0x5786c0): the bindable actions, EProfileSettingID 29..64 without their PSI_
// prefix (2012 stopped at 62), in id order - index i is setting 29 + i.
void UArkProfileSettings::FillGBAList( TArray<FString>& _rOutGBAs )
{
	for( INT SettingID = 29; SettingID < 65; SettingID++ )
	{
		_rOutGBAs.AddItem( DisEnumTypeToString( SettingID, TEXT("Engine.OnlineProfileSettings.EProfileSettingID") ).Replace( TEXT("PSI_"), TEXT("") ) );
	}
}

// DISHONORED(port): agent HJ, 2013 rva 0x53b8c0 (2012 0x57c1b0). The profile with nothing read is UOnlineProfileSettings' defaults plus
// one key per bindable action, read off the PlayerInput class default's m_PCBindings ([Engine.PlayerInput] in DishonoredInput.ini):
// the first binding whose command (each '|' part trimmed in place) names the action gives its key's index in m_BindableKeyMapping, and
// a key that is not bindable (index 0) writes nothing. Without it every action read 0 = None, and ReadPCBindingsFromProfile unbound
// the whole keyboard.
// DISHONORED(bringup): two retail branches are not here. On a keyboard layout other than US English retail maps each key's US
// virtual key through MapVirtualKeyEx to the user's layout and uses that key when it is bindable (UWindowsClient::KeyMapNameToVirtual
// / KeyMapVirtualToName, FindBindableKey); and 2013 ends by writing ids 119..123 from the system settings it detected once at
// startup (six cached globals at 2013 0x1458b10..0x1458b24).
void UArkProfileSettings::SetToDefaults()
{
	UOnlineProfileSettings::SetToDefaults();
	const UPlayerInput* DefaultInput = (UPlayerInput*)UPlayerInput::StaticClass()->GetDefaultObject();
	TArray<FString> GBAs;
	FillGBAList( GBAs );
	for( INT BindingIdx = 0; BindingIdx < DefaultInput->m_PCBindings.Num(); BindingIdx++ )
	{
		const FKeyBind& KeyBind = DefaultInput->m_PCBindings(BindingIdx);
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
			INT BindableKey = 0;
			for( INT KeyIdx = 0; KeyIdx < ARRAY_COUNT(m_BindableKeyMapping); KeyIdx++ )
			{
				if( m_BindableKeyMapping[KeyIdx] == KeyBind.Name )
				{
					BindableKey = KeyIdx;
					break;
				}
			}
			if( BindableKey != 0 )
			{
				SetProfileSettingValueInt( GBAIndex + 29, BindableKey );
			}
			GBAs(GBAIndex).Empty();
		}
	}
}
