#pragma once
// DishonoredGame/inc/disheadcensus.h
// DISHONORED(written): agent DI. The -dishead census, the measurement half of the modular-character package: how many
// spawned NPC pawns have a head mesh component, whether it is attached, whether a USkeletalMesh is set on it, whether
// it has materials and whether it is visible. Modelled on agent CG's -disai and agent AU's -dispickup: free when the
// switch is absent, hung off the same per-frame script call, no engine file touched for it.

class UWorld;

/** TRUE when -dishead is on the command line (resolved on first use, never at static-init time). */
UBOOL DisHeadCensusEnabled();

/** Once a second, per world: the head-mesh census line for every ADishonoredNPCPawn in the world. */
void DisHeadReport( UWorld* World, FLOAT DeltaSeconds );

/** Bumped by ADishonoredNPCPawn::PostBeginPlay_Body so the census can report flow, not only state. */
extern INT GDisHeadBodyPasses;
extern INT GDisHeadMeshesSet;
extern INT GDisHeadMeshesDetached;

/** TRUE when -disheadcam is on the command line (resolved once). */
UBOOL DisHeadCamEnabled();

/** -disheadcam[=<seconds>]: from that world time on, hold one spawned NPC in front of the player's own view point so a
    screenshot shows its head from close up. Instrumentation, behind the switch, called from DisHeadReport. */
void DisHeadFrameOneNPC( class ADishonoredPlayerController* Controller, class UWorld* World );

/** TRUE when -disnohead is on the command line: ADishonoredNPCPawn::PostBeginPlay_Body then skips the head mesh, which
    reproduces the defect this package fixed on the same binary, so the before/after pair is one controlled change. */
UBOOL DisHeadSuppressed();
