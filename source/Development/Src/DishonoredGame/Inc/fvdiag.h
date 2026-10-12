// agent FV (PHASE18 FV): -fvdiag, the go-to and basing measurement. Defined in Src/dishonorednpccontroller.cpp.
#ifndef DISHONORED_FVDIAG_H
#define DISHONORED_FVDIAG_H

UBOOL DisFVDiagEnabled();

#define FVDIAG( fmt, ... ) { if( DisFVDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): fvdiag " fmt), __VA_ARGS__ ); } }

#endif
