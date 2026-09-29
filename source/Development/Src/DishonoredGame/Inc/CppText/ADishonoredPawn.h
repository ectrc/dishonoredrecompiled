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
	// ---- agent BF (PHASE8 BF): the attributes system and the fall natives ----
	// Bodies in dishonoredpawn_attributes.cpp (GetAttributes 2013 rva 0x7497b0, PreBeginPlay_Attributes 0x762190,
	// OnDifficultyChange 2012 0x794cd0), dishonoredpawn_body.cpp (TakeFallingDamage_Native 0x74dcc0 + its exec 0x5ec5f0)
	// and dishonoredpawn.cpp (Landed_Native 0x74d120 + its exec 0x5ec7e0).
	virtual UObject* GetUObjectInterfaceDisAttributesInterface() { return this; }
	virtual class UDisAttributes& GetAttributes();
	virtual void PreBeginPlay_Attributes();
	/** Retail takes the FArkGameEvent that fired it; FArkGameEventDispatcher is unported, so it takes nothing. */
	void OnDifficultyChange();

	virtual INT TakeFallingDamage_Native( FVector HitNormal, class AActor* FloorActor );
	virtual void Landed_Native( FVector HitNormal, class AActor* FloorActor );
	/** 2013 rva 0x74a340; body in dishonoredpawn_health.cpp. */
	virtual void TakeDamage( INT Damage, class AController* InstigatedBy, FVector HitLocation, FVector Momentum,
	                         UClass* DamageType, struct FTraceHitInfo HitInfo = FTraceHitInfo(EC_EventParm),
	                         class AActor* DamageCauser = NULL );

	// ---- agent DI (PHASE10 DI): the PostBeginPlay chain that builds the modular character ----
	// PostBeginPlay 2013 rva 0x75bce0 (2012 0x7995f0), PostBeginPlay_Body 0x76a130 (0x7a4b70), bodies in
	// dishonoredpawn.cpp and dishonoredpawn_body.cpp. PostBeginPlay_Body is the virtual ADishonoredNPCPawn overrides
	// to give itself a head mesh, and nothing in this tree called it before.
	virtual void PostBeginPlay();
	virtual void PostBeginPlay_Body();
	/** 2013 rva 0x757730 (2012 0x7956a0); body in dishonoredpawn_body.cpp. */
	void GetBone_ByName( FName _BoneName, FVector* _pPos, FRotator* _pRotator ) const;

	/**
	 * DISHONORED(port): agent DO follow-up. 2013 rva 0x7497c0 (2012 0x78e8c0) - m_bSprinting and the one notification
	 * that goes with it; body in dishonoredpawn_attributes.cpp, retail's own unit for it.
	 * DISHONORED(bringup): OnSprintChange_Derived is empty in the base. ADishonoredPlayerPawn overrides it
	 * (2012 rva 0x70fc00) and that override is not ported, so a sprint change reaches no animation or camera yet.
	 */
	virtual void SetSprinting( UBOOL bSprinting );
	virtual void OnSprintChange_Derived() {}

	// DISHONORED(port): agent ED (PHASE11 ED), 2012 rva 0x79b360 - ported only as far as AActor::GameLoad,
	// which is where the player's transform comes back; it then stops the stream and says so. GameSave is
	// not ported and not declared (the writing half of the object layer does not exist here). Body in
	// dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
