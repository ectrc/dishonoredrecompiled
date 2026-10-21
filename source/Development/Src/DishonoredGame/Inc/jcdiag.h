// agent JC (PHASE24 JC): -jcdiag, the matinee scene placement (special root-motion keys, FaceTo and AIControl keys), and the build-1
// A/B switches: -jcoldrm puts the special root-motion keys back on the pre-JC path (mesh RMM_Translate, the cumulative delta applied
// every frame, no backup side effects), -jcnofaceto leaves the FaceTo track inert, -jcnoaicontrol the AIControl track.
// Defined in Src/dishonoredpawn.cpp.
#ifndef DISHONORED_JCDIAG_H
#define DISHONORED_JCDIAG_H

UBOOL DisJCDiagEnabled();
UBOOL DisJCOldRootMotion();
UBOOL DisJCNoFaceTo();
UBOOL DisJCNoAIControl();
/** -jcwatch=<names>: the watched pawns' place in the local player's view, from UDisSkeletalMeshComponent::TickEnd. */
void DisJCDiagReportView( const class ADishonoredPawn* Pawn, const FVector& MeshCenter );

#endif
