/*-----------------------------------------------------------------------------
	Agent FK (PHASE15 package FK): -fkdiag. Body in Src/fkmatineediag.cpp.
-----------------------------------------------------------------------------*/

#ifndef DISHONORED_FKMATINEEDIAG_H
#define DISHONORED_FKMATINEEDIAG_H

/** TRUE when -fkdiag is on the command line; the switch is read once. */
UBOOL DisFKDiagEnabled();

/** The matinee-data and condition census. Called once a frame from ADishonoredPlayerController::Tick. */
void DisFKReport( class AActor* Owner );

#endif
