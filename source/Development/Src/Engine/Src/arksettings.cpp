// Engine/src/arksettings.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (27):
//   0x574de0  public: static void __cdecl UArkSettingsListenerInterface::InitializePrivateStaticClassUArkSettingsListenerInterface(void)
//   0x574e00  public: static void __cdecl UArkProfileSettings::InitializePrivateStaticClassUArkProfileSettings(void)
//   0x574e20  public: static void __cdecl ArkSettings::SaveSettings(class APlayerController *)
//   0x574ea0  public: class FName __thiscall UArkProfileSettings::GetKeyName(enum EBindableKey)const
//   0x574ed0  public: enum EBindableKey __thiscall UArkProfileSettings::FindBindableKey(class FName)const
//   0x575500  public: static class UClass * __cdecl UArkSettingsListenerInterface::GetPrivateStaticClassUArkSettingsListenerInterface(wchar_t const *)
//   0x576b30  public: static class UClass * __cdecl UArkSettingsListenerInterface::StaticClassNoInline(void)
//   0x578330  public: virtual int __thiscall PCResolutionSettingProvider::GetInnerValue(int, int)
//   0x578420  public: virtual void __thiscall PCResolutionSettingProvider::ReadFromSystemSettings(void)
//   0x5784a0  public: virtual __thiscall PCResolutionSettingProvider::~PCResolutionSettingProvider(void)
//   0x578500  public: virtual void __thiscall PCResolutionSettingProvider::GetDynamicValueNames(class TArray<class FString, class FDefaultAllocator> &)
//   0x5785e0  public: virtual void __thiscall PCResolutionSettingProvider::Refresh(void)
//   0x5786c0  public: static void __cdecl UArkProfileSettings::FillGBAList(class TArray<class FString, class FDefaultAllocator> &)
//   0x578790  public: void __thiscall UArkProfileSettings::SetMissionData(int, int, int, float const *, int)
//   0x5788e0  public: void __thiscall UArkProfileSettings::GetMissionData(int, int *, int *, float *, int)const
//   0x578a50  public: void __thiscall UArkProfileSettings::GetFinishedMissionIndices(class TArray<int, class FDefaultAllocator> &)const
//   0x57a0e0  public: static void __cdecl ArkSettings::FindListeners(class TArray<class TScriptInterface<class IArkSettingsListenerInterface>, class FDefaultAllocator> &)
//   0x57a1d0  public: static class ArkSettings::SettingProvider & __cdecl ArkSettings::GetSettingProvider(int)
//   0x57c1b0  public: virtual void __thiscall UArkProfileSettings::SetToDefaults(void)
//   0x57de30  public: static class UClass * __cdecl UArkProfileSettings::GetPrivateStaticClassUArkProfileSettings(wchar_t const *)
//   0x57e1c0  public: static class UClass * __cdecl UArkProfileSettings::StaticClassNoInline(void)
//   0x57e580  private: static class ArkSettingsParameters & __cdecl ArkSettings::GetParameters(void)
//   0x57e5e0  public: static void __cdecl ArkSettings::ApplyCurrentSettings(class IArkSettingsListenerInterface *)
//   0x57e600  public: static void __cdecl ArkSettings::UpdateSettingsFromSystemSettings(class APlayerController *)
//   0x57e630  public: static void __cdecl ArkSettings::OnSettingsChanged(class UOnlinePlayerStorage *, class TArray<class TScriptInterface<class IArkSettingsListenerInterface>, class FDefaultAllocator> &, enum ArkSettings::EChangeReason)
//   0x57e7f0  public: static void __cdecl ArkSettings::ResetSettings(class APlayerController *, class TArray<int, class FDefaultAllocator>)
//   0x57eb60  public: static void __cdecl ArkSettings::ResetAllSettings(class APlayerController *)

#include "EnginePrivate.h"
// DISHONORED(written): agent EQ. SaveSettings and UpdateSettingsFromSystemSettings reach the profile through
// APlayerController::OnlinePlayerData, whose type and its ProfileProvider are declared in the UI private
// header, exactly as UnUIDataStores.cpp includes it.
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

// DISHONORED(port): Engine.ArkSettingsListenerInterface, 2013 GetPrivateStaticClass rva 0x533b00 / StaticClassNoInline 0x576b30 (native_class_sizes.csv: 56 bytes, flags 0x10004001)
IMPLEMENT_CLASS(UArkSettingsListenerInterface);

// DISHONORED(port): agent EQ. PCResolutionSettingProvider, the one provider retail has a subclass for
// (vftable 2013 rva 0xca44f4; GetInnerValue 0x537260, ReadFromSystemSettings 0x537350, the destructor 0x537400,
// GetDynamicValueNames 0x537460, Refresh 0x537520). Retail keeps it in this unit with no declaration in a
// header, which is why the PDB attributes only its vtable and its five bodies here. m_Resolutions is the
// TArray<FScreenResolutionRHI> the RHI fills and m_CurrentIndex the row the options list shows.
class PCResolutionSettingProvider : public ArkSettings::SettingProvider
{
public:
	FScreenResolutionArray m_Resolutions;
	INT m_CurrentIndex;

	PCResolutionSettingProvider() : m_CurrentIndex( 0 ) {}

	// DISHONORED(port): 2013 rva 0x537460. "%4d x %4d" is retail's own format string, spaces and all.
	virtual void GetDynamicValueNames( TArray<FString>& OutNames )
	{
		for( INT Index = 0; Index < m_Resolutions.Num(); Index++ )
		{
			OutNames.AddItem( FString::Printf( TEXT("%4d x %4d"), m_Resolutions(Index).Width, m_Resolutions(Index).Height ) );
		}
	}

	virtual INT GetCurrentValueIndex() { return m_CurrentIndex; }
	virtual void SetCurrentValueIndex( INT ValueIndex ) { m_CurrentIndex = ValueIndex; }

	// DISHONORED(port): 2013 rva 0x537260. Component 0 is the width, 1 the height, 2 the refresh rate.
	virtual INT GetInnerValue( INT ValueIndex, INT Component )
	{
		if( !m_Resolutions.IsValidIndex( ValueIndex ) )
		{
			return 0;
		}
		const FScreenResolutionRHI& Resolution = m_Resolutions(ValueIndex);
		switch( Component )
		{
		case 0:		return (INT)Resolution.Width;
		case 1:		return (INT)Resolution.Height;
		case 2:		return (INT)Resolution.RefreshRate;
		default:	return 0;
		}
	}

	// DISHONORED(port): 2013 rva 0x537350. The row whose width and height are the ones in force, or the first row.
	virtual void ReadFromSystemSettings()
	{
		m_CurrentIndex = -1;
		for( INT Index = 0; Index < m_Resolutions.Num(); Index++ )
		{
			if( (INT)m_Resolutions(Index).Width == GSystemSettings.ResX && (INT)m_Resolutions(Index).Height == GSystemSettings.ResY )
			{
				m_CurrentIndex = Index;
			}
		}
		if( m_CurrentIndex == -1 )
		{
			m_CurrentIndex = 0;
		}
	}

	// DISHONORED(port): 2013 rva 0x537520. Retail empties the array, asks the RHI for the modes with the refresh
	// rate ignored, and then walks the result backwards dropping everything below 800x600.
	virtual void Refresh()
	{
		m_Resolutions.Empty();
		if( RHIGetAvailableResolutions( m_Resolutions, TRUE ) )
		{
			for( INT Index = m_Resolutions.Num() - 1; Index >= 0; Index-- )
			{
				if( m_Resolutions(Index).Width < 800 || m_Resolutions(Index).Height < 600 )
				{
					m_Resolutions.Remove( Index, 1 );
				}
			}
		}
	}
};

// DISHONORED(port): agent EQ. The provider every other setting gets. Retail's type name is
// InvalidDynamicSettingProvider (match_2012_2013.csv, vftable 2013 rva 0xca3f08); it adds nothing, which is
// why all six of its slots are ICF folds onto the base's.
class InvalidDynamicSettingProvider : public ArkSettings::SettingProvider
{
};

// DISHONORED(port): 2013 rva 0x5396a0 (2012 0x57a1d0). Two function-statics, and the resolution one is
// Refresh()ed on every call - which is what keeps the mode list current after a display change. Retail
// inlines this whole body into ArkSettingsParameters::Read as well, which is how the same static is reached
// from two places.
ArkSettings::SettingProvider& ArkSettings::GetSettingProvider( INT SettingID )
{
	if( SettingID == 115 )
	{
		static PCResolutionSettingProvider ResolutionProvider;
		ResolutionProvider.Refresh();
		return ResolutionProvider;
	}
	static InvalidDynamicSettingProvider InvalidProvider;
	return InvalidProvider;
}

// DISHONORED(port): 2013 rva 0x53b450 (2012 0x57e3c0, arksettingsparameters.cpp:14): a transient UArkProfileSettings set to its defaults
// is read with the system-settings override. UArkProfileSettings is an Engine class the DishonoredGame module still declares (shim), so
// it is found by name; without it (Engine-only builds) the plain UOnlinePlayerStorage defaults are read.
ArkSettingsParameters::ArkSettingsParameters()
{
	UClass* ProfileClass = FindObject<UClass>( ANY_PACKAGE, TEXT("ArkProfileSettings") );
	if( !ProfileClass )
	{
		ProfileClass = UOnlinePlayerStorage::StaticClass();
	}
	UOnlinePlayerStorage* Defaults = ConstructObject<UOnlinePlayerStorage>( ProfileClass, UObject::GetTransientPackage() );
	Defaults->SetToDefaults();
	Read( Defaults, TRUE );
}

// DISHONORED(port): 2013 rva 0x539730 (2012 0x57a260, arksettingsparameters.cpp:21). Every id below is
// retail 2013's own, read off the decompile and named from 2013's Engine.OnlineProfileSettings.EProfileSettingID
// (155 values; the 2012 enum has 133 and renumbers 80 of them, so no 2012 id carries over). The accessors are
// retail's too, resolved by hand out of the 2013 UArkProfileSettings vtable at 0xca3d58: +332
// GetProfileSettingValueId (0x4f36b0), +348 SetProfileSettingValueId (0x4f3ac0), +376
// SetRangedProfileSettingValueInt (0x4f75d0), +384 GetRangedProfileSettingValueFloat (0x4f77e0), +388
// GetRangedProfileSettingValueInt (0x4f7860). Every slider and volume is a PVMT_Ranged mapping, which the plain
// GetProfileSettingValueInt/Float reject outright, so reading them through the ranged accessors is what makes
// them readable at all - the gamma among them. The resolution comes from the static PCResolutionSettingProvider
// (not ported): with the override it is GSystemSettings like retail, without it retail asks the provider's
// current value, which is GSystemSettings as well after Refresh().
void ArkSettingsParameters::Read( UOnlinePlayerStorage* Settings, UBOOL bOverrideStorageSettingsWithSystemSettings )
{
	INT Value = 0;
	Settings->GetRangedProfileSettingValueFloat( 67, m_fMouseSensitivity );
	Settings->GetProfileSettingValueId( 68, Value ); m_bMouseSmoothing = Value == 1;
	Settings->GetProfileSettingValueId( 69, Value ); m_bMouseInvertY = Value == 1;
	Settings->GetProfileSettingValueId( 70, Value ); m_bMouseAutoAim = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 71, m_MouseAutoAimStrength );
	Settings->GetProfileSettingValueId( 72, Value ); m_bMouseFriction = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 73, m_MouseFrictionStrength );
	Settings->GetProfileSettingValueId( 76, m_GamepadBindingSet );
	Settings->GetProfileSettingValueId( 77, Value ); m_bGamepadVibration = Value == 1;
	// DISHONORED(port): both pad look sensitivities fall back to the Live-standard PSI_ControllerSensitivity
	// (id 13), whose low/medium/high ids land on 0/20/60 here, and the resolved value is written back so the
	// fallback is taken once.
	Settings->GetRangedProfileSettingValueInt( 78, m_GamepadLookXSensitivity );
	if( m_GamepadLookXSensitivity == -1 )
	{
		Settings->GetProfileSettingValueId( 13, Value );
		m_GamepadLookXSensitivity = Value == 1 ? 0 : ( Value == 2 ? 60 : 20 );
		Settings->SetRangedProfileSettingValueInt( 78, m_GamepadLookXSensitivity );
	}
	Settings->GetRangedProfileSettingValueInt( 79, m_GamepadLookYSensitivity );
	if( m_GamepadLookYSensitivity == -1 )
	{
		Settings->GetProfileSettingValueId( 13, Value );
		m_GamepadLookYSensitivity = Value == 1 ? 0 : ( Value == 2 ? 60 : 20 );
		Settings->SetRangedProfileSettingValueInt( 79, m_GamepadLookYSensitivity );
	}
	Settings->GetProfileSettingValueId( 80, Value );
	m_bGamepadInvertY = Value == 1;
	if( Value == -1 )
	{
		Settings->GetProfileSettingValueId( 2, Value );
		m_bGamepadInvertY = Value == 1;
		Settings->SetProfileSettingValueId( 80, m_bGamepadInvertY );
	}
	Settings->GetProfileSettingValueId( 81, Value );
	m_bGamepadAutoAim = Value == 1;
	if( Value == -1 )
	{
		Settings->GetProfileSettingValueId( 16, Value );
		m_bGamepadAutoAim = Value == 1;
		Settings->SetProfileSettingValueId( 81, m_bGamepadAutoAim );
	}
	Settings->GetRangedProfileSettingValueInt( 82, m_GamepadAutoAimStrength );
	Settings->GetProfileSettingValueId( 83, Value ); m_bGamepadFriction = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 84, m_GamepadFrictionStrength );
	Settings->GetProfileSettingValueId( 87, m_HUDVisibility );
	Settings->GetProfileSettingValueId( 88, Value ); m_bShowObjectivePopups = Value == 1;
	Settings->GetProfileSettingValueId( 89, Value ); m_bShowTutorialNotifications = Value == 1;
	Settings->GetProfileSettingValueId( 90, Value ); m_bShowInteractions = Value == 1;
	Settings->GetProfileSettingValueId( 91, Value ); m_bShowHighlight = Value == 1;
	Settings->GetProfileSettingValueId( 92, Value ); m_bShowPickupLog = Value == 1;
	Settings->GetProfileSettingValueId( 93, Value ); m_bShowContextualIcons = Value == 1;
	Settings->GetProfileSettingValueId( 94, Value ); m_bShowPlayerStance = Value == 1;
	Settings->GetProfileSettingValueId( 95, Value ); m_bShowObjectiveMarkers = Value == 1;
	Settings->GetProfileSettingValueId( 96, Value ); m_bShowGrenadeMarkers = Value == 1;
	Settings->GetProfileSettingValueId( 97, Value ); m_bShowAwarenessMarkers = Value == 1;
	Settings->GetProfileSettingValueId( 98, Value ); m_bShowHeartTargetMarkers = Value == 1;
	Settings->GetProfileSettingValueId( 99, m_CrosshairStyle );
	Settings->GetProfileSettingValueId( 100, Value ); m_bCrosshairMovement = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 101, m_CrosshairOpacity );
	Settings->GetProfileSettingValueId( 104, Value ); m_bAutoUseManaElixir = Value == 1;
	Settings->GetProfileSettingValueId( 105, m_KillCamMode );
	// DISHONORED(port): the campaign difficulty falls back to the Live-standard PSI_GameDifficulty (id 12),
	// whose three ids map onto EDifficulty_Easy/Normal/Hard (0/1/2), and the DLC06 difficulty falls back to the
	// campaign's own clamped to EDifficulty_Hard. EDifficulty and ESubtitlesMode are ArkProfileSettings script
	// enums that only the DishonoredGame shim header declares, so the Engine spells their values out.
	Settings->GetProfileSettingValueId( 106, m_Difficulty );
	if( m_Difficulty == -1 )
	{
		Settings->GetProfileSettingValueId( 12, Value );
		m_Difficulty = Value == 1 ? 0 : ( Value == 2 ? 2 : 1 );
		Settings->SetProfileSettingValueId( 106, m_Difficulty );
	}
	Settings->GetProfileSettingValueId( 152, m_DifficultyDLC06 );
	if( m_DifficultyDLC06 == -1 )
	{
		m_DifficultyDLC06 = Min<INT>( m_Difficulty, 2 );
		Settings->SetProfileSettingValueId( 152, m_DifficultyDLC06 );
	}
	Settings->GetProfileSettingValueId( 107, Value ); m_bAutoSaveInMenu = Value == 1;
	Settings->GetRangedProfileSettingValueFloat( 108, m_fHeadBobAmount );
	Settings->GetProfileSettingValueId( 109, Value ); m_bCameraRelativeClimbing = Value == 1;
	Settings->GetRangedProfileSettingValueFloat( 112, m_fGamma );
	// DISHONORED(port): agent EQ. Retail's resolution arm, which is where the inlined GetSettingProvider(115)
	// sits: with the system-settings override the provider is told to look the current mode up and the
	// parameters take GSystemSettings' own width and height; without it they take the width and height of the
	// row the provider currently points at, which is the row the options list last wrote.
	{
		ArkSettings::SettingProvider& Provider = ArkSettings::GetSettingProvider( 115 );
		if( bOverrideStorageSettingsWithSystemSettings )
		{
			Provider.ReadFromSystemSettings();
			m_ResX = GSystemSettings.ResX;
			m_ResY = GSystemSettings.ResY;
			// DISHONORED(bringup): agent EQ's census, once per run. The resolution row's whole content - the
			// value list the options screen shows and the row it points at - is this provider, so one line
			// here is the record that it produces both, on retail's own path and with nothing forced.
			static UBOOL bResolutionCensus = FALSE;
			if( !bResolutionCensus )
			{
				bResolutionCensus = TRUE;
				TArray<FString> Names;
				Provider.GetDynamicValueNames( Names );
				FString List;
				for( INT Index = 0; Index < Names.Num(); Index++ )
				{
					List += ( Index ? TEXT(", ") : TEXT("") );
					List += Names(Index);
				}
				debugf( TEXT("ArkSettings::GetSettingProvider(115): %d modes, current %d, [%s]"),
					Names.Num(), Provider.GetCurrentValueIndex(), *List );
			}
		}
		else
		{
			m_ResX = Provider.GetInnerValue( Provider.GetCurrentValueIndex(), 0 );
			m_ResY = Provider.GetInnerValue( Provider.GetCurrentValueIndex(), 1 );
		}
	}
	Settings->GetProfileSettingValueId( 116, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.bFullscreen != 0;
		Settings->SetProfileSettingValueId( 116, Value );
	}
	m_bFullscreen = Value == 1;
	Settings->GetProfileSettingValueId( 117, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.bUseVSync != 0;
		Settings->SetProfileSettingValueId( 117, Value );
	}
	m_bVSync = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 118, m_FOV );
	Settings->GetProfileSettingValueId( 119, m_TextureDetails );
	Settings->GetProfileSettingValueId( 120, m_ModelDetails );
	Settings->GetProfileSettingValueId( 121, Value ); m_bLightShaftEnable = Value == 1;
	Settings->GetProfileSettingValueId( 122, m_AntiAliasingMode );
	Settings->GetProfileSettingValueId( 123, Value ); m_bRatShadows = Value == 1;
	Settings->GetRangedProfileSettingValueInt( 126, m_GlobalVolume );
	Settings->GetRangedProfileSettingValueInt( 127, m_MusicVolume );
	Settings->GetRangedProfileSettingValueInt( 128, m_SFXVolume );
	Settings->GetRangedProfileSettingValueInt( 129, m_VoicesVolume );
	// DISHONORED(port): an unset subtitle mode defaults to SubtitlesMode_All (2) for the four languages
	// Dishonored ships without localised speech, and to SubtitlesMode_Off (0) for the rest.
	Settings->GetProfileSettingValueId( 130, m_SubtitlesMode );
	if( m_SubtitlesMode == -1 )
	{
		const FString Language = appGetLanguageExt();
		m_SubtitlesMode = ( Language == TEXT("RUS") || Language == TEXT("CZE") || Language == TEXT("HUN") || Language == TEXT("POL") )
			? 2
			: 0;
		Settings->SetProfileSettingValueId( 130, m_SubtitlesMode );
	}
	Settings->GetProfileSettingValueId( 133, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.SpeakerConfiguration;
		Settings->SetProfileSettingValueId( 133, Value );
	}
	m_SpeakerConfiguration = Value;
}

// DISHONORED(port): 2013 rva 0x53b730 (2012 0x57e580, identical): function-static instance, built on first use
ArkSettingsParameters& ArkSettings::GetParameters()
{
	static ArkSettingsParameters s_Parameters;
	return s_Parameters;
}

// DISHONORED(port): 2013 rva 0x53b790 (2012 0x57e5e0, identical)
void ArkSettings::ApplyCurrentSettings( IArkSettingsListenerInterface* Listener )
{
	Listener->ApplyGameSettings( &GetParameters(), IArkSettingsListenerInterface::ASLI_ApplyCurrentValues );
}

// DISHONORED(port): 2013 rva 0x539580 (2012 0x57a0e0, arksettings.cpp): every live UObject that implements
// Engine.ArkSettingsListenerInterface, skipping the ones flagged 0x200 (RF_Unreachable).
void ArkSettings::FindListeners( TArray<TScriptInterface<IArkSettingsListenerInterface> >& OutListeners )
{
	for ( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		if ( Object->HasAnyFlags( RF_Unreachable ) || Object->GetInterfaceAddress( UArkSettingsListenerInterface::StaticClass() ) == NULL )
		{
			continue;
		}
		TScriptInterface<IArkSettingsListenerInterface> Entry;
		Entry.SetObject( Object );
		Entry.SetInterface( Object->GetInterfaceAddress( UArkSettingsListenerInterface::StaticClass() ) );
		OutListeners.AddItem( Entry );
	}
}

