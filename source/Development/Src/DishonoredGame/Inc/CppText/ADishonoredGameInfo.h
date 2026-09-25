// ADishonoredGameInfo cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable (2013 rva of ??_7ADishonoredGameInfo@@6BAGameInfo@@@ 0xcdb838) slots +1024 CanStartMatch,
// +1028 SpawnPlayer, +1032 PreventDeath_Native; our AGameInfo has none of them as C++ virtuals, so they are introduced here.
public:
	virtual UBOOL CanStartMatch();
	virtual APawn* SpawnPlayer( UClass* SpawnClass, FVector SpawnLocation, FRotator SpawnRotation );
	virtual UBOOL PreventDeath_Native( APawn* KilledPawn, AController* Killer, UClass* DamageType, FVector HitLocation );
	UDisTweaks_PlayerPawn* LoadDefaultPlayerTweaks( UBOOL bCampaign );
