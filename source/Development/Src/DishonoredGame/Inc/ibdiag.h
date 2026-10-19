// agent IB (PHASE23 IB): -ibdiag, the ride NPCs' locomotion once a second. Build 1's A/B switches (-ibphystick = the physWalking tick of
// build/agentHC/revert_step5_loco.py, -ibnovel = HEAD's velocity source) are gone from build 2; resources/docs/agents/agentIB.md.
// Defined in Src/dishonorednpcpawn_locomotion.cpp.
#ifndef DISHONORED_IBDIAG_H
#define DISHONORED_IBDIAG_H

UBOOL DisIBDiagEnabled();
/** -ibdiag watches the ride's captain (_26), tiller guard (_27), WorkerB (_20) and the walker HC measured (_3). */
UBOOL DisIBDiagWatched( const AActor* _pActor );
void DisIBDiagNoteGoTo( const AActor* _pPawn, INT _Stage, UBOOL _bPathingFail );
/** Once a second per watched NPC, from ADishonoredNPCPawn::Tick. */
void DisIBDiagReport( class ADishonoredNPCPawn* _pPawn );

/** agent IK (PHASE23 IK): -ikdiag, the watched NPCs' moves, root motion and start-poly decisions (agentIK.md). */
UBOOL DisIKDiagEnabled();
UBOOL DisIKDiagWatched( const AActor* _pActor );

#endif
