// UStatePlayerMasterSwim cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IF (PHASE23 IF). Retail 2012 / 2013 rvas (stateplayermasterswim.cpp): OnEnterState 0x6b7940 / 0x673190,
// OnExitState 0x6b3e20 / 0x673470, TickState 0x6c3bb0 / 0x676710, CanTransitionExternal 0x6b3ef0 / 0x66f990, OnRejectStateRequest
// 0x6b3f20 / 0x66f9c0, GetSwimStrokeAccel 0x6b3f50 / 0x660690, IsOnSurface 0x6af7e0 / 0x660650, Stroke 0x6af800 (7 B, inlined in 2013).
// Bodies in Src/stateplayermasterswim.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
	virtual void OnRejectStateRequest( const UClass* StateID );
	FLOAT GetSwimStrokeAccel() const;
	UBOOL IsOnSurface() const;
	void Stroke() { m_fCurStrokeTime = m_fStrokeTime; }
