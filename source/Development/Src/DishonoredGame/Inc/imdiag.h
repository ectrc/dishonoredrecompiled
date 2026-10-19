/*=============================================================================
	imdiag.h: agent IM (PHASE23 IM) - the -imdiag switch: the player's action base and anim states, measured.

	  imdiag init        the three player FSMs after PreBeginPlay_Actions (states, current), the three pickers FindStatePickers built
	                     (picker node, parent, filter, number of states in each map)
	  imdiag anim        every anim state the player sets (picker, requested name, state and tree, result, the custom sequences'
	                     alias -> sequence) and every picker cleared; a failed name once
	  imdiag fsm         the upper-body and left-arm machines' transitions and refusals (the master's are -ifdiag's)
	  imdiag action      each RequestAction (usage, action, its anim states, granted or refused and by what)
	  imdiag exit        each AnimState_TriggerExit / AnimState_Advance reaching the player (picker, attached state, time to go)
	  imdiag mantle      the animated mantle: Z, physics and master state every tick from the request until 1.5 s after the
	                     state is left, and a summary (rise, end physics, end state, collision)
	  IMSCAN [X Y Z]     console command (through the viewport client): the ledges in the finder's edge range (tweak min..max)
	                     within 3000 uu of the point (default the player), found by downward traces on a 48 uu grid, then each
	                     of the nearest 24 tested in turn - the player is put in front of it facing it and the real edge finder's
	                     answer is logged; at the end the player is put in front of the highest accepted one
	Nothing here changes behaviour except IMSCAN, which moves the player (FarMoveActor, like IGLEDGE) only under the switch.
	Body in Src/imdiag.cpp.
=============================================================================*/
#ifndef _INC_IMDIAG
#define _INC_IMDIAG

UBOOL DisIMDiagEnabled();
void DisIMNoteInit( class ADishonoredPlayerPawn* Pawn );
void DisIMNoteSetAnimState( class ADishonoredPlayerPawn* Pawn, INT StateType, FName Requested, const struct FDisPawnAnimState* State, INT Status, class UDishonoredNativeState* AttachToState );
void DisIMNoteClearAnimState( class ADishonoredPlayerPawn* Pawn, INT StateType, FLOAT BlendTime );
void DisIMNoteTransition( class UDishonoredNativeStateMachine* StateMachine, class UDishonoredNativeState* Last, class UDishonoredNativeState* Next );
void DisIMNoteRequest( class ADishonoredPlayerPawn* Pawn, BYTE ActionUsage, const struct FPawnAction* Action, UBOOL bGranted, const TCHAR* Why );
void DisIMNoteExit( const TCHAR* Kind, INT StateType, class UDishonoredNativeState* AttachedState, FLOAT TimeToGo );
void DisIMNoteMantleRequest( class ADishonoredPlayerPawn* Pawn, FLOAT EdgeHeight );
void DisIMReport( class ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime );
UBOOL DisIMDiagExec( const TCHAR* Cmd, FOutputDevice& Ar );

#define IMDIAG( Fmt, ... ) do { if( DisIMDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): imdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )
#define IMDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): im %s"), Msg ); } } while( 0 )

#endif
