// DishonoredGame/src/stateplayermasterchoice_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (14):
//   0x6af8d0  public: static void __cdecl UStatePlayerMasterChoice_Base::InitializePrivateStaticClassUStatePlayerMasterChoice_Base(void)
//   0x6af8f0  public: void __thiscall UStatePlayerMasterChoice_Base::PlayerRequestSkip(void)
//   0x6af910  public: unsigned int __thiscall UStatePlayerMasterChoice_Base::IsSkippable(void)const
//   0x6af930  protected: unsigned int __thiscall UStatePlayerMasterChoice_Base::ReadyToMakeChoice(void)const
//   0x6b4010  public: virtual void __thiscall FStatePlayerMasterChoice_Base_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6b84d0  public: static class UClass * __cdecl UStatePlayerMasterChoice_Base::GetPrivateStaticClassUStatePlayerMasterChoice_Base(wchar_t const *)
//   0x6b8560  public: virtual void __thiscall UStatePlayerMasterChoice_Base::OnEnterState(class UDishonoredNativeState *)
//   0x6b8600  public: virtual void __thiscall UStatePlayerMasterChoice_Base::OnExitState(class UDishonoredNativeState *)
//   0x6b8680  protected: void __thiscall UStatePlayerMasterChoice_Base::PauseBendTime(void)
//   0x6b8750  protected: void __thiscall UStatePlayerMasterChoice_Base::UnpauseBendTime(void)
//   0x6b8820  public: unsigned int __thiscall UStatePlayerMasterChoice_Base::MakePlayerChoice(int)
//   0x6bebd0  public: static class UClass * __cdecl UStatePlayerMasterChoice_Base::StaticClassNoInline(void)
//   0x6c7450  protected: void __thiscall UStatePlayerMasterChoice_Base::ShowChoiceSelection(void)
//   0x6c9470  public: virtual void __thiscall UStatePlayerMasterChoice_Base::TickState(float)

/*-----------------------------------------------------------------------------
	agent JE (PHASE24 JE): the enter / exit / bend-time half of UStatePlayerMasterChoice_Base, the base of the player's dialog
	(UStatePlayerMasterInDialog) and scripted-choice states. Entering takes every master input but look (bit 2) away, locks the
	save, and moves the PlayerState HUD level to the dialog set: the gauges and the crosshair off (0x7FAF), the letterbox (0x10)
	and the subtitle / choice elements (0x6000) on - the letterbox only when the state uses it. That HUD bit 0x10 is what the GFx
	HUD's Tick_Cinematic turns into SetBlackStripes. Leaving reverses it. 2013 bodies (the 2013 OnEnterState inlines the master base's
	enter and drops the 2012 0x810 for 0x6010; OnExitState hides the choice through its own helper, 2013 0xa69340, which also raises
	0x6000 again).
	Not here (agent JF): ShowChoiceSelection, MakePlayerChoice, ReadyToMakeChoice and the choice getters; TickState names the
	ShowChoiceSelection call it leaves to that package.
	DISHONORED(bringup), named and not called: ADishonoredGameInfo::BendTime (2012 0x63fcf0, unported) in the two bend-time
	functions, which therefore only move the out-of-bend-time flags; UDisGFxMoviePlayerHUD::HideChoiceSelection (2013 0x78d260, JF).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "jediag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkJE_UStatePlayerMasterChoice_Base()
{
}

// 2013 0x673640 (208 B)
void UStatePlayerMasterChoice_Base::OnEnterState( UDishonoredNativeState* LastState )
{
	m_DialogState = 0;
	UStatePlayerMasterBase::OnEnterState( LastState );
	DisPushDisableSave( DDST_StatePlayerMasterChoice_Base );
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, FALSE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->EnableControllerInput( 2, TRUE, DisInputMaskLevel_MasterAction );
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	ADishonoredHUD* HUD = PC ? Cast<ADishonoredHUD>( PC->myHUD ) : NULL;
	if( HUD )
	{
		JEDIAG( "bars writer %s::OnEnterState: PlayerState level disable 0x7FAF enable 0x6010%s", *GetClass()->GetName(), m_bUseLetterboxing ? TEXT("") : TEXT(", letterbox off (m_bUseLetterboxing 0)") );
		HUD->DisableHUDElements( eDisHUDMaskLevel_PlayerState, 0x7FAF );
		HUD->EnableHUDElements( eDisHUDMaskLevel_PlayerState, 0x6010 );
		if( !m_bUseLetterboxing )
		{
			HUD->DisableHUDElements( eDisHUDMaskLevel_PlayerState, 0x10 );
		}
	}
}

// 2013 0x66c7e0 (104 B)
void UStatePlayerMasterChoice_Base::OnExitState( UDishonoredNativeState* NextState )
{
	UStatePlayerMasterBase::OnExitState( NextState );
	HideChoiceSelection();
	DisPopDisableSave( DDST_StatePlayerMasterChoice_Base, 0.1f );
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	ADishonoredHUD* HUD = PC ? Cast<ADishonoredHUD>( PC->myHUD ) : NULL;
	if( HUD )
	{
		JEDIAG( "bars writer %s::OnExitState: PlayerState level disable 0x6010 enable 0x7FAF", *GetClass()->GetName() );
		HUD->DisableHUDElements( eDisHUDMaskLevel_PlayerState, 0x6010 );
		HUD->EnableHUDElements( eDisHUDMaskLevel_PlayerState, 0x7FAF );
	}
}

// 2013 0xa69340 - 0x400000 (82 B, unnamed): the GFx HUD hides the choice list, and the subtitle / choice elements are raised.
void UStatePlayerMasterChoice_Base::HideChoiceSelection()
{
	UDisGFxMoviePlayerHUD* GFxHUD = DisGetGFxHUD();
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	if( GFxHUD )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( TEXT("DISHONORED(bringup): UStatePlayerMasterChoice_Base: UDisGFxMoviePlayerHUD::HideChoiceSelection (2013 0x78d260) is agent JF's and not ported here") );
		}
	}
	ADishonoredHUD* HUD = PC ? Cast<ADishonoredHUD>( PC->myHUD ) : NULL;
	if( HUD )
	{
		HUD->EnableHUDElements( eDisHUDMaskLevel_PlayerState, 0x6000 );
	}
}

// 2013 0x679230 (77 B): the choice list once the bend time has stopped (agent JF), the walk/nav anim state, and a pending game
// over held for as long as the choice state lasts.
void UStatePlayerMasterChoice_Base::TickState( FLOAT DeltaSeconds )
{
	if( m_DialogState == 1 )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( TEXT("DISHONORED(bringup): UStatePlayerMasterChoice_Base::TickState: ShowChoiceSelection (2012 0x6c7450) is agent JF's and not ported here") );
		}
	}
	UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	if( ADishonoredGameInfo* GameInfo = DisGetGameInfo() )
	{
		if( GameInfo->m_bGameOverPending )
		{
			GameInfo->m_fGameOverDelay = -1.f;
		}
	}
}

// 2013 0x660810 / 0x660830 (2012 0x6af8f0 / 0x6af910): skipping is the derived state's, and never while the choice is up
void UStatePlayerMasterChoice_Base::PlayerRequestSkip()
{
	if( m_DialogState != 1 )
	{
		PlayerRequestSkip_Derived();
	}
}

UBOOL UStatePlayerMasterChoice_Base::IsSkippable() const
{
	return m_DialogState != 1 ? IsSkippable_Derived() : FALSE;
}

// 2013 0x6690a0 (205 B): the pawn, its controller and the camera go into the bend time (their out-of-bend-time flags drop), and
// the master-action channel would be bent to a stop over the tweaks' m_fDialogChoiceBendTime_BlendTime.
void UStatePlayerMasterChoice_Base::PauseBendTime()
{
	if( !DisGetGameInfo() )
	{
		return;
	}
	m_pPlayerPawn->m_bAlwaysOutOfBendTime = FALSE;
	m_pPlayerPawn->m_bOutOfBendTime = FALSE;
	if( m_pPlayerPawn->Controller )
	{
		m_pPlayerPawn->Controller->m_bAlwaysOutOfBendTime = FALSE;
		m_pPlayerPawn->Controller->m_bOutOfBendTime = FALSE;
		if( ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera() )
		{
			Camera->m_bAlwaysOutOfBendTime = FALSE;
			Camera->m_bOutOfBendTime = FALSE;
		}
	}
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( TEXT("DISHONORED(bringup): UStatePlayerMasterChoice_Base::PauseBendTime: ADishonoredGameInfo::BendTime( BTC_PlayerMasterAction, 0, 0, 0, m_fDialogChoiceBendTime_BlendTime ) is not ported") );
	}
}

// 2013 0x669170 (198 B): the reverse - out of the bend time again, the channel back to 1 over m_fDialogChoiceBendTime_BlendOutTime.
void UStatePlayerMasterChoice_Base::UnpauseBendTime()
{
	if( !DisGetGameInfo() )
	{
		return;
	}
	m_pPlayerPawn->m_bAlwaysOutOfBendTime = TRUE;
	m_pPlayerPawn->m_bOutOfBendTime = TRUE;
	if( m_pPlayerPawn->Controller )
	{
		m_pPlayerPawn->Controller->m_bAlwaysOutOfBendTime = TRUE;
		m_pPlayerPawn->Controller->m_bOutOfBendTime = TRUE;
		if( ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera() )
		{
			Camera->m_bAlwaysOutOfBendTime = TRUE;
			Camera->m_bOutOfBendTime = TRUE;
		}
	}
}
