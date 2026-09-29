// ADishonoredHUD cpptext: included inside the generated class body (DishonoredGameUIClasses.h).
// DISHONORED(written): retail vtable (??_7ADishonoredHUD@@6B@) slots +944 DrawHUD_Native, +948 PlayerDisplayDebug_Native,
// +964 DebugClear; the other debug slots (+952 .. +960, +968) are empty in retail and handled in dishonoredhud.cpp.
public:
	virtual void DrawHUD_Native();
	virtual void PlayerDisplayDebug_Native( FLOAT& OutYL, FLOAT& OutYPos );
	virtual void DebugClear();

// ---- agent EB (PHASE11 EB): the object layer ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x5fa6f0. Not a virtual and not an override: retail's
	// ADishonoredPlayerController::GameLoad calls it directly on its own HUD. Body in dissavegame.cpp.
	void SerializeForGameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

// ---- agent EK (PHASE11 EK): the in-game HUD ----
public:
	// DISHONORED(port): agent EK, 2013 rva 0x5fa6b0. Bodies in dishonoredhud.cpp.
	virtual void PostBeginPlay();
	// DISHONORED(port): agent EK, 2013 rva 0x601e70.
	virtual UBOOL Tick( FLOAT DeltaSeconds, enum ELevelTick TickType );

	// The six show-flag masks and the player-info timer. 2013 rvas in order.
	void EnableHUDElements( BYTE _MaskLevel, INT _Elements );            // 0x5ea0a0
	void DisableHUDElements( BYTE _MaskLevel, INT _Elements );           // 0x5ea0c0
	UBOOL IsHUDElementEnabled( BYTE _MaskLevel, INT _Elements ) const;   // 0x5ea0e0
	UBOOL IsHUDElementEnabledByAll( INT _Elements ) const;               // 0x5ea100
	UBOOL IsHUDElementEnabledOnce( INT _Elements ) const;                // 0x5ea130
	void RequestPlayerInfoDisplay( UBOOL _bShow );                       // 0x5ea160
	void UpdatePlayerInfoDisplay( FLOAT _fDeltaTime, UBOOL _bHide );     // 0x5ea1b0
