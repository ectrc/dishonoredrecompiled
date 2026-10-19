// UDisMantleFinderState_Base cpptext: included inside the generated class body (DishonoredGameMantleClasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). Retail 2012 / 2013 rvas (dismantlefinderstate_base.cpp): OnExitState 0x722430 / 0x6c1ea0,
// OnCancelState 0x3640 / 0x3610 (8 B, ICF-folded empty), InitMantlerInfo 0x727a20 / 0x6c6b80, SetMantleCheckSucceeded 0x727be0 /
// 0x6c6d40 (unnamed in 2013, same 108 B). 2013 0x6c1eb0 (unnamed) is the shared failure tail the 2012 bodies inline:
// MantleCheckFailed here. Bodies in Src/dismantlefinderstate_base.cpp.
protected:
	virtual void OnExitState( UDishonoredNativeState* NextState );
	virtual void OnCancelState() {}
	void InitMantlerInfo();
	void SetMantleCheckSucceeded( const FVector& StartLocation, INT StartYaw, FLOAT EdgeHeight, UBOOL bImpactAnimRequired, UBOOL bForceCrouch );
	void MantleCheckFailed( const TCHAR* Reason );
public:
