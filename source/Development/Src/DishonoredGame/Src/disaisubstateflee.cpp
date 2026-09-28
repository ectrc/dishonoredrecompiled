// DishonoredGame/src/disaisubstateflee.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateFlee and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"
#include "dishonoredutilities.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateFlee_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x781c80
FDisAISubStateFlee_Param::FDisAISubStateFlee_Param( AActor* _pThreat, UBOOL _bForReal )
	: FDisAISubState_Param( UDisAISubStateFlee::StaticClass() )
	, m_pThreat( _pThreat )
	, m_bForReal( _bForReal )
{
}

// DISHONORED(port): 2012 rva 0x78c810. The only OnPending of the 25 that calls a METHOD of its sub-state rather than writing
// members, because setting the threat also moves the actor-termination subscription - see SetThreat.
void FDisAISubStateFlee_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );

	UDisAISubStateFlee* Flee = (UDisAISubStateFlee*)PendingState;
	Flee->SetThreat( m_pThreat );
	Flee->m_bForReal = m_bForReal ? 1 : 0;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateFlee
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x765480
const BYTE* UDisAISubStateFlee::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_DestinationReached };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

FDisStimPredicateDelegate UDisAISubStateFlee::GetFilterStimDelegate_SubState( BYTE StimID )
{
	if( StimID == EAIStimID_DestinationReached )
	{
		return DIS_BIND_STIM_PREDICATE( UDisAISubStateFlee, FAIStimStruct_DestinationReached, FilterDestinationReached );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x78b440: the threat is an actor the sub-state outlives, so changing it moves the termination
// subscription with it. This is the reason FDisAISubStateFlee_Param::OnPending calls a method.
void UDisAISubStateFlee::SetThreat( AActor* _pThreat )
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( m_pThreat && Dispatcher )
	{
		Dispatcher->UnregisterToEvent( EventType, this, &UDisAISubStateFlee::OnOtherActorTerminatedEvent );
	}
	m_pThreat = _pThreat;
	if( m_pThreat && Dispatcher )
	{
		Dispatcher->RegisterToEvent( EventType, this, &UDisAISubStateFlee::OnOtherActorTerminatedEvent );
	}
}

// DISHONORED(port): 2012 rva 0x773870: the eighth delegate, registered by the behaviour that uses it, same name
// composition as the six base callbacks.
void UDisAISubStateFlee::RegisterDelegate_ThreatTerminated( UDishonoredAIBehavior* const _pOwningBehavior )
{
	const FString FunctionName = FString( TEXT("ThreatTerminatedCallback") ) + m_StateSuffix.GetNameString();
	const FName TargetFunction( *FunctionName, FNAME_Add, TRUE );
	if( _pOwningBehavior && _pOwningBehavior->FindFunction( TargetFunction ) )
	{
		__ThreatTerminatedCallback__Delegate.Object = _pOwningBehavior;
		__ThreatTerminatedCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__ThreatTerminatedCallback__Delegate.Object = NULL;
		__ThreatTerminatedCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2012 rva 0x773820-region
void UDisAISubStateFlee::OnOtherActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_pThreat )
	{
		SetThreat( NULL );
		delegateThreatTerminatedCallback( this );
	}
}

// DISHONORED(port): 2012 rva 0x78b4b0. A flee that cannot find anywhere to run raises a PathingFail stim against itself,
// which is how the panic behaviour learns to make the NPC cower instead.
// DISHONORED(bringup): FindNewFleeDestination is the nav-mesh half - retail asks UDisFleeComponent for a flee point and
// then DisComputeNearestNavMeshLocFromLocation to put it on the mesh, neither ported (agentCG.md hand-over 3). So the
// destination search always fails, the stim is raised, and the behaviour above takes its "nowhere to run" branch, which is
// retail's own behaviour for a cornered civilian.
UBOOL UDisAISubStateFlee::FindNewFleeDestination()
{
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisFleeComponent and the nav-mesh point query are not ported; a fleeing NPC finds nowhere to run") );
	}
	return FALSE;
}

void UDisAISubStateFlee::BeginSubState_Derived()
{
	m_bFleeHasStartedMoving = FALSE;
	if( !FindNewFleeDestination() )
	{
		FAIStimStruct_PathingFail Stim = DisMakeStim< FAIStimStruct_PathingFail >( EAIStimID_PathingFail );
		Stim.m_pRequestOriginator = this;
		DisHandleAIStim( m_pOwningBrain, Stim, this );
	}
}

// DISHONORED(port): 2012 rva 0x768c30: arriving at a flee point tells the component so the next NPC does not pick the same
// one, and ends the state.
UBOOL UDisAISubStateFlee::FilterDestinationReached( const FAIStimStruct_DestinationReached& _rStim )
{
	const UBOOL bMine = ( _rStim.m_pRequestOriginator == this );
	if( bMine )
	{
		if( m_bForReal && m_pCurrentFleeComponent )
		{
			// DISHONORED(bringup): UDisFleeComponent::OnReached, unported.
		}
		RequestStateExit();
	}
	return bMine;
}

void UDisAISubStateFlee::OnReachedFleePoint()
{
	RequestStateExit();
}

// DISHONORED(port): 2012 rva 0x768b70: the one place in the AI that watches the pawn's own velocity rather than a stim -
// a flee that has started moving and then stopped has arrived, whatever the loco layer says.
void UDisAISubStateFlee::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );

	if( !m_bForReal )
	{
		return;
	}
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	const UBOOL bMoving = Pawn ? ( Pawn->Velocity.SizeSquared() > KINDA_SMALL_NUMBER ) : FALSE;
	if( m_bFleeHasStartedMoving )
	{
		if( !bMoving )
		{
			OnReachedFleePoint();
		}
	}
	else
	{
		m_bFleeHasStartedMoving = bMoving;
	}
}

void UDisAISubStateFlee::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
}

// DISHONORED(port): 2012 rva 0x773800-region
void UDisAISubStateFlee::BeginDestroy()
{
	if( m_pThreat )
	{
		SetThreat( NULL );
	}
	Super::BeginDestroy();
}
