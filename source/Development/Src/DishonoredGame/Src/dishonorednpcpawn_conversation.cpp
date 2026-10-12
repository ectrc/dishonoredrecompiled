// DishonoredGame/src/dishonorednpcpawn_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The NPC's speaker overrides, 2012 rvas: FireDialogHook 0x7ac070 (32 B),
// SupportsDialogHook 0x7ac090 (25 B), IsAvailableForDialog 0x7c17c0 (2013 0x774280, 188 B), StartDialog 0x7c1880 (300 B),
// EndDialog 0x7c19b0 (247 B), ShouldConvLowerAlertness 0x7ac050.
// The AI halves are the brain package's and are named at the site: IsAvailableForDialog asks the current behaviour
// (UDishonoredAIBehavior::CallGetDialogAvailability) and the master action state (UStateNPCMasterAction::GetDialogAvailability_Action);
// StartDialog requests a soirée LOD (FDisComponentLODManager::StartLOD, eDisLODStatus_Soiree) and raises
// FAIStimStruct_BlockingDialogRequest on the brain when the dialog is blocking; EndDialog raises FAIStimStruct_BlockingDialogEnd
// and stops the LOD. Without the two stims the brain is not told to hold the pawn for the scene - that is agent FS's
// MAT_BeginAIGroup / behaviour contract (PHASE17 FS).
#include "DishonoredGame.h"
#include "frdialogdiag.h"

void ADishonoredNPCPawn::FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator )
{
	if( m_pConvComponent )
	{
		m_pConvComponent->FireDialogHook( _Hook, _pInitiator, FALSE );
	}
}

UBOOL ADishonoredNPCPawn::SupportsDialogHook( eDisDialogHook _Hook ) const
{
	return m_pConvComponent && m_pConvComponent->SupportsDialogHook( _Hook );
}

eDisDialogAvailability ADishonoredNPCPawn::IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	const eDisDialogAvailability Availability = IDisConvSpeakerInterface::IsAvailableForDialog( _Priority, _StartDialogFlags );
	if( Availability != DDA_AVAILABLE )
	{
		return Availability;
	}
	const UDishonoredAIBrain* Brain = const_cast<ADishonoredNPCPawn*>( this )->GetAIBrain();
	if( !Brain )
	{
		return DDA_BRAIN_HAS_NO_BLOCKING_BEHAVIOR;
	}
	if( !Brain->m_pCurrentBehavior )
	{
		return DDA_AVAILABLE;
	}
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredNPCPawn::IsAvailableForDialog: the behaviour's dialog availability (UDishonoredAIBehavior::CallGetDialogAvailability) is not ported; %s answers available") , *GetName() );
	}
	return DDA_AVAILABLE;
}

void ADishonoredNPCPawn::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex )
{
	IDisConvSpeakerInterface::StartDialog( _pDialogRunningInst, _bBlocking, _StartDialogFlags, _FiringHookIndex );
	FRDIAG( "bind NPC %s StartDialog blocking %d flags %d (soiree LOD and FAIStimStruct_BlockingDialogRequest not ported)", *GetName(), _bBlocking ? 1 : 0, _StartDialogFlags );
}

void ADishonoredNPCPawn::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
	FRDIAG( "bind NPC %s EndDialog reason %d (FAIStimStruct_BlockingDialogEnd not ported)", *GetName(), (INT)_ConvoEndReason );
	IDisConvSpeakerInterface::EndDialog( _pDialogRunningInst, _ConvoEndReason );
}
