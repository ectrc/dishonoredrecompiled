/*=============================================================================
	gbdiag.h: agent GB (PHASE19 GB) - the ride boat's Move track against the dock, measured.

	  -gbdiag   once per live Move track instance: every key - its relative position, euler, lookup group and the WORLD
	            position the runtime puts the actor at for that key (the group's reference frame applied); once a second:
	            each moving group's actor (location, rotation, base) against the track's own evaluation at the matinee
	            position, and the dock actors by name (SkeletalMeshActorMAT_1/_2, SkeletalMeshActor_2, InterpActor_3)
	Nothing here changes behaviour; every line is a debugf under the switch.
=============================================================================*/
#ifndef _INC_GBDIAG
#define _INC_GBDIAG

UBOOL DisGBDiagEnabled();
void DisGBReport();

#endif
