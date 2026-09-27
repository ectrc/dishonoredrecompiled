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
