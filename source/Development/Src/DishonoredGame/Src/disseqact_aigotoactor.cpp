// DishonoredGame/src/disseqact_aigotoactor.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11), 2012 rvas:
//   0x7cb780  public: enum ETransitSpeed __thiscall UDisSeqAct_AIGoToActor::GetDesiredTransitSpeed(void)const
//   0x7cb790  public: unsigned int __thiscall UDisSeqAct_AIGoToActor::ShouldAlignRotation(void)const
//   0x7cb9a0  public: virtual void __thiscall UDisSeqAct_AIShoot::Activated(void)
//   0x7cdc20  public: class AActor * __thiscall UDisSeqAct_AIGoToActor::GetDestinationActor(void)const
//   0x7d3220  public: virtual void __thiscall UDisSeqAct_AIShoot::PreActorHandle(class AActor *)
//   0x7d3540  public: virtual unsigned int __thiscall UDisSeqAct_NPCDoTeleportSpell::UpdateOp(float)
//   0x7d8da0  public: void __thiscall UDisSeqAct_AIGoToActor::SetGoToStatus(class ADishonoredNPCController * const, enum AIGoToActorOutputEnum)
//   0x7d8e50  public: virtual void __thiscall UDisSeqAct_AIGoToActor::DeActivated(void)
//   0x7eb580  public: static class UClass * __cdecl UDisSeqAct_AIGoToActor::GetPrivateStaticClassUDisSeqAct_AIGoToActor(wchar_t const *)
//   0x7ee410  public: static void __cdecl UDisSeqAct_AIGoToActor::InitializePrivateStaticClassUDisSeqAct_AIGoToActor(void)
//   0x7ef480  public: static class UClass * __cdecl UDisSeqAct_AIGoToActor::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent FV (PHASE18 FV): the Kismet go-to, the action half.

	A DisSeqAct_AIGoToActor is a latent action: Activated (USequenceAction's, through the handler name) reaches
	ADishonoredNPCController::OnAIGoToActor on each target's controller, which remembers the action and hands the brain a
	FAIStimStruct_GoToRequest; the controller is added to LatentActors by the engine's latent-action plumbing. The
	behaviour that walks the pawn (UDisBehaviorGoTo_Base) reports back through SetGoToStatus, which drops the controller
	from LatentActors and fires the matching output - Success only when the LAST controller has arrived, which is why
	the Empress's chain hangs off output 1, 'Success (Last)'. UDisSeqAct_AIShoot and UDisSeqAct_NPCDoTeleportSpell share
	this unit in retail and are not this package's.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x78ae40 (2012 0x7cdc20, 71 B): a destination given as a controller means its pawn.
AActor* UDisSeqAct_AIGoToActor::GetDestinationActor() const
{
	AController* AsController = Cast<AController>( m_pDestinationActor );
	return AsController ? AsController->Pawn : m_pDestinationActor;
}

// DISHONORED(port): 2013 rva 0x7a1fc0 (2012 0x7d8da0, 170 B). Any status but Out unlinks the controller from this action
// and this action from the controller; the output link named by the status is then activated unless it is Success with
// other controllers still walking - Success means "the last one arrived". Retail's bDisabled / bDisabledPIE test on the
// link is USequenceOp::ActivateOutputLink's.
void UDisSeqAct_AIGoToActor::SetGoToStatus( ADishonoredNPCController* const _pNPCController, BYTE _Status )
{
	if( _Status != AIGoToActorOutput_Out && _pNPCController )
	{
		LatentActors.RemoveItem( _pNPCController );
		_pNPCController->LatentActions.RemoveItem( this );
	}
	if( _Status != AIGoToActorOutput_Success || LatentActors.Num() == 0 )
	{
		if( _Status < OutputLinks.Num() )
		{
			ActivateOutputLink( _Status );
		}
	}
}

// DISHONORED(port): 2013 rva 0x786360 (2012 0x7cb780): the designer's two-state speed is Walk or Run, never Idle.
BYTE UDisSeqAct_AIGoToActor::GetDesiredTransitSpeed() const
{
	return m_DesiredMovementSpeed != 0 ? ETransitSpeed_Run : ETransitSpeed_Walk;
}

// DISHONORED(port): 2013 rva 0x786370 (2012 0x7cb790)
UBOOL UDisSeqAct_AIGoToActor::ShouldAlignRotation() const
{
	return m_bAlignRotation;
}

// DISHONORED(port): 2013 rva 0x7a2070 (2012 0x7d8e50): a jump to UDisSeqAct_Latent::CleanUpLatentActors in both builds.
void UDisSeqAct_AIGoToActor::DeActivated()
{
	CleanUpLatentActors();
}
