// UDishonoredCamera_BumpSmoother cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent IP (PHASE23 IP). The reaction group's first influence: a sudden change of the pawn's height (a step,
// a ledge, a landing) is absorbed and given back at m_fCompensationSpeed. OnActivate_Derived 2012 0x6fc540 / 2013 0x6c07a0,
// GatherInfluence_Derived 2012 0x7006b0 / 2013 0x6c51a0. Body in Src/dishonoredcamera_bumpsmoother.cpp.
protected:
	virtual void OnActivate_Derived( APawn* Pawn );
	virtual UBOOL GatherInfluence_Derived( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
public:
