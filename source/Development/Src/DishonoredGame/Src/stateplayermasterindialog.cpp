/*=============================================================================
	stateplayermasterindialog.cpp: agent FZ (PHASE18 FZ) - the soiree hand-over of UStatePlayerMasterInDialog, which is
	what the player MAT_* contract reaches when the master FSM is already in a dialog: StartSoireeControl 2012 0x6afa80
	(2013 0x6696e0), StopSoireeControl 0x6afab0 (2013 0x663e20), SetSoireeBlendOutTime 0x6afae0 (2013 0x6609c0, inline in
	the CppText). The dialog state itself - OnEnterState 2012 0x6b0c40 and the eleven other functions of this unit's
	banner - is not ported; nothing enters it yet (ADishonoredPlayerPawn::StartDialog is FR's and checks CanTransitionTo).
=============================================================================*/
#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_UStatePlayerMasterInDialog()
{
}

void UStatePlayerMasterInDialog::StartSoireeControl( USeqAct_Interp* Matinee, FLOAT BlendInTime, INT Flags )
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.EnterSoireeMode( Matinee, m_pPlayerPawn, BlendInTime, Flags );
	}
}

void UStatePlayerMasterInDialog::StopSoireeControl( UBOOL bInterrupted )
{
	if( m_pPlayerPawn )
	{
		m_SoireeControlState.ExitSoireeMode( m_pPlayerPawn, m_fSoireeBlendOutTime, bInterrupted );
	}
}
