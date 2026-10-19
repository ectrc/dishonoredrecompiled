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
// cosine, the end-of-event queue (agent ID) and the settings pass are done here; the fourteen ids are not resolved.
// DISHONORED(port): retail keeps the cosine in a unit global (2013 flt_12E98B8), not in the object
static FLOAT GOccludingDoorOpeningAngleCos = 0.0f;

/**
 * DISHONORED(port): agent ID (PHASE23 ID). The 0x834-byte end-of-event queue Init allocates (2012 PDB type
 * FDisEndOfEventNotifyQueue): a 256-entry single-producer ring the Wwise thread fills (pushed count @2048, popped count @2052,
 * capacity @2056), and an overflow array under a critical section for a full ring. DisAkEndOfEventCB (2013 rva 0x793340,
 * 2012 0x7f9040) fills it, ConsumeEndOfEventNotifies (2013 0x7a9970, 2012 0x80b800) drains it on the game thread and drops each
 * ended playing id from its component's m_PlayingRadii - which is what UDisConversationComponent::IsAnyAkEventPlaying reads.
 */
struct FDisEndOfEventNotify
{
	UAkComponent* m_pComponent;
	INT m_PlayingID;
};

struct FDisEndOfEventNotifyQueue
{
	FDisEndOfEventNotify m_DisEndOfEventNotifies[256];
	volatile INT m_PushedCount;
	volatile INT m_PoppedCount;
	INT m_Capacity;
	FCriticalSection m_CriticalSection;
	TArray<FDisEndOfEventNotify> m_AdditionalDisEndOfEventNotifies;

	FDisEndOfEventNotifyQueue()
	:	m_PushedCount( 0 )
	,	m_PoppedCount( 0 )
	,	m_Capacity( ARRAY_COUNT(m_DisEndOfEventNotifies) )
	{
	}
};

void DisAkEndOfEventCB( AkCallbackType _Type, AkCallbackInfo* _pInfo )
{
	if( GIsRequestingExit || _pInfo->gameObjID == AKGLOBALSOUNDOBJECT || !_pInfo->pCookie )
	{
		return;
	}
	FDisEndOfEventNotifyQueue* Queue = (FDisEndOfEventNotifyQueue*)( (UDishonoredAudioSystem*)_pInfo->pCookie )->m_pEndOfEventNotifyQueue;
	if( !Queue )
	{
		return;
	}
	FDisEndOfEventNotify Notify;
	Notify.m_pComponent = (UAkComponent*)_pInfo->gameObjID;
	Notify.m_PlayingID = (INT)( (AkEventCallbackInfo*)_pInfo )->playingID;
	if( Queue->m_PushedCount - Queue->m_PoppedCount == Queue->m_Capacity )
	{
		FScopeLock Lock( &Queue->m_CriticalSection );
		Queue->m_AdditionalDisEndOfEventNotifies.AddItem( Notify );
		return;
	}
	Queue->m_DisEndOfEventNotifies[Queue->m_PushedCount & ( Queue->m_Capacity - 1 )] = Notify;
	appInterlockedIncrement( &Queue->m_PushedCount );
}

static void DisDropPlayingID( TSet<UAkComponent*>& _rGameObjects, const FDisEndOfEventNotify& _rNotify )
{
	if( !_rGameObjects.Contains( _rNotify.m_pComponent ) )
	{
		return;
	}
	TArray<FAkPlayingRadius>& Radii = _rNotify.m_pComponent->m_PlayingRadii;
	for( INT i = 0; i < Radii.Num(); i++ )
	{
		if( Radii(i).m_PlayingID == _rNotify.m_PlayingID )
		{
			Radii.Remove( i );
			return;
		}
	}
}

