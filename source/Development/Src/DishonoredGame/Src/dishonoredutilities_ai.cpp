// DishonoredGame/src/dishonoredutilities_ai.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12):
//   0x823a70  void __cdecl TriggerSearchRequestResponseDialog(class UDishonoredAIBehavior *, class IDisConvSpeakerInterface * const, enum EDisSearchRequestStimCause)
//   0x823ab0  void __cdecl DisTriggerCombatLostDialog(class UDishonoredAIBehavior &, struct FDisAttentionProxy const &, struct FAIStimStruct_CombatToSearch const &)
//   0x823b10  void __cdecl DisBreakOutOfCasualConversations(class UDishonoredAIBrain *)
//   0x827500  unsigned int __cdecl DisContinuePreviousBodyIntention(class ADishonoredNPCPawn *, struct FDisBodyIntention const &, struct FDisBodyIntention &)
//   0x832300  void __cdecl MakeNoiseAboutTarget(class UDishonoredAIBrain *, struct FDisAttentionProxy const &, enum EAINoiseContext, class FVector const &, int)
//   0x8323c0  void __cdecl TriggerSawSomethingDialog(class UDishonoredAIBrain *, class UDishonoredAIBehavior *, struct FDisAttentionProxy const &)
//   0x8324d0  void __cdecl TriggerHeardSomethingDialog(class UDishonoredAIBrain *, class UDishonoredAIBehavior *, struct FDisAttentionProxy const &, enum EAINoiseContext, unsigned int)
//   0x832550  void __cdecl TriggerDeadBodyDialog(class UDishonoredAIBrain *, class UDishonoredAIBehavior *, struct FDisAttentionProxy const &, struct FDisAttentionChangeReason const &, unsigned int)
//   0x832620  void __cdecl DisTriggerInvestigateDialog(class UDishonoredAIBrain *, class UDishonoredAIBehavior *, struct FDisAttentionProxy const &, struct FDisAttentionChangeReason const &, enum eDisHookInvestigate, unsigned int)
//   0x839340  void __cdecl DisClaimShootingPosition(class ADishonoredNPCPawn *, class FVector const &)
//   0x83a100  unsigned int __cdecl DisIsNearAnotherShootingPosition(class ADishonoredNPCPawn *, class FVector const &, float)
//   0x83d310  void __cdecl DisRelinquishShootingPosition(class ADishonoredNPCPawn *)

// ---- agent IC (PHASE23 IC) ----

#include "DishonoredGame.h"
#include "dishonoredutilities_ai.h"

// 2012 rva 0x827500 / 2013 0xbbf7a0
UBOOL DisContinuePreviousBodyIntention( ADishonoredNPCPawn* _pOwningPawn, const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention )
{
	if( !_pOwningPawn->MAT_ShouldForceInventory() )
	{
		_rResumingBodyIntention = _rPreviousBodyIntention;
		return TRUE;
	}
	BYTE BodyStance = _rPreviousBodyIntention.m_IntendedBodyStance;
	UClass* PrimaryItem = _rPreviousBodyIntention.m_pDesiredPrimaryItemClass;
	UClass* SecondaryItem = _rPreviousBodyIntention.m_pDesiredSecondaryItemClass;
	_pOwningPawn->MAT_GetBodyIntention( BodyStance, PrimaryItem, SecondaryItem );
	_rResumingBodyIntention.m_IntendedBodyStance = BodyStance;
	_rResumingBodyIntention.m_pDesiredPrimaryItemClass = PrimaryItem;
	_rResumingBodyIntention.m_pDesiredSecondaryItemClass = SecondaryItem;
	return TRUE;
}
