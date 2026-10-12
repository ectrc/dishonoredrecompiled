// DishonoredGame/src/disconv_conditions.cpp
// DISHONORED(port): agent FR (PHASE17 FR). UDisConv_Condition::GetClassVars_Derived 2013 rva 0x8a77f0 (2012 0x8f87e0,
// 21 B): a condition plays while text is extracted. The condition nodes' PlayNode_Derived bodies (IsObjectiveComplete
// 0x8a8df0, HasObjective 0x8a8e40, HasInventoryItem 0x8ab690, HasInventoryItemEquipped 0x8ab730, SeenDialogLabel 0x8a7570,
// CheckStoryFlag 0x8a8ba0, CheckTaskState 0x8a8c10, ConversationFired 0x8a8eb0, CompareDarknessLevel 0x8a8fe0,
// CheckSpeakerSuspicionLevel 0x8ab4b0, HasAbstractItem 0x8ab870, IsPossessed 0x8ab940, SpeakerCombatStatus 0x8aba70,
// IsSpeakerAvailable 0x8a76f0, HadConversation 0x8a7650) are the branching surface and are not on the direct path - a
// condition that is not ported plays its output 0 (agentFR.md inventory).
#include "DishonoredGame.h"

void UDisConv_Condition::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	UDisConv_Node::GetClassVars_Derived( _rVarsOut );
	_rVarsOut.m_bPlayWhenExtractingText = TRUE;
}
