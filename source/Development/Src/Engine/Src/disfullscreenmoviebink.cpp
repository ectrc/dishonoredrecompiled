/*=============================================================================
	DisFullScreenMovieBink.cpp: Arkane's Bink movie player.
	DISHONORED(port): retail source file of the 2012 PDB (18 functions, Unity_ArkSettingsParametersEtAl). The 2013 exe is the
	target; each function names its 2013 rva, the 2012 rva is the readable decompile of the same function. 2013 differences
	from 2012 are called out per function.
=============================================================================*/

#include "EnginePrivate.h"

#if USE_BINK_CODEC

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "../Bink/Src/DisFullScreenMovieBink.h"

/**
 * DISHONORED(port): Engine.ArkBinkOverlayManager (retail Engine class, IMPLEMENT_CLASS at disfullscreenmoviebink.cpp:13,
 * sizeof 56). The registrant lives in DishonoredGameRegistrants.cpp and the DishonoredGame module declares the same body as a
 * generated shim plus DishonoredGame/Inc/CppText/UArkBinkOverlayManager.h: keep the virtuals below identical to that file
 * (2013 vtable slots +292 .. +308, see there).
 */
#define ENABLE_DECLARECLASS_MACRO 1
#include "UnObjBas.h"
#undef ENABLE_DECLARECLASS_MACRO

class UArkBinkOverlayManager : public UObject
{
public:
	DECLARE_CLASS(UArkBinkOverlayManager,UObject,0,Engine)

	virtual void Tick() {}
	virtual void OnBinkTick(FLOAT DeltaTime) {}
	virtual void OnBinkRenderFrame(FViewport* Viewport, FCanvas* Canvas) {}
	virtual void LoadContentPackage() {}
	virtual void OnLoadingMovieStopped() {}
};

#undef DECLARE_CLASS
#undef DECLARE_CASTED_CLASS
#undef DECLARE_ABSTRACT_CLASS
#undef DECLARE_ABSTRACT_CASTED_CLASS


/** 2012 rva 0x57dad0 / 2013 rva 0x53d470 */
FFullScreenMovieSupport* FDisFullScreenMovieBink::StaticInitialize(UBOOL bUseSound)
{
	static FDisFullScreenMovieBink* StaticInstance = NULL;
	if( !StaticInstance )
	{
		StaticInstance = new FDisFullScreenMovieBink(bUseSound);
	}
	return StaticInstance;
}

/** 2012 rva 0x57d750 / 2013 rva 0x53d130: [DisFullScreenMovieBink] fLoadingDelay, bAlwaysAutoStart, +MapsToAutoStart */
FDisFullScreenMovieBink::FDisFullScreenMovieBink(UBOOL bUseSound)
:	FFullScreenMovieBink(bUseSound)
,	m_fLoadingDelay(0.f)
,	m_bAlwaysAutoStart(FALSE)
,	m_bIsStartupMovie(FALSE)
,	m_bIsLoadingMovie(FALSE)
,	m_fSkippableTimer(0.f)
,	m_pOverlayMgr(NULL)
{
	GConfig->GetFloat(TEXT("DisFullScreenMovieBink"), TEXT("fLoadingDelay"), m_fLoadingDelay, GEngineIni);
	GConfig->GetBool(TEXT("DisFullScreenMovieBink"), TEXT("bAlwaysAutoStart"), m_bAlwaysAutoStart, GEngineIni);
	FConfigSection* Section = GConfig->GetSectionPrivate(TEXT("DisFullScreenMovieBink"), FALSE, TRUE, GEngineIni);
	if( Section )
	{
		for( FConfigSectionMap::TIterator It(*Section); It; ++It )
		{
			if( It.Key() == TEXT("MapsToAutoStart") )
			{
				m_MapsToAutoStart.AddItem(FString(*It.Value()));
			}
		}
	}
}

/** 2012 rva 0x57d9a0 / 2013 rva 0x53d370 (compiler generated) */
FDisFullScreenMovieBink::~FDisFullScreenMovieBink()
{
}

/** 2012 rva 0x574510 / 2013 rva 0x532b70: a loading movie becomes skippable once m_fSkippableTimer runs out */
void FDisFullScreenMovieBink::Tick(FLOAT DeltaTime)
{
	FFullScreenMovieBink::Tick(DeltaTime);
	FScopeLock Lock(&m_CriticalSection);
	if( m_fSkippableTimer > 0.f )
	{
		m_fSkippableTimer -= DeltaTime;
		if( m_fSkippableTimer <= 0.01f )
		{
			bIsMovieSkippable = TRUE;
		}
	}
}

/**
 * 2012 rva 0x575130 / 2013 rva 0x533bb0: any key but Alt/Tab/wheel/mouse buttons skips a skippable loading movie (as Escape).
 * 2013: only the first local player's controller reaches the Bink input (2012 accepts every controller).
 */
UBOOL FDisFullScreenMovieBink::InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event, FLOAT AmountDepressed, UBOOL bGamepad)
{
	const UBOOL bSkippableLoading = m_bIsLoadingMovie && bIsMovieSkippable;
	if( bSkippableLoading && !bGamepad && Event == IE_Pressed
		&& Key != KEY_LeftAlt && Key != KEY_RightAlt && Key != KEY_Tab
		&& Key != KEY_MouseScrollUp && Key != KEY_MouseScrollDown
		&& Key != KEY_LeftMouseButton && Key != KEY_RightMouseButton && Key != KEY_MiddleMouseButton
		&& Key != KEY_ThumbMouseButton && Key != KEY_ThumbMouseButton2 )
	{
		Key = KEY_Escape;
	}

	UBOOL bResult = FALSE;
	if( bIsMovieSkippable )
	{
		const INT FirstPlayerControllerId = (GEngine && GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0)) ? GEngine->GamePlayers(0)->ControllerId : -1;
		if( ControllerId == FirstPlayerControllerId )
		{
			bResult = FFullScreenMovieBink::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
		}
	}

	if( bSkippableLoading && bResult )
	{
		if( m_IntroMovieToPlay.Len() == 0 )
		{
			OnLoadingMovieSkipped();
		}
		else
		{
			MovieFinishEvent->Trigger();
		}
	}
	return bResult || Event != IE_Released;
}

/**
 * 2012 rva 0x5752c0 / 2013 rva 0x533790: the loading movie loops until the level is loaded, then the intro plays.
 * 2013 also calls the online subsystem's vtable +320 (an empty UOnlineSubsystem virtual in retail, not declared in our
 * UOnlineSubsystem): DISHONORED(port) TODO once the OnlineSubsystem vtable converges.
 */
void FDisFullScreenMovieBink::GameThreadPlayLoadingMovieAndIntro(EMovieMode InMovieMode, const TCHAR* MovieFilename, const TCHAR* IntroFilename, INT StartFrame, INT InStartOfRenderingMovieFrame, INT InEndOfRenderingMovieFrame)
{
	m_bIsStartupMovie = FALSE;
	m_bIsLoadingMovie = TRUE;
	m_fSkippableTimer = 0.f;
	m_IntroMovieToPlay = IntroFilename;
	FFullScreenMovieBink::GameThreadPlayMovie((EMovieMode)(InMovieMode | MF_LoopPlayback), MovieFilename, StartFrame, InStartOfRenderingMovieFrame, InEndOfRenderingMovieFrame);
}

/**
 * 2012 rva 0x5745a0 / 2013 rva 0x532c00: a movie named *LOADING* loops. 2013 additions: the overlay manager loads its content
 * package first (vtable +304) and a *CREDITS* movie drops mode bit 0x100.
 */
void FDisFullScreenMovieBink::GameThreadPlayMovie(EMovieMode InMovieMode, const TCHAR* MovieFilename, INT StartFrame, INT InStartOfRenderingMovieFrame, INT InEndOfRenderingMovieFrame)
{
	if( m_pOverlayMgr )
	{
		m_pOverlayMgr->LoadContentPackage();
	}
	m_bIsStartupMovie = FALSE;
	m_bIsLoadingMovie = FALSE;
	m_fSkippableTimer = 0.f;
	DWORD Mode = InMovieMode;
	if( appStrfind(MovieFilename, TEXT("LOADING")) )
	{
		m_bIsLoadingMovie = TRUE;
		Mode |= MF_LoopPlayback;
	}
	else if( appStrfind(MovieFilename, TEXT("CREDITS")) )
	{
		Mode &= ~0x100;
	}
	FFullScreenMovieBink::GameThreadPlayMovie((EMovieMode)Mode, MovieFilename, StartFrame, InStartOfRenderingMovieFrame, InEndOfRenderingMovieFrame);
}

/**
 * 2012 rva 0x577270 / 2013 rva 0x535f20: a loading movie keeps playing (game rendering off) until the player may leave it.
 * 2013 additions: the online subsystem's vtable +324 first (see GameThreadPlayLoadingMovieAndIntro) and the overlay manager's
 * OnLoadingMovieStopped (vtable +308) before the stop. The audio system's resume (UWorld::m_pAudioSystem vtable +320, a
 * DishonoredGame shim class without virtuals here) is a DISHONORED(port) TODO.
 */
void FDisFullScreenMovieBink::GameThreadStopMovie(FLOAT DelayInSeconds, UBOOL bWaitForMovie, UBOOL bForceStop)
{
	if( GameThreadIsMoviePlaying(TEXT("")) && m_bIsLoadingMovie )
	{
		if( !OnRequestLoadingMovieExit() )
		{
			FViewport::SetGameRenderingEnabled(FALSE, 0);
			return;
		}
	}
	if( m_pOverlayMgr )
	{
		m_pOverlayMgr->OnLoadingMovieStopped();
	}
	FFullScreenMovieBink::GameThreadStopMovie(DelayInSeconds, bWaitForMovie, bForceStop);
}

/** 2012 rva 0x574580 / 2013 rva 0x532be0 */
void FDisFullScreenMovieBink::GameThreadInitiateStartupSequence()
{
	FFullScreenMovieBink::GameThreadInitiateStartupSequence();
	m_bIsStartupMovie = TRUE;
}

/**
 * 2012 rva 0x577170 / 2013 rva 0x535e20: when a loading movie with a pending intro may not be left yet, wait for the player,
 * then play the intro once.
 */
void FDisFullScreenMovieBink::GameThreadRequestDelayedStopMovie()
{
	if( GameThreadIsMoviePlaying(TEXT("")) && m_bIsLoadingMovie )
	{
		if( m_IntroMovieToPlay.Len() > 0 && !OnRequestLoadingMovieExit() )
		{
			if( m_pOverlayMgr )
			{
				m_pOverlayMgr->Tick();
			}
			GameThreadWaitForMovie();
			MovieFinishEvent->Reset();
			m_bIsLoadingMovie = FALSE;
			GameThreadStopMovie(0.f, TRUE, TRUE);
			GameThreadPlayMovie(MM_PlayOnceFromStream, *m_IntroMovieToPlay);
			FViewport::SetGameRenderingEnabled(FALSE, 0);
			GameThreadWaitForMovie();
			m_IntroMovieToPlay = TEXT("");
			OnLoadingMovieSkipped();
		}
	}
	FFullScreenMovieBink::GameThreadRequestDelayedStopMovie();
}

/** 2012 rva 0x5746b0 / 2013 rva 0x532d10 */
void FDisFullScreenMovieBink::OnBinkTick(FLOAT DeltaTime)
{
	if( m_pOverlayMgr )
	{
		m_pOverlayMgr->OnBinkTick(DeltaTime);
	}
}

/** 2012 rva 0x5746e0 / 2013 rva 0x532d40 */
void FDisFullScreenMovieBink::OnBinkRenderFrame(FViewport* Viewport, FCanvas* Canvas)
{
	if( m_pOverlayMgr )
	{
		m_pOverlayMgr->OnBinkRenderFrame(Viewport, Canvas);
	}
}

/**
 * 2012 rva 0x575640 / 2013 rva 0x533dc0: TRUE when the loading movie may end now. Outside the startup sequence, unless
 * bAlwaysAutoStart or the map is in MapsToAutoStart (or an auto-test runs), the UI pauses the game (2013: m_bBinkPause + UpdatePausedState(0), vtable +336; 2012: eventPauseGame(TRUE, 0)) and the
 * movie waits for a key, skippable at once or after fLoadingDelay.
 */
UBOOL FDisFullScreenMovieBink::OnRequestLoadingMovieExit()
{
	UBOOL bExit = TRUE;
	if( GWorld && GWorld->PersistentLevel && GEngine )
	{
		const FString MapName = GWorld->GetMapName();
		if( !m_bIsStartupMovie && !m_bAlwaysAutoStart && !m_MapsToAutoStart.ContainsItem(MapName) )
		{
			AGameInfo* GameInfo = GWorld->GetGameInfo();
			if( !GameInfo || !GameInfo->MyAutoTestManager )
			{
				UGameUISceneClient* SceneClient = UUIRoot::GetSceneClient();
				if( SceneClient )
				{
					SceneClient->m_bBinkPause = TRUE;
					SceneClient->UpdatePausedState(0);
					bExit = FALSE;
					if( m_fLoadingDelay <= 0.f )
					{
						bIsMovieSkippable = TRUE;
					}
					else
					{
						FScopeLock Lock(&m_CriticalSection);
						m_fSkippableTimer = m_fLoadingDelay;
					}
				}
			}
		}
	}
	return bExit;
}

/**
 * 2012 rva 0x574640 / 2013 rva 0x532cd0: unpause (2013: clears m_bBinkPause, UpdatePausedState(0); 2012: eventPauseGame(FALSE, 0)). The audio system's resume (vtable +320) is a
 * DISHONORED(port) TODO (see GameThreadStopMovie).
 */
void FDisFullScreenMovieBink::OnLoadingMovieSkipped()
{
	UGameUISceneClient* SceneClient = UUIRoot::GetSceneClient();
	if( SceneClient )
	{
		SceneClient->m_bBinkPause = FALSE;
		SceneClient->UpdatePausedState(0);
	}
}

#endif
