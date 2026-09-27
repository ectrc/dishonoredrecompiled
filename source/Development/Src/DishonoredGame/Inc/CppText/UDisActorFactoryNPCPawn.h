// UDisActorFactoryNPCPawn cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent CG. A sub-object of ADishonoredSpawner and the only place an ADishonoredNPCPawn is
// constructed. Bodies in Src/disactorfactorynpcpawn.cpp.
public:
	virtual class AActor* GetDefaultActor();
	virtual UBOOL CanCreateActor( FString& OutErrorMsg, UBOOL bFromAssetOnly );
	virtual class AActor* CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
	class ADishonoredNPCPawn* CreateNPCPawn( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData );
