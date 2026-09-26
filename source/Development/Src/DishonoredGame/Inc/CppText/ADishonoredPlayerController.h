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
