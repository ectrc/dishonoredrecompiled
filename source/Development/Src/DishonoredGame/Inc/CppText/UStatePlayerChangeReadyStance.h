// UStatePlayerChangeReadyStance cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IM (PHASE23 IM). The hand's go-to-ready / exit-ready action: the new stance is stored on the pawn once the
// upper-body state is at full weight, or when the action ends. 2012 / 2013 rvas: OnEnterState 0x6afc30, OnExitState 0x6afc50 /
// 0x660c70, TickState 0x6afc90 / 0x660ca0, DoAction 0x6b93d0 / 0x66fdd0, RequestStateExit_Derived 0x6c4430 / 0x6773f0,
// CanTransitionExternal 0x6c61f0 / 0x673da0, the param's ctor 0x6c6190 / 0x673d40 and OnPending 0x6b45b0.
// Bodies in Src/stateplayerchangereadystance.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual UBOOL DoAction();
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
protected:
	virtual void RequestStateExit_Derived();
public:
