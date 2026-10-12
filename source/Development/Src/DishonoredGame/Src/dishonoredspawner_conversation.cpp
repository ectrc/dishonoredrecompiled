// DishonoredGame/src/dishonoredspawner_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The spawner as a one-shot owner (retail keeps these in dishonoredspawner.cpp;
// they get their own unit here beside agent EP's spawn port). 2013 rvas: AddDialogOneShot 0x64b230 (2012 0x697300, 60 B),
// GetSpeakerFromOneShotOwner 0x64b270 (0x697340, 134 B; retail's ADishonoredPawn::IsDead test is the pawn's Health here, the
// pawn's health predicate being unported), RemoveDialogOneShot 2012 0x69f750 (300 B).
#include "DishonoredGame.h"
#include "frdialogdiag.h"

void ADishonoredSpawner::AddDialogOneShot( UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner )
{
	FDisSpeakerOneShotInfo& Info = m_DialogOneShots( m_DialogOneShots.AddZeroed() );
	Info.m_pOneShot = _pOneShot;
	Info.m_bIsOwner = _bIsOwner ? 1 : 0;
	FRDIAG( "oneshot spawner %s now holds %d one-shot(s) (%d pawn(s) already spawned)", *GetName(), m_DialogOneShots.Num(), m_SpawnedPawns.Num() );
}

void ADishonoredSpawner::RemoveDialogOneShot( UDisDialogVoiceData* _pOneShot )
{
	for( INT i = 0; i < m_DialogOneShots.Num(); i++ )
	{
		if( m_DialogOneShots(i).m_pOneShot != _pOneShot )
		{
			continue;
		}
		for( INT j = 0; j < m_SpawnedPawns.Num(); j++ )
		{
			if( m_SpawnedPawns(j) )
			{
				m_SpawnedPawns(j)->RemoveOneShot( m_DialogOneShots(i) );
			}
		}
		m_DialogOneShots.Remove( i );
		return;
	}
}

IDisConvSpeakerInterface* ADishonoredSpawner::GetSpeakerFromOneShotOwner( IDisConvSpeakerInterface* _pTreeOwner )
{
	for( INT i = 0; i < m_SpawnedPawns.Num(); i++ )
	{
		ADishonoredNPCPawn* Pawn = m_SpawnedPawns(i);
		if( Pawn && Pawn->Health > 0 && (IDisConvSpeakerInterface*)Pawn != _pTreeOwner )
		{
			return Pawn;
		}
	}
	return NULL;
}
