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

// DISHONORED(port): Engine.ArkSettingsListenerInterface, 2013 GetPrivateStaticClass rva 0x533b00 / StaticClassNoInline 0x576b30 (native_class_sizes.csv: 56 bytes, flags 0x10004001)
IMPLEMENT_CLASS(UArkSettingsListenerInterface);

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

// DISHONORED(port): 2013 rva 0x539730 (2012 0x57a260, arksettingsparameters.cpp:21): the profile setting ids of UArkProfileSettings read
// through UOnlinePlayerStorage (GetProfileSettingValueFloat +380, GetProfileSettingValueId +328, GetProfileSettingValueInt +384,
// SetProfileSettingValueId +344). The resolution comes from the static PCResolutionSettingProvider (not ported): with the override it is
// GSystemSettings like retail, without it retail asks the provider's current value, which is GSystemSettings as well after Refresh().
void ArkSettingsParameters::Read( UOnlinePlayerStorage* Settings, UBOOL bOverrideStorageSettingsWithSystemSettings )
{
	INT Value = 0;
	Settings->GetProfileSettingValueFloat( 65, m_fMouseSensitivity );
	Settings->GetProfileSettingValueId( 66, Value ); m_bMouseSmoothing = Value == 1;
	Settings->GetProfileSettingValueId( 67, Value ); m_bMouseInvertY = Value == 1;
	Settings->GetProfileSettingValueId( 68, Value ); m_bMouseAutoAim = Value == 1;
	Settings->GetProfileSettingValueInt( 69, m_MouseAutoAimStrength );
	Settings->GetProfileSettingValueId( 70, Value ); m_bMouseFriction = Value == 1;
	Settings->GetProfileSettingValueInt( 71, m_MouseFrictionStrength );
	Settings->GetProfileSettingValueId( 74, m_GamepadBindingSet );
	Settings->GetProfileSettingValueId( 75, Value ); m_bGamepadVibration = Value == 1;
	Settings->GetProfileSettingValueInt( 76, m_GamepadLookXSensitivity );
	Settings->GetProfileSettingValueInt( 77, m_GamepadLookYSensitivity );
	Settings->GetProfileSettingValueId( 78, Value ); m_bGamepadInvertY = Value == 1;
	if( Value == -1 )
	{
		Settings->GetProfileSettingValueId( 2, Value );
		m_bGamepadInvertY = Value == 1;
		Settings->SetProfileSettingValueId( 78, m_bGamepadInvertY );
	}
	Settings->GetProfileSettingValueId( 79, Value ); m_bGamepadAutoAim = Value == 1;
	Settings->GetProfileSettingValueInt( 80, m_GamepadAutoAimStrength );
	Settings->GetProfileSettingValueId( 81, Value ); m_bGamepadFriction = Value == 1;
	Settings->GetProfileSettingValueInt( 82, m_GamepadFrictionStrength );
	// DISHONORED(port): agent EK. This block is retail 2013's own id run, read off
	// ArkSettingsParameters::Read (2013 rva 0x539730): 87 is the HUD visibility, 88..98 are ELEVEN show
	// flags, and 99/100/101 are the crosshair's style, movement and opacity. The ids below it are still
	// the 2012 build's (the two lists are offset by two), which is why 101 is read twice here - as the
	// crosshair opacity, which is what retail reads it as, and again as m_bAutoUseManaElixir, which is
	// what the 2012 list calls it. Measured against the cooked profile's own mapping table:
	// build/agentEK/r3_log.txt.
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
	Settings->GetProfileSettingValueInt( 101, m_CrosshairOpacity );
	Settings->GetProfileSettingValueId( 101, Value ); m_bAutoUseManaElixir = Value == 1;
	Settings->GetProfileSettingValueId( 102, m_KillCamMode );
	Settings->GetProfileSettingValueId( 104, Value ); m_bAutoSaveInMenu = Value == 1;
	Settings->GetProfileSettingValueFloat( 105, m_fHeadBobAmount );
	Settings->GetProfileSettingValueId( 106, Value ); m_bCameraRelativeClimbing = Value == 1;
	Settings->GetProfileSettingValueFloat( 109, m_fGamma );
	m_ResX = GSystemSettings.ResX;
	m_ResY = GSystemSettings.ResY;
	Settings->GetProfileSettingValueId( 113, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.bFullscreen != 0;
		Settings->SetProfileSettingValueId( 113, Value );
	}
	m_bFullscreen = Value == 1;
	Settings->GetProfileSettingValueId( 114, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.bUseVSync != 0;
		Settings->SetProfileSettingValueId( 114, Value );
	}
	m_bVSync = Value == 1;
	Settings->GetProfileSettingValueInt( 115, m_FOV );
	Settings->GetProfileSettingValueId( 116, m_TextureDetails );
	Settings->GetProfileSettingValueId( 117, m_ModelDetails );
	Settings->GetProfileSettingValueId( 118, m_PostProcessQuality );
	Settings->GetProfileSettingValueId( 119, m_AntiAliasingMode );
	Settings->GetProfileSettingValueId( 120, Value ); m_bRatShadows = Value == 1;
	Settings->GetProfileSettingValueInt( 123, m_GlobalVolume );
	Settings->GetProfileSettingValueInt( 124, m_MusicVolume );
	Settings->GetProfileSettingValueInt( 125, m_SFXVolume );
	Settings->GetProfileSettingValueInt( 126, m_VoicesVolume );
	Settings->GetProfileSettingValueId( 127, m_SubtitlesMode );
	Settings->GetProfileSettingValueId( 130, Value );
	if( bOverrideStorageSettingsWithSystemSettings )
	{
		Value = GSystemSettings.SpeakerConfiguration;
		Settings->SetProfileSettingValueId( 130, Value );
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

	for ( INT Index = 0; Index < Listeners.Num(); Index++ )
	{
		IArkSettingsListenerInterface* Listener = (IArkSettingsListenerInterface*)Listeners( Index ).GetInterface();
		if ( Listener != NULL )
		{
			Listener->ApplyGameSettings( &Parameters, ListenerReason );
		}
	}

	// DISHONORED(bringup): retail ends with GEnableForceFeedback = Parameters.m_bGamepadVibration; that
	// global does not exist in this tree (WinDrv drives force feedback through UForceFeedbackManager).
}

