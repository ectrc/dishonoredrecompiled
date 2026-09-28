// UDisBehaviorTriggerAlarm cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent CG. The two sub-state callbacks whose retail bodies need nothing outside the behaviour.
// Bodies in disbehaviortriggeralarm.cpp.
public:
	virtual void OnExitCallback_GenericAction( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
	virtual void RequestStateExitCallback_GenericAction( class UDishonoredNativeState* _pThisState );
	// ---- agent CG natives sweep, round 2 ----
	virtual void OnEnterCallback_GenericAction( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pLastState );

	// ---- agent DF: the callback that needed FDisAISubStateGenericAction_Param (2012 rva 0x75f240) ----
public:
	virtual void RequestStateExitCallback_TakePosition( class UDishonoredNativeState* _pThisState );
