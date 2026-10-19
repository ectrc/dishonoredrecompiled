// UStatePlayerAction cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The upper-body / left-arm action state: its anim state goes to the picker the action
// machine is bound to, the hand's item follows it, and leaving goes back to the machine's idle (or navigation) state.
// 2012 / 2013 rvas: OnEnterState 0x6afc20, DoAction 0x6b92e0 / 0x66fc60, RequestStateExit_Derived 0x6c4370 / 0x677290, the param's
// OnPending 0x6b4580. Bodies in Src/stateplayeraction.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual UBOOL DoAction();
protected:
	virtual void RequestStateExit_Derived();
public:
