// UDisMantleFinderState_ForwardSearches cpptext: included inside the generated class body (DishonoredGameMantleClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). Retail 2012 / 2013 rvas (dismantlefinderstate_forwardsearches.cpp): OnEnterState
// 0x744da0 / 0x6da6f0, TickState 0x7442e0 / 0x6d6600, CalcSearchStartHeight 0x7446a0 / 0x6d69b0. Bodies in
// Src/dismantlefinderstate_forwardsearches.cpp.
private:
	virtual void OnEnterState( UDishonoredNativeState* LastState );
	virtual void TickState( FLOAT DeltaSeconds );
	FLOAT CalcSearchStartHeight( UBOOL& bOutImpactAnimRequired ) const;
public:
