// IDisTweaksInterface cpptext: included inside the generated interface body (DishonoredGameClasses.h).
// DISHONORED(written): 2012 PDB IDisTweaksInterface_vtbl (28 bytes, extends IDisEngineTweaksInterface): +12
// GetUObjectInterfaceDisTweaksInterface, +16 GetTweaks_Derived, +20 SetTweaks_Derived, +24 ApplyTweakChanges_Derived. The
// non-virtual bodies (SetTweaks 2013 rva 0x661fe0, ApplyTweakChanges 0x882a30, IsTweaksValid 0x5fb110, HasTweaks_Derived
// 0x873d60) live in distweaksbase.cpp. Implementing classes override the _Derived slots (ADishonoredPawn: m_pPawnTweaks).
public:
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return NULL; }
	virtual class UDisTweaksBase* GetTweaks_Derived() { return NULL; }
	virtual void SetTweaks_Derived( class UDisTweaksBase* Tweaks ) {}
	virtual void ApplyTweakChanges_Derived() {}
	virtual UBOOL HasTweaks_Derived( const class UDisEngineTweaksBase& Tweaks ) const;

	class UDisTweaksBase* GetTweaks() const { return const_cast<IDisTweaksInterface*>( this )->GetTweaks_Derived(); }
	void SetTweaks( class UDisTweaksBase* Tweaks );
	void ApplyTweakChanges();
	UBOOL IsTweaksValid() const;
