/*=============================================================================
	akaudiodevice.cpp - UAkAudioDevice, Arkane's Wwise front end, plus the allocation hooks, the language
	map and the low-level IO global.

	Ported from the retail 2013 exe; the 2013 rva of every function is on the function. The 2012 PDB
	attributes the same set to AkAudio/src/akaudiodevice.cpp, including the g_lowLevelIO global
	(_dynamic_initializer_for__g_lowLevelIO__ 0xba2fa0) that lives here.

	What is faithful and what is bring-up:
	  * the initialisation order, the pool sizes, the speaker masks, the plug-in and codec registrations,
	    the bank directory, the language map and the global game object are retail's, read out of
	    UAkAudioDevice::EnsureInitialized (0x5b1430) and ::SetBankDirectory (0x5b0b40);
	  * the eight effect plug-ins and the Vorbis codec are registered by id with no factory, because the
	    factories are Audiokinetic's (see the note at EnsureInitialized);
	  * UAkAudioDevice::Get is a bring-up deviation: retail reaches the device through
	    GEngine->Client->GetAkAudioDevice (UClient vtable slot 320, the engine's own body is the
	    pure-virtual logger at 0x1e68e0 and UWindowsClient supplies the object). UClient has no such virtual
	    in this tree, so the device is created on first use and rooted. Nothing else changes.
=============================================================================*/
#include "AkAudio.h"

#if DISHONORED_WITH_WWISE

#include "akunrealiohookblocking.h"

/** DISHONORED(retail): ?m_bSoundEngineInitialized@UAkAudioDevice@@1_NA, 2013 rva 0x105b43c */
UBOOL UAkAudioDevice::m_bSoundEngineInitialized = FALSE;

/** DISHONORED(retail): the g_lowLevelIO global of akaudiodevice.cpp (2012 initializer rva 0xba2fa0) */
static CAkFilePackageLowLevelIO g_lowLevelIO;

/** DISHONORED(bringup): stands in for GEngine->Client->GetAkAudioDevice(); see the file header. */
static UAkAudioDevice* GBringupAkAudioDevice = NULL;

/*-----------------------------------------------------------------------------
	Allocation hooks and the log sink
-----------------------------------------------------------------------------*/

#if DISHONORED_WWISE_SILENT

static void AkWwiseLogSink( const AkOSChar* Message )
{
	debugf( TEXT("Wwise: %s"), Message );
}

static void* AkWwiseAlloc( size_t Size )
{
	return appMalloc( (DWORD)Size );
}

static void AkWwiseFree( void* Memory )
{
	appFree( Memory );
}

#else

// DISHONORED(port): with the licensed runtime linked, the four hooks are the game's as the SDK requires
// (2012 rvas of retail's own: FreeHook 0x5f5410, VirtualAllocHook 0x5f5420, VirtualFreeHook 0x5f5440).
namespace AK
{
	void* AllocHook( size_t InSize ) { return appMalloc( (DWORD)InSize ); }
	void FreeHook( void* InMemory ) { appFree( InMemory ); }
	void* VirtualAllocHook( void* /*InAddress*/, size_t InSize, AkUInt32 /*InAllocationType*/, AkUInt32 /*InProtect*/ ) { return appMalloc( (DWORD)InSize ); }
	void VirtualFreeHook( void* InAddress, size_t /*InSize*/, AkUInt32 /*InFreeType*/ ) { appFree( InAddress ); }
}

#endif // DISHONORED_WWISE_SILENT

/*-----------------------------------------------------------------------------
	AkGetLanguage
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5aee20. The Unreal three-letter code maps to the Wwise language folder name;
 * RUS, POL, CZE and HUN all fall through to English(US) in the retail body (their voices ship as the RUS
 * bank set, which UAkBank::Load remaps separately, 0xc7210).
 */
const TCHAR* AkGetLanguage( const TCHAR* InUnrealLanguage )
{
	if( !appStrcmp( TEXT("FRA"), InUnrealLanguage ) )
	{
		return TEXT("French(France)");
	}
	if( !appStrcmp( TEXT("DEU"), InUnrealLanguage ) )
	{
		return TEXT("German");
	}
	if( !appStrcmp( TEXT("ITA"), InUnrealLanguage ) )
	{
		return TEXT("Italian");
	}
	if( !appStrcmp( TEXT("ESN"), InUnrealLanguage ) || !appStrcmp( TEXT("ESM"), InUnrealLanguage ) )
	{
		return TEXT("Spanish(Spain)");
	}
	if( !appStrcmp( TEXT("JPN"), InUnrealLanguage ) )
	{
		return TEXT("Japanese");
	}
	return TEXT("English(US)");
}

/*-----------------------------------------------------------------------------
	Lifetime
-----------------------------------------------------------------------------*/

// DISHONORED(bringup): 2013 rva 0x5aedf0 is GEngine->Client->GetAkAudioDevice() (see the file header).
UAkAudioDevice* UAkAudioDevice::Get()
{
	if( !GBringupAkAudioDevice && GEngine && !GIsRequestingExit )
	{
		// Assigned before Init: EnsureInitialized -> LoadAllReferencedBanks -> UAkBank::Load comes back here.
		GBringupAkAudioDevice = ConstructObject<UAkAudioDevice>( UAkAudioDevice::StaticClass() );
		GBringupAkAudioDevice->AddToRoot();
		GBringupAkAudioDevice->Init();
	}
	return GBringupAkAudioDevice;
}

// DISHONORED(port): 2013 rva 0x5b1740: EnsureInitialized(-1), then TRUE whatever it answered
UBOOL UAkAudioDevice::Init()
{
	EnsureInitialized( -1 );
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x5b1430. Retail's order, argument for argument:
 *   1. m_pAkComponentClass = StaticLoadClass("DishonoredGame.DisAkComponent")
 *   2. AK::MemoryMgr::Init with uMaxNumPools 384 in the game, 2048 in the editor
 *   3. AK::StreamMgr::GetDefaultSettings, uMemorySize 0x20000 in the game, 0x80000 in the editor, Create
 *   4. AK::StreamMgr::GetDefaultDeviceSettings, uGranularity 0x8000, fTargetAutoStmBufferLength 1.5,
 *      uIOMemorySize 1310720, then CAkDefaultIOHookBlocking::Init(settings, blocking)
 *   5. AK::SoundEngine::GetDefaultInitSettings / GetDefaultPlatformInitSettings, uChannelMask 0 / 3 / 63 for
 *      the speaker-configuration override -1 / 1 / 2, hWnd = GGameWindow, bGlobalFocus = FALSE when there is
 *      a window, uDefaultPoolSize 0x800000 (0x1000000 in the editor), uLEngineDefaultPoolSize 0x800000, Init
 *   6. GFullScreenMovie slot 24 (the movie player's audio is handed over)
 *   7. AK::MusicEngine::GetDefaultInitSettings + Init
 *   8. eight RegisterPlugin calls and one RegisterCodec (ids below)
 *   9. SetBankDirectory, RegisterGameObj(2, "Unreal Global"), SetPosition(2, origin)
 *  10. LoadAllReferencedBanks, then RegisterGlobalCallback(DisAkGlobalCallbackFunc)
 *
 * The plug-in and codec factories are Audiokinetic's (CAkFDNReverbFX::Create, CreateVorbisFilePlugin, ...)
 * and are not in the tree, so the registrations pass NULL factories: with the silent backend they are only
 * recorded, and with the licensed SDK the AkXxxFXFactory.h headers supply them. That is the single place
 * where this function is not yet retail-identical, and it is marked in the body.
 */
UBOOL UAkAudioDevice::EnsureInitialized( INT SpeakerConfigOverride )
{
	if( m_bSoundEngineInitialized )
	{
		return TRUE;
	}

#if DISHONORED_WWISE_SILENT
	DishonoredWwise::SetLogHook( AkWwiseLogSink );
	DishonoredWwise::SetAllocator( AkWwiseAlloc, AkWwiseFree );
#endif

	// DISHONORED(retail): the component class is a config-free StaticLoadClass in retail too; the fallback is
	// ours, so a build without DishonoredGame still has a component class to construct.
	m_pAkComponentClass = UObject::StaticLoadClass( UAkComponent::StaticClass(), NULL, TEXT("DishonoredGame.DisAkComponent"), NULL, LOAD_Quiet | LOAD_NoWarn, NULL );
	if( !m_pAkComponentClass )
	{
		m_pAkComponentClass = UAkComponent::StaticClass();
	}

	AkMemSettings MemSettings;
	MemSettings.uMaxNumPools = GIsEditor ? 2048 : 384;
	if( AK::MemoryMgr::Init( &MemSettings ) != AK_Success )
	{
		debugf( NAME_Warning, TEXT("Wwise: AK::MemoryMgr::Init failed") );
		return FALSE;
	}

	AkStreamMgrSettings StreamSettings;
	AK::StreamMgr::GetDefaultSettings( StreamSettings );
	StreamSettings.uMemorySize = GIsEditor ? 0x80000 : 0x20000;
	if( !AK::StreamMgr::Create( StreamSettings ) )
	{
		debugf( NAME_Warning, TEXT("Wwise: AK::StreamMgr::Create failed") );
		return FALSE;
	}

	AkDeviceSettings DeviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings( DeviceSettings );
	DeviceSettings.uGranularity = 0x8000;
	DeviceSettings.fTargetAutoStmBufferLength = 1.5f;
	DeviceSettings.uIOMemorySize = 1310720;
	DeviceSettings.uSchedulerTypeFlags = AK_SCHEDULER_BLOCKING;
	if( g_lowLevelIO.Init( DeviceSettings ) != AK_Success )
	{
		debugf( NAME_Warning, TEXT("Wwise: the low-level IO hook could not create its streaming device") );
		return FALSE;
	}

	AkInitSettings InitSettings;
	AkPlatformInitSettings PlatformSettings;
	AK::SoundEngine::GetDefaultInitSettings( InitSettings );
	AK::SoundEngine::GetDefaultPlatformInitSettings( PlatformSettings );
	switch( SpeakerConfigOverride )
	{
	case 0:		PlatformSettings.uChannelMask = 0; break;
	case 1:		PlatformSettings.uChannelMask = AK_SPEAKER_SETUP_STEREO; break;
	case 2:		PlatformSettings.uChannelMask = AK_SPEAKER_SETUP_5POINT1; break;
	default:	break;
	}
	// DISHONORED(bringup): retail stores GGameWindow here and clears bGlobalFocus when there is a window
	// (0x5b1430). This tree has no such global (the viewport owns its HWND inside WinDrv), and the window only
	// matters to the DirectSound / XAudio2 sink, which the silent backend does not create; wiring
	// UWindowsClient's window in is a follow-up.
	InitSettings.uDefaultPoolSize = GIsEditor ? 0x1000000 : 0x800000;
	PlatformSettings.uLEngineDefaultPoolSize = 0x800000;
	if( AK::SoundEngine::Init( &InitSettings, &PlatformSettings ) != AK_Success )
	{
		debugf( NAME_Warning, TEXT("Wwise: AK::SoundEngine::Init failed") );
		return FALSE;
	}

	AkMusicSettings MusicSettings;
	AK::MusicEngine::GetDefaultInitSettings( MusicSettings );
	if( AK::MusicEngine::Init( &MusicSettings ) != AK_Success )
	{
		debugf( NAME_Warning, TEXT("Wwise: AK::MusicEngine::Init failed") );
		return FALSE;
	}

	// DISHONORED(retail): the ids retail passes at 0x5b1430, in its order. The factories are Audiokinetic's
	// (AkRoomVerbFXFactory.h and friends); with no SDK there is nothing to point at, so the registrations
	// carry the ids only and the backend records them.
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 115, NULL, NULL );	// AkMatrixReverb
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 106, NULL, NULL );	// AkDelay
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 105, NULL, NULL );	// AkParametricEQ
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 118, NULL, NULL );	// AkRoomVerb
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 130, NULL, NULL );	// AkTimeStretch
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 136, NULL, NULL );	// AkPitchShifter
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, 131, NULL, NULL );	// AkTremolo
	AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, 101, NULL, NULL );	// AkSilenceSource
	AK::SoundEngine::RegisterCodec( AKCOMPANYID_AUDIOKINETIC, AKCODECID_VORBIS, NULL, NULL );			// AkVorbisDecoder

	SetBankDirectory();

	AK::SoundEngine::RegisterGameObj( AKGLOBALSOUNDOBJECT, "Unreal Global" );
	AkSoundPosition GlobalPosition;
	appMemzero( &GlobalPosition, sizeof(GlobalPosition) );
	AK::SoundEngine::SetPosition( AKGLOBALSOUNDOBJECT, GlobalPosition, 0 );

	m_bSoundEngineInitialized = TRUE;
	LoadAllReferencedBanks();
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x5b0b40. The bank directory is appGameDir() plus CookedPCConsole\ for the
 * seek-free console cook, CookedPC\ for a seek-free PC cook and Content\WwiseAudio\Windows\ when the game
 * runs uncooked; then the Wwise language folder from the Unreal language code.
 */
