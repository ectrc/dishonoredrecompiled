/*=============================================================================
	gfarmsdiag.h: agent GF (PHASE19 GF) - the -gfdiag switch. Once a second: the player's arms state (body mode, the mesh's depth
	group / LOD / offset / FOV, the three arm skel controls, the arm-follow weights, what the inventory holds) and, for every pawn
	with a body mesh, how far its posed skeleton departs from its reference skeleton (the stretched-silhouette measure: the
	component-space height of the pose against the reference pose's, the largest bone scale, the largest local translation ratio),
	with the mesh's own scale, the owner's draw scale, the base and the top-weight animation. Nothing here changes behaviour; every
	line is a debugf under the switch. Bodies in Src/gfarmsdiag.cpp.
=============================================================================*/
#ifndef _INC_GFARMSDIAG
#define _INC_GFARMSDIAG

UBOOL DisGFDiagEnabled();
/** Once a second per pawn, from ADishonoredPawn::Tick: the posed skeleton against the reference skeleton, the mesh and actor scales, the base. */
void DisGFReportPawn( class ADishonoredPawn* Pawn );
/** Once a second, from ADishonoredPlayerPawn::Tick: the arms. */
void DisGFReportPlayer( class ADishonoredPlayerPawn* Player );

#define GFDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): gf %s"), Msg ); } } while( 0 )

#endif
