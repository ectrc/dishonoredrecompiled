// UStatePlayerMasterFalling cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IF (PHASE23 IF). Retail 2012 / 2013 rvas (stateplayermasterfalling.cpp): OnEnterState 0x6a76d0 / 0x6759f0,
// OnExitState 0x696260 / 0x663110, TickState 0x69b650 / 0x675b80, ShouldAnimateFalling 0x692ec0 / 0x65ff10, LoadPartialState
// 0x696340 / 0x663260. Bodies in Src/stateplayermasterfalling.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual void LoadPartialState( UDishonoredNativeStateMachine* StateMachine, UObject* ManagedObject, FArchive& Ar, ESaveLoadLocation Location );
	UBOOL ShouldAnimateFalling() const { return m_bCameFromJump || m_fFallTime > m_fAnimateFallTime; }
