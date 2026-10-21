// ADishonoredPlayerPawn cpptext: included inside the generated class body (DishonoredGameClasses.h).
public:
	// DISHONORED(written): ?s_pInstance@ADishonoredPlayerPawn@@0PAV1@A, set by the constructor for every pawn that is not a
	// template (class default object or archetype, itself or through an outer)
	static ADishonoredPlayerPawn* s_pInstance;

	ADishonoredPlayerPawn();
	/** agent IH4: 2012 0x70ef70, clears s_pInstance (body in dishonoredplayerpawn.cpp). */
	virtual void ShutPawnDown_Derived();
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

// ---- agent GF (PHASE19 GF): the first tick phase, and the arms' camera pass ----
// DISHONORED(port): Tick_Begin 2012 0x715a00 / 2013 0x6b0850 (269 B both; the 2013 one matched by neighbours and checked: the same
// mesh-transform update, the camera bone written into the audio-log dummy actor, the Tick_PlayerAction vtable call, the corpse-drop
// bits). OnCameraUpdate_Derived 2012 0x71e3b0 / 2013 0x6abdd0 (1227 / 1242 B; the 2013 body checked: the same three skel controls off
// m_pCameraTweaks, SetOffset / SetFOV / DisableFOV, the upper FSM and the powers on pass 1). Bodies in dishonoredplayerpawn.cpp and
// dishonoredplayerpawn_camera.cpp.
public:
	virtual UBOOL Tick_Begin( FLOAT _fDeltaTime, ELevelTick _TickType );
	virtual void OnCameraUpdate_Derived( const struct FDishonoredViewTarget& _VT, INT _PassCount );
// ---- agent GF (PHASE19 GF, the coordinator's message from GB): the player's root-motion mode overrides ----
// DISHONORED(port): StopRootMotionMode 2012 0x6facb0 (15 B, protected virtual; 2013 0x6a1af0, unnamed, by vtable neighbours) =
// ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore ); ChangeAnimStateMotionModes 2012 0x6fae70 (222 B) / 2013 0x6ab1d0 (375 B,
// unnamed, matched by vtable and checked by shape: Mesh +908 RootMotionRotationMode, +902 PendingRMM, the pawn's +2234
// m_NextPhysicsMode, setPhysics at vtable +536; the 2013 body also stops an active Blink power on the RMM_Translate side).
// Without them GA's unconditional base StopRootMotionMode put the player mesh in RMM_Velocity at the ride's first second and
// APawn::physWalking took Velocity from the zero root-motion delta: the player could not walk after the ride.
// Bodies in dishonoredplayerpawn_action.cpp.
public:
	virtual void ChangeAnimStateMotionModes( BYTE _RootMotion, BYTE _RootRotation );
protected:
	virtual void StopRootMotionMode();
public:

// ---- agent GJ (PHASE20 GJ): the player's half of the equip path ----
// DISHONORED(port): 2012 / 2013 rvas - PostBeginPlay_Inventory 0x718360 / 0x6ad200, EquipItemByType 0x70ce90 / 0x6b3210 (the
// equip-change action it requests from the hand's FSM is a skeleton: every equip is taken the instant way, said once),
// TransitionItemIn 0x6fb760 / 0x6a2590, GetItemClassBeingEquipped 0x706d80 (NULL: no UStatePlayerEquipChange to read),
// GetDesiredPlayerStance 0x6fb250 / 0x6a2120, GetActualPlayerStance 0x6fb220 / 0x6a20f0, SetPlayerStance 0x6fb200 (20 B),
// IsLeftHandDisabled 0x6fb110 (m_bLeftHandDisabled), IsSneaking 0x707470 (agent IF: moved to retail's unit,
// dishonoredplayerpawn_stealth.cpp, with the 2013 body). Bodies in dishonoredplayerpawn_inventory.cpp.
public:
	virtual void PostBeginPlay_Inventory();
	virtual UBOOL EquipItemByType( UClass* _pType, BYTE _EquipUsage, INT _Flags );
	BYTE TransitionItemIn( BYTE _EquipUsage, BYTE _ActionUsage, UBOOL _bFromOffscreen, UBOOL _bUseFastEquip );
	virtual UClass* GetItemClassBeingEquipped( BYTE _EquipUsage ) const;
	BYTE GetDesiredPlayerStance( BYTE _EquipUsage ) const;
	BYTE GetActualPlayerStance( BYTE _EquipUsage ) const;
	void SetPlayerStance( BYTE _EquipUsage, BYTE _Stance ) { m_PlayerStance[_EquipUsage] = _Stance; }
	UBOOL IsLeftHandDisabled() const { return m_bLeftHandDisabled; }
	virtual UBOOL IsSneaking() const;
