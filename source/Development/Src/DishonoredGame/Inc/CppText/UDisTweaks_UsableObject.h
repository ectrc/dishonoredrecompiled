// UDisTweaks_UsableObject cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x64e170 (2012 0x671590), dishonoredusableobject.cpp
public:
	virtual void Serialize( FArchive& Ar );

	// DISHONORED(port): agent JH (PHASE24 JH). The two stage accessors of the tweaks' vtable (2013 +320 / +324): GetNumUsableStages
	// (2012 0x679ac0, folded: m_Stages' count) and GetUsableStage (2012 0x679ad0).
	virtual INT GetNumUsableStages() const { return m_Stages.Num(); }
	virtual const struct FUsableObjectStage& GetUsableStage( INT _iIndex ) const { return m_Stages(_iIndex); }
