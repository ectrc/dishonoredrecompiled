/*=============================================================================
	fzsoireediag.h: agent FZ (PHASE18 FZ) - the -fzdiag switch: the player master FSM's state, the camera's view, FOV and
	look limits once a second, and the one-shot notes on what the soiree path still lacks. Nothing here changes behaviour;
	every line is a debugf under the switch. Body in Src/fzsoireediag.cpp.
=============================================================================*/
#ifndef _INC_FZSOIREEDIAG
#define _INC_FZSOIREEDIAG

UBOOL DisFZDiagEnabled();
/** Once a second from ADishonoredPlayerCamera::UpdateViewTarget: FOV, view rotation, the core group's weights, the AnimDriven limits, the FSM state, the body mode. */
void DisFZReportCamera( class ADishonoredPlayerCamera* Camera, const struct FTViewTarget& OutVT, class APawn* Pawn, FLOAT DeltaTime );

#define FZDIAG( Fmt, ... ) do { if( DisFZDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): fzdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )
#define FZDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): fz %s"), Msg ); } } while( 0 )

#endif
