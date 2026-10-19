// ADisStaticCollisionActor cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent IG (PHASE23 IG). ShouldRejectPlayerMantle 2012 0x631000 / 2013 0x618020 (9 B: m_bRejectPlayerMantle).
// PostLoad (2012 0x630fc0) is not ported.
public:
	virtual class UObject* GetUObjectInterfaceDisMantleableInterface() { return this; }
	virtual UBOOL ShouldRejectPlayerMantle() const { return m_bRejectPlayerMantle; }
