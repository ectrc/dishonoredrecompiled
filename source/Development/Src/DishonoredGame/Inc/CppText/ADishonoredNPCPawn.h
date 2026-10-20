// ADishonoredNPCPawn cpptext: included inside the generated class body (dishonoredgameclasses.h).
// ---- agent BF (PHASE8 BF) ----
// The class re-declares TakeFallingDamage_Native in script but has no C++ override, so only the generated exec wrapper is
// needed; it dispatches through the virtual to ADishonoredPawn::TakeFallingDamage_Native (2013 rva 0x74dcc0). Body in
// dishonorednpcpawn_body.cpp.
// ---- agent CG (PHASE9 CG) ----
// SetDesiredRotation 2013 rva 0x76e960 (body in dishonorednpcpawn_rotation.cpp) is retail's 8-byte `return TRUE`: the
// NPC overrides the script entry point and does not feed it into the rotation-intent system.
// OnOverridePossess 2013 rva 0x7748f0 (body in dishonorednpcpawn_possession.cpp).
public:
	virtual UBOOL SetDesiredRotation( FRotator _TargetDesiredRotation, UBOOL _bInLockDesiredRotation, UBOOL _bInUnlockWhenReached, FLOAT _InterpolationTime, UBOOL _bResetRotationRate );
	virtual void OnOverridePossess( class UDisSeqAct_OverridePossess* _pAction );

// ---- agent CG (PHASE9 CG): the accessors the brain reads every frame ----
// The body intention is an array indexed by priority: each of the three "current" priority bytes says which entry is
// the live one for that aspect, which is how a matinee can override the stance without losing the AI's own intent.
public:
	BYTE GetBodyStance() const;
	class UClass* GetDesiredPrimaryItem() const;
	class UClass* GetDesiredSecondaryItem() const;
	void SetBodyIntention( BYTE _Priority, BYTE _BodyStance, class UClass* _pPrimaryItemClass, class UClass* _pSecondaryItemClass );
	UBOOL ShouldAIBeNotifiedOfRelationshipChange() const;
	void AcknowledgeRelationshipChangeHandledByAI();
	class UDishonoredAIBrain* GetAIBrain();
	UBOOL IsAFighter() const;

	// ---- agent CG: the damage entry point ----
public:
	virtual void TakeDamage_Native( INT& _rDamage, class AController* _pInstigatedBy, FVector _HitLocation, FVector& _rMomentum, class UClass* _pDamageType, struct FTraceHitInfo _HitInfo, class AActor* _pDamageCauser );
	// ---- agent CG natives sweep, round 2 ----
	virtual void OnSpawnStealable( class UDisSeqAct_SpawnStealable* _pAction );

// ---- agent DI (PHASE10 DI): the modular character - body mesh plus head mesh plus accessories ----
// PostBeginPlay 2013 rva 0x76a610 (2012 0x7c8770), PostBeginPlay_Body 0x77f8a0 (0x7bc180),
// CreateAccessoryStaticMeshComponent 0x7706b0 (0x7ae5f0), GetAccessoryStaticMeshComponent 0x76e0f0 (0x7abc40).
// Bodies in dishonorednpcpawn.cpp and dishonorednpcpawn_body.cpp.
public:
	virtual void PostBeginPlay();
	virtual void PostBeginPlay_Body();
	/** Retail has this private; it is public here because the census in the same unit reports what it built. */
	class UStaticMeshComponent* CreateAccessoryStaticMeshComponent( BYTE _eAccessoryType );
	class UStaticMeshComponent* GetAccessoryStaticMeshComponent( BYTE _eAccessoryType ) const;

// ---- agent DN (PHASE10 DN): locomotion ----
// Spawned 2013 rva 0x752270 (2012 0x7b0a60) creates the component from the NPC tweaks' UArkComponentLocomotionConfig and
// starts it; physWalking 0x74e8a0 (0x7ad240) hands the frame to it instead of to APawn's walking physics;
// SetupPathfindingParams 0x74ad20 (0x7ab3a0) and SetupPathGoalsAndConstraints 0x75e490 (0x7c2040) are the pawn's half of
// the nav-mesh contract. Bodies in Src/dishonorednpcpawn_locomotion.cpp.
public:
	virtual void Spawned();
	virtual void physWalking( FLOAT DeltaTime, INT Iterations );
	class FArkComponentLocomotion* GetComponentLocomotion() const;
	virtual UBOOL SetupPathGoalsAndConstraints( const FVector& _rFinalDestination, UBOOL _bForReachability );
	UBOOL IsMoving() const;
private:
	virtual void SetupPathfindingParams( struct FNavMeshPathParams& _rOut_ParamCache );
	void UpdateLocomotion();

