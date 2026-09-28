// DishonoredGame/src/disaisubstatetakeposition.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateTakePosition and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateTakePosition_Param

	The three constructors are the three rotation contracts, and m_eTakePosRotationTarget in OnPending is how the
	sub-state tells them apart afterwards: None (do not turn at all), ByAngle (turn to a yaw) or ByActor (turn to face
	an actor).
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7821e0
FDisAISubStateTakePosition_Param::FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed )
	: FDisAISubState_Param( UDisAISubStateTakePosition::StaticClass() )
	, m_vDestination( _vDestination )
	, m_pFocusTarget( NULL )
	, m_iRotationTargetAngle( INDEX_NONE )
	, m_StopType( _StopType )
	, m_bRotationSetByAngle( FALSE )
	, m_bFullSpeed( _bFullSpeed )
	, m_bExactRotation( FALSE )
{
}

// DISHONORED(port): 2012 rva 0x782260
FDisAISubStateTakePosition_Param::FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed, AActor* const _pFocusTarget, UBOOL _bExactRotation )
	: FDisAISubState_Param( UDisAISubStateTakePosition::StaticClass() )
	, m_vDestination( _vDestination )
	, m_pFocusTarget( _pFocusTarget )
	, m_iRotationTargetAngle( INDEX_NONE )
	, m_StopType( _StopType )
	, m_bRotationSetByAngle( FALSE )
	, m_bFullSpeed( _bFullSpeed )
	, m_bExactRotation( _bExactRotation )
{
}

// DISHONORED(port): 2012 rva 0x7822f0: only the yaw of the rotator is kept - an NPC's facing is a yaw, and the pitch and
// roll a caller passes are dropped here rather than later.
FDisAISubStateTakePosition_Param::FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed, FRotator _RotationTargetAngle, UBOOL _bExactRotation )
	: FDisAISubState_Param( UDisAISubStateTakePosition::StaticClass() )
	, m_vDestination( _vDestination )
	, m_pFocusTarget( NULL )
	, m_iRotationTargetAngle( _RotationTargetAngle.Yaw )
	, m_StopType( _StopType )
	, m_bRotationSetByAngle( TRUE )
	, m_bFullSpeed( _bFullSpeed )
	, m_bExactRotation( _bExactRotation )
{
}

// DISHONORED(port): 2012 rva 0x777980. The destination goes into m_lrDestination.m_Loc and the target angle into its
// m_Rot.Yaw, with pitch and roll zeroed; the focus target's termination subscription is moved here rather than in
// BeginSubState, because OnPending runs before the state is entered and the sub-state must not outlive the actor.
void FDisAISubStateTakePosition_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateTakePosition* TakePosition = (UDisAISubStateTakePosition*)PendingState;
	TakePosition->m_bFullSpeed = m_bFullSpeed ? 1 : 0;
	TakePosition->m_lrDestination.m_Loc = m_vDestination;
	TakePosition->m_lrDestination.m_Rot.Pitch = 0;
	TakePosition->m_lrDestination.m_Rot.Yaw = m_iRotationTargetAngle;
	TakePosition->m_lrDestination.m_Rot.Roll = 0;

	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( TakePosition->m_pFocusTarget && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, TakePosition, &UDisAISubStateTakePosition::OnOtherActorTerminatedEvent );
	}
	TakePosition->m_pFocusTarget = m_pFocusTarget;
	if( TakePosition->m_pFocusTarget && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, TakePosition, &UDisAISubStateTakePosition::OnOtherActorTerminatedEvent );
	}

	TakePosition->m_eTakePosRotationTarget = m_bRotationSetByAngle ? EDisTakePosRotationTarget_ByAngle
		: ( TakePosition->m_pFocusTarget ? EDisTakePosRotationTarget_ByActor : EDisTakePosRotationTarget_None );
	TakePosition->m_StopType = m_StopType;
	TakePosition->m_bExactRotation = m_bExactRotation ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateTakePosition
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765920. Three stims, three bits in the 2012 mask body.
const BYTE* UDisAISubStateTakePosition::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached, EAIStimID_PathingFail, EAIStimID_RotationReached };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x78cc10
FDisStimPredicateDelegate UDisAISubStateTakePosition::GetFilterStimDelegate_SubState( BYTE StimID )
{
	switch( StimID )
	{
	case EAIStimID_DestinationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateTakePosition, FAIStimStruct_DestinationReached, FilterDestinationReached );
	case EAIStimID_PathingFail:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateTakePosition, FAIStimStruct_PathingFail, FilterPathingFail );
	case EAIStimID_RotationReached:
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateTakePosition, FAIStimStruct_RotationReached, FilterRotationReached );
	default:
		return FDisStimPredicateDelegate();
	}
}

// DISHONORED(port): 2012 rva 0x78ba40. One loco desire, the three progress bits cleared, and - only when the slot's tweaks
// say the rotation starts at once - the facing stated straight away.
void UDisAISubStateTakePosition::BeginSubState_Derived()
{
	const BYTE TransitSpeed = m_bFullSpeed ? ETransitSpeed_Run : ETransitSpeed_Walk;

	const UDisTweaks_AISubState_TakePosition* Tweaks = Cast<UDisTweaks_AISubState_TakePosition>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_TakePosition*)UDisTweaks_AISubState_TakePosition::StaticClass()->GetDefaultObject();
	}

	SetLocoLocationDesire( m_lrDestination.m_Loc, TransitSpeed, Tweaks->m_fDestinationThreshold, 1.f,
		m_StopType != DisTakePositionStopType_Anim, Tweaks->m_bSpeedIsLookAtDependent != 0 );

	m_bDestinationReached = FALSE;
	m_bRotationReached = FALSE;
	m_bRotationFocusSet = FALSE;

	if( Tweaks->m_eRotationStarts == EDisRotationStarts_OnStateBegin )
	{
		StartRotation();
	}
}

// DISHONORED(port): 2012 rva 0x788650. The rotation half, stated once: a yaw target becomes a face-to-yaw desire, an actor
// target becomes a look-at AND a face-to so the head leads the body.
void UDisAISubStateTakePosition::StartRotation()
{
	GetTweaks_Derived();

	if( m_eTakePosRotationTarget == EDisTakePosRotationTarget_ByAngle )
	{
		SetFaceToYawDesire( m_lrDestination.m_Rot.Yaw, -100.f, m_bExactRotation != 0 );
		m_bRotationFocusSet = TRUE;
	}
	else if( m_eTakePosRotationTarget == EDisTakePosRotationTarget_ByActor )
	{
		SetLookAtActorDesire( m_pFocusTarget, FDisLookAtInfluence::Torso, -1.f );
		SetFaceToActorDesire( m_pFocusTarget, -100.f, m_bExactRotation != 0 );
		m_bRotationFocusSet = TRUE;
	}
}

// DISHONORED(port): 2012 rva 0x788720. The thinking half: when the slot says "start turning once you are within this far",
// the distance is compared against the row of m_fRotationStartDistance for the speed the pawn is actually moving at.
void UDisAISubStateTakePosition::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );

	if( m_bRotationFocusSet )
	{
		return;
	}

	const UDisTweaks_AISubState_TakePosition* Tweaks = Cast<UDisTweaks_AISubState_TakePosition>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubState_TakePosition*)UDisTweaks_AISubState_TakePosition::StaticClass()->GetDefaultObject();
	}
	if( Tweaks->m_eRotationStarts != EDisRotationStarts_ByDistanceToTarget )
	{
		return;
	}

	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}
	const FVector ToDestination = m_lrDestination.m_Loc - Pawn->Location;
	const BYTE TransitSpeed = m_pOwningBrain->GetCurrentDesiredTransitSpeed();
	const FLOAT StartDistance = Tweaks->m_fRotationStartDistance[ TransitSpeed ];
	if( StartDistance == 0.f || ( StartDistance * StartDistance ) > ToDestination.SizeSquared() )
	{
		StartRotation();
	}
}

// DISHONORED(port): 2012 rva 0x7695e0: the state leaves itself once the destination is reached and, when the caller asked
// for a yaw, once the rotation is reached too. This is the transition that makes a patrol step forward.
void UDisAISubStateTakePosition::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( m_bDestinationReached && ( m_eTakePosRotationTarget != EDisTakePosRotationTarget_ByAngle || m_bRotationReached ) )
	{
		RequestStateExit();
	}
}

// DISHONORED(port): 2012 rva 0x788860: the stim is only ours if our own loco request raised it, and it only counts as
// arrival if it was the exact one or the slot never meant to stop.
UBOOL UDisAISubStateTakePosition::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine && ( _rStim.m_bExact || m_StopType == DisTakePositionStopType_Continuous ) )
	{
		if( !m_bRotationFocusSet )
		{
			const UDisTweaks_AISubState_TakePosition* Tweaks = Cast<UDisTweaks_AISubState_TakePosition>( GetTweaks_Derived() );
			if( !Tweaks )
			{
				Tweaks = (const UDisTweaks_AISubState_TakePosition*)UDisTweaks_AISubState_TakePosition::StaticClass()->GetDefaultObject();
			}
			if( Tweaks->m_eRotationStarts == EDisRotationStarts_AtDestination )
			{
				StartRotation();
			}
		}
		m_bDestinationReached = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x765960
UBOOL UDisAISubStateTakePosition::FilterRotationReached( const FAIStimStruct_RotationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine && m_bRotationFocusSet )
	{
		m_bRotationReached = TRUE;
	}
	return bMine;
}

// DISHONORED(port): 2012 rva 0x771130.
// DISHONORED(bringup): an NPC that cannot path to its destination tries to teleport there instead - retail calls the free
// function AttemptTeleportSpell (2012 rva 0x832a50, 228 bytes), which belongs to the powers package and is not ported. It
// is what makes an assassin appear behind you when the nav mesh will not take it. Nothing else happens on a pathing
// failure, so this sub-state simply keeps waiting; its behaviour's RefreshCallback is what gives up.
UBOOL UDisAISubStateTakePosition::FilterPathingFail( const FAIStimStruct_PathingFail& _rStim )
{
	if( _rStim.m_pRequestOriginator == this )
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): AttemptTeleportSpell is not ported; an NPC that cannot path to its position does not teleport") );
		}
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x765990 / 0x7659a0 / 0x769620
UBOOL UDisAISubStateTakePosition::DestinationReached() const
{
	return m_bDestinationReached != 0;
}

// DISHONORED(port): changing the speed re-states the loco desire, which is why it calls the whole of BeginSubState_Derived
// rather than touching the request.
void UDisAISubStateTakePosition::SetFullSpeed( UBOOL _bFullSpeed )
{
	const UBOOL bChanged = ( ( m_bFullSpeed != 0 ) != ( _bFullSpeed != 0 ) );
	m_bFullSpeed = _bFullSpeed ? 1 : 0;
	if( bChanged )
	{
		BeginSubState_Derived();
	}
}

FVector UDisAISubStateTakePosition::GetDestinationTarget() const
{
	return m_lrDestination.m_Loc;
}

// DISHONORED(port): 2012 rva 0x775e10
void UDisAISubStateTakePosition::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_pFocusTarget )
	{
		FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
		if( Dispatcher )
		{
			const INT EventType = GDisAIEvent_OtherActorTerminated;
			Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateTakePosition::OnOtherActorTerminatedEvent );
		}
		m_pFocusTarget = NULL;
	}
}

// DISHONORED(port): 2012 rva 0x777ac0: two subscriptions to drop - the focus target's, and the action target's, which the
// base class registered.
void UDisAISubStateTakePosition::BeginDestroy()
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( Dispatcher )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		if( m_pFocusTarget )
		{
			Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateTakePosition::OnOtherActorTerminatedEvent );
		}
		if( m_ActionTargetProxy.HasActorReference() )
		{
			Dispatcher->UnregisterToEvent( EventType, (UDisAISubState*)this, &UDisAISubState::OnOtherActorTerminatedEvent );
		}
	}
	Super::BeginDestroy();
}
