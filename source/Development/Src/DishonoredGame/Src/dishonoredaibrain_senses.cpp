// DishonoredGame/src/dishonoredaibrain_senses.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG): the brain's senses pass ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x717880 (2012 0x7562d0, 118 bytes): the senses pass of one brain tick. Retail asks the
// controller's FDisComponentVisionNPC what it can see, hands each sighting to the attention process
// (UDisAIBrainProcessAttention, 2013 vtable +? through UDisAIBrainProcess::TickBrainProcess) and raises the
// TargetSighted / TargetUnsighted stims from the difference.
// DISHONORED(bringup): the vision component is not ported (FDisComponentVisionNPC, creator 2012 rva 0x632020, bodies in
// discomponentvisionnpc.cpp which is a comment-only skeleton) and neither is the attention process's own body
// (disaibrainprocessattention.cpp, 84 functions). ADishonoredNPCController::GetVisionComponent therefore answers NULL
// and this pass has nothing to iterate, which is exactly the path retail takes for a brain whose controller has no
// vision component. The brain process list is still ticked, by TickBrain_Processes, so a brain process that does not
// need vision runs.
// What this costs, precisely: an NPC cannot see. Everything downstream of sight - suspicion escalation, the attention
// meter, combat engagement - is therefore never raised by vision. Stims raised by other systems (Kismet, noise,
// damage, the brain-init stim) still flow, which is what makes the behaviour stack observable at all.
void UDishonoredAIBrain::TickBrain_Senses( FLOAT _fDeltaSeconds )
{
	static UBOOL bWarnedOnce = FALSE;
	if( !bWarnedOnce )
	{
		bWarnedOnce = TRUE;
		debugf( TEXT("DISHONORED(bringup): UDishonoredAIBrain::TickBrain_Senses (2013 rva 0x717880) needs FDisComponentVisionNPC and UDisAIBrainProcessAttention; NPCs cannot see yet") );
	}
}
