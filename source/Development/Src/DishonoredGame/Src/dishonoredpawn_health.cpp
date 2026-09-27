// DishonoredGame/src/dishonoredpawn_health.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x7aaad0  public: unsigned int __thiscall ADishonoredPawn::HasMinimumHealthEnabled(void)const
//   0x7aaae0  public: void __thiscall ADishonoredPawn::PostBeginPlay_Health(void)
//   0x7aca40  public: void __thiscall ADishonoredPawn::Tick_Health(float, enum ELevelTick)
//   0x7acab0  public: void __thiscall ADishonoredPawn::SetMinimumScriptedHeatlh(int)
//   0x7acae0  public: virtual void __thiscall ADishonoredPawn::TakeDamage(int, class AController *, class FVector, class FVector, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x7afa50  public: void __thiscall ADishonoredPawn::DoHUDHitReact(int, class FVector const &, class AActor *, class UClass *, class ADishonoredPawn *)
//   0x7c30c0  public: virtual void __thiscall ADishonoredPawn::Regen(float)
//   0x7c3230  public: void __thiscall ADishonoredPawn::DoCameraHitReact(int, class FVector const &, class AActor const *, class UClass * const)
//   0x7c3850  public: unsigned int __thiscall ADishonoredPawn::IsImmuneToDamageType(class UClass const *)const
//   0x7c5610  public: virtual void __thiscall ADishonoredPawn::TakeDamage_Native(int &, class AController *, class FVector, class FVector &, class UClass *, struct FTraceHitInfo, class AActor *)

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): the damage entry the fall damage lands through ----
//
// Porting the two fall natives made the walking path's last warn-once line appear rather than disappear:
// ADishonoredPawn::execTakeDamage never fired before, because TakeFallingDamage_Native was a stub that returned 0 and
// script therefore never asked for the damage to be applied. With 65 points of fall damage coming back it does, so the
// entry is ported here and the pawn actually loses health on a long fall.

// DISHONORED(written): 2013 rva 0x74a340 (2012 0x7acae0): the Arkane pass over the damage, then the engine's own
// APawn::TakeDamage, which is what subtracts it from Health and calls Died when it runs out.
// DISHONORED(port): the Arkane pass is TakeDamage_Native (2013 rva 0x75ccd0, 1,796 bytes), which takes Damage by
// reference and rewrites it - vulnerabilities (IDisVulnerabilityInterface), immunities (IsImmuneToDamageType 2012
// 0x7c3850), the minimum-health floor (HasMinimumHealthEnabled / SetMinimumScriptedHeatlh), the HUD and camera hit
// reactions (DoHUDHitReact, DoCameraHitReact) and the contact system. None of that subsystem is ported, so the damage
// reaches the engine unmodified, which for a fall is the whole of it.
void ADishonoredPawn::TakeDamage( INT Damage, AController* InstigatedBy, FVector HitLocation, FVector Momentum,
                                  UClass* DamageType, FTraceHitInfo HitInfo, AActor* DamageCauser )
{
	const INT HealthBefore = Health;
	APawn::TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	if( UDisAttributes::IsCensusEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): disattrib damage %s: %i of type %s from %s, health %i -> %i"),
			*GetName(), Damage, DamageType ? *DamageType->GetName() : TEXT("none"),
			DamageCauser ? *DamageCauser->GetName() : TEXT("none"), HealthBefore, Health );
	}
}

// DISHONORED(written): the generated exec wrapper. Retail's is ICF-folded onto AActor::execTakeDamage (2013 rva
// 0x1c1640) because it is the same seven parameters dispatched through the virtual, which is exactly what
// APawn::execTakeDamage already does in Engine/Src/UnPawn.cpp.
void ADishonoredPawn::execTakeDamage( FFrame& Stack, RESULT_DECL )
{
	AActor::execTakeDamage( Stack, Result );
}
