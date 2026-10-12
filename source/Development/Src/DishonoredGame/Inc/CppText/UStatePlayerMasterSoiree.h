// UStatePlayerMasterSoiree cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The state ADishonoredPlayerPawn::MAT_BeginPlayerGroup requests for a
// cutscene's player group. Retail 2012 rvas (stateplayermastersoiree.cpp): OnEnterState 0x6af580 (2013 0x6730b0),
// OnResetState 0x6af5f0 (2013 0x668670), OnCancelState 0x6af630, OnExitState 0x6af650 (2013 0x6639e0), TickState
// 0x6af690, SetSoireeBlendOutTime 0x6af6c0 (2013 0x6604c0), OnSoireeFinished 0x6af6e0, CanTransitionExternal 0x6b76f0
// (2013 0x673150). Bodies in Src/stateplayermastersoiree.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual UBOOL OnResetState();
	virtual void OnCancelState();
	virtual void TickState( FLOAT DeltaSeconds );
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
	void SetSoireeBlendOutTime( FLOAT BlendOutTime );
	void OnSoireeFinished();
