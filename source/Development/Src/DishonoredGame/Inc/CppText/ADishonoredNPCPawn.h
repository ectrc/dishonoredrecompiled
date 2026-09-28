// ADishonoredNPCPawn cpptext: included inside the generated class body (dishonoredgameclasses.h).
// ---- agent BF (PHASE8 BF) ----
// The class re-declares TakeFallingDamage_Native in script but has no C++ override, so only the generated exec wrapper is
// needed; it dispatches through the virtual to ADishonoredPawn::TakeFallingDamage_Native (2013 rva 0x74dcc0). Body in
// dishonorednpcpawn_body.cpp.
// ---- agent CG (PHASE9 CG) ----
// SetDesiredRotation 2013 rva 0x76e960 (body in dishonorednpcpawn_rotation.cpp) is retail's 8-byte `return TRUE`: the
// NPC overrides the script entry point and does not feed it into the rotation-intent system.
// OnOverridePossess 2013 rva 0x7748f0 (body in dishonorednpcpawn_possession.cpp).
public:
	virtual UBOOL SetDesiredRotation( FRotator _TargetDesiredRotation, UBOOL _bInLockDesiredRotation, UBOOL _bInUnlockWhenReached, FLOAT _InterpolationTime, UBOOL _bResetRotationRate );
	virtual void OnOverridePossess( class UDisSeqAct_OverridePossess* _pAction );

// ---- agent CG (PHASE9 CG): the accessors the brain reads every frame ----
// The body intention is an array indexed by priority: each of the three "current" priority bytes says which entry is
// the live one for that aspect, which is how a matinee can override the stance without losing the AI's own intent.
public:
	BYTE GetBodyStance() const;
	class UClass* GetDesiredPrimaryItem() const;
	class UClass* GetDesiredSecondaryItem() const;
	void SetBodyIntention( BYTE _Priority, BYTE _BodyStance, class UClass* _pPrimaryItemClass, class UClass* _pSecondaryItemClass );
	UBOOL ShouldAIBeNotifiedOfRelationshipChange() const;
	void AcknowledgeRelationshipChangeHandledByAI();
	class UDishonoredAIBrain* GetAIBrain();
	UBOOL IsAFighter() const;

	// ---- agent CG: the damage entry point ----
public:
	virtual void TakeDamage_Native( INT& _rDamage, class AController* _pInstigatedBy, FVector _HitLocation, FVector& _rMomentum, class UClass* _pDamageType, struct FTraceHitInfo _HitInfo, class AActor* _pDamageCauser );
	// ---- agent CG natives sweep, round 2 ----
	virtual void OnSpawnStealable( class UDisSeqAct_SpawnStealable* _pAction );

// ---- agent DI (PHASE10 DI): the modular character - body mesh plus head mesh plus accessories ----
// PostBeginPlay 2013 rva 0x76a610 (2012 0x7c8770), PostBeginPlay_Body 0x77f8a0 (0x7bc180),
// CreateAccessoryStaticMeshComponent 0x7706b0 (0x7ae5f0), GetAccessoryStaticMeshComponent 0x76e0f0 (0x7abc40).
// Bodies in dishonorednpcpawn.cpp and dishonorednpcpawn_body.cpp.
public:
	virtual void PostBeginPlay();
	virtual void PostBeginPlay_Body();
	/** Retail has this private; it is public here because the census in the same unit reports what it built. */
	class UStaticMeshComponent* CreateAccessoryStaticMeshComponent( BYTE _eAccessoryType );
	class UStaticMeshComponent* GetAccessoryStaticMeshComponent( BYTE _eAccessoryType ) const;

// ---- agent DN (PHASE10 DN): locomotion ----
// Spawned 2013 rva 0x752270 (2012 0x7b0a60) creates the component from the NPC tweaks' UArkComponentLocomotionConfig and
// starts it; physWalking 0x74e8a0 (0x7ad240) hands the frame to it instead of to APawn's walking physics;
// SetupPathfindingParams 0x74ad20 (0x7ab3a0) and SetupPathGoalsAndConstraints 0x75e490 (0x7c2040) are the pawn's half of
// the nav-mesh contract. Bodies in Src/dishonorednpcpawn_locomotion.cpp.
public:
	virtual void Spawned();
	virtual void physWalking( FLOAT DeltaTime, INT Iterations );
	class FArkComponentLocomotion* GetComponentLocomotion() const;
	virtual UBOOL SetupPathGoalsAndConstraints( const FVector& _rFinalDestination, UBOOL _bForReachability );
	UBOOL IsMoving() const;
private:
	virtual void SetupPathfindingParams( struct FNavMeshPathParams& _rOut_ParamCache );
	void UpdateLocomotion();
