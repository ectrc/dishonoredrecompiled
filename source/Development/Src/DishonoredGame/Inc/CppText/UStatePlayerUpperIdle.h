// UStatePlayerUpperIdle cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The upper-body / left-arm machine's default state: its picker is cleared and the hand's
// item follows. 2012 / 2013 rvas: OnEnterState 0x6c6550 / 0x6741a0, RequestStateExit_Derived 0x6c45a0 / 0x677ad0, the upper params'
// OnPending 0x6b4560 / 0x664230. Bodies in Src/stateplayerupperidle.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
protected:
	virtual void RequestStateExit_Derived();
public:
