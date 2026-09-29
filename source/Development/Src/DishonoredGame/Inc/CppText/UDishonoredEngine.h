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

	// DISHONORED(port): agent CF (PHASE9 CF) - the save list agent BE's menu data models were blocked on.
	// m_pSaveGameList / m_pSaveGameDeleter are the reflected FPointer properties of this class; the types they
	// point at are in DishonoredGame/Inc/dishonoredutilities_saveload.h, where retail declares them.
	FMapConfig* FindMapConfigFromFriendlyName( const FString& FriendlyName );	// 2013 rva 0x5fbef0
	void LoadGame();												// 2013 rva 0x614c90
	void SetSaveLoadEnabled( UBOOL bEnabled );								// 2012 rva 0x62bb30
	void RefreshSaveGameList();													// 2013 rva 0x609870
	UBOOL IsSaveGameListReady() const;											// 2013 rva 0x5e44d0
	void WaitSaveGameListReady() const;											// 2012 rva 0x62bb80
	UBOOL HasSaveGame( INT _Slot ) const;										// 2013 rva 0x609920
	INT GetNumSaveGames() const;												// 2013 rva 0x609a30
	struct FDisSaveGame* GetSaveGame( INT _ListIdx ) const;						// 2013 rva 0x6099c0
	void DeleteSaveGame( INT _Slot );											// 2013 rva 0x5fbc80
	INT GetNextAutoSaveSlot();													// 2013 rva 0x60b050
	INT GetNextUserSaveSlot() const;											// 2013 rva 0x5fb930

	// DISHONORED(port): agent ED (PHASE11 ED) - the inner half of ProcessSaveLoadCmd's SLC_PostLoad arm
	// (2013 rva 0x6162d0): hand every ULevel in the world to FGameState::LoadLevel and log the census.
	void RestoreLoadedLevels();
