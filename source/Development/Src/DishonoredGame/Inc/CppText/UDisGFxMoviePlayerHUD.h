// UDisGFxMoviePlayerHUD cpptext: included inside the generated class body.

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x7ac2e0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EK (PHASE11 EK): the in-game HUD ----
// Retail's own non-virtual members. The virtual ones (PostStart 2013 vtable +468, PreAdvance +476,
// PreClose +472, ApplyGameSettings) cannot be declared here without regenerating
// DishonoredGameUIClasses.h, so they arrive through the GFxUI seam as free functions, exactly as
// agent DG's three base-class overrides do (GFxUI/Inc/gfxuiengine.h). Bodies in
// disgfxmovieplayerhud.cpp.
public:
	void StartHUD();                                                    // 2013 rva 0x78cd00
	void CloseHUD();                                                    // 2013 rva 0x787e30
	UBOOL ShouldAlwaysShowPlayerInfo() const;                           // 2013 rva 0x78cd40
	void PreRender( class UCanvas* _pCanvas );                          // 2013 rva 0x7a5840
	void HideGauge( BYTE _Gauge, UBOOL _bSuccess );                     // 2013 rva 0x794c10
	void UpdateGauge( BYTE _Gauge, FLOAT _fValue );                     // 2013 rva 0x794d40

	// The two seam bodies reach the protected tick halves retail's own PreAdvance reaches.
	friend void DisGFxMoviePlayerHUDPostStart( class UDisGFxMoviePlayerHUD* HUD );
	friend void DisGFxMoviePlayerHUDPreAdvance( class UDisGFxMoviePlayerHUD* HUD, FLOAT DeltaTime );

protected:
	void PreRender_Layout( class UCanvas* _pCanvas );                   // 2013 rva 0x796250
	void Tick_PlayerStatus( INT _ShowFlags );                           // 2013 rva 0x79f6e0
	void Tick_PlayerState( INT _ShowFlags );                            // 2013 rva 0x795ed0
	void UpdateHealthGauge( const struct FDisPlayerStatus_Health& _rHealth );        // 2013 rva 0x797d40
	void UpdateManaGauge( const struct FDisPlayerStatus_Mana& _rMana );              // 2013 rva 0x797f80
	void UpdateEquipmentInfo( const struct FDisPlayerStatus_Equipment& _rEquipment ); // 2013 rva 0x7981f0
