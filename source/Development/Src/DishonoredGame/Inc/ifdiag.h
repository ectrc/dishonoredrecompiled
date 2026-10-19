/*=============================================================================
	ifdiag.h: agent IF (PHASE23 IF) - the -ifdiag switch: the player's crouch, sprint, jump and swim, measured.

	  ifdiag action      each game-binding action the movement code receives (sneak/slide, sprint, jump) and what it did
	  ifdiag height      the collision and eye height before and after every crouch-height change
	  ifdiag speed       the MaxSpeedModifier inputs and result, for the player on every change, for an NPC the first time
	  ifdiag fsm         the player master FSM: each transition (from, to) and each refused request (state, refused by)
	  ifdiag water       water volume enter/exit and every player physics change
	  ifdiag player      four times a second: location, 2D speed, physics, collision/eye height, sneaking, master state
	  IFWATER [EXIT]     console command (through the viewport client): put the player just above the nearest
	                     DishonoredWaterVolume's surface, or back where it was - a measurement aid, not a retail command
	Nothing here changes behaviour; every line is a debugf under the switch. Body in Src/ifdiag.cpp.
=============================================================================*/
#ifndef _INC_IFDIAG
#define _INC_IFDIAG

UBOOL DisIFDiagEnabled();
void DisIFReportPlayer( class ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime );
void DisIFNoteSpeedModifier( class ADishonoredPawn* Pawn, FLOAT EngineModifier, FLOAT DerivedModifier, FLOAT Result );
void DisIFNoteTransition( class UDishonoredNativeStateMachine* StateMachine, class UDishonoredNativeState* Last, class UDishonoredNativeState* Next );
void DisIFNoteRejection( class UDishonoredNativeStateMachine* StateMachine, const UClass* StateID, class UDishonoredNativeState* RejectingState );
UBOOL DisIFDiagExec( const TCHAR* Cmd, FOutputDevice& Ar );

#define IFDIAG( Fmt, ... ) do { if( DisIFDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): ifdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )
#define IFDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): if %s"), Msg ); } } while( 0 )

#endif
