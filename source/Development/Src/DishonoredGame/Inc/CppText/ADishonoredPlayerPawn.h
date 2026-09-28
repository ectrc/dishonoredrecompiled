// ADishonoredPlayerPawn cpptext: included inside the generated class body (DishonoredGameClasses.h).
public:
	// DISHONORED(written): ?s_pInstance@ADishonoredPlayerPawn@@0PAV1@A, set by the constructor for every pawn that is not a
	// template (class default object or archetype, itself or through an outer)
	static ADishonoredPlayerPawn* s_pInstance;

	ADishonoredPlayerPawn();
	// ---- agent BF (PHASE8 BF): the attributes system and the fall natives ----
	// Bodies in dishonoredplayerpawn_body.cpp (TakeFallingDamage_Native 2013 rva 0x6a4fa0) and
	// dishonoredplayerpawn.cpp (Landed_Native 0x6b53a0).
	virtual INT TakeFallingDamage_Native( FVector HitNormal, class AActor* FloorActor );
	virtual void Landed_Native( FVector HitNormal, class AActor* FloorActor );

	/**
	 * DISHONORED(port): agent DO. 2013 rva 0x6ac910 (2012 0x705e30) - the HEALTH half of the ACTOR feed: each
	 * m_HealthEffects entry that applies post-process and carries weight blends its own FArkPpConfig in. Body in
	 * dishonoredplayerpawn_combat.cpp, which is retail's unit for it.
	 */
	void ApplyHealthEffectsPost( struct FArkPpConfig& Config );
