// ADishonoredGameInfo cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable (2013 rva of ??_7ADishonoredGameInfo@@6BAGameInfo@@@ 0xcdb838) slots +1024 CanStartMatch,
// +1028 SpawnPlayer, +1032 PreventDeath_Native; our AGameInfo has none of them as C++ virtuals, so they are introduced here.
public:
	virtual UBOOL CanStartMatch();
	virtual APawn* SpawnPlayer( UClass* SpawnClass, FVector SpawnLocation, FRotator SpawnRotation );
	virtual UBOOL PreventDeath_Native( APawn* KilledPawn, AController* Killer, UClass* DamageType, FVector HitLocation );
	UDisTweaks_PlayerPawn* LoadDefaultPlayerTweaks( UBOOL bCampaign );
	// DISHONORED(written): agent AU. GameEnding 2013 rva 0x5f9f60 (2012 0x63fb20), body in dishonoredgameinfo.cpp.
	virtual void GameEnding();

// ---- agent CG (PHASE9 CG): the global managers ----
public:
	virtual void PostBeginPlay();
	void InitGlobalManagers();
	// DISHONORED(port, partial): agent FP. The two map-change virtuals of AGameInfo (2012 vtable slots 246 / 247),
	// 2013 rvas 0x605c50 (689 bytes, 2012 0x661260) and 0x607370 (961 bytes, 2012 0x65f400). Only the UI manager's
	// share is written; the rest is named at the site. Bodies in dishonoredgameinfo.cpp.
	virtual void PreCommitMapChange( const FString& PreviousMapName, const FString& NextMapName );
	virtual void PostCommitMapChange();

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x5fa3f0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x5e9db0 (new in 2013 - DishonoredGameInfo only implements the interface in
	// 2013's script package), and the SetDifficulty (0x5e9f60, retail vtable +1052) it is a call to. Bodies in
	// dishonoredgameinfo.cpp.
	virtual void SetDifficulty( BYTE NewDifficulty );
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );

// ---- agent FR (PHASE17 FR): the conversation manager's tick ----
public:
	// DISHONORED(port, partial): retail's Tick is 2012 rva 0x64b480 (2013 ??_7ADishonoredGameInfo slot); the only share
	// written here is the UDisConvGlobalMan::TickGlobalConvMan call at its 0x64b533. Body in dishonoredgameinfo.cpp.
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
