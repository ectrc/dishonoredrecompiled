// UDishonoredMantleEdgeFinderComponent cpptext: included inside the generated class body (DishonoredGameMantleClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). Retail 2012 / 2013 rvas (dishonoredmantleedgefindercomponent.cpp): the ctor 0x72fa20 /
// 0x6d3530 only zeroes the two rolling averages, which the zeroed object allocation already does (not declared). Attach 0x7354c0 / 0x6d5e30, Detach 0x722280 / 0x6c1c40, Tick 0x744a30 / 0x6da380 (unnamed in 2013: vtable slot 79),
// OnMantleCheckEnd 0x7222a0, HasValidMantleSpot 0x7222f0 / 0x6c1ce0, SetMantleCheckDisabled 0x722300 / 0x6c1cf0 (inlined into 2013
// Tick_Mantle), SetMantleStartPos 0x722340 / 0x6c1d30, DebugSingleStep 0x722390 / 0x6c1d80, DebugSingleStepOff 0x7223a0 / 0x6c1d90,
// DebugOverrideMantleCheckTime 0x7223b0, DebugOverrideMantleStepSize 0x7223d0 / 0x6c1dc0, GetMantleSpot 0x72ae60 / 0x6c6ab0,
// ResetMantleFinder 0x7354f0 / 0x6d5ed0, GetCurrentMaxTickTime 0x743ba0 / 0x6d5e60; IDisTweaksInterface: GetTweaks_Derived (2013
// 0x7482d0, 4 B, m_pTweaks), SetTweaks_Derived (2013 0x72ace0, 13 B). Bodies in Src/dishonoredmantleedgefindercomponent.cpp.
public:
	virtual void Attach();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void Tick( FLOAT DeltaTime );
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	void OnMantleCheckEnd();
	UBOOL HasValidMantleSpot() const { return m_bHasValidMantleSpot; }
	void SetMantleCheckDisabled( UBOOL bDisabled );
	void SetMantleStartPos( const FVector& FootPos, const FVector& Facing );
	void DebugSingleStep() { m_Debug_bSingleStepMode = TRUE; m_Debug_bSingleStepMode_DoStep = TRUE; }
	void DebugSingleStepOff() { m_Debug_bSingleStepMode = FALSE; }
	void DebugOverrideMantleCheckTime( FLOAT CheckTime ) { m_Debug_fOverrideMantleCheckTime_ms = CheckTime; }
	void DebugOverrideMantleStepSize( FLOAT StepSize ) { m_Debug_fOverrideMantleStepSize = StepSize; }
	FDisMantleSpot GetMantleSpot() const { return m_CurrentMantleSpot; }
	void ResetMantleFinder();
	UDisTweaks_PlayerPawn_Mantle* GetMantleTweaks();
private:
	FLOAT GetCurrentMaxTickTime();
public:
