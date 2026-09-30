#pragma once
// DishonoredGame/inc/disaicensus.h
// DISHONORED(written): agent CG. The -disai census and the -disaiprobe walk-in, the measurement half of the AI brain
// package. Both are DISHONORED(bringup) instrumentation and cost nothing when their switch is absent, modelled on
// agent AS's -distouch and agent AU's -dispickup.

class UWorld;

/** TRUE when -disai is on the command line (resolved once). */
UBOOL DisAICensusEnabled();

/** TRUE when -disaiprobe is on the command line (resolved once). */
UBOOL DisAIProbeEnabled();

/** Once a second, per world: the NPC population, how many of them have a brain, what each brain is thinking, and how
    far each NPC has moved and turned since the last report. Called every frame from ADishonoredPlayerController. */
void DisAIReport( UWorld* World, FLOAT DeltaSeconds );

/** Counters the AI spine bumps so the census can report flow rather than state. Definitions in disaicensus.cpp. */
extern INT GDisAIBrainTicks;
extern INT GDisAIStimsEnqueued;
extern INT GDisAIStimsProcessed;
extern INT GDisAIBehaviorActivations;
extern INT GDisAIBehaviorInits;
extern INT GDisAISubStateEnters;
extern INT GDisAISubStateTicks;
extern INT GDisAISubProcessBegins;
extern INT GDisAICallbacksFired;
extern INT GDisAIMoveRequests;

/*-----------------------------------------------------------------------------
	agent DF's extension.

	Agent CG's census could already say "26 NPCs, 26 brains, 684 sub-state entries, DisAISubStateInit" - which was the
	honest answer while every sub-state was an empty class. What it could not say is WHICH sub-states they enter and how
	often they change, because there was only ever one. These four tables answer the three questions this package's accept
	line asks: which sub-states beyond DisAISubStateInit, how many transitions, and which behaviours take slot 0.

	All of it is counted, not sampled: DisAINoteSubStateEnter is called from UDisAISubState::OnEnterState and
	DisAINoteBehaviorSlot0 from UDishonoredAIBrain, so a sub-state entered once in a 150-second run still appears.
-----------------------------------------------------------------------------*/

/** Called from UDisAISubState::OnEnterState for every entry, with the class entered and the class left (or NULL). */
void DisAINoteSubStateEnter( class UClass* Entered, class UClass* Left );

/** Called from UDishonoredAIBrain whenever a behaviour becomes the current one. */
void DisAINoteBehaviorSlot0( class UClass* Behavior );

/** The per-class tables, formatted for the census line. */
FString DisAISubStateHistogram();
FString DisAIBehaviorHistogram();

/** Sub-state changes where the class actually changed, i.e. real transitions rather than re-entries. */
extern INT GDisAISubStateTransitions;

/** TRUE when -dislocowalk is on the command line (agent DN). */
UBOOL DisAIWalkTestEnabled();

/*-----------------------------------------------------------------------------
	agent EP's extension: what the NPC PERCEIVES and where it is asked to WALK.

	Agent DF's tables say which behaviours and sub-states ran. They cannot say why one never ran, because the brain
	drops a stim no behaviour asked for without a word (UDishonoredAIBrain::ProcessOneStim). The stim histogram is the
	missing half: every stim EnqueueStim ever saw, by EAIStimID, so "DisBehaviorPatrol never activated" separates into
	"the patrol request was never raised" and "it was raised and nothing wanted it".
-----------------------------------------------------------------------------*/

/** Called from UDishonoredAIBrain::EnqueueStim for every stim offered to any brain. */
void DisAINoteStim( BYTE _StimID );

/** The stim table, formatted for the census line, EAIStimID names where the enum is loaded. */
FString DisAIStimHistogram();

/** Once, as soon as the level has NPCs: the patrol data the level actually carries - routes, their points, the patrol
    manager the map info holds and the spawners that ask for a patrol. */
void DisAIPatrolReport( UWorld* World );

/** Called from FDisComponentVisionNPC when a vision component starts seeing or stops seeing something. */
void DisAINoteSighting( const class AActor* _pSeer, const class AActor* _pSeen, UBOOL _bStart );

/** The sighting table, formatted for the census line. */
FString DisAISightingReport();

/** Once a second: what every patrolling brain holds - its route, index, direction, guard post and live sub-state - plus
    the distance to the nearest route and whether any route would accept it. */
FString DisAIPatrolState( UWorld* World );
