// UStatePlayerMasterMantle cpptext: included inside the generated class body (DishonoredGameMantleClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). Retail 2012 / 2013 rvas (stateplayermastermantle.cpp): OnEnterState 0x6be610 / 0x66ec20
// (unnamed in 2013: vtable slot 73), OnExitState 0x6be7b0 / 0x66ee00, TickState 0x6c1e20 / 0x66c590 (slot 77),
// OnEnterState_IntroTransition 0x6af340 / 0x660270, OnExitState_Crouching 0x6af3b0 / 0x6602e0, Tick_IntroTransition 0x6b3a00 / 0x6636c0,
// OnEnterState_ImpactAnim 0x6b6d10 / 0x667c50, Tick_ImpactAnim 0x6b6d70 / 0x667cb0, GetCurrentMantleHeight 0x6af420 / 0x660350,
// GetCrouchRequired 0x6af430 / 0x660360, SetAllowPlayerEnd 0x6af440, SetAllowPlayerCancel 0x6af450, StartCrouchTransition 0x6af470.
// The parameters (FStatePlayerMasterMantle_Param ctor 0x6c3b20, OnPending 0x6b3980) live in stateplayermasterparams.h.
// Bodies in Src/stateplayermastermantle.cpp.
public:
	FLOAT GetCurrentMantleHeight() const { return m_fCurrentMantleHeight; }
	UBOOL GetCrouchRequired() const { return m_bCrouchRequired; }
	void SetAllowPlayerEnd() { m_bAllowPlayerEnd = TRUE; }
	void SetAllowPlayerCancel( UBOOL bAllowCancel ) { m_bAllowPlayerCancelActive = bAllowCancel; }
	void StartCrouchTransition( FLOAT TransitionTime );
private:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	void OnEnterState_IntroTransition();
	void OnExitState_Crouching();
	void Tick_IntroTransition( FLOAT DeltaSeconds );
	void OnEnterState_ImpactAnim();
	void Tick_ImpactAnim();
public:
