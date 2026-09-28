// DishonoredGame/src/disaisubstatetakeactorposition.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateTakeActorPosition and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateTakeActorPosition_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7846e0: the base is constructed with the actor's position NOW, which is the snapshot the
// loco desire starts from; the actor itself is kept so RefreshSubState can follow it.
FDisAISubStateTakeActorPosition_Param::FDisAISubStateTakeActorPosition_Param( AActor* _pDestinationActor, BYTE _StopType, UBOOL _bFullSpeed, FLOAT _fMinAllowedDistanceFromActor )
	: FDisAISubStateTakePosition_Param( _pDestinationActor ? _pDestinationActor->Location : FVector( 0.f, 0.f, 0.f ), _StopType, _bFullSpeed )
	, m_pDestinationActor( _pDestinationActor )
	, m_fMinAllowedDistanceFromActor( _fMinAllowedDistanceFromActor )
{
	m_pStateClass = UDisAISubStateTakeActorPosition::StaticClass();
}

// DISHONORED(port): 2012 rva 0x77bee0
void FDisAISubStateTakeActorPosition_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubStateTakePosition_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateTakeActorPosition* TakeActorPosition = (UDisAISubStateTakeActorPosition*)PendingState;
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( TakeActorPosition->m_pDestinationActor && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, TakeActorPosition, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
	}
	TakeActorPosition->m_pDestinationActor = m_pDestinationActor;
	if( TakeActorPosition->m_pDestinationActor && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, TakeActorPosition, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
	}
	TakeActorPosition->m_fMinAllowedDistanceFromActor = m_fMinAllowedDistanceFromActor;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateTakeActorPosition
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x77bf00
void UDisAISubStateTakeActorPosition::SetDestinationActor( AActor* _pDestinationActor )
{
	if( m_pDestinationActor == _pDestinationActor )
	{
		return;
	}
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( m_pDestinationActor && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
	}
	m_pDestinationActor = _pDestinationActor;
	if( m_pDestinationActor && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, this, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
	}
}

// DISHONORED(port): 2012 rva 0x78c960: the destination follows the actor, stopping short by m_fMinAllowedDistanceFromActor
// along the line from the pawn, and the loco desire is re-stated whenever that moves. Then the base's own refresh runs, so
// the rotation-by-distance rule still applies.
void UDisAISubStateTakeActorPosition::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( m_pDestinationActor && Pawn )
	{
		FVector Destination = m_pDestinationActor->Location;
		if( m_fMinAllowedDistanceFromActor > 0.f )
		{
			const FVector FromActor = Pawn->Location - Destination;
			const FLOAT Distance = FromActor.Size();
			if( Distance > KINDA_SMALL_NUMBER )
			{
				Destination += FromActor * ( ::Min( m_fMinAllowedDistanceFromActor, Distance ) / Distance );
			}
		}
		if( m_lrDestination.m_Loc != Destination )
		{
			m_lrDestination.m_Loc = Destination;
			BeginSubState_Derived();
		}
	}
	Super::RefreshSubState( TimeSinceLastThought );
}

// DISHONORED(port): 2012 rva 0x775cc0
void UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_pDestinationActor )
	{
		FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
		if( Dispatcher )
		{
			const INT EventType = GDisAIEvent_OtherActorTerminated;
			Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
		}
		m_pDestinationActor = NULL;
	}
}

// DISHONORED(port): 2012 rva 0x77bf60
void UDisAISubStateTakeActorPosition::BeginDestroy()
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( m_pDestinationActor && Dispatcher )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateTakeActorPosition::OnOtherActorTerminatedEvent );
	}
	Super::BeginDestroy();
}
