// ADishonoredPlayerController cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(written): IsMoveInputIgnored/IsLookInputIgnored are APlayerController virtuals in retail (2013 vtable +1172/+1176,
// native execs 0x1d3a70/0x1d3ab0); our APlayerController has them as script events, so they are introduced here.
public:
	// DISHONORED(written): ?s_pInstance@ADishonoredPlayerController@@0PAV1@A, set by PostBeginPlay, cleared by BeginDestroy
	// (2013: 205 references)
	static ADishonoredPlayerController* s_pInstance;

	// DISHONORED(written): retail vtable +956 Possess (2013 rva 0x6a0ae0, 2012 0x6d1320: Super only), +960 UnPossess (0x6a0af0,
	// a thunk to APlayerController::UnPossess), +1156 HandleWalking (0x6a0770)
	virtual void Possess( APawn* inPawn );
	virtual void UnPossess();
	virtual void HandleWalking( FLOAT DeltaTime );

	virtual void PostBeginPlay();
	virtual void BeginDestroy();
	virtual UBOOL IsMoveInputIgnored() const;
	virtual UBOOL IsLookInputIgnored() const;
	virtual void ReceivedPlayer_Native();
	virtual UArkProfileSettings* GetProfileSettings();
	virtual void OnControllerChanged_Native( UBOOL bIsConnected );

	UBOOL IsInputEnabled( INT InputMask ) const;

	

	

	

	

	

	// DISHONORED(written): agent AU. HandleHeldButtons 2013 rva 0x6ba9a0 (2012 0x6f9b80), _Context 0x6ba900 (0x6f9ae0),
	// _Context_Interactables 0x6a2e70 (0x6d6e10), Dis_Zoom 0x6a2e40 (0x6d6de0). Bodies in dishonoredplayercontroller.cpp.
	virtual void HandleHeldButtons( FLOAT DeltaSeconds );
	virtual void Dis_Zoom();
	void HandleHeldButtons_Context( FLOAT DeltaSeconds );
	void HandleHeldButtons_Context_Interactables( FLOAT DeltaSeconds );

	/**
	 * DISHONORED(port): agent DO, the ACTOR feed of ULocalPlayer::UpdatePostProcessSettings and the channel that
	 * carries the powers (agent DE hand-over 1).
	 *   ModifyPostProcessSettings            2013 rva 0x6adeb0 (2012 0x6eb900), 1,498 bytes
	 *   Tick                                 0x6b6e80 (2012 0x6ef510) - and this, not ModifyPostProcessSettings, is
	 *                                        where retail ticks the post-process node controllers
	 *   ApplyWaterPostProcessSettings        0x6a6750 (2012 0x6e2550)
	 *   ApplyDarkVisionPostProcessSettings   0x6a68d0 (2012 0x6d6d40)
	 *   ApplyPossessionPostProcessSettings   0x6a6c30 (2012 0x6e2880)
	 */
	virtual void ModifyPostProcessSettings( struct FArkPpConfig& Config );
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	void ApplyWaterPostProcessSettings( FLOAT DeltaSeconds );
	void ApplyDarkVisionPostProcessSettings( class ADishonoredPlayerPawn* PlayerPawn, FLOAT DeltaSeconds );
	void ApplyPossessionPostProcessSettings( FLOAT DeltaSeconds );

	/**
	 * DISHONORED(port): agent DO follow-up. 2013 rva 0x6af250 (2012 0x6f8690), vtable +1380 - the sprint key's
	 * toggle/hold split. The exec (2013 rva 0x5ee690) is the native the regression caught firing unbound.
	 * NOTE the rvas: the stub's comment block and the note at dishonoredplayercontroller.cpp:105 carry the 2012
	 * addresses (0x634cb0 / 0x6f8690); retail 2013's are 0x5ee690 and 0x6af250.
	 */
	virtual void DisToggleSprint( UBOOL bFromGamePad );

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x6a31e0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): retail's body is empty in both builds - the 2012 interface vtable slot and the 2013 one
	// (0xd18730 +4) both point at an ICF-folded three-byte `ret 8` (2013 rva 0x128ad0, which the PDB names
	// UGameViewportClient::SetOnlyUseControllerTiltInput).
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );

// ---- agent FI (PHASE14 FI): the cinematic switch behind the letterbox bars ----
public:
	/**
	 * DISHONORED(port): agent FI. SeqAct_ToggleCinematicMode raises PlayerController.SetCinematicMode, whose
	 * script body ends in these two natives; both were DISHONORED_NATIVE_STUBs, so the whole cinematic
	 * presentation - the bars, the hidden player, the save lock - did nothing.
	 *   PreSetCinematicMode_Native  2013 rva 0x6a33a0 (49 bytes, vtable +1408), exec 0x5ee960, 2 parameters
	 *   SetCinematicMode_Native     2013 rva 0x6af150 (252 bytes, vtable +1412), exec 0x5eea00, 9 parameters
	 * The parameter count and the names are retail_sdk_layout.json's
	 * (DishonoredGame.DishonoredPlayerController.SetCinematicMode_Native, params_size 36, nine CPF_Parm bits,
	 * none optional) and they match the nine Stack.Step calls of the exec one for one.
	 */
	// DISHONORED(port): agent FP, 2013 rva 0x6a0b00 (2012 0x6d1340, same bytes): sets or clears _Mask in
	// m_InputEnableMask[_Level]. UDisGlobalUIManager::RefreshGlobalUIState drives DisInputMaskLevel_BlockedUI with it.
	// Body in dishonoredplayercontroller.cpp.
	void EnableControllerInput( INT _Mask, UBOOL _bEnable, DisInputMaskLevel _Level );
	virtual void PreSetCinematicMode_Native( UBOOL bNewInCinematicMode, UBOOL bHidePlayer );
	virtual void SetCinematicMode_Native( UBOOL bInCinematicMode, UBOOL bWasInCinematicMode, UBOOL bHidePlayer,
		UBOOL bHideHUD, UBOOL bAffectsMovement, UBOOL bAffectsTurning, UBOOL bAffectsButtons,
		UBOOL bHideLetterbox, UBOOL bAllowProjectilesAndExplosions );

	// ---- agent GD (PHASE19 GD): the crosshair and the use path. Bodies in dishonoredplayercontroller_use.cpp ----
	// UpdateCrosshair 0x6b7b80 (0x6f7910), SetCrosshairActor 0x6a6280
	// (0x6e2090), GetPlayerCrosshairTarget 0x6a5ff0 (0x6e1e00), FindInteractableActor 0x6b70f0 (0x6f73f0),
	// CanInteractWithInteractable 0x6a2dc0 (0x6d6cc0).
public:
	void UpdateCrosshair( const struct FDishonoredViewTarget& _VT );
	void GetPlayerCrosshairTarget( struct FImpactInfo& _rImpactInfoOut, INT _ExcludeDisTraceMask, const FVector* _pOverrideDir ) const;
	class AActor* FindInteractableActor( const struct FDishonoredViewTarget& _VT );
private:
	UBOOL CanInteractWithInteractable( const class IDisInteractableInterface& _rInteractable, const struct FCanInteractParams& _rInteractParams, const FVector& _rStartLoc, const FVector& _rHitLoc ) const;
	void SetCrosshairActor( class AActor* _pActor, enum eCrossHairStatus _Status, UBOOL _bCanInteract );
public:
// ---- agent GF (PHASE19 GF): the camera pass ----
// DISHONORED(port): OnCameraUpdate 2012 0x6fa840 / 2013 0x6ba860 (104 / 155 B): the pawn's OnCameraUpdate, then on pass 0
// TickCameraTracking (0x6fa400; the 2013 body builds its target from the pawn's X/Y and the view's Z) and on pass 1 UpdateCrosshair
// (0x6f9d80) and UpdateBlindedEffect (0x6f87d0) - the three are skeletons here, named once. Body in dishonoredplayercontroller.cpp.
public:
	void OnCameraUpdate( const struct FDishonoredViewTarget& _VT, INT _PassCount );

// ---- agent GJ (PHASE20 GJ): the equip exec the key bindings reach ----
// DISHONORED(port): Dis_EquipItemByType 2012 0x6ec850 / 2013 0x6b4a70 (384 / 385 B, same: the GBA_Equip* bindings' command), its exec
// 0x634650 / 0x5ee010 (a class and a byte), CanEquip 0x6e3280 (90 B; 2013 inlined). EquipBestSuitableItem (0x6e3ac0, the other
// hand's best item when it is empty) and UDishonoredInventoryItem::NextItemOption (the item's own cycle) are not ported - named once.
// Bodies in dishonoredplayercontroller.cpp.
public:
	virtual void Dis_EquipItemByType( UClass* _pItemClass, BYTE _Usage );
	UBOOL CanEquip( BYTE _EquipUsage ) const;

// ---- agent HD (PHASE22 HD): the fixed-angle view tracking. Bodies in dishonoredplayercontroller_tracking.cpp ----
// StartTrackingFixedAngles 2012 0x6f8b90 / 2013 0x6af6f0, StopTrackingTarget 0x6d1690 / 0x6a0ec0, FetchTrackedTargetRot 0x6d16e0 /
// 0x6a0f10, TickCameraTracking 0x6f9ce0 / 0x6b4e60.
public:
	INT StartTrackingFixedAngles( INT _PitchFixed, INT _YawFixed, FLOAT _fBlendTime, INT _Flags, const struct FDisPlayerViewTracking_ExtraParams* _pParams );
	void StopTrackingTarget( INT _TrackID );
protected:
	INT FetchTrackedTargetRot( struct FDisPlayerViewTarget& _rTarget, UBOOL _bIsPitch, const FVector& _rCurCamPos );
	void TickCameraTracking( FLOAT _fDeltaTime, const FVector& _rCurCamPos );
public:
