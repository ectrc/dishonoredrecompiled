#pragma once
// Engine/inc/arksettings.h
// DISHONORED(written): ArkSettingsParameters (2012 PDB, 208 bytes, 52 four-byte members) and the ArkSettings statics the listeners
// use (IArkSettingsListenerInterface, UnClient.h). 2013 rvas: ArkSettings::GetParameters 0x53b730, ApplyCurrentSettings 0x53b790,
// ArkSettingsParameters::ArkSettingsParameters 0x53b450, ArkSettingsParameters::Read 0x539730 (2012 0x57e580 / 0x57e5e0 / 0x57e3c0 / 0x57a260).
// The rest of the 2012 unit (FindListeners, OnSettingsChanged, ResetSettings, SaveSettings, the setting providers) is not ported.
//
// PDB functions attributed to this file (1):
//   0x574dd0  public: virtual __thiscall ArkSettings::SettingProvider::~SettingProvider(void)

class UOnlinePlayerStorage;
class IArkSettingsListenerInterface;

class ArkSettingsParameters
{
public:
	FLOAT m_fMouseSensitivity;
	UBOOL m_bMouseSmoothing;
	UBOOL m_bMouseInvertY;
	UBOOL m_bMouseAutoAim;
	INT m_MouseAutoAimStrength;
	UBOOL m_bMouseFriction;
	INT m_MouseFrictionStrength;
	INT m_GamepadBindingSet;
	UBOOL m_bGamepadVibration;
	INT m_GamepadLookXSensitivity;
	INT m_GamepadLookYSensitivity;
	UBOOL m_bGamepadInvertY;
	UBOOL m_bGamepadAutoAim;
	INT m_GamepadAutoAimStrength;
	UBOOL m_bGamepadFriction;
	INT m_GamepadFrictionStrength;
	INT m_HUDVisibility;
	UBOOL m_bShowObjectivePopups;
	UBOOL m_bShowTutorialNotifications;
	UBOOL m_bShowInteractions;
	UBOOL m_bShowHighlight;
	UBOOL m_bShowPickupLog;
	UBOOL m_bShowContextualIcons;
	UBOOL m_bShowPlayerStance;
	// DISHONORED(written): agent EK. Retail 2013's ArkSettingsParameters has ELEVEN HUD show flags,
	// not ten: UDisGFxMoviePlayerHUD::ApplyGameSettings (2013 rva 0x7af370) copies members 17..27 into
	// FDisHUDSettings' eleven bits and then reads member 28 as m_CrosshairStyle, and
	// ArkSettingsParameters::Read (2013 rva 0x539730) fills 17..27 from profile setting ids 88..98 and
	// 28..30 from 99..101. Without this member every field from m_CrosshairStyle on is one slot early
	// and the struct is 208 bytes where retail's is 212. This tree's Read is written against the 2012
	// build's id list (85..98), which has no id for this flag, so it is left at its zero initialiser here.
	UBOOL m_bShowObjectiveMarkers;
	UBOOL m_bShowGrenadeMarkers;
	UBOOL m_bShowAwarenessMarkers;
	UBOOL m_bShowHeartTargetMarkers;
	INT m_CrosshairStyle;
	UBOOL m_bCrosshairMovement;
	INT m_CrosshairOpacity;
	UBOOL m_bAutoUseManaElixir;
	INT m_KillCamMode;
	UBOOL m_bAutoSaveInMenu;
	FLOAT m_fHeadBobAmount;
	UBOOL m_bCameraRelativeClimbing;
	FLOAT m_fGamma;
	INT m_ResX;
	INT m_ResY;
	UBOOL m_bFullscreen;
	UBOOL m_bVSync;
	INT m_FOV;
	INT m_TextureDetails;
	INT m_ModelDetails;
	INT m_PostProcessQuality;
	INT m_AntiAliasingMode;
	UBOOL m_bRatShadows;
	INT m_GlobalVolume;
	INT m_MusicVolume;
	INT m_SFXVolume;
	INT m_VoicesVolume;
	INT m_SubtitlesMode;
	INT m_SpeakerConfiguration;

	ArkSettingsParameters();
	void Read( UOnlinePlayerStorage* Settings, UBOOL bOverrideStorageSettingsWithSystemSettings );
};

class ArkSettings
{
public:
	// DISHONORED(layout): ArkSettings' own change reason, mapped onto the listener's one by
	// OnSettingsChanged (2013 rva 0x53b7e0: 0 -> ASLI_ReadProfileFromStorage, 1 -> ASLI_ModifiedByUser,
	// 2 -> ASLI_ValidatedByUser).
	enum EChangeReason
	{
		ECR_ReadFromStorage = 0,
		ECR_ModifiedByUser = 1,
		ECR_ValidatedByUser = 2,
	};

	static ArkSettingsParameters& GetParameters();
	static void ApplyCurrentSettings( IArkSettingsListenerInterface* Listener );
	static void FindListeners( TArray<TScriptInterface<IArkSettingsListenerInterface> >& OutListeners );
	static void OnSettingsChanged( UOnlinePlayerStorage* Settings, TArray<TScriptInterface<IArkSettingsListenerInterface> >& Listeners, EChangeReason Reason );
};
