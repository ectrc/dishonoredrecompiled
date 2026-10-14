// Agent GJ (PHASE20 GJ): the equip-path instrument. -gjdiag logs the inventory and the equipped items' meshes once a second from
// the player report (gfarmsdiag.cpp); GJDIAG_ONCE names, once, a retail callee this package could not make. Nothing here changes
// behaviour.
#ifndef GJEQUIPDIAG_H
#define GJEQUIPDIAG_H

UBOOL DisGJDiagEnabled();
void DisGJReportInventory( class ADishonoredPawn* Pawn );

#define GJDIAG_ONCE( Msg ) do { static UBOOL bSaidOnce = FALSE; if( !bSaidOnce ) { bSaidOnce = TRUE; debugf( NAME_Warning, TEXT("DISHONORED(bringup): gj %s"), Msg ); } } while( 0 )

#endif
