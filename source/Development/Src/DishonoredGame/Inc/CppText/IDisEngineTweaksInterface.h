// IDisEngineTweaksInterface cpptext: included inside the generated interface body (DishonoredGameEngineShims.h).
// DISHONORED(written): 2012 PDB IDisEngineTweaksInterface_vtbl (12 bytes): +0 ~, +4 GetUObjectInterfaceDisEngineTweaksInterface,
// +8 HasTweaks_Derived. HasTweaks is 2013 rva 0x11b1d0 (2012 0xd7f00): a call through slot +8. The script compiler emits the
// GetUObjectInterface glue per implementing class in retail; the generated classes have none, so the base returns NULL.
public:
	virtual UObject* GetUObjectInterfaceDisEngineTweaksInterface() { return NULL; }
	virtual UBOOL HasTweaks_Derived( const class UDisEngineTweaksBase& Tweaks ) const { return FALSE; }
	UBOOL HasTweaks( const class UDisEngineTweaksBase& Tweaks ) const { return HasTweaks_Derived( Tweaks ); }
