// DishonoredGame/src/dishonoredengine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (61):
//   0x62b950  public: static void __cdecl UDishonoredEngine::InitializePrivateStaticClassUDishonoredEngine(void)
//   0x62b970  public: void __thiscall FDisAsyncSaveGameLister::Wait(void)
//   0x62b9a0  public: unsigned int __thiscall UDishonoredEngine::IsSavingAllowed(void)const
//   0x62b9c0  public: unsigned int __thiscall UDishonoredEngine::IsSaving(void)const
//   0x62b9e0  public: virtual void __thiscall UDishonoredEngine::OpenControllerConnectionMenu(void)const
//   0x62ba10  public: void __thiscall UDishonoredEngine::CancelMissionStartSave(class UDisSeqAct_AutoSave *)
//   0x62ba40  public: void __thiscall UDishonoredEngine::ResetPeriodicAutosave(void)
//   0x62ba80  public: virtual void __thiscall UDishonoredEngine::PushIgnoreAutosave(unsigned char)
//   0x62baa0  public: virtual void __thiscall UDishonoredEngine::PopIgnoreAutosave(unsigned char)
//   0x62bac0  public: virtual unsigned int __thiscall UDishonoredEngine::Exec(wchar_t const *, class FOutputDevice &)
//   0x62bb00  public: void __thiscall UDishonoredEngine::RequestLevelStateSaving(unsigned int)
//   0x62bb30  public: void __thiscall UDishonoredEngine::SetSaveLoadEnabled(unsigned int)
//   0x62bb50  public: unsigned int __thiscall UDishonoredEngine::IsSaveLoadEnabled(void)const
//   0x62bb60  public: unsigned int __thiscall UDishonoredEngine::IsSaveGameListReady(void)const
//   0x62bb80  public: void __thiscall UDishonoredEngine::WaitSaveGameListReady(void)const
//   0x62bb90  public: virtual unsigned int __thiscall UDishonoredEngine::IsLoadingGame(void)const
//   0x62bbb0  public: virtual unsigned int __thiscall UDishonoredEngine::IsLoadingLevelState(void)const
//   0x62bbc0  public: virtual void __thiscall UDishonoredEngine::PreExit(void)
//   0x62bc30  protected: virtual void __thiscall UDishonoredEngine::StopMovie(unsigned int)
//   0x63ed20  GetSaveGameSlot
//   0x641fc0  public: __thiscall FDisAsyncSaveGameDeleter::FDisAsyncSaveGameDeleter(wchar_t const *)
//   0x642090  public: int __thiscall UDishonoredEngine::GetNextAutoSaveSlot(void)
//   0x6421c0  public: int __thiscall UDishonoredEngine::GetNextUserSaveSlot(void)const
//   0x6422b0  public: virtual void __thiscall UDishonoredEngine::OpenPauseMenu(void)
//   0x642490  public: virtual void __thiscall UDishonoredEngine::OnControllerDisconnected(int)
//   0x642540  public: unsigned int __thiscall UDishonoredEngine::HasSaveGame(int)const
//   0x6425f0  public: struct FDisSaveGame * __thiscall UDishonoredEngine::GetSaveGame(int)const
//   0x642660  public: void __thiscall UDishonoredEngine::DeleteSaveGame(int)
//   0x642750  public: virtual void __thiscall UDishonoredEngine::NotifyActorDestroyed(class AActor *)
//   0x6427d0  public: struct FMapConfig * __thiscall UDishonoredEngine::FindMapConfig(class FString const &)
//   0x642900  public: struct FMapConfig * __thiscall UDishonoredEngine::FindMapConfigFromFriendlyName(class FString const &)
//   0x648180  public: virtual void __thiscall FDisAsyncSaveGameDeleter::DoWork(void)
//   0x6482b0  public: virtual void __thiscall UDishonoredEngine::PopDisableSave(unsigned char, float)
//   0x648300  public: unsigned int __thiscall UDishonoredEngine::IsObjectPartOfSavedLevelState(class UObject *)const
//   0x648310  public: virtual unsigned int __thiscall UDishonoredEngine::PlayLoadMapMovie(class FString const &, class FString const &)
//   0x64cca0  public: void __thiscall UDishonoredEngine::CancelPendingAutosave(class UDisSeqAct_AutoSave *)
//   0x64ccf0  public: void __thiscall UDishonoredEngine::CancelAllPendingAutosaves(void)
//   0x64cd80  public: virtual void __thiscall UDishonoredEngine::Dis_Load(int)
//   0x64ce40  public: virtual void __thiscall UDishonoredEngine::PushDisableSave(unsigned char)
//   0x64cf10  public: virtual unsigned int __thiscall UDishonoredEngine::LoadMap(struct FURL const &, class UPendingLevel *, class FString &)
//   0x64f3b0  public: virtual void __thiscall FDisAsyncSaveGameLister::DoWork(void)
//   0x64f8c0  private: void __thiscall UDishonoredEngine::DoSaveGame(int, struct TMemStackArray<class UDisSeqAct_AutoSave *> const &)
//   0x64f960  private: void __thiscall UDishonoredEngine::TryDoMissionStartSave(void)
//   0x64fa60  public: void __thiscall UDishonoredEngine::QueueMissionStartSave(int, class UDisSeqAct_AutoSave *)
//   0x64fab0  public: void __thiscall UDishonoredEngine::RefreshSaveGameList(void)
//   0x650fb0  private: void __thiscall UDishonoredEngine::ProcessSaveComplete(unsigned int)
//   0x6510c0  public: virtual void __thiscall UDishonoredEngine::Dis_Save(int)
//   0x653070  private: void __thiscall UDishonoredEngine::TryDoAutosave(void)
//   0x654ac0  public: void __thiscall UDishonoredEngine::QueueAutosave(class UDisSeqAct_AutoSave *)
//   0x654b40  public: void __thiscall UDishonoredEngine::PeriodicAutosave(float)
//   0x657160  public: static class UClass * __cdecl UDishonoredEngine::GetPrivateStaticClassUDishonoredEngine(wchar_t const *)
//   0x6584e0  public: static class UClass * __cdecl UDishonoredEngine::StaticClassNoInline(void)
//   0x65c990  private: void __thiscall UDishonoredEngine::WriteSaveGame(void)
//   0x65cb10  private: void __thiscall UDishonoredEngine::LoadGame(void)
//   0x65d250  public: void __thiscall UDishonoredEngine::DiscardLevelState(class FName const &)
//   0x65d280  public: virtual void __thiscall UDishonoredEngine::Init(void)
//   0x65d5d0  public: virtual void __thiscall UDishonoredEngine::PreCommitMapChange(void)
//   0x65d6b0  public: virtual void __thiscall UDishonoredEngine::PostCommitMapChange(void)
//   0x65f100  private: void __thiscall UDishonoredEngine::ProcessSaveLoadCmd(float &)
//   0x661150  public: virtual void __thiscall UDishonoredEngine::Tick(float)
//   0xba34b0  _dynamic_initializer_for__gs_SaveFileError__

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): chapters 16..25 (DLC06/DLC07 missions) never publish a chaos level, like chapter 0 (2013 rvas 0x5ecf90,
// 0x5fb1b0, 0x5fb230, 0x601b80 inline the same test; 2012 only tests chapter 0)
static UBOOL IsChaosHiddenForChapter( INT Chapter )
{
	return Chapter == 0 || ( Chapter >= 16 && Chapter <= 25 );
}

// DISHONORED(written): 2013 rva 0x614e20 (2012 0x65d280). Differences from 2012: the 2012 FDisAsyncSaveGameLister queued here is
// gone, m_TransitionSaveType is reset, the DLC06->DLC07 chaos bits are cleared, m_dLastAutoSaveTime has no +16777216 offset,
// the controller id test reads m_bInitControllerToZero at its 2013 bit (0x40 @700).
void UDishonoredEngine::Init()
{
	// DISHONORED(bringup): DisSaveLoad::FGameState (156 bytes, 2013 ctor rva 0x6147a0) is not ported; m_pGameState stays NULL
	m_TransitionSaveType = 0;
	// DISHONORED(written): 2013 rva 0x601210 is left out on purpose: the 2026 patch's telemetry (libcurl "game_start" POST, a
	// heartbeat every 120 s from Tick, a persistent id in SaveData\Puid.txt); its only other effect is creating <GameDir>\SaveData\.
	UGameEngine::Init();

	const INT InitialControllerId = ( m_bInitControllerToZero || GIsEditor ) ? 0 : -1;
	for( INT PlayerIndex = 0; PlayerIndex < GamePlayers.Num(); PlayerIndex++ )
	{
		GamePlayers(PlayerIndex)->ControllerId = InitialControllerId;
	}

	// DISHONORED(bringup): UDishonoredActivePowerComponent::GetPowerClasses (2013 rva 0x7f9ea0) only warms its static FClassTree
	// cache here (the result is discarded); not ported
	if( GUseSeekFreeLoading )
	{
		UGameEngine* GameEngine = Cast<UGameEngine>( GEngine );
		if( GameEngine && GameEngine->DLCEnumerator )
		{
			GameEngine->DLCEnumerator->DLCRootDir = GIsSeekFreePCConsole ? TEXT("../../DishonoredGame/DLC/PCConsole/") : TEXT("../../DishonoredGame/DLC/PC/");
		}
		UDownloadableContentManager* DLCManager = GameEngine ? GameEngine->DLCManager : NULL;
		if( DLCManager )
		{
			DLCManager->ProcessEvent( DLCManager->FindFunctionChecked( FName(TEXT("RefreshDLCFromNative")) ), NULL );
		}
		for( FObjectIterator It; It; ++It )
		{
			const DWORD ClassFlags = It->GetClass()->ClassFlags;
			if( ( ClassFlags & ( CLASS_Config | CLASS_Localized ) ) && !( ClassFlags & CLASS_PerObjectConfig ) )
			{
				It->ReloadConfig();
				It->ReloadLocalized();
			}
		}
	}

	m_dLastAutoSaveTime = appSeconds();
	// DISHONORED(bringup): UDisBinkOverlayManager::Initialize (2013 rva 0x79e440) registers the overlay manager with GFullScreenMovie
	// (Bink movie player not ported); the static transition-save buffer retail empties here has no writer in our tree yet
	// DISHONORED(bringup): UInterpTrackFaceTo::SetFaceToPriority(15), UInterpTrackLocomotion::SetLocomotionPriority(11),
	// UInterpTrackLookAt::SetLookatPriority(1, 5, 19) (2013 rvas 0x4fc8a0 / 0x4fcc30 / 0x4fcc90) are Engine statics our Engine lacks
	m_bDLC06ToDLC07LowChaosFound = FALSE;
	m_bDLC06ToDLC07HighChaosFound = FALSE;
}

// DISHONORED(written): 2013 rva 0x5fbdc0 (2012 0x6427d0): case-insensitive search of m_MapConfig by map name
FMapConfig* UDishonoredEngine::FindMapConfig( const FString& MapName )
{
	for( INT ConfigIndex = 0; ConfigIndex < m_MapConfig.Num(); ConfigIndex++ )
	{
		if( appStricmp( *MapName, *m_MapConfig(ConfigIndex).m_Name ) == 0 )
		{
			return &m_MapConfig(ConfigIndex);
		}
	}
	return NULL;
}

// DISHONORED(written): 2013 rva 0x601b80 (2012 0x648310). 2013 adds m_dSaveNotificationMovieEndTime = -1 up front, the 16..25
// chapter rule, and the save-notification test reads FMapConfig::m_bIsSaveNotificationMovie (bit 2; 2012 bit 1).
UBOOL UDishonoredEngine::PlayLoadMapMovie( const FString& MapName, const FString& MovieName )
{
	FMapConfig* MapConfig = FindMapConfig( MapName );
	m_dSaveNotificationMovieEndTime = -1.0;
	if( MapConfig )
	{
		if( MapConfig->m_nRichPresenceChapter != m_nCurrentRichPresenceChapter )
		{
			m_nCurrentRichPresenceChapter = MapConfig->m_nRichPresenceChapter;
			if( IsChaosHiddenForChapter( m_nCurrentRichPresenceChapter ) )
			{
				m_nCurrentRichPresenceChaos = 0;
			}
		}
		if( MapConfig->m_bIsSaveNotificationMovie && m_bShowSaveNotificationMovie )
		{
			// DISHONORED(bringup): m_pBinkOverlayManager->OnPlaySaveNotificationMovie() (2013 rva 0x7b69c0), Bink not ported
			m_bShowSaveNotificationMovie = FALSE;
			m_dSaveNotificationMovieEndTime = appSeconds() + m_fSaveNotificationMovieMinimumDuration;
		}
		// DISHONORED(bringup): otherwise m_pBinkOverlayManager->OnPlayLoadingMovie(MapName, TRUE, m_bShowLoadingTexts) (2013 rva 0x7b60e0)
	}
	// DISHONORED(bringup): without a map config OnPlayLoadingMovie(MapName, FALSE, m_bShowLoadingMapNameAndHints).
	// Retail chains to UEngine::PlayLoadMapMovie(MapName, MapConfig ? m_LoadingMovieName : MovieName) (2013 rva 0x2097d0: the
	// [FullScreenMovie] entry keyed by the map name, else LoadMapMovies); our UEngine only has the reference no-argument version.
	return UEngine::PlayLoadMapMovie();
}

// DISHONORED(written): 2013 rva 0x605150 (2012 0x6422b0): nothing happens while a movie plays
void UDishonoredEngine::OpenPauseMenu()
{
	if( GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")) )
	{
		return;
	}
	// DISHONORED(bringup): the pause menu is UDisGlobalUIManager / UDisGFxMoviePlayerPauseMenu (Scaleform, WITH_GFx=0); the
	// request is dropped the way retail drops it on the main menu or while another menu is up
	m_bRequestOpenPauseMenu_FocusLost = FALSE;
	m_bRequestOpenPauseMenu_NoController = FALSE;
}

// DISHONORED(written): the 2013 vtable slot +304 is the empty base body (2013 rva 0x1cb0c0); the 2012 override (0x642490) is gone
void UDishonoredEngine::OnControllerDisconnected( INT ControllerId )
{
}

// DISHONORED(written): 2013 rva 0x5e4270: UDisGlobalUIManager::m_pGlobal vtable +500 with the UEngine byte @1476
void UDishonoredEngine::OpenControllerConnectionMenu() const
{
	// DISHONORED(bringup): UDisGFxMoviePlayerGlobal (Scaleform, WITH_GFx=0)
}

// DISHONORED(written): 2013 rva 0x5e42a0, new in 2013: UDisGlobalUIManager::m_pGlobal vtable +560
void UDishonoredEngine::OpenContentUnavailableMenu() const
{
	// DISHONORED(bringup): UDisGFxMoviePlayerGlobal (Scaleform, WITH_GFx=0)
}

// DISHONORED(written): 2013 rva 0x5e4500 (2012 0x62bb90 tested m_SaveLoadCmd)
UBOOL UDishonoredEngine::IsLoadingGame() const
{
	return m_SaveLoadMode == SLM_Loading;
}

// DISHONORED(written): 2013 rva 0x5e41c0: no disable-save reference held and no pending wait. Retail then asks the game info
// (ADishonoredGameInfo vtable +1092; 2013 0x17d890 = return TRUE in the base game info), not declared in our class yet.
UBOOL UDishonoredEngine::IsSavingAllowed() const
{
	UBOOL bAllowed = m_fDisallowSavingTimer <= 0.f;
	for( INT TypeIndex = 0; TypeIndex < DDST_MAX; TypeIndex++ )
	{
		if( m_DisallowSavingRefCount[TypeIndex] )
		{
			bAllowed = FALSE;
		}
	}
	return bAllowed;
}

// DISHONORED(written): 2013 rva 0x60b680 (2012 0x6510c0)
void UDishonoredEngine::Dis_Save( INT SaveSlot )
{
	// DISHONORED(bringup): DoSaveGame (2013 rva 0x6095a0) and the DisSaveLoad archive are not ported, so save requests are dropped.
	// The retail gate is SaveSlot 2/3 (mission start) or > 14, or IsSavingAllowed() without a pending mission-start save.
	static UBOOL bWarned = FALSE;
	if( !bWarned )
	{
		bWarned = TRUE;
		warnf( TEXT("DISHONORED(bringup): UDishonoredEngine::Dis_Save(%d) dropped, save system not ported"), SaveSlot );
	}
}

// DISHONORED(written): 2013 rva 0x60c750 (2012 0x64cd80)
void UDishonoredEngine::Dis_Load( INT SaveSlot )
{
	// DISHONORED(bringup): retail clamps the slot to 0..55, cancels pending autosaves and sets m_SaveLoadCmd = SLC_Load for
	// ProcessSaveLoadCmd (Tick, 2013 rva 0x617890); neither the loader nor the Tick override is ported, so the request is dropped
	static UBOOL bWarned = FALSE;
	if( !bWarned )
	{
		bWarned = TRUE;
		warnf( TEXT("DISHONORED(bringup): UDishonoredEngine::Dis_Load(%d) dropped, save system not ported"), SaveSlot );
	}
}

// DISHONORED(written): 2013 rva 0x605330 (2012 0x64ce40). 2013 keeps one count per EDisDisableSaveType instead of the 2012 total
// plus debug list. A pending save request (SLC_Save) is cancelled and its autosave actions denied.
void UDishonoredEngine::PushDisableSave( BYTE DisableSaveType )
{
	m_DisallowSavingRefCount[DisableSaveType]++;
	if( m_SaveLoadCmd == SLC_Save )
	{
		m_SaveLoadCmd = SLC_None;
		for( INT ActionIndex = 0; ActionIndex < m_SaveActionsToNotify.Num(); ActionIndex++ )
		{
			m_SaveActionsToNotify(ActionIndex)->SetAutoSaveDenied();
		}
		m_SaveActionsToNotify.Empty();
	}
}

// DISHONORED(written): 2013 rva 0x5f95f0 (2012 0x6482b0)
void UDishonoredEngine::PopDisableSave( BYTE DisableSaveType, FLOAT WaitToEnable )
{
	if( m_DisallowSavingRefCount[DisableSaveType] )
	{
		m_DisallowSavingRefCount[DisableSaveType]--;
		m_fDisallowSavingTimer = Max( m_fDisallowSavingTimer, WaitToEnable );
	}
}

// DISHONORED(written): 2013 rva 0x5e4340 (2012 0x62ba80)
void UDishonoredEngine::PushIgnoreAutosave( BYTE Type )
{
	m_IgnoreAutosavingRefCount[Type]++;
}

// DISHONORED(written): 2013 rva 0x5e4360 (2012 0x62baa0)
void UDishonoredEngine::PopIgnoreAutosave( BYTE Type )
{
	if( m_IgnoreAutosavingRefCount[Type] )
	{
		m_IgnoreAutosavingRefCount[Type]--;
	}
}

// DISHONORED(written): the 2013 vtable slot +536 is an empty body (2013 rva 0x1cb0c0); Tick refreshes m_bUsingGamepad instead
void UDishonoredEngine::OnControllerChanged( UBOOL bUsingGamepad )
{
}

// DISHONORED(written): 2013 rva 0x5ecf90 (2012 0x633a00)
void UDishonoredEngine::execPublishRichPresence( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	if( IsChaosHiddenForChapter( m_nCurrentRichPresenceChapter ) )
	{
		m_nCurrentRichPresenceChaos = 0;
	}
}

// DISHONORED(written): 2013 rva 0x5fb230 (2012 0x641b80)
void UDishonoredEngine::execUpdateRichPresenceChaos( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(nCurrentRichPresenceChaos);
	P_FINISH;
	if( nCurrentRichPresenceChaos != m_nCurrentRichPresenceChaos )
	{
		m_nCurrentRichPresenceChaos = nCurrentRichPresenceChaos;
		if( IsChaosHiddenForChapter( m_nCurrentRichPresenceChapter ) )
		{
			m_nCurrentRichPresenceChaos = 0;
		}
	}
}

// DISHONORED(written): 2013 rva 0x5fb1b0 (2012 0x641b10)
void UDishonoredEngine::execUpdateRichPresenceChapter( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(nCurrentRichPresenceChapter);
	P_FINISH;
	if( nCurrentRichPresenceChapter != m_nCurrentRichPresenceChapter )
	{
		m_nCurrentRichPresenceChapter = nCurrentRichPresenceChapter;
		if( IsChaosHiddenForChapter( nCurrentRichPresenceChapter ) )
		{
			m_nCurrentRichPresenceChaos = 0;
		}
	}
}

// DISHONORED(written): 2013 rva 0x1c5e90 = UEngine::execOpenContentUnavailableMenu (vtable +312)
void UDishonoredEngine::execOpenContentUnavailableMenu( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	OpenContentUnavailableMenu();
}

// DISHONORED(written): 2013 rva 0x1d4820 = UEngine::execOpenControllerConnectionMenu (vtable +308)
void UDishonoredEngine::execOpenControllerConnectionMenu( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	OpenControllerConnectionMenu();
}

// DISHONORED(written): 2013 rva 0x1d47c0 = UEngine::execOnControllerDisconnected (vtable +304)
void UDishonoredEngine::execOnControllerDisconnected( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(ControllerId);
	P_FINISH;
	OnControllerDisconnected( ControllerId );
}

// DISHONORED(written): 2013 rva 0x5ecf20 (vtable +536)
void UDishonoredEngine::execOnControllerChanged( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(bUsingGamepad);
	P_FINISH;
	OnControllerChanged( bUsingGamepad );
}

// DISHONORED(written): 2013 rva 0x1c8910 = UEngine::execOpenPauseMenu (vtable +300)
void UDishonoredEngine::execOpenPauseMenu( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	OpenPauseMenu();
}

// DISHONORED(written): 2013 rva 0x1eee70 = UEngine::execPlayLoadMapMovie (vtable +296)
void UDishonoredEngine::execPlayLoadMapMovie( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(MapName);
	P_GET_STR(MovieName);
	P_FINISH;
	*(UBOOL*)Result = PlayLoadMapMovie( MapName, MovieName );
}

// DISHONORED(written): 2013 rva 0x5ecec0 (vtable +532)
void UDishonoredEngine::execPopIgnoreAutosave( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(Type);
	P_FINISH;
	PopIgnoreAutosave( Type );
}

// DISHONORED(written): 2013 rva 0x5f6f70 (vtable +528)
void UDishonoredEngine::execPushIgnoreAutosave( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(Type);
	P_FINISH;
	PushIgnoreAutosave( Type );
}

// DISHONORED(written): 2013 rva 0x5ece30 (vtable +524)
void UDishonoredEngine::execPopDisableSave( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(DisableSaveType);
	P_GET_FLOAT_OPTX(WaitToEnable, 0.f);
	P_FINISH;
	PopDisableSave( DisableSaveType, WaitToEnable );
}

// DISHONORED(written): 2013 rva 0x5ecdd0 (vtable +520)
void UDishonoredEngine::execPushDisableSave( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(DisableSaveType);
	P_FINISH;
	PushDisableSave( DisableSaveType );
}

// DISHONORED(written): 2013 rva 0x5ecd70 (vtable +516)
void UDishonoredEngine::execDis_Load( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(SaveSlot);
	P_FINISH;
	Dis_Load( SaveSlot );
}

// DISHONORED(written): 2013 rva 0x5ecd10 (vtable +512)
void UDishonoredEngine::execDis_Save( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(SaveSlot);
	P_FINISH;
	Dis_Save( SaveSlot );
}