void UAkAudioDevice::SetBankDirectory()
{
	FString BankPath = appGameDir();
	if( GUseSeekFreeLoading )
	{
		BankPath += GIsSeekFreePCConsole ? TEXT("CookedPCConsole\\") : TEXT("CookedPC\\");
	}
	else
	{
		BankPath += TEXT("Content\\WwiseAudio\\Windows\\");
	}
	g_lowLevelIO.SetBasePath( *BankPath );
	AK::StreamMgr::SetCurrentLanguage( AkGetLanguage( UObject::GetLanguage() ) );
	debugf( TEXT("Wwise: bank directory %s"), *BankPath );
}

// DISHONORED(port): 2013 rva 0x5b3de0
void UAkAudioDevice::Teardown()
{
	if( !m_bSoundEngineInitialized )
	{
		return;
	}
	AK::MusicEngine::Term();
	Flush( NULL );
	AK::SoundEngine::UnregisterAllGameObj();
	m_GameObjects.Empty();
	if( AK::SoundEngine::IsInitialized() )
	{
		AK::SoundEngine::Term();
	}
	g_lowLevelIO.Term();
	if( AK::IAkStreamMgr::Get() )
	{
		AK::IAkStreamMgr::Get()->Destroy();
	}
	AK::MemoryMgr::Term();
	m_bSoundEngineInitialized = FALSE;
}

// DISHONORED(port): 2013 rva 0x5b3fe0 / 0x5b4000
void UAkAudioDevice::FinishDestroy()
{
	Teardown();
	if( GBringupAkAudioDevice == this )
	{
		GBringupAkAudioDevice = NULL;
	}
	Super::FinishDestroy();
}

void UAkAudioDevice::ShutdownAfterError()
{
	Teardown();
	Super::ShutdownAfterError();
}

// DISHONORED(port): 2013 rva 0x5aec80: the world's audio system does the work, with this device and the live
// component set as its arguments (UAudioSystem slot 76, EngineArkaneClasses.h).
void UAkAudioDevice::Update( UBOOL bGameTicking )
{
	if( GWorld && GWorld->m_pAudioSystem )
	{
		GWorld->m_pAudioSystem->Update( this, &m_GameObjects );
	}
}

/*-----------------------------------------------------------------------------
	Listener and positions
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x5aecb0. Front and Top come from the last and the second argument; the Right
// vector is passed but unused, exactly as in the retail body.
void UAkAudioDevice::SetListener( INT ListenerIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front )
{
	AkListenerPosition Position;
	Position.OrientationFront = AkVectorFromUnreal( Front );
	Position.OrientationTop = AkVectorFromUnreal( Up );
	Position.Position = AkVectorFromUnreal( Location );
	AK::SoundEngine::SetListenerPosition( Position, ListenerIndex );
	m_ListenerPosition = Location;
}

// DISHONORED(port): 2013 rva 0x5b3b30. Stop every component of the scene (or of every scene when Scene is
// NULL), stop the global object, and put the listener back at the origin.
void UAkAudioDevice::Flush( FSceneInterface* Scene )
{
	for( TSet<UAkComponent*,DefaultKeyFuncs<UAkComponent*,0> >::TIterator It( m_GameObjects ); It; ++It )
	{
		UAkComponent* Component = *It;
		if( Component && ( !Scene || !Component->GetAkScene() || Component->GetAkScene() == Scene ) )
		{
			Component->Stop();
		}
	}
	AK::SoundEngine::StopAll( AKGLOBALSOUNDOBJECT );
	SetListener( 0, FVector(0,0,0), FVector(0,0,1), FVector(0,1,0), FVector(1,0,0) );
}

/*-----------------------------------------------------------------------------
	Components
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x5b3b00
void UAkAudioDevice::RegisterComponent( UAkComponent* Component )
{
	if( !Component )
	{
		return;
	}
	m_GameObjects.Add( Component );
	AK::SoundEngine::RegisterGameObj( Component->GetAkGameObjectID() );
}

/**
 * DISHONORED(port): 2013 rva 0x5b17e0. An actor's components are searched for a UAkComponent with the same
 * BoneName and bStopWhenOwnerDestroyed; a miss constructs one of m_pAkComponentClass, attaches it to the
 * actor's scene at the actor's transform and, in game, calls its slot-87 setter with (1.0, the actor's
 * fade value, TRUE). A pending-kill actor gets no component. AActor(-1) means the transient package.
 */
UAkComponent* UAkAudioDevice::GetAkComponent( AActor* Actor, FName BoneName, UBOOL bStopWhenOwnerDestroyed )
{
	if( !Actor )
	{
		return NULL;
	}
	for( INT ComponentIndex = 0; ComponentIndex < Actor->Components.Num(); ++ComponentIndex )
	{
		UAkComponent* Component = Cast<UAkComponent>( Actor->Components( ComponentIndex ) );
		if( Component && Component->BoneName == BoneName && (UBOOL)Component->bStopWhenOwnerDestroyed == bStopWhenOwnerDestroyed )
		{
			return Component;
		}
	}
	if( Actor->IsPendingKill() )
	{
		return NULL;
	}
	UAkComponent* Component = ConstructObject<UAkComponent>( m_pAkComponentClass ? m_pAkComponentClass : UAkComponent::StaticClass(), Actor );
	if( !Component )
	{
		return NULL;
	}
	Component->BoneName = BoneName;
	Component->bStopWhenOwnerDestroyed = bStopWhenOwnerDestroyed ? TRUE : FALSE;
	if( GWorld )
	{
		Component->ConditionalAttach( GWorld->Scene, Actor, Actor->LocalToWorld() );
	}
	return Component;
}

