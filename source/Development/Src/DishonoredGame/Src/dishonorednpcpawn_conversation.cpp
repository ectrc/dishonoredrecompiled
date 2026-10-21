// DishonoredGame/src/dishonorednpcpawn_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The NPC's speaker overrides, 2012 rvas: FireDialogHook 0x7ac070 (32 B),
// SupportsDialogHook 0x7ac090 (25 B), IsAvailableForDialog 0x7c17c0 (2013 0x774280, 188 B), StartDialog 0x7c1880 (300 B),
// EndDialog 0x7c19b0 (247 B), ShouldConvLowerAlertness 0x7ac050.
// agent JE (PHASE24 JE): the AI halves are ported (2013 bodies): IsAvailableForDialog asks the current behaviour
// (UDishonoredAIBehavior::CallGetDialogAvailability) and the master action state (UStateNPCMasterAction::GetDialogAvailability_Action);
// StartDialog raises FAIStimStruct_BlockingDialogRequest on the brain when the dialog is blocking; EndDialog raises
// FAIStimStruct_BlockingDialogEnd. The soiree LOD request / release around them (FDisComponentLODManager) is not ported.
#include "DishonoredGame.h"
#include "frdialogdiag.h"
#include "aistimstruct.h"
#include "jediag.h"

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

// 2013 0x774280 (188 B, 2012 0x7c17c0 the same): the speaker's own answer, then a brain is needed (none: no blocking
// behaviour), then the current behaviour's say and, when that is available, the master action state's.
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
	const eDisDialogAvailability BehaviorAvailability = Brain->m_pCurrentBehavior->CallGetDialogAvailability( _Priority, _StartDialogFlags );
	if( BehaviorAvailability != DDA_AVAILABLE )
	{
		JEDIAG( "dialog availability %s: behaviour %s answers %d (flags 0x%x)", *GetName(), *Brain->m_pCurrentBehavior->GetClass()->GetName(), (INT)BehaviorAvailability, _StartDialogFlags );
		return BehaviorAvailability;
	}
	const UStateNPCMasterAction* Action = m_pNPCMasterFSM ? Cast<UStateNPCMasterAction>( m_pNPCMasterFSM->GetLogicalState() ) : NULL;
	return Action ? Action->GetDialogAvailability_Action( _Priority, _StartDialogFlags ) : BehaviorAvailability;
}

/**
 * 2013 0x77fe50 (296 B; 2012 0x7c1880): the speaker's own start, then - for a blocking conversation - the brain hears
 * BlockingDialogRequest with the conversation component's running instance and flag 1 (stop blocking for the AI's notice).
 * DISHONORED(bringup), named and not called: the soiree LOD request FDisComponentLODManager::StartLOD( eLRP_Soiree, Soiree / Auto /
 * MAX, no time-out ) - the LOD manager is not ported.
 */
void ADishonoredNPCPawn::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex )
{
	IDisConvSpeakerInterface::StartDialog( _pDialogRunningInst, _bBlocking, _StartDialogFlags, _FiringHookIndex );
	if( _bBlocking )
	{
		if( UDishonoredAIBrain* Brain = GetAIBrain() )
		{
			FAIStimStruct_BlockingDialogRequest Stim = DisMakeStim<FAIStimStruct_BlockingDialogRequest>( EAIStimID_BlockingDialogRequest );
			Stim.m_iRunningInstID = m_pConvComponent ? m_pConvComponent->GetRunningInstID() : 0;
			Stim.m_bStopBlockingForAINotice = ( _StartDialogFlags & 1 ) ? 1 : 0;
			DisHandleAIStim( Brain, Stim, this );
		}
	}
	FRDIAG( "bind NPC %s StartDialog blocking %d flags %d", *GetName(), _bBlocking ? 1 : 0, _StartDialogFlags );
	JEDIAG( "NPC %s StartDialog blocking %d flags 0x%x running id %d", *GetName(), _bBlocking ? 1 : 0, _StartDialogFlags, m_pConvComponent ? m_pConvComponent->GetRunningInstID() : -1 );
}

/**
 * 2013 0x77ff80 (226 B; 2012 0x7c19b0, which tested IsDead instead): unless the pawn is being destroyed or has terminated, its brain
 * hears BlockingDialogEnd with the running instance; then the speaker's own end.
 * DISHONORED(bringup), named and not called: FDisComponentLODManager::StopAllLODByAsker (the LOD manager is not ported).
 */
void ADishonoredNPCPawn::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
	if( _ConvoEndReason != DisConvoEndReason_Destroyed && !( GIsGame && m_bLegalToTerminate && m_bHasTerminated ) )
	{
		if( UDishonoredAIBrain* Brain = GetAIBrain() )
		{
			FAIStimStruct_BlockingDialogEnd Stim = DisMakeStim<FAIStimStruct_BlockingDialogEnd>( EAIStimID_BlockingDialogEnd );
			Stim.m_iRunningInstID = m_pConvComponent ? m_pConvComponent->GetRunningInstID() : 0;
			DisHandleAIStim( Brain, Stim, this );
		}
	}
	FRDIAG( "bind NPC %s EndDialog reason %d", *GetName(), (INT)_ConvoEndReason );
	JEDIAG( "NPC %s EndDialog reason %d", *GetName(), (INT)_ConvoEndReason );
	IDisConvSpeakerInterface::EndDialog( _pDialogRunningInst, _ConvoEndReason );
}
