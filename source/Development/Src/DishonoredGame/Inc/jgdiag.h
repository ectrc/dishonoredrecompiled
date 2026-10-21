/*=============================================================================
	jgdiag.h: agent JG (PHASE24 JG) - the -jgdiag switch: the jump's camera, measured.

	  jgdiag react     the physical react's spring every gather while awake: weight, the two points against their bound positions,
	                   the strength point's velocity, the view offset and tilt it gives, the arm bounce; its PostBeginPlay and impacts
	  jgdiag player    every frame from the jump until 1.5 s after the landing: pawn Z and vertical speed, physics, master state, the
	                   bump smoother's and the physical react's weight / target / active / sleeping
	  jgdiag smoother  each SetWeight the jump and falling states give the bump smoother
	  jgdiag land      each landing: velocity, the contact type chosen, the camera impact, the FOV target cleared, the Kismet event
	  jgdiag attrib    ApplyAttributes' results: the player on every change, each NPC once
	  jgdiag NAN       a non-finite spring value (once)
	The view itself is -ipdiag's per-pass camera line (pov and pawn Z). Nothing here changes behaviour. Body in Src/jgdiag.cpp.
=============================================================================*/
#ifndef _INC_JGDIAG
#define _INC_JGDIAG

UBOOL DisJGDiagEnabled();
void DisJGNotePhysicalReact( class UDishonoredCamera_PhysicalReact* React, const TCHAR* What );
void DisJGNoteSpringStep( class UDishonoredCamera_PhysicalReact* React );
void DisJGNoteReactOutput( class UDishonoredCamera_PhysicalReact* React, const struct FDishonoredViewTarget& OutVT );
void DisJGNoteSmootherWeight( class ADishonoredPawn* Pawn, const TCHAR* Where, FLOAT TargetWeight, FLOAT BlendTime );
void DisJGNoteLanded( class ADishonoredPlayerPawn* Pawn, const FVector& Velocity, class UClass* ContactType, UBOOL bImpact, UBOOL bCamera );
void DisJGNoteAttributes( class ADishonoredPawn* Pawn );
void DisJGReportPlayer( class ADishonoredPlayerPawn* Pawn );

#define JGDIAG( Fmt, ... ) do { if( DisJGDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): jgdiag ") Fmt, __VA_ARGS__ ); } } while( 0 )
#define JGDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): jg %s"), Msg ); } } while( 0 )

#endif
