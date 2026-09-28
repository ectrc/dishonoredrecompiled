// DishonoredGame/src/disdesiresinterface.cpp
// ---- agent DF ports (PHASE10 DF): IDisDesiresInterface ----
//
// The AI's voice. A behaviour, a sub-state or a sub-process never moves or turns a pawn; it says what it wants and this
// interface routes that wish to the matching FDis*Request on the implementing class. Which request that is, and at what
// priority, is decided entirely by virtual accessors, which is how the same four verbs serve three levels of the AI at
// once and how a sub-state's wish overrides its behaviour's.
//
// This is the second of the three blockers agent CG measured (~23 of the remaining natives) and the reason the tree is
// full of DisAINoteDesiresGap notes: without it CallTickBehavior, CallRefreshThoughts and every sub-state transition had
// nowhere to put the desires step.
//
// DISHONORED(bringup): the Ark components the requests ultimately drive (FArkComponentFaceTo, FArkComponentLocomotion,
// FArkComponentLookat) are not ported, so the four desires are resolved, filtered and counted but not executed - see the
// head of Src/disdesirestructs.cpp. Everything in this file is the faithful port.

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"

/*-----------------------------------------------------------------------------
	The life cycle
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8ba1f0 (2012 0x905330): each request that exists on this implementor is bound to the
// pawn's matching component at this implementor's own priority, and then immediately paused. Pausing is what makes
// InitializeDesires safe to call before the owner is running: the desires exist, hold their priority, and issue nothing
// until ResumeDesires.
void IDisDesiresInterface::InitializeDesires()
{
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	if( !OwningPawn )
	{
		return;
	}
	UObject* Asker = GetUObjectInterfaceDisDesiresInterface();

	if( FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest() )
	{
		FaceToRequest->Initialize( (FArkComponentFaceTo*)OwningPawn->m_pCpntFaceTo, Asker, GetDesiresFaceToPriority(), GetDesiresFaceToEventCallback() );
		FaceToRequest->PauseRequest();
	}
	if( FDisLocoRequest* LocoRequest = GetDesiresLocoRequest() )
	{
		LocoRequest->Initialize( (FArkComponentLocomotion*)OwningPawn->m_pCpntLocomotion, Asker, GetDesiresLocoPriority(), GetDesiresLocoEventCallback() );
		LocoRequest->PauseRequest();
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		LookAtRequest->Initialize( (FArkComponentLookat*)OwningPawn->m_pCpntLookat, Asker, GetDesiresLookAtPriority() );
		// DISHONORED(retail): the look-at is paused inline here rather than through PauseRequest, because Initialize has
		// only just cleared it: there is no live request to stop, so retail skips straight to raising m_bPaused.
		if( !LookAtRequest->m_bPaused )
		{
			LookAtRequest->SyncRequest();
			LookAtRequest->m_RequestID = INDEX_NONE;
			LookAtRequest->m_bPaused = TRUE;
		}
	}
	if( FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest() )
	{
		BodyIntentionRequest->Initialize( OwningPawn, GetDesiresBodyIntentionPriority() );
		if( !BodyIntentionRequest->m_bPaused )
		{
			BodyIntentionRequest->ClearIntention();
			BodyIntentionRequest->m_bDesired = FALSE;
			BodyIntentionRequest->m_bPaused = TRUE;
		}
	}
}

// DISHONORED(port): 2013 rva 0x8b22d0 (2012 0x8ffa30): the owner is finished with its desires but keeps its binding, so
// it can state new ones later.
void IDisDesiresInterface::StopDesires()
{
	if( FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest() )
	{
		FaceToRequest->ClearRequest();
	}
	if( FDisLocoRequest* LocoRequest = GetDesiresLocoRequest() )
	{
		LocoRequest->ClearRequest();
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		LookAtRequest->ClearRequest();
	}
	if( FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest() )
	{
		BodyIntentionRequest->ClearIntention();
	}
}

// DISHONORED(port): 2013 rva 0x8ba360 (2012 0x9054a0): the orders are withdrawn but the targets are kept, so ResumeDesires
// re-issues exactly the same ones. This is what a behaviour pushed off slot 0 does.
void IDisDesiresInterface::PauseDesires()
{
	if( FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest() )
	{
		FaceToRequest->PauseRequest();
	}
	if( FDisLocoRequest* LocoRequest = GetDesiresLocoRequest() )
	{
		LocoRequest->PauseRequest();
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		if( !LookAtRequest->m_bPaused )
		{
			LookAtRequest->SyncRequest();
			if( LookAtRequest->m_RequestID != INDEX_NONE )
			{
				DisDesireStructs::NoteComponentGap( DisDesireStructs::DDK_LookAt, TEXT("FArkComponentLookat::StopLookAt") );
				LookAtRequest->m_RequestID = INDEX_NONE;
			}
			LookAtRequest->m_bPaused = TRUE;
		}
	}
	if( FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest() )
	{
		if( !BodyIntentionRequest->m_bPaused )
		{
			if( BodyIntentionRequest->m_bDesired && BodyIntentionRequest->m_pNPCPawn )
			{
				BodyIntentionRequest->m_pNPCPawn->SetBodyIntention( BodyIntentionRequest->m_Priority, eDisNPCBodyStance_None, NULL, NULL );
			}
			BodyIntentionRequest->m_bPaused = TRUE;
		}
	}
}

// DISHONORED(port): 2013 rva 0x8bbf50 (2012 0x907ee0): un-pause each request and re-issue it. The body intention is the
// interesting one: the owner is asked what its stance should be GIVEN the one the pawn had while it was away
// (GetResumingBodyIntentionDesire), which is how a sub-state that was interrupted mid-action comes back holding the same
// weapon rather than resetting to an empty stance.
void IDisDesiresInterface::ResumeDesires( const FDisBodyIntention& _rPreviousBodyIntention )
{
	if( FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest() )
	{
		if( FaceToRequest->m_bPaused )
		{
			FaceToRequest->m_bPaused = FALSE;
			FaceToRequest->UpdateRequest();
		}
	}
	if( FDisLocoRequest* LocoRequest = GetDesiresLocoRequest() )
	{
		if( LocoRequest->m_bPaused )
		{
			LocoRequest->m_bPaused = FALSE;
			LocoRequest->UpdateRequest();
		}
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		if( LookAtRequest->m_bPaused )
		{
			LookAtRequest->m_bPaused = FALSE;
			LookAtRequest->SyncRequest();
			const EDisDesireRequestStatus Status = LookAtRequest->GetRequestStatus( LookAtRequest->m_pActorTarget, LookAtRequest->m_bUsingLocation, LookAtRequest->m_LocationTarget );
			if( Status != DTDRS_Unchanged )
			{
				LookAtRequest->DoRequest( Status );
			}
		}
	}

	FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest();
	if( !BodyIntentionRequest )
	{
		return;
	}
	FDisBodyIntention ResumingBodyIntention;
	ResumingBodyIntention.m_IntendedBodyStance = eDisNPCBodyStance_None;
	ResumingBodyIntention.m_pDesiredPrimaryItemClass = NULL;
	ResumingBodyIntention.m_pDesiredSecondaryItemClass = NULL;
	if( GetResumingBodyIntentionDesire( _rPreviousBodyIntention, ResumingBodyIntention ) )
	{
		if( FDisBodyIntentionRequest* Request = GetDesiresBodyIntentionRequest() )
		{
			Request->RequestBodyIntention( ResumingBodyIntention.m_IntendedBodyStance, ResumingBodyIntention.m_pDesiredPrimaryItemClass, ResumingBodyIntention.m_pDesiredSecondaryItemClass );
		}
	}
	if( BodyIntentionRequest->m_bPaused )
	{
		if( BodyIntentionRequest->m_bDesired && BodyIntentionRequest->m_pNPCPawn )
		{
			BodyIntentionRequest->m_pNPCPawn->SetBodyIntention( BodyIntentionRequest->m_Priority, BodyIntentionRequest->m_BodyStance,
				BodyIntentionRequest->m_pPrimaryItemClass, BodyIntentionRequest->m_pSecondaryItemClass );
		}
		BodyIntentionRequest->m_bPaused = FALSE;
	}
}

// DISHONORED(port): 2013 rva 0x8bc0e0 (2012 0x908070): a loaded save has the request state but not the component pointers,
// so each request is re-bound and, unless it was saved paused, re-issued.
void IDisDesiresInterface::PostGameLoad_Desires()
{
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	if( !OwningPawn )
	{
		return;
	}
	UObject* Asker = GetUObjectInterfaceDisDesiresInterface();

	if( FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest() )
	{
		const UBOOL bWasUnpaused = !FaceToRequest->m_bPaused;
		FaceToRequest->m_pFaceToComponent = OwningPawn->m_pCpntFaceTo;
		FaceToRequest->m_pAsker = Asker;
		FaceToRequest->m_pCallback = (FPointer)GetDesiresFaceToEventCallback();
		if( bWasUnpaused )
		{
			FaceToRequest->UpdateRequest();
		}
	}
	if( FDisLocoRequest* LocoRequest = GetDesiresLocoRequest() )
	{
		const UBOOL bWasUnpaused = !LocoRequest->m_bPaused;
		LocoRequest->m_pLocoComponent = OwningPawn->m_pCpntLocomotion;
		LocoRequest->m_pAsker = Asker;
		LocoRequest->m_pCallback = (FPointer)GetDesiresLocoEventCallback();
		if( bWasUnpaused )
		{
			LocoRequest->UpdateRequest();
		}
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		LookAtRequest->PostGameLoad( (FArkComponentLookat*)OwningPawn->m_pCpntLookat, Asker );
	}
	if( FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest() )
	{
		BodyIntentionRequest->m_pNPCPawn = OwningPawn;
		if( BodyIntentionRequest->m_bDesired && !BodyIntentionRequest->m_bPaused )
		{
			OwningPawn->SetBodyIntention( BodyIntentionRequest->m_Priority, BodyIntentionRequest->m_BodyStance,
				BodyIntentionRequest->m_pPrimaryItemClass, BodyIntentionRequest->m_pSecondaryItemClass );
		}
	}
}

// DISHONORED(port): 2013 rva 0x8bc080 (2012 0x908010): the face-to and loco desires only re-resolve their target while
// unpaused; the look-at ticks either way, because its duration runs down even when it is not being issued.
void IDisDesiresInterface::TickDesires( FLOAT _fDeltaTime )
{
	FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest();
	if( FaceToRequest && !FaceToRequest->m_bPaused )
	{
		FaceToRequest->UpdateRequest();
	}
	FDisLocoRequest* LocoRequest = GetDesiresLocoRequest();
	if( LocoRequest && !LocoRequest->m_bPaused )
	{
		LocoRequest->UpdateRequest();
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		LookAtRequest->TickRequest( _fDeltaTime );
	}
}

// DISHONORED(port): 2013 rva 0x8b3cb0 (2012 0x9002d0): the owner is going away for good - every request is cleared AND
// unbound, so the component keeps nothing pointing at a dead asker.
void IDisDesiresInterface::FinalizeDesires()
{
	FDisFaceToRequest* FaceToRequest = GetDesiresFaceToRequest();
	if( FaceToRequest && FaceToRequest->m_pFaceToComponent )
	{
		FaceToRequest->ClearRequest();
		FaceToRequest->m_bPaused = FALSE;
		FaceToRequest->m_pFaceToComponent = NULL;
		FaceToRequest->m_pAsker = NULL;
		FaceToRequest->m_Priority = 0;
		FaceToRequest->m_pCallback = NULL;
	}
	FDisLocoRequest* LocoRequest = GetDesiresLocoRequest();
	if( LocoRequest && LocoRequest->m_pLocoComponent )
	{
		LocoRequest->ClearRequest();
		LocoRequest->m_bPaused = FALSE;
		LocoRequest->m_pLocoComponent = NULL;
		LocoRequest->m_pAsker = NULL;
		LocoRequest->m_Priority = 0;
		LocoRequest->m_pCallback = NULL;
	}
	if( FDisLookAtRequest* LookAtRequest = GetDesiresLookAtRequest() )
	{
		LookAtRequest->Finalize();
	}
	FDisBodyIntentionRequest* BodyIntentionRequest = GetDesiresBodyIntentionRequest();
	if( BodyIntentionRequest && BodyIntentionRequest->m_pNPCPawn )
	{
		BodyIntentionRequest->ClearIntention();
		BodyIntentionRequest->m_bPaused = FALSE;
		BodyIntentionRequest->m_pNPCPawn = NULL;
		BodyIntentionRequest->m_Priority = 0;
	}
}

/*-----------------------------------------------------------------------------
	The face-to desires
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rvas 0x8ba3f0 / 0x8ba420 / 0x8ba450 / 0x8ba480 / 0x8b2350
void IDisDesiresInterface::SetFaceToProxyDesire( const FDisAttentionProxy& _rProxy, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	if( FDisFaceToRequest* Request = GetDesiresFaceToRequest() )
	{
		Request->RequestProxyTarget( _rProxy, _fRotationSpeed, _bExactRotation );
	}
}

void IDisDesiresInterface::SetFaceToActorDesire( AActor* _pActor, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	if( FDisFaceToRequest* Request = GetDesiresFaceToRequest() )
	{
		Request->RequestActorTarget( _pActor, _fRotationSpeed, _bExactRotation );
	}
}

void IDisDesiresInterface::SetFaceToLocationDesire( const FVector& _rLocation, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	if( FDisFaceToRequest* Request = GetDesiresFaceToRequest() )
	{
		Request->RequestLocationTarget( _rLocation, _fRotationSpeed, _bExactRotation );
	}
}

void IDisDesiresInterface::SetFaceToYawDesire( INT _Yaw, FLOAT _fRotationSpeed, UBOOL _bExactRotation )
{
	if( FDisFaceToRequest* Request = GetDesiresFaceToRequest() )
	{
		Request->RequestYawTarget( _Yaw, _fRotationSpeed, _bExactRotation );
	}
}

void IDisDesiresInterface::ClearFaceToDesire()
{
	if( FDisFaceToRequest* Request = GetDesiresFaceToRequest() )
	{
		Request->ClearRequest();
	}
}

/*-----------------------------------------------------------------------------
	The loco desires
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the ETransitSpeed -> locomotion speed index translation retail inlines into all four loco setters
 * (2013 rvas 0x8ba4b0 / 0x8ba540 / 0x8ba670 / 0x8ba5d0): an AI asks for Idle, Walk or Run and the pawn's own
 * UDisTweaks_NPCPawn::m_TransitSpeedToLocomotionSpeed[3] says which of its locomotion speeds that is, which is how a
 * wolfhound's "walk" is faster than a citizen's.
 */
