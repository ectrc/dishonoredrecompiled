// DishonoredGame/src/disusestate_waitforinput.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x72add0  public: static class UClass * __cdecl UDisUseState_WaitForInput::GetPrivateStaticClassUDisUseState_WaitForInput(wchar_t const *)
//   0x72db70  public: static void __cdecl UDisUseState_WaitForInput::InitializePrivateStaticClassUDisUseState_WaitForInput(void)
//   0x72ff50  public: static class UClass * __cdecl UDisUseState_WaitForInput::StaticClassNoInline(void)
//   0x731c20  public: __thiscall FDisUseState_WaitForInput_Param::FDisUseState_WaitForInput_Param(void)
//   0x735210  public: virtual void __thiscall UDisUseState_WaitForInput::TickState(float)

// DISHONORED(port): agent GD (PHASE19 GD). TickState 2013 rva 0x6d5ba0 (2012 0x735210, 639 B), transcribed; the order is retail's.
// Nothing happens until the Use button is down and use input is enabled (mask 0x80). A player pawn first drops a level-2 zoom
// and ends a climb; then the crosshair actor, if the player can interact with it: a CANNOT_USE status asks it for the
// cannot-use interaction, an alt-interactable goes to the AltInteract state, anything else AttemptInteract - which on an
// NPC is ADishonoredNPCPawn::AttemptInteract_Derived and the brain's HandleUsedByOther. Then a held movable object is
// dropped, a power's active contexts cancelled, the last latent interactable ended, and failing all of that the holster
// state is entered when the weapons can be holstered. Every road ends in the Finished state.
// DISHONORED(bringup), each named: UDishonoredInventoryItem::IsItemZoomed / OnChangeZoomLevel and the ZoomLevel2 attribute,
// ADishonoredPlayerPawn::AttemptEndClimb, DropMovableObject, UDishonoredInventoryItem::CancelActiveContexts,
// ADishonoredPlayerController::CanHolsterWeapons (so the Holster state is never entered from here).
#include "DishonoredGame.h"
#include "disusestate.h"
#include "gddiag.h"

void UDisUseState_WaitForInput::TickState( FLOAT _fDeltaSeconds )
{
	ADishonoredPlayerController* PC = m_pPlayerController;
	if( !PC || !PC->m_bUseButton )
	{
		return;
	}
	if( !PC->IsInputEnabled( 0x80 ) )
	{
		if( DisGDDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): gddiag use pressed but use input is disabled (mask 0x80; bIgnoreLookInput %d bIgnoreMoveInput %d)"), (INT)PC->bIgnoreLookInput, (INT)PC->bIgnoreMoveInput );
		}
		return;
	}
	ADishonoredPawn* pPawn = Cast<ADishonoredPawn>( PC->Pawn );
	ADishonoredPlayerPawn* pPlayerPawn = Cast<ADishonoredPlayerPawn>( PC->Pawn );
	UBOOL bFinished = FALSE;
	if( pPlayerPawn )
	{
		UDishonoredNativeState* Current = pPlayerPawn->GetPlayerMasterFSM() ? pPlayerPawn->GetPlayerMasterFSM()->GetCurrentlyActiveState() : NULL;
		if( pPlayerPawn->m_pClimbable && !( Current && Current->IsA( UStatePlayerMasterJump::StaticClass() ) ) )
		{
			static UBOOL bNoted = FALSE;
			if( !bNoted )
			{
				bNoted = TRUE;
				debugf( TEXT("DISHONORED(bringup): UDisUseState_WaitForInput: ADishonoredPlayerPawn::AttemptEndClimb is not ported; Use on a climbable ends here") );
			}
			bFinished = TRUE;
		}
	}
	if( !bFinished && PC->m_pCrosshairActor && PC->m_bCanInteractWithCrosshairActor )
	{
		IDisInteractableInterface* Interactable = DisGDInteractableOf( PC->m_pCrosshairActor );
		if( Interactable )
		{
			UBOOL bHandled = FALSE;
			if( PC->m_CrosshairStatus == CHS_OVER_CANNOT_USE )
			{
				bHandled = Interactable->AttemptCannotUseInteract( pPawn );
			}
			else if( Interactable->CanAltInteract( pPawn ) )
			{
				if( DisGDDiagEnabled() )
				{
					debugf( TEXT("DISHONORED(bringup): gddiag use on %s: alt-interact"), *PC->m_pCrosshairActor->GetName() );
				}
				FDisUseState_AltInteract_Param Param( Interactable );
				DemandStateChange( Param );
				return;
			}
			else
			{
				bHandled = Interactable->AttemptInteract( pPawn );
			}
			if( DisGDDiagEnabled() )
			{
				debugf( TEXT("DISHONORED(bringup): gddiag use on %s status %d -> handled %d"), *PC->m_pCrosshairActor->GetName(), (INT)PC->m_CrosshairStatus, (INT)bHandled );
			}
			bFinished = bHandled;
		}
	}
	if( DisGDDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): gddiag use tick: crosshair %s canInteract %d interact-handled %d"), PC->m_pCrosshairActor ? *PC->m_pCrosshairActor->GetName() : TEXT("none"), (INT)PC->m_bCanInteractWithCrosshairActor, (INT)bFinished );
	}
	if( !bFinished && pPawn && pPawn->IsHoldingMovableObject() )
	{
		bFinished = TRUE;
	}
	if( !bFinished && pPlayerPawn && pPlayerPawn->m_LatentInteractables.Num() > 0 )
	{
		bFinished = TRUE;
	}
	FDisUseState_Finished_Param Finished;
	DemandStateChange( Finished );
}
