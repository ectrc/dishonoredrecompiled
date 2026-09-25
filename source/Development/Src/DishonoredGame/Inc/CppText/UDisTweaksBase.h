// UDisTweaksBase cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable slots +300 GetSpawnedObjectClass and +316 SpawnActor_Derived (2013 rvas in distweaksbase.cpp);
// the tweak subclasses that override them (NPC pawn tweaks: FSpawnNPCPawn_TweakObj) are not ported.
public:
	virtual UClass* GetSpawnedObjectClass( BYTE SpawnType ) const;
	AActor* SpawnActor( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const;
	UBOOL CanSpawnObjectOfClass( const UClass* Class, BYTE SpawnType ) const;
protected:
	virtual AActor* SpawnActor_Derived( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const;
