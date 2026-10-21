// DishonoredGame/src/dishonoredutilities_npc.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x824450  void __cdecl DoDeadBodyPoseHACK(class ADishonoredNPCPawn *)
//   0x8244e0  class FVector __cdecl ConvertMomentumToItemDropVelocity(class FVector const &)
//   0x82e180  class FVector __cdecl ConvertMomentumToNPCThrowVelocity(class ADishonoredNPCPawn *, class FVector const &)
//   0x82e220  unsigned int __cdecl HasAttachedObjectsToTake(class AActor const * const, class ADishonoredPawn * const)
//   0x82e330  unsigned int __cdecl TakeAttachedObjects(class AActor * const, class ADishonoredPawn * const, unsigned int)
//   0x82e550  void __cdecl DetachUnwantedPooPoo(class ADishonoredNPCPawn *)
//   0x832a50  unsigned int __cdecl AttemptTeleportSpell(class ADishonoredNPCPawn *, class AActor *, enum EDisTeleportIntention, class FVector)

/*-----------------------------------------------------------------------------
	agent JE (PHASE24 JE): AttemptTeleportSpell, the blink a go-to or a take-position falls back on when the navmesh will not
	take the NPC to its mark (UDisBehaviorGoTo_Base::FilterPathingFail, UDisAISubStateTakePosition::FilterPathingFail).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities_ai.h"
#include "jediag.h"

/**
 * DISHONORED(port): 2012 0x832a50 (228 B). Nothing to blink to without a focus actor, nor in game to a terminated one; the spell
 * is an item context of the NPC's secondary weapon (UDisItemContext_NPCTeleportSpell): it is prepared for a manual teleport with
 * the intention and the target location, used directly with an FDisItemContextParam_UseNPC aimed at the focus actor (cooldown
 * multiplier 1), and cancelled when the use is refused.
 * DISHONORED(bringup): the item-context half - UDishonoredInventoryItem::FindItemContext<UDisItemContext_NPCTeleportSpell>,
 * UDisItemContext_NPCTeleportSpell::PrepareManualTeleport (2012 0x8865c0) / CancelManualTeleport (0x886600) and
 * UDishonoredInventoryItem::AttemptUse_DirectUse (0x85ad20) - is not ported (agent AJ's item contexts), so a pawn that does
 * carry a secondary weapon is refused, named once; one without (every non-caster, as retail) is refused before it.
 */
UBOOL AttemptTeleportSpell( ADishonoredNPCPawn* _pOwningNPCPawn, AActor* _pFocusActor, BYTE _TeleportIntention, FVector _TargetLocation )
{
	if( !_pFocusActor )
	{
		return FALSE;
	}
	if( GIsGame && _pFocusActor->m_bLegalToTerminate && _pFocusActor->m_bHasTerminated )
	{
		return FALSE;
	}
	UDishonoredWeapon* Weapon = Cast<UDishonoredWeapon>( _pOwningNPCPawn->GetEquippedItem( EDisEquipUsage_Secondary ) );
	JEDIAG( "teleport spell %s to %s (intention %d): secondary weapon %s", *_pOwningNPCPawn->GetName(), *_TargetLocation.ToString(),
		(INT)_TeleportIntention, Weapon ? *Weapon->GetName() : TEXT("none") );
	if( !Weapon )
	{
		return FALSE;
	}
	static UBOOL bNoted = FALSE;
	if( !bNoted )
	{
		bNoted = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): AttemptTeleportSpell: %s carries %s, but the teleport-spell item context (FindItemContext / PrepareManualTeleport / AttemptUse_DirectUse) is not ported; refused"),
			*_pOwningNPCPawn->GetName(), *Weapon->GetName() );
	}
	return FALSE;
}
