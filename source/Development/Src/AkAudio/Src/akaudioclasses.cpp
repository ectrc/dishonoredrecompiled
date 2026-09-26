/*=============================================================================
	akaudioclasses.cpp - the rest of AkAudio: the Wwise callbacks, UAkComponent (which IS the Wwise game
	object), AAkAmbientSound and the nine Kismet actions.

	Ported from the retail 2013 exe; each function carries its 2013 rva. The 2012 PDB attributes the same
	set to AkAudio/src/akaudioclasses.cpp.

	Not ported in this wave, recorded as follow-ups in resources/docs/agents/agentAN.md:
	UInterpTrackAkEvent / UInterpTrackAkRTPC and their track instances (the matinee audio tracks, 0x5b0570..),
	UActorFactoryAkAmbientSound (editor only), AAkAmbientSound::GameSave / GameLoad (the save-game path).
=============================================================================*/
#include "AkAudio.h"

#if DISHONORED_WITH_WWISE

/*-----------------------------------------------------------------------------
	Wwise callbacks
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5b2150's DisAkEndOfEventCB. The cookie is the world's audio system, which
 * keeps the playing ids it still cares about; DisAkEndOfEventCB is 2012 rva 0x7f9040 and
 * UDishonoredAudioSystem::ConsumeEndOfEventNotifies (2013 0x7a9970)
 * drains them on its next update. Our UAudioSystem base has no such queue yet, so the notify is logged and
 * the backend's own bookkeeping (which drops the playing id) is what the game observes.
 */
void DisAkEndOfEventCallback( AkCallbackType InType, AkCallbackInfo* InInfo )
{
	if( InType != AK_EndOfEvent || !InInfo )
	{
		return;
	}
	const AkEventCallbackInfo* EventInfo = (const AkEventCallbackInfo*)InInfo;
	debugfSuppressed( NAME_DevAudio, TEXT("Wwise: end of event 0x%08x (playing id %u)"), EventInfo->eventID, EventInfo->playingID );
}

/**
 * DISHONORED(port): 2013 rva 0x5ae840 (AkCallback, 2012 0x5f5000). The cookie is the USeqAct_AkPostEvent
 * that posted the event; every end-of-event decrements its outstanding count so UpdateOp can finish the
 * latent action.
 */
static void AkCallback( AkCallbackType InType, AkCallbackInfo* InInfo )
{
	if( InType != AK_EndOfEvent || !InInfo || !InInfo->pCookie )
	{
		return;
	}
	USeqAct_AkPostEvent* Action = (USeqAct_AkPostEvent*)InInfo->pCookie;
	if( Action->Signal > 0 )
	{
		--Action->Signal;
	}
}

/**
 * DISHONORED(port): 2013 rva 0x5ae7d0 (AkBankCallback, 2012 0x5f4f90): the cookie is the USeqAct_AkLoadBank, whose
 * bWaitingCallback drops and whose Signal is raised when the bank finishes.
 */
static void AkBankCallback( AkUInt32 /*InBankID*/, AKRESULT /*InResult*/, AkMemPoolId /*InMemPoolID*/, void* InCookie )
{
	USeqAct_AkLoadBank* Action = (USeqAct_AkLoadBank*)InCookie;
	if( Action )
	{
		Action->bWaitingCallback = FALSE;
		Action->Signal = 1;
	}
}

/**
 * DISHONORED(port): 2013 rva 0x5ae8e0 (AkAmbientSoundCallback, 2012 0x5f50a0): the cookie is the AAkAmbientSound, which
 * records that its event ended so the audio system can start it again on the next pass.
 */
static void AkAmbientSoundCallback( AkCallbackType InType, AkCallbackInfo* InInfo )
{
	if( InType != AK_EndOfEvent || !InInfo || !InInfo->pCookie )
	{
		return;
	}
	AAkAmbientSound* AmbientSound = (AAkAmbientSound*)InInfo->pCookie;
	AmbientSound->m_bEnded = 1;
	AmbientSound->m_bStarted = FALSE;
}

/*-----------------------------------------------------------------------------
	UAkComponent
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x5b3d20: attach, then become a Wwise game object
void UAkComponent::Attach()
{
	Super::Attach();
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->RegisterComponent( this );
	}
}

// DISHONORED(port): 2013 rva 0x5b3980: stop first when the component owns its sounds, then unregister
void UAkComponent::Detach( UBOOL bWillReattach )
{
	if( bStopWhenOwnerDestroyed )
	{
		Stop();
	}
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice && AudioDevice->m_GameObjects.RemoveKey( this ) )
	{
		AK::SoundEngine::UnregisterGameObj( GetAkGameObjectID() );
	}
	Super::Detach( bWillReattach );
}

// DISHONORED(port): 2013 rva 0x5b39e0: the same as Stop plus the unregister, in that order
void UAkComponent::FinishDestroy()
{
	Stop();
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice && AudioDevice->m_GameObjects.RemoveKey( this ) )
	{
		AK::SoundEngine::UnregisterGameObj( GetAkGameObjectID() );
	}
	Super::FinishDestroy();
}

// DISHONORED(port): 2013 rva 0x5b3ab0: unregister only; nothing else is safe after an error
void UAkComponent::ShutdownAfterError()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice && AudioDevice->m_GameObjects.RemoveKey( this ) )
	{
		AK::SoundEngine::UnregisterGameObj( GetAkGameObjectID() );
	}
	Super::ShutdownAfterError();
}

/**
 * DISHONORED(port): 2013 rva 0x5b1150. Cancel the callback of every playing id this component still holds
 * (so a stopped sound cannot notify a dead cookie), drop the radius list, then stop everything on the object.
 */
void UAkComponent::Stop()
{
	for( INT RadiusIndex = 0; RadiusIndex < m_PlayingRadii.Num(); ++RadiusIndex )
	{
		AK::SoundEngine::CancelEventCallback( (AkPlayingID)m_PlayingRadii( RadiusIndex ).m_PlayingID );
	}
	m_PlayingRadii.Empty();
	if( UAkAudioDevice::Get() )
	{
		AK::SoundEngine::StopAll( GetAkGameObjectID() );
	}
}

/*-----------------------------------------------------------------------------
	AAkAmbientSound
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5b3440. The event is posted on the ambient sound itself with AK_EndOfEvent and
 * AkAmbientSoundCallback, so the sound knows when to restart; m_bStarted is set whether or not there was an
 * event to post, which is retail's behaviour.
 */
void AAkAmbientSound::StartEvent()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice && PlayEvent )
	{
		AudioDevice->PostEvent( PlayEvent->m_akID, PlayEvent->GetMaxRadius(), this, NAME_None, AK_EndOfEvent,
			AkAmbientSoundCallback, this, StopWhenOwnerIsDestroyed ? TRUE : FALSE );
	}
	m_bStarted = TRUE;
}

// DISHONORED(port): 2013 rva 0x5b2060
void AAkAmbientSound::StopEvent()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		UAkComponent* Component = AudioDevice->GetAkComponent( this, NAME_None, StopWhenOwnerIsDestroyed ? TRUE : FALSE );
		if( Component )
		{
			Component->Stop();
		}
	}
	m_bStarted = FALSE;
}

/**
 * DISHONORED(port): 2013 rva 0x5ae910. An unregistered ambient sound with an event registers itself with the
 * world's audio system, which decides from the listener's cell when StartEvent actually fires; an already
 * registered one only clears m_bEnded and m_bStarted so the system starts it again.
 */
void AAkAmbientSound::StartPlayback()
{
	if( !m_bRegistered && PlayEvent )
	{
		m_bEnded = 0;
		if( GWorld && GWorld->m_pAudioSystem )
		{
			GWorld->m_pAudioSystem->RegisterAmbientSound( this );
			m_bRegistered = TRUE;
		}
		return;
	}
	if( m_bRegistered && m_bEnded )
	{
		m_bStarted = FALSE;
		m_bEnded = 0;
	}
}

// DISHONORED(port): 2013 rva 0x5ae970
void AAkAmbientSound::StopPlayback()
{
	if( m_bRegistered )
	{
		if( GWorld && GWorld->m_pAudioSystem )
		{
			GWorld->m_pAudioSystem->UnregisterAmbientSound( this );
		}
		m_bRegistered = FALSE;
	}
}

// DISHONORED(port): 2013 rva 0x5aea60: a movable ambient sound does not wait for a Kismet action
void AAkAmbientSoundMovable::PostBeginPlay()
{
	Super::PostBeginPlay();
	if( bPlayOnStartAll )
	{
		StartPlayback();
	}
}

/*-----------------------------------------------------------------------------
	Kismet actions
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): 2013 rva 0x5b08e0. Input 0 loads, input 1 unloads; the Async flag picks the callback
 * form, and a synchronous call (or a failed asynchronous start) raises Signal immediately.
 */
void USeqAct_AkLoadBank::Activated()
{
	if( !Bank )
	{
		return;
	}
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || !GEngine || !GEngine->bUseSound )
	{
		Signal = 1;
		return;
	}
	if( InputLinks.Num() > 0 && InputLinks(0).bHasImpulse )
	{
		if( Async )
		{
			bWaitingCallback = TRUE;
			if( Bank->LoadAsync( (void*)AkBankCallback, this ) )
			{
				return;
			}
			bWaitingCallback = FALSE;
		}
		else
		{
			Bank->Load();
		}
		Signal = 1;
	}
	else if( InputLinks.Num() > 1 && InputLinks(1).bHasImpulse )
	{
		if( Async )
		{
			bWaitingCallback = TRUE;
			Bank->UnloadAsync( (void*)AkBankCallback, this );
			return;
		}
		Bank->Unload();
		Signal = 1;
	}
}

// DISHONORED(port): 2013 rva 0x5ae7f0: latent until the callback arrived
UBOOL USeqAct_AkLoadBank::UpdateOp( FLOAT /*DeltaTime*/ )
{
	return Signal != 0;
}

// DISHONORED(port): 2013 rva 0x5ae810: a bank action that dies while waiting must not be called back
void USeqAct_AkLoadBank::BeginDestroy()
{
	if( bWaitingCallback )
	{
		AK::SoundEngine::CancelBankCallbackCookie( this );
		bWaitingCallback = FALSE;
	}
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x5b3810
void USeqAct_AkPostEvent::Activated()
{
	if( Event && InputLinks.Num() > 0 && InputLinks(0).bHasImpulse )
	{
		PlayEventOnTargets();
	}
}

/**
 * DISHONORED(port): 2013 rva 0x5b31b0. Signal counts the events still playing and m_pAudioSystem remembers
 * the world's audio system for the duration; each target gets the event with AK_EndOfEvent and AkCallback so
 * the count comes back down.
 */
void USeqAct_AkPostEvent::PlayEventOnTargets()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	Signal = 0;
	m_pAudioSystem = GWorld ? GWorld->m_pAudioSystem : NULL;
	if( !AudioDevice || !Event )
	{
		return;
	}
	TArray<UObject**> Targets;
	GetObjectVars( Targets, TEXT("Target") );
	const FLOAT MaxRadius = Event->GetMaxRadius();
	for( INT TargetIndex = 0; TargetIndex < Targets.Num(); ++TargetIndex )
	{
		AActor* Target = Cast<AActor>( *Targets( TargetIndex ) );
		if( AudioDevice->PostEvent( Event->m_akID, MaxRadius, Target, NAME_None, AK_EndOfEvent, AkCallback, this, FALSE ) != AK_INVALID_PLAYING_ID )
		{
			++Signal;
		}
	}
}

// DISHONORED(port): 2013 rva 0x5b3860
UBOOL USeqAct_AkPostEvent::UpdateOp( FLOAT /*DeltaTime*/ )
{
	return Signal == 0;
}

// DISHONORED(port): 2013 rva 0x5afac0
void USeqAct_AkPostEvent::FinishDestroy()
{
	if( Signal != 0 )
	{
		AK::SoundEngine::CancelEventCallbackCookie( this );
		Signal = 0;
	}
	Super::FinishDestroy();
}

// DISHONORED(port): 2013 rva 0x5b25a0
void USeqAct_AkPostTrigger::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || Trigger.Len() == 0 )
	{
		return;
	}
	TArray<UObject**> Targets;
	GetObjectVars( Targets, TEXT("Target") );
	if( Targets.Num() == 0 )
	{
		AudioDevice->PostTrigger( *Trigger, NULL, NAME_None );
		return;
	}
	for( INT TargetIndex = 0; TargetIndex < Targets.Num(); ++TargetIndex )
	{
		AudioDevice->PostTrigger( *Trigger, Cast<AActor>( *Targets( TargetIndex ) ), NAME_None );
	}
}

// DISHONORED(port): 2013 rva 0x5b2730
void USeqAct_AkSetRTPCValue::SetRTPCValue()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || Param.Len() == 0 )
	{
		return;
	}
	TArray<UObject**> Targets;
	GetObjectVars( Targets, TEXT("Target") );
	if( Targets.Num() == 0 )
	{
		AudioDevice->SetRTPCValue( *Param, Value, NULL, NAME_None );
		return;
	}
	for( INT TargetIndex = 0; TargetIndex < Targets.Num(); ++TargetIndex )
	{
		AudioDevice->SetRTPCValue( *Param, Value, Cast<AActor>( *Targets( TargetIndex ) ), NAME_None );
	}
}

// DISHONORED(port): 2013 rvas 0x5b0a10 / 0x5b33d0: the value is pushed once on activation and again on every
// update tick, because the RTPC is usually driven by a matinee-animated variable
void USeqAct_AkSetRTPCValue::Activated()
{
	SetRTPCValue();
}

UBOOL USeqAct_AkSetRTPCValue::UpdateOp( FLOAT /*DeltaTime*/ )
{
	SetRTPCValue();
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x5afb10
void USeqAct_AkSetState::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice && StateGroup.Len() > 0 && State.Len() > 0 )
	{
		AudioDevice->SetState( *StateGroup, *State );
	}
}

// DISHONORED(port): 2013 rva 0x5b2920: no target means the global game object
void USeqAct_AkSetSwitch::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || SwitchGroup.Len() == 0 || Switch.Len() == 0 )
	{
		return;
	}
	TArray<UObject**> Targets;
	GetObjectVars( Targets, TEXT("Target") );
	if( Targets.Num() == 0 )
	{
		AudioDevice->SetSwitch( *SwitchGroup, *Switch, NULL, NAME_None );
		return;
	}
	for( INT TargetIndex = 0; TargetIndex < Targets.Num(); ++TargetIndex )
	{
		AudioDevice->SetSwitch( *SwitchGroup, *Switch, Cast<AActor>( *Targets( TargetIndex ) ), NAME_None );
	}
}

/**
 * DISHONORED(port): 2013 rva 0x5b1ce0. Input 0 starts every AAkAmbientSound of the current scene that has
 * bPlayOnStartAll, input 1 stops them; the scene check keeps a streamed-out level's sounds out of it.
 */
void USeqAct_AkStartAmbientSound::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( !AudioDevice || !GWorld )
	{
		return;
	}
	const UBOOL bStart = InputLinks.Num() > 0 && InputLinks(0).bHasImpulse;
	for( FActorIterator It; It; ++It )
	{
		AAkAmbientSound* AmbientSound = Cast<AAkAmbientSound>( *It );
		if( !AmbientSound || !AmbientSound->bPlayOnStartAll )
		{
			continue;
		}
		UAkComponent* Component = AudioDevice->GetAkComponent( AmbientSound, NAME_None, AmbientSound->StopWhenOwnerIsDestroyed ? TRUE : FALSE );
		if( !Component || Component->GetAkScene() != GWorld->Scene )
		{
			continue;
		}
		if( bStart )
		{
			AmbientSound->StartPlayback();
		}
		else
		{
			AmbientSound->StopPlayback();
		}
	}
}

// DISHONORED(port): 2013 rva 0x5b44f0
void USeqAct_AkStopAll::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->StopAllSounds( FALSE );
	}
}

// DISHONORED(port): 2013 rva 0x5b3180
void USeqAct_AkClearBanks::Activated()
{
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		AudioDevice->ClearBanks();
	}
}

#endif // DISHONORED_WITH_WWISE
