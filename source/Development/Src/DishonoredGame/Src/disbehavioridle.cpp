// DishonoredGame/src/disbehavioridle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG): the idle behaviour's stim table ----
//
// This is the smallest complete example of how a behaviour is activated, and it is the one that matters most: it is the
// behaviour every NPC falls back to, and the only one that answers the BrainInit stim UDishonoredAIBrain::InitBrain
// raises. Without it a brain finishes initialising with 21 constructed behaviours and an empty active stack, which is
// exactly what the -disai census reported before this file: "26 initialized, 0 activations".
//
// The pattern is retail's and every other UDisBehavior* subclass repeats it with a longer table:
//   BuildEvaluateStimMask   a static byte per EAIStimID, bit 0 set for each id this behaviour evaluates. Built once,
//                           shared by every instance of the class - retail guards it with a b_Initialized flag.
//   GetEvaluateStimDelegate for each of those ids, the predicate the brain calls to ask "do you want to activate?".

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"

/** DISHONORED(port): the shared "yes" predicate retail calls s_DelegateReturnTrue. A behaviour that wants a stim
    unconditionally hands this out instead of a bound method, which is why the idle behaviour needs no Evaluate* member
    of its own. */
static UBOOL DisStimAlwaysTrue( void* /*_pObject*/, const FAIStimStruct& /*_rStim*/ )
{
	return TRUE;
}

static const FDisStimPredicateDelegate GDisDelegateReturnTrue( NULL, &DisStimAlwaysTrue );

// DISHONORED(port): 2013 rva 0x6e3900 (2012 0x723330): the mask is a static built once per class and shared, which is
// why retail guards it with its own initialised flag rather than rebuilding it per behaviour.
const BYTE* UDisBehaviorIdle::BuildEvaluateStimMask()
{
	static BYTE s_Mask[EAIStimID_MAX];
	static UBOOL s_bInitialized = FALSE;
	if( !s_bInitialized )
	{
		s_bInitialized = TRUE;
		appMemzero( s_Mask, sizeof(s_Mask) );
		s_Mask[EAIStimID_BrainInit] |= 1;
		s_Mask[EAIStimID_IdleRequest] |= 1;
	}
	return s_Mask;
}

// DISHONORED(port): 2013 rva 0x6e6650 (2012 0x7284f0): the idle behaviour takes the brain-init and the idle-request stim
// unconditionally and nothing else. Every other stim gets the null delegate, which the brain reads as "not interested".
FDisStimPredicateDelegate UDisBehaviorIdle::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_BrainInit || _StimID == EAIStimID_IdleRequest )
	{
		return GDisDelegateReturnTrue;
	}
	return FDisStimPredicateDelegate();
}