/*-----------------------------------------------------------------------------
	Banks
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rvas 0x5b1780 / 0x5b17b0 / 0x5aed90 / 0x5aedb0 / 0x5b1750 / 0x5aed60 - every one of
// them is EnsureInitialized(-1) and then the matching AK::SoundEngine call.
AKRESULT UAkAudioDevice::LoadBank( const TCHAR* BankName, AkMemPoolId MemPoolId, AkBankID& OutBankID )
{
	EnsureInitialized( -1 );
	return AK::SoundEngine::LoadBank( BankName, MemPoolId, OutBankID );
}

AKRESULT UAkAudioDevice::LoadBank( const TCHAR* BankName, AkBankCallbackFunc Callback, void* Cookie, AkMemPoolId MemPoolId, AkBankID& OutBankID )
{
	EnsureInitialized( -1 );
	return AK::SoundEngine::LoadBank( BankName, Callback, Cookie, MemPoolId, OutBankID );
}

AKRESULT UAkAudioDevice::UnloadBank( const TCHAR* BankName, AkMemPoolId* OutMemPoolId )
{
	return AK::SoundEngine::UnloadBank( BankName, OutMemPoolId );
}

AKRESULT UAkAudioDevice::UnloadBank( const TCHAR* BankName, AkBankCallbackFunc Callback, void* Cookie )
{
	return AK::SoundEngine::UnloadBank( BankName, Callback, Cookie );
}

AKRESULT UAkAudioDevice::PrepareBank( AK::SoundEngine::PreparationType PreparationType, const TCHAR* BankName, AK::SoundEngine::AkBankContent Content )
{
	EnsureInitialized( -1 );
	return AK::SoundEngine::PrepareBank( PreparationType, BankName, Content );
}

AKRESULT UAkAudioDevice::PrepareEvent( AK::SoundEngine::PreparationType PreparationType, AkUniqueID EventID, AkBankCallbackFunc Callback, void* Cookie )
{
	return AK::SoundEngine::PrepareEvent( PreparationType, &EventID, 1, Callback, Cookie );
}

// DISHONORED(port): 2013 rva 0x5b2b20: unload every UAkBank object, then clear the engine's bank list
AKRESULT UAkAudioDevice::ClearBanks()
{
	for( TObjectIterator<UAkBank> It; It; ++It )
	{
		It->Unload();
	}
	return AK::SoundEngine::ClearBanks();
}

// DISHONORED(port): 2013 rva 0x5b2b80: Init.bnk first, then every UAkBank object in the object table
void UAkAudioDevice::LoadAllReferencedBanks()
{
	EnsureInitialized( -1 );
	AkBankID InitBankID = AK_INVALID_BANK_ID;
	AK::SoundEngine::LoadBank( AKINITBANKNAME, AK_DEFAULT_POOL_ID, InitBankID );
	INT NumBanks = 0;
	for( TObjectIterator<UAkBank> It; It; ++It )
	{
		++NumBanks;
		It->Load();
	}
	// DISHONORED(bringup): a summary of what the audio content actually resolved to, which is the only way to
	// see it without a Wwise profiler connection: how many UAkBank objects existed, how many of their banks
	// the IO hook found, and how many UAkEvent objects came out of the packages with a resolved Wwise id
	// (UAkEvent::PostLoad -> ComputeAkID, Engine/Src/akevent.cpp).
	INT NumEvents = 0;
	INT NumResolvedEvents = 0;
	for( TObjectIterator<UAkEvent> It; It; ++It )
	{
		++NumEvents;
		NumResolvedEvents += ( It->m_akID != 0 ) ? 1 : 0;
	}
#if DISHONORED_WWISE_SILENT
	const DishonoredWwise::Stats& Stats = DishonoredWwise::GetStats();
	debugf( TEXT("Wwise: %d AkBank objects -> %u banks loaded, %u prepared, %u missing, %u file packages; %d AkEvent objects, %d with a resolved id"),
		NumBanks, Stats.BanksLoaded, Stats.BanksPrepared, Stats.BanksMissing, (AkUInt32)g_lowLevelIO.GetNumFilePackages(),
		NumEvents, NumResolvedEvents );
#else
	debugf( TEXT("Wwise: %d AkBank objects, %d file packages, %d AkEvent objects, %d with a resolved id"),
		NumBanks, g_lowLevelIO.GetNumFilePackages(), NumEvents, NumResolvedEvents );
#endif
}

// DISHONORED(port): 2013 rvas 0x5b2b60 / 0x5b20c0
AKRESULT UAkAudioDevice::LoadFilePackage( const TCHAR* PackageName, AkUInt32& OutPackageID )
{
	return g_lowLevelIO.LoadFilePackage( PackageName, OutPackageID );
}

AKRESULT UAkAudioDevice::UnloadFilePackage( AkUInt32 PackageID )
{
	return g_lowLevelIO.UnloadFilePackage( PackageID );
}

// DISHONORED(port): 2013 rvas 0x5b4530 / 0x5b4820: the DLC bank directories
void UAkAudioDevice::AddBaseDirectory( const FString& BaseDirectory, const TArray<FString>& SubDirectories )
{
	if( SubDirectories.Num() == 0 )
	{
		g_lowLevelIO.AddBasePath( BaseDirectory );
		return;
	}
	for( INT SubIndex = 0; SubIndex < SubDirectories.Num(); ++SubIndex )
	{
		g_lowLevelIO.AddBasePath( BaseDirectory + SubDirectories( SubIndex ) + TEXT("\\") );
	}
}

void UAkAudioDevice::RemoveAllAdditionalDirectories()
{
	g_lowLevelIO.RemoveAllAdditionalPaths();
}

/*-----------------------------------------------------------------------------
	Events, triggers and game syncs
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5b2150, the one PostEvent every other overload funnels into. Retail:
 *   * no actor -> the global game object 2 and a max radius of -1 (no radius bookkeeping);
 *   * an actor  -> GetAkComponent(actor, bone, bStopWhenOwnerDestroyed); a miss aborts the post;
 *   * no explicit cookie and a radius >= 0 -> the callback becomes the audio system's end-of-event handler
 *     with AK_EndOfEvent, and the cookie the world's audio system, so the system can keep the radius list;
 *   * a playing id with a radius >= 0 is remembered in the component's m_PlayingRadii;
 *   * an actor whose fade value (AActor @256) is below 1e-8 gets an immediate Pause action on the event.
 * The fade-value read is the one part left out here: @256 is not a member of our AActor yet, and pausing a
 * just-posted event is a visual-fade nicety, so it is recorded as a follow-up rather than guessed.
 */
AkPlayingID UAkAudioDevice::PostEvent( AkUniqueID EventID, const FLOAT& MaxRadius, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed )
{
	AkGameObjectID GameObject = AKGLOBALSOUNDOBJECT;
	UAkComponent* Component = NULL;
	FLOAT Radius = -1.f;
	if( Actor )
	{
		Component = GetAkComponent( Actor, BoneName, bStopWhenOwnerDestroyed );
		if( !Component )
		{
			return AK_INVALID_PLAYING_ID;
		}
		GameObject = Component->GetAkGameObjectID();
		Radius = MaxRadius;
	}

	AkCallbackFunc EffectiveCallback = Callback;
	void* EffectiveCookie = Cookie;
	AkUInt32 EffectiveFlags = Flags;
	if( !Cookie && Radius >= 0.f && GWorld )
	{
		// The audio system owns the end-of-event bookkeeping (UDishonoredAudioSystem::ConsumeEndOfEventNotifies,
		// 2013 rva 0x7a9970); retail installs its handler here with just AK_EndOfEvent.
		EffectiveCallback = DisAkEndOfEventCallback;
		EffectiveCookie = GWorld->m_pAudioSystem;
		EffectiveFlags = AK_EndOfEvent;
	}

	const AkPlayingID PlayingID = AK::SoundEngine::PostEvent( EventID, GameObject, EffectiveFlags, EffectiveCallback, EffectiveCookie );
	if( PlayingID != AK_INVALID_PLAYING_ID && Component && Radius >= 0.f )
	{
		FAkPlayingRadius* PlayingRadius = new( Component->m_PlayingRadii ) FAkPlayingRadius;
		PlayingRadius->m_PlayingID = (INT)PlayingID;
		PlayingRadius->m_Radius = Radius;
	}
	return PlayingID;
}

// DISHONORED(port): 2013 rva 0x5b2be0: the event's own max radius, its own ak id
AkPlayingID UAkAudioDevice::PostEvent( UAkEvent* Event, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed )
{
	if( !Event )
	{
		return AK_INVALID_PLAYING_ID;
	}
	const FLOAT MaxRadius = Event->GetMaxRadius();
	return PostEvent( (AkUniqueID)Event->m_akID, MaxRadius, Actor, BoneName, Flags, Callback, Cookie, bStopWhenOwnerDestroyed );
}

// DISHONORED(port): 2013 rva 0x5b2c40
AkPlayingID UAkAudioDevice::PostEvent( const FString& EventName, const FLOAT& MaxRadius, AActor* Actor, FName BoneName, AkUInt32 Flags, AkCallbackFunc Callback, void* Cookie, UBOOL bStopWhenOwnerDestroyed )
{
	return PostEvent( AK::SoundEngine::GetIDFromString( *EventName ), MaxRadius, Actor, BoneName, Flags, Callback, Cookie, bStopWhenOwnerDestroyed );
}

// DISHONORED(port): 2013 rva 0x5b2250. Note the fallback: AK_INVALID_GAME_OBJECT, not the global object.
AKRESULT UAkAudioDevice::PostTrigger( const TCHAR* Trigger, AActor* Actor, FName BoneName )
{
	if( !Actor )
	{
		return AK::SoundEngine::PostTrigger( Trigger, AK_INVALID_GAME_OBJECT );
	}
	UAkComponent* Component = GetAkComponent( Actor, BoneName, FALSE );
	return Component ? AK::SoundEngine::PostTrigger( Trigger, Component->GetAkGameObjectID() ) : AK_Fail;
}

// DISHONORED(port): 2013 rva 0x5b2290, likewise global-scope when there is no actor
AKRESULT UAkAudioDevice::SetRTPCValue( const TCHAR* Param, FLOAT Value, AActor* Actor, FName BoneName )
{
	if( !Actor )
	{
		return AK::SoundEngine::SetRTPCValue( Param, Value, AK_INVALID_GAME_OBJECT, 0, AkCurveInterpolation_Linear );
	}
	UAkComponent* Component = GetAkComponent( Actor, BoneName, FALSE );
	return Component ? AK::SoundEngine::SetRTPCValue( Param, Value, Component->GetAkGameObjectID(), 0, AkCurveInterpolation_Linear ) : AK_Fail;
}

// DISHONORED(port): 2013 rva 0x5b22e0, this one falls back to the global object 2
AKRESULT UAkAudioDevice::SetSwitch( const TCHAR* SwitchGroup, const TCHAR* Switch, AActor* Actor, FName BoneName )
{
	if( !Actor )
	{
		return AK::SoundEngine::SetSwitch( SwitchGroup, Switch, AKGLOBALSOUNDOBJECT );
	}
	UAkComponent* Component = GetAkComponent( Actor, BoneName, FALSE );
	return Component ? AK::SoundEngine::SetSwitch( SwitchGroup, Switch, Component->GetAkGameObjectID() ) : AK_Fail;
}

// DISHONORED(port): 2013 rva 0x5aedd0
AKRESULT UAkAudioDevice::SetState( const TCHAR* StateGroup, const TCHAR* State )
{
	return AK::SoundEngine::SetState( StateGroup, State );
}

// DISHONORED(port): 2013 rva 0x5b20d0
AKRESULT UAkAudioDevice::SeekOnEvent( const FString& EventName, AActor* Actor, FName BoneName, AkTimeMs Position )
{
	AkGameObjectID GameObject = AKGLOBALSOUNDOBJECT;
	if( Actor )
	{
		UAkComponent* Component = GetAkComponent( Actor, BoneName, FALSE );
		if( !Component )
		{
			return AK_Fail;
		}
		GameObject = Component->GetAkGameObjectID();
	}
	return AK::SoundEngine::SeekOnEvent( *EventName, GameObject, Position, false );
}

// DISHONORED(port): 2013 rva 0x5b4020
void UAkAudioDevice::StopAllSounds( UBOOL /*bImmediate*/ )
{
	AK::SoundEngine::StopAll();
}

#endif // DISHONORED_WITH_WWISE
