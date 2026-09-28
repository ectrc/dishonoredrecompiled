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
