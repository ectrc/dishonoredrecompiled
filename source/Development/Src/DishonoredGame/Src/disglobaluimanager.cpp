// DishonoredGame/src/disglobaluimanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (26):
//   0x8aed40  public: static void __cdecl UDisGlobalUIManager::InitializePrivateStaticClassUDisGlobalUIManager(void)
//   0x8aed60  public: void __thiscall UDisGlobalUIManager::PreCommitMapChange(void)
//   0x8aedc0  public: void __thiscall UDisGlobalUIManager::PostCommitMapChange(void)
//   0x8aedf0  public: void __thiscall UDisGlobalUIManager::OnActorTerminated(class AActor const &)
//   0x8aee10  public: unsigned int __thiscall UDisGlobalUIManager::CanOpenInGameMenu(class UDisGFxMoviePlayerBase *)const
//   0x8aeee0  public: int __thiscall UDisGlobalUIManager::ShowMessageBox(struct FDisMsgBoxInfo const &)
//   0x8aef00  public: void __thiscall UDisGlobalUIManager::AddMessageBoxTimer(int, float)
//   0x8aef20  public: void __thiscall UDisGlobalUIManager::HideMessageBox(int)
//   0x8aef30  protected: virtual void __thiscall UDisGlobalUIManager::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x8aef60  public: void __thiscall UDisGlobalUIManager::AutoSave(void)const
//   0x8b7100  public: int __thiscall UDisGlobalUIManager::FindTexturePackageIndex(wchar_t const *)
//   0x8b71c0  public: static void __cdecl UDisGlobalUIManager::StaticOnCompleteTexturePackageLoading(class UObject *, void *)
//   0x8b7510  public: void __thiscall UDisGlobalUIManager::FindItemIconPath(wchar_t const *, unsigned int, wchar_t *, int)const
//   0x8b75e0  private: void __thiscall UDisGlobalUIManager::RefreshGlobalUIState(void)
//   0x8bc570  public: void __thiscall UDisGlobalUIManager::OnMovieStackChanged(void)
//   0x8bc580  public: void __thiscall UDisGlobalUIManager::OnMovieAttributesChanged(class UDisGFxMoviePlayerBase *)
//   0x8bc5a0  public: void __thiscall UDisGlobalUIManager::FinishLoadTexturePackage(wchar_t const *)
//   0x8c0020  private: void __thiscall UDisGlobalUIManager::FindCurrentMovieSet(struct FDisUIMovieSet &)const
//   0x8c45d0  public: void __thiscall UDisGlobalUIManager::Init(void)
//   0x8c47d0  public: virtual void __thiscall UDisGlobalUIManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8c4830  public: virtual void __thiscall UDisGlobalUIManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8c9ea0  public: void __thiscall UDisGlobalUIManager::ReleaseTexturePackage(wchar_t const *)
//   0x8cb280  public: void __thiscall UDisGlobalUIManager::Term(void)
//   0x8cb300  public: unsigned int __thiscall UDisGlobalUIManager::LoadTexturePackageAsync(wchar_t const *)
//   0x8cc8d0  public: static class UClass * __cdecl UDisGlobalUIManager::GetPrivateStaticClassUDisGlobalUIManager(wchar_t const *)
//   0x8ceba0  public: static class UClass * __cdecl UDisGlobalUIManager::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gfxui_gfx3.h"

/*-----------------------------------------------------------------------------
	Agent EI (PHASE12 package EI): the three message-box entry points of the manager. All three are forwarders to
	the one global movie player; the queue itself is a file static of disgfxmovieplayerglobal.cpp, which is where
	retail keeps it too.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x83dc00. Retail passes the priority by value and reuses its own stack slot as the
// out-id, which is why the body reads `AddMessageBox(Info, Priority, Priority); return Priority;`.
INT UDisGlobalUIManager::ShowMessageBox( const FDisMsgBoxInfo& _rInfo, UINT _Priority )
{
	INT ID = 0;
	UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
	if( Global != NULL )
	{
		Global->AddMessageBox( _rInfo, ID, _Priority );
	}
	return ID;
}

// DISHONORED(port): 2013 rva 0x83dc50
void UDisGlobalUIManager::HideMessageBox( INT _ID )
{
	UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
	if( Global != NULL )
	{
		Global->RemoveMessageBox( _ID );
	}
}

// DISHONORED(port): 2013 rva 0x83dc30
void UDisGlobalUIManager::AddMessageBoxTimer( INT _ID, FLOAT _fDuration )
{
	UDisGFxMoviePlayerGlobal* Global = DisGetGlobalMoviePlayer();
	if( Global != NULL )
	{
		Global->AddMessageBoxTimer( _ID, _fDuration );
	}
}

// DISHONORED(port): agent EQ, 2013 rva 0x841250. New in 2013 in everything but name: the 2012 body at 0x8aef30
// is 35 bytes and this one is 152, because 2013 added the two DLC tests. m_bEnableTutorials is off in the
// Dunwall City Trials whatever the player set, and m_bEnableBaseTutorials is off in the DLC06 campaign as well -
// retail reads both off GetOuter(), which for the global UI manager is the game info that constructed it.
// The two Cast<> targets are the only two ADishonoredGameInfo subclasses 2013's script package has
// (DisDLC05GameInfo, DisDLC06GameInfo); the DLC06 one is identified by its static class's
// InitializePrivateStaticClass (0x8bf170) sitting sixteen bytes before ADisDLC06GameInfo::ApplyGameSettings
// (0x8bf190), the body that applies m_DifficultyDLC06.
void UDisGlobalUIManager::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	const UBOOL bDLC05 = Cast<ADisDLC05GameInfo>( GetOuter() ) != NULL;
	const UBOOL bDLC06 = Cast<ADisDLC06GameInfo>( GetOuter() ) != NULL;
	m_bEnableAutoSaveInMenus = Parameters->m_bAutoSaveInMenu ? TRUE : FALSE;
	m_bEnableTutorials = ( Parameters->m_bShowTutorialNotifications && !bDLC05 ) ? TRUE : FALSE;
	m_bEnableBaseTutorials = ( m_bEnableTutorials && !bDLC06 ) ? TRUE : FALSE;
}

/*-----------------------------------------------------------------------------
	Agent FA (PHASE12 package FA): the blur behind a modal.

	The chain the coordinator's brief calls "the game's post-process family", read end to end out of retail:
	UDisGFxMoviePlayerGlobal::UpdateMessageBoxAttributes (2013 0x7a5040, already ported) sets
	m_bBlurGameWhileActive on the global movie and then calls OnMovieAttributesChanged, which is the piece
	that was missing. It ORs that one bit across every open Dishonored movie and hands the answer to the
	post-process manager as Epp_UberUI, whose parameters are the m_pBlurTweaks asset's - a depth of field
	with no in-focus radius, which is what puts the whole scene behind the box out of focus.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x84cf80. Retail's gate is the movie's own "is open" bit, because a movie whose
// attributes change while it is closed contributes nothing to the stack.
void UDisGlobalUIManager::OnMovieAttributesChanged( UDisGFxMoviePlayerBase* _pMovie )
{
	if( _pMovie != NULL && _pMovie->bMovieIsOpen )
	{
		RefreshGlobalUIState();
	}
}

/*-----------------------------------------------------------------------------
	Agent FP (PHASE16 FP): the rest of RefreshGlobalUIState, and one owner for every movie player.

	Retail recomputes five things from the open movie stack on every change to it (2013 rva 0x847070, 901 bytes,
	decoded member by member against retail_sdk_layout.json):

	  bAllowMouseCursor  = top->bAllowInput && top->m_bAllowMouseCursor            (FocusMovies[0], +196 bit 5, +380 bit 2)
	  bShowHUD           = !PersistentWorldInfo.m_bNeverDisplayHUD                 (AWorldInfo +752 mask 0x400)
	  bLoadingMenu       = any of pJournal/pNote/pPauseMenu/pStore/pMissionStats/pResultsMenu has not finished its
	                       async package load (vtable +436, HasFinishedAsyncLoading), or pPauseMenu is waiting for a
	                       save/load to start (+504 bit 0)
	  bBlockInput        = bLoadingMenu || top->bCaptureInput                     (+196 bit 12)
	  for every open UDisGFxMoviePlayerBase:  bBlurGame |= bit 0, bDrawBlackStripes |= bit 1, bShowHUD &= bit 3,
	                                          bPauseHUD |= bit 4                   (+380)
	  the UberUI blur                                                              (agent FA's arm, unchanged)
	  m_pGlobal->m_bRequestBlackStripes = bDrawBlackStripes; m_pGlobal->m_bMouseCursorAllowed = bAllowMouseCursor
	  s_pInstance->EnableControllerInput( 0x7FFFF, !bBlockInput, DisInputMaskLevel_BlockedUI )
	  m_pHUD->pMovie->fVisible = bShowHUD; fUpdate = !bPauseHUD
	  m_pHUD->m_pHudFX->pMovie->fVisible = TRUE; fUpdate = !( bPauseHUD || bLoadingMenu )

	The HUD movie is open on every map, so what hides it on the menu is bShowHUD, AND-ed across every open
	movie's m_bShowHUDWhileActive - a class default out of the cook, which is why the walk below logs each
	movie's bits: the expression is only as right as those defaults.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x84cf70 (2012 0x8bc570): `E9 FB A0 FF FF`, a jmp onto 0x847070
void UDisGlobalUIManager::OnMovieStackChanged()
{
	RefreshGlobalUIState();
}

static UBOOL DisMenuMovieIsLoading( const UDisGFxMoviePlayerBase* Player )
{
	return Player != NULL && !Player->HasFinishedAsyncLoading();
}

// DISHONORED(port): 2013 rva 0x847070 (2012 0x8b75e0, private)
void UDisGlobalUIManager::RefreshGlobalUIState()
{
	FGFxEngine* Engine = FGFxEngine::GetEngine();
	if( Engine == NULL )
	{
		return;
	}
	// DISHONORED(port, agent FU): retail reads FGFxEngine+40, which is FocusMovies[0] - the movie ReevaluateFocus
	// (2013 0x586b60) gave the first local player: the TOPMOST open movie that can receive focus, not OpenMovies(0),
	// which is the lowest priority (InsertMovieIntoList, 2013 0x585350, sorts ascending). Measured on 80cc219:
	// OpenMovies(0) was UI_HUDFX, which never takes input, so the bit read 0 on the menu as well as in the mission.
	FGFxMovie* FocusMovie = Engine->GetFocusedMovieFromControllerID( 0 );
	for( INT Index = Engine->OpenMovies.Num() - 1; FocusMovie == NULL && Index >= 0; Index-- )
	{
		if( Engine->OpenMovies(Index)->bCanReceiveFocus )
		{
			FocusMovie = Engine->OpenMovies(Index);
		}
	}
	UDisGFxMoviePlayerBase* Top = FocusMovie != NULL ? Cast<UDisGFxMoviePlayerBase>( FocusMovie->pUMovie ) : NULL;
	const UBOOL bAllowMouseCursor = Top != NULL && Top->bAllowInput && Top->m_bAllowMouseCursor;
	AWorldInfo* WorldInfo = GWorld != NULL ? GWorld->GetWorldInfo( TRUE ) : NULL;
	UBOOL bShowHUD = WorldInfo == NULL || !WorldInfo->m_bNeverDisplayHUD;
	const UBOOL bLoadingMenu = DisMenuMovieIsLoading( m_pJournal ) || DisMenuMovieIsLoading( m_pNote )
		|| DisMenuMovieIsLoading( m_pPauseMenu ) || ( m_pPauseMenu != NULL && m_pPauseMenu->m_bWaitingSaveLoadToStart )
		|| DisMenuMovieIsLoading( m_pStore ) || DisMenuMovieIsLoading( m_pMissionStats )
		|| DisMenuMovieIsLoading( m_pResultsMenu );
	const UBOOL bBlockInput = bLoadingMenu || ( Top != NULL && Top->bCaptureInput );
	UBOOL bBlurGame = bLoadingMenu;
	UBOOL bDrawBlackStripes = FALSE;
	UBOOL bPauseHUD = FALSE;
	FString Census;
	for( INT Index = 0; Index < Engine->OpenMovies.Num(); Index++ )
	{
		UDisGFxMoviePlayerBase* Player = Cast<UDisGFxMoviePlayerBase>( Engine->OpenMovies( Index )->pUMovie );
		if( Player == NULL )
		{
			continue;
		}
		bBlurGame |= Player->m_bBlurGameWhileActive;
		bDrawBlackStripes |= Player->m_bDrawBlackStripesWhileActive;
		bShowHUD &= Player->m_bShowHUDWhileActive;
		bPauseHUD |= Player->m_bPauseHUDWhileActive;
		Census += FString::Printf( TEXT(" %s(showHUD %d blur %d stripes %d pauseHUD %d cursor %d)"),
			*Player->GetClass()->GetName(), Player->m_bShowHUDWhileActive ? 1 : 0, Player->m_bBlurGameWhileActive ? 1 : 0,
			Player->m_bDrawBlackStripesWhileActive ? 1 : 0, Player->m_bPauseHUDWhileActive ? 1 : 0,
			Player->m_bAllowMouseCursor ? 1 : 0 );
	}
	debugf( TEXT("DISHONORED(bringup): UI state: %d open%s -> HUD visible %d update %d, stripes %d, cursor %d, ")
		TEXT("input blocked %d, blur %d, neverDisplayHUD %d"), Engine->OpenMovies.Num(), *Census, bShowHUD ? 1 : 0,
		bPauseHUD ? 0 : 1, bDrawBlackStripes ? 1 : 0, bAllowMouseCursor ? 1 : 0, bBlockInput ? 1 : 0, bBlurGame ? 1 : 0,
		( WorldInfo != NULL && WorldInfo->m_bNeverDisplayHUD ) ? 1 : 0 );

	// agent FA's arm: the UberUI blur. Retail gates it on the game info's post-process manager existing.
	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( PpManager != NULL )
	{
		const UBOOL bRunning = PpManager->IsEffectRequired( Epp_UberUI );
		if( bBlurGame )
		{
			if( !bRunning )
			{
				if( m_pBlurTweaks == NULL )
				{
					debugf( TEXT("DISHONORED(bringup): UI blur asked for and UDisGlobalUIManager::m_pBlurTweaks is NULL - ")
						TEXT("the [DishonoredGame.DisGlobalUIManager] m_pBlurTweaks entry of DefaultUI.ini names the asset") );
				}
				else
				{
					PpManager->SetUIPPParams( m_pBlurTweaks->m_Parameters, m_pBlurTweaks->m_fWeight,
											  m_pBlurTweaks->m_fFadeInTime, m_pBlurTweaks->m_fFadeOutTime );
					PpManager->StartEffect( Epp_UberUI, TRUE );
					debugf( TEXT("DISHONORED(bringup): UI blur census: started, weight %.3f fade %.3f/%.3f, DOF focus %.1f ")
						TEXT("radius %.1f far %.3f"), m_pBlurTweaks->m_fWeight, m_pBlurTweaks->m_fFadeInTime,
						m_pBlurTweaks->m_fFadeOutTime, m_pBlurTweaks->m_Parameters.m_DOFParameters.m_FocusDistance,
						m_pBlurTweaks->m_Parameters.m_DOFParameters.m_InFocusRadius,
						m_pBlurTweaks->m_Parameters.m_DOFParameters.m_FarBlurAmount );
				}
			}
		}
		else if( bRunning )
		{
			PpManager->StopEffect( Epp_UberUI );
			debugf( TEXT("DISHONORED(bringup): UI blur census: stopped") );
		}
	}

	if( m_pGlobal != NULL )
	{
		m_pGlobal->m_bRequestBlackStripes = bDrawBlackStripes;
		m_pGlobal->m_bMouseCursorAllowed = bAllowMouseCursor;
	}
	if( ADishonoredPlayerController::s_pInstance != NULL )
	{
		ADishonoredPlayerController::s_pInstance->EnableControllerInput( 0x7FFFF, !bBlockInput, DisInputMaskLevel_BlockedUI );
	}
	if( m_pHUD != NULL )
	{
		FGFxMovie* HUDMovie = m_pHUD->GetMovie();
		if( HUDMovie != NULL )
		{
			HUDMovie->fVisible = bShowHUD;
			HUDMovie->fUpdate = !bPauseHUD;
		}
		FGFxMovie* FXMovie = m_pHUD->m_pHudFX != NULL ? m_pHUD->m_pHudFX->GetMovie() : NULL;
		if( FXMovie != NULL )
		{
			FXMovie->fVisible = TRUE;
			FXMovie->fUpdate = !( bPauseHUD || bLoadingMenu );
		}
	}
}

/*-----------------------------------------------------------------------------
	Init: the one owner. Retail reaches it from ADishonoredGameInfo::PostBeginPlay (2013 rva 0x6155d0) ->
	InitGlobalManagers (0x5e9bd0, call site 0x5e9c38) on every map, the menu map included, with no condition
	anywhere on the chain (agent FM's resolution is in disgfxmovieplayerhud.cpp's banner). m_pGlobal is not
	constructed here: it is the manager's own default sub-object pGlobal (retail's live object is
	DishonoredGameInfo.pGlobalUIManager.pGlobal, agent FB's ue.log), and m_pMainMenu is not constructed here
	either - the main menu is opened by the menu map's Kismet, Main_Sequence.DisGFxActionOpenMovie_0, and
	registers itself in UDisGFxMoviePlayerMainMenu::PostStart.
-----------------------------------------------------------------------------*/

static void DisLogMoviePlayerDefaults( UClass* Class )
{
	UDisGFxMoviePlayerBase* Defaults = Class != NULL ? (UDisGFxMoviePlayerBase*)Class->GetDefaultObject() : NULL;
	if( Defaults == NULL )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): cooked defaults %s: showHUD %d blur %d stripes %d pauseHUD %d cursor %d | ")
		TEXT("allowFocus %d allowInput %d captureInput %d hudOff %d closeOnLevelChange %d priority %d tweaks %s"),
		*Class->GetName(), Defaults->m_bShowHUDWhileActive ? 1 : 0, Defaults->m_bBlurGameWhileActive ? 1 : 0,
		Defaults->m_bDrawBlackStripesWhileActive ? 1 : 0, Defaults->m_bPauseHUDWhileActive ? 1 : 0,
		Defaults->m_bAllowMouseCursor ? 1 : 0, Defaults->bAllowFocus ? 1 : 0, Defaults->bAllowInput ? 1 : 0,
		Defaults->bCaptureInput ? 1 : 0, Defaults->bDisplayWithHudOff ? 1 : 0, Defaults->bCloseOnLevelChange ? 1 : 0,
		(INT)Defaults->Priority, Defaults->m_pTweaks ? *Defaults->m_pTweaks->GetPathName() : TEXT("none") );
}

