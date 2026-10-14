// DishonoredGame/src/dishonorednpcpawn_interact.cpp
// DISHONORED(port): agent GD (PHASE19 GD). What the player's crosshair and Use reach on an NPC: the three IDisInteractableInterface
// bodies of ADishonoredNPCPawn, read off both databases (the NPC's interface vtable is 2013 0x1141458; the 2012 names are the PDB's):
//   GetCrosshairStatus        2013 rva 0x759c40 (2012 0x7c02b0, 196 B) - ported in its 2013 shape, which differs from 2012's
//                             (2013 refuses the brain's answer when the awareness is Fearful or Begging and tests a second
//                             master-FSM state class beside PostPossessed; 2012 had neither)
//   GetCrosshairStatus_Helper 0x759a50 (0x7c00d0, 465 B) - the dead / frozen / stealable classification
//   AttemptInteract_Derived   0x7678c0 (0x7c93b0, 240 B) - the switch on the player controller's crosshair status
// DISHONORED(bringup), each named at its site: IsPossessed (pawn vtable +296), IsStunned (+1648), DisIsActorFrozenByBendTimePower
// (2013 0xbbf370 / 2012 0x827170: this tree has DisIsBendTimeOn only, so "frozen" is "bend time is on"), HasAttachedObjectsToTake
// (0xbeb780), ADisStatPickup::HasStatsSet, UDishonoredAIBrain::CalcPlayerAttnScale + the player's IsSneaking, OnLooted / OnGave /
// StealableInteract / OnStolen, and the second 2013 master-FSM state class (its StaticClassNoInline is 2013 0xcec7b0).

#include "DishonoredGame.h"
#include "gddiag.h"
#include "dishonoredutilities.h"

static UBOOL DisGDIsFrozenByBendTime( const AActor* _pActor )
{
	return DisIsBendTimeOn();
}

eCrossHairStatus ADishonoredNPCPawn::GetCrosshairStatus_Helper( ADishonoredPawn* _pPawn ) const
{
	eCrossHairStatus Result = CHS_OVER_NEUTRAL;
	UDishonoredAIBrain* Brain = NULL;
	ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Controller );
	if( !m_bIsOfficiallyDead && NPCController && NPCController->m_pAIBrain && NPCController->m_pAIBrain->IsBrainInitialized() )
	{
		Brain = NPCController->m_pAIBrain;
	}
	const UBOOL bDead = m_bIsOfficiallyDead ? TRUE : FALSE;
	if( bDead || DisGDIsFrozenByBendTime( this ) )
	{
		if( m_pStealable )
		{
			return bDead ? CHS_OVER_LOOT : CHS_OVER_STEAL;
		}
		if( bDead )
		{
			Result = CHS_OVER_MOVABLE_CORPSE;
			for( INT Idx = 0; Idx < m_AssociatedActors.Num(); Idx++ )
			{
				ADisStatPickup* StatPickup = Cast<ADisStatPickup>( m_AssociatedActors(Idx) );
				if( StatPickup && StatPickup->CanBePickedUp( Cast<ADishonoredPlayerPawn>( _pPawn ) ) )
				{
					return CHS_OVER_LOOT;
				}
			}
		}
	}
	else if( Brain && m_pStealable )
	{
		const BYTE Awareness = Brain->GetAwarenessLevel();
		if( Awareness == EAIAwareness_Fearful || Awareness == EAIAwareness_Begging )
		{
			return CHS_OVER_STEAL;
		}
	}
	return Result;
}

eCrossHairStatus ADishonoredNPCPawn::GetCrosshairStatus( ADishonoredPawn* _pPawn ) const
{
	const eCrossHairStatus Status = GetCrosshairStatus_Helper( _pPawn );
	UDishonoredAIBrain* Brain = NULL;
	ADishonoredNPCController* NPCController = Cast<ADishonoredNPCController>( Controller );
	if( !m_bIsOfficiallyDead && NPCController && NPCController->m_pAIBrain && NPCController->m_pAIBrain->IsBrainInitialized() )
	{
		Brain = NPCController->m_pAIBrain;
	}
	if( Status != CHS_OVER_NEUTRAL )
	{
		return Status;
	}
	if( m_pNPCMasterFSM && m_pNPCMasterFSM->IsCurState( UStateNPCMasterActionPostPossessed::StaticClass(), TRUE ) )
	{
		return Status;
	}
	eCrossHairStatus BehaviorUseStatus = CHS_OVER_MAX;
	if( Brain )
	{
		const BYTE Awareness = Brain->GetAwarenessLevel();
		if( Brain->AllowPlayerUse( &BehaviorUseStatus ) && Awareness != EAIAwareness_Fearful && Awareness != EAIAwareness_Begging )
		{
			return BehaviorUseStatus;
		}
	}
	return Status;
}

UBOOL ADishonoredNPCPawn::AttemptInteract_Derived( ADishonoredPawn* _pPawn, UBOOL& _rOut_bCanBeWitnessed )
{
	UBOOL bDidSomething = FALSE;
	ADishonoredPlayerPawn* Player = Cast<ADishonoredPlayerPawn>( _pPawn );
	ADishonoredPlayerController* PC = Player ? Cast<ADishonoredPlayerController>( Player->Controller ) : NULL;
	if( Player && PC )
	{
		if( DisGDDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): gddiag %s AttemptInteract_Derived by %s crosshair status %d"), *GetName(), *Player->GetName(), (INT)PC->m_CrosshairStatus );
		}
		switch( PC->m_CrosshairStatus )
		{
		case CHS_OVER_USABLE:
			if( UDishonoredAIBrain* Brain = GetAIBrain() )
			{
				Brain->HandleUsedByOther( Player, (eCrossHairStatus)PC->m_CrosshairStatus );
				bDidSomething = TRUE;
			}
			break;
		case CHS_OVER_USABLE_BUT_BUSY:
			bDidSomething = TRUE;
			break;
		case CHS_OVER_PICKUP:
		case CHS_OVER_LOOT:
		case CHS_OVER_STEAL:
			{
				static UBOOL bNoted = FALSE;
				if( !bNoted )
				{
					bNoted = TRUE;
					debugf( TEXT("DISHONORED(bringup): ADishonoredNPCPawn::AttemptInteract_Derived: OnLooted / OnGave / StealableInteract / OnStolen (2012 0x7c93b0's cases 2, 6 and 7) are not ported; %s status %d does nothing"), *GetName(), (INT)PC->m_CrosshairStatus );
				}
			}
			break;
		default:
			break;
		}
	}
	_rOut_bCanBeWitnessed = bDidSomething;
	return bDidSomething;
}

// DISHONORED(bringup, agent GD): every function above is a virtual or is called only from one, so nothing references this unit
// and MSVC would drop it out of the static library. Named from DishonoredGameRegistrants.cpp.
void DishonoredLinkDishonoredNPCPawnInteract()
{
}