static INT DisLocoSpeedIndexForTransitSpeed( IDisDesiresInterface* _pDesires, BYTE _TransitSpeed )
{
	ADishonoredNPCPawn* OwningPawn = _pDesires->GetDesiresOwningPawn();
	const UDisTweaks_NPCPawn* Tweaks = OwningPawn ? Cast<UDisTweaks_NPCPawn>( OwningPawn->GetTweaks_Derived() ) : NULL;
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	if( _TransitSpeed >= ARRAY_COUNT(Tweaks->m_TransitSpeedToLocomotionSpeed) )
	{
		return 0;
	}
	return Tweaks->m_TransitSpeedToLocomotionSpeed[_TransitSpeed];
}

// DISHONORED(port): 2013 rva 0x8ba4b0 (2012 0x9055f0)
void IDisDesiresInterface::SetLocoProxyDesire( const FDisAttentionProxy& _rProxy, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	const INT MaxSpeedIndex = DisLocoSpeedIndexForTransitSpeed( this, _TransitSpeed );
	if( FDisLocoRequest* Request = GetDesiresLocoRequest() )
	{
		Request->RequestProxyTarget( _rProxy, MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
	}
}

// DISHONORED(port): 2013 rva 0x8ba540 (2012 0x905680)
void IDisDesiresInterface::SetLocoActorDesire( AActor* _pActor, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	const INT MaxSpeedIndex = DisLocoSpeedIndexForTransitSpeed( this, _TransitSpeed );
	if( FDisLocoRequest* Request = GetDesiresLocoRequest() )
	{
		Request->RequestActorTarget( _pActor, MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
	}
}

// DISHONORED(port): 2013 rva 0x8ba670 (2012 0x9057b0)
void IDisDesiresInterface::SetLocoLocationDesire( const FVector& _rLocation, BYTE _TransitSpeed, FLOAT _fEndLocationThreshold, FLOAT _fMaxFunnelRadiusMultiplier, UBOOL _bAccurateStop, UBOOL _bSpeedIsLookAtDependent )
{
	const INT MaxSpeedIndex = DisLocoSpeedIndexForTransitSpeed( this, _TransitSpeed );
	if( FDisLocoRequest* Request = GetDesiresLocoRequest() )
	{
		Request->RequestLocationTarget( _rLocation, MaxSpeedIndex, _fEndLocationThreshold, _fMaxFunnelRadiusMultiplier, _bAccurateStop, _bSpeedIsLookAtDependent );
	}
}

// DISHONORED(port): 2013 rva 0x8ba5d0 (2012 0x905710): the four constants retail passes are the follow contract - never
// stop short, full funnel, accurate stop off, speed not look-at dependent, follow on.
void IDisDesiresInterface::SetLocoFollowDesire( AActor* _pActor, BYTE _TransitSpeed, FLOAT _fFollowAngle, FLOAT _fFollowDist )
{
	const INT MaxSpeedIndex = DisLocoSpeedIndexForTransitSpeed( this, _TransitSpeed );
	if( FDisLocoRequest* Request = GetDesiresLocoRequest() )
	{
		Request->RequestFollowTarget( _pActor, MaxSpeedIndex, -1.f, 1.f, TRUE, FALSE, TRUE, _fFollowAngle, _fFollowDist );
	}
}

void IDisDesiresInterface::ClearLocoDesire()
{
	if( FDisLocoRequest* Request = GetDesiresLocoRequest() )
	{
		Request->ClearRequest();
	}
}

// DISHONORED(port): 2013 rva 0x8adec0 (2012 0x8f9df0): "where am I going?" - the actor target's current position if there
// is one, else the recorded location, else where the pawn already stands (which is how a caller with no desire reads back
// "here" rather than the origin).
FVector IDisDesiresInterface::GetLocoDesireDestination()
{
	FDisLocoRequest* Request = GetDesiresLocoRequest();
	if( Request && Request->m_bDesired )
	{
		return Request->m_pActorTarget ? Request->m_pActorTarget->Location : Request->m_LocationTarget;
	}
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	return OwningPawn ? OwningPawn->Location : FVector( 0.f, 0.f, 0.f );
}

// DISHONORED(port): 2013 rva 0x8adf60 (2012 0x8fca90): the inverse translation of DisLocoSpeedIndexForTransitSpeed, through
// UDisTweaks_NPCPawn::m_LocomotionSpeedToTransitSpeed.
BYTE IDisDesiresInterface::GetLocoDesireTransitSpeed()
{
	FDisLocoRequest* Request = GetDesiresLocoRequest();
	if( !Request || !Request->m_bDesired )
	{
		return ETransitSpeed_Idle;
	}
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	const UDisTweaks_NPCPawn* Tweaks = OwningPawn ? Cast<UDisTweaks_NPCPawn>( OwningPawn->GetTweaks_Derived() ) : NULL;
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_NPCPawn*)UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject();
	}
	if( Request->m_MaxSpeedIndex < 0 || Request->m_MaxSpeedIndex >= Tweaks->m_LocomotionSpeedToTransitSpeed.Num() )
	{
		return ETransitSpeed_Idle;
	}
	return (BYTE)Tweaks->m_LocomotionSpeedToTransitSpeed( Request->m_MaxSpeedIndex );
}

/*-----------------------------------------------------------------------------
	The look-at and aim-at desires
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8ba700 (2012 0x905840)
void IDisDesiresInterface::SetLookAtProxyDesire( const FDisAttentionProxy& _rProxy, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	if( FDisLookAtRequest* Request = GetDesiresLookAtRequest() )
	{
		Request->RequestProxyTarget( _rProxy, _rLookAtInfluence, _fDuration );
	}
}

// DISHONORED(port): 2013 rva 0x8ba760 (2012 0x9058a0)
void IDisDesiresInterface::SetLookAtTargetDesire( IDisLookAtInterface* _pTarget, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	FDisLookAtRequest* Request = GetDesiresLookAtRequest();
	if( !Request )
	{
		return;
	}
	Request->SyncRequest();
	EDisDesireRequestStatus Status = Request->SetLookAtTarget( _pTarget );
	Request->SetParams( FALSE, FALSE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		Request->DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba7c0 (2012 0x905900)
void IDisDesiresInterface::SetLookAtActorDesire( AActor* _pActor, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	if( FDisLookAtRequest* Request = GetDesiresLookAtRequest() )
	{
		Request->RequestActorTarget( _pActor, _rLookAtInfluence, _fDuration );
	}
}

// DISHONORED(port): 2013 rva 0x8ba7f0 (2012 0x905930)
void IDisDesiresInterface::SetLookAtLocationDesire( const FVector& _rLocation, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	FDisLookAtRequest* Request = GetDesiresLookAtRequest();
	if( !Request )
	{
		return;
	}
	Request->SyncRequest();
	EDisDesireRequestStatus Status = Request->SetLocationTarget( _rLocation );
	Request->SetParams( FALSE, FALSE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		Request->DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba8e0 (2012 0x905a20)
void IDisDesiresInterface::SetLookAtProceduralDesire( BYTE _ProceduralPatternIndex, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	if( FDisLookAtRequest* Request = GetDesiresLookAtRequest() )
	{
		Request->RequestProceduralLookAt( _ProceduralPatternIndex, _rLookAtInfluence, _fDuration );
	}
}

// DISHONORED(port): 2013 rva 0x8ba910 (2012 0x905a50)
void IDisDesiresInterface::SetLookAtProceduralProxyDesire( const FDisAttentionProxy& _rProxy, BYTE _ProceduralPatternIndex, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	FDisLookAtRequest* Request = GetDesiresLookAtRequest();
	if( !Request )
	{
		return;
	}
	Request->SyncRequest();
	EDisDesireRequestStatus Status = Request->SetProxyTarget( _rProxy );
	Request->SetParams( FALSE, FALSE, _ProceduralPatternIndex, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		Request->DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba970 (2012 0x905ab0)
void IDisDesiresInterface::SetLookAtProceduralLocationDesire( const FVector& _rLocation, BYTE _ProceduralPatternIndex, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	FDisLookAtRequest* Request = GetDesiresLookAtRequest();
	if( !Request )
	{
		return;
	}
	Request->SyncRequest();
	EDisDesireRequestStatus Status = Request->SetLocationTarget( _rLocation );
	Request->SetParams( FALSE, FALSE, _ProceduralPatternIndex, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		Request->DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba850 (2012 0x905990): aiming is a look-at with the aim flag, which is what makes a
// pistol point where the head is looking.
void IDisDesiresInterface::SetAimAtProxyDesire( const FDisAttentionProxy& _rProxy, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	FDisLookAtRequest* Request = GetDesiresLookAtRequest();
	if( !Request )
	{
		return;
	}
	Request->SyncRequest();
	EDisDesireRequestStatus Status = Request->SetProxyTarget( _rProxy );
	Request->SetParams( FALSE, TRUE, DisLookAtProceduralPattern_MAX, _rLookAtInfluence, _fDuration, Status );
	if( Status != DTDRS_Unchanged )
	{
		Request->DoRequest( Status );
	}
}

// DISHONORED(port): 2013 rva 0x8ba8b0 (2012 0x9059f0)
void IDisDesiresInterface::SetAimAtActorDesire( AActor* _pActor, const FDisLookAtInfluence& _rLookAtInfluence, FLOAT _fDuration )
{
	if( FDisLookAtRequest* Request = GetDesiresLookAtRequest() )
	{
		Request->RequestActorAimTarget( _pActor, _rLookAtInfluence, _fDuration );
	}
}

void IDisDesiresInterface::ClearLookAtDesire()
{
	if( FDisLookAtRequest* Request = GetDesiresLookAtRequest() )
	{
		Request->ClearRequest();
	}
}

/*-----------------------------------------------------------------------------
	The body intention
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8a9a70 (2012 0x8f9e90)
UBOOL IDisDesiresInterface::SetBodyIntentionDesire( BYTE _BodyStance, UClass* _pPrimaryItemClass, UClass* _pSecondaryItemClass )
{
	if( FDisBodyIntentionRequest* Request = GetDesiresBodyIntentionRequest() )
	{
		return Request->RequestBodyIntention( _BodyStance, _pPrimaryItemClass, _pSecondaryItemClass );
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x8ae040 (2012 0x8fcb70)
void IDisDesiresInterface::ClearBodyIntentionDesire()
{
	if( FDisBodyIntentionRequest* Request = GetDesiresBodyIntentionRequest() )
	{
		Request->ClearIntention();
	}
}

/*-----------------------------------------------------------------------------
	What the components report back
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x8b7230 (2012 0x901a10): the component says it has turned; that becomes a RotationReached
// stim, which is how a sub-state waiting to finish rotating is told to move on. An aborted rotation is deliberately
// silent - the AI cancelled it itself.
void IDisDesiresInterface::HandleFaceToEvent( BYTE _FaceToEvent )
{
	FDisFaceToRequest* Request = GetDesiresFaceToRequest();
	if( !Request )
	{
		return;
	}
	Request->m_DebugLastFaceToEvent = _FaceToEvent;
	if( _FaceToEvent == CPNT_FACETO_EVENT_ABORTED )
	{
		return;
	}
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	UDishonoredAIBrain* Brain = OwningPawn ? OwningPawn->GetAIBrain() : NULL;
	if( !Brain )
	{
		return;
	}
	UObject* Asker = GetUObjectInterfaceDisDesiresInterface();
	const FVector RotationFocus = Request->m_pActorTarget ? Request->m_pActorTarget->Location : Request->m_LocationTarget;
	if( _FaceToEvent == CPNT_FACETO_EVENT_ORIENTATION_REACHED )
	{
		FAIStimStruct_RotationReached Stim = DisMakeStim< FAIStimStruct_RotationReached >( EAIStimID_RotationReached );
		Stim.m_RotationFocus = RotationFocus;
		Stim.m_pRequestOriginator = Asker;
		DisHandleAIStim( Brain, Stim, Asker );
	}
}

// DISHONORED(port): 2013 rva 0x8b7330 (2012 0x901b10): the component says it has arrived, or reached its threshold, or
// cannot get there; each becomes the matching stim. This is the whole feedback path from locomotion back into the
// behaviour stack - which is why the NPCs are currently silent on arrival.
void IDisDesiresInterface::HandleLocoEvent( BYTE _LocoEvent )
{
	FDisLocoRequest* Request = GetDesiresLocoRequest();
	if( !Request )
	{
		return;
	}
	Request->m_DebugLastLocoEvent = _LocoEvent;
	if( _LocoEvent == CPNT_LOCO_EVENT_ABORTED )
	{
		return;
	}
	ADishonoredNPCPawn* OwningPawn = GetDesiresOwningPawn();
	UDishonoredAIBrain* Brain = OwningPawn ? OwningPawn->GetAIBrain() : NULL;
	if( !Brain )
	{
		return;
	}
	UObject* Asker = GetUObjectInterfaceDisDesiresInterface();
	const FVector Location = Request->m_pActorTarget ? Request->m_pActorTarget->Location : Request->m_LocationTarget;
	switch( _LocoEvent )
	{
	case CPNT_LOCO_EVENT_PATHFINDING_SUCCEED:
		{
			FAIStimStruct_PathingSuccess Stim = DisMakeStim< FAIStimStruct_PathingSuccess >( EAIStimID_PathingSuccess );
			Stim.m_pRequestOriginator = Asker;
			DisHandleAIStim( Brain, Stim, Asker );
		}
		break;
	case CPNT_LOCO_EVENT_PATHFINDING_FAILED:
		{
			FAIStimStruct_PathingFail Stim = DisMakeStim< FAIStimStruct_PathingFail >( EAIStimID_PathingFail );
			Stim.m_pRequestOriginator = Asker;
			DisHandleAIStim( Brain, Stim, Asker );
		}
		break;
	case CPNT_LOCO_EVENT_THRESHOLD_REACHED:
	case CPNT_LOCO_EVENT_DESTINATION_REACHED:
		{
			// DISHONORED(retail): the two events raise the same stim and differ only in m_bExact - "I am within the
			// arrival threshold" against "I am standing on it", which is what an accurate-stop sub-state waits for.
			FAIStimStruct_DestinationReached Stim = DisMakeStim< FAIStimStruct_DestinationReached >( EAIStimID_DestinationReached );
			Stim.m_Location = Location;
			Stim.m_bExact = ( _LocoEvent == CPNT_LOCO_EVENT_DESTINATION_REACHED ) ? 1 : 0;
			Stim.m_pRequestOriginator = Asker;
			DisHandleAIStim( Brain, Stim, Asker );
		}
		break;
	default:
		break;
	}
}
