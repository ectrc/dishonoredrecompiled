// UArkProfileSettings cpptext: included inside the generated shim class body (DishonoredGameEngineShims.h).
// agent HJ (PHASE22 HJ): the key-binding half of the profile. Bodies in Src/arkprofilesettings.cpp; retail keeps them in
// Engine/src/arksettings.cpp (2012 PDB).
public:
	// DISHONORED(port): 2013 rva 0x53b8c0 (2012 0x57c1b0)
	virtual void SetToDefaults();
	// DISHONORED(port): 2013 rva 0x537610 (2012 0x5786c0)
	static void FillGBAList( TArray<FString>& _rOutGBAs );
	// DISHONORED(port): 2013 rva 0x533560 (2012 0x574ea0, 34 bytes, no range check)
	FName GetKeyName( INT _BindableKey ) const { return m_BindableKeyMapping[_BindableKey]; }
