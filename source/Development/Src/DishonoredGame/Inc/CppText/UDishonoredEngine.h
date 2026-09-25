// UDishonoredEngine cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable 2013 rva 0xcdb390. PlayLoadMapMovie (+296), OpenPauseMenu (+300), OnControllerDisconnected
// (+304), OpenControllerConnectionMenu (+308), OpenContentUnavailableMenu (+312) and IsLoadingGame (+412) are UEngine virtuals in
// retail; Dis_Save .. OnControllerChanged (+512 .. +536) follow the UGameEngine slots. Our UEngine (reference 10897) has none of
// them, so they are introduced here and Engine code cannot reach them yet (follow-up in agentAC.md).
public:
	virtual void Init();
	virtual UBOOL PlayLoadMapMovie( const FString& MapName, const FString& MovieName );
	virtual void OpenPauseMenu();
	virtual void OnControllerDisconnected( INT ControllerId );
	virtual void OpenControllerConnectionMenu() const;
	virtual void OpenContentUnavailableMenu() const;
	virtual UBOOL IsLoadingGame() const;
	virtual void Dis_Save( INT SaveSlot );
	virtual void Dis_Load( INT SaveSlot );
	virtual void PushDisableSave( BYTE DisableSaveType );
	virtual void PopDisableSave( BYTE DisableSaveType, FLOAT WaitToEnable );
	virtual void PushIgnoreAutosave( BYTE Type );
	virtual void PopIgnoreAutosave( BYTE Type );
	virtual void OnControllerChanged( UBOOL bUsingGamepad );

	FMapConfig* FindMapConfig( const FString& MapName );
	UBOOL IsSavingAllowed() const;
