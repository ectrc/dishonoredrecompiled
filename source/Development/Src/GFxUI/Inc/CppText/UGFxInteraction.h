// UGFxInteraction cpptext: included inside the generated class body (GFxUIClasses.h). Bodies in
// Src/gfxuinatives.cpp; retail has them in gfxui/src/gfxuiinteraction.cpp, whose input pipeline
// (Init, InputKey, InputAxis, InputChar, Exec, Tick) belongs to the runtime-glue package.
public:
	virtual void CloseAllMoviePlayers();                                                      // 2012 0x5d22b0
	virtual void NotifyGameSessionEnded();                                                    // 2012 0x5d4f30
	virtual void NotifyPlayerAdded( INT PlayerIndex, class ULocalPlayer* AddedPlayer );        // 2012 gfxuiinteraction.cpp
	virtual void NotifyPlayerRemoved( INT PlayerIndex, class ULocalPlayer* RemovedPlayer );    // 2012 gfxuiinteraction.cpp
	virtual class UGFxMoviePlayer* GetFocusMovie( INT ControllerId );                          // 2012 0x5caf40

	// DISHONORED(port, agent DC): the per-frame owner of the interface. Bodies in
	// Src/gfxuiinteraction.cpp, which is the file the 2012 PDB attributes them to. These override
	// UInteraction's own virtuals and FCallbackEventDevice::Send, so no new vtable slot is introduced.
	virtual void Init();                                                                      // 2013 0x5a2c40
	virtual void Tick( FLOAT DeltaTime );                                                     // 2013 0x57b790
	virtual void SetRenderViewport( class FViewport* InViewport );                              // 2013 0x57b770
	virtual void Send( ECallbackEventType InType, class FViewport* InViewport, UINT InMessage ); // 2013 0x57b860
	virtual UBOOL InputKey( INT ControllerId, FName Key, EInputEvent Event,
		FLOAT AmountDepressed = 1.f, UBOOL bGamepad = FALSE );                                 // 2013 0x591f10
	virtual UBOOL InputChar( INT ControllerId, TCHAR Character );                              // 2013 0x591fd0
	virtual UBOOL InputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime,
		UBOOL bGamepad = FALSE );                                                              // 2013 0x595140
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );                                 // 2012 0x5e4af0
