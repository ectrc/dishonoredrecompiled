/*=============================================================================
	iddiag.h: agent ID (PHASE23 ID) - the -iddiag switch: every blurb start / done / interrupt, every subtitle add / kill /
	part and every subtitles_mc Invoke, with world times. Nothing here changes behaviour.
=============================================================================*/
#ifndef _INC_IDDIAG
#define _INC_IDDIAG

UBOOL DisIDDiagEnabled();

#define IDDIAG( Fmt, ... ) do { if( DisIDDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): iddiag t=%.2f ") TEXT(Fmt), GWorld ? GWorld->GetTimeSeconds() : 0.f, __VA_ARGS__ ); } } while( 0 )

#endif
