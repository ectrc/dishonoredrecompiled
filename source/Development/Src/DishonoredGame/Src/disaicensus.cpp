// DishonoredGame/src/disaicensus.cpp
// DISHONORED(written): agent CG. Not a retail unit: this is the census that measures the AI brain package, the way
// agent AP's scene census, agent AS's -distouch and agent AU's -dispickup measure theirs. Nothing here runs unless
// -disai (report) or -disaiprobe (walk-in) is on the command line.
//
// -disai       once a second, per world, re-armed for each new world: how many NPC pawns, NPC controllers and
//              spawners the map holds, how many controllers hold a brain and how many of those brains are
//              initialized, what behaviour and sub-state each brain is in, the per-NPC 2D distance moved and yaw
//              turned since the last report, and the flow counters the spine bumps (stims, behaviour activations,
//              sub-state entries, callbacks fired, move requests). When the map holds no NPC pawn at all it prints
//              the ten most common actor classes instead, so "the AI does nothing" can be told apart from "there is
//              nobody to do it".
// -disaiprobe  after the census has settled, build one NPC brain per second the way ADishonoredNPCController::InitNPC
//              (2013 rva 0x7632e0) does, on an NPC pawn that has a controller but no brain, and report the result.
//              DISHONORED(port): agent EP corrected 0x7549d0 -> 0x7632e0 here. 0x7549d0 is not a function start in
//              2013 - it is inside UDisStimManager::NewStim<FAIStimStruct_IdleRequest> - and rva_sweep.py passes it as
//              ok-2013-mid, which is why build/agentEP/addr_audit.py resolves every citation by NAME as well.
//              This is the stand-in for ADishonoredSpawner, which is not in this package (see agentCG.md).

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "disdesirestructs.h"
#include "arkcomponentlocomotion.h"
#include "disaisubstate.h"

INT GDisAIBrainTicks = 0;
INT GDisAIStimsEnqueued = 0;
INT GDisAIStimsProcessed = 0;
INT GDisAIBehaviorActivations = 0;
INT GDisAIBehaviorInits = 0;
INT GDisAISubStateEnters = 0;
INT GDisAISubStateTicks = 0;
INT GDisAISubProcessBegins = 0;
INT GDisAICallbacksFired = 0;
INT GDisAIMoveRequests = 0;

static INT GDisAICensus = -1;
static INT GDisAIProbe = -1;

UBOOL DisAICensusEnabled()
{
	if( GDisAICensus < 0 )
	{
		GDisAICensus = ( appStrfind( appCmdLine(), TEXT("-disai") ) != NULL ) ? 1 : 0;
	}
	return GDisAICensus != 0;
}

UBOOL DisAIProbeEnabled()
{
	if( GDisAIProbe < 0 )
	{
		GDisAIProbe = ( appStrfind( appCmdLine(), TEXT("-disaiprobe") ) != NULL ) ? 1 : 0;
	}
	return GDisAIProbe != 0;
}

/** One NPC's pose at the previous report, so the census can report movement rather than position. */
struct FDisAINPCTrack
{
	ADishonoredNPCPawn* Pawn;
	FVector Location;
	INT Yaw;
	FLOAT Moved;
	FLOAT MovedTotal;
	INT Turned;
	INT TurnedTotal;
};

struct FDisAICensusState
{
	UWorld* World;
	FLOAT NextReportTime;
	FLOAT NextProbeTime;
	INT Probed;
	INT ProbedOk;
	TArray<FDisAINPCTrack> Tracks;

	FDisAICensusState() : World(NULL), NextReportTime(0.f), NextProbeTime(0.f), Probed(0), ProbedOk(0) {}
};

static FDisAICensusState GDisAIState;

static FDisAINPCTrack* DisAIFindTrack( ADishonoredNPCPawn* Pawn )
{
	for( INT Idx = 0; Idx < GDisAIState.Tracks.Num(); Idx++ )
	{
		if( GDisAIState.Tracks(Idx).Pawn == Pawn )
		{
			return &GDisAIState.Tracks(Idx);
		}
	}
	FDisAINPCTrack Track;
	Track.Pawn = Pawn;
	Track.Location = Pawn->Location;
	Track.Yaw = Pawn->Rotation.Yaw;
	Track.Moved = 0.f;
	Track.MovedTotal = 0.f;
	Track.Turned = 0;
	Track.TurnedTotal = 0;
	const INT Added = GDisAIState.Tracks.AddItem( Track );
	return &GDisAIState.Tracks(Added);
}

/** agent DN: per-NPC total 2D distance since the census started, so "they moved" can be read per guard rather than as a
    single sum. The tracker is agent CG's; this only formats it, sorted by distance, largest first. */
static FString DisAIPerNPCMoved()
{
	TArray<INT> Order;
	for( INT i = 0; i < GDisAIState.Tracks.Num(); ++i )
	{
		if( GDisAIState.Tracks( i ).MovedTotal >= 1.f )
		{
			Order.AddItem( i );
		}
	}
	for( INT a = 0; a < Order.Num(); ++a )
	{
		for( INT b = a + 1; b < Order.Num(); ++b )
		{
			if( GDisAIState.Tracks( Order( b ) ).MovedTotal > GDisAIState.Tracks( Order( a ) ).MovedTotal )
			{
				const INT Tmp = Order( a ); Order( a ) = Order( b ); Order( b ) = Tmp;
			}
		}
	}
	FString Out = FString::Printf( TEXT("%i of %i NPCs moved:"), Order.Num(), GDisAIState.Tracks.Num() );
	for( INT i = 0; i < Order.Num() && i < 12; ++i )
	{
		const FDisAINPCTrack& rTrack = GDisAIState.Tracks( Order( i ) );
		Out += FString::Printf( TEXT(" %s=%.0f"), rTrack.Pawn ? *rTrack.Pawn->GetName() : TEXT("?"), rTrack.MovedTotal );
	}
	return Out;
}

/*-----------------------------------------------------------------------------
	agent DF: the sub-state and behaviour histograms
-----------------------------------------------------------------------------*/

INT GDisAISubStateTransitions = 0;

struct FDisAIClassCount
{
	UClass* Class;
	INT Count;
};

static TArray<FDisAIClassCount> GDisAISubStateCounts;
static TArray<FDisAIClassCount> GDisAIBehaviorCounts;

static void DisAIBump( TArray<FDisAIClassCount>& _rTable, UClass* _pClass )
{
	if( !_pClass )
	{
		return;
	}
	for( INT Idx = 0; Idx < _rTable.Num(); Idx++ )
	{
		if( _rTable(Idx).Class == _pClass )
		{
			_rTable(Idx).Count++;
			return;
		}
	}
	FDisAIClassCount Row;
	Row.Class = _pClass;
	Row.Count = 1;
	_rTable.AddItem( Row );
}

/** Highest count first, so the line reads as "what the NPCs mostly do". */
static FString DisAIFormatHistogram( const TArray<FDisAIClassCount>& _rTable )
{
	FString Out;
	TArray<INT> Used;
	for( INT Rank = 0; Rank < _rTable.Num(); Rank++ )
	{
		INT Best = INDEX_NONE;
		for( INT Idx = 0; Idx < _rTable.Num(); Idx++ )
		{
			if( Used.FindItemIndex( Idx ) != INDEX_NONE )
			{
				continue;
			}
			if( Best == INDEX_NONE || _rTable(Idx).Count > _rTable(Best).Count )
			{
				Best = Idx;
			}
		}
		if( Best == INDEX_NONE )
		{
			break;
		}
		Used.AddItem( Best );
		Out += FString::Printf( TEXT("%s=%i "), *_rTable(Best).Class->GetName(), _rTable(Best).Count );
	}
	return Out;
}

