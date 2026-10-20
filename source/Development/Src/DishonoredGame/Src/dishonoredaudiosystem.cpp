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
#include "dishonoredutilities.h"
#include "iediag.h"
#include "disaudiopropagation.h"

// DISHONORED(port): 2013 rva 0x7a3120 (2012 0x807710, dishonoredaudiosystem.cpp:1712 region). Retail resolves fourteen Wwise ids
// (TimeDilation, Freeze, Unfreeze, Pause_All, Resume_All, All_State_None, Scale_RTPC, Global_Volume, Music_Volume, SFX_Volume,
// Voices_Volume, Ambient_Volume, Menu_Volume, Overseer_Gapper_RTPC, Wwise_Gapper) through AK::SoundEngine::GetIDFromString, allocates
// the end-of-event notify queue (0x834 bytes with its critical section) and finishes with ArkSettings::ApplyCurrentSettings(this).
// Agent IH3 (fork G) resolves the ids (Init below).
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

// DISHONORED(port): agent IE. 2013 rva 0x792d10 (2012 0x7f8a50, 713 B; UAudioSystem slot 75): the audio cell holding a point. The
// cached cell first, then the cells behind its portals, then every cell not yet tried; none when the level has no propagation info.
AVolume* UDishonoredAudioSystem::GetCellAtPoint( const FVector& _rPoint, AVolume* _pCachedCellAtPoint ) const
{
	UDisAudioPropagationInfo* PropagationInfo = DisGetAudioPropagationInfo();
	if( !PropagationInfo || PropagationInfo->m_Cells.Num() <= 0 )
	{
		return NULL;
	}
	const INT NumCells = PropagationInfo->m_Cells.Num();
	ADishonoredAudioVolume* CachedCell = (ADishonoredAudioVolume*)_pCachedCellAtPoint;
	if( !CachedCell )
	{
		for( INT i = 0; i < NumCells; i++ )
		{
			if( PropagationInfo->m_Cells(i)->Encompasses( _rPoint ) )
			{
				return PropagationInfo->m_Cells(i);
			}
		}
		return NULL;
	}
	if( CachedCell->Encompasses( _rPoint ) )
	{
		return CachedCell;
	}
	UBOOL* CheckedCells = (UBOOL*)appAlloca( NumCells * sizeof(UBOOL) );
	appMemzero( CheckedCells, NumCells * sizeof(UBOOL) );
	CheckedCells[CachedCell->m_AudioPropagationIndex] = TRUE;
	for( INT i = 0; i < CachedCell->m_LinkedPortals.Num(); i++ )
	{
		ADishonoredAudioPortal* Portal = CachedCell->m_LinkedPortals(i);
		ADishonoredAudioVolume* OtherCell = CachedCell == Portal->m_pCellA ? Portal->m_pCellB : CachedCell == Portal->m_pCellB ? Portal->m_pCellA : NULL;
		if( !CheckedCells[OtherCell->m_AudioPropagationIndex] && OtherCell->Encompasses( _rPoint ) )
		{
			return OtherCell;
		}
		CheckedCells[OtherCell->m_AudioPropagationIndex] = TRUE;
	}
	for( INT i = 0; i < NumCells; i++ )
	{
		if( !CheckedCells[i] && PropagationInfo->m_Cells(i)->Encompasses( _rPoint ) )
		{
			return PropagationInfo->m_Cells(i);
		}
	}
	return NULL;
}

// DISHONORED(port): agent IE. 2013 rva 0x792fe0 (2012 0x7f8d20, 453 B): one Wwise position per audible path, along the path's
// direction at its final distance from the listener; without one, a single position straight above the listener at twice the
// audible distance (out of earshot).
static void DisSetAkSoundPositions( FDisAudioPropagationCache& _rPropagationCache, AkGameObjectID _GameObjectID )
{
	const INT NumPaths = _rPropagationCache.m_Paths.Num();
	AkSoundPosition* Positions = NumPaths > 0 ? (AkSoundPosition*)appAlloca( NumPaths * sizeof(AkSoundPosition) ) : NULL;
	AkUInt16 NumPositions = 0;
	for( INT i = 0; i < NumPaths; i++ )
	{
		const FDisAudioPath& Path = _rPropagationCache.m_Paths(i);
		if( _rPropagationCache.m_fMaxAudibleDistance > Path.m_fFinalDist )
		{
			Positions[NumPositions].Position = AkVectorFromUnreal( _rPropagationCache.m_ListenerPoint + Path.m_Direction * Path.m_fFinalDist );
			Positions[NumPositions].Orientation = AkVectorFromUnreal( FVector( 0.0f, 0.0f, 1.0f ) );
			NumPositions++;
		}
	}
	if( NumPositions )
	{
		AK::SoundEngine::SetMultiplePositions( _GameObjectID, Positions, NumPositions, AK::SoundEngine::MultiPositionType_MultiDirections );
		return;
	}
	AkSoundPosition Position;
	Position.Position = AkVectorFromUnreal( _rPropagationCache.m_ListenerPoint + FVector( 0.0f, 0.0f, _rPropagationCache.m_fMaxAudibleDistance * 2.0f ) );
	Position.Orientation = AkVectorFromUnreal( FVector( 0.0f, 0.0f, 1.0f ) );
	AK::SoundEngine::SetPosition( _GameObjectID, Position );
}

// DISHONORED(port): agent IH3 (fork G). The Wwise ids Init resolves (2013 statics dword_14671C8..14671F8) and the overseer effect's
// globals (2012 GOverseerEffectRTPC / GOverseerEffectRTPC_ID); DisAkGlobalCallbackFunc below writes GOverseerEffectRTPC (agent IH4).
DWORD UDishonoredAudioSystem::ms_TimeDilationRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_FreezeEvent_ID = 0;
DWORD UDishonoredAudioSystem::ms_UnfreezeEvent_ID = 0;
DWORD UDishonoredAudioSystem::ms_PauseEvent_ID = 0;
DWORD UDishonoredAudioSystem::ms_ResumeEvent_ID = 0;
DWORD UDishonoredAudioSystem::ms_AllStateNoneEvent_ID = 0;
DWORD UDishonoredAudioSystem::ms_ScaleRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_GlobalVolumeRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_MusicVolumeRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_SFXVolumeRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_VoicesVolumeRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_AmbientVolumeRTPC_ID = 0;
DWORD UDishonoredAudioSystem::ms_MenuVolumeRTPC_ID = 0;
FLOAT GOverseerEffectRTPC = 0.f;
DWORD GOverseerEffectRTPC_ID = 0;