// DISHONORED(port): 2013 rva 0x53b7e0 (2012 0x57e630): rereads the shared parameters out of the storage
// object - with the system-settings override only when the profile was just read - hands them to every
// listener and republishes the vibration setting through GEnableForceFeedback.
void ArkSettings::OnSettingsChanged( UOnlinePlayerStorage* Settings, TArray<TScriptInterface<IArkSettingsListenerInterface> >& Listeners, EChangeReason Reason )
{
	IArkSettingsListenerInterface::EChangeReason ListenerReason = IArkSettingsListenerInterface::ASLI_ReadProfileFromStorage;
	if ( Reason == ECR_ModifiedByUser )
	{
		ListenerReason = IArkSettingsListenerInterface::ASLI_ModifiedByUser;
	}
	else if ( Reason == ECR_ValidatedByUser )
	{
		ListenerReason = IArkSettingsListenerInterface::ASLI_ValidatedByUser;
	}

	ArkSettingsParameters& Parameters = GetParameters();
	Parameters.Read( Settings, ListenerReason == IArkSettingsListenerInterface::ASLI_ReadProfileFromStorage );

	INT Published = 0;
	for ( INT Index = 0; Index < Listeners.Num(); Index++ )
	{
		IArkSettingsListenerInterface* Listener = (IArkSettingsListenerInterface*)Listeners( Index ).GetInterface();
		if ( Listener != NULL )
		{
			Listener->ApplyGameSettings( &Parameters, ListenerReason );
			Published++;
		}
	}
	// DISHONORED(bringup): agent EQ's census. The loop has no early exit, so a line here is the record that
	// every listener the caller collected was published to and that none of them aborted the run.
	debugf( TEXT("ArkSettings::OnSettingsChanged: reason %d -> %d/%d listeners, gamma %.4f, res %dx%d, AA %d"),
		(INT)ListenerReason, Published, Listeners.Num(), Parameters.m_fGamma,
		Parameters.m_ResX, Parameters.m_ResY, Parameters.m_AntiAliasingMode );

	// DISHONORED(bringup): retail ends with GEnableForceFeedback = Parameters.m_bGamepadVibration; that
	// global does not exist in this tree (WinDrv drives force feedback through UForceFeedbackManager).
}

// DISHONORED(port): 2013 rva 0x53b7b0 (2012 0x57e600, byte-identical): the shared parameters are reread out of
// the controller's own profile object with the system-settings override on.
void ArkSettings::UpdateSettingsFromSystemSettings( APlayerController* PC )
{
	UUIDataStore_OnlinePlayerData* PlayerData = PC ? PC->OnlinePlayerData : NULL;
	UUIDataProvider_OnlineProfileSettings* Provider = PlayerData ? PlayerData->ProfileProvider : NULL;
	if( Provider == NULL || Provider->Profile == NULL )
	{
		return;
	}
	GetParameters().Read( Provider->Profile, TRUE );
}

// DISHONORED(port): 2013 rva 0x5334e0 (2012 0x574e20). Sixty-six bytes: the profile write is one script event,
// UIDataStore_OnlinePlayerData.SaveProfileData, raised on the controller's own data store with its four-byte
// bool return as the parameter block. Retail dereferences PC and PC->OnlinePlayerData unconditionally.
// DISHONORED(bringup): what that event reaches is the online subsystem's WriteProfileSettings, and in retail
// and here alike the whole of UOnlineSubsystemSteamworks::WriteProfileSettings sits behind
// IsSteamClientAvailable() - so with -nosteam this call raises the event, the event finds no Steam client and
// nothing is written. Measured and explained in resources/docs/agents/agentEQ.md section 5.
void ArkSettings::SaveSettings( APlayerController* PC )
{
	UUIDataStore_OnlinePlayerData* PlayerData = PC ? PC->OnlinePlayerData : NULL;
	if( PlayerData == NULL )
	{
		return;
	}
	UFunction* SaveProfileData = PlayerData->FindFunction( FName(TEXT("SaveProfileData")) );
	if( SaveProfileData == NULL )
	{
		warnf( TEXT("ArkSettings::SaveSettings: %s has no SaveProfileData event"), *PlayerData->GetClass()->GetName() );
		return;
	}
	UBOOL bSaved = FALSE;
	PlayerData->ProcessEvent( SaveProfileData, &bSaved );
	debugf( TEXT("ArkSettings::SaveSettings: SaveProfileData -> %d"), bSaved ? 1 : 0 );
}
