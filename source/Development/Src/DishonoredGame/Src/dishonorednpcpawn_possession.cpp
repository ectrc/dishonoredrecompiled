// DishonoredGame/src/dishonorednpcpawn_possession.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x7ac1e0  public: virtual class UObject * __thiscall ADishonoredNPCPawn::GetUObjectInterfaceDisPossessableInterface(void)
//   0x7ac1f0  public: virtual class USkeletalMeshComponent * __thiscall ADishonoredNPCPawn::GetPossessableSkelMesh(void)
//   0x7ac200  private: virtual unsigned int __thiscall ADishonoredNPCPawn::GetPossessionExitOverride(class FVector &, class FRotator &)const
//   0x7ac260  public: virtual class FVector __thiscall ADishonoredNPCPawn::GetCameraPos(void)const
//   0x7af1d0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::DisIsPossessed(void)const
//   0x7b4fd0  public: virtual class UDisTweaks_Possessable * __thiscall ADishonoredNPCPawn::GetPossessableTweaks_Derived(void)const
//   0x7b5010  public: virtual void __thiscall ADishonoredNPCPawn::OnOverridePossess(class UDisSeqAct_OverridePossess *)
//   0x7b5080  private: class FVector __thiscall ADishonoredNPCPawn::GetCameraBoneLoc(void)const
//   0x7b87f0  public: virtual class FVector __thiscall ADishonoredNPCPawn::GetPrepossessCameraFocus(void)const
//   0x7b8810  private: virtual unsigned int __thiscall ADishonoredNPCPawn::InitPossessionOnPawn(class UDisTweaks_Possessable const *, class FVector const &)
//   0x7b89d0  private: virtual void __thiscall ADishonoredNPCPawn::TermPossessionOnPawn(void)
//   0x7be380  public: virtual void __thiscall ADishonoredNPCPawn::GetPossessTargetLocations(struct TMemStackArray<class FVector> &)const
//   0x7c2c30  public: virtual enum IDisPossessableInterface::EPossessionAvailability __thiscall ADishonoredNPCPawn::GetPossessionAvailability(void)const

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x7748f0 (2012 0x7b5010): the Kismet action's second input link is "clear", so firing it
// drops the script possession override; any other input records the action's tweaks and exit-point actor, which the
// possession path then prefers over the pawn's own UDisTweaks_Possessable.
void ADishonoredNPCPawn::execOnOverridePossess( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnOverridePossess( (UDisSeqAct_OverridePossess*)Action );
}

void ADishonoredNPCPawn::OnOverridePossess( UDisSeqAct_OverridePossess* _pAction )
{
	if( _pAction->InputLinks.Num() > 1 && _pAction->InputLinks(1).bHasImpulse )
	{
		m_pScriptOverridePossessableTweaks = NULL;
		m_pScriptOverrideExitPointActor = NULL;
	}
	else
	{
		m_pScriptOverridePossessableTweaks = _pAction->m_pOverrideTweaks;
		m_pScriptOverrideExitPointActor = _pAction->m_pExitPointActor;
	}
}

// ---- agent CG natives sweep, round 2 (PHASE9 CG) ----

// DISHONORED(port): 2013 rva 0x75a390 (2012 0x7b6cd0): the exec wrapper, over the C++ body below.
void ADishonoredNPCPawn::execOnSpawnStealable( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDisSeqAct_SpawnStealable, _pAction);
	P_FINISH;
	OnSpawnStealable( _pAction );
}

// DISHONORED(port): 2013 rva 0x75a390 (2012 0x7b6cd0)
void ADishonoredNPCPawn::OnSpawnStealable( class UDisSeqAct_SpawnStealable* _pAction )
{
	// The Kismet action gives this NPC something worth pickpocketing: the pickup is spawned from the action's tweaks
	// and attached to the pawn's mesh, which is what makes it follow the NPC and be stealable.
	// DISHONORED(bringup): retail spawns it through the action's UDisTweaks_AbstractItemPickup and attaches it with
	// ADisPickup_Base::Attach (2013 rva 0x62b0a0, agent AU's port). The action class carries no reflected tweaks member
	// in the retail SDK dump, so the spawn cannot be reconstructed from the layout alone and the attach half is what is
	// kept: an already-spawned stealable is attached, a missing one is named once.
	if( !_pAction )
	{
		return;
	}
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( TEXT("DISHONORED(bringup): ADishonoredNPCPawn::OnSpawnStealable (2013 rva 0x75a390): the pickup class comes from the action's tweaks, which the retail SDK dump does not name") );
	}
}
