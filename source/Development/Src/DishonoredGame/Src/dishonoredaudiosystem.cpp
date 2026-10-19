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

/**
 * DISHONORED(port): agent ID, then agent IE. 2013 rva 0x7b3520 (2012 0x818ff0, 4,734 B). Ported, in retail's order: not while the
 * update is suspended (m_bSuspended) and not while the world is paused (retail then posts Pause_All once and returns); the listener
 * cell (GetCellAtPoint at UAkAudioDevice::m_ListenerPosition, kept in m_pListenerCell); the end-of-event drain (agent ID); the
 * sound capture box's bounds; and the per-component propagation cache - the largest playing radius becomes m_fMaxAudibleDistance
 * (0 when nothing plays), the sound point is the owner's location (the capture box's redirected actor when the owner is inside the
 * box, the BoneName bone of the owner's first unparented skeletal mesh that has it), a sound and listener that moved by less than
 * 5 % of their distance keep the cache (only the portal occlusion of its paths is refreshed), else the cache takes the new points
 * and cells and the Wwise position is set: straight when both points share a cell (or either has none), else through
 * DisSetAkSoundPositions with the paths UpdateAudioPropagationCache (2012 0x807d30) finds.
 * Not ported: UpdateAudioPropagationCache with FindShortestPath / FinalizeAudioPath (the portal path search, IH3), so a sound in
 * another cell keeps no path and reads FLT_MAX from GetShortestAudiblePathLength; the ambient sounds, the listener cell's ambient
 * event and RTPCs, the environment (aux bus) values per game object, the time-dilation RTPCs of the playing components and the
 * overseer effect. The component set is iterated as UDisAkComponent (retail casts nothing); a plain UAkComponent is skipped here.
 * UAkAudioDevice::Update issues the audio frame itself.
 */
void UDishonoredAudioSystem::Update( UAkAudioDevice* AudioDevice, void* ActiveComponents )
{
	if( m_bSuspended || !m_pEndOfEventNotifyQueue || ( GWorld && GWorld->IsPaused() ) )
	{
		return;
	}
	TSet<UAkComponent*>& GameObjects = *(TSet<UAkComponent*>*)ActiveComponents;
	const FVector& ListenerPosition = AudioDevice->m_ListenerPosition;
	ADishonoredAudioVolume* ListenerCell = Cast<ADishonoredAudioVolume>( GetCellAtPoint( ListenerPosition, m_pListenerCell ) );
	if( m_pListenerCell != ListenerCell )
	{
		IEDIAG( "listener cell %s -> %s at (%.0f %.0f %.0f)", m_pListenerCell ? *m_pListenerCell->GetName() : TEXT("<none>"),
			ListenerCell ? *ListenerCell->GetName() : TEXT("<none>"), ListenerPosition.X, ListenerPosition.Y, ListenerPosition.Z );
		m_pListenerCell = ListenerCell;
	}
	ConsumeEndOfEventNotifies( GameObjects );

	FBox CaptureBox( 0 );
	if( m_pDisSoundCaptureBox && m_pDisSoundCaptureBox->m_pSoundRedirectedActor )
	{
		const FVector Extent = m_pDisSoundCaptureBox->DrawScale3D * m_pDisSoundCaptureBox->DrawScale * 200.0f;
		CaptureBox = FBox( m_pDisSoundCaptureBox->Location - Extent, m_pDisSoundCaptureBox->Location + Extent );
	}

	static FLOAT GIEDiagLastListenerLog = -1.0f;
	const UBOOL bDiagThisFrame = DisIEDiagEnabled() && GWorld && GWorld->GetTimeSeconds() - GIEDiagLastListenerLog >= 1.0f;
	if( bDiagThisFrame )
	{
		GIEDiagLastListenerLog = GWorld->GetTimeSeconds();
		UDisAudioPropagationInfo* PropagationInfo = DisGetAudioPropagationInfo();
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
			}
		}
	}
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