// ---- agent HE (PHASE22 HE): the stance icon's gates. DisIsPossessing 2012 0x6fb7d0 (retail's is APawn virtual +1188, called through
// ADishonoredPawn::IsPossessing 2012 0x7aac60 / 2013 0x74a550), IsSliding 2012 0x6fec70. Bodies in dishonoredplayerpawn_inventory.cpp.
	UBOOL DisIsPossessing() const;
	UBOOL IsSliding() const;
// ---- agent GI (PHASE20 GI): the player half of the attention target ----
// CollectAttnTargetParams 2012 0x6fb080 / 2013 0x6a1ed0 (the base plus m_bIsPlayer), IsAttnTargetPlayer (a folded `return TRUE`),
// AllowAttentionGain 0x6fb0a0 (TRUE unless possessing - the possession state is not ported here, named in the body),
// OnAttentionReceived 0x705250 / 0x6ab710 and OnAttentionLost 0x6fe650 (the m_pAttentiveBrainsList chain the stealth HUD walks),
// CanAttnTargetBeAttacked 0x705290 / 0x6ab750. Bodies in Src/dishonoredpawn_attentiontarget.cpp.
public:
	virtual void CollectAttnTargetParams( struct FDisAttnTargetParams& _rOut_Params );
	virtual UBOOL IsAttnTargetPlayer() const { return TRUE; }
	virtual UBOOL AllowAttentionGain() const;
	virtual void OnAttentionReceived( class UDishonoredAIBrain* _pBrain );
	virtual void OnAttentionLost( class UDishonoredAIBrain* _pBrain );
	virtual UBOOL CanAttnTargetBeAttacked() const;
// ---- agent HD (PHASE22 HD): the pawn's facing. FaceRotation 2012 0x70afd0 / 2013 0x6b0d40, vtable override of APawn's; body and exec
// in Src/dishonoredplayerpawn.cpp ----
public:
	virtual void FaceRotation( FRotator NewRotation, FLOAT DeltaTime );
// ---- agent HL (PHASE22 HL): the player Kismet toggles HI's PlayerAction target rule reaches. 2012 / 2013 rvas: OnToggleTutorial
// 0x704680 / 0x6aa300, OnToggleAchievementEval 0x704080 / 0x6a9df0, OnToggleChoke 0x704480 / 0x6b1bd0, OnToggleJournal 0x70b200 /
// 0x6b5c40, OnTogglePowerWheel 0x70b130 / 0x6b5b60. Bodies and execs in Src/dishonoredplayerpawn.cpp ----
public:
	virtual void OnToggleTutorial( class UDisSeqAct_ToggleTutorial* _pAction );
	virtual void OnToggleAchievementEval( class UDisSeqAct_ToggleAchievementEval* _pAction );
	virtual void OnToggleChoke( class UDisSeqAct_ToggleChoke* _pAction );
	virtual void OnToggleJournal( class UDisSeqAct_ToggleJournal* _pAction );
	virtual void OnTogglePowerWheel( class UDisSeqAct_TogglePowerWheel* _pAction );
// ---- agent IF (PHASE23 IF): crouch (the sneak toggle), sprint, jump and swim ----
// 2012 / 2013 rvas: StartSneaking 0x70d450 / 0x6d3860, AttemptChangeToSneakHeight 0x7074c0 / 0x6cda60, AttemptChangeCollisionHeight
// 0x6feca0 / 0x6c3430, StartCrouch 0x6fcfe0 / 0x6a3720, EndCrouch 0x6fd080 / 0x6a37c0, UnCrouch 0x70ae50, GetSpeedFraction 0x6fb8a0 /
// 0x6c33b0, IsCarryingCorpse 0x6febc0 / 0x6a5220, MaxSpeedModifier_Derived 0x6fe140 / 0x6a4700, DoJump 0x715fe0 / 0x6a3820,
// DoJump_Mantle 0x712b10 / 0x6dbe70, IsMantleRequested 0x707170 / 0x6c8000, IsPressingForward 0x7071a0 / 0x6c8030, PerformMantle
// 0x70ff10 (agent IG's), setPhysics 0x6fe380 / 0x6a4b50, RequestAction 0x70bb90 / 0x6b1d00, RequestActionHelper 0x704ad0 /
// 0x6aad90, GetSwimmingState 0x705510 / 0x6ab9d0, GetApneaDuration 0x709050 / 0x6cf570, OnEnterWaterVolume 0x6faa30 / 0x6a15a0,
// OnExitWaterVolume 0x70b300 / 0x6b11d0, OnChangeWaterVolume 0x70dea0 / 0x6d4120, Tick_Water 0x708ef0 / 0x6d7be0,
// OnCameraEnterWaterVolume 0x6ffdc0 / 0x6c46a0, OnCameraExitWaterVolume 0x7090c0 / 0x6cf5e0. Bodies in
// dishonoredplayerpawn_stealth.cpp, dishonoredplayerpawn.cpp, dishonoredplayerpawn_inventory.cpp, dishonoredplayerpawn_mantle.cpp,
// dishonoredplayerpawn_action.cpp, dishonoredplayerpawn_body.cpp and displayerpawn_water.cpp (retail's units).
public:
	void StartSneaking( UBOOL bSkipTransitions );
	virtual void StartCrouch( FLOAT HeightAdjust );
	virtual void EndCrouch( FLOAT HeightAdjust );
	virtual void UnCrouch( INT bClientSimulation = 0 );
	virtual FLOAT GetSpeedFraction() const;
	virtual UBOOL IsCarryingCorpse() const;
	virtual UBOOL DoJump( UBOOL bUpdating );
	virtual void setPhysics( BYTE NewPhysics, AActor* NewFloor = NULL, FVector NewFloorV = FVector( 0, 0, 1 ) );
	UBOOL RequestAction( BYTE ActionUsage, struct FStateSharedActionBase_PlayerOnly_Param& ActionParam, UObject* FiringObject, UBOOL bCheckOnly );
	UBOOL PerformMantle( UBOOL bFromJump, UBOOL bFromBlink );
	BYTE GetSwimmingState() const;
	FLOAT GetApneaDuration() const;
	virtual void OnEnterWaterVolume( class ADishonoredWaterVolume* Volume );
	virtual void OnExitWaterVolume( class ADishonoredWaterVolume* Volume );
	virtual void OnChangeWaterVolume( class ADishonoredWaterVolume* OldVolume, class ADishonoredWaterVolume* NewVolume );
	virtual void Tick_Water( FLOAT DeltaTime, ELevelTick TickType );
protected:
	UBOOL AttemptChangeToSneakHeight();
	UBOOL AttemptChangeCollisionHeight( FLOAT NewHeight );
	virtual FLOAT MaxSpeedModifier_Derived();
	UBOOL RequestActionHelper( UDishonoredNativeStateMachine* FSM, UBOOL bIsDeathAction, struct FStateSharedActionBase_PlayerOnly_Param& ActionParam, UObject* FiringObject, UBOOL bCheckOnly );
private:
	UBOOL DoJump_Mantle();
	UBOOL IsMantleRequested() const;
	UBOOL IsPressingForward() const;
	void OnCameraEnterWaterVolume();
	void OnCameraExitWaterVolume();
public:
// ---- agent IG (PHASE23 IG): the mantle ----
// 2012 / 2013 rvas: IsMantleAllowed 0x6fb830, ClearVisAndMantleSettings 0x707400 / 0x6d7a20, DoMantle_StepUp 0x706f00 / 0x6d35e0,
// DoMantle_Anim 0x70d380 / 0x6cd820, Tick_Mantle 0x7129d0 / 0x6dbd00. Bodies in dishonoredplayerpawn_mantle.cpp and
// dishonoredplayerpawn_stealth.cpp (ClearVisAndMantleSettings, retail's unit).
// ApplyTweakChanges_Derived 0x71d1d0 / 0x6be9b0 (the IDisTweaksInterface slot; partial: the edge finders' tweaks, then the base).
public:
	UBOOL IsMantleAllowed() const;
	void ClearVisAndMantleSettings();
	virtual void ApplyTweakChanges_Derived();
private:
	void DoMantle_StepUp( FLOAT MantleHeight, const FVector& MantleStartLocation, INT MantleStartYaw );
	void DoMantle_Anim( FLOAT MantleHeight, const FVector& MantleStartLocation, INT MantleStartYaw, UBOOL bImpactAnimRequired, UBOOL bCrouchRequired, UBOOL bCancelable );
	void Tick_Mantle( FLOAT DeltaTime, ELevelTick TickType );
