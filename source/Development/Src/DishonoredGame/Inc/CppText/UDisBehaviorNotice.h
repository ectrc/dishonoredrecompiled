// UDisBehaviorNotice cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): agent DF. Bodies in disbehaviornotice.cpp.
//
// DISHONORED(retail): this is the one behaviour that does its work from the IDLE sub-state's callbacks. Noticing something
// is not an action - the NPC keeps doing whatever it was doing and just looks - so the behaviour takes slot 0, whose
// sub-state is UDisAISubStateInit, and states a look-at desire from its OnEnter callback. That is why
// UDisTweaks_AIBehavior_Notice's slot 0 is a UDisTweaks_AISubState_Init at all, and why FDisAISubStateInit_Param needs a
// constructor of its own.
public:
	virtual void OnEnterCallback_Init( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pLastState );
	virtual void OnExitCallback_Init( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
	virtual void RequestStateExitCallback_GenericAction( class UDishonoredNativeState* _pThisState );
