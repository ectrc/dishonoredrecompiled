/*=============================================================================
	fwdiag.h: agent FW (PHASE18 FW) - the player's walk to the Empress, measured.

	  -fwdiag        the player pawn's location, heading, velocity, physics and base once a second (DisFWReport, from the
	                 player controller's tick beside -fkdiag)
	  -fwdialogmap   once per map, on -fkdiag's passes: every DisDialogOneShot's tree walked forward from its hook nodes -
	                 which Kismet input name or which distance / look-at / loiter hook leads to which conversation, which
	                 soiree node and so which matinee, and which Kismet remote events a conversation raises on its way
	Nothing here changes behaviour; every line is a debugf under a switch.
=============================================================================*/
#ifndef _INC_FWDIAG
#define _INC_FWDIAG

UBOOL DisFWDiagEnabled();
void DisFWReport( class APlayerController* PC );
void DisFWReportDialogMap();

#endif
