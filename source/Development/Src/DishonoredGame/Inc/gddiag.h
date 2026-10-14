// DishonoredGame/Inc/gddiag.h
// DISHONORED(bringup): agent GD (PHASE19 GD). -gddiag: the crosshair's changes, the Use interactions and the dialog hook they
// fire, the use FSM's state once a second; -gdface=<actor>@<seconds>[@<dist>]: at that world time the player pawn is placed
// <dist> uu (default 150) in front of the named actor, facing it, so a scripted run can press Use on him without walking;
// -gduse=<seconds>[@<hold>]: the Use button is held for <hold> s (default 0.5) from that time, beside game_input.py's `key F`.
// Free when the switches are absent.
#ifndef _INC_GDDIAG
#define _INC_GDDIAG

UBOOL DisGDDiagEnabled();
void DisGDReport( class ADishonoredPlayerController* PC );
class IDisInteractableInterface* DisGDInteractableOf( class AActor* _pActor );

#endif
