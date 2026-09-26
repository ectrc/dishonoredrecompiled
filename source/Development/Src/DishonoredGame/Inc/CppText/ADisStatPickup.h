// ADisStatPickup cpptext: included inside the generated class body (DishonoredGameItemClasses.h).
// DISHONORED(written): agent AU. GetTweaks_Derived 2013 rva 0x3710 (2012 0x66aa40), SetTweaks_Derived 0x61dd90 (0x66cc90). Bodies in disstatpickup.cpp.
public:
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	virtual void PostBeginPlay();
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	virtual UBOOL CanBePickedUp( class ADishonoredPlayerPawn* PlayerPawn ) const;
	virtual UBOOL DoInteract_Impl( class ADishonoredPlayerPawn* PlayerPawn );
	virtual UBOOL WantsTick_Derived() const;
	virtual const FString& GetUseMessage() const;
	virtual void FormatText( FString& Text ) const;

	UBOOL HasStatsSet() const;
	void Explode();
	class UDisTweaks_StatPickup* GetStatTweaks() const;
