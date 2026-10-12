// UDishonoredCameraInfluenceGroup cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). Retail 2012 rvas (dishonoredcamerainfluencegroup.cpp): PostBeginPlay 0x71bac0
// (2013 0x6cfbb0), GatherInfluences 0x70e110 (2013 0x6cfcb0), SetWeight 0x709770 (2013 0x6c9c70 - gained the
// bWaitToBlendOneFrame argument), GetInfluence 0x709710 (2013 0x6c9c10), WeightRotator 0x6fbdf0 (2013 0x6c00a0),
// OnTeleport 0x7099c0. Bodies in Src/dishonoredcamerainfluencegroup.cpp.
public:
	void PostBeginPlay( class ADishonoredPlayerCamera* OwningCam );
	UBOOL GatherInfluences( struct FDishonoredViewTarget& OutVT, APawn* Pawn, FLOAT DeltaTime, FLOAT UncappedDeltaTime );
	void SetWeight( FLOAT Weight, const UClass* InfluenceType, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame = FALSE );
	class UDishonoredCameraInfluence* GetInfluence( const UClass* InfluenceType );
	void OnTeleport( class USeqAct_Teleport* Action );
protected:
	void WeightRotator( FRotator& InOut, const FRotator& CurRot, FLOAT Weight );
public:
