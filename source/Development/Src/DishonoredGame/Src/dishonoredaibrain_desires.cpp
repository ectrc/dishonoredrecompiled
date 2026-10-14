// DishonoredGame/src/dishonoredaibrain_desires.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x746110  public: class FVector __thiscall UDishonoredAIBrain::GetCurrentDesiredPosition(void)const
//   0x749d10  public: enum ETransitSpeed __thiscall UDishonoredAIBrain::GetCurrentDesiredTransitSpeed(void)const
//   0x74d880  public: unsigned int __thiscall UDishonoredAIBrain::GetActionTargetLocation(class FVector &)const
//   0x74d8a0  public: class AActor * __thiscall UDishonoredAIBrain::GetActionTargetActor(void)const

// ---- agent GE ports (PHASE19 GE): the one accessor the head-track pause needs; the rest of this unit is still skeleton ----
#include "DishonoredGame.h"
#include "arkcomponentlocomotion.h"

// DISHONORED(port): 2013 rva 0x700eb0 (2012 0x746110, 33 B): where the locomotion component's active request is going
FVector UDishonoredAIBrain::GetCurrentDesiredPosition() const
{
	FArkComponentLocomotion* Locomotion = m_pOwningPawn ? m_pOwningPawn->GetComponentLocomotion() : NULL;
	return Locomotion ? Locomotion->GetActiveRequestTargetedLocation() : FVector( 0.f, 0.f, 0.f );
}
