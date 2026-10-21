// UDishonoredCamera_PhysicalReact cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent JG (PHASE24 JG). The reaction group's spring: two points (stability, strength) pushed by an impact and pulled
// back to their bound positions at a fixed 60 Hz step; the view takes the strength point's offset and the tilt of the line between them.
// 2013 vtable (two copies, PhysicalReact and HitReact, at 0x111eadc+28 / 0x111ec24+28): OnTeleport 0x6c0900 (shared with the bump
// smoother), PostBeginPlay_Derived 0x6c0c50, OnActivate_Derived 0x6c0cf0, OnCameraCollided_Derived 0x6c0e10, GatherInfluence_Start
// 0x6d49b0, GatherInfluence_Derived 0x6c5be0, GatherInfluence_FixedTimeStep 0x6c0f30; OnImpact 0x6c10f0 (non-virtual). 2012: 0x721bc0,
// 0x726140, 0x7261e0, 0x726310, 0x734b20, 0x7265e0, 0x726430, 0x726a20. Bodies in Src/dishonoredcamera_physicalreact.cpp.
public:
	virtual void OnTeleport( class USeqAct_Teleport* Action );
	void OnImpact( const FVector& Velocity );
protected:
	virtual void PostBeginPlay_Derived();
	virtual void OnActivate_Derived( APawn* Pawn );
	virtual void OnCameraCollided_Derived( const FVector& HitDirection );
	virtual void GatherInfluence_Start( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
	virtual UBOOL GatherInfluence_Derived( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
	virtual UBOOL GatherInfluence_FixedTimeStep( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, UBOOL bFirstStep, UBOOL bLastStep );
public:
