// UStatePlayerMasterWalk cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The player master FSM's default state (PreBeginPlay_Actions enters it).
// Retail 2012 rvas: OnEnterState 0x6af890 (2013 0x676980), TickState 0x6beba0 (2013 0x6769f0),
// ShouldLimitControllerSensitivity 0x6b7c20 (2013 0x6687b0), CheckForAutoCrouch 0x6b7ce0 (2013 0x668870, 2053 B,
// not ported). Bodies in Src/stateplayermasterwalk.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void TickState( FLOAT DeltaSeconds );
