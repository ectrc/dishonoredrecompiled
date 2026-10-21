// DishonoredGame/src/dishonoredplayerpawn_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The player speaks through m_pAudioLogConvActor, an ADisPlayerAudioLogDummyActor
// that PostBeginPlay_Dialog (2012 rva 0x7063a0, 275 B) spawns 100 units in front of the camera bone the player tweaks name.
// 2012 rvas: FireDialogHook 0x6fb4a0 (the dummy's BreakOutOfConversation( NewConvo, MAX ) then its FireDialogHook),
// FireDialogHookForAudioLog 0x6fb4f0, GetHookParameters_Derived 0x6fb530, AddDialogOneShot 0x6fb5a0, RemoveDialogOneShot
// 0x6fb5c0, GetSpeakerFromOneShotOwner 0x6fb5e0, IsAvailableForDialog 0x706580 (2013 0x6ad070), StartDialog 0x6fb550,
// EndDialog 0x706600 (2013 0x6ad0f0).
// The player's dialog state is UStatePlayerMasterInDialog on the master FSM (StartDialog requests it with
// FStatePlayerMasterInDialog_Param, EndDialog asks the logical state to exit, IsAvailableForDialog asks the FSM whether the
// transition is allowed). agent JE (PHASE24 JE): the three are retail's 2013 bodies, the state is ported.
// PostBeginPlay_Dialog spawns the dummy at the pawn's location; the tweaks' camera bone and the 100-unit offset are the
// audio-log package's.
#include "DishonoredGame.h"
#include "frdialogdiag.h"
#include "stateplayermasterparams.h"
#include "jediag.h"

IDisConvSpeakerInterface* ADishonoredPlayerPawn::GetAudioLogSpeaker() const
{
	return m_pAudioLogConvActor ? (IDisConvSpeakerInterface*)m_pAudioLogConvActor : NULL;
}

void ADishonoredPlayerPawn::PostBeginPlay_Dialog()
{
	if( !m_pAudioLogConvActor && GWorld )
	{
		m_pAudioLogConvActor = Cast<ADisPlayerAudioLogDummyActor>( GWorld->SpawnActor( ADisPlayerAudioLogDummyActor::StaticClass(), FName( TEXT("DisPlayerAudioLogDummyActor") ), Location, FRotator(0,0,0), NULL, TRUE, FALSE, this, this ) );
		FRDIAG( "spawned the player's audio-log dummy %s", m_pAudioLogConvActor ? *m_pAudioLogConvActor->GetName() : TEXT("<failed>") );
	}
}

void ADishonoredPlayerPawn::AddDialogOneShot( UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner )
{
	if( m_pAudioLogConvActor )
	{
		m_pAudioLogConvActor->AddDialogOneShot( _pOneShot, _bIsOwner );
	}
}

void ADishonoredPlayerPawn::RemoveDialogOneShot( UDisDialogVoiceData* _pOneShot )
{
	if( m_pAudioLogConvActor )
	{
		m_pAudioLogConvActor->RemoveDialogOneShot( _pOneShot );
	}
}

IDisConvSpeakerInterface* ADishonoredPlayerPawn::GetSpeakerFromOneShotOwner( IDisConvSpeakerInterface* _pTreeOwner )
{
	return GetAudioLogSpeaker();
}

void ADishonoredPlayerPawn::FireDialogHook( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator )
{
	if( IDisConvSpeakerInterface* Speaker = GetAudioLogSpeaker() )
	{
		Speaker->BreakOutOfConversation( DisConvoEndReason_NewConvo, eDisDialogHookFireGroup_MAX );
		Speaker->FireDialogHook( _Hook, _pInitiator );
	}
}

void ADishonoredPlayerPawn::FireDialogHookForAudioLog( eDisDialogHook _Hook, IDisConvSpeakerInterface* _pInitiator )
{
	if( IDisConvSpeakerInterface* Speaker = GetAudioLogSpeaker() )
	{
		Speaker->BreakOutOfConversation( DisConvoEndReason_NewConvo, eDisDialogHookFireGroup_MAX );
		Speaker->FireDialogHookForAudioLog( _Hook, _pInitiator );
	}
}

FDisHookParameters* ADishonoredPlayerPawn::GetHookParameters_Derived()
{
	IDisConvSpeakerInterface* Speaker = GetAudioLogSpeaker();
	return Speaker ? Speaker->GetHookParameters() : NULL;
}

// 2013 0x6ad070 (127 B, 2012 0x706580 the same): only a conversation that blocks the player asks; already in a dialog of at
// least that priority with flag 4 (not started yet) is "in a higher-priority conversation", otherwise the master FSM decides.
eDisDialogAvailability ADishonoredPlayerPawn::IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	if( ( _StartDialogFlags & 2 ) == 0 )
	{
		return DDA_IGNORED;
	}
	UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( m_pPlayerMasterFSM->GetLogicalState() );
	if( InDialog )
	{
		const FDisDialogRunningInstance* Instance = (const FDisDialogRunningInstance*)InDialog->m_pDialogRunningInst;
		const INT CurPriority = Instance ? Instance->m_CurPriority : NOT_IN_A_CONV;
		if( _Priority <= CurPriority && ( _StartDialogFlags & 4 ) )
		{
			return DDA_IN_HIGHER_PRIORITY_CONVO;
		}
	}
	return m_pPlayerMasterFSM->CanTransitionTo( UStatePlayerMasterInDialog::StaticClass(), NULL ) ? DDA_AVAILABLE : DDA_REJECTED_BY_PAWN_STATE;
}

// 2013 0x6a2460 (93 B; 2012 0x6fb550, 47 B): a conversation that has not started yet first breaks the audio log out of whatever it
// plays (skip-whole, every fire group), then the master FSM is asked for the dialog state with the running instance and the
// carry-body flag (2013 bit 0x10).
void ADishonoredPlayerPawn::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex )
{
	if( m_pAudioLogConvActor && ( _StartDialogFlags & 4 ) )
	{
		( (IDisConvSpeakerInterface*)m_pAudioLogConvActor )->BreakOutOfConversation( DisConvoEndReason_PlayerSkipWhole, eDisDialogHookFireGroup_MAX );
	}
	FStatePlayerMasterInDialog_Param MasterParam( _pDialogRunningInst, ( _StartDialogFlags & 0x10 ) != 0 );
	const UBOOL bAccepted = m_pPlayerMasterFSM->RequestStateChange( MasterParam, NULL, FALSE );
	JEDIAG( "player StartDialog blocking %d flags 0x%x hook %d: dialog state %s", _bBlocking ? 1 : 0, _StartDialogFlags, _FiringHookIndex, bAccepted ? TEXT("requested") : TEXT("refused") );
}

// 2013 0x6ad0f0 (34 B, 2012 0x706600 the same): the dialog state, if that is where the player is, is asked to exit
void ADishonoredPlayerPawn::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
	UStatePlayerMasterInDialog* InDialog = Cast<UStatePlayerMasterInDialog>( m_pPlayerMasterFSM->GetLogicalState() );
	JEDIAG( "player EndDialog reason %d: dialog state %s", (INT)_ConvoEndReason, InDialog ? TEXT("asked to exit") : TEXT("not current") );
	if( InDialog )
	{
		InDialog->RequestStateExit();
	}
}