// ---- agent EC (PHASE11 EC): the save body the object stream stops on ----
// GameLoad 2013 rva 0x76d3f0 (2012 0x7c7c40); IsSaveable 0x74aa80 (2012 0x7ab370, byte-identical between the
// builds); GameLoad_Possession 0x771340. GameLoad_Dialog is this class's half of retail's
// IDisConvSpeakerInterface::GameLoad_Dialog (0x897060), which this tree's method-less interface cannot carry.
// Bodies in dissavegame.cpp, with the other ported override bodies.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const
	{
		return Location == SLL_FILE || bKillDuringLevelTransition;
	}
private:
	void GameLoad_Dialog( FArchive& _rArchive, ESaveLoadLocation _Location );
	void GameLoad_Possession( FArchive& _rArchive, ESaveLoadLocation _Location );

// ---- agent FT (PHASE17 FT): the look-at component ----
// Spawned (above) creates FArkComponentLookat after the locomotion component, hands it the NPC tweaks'
// m_pComponentLookatConfig and starts it; GetComponentLookat 2013 rva 0x74b3f0 (2012 0x7ab8e0) is the member.
// Body in Src/dishonorednpcpawn_locomotion.cpp beside Spawned.
public:
	class FArkComponentLookat* GetComponentLookat() const;
private:
	void SpawnedLocomotion( class UDisTweaks_NPCPawn* NPCTweaks );

// ---- agent FS (PHASE17 FS): the matinee (MAT_*) contract, the AI-group half ----
// 2012 rvas: MAT_BeginAIGroup 0x7c0570, MAT_FinishAIGroup 0x7c0830, MAT_SetForceInventory 0x7ab5c0,
// MAT_SetCollision 0x7ab7a0, MAT_SetMeshTranslationMode 0x7ab6e0, MAT_DisableMeshTranslation 0x7ab720,
// MAT_IsCollisionDisable 0x7ab8a0, MAT_PermitLODPause 0x7ab710. Bodies in dishonorednpcpawn.cpp; the bases
// are APawn's (EnginePawnClasses.h) and UInterpGroupInstAI calls them. The other fourteen NPC MAT_* are
// inventoried in resources/docs/agents/agentFS.md.
public:
	virtual void MAT_BeginAIGroup( class UInterpGroupInstAI* GroupInst );
	virtual void MAT_FinishAIGroup( class UInterpGroupInstAI* GroupInst );
	virtual void MAT_SetForceInventory( UBOOL bShouldForceEmptyHand );
	virtual void MAT_SetCollision( UBOOL bDisableCollisionForSoiree, UBOOL bHaveMoveTrack );
	virtual void MAT_SetMeshTranslationMode( BYTE MatMeshTranslationMode );
	virtual void MAT_DisableMeshTranslation( UBOOL bReset );
	UBOOL MAT_IsCollisionDisable() const;
	UBOOL MAT_PermitLODPause() const;
protected:
	// agent FS: 2012 rva 0x7bac70, the first thing MAT_BeginAIGroup does - the matinee slots re-collected
	virtual void UpdateNodeSlot();

// ---- agent HB (PHASE22 HB): the matinee pose blend and the controller list ----
// MAT_BlendOut 2012 0x7ad5e0 / 2013 0xb4ec80 (APawn's slot, base 0xc8750), MatineeControllerAdd 0x7b19a0 / 2013 0xb53500,
// MatineeControllerRemove 0x7baea0 / 2013 0xb5f760. Bodies in dishonorednpcpawn.cpp. PostInitAnimTree (2012 0x7ad200, 53 B:
// APawn's, then AddNewComponent<FDisComponentAnimPlayer> + Starting) is not overridden: FDisComponentAnimPlayer is unported.
public:
	virtual void MAT_BlendOut( class UInterpGroupInst* GroupInst );
	void MatineeControllerAdd( class USeqAct_Interp* const Matinee, BYTE SoireePriority, UBOOL bIsBlocking );
	void MatineeControllerRemove( class USeqAct_Interp* const Matinee, BYTE SoireePriority );

// ---- agent IC (PHASE23 IC): the soiree side of the controller list ----
// GetMatineeControllerCount 2012 0x7b1a60 / 2013 0xb53570, ExitFromMatinees( FGuid ) 0x7bdfd0 / 0xb65940, ExitFromMatinees( priority )
// 0x7bde30 / 0xb657a0, StopMatinees 0x7b6b40 / 0xb5a120, MAT_ShouldForceInventory 0x7ad720, MAT_GetBodyIntention 0x7ad740.
// DISHONORED(bringup): retail's ExitFromMatinees pair and the two MAT_ queries are virtual (NPC vtable +1236/+1240, +1228/+1232 in
// 2012); nothing overrides them, so they are plain members here.
public:
	INT GetMatineeControllerCount( BYTE SoireePriority, UBOOL bIsBlocking ) const;
	void ExitFromMatinees( FGuid MatineeGuid, const FString* Reason );
	void ExitFromMatinees( BYTE SoireePriority, const FString* Reason );
	void StopMatinees( const TArray<class USeqAct_Interp*>& MatineesToStop, const FString* Reason );
	UBOOL MAT_ShouldForceInventory() const;
	void MAT_GetBodyIntention( BYTE& BodyStance, class UClass*& PrimaryItem, class UClass*& SecondaryItem ) const;

// ---- agent HA (PHASE22 HA): the NPC's root-motion pair ----
// 2012 rvas StartRootMotionMode 0x7ac100 (136 B), StopRootMotionMode 0x7af1a0 (47 B); 2013 rvas 0x76e680 / 0x771280 (VA
// 0xb6e680 / 0xb71280), byte-identical. Bodies in dishonorednpcpawn_locomotion.cpp (retail's unit).
public:
	virtual void StartRootMotionMode( BYTE _RootMotionMode, BYTE _RootRotationMode );
	virtual void StopRootMotionMode();

// ---- agent FR (PHASE17 FR): the NPC's speaker overrides, dishonorednpcpawn_conversation.cpp ----
// DISHONORED(port): 2012 rvas GetConversationComponent_Derived 0x7ac040, FireDialogHook 0x7ac070, SupportsDialogHook
// 0x7ac090, IsAvailableForDialog 0x7c17c0 (2013 0x774280, 188 B), StartDialog 0x7c1880 (300 B), EndDialog 0x7c19b0 (247 B).
public:
	virtual class UDisConversationComponent* GetConversationComponent_Derived() { return m_pConvComponent; }
	virtual void FireDialogHook( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator );
	virtual UBOOL SupportsDialogHook( eDisDialogHook _Hook ) const;
	virtual eDisDialogAvailability IsAvailableForDialog( eDisConvPriority _Priority, INT _StartDialogFlags ) const;
	virtual void StartDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, UBOOL _bBlocking, INT _StartDialogFlags, INT _FiringHookIndex );
	virtual void EndDialog( struct FDisDialogRunningInstance* _pDialogRunningInst, DisConvoEndReasonEnum _ConvoEndReason );

// ---- agent GA (PHASE18 GA): the NPC's native components ----
// PreBeginPlay 2012 rva 0x7ab250 (19 B: the base, then TG_PreAsyncWork), PreBeginPlay_NativeComponents 0x7b0c10 (247 B:
// the hit-reaction-throw, mesh-offset, LOD-manager and RB-damages components out of the NPC tweaks). Bodies in
// Src/dishonorednpcpawn_nativecomponents.cpp.
public:
	virtual void PreBeginPlay();
protected:
	virtual void PreBeginPlay_NativeComponents();

// ---- agent FV (PHASE18 FV): the NPC tick, the body pass that drives the look-at and the spine bender, the relationship change ----
// 2013 rvas (2012 in brackets): Tick 0x76da40 (0x7c8090, 1085 B), Tick_Body 0x783380 (0x7be180, 310 B), ChooseLookAt 0x7794d0
// (0x7b4d90, 475 B), GetCurrentTransitSpeed 0x779430 (0x7b4cf0; agent DN's comment in dishonorednpcpawn_locomotion.cpp cited
// 0x779850, which is not this function), Tick_SpineBending 0x77fbd0 (0x7bc480), Tick_SpineBending_Apply 0x77b3d0 (0x7b7120,
// 2006 B), IsActionSpineBendingEnabled 0x7772c0 (0x7b2b40), IsSlopeOffsettingEnabled 0x77b300 (0x7b7050), IsFootPlacementEnabled
// (0x7b2a70), OnRelationshipChange 0x759de0 (0x7c04e0). Bodies in dishonorednpcpawn.cpp, dishonorednpcpawn_body.cpp and
// dishonorednpcpawn_locomotion.cpp.
public:
	virtual UBOOL Tick( FLOAT _fDeltaTime, ELevelTick _TickType );
	virtual void Tick_Body( FLOAT _fDeltaTime, ELevelTick _TickType );
	virtual BYTE ChooseLookAt();
	BYTE GetCurrentTransitSpeed() const;
	virtual void OnRelationshipChange();
