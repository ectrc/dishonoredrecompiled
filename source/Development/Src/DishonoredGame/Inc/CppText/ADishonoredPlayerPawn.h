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

	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x6b8b60 - ported as far as the Super call, which
	// is where the transform lands. Bodies in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	void SaveLoadTutorialTrackers( FArchive& _rArchive ) const;	// 2012 rva 0x6fa970
protected:
	void GameLoad_Body( FArchive& _rArchive, ESaveLoadLocation _Location );	// 2012 rva 0x6fe820
public:

	// DISHONORED(port): agent ED (PHASE11 ED), 2012 rva 0x6fa960 (dishonoredplayerpawn.cpp:1541), the body
	// retail shares between this class, UDishonoredGlobalAIManager and UDisNPCTravelManager.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return Location == SLL_FILE; }

	// DISHONORED(port): agent EF (PHASE11 EF) - the rest of GameLoad (2013 rva 0x6b8b60) past the Super call:
	// the mana triple, the inventory, the dialog, the stealth vars, the two player FSMs' partial state, the
	// adrenaline, the key ring, the darkness manager, the crouch, the two stat arrays, the 80 achievement
	// trackers, the upgrades, ten whale-bone charm slots, the charms, the last-second location, the visibility
	// component, the velocity, the climbable, the power-inhibited message and 43 tutorial-note bits.
	// Bodies in dissavegame.cpp.
protected:
	void GameLoad_Inventory( FArchive& _rArchive, ESaveLoadLocation _Location );	// 2013 rva 0x6ad350
	void GameLoad_Stealth( FArchive& _rArchive, ESaveLoadLocation _Location );	// 2013 rva 0x6d3950
	/** retail reaches this through IDisConvSpeakerInterface::GameLoad_Dialog (0x897060); this tree's interface
	    carries only its vptr, so it sits on the class that needs it, as agent EC did for the NPC pawn. */
	void GameLoad_Dialog( FArchive& _rArchive, ESaveLoadLocation _Location );
public:

	// ---- agent EL (PHASE12 EL): the story-flag store the Tower's arrival chain reads and the menu's
	// GoToTowerEmpress sequence writes. Bodies in dishonoredplayerpawn.cpp. ----
	// DISHONORED(port): agent EL, 2013 rvas 0x6b1370 / 0x6b1430 (the UDisStoryFlagSet overloads, which resolve the
	// set to its path name) and 0x6a8c30 / 0x6a8cc0 (the FName overloads, which are the store itself).
	UBOOL CheckStoryFlag( const class UDisStoryFlagSet* _pStoryFlagSet, const FGuid& _rGUID ) const;
	void SetStoryFlag( const class UDisStoryFlagSet* _pStoryFlagSet, const FGuid& _rGUID, UBOOL _bValue );
	UBOOL CheckStoryFlag( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const;
	void SetStoryFlag( const FName& _rStoryFlagSetPath, const FGuid& _rGUID, UBOOL _bValue );

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x6a1a80 (2012 0x6fac40). Body in dishonoredplayerpawn.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
