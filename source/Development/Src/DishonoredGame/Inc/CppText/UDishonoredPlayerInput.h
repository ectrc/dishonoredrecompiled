// UDishonoredPlayerInput cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable slots +352 GetFOVScale, +356 Dis_SneakOrSlide, +360 Dis_Jump_ButtonDown,
// +364 Dis_PlayerChoice_RequestSkip, +368 Dis_PlayerChoice_RequestSkip_Released (2013 rvas in dishonoredplayerinput.cpp).
public:
	// DISHONORED(port): agent IA, 2013 rva 0x6b01e0 (2012 0x6f9380). Body in dishonoredplayerinput.cpp.
	virtual FLOAT GetFOVScale( UBOOL bIsForMouse );
	virtual void Dis_Jump_ButtonDown();
	virtual void PlayerInput( FLOAT DeltaTime );
	void OnInit( ADishonoredPlayerController* Owner );
	// DISHONORED(written): the binding half of ApplyGameSettings (2013 rva 0x6bd610): AddBindingSet 0x6b84f0
	void AddBindingSet( const TArrayNoInit<FKeyBind>& Set );
	// DISHONORED(port): 2013 rva 0x6afe50 (2012 0x6f8db0)
	INT TranslateBindingSet( INT _BindingSet ) const;
protected:
	// DISHONORED(port): 2013 rva 0x6b8240 (2012 0x6ef8f0)
	void ReadPCBindingsFromProfile();
public:

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x6bd610 (2012 0x6f8e10, byte-identical). Body in dishonoredplayerinput.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );

// ---- agent HF (PHASE22 HF): the action -> key table the HUD's key substitution reads ----
public:
	// DISHONORED(port): 2013 rva 0x6a7870 (2012 0x6e4100). Body in dishonoredplayerinput.cpp.
	const FKeyBind* GetKeyBindFromAction( FName _ActionName, UBOOL _bForcePC, UBOOL _bForceGamepad ) const;
protected:
	// DISHONORED(port): 2013 rva 0x6bac10 (2012 0x6f3fe0)
	void InitGameActionBindings();
