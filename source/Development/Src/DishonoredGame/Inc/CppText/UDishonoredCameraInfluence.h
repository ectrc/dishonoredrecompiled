// UDishonoredCameraInfluence cpptext: included inside the generated class body (DishonoredGameCameraClasses.h).
// DISHONORED(port): agent FZ (PHASE18 FZ). The base of every player-camera influence. Retail vtable after the UObject
// slots (vtables.csv, 2012 slots 72..80; 2013 is one slot later): OnTeleport, PostBeginPlay_Derived, OnActivate_Derived,
// OnDeactivate_Derived, an unnamed empty slot, AdjustCameraCollision (the FVector&,FLOAT&,FLOAT call UpdateViewTarget
// makes on every active influence), GatherInfluence_Start, GatherInfluence_Derived, GatherInfluence_FixedTimeStep.
// Non-virtual: SetWeight 2012 0x6fff80 (2013 0x6c49a0 - gained the bWaitToBlendOneFrame argument), GatherInfluence
// 0x700010 (2013 0x6c4a40), OnActivate 0x6fbd90, GetFixedInterpScale 0x6fbd40, PutToSleep 0x6fbdb0, Wake 0x6fbdc0.
// Bodies in Src/dishonoredcamerainfluence.cpp.
public:
	virtual void OnTeleport( class USeqAct_Teleport* Action ) {}
	virtual void PostBeginPlay_Derived() {}
	virtual void OnActivate_Derived( APawn* Pawn ) {}
	virtual void OnDeactivate_Derived( APawn* Pawn ) {}
	virtual void AdjustCameraCollision( FVector& InOutRadius, FLOAT& InOutHeight, FLOAT PawnHeight ) {}
	virtual void GatherInfluence_Start( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime ) {}
	virtual UBOOL GatherInfluence_Derived( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime ) { return FALSE; }
	virtual UBOOL GatherInfluence_FixedTimeStep( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, UBOOL bFirstStep, UBOOL bLastStep ) { return FALSE; }

	void SetWeight( FLOAT TargetWeight, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame = FALSE );
	UBOOL GatherInfluence( struct FDishonoredViewTarget& OutVT, const struct FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime );
	void OnActivate( APawn* Pawn );
	void PutToSleep() { m_bIsSleeping = TRUE; }
	void Wake() { m_bIsSleeping = FALSE; }
	UBOOL IsActive() const { return m_bActive; }
protected:
	FLOAT GetFixedInterpScale() const;
public:
