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
#include "dishonoredutilities_saveload.h"
#include "dissaveload_classlists.h"	// DISHONORED(written): agent ED - the ported/unported override lists

void DisSaveLoadArmRestore();	// DISHONORED(written): agent ED, at the foot of this file
#include <sys/stat.h>

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

	// DISHONORED(port): agent CF - retail creates <GameDir>\SaveData\ here (2013 rva 0x614e20) and refreshes
	// the save list from ProcessSaveLoadCmd (2013 0x6162d0) and the UI (0x7d9f80), neither of which is ported;
	// kicking the lister off here is the bring-up placement, so the menu has a list the first time it asks.
	GFileManager->MakeDirectory( *DisGetSaveGameDir(), TRUE );
	SetSaveLoadEnabled( TRUE );
	RefreshSaveGameList();
	if( ParseParam( appCmdLine(), TEXT("savetest") ) )
	{
		DisSaveGameSelfTest();
	}
	INT LoadSlot = 0;
	if( Parse( appCmdLine(), TEXT("disloadslot="), LoadSlot ) )
	{
		Dis_Load( LoadSlot );
	}
	// DISHONORED(written): agent ED - -disrestoreslot=<slot> defers the load to the per-world-tick hook, so
	// that the map is open and its sub-levels are visible before the object layer runs (retail's SLC_PostLoad)
	DisSaveLoadArmRestore();
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
	// DISHONORED(port): retail chains to UEngine::PlayLoadMapMovie(MapName, MapConfig ? m_LoadingMovieName : MovieName)
	// (2013 rva 0x2097d0: the [FullScreenMovie] entry keyed by the map name, else a random LoadMapMovies entry). The call is
	// qualified on purpose: agent AI's reference no-argument UEngine::PlayLoadMapMovie() forwards to this two-argument virtual, so
	// chaining to the no-argument one recursed until the stack overflowed (0xC00000FD in FindMapConfig).
	return UEngine::PlayLoadMapMovie( MapName, MapConfig ? MapConfig->m_LoadingMovieName : MovieName );
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
	// DISHONORED(bringup): the container half of DisSaveLoad is ported (agent CF: FGameState::Save, 2013 rva 0x612340) but
	// DoSaveGame (0x6095a0) and FGameState::SaveGameState (0x602920) are not, so there are no level states to write and a
	// save here would produce a file with an empty game state and overwrite the slot. Dropped on purpose.
	// The retail gate is SaveSlot 2/3 (mission start) or > 14, or IsSavingAllowed() without a pending mission-start save.
	static UBOOL bWarned = FALSE;
	if( !bWarned )
	{
		bWarned = TRUE;
		warnf( TEXT("DISHONORED(bringup): UDishonoredEngine::Dis_Save(%d) dropped, save system not ported"), SaveSlot );
	}
}

// DISHONORED(port): 2013 rva 0x60c750 (2012 0x64cd80). Retail clamps the slot, cancels a pending autosave and sets
// m_SaveLoadCmd = SLC_Load so ProcessSaveLoadCmd (2013 rva 0x6162d0, not ported) drives the rest; agent CF calls
// LoadGame straight away instead, which is the same body one tick earlier.
void UDishonoredEngine::Dis_Load( INT SaveSlot )
{
	if( SaveSlot < 0 || SaveSlot > DIS_SAVE_SLOT_MAX )
	{
		return;
	}
	m_SavegameSlot = SaveSlot;
	m_SaveLoadMode = SLM_Loading;
	m_SaveLoadCmd = SLC_Load;
	LoadGame();
}

// DISHONORED(port): 2013 rva 0x614c90 (2012 0x65cb10). Reads the slot's file into the game state and records whether it
// failed; it opens no map - the travel and the object restore are SLC_PostLoad's, in ProcessSaveLoadCmd.
void UDishonoredEngine::LoadGame()
{
	const UBOOL bMissionStartSave = m_SavegameSlot >= DIS_SAVE_SLOT_FIRST_NAMED && m_SavegameSlot <= 12;
	const UBOOL bChaosMarker = m_SavegameSlot == DIS_SAVE_SLOT_DLC02_LOW || m_SavegameSlot == DIS_SAVE_SLOT_DLC02_HIGH;
	const FString SavePath = DisGetSaveGamePath( m_SavegameSlot );

	DisSaveLoad::FGameState* State = (DisSaveLoad::FGameState*)m_pGameState;
	if( State == NULL )
	{
		State = new DisSaveLoad::FGameState();
		m_pGameState = State;
	}

	FArchive* Reader = GFileManager->CreateFileReader( *SavePath, 0, GNull );
	if( Reader == NULL )
	{
		m_SaveLoadStatus = SLS_Failed;
		warnf( TEXT("DisLoadGame: slot %d (%s) could not be opened"), m_SavegameSlot, *SavePath );
		return;
	}
	if( bChaosMarker )
	{
		// DISHONORED(bringup): the third argument of FGameState::Load (2013 rva 0x614020) selects a short read that
		// only recovers the DLC02 chaos flag; not ported, and nothing in the base campaign asks for it
		delete Reader;
		m_SaveLoadStatus = SLS_Failed;
		return;
	}
	const UBOOL bLoaded = State->Load( *Reader );
	delete Reader;

	if( !bLoaded )
	{
		m_SaveLoadStatus = SLS_Failed;
		warnf( TEXT("DisLoadGame: slot %d (%s) refused: corrupt %d, newer than this build %d, missing content %d"),
			m_SavegameSlot, *SavePath, State->IsCorrupt(), State->IsNewerThanBuild(), State->IsMissingContent() );
		return;
	}
	m_SaveLoadStatus = SLS_Succeeded;
	warnf( TEXT("DisLoadGame: slot %d loaded '%s' (mission %d, %d level states, %d names%s); root level %s"),
		m_SavegameSlot, *State->GetData().m_SaveDetails, State->GetData().m_nMissionIndex,
		State->NumLevelStates(), State->GetStringDictionary().Num(),
		bMissionStartSave ? TEXT(", mission-start save") : TEXT(""),
		*State->GetData().m_RootLevelName.ToString() );
	for( INT SubIdx = 0; SubIdx < State->GetData().m_SubLevels.Num(); SubIdx++ )
	{
		if( State->GetData().m_SubLevels(SubIdx).m_Flags != 0 )
		{
			warnf( TEXT("DisLoadGame:   the player was in %s (flags %d)"),
				*State->GetData().m_SubLevels(SubIdx).m_PackageName.ToString(),
				(INT)State->GetData().m_SubLevels(SubIdx).m_Flags );
		}
	}
	// DISHONORED(bringup): what is left is SLC_PostLoad's half - LoadMap to m_RootLevelName, stream the flagged
	// sub-levels and hand each ULevel to FGameState::LoadLevel, which needs the object layer
	// (UObject::GameLoad / PostGameLoad: 108 overrides in retail, none declared here). See agentCF.md section 5.
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

/*-----------------------------------------------------------------------------
	agent CF (PHASE9 CF): the save-game list, the slot names and the async workers.
	Retail has all of this in dishonoredengine.cpp too; the declarations are in
	DishonoredGame/Inc/dishonoredutilities_saveload.h, where retail declares them.
-----------------------------------------------------------------------------*/


// DISHONORED(layout): 2012 PDB GDisSaveGameSlotNames @0xe34180, const wchar_t*[12] - slot N is entry N-1, read
// out of the 2012 exe's .data (build/agentCF dump). Slots 13..52 are Dishonored0..39, formatted on demand.
const TCHAR* GDisSaveGameSlotNames[12] =
{
	TEXT("DisMission0"),
	TEXT("DisMission1"),
	TEXT("DisMission2"),
	TEXT("DisMission3"),
	TEXT("DisMission4"),
	TEXT("DisMission5"),
	TEXT("DisMission6"),
	TEXT("DisMission7"),
	TEXT("DisMission8"),
	TEXT("DisAutoSave0"),
	TEXT("DisAutoSave1"),
	TEXT("DisQuickSave"),
};

// DISHONORED(port): 2013 rva 0x5fb660 - the one name builder the lister, the deleter and the writer share. The
// campaign prefix comes from the build-configuration queries at 0xbbf110 / 0xbbf1b0, which decide which of the
// three campaigns this executable is; a base-game build uses the empty prefix, which is this one.
FString DisGetSaveGameSlotName( INT _Slot )
{
	switch( _Slot )
	{
	case DIS_SAVE_SLOT_OPTIONS:		return FString( TEXT("OPTIONS") );
	case DIS_SAVE_SLOT_DLC02_LOW:	return FString( TEXT("DLC02_LOW_AUTOSAVE") );
	case DIS_SAVE_SLOT_DLC02_HIGH:	return FString( TEXT("DLC02_HIGH_AUTOSAVE") );
	}
	// DISHONORED(bringup): UDisGlobalDLCManager's campaign query is not declared in this tree, so the prefix is
	// the base game's empty one (2013 rva 0x5fb660 would use "DLC02_" / "DLC03_" in a DLC build)
	const FString Prefix;
	if( _Slot >= DIS_SAVE_SLOT_FIRST_USER )
	{
		return Prefix + FString::Printf( TEXT("Dishonored%i"), _Slot - DIS_SAVE_SLOT_FIRST_USER );
	}
	if( _Slot >= DIS_SAVE_SLOT_FIRST_NAMED && _Slot < DIS_SAVE_SLOT_FIRST_NAMED + ARRAY_COUNT(GDisSaveGameSlotNames) )
	{
		return Prefix + FString( GDisSaveGameSlotNames[_Slot - DIS_SAVE_SLOT_FIRST_NAMED] );
	}
	return FString();
}

// DISHONORED(port): 2013 rva 0x5fb7f0 (2012 0x63ed20) - the loop really does run 4..55 and never answers 1, 2 or 3,
// so OPTIONS.sav and the two DLC02 chaos markers are not game states and the lister skips them. (The 2012
// decompile's loop bound is the end of its 12-entry name table, which would make its own
// "Slot >= 13 -> Dishonored%i" branch unreachable; the 2013 body settles it.)
INT GetSaveGameSlot( const TCHAR* _Path )
{
	for( INT Slot = DIS_SAVE_SLOT_FIRST_NAMED; Slot <= DIS_SAVE_SLOT_MAX; Slot++ )
	{
		const FString SlotName = DisGetSaveGameSlotName( Slot );
		if( SlotName.Len() > 0 && appStricmp( _Path, *SlotName ) == 0 )
		{
			return Slot;
		}
	}
	return 0;
}

// DISHONORED(written): appGameDir() + "SaveData\" through the file manager's absolute and user mappings, which is
// what the lister (2013 rva 0x614e20) and the writer build. -savedir=<path> overrides it for agent CF's tests so a
// round-trip never writes into the retail Steam-cloud save folder.
FString DisGetSaveGameDir()
{
	FString Override;
	if( Parse( appCmdLine(), TEXT("savedir="), Override ) && Override.Len() > 0 )
	{
		if( !Override.EndsWith( TEXT("\\") ) && !Override.EndsWith( TEXT("/") ) )
		{
			Override += TEXT("\\");
		}
		return Override;
	}
	const FString Relative = appGameDir() + TEXT("SaveData\\");
	const FString Absolute = GFileManager->ConvertToAbsolutePath( *Relative );
	return GFileManager->ConvertAbsolutePathToUserPath( *Absolute );
}

// DISHONORED(written): 2013 deleter rva 0x601950 - the directory, the slot's base name and ".sav"
FString DisGetSaveGamePath( INT _Slot )
{
	return DisGetSaveGameDir() + DisGetSaveGameSlotName( _Slot ) + TEXT(".sav");
}

// DISHONORED(port): 2013 rva 0xb7aa0 over UGameEngine::DLCManagementBridge (+1688) - one bit per content entry
// whose state byte is 2. Every retail save on this machine carries 0x7f, i.e. seven entries present.
INT DisGetInstalledContentMask()
{
	// DISHONORED(bringup): UArkDLCManagementBridge is not declared in this tree, so the current mask is the
	// "everything this build could want is here" answer. Only the low four bits gate a load (2013 0x614020),
	// and answering with all of them set is what lets a retail save with DLC installed load in a build that has
	// no DLC manager; the alternative - 0 - would reject every save made with the campaign packs installed.
	return 0x0f;
}

/*-----------------------------------------------------------------------------
	FAsyncWorkBase - retail Core/Src/UnAsyncWork.cpp
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x4790 (2012 0x47d0)
FAsyncWorkBase::FAsyncWorkBase( FThreadSafeCounter* InWorkCompletionCounter, const TCHAR* InTaskName )
:	DoneEvent(NULL)
,	WorkCompletionCounter(InWorkCompletionCounter)
{
	if( WorkCompletionCounter == NULL )
	{
		DoneEvent = GSynchronizeFactory->CreateSynchEvent( TRUE, InTaskName );
	}
}

// DISHONORED(port): 2013 rva 0x4830 (2012 0x4870)
FAsyncWorkBase::~FAsyncWorkBase()
{
	if( DoneEvent )
	{
		GSynchronizeFactory->Destroy( DoneEvent );
		DoneEvent = NULL;
	}
}

// DISHONORED(port): 2013 rva 0x4810 (2012 0x4850)
void FAsyncWorkBase::DoThreadedWork()
{
	DoWork();
	Dispose();
}

// DISHONORED(port): 2013 rva 0x48a0 (2012 0x48e0) - with a counter the work deletes itself, without one it
// signals and the caller owns it
void FAsyncWorkBase::Dispose()
{
	if( WorkCompletionCounter )
	{
		WorkCompletionCounter->Decrement();
		delete this;
	}
	else
	{
		DoneEvent->Trigger();
	}
}

// DISHONORED(port): 2013 rva 0x48d0 (2012 0x4910) - a zero wait on a manual-reset event is the "is it set" test
UBOOL FAsyncWorkBase::IsDone()
{
	return DoneEvent ? DoneEvent->Wait( 0 ) : TRUE;
}

/*-----------------------------------------------------------------------------
	FDisAsyncSaveGameLister / FDisAsyncSaveGameDeleter
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x608ef0 (2012 inside RefreshSaveGameList). The 2013 lister also takes the DLC
// campaign's file-name prefix ("DLC02_" / "DLC03_"); the base campaign's is empty, which is what this build is.
FDisAsyncSaveGameLister::FDisAsyncSaveGameLister()
:	FAsyncWorkBase( NULL, TEXT("DisAsyncSaveGameLister") )
,	m_fStartTime(0.0)
,	m_fEndTime(0.0)
,	m_bDLC02LowChaosFound(FALSE)
,	m_bDLC02HighChaosFound(FALSE)
{
}

// DISHONORED(port): 2013 rva 0x5e40d0 (2012 0x62b970)
void FDisAsyncSaveGameLister::Wait()
{
	while( !IsDone() )
	{
		appSleep( 0.1f );
	}
}

/** DISHONORED(port): 2013 rva 0x6086f0 (2012 0x64da40) - the list is newest first */
IMPLEMENT_COMPARE_CONSTREF( FDisSaveGame, DishonoredEngine, { return (A.m_Time < B.m_Time) ? 1 : ((A.m_Time > B.m_Time) ? -1 : 0); } )

// DISHONORED(port): 2013 rva 0x614e20 (2012 0x64f3b0). Every *.sav of the save directory whose base name is a
// known slot and whose header passes FGameState::LoadMapName becomes a row, timestamped with the file's mtime and
// sorted newest first. Retail walks the directory with FindFirstFileW; this uses the file manager's FindFiles,
// which returns the same base names.
void FDisAsyncSaveGameLister::DoWork()
{
	m_fStartTime = appSeconds();

	const FString SavePath = DisGetSaveGameDir();
	TArray<FString> FileNames;
	GFileManager->FindFiles( FileNames, *(SavePath + TEXT("*.sav")), TRUE, FALSE );

	for( INT FileIdx = 0; FileIdx < FileNames.Num(); FileIdx++ )
	{
		const FFilename Filename = SavePath + FileNames(FileIdx);
		struct _stat64i32 FileInfo;
		if( _wstat64i32( *Filename, &FileInfo ) != 0 )
		{
			continue;
		}
		const FString BaseFilename = Filename.GetBaseFilename( TRUE );
		const INT SaveGameSlot = GetSaveGameSlot( *BaseFilename );
		if( SaveGameSlot < 2 )
		{
			// not a game state. The two DLC02 chaos markers are files whose existence is the whole message,
			// which is what the two flags record; OPTIONS.sav and anything else is simply skipped.
			if( appStricmp( *BaseFilename, TEXT("DLC02_LOW_AUTOSAVE") ) == 0 )
			{
				m_bDLC02LowChaosFound = TRUE;
			}
			else if( appStricmp( *BaseFilename, TEXT("DLC02_HIGH_AUTOSAVE") ) == 0 )
			{
				m_bDLC02HighChaosFound = TRUE;
			}
			continue;
		}
		FArchive* Reader = GFileManager->CreateFileReader( *Filename, 0, GNull );
		if( Reader == NULL )
		{
			continue;
		}
		// 2013 reads the header inline instead of through FGameState::LoadMapName and does not validate it:
		// the changelist, then the version when the file is new enough to carry one, then the details string
		INT SaveLoadVersion = 0;
		Reader->ByteOrderSerialize( &SaveLoadVersion, sizeof(SaveLoadVersion) );
		INT SaveVersion = 0;
		if( SaveLoadVersion > DIS_SAVE_CHANGELIST_MIN )
		{
			Reader->ByteOrderSerialize( &SaveVersion, sizeof(SaveVersion) );
		}
		FString SaveDetails;
		*Reader << SaveDetails;
		delete Reader;

		FDisSaveGame& SaveGame = m_SaveGames(m_SaveGames.AddZeroed());
		SaveGame.m_Slot = SaveGameSlot;
		// DISHONORED(bringup): 2013 asks Steam for the timestamp (ISteamRemoteStorage vtable +36,
		// GetFileTimestamp, on the bare file name); with -nosteam there is no cloud, so the file's own mtime
		// stands in - the same value Steam reports for a synced file
		SaveGame.m_Time = (QWORD)FileInfo.st_mtime;
		SaveGame.m_bIsOwner = TRUE;
		// the details string is "<chapter> - <map friendly name>": the number becomes m_MissionIndex and the
		// name becomes m_MapName, which is the key FindMapConfigFromFriendlyName (an exact compare against
		// FMapConfig::m_FriendlyName) needs
		FString ChapterText;
		SaveDetails.Split( FString(TEXT(" - ")), &ChapterText, &SaveGame.m_MapName, FALSE );
		SaveGame.m_MapName.Split( FString(TEXT(" - ")), &SaveGame.m_MapName, NULL, FALSE );
		SaveGame.m_MissionIndex = appAtoi( *ChapterText );
	}

	Sort<USE_COMPARE_CONSTREF(FDisSaveGame,DishonoredEngine)>( m_SaveGames.GetTypedData(), m_SaveGames.Num() );
	m_fEndTime = appSeconds();
}

// DISHONORED(port): 2013 rva 0x5fb860 (2012 0x641fc0)
FDisAsyncSaveGameDeleter::FDisAsyncSaveGameDeleter( const TCHAR* _SaveGamePath )
:	FAsyncWorkBase( NULL, TEXT("DisAsyncSaveGameDeleter") )
,	m_SaveGamePath(_SaveGamePath)
,	m_bSucceeded(FALSE)
{
}

// DISHONORED(port): 2013 rva 0x601950 (2012 0x648180)
void FDisAsyncSaveGameDeleter::DoWork()
{
	const FString SavePath = DisGetSaveGameDir() + m_SaveGamePath + TEXT(".sav");
	m_bSucceeded = GFileManager->Delete( *SavePath, FALSE, FALSE ) != 0;
}

/*-----------------------------------------------------------------------------
	UDishonoredEngine's save list
-----------------------------------------------------------------------------*/

/** the FPointer members of the generated class, typed */
static FDisAsyncSaveGameLister* DisSaveGameList( const UDishonoredEngine* Engine )
{
	return (FDisAsyncSaveGameLister*)Engine->m_pSaveGameList;
}

// DISHONORED(port): 2013 rva 0x608ef0's caller 0x609870 (2012 0x64fab0) - the previous list is waited on and
// destroyed, a fresh one is queued on the thread pool
void UDishonoredEngine::RefreshSaveGameList()
{
	if( !m_bSaveLoadEnabled )
	{
		return;
	}
	FDisAsyncSaveGameLister* Existing = DisSaveGameList( this );
	if( Existing )
	{
		Existing->Wait();
		delete Existing;
		m_pSaveGameList = NULL;
	}
	FDisAsyncSaveGameLister* Lister = new FDisAsyncSaveGameLister();
	m_pSaveGameList = Lister;
	if( GThreadPool )
	{
		GThreadPool->AddQueuedWork( Lister );
	}
	else
	{
		// DISHONORED(written): without a thread pool the work runs inline; Wait() then returns at once because
		// Dispose() has already triggered the event
		Lister->DoThreadedWork();
	}
}

// DISHONORED(port): 2013 rva 0x5e44d0 (2012 0x62bb60)
UBOOL UDishonoredEngine::IsSaveGameListReady() const
{
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	return Lister ? Lister->IsDone() : TRUE;
}

// DISHONORED(port): 2012 rva 0x62bb80
void UDishonoredEngine::WaitSaveGameListReady() const
{
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	if( Lister )
	{
		Lister->Wait();
	}
}

// DISHONORED(port): 2013 rva 0x609920 (2012 0x642540) - slot 0 asks "is there any save at all"
UBOOL UDishonoredEngine::HasSaveGame( INT _Slot ) const
{
	if( !m_bSaveLoadEnabled )
	{
		return FALSE;
	}
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	if( Lister == NULL )
	{
		return FALSE;
	}
	Lister->Wait();
	if( _Slot == 0 )
	{
		return Lister->m_SaveGames.Num() > 0;
	}
	for( INT SaveIdx = 0; SaveIdx < Lister->m_SaveGames.Num(); SaveIdx++ )
	{
		if( Lister->m_SaveGames(SaveIdx).m_Slot == _Slot )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x62bb30 - the bit every save and load path gates on
void UDishonoredEngine::SetSaveLoadEnabled( UBOOL bEnabled )
{
	m_bSaveLoadEnabled = bEnabled ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x609a30 - the row count the 2013 menus loop over
INT UDishonoredEngine::GetNumSaveGames() const
{
	if( !m_bSaveLoadEnabled )
	{
		return 0;
	}
	WaitSaveGameListReady();
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	return Lister ? Lister->m_SaveGames.Num() : 0;
}

// DISHONORED(port): 2013 rva 0x6099c0 (2012 0x6425f0) - indexed by list position, not by slot, which is why the
// menu keeps m_LoadGameSlots as a parallel array
FDisSaveGame* UDishonoredEngine::GetSaveGame( INT _ListIdx ) const
{
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	if( Lister == NULL )
	{
		return NULL;
	}
	Lister->Wait();
	if( !Lister->m_SaveGames.IsValidIndex( _ListIdx ) )
	{
		return NULL;
	}
	return &Lister->m_SaveGames(_ListIdx);
}

// DISHONORED(port): 2013 rva 0x5fbc80 (2012 0x642660). 2013 dropped 2012's "only when idle" guard and sets
// SLC_WaitDeleting plus, when no mode is running, SLM_Deleting.
void UDishonoredEngine::DeleteSaveGame( INT _Slot )
{
	m_SaveLoadStatus = SLS_None;
	FDisAsyncSaveGameDeleter* Deleter = new FDisAsyncSaveGameDeleter( *DisGetSaveGameSlotName( _Slot ) );
	m_pSaveGameDeleter = Deleter;
	if( GThreadPool )
	{
		GThreadPool->AddQueuedWork( Deleter );
	}
	else
	{
		Deleter->DoThreadedWork();
	}
	m_SaveLoadCmd = SLC_WaitDeleting;
	if( m_SaveLoadMode == SLM_None )
	{
		m_SaveLoadMode = SLM_Deleting;
	}
}

// DISHONORED(port): 2013 rva 0x60b050 (2012 0x642090) - the two autosave slots alternate, and the choice is
// cached in m_NextAutoSaveSlot until a save consumes it
INT UDishonoredEngine::GetNextAutoSaveSlot()
{
	if( m_NextAutoSaveSlot )
	{
		return m_NextAutoSaveSlot;
	}
	m_NextAutoSaveSlot = DIS_SAVE_SLOT_FIRST_AUTO;
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	if( Lister == NULL )
	{
		return m_NextAutoSaveSlot;
	}
	Lister->Wait();
	for( INT SaveIdx = 0; SaveIdx < Lister->m_SaveGames.Num(); SaveIdx++ )
	{
		const INT Slot = Lister->m_SaveGames(SaveIdx).m_Slot;
		if( Slot == DIS_SAVE_SLOT_FIRST_AUTO )
		{
			m_NextAutoSaveSlot = DIS_SAVE_SLOT_FIRST_AUTO + 1;
			return m_NextAutoSaveSlot;
		}
		if( Slot == DIS_SAVE_SLOT_FIRST_AUTO + 1 )
		{
			m_NextAutoSaveSlot = DIS_SAVE_SLOT_FIRST_AUTO;
			return m_NextAutoSaveSlot;
		}
	}
	return m_NextAutoSaveSlot;
}

// DISHONORED(port): 2013 rva 0x5fb930 (2012 0x6421c0) - the lowest free user slot, -1 when all 40 are taken
INT UDishonoredEngine::GetNextUserSaveSlot() const
{
	BYTE Used[DIS_SAVE_SLOT_NUM_USER];
	appMemzero( Used, sizeof(Used) );
	FDisAsyncSaveGameLister* Lister = DisSaveGameList( this );
	if( Lister )
	{
		Lister->Wait();
		for( INT SaveIdx = 0; SaveIdx < Lister->m_SaveGames.Num(); SaveIdx++ )
		{
			const INT UserIdx = Lister->m_SaveGames(SaveIdx).m_Slot - DIS_SAVE_SLOT_FIRST_USER;
			if( UserIdx >= 0 && UserIdx < DIS_SAVE_SLOT_NUM_USER )
			{
				Used[UserIdx] = 1;
			}
		}
	}
	for( INT UserIdx = 0; UserIdx < DIS_SAVE_SLOT_NUM_USER; UserIdx++ )
	{
		if( !Used[UserIdx] )
		{
			return UserIdx + DIS_SAVE_SLOT_FIRST_USER;
		}
	}
	return -1;
}

// DISHONORED(port): 2013 rva 0x5fbef0 (2012 0x642900) - FindMapConfig over m_FriendlyName, which is the string a
// save file's header carries
FMapConfig* UDishonoredEngine::FindMapConfigFromFriendlyName( const FString& FriendlyName )
{
	for( INT ConfigIndex = 0; ConfigIndex < m_MapConfig.Num(); ConfigIndex++ )
	{
		if( appStricmp( *FriendlyName, *m_MapConfig(ConfigIndex).m_FriendlyName ) == 0 )
		{
			return &m_MapConfig(ConfigIndex);
		}
	}
	return NULL;
}

/*-----------------------------------------------------------------------------
	-savetest: the container against the real files
-----------------------------------------------------------------------------*/

IMPLEMENT_COMPARE_CONSTREF( FString, DisSaveTestName, { return appStricmp( *A, *B ); } )

// DISHONORED(written): agent CF's acceptance harness. It reads every *.sav of the save directory through the
// ported DisSaveLoad container and prints what it found; with -savetestwrite it then writes the first one back
// out through FGameState::Save and re-reads it, comparing every field and every level blob byte for byte.
void DisSaveGameSelfTest()
{
	const FString SavePath = DisGetSaveGameDir();
	warnf( TEXT("DisSaveTest: directory %s"), *SavePath );

	TArray<FString> FileNames;
	GFileManager->FindFiles( FileNames, *(SavePath + TEXT("*.sav")), TRUE, FALSE );
	Sort<USE_COMPARE_CONSTREF(FString,DisSaveTestName)>( FileNames.GetTypedData(), FileNames.Num() );

	INT NumRead = 0;
	INT NumFailed = 0;
	INT NumRejected = 0;
	INT NumLevelStates = 0;
	INT NumRoundTrip = 0;
	INT NumRoundTripFailed = 0;
	const UBOOL bWrite = ParseParam( appCmdLine(), TEXT("savetestwrite") );

	for( INT FileIdx = 0; FileIdx < FileNames.Num(); FileIdx++ )
	{
		const FFilename Filename = SavePath + FileNames(FileIdx);
		const FString BaseFilename = Filename.GetBaseFilename( TRUE );
		const INT Slot = GetSaveGameSlot( *BaseFilename );
		if( Slot < DIS_SAVE_SLOT_FIRST_NAMED )
		{
			// slots 1..3 are the profile blob and the two DLC02 chaos markers, which are not game states;
			// retail never hands them to FGameState::Load either, and its Load reads the map-name FString
			// before it validates the changelist, so feeding it OPTIONS.sav asks for a 2 GB FString
			warnf( TEXT("DisSaveTest:   %-22s skipped (slot %d is not a game state)"), *FileNames(FileIdx), Slot );
			continue;
		}

		FArchive* Reader = GFileManager->CreateFileReader( *Filename, 0, GNull );
		if( Reader == NULL )
		{
			warnf( TEXT("DisSaveTest:   %-22s UNREADABLE"), *FileNames(FileIdx) );
			NumFailed++;
			continue;
		}
		DisSaveLoad::FGameState State;
		const UBOOL bLoaded = State.Load( *Reader );
		const INT Left = Reader->TotalSize() - Reader->Tell();
		delete Reader;

		if( !bLoaded )
		{
			warnf( TEXT("DisSaveTest:   %-22s REJECTED (corrupt %d, newer %d, content %d)"), *FileNames(FileIdx),
				State.IsCorrupt(), State.IsNewerThanBuild(), State.IsMissingContent() );
			NumRejected++;
			continue;
		}
		NumRead++;
		NumLevelStates += State.NumLevelStates();
		warnf( TEXT("DisSaveTest:   %-22s slot %2d v%d dlc 0x%02x names %5d sub %2d levels %d mission %2d root %s left %d :: %s"),
			*FileNames(FileIdx), Slot, State.GetSaveVersion(), State.GetDLCMask(),
			State.GetStringDictionary().Num(), State.GetData().m_SubLevels.Num(), State.NumLevelStates(),
			State.GetData().m_nMissionIndex, *State.GetData().m_RootLevelName.ToString(), Left,
			*State.GetData().m_SaveDetails );
		for( INT LevelIdx = 0; LevelIdx < State.NumLevelStates(); LevelIdx++ )
		{
			const DisSaveLoad::FLevelState& LevelState = State.GetLevelState( LevelIdx );
			warnf( TEXT("DisSaveTest:       level %s (package %s) loc %d objects %d/%d shared %d/%d v%d dict %d data %d"),
				*LevelState.m_LevelName.ToString(), *LevelState.m_PackageName.ToString(), (INT)LevelState.m_Location,
				(INT)LevelState.m_NumSavedObjects, (INT)LevelState.m_NumObjects,
				(INT)LevelState.m_NumSavedSharedObjects, (INT)LevelState.m_NumSharedObjects,
				LevelState.m_SaveVersion, LevelState.m_CompressedObjectDictionary.Num(),
				LevelState.m_CompressedObjectData.Num() );
		}
		// the area the player was in: the one sub-level whose flags are non-zero
		for( INT SubIdx = 0; SubIdx < State.GetData().m_SubLevels.Num(); SubIdx++ )
		{
			if( State.GetData().m_SubLevels(SubIdx).m_Flags != 0 )
			{
				warnf( TEXT("DisSaveTest:       placement: open %s and stream %s (flags %d)"),
					*State.GetData().m_RootLevelName.ToString(),
					*State.GetData().m_SubLevels(SubIdx).m_PackageName.ToString(),
					(INT)State.GetData().m_SubLevels(SubIdx).m_Flags );
			}
		}
		FString SubLevels;
		for( INT SubIdx = 0; SubIdx < State.GetData().m_SubLevels.Num(); SubIdx++ )
		{
			SubLevels += FString::Printf( TEXT("%s(%d) "), *State.GetData().m_SubLevels(SubIdx).m_PackageName.ToString(),
				(INT)State.GetData().m_SubLevels(SubIdx).m_Flags );
		}
		warnf( TEXT("DisSaveTest:       sublevels %s"), *SubLevels );
		if( Left != 0 )
		{
			warnf( TEXT("DisSaveTest:       *** %d bytes of %s were not consumed"), Left, *FileNames(FileIdx) );
			NumFailed++;
		}

		if( bWrite )
		{
			const FString OutName = SavePath + TEXT("agentCF_roundtrip.savtmp");
			FArchive* Writer = GFileManager->CreateFileWriter( *OutName, 0, GNull );
			if( Writer == NULL )
			{
				warnf( TEXT("DisSaveTest:       round trip could not write %s"), *OutName );
				NumRoundTripFailed++;
				continue;
			}
			State.Save( *Writer, State.GetData().m_SaveDetails );
			delete Writer;

			FArchive* ReReader = GFileManager->CreateFileReader( *OutName, 0, GNull );
			DisSaveLoad::FGameState Again;
			UBOOL bOk = ReReader != NULL && Again.Load( *ReReader );
			INT LeftAgain = bOk ? ( ReReader->TotalSize() - ReReader->Tell() ) : -1;
			delete ReReader;

			if( bOk )
			{
				bOk = bOk && ( Again.GetSaveVersion() == State.GetSaveVersion() );
				bOk = bOk && ( Again.GetData().m_SaveDetails == State.GetData().m_SaveDetails );
				bOk = bOk && ( Again.GetData().m_nMissionIndex == State.GetData().m_nMissionIndex );
				bOk = bOk && ( Again.GetData().m_RootLevelName == State.GetData().m_RootLevelName );
				bOk = bOk && ( Again.GetData().m_SubLevels.Num() == State.GetData().m_SubLevels.Num() );
				for( INT SubIdx = 0; bOk && SubIdx < State.GetData().m_SubLevels.Num(); SubIdx++ )
				{
					bOk = bOk && ( Again.GetData().m_SubLevels(SubIdx).m_PackageName == State.GetData().m_SubLevels(SubIdx).m_PackageName );
					bOk = bOk && ( Again.GetData().m_SubLevels(SubIdx).m_Flags == State.GetData().m_SubLevels(SubIdx).m_Flags );
				}
				bOk = bOk && ( Again.NumLevelStates() == State.NumLevelStates() );
				for( INT LevelIdx = 0; bOk && LevelIdx < State.NumLevelStates(); LevelIdx++ )
				{
					const DisSaveLoad::FLevelState& A = State.GetLevelState( LevelIdx );
					const DisSaveLoad::FLevelState& B = Again.GetLevelState( LevelIdx );
					bOk = bOk && ( A.m_LevelName == B.m_LevelName ) && ( A.m_PackageName == B.m_PackageName );
					bOk = bOk && ( A.m_Location == B.m_Location ) && ( A.m_SaveVersion == B.m_SaveVersion );
					bOk = bOk && ( A.m_NumObjects == B.m_NumObjects ) && ( A.m_NumSavedObjects == B.m_NumSavedObjects );
					bOk = bOk && ( A.m_NumSharedObjects == B.m_NumSharedObjects ) && ( A.m_NumSavedSharedObjects == B.m_NumSavedSharedObjects );
					bOk = bOk && ( A.m_CompressedObjectDictionary.Num() == B.m_CompressedObjectDictionary.Num() );
					bOk = bOk && ( A.m_CompressedObjectData.Num() == B.m_CompressedObjectData.Num() );
					if( bOk )
					{
						bOk = appMemcmp( A.m_CompressedObjectDictionary.GetData(), B.m_CompressedObjectDictionary.GetData(), A.m_CompressedObjectDictionary.Num() ) == 0;
						bOk = bOk && appMemcmp( A.m_CompressedObjectData.GetData(), B.m_CompressedObjectData.GetData(), A.m_CompressedObjectData.Num() ) == 0;
					}
				}
				// every dictionary name must come back, and at the same index
				bOk = bOk && ( Again.GetStringDictionary().Num() == State.GetStringDictionary().Num() );
				for( INT NameIdx = 0; bOk && NameIdx < State.GetStringDictionary().Num(); NameIdx++ )
				{
					bOk = bOk && ( Again.GetStringDictionary().GetName( NameIdx ) == State.GetStringDictionary().GetName( NameIdx ) );
				}
			}
			if( bOk && LeftAgain == 0 )
			{
				NumRoundTrip++;
			}
			else
			{
				warnf( TEXT("DisSaveTest:       *** round trip of %s FAILED (left %d)"), *FileNames(FileIdx), LeftAgain );
				NumRoundTripFailed++;
			}
			GFileManager->Delete( *OutName, FALSE, TRUE );
		}
	}

	warnf( TEXT("DisSaveTest census: %d files, %d read, %d rejected, %d failed, %d level states, round trip %d ok / %d failed"),
		FileNames.Num(), NumRead, NumRejected, NumFailed, NumLevelStates, NumRoundTrip, NumRoundTripFailed );

	// the second half: the engine's own save list, i.e. exactly the rows the load menu builds from
	// (2012 rva 0x813930 CreateGFxLoadGameList). Printed here because the menu itself needs a running movie.
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	if( Engine == NULL )
	{
		warnf( TEXT("DisSaveTest list: no UDishonoredEngine") );
		return;
	}
	Engine->RefreshSaveGameList();
	const INT NumRows = Engine->GetNumSaveGames();
	INT NumListed = 0;
	INT NumWithMapConfig = 0;
	for( INT RowIdx = 0; RowIdx < NumRows; RowIdx++ )
	{
		FDisSaveGame* Row = Engine->GetSaveGame( RowIdx );
		if( Row == NULL )
		{
			break;
		}
		FMapConfig* MapConfig = Engine->FindMapConfigFromFriendlyName( Row->m_MapName );
		if( MapConfig )
		{
			NumWithMapConfig++;
		}
		const UBOOL bInLoadList = Row->m_Slot >= DIS_SAVE_SLOT_FIRST_AUTO;
		if( bInLoadList )
		{
			NumListed++;
		}
		const __time64_t RowTime = (__time64_t)Row->m_Time;
		struct tm* Local = _localtime64( &RowTime );
		warnf( TEXT("DisSaveTest list: row %2d slot %2d %s %04d-%02d-%02d %02d:%02d  mapconfig %-24s :: %s"),
			RowIdx, Row->m_Slot, bInLoadList ? TEXT("shown") : TEXT("hidden"),
			Local ? Local->tm_year + 1900 : 0, Local ? Local->tm_mon + 1 : 0, Local ? Local->tm_mday : 0,
			Local ? Local->tm_hour : 0, Local ? Local->tm_min : 0,
			MapConfig ? *MapConfig->m_Name : TEXT("<none>"), *Row->m_MapName );
	}
	warnf( TEXT("DisSaveTest list census: %d rows, %d shown in the load list, %d resolved a map config, HasSaveGame(0) %d, next auto slot %d, next user slot %d"),
		NumRows, NumListed, NumWithMapConfig, Engine->HasSaveGame( 0 ), Engine->GetNextAutoSaveSlot(), Engine->GetNextUserSaveSlot() );
}

/*-----------------------------------------------------------------------------
	SLC_PostLoad: the per-level restore

	DISHONORED(port): agent ED (PHASE11 ED). Retail does this in UDishonoredEngine::ProcessSaveLoadCmd's
	SLC_PostLoad arm (2013 rva 0x6162d0): once the travel to m_RootLevelName has finished and the sub-levels
	the save flagged are visible, every ULevel in the world is handed to FGameState::LoadLevel, which finds
	the matching level state and restores the objects inside it. The state machine itself is not ported and
	UDishonoredEngine has no Tick in this tree, so the restore is driven from Engine's per-world-tick hook
	(GDisEngineTickHook, Engine/Inc/UnWorld.h) and armed by -disrestoreslot=<slot>.

	What it measures is the point of the package: DisSaveLoad::GSaveLoadCensus counts the objects in each
	level state's dictionary against the state's own m_NumObjects, and the bytes taken out of each blob
	against the blob's uncompressed length. Those two numbers are what prove the stream stayed in step,
	because the object data carries no length prefixes.
-----------------------------------------------------------------------------*/

/** DISHONORED(written): -disrestoreslot=<slot>, armed in Init and consumed once by the tick hook */
static INT GDisRestoreSlot = -1;
static INT GDisRestoreDelayFrames = 0;
static UBOOL GDisRestoreDone = FALSE;
/** DISHONORED(written): 0 = waiting for the startup world and the file, 1 = waiting for the save's own world */
static INT GDisRestoreStage = 0;
static INT GDisRestoreStageFrames = 0;
static FName GDisRestoreStreamLevel = NAME_None;
/** DISHONORED(written): how many times the per-world-tick hook has run, so that "the world is ticking" is a
    number in the log and not an inference from which -disrestoredelay= value happened to fire */
static INT GDisRestoreTicks = 0;

/** DISHONORED(written): RestoreLoadedLevels is a member; this keeps the tick hook free of the class */
static void Engine_RestoreLoadedLevels( UDishonoredEngine* _pEngine )
{
	_pEngine->RestoreLoadedLevels();
}

/** DISHONORED(written): restore every level state the world has a level for. This is SLC_PostLoad's inner
    half; the travel half is the map the engine already opened. */
void UDishonoredEngine::RestoreLoadedLevels()
{
	DisSaveLoad::FGameState* State = (DisSaveLoad::FGameState*)m_pGameState;
	if( State == NULL || GWorld == NULL )
	{
		return;
	}

	DisSaveLoad::GSaveLoadCensus.Reset();

	TArray<ULevel*> Levels;
	if( GWorld->PersistentLevel != NULL )
	{
		Levels.AddItem( GWorld->PersistentLevel );
	}
	AWorldInfo* Info = GWorld->GetWorldInfo();
	for( INT StreamIdx = 0; Info != NULL && StreamIdx < Info->StreamingLevels.Num(); StreamIdx++ )
	{
		ULevelStreaming* Streaming = Info->StreamingLevels(StreamIdx);
		if( Streaming != NULL && Streaming->LoadedLevel != NULL )
		{
			Levels.AddUniqueItem( Streaming->LoadedLevel );
		}
	}

	INT Matched = 0;
	INT Unmatched = 0;
	warnf( TEXT("DisRestore: %d level state(s) in the save, %d level(s) in the world"),
		State->NumLevelStates(), Levels.Num() );
	for( INT StateIdx = 0; StateIdx < State->NumLevelStates(); StateIdx++ )
	{
		const DisSaveLoad::FLevelState& LS = State->GetLevelState( StateIdx );
		warnf( TEXT("DisRestore:   state %d: level %s package %s location %d objects %d/%d shared %d/%d dict %d bytes data %d bytes"),
			StateIdx, *LS.m_LevelName.ToString(), *LS.m_PackageName.ToString(), (INT)LS.m_Location,
			(INT)LS.m_NumSavedObjects, (INT)LS.m_NumObjects,
			(INT)LS.m_NumSavedSharedObjects, (INT)LS.m_NumSharedObjects,
			DisSaveLoad::GetUncompressedSize( LS.m_CompressedObjectDictionary ),
			DisSaveLoad::GetUncompressedSize( LS.m_CompressedObjectData ) );
	}
	for( INT LevelIdx = 0; LevelIdx < Levels.Num(); LevelIdx++ )
	{
		const FName LevelName = DisGetLevelName( Levels(LevelIdx) );
		if( State->findLevelIndex( LevelName ) == INDEX_NONE )
		{
			Unmatched++;
			continue;
		}
		warnf( TEXT("DisRestore: restoring level %s"), *LevelName.ToString() );
		State->LoadLevel( Levels(LevelIdx) );
		Matched++;
	}
	warnf( TEXT("DisRestore: %d level(s) restored, %d level(s) in the world with no state in this save, %d state(s) left over"),
		Matched, Unmatched, State->NumLevelStates() );
	DisSaveLoad::GSaveLoadCensus.Log( TEXT("restore") );

	// where the player ended up, which is what the whole object layer is for
	for( APawn* Pawn = GWorld->GetFirstPawn(); Pawn != NULL; Pawn = Pawn->NextPawn )
	{
		if( Pawn->Controller != NULL && Pawn->Controller->IsA( APlayerController::StaticClass() ) )
		{
			warnf( TEXT("DisRestore: player pawn %s at %s rotation %s"),
				*Pawn->GetName(), *Pawn->Location.ToString(), *Pawn->Rotation.ToString() );
		}
	}
}

// DISHONORED(written): -disobjtree. Retail's save dictionary names sub-objects by their bare template name
// (DishonoredGameInfo.pGlobalUIManager, DishonoredPlayerPawn.PowersComp), so whether the restore can resolve
// a record is entirely a question of whether the instance exists under that name. This prints the class
// default object's sub-objects beside the live object's, which is the comparison that answers it.
static void DisDumpOneTree( const TCHAR* _pWhat, UObject* _pObject )
{
	if( _pObject == NULL )
	{
		warnf( TEXT("DisObjTree: %s: no live object"), _pWhat );
		return;
	}
	warnf( TEXT("DisObjTree: %s: %s (class %s)"), _pWhat, *_pObject->GetPathName(), *_pObject->GetClass()->GetName() );
	UObject* pDefaults = _pObject->GetClass()->GetDefaultObject();
	for( INT Pass = 0; Pass < 2; Pass++ )
	{
		UObject* pOuter = ( Pass == 0 ) ? pDefaults : _pObject;
		INT Found = 0;
		for( FObjectIterator It; It; ++It )
		{
			if( It->GetOuter() != pOuter )
			{
				continue;
			}
			warnf( TEXT("DisObjTree:     %s %s : %s"), ( Pass == 0 ) ? TEXT("template") : TEXT("instance"),
				*It->GetName(), *It->GetClass()->GetName() );
			Found++;
		}
		warnf( TEXT("DisObjTree:   %s sub-objects: %d"), ( Pass == 0 ) ? TEXT("template") : TEXT("instance"), Found );
	}
}

void DisDumpObjectTrees()
{
	AWorldInfo* pInfo = ( GWorld != NULL ) ? GWorld->GetWorldInfo() : NULL;
	DisDumpOneTree( TEXT("game info"), pInfo != NULL ? pInfo->Game : NULL );
	APlayerController* pPC = NULL;
	APawn* pPawn = NULL;
	for( AController* pController = ( GWorld != NULL ) ? GWorld->GetFirstController() : NULL;
		 pController != NULL; pController = pController->NextController )
	{
		if( pController->IsA( APlayerController::StaticClass() ) )
		{
			pPC = (APlayerController*)pController;
			pPawn = pController->Pawn;
			break;
		}
	}
	DisDumpOneTree( TEXT("player controller"), pPC );
	DisDumpOneTree( TEXT("player pawn"), pPawn );
}

/** DISHONORED(written): are all of the world's streaming levels loaded and visible */
static UBOOL DisWorldIsSettled( AWorldInfo* _pInfo )
{
	if( _pInfo == NULL || _pInfo->StreamingLevels.Num() == 0 )
	{
		return FALSE;
	}
	for( INT StreamIdx = 0; StreamIdx < _pInfo->StreamingLevels.Num(); StreamIdx++ )
	{
		ULevelStreaming* pStreaming = _pInfo->StreamingLevels(StreamIdx);
		if( pStreaming != NULL && pStreaming->bShouldBeLoaded
			&& ( pStreaming->LoadedLevel == NULL || !pStreaming->bIsVisible ) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/** DISHONORED(written): the streaming-persistent sub-level the save was taken in - m_SubLevels' bit-0 entry */
static FName DisSaveStreamingPersistentLevel( DisSaveLoad::FGameState* _pState )
{
	if( _pState == NULL )
	{
		return NAME_None;
	}
	const TArray<DisSaveLoad::FSubLevelState>& rSubLevels = _pState->GetData().m_SubLevels;
	for( INT SubIdx = 0; SubIdx < rSubLevels.Num(); SubIdx++ )
	{
		if( ( rSubLevels(SubIdx).m_Flags & 1 ) != 0 )
		{
			return rSubLevels(SubIdx).m_PackageName;
		}
	}
	return NAME_None;
}

// DISHONORED(written): the tick hook, as retail's UDishonoredEngine::ProcessSaveLoadCmd (2013 rva 0x6162d0) is
// staged: read the file, bring the world the save was taken in up, then restore. -disrestoredelay=<n> is the
// per-stage patience in frames.
static void DisSaveLoadRestoreTick()
{
	if( GDisRestoreDone || GDisRestoreSlot < 0 || GWorld == NULL )
	{
		return;
	}
	UDishonoredEngine* pEngine = Cast<UDishonoredEngine>( GEngine );
	AWorldInfo* pInfo = GWorld->GetWorldInfo();
	if( pEngine == NULL || pInfo == NULL )
	{
		return;
	}
	GDisRestoreDelayFrames--;
	GDisRestoreTicks++;
	const UBOOL bSettled = DisWorldIsSettled( pInfo );

	// SLC_Load: read the file, then look at the world it asks for
	if( GDisRestoreStage == 0 )
	{
		if( !bSettled && GDisRestoreDelayFrames > 0 )
		{
			return;
		}
		warnf( TEXT("DisRestore: armed slot %d; the startup world is up after %d world tick(s) (%d streaming level(s), settled %d) - reading the save"),
			GDisRestoreSlot, GDisRestoreTicks, pInfo->StreamingLevels.Num(), (INT)bSettled );
		warnf( TEXT("DisRestore: retail has %d save entry classes and %d GameSave/GameLoad override classes; this tree ports %d of them"),
			(INT)ARRAY_COUNT(GDisRetailSaveEntryClasses),
			(INT)ARRAY_COUNT(GDisPortedGameLoadClasses) + (INT)ARRAY_COUNT(GDisUnportedGameLoadClasses),
			(INT)ARRAY_COUNT(GDisPortedGameLoadClasses) );
		pEngine->Dis_Load( GDisRestoreSlot );
		GDisRestoreStreamLevel = DisSaveStreamingPersistentLevel( (DisSaveLoad::FGameState*)pEngine->m_pGameState );
		GDisRestoreStage = 1;
		GDisRestoreDelayFrames = GDisRestoreStageFrames;
		// SLC_Travel: the save's own streaming-persistent sub-level has to be the world's, or the objects its
		// dictionary names - which are cooked into that mission's sub-levels as forced exports - are not in
		// memory at all.
		if( GDisRestoreStreamLevel != NAME_None && pInfo->CommittedPersistentLevelName != GDisRestoreStreamLevel )
		{
			warnf( TEXT("DisRestore: the save was taken in '%s'; this world has committed '%s' - streaming it in"),
				*GDisRestoreStreamLevel.ToString(), *pInfo->CommittedPersistentLevelName.ToString() );
			const FString Command = FString::Printf( TEXT("STREAMMAP %s"), *GDisRestoreStreamLevel.ToString() );
			GEngine->Exec( *Command );
		}
		return;
	}

	// SLC_PostLoad: wait for the travel to land, then restore
	const UBOOL bArrived = ( GDisRestoreStreamLevel == NAME_None )
		|| ( pInfo->CommittedPersistentLevelName == GDisRestoreStreamLevel && !pEngine->IsPreparingMapChange() );
	if( !( bArrived && bSettled ) && GDisRestoreDelayFrames > 0 )
	{
		return;
	}
	GDisRestoreDone = TRUE;
	warnf( TEXT("DisRestore: the world is up after %d world tick(s) (%d streaming level(s), settled %d, committed '%s', wanted '%s') - restoring"),
		GDisRestoreTicks, pInfo->StreamingLevels.Num(), (INT)bSettled, *pInfo->CommittedPersistentLevelName.ToString(),
		*GDisRestoreStreamLevel.ToString() );
	if( ParseParam( appCmdLine(), TEXT("disobjtree") ) )
	{
		DisDumpObjectTrees();
	}
	Engine_RestoreLoadedLevels( pEngine );
}

/** DISHONORED(written): called from UDishonoredEngine::Init */
void DisSaveLoadArmRestore()
{
	INT Slot = -1;
	if( !Parse( appCmdLine(), TEXT("disrestoreslot="), Slot ) )
	{
		return;
	}
	GDisRestoreSlot = Slot;
	GDisRestoreDelayFrames = 900;
	Parse( appCmdLine(), TEXT("disrestoredelay="), GDisRestoreDelayFrames );
	GDisRestoreStageFrames = GDisRestoreDelayFrames;
	GDisEngineTickHook = DisSaveLoadRestoreTick;
	warnf( TEXT("DisRestore: -disrestoreslot=%d armed (delay %d frames)"), GDisRestoreSlot, GDisRestoreDelayFrames );
}