/** DISHONORED(port, agent IH4): 2012 0x7f3f50 / 2013 0x7873a0. Wwise's global callback (the audio thread, once per audio frame):
    the overseer effect's RTPC read back as its global value into GOverseerEffectRTPC, which UDishonoredAudioSystem::Update tests. */
void DisAkGlobalCallbackFunc( bool /*in_bLastCall*/ )
{
	AK::SoundEngine::Query::RTPCValue_type Type = AK::SoundEngine::Query::RTPCValue_Global;
	AK::SoundEngine::Query::GetRTPCValue( GOverseerEffectRTPC_ID, AK_INVALID_GAME_OBJECT, GOverseerEffectRTPC, Type );
}

/** DISHONORED(deviation, agent IH4): hands DisAkGlobalCallbackFunc to UAkAudioDevice across the module boundary (AkAudioWwise.h). */
static struct FDisAkGlobalCallbackInstaller
{
	FDisAkGlobalCallbackInstaller()
	{
		GDisAkGlobalCallback = &DisAkGlobalCallbackFunc;
	}
} GDisAkGlobalCallbackInstaller;

UBOOL DisIH3DiagEnabled()
{
	static const UBOOL bEnabled = ParseParam( appCmdLine(), TEXT("ih3diag") ) || ParseParam( appCmdLine(), TEXT("akdiag") );
	return bEnabled;
}

/**
 * DISHONORED(port): agent ID, then agent IH3 (fork G). 2013 rva 0x7a3120 (2012 0x807710): the occluding-door cosine (retail keeps it in
 * ADisDoor::ms_fOccludingDoorOneMinusOpeningHalfAngleCosine, here a unit global), the fifteen Wwise ids, the end-of-event queue,
 * the overseer ids and the settings pass.
 */
void UDishonoredAudioSystem::Init()
{
	GOccludingDoorOpeningAngleCos = 1.0f - appCos( m_fOccludingDoorOpeningAngle * (PI / 180.0f) * 0.5f );
	ms_TimeDilationRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("TimeDilation") );
	ms_FreezeEvent_ID = AK::SoundEngine::GetIDFromString( TEXT("Freeze") );
	ms_UnfreezeEvent_ID = AK::SoundEngine::GetIDFromString( TEXT("Unfreeze") );
	ms_PauseEvent_ID = AK::SoundEngine::GetIDFromString( TEXT("Pause_All") );
	ms_ResumeEvent_ID = AK::SoundEngine::GetIDFromString( TEXT("Resume_All") );
	ms_AllStateNoneEvent_ID = AK::SoundEngine::GetIDFromString( TEXT("All_State_None") );
	ms_ScaleRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Scale_RTPC") );
	ms_GlobalVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Global_Volume") );
	ms_MusicVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Music_Volume") );
	ms_SFXVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("SFX_Volume") );
	ms_VoicesVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Voices_Volume") );
	ms_AmbientVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Ambient_Volume") );
	ms_MenuVolumeRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Menu_Volume") );
	m_pEndOfEventNotifyQueue = new FDisEndOfEventNotifyQueue;
	GOverseerEffectRTPC_ID = AK::SoundEngine::GetIDFromString( TEXT("Overseer_Gapper_RTPC") );
	m_OverseerEffectID = AK::SoundEngine::GetIDFromString( TEXT("Wwise_Gapper") );
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

/**
 * DISHONORED(port): agent IH3 (fork G). 2013 rva 0x7872b0 (2012 0x7f3e60, dishonoredaudiosystem.cpp:121): with the Wwise device up,
 * the global, music, SFX and voices sliders (0..100) into their RTPCs on every game object, and the global volume into the Bink
 * player. Deviation (named): FFullScreenMovieSupport has no volume member here (retail writes GFullScreenMovie + 8 =
 * m_GlobalVolume * 0.01), so the Bink half is not done.
 */
void UDishonoredAudioSystem::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	if( !UAkAudioDevice::Get() )
	{
		return;
	}
	AK::SoundEngine::SetRTPCValue( ms_GlobalVolumeRTPC_ID, (FLOAT)Parameters->m_GlobalVolume, AK_INVALID_GAME_OBJECT, 0, AkCurveInterpolation_Linear );
	AK::SoundEngine::SetRTPCValue( ms_MusicVolumeRTPC_ID, (FLOAT)Parameters->m_MusicVolume, AK_INVALID_GAME_OBJECT, 0, AkCurveInterpolation_Linear );
	AK::SoundEngine::SetRTPCValue( ms_SFXVolumeRTPC_ID, (FLOAT)Parameters->m_SFXVolume, AK_INVALID_GAME_OBJECT, 0, AkCurveInterpolation_Linear );
	AK::SoundEngine::SetRTPCValue( ms_VoicesVolumeRTPC_ID, (FLOAT)Parameters->m_VoicesVolume, AK_INVALID_GAME_OBJECT, 0, AkCurveInterpolation_Linear );
	IH3DIAG( "volume RTPCs: Global_Volume (0x%08x) %d, Music_Volume (0x%08x) %d, SFX_Volume (0x%08x) %d, Voices_Volume (0x%08x) %d",
		ms_GlobalVolumeRTPC_ID, Parameters->m_GlobalVolume, ms_MusicVolumeRTPC_ID, Parameters->m_MusicVolume, ms_SFXVolumeRTPC_ID, Parameters->m_SFXVolume,
		ms_VoicesVolumeRTPC_ID, Parameters->m_VoicesVolume );
}

