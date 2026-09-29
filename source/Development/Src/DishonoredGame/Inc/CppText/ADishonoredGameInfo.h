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

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x5fa3f0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
