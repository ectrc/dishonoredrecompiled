// ADishonoredPlayerController cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): IsMoveInputIgnored/IsLookInputIgnored are APlayerController virtuals in retail (2013 vtable +1172/+1176,
// native execs 0x1d3a70/0x1d3ab0); our APlayerController has them as script events, so they are introduced here.
public:
	// DISHONORED(written): ?s_pInstance@ADishonoredPlayerController@@0PAV1@A, set by PostBeginPlay, cleared by BeginDestroy
	// (2013: 205 references)
	static ADishonoredPlayerController* s_pInstance;

	// DISHONORED(written): retail vtable +956 Possess (2013 rva 0x6a0ae0, 2012 0x6d1320: Super only), +960 UnPossess (0x6a0af0,
	// a thunk to APlayerController::UnPossess), +1156 HandleWalking (0x6a0770)
	virtual void Possess( APawn* inPawn );
	virtual void UnPossess();
	virtual void HandleWalking( FLOAT DeltaTime );

	virtual void PostBeginPlay();
	virtual void BeginDestroy();
	virtual UBOOL IsMoveInputIgnored() const;
	virtual UBOOL IsLookInputIgnored() const;
	virtual void ReceivedPlayer_Native();
	virtual UArkProfileSettings* GetProfileSettings();
	virtual void OnControllerChanged_Native( UBOOL bIsConnected );

	UBOOL IsInputEnabled( INT InputMask ) const;

	

	

	

	

	

	// DISHONORED(written): agent AU. HandleHeldButtons 2013 rva 0x6ba9a0 (2012 0x6f9b80), _Context 0x6ba900 (0x6f9ae0),
	// _Context_Interactables 0x6a2e70 (0x6d6e10), Dis_Zoom 0x6a2e40 (0x6d6de0). Bodies in dishonoredplayercontroller.cpp.
	virtual void HandleHeldButtons( FLOAT DeltaSeconds );
	virtual void Dis_Zoom();
	void HandleHeldButtons_Context( FLOAT DeltaSeconds );
	void HandleHeldButtons_Context_Interactables( FLOAT DeltaSeconds );

	/**
	 * DISHONORED(port): agent DO, the ACTOR feed of ULocalPlayer::UpdatePostProcessSettings and the channel that
	 * carries the powers (agent DE hand-over 1).
	 *   ModifyPostProcessSettings            2013 rva 0x6adeb0 (2012 0x6eb900), 1,498 bytes
	 *   Tick                                 0x6b6e80 (2012 0x6ef510) - and this, not ModifyPostProcessSettings, is
	 *                                        where retail ticks the post-process node controllers
	 *   ApplyWaterPostProcessSettings        0x6a6750 (2012 0x6e2550)
	 *   ApplyDarkVisionPostProcessSettings   0x6a68d0 (2012 0x6d6d40)
	 *   ApplyPossessionPostProcessSettings   0x6a6c30 (2012 0x6e2880)
	 */
	virtual void ModifyPostProcessSettings( struct FArkPpConfig& Config );
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	void ApplyWaterPostProcessSettings( FLOAT DeltaSeconds );
	void ApplyDarkVisionPostProcessSettings( class ADishonoredPlayerPawn* PlayerPawn, FLOAT DeltaSeconds );
	void ApplyPossessionPostProcessSettings( FLOAT DeltaSeconds );

	/**
	 * DISHONORED(port): agent DO follow-up. 2013 rva 0x6af250 (2012 0x6f8690), vtable +1380 - the sprint key's
	 * toggle/hold split. The exec (2013 rva 0x5ee690) is the native the regression caught firing unbound.
	 * NOTE the rvas: the stub's comment block and the note at dishonoredplayercontroller.cpp:105 carry the 2012
	 * addresses (0x634cb0 / 0x6f8690); retail 2013's are 0x5ee690 and 0x6af250.
	 */
	virtual void DisToggleSprint( UBOOL bFromGamePad );
