// DishonoredGame/src/dishonoredaudiosystem.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x7f3e20  public: static void __cdecl UDishonoredAudioSystem::InitializePrivateStaticClassUDishonoredAudioSystem(void)
//   0x7f3e40  public: static void __cdecl UDisAkComponent::InitializePrivateStaticClassUDisAkComponent(void)
//   0x7f3e60  public: virtual void __thiscall UDishonoredAudioSystem::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x7f3f40  protected: virtual class UDisTweaksBase * __thiscall ADisRiverKrustBodyPart::GetTweaks_Derived(void)
//   0x7f3f50  void __cdecl DisAkGlobalCallbackFunc(bool)
//   0x7f3f80  public: virtual void __thiscall UDishonoredAudioSystem::SuspendUpdate(void)
//   0x7f3f90  public: virtual void __thiscall UDishonoredAudioSystem::ResumeUpdate(void)
//   0x7f5620  public: virtual void __thiscall UDisAkComponent::OnTimeDilationChanged(float, float, unsigned int)
//   0x7f8a50  public: virtual class AInOutVolume * __thiscall UDishonoredAudioSystem::GetCellAtPoint(class FVector const &, class AInOutVolume *)const
//   0x7f8d20  void __cdecl DisSetAkSoundPositions(struct FDisAudioPropagationCache &, unsigned int)
//   0x7f8ef0  public: unsigned int __thiscall UDisAkComponent::IsPlayingID(int)const
//   0x7f8f60  public: float __thiscall UDisAkComponent::GetShortestAudiblePathLength(void)const
//   0x7f9040  void __cdecl DisAkEndOfEventCB(enum AkCallbackType, struct AkCallbackInfo *)
//   0x7f9160  public: virtual void __thiscall UDishonoredAudioSystem::RegisterAmbientSound(class AAkAmbientSound *)
//   0x807710  public: virtual void __thiscall UDishonoredAudioSystem::Init(void)
//   0x807890  public: void __thiscall UDishonoredAudioSystem::SetRTPC(class FName, float, float)
//   0x807ba0  private: void __thiscall UDishonoredAudioSystem::UpdateRTPCs(float)
//   0x807d30  private: unsigned int __thiscall UDishonoredAudioSystem::UpdateAudioPropagationCache(class UDisAudioPropagationInfo *, struct FDisAudioPropagationCache &, float, unsigned int, class TArray<struct FDisAudioDebugPath, class FDefaultAllocator> *)
//   0x8083b0  public: void __thiscall UDishonoredAudioSystem::CleanUp(void)
//   0x8083f0  public: virtual void __thiscall UDishonoredAudioSystem::UnregisterAmbientSound(class AAkAmbientSound *)
//   0x80b7d0  public: virtual void __thiscall UDishonoredAudioSystem::FinishDestroy(void)
//   0x80b800  public: void __thiscall UDishonoredAudioSystem::ConsumeEndOfEventNotifies(class TSet<class UAkComponent *, struct DefaultKeyFuncs<class UAkComponent *, 0>, class FDefaultSetAllocator> &)
//   0x814b30  public: static class UClass * __cdecl UDishonoredAudioSystem::GetPrivateStaticClassUDishonoredAudioSystem(wchar_t const *)
//   0x816d40  public: static class UClass * __cdecl UDishonoredAudioSystem::StaticClassNoInline(void)
//   0x817aa0  public: unsigned int __thiscall UDishonoredAudioSystem::IsSoundAudible(class FVector const &, class ADishonoredAudioVolume *, class FVector const &, class ADishonoredAudioVolume *, float)
//   0x817cb0  public: float __thiscall UDishonoredAudioSystem::GetMinAudibleDistance(class FVector const &, class ADishonoredAudioVolume *, class FVector const &, class ADishonoredAudioVolume *, float)
//   0x818f30  public: static class UClass * __cdecl UDisAkComponent::GetPrivateStaticClassUDisAkComponent(wchar_t const *)
//   0x818fc0  public: class ADishonoredAudioVolume * __thiscall UDishonoredAudioSystem::GetAudioCellAtPoint(class FVector const &, class ADishonoredAudioVolume *)const
//   0x818ff0  public: virtual void __thiscall UDishonoredAudioSystem::Update(class UAkAudioDevice *, class TSet<class UAkComponent *, struct DefaultKeyFuncs<class UAkComponent *, 0>, class FDefaultSetAllocator> &)
//   0x81b460  public: static class UClass * __cdecl UDisAkComponent::StaticClassNoInline(void)
//   0x81e7d0  public: void __thiscall UDishonoredAudioSystem::PostAkEventAtPoint(class UAkEvent *, class FVector const &, class AActor *, float)

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7a3120 (2012 0x807710, dishonoredaudiosystem.cpp:1712 region). Retail resolves fourteen Wwise ids
// (TimeDilation, Freeze, Unfreeze, Pause_All, Resume_All, All_State_None, Scale_RTPC, Global_Volume, Music_Volume, SFX_Volume,
// Voices_Volume, Ambient_Volume, Menu_Volume, Overseer_Gapper_RTPC, Wwise_Gapper) through AK::SoundEngine::GetIDFromString, allocates
// the end-of-event notify queue (0x834 bytes with its critical section) and finishes with ArkSettings::ApplyCurrentSettings(this).
// DISHONORED(bringup): Wwise 2012.1 is a user blocker (middleware.md) and the AkAudio module is a stub, so only the occluding-door
// cosine and the settings pass are done here; the system is silent.
// DISHONORED(port): retail keeps the cosine in a unit global (2013 flt_12E98B8), not in the object
static FLOAT GOccludingDoorOpeningAngleCos = 0.0f;

void UDishonoredAudioSystem::Init()
{
	GOccludingDoorOpeningAngleCos = 1.0f - appCos( m_fOccludingDoorOpeningAngle * (PI / 180.0f) * 0.5f );
	m_pEndOfEventNotifyQueue = NULL;
	ArkSettings::ApplyCurrentSettings( this );
}

// DISHONORED(port): 2013 rva 0x7873d0 / 0x7873e0 (2012 0x7f3f80 / 0x7f3f90): the suspend bit of the flags DWORD
void UDishonoredAudioSystem::SuspendUpdate()
{
	m_bSuspended = TRUE;
}

void UDishonoredAudioSystem::ResumeUpdate()
{
	m_bSuspended = FALSE;
}

// DISHONORED(port): 2013 rva 0x7872b0 (2012 0x7f3e60, dishonoredaudiosystem.cpp:121): with a Wwise device present retail pushes the
// four volume sliders into their RTPCs and the global volume into the Bink player.
// DISHONORED(bringup): there is no device and FFullScreenMovieSupport has no volume member in our tree (retail's Bink player takes
// m_GlobalVolume * 0.01), so nothing is applied yet; the sliders are read and reach here, which is what the settings path needs.
void UDishonoredAudioSystem::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
}
