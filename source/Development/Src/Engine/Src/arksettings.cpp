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
