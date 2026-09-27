// UDisBehaviorSearch cpptext: included inside the generated class body (DishonoredGameSearchClasses.h).
// DISHONORED(port): agent CG. Bodies in disbehaviorsearch.cpp.
public:
	virtual void OnExitCallback_Investigate( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
	virtual void TickCallback_Stand( class UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds );
	// ---- agent CG natives sweep, round 2 ----
	virtual void OnExitCallback_TrackTarget( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState );
