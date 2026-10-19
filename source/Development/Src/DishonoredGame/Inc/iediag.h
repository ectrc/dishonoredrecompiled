/*=============================================================================
	iediag.h: agent IE (PHASE23 IE) - the -iediag switch: every matinee dialog key (resolution, fire, done), every sync wait
	and its length, the audio listener once a second and the speaking components' propagation distances. Nothing here
	changes behaviour.
=============================================================================*/
#ifndef _INC_IEDIAG
#define _INC_IEDIAG

UBOOL DisIEDiagEnabled();

#define IEDIAG( Fmt, ... ) do { if( DisIEDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): iediag t=%.2f ") TEXT(Fmt), GWorld ? GWorld->GetTimeSeconds() : 0.f, __VA_ARGS__ ); } } while( 0 )

#endif
