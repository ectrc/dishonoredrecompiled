// DishonoredGame/src/dishonorednpcpawn_rotation.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x7c9570  public: void __thiscall ADishonoredNPCPawn::Init_Rotation(void)
//   0x7c95a0  public: void __thiscall ADishonoredNPCPawn::CancelAllNPCRotations(void)
//   0x7c95e0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::SetDesiredRotation(class FRotator, unsigned int, unsigned int, float, unsigned int)
//   0x7c95f0  private: virtual void __thiscall ADishonoredNPCPawn::physicsRotation(float, class FVector)
//   0x7cbee0  public: void __thiscall FDisNPCRotationIntent::SetTargetRotation(class ADishonoredNPCPawn * const, class FVector const &, float, float, enum EDisFaceToPriority, void * const, class FName)
//   0x7cbf90  public: void __thiscall ADishonoredNPCPawn::SetNPCRotationFocus(enum EDisFaceToPriority, class FVector const &, float, float, void * const, class FName)
//   0x7cc020  public: void __thiscall ADishonoredNPCPawn::LockNPCRotation(enum EDisFaceToPriority, void * const, class FName)
//   0x7cc0f0  private: void __thiscall ADishonoredNPCPawn::OnUnlockNPCRotation(enum EDisFaceToPriority)
//   0x7ce4a0  public: void __thiscall ADishonoredNPCPawn::SetNPCRotation(enum EDisFaceToPriority, class FRotator const &, float, float, void * const, class FName)
//   0x7ce540  public: void __thiscall ADishonoredNPCPawn::UnlockNPCRotation(enum EDisFaceToPriority)
//   0x7d4d40  public: void __thiscall ADishonoredNPCPawn::Tick_Rotation(float)

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x76e960 (2012 0x7c95e0): retail's ADishonoredNPCPawn::SetDesiredRotation is an 8-byte
// `mov eax,1 / ret 14h`. The class overrides the script-facing entry point and does NOT route it into the
// rotation-intent system: an NPC's facing comes from m_NPCRotationIntent through the rotation tick, so a script or
// Kismet request to set the desired rotation directly is accepted and then ignored. The 2013 body is the same eight
// bytes as the 2012 one, so this is retail behaviour and not a stripped build.
void ADishonoredNPCPawn::execSetDesiredRotation( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FRotator, _TargetDesiredRotation);
	P_GET_UBOOL(_bInLockDesiredRotation);
	P_GET_UBOOL(_bInUnlockWhenReached);
	P_GET_FLOAT(_InterpolationTime);
	P_GET_UBOOL(_bResetRotationRate);
	P_FINISH;
	*(UBOOL*)Result = SetDesiredRotation( _TargetDesiredRotation, _bInLockDesiredRotation, _bInUnlockWhenReached, _InterpolationTime, _bResetRotationRate );
}

UBOOL ADishonoredNPCPawn::SetDesiredRotation( FRotator _TargetDesiredRotation, UBOOL _bInLockDesiredRotation, UBOOL _bInUnlockWhenReached, FLOAT _InterpolationTime, UBOOL _bResetRotationRate )
{
	return TRUE;
}
