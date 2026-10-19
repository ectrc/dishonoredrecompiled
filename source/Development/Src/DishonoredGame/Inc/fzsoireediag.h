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

UBOOL DisPCamDiagEnabled();
/** agent HD -pcamdiag: once a second, and every frame for two seconds after a soiree exit, the controller/pawn/mesh/bone/view yaw and the camera reset state. */
void DisPCamReport( class ADishonoredPlayerCamera* Camera, const struct FTViewTarget& OutVT, class APawn* Pawn );
/** agent HD -pcamdiag: opens the dense window and replays the eight frames before the exit. */
void DisPCamNoteSoireeExit();
#define PCAMDIAG( Fmt, ... ) do { if( DisPCamDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): pcamdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )

UBOOL DisIADiagEnabled();
/** agent IA -iadiag: once a second from ADishonoredPlayerCamera::TickFOV: DefaultFOV, the settings FOV, the current view and arms FOV and every FOV target slot holding a value. */
void DisIAReportFOV( class ADishonoredPlayerCamera* Camera );
#define IADIAG( Fmt, ... ) do { if( DisIADiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): iadiag ") Fmt, __VA_ARGS__ ); } } while( 0 )

#define FZDIAG( Fmt, ... ) do { if( DisFZDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): fzdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )
#define FZDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): fz %s"), Msg ); } } while( 0 )

#endif