/** DISHONORED(port): agent IH3 (fork G). 2012 rva 0x7f9160: the start radius is the event's max radius, the stop radius 100 uu more. */
void UDishonoredAudioSystem::RegisterAmbientSound( AActor* AmbientSound )
{
	AAkAmbientSound* Sound = (AAkAmbientSound*)AmbientSound;
	FDisAmbientSound* Entry = new( m_RegisteredAmbientSounds ) FDisAmbientSound;
	Entry->m_pAmbientSound = Sound;
	const FLOAT fStartRadius = Sound->PlayEvent->GetMaxRadius();
	Entry->m_fStartRadiusSq = fStartRadius * fStartRadius;
	Entry->m_fStopRadiusSq = ( fStartRadius + 100.f ) * ( fStartRadius + 100.f );
	Sound->m_bRegistered = TRUE;
	IH3DIAG( "ambient %s registered: %s, radius %.0f at (%.0f %.0f %.0f)", *Sound->GetName(), *Sound->PlayEvent->GetName(), fStartRadius,
		Sound->Location.X, Sound->Location.Y, Sound->Location.Z );
}

/** DISHONORED(port): agent IH3 (fork G). 2013 rva 0x7a3ea0 (2012 0x8083f0) */
void UDishonoredAudioSystem::UnregisterAmbientSound( AActor* AmbientSound )
{
	AAkAmbientSound* Sound = (AAkAmbientSound*)AmbientSound;
	for( INT i = 0; i < m_RegisteredAmbientSounds.Num(); i++ )
	{
		if( m_RegisteredAmbientSounds(i).m_pAmbientSound == Sound )
		{
			if( Sound->m_bStarted )
			{
				Sound->StopEvent();
			}
			Sound->m_bRegistered = FALSE;
			m_RegisteredAmbientSounds.RemoveSwap( i );
			return;
		}
	}
}

/**
 * DISHONORED(port): agent IH3 (fork G). 2013 rva 0x7a32a0 (2012 0x807890): a rate above 0 ramps the global RTPC from its current value
 * (Query::GetRTPCValue when it is not ramping yet) through UpdateRTPCs; otherwise it is set at once and any ramp dropped.
 */
void UDishonoredAudioSystem::SetRTPC( FName _Parameter, FLOAT _fValue, FLOAT _fChangeRate )
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice )
	{
		return;
	}
	const INT NumUpdaters = m_RTPCUpdaters.Num();
	INT UpdaterIndex = 0;
	while( UpdaterIndex < NumUpdaters && m_RTPCUpdaters(UpdaterIndex).m_Parameter != _Parameter )
	{
		UpdaterIndex++;
	}
	IH3DIAG( "RTPC %s -> %.2f (rate %.2f)", *_Parameter.ToString(), _fValue, _fChangeRate );
	if( _fChangeRate > 0.f )
	{
		if( UpdaterIndex == NumUpdaters )
		{
			AkRtpcValue CurrentValue = 0.f;
			AK::SoundEngine::Query::RTPCValue_type ValueType = AK::SoundEngine::Query::RTPCValue_Default;
			AK::SoundEngine::Query::GetRTPCValue( *_Parameter.ToString(), AK_INVALID_GAME_OBJECT, CurrentValue, ValueType );
			m_RTPCUpdaters.Add( 1 );
			m_RTPCUpdaters(UpdaterIndex).m_Parameter = _Parameter;
			m_RTPCUpdaters(UpdaterIndex).m_fCurrentValue = CurrentValue;
		}
		m_RTPCUpdaters(UpdaterIndex).m_fTargetValue = _fValue;
		m_RTPCUpdaters(UpdaterIndex).m_fChangeRate = _fChangeRate;
	}
	else
	{
		AudioDevice->SetRTPCValue( *_Parameter.ToString(), _fValue, NULL, NAME_None );
		if( UpdaterIndex < NumUpdaters )
		{
			m_RTPCUpdaters.RemoveSwap( UpdaterIndex );
		}
	}
}

/** DISHONORED(port): agent IH3 (fork G). 2013 rva 0x7a3580 (2012 0x807ba0): each ramp moves by rate x dt and ends on its target. */
void UDishonoredAudioSystem::UpdateRTPCs( FLOAT _fDeltaTime )
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice )
	{
		return;
	}
	for( INT i = m_RTPCUpdaters.Num() - 1; i >= 0; i-- )
	{
		FDisSoundRTPCUpdater& Updater = m_RTPCUpdaters(i);
		const FLOAT fStep = Updater.m_fChangeRate * _fDeltaTime;
		UBOOL bReached;
		if( Updater.m_fCurrentValue <= Updater.m_fTargetValue )
		{
			Updater.m_fCurrentValue += fStep;
			bReached = Updater.m_fCurrentValue >= Updater.m_fTargetValue;
		}
		else
		{
			Updater.m_fCurrentValue -= fStep;
			bReached = Updater.m_fTargetValue >= Updater.m_fCurrentValue;
		}
		if( bReached )
		{
			Updater.m_fCurrentValue = Updater.m_fTargetValue;
		}
		AudioDevice->SetRTPCValue( *Updater.m_Parameter.ToString(), Updater.m_fCurrentValue, NULL, NAME_None );
		if( bReached )
		{
			m_RTPCUpdaters.RemoveSwap( i );
		}
	}
}

/** DISHONORED(port): agent IH3 (fork G). 2012 rva 0x8083b0 */
void UDishonoredAudioSystem::CleanUp()
{
	m_pPendingAmbientEvent = NULL;
	m_pLastAmbientEvent = NULL;
	m_RegisteredAmbientSounds.Empty();
}

/**
 * DISHONORED(port): agent IH3 (fork G). 2013 rva 0x316bd0 (2012 0x339320, unskeletalanim.cpp): the "<event>v" and "<event>p" RTPCs take
 * the notify's volume and pitch multipliers on the owner's component, then the event is posted on it.
 */
