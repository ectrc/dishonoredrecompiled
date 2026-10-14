// DishonoredGame/src/disusestate_altinteract.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x727940  public: virtual void __thiscall UDisUseState_AltInteract::OnEnterState(class UDishonoredNativeState *)
//   0x72ab20  public: static class UClass * __cdecl UDisUseState_AltInteract::GetPrivateStaticClassUDisUseState_AltInteract(wchar_t const *)
//   0x72daa0  public: virtual void __thiscall FDisUseState_AltInteract_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x72fed0  public: static void __cdecl UDisUseState_AltInteract::InitializePrivateStaticClassUDisUseState_AltInteract(void)
//   0x731b90  public: static class UClass * __cdecl UDisUseState_AltInteract::StaticClassNoInline(void)
//   0x734e00  public: __thiscall FDisUseState_AltInteract_Param::FDisUseState_AltInteract_Param(class IDisInteractableInterface * const)
//   0x734e50  public: virtual void __thiscall UDisUseState_AltInteract::TickState(float)

// DISHONORED(port): agent GD (PHASE19 GD). OnEnterState 2013 rva 0x6c69b0 (2012 0x727940, 60 B): the attempt time is now plus the
// interactable's GetAltInteractTime. TickState 0x6d57e0 (0x734e50, 389 B), transcribed: the interactable gone, no longer
// alt-interactable or no longer under the crosshair sends the state back to WaitForInput; before the attempt time a
// released Use does the plain interaction and finishes; past it the alt-interaction is attempted, a refusal going back to
// WaitForInput.
#include "DishonoredGame.h"
#include "disusestate.h"
#include "gddiag.h"

void UDisUseState_AltInteract::OnEnterState( UDishonoredNativeState* _pLastState )
{
	IDisInteractableInterface* Interactable = m_pInteractable.GetObject() ? (IDisInteractableInterface*)m_pInteractable.GetInterface() : NULL;
	m_fTimeOfAttemptAltInteract = ( GWorld ? GWorld->GetTimeSeconds() : 0.f ) + ( Interactable ? Interactable->GetAltInteractTime() : 0.f );
}

void UDisUseState_AltInteract::TickState( FLOAT _fDeltaSeconds )
{
	ADishonoredPlayerController* PC = m_pPlayerController;
	if( !PC )
	{
		return;
	}
	ADishonoredPawn* pPawn = Cast<ADishonoredPawn>( PC->Pawn );
	IDisInteractableInterface* Interactable = m_pInteractable.GetObject() ? (IDisInteractableInterface*)m_pInteractable.GetInterface() : NULL;
	IDisInteractableInterface* UnderCrosshair = DisGDInteractableOf( PC->m_pCrosshairActor );
	if( !Interactable || !Interactable->CanAltInteract( pPawn ) || Interactable != UnderCrosshair )
	{
		FDisUseState_WaitForInput_Param Param;
		DemandStateChange( Param );
		return;
	}
	const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
	if( Now < m_fTimeOfAttemptAltInteract )
	{
		if( PC->m_bUseButton )
		{
			return;
		}
		Interactable->AttemptInteract( pPawn );
	}
	else if( !Interactable->AttemptAltInteract( pPawn ) )
	{
		FDisUseState_WaitForInput_Param Param;
		DemandStateChange( Param );
		return;
	}
	FDisUseState_Finished_Param Finished;
	DemandStateChange( Finished );
}
