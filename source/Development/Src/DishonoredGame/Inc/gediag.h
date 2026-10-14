/*=============================================================================
	gediag.h: agent GE (PHASE19 GE) - the camera fade, the Kismet links behind it, and the player's deaths, measured.

	  -gediag            once a second (from the player controller's tick beside -fwdiag): the player camera's fade state
	                     (bEnableFading, FadeAmount, FadeAlpha, FadeTime/Remaining, FadeColor) and every active
	                     SeqAct_CameraFade with its opacity, time, persist flag and object version
	  -gelinks=<substr>[,<substr>]  once per map, on -fkdiag's passes: every Kismet op whose path contains a pattern - its
	                     output links with their targets, and every property that differs from the class default
	  -gedeathdiag       every TakeDamage on a pawn and every APawn::Died, with the damage event named (damage, type,
	                     instigator, causer, hit location, momentum, health before/after, the pawn's location, velocity,
	                     physics, base) and the PhysX scene's static shape count at that moment
	Nothing here changes behaviour; every line is a debugf under a switch.
=============================================================================*/
#ifndef _INC_GEDIAG
#define _INC_GEDIAG

UBOOL DisGEDiagEnabled();
UBOOL DisGEDeathDiagEnabled();
void DisGEReport( class APlayerController* PC );
void DisGEReportLinks();
void DisGEReportDamage( class APawn* Pawn, INT Damage, class AController* InstigatedBy, const FVector& HitLocation, const FVector& Momentum,
	class UClass* DamageType, class AActor* DamageCauser, INT HealthBefore );

#endif
