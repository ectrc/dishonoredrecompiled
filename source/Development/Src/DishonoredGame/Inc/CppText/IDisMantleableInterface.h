// IDisMantleableInterface cpptext: included inside the generated interface body (dishonoredgameclasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). 2012 PDB IDisMantleableInterface_vtbl: +0 destructor, +4 the UObject accessor, +8
// ShouldRejectPlayerMantle (read by UDisMantleFinderState_ForwardSearches::TickState, 2012 0x7442e0 / 2013 0x6d6600). Retail's one
// implementor is ADisStaticCollisionActor; the base answer here is "not rejected" where retail's slot is pure.
public:
	virtual class UObject* GetUObjectInterfaceDisMantleableInterface() { return NULL; }
	virtual UBOOL ShouldRejectPlayerMantle() const { return FALSE; }
