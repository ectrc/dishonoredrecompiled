// UDisTweaks_NPCPawn cpptext: included inside the generated class body (dishonoredgameaiclasses.h).
// DISHONORED(written): agent EP (PHASE14 EP). Body in Src/distweaks_npcpawn.cpp.
public:
	/** The NPC-pawn spawn, which differs from UDisTweaksBase::SpawnActor_Derived in exactly one way: the init functor it
	    hands UWorld::SpawnActor also carries the spawner, so the pawn's FDisSpawnerInfo is filled before PostBeginPlay. */
	class AActor* SpawnActor_WithSpawner( BYTE _SpawnType, class ADishonoredSpawner* _pSpawner, FName _InName,
		const FVector& _rLocation, const FRotator& _rRotation, class AActor* _pTemplate, UBOOL _bNoCollisionFail,
		UBOOL _bRemoteOwned, class AActor* _pOwner, class APawn* _pInstigator, UBOOL _bNoFail ) const;