void UAnimNotify_AkEvent::Notify( UAnimNodeSequence* NodeSeq )
{
	AActor* Owner = NodeSeq->SkelComponent->GetOwner();
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || !AkEvent )
	{
		return;
	}
	FString EventName = AkEvent->GetName() + TEXT("v");
	AudioDevice->SetRTPCValue( *EventName, VolumeMultiplier, Owner, BoneName );
	EventName[EventName.Len() - 1] = TEXT('p');
	AudioDevice->SetRTPCValue( *EventName, PitchMultiplier, Owner, BoneName );
	const AkPlayingID PlayingID = AudioDevice->PostEvent( AkEvent, Owner, BoneName, 0, NULL, NULL, FALSE );
	IH3DIAG( "anim notify %s on %s -> playing id %u", *AkEvent->GetName(), Owner ? *Owner->GetName() : TEXT("<none>"), PlayingID );
}

/**
 * DISHONORED(port): agent ID, then agent IE, then agent IH3 (fork G). 2013 rva 0x7b3520 (2012 0x818ff0, 4,492 B; 2013 4,734 B). In
 * retail's order: nothing while the update is suspended; a paused world posts Pause_All once (Resume_All when it resumes) and only
 * renders. Otherwise: the registered ambient sounds start inside their event's radius and stop 100 uu beyond it (an ended one is
 * dropped from the list); the listener cell (GetCellAtPoint at UAkAudioDevice::m_ListenerPosition) - entering one queues its
 * ambient event and sets its RTPCs (SetRTPC, ramped by UpdateRTPCs), the queued ambient event is posted on the global object once
 * m_fMinTimeBetweenAmbientSwitch has passed since the last; the environment (aux bus) values - the overseer effect while
 * GOverseerEffectRTPC is not 0 (on the global object when it toggles) and the listener cell's m_Environment - go to every playing
 * component when they change or when its cache is reset; the world time dilation RTPC on the global object, the end-of-event drain
 * (agent ID), the time dilation RTPC of every playing component (a stopped one returns to m_FreeComponents with its cache cleared);
 * the sound capture box; and the per-component propagation cache (agent IE): the largest playing radius is m_fMaxAudibleDistance,
 * the sound point is the owner's location (the capture box's redirected actor when inside the box, else the BoneName bone of the
 * owner's first unparented skeletal mesh), a sound and listener that moved by less than 5 % of their distance keep the cache (only
 * the portal occlusion of its paths is refreshed), else the cache takes the new points and cells and the Wwise position is set:
 * straight when both points share a cell (or either has none), else through the portal paths UpdateAudioPropagationCache finds
 * (when the sound is within its radius) and DisSetAkSoundPositions. Last, the audio frame (AK::SoundEngine::RenderAudio, which
 * UAkAudioDevice::Update issued on retail's behalf until this port).
 * Deviations (named): 2013's ambient-sound time dilation (AActor::SetAkComponentsTimeDilation 0x38a670 with the game info's
 * per-channel dilation, slot +976) is not ported - neither exists in this tree - so the 2012 shape of that loop is kept; the
 * component set is iterated as UDisAkComponent (retail casts nothing); GOverseerEffectRTPC is written by DisAkGlobalCallbackFunc (agent IH4).
 */
