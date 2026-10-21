// agent JE (PHASE24 JE): -jediag, the Empress chain measurement - every HUD letterbox (bit 0x10) write with its writer, the Kismet
// teleport's destination and its navmesh test, the go-to stages, the face-to requests and the player's dialog state. Body in
// Src/jediag.cpp; nothing runs without the switch.
#ifndef DISHONORED_JEDIAG_H
#define DISHONORED_JEDIAG_H

UBOOL DisJEDiagEnabled();
FLOAT DisJEDiagNow();

#define JEDIAG( fmt, ... ) { if( DisJEDiagEnabled() ) { debugf( TEXT("DISHONORED(bringup): jediag t=%.2f " fmt), DisJEDiagNow(), __VA_ARGS__ ); } }

#endif
