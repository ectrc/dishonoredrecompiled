// DishonoredGame/src/dishonoredplayerpawn_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The player speaks through m_pAudioLogConvActor, an ADisPlayerAudioLogDummyActor
// that PostBeginPlay_Dialog (2012 rva 0x7063a0, 275 B) spawns 100 units in front of the camera bone the player tweaks name.
// 2012 rvas: FireDialogHook 0x6fb4a0 (the dummy's BreakOutOfConversation( NewConvo, MAX ) then its FireDialogHook),
// FireDialogHookForAudioLog 0x6fb4f0, GetHookParameters_Derived 0x6fb530, AddDialogOneShot 0x6fb5a0, RemoveDialogOneShot
// 0x6fb5c0, GetSpeakerFromOneShotOwner 0x6fb5e0, IsAvailableForDialog 0x706580 (2013 0x6ad070), StartDialog 0x6fb550,
// EndDialog 0x706600 (2013 0x6ad0f0).
// The player's dialog state is UStatePlayerMasterInDialog on the master FSM (StartDialog requests it with
// FStatePlayerMasterInDialog_Param, EndDialog asks the logical state to exit, IsAvailableForDialog asks the FSM whether the
// transition is allowed) - agent FK's FSM package (agentFO.md section 9.4). The state class is excluded here, so the FSM
// has no such state: a blocking-player conversation finds the player unavailable, which -frdiag reports.
// PostBeginPlay_Dialog spawns the dummy at the pawn's location; the tweaks' camera bone and the 100-unit offset are the
// audio-log package's.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

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

eDisDialogAvailability ADishonoredPlayerPawn::IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	if( ( _StartDialogFlags & 2 ) == 0 )
	{
		return DDA_IGNORED;
	}
	if( m_pPlayerMasterFSM && m_pPlayerMasterFSM->CanTransitionTo( UStatePlayerMasterInDialog::StaticClass(), NULL ) )
	{
		return DDA_AVAILABLE;
	}
	FRDIAG( "bind player %s: the master FSM cannot enter UStatePlayerMasterInDialog (unported state), so the player is rejected for a blocking conversation", *GetName() );
	return DDA_REJECTED_BY_PAWN_STATE;
}

void ADishonoredPlayerPawn::StartDialog( FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex )
{
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredPlayerPawn::StartDialog: UStatePlayerMasterInDialog is not ported (agent FK's FSM package); the player does not enter its dialog state") );
	}
}

void ADishonoredPlayerPawn::EndDialog( FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason )
{
}
