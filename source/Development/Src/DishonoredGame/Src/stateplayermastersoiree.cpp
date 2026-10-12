/*=============================================================================
	stateplayermastersoiree.cpp: agent FZ (PHASE18 FZ) - UStatePlayerMasterSoiree, the player master state a cutscene's
	player group puts the player in, and FStatePlayerMasterSoiree_Param, what MAT_BeginPlayerGroup requests it with.
	Retail 2012 rvas from the Shipping PDB (this unit's banner): OnEnterState 0x6af580 (2013 0x6730b0, 156 B: the base is
	inlined), OnResetState 0x6af5f0 (2013 0x668670), OnCancelState 0x6af630, OnExitState 0x6af650 (2013 0x6639e0),
	TickState 0x6af690, SetSoireeBlendOutTime 0x6af6c0 (2013 0x6604c0), OnSoireeFinished 0x6af6e0, CanTransitionExternal
	0x6b76f0 (2013 0x673150), FStatePlayerMasterSoiree_Param::OnPending 0x6c5f80. The five unnamed in the 2013 database
	are matched by neighbourhood only and are quoted as 2012.
=============================================================================*/
#include "DishonoredGame.h"
#include "stateplayermasterparams.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_UStatePlayerMasterSoiree()
{
}

FStatePlayerMasterSoiree_Param::FStatePlayerMasterSoiree_Param( USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags )
	: FStatePlayerMasterBase_Param( UStatePlayerMasterSoiree::StaticClass() )
	, m_pMatinee( Matinee )
	, m_fBlendInTime( BlendInTime )
	, m_Flags( Flags )
{
}

/**
 * DISHONORED(port): 2012 0x6c5f80. The base OnPending (FDisUseState_Base_Param's, 0x6959c0: the state's firing object),
 * then the matinee, the blend-in time, a 0.4 s default blend-out and the flags onto the pending state.
 */
void FStatePlayerMasterSoiree_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FStatePlayerMasterBase_Param::OnPending( PendingState, ManagedObject );
	UStatePlayerMasterSoiree* Soiree = Cast<UStatePlayerMasterSoiree>( PendingState );
	if( Soiree )
	{
		Soiree->m_pMatinee = m_pMatinee;
		Soiree->m_fSoireeBlendInTime = m_fBlendInTime;
		Soiree->m_fSoireeBlendOutTime = 0.4f;
		Soiree->m_Flags = m_Flags;
	}
}

/**
 * DISHONORED(port): 2012 0x6af580. The base, every master-action input bit cleared, the control state entered with the
 * parameters OnPending stored, the matinee pointer consumed, and a held movable object dropped.
 */
void UStatePlayerMasterSoiree::OnEnterState( UDishonoredNativeState* LastState )
{
	Super::OnEnterState( LastState );
	if( !m_pPlayerPawn )
	{
		return;
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, FALSE, DisInputMaskLevel_MasterAction );
	m_SoireeControlState.EnterSoireeMode( m_pMatinee, m_pPlayerPawn, m_fSoireeBlendInTime, m_Flags );
	m_pMatinee = NULL;
	if( m_pPlayerPawn->IsHoldingMovableObject() )
	{
		m_pPlayerPawn->DropMovableObject( FALSE );
	}
}

// DISHONORED(port): 2012 0x6af650: the base, the control state left with the blend-out time as an interruption, the input back
void UStatePlayerMasterSoiree::OnExitState( UDishonoredNativeState* NextState )
{
	Super::OnExitState( NextState );
	if( !m_pPlayerPawn )
	{
		return;
	}
	m_SoireeControlState.ExitSoireeMode( m_pPlayerPawn, m_fSoireeBlendOutTime, TRUE );
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
}

// DISHONORED(port): 2012 0x6af5f0: a second request while in the state re-enters the control state with the new parameters
UBOOL UStatePlayerMasterSoiree::OnResetState()
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.EnterSoireeMode( m_pMatinee, m_pPlayerPawn, m_fSoireeBlendInTime, m_Flags );
	}
	m_pMatinee = NULL;
	return TRUE;
}

// DISHONORED(port): 2012 0x6af630
void UStatePlayerMasterSoiree::OnCancelState()
{
	m_pMatinee = NULL;
}

// DISHONORED(port): 2012 0x6af690: the base tick (empty), then the control state's
void UStatePlayerMasterSoiree::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.TickSoireeMode( m_pPlayerPawn );
	}
}

// DISHONORED(port): 2012 0x6b76f0: while in a soiree the only external transition allowed is into a dead state
UBOOL UStatePlayerMasterSoiree::CanTransitionExternal( const UClass* StateID ) const
{
	return StateID != NULL && StateID->IsChildOf( UStatePlayerMasterDead::StaticClass() );
}

// DISHONORED(port): 2012 0x6af6c0
void UStatePlayerMasterSoiree::SetSoireeBlendOutTime( FLOAT BlendOutTime )
{
	m_fSoireeBlendOutTime = BlendOutTime;
}

// DISHONORED(port): 2012 0x6af6e0: MAT_FinishPlayerGroup's end of the scene - the control state left cleanly, then the state exits
void UStatePlayerMasterSoiree::OnSoireeFinished()
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.ExitSoireeMode( m_pPlayerPawn, m_fSoireeBlendOutTime, FALSE );
	}
	RequestStateExit();
}
