// DishonoredGame/src/displayeraudiologdummyactor.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The actor the player speaks through. 2012 rvas: GetSpeakerLocInfo 0x69fdb0
// (71 B: one entry at the actor's location, no mesh), AddDialogOneShot 0x699670 (ADisDialogInanimateDummy's 0x6985c0 shape:
// the one-shot is kept in m_DialogOneShots and, once the component is attached, bound at once), RemoveDialogOneShot
// 0x691f60, GetSpeakerFromOneShotOwner (ADisDialogInanimateDummy's 0x691c00: the actor's own speaker interface).
// Not ported (agentFR.md inventory): FireDialogHook 0x691fb0 / FireDialogHookWithFlavor 0x691fc0, IsReadyToSpeak 0x691ff0,
// StartDialog 0x692010, PlayDialogBlurb 0x692090, EndDialog 0x6920f0, NativePostRenderFor 0x695460, Tick 0x6a8ee0 (the
// heart gadget's one-shots and the audio-log playback), the Ak callback. GameLoad / PostGameLoad are agent EF's
// (dissavegame.cpp).
#include "DishonoredGame.h"

void ADisPlayerAudioLogDummyActor::GetSpeakerLocInfo( TArray<FDisConvSpeakerLocInfo>& _rLocInfoOut ) const
{
	FDisConvSpeakerLocInfo& Info = _rLocInfoOut( _rLocInfoOut.AddZeroed() );
	Info.m_Location = Location;
	Info.m_pAKEventOwner = const_cast<ADisPlayerAudioLogDummyActor*>( this );
}

void ADisPlayerAudioLogDummyActor::AddDialogOneShot( UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner )
{
	FDisSpeakerOneShotInfo& Info = m_DialogOneShots( m_DialogOneShots.AddZeroed() );
	Info.m_pOneShot = _pOneShot;
	Info.m_bIsOwner = _bIsOwner ? 1 : 0;
	if( m_pConvComponent && m_pConvComponent->IsAttached() )
	{
		m_pConvComponent->AddOneShotAfterSpawned( Info );
	}
}

void ADisPlayerAudioLogDummyActor::RemoveDialogOneShot( UDisDialogVoiceData* _pOneShot )
{
	for( INT i = 0; i < m_DialogOneShots.Num(); i++ )
	{
		if( m_DialogOneShots(i).m_pOneShot == _pOneShot )
		{
			RemoveOneShot( m_DialogOneShots(i) );
			m_DialogOneShots.Remove( i-- );
		}
	}
}