void UDishonoredAudioSystem::ConsumeEndOfEventNotifies( TSet<UAkComponent*>& _rGameObjects )
{
	FDisEndOfEventNotifyQueue* Queue = (FDisEndOfEventNotifyQueue*)m_pEndOfEventNotifyQueue;
	while( Queue->m_PushedCount != Queue->m_PoppedCount )
	{
		const FDisEndOfEventNotify Notify = Queue->m_DisEndOfEventNotifies[Queue->m_PoppedCount & ( Queue->m_Capacity - 1 )];
		appInterlockedIncrement( &Queue->m_PoppedCount );
		DisDropPlayingID( _rGameObjects, Notify );
	}
	if( Queue->m_AdditionalDisEndOfEventNotifies.Num() > 0 )
	{
		FScopeLock Lock( &Queue->m_CriticalSection );
		for( INT i = 0; i < Queue->m_AdditionalDisEndOfEventNotifies.Num(); i++ )
		{
			DisDropPlayingID( _rGameObjects, Queue->m_AdditionalDisEndOfEventNotifies(i) );
		}
		Queue->m_AdditionalDisEndOfEventNotifies.Empty();
	}
}

/**
 * DISHONORED(port): agent ID. 2013 rva 0x7b3520 (2012 0x818ff0). Of the 4,734 bytes only the end-of-event drain is ported, in its
 * place: not while the update is suspended (m_bSuspended) and not while the world is paused (retail then posts Pause_All once and
 * returns). The pause/resume events, the time-dilation RTPC, the listener cell, the per-component propagation caches
 * (UpdateAudioPropagationCache 2012 0x807d30, so every UDisAkComponent keeps m_fMaxAudibleDistance 0), the RTPC updaters and the
 * ambient sounds are not ported; UAkAudioDevice::Update issues the audio frame itself.
 */
void UDishonoredAudioSystem::Update( UAkAudioDevice* AudioDevice, void* ActiveComponents )
{
	if( m_bSuspended || !m_pEndOfEventNotifyQueue || ( GWorld && GWorld->IsPaused() ) )
	{
		return;
	}
	ConsumeEndOfEventNotifies( *(TSet<UAkComponent*>*)ActiveComponents );
}

// DISHONORED(port): agent ID. 2012 rva 0x80b7d0 (39 B): the queue Init allocated
void UDishonoredAudioSystem::FinishDestroy()
{
	delete (FDisEndOfEventNotifyQueue*)m_pEndOfEventNotifyQueue;
	m_pEndOfEventNotifyQueue = NULL;
	Super::FinishDestroy();
}

// DISHONORED(port): agent ID. 2013 rva 0x7931b0 (2012 0x7f8ef0, 100 B)
UBOOL UDisAkComponent::IsPlayingID( INT _PlayingID ) const
{
	for( INT i = 0; i < m_PlayingRadii.Num(); i++ )
	{
		if( m_PlayingRadii(i).m_PlayingID == _PlayingID )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): agent ID. 2013 rva 0x793220 (2012 0x7f8f60): with a live propagation cache, the shortest portal path when the
// sound and the listener are in different audio cells, else the straight distance; without one, FLT_MAX and FALSE.
UBOOL UDisAkComponent::GetShortestAudiblePathLength( FLOAT& _rLengthOut ) const
{
	if( m_PropagationCache.m_fMaxAudibleDistance <= 0.0f )
	{
		_rLengthOut = 3.4e38f;
		return FALSE;
	}
	if( m_PropagationCache.m_pSoundCell && m_PropagationCache.m_pListenerCell && m_PropagationCache.m_pSoundCell != m_PropagationCache.m_pListenerCell )
	{
		FLOAT Shortest = 3.4e38f;
		for( INT i = 0; i < m_PropagationCache.m_Paths.Num(); i++ )
		{
			Shortest = Min( Shortest, m_PropagationCache.m_Paths(i).m_fFinalDist );
		}
		_rLengthOut = Shortest;
		return TRUE;
	}
	_rLengthOut = ( m_PropagationCache.m_ListenerPoint - m_PropagationCache.m_SoundPoint ).Size();
	return TRUE;
}

void UDishonoredAudioSystem::Init()
{
	GOccludingDoorOpeningAngleCos = 1.0f - appCos( m_fOccludingDoorOpeningAngle * (PI / 180.0f) * 0.5f );
	m_pEndOfEventNotifyQueue = new FDisEndOfEventNotifyQueue;
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
