// ADisPickup_Base cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(written): agent AU. The collection path, bodies in dispickup_base.cpp with their 2013 rvas:
// PostBeginPlay 0x62b4c0 (2012 0x680a00), Tick 0x62b130 (0x6806b0), ShouldTrace 0x61d820 (0x66c900),
// GetCrosshairStatus 0x631ba0 (0x682c50), AttemptInteract_Derived 0x631bf0 (0x682ca0),
// AttemptCannotUseInteract_Derived 0x62adc0 (0x680350), ConsumePickup 0x61d770 (0x66c840),
// StartPickupTravel 0x62ab40 (0x6800d0), Steal 0x63f8d0 (0x680650), Attach 0x62b0a0 (0x6805c0),
// Detach 0x6226c0 (0x6705f0), BaseChange 0x61dbb0 (0x6706b0), ClearComponents 0x622750, PostLoad 0x619a60,
// ApplyTweakChanges_Derived 0x62aac0, GetInteractableTweaks_Derived 0x62ad90, IsUseBlockedByPossession 0x62ad30,
// GetPlayerPawnFromUser 0x62afe0, WantsTick_Derived 0x619be0, OnRigidBodyStatusChange 0x61d6e0,
// IgnoreBlockingBy 0x61d6a0, setPhysics 0x62b440, OnActorTerminated 0x62acd0, HasSoul 0x62af90,
// GetMovableWeightClass 0x62b060, CanSplash (2012 0x66c820), TakeDamage_Impl 0x619a90.
public:
	virtual void PostBeginPlay();
	virtual void PostLoad();
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	virtual UBOOL ShouldTrace( UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags );
	virtual void ClearComponents();
	virtual void setPhysics( BYTE NewPhysics, AActor* NewFloor = NULL, FVector NewFloorV = FVector(0,0,1) );
	virtual UBOOL IgnoreBlockingBy( const AActor* Other ) const;

	/** IDisInteractableInterface */
	virtual UObject* GetUObjectInterfaceDisInteractableInterface() { return this; }
	virtual INT* GetHighlightFlags() { return &m_HighlightFlags; }
	virtual const class UDisTweaks_InteractableInterface* GetInteractableTweaks_Derived() const;
	virtual UBOOL AttemptInteract_Derived( class ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed );
	virtual UBOOL AttemptCannotUseInteract_Derived( class ADishonoredPawn* Pawn, UBOOL& bOutCanBeWitnessed );

	/** IDisSoulRenderInterface (the Heart) */
	virtual UBOOL HasSoul( INT PowerLevel ) const;

	/** IDisMovableInterface */
	virtual BYTE GetMovableWeightClass() const;

	/** Arkane virtuals of the pickup itself; the retail slots are +952 CanBePickedUp, +956 WantsTick_Derived,
	 *  +960 DoInteract_Impl on the actor vtable. */
	virtual UBOOL CanBePickedUp( class ADishonoredPlayerPawn* PlayerPawn ) const { return TRUE; }
	virtual UBOOL WantsTick_Derived() const;
	virtual UBOOL DoInteract_Impl( class ADishonoredPlayerPawn* PlayerPawn ) { return FALSE; }
	virtual enum eCrossHairStatus GetCrosshairStatus( class ADishonoredPawn* Pawn ) const;
	virtual UBOOL CanSplash();
	virtual void OnRigidBodyStatusChange();
	virtual void OnActorTerminated();
	virtual void ApplyTweakChanges_Derived();
	virtual void TakeDamage_Impl( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const struct FTraceHitInfo& HitInfo, AActor* const DamageCauser );
	virtual void Steal( class ADishonoredPlayerPawn* PlayerPawn );
	virtual void BaseChange();

	void ConsumePickup();
	void StartPickupTravel( class ADishonoredPawn* Pawn );
	void Attach( AActor* Other, class USkeletalMeshComponent* SkelComp );
	void Detach();
	void AdjustDetachedPosition();

	class UDisTweaks_PickupBase* GetPickupTweaks() const;
	class ADishonoredPlayerPawn* GetPlayerPawnFromUser( class ADishonoredPawn* Pawn ) const;
	UBOOL IsUseBlockedByPossession( const class ADishonoredPlayerPawn* Player ) const;
