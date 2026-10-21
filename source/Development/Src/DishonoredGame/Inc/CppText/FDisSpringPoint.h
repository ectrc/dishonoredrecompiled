// FDisSpringPoint cpptext: included inside the generated struct body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent JG (PHASE24 JG). StorePreviousPhysResult 2012 0x721b90 (inlined in 2013), IsSpringing 0x725ce0 / 2013 0x6c09a0,
// UpdateSpringPos 0x725d40 / 2013 0x6c0a00 (byte-identical), ApplyMaxConstraint 0x725f90, GetPos_Interpolated 0x7260e0 (both inlined or
// dropped in 2013). Bodies in Src/dishonoredcamera_physicalreact.cpp.
public:
	void StorePreviousPhysResult();
	UBOOL IsSpringing() const;
	void UpdateSpringPos( const struct FDisSpringConstant& SpringConst, FLOAT DeltaTime, class UObject* FuncOwner, void (*VelAdjustFunc)( class UObject*, FVector& ) );
	UBOOL ApplyMaxConstraint( FLOAT MaxDist, const FVector* FromPos, INT Flags );
	const FVector GetPos_Interpolated( FLOAT InterpScale ) const;
