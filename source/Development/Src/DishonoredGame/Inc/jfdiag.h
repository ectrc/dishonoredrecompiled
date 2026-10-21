// agent JF (PHASE24 JF): -jfdiag, the hide-and-seek choice measurement - the choice node's play and ticks, the dialog state's choice
// half, the bend-time channel the choice pauses, the HUD's choice list and the confirm / skip inputs. Body in Src/jfdiag.cpp; nothing
// runs without the switch.
#ifndef DISHONORED_JFDIAG_H
#define DISHONORED_JFDIAG_H

UBOOL DisJFDiagEnabled();
FLOAT DisJFDiagNow();

#define JFDIAG( fmt, ... ) { if( DisJFDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): jfdiag t=%.2f " fmt), DisJFDiagNow(), __VA_ARGS__ ); } }

#endif
