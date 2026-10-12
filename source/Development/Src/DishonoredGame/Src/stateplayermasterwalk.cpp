/*=============================================================================
	stateplayermasterwalk.cpp: agent FZ (PHASE18 FZ) - UStatePlayerMasterWalk, the player master FSM's default state.
	Retail 2012 rvas: OnEnterState 0x6af890 (2013 0x676980), TickState 0x6beba0 (2013 0x6769f0),
	ShouldLimitControllerSensitivity 0x6b7c20 (2013 0x6687b0), CheckForAutoCrouch 0x6b7ce0 (2013 0x668870).
=============================================================================*/
#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_UStatePlayerMasterWalk()
{
}

FStatePlayerMasterWalk_Param::FStatePlayerMasterWalk_Param()
	: FStatePlayerMasterBase_Param( UStatePlayerMasterWalk::StaticClass() )
{
}

// DISHONORED(port): 2012 0x6959c0 - the pending master state is told its pawn
void FStatePlayerMasterBase_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UStatePlayerMasterBase* State = Cast<UStatePlayerMasterBase>( PendingState );
	if( State )
	{
		State->m_pPlayerPawn = Cast<ADishonoredPlayerPawn>( ManagedObject );
	}
}

/**
 * DISHONORED(port): 2012 0x6af890. The base, every master-action input bit enabled again
 * (EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction ) - the soiree state cleared them), then the
 * navigation anim state.
 */
void UStatePlayerMasterWalk::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	if( m_pPlayerPawn )
	{
		m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
		UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	}
}

/**
 * DISHONORED(port): 2012 0x6beba0: the base tick (empty), CheckForAutoCrouch, UpdateWalkNavAnimState.
 * DISHONORED(bringup): CheckForAutoCrouch (2012 0x6b7ce0, 2021 B: the head-room line checks that crouch the player
 * under low geometry) is not ported - it stands on DisGetPawnFeet, DisIsCrouchObstructed and the crouch tweaks.
 */
void UStatePlayerMasterWalk::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );
	if( m_pPlayerPawn )
	{
		UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	}
}
