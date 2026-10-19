/*=============================================================================
	igdiag.h: agent IG (PHASE23 IG) - the -igdiag switch: the player's mantle, measured.

	  igdiag finder      each finder's attach (its FSM states, the tweaks it reads and their values); each change of a finder's
	                     result (spot: start, yaw, edge height, impact, crouch - or the reason there is none); every 5 s the
	                     steps and milliseconds the finder spent per tick (average and worst)
	  igdiag perform     each PerformMantle outcome (no spot / step-up / refused and why), with the spot and the pawn
	  igdiag stepup      each DoMantle_StepUp: from, start, edge height; then the pawn's Z, physics and master state every tick
	                     for 1.5 s and a summary (rise, end physics, end state)
	  igdiag mantle      UStatePlayerMasterMantle enter / exit
	  IGLEDGE [X Y Z YAW] console command (through the viewport client): without arguments, scan around the player for the nearest
	                     ledge in the step-up range and put the player in front of it facing it; with arguments, put the player
	                     there - a measurement aid like IFWATER, only under the switch
	Nothing here changes behaviour except IGLEDGE; every line is a debugf under the switch. Body in Src/igdiag.cpp.
=============================================================================*/
#ifndef _INC_IGDIAG
#define _INC_IGDIAG

UBOOL DisIGDiagEnabled();
void DisIGNoteFinderAttach( class UDishonoredMantleEdgeFinderComponent* Finder );
void DisIGNoteFinderResult( class UDishonoredMantleEdgeFinderComponent* Finder, UBOOL bValid, const TCHAR* Reason );
void DisIGNoteFinderTick( class UDishonoredMantleEdgeFinderComponent* Finder, INT NumSteps, FLOAT Milliseconds );
void DisIGNotePerformMantle( class ADishonoredPlayerPawn* Pawn, class UDishonoredMantleEdgeFinderComponent* Finder, const TCHAR* Outcome );
void DisIGNoteStepUp( class ADishonoredPlayerPawn* Pawn, const FVector& From, const FVector& Start, FLOAT Height, UBOOL bCameraBlend );
void DisIGNoteMantleState( class ADishonoredPlayerPawn* Pawn, UBOOL bEnter, FLOAT Height );
void DisIGReportMantle( class ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime );
UBOOL DisIGDiagExec( const TCHAR* Cmd, FOutputDevice& Ar );

#define IGDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): ig %s"), Msg ); } } while( 0 )

#endif
