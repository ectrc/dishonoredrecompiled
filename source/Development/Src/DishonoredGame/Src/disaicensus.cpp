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
//              (2013 rva 0x7549d0) does, on an NPC pawn that has a controller but no brain, and report the result.
//              This is the stand-in for ADishonoredSpawner, which is not in this package (see agentCG.md).

#include "DishonoredGame.h"
#include "disaicensus.h"

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
