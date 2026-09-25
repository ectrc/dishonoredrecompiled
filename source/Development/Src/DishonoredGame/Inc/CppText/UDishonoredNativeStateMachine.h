// UDishonoredNativeStateMachine cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): accessors of dishonorednativestatemachine.cpp; the transition machinery (RequestStateChange,
// DemandStateChange, DoStateChange, TickStateMachine, InitFSM) and the UDishonoredNativeState virtuals are not ported yet.
public:
	UDishonoredNativeState* GetCurrentlyActiveState() const;
	UDishonoredNativeState* GetPendingState() const;
	UDishonoredNativeState* GetLogicalState() const;
	const UClass* GetPendingStateID() const;
	UDishonoredNativeState* FindState( UClass* StateID ) const;