void DisAINoteSubStateEnter( UClass* Entered, UClass* Left )
{
	if( !DisAICensusEnabled() )
	{
		return;
	}
	DisAIBump( GDisAISubStateCounts, Entered );
	if( Entered && Left && Entered != Left )
	{
		GDisAISubStateTransitions++;
	}
}

void DisAINoteBehaviorSlot0( UClass* Behavior )
{
	if( !DisAICensusEnabled() )
	{
		return;
	}
	DisAIBump( GDisAIBehaviorCounts, Behavior );
}

FString DisAISubStateHistogram()
{
	return DisAIFormatHistogram( GDisAISubStateCounts );
}

FString DisAIBehaviorHistogram()
{
	return DisAIFormatHistogram( GDisAIBehaviorCounts );
}

/**
 * DISHONORED(bringup): agent DF. The slot table of one brain, printed once. A behaviour requests a sub-state BY SLOT, and
 * the slot resolves through its own tweaks object's m_SubStateTweaks array - so an empty array means the request is
 * refused and the NPC never leaves DisAISubStateInit no matter how many sub-states are ported. This says whether the
 * array is there, how long it is, and which class each entry spawns.
 */
static void DisAIReportSlotTable( UDishonoredAIBrain* Brain )
{
	if( !Brain )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): disai slots: brain %s has %i behaviours"), *Brain->GetName(), Brain->m_BehaviorArray.Num() );
	for( INT Idx = 0; Idx < Brain->m_BehaviorArray.Num(); Idx++ )
	{
		UDishonoredAIBehavior* Behavior = Brain->m_BehaviorArray(Idx);
		if( !Behavior )
		{
			continue;
		}
		UDisTweaks_AIBehavior* Tweaks = (UDisTweaks_AIBehavior*)Behavior->GetTweaks_Derived();
		FString Slots;
		if( Tweaks )
		{
			for( INT Slot = 0; Slot < Tweaks->m_SubStateTweaks.Num(); Slot++ )
			{
				UDisTweaks_AISubState* SlotTweaks = Tweaks->m_SubStateTweaks(Slot);
				UClass* SpawnClass = SlotTweaks ? SlotTweaks->GetSpawnedObjectClass( eDisTweaksSpawnType_InGame ) : NULL;
				Slots += FString::Printf( TEXT("%i:%s->%s "), Slot,
					SlotTweaks ? *SlotTweaks->GetClass()->GetName() : TEXT("none"),
					SpawnClass ? *SpawnClass->GetName() : TEXT("none") );
			}
		}
		debugf( TEXT("DISHONORED(bringup): disai slots:   %s tweaks %s slots %i [%s] fsm states %i"),
			*Behavior->GetClass()->GetName(),
			Tweaks ? *Tweaks->GetClass()->GetName() : TEXT("NONE"),
			Tweaks ? Tweaks->m_SubStateTweaks.Num() : -1,
			*Slots,
			Behavior->m_pBehaviorFSM ? Behavior->m_pBehaviorFSM->m_NativeStates.Num() : -1 );
	}
}

/*-----------------------------------------------------------------------------
	agent DF: does the dispatcher the engine start-up path now creates actually deliver?

	FArkGameEventDispatcher::CreateInstance had never been called in this tree (agentCG.md hand-over 4), so every path
	through the dispatcher was dead code. Turning it on is one line; proving it works needs an event to go in and come out
	again, which is what this does - including the deferral, by registering a second listener from inside the first, which
	is the case the whole m_PendingRegistrations design exists for.
-----------------------------------------------------------------------------*/

class FDisAIArkEventProbe
{
public:
	FDisAIArkEventProbe() : m_Received( 0 ), m_ReceivedDeferred( 0 ), m_ReentrantType( 0 ) {}

	void OnEvent( const FArkGameEvent& _rEvent )
	{
		m_Received++;
		// Register a second listener from inside a dispatch of the same type: the dispatcher must defer it rather than
		// mutate the array it is walking.
		FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
		if( Dispatcher && m_Received == 1 )
		{
			Dispatcher->RegisterToEvent( m_ReentrantType, this, &FDisAIArkEventProbe::OnDeferredEvent );
		}
	}

	void OnDeferredEvent( const FArkGameEvent& _rEvent )
	{
		m_ReceivedDeferred++;
	}

	INT m_Received;
	INT m_ReceivedDeferred;
	INT m_ReentrantType;
};

/** The highest event type the retail tables are sized for, so the probe cannot collide with a real subscriber. */
enum { DIS_AI_ARKEVENT_PROBE_TYPE = ARK_GAME_EVENT_TYPE_COUNT - 1 };

static void DisAIArkEventSelfTest()
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( !Dispatcher )
	{
		debugf( TEXT("DISHONORED(bringup): disai arkevents selftest: no instance - FArkGameEventDispatcher::CreateInstance was not called") );
		return;
	}

	FDisAIArkEventProbe Probe;
	Probe.m_ReentrantType = DIS_AI_ARKEVENT_PROBE_TYPE;
	const INT EventType = DIS_AI_ARKEVENT_PROBE_TYPE;

	Dispatcher->RegisterToEvent( EventType, &Probe, &FDisAIArkEventProbe::OnEvent );
	Dispatcher->ProcessEvent( FArkGameEvent( EventType, NULL, NULL ) );
	const INT AfterFirst = Probe.m_Received;
	const INT DeferredAfterFirst = Probe.m_ReceivedDeferred;

	// The second dispatch is where the deferred registration must have been applied.
	Dispatcher->ProcessEvent( FArkGameEvent( EventType, NULL, NULL ) );
	const INT AfterSecond = Probe.m_Received;
	const INT DeferredAfterSecond = Probe.m_ReceivedDeferred;

	Dispatcher->UnregisterToEvent( EventType, &Probe, &FDisAIArkEventProbe::OnEvent );
	Dispatcher->UnregisterToEvent( EventType, &Probe, &FDisAIArkEventProbe::OnDeferredEvent );
	Dispatcher->ProcessEvent( FArkGameEvent( EventType, NULL, NULL ) );

	debugf( TEXT("DISHONORED(bringup): disai arkevents selftest: delivered %i/1 then %i/2, deferred registration delivered %i then %i, after unregister %i (expect 2 and 1)"),
		AfterFirst, AfterSecond, DeferredAfterFirst, DeferredAfterSecond, Probe.m_Received );
	const UBOOL bOk = ( AfterFirst == 1 && AfterSecond == 2 && DeferredAfterFirst == 0 && DeferredAfterSecond == 1 && Probe.m_Received == 2 );
	debugf( TEXT("DISHONORED(bringup): disai arkevents selftest: %s"), bOk ? TEXT("ok - register, dispatch, defer and unregister all work") : TEXT("FAILED") );
}

/** The ten most common actor classes of the world, so an empty NPC census can be read. */
static FString DisAITopActorClasses()
{
	TArray<UClass*> Classes;
	TArray<INT> Counts;
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor )
		{
			continue;
		}
		INT Found = Classes.FindItemIndex( Actor->GetClass() );
		if( Found == INDEX_NONE )
		{
			Found = Classes.AddItem( Actor->GetClass() );
			Counts.AddItem( 0 );
		}
		Counts(Found)++;
	}
	FString Out;
	for( INT Rank = 0; Rank < 10; Rank++ )
	{
		INT Best = INDEX_NONE;
		for( INT Idx = 0; Idx < Counts.Num(); Idx++ )
		{
			if( Counts(Idx) > 0 && ( Best == INDEX_NONE || Counts(Idx) > Counts(Best) ) )
			{
				Best = Idx;
			}
		}
		if( Best == INDEX_NONE )
		{
			break;
		}
		Out += FString::Printf( TEXT("%s=%i "), *Classes(Best)->GetName(), Counts(Best) );
		Counts(Best) = 0;
	}
	return Out;
}

/*-----------------------------------------------------------------------------
	agent DN: the navigation-mesh census.

	The locomotion component's only entries into the nav mesh are static UNavigationHandle queries
	(FArkComponentLocomotion::FindNearestLocationOnNavMesh 2013 rva 0x545660 calls GetAllPolysFromPos,
	UpdateStartLocAndVerifyIfOnValidPoly 0x53ed10 calls GetPylonAndPolyFromPos) plus UNavigationHandle::FindPath for
	the A*. So before porting any of it: is there a mesh, and is there a poly under each NPC's feet? Reported once,
	the first time the census runs with a world that holds NPCs, because it walks every pylon.
-----------------------------------------------------------------------------*/
static void DisAINavMeshReport( UWorld* World )
{
	INT Pylons = 0, PylonsWithMesh = 0, PylonsEnabled = 0, Polys = 0, Verts = 0, Edges = 0;
	for( FActorIterator It; It; ++It )
	{
		APylon* Pylon = Cast<APylon>( *It );
		if( !Pylon )
		{
			continue;
		}
		Pylons++;
		PylonsEnabled += Pylon->bDisabled ? 0 : 1;
		UNavigationMeshBase* Mesh = Pylon->NavMeshPtr;
		if( !Mesh )
		{
			continue;
		}
		PylonsWithMesh++;
		Polys += Mesh->Polys.Num();
		Verts += Mesh->Verts.Num();
		Edges += Mesh->EdgePtrs.Num();
	}
	debugf( TEXT("DISHONORED(bringup): disai navmesh: %i pylons (%i enabled, %i with a mesh), %i polys, %i verts, %i edges"),
		Pylons, PylonsEnabled, PylonsWithMesh, Polys, Verts, Edges );

	INT Npcs = 0, OnMesh = 0, NearMesh = 0;
	FString First;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
		if( !Pawn || Pawn->IsPendingKill() )
		{
			continue;
		}
		Npcs++;
		FVector Ground = Pawn->Location;
		if( Pawn->CylinderComponent )
		{
			Ground.Z -= Pawn->CylinderComponent->CollisionHeight;
		}
		APylon* FoundPylon = NULL;
		FNavMeshPolyBase* FoundPoly = NULL;
		if( UNavigationHandle::GetPylonAndPolyFromPos( Ground, 0.f, FoundPylon, FoundPoly ) )
		{
			OnMesh++;
		}
		else
		{
			TArray<FNavMeshPolyBase*> Near;
			const FVector Extent( 150.f, 150.f, 150.f );
			if( UNavigationHandle::GetAllPolysFromPos( Pawn->Location, Extent, Near, FALSE ) && Near.Num() > 0 )
			{
				NearMesh++;
			}
		}
		if( !First.Len() )
		{
			First = FString::Printf( TEXT("%s at %s ground %s poly %s"),
				*Pawn->GetName(), *Pawn->Location.ToString(), *Ground.ToString(),
				FoundPoly ? TEXT("yes") : TEXT("no") );
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai navmesh: %i NPCs, %i standing on a poly, %i with a poly within 150uu; first %s"),
		Npcs, OnMesh, NearMesh, *First );
}

/*-----------------------------------------------------------------------------
	agent DN: the locomotion census.
-----------------------------------------------------------------------------*/

/** The most recent path-finding error any component recorded, named rather than numbered. */
static FString DisAILocoLastError()
{
	INT Best = INDEX_NONE;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
		FArkComponentLocomotion* pLoco = Pawn ? Pawn->GetComponentLocomotion() : NULL;
		if( pLoco && pLoco->GetLastPathFindingError() != PATHERROR_MAX )
		{
			Best = pLoco->GetLastPathFindingError();
			break;
		}
	}
	if( Best == INDEX_NONE )
	{
		return FString( TEXT("none") );
	}
	static const TCHAR* Names[] = { TEXT("StartPolyNotFound"), TEXT("GoalPolyNotFound"), TEXT("AnchorPylonNotFound"),
		TEXT("NoPathFound"), TEXT("ComputeValidFinalDestFail"), TEXT("GetNextMoveLocationFail"), TEXT("MoveTimeout") };
	return ( Best >= 0 && Best < ARRAY_COUNT( Names ) ) ? FString( Names[Best] ) : FString::Printf( TEXT("%i"), Best );
}

/** Per-NPC locomotion state, for the thoughts line: how far this NPC's own component has taken it along its path. */
FString DisAILocoThought( class ADishonoredNPCPawn* _pPawn )
{
	FArkComponentLocomotion* pLoco = _pPawn ? _pPawn->GetComponentLocomotion() : NULL;
	if( !pLoco )
	{
		return FString( TEXT(" loco none") );
	}
	return FString::Printf( TEXT(" loco req %i path %i/%i%s"),
		pLoco->GetRequestsCount(), pLoco->GetCurPathPointIdx(), pLoco->GetPathPoints().Num(),
		pLoco->IsArrived() ? TEXT(" arrived") : TEXT("") );
}

/*-----------------------------------------------------------------------------
	agent DN: -dislocowalk[=<seconds>] - give every NPC somewhere to go, once, and let the ported machinery take it there.
-----------------------------------------------------------------------------*/

static INT GDisLocoWalk = -1;
static FLOAT GDisLocoWalkTime = 20.f;

UBOOL DisAIWalkTestEnabled()
{
	if( GDisLocoWalk < 0 )
	{
		const TCHAR* Found = appStrfind( appCmdLine(), TEXT("-dislocowalk") );
		GDisLocoWalk = Found ? 1 : 0;
		if( Found && Found[12] == TEXT('=') )
		{
			GDisLocoWalkTime = appAtof( Found + 13 );
		}
	}
	return GDisLocoWalk != 0;
}

/** The farthest poly centre within _fRadius of the pawn that the pawn can path to. */
static UBOOL DisAIPickWalkTarget( ADishonoredNPCPawn* _pPawn, FLOAT _fRadius, FVector& _rOut )
{
	TArray<FNavMeshPolyBase*> Polys;
	const FVector Extent( _fRadius, _fRadius, _fRadius );
	if( !UNavigationHandle::GetAllPolysFromPos( _pPawn->Location, Extent, Polys, FALSE ) || Polys.Num() == 0 )
	{
		return FALSE;
	}
	// Where to send them: the farthest poly centre within the radius that is on the NPC's own level, give or take a
	// storey. Measured alternative, kept as a finding rather than as code: picking the poly nearest the PLAYER instead
	// sends every NPC to a poly 470 units below itself, because the player spawns at the foot of the tower - and the
	// searches then fail with GoalPolyNotFound, because reaching another floor needs the stair edges the cooked mesh
	// links through and the reference A* did not find them from those starts. Farthest-on-my-own-level produced 812 paths
	// and 30,088 units of walking; nearest-to-the-player produced none.
	FLOAT fBest = -1.f;
	UBOOL bFound = FALSE;
	for( INT i = 0; i < Polys.Num(); ++i )
	{
		const FVector Centre = Polys( i )->GetPolyCenter();
		if( Abs( Centre.Z - _pPawn->Location.Z ) > 200.f )
		{
			continue;
		}
		const FLOAT fFromNPCSq = ( Centre - _pPawn->Location ).SizeSquared2D();
		if( fFromNPCSq <= ( 200.f * 200.f ) )
		{
			continue;
		}
		if( !bFound || fFromNPCSq > fBest )
		{
			fBest = fFromNPCSq;
			_rOut = Centre;
			bFound = TRUE;
		}
	}
	return bFound;
}

/**
 * One order per NPC, through its own live sub-state's desire. The sub-state does not fight it: its own
 * RefreshSubState issues its target once (agent DF measured 26 new / 0 update / 0 stop over 150 seconds) and
 * FDisDesireRequest::GetRequestStatus then answers Unchanged, because the target it compares against is the one stored in
 * the request - which is now the probe's.
 */
/** What the first pass asked of one NPC, so the second pass can re-state the same order at a different speed. */
struct FDisAIWalkOrder
{
	ADishonoredNPCPawn*	Pawn;
	ADishonoredNPCPawn*	ActorTarget;	// NULL for a location order
	FVector				Location;
};
static TArray<FDisAIWalkOrder> GDisAIWalkOrders;

/**
 * agent DN: the second pass. The same order, at the RUN speed index instead of the walk one. Nothing about the target
 * changes, so FDisDesireRequest::GetRequestStatus answers Unchanged for the target and FDisLocoRequest::SetParams then
 * raises DTDRS_UpdateRequestNeeded because the speed index differs - which is the only thing that makes the desire layer
 * take the update branch rather than stop-and-restart, and the only thing that reaches
 * FArkRequestManager::UpdateRequestByIdx. It is what a behaviour escalating from a walk to a chase does.
 */
static void DisAIWalkTestUpdate()
{
	INT Updated = 0;
	for( INT i = 0; i < GDisAIWalkOrders.Num(); ++i )
	{
		const FDisAIWalkOrder& rOrder = GDisAIWalkOrders( i );
		ADishonoredNPCPawn* Pawn = rOrder.Pawn;
		if( !Pawn || Pawn->IsPendingKill() )
		{
			continue;
		}
		ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Pawn->Controller );
		UDishonoredAIBrain* Brain = NPCController ? NPCController->GetAIBrain() : NULL;
		UDishonoredAIBehavior* Behavior = Brain ? Brain->GetCurrentBehavior() : NULL;
		UDisAISubState* SubState = Behavior ? Behavior->GetCurrentSubState() : NULL;
		IDisDesiresInterface* Desires = SubState ? SubState->GetDesires() : NULL;
		FDisLocoRequest* LocoRequest = Desires ? Desires->GetDesiresLocoRequest() : NULL;
		if( !LocoRequest )
		{
			continue;
		}
		if( rOrder.ActorTarget )
		{
			LocoRequest->RequestActorTarget( rOrder.ActorTarget, 2, -1.f, 1.f, TRUE, FALSE );
		}
		else
		{
			LocoRequest->RequestLocationTarget( rOrder.Location, 2, -1.f, 1.f, TRUE, FALSE );
		}
		Updated++;
	}
	debugf( TEXT("DISHONORED(bringup): disai locowalk: re-stated %i orders at the run speed index, which is what the update path is for"), Updated );
}

static void DisAIWalkTest( UWorld* World )
{
	GDisAIWalkOrders.Empty();
	INT Asked = 0;
	INT NoSubState = 0;
	INT NoTarget = 0;
	INT Followers = 0;
	ADishonoredNPCPawn* pPrevAsked = NULL;
	FString First;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
		if( !Pawn || Pawn->IsPendingKill() )
		{
			continue;
		}
		ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Pawn->Controller );
		UDishonoredAIBrain* Brain = NPCController ? NPCController->GetAIBrain() : NULL;
		UDishonoredAIBehavior* Behavior = Brain ? Brain->GetCurrentBehavior() : NULL;
		UDisAISubState* SubState = Behavior ? Behavior->GetCurrentSubState() : NULL;
		IDisDesiresInterface* Desires = SubState ? SubState->GetDesires() : NULL;
		FDisLocoRequest* LocoRequest = Desires ? Desires->GetDesiresLocoRequest() : NULL;
		if( !LocoRequest )
		{
			NoSubState++;
			continue;
		}
		// Walk speed is index 1 of the config's speed names (index 0 is idle); the threshold is retail's own default of
		// -1, i.e. "use the pawn's radius".
		// Every fourth NPC is given the PREVIOUS one as an actor target rather than a location. That is not decoration: an
		// actor target is re-resolved on every TickDesires, so as the target walks, FDisDesireRequest::GetRequestStatus
		// answers UpdateRequestNeeded and the order goes down FArkComponentLocomotion::UpdateLocoToActor ->
		// FArkRequestManager::UpdateRequestByIdx - the update half of the request queue, which a fixed destination never
		// exercises. It is also what a guard following another guard asks for.
		LocoRequest->m_bPaused = FALSE;
		if( pPrevAsked && ( Asked % 4 ) == 3 )
		{
			LocoRequest->RequestActorTarget( pPrevAsked, 1, -1.f, 1.f, TRUE, FALSE );
			FDisAIWalkOrder Order;
			Order.Pawn = Pawn;
			Order.ActorTarget = pPrevAsked;
			Order.Location = FVector( 0.f, 0.f, 0.f );
			GDisAIWalkOrders.AddItem( Order );
			Followers++;
			Asked++;
			continue;
		}
		FVector Target( 0.f, 0.f, 0.f );
		if( !DisAIPickWalkTarget( Pawn, 1500.f, Target ) )
		{
			NoTarget++;
			continue;
		}
		LocoRequest->RequestLocationTarget( Target, 1, -1.f, 1.f, TRUE, FALSE );
		FDisAIWalkOrder Order;
		Order.Pawn = Pawn;
		Order.ActorTarget = NULL;
		Order.Location = Target;
		GDisAIWalkOrders.AddItem( Order );
		pPrevAsked = Pawn;
		Asked++;
		if( !First.Len() )
		{
			First = FString::Printf( TEXT("%s %s -> %s (%.0f uu)"), *Pawn->GetName(), *Pawn->Location.ToString(),
				*Target.ToString(), ( Target - Pawn->Location ).Size2D() );
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai locowalk: asked %i NPCs to walk, %i had no sub-state desire, %i had no reachable target; first %s"),
		Asked, NoSubState, NoTarget, *First );
	debugf( TEXT("DISHONORED(bringup): disai locowalk: %i of them were given another NPC as an actor target, which is what makes the queue's update path run"), Followers );

	// -dislocowatch=<n>: put the camera behind NPC n so the walk is in front of it. This is a measurement switch in the
	// same family as -apshottime: the player's own pawn is moved and pointed, and nothing about the NPC changes. Without
	// it the player spawns at the water's edge of L_Tower_P looking at the boats, and no guard is in frame.
	INT WatchIdx = -1;
	{
		const TCHAR* Found = appStrfind( appCmdLine(), TEXT("-dislocowatch") );
		if( Found )
		{
			WatchIdx = ( Found[13] == TEXT('=') ) ? appAtoi( Found + 14 ) : 0;
		}
	}
	if( WatchIdx >= 0 )
	{
		// By name, not by iteration order: -dislocowatch=13 means DishonoredNPCPawn_13, which is the one the -disai
		// locomoved line says walked farthest, and iteration order is not that.
		const FString WantedName = FString::Printf( TEXT("DishonoredNPCPawn_%i"), WatchIdx );
		ADishonoredNPCPawn* Watched = NULL;
		INT Seen = 0;
		for( FActorIterator It; It; ++It )
		{
			ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
			FArkComponentLocomotion* pLoco = Pawn ? Pawn->GetComponentLocomotion() : NULL;
			if( !pLoco || pLoco->GetRequestsCount() <= 0 )
			{
				continue;
			}
			Seen++;
			if( Pawn->GetName() == WantedName )
			{
				Watched = Pawn;
				break;
			}
		}
		APlayerController* PC = NULL;
		for( AController* C = GWorld->GetFirstController(); C; C = C->NextController )
		{
			PC = Cast<APlayerController>( C );
			if( PC && PC->Pawn )
			{
				break;
			}
			PC = NULL;
		}
		if( Watched && PC && PC->Pawn )
		{
			const FVector Facing = Watched->Rotation.Vector();
			FVector CamLoc = Watched->Location - Facing * 420.f;
			CamLoc.Z += 90.f;
			GWorld->FarMoveActor( PC->Pawn, CamLoc, FALSE, TRUE, FALSE );
			const FRotator Look = ( Watched->Location - PC->Pawn->Location ).Rotation();
			PC->Pawn->SetRotation( Look );
			PC->SetRotation( Look );
			// The camera has to stay put, or a before-and-after pair ten world-seconds apart compares two different places
			// rather than two positions of the same guard: the player's pawn is a walking pawn and it drifts. Physics off,
			// world collision off, velocity zero - it becomes a tripod.
			PC->Pawn->Velocity = FVector( 0.f, 0.f, 0.f );
			PC->Pawn->Acceleration = FVector( 0.f, 0.f, 0.f );
			PC->Pawn->bCollideWorld = FALSE;
			PC->Pawn->setPhysics( PHYS_None );
			debugf( TEXT("DISHONORED(bringup): disai locowatch: camera behind %s at %s looking %s (NPC at %s)"),
				*Watched->GetName(), *PC->Pawn->Location.ToString(), *Look.Vector().ToString(), *Watched->Location.ToString() );
		}
		else
		{
			debugf( TEXT("DISHONORED(bringup): disai locowatch: no NPC %i (%i with a request) or no player pawn"), WatchIdx, Seen );
		}
	}
}

