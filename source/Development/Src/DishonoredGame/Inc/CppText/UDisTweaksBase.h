// UDisTweaksBase cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): retail vtable order after the UObject slots (2012 PDB UDisTweaksBase_vtbl, 2013 = 2012 + 4):
// EditConditionAskObj_IsConditionMet, EditConditionAskObj_SetCondition (editor, not ported), GetSpawnedObjectClass (+300),
// ApplyFallbackChain_Derived, GatherTweakChildren_Derived, FixupDefaults_Derived, SpawnActor_Derived (+316). Bodies in
// distweaksbase.cpp with their 2013 rvas. The tweak subclasses' GatherTweakChildren_Derived overrides (sub-tweak lists) are
// not ported yet, so the fallback chain covers the top-level tweak objects only.
public:
	virtual void Serialize( FArchive& Ar );
	virtual void PostLoad();
	virtual UClass* GetSpawnedObjectClass( BYTE SpawnType ) const;
	virtual void ApplyFallbackChain_Derived() {}
	virtual void GatherTweakChildren_Derived( TArray<FDisTweakChildInfo>& Children ) {}
	virtual UBOOL FixupDefaults_Derived() { return FALSE; }
	AActor* SpawnActor( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const;
	UBOOL CanSpawnObjectOfClass( const UClass* Class, BYTE SpawnType ) const;
	UBOOL IsFallbackDerivedFrom( const UDisTweaksBase* Tweaks ) const;
	UBOOL IsFallbackRelated( const UDisTweaksBase* Tweaks ) const;
	void ApplyFallbackChain();
	void SkipFallback( const FName& PropertyName );
protected:
	virtual AActor* SpawnActor_Derived( BYTE SpawnType, FName InName, const FVector& Location, const FRotator& Rotation, AActor* Template, UBOOL bNoCollisionFail, UBOOL bRemoteOwned, AActor* Owner, APawn* Instigator, UBOOL bNoFail ) const;
	INT FindFallbackSkip( const FString& PropertyPath ) const;
	void ApplyFallbackChain_Struct( const UStruct* Struct, const UProperty* OwnerProperty, const FString& CurPath, INT ParentOffset );
	void InvalidateFallback_Recurse();
