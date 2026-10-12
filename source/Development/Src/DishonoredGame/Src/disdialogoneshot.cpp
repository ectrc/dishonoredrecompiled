// DishonoredGame/src/disdialogoneshot.cpp
// DISHONORED(port): agent FR (PHASE17 FR). The level actor that owns a one-shot dialog tree. 2013 rvas (2012 in parentheses):
//   PostLoad 0x64c730 (0x6986d0, 382)      PostBeginPlay 0x659da0 (0x69d630, 16)     NotifyOneShotSpeakers (0x69d4f0, 317)
//   InitSpeaker 0x647600 (0x6948c0, 114)   InitDialogTreeInternal (0x698850, 201)     Spawned (0x691c40, 41)
//   GetOwnerForSpeaker (0x698920, 68)      GetOwnerActorForSpeaker(_Derived) (0x691c90 / 0x691ca0)
// PostLoad moves each speaker's m_pSpawner into m_pAttachedActor (the editor-time reference becomes the runtime one); the
// ValidateAndFixupFolders it then runs is editor-only. Not ported (agentFR.md inventory): DestroyForSpawners 0x69d640,
// GetOneShotName 0x698600 / FDisOneOffSpeaker::GetOneShotName 0x69d2e0, GetDisplayName 0x69d1b0, PostEditImport,
// PostScriptDestroyed, BeginDestroy, MarkComponentsAsPendingKill (the actor's teardown).
#include "DishonoredGame.h"
#include "frdialogdiag.h"

void ADisDialogOneShot::PostLoad()
{
	Super::PostLoad();
	if( m_MeSpeaker.m_pSpawner )
	{
		m_MeSpeaker.m_pAttachedActor = m_MeSpeaker.m_pSpawner;
		m_MeSpeaker.m_pSpawner = NULL;
	}
	for( INT i = 0; i < m_OtherSpeakers.Num(); i++ )
	{
		if( m_OtherSpeakers(i).m_pSpawner )
		{
			m_OtherSpeakers(i).m_pAttachedActor = m_OtherSpeakers(i).m_pSpawner;
			m_OtherSpeakers(i).m_pSpawner = NULL;
		}
	}
}

void ADisDialogOneShot::Spawned()
{
	Super::Spawned();
	if( GIsEditor && !GIsGame )
	{
		InitDialogTreeInternal();
	}
}

void ADisDialogOneShot::PostBeginPlay()
{
	Super::PostBeginPlay();
	NotifyOneShotSpeakers();
}

static UBOOL DisNotifyOneShotOwner( ADisDialogOneShot* OneShot, FDisOneOffSpeaker& Speaker, UBOOL bIsOwner )
{
	AActor* Actor = OneShot->GetOwnerActorForSpeaker( Speaker );
	IDisConvOneShotOwnerInterface* Owner = Actor ? (IDisConvOneShotOwnerInterface*)Actor->GetInterfaceAddress( UDisConvOneShotOwnerInterface::StaticClass() ) : NULL;
	if( !Owner || !Speaker.m_pVoiceData )
	{
		FRDIAG( "oneshot %s: speaker '%s' (%s) %s", *OneShot->GetName(), *Speaker.m_SpeakerNickName.ToString(), Actor ? *Actor->GetName() : TEXT("no actor"),
			Owner ? TEXT("has no voice data") : TEXT("is not a one-shot owner") );
		return FALSE;
	}
	Speaker.m_pVoiceData->InitForGame();
	Owner->AddDialogOneShot( Speaker.m_pVoiceData, bIsOwner );
	Speaker.m_bRemovedOneShots = 0;
	FRDIAG( "oneshot %s: speaker '%s' -> %s gets %s (%s)", *OneShot->GetName(), *Speaker.m_SpeakerNickName.ToString(), *Actor->GetName(), *Speaker.m_pVoiceData->GetName(), bIsOwner ? TEXT("owner") : TEXT("other") );
	return TRUE;
}

void ADisDialogOneShot::NotifyOneShotSpeakers()
{
	if( m_bNotifiedSpeakers )
	{
		return;
	}
	if( !DisNotifyOneShotOwner( this, m_MeSpeaker, TRUE ) )
	{
		return;
	}
	for( INT i = 0; i < m_OtherSpeakers.Num(); i++ )
	{
		DisNotifyOneShotOwner( this, m_OtherSpeakers(i), FALSE );
	}
	m_bNotifiedSpeakers = TRUE;
}

void ADisDialogOneShot::InitSpeaker( FDisOneOffSpeaker& _rSpeaker )
{
	if( !_rSpeaker.m_pVoiceData )
	{
		_rSpeaker.m_pVoiceData = ConstructObject<UDisDialogVoiceData_OneShot>( UDisDialogVoiceData_OneShot::StaticClass(), this );
	}
	_rSpeaker.m_pVoiceData->BindToOneShot( this );
}

void ADisDialogOneShot::InitDialogTreeInternal()
{
	if( !m_pDialogTree )
	{
		m_pDialogTree = ConstructObject<UDisDialogTree_OneShot>( UDisDialogTree_OneShot::StaticClass(), this );
	}
	m_pDialogTree->BindToOneShot( this );
	InitSpeaker( m_MeSpeaker );
	for( INT i = 0; i < m_OtherSpeakers.Num(); i++ )
	{
		InitSpeaker( m_OtherSpeakers(i) );
	}
}

IDisConvOneShotOwnerInterface* ADisDialogOneShot::GetOwnerForSpeaker( FDisOneOffSpeaker& _rSpeakerStruct )
{
	AActor* Actor = GetOwnerActorForSpeaker_Derived( _rSpeakerStruct );
	return Actor ? (IDisConvOneShotOwnerInterface*)Actor->GetInterfaceAddress( UDisConvOneShotOwnerInterface::StaticClass() ) : NULL;
}

AActor* ADisDialogOneShot::GetOwnerActorForSpeaker_Derived( FDisOneOffSpeaker& _rSpeakerStruct )
{
	return _rSpeakerStruct.m_pAttachedActor;
}
