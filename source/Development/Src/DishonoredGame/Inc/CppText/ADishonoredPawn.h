// ADishonoredPawn cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): Serialize 2013 rva 0x748ee0 (2012 0x78e1f0); PossessedBy 0x748f60; the IDisTweaksInterface slots
// GetTweaks_Derived 0x749110 / SetTweaks_Derived 0x74d350 (m_pPawnTweaks) / ApplyTweakChanges_Derived 0x769720 with
// ApplyTweakChanges_Body 0x757060. Bodies in dishonoredpawn.cpp.
public:
	virtual void Serialize( FArchive& Ar );
	virtual void PossessedBy( AController* C );
	virtual UObject* GetUObjectInterfaceDisTweaksInterface() { return this; }
	virtual UDisTweaksBase* GetTweaks_Derived();
	virtual void SetTweaks_Derived( UDisTweaksBase* Tweaks );
	virtual void ApplyTweakChanges_Derived();
	void ApplyTweakChanges_Body();
	UDisTweaks_Pawn* GetPawnTweaks() const;
	virtual void OnAddAbstractItem( class UDisSeqAct_AddAbstractItem* Action );
	virtual void OnRemoveAbstractItem( class UDisSeqAct_RemoveAbstractItem* Action );
	virtual void OnGetAbstractItemQuantity( class UDisSeqAct_GetAbstractItemQuantity* Action );
	virtual void OnModifyElixirCount( class UDisSeqAct_ModifyElixirCount* Action );
	virtual void OnModifyAmmo( class UDisSeqAct_ModifyAmmo* Action );
	virtual UBOOL ChooseAndTriggerDeathEvent_Native( UClass* DamageType );
	virtual void PlayDying_Native( AController* Killer, UClass* DamageType, FVector HitLocation );
