/*=============================================================================
	frdialogdiag.h: agent FR (PHASE17 FR) - the -frdiag switch, a trace of the conversation system's direct path.
	Nothing here changes behaviour; every line is a debugf under the switch.
=============================================================================*/
#ifndef _INC_FRDIALOGDIAG
#define _INC_FRDIALOGDIAG

UBOOL DisFRDiagEnabled();
/** -fkdiag's report hook (fkmatineediag.cpp): the one-shot actors, their speakers and the components that bound them. */
void DisFRReportDialog();

#define FRDIAG( Fmt, ... ) do { if( DisFRDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): frdiag ") TEXT(Fmt), __VA_ARGS__ ); } } while( 0 )

#endif
