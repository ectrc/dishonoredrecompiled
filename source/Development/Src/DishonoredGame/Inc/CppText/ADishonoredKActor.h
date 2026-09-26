// ADishonoredKActor cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(written): agent AU. PropagateMaxDrawDistance 2013 rva 0x6210b0 (2012 0x66f6c0),
// TakeDamage_Native 0x621010 (0x66f620), DestroyIfPlayerCantSeeMe 0x6185f0 (0x669870). Bodies in dishonoredkactor.cpp.
// TakeDamage_Impl is retail's vtable +940 on this class; ADisPickup_Base overrides it (2013 rva 0x619a90).
public:
	void PropagateMaxDrawDistance();
	virtual void TakeDamage_Native( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const struct FTraceHitInfo& HitInfo, AActor* const DamageCauser );
	virtual void TakeDamage_Impl( INT Damage, AController* const InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* const DamageType, const struct FTraceHitInfo& HitInfo, AActor* const DamageCauser ) {}
	virtual void DestroyIfPlayerCantSeeMe();
