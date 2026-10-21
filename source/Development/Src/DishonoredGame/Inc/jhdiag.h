/*=============================================================================
	jhdiag.h: agent JH (PHASE24 JH) - the -jhdiag switch: the interaction prompt and the hover glow, measured.

	  jhdiag invoke     every Invoke the prompt makes (crosshairInfosTxt_mc.SetText, interactions_window_mc / context / special /
	                    QTE SetInteractions) with its arguments, the frame and the world time
	  jhdiag highlight  each ShowHighlight / HideHighlight / refused DoHighlight with the actor, its highlight flags, the frame
	  jhdiag focus      each crosshair actor change with the frame (beside -gddiag's line)
	  jhdiag census     once per map: every actor implementing the interactable interface, its name / interact texts
	  jhdiag fill       at most once a second while the prompt is shown: the context's four groups
	-jhface=<actor name or class:<Class>>@<seconds>[@<dist>]: at that world time the player is placed <dist> uu (default 120)
	from the actor's bounds centre, on the side facing the player's current position, looking at the centre (pitch included);
	several orders separated by commas. Nothing here changes behaviour. Body in Src/jhdiag.cpp.
=============================================================================*/
#ifndef _INC_JHDIAG
#define _INC_JHDIAG

UBOOL DisJHDiagEnabled();
void DisJHLogHighlight( class IDisInteractableInterface* _pInteractable, const TCHAR* _What, INT _State );
void DisJHLogInvoke( const TCHAR* _Clip, const TCHAR* _Method, const FString& _Args );
void DisJHLogFocus( class AActor* _pOld, class AActor* _pNew, INT _Status, UBOOL _bCanInteract );
void DisJHLogContext( const struct FDisUIInteractionContext& _rContext, const FString& _rName, const FString& _rFocusText );
void DisJHReport( class ADishonoredPlayerController* _pPC );

#endif