public:
// ---- agent IM (PHASE23 IM): the anim-state system of the player (the three pickers, the state maps, SetAnimState) ----
// 2012 / 2013 rvas: PreBeginPlay_AnimStates 0x7200e0 / 0x6e2060, FindStatePickers 0x71ed50 / 0x6e0f90, BuildAnimStateMap 0x71b9b0 /
// 0x6dfc70, GatherAnimStateArrays 0x704a00 / 0x6aac90, FindAnimState 0x71f740 / 0x6e16c0, AnimState_AddItemPrefix 0x71f800 / 0x6e1780,
// SetAnimState 0x71fe40 / 0x6e1da0, SupportsAnimState 0x71ff40 / 0x6e1ea0, ClearAnimState 0x6fbae0, IsCurAnimStateFullWeight 0x6fbb00,
// FindNodeInCurrentAnimStateByName 0x6fbb10, AnimState_GetSkeletalComponent 0x71eb80, AnimState_GetAnimStateComp 0x71eb90,
// AnimState_TriggerExit_Derived 0x704b30 / 0x6aadf0, AnimState_LockoutPicker_Derived 0x704be0 / 0x6aaea0, AnimState_SyncEnd 0x704d00 /
// 0x6aafc0, AnimState_PlayAnim_Derived 0x705820 / 0x6ac2f0. Bodies in displayerpawn_animstates.cpp and dishonoredplayerpawn_action.cpp.
public:
	void PreBeginPlay_AnimStates();
	UBOOL FindAnimState( INT StateType, FName StateName, struct FDisPawnAnimState*& OutState );
	struct FDisPawnAnimState* SetAnimState( FName StateName, INT StateType, class UDishonoredNativeState* AttachToState, FLOAT BlendTime, UObject* FiringObj );
	UBOOL SupportsAnimState( FName StateName, INT StateType );
	void ClearAnimState( INT StateType, FLOAT BlendTime );
	UBOOL IsCurAnimStateFullWeight( INT StateType );
	class UAnimNode* FindNodeInCurrentAnimStateByName( INT StateType, const FName& NodeName ) const;
	virtual class USkeletalMeshComponent* AnimState_GetSkeletalComponent() { return Mesh; }
	virtual class UDisAnimStateComponent* AnimState_GetAnimStateComp() { return m_pAnimStateComp; }
	virtual void AnimState_SyncEnd( INT StateType );
protected:
	void FindStatePickers();
	void BuildAnimStateMap( struct FDisPawnAnimState_PickerInfo& PickerInfo );
	void GatherAnimStateArrays( INT StateType, struct FDisPawnAnimState_PickerInfo& PickerInfo );
	struct FDisPawnAnimState* AnimState_AddItemPrefix( struct FDisPawnAnimState_PickerInfo& PickerInfo, INT StateType, const struct FDisPawnAnimState* State, FName& StateName, BYTE EquipUsage );
	virtual UBOOL AnimState_TriggerExit_Derived( INT StateType, class UDishonoredNativeState* AttachedState, FLOAT TimeToGo );
	virtual void AnimState_LockoutPicker_Derived( INT StateType, FLOAT TimeToGo, UBOOL bLocked );
	virtual UBOOL AnimState_PlayAnim_Derived( class UAnimNodeSequence* Seq );
public:
// agent IP (PHASE23 IP): GetAimStability 2012 0x7032f0 / 2013 0x6a9030 (248 B), the out-of-breath weight (body in dishonoredplayerpawn.cpp).
public:
	FLOAT GetAimStability() const;
// agent JG (PHASE24 JG): ApplyAttributes 2012 0x701cf0 / 2013 0x6a7910 (258 B both), performPhysics 2012 0x6fd0e0 / 2013 0x6a1670 (148 B
// both). Bodies in dishonoredplayerpawn.cpp.
public:
	virtual void ApplyAttributes();
	virtual void performPhysics( FLOAT DeltaSeconds );
// agent JH (PHASE24 JH): FillUIInteractions 2012 0x7033f0 / 2013 0x6a9130, the interaction prompt's context (body in
// dishonoredplayerpawn.cpp).
public:
	void FillUIInteractions( struct FDisUIInteractionContext& _rContext ) const;
// agent JH (PHASE24 JH): HasKey 2012 0x70fef0 (body in dishonoredplayerpawn_inventory.cpp)
public:
	virtual UBOOL HasKey( const FString& _rKey ) const;
