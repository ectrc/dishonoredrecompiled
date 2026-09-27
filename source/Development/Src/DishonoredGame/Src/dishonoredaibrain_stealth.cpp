// DishonoredGame/src/dishonoredaibrain_stealth.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG): the brain's stealth pass ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x724340 (2012 0x758f90, 174 bytes): the stealth pass maintains the player's stealth
// indicator for this brain - m_PlayerAttentiveInfo and the m_pNextPlayerAttentiveBrain chain the HUD walks - from the
// brain's current attention on the player.
// DISHONORED(bringup): the attention level it reads comes from UDisAIBrainProcessAttention, which is not ported, so the
// indicator would be built from an attention that is always Unaware. Writing that would put a wrong value in front of
// the HUD rather than none, so the pass is left as this documented no-op: m_PlayerAttentiveInfo keeps its zeroed
// default, which the HUD reads as "no NPC is attentive to the player".
void UDishonoredAIBrain::TickBrain_Stealth()
{
}
