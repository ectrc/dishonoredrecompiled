// DishonoredGame/src/disaisubstateinit.cpp
// ---- agent DF ports (PHASE10 DF): the idle sub-state ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	UDisAISubStateInit

	DISHONORED(retail): the class has no body at all - no members, no overrides, not one function of its own in either
	build. It exists to be the state a behaviour's machine sits in when the behaviour has nothing to do, and the base
	UDisAISubState is written around that: OnEnterState, OnExitState and OnResetState all test `GetClass() !=
	UDisAISubStateInit::StaticClass()` and skip the desires and the derived hooks for it (2013 rvas 0x705850, 0x724420,
	0x724590), and UDisAISubStateMachine::OnOwningBehaviorResume / OnOwningBehaviorPause exempt it as well. So the idle
	state is not a state that does nothing; it is the absence of a state, spelled as one.

	Only its parameter has a body, and all that does is name the class.
-----------------------------------------------------------------------------*/

// DISHONORED(port): ctor 2012 rva 0x77bc60. The 2012 constructor stores FDisAISubState_Param's vtable rather than one of
// its own, which is how the PDB shows that FDisAISubStateInit_Param adds no OnPending.
FDisAISubStateInit_Param::FDisAISubStateInit_Param()
	: FDisAISubState_Param( UDisAISubStateInit::StaticClass() )
{
}
