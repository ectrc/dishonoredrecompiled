// UStatePlayerMasterJump cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent IF (PHASE23 IF). Retail 2012 / 2013 rvas (stateplayermasterjump.cpp): OnEnterState 0x6a79a0 / 0x675ee0,
// OnExitState 0x693020 / 0x65ffa0, TickState 0x6a7bc0 / 0x679150, CanTransitionExternal 0x6a4190 / 0x66eb00, DoNormalJump_Fixed
// 0x693090 / 0x660010, DoPowerJump_Fixed 0x6a6460 / 0x671370, HasPowerJump 0x6930e0 / 0x660060, DisablePowerJump 0x693150 / 0x6600d0,
// PlayJumpInActions 0x69ba10 / 0x672ed0, TickJumpLogic_FixedPower 0x6a64e0 / 0x676110, _HeldPower_FullStop 0x6a65d0 / 0x676200,
// _ContinuousPower 0x6a66b0 / 0x6762e0, _NoPower 0x6a6800 / 0x676430. Bodies in Src/stateplayermasterjump.cpp.
public:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void TickState( FLOAT DeltaSeconds );
	virtual UBOOL CanTransitionExternal( const UClass* StateID ) const;
	void DisablePowerJump() { m_bPowerJumpAllowed = FALSE; }
protected:
	void DoNormalJump_Fixed();
	void DoPowerJump_Fixed();
	UBOOL HasPowerJump() const;
	void PlayJumpInActions();
	void TickJumpLogic_FixedPower( FLOAT DeltaSeconds );
	void TickJumpLogic_HeldPower_FullStop( FLOAT DeltaSeconds );
	void TickJumpLogic_ContinuousPower( FLOAT DeltaSeconds );
	void TickJumpLogic_NoPower( FLOAT DeltaSeconds );
public:
