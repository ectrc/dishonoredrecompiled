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

// DISHONORED(port, partial): 2013 rva 0x847070 (2012 0x8b75e0, private). Retail's body recomputes five things from the same walk - the
// blur, the black stripes, the HUD's visibility and pause, the controller input mask and the mouse cursor -
// and each has its own consumer. Only the blur is ported here; the other four are one bit each of the same
// word (m_bDrawBlackStripesWhileActive, m_bShowHUDWhileActive, m_bPauseHUDWhileActive, m_bAllowMouseCursor)
// and none of their consumers exists in this tree yet. Retail also seeds the accumulator from the manager's
// own movie set before the walk; that term is not reproduced, and every movie it would have covered is in
// the walk below anyway.
void UDisGlobalUIManager::RefreshGlobalUIState()
{
	FGFxEngine* Engine = FGFxEngine::GetEngine();
	if( Engine == NULL )
	{
		return;
	}
	UBOOL bBlurGame = FALSE;
	for( INT Index = 0; Index < Engine->OpenMovies.Num(); Index++ )
	{
		UDisGFxMoviePlayerBase* Player = Cast<UDisGFxMoviePlayerBase>( Engine->OpenMovies( Index )->pUMovie );
		if( Player != NULL && Player->m_bBlurGameWhileActive )
		{
			bBlurGame = TRUE;
		}
	}
	UDisPostProcessManager* PpManager = DisGetPpManager();
	if( PpManager == NULL )
	{
		return;
	}
	const UBOOL bRunning = PpManager->IsEffectRequired( Epp_UberUI );
	if( bBlurGame )
	{
		if( !bRunning )
		{
			if( m_pBlurTweaks == NULL )
			{
				debugf( TEXT("DISHONORED(bringup): UI blur asked for and UDisGlobalUIManager::m_pBlurTweaks is NULL - ")
					TEXT("the [DishonoredGame.DisGlobalUIManager] m_pBlurTweaks entry of DefaultUI.ini names the asset") );
				return;
			}
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
	else if( bRunning )
	{
		PpManager->StopEffect( Epp_UberUI );
		debugf( TEXT("DISHONORED(bringup): UI blur census: stopped") );
	}
}