void UDishonoredAudioSystem::Update( UAkAudioDevice* AudioDevice, void* ActiveComponents )
{
	if( m_bSuspended )
	{
		return;
	}
	if( GWorld->IsPaused() )
	{
		if( !m_bPaused )
		{
			AK::SoundEngine::PostEvent( ms_PauseEvent_ID, AKGLOBALSOUNDOBJECT );
			m_bPaused = TRUE;
		}
	}
	else if( m_bPaused )
	{
		AK::SoundEngine::PostEvent( ms_ResumeEvent_ID, AKGLOBALSOUNDOBJECT );
		m_bPaused = FALSE;
	}
	if( m_bPaused || !m_pEndOfEventNotifyQueue )
	{
		AK::SoundEngine::RenderAudio();
		return;
	}
	TSet<UAkComponent*>& GameObjects = *(TSet<UAkComponent*>*)ActiveComponents;
	const FVector& ListenerPosition = AudioDevice->m_ListenerPosition;
	for( INT i = m_RegisteredAmbientSounds.Num() - 1; i >= 0; i-- )
	{
		FDisAmbientSound& Entry = m_RegisteredAmbientSounds(i);
		AAkAmbientSound* AmbientSound = Entry.m_pAmbientSound;
		if( !AmbientSound )
		{
			m_RegisteredAmbientSounds.RemoveSwap( i );
			continue;
		}
		if( AmbientSound->m_bEnded )
		{
			AmbientSound->m_bRegistered = FALSE;
			AmbientSound->m_bStarted = FALSE;
			IH3DIAG( "ambient %s ended, unregistered", *AmbientSound->GetName() );
			m_RegisteredAmbientSounds.RemoveSwap( i );
			continue;
		}
		const FLOAT DistSq = ( AmbientSound->Location - ListenerPosition ).SizeSquared();
		if( Entry.m_fStartRadiusSq < DistSq )
		{
			if( DistSq >= Entry.m_fStopRadiusSq && AmbientSound->m_bStarted )
			{
				IH3DIAG( "ambient %s stop at %.0f (radius %.0f)", *AmbientSound->GetName(), appSqrt( DistSq ), appSqrt( Entry.m_fStartRadiusSq ) );
				AmbientSound->StopEvent();
			}
		}
		else if( !AmbientSound->m_bStarted )
		{
			IH3DIAG( "ambient %s start %s at %.0f (radius %.0f)", *AmbientSound->GetName(), AmbientSound->PlayEvent ? *AmbientSound->PlayEvent->GetName() : TEXT("<none>"),
				appSqrt( DistSq ), appSqrt( Entry.m_fStartRadiusSq ) );
			AmbientSound->StartEvent();
		}
	}

	ADishonoredAudioVolume* ListenerCell = Cast<ADishonoredAudioVolume>( GetCellAtPoint( ListenerPosition, m_pListenerCell ) );
	if( m_pListenerCell != ListenerCell )
	{
		IEDIAG( "listener cell %s -> %s at (%.0f %.0f %.0f)", m_pListenerCell ? *m_pListenerCell->GetName() : TEXT("<none>"),
			ListenerCell ? *ListenerCell->GetName() : TEXT("<none>"), ListenerPosition.X, ListenerPosition.Y, ListenerPosition.Z );
		m_pListenerCell = ListenerCell;
		if( ListenerCell )
		{
			if( ListenerCell->m_pSoundEvent )
			{
				m_pPendingAmbientEvent = ListenerCell->m_pSoundEvent;
			}
			IH3DIAG( "listener cell %s: ambient event %s, %d RTPC(s), environment %s", *ListenerCell->GetName(),
				ListenerCell->m_pSoundEvent ? *ListenerCell->m_pSoundEvent->GetName() : TEXT("<none>"), ListenerCell->m_SoundParameters.Num(), *ListenerCell->m_Environment.ToString() );
			for( INT i = 0; i < m_pListenerCell->m_SoundParameters.Num(); i++ )
			{
				const FDisSoundRTPC& Parameter = m_pListenerCell->m_SoundParameters(i);
				SetRTPC( Parameter.m_Parameter, Parameter.m_fValue, Parameter.m_fChangeRate );
			}
		}
	}
	UpdateRTPCs( GWorld->GetDeltaSeconds() );
	if( m_pPendingAmbientEvent && m_pPendingAmbientEvent != m_pLastAmbientEvent )
	{
		const FLOAT RealTimeSeconds = GWorld->GetRealTimeSeconds();
		if( RealTimeSeconds > m_fMinTimeBetweenAmbientSwitch + m_fLastAmbientTime )
		{
			const AkPlayingID PlayingID = AudioDevice->PostEvent( m_pPendingAmbientEvent, NULL, NAME_None, 0, NULL, NULL, FALSE );
			IH3DIAG( "ambient event %s posted on the global object -> playing id %u", *m_pPendingAmbientEvent->GetName(), PlayingID );
			m_pLastAmbientEvent = m_pPendingAmbientEvent;
			m_pPendingAmbientEvent = NULL;
			m_fLastAmbientTime = RealTimeSeconds;
		}
	}

	AkEnvironmentValue EnvironmentValues[2];
	appMemzero( EnvironmentValues, sizeof( EnvironmentValues ) );
	AkUInt32 NumEnvironments = 0;
	const UBOOL bOverseerEffect = GOverseerEffectRTPC != 0.f;
	UBOOL bEnvironmentChanged = m_bOverseerEffectEnabled != bOverseerEffect;
	m_bOverseerEffectEnabled = bOverseerEffect;
	if( bOverseerEffect )
	{
		EnvironmentValues[0].EnvID = m_OverseerEffectID;
		EnvironmentValues[0].fControlValue = 1.f;
		NumEnvironments = 1;
	}
	if( bEnvironmentChanged )
	{
		AK::SoundEngine::SetGameObjectEnvironmentsValues( AKGLOBALSOUNDOBJECT, EnvironmentValues, NumEnvironments );
	}
	if( m_pListenerCell && m_LastEnvironment != m_pListenerCell->m_Environment )
	{
		m_LastEnvironment = m_pListenerCell->m_Environment;
		m_LastEnvironmentID = AK::SoundEngine::GetIDFromString( *m_LastEnvironment.ToString() );
		IH3DIAG( "environment %s (id 0x%08x)", *m_LastEnvironment.ToString(), m_LastEnvironmentID );
		bEnvironmentChanged = TRUE;
	}
	if( m_LastEnvironment != NAME_None )
	{
		EnvironmentValues[NumEnvironments].EnvID = m_LastEnvironmentID;
		EnvironmentValues[NumEnvironments].fControlValue = 1.f;
		NumEnvironments++;
	}

	ADishonoredGameInfo* GameInfo = DisGetGameInfo();
	const FLOAT WorldTimeDilation = GameInfo ? GameInfo->m_fCurrentWorldTimeDilation : 1.f;
	const FLOAT PlayerTimeDilation = GameInfo ? GameInfo->m_fCurrentPlayerTimeDilation : 1.f;
	AK::SoundEngine::SetRTPCValue( ms_TimeDilationRTPC_ID, WorldTimeDilation, AKGLOBALSOUNDOBJECT, 0, AkCurveInterpolation_Linear );
	ConsumeEndOfEventNotifies( GameObjects );
	for( INT i = m_PlayingComponents.Num() - 1; i >= 0; i-- )
	{
		FDisPlayingComponent& Playing = m_PlayingComponents(i);
		UDisAkComponent* Component = Playing.m_pComponent;
		if( Component->m_PlayingRadii.Num() <= 0 )
		{
			Component->m_PropagationCache.m_Paths.Empty();
			appMemzero( &Component->m_PropagationCache, sizeof( FDisAudioPropagationCache ) );
			m_FreeComponents.AddItem( Component );
			m_PlayingComponents.RemoveSwap( i );
		}
		else
		{
			const FLOAT TimeDilation = Playing.m_pInstigator ? Playing.m_pInstigator->CustomTimeDilation * PlayerTimeDilation : WorldTimeDilation;
			AK::SoundEngine::SetRTPCValue( ms_TimeDilationRTPC_ID, TimeDilation, Component->GetAkGameObjectID(), 0, AkCurveInterpolation_Linear );
		}
	}

	UDisAudioPropagationInfo* PropagationInfo = DisGetAudioPropagationInfo();
	FBox CaptureBox( 0 );
	if( m_pDisSoundCaptureBox && m_pDisSoundCaptureBox->m_pSoundRedirectedActor )
	{
		const FVector Extent = m_pDisSoundCaptureBox->DrawScale3D * m_pDisSoundCaptureBox->DrawScale * 200.0f;
		CaptureBox = FBox( m_pDisSoundCaptureBox->Location - Extent, m_pDisSoundCaptureBox->Location + Extent );
	}

	static FLOAT GIEDiagLastListenerLog = -1.0f;
	const UBOOL bDiagThisFrame = ( DisIEDiagEnabled() || DisIH3DiagEnabled() ) && GWorld->GetTimeSeconds() - GIEDiagLastListenerLog >= 1.0f;
	if( bDiagThisFrame )
	{
		GIEDiagLastListenerLog = GWorld->GetTimeSeconds();
		IEDIAG( "listener (%.0f %.0f %.0f) cell %s, %d game object(s), propagation info %s with %d cell(s)", ListenerPosition.X, ListenerPosition.Y,
			ListenerPosition.Z, m_pListenerCell ? *m_pListenerCell->GetName() : TEXT("<none>"), GameObjects.Num(),
			PropagationInfo ? *PropagationInfo->GetName() : TEXT("<none>"), PropagationInfo ? PropagationInfo->m_Cells.Num() : 0 );
	}

	for( TSet<UAkComponent*>::TIterator It( GameObjects ); It; ++It )
	{
		UDisAkComponent* Component = Cast<UDisAkComponent>( *It );
		if( !Component )
		{
			continue;
		}
		FDisAudioPropagationCache& Cache = Component->m_PropagationCache;
		FLOAT MaxRadius = -1.0f;
		for( INT i = 0; i < Component->m_PlayingRadii.Num(); i++ )
		{
			MaxRadius = Max( MaxRadius, Component->m_PlayingRadii(i).m_Radius );
		}
		if( Component->m_PlayingRadii.Num() == 0 || MaxRadius < 0.0f )
		{
			Cache.m_fMaxAudibleDistance = 0.0f;
			continue;
		}
		if( bEnvironmentChanged || Cache.m_fMaxAudibleDistance <= 0.0f )
		{
			AK::SoundEngine::SetGameObjectEnvironmentsValues( Component->GetAkGameObjectID(), EnvironmentValues, NumEnvironments );
		}
		if( MaxRadius <= 0.0f )
		{
			continue;
		}
		AActor* Owner = Component->GetOwner();
		FVector SoundPoint;
		if( !Owner )
		{
			SoundPoint = Cache.m_SoundPoint;
		}
		else if( Owner->m_bBypassAudioPropagation )
		{
			AkSoundPosition Position;
			Position.Position = AkVectorFromUnreal( Owner->Location );
			Position.Orientation = AkVectorFromUnreal( FVector( 0.0f, 0.0f, 1.0f ) );
			AK::SoundEngine::SetPosition( Component->GetAkGameObjectID(), Position );
			continue;
		}
		else if( CaptureBox.IsValid && Owner->Location.X > CaptureBox.Min.X && CaptureBox.Max.X > Owner->Location.X
			&& Owner->Location.Y > CaptureBox.Min.Y && CaptureBox.Max.Y > Owner->Location.Y
			&& Owner->Location.Z > CaptureBox.Min.Z && CaptureBox.Max.Z > Owner->Location.Z )
		{
			SoundPoint = m_pDisSoundCaptureBox->m_pSoundRedirectedActor->Location;
		}
		else
		{
			SoundPoint = Owner->Location;
			if( Component->BoneName != NAME_None )
			{
				for( INT i = 0; i < Owner->AllComponents.Num(); i++ )
				{
					USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( Owner->AllComponents(i) );
					if( !SkelComp || SkelComp->ParentAnimComponent )
					{
						continue;
					}
					const INT BoneIndex = SkelComp->MatchRefBone( Component->BoneName );
					if( BoneIndex != INDEX_NONE )
					{
						SoundPoint = SkelComp->GetBoneMatrix( BoneIndex ).GetOrigin();
						break;
					}
				}
			}
		}
		const FLOAT RawDistSq = ( ListenerPosition - SoundPoint ).SizeSquared();
		if( MaxRadius <= Cache.m_fMaxAudibleDistance )
		{
			const FLOAT ListenerMovedSq = ( ListenerPosition - Cache.m_ListenerPoint ).SizeSquared();
			const FLOAT Threshold = RawDistSq * 0.0025f;
			if( ListenerMovedSq <= Threshold && ( SoundPoint - Cache.m_SoundPoint ).SizeSquared() + ListenerMovedSq <= Threshold )
			{
				UBOOL bOcclusionChanged = FALSE;
				for( INT PathIndex = 0; PathIndex < Cache.m_Paths.Num(); PathIndex++ )
				{
					FDisAudioPath& Path = Cache.m_Paths(PathIndex);
					FLOAT Openness = 1.0f;
					for( INT PortalIndex = 0; PortalIndex < Path.m_Portals.Num(); PortalIndex++ )
					{
						Openness *= 1.0f - Path.m_Portals(PortalIndex)->m_fOcclusion_HeardByPlayer;
					}
					if( Path.m_fOcclusion != 1.0f - Openness )
					{
						Path.m_fOcclusion = 1.0f - Openness;
						bOcclusionChanged = TRUE;
					}
				}
				if( bOcclusionChanged )
				{
					FLOAT MinFinalDist = 3.4e38f;
					for( INT PathIndex = 0; PathIndex < Cache.m_Paths.Num(); PathIndex++ )
					{
						FDisAudioPath& Path = Cache.m_Paths(PathIndex);
						Path.m_fFinalDist = Abs( Path.m_fOcclusion - 1.0f ) >= 0.0001f ? Max( Path.m_fDistance / ( 1.0f - Path.m_fOcclusion ), Cache.m_fRawDistance ) : 3.4e38f;
						MinFinalDist = Min( MinFinalDist, Path.m_fFinalDist );
					}
					const FLOAT Coeff = MinFinalDist <= Cache.m_fRawDistance ? 3.4e38f : 1.0f / ( MinFinalDist - Cache.m_fRawDistance );
					FLOAT WeightSum = 0.0f;
					FLOAT OcclusionSum = 0.0f;
					FLOAT ObstructionSum = 0.0f;
					for( INT PathIndex = 0; PathIndex < Cache.m_Paths.Num(); PathIndex++ )
					{
						FDisAudioPath& Path = Cache.m_Paths(PathIndex);
						if( Path.m_fFinalDist > MinFinalDist )
						{
							Path.m_fFinalDist = ( Path.m_fFinalDist - Cache.m_fRawDistance ) * Coeff * Path.m_fFinalDist;
						}
						const FLOAT Weight = 1.0f / Path.m_fFinalDist;
						WeightSum += Weight;
						OcclusionSum += Path.m_fOcclusion * Weight;
						ObstructionSum += Path.m_fObstruction * Weight;
					}
					const FLOAT Occlusion = OcclusionSum / WeightSum;
					const FLOAT Obstruction = ObstructionSum / WeightSum;
					if( Cache.m_fOcclusion != Occlusion || Cache.m_fObstruction != Obstruction )
					{
						Cache.m_fOcclusion = Occlusion;
						Cache.m_fObstruction = Obstruction;
						AK::SoundEngine::SetObjectObstructionAndOcclusion( Component->GetAkGameObjectID(), 0, Obstruction, Occlusion );
					}
					DisSetAkSoundPositions( Cache, Component->GetAkGameObjectID() );
				}
				continue;
			}
		}
		Cache.m_fMaxAudibleDistance = MaxRadius;
		Cache.m_ListenerPoint = ListenerPosition;
		Cache.m_pListenerCell = ListenerCell;
		if( Cache.m_SoundPoint != SoundPoint )
		{
			Cache.m_SoundPoint = SoundPoint;
			Cache.m_pSoundCell = Cast<ADishonoredAudioVolume>( GetCellAtPoint( SoundPoint, Cache.m_pSoundCell ) );
		}
		Cache.m_Paths.Reset();
		if( Cache.m_pListenerCell && Cache.m_pSoundCell && Cache.m_pListenerCell != Cache.m_pSoundCell )
		{
			if( MaxRadius * MaxRadius > RawDistSq )
			{
				UpdateAudioPropagationCache( PropagationInfo, Cache, RawDistSq, FALSE, NULL );
			}
			DisSetAkSoundPositions( Cache, Component->GetAkGameObjectID() );
		}
		else
		{
			AkSoundPosition Position;
			Position.Position = AkVectorFromUnreal( Cache.m_SoundPoint );
			Position.Orientation = AkVectorFromUnreal( FVector( 0.0f, 0.0f, 1.0f ) );
			AK::SoundEngine::SetPosition( Component->GetAkGameObjectID(), Position );
			Cache.m_fObstruction = 0.0f;
			Cache.m_fOcclusion = 0.0f;
		}
		AK::SoundEngine::SetObjectObstructionAndOcclusion( Component->GetAkGameObjectID(), 0, Cache.m_fObstruction, Cache.m_fOcclusion );
	}

	if( bDiagThisFrame )
	{
		for( TSet<UAkComponent*>::TIterator It( GameObjects ); It; ++It )
		{
			UDisAkComponent* Component = Cast<UDisAkComponent>( *It );
			if( Component && Component->m_PlayingRadii.Num() > 0 )
			{
				FLOAT PathLength = 0.0f;
				const UBOOL bLive = Component->GetShortestAudiblePathLength( PathLength );
				const FDisAudioPropagationCache& Cache = Component->m_PropagationCache;
				IEDIAG( "  voice on %s: %d playing, radius %.0f, sound (%.0f %.0f %.0f) cell %s, listener cell %s, path %.0f (live %d)",
					Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("<no owner>"), Component->m_PlayingRadii.Num(), Cache.m_fMaxAudibleDistance,
					Cache.m_SoundPoint.X, Cache.m_SoundPoint.Y, Cache.m_SoundPoint.Z, Cache.m_pSoundCell ? *Cache.m_pSoundCell->GetName() : TEXT("<none>"),
					Cache.m_pListenerCell ? *Cache.m_pListenerCell->GetName() : TEXT("<none>"), PathLength, bLive ? 1 : 0 );
				if( Cache.m_pSoundCell && Cache.m_pListenerCell && Cache.m_pSoundCell != Cache.m_pListenerCell )
				{
					IH3DIAG( "other-cell voice on %s: sound cell %s, listener cell %s, raw %.0f, radius %.0f, %d path(s), shortest %.0f (live %d), obstruction %.2f occlusion %.2f",
						Component->GetOwner() ? *Component->GetOwner()->GetName() : TEXT("<no owner>"), *Cache.m_pSoundCell->GetName(), *Cache.m_pListenerCell->GetName(),
						Cache.m_fRawDistance, Cache.m_fMaxAudibleDistance, Cache.m_Paths.Num(), PathLength, bLive ? 1 : 0, Cache.m_fObstruction, Cache.m_fOcclusion );
				}
			}
		}
	}
	AK::SoundEngine::RenderAudio();
}

/**
 * DISHONORED(port): agent IH3 (fork G). 2013 rva 0x7a3700 (2012 0x807d30, 1,662 B): every pair of a listener-cell portal and a
 * sound-cell portal is a candidate. The baked shortest path between them (FindShortestPath, keyed from the cell with the lower
 * propagation index) loses its first portal when the listener can be taken to be in the next cell already, and its last when the
 * sound can; a path whose ends plus its baked length reach the audible distance is dropped, the rest become cache paths
 * (FinalizeAudioPath). An AI noise only asks whether one path is audible. With paths, the raw distance, each path's final distance
 * (its length over its openness, never below the raw distance), the weighting that favours the shortest, and the weighted
 * occlusion / obstruction.
 */
UBOOL UDishonoredAudioSystem::UpdateAudioPropagationCache( UDisAudioPropagationInfo* _pAudioPropagationInfo, FDisAudioPropagationCache& _rPropagationCache, FLOAT _fRawDistSq,
	UBOOL _bForAINoise, TArray<FDisAudioDebugPath>* _pDebugPaths )
{
	ADishonoredAudioVolume* ListenerCell = _rPropagationCache.m_pListenerCell;
	ADishonoredAudioVolume* SoundCell = _rPropagationCache.m_pSoundCell;
	const INT NumListenerPortals = ListenerCell->m_LinkedPortals.Num();
	const INT NumSoundPortals = SoundCell->m_LinkedPortals.Num();
	for( INT pL = 0; pL < NumListenerPortals; pL++ )
	{
		ADishonoredAudioPortal* ListenerPortal = ListenerCell->m_LinkedPortals(pL);
		for( INT pS = 0; pS < NumSoundPortals; pS++ )
		{
			ADishonoredAudioPortal* SoundPortal = SoundCell->m_LinkedPortals(pS);
			FDisAudioPathEndPair PathEndPair;
			if( ListenerCell->m_AudioPropagationIndex >= SoundCell->m_AudioPropagationIndex )
			{
				PathEndPair.m_PathEnd0.m_pCell = SoundCell;
				PathEndPair.m_PathEnd0.m_pPortal = SoundPortal;
				PathEndPair.m_PathEnd1.m_pCell = ListenerCell;
				PathEndPair.m_PathEnd1.m_pPortal = ListenerPortal;
			}
			else
			{
				PathEndPair.m_PathEnd0.m_pCell = ListenerCell;
				PathEndPair.m_PathEnd0.m_pPortal = ListenerPortal;
				PathEndPair.m_PathEnd1.m_pCell = SoundCell;
				PathEndPair.m_PathEnd1.m_pPortal = SoundPortal;
			}
			FDisAudioPortalPath ShortestPath;
			if( !_pAudioPropagationInfo->FindShortestPath( PathEndPair, _rPropagationCache.m_fMaxAudibleDistance, PathEndPair.m_PathEnd0.m_pCell != ListenerCell, ShortestPath ) )
			{
				continue;
			}
			const INT SkipFirstPortal = ShortestPath.m_Nodes(0).m_pPortal->CanConsiderPointInOtherCell( _rPropagationCache.m_ListenerPoint, ListenerCell ) ? 1 : 0;
			const INT NumNodes = ShortestPath.m_Nodes.Num();
			INT SkipLastPortal = 0;
			if( NumNodes > SkipFirstPortal && ShortestPath.m_Nodes(NumNodes - 1).m_pPortal->CanConsiderPointInOtherCell( _rPropagationCache.m_SoundPoint, SoundCell ) )
			{
				SkipLastPortal = 1;
			}
			if( SkipFirstPortal < NumNodes - SkipLastPortal )
			{
				FVector FirstPortalPoint, LastPortalPoint;
				ShortestPath.m_Nodes(SkipFirstPortal).m_pPortal->GetNearestPoint( _rPropagationCache.m_ListenerPoint, FirstPortalPoint );
				const FLOAT fMinLength = ( _rPropagationCache.m_ListenerPoint - FirstPortalPoint ).Size() + ShortestPath.m_fMinLength;
				if( fMinLength >= _rPropagationCache.m_fMaxAudibleDistance )
				{
					continue;
				}
				ShortestPath.m_Nodes(NumNodes - SkipLastPortal - 1).m_pPortal->GetNearestPoint( _rPropagationCache.m_SoundPoint, LastPortalPoint );
				if( ( _rPropagationCache.m_SoundPoint - LastPortalPoint ).Size() + fMinLength >= _rPropagationCache.m_fMaxAudibleDistance )
				{
					continue;
				}
			}
			if( _pAudioPropagationInfo->FinalizeAudioPath( _rPropagationCache, ShortestPath, SkipFirstPortal, SkipLastPortal, _rPropagationCache.m_ListenerPoint,
				_rPropagationCache.m_SoundPoint, _fRawDistSq, _bForAINoise, _pDebugPaths ) && _bForAINoise )
			{
				return TRUE;
			}
		}
	}
	if( _bForAINoise )
	{
		return FALSE;
	}
	const INT NumPaths = _rPropagationCache.m_Paths.Num();
	if( NumPaths )
	{
		const FLOAT fRawDist = appSqrt( _fRawDistSq );
		_rPropagationCache.m_fRawDistance = fRawDist;
		FLOAT fMinDist = 3.4e38f;
		for( INT i = 0; i < NumPaths; i++ )
		{
			FDisAudioPath& Path = _rPropagationCache.m_Paths(i);
			Path.m_fFinalDist = Abs( Path.m_fOcclusion - 1.f ) >= 0.0001f ? Max( Path.m_fDistance / ( 1.f - Path.m_fOcclusion ), fRawDist ) : 3.4e38f;
			fMinDist = Min( fMinDist, Path.m_fFinalDist );
		}
		const FLOAT fCoeff = fMinDist > fRawDist ? 1.f / ( fMinDist - fRawDist ) : 3.4e38f;
		for( INT i = 0; i < NumPaths; i++ )
		{
			FDisAudioPath& Path = _rPropagationCache.m_Paths(i);
			if( Path.m_fFinalDist > fMinDist )
			{
				Path.m_fFinalDist = ( Path.m_fFinalDist - fRawDist ) * fCoeff * Path.m_fFinalDist;
			}
		}
		FLOAT fWeightSum = 0.f;
		FLOAT fOcclusionSum = 0.f;
		FLOAT fObstructionSum = 0.f;
		for( INT i = 0; i < NumPaths; i++ )
		{
			const FDisAudioPath& Path = _rPropagationCache.m_Paths(i);
			const FLOAT fWeight = 1.f / Path.m_fFinalDist;
			fWeightSum += fWeight;
			fOcclusionSum += Path.m_fOcclusion * fWeight;
			fObstructionSum += Path.m_fObstruction * fWeight;
		}
		_rPropagationCache.m_fOcclusion = fOcclusionSum / fWeightSum;
		_rPropagationCache.m_fObstruction = fObstructionSum / fWeightSum;
	}
	return _rPropagationCache.m_Paths.Num() > 0;
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