// DISHONORED(port): 2013 rva 0x85fdb0 (2012 0x8c45d0)
void UDisGlobalUIManager::Init()
{
	FDisUIMovieSet MovieSet( EC_EventParm );
	FindCurrentMovieSet( MovieSet );
	if( MovieSet.m_SFTweaksPackageName.Len() > 0 )
	{
		LoadSFTweaksPackage( MovieSet.m_SFTweaksPackageName );
	}
	InitItemIconPackages( MovieSet.m_bHasDLCItemIconPackages, MovieSet.m_MapFilter );

	// DISHONORED(port, medium confidence): retail tests the outer against two game-info classes whose static-class
	// globals (2013 0x1072040 and 0x10721f8) carry no name in the database; both tests only choose which tweaks slot
	// an SF-tweaks-package hit lands in, and the package is named by the DLC06 and DLC07 sets alone.
	const UBOOL bDLC06Outer = Cast<ADisDLC06GameInfo>( GetOuter() ) != NULL;
	const UBOOL bDLC07Outer = Cast<ADisDLC07GameInfo>( GetOuter() ) != NULL;

	m_pHUD = (UDisGFxMoviePlayerHUD*)CreateMoviePlayer( UDisGFxMoviePlayerHUD::StaticClass(), TEXT("pHUD"),
		MovieSet.m_HUDClassName, MovieSet.m_HUDMoviePath, TRUE, bDLC06Outer );
	m_pPowerWheel = (UDisGFxMoviePlayerPowerWheel*)CreateMoviePlayer( UDisGFxMoviePlayerPowerWheel::StaticClass(),
		TEXT("pPowerWheel"), MovieSet.m_PowerWheelClassName, MovieSet.m_PowerWheelMoviePath, TRUE, FALSE );
	m_pJournal = (UDisGFxMoviePlayerJournal*)CreateMoviePlayer( UDisGFxMoviePlayerJournal::StaticClass(), TEXT("pJournal"),
		MovieSet.m_JournalClassName, MovieSet.m_JournalMoviePath, FALSE, bDLC07Outer );
	m_pNote = (UDisGFxMoviePlayerNote*)CreateMoviePlayer( UDisGFxMoviePlayerNote::StaticClass(), TEXT("pNote"),
		MovieSet.m_NoteClassName, MovieSet.m_NoteMoviePath, TRUE, FALSE );
	m_pPauseMenu = (UDisGFxMoviePlayerPauseMenu*)CreateMoviePlayer( UDisGFxMoviePlayerPauseMenu::StaticClass(),
		TEXT("pPauseMenu"), MovieSet.m_PauseMenuClassName, MovieSet.m_PauseMenuMoviePath, FALSE, bDLC06Outer );
	if( Cast<ADisDLC05GameInfo>( GetOuter() ) != NULL )
	{
		m_pBrief = (UDisDLC05MoviePlayerBrief*)CreateMoviePlayer( UDisDLC05MoviePlayerBrief::StaticClass(), TEXT("pBrief"),
			FString(), FString(), FALSE, FALSE );
		m_pResultsMenu = (UDisDLC05MoviePlayerResultsMenu*)CreateMoviePlayer( UDisDLC05MoviePlayerResultsMenu::StaticClass(),
			TEXT("pResultsMenu"), FString(), FString(), FALSE, FALSE );
	}
	else
	{
		m_pMissionStats = (UDisGFxMoviePlayerMissionStats*)CreateMoviePlayer( UDisGFxMoviePlayerMissionStats::StaticClass(),
			TEXT("pMissionStats"), FString(), MovieSet.m_MissionStatsMoviePath, FALSE, FALSE );
		m_bJournalAllowed = TRUE;
	}

	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerGlobal::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerHUD::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerMainMenu::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerGamma::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerPauseMenu::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerJournal::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerNote::StaticClass() );
	DisLogMoviePlayerDefaults( UDisGFxMoviePlayerPowerWheel::StaticClass() );

	// DISHONORED(bringup): -nodishud is the regression harness's: the three calibrated stages (nullrhi, d3d9,
	// inputtest) have cliff-detector thresholds measured with no movie open, and it keeps them that way by
	// leaving the manager's two movies closed. Retail has no such switch and no condition here (agent FM).
	if( ParseParam( appCmdLine(), TEXT("nodishud") ) )
	{
		debugf( TEXT("DISHONORED(bringup): UDisGlobalUIManager::Init: -nodishud, the global movie and the HUD stay closed") );
		return;
	}
	if( m_pGlobal != NULL )
	{
		m_pGlobal->Start( FALSE );
		debugf( TEXT("DISHONORED(bringup): UDisGlobalUIManager::Init: global movie %s %s"),
			m_pGlobal->bMovieIsOpen ? TEXT("opened") : TEXT("FAILED to open"),
			m_pGlobal->MovieInfo ? *m_pGlobal->MovieInfo->GetPathName() : TEXT("(no MovieInfo)") );
	}
	else
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalUIManager::Init: no pGlobal default sub-object on %s"),
			*GetPathName() );
	}
	if( m_pHUD != NULL )
	{
		m_pHUD->StartHUD();
		debugf( TEXT("DISHONORED(bringup): HUD: %s %s through %s (fx %s), tweaks %s"),
			m_pHUD->bMovieIsOpen ? TEXT("opened") : TEXT("FAILED to open"),
			m_pHUD->MovieInfo ? *m_pHUD->MovieInfo->GetPathName() : TEXT("(no MovieInfo)"), *m_pHUD->GetClass()->GetName(),
			( m_pHUD->m_pHudFX && m_pHUD->m_pHudFX->MovieInfo ) ? *m_pHUD->m_pHudFX->MovieInfo->GetPathName() : TEXT("none"),
			m_pHUD->m_pTweaks ? *m_pHUD->m_pTweaks->GetPathName() : TEXT("none") );
	}
}

// DISHONORED(port, partial): 2013 rva 0x841120 (2012 0x8aed60). Retail, in order: the open global movie's
// PreCommitMapChange (0x79eb30: ConditionalHideMouseCursor and a Shrink of the message-box queue), the open main
// menu's (0x7bc250: Close(TRUE)), CloseHUD on the open HUD, the power wheel's close when its +536 bit is set
// (0x7dc5f0 -> 0x7d4fc0), then Shrink on m_DLCUI, m_TexturePackages and m_UpgradesAlreadyDisplayed. The global
// power wheel's close is not ported (no body in this tree); the three Shrinks are bookkeeping. 2012 also cleared
// m_bJournalAllowed here and set it back in PostCommitMapChange; 2013 does neither.
void UDisGlobalUIManager::PreCommitMapChange()
{
	if( m_pGlobal != NULL && m_pGlobal->bMovieIsOpen )
	{
		m_pGlobal->PreCommitMapChange();
	}
	if( m_pMainMenu != NULL && m_pMainMenu->bMovieIsOpen )
	{
		m_pMainMenu->Close( TRUE );
	}
	if( m_pHUD != NULL && m_pHUD->bMovieIsOpen )
	{
		m_pHUD->CloseHUD();
	}
	m_DLCUI.Shrink();
	m_TexturePackages.Shrink();
	m_UpgradesAlreadyDisplayed.Shrink();
}

// DISHONORED(port): 2013 rva 0x83daa0 (2012 0x8aedc0, which only restarted a closed HUD). 2013 starts both the global
// movie and the HUD unconditionally; UDisGFxMoviePlayerBase::Start's one early-out is "already open". The new map's
// Kismet opens its own menu, if it has one, through UGFxAction_OpenMovie.
void UDisGlobalUIManager::PostCommitMapChange()
{
	if( ParseParam( appCmdLine(), TEXT("nodishud") ) )
	{
		return;
	}
	if( m_pGlobal != NULL )
	{
		m_pGlobal->Start( FALSE );
	}
	if( m_pHUD != NULL )
	{
		m_pHUD->StartHUD();
		debugf( TEXT("DISHONORED(bringup): HUD: %s %s after the map change"),
			m_pHUD->bMovieIsOpen ? TEXT("reopened") : TEXT("FAILED to reopen"),
			m_pHUD->MovieInfo ? *m_pHUD->MovieInfo->GetPathName() : TEXT("(no MovieInfo)") );
	}
}

// DISHONORED(port): 2013 rva 0x851970 (2012 0x8c0020). The 2012 body keys the DLC sets on an exact m_MapName and copies
// four paths; 2013's FDisUIMovieSet is 160 bytes keyed on m_MapFilter, matched as a case-insensitive substring of the
// persistent map's name, and every non-empty field of the matching set overrides the default's, which retail does by
// walking the struct's UProperties. The first match wins.
void UDisGlobalUIManager::FindCurrentMovieSet( FDisUIMovieSet& _rMovieSet ) const
{
	const FString PersistentMapName = ( GWorld != NULL && GWorld->PersistentLevel != NULL )
		? GWorld->PersistentLevel->GetOutermost()->GetName() : FString();
	_rMovieSet = m_DefaultUI;
	for( INT Index = 0; Index < m_DLCUI.Num(); Index++ )
	{
		const FDisUIMovieSet& DLCSet = m_DLCUI( Index );
		if( DLCSet.m_MapFilter.Len() == 0 || appStristr( *PersistentMapName, *DLCSet.m_MapFilter ) == NULL )
		{
			continue;
		}
		struct { const FStringNoInit* From; FStringNoInit* To; } Fields[] =
		{
			{ &DLCSet.m_MapFilter, &_rMovieSet.m_MapFilter },
			{ &DLCSet.m_SFTweaksPackageName, &_rMovieSet.m_SFTweaksPackageName },
			{ &DLCSet.m_HUDClassName, &_rMovieSet.m_HUDClassName },
			{ &DLCSet.m_HUDMoviePath, &_rMovieSet.m_HUDMoviePath },
			{ &DLCSet.m_PowerWheelClassName, &_rMovieSet.m_PowerWheelClassName },
			{ &DLCSet.m_PowerWheelMoviePath, &_rMovieSet.m_PowerWheelMoviePath },
			{ &DLCSet.m_JournalClassName, &_rMovieSet.m_JournalClassName },
			{ &DLCSet.m_JournalMoviePath, &_rMovieSet.m_JournalMoviePath },
			{ &DLCSet.m_NoteClassName, &_rMovieSet.m_NoteClassName },
			{ &DLCSet.m_NoteMoviePath, &_rMovieSet.m_NoteMoviePath },
			{ &DLCSet.m_PauseMenuClassName, &_rMovieSet.m_PauseMenuClassName },
			{ &DLCSet.m_PauseMenuMoviePath, &_rMovieSet.m_PauseMenuMoviePath },
			{ &DLCSet.m_MissionStatsMoviePath, &_rMovieSet.m_MissionStatsMoviePath },
		};
		for( INT Field = 0; Field < ARRAY_COUNT( Fields ); Field++ )
		{
			if( Fields[Field].From->Len() > 0 )
			{
				*Fields[Field].To = *Fields[Field].From;
			}
		}
		if( DLCSet.m_bHasDLCItemIconPackages )
		{
			_rMovieSet.m_bHasDLCItemIconPackages = TRUE;
		}
		break;
	}
	debugf( TEXT("DISHONORED(bringup): UI movie set for %s: HUD %s (%s), power wheel %s, journal %s, note %s, pause menu %s, ")
		TEXT("mission stats %s, SF tweaks %s"), *PersistentMapName, *_rMovieSet.m_HUDMoviePath,
		_rMovieSet.m_HUDClassName.Len() ? *_rMovieSet.m_HUDClassName : TEXT("default class"), *_rMovieSet.m_PowerWheelMoviePath,
		*_rMovieSet.m_JournalMoviePath, *_rMovieSet.m_NoteMoviePath, *_rMovieSet.m_PauseMenuMoviePath,
		*_rMovieSet.m_MissionStatsMoviePath, _rMovieSet.m_SFTweaksPackageName.Len() ? *_rMovieSet.m_SFTweaksPackageName : TEXT("none") );
}

// DISHONORED(port): 2013 rva 0x846e70. The config class name is loaded as DishonoredGame.<name> under
// UDisGFxMoviePlayerBase; the object is outered to this manager under the retail sub-object name, which is the path
// agent FB read out of the running game (DishonoredGameInfo.pGlobalUIManager.pHUD). A Twk_<class> object in the SF
// tweaks package goes into m_pTweaks when that is empty or when the caller says so, else into m_pDLCTweaks. The movie
// is found by path and loaded when it is not resident, or only remembered in m_MovieToLoad for the players that load
// theirs when they open.
UDisGFxMoviePlayerBase* UDisGlobalUIManager::CreateMoviePlayer( UClass* _pDefaultClass, const TCHAR* _pName,
	const FString& _rClassName, const FString& _rMoviePath, UBOOL _bLoadMovieNow, UBOOL _bOverrideTweaks )
{
	UClass* Class = _pDefaultClass;
	if( _rClassName.Len() > 0 )
	{
		Class = StaticLoadClass( UDisGFxMoviePlayerBase::StaticClass(), NULL,
			*FString::Printf( TEXT("DishonoredGame.%s"), *_rClassName ), NULL, 0, NULL );
	}
	if( Class == NULL )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalUIManager::CreateMoviePlayer: no class for %s (%s)"),
			_pName, *_rClassName );
		return NULL;
	}
	UDisGFxMoviePlayerBase* Player = (UDisGFxMoviePlayerBase*)StaticConstructObject( Class, this, FName( _pName ) );
	if( m_pSFTweaksPackage != NULL )
	{
		UDisTweaks_GFxMoviePlayerBase* Tweaks = FindObject<UDisTweaks_GFxMoviePlayerBase>( (UPackage*)m_pSFTweaksPackage,
			*FString::Printf( TEXT("Twk_%s"), *Class->GetName() ) );
		if( Tweaks != NULL )
		{
			if( Player->m_pTweaks == NULL || _bOverrideTweaks )
			{
				Player->m_pTweaks = Tweaks;
			}
			else if( Player->m_pDLCTweaks == NULL )
			{
				Player->m_pDLCTweaks = Tweaks;
			}
		}
	}
	if( _rMoviePath.Len() > 0 )
	{
		if( _bLoadMovieNow )
		{
			Player->MovieInfo = FindObject<USwfMovie>( NULL, *_rMoviePath );
			if( Player->MovieInfo == NULL )
			{
				Player->LoadMoviePackage( *_rMoviePath );
			}
		}
		else
		{
			Player->m_MovieToLoad = _rMoviePath;
		}
	}
	return Player;
}

// DISHONORED(port): 2013 rva 0x846d00. Retail brackets the load with the streaming manager's
// BlockTillAllRequestsFinished/CancelPendingTextureStreaming pair; this tree's streaming manager has no such
// bracket to call, and the package is a handful of tweak objects.
void UDisGlobalUIManager::LoadSFTweaksPackage( const FString& _rPackageName )
{
	FString FileName = _rPackageName;
	if( GUseSeekFreeLoading )
	{
		FileName += TEXT("_SF");
	}
	FString Unused;
	if( !GPackageFileCache->FindPackageFile( *FileName, NULL, Unused ) )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalUIManager: no SF tweaks package file %s"), *FileName );
		return;
	}
	if( UObject::LoadPackage( NULL, *FileName, LOAD_None ) != NULL )
	{
		m_pSFTweaksPackage = FindObject<UPackage>( NULL, *_rPackageName );
	}
}

// DISHONORED(port): 2013 rva 0x85dbb0. Retail's find of the small-icon package is Checked; here it warns, because the
// package's presence depends on the cook's start-up set. The DLC half - a suffixed copy of the package loaded through
// LoadTexturePackageAsync (2013 rva 0x8bb300), FlushAsyncLoading, and the suffix dropped on failure - waits on the
// texture-package cache, which is not ported; the suffix is left empty, which is the campaign's answer anyway.
void UDisGlobalUIManager::InitItemIconPackages( UBOOL _bHasDLCItemIconPackages, const FString& _rMapFilter )
{
	m_pItemSmallIconPackage = FindObject<UPackage>( NULL, *m_ItemSmallIconPackageName );
	if( m_pItemSmallIconPackage == NULL )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalUIManager: item icon package %s is not resident"),
			*m_ItemSmallIconPackageName );
	}
	if( _bHasDLCItemIconPackages )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisGlobalUIManager: DLC item icon packages (%s_%s) need ")
			TEXT("LoadTexturePackageAsync, which is not ported"), *m_ItemSmallIconPackageName, *_rMapFilter );
	}
	m_DLCItemIconPackagesSuffix.Empty();
}