protected:
	void Tick_SpineBending( FLOAT _fDeltaTime );
	void Tick_SpineBending_Apply( FLOAT _fDeltaTime );
	UBOOL IsActionSpineBendingEnabled() const;
	UBOOL IsSlopeOffsettingEnabled() const;
	UBOOL IsFootPlacementEnabled() const;

	// ---- agent GD (PHASE19 GD): what the player's Use reaches on an NPC. Bodies in dishonorednpcpawn_interact.cpp ----
	// GetCrosshairStatus 2013 rva 0x759c40 (2012 0x7c02b0), GetCrosshairStatus_Helper 0x759a50 (0x7c00d0),
	// AttemptInteract_Derived 0x7678c0 (0x7c93b0): the IDisInteractableInterface slots +12 and +96 of the NPC's vtable (2013 0x1141458).
public:
	virtual enum eCrossHairStatus GetCrosshairStatus( class ADishonoredPawn* _pPawn ) const;
	virtual UBOOL AttemptInteract_Derived( class ADishonoredPawn* _pPawn, UBOOL& _rOut_bCanBeWitnessed );
private:
	enum eCrossHairStatus GetCrosshairStatus_Helper( class ADishonoredPawn* _pPawn ) const;
public:
	// DISHONORED(port): agent GB. 2012 0x7ab5e0 / 2013 0x74b080 and 0x7ab690: the matinee Attachment track's attach / detach on an
	// NPC (dishonorednpcpawn.cpp).
	virtual void MAT_Attach( AActor* const InBase, const FAttachmentInfos& AttachInfos );
	virtual void MAT_Detach();

// ---- agent GI (PHASE20 GI): the NPC half of the attention target ----
// GetAttnTargetBrain 2012 0x7c0080 / 2013 0x759a00 (the controller's initialized brain, none when dead), IsAttnTargetBeingMurdered
// 0x7ae590 / 0x770650 (the pawn choking this one, as an attention target - the choke system is not ported, so FALSE, named),
// AreReactionsDisabled 0x7ab9c0. Bodies in Src/dishonoredpawn_attentiontarget.cpp.
public:
	virtual const class UDishonoredAIBrain* GetAttnTargetBrain() const;
	virtual UBOOL IsAttnTargetBeingMurdered( class IDisAttentionTargetInterface** _ppMurderer ) const;
	UBOOL AreReactionsDisabled() const;

// ---- agent HC (PHASE22 HC): the rotation intents. Bodies in Src/dishonorednpcpawn_rotation.cpp ----
// 2012 rvas: physicsRotation 0x7c95f0, Tick_Rotation 0x7d4d40, Init_Rotation 0x7c9570, CancelAllNPCRotations 0x7c95a0,
// SetNPCRotation 0x7ce4a0, SetNPCRotationFocus 0x7cbf90, LockNPCRotation 0x7cc020, UnlockNPCRotation 0x7ce540,
// OnUnlockNPCRotation 0x7cc0f0. One FDisNPCRotationIntent per EDisFaceToPriority; the highest active one is
// m_CurrentNPCRotationPriority and drives the FArkComponentFaceTo request; physicsRotation only publishes the result.
public:
	virtual void physicsRotation( FLOAT deltaTime, FVector OldVelocity );
	void Tick_Rotation( FLOAT _fDeltaTime );
	void Init_Rotation();
	void CancelAllNPCRotations();
	// agent IC: 2012 rva 0x7ab8d0 / 2013 0x7ab560, m_pCpntFaceTo (set by Spawned from the locomotion component's face-to)
	class FArkComponentFaceTo* GetComponentFaceTo() const;
	void SetNPCRotation( EDisFaceToPriority _RotationPriority, const FRotator& _rRotation, FLOAT _fYawRateDegrees, FLOAT _fYawAccelerationTime, void* const _pAsker, FName _AskerName );
	void SetNPCRotationFocus( EDisFaceToPriority _RotationPriority, const FVector& _rTarget, FLOAT _fYawRateDegrees, FLOAT _fYawAccelerationTime, void* const _pAsker, FName _AskerName );
	void LockNPCRotation( EDisFaceToPriority _RotationPriority, void* const _pAsker, FName _AskerName );
	void UnlockNPCRotation( EDisFaceToPriority _RotationPriority );
private:
	void OnUnlockNPCRotation( EDisFaceToPriority _UnlockedPriority );
public:

// ---- agent IH4b (PHASE23 IH4): the NPC's part of ADishonoredPawn::ShutPawnDown ----
protected:
	// DISHONORED(port): 2012 0x7c1d20 (271 B), 2013 0x76a880 (549 B). Body in dishonorednpcpawn.cpp.
	virtual void ShutPawnDown_Derived();
public:
