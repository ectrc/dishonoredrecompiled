// UDisBehaviorSearch cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(port): agent CG. Bodies in disbehaviorsearch.cpp.
public:
	virtual void OnExitCallback_Investigate( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
	virtual void TickCallback_Stand( class UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	// ---- agent CG natives sweep, round 2 ----
	virtual void OnExitCallback_TrackTarget( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );

	// ---- agent DF: the two callbacks the desire layer unblocked (2012 rvas 0x724850 / 0x73f770) ----
public:
	virtual void OnEnterCallback_GenericAction( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pLastState );
	virtual void OnExitCallback_GenericAction( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
