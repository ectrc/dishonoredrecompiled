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