/*-----------------------------------------------------------------------------
	agent EP: the stim table, the patrol data and the sighting table.

	Three questions this file could not answer before, each of which separates "the system is missing" from "the data
	never asked for it":

	  * which stims were raised at all. UDishonoredAIBrain::ProcessOneStim drops a stim no behaviour's evaluate mask
	    covers without a log line, so an absent behaviour and an absent stim look identical from outside.
	  * what patrol data the level carries. A patrol needs an ADishonoredRoute registered with the map info's
	    UDisPatrolManager and a spawner that asks for one; if the level has none of either, porting UDisBehaviorPatrol
	    changes nothing and the measurement has to say so.
	  * what any vision component saw. Counted from FDisComponentVisionNPC::VisionStatusChanged, i.e. from the one
	    place retail turns a cone-and-line-of-sight test into a fact about an actor.
-----------------------------------------------------------------------------*/

static INT GDisAIStimCounts[256] = { 0 };
static INT GDisAIStimTotal = 0;

void DisAINoteStim( BYTE _StimID )
{
	if( !DisAICensusEnabled() )
	{
		return;
	}
	GDisAIStimCounts[_StimID]++;
	GDisAIStimTotal++;
}

FString DisAIStimHistogram()
{
	// The enum is loaded from the script package, so a name is available once the class hierarchy is up; the numeric id
	// is the fallback rather than an omission, because an unnamed id still answers "was it raised".
	static UEnum* StimEnum = NULL;
	static UBOOL bLookedUp = FALSE;
	if( !bLookedUp )
	{
		bLookedUp = TRUE;
		StimEnum = FindObject<UEnum>( ANY_PACKAGE, TEXT("EAIStimID") );
	}
	FString Out;
	for( INT Id = 0; Id < 256; Id++ )
	{
		if( GDisAIStimCounts[Id] == 0 )
		{
			continue;
		}
		FString Name = StimEnum ? StimEnum->GetEnum( Id ).ToString() : FString::Printf( TEXT("%i"), Id );
		Out += FString::Printf( TEXT("%s(%i)=%i "), *Name, Id, GDisAIStimCounts[Id] );
	}
	return Out.Len() ? Out : FString( TEXT("none") );
}

struct FDisAISighting
{
	FString Seer;
	FString Seen;
	INT Starts;
	INT Stops;
};
static TArray<FDisAISighting> GDisAISightings;
static INT GDisAISightingStarts = 0;
static INT GDisAISightingStops = 0;

void DisAINoteSighting( const AActor* _pSeer, const AActor* _pSeen, UBOOL _bStart )
{
	if( _bStart )
	{
		GDisAISightingStarts++;
	}
	else
	{
		GDisAISightingStops++;
	}
	if( !DisAICensusEnabled() )
	{
		return;
	}
	const FString Seer = _pSeer ? _pSeer->GetName() : FString( TEXT("none") );
	const FString Seen = _pSeen ? _pSeen->GetName() : FString( TEXT("none") );
	for( INT Idx = 0; Idx < GDisAISightings.Num(); Idx++ )
	{
		if( GDisAISightings(Idx).Seer == Seer && GDisAISightings(Idx).Seen == Seen )
		{
			if( _bStart )
			{
				GDisAISightings(Idx).Starts++;
			}
			else
			{
				GDisAISightings(Idx).Stops++;
			}
			return;
		}
	}
	if( GDisAISightings.Num() >= 32 )
	{
		return;
	}
	const INT New = GDisAISightings.Add();
	GDisAISightings(New).Seer = Seer;
	GDisAISightings(New).Seen = Seen;
	GDisAISightings(New).Starts = _bStart ? 1 : 0;
	GDisAISightings(New).Stops = _bStart ? 0 : 1;
}

FString DisAISightingReport()
{
	FString Out = FString::Printf( TEXT("%i started, %i stopped; "), GDisAISightingStarts, GDisAISightingStops );
	for( INT Idx = 0; Idx < GDisAISightings.Num(); Idx++ )
	{
		Out += FString::Printf( TEXT("[%s saw %s %i/%i]"), *GDisAISightings(Idx).Seer, *GDisAISightings(Idx).Seen,
			GDisAISightings(Idx).Starts, GDisAISightings(Idx).Stops );
	}
	return Out;
}

/** agent EP: what each patrolling brain is actually holding. The four numbers that separate "the behaviour never
    activated" from "it activated and could not find a route" from "it found one and the sub-state machine refused". */
