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
