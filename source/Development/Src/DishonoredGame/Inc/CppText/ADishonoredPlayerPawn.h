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

// ---- agent FR (PHASE17 FR): the player as a speaker and as a one-shot owner, dishonoredplayerpawn_conversation.cpp ----
// DISHONORED(port): retail routes every speaker call of the player through m_pAudioLogConvActor (an
// ADisPlayerAudioLogDummyActor spawned by PostBeginPlay_Dialog, 2012 rva 0x7063a0) - FireDialogHook 0x6fb4a0,
// FireDialogHookForAudioLog 0x6fb4f0, GetHookParameters_Derived 0x6fb530, AddDialogOneShot 0x6fb5a0, RemoveDialogOneShot
// 0x6fb5c0, GetSpeakerFromOneShotOwner 0x6fb5e0 all go through it; IsAvailableForDialog 0x706580 (2013 0x6ad070), StartDialog
// 0x6fb550 and EndDialog 0x706600 (2013 0x6ad0f0) go through the player master FSM's UStatePlayerMasterInDialog, which is
// agent FK's unported FSM package (agentFO.md section 9.4); GetDialogSaveData 0x6fb580 / _Global 0x6fb590 are the two
// FDisConvSaveData members.
public:
	virtual UObject* GetUObjectInterfaceDisConvOneShotOwnerInterface() { return this; }
	virtual void AddDialogOneShot( class UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner );
	virtual void RemoveDialogOneShot( class UDisDialogVoiceData* _pOneShot );
	virtual class IDisConvSpeakerInterface* GetSpeakerFromOneShotOwner( class IDisConvSpeakerInterface* _pTreeOwner );
	virtual void FireDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator );
	virtual void FireDialogHookForAudioLog( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator );
	virtual struct FDisHookParameters* GetHookParameters_Derived();
	virtual eDisDialogAvailability IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const;
	virtual void StartDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex );
	virtual void EndDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason );
	virtual struct FDisConvSaveData* GetDialogSaveData() { return &m_DialogSaveData; }
	struct FDisConvSaveData* GetDialogSaveData_Global() { return &m_DialogSaveData_Global; }
	virtual void PostBeginPlay_Dialog();
	class IDisConvSpeakerInterface* GetAudioLogSpeaker() const;

// ---- agent FZ (PHASE18 FZ): the player master FSM, the soiree, the player MAT_* five, the camera glue ----
// DISHONORED(port): retail 2012 rvas - PreBeginPlay_Actions 0x720260 (2013 0x6a4a50; the three FSMs' InitFSM and
// m_pMatineeBlender), Tick_PlayerAction 0x6fae20 (2013 0x6a1ca0), MAT_BeginPlayerGroup 0x70fcc0 (2013 0x6ab570),
// MAT_FinishPlayerGroup 0x7051f0 (2013 0x6ab6a0), MAT_BlendOut 0x705120 (2013 0x6ab4a0), MAT_IsUnequipped 0x6fb030 (2013
// 0x6a1e80), MAT_SetForceInventory 0x6fe570 (2013 0x6a4d40), SetBodyMode 0x702070 (2013 0x6a7c20), EnableControllerInput
// 0x702550, EnableCoreCamInfluence 0x70be10 (2013 0x6aba00), DisableArmFollow 0x704d50 (2013 0x6a1cf0), GetCameraBone
// 0x705540 (2013 0x6aba60), GetCameraPos 0x70bdf0, StopSneaking 0x70d570 (2013 0x6acd960, partial), CancelAllActivePowers
// 0x7048c0 (not ported: the power components have no bodies). Retail puts the MAT_ five on APawn's vtable; here they stay
// on the player class and interpgroupplayer.cpp casts, as FS did for the NPC half. Retail reaches PreBeginPlay_Actions and
// Tick_PlayerAction through ADishonoredPawn::PreBeginPlay (2012 0x78e220) and ADishonoredPawn::Tick -> Tick_Begin (0x715a00),
// neither of which exists here; the two overrides below are the stand-ins and chain to Super first.
// Bodies in dishonoredplayerpawn_action.cpp, dishonoredplayerpawn.cpp (SetBodyMode, EnableControllerInput, Tick,
// PreBeginPlay), dishonoredplayerpawn_camera.cpp (EnableCoreCamInfluence, GetCameraBone, GetCameraPos) and
// dishonoredplayerpawn_stealth.cpp (StopSneaking).
public:
	virtual void PreBeginPlay();
	virtual UBOOL Tick( FLOAT DeltaTime, ELevelTick TickType );
	virtual void PreBeginPlay_Actions();
	virtual UBOOL Tick_PlayerAction( FLOAT DeltaTime, ELevelTick TickType );
	virtual void MAT_BeginPlayerGroup( class UInterpGroupPlayer* GroupPlayer, class UInterpGroupInstPlayer* GroupInstPlayer );
	virtual void MAT_FinishPlayerGroup( class UInterpGroupPlayer* GroupPlayer, class UInterpGroupInstPlayer* GroupInstPlayer );
	virtual void MAT_BlendOut( class UInterpGroupInst* PlayerGroup );
	virtual UBOOL MAT_IsUnequipped() const;
	virtual void MAT_SetForceInventory( UBOOL bShouldForceEmptyHand );
	void SetBodyMode( BYTE BodyMode );
	void EnableControllerInput( INT Mask, UBOOL bEnable, DisInputMaskLevel Level );
	void EnableCoreCamInfluence( const UClass* InfluenceClass, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame = FALSE );
	void DisableArmFollow( BYTE Usage, UBOOL bDisable, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame = FALSE );
	void GetCameraBone( FVector* OutLocation, FRotator* OutRotation ) const;
	virtual FVector GetCameraPos() const;
	void StopSneaking( UBOOL bSkipTransitions );
	void CancelAllActivePowers() const;
	UDishonoredNativeStateMachine* GetPlayerMasterFSM() const { return m_pPlayerMasterFSM; }