FString DisAIPatrolState( UWorld* World )
{
	FString Out;
	INT Shown = 0;
	for( FActorIterator It; It && Shown < 8; ++It )
	{
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
		UDishonoredAIBrain* Brain = Pawn ? Pawn->GetAIBrain() : NULL;
		if( !Brain )
		{
			continue;
		}
		UDisBehaviorPatrol* Patrol = NULL;
		for( INT Slot = 0; Slot < ARRAY_COUNT(Brain->m_ActiveBehaviorStack); Slot++ )
		{
			UDisBehaviorPatrol* Candidate = Cast<UDisBehaviorPatrol>( Brain->m_ActiveBehaviorStack[Slot] );
			if( Candidate )
			{
				Patrol = Candidate;
			}
		}
		if( !Patrol )
		{
			continue;
		}
		Shown++;
		UDisPatrolManager* PatrolManager = NULL;
		if( World && World->GetWorldInfo() )
		{
			UDishonoredMapInfo* MapInfo = Cast<UDishonoredMapInfo>( World->GetWorldInfo()->GetMapInfo() );
			PatrolManager = MapInfo ? MapInfo->m_pPatrolManager : NULL;
		}
		UBOOL bAnyCanAdopt = FALSE;
		FLOAT NearestRouteDist = -1.f;
		ADishonoredRoute* Nearest = NULL;
		if( PatrolManager )
		{
			for( ADishonoredRoute* Route = PatrolManager->m_pRouteList; Route; Route = Route->m_pNextRoute )
			{
				const FLOAT Dist = ( Pawn->Location - Route->Location ).Size();
				if( NearestRouteDist < 0.f || Dist < NearestRouteDist )
				{
					NearestRouteDist = Dist;
					Nearest = Route;
				}
				if( Route->CanAdopt( Pawn, FALSE ) )
				{
					bAnyCanAdopt = TRUE;
				}
			}
		}
		// Which of CanAdopt's five tests refuses the nearest route. Written out because "canadopt 0" is the answer to
		// the wrong question: the five tests fail for completely different reasons and only one of them is a defect.
		if( Nearest )
		{
			INT NullPoints = 0;
			for( INT Idx = 0; Idx < Nearest->RouteList.Num(); Idx++ )
			{
				if( !Nearest->RouteList(Idx).Actor )
				{
					NullPoints++;
				}
			}
			Out += FString::Printf( TEXT("{nearest %s active %i points %i null %i squad '%s' pawnsquad '%s' squadok %i cap %i/%i range %.0f/%.0f}"),
				*Nearest->GetName(), (INT)Nearest->m_bIsActive, Nearest->RouteList.Num(), NullPoints,
				Nearest->m_SupportedSquads.Num() ? *Nearest->m_SupportedSquads(0).m_SquadName.ToString() : TEXT("<none>"),
				*Pawn->m_SpawnerInfo.m_Squad.ToString(),
				(INT)IDisSquadInterface::IsSquadSupported( Nearest->m_SupportedSquads, Pawn->m_SpawnerInfo.m_Squad ),
				Nearest->m_NumNPCAdopters, Nearest->m_Capacity,
				NearestRouteDist, Nearest->m_AdoptionRange );
		}
		// The sub-state's own exit condition, because "the guard arrived and stopped there" and "the guard arrived and
		// walked on" differ by exactly these three flags (UDisAISubStateTakePosition::TickState).
		UDisAISubStateTakePosition* TakePos = Cast<UDisAISubStateTakePosition>( Patrol->GetCurrentSubState() );
		if( TakePos )
		{
			// m_pLocoComponent is the one that separates the two candidate causes: a request whose component pointer is
			// NULL was never bound, which means IDisDesiresInterface::InitializeDesires early-returned on
			// GetDesiresOwningPawn() for this sub-state - and a request that was never bound was also never paused, so it
			// still carries m_RequestID 0 where every bound one carries INDEX_NONE.
			UDisAISubState* IdleStand = NULL;
			for( INT Slot = 0; Slot < ARRAY_COUNT(Brain->m_ActiveBehaviorStack); Slot++ )
			{
				UDishonoredAIBehavior* Other = Brain->m_ActiveBehaviorStack[Slot];
				UDisAISubStateTakePosition* OtherTakePos = Other ? Cast<UDisAISubStateTakePosition>( Other->GetCurrentSubState() ) : NULL;
				if( Other && Other != Patrol && OtherTakePos )
				{
					IdleStand = OtherTakePos;
				}
			}
			Out += FString::Printf( TEXT("(destreached %i rotreached %i rotfocus %i stoptype %i rottarget %i; loco desired %i paused %i id %i cpnt %i dest %s; otherstate %s cpnt %i id %i)"),
				(INT)TakePos->m_bDestinationReached, (INT)TakePos->m_bRotationReached, (INT)TakePos->m_bRotationFocusSet,
				(INT)TakePos->m_StopType, (INT)TakePos->m_eTakePosRotationTarget,
				(INT)TakePos->m_LocoRequest.m_bDesired, (INT)TakePos->m_LocoRequest.m_bPaused,
				TakePos->m_LocoRequest.m_RequestID,
				TakePos->m_LocoRequest.m_pLocoComponent ? 1 : 0,
				*TakePos->m_lrDestination.m_Loc.ToString(),
				IdleStand ? *IdleStand->GetClass()->GetName() : TEXT("none"),
				( IdleStand && ((UDisAISubStateTakePosition*)IdleStand)->m_LocoRequest.m_pLocoComponent ) ? 1 : 0,
				IdleStand ? ((UDisAISubStateTakePosition*)IdleStand)->m_LocoRequest.m_RequestID : -999 );
		}
		FArkComponentLocomotion* pLoco = Pawn->GetComponentLocomotion();
		Out += FString::Printf( TEXT("<loco req %i path %i/%i%s%s speed %.0f/%.0f dist %.0f>"),
			pLoco ? pLoco->GetRequestsCount() : -1,
			pLoco ? pLoco->GetCurPathPointIdx() : -1,
			pLoco ? pLoco->GetPathPoints().Num() : -1,
			( pLoco && pLoco->IsArrived() ) ? TEXT(" arrived") : TEXT(""),
			( pLoco && !pLoco->HasComputedPath() ) ? TEXT(" nopath") : TEXT(""),
			pLoco ? pLoco->GetCurMoveSpeed() : -1.f, pLoco ? pLoco->GetTargetMoveSpeed() : -1.f,
			pLoco ? appSqrt( pLoco->GetSq2DDistToPathEnd() ) : -1.f );
		Out += FString::Printf( TEXT("[%s %s route %s idx %i/%i rep %i dir %i guard %s substate %i/%s nearestroute %.0f canadopt %i]"),
			*Pawn->GetName(), *Patrol->GetClass()->GetName(),
			Patrol->m_pCurrentRoute ? *Patrol->m_pCurrentRoute->GetName() : TEXT("none"),
			Patrol->m_CurrentIndex, Patrol->m_StartingIndex, Patrol->m_Repetitions, (INT)Patrol->m_RouteDirection,
			Patrol->m_pCurrentGuardPoint ? *Patrol->m_pCurrentGuardPoint->GetName() : TEXT("none"),
			Patrol->GetActiveSubStateIndex(),
			Patrol->GetCurrentSubState() ? *Patrol->GetCurrentSubState()->GetClass()->GetName() : TEXT("none"),
			NearestRouteDist, (INT)bAnyCanAdopt );
	}
	return Out.Len() ? Out : FString( TEXT("no patrolling brain") );
}

void DisAIPatrolReport( UWorld* World )
{
	UDishonoredMapInfo* MapInfo = World && World->GetWorldInfo() ? Cast<UDishonoredMapInfo>( World->GetWorldInfo()->GetMapInfo() ) : NULL;
	UDisPatrolManager* PatrolManager = MapInfo ? MapInfo->m_pPatrolManager : NULL;
	INT Routes = 0;
	INT ActiveRoutes = 0;
	INT RoutePoints = 0;
	INT NavPoints = 0;
	INT GuardPoints = 0;
	FString RouteDetail;
	for( FActorIterator It; It; ++It )
	{
		if( Cast<ADishonoredNavPoint>( *It ) )
		{
			NavPoints++;
			if( ((ADishonoredNavPoint*)*It)->m_bGuardForever || ((ADishonoredNavPoint*)*It)->m_fGuardDuration > 0.f )
			{
				GuardPoints++;
			}
		}
		ADishonoredRoute* Route = Cast<ADishonoredRoute>( *It );
		if( !Route )
		{
			continue;
		}
		Routes++;
		if( Route->m_bIsActive )
		{
			ActiveRoutes++;
		}
		RoutePoints += Route->RouteList.Num();
		if( Routes <= 6 )
		{
			RouteDetail += FString::Printf( TEXT("[%s type %i %i points active %i cap %i range %.0f squads %i]"),
				*Route->GetName(), (INT)Route->RouteType, Route->RouteList.Num(), (INT)Route->m_bIsActive,
				Route->m_Capacity, Route->m_AdoptionRange, Route->m_SupportedSquads.Num() );
		}
	}
	INT Spawners = 0;
	INT PatrolSpawners = 0;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredSpawner* Spawner = Cast<ADishonoredSpawner>( *It );
		if( !Spawner )
		{
			continue;
		}
		Spawners++;
		if( Spawner->m_bPatrolUponStartup )
		{
			PatrolSpawners++;
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai patrol: map info %s, patrol manager %s (route list %s); %i routes (%i active, %i points), %i nav points (%i guard); %i spawners, %i ask for a patrol"),
		MapInfo ? TEXT("yes") : TEXT("no"),
		PatrolManager ? TEXT("yes") : TEXT("no"),
		( PatrolManager && PatrolManager->m_pRouteList ) ? *PatrolManager->m_pRouteList->GetName() : TEXT("empty"),
		Routes, ActiveRoutes, RoutePoints, NavPoints, GuardPoints, Spawners, PatrolSpawners );
	if( RouteDetail.Len() )
	{
		debugf( TEXT("DISHONORED(bringup): disai routes: %s"), *RouteDetail );
	}
}

void DisAIReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World || !World->GetWorldInfo() )
	{
		return;
	}
	const FLOAT Now = World->GetTimeSeconds();
	if( GDisAIState.World != World )
	{
		GDisAIState = FDisAICensusState();
		GDisAIState.World = World;
		GDisAIState.NextProbeTime = Now + 16.f;
	}
	if( Now < GDisAIState.NextReportTime )
	{
		return;
	}
	GDisAIState.NextReportTime = Now + 1.f;

	INT Pawns = 0;
	INT Controllers = 0;
	INT Spawners = 0;
	INT Brains = 0;
	INT BrainsInit = 0;
	INT Moving = 0;
	FLOAT MovedThisSecond = 0.f;
	INT TurnedThisSecond = 0;
	FString Thoughts;
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;
		if( !Actor || Actor->IsPendingKill() )
		{
			continue;
		}
		if( Cast<ADishonoredSpawner>( Actor ) != NULL )
		{
			Spawners++;
		}
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( Actor );
		if( !Pawn )
		{
			continue;
		}
		Pawns++;
		// The controllers are counted through the pawns rather than by iterating the world: a controller is an actor with
		// no components, and FActorIterator's per-level actor lists do not always carry it.
		ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Pawn->Controller );
		if( NPCController )
		{
			Controllers++;
			if( NPCController->m_pAIBrain )
			{
				Brains++;
				if( NPCController->m_pAIBrain->IsBrainInitialized() )
				{
					BrainsInit++;
				}
			}
		}
		FDisAINPCTrack* Track = DisAIFindTrack( Pawn );
		Track->Moved = ( Pawn->Location - Track->Location ).Size2D();
		Track->Turned = Abs( Pawn->Rotation.Yaw - Track->Yaw );
		Track->MovedTotal += Track->Moved;
		Track->TurnedTotal += Track->Turned;
		Track->Location = Pawn->Location;
		Track->Yaw = Pawn->Rotation.Yaw;
		MovedThisSecond += Track->Moved;
		TurnedThisSecond += Track->Turned;
		if( Track->Moved > 1.f )
		{
			Moving++;
		}
		if( Pawns <= 8 )
		{
			UDishonoredAIBrain* Brain = Pawn->GetAIBrain();
			UDishonoredAIBehavior* Behavior = Brain ? Brain->m_pCurrentBehavior : NULL;
			UDisAISubState* SubState = Behavior ? Behavior->GetCurrentSubState() : NULL;
			Thoughts += FString::Printf( TEXT("[%s ctrl %s brain %s behavior %s substate %s moved %.1f turned %i] "),
				*Pawn->GetName(),
				Pawn->Controller ? *Pawn->Controller->GetName() : TEXT("none"),
				Brain ? ( Brain->IsBrainInitialized() ? TEXT("init") : TEXT("raw") ) : TEXT("none"),
				Behavior ? *Behavior->GetClass()->GetName() : TEXT("none"),
				SubState ? *SubState->GetClass()->GetName() : TEXT("none"),
				Track->Moved, Track->Turned );
		}
	}

	debugf( TEXT("DISHONORED(bringup): disai census: %i NPC pawns, %i controllers (%i with a brain, %i initialized), %i spawners; ")
			TEXT("moved %.1f uu this second (%i moving), turned %i; brain ticks %i, stims %i/%i, behaviors %i init %i activations, ")
			TEXT("substates %i enters %i ticks, subprocesses %i, callbacks %i, move requests %i"),
		Pawns, Controllers, Brains, BrainsInit, Spawners,
		MovedThisSecond, Moving, TurnedThisSecond,
		GDisAIBrainTicks, GDisAIStimsProcessed, GDisAIStimsEnqueued,
		GDisAIBehaviorInits, GDisAIBehaviorActivations,
		GDisAISubStateEnters, GDisAISubStateTicks, GDisAISubProcessBegins,
		GDisAICallbacksFired, GDisAIMoveRequests );
	// agent DF: what the machines actually did, which is the question agent CG's census could not answer while every
	// sub-state was an empty class.
	// agent DF: the slot table, once, as soon as there is a brain to read it from.
	{
		static UBOOL bReportedSlots = FALSE;
		if( !bReportedSlots && BrainsInit > 0 )
		{
			bReportedSlots = TRUE;
			for( FActorIterator It; It; ++It )
			{
				ADishonoredNPCPawn* SlotPawn = Cast<ADishonoredNPCPawn>( *It );
				UDishonoredAIBrain* SlotBrain = SlotPawn ? SlotPawn->GetAIBrain() : NULL;
				if( SlotBrain )
				{
					DisAIReportSlotTable( SlotBrain );
					break;
				}
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai substates: %i transitions; entered %s"),
		GDisAISubStateTransitions, *DisAISubStateHistogram() );
	debugf( TEXT("DISHONORED(bringup): disai slot0: %s"), *DisAIBehaviorHistogram() );
	// agent EP: every stim any brain was offered, and the level's patrol data once.
	debugf( TEXT("DISHONORED(bringup): disai stims: %i offered; %s"), GDisAIStimTotal, *DisAIStimHistogram() );
	debugf( TEXT("DISHONORED(bringup): disai sightings: %s"), *DisAISightingReport() );
	debugf( TEXT("DISHONORED(bringup): disai patrolstate: %s"), *DisAIPatrolState( World ) );
	{
		static UBOOL bReportedPatrol = FALSE;
		if( !bReportedPatrol && Pawns > 0 )
		{
			bReportedPatrol = TRUE;
			DisAIPatrolReport( World );
		}
	}
	// agent DF: the Ark game-event dispatcher, created for the first time this wave (LaunchEngineLoop.cpp).
	// agent DF: prove the dispatcher once, the first time the census runs.
	{
		static UBOOL bSelfTested = FALSE;
		if( !bSelfTested )
		{
			bSelfTested = TRUE;
			DisAIArkEventSelfTest();
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai arkevents: instance %s, %i global + %i per-object registrations, %i unregistrations, %i deferred, %i dispatches, %i callbacks invoked"),
		FArkGameEventDispatcher::GetInstance() ? TEXT("yes") : TEXT("no"),
		GArkGameEventRegistrations, GArkGameEventPerObjectRegistrations, GArkGameEventUnregistrations,
		GArkGameEventDeferred, GArkGameEventDispatches, GArkGameEventCallbacksInvoked );
	// agent DN: -dislocowalk, once, after the brains have settled.
	if( DisAIWalkTestEnabled() )
	{
		// -dislocoshot=<seconds after the order>: the screenshot is raised HERE, from the census, and not by
		// -apshottime. Both end up setting the same two globals that UGameViewportClient::Draw consumes, but -apshottime
		// keys on FSceneViewFamily::CurrentWorldTime, which is per-world and restarts at zero when the game travels - so
		// on a slow machine it captures the startup map's boat ride instead of the tower, and which map you get depends on
		// how loaded the machine is. Measured: the same command line captured the terrace once and the intro boat the next
		// time. The census only ever runs on the world that holds the NPCs, so keying off it cannot pick the wrong map.
		// This is the same lesson PHASE10 records as "measure the map you mean", arriving from a third direction.
		static FLOAT ShotAt = -1.f;
		static UBOOL bShot = FALSE;
		if( ShotAt < 0.f )
		{
			const TCHAR* Found = appStrfind( appCmdLine(), TEXT("-dislocoshot") );
			ShotAt = ( Found && Found[12] == TEXT('=') ) ? appAtof( Found + 13 ) : 0.f;
		}
		static UBOOL bWalked = FALSE;
		static UBOOL bUpdated = FALSE;
		if( !bWalked && Pawns > 0 && Now >= GDisLocoWalkTime )
		{
			bWalked = TRUE;
			DisAIWalkTest( World );
		}
		else if( bWalked && !bUpdated && Now >= ( GDisLocoWalkTime + 10.f ) )
		{
			bUpdated = TRUE;
			DisAIWalkTestUpdate();
		}
		if( bWalked && !bShot && ShotAt > 0.f && Now >= ( GDisLocoWalkTime + ShotAt ) )
		{
			extern UBOOL GScreenShotRequest;	// Engine/Src/UnPlayer.cpp
			extern FString GScreenShotName;		// Engine/Src/UnPlayer.cpp
			bShot = TRUE;
			GScreenShotName = TEXT("dislocoshot");
			GScreenShotRequest = TRUE;
			debugf( TEXT("DISHONORED(bringup): disai locoshot: screenshot requested %.1f s after the order, world time %.3f, map %s"),
				ShotAt, Now, *World->GetOutermost()->GetName() );
		}
	}
	// agent DN: three NPCs in detail, every second. The five numbers that say what the component is doing: how many
	// orders it holds, where along its path it is, what speed it wants and has, and how far it thinks it still has to go.
	{
		INT Shown = 0;
		FString Detail;
		for( FActorIterator It; It && Shown < 3; ++It )
		{
			ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
			FArkComponentLocomotion* pLoco = Pawn ? Pawn->GetComponentLocomotion() : NULL;
			if( !pLoco )
			{
				continue;
			}
			Shown++;
			Detail += FString::Printf( TEXT("[%s req %i path %i/%i%s%s speed %.0f/%.0f dist %.0f loc %s]"),
				*Pawn->GetName(), pLoco->GetRequestsCount(), pLoco->GetCurPathPointIdx(), pLoco->GetPathPoints().Num(),
				pLoco->IsArrived() ? TEXT(" arrived") : TEXT(""),
				pLoco->HasComputedPath() ? TEXT("") : TEXT(" nopath"),
				pLoco->GetCurMoveSpeed(), pLoco->GetTargetMoveSpeed(), appSqrt( pLoco->GetSq2DDistToPathEnd() ),
				*Pawn->Location.ToString() );
		}
		if( Detail.Len() )
		{
			debugf( TEXT("DISHONORED(bringup): disai locodetail: %s"), *Detail );
		}
	}
	// agent DN: the nav-mesh census, once, as soon as there is an NPC to stand on it.
	{
		static UBOOL bReportedNavMesh = FALSE;
		if( !bReportedNavMesh && Pawns > 0 )
		{
			bReportedNavMesh = TRUE;
			DisAINavMeshReport( World );
		}
	}
	debugf( TEXT("DISHONORED(bringup): disai desires: %i set calls; faceto %i new %i update %i stop; loco %i/%i/%i; lookat %i/%i/%i; body intentions %i"),
		GDisDesireSetCalls,
		GDisDesireRequests[DisDesireStructs::DDK_FaceTo], GDisDesireUpdates[DisDesireStructs::DDK_FaceTo], GDisDesireStops[DisDesireStructs::DDK_FaceTo],
		GDisDesireRequests[DisDesireStructs::DDK_Loco], GDisDesireUpdates[DisDesireStructs::DDK_Loco], GDisDesireStops[DisDesireStructs::DDK_Loco],
		GDisDesireRequests[DisDesireStructs::DDK_LookAt], GDisDesireUpdates[DisDesireStructs::DDK_LookAt], GDisDesireStops[DisDesireStructs::DDK_LookAt],
		GDisDesireBodyIntentions );
	// agent DN: what the locomotion component did with the orders the line above says were given. The two numbers that
	// matter are "paths built" (the nav-mesh A* answered) and "uu moved" (the component translated a pawn); a non-zero
	// request count with zero paths means the search failed, and a non-zero path count with zero movement means the
	// dynamics did.
	debugf( TEXT("DISHONORED(bringup): disai locomoved: %s"), *DisAIPerNPCMoved() );
	debugf( TEXT("DISHONORED(bringup): disai loco: %i components; requests %i start %i update %i stop; paths %i built %i failed, %i path points; %i arrivals; %i MovePawn ticks, %.1f uu moved; %i teleports onto the mesh; last path error %s"),
		GDisLocoComponents,
		GDisLocoRequestsStarted, GDisLocoRequestsUpdated, GDisLocoRequestsStopped,
		GDisLocoPathsBuilt, GDisLocoPathsFailed, GDisLocoPathPointsBuilt,
		GDisLocoArrivals, GDisLocoMovePawnCalls, GDisLocoDistanceMoved, GDisLocoTeleports,
		*DisAILocoLastError() );
	if( Thoughts.Len() )
	{
		debugf( TEXT("DISHONORED(bringup): disai thoughts: %s"), *Thoughts );
	}
	if( Pawns == 0 )
	{
		static UBOOL bReportedClasses = FALSE;
		if( !bReportedClasses && Now > 10.f )
		{
			bReportedClasses = TRUE;
			debugf( TEXT("DISHONORED(bringup): disai no NPC pawn in %s; top actor classes: %s"),
				*World->GetOutermost()->GetName(), *DisAITopActorClasses() );
		}
	}

	if( DisAIProbeEnabled() && Now >= GDisAIState.NextProbeTime )
	{
		GDisAIState.NextProbeTime = Now + 1.f;
		for( FActorIterator It; It; ++It )
		{
			ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
			if( !Pawn || Pawn->IsPendingKill() )
			{
				continue;
			}
			ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Pawn->Controller );
			if( !NPCController || NPCController->m_pAIBrain )
			{
				continue;
			}
			// The same call the spawner makes after it has possessed the pawn (ADishonoredSpawner::OnSpawned,
			// 2013 rva 0x6590e0): the brain tweaks come from the pawn's own NPC tweaks.
			UDisTweaks_NPCPawn* NPCTweaks = Cast<UDisTweaks_NPCPawn>( Pawn->GetTweaks_Derived() );
			UDisTweaks_AIBrain* BrainTweaks = NPCTweaks ? NPCTweaks->m_pBrainTweak : NULL;
			GDisAIState.Probed++;
			if( !BrainTweaks )
			{
				debugf( TEXT("DISHONORED(bringup): disai probe %i %s: no UDisTweaks_AIBrain on its UDisTweaks_NPCPawn (tweaks %s)"),
					GDisAIState.Probed, *Pawn->GetName(), NPCTweaks ? *NPCTweaks->GetName() : TEXT("none") );
				break;
			}
			NPCController->InitNPC( BrainTweaks, DAISL_Unsuspecting );
			const UBOOL bOk = NPCController->m_pAIBrain != NULL && NPCController->m_pAIBrain->IsBrainInitialized();
			GDisAIState.ProbedOk += bOk ? 1 : 0;
			debugf( TEXT("DISHONORED(bringup): disai probe %i %s: InitNPC with %s -> brain %s initialized %i (%i/%i probed ok)"),
				GDisAIState.Probed, *Pawn->GetName(), *BrainTweaks->GetName(),
				NPCController->m_pAIBrain ? *NPCController->m_pAIBrain->GetName() : TEXT("none"),
				bOk ? 1 : 0, GDisAIState.ProbedOk, GDisAIState.Probed );
			break;
		}
	}
}
