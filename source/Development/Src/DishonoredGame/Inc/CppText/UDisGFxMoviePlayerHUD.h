// UDisGFxMoviePlayerHUD cpptext: included inside the generated class body.

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x7ac2e0. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EK (PHASE11 EK): the in-game HUD ----
// Retail's own non-virtual members. The virtual ones (PostStart 2013 vtable +468, PreAdvance +476,
// PreClose +472, ApplyGameSettings) cannot be declared here without regenerating
// DishonoredGameUIClasses.h, so they arrive through the GFxUI seam as free functions, exactly as
// agent DG's three base-class overrides do (GFxUI/Inc/gfxuiengine.h). Bodies in
// disgfxmovieplayerhud.cpp.
public:
	void StartHUD();                                                    // 2013 rva 0x78cd00
	void CloseHUD();                                                    // 2013 rva 0x787e30
	UBOOL ShouldAlwaysShowPlayerInfo() const;                           // 2013 rva 0x78cd40
	void PreRender( class UCanvas* _pCanvas );                          // 2013 rva 0x7a5840
	void HideGauge( BYTE _Gauge, UBOOL _bSuccess );                     // 2013 rva 0x794c10
	void UpdateGauge( BYTE _Gauge, FLOAT _fValue );                     // 2013 rva 0x794d40

	// The two seam bodies reach the protected tick halves retail's own PreAdvance reaches.
	friend void DisGFxMoviePlayerHUDPostStart( class UDisGFxMoviePlayerHUD* HUD );
	friend void DisGFxMoviePlayerHUDPreAdvance( class UDisGFxMoviePlayerHUD* HUD, FLOAT DeltaTime );

	// DISHONORED(port): agent HE, 2013 rva 0x7af820 (2012 0x811e40), the vtable slot after PostStart (2013 vtable
	// 0x115cf68, 2012 0xd53334). Reached through the GFxUI PreClose seam like UDisGFxMoviePlayerGlobal's.
	void PreClose( UBOOL _bUnload );
	// DISHONORED(bringup): agent HE, -phuddiag
	static UBOOL IsDiagEnabled();

protected:
	void PreRender_Layout( class UCanvas* _pCanvas );                   // 2013 rva 0x796250
	void Tick_PlayerStatus( INT _ShowFlags );                           // 2013 rva 0x79f6e0
	void Tick_PlayerState( INT _ShowFlags );                            // 2013 rva 0x795ed0
	void Tick_Cinematic();                                              // 2013 rva 0x7960e0
	void UpdateHealthGauge( const struct FDisPlayerStatus_Health& _rHealth );        // 2013 rva 0x797d40
	void UpdateManaGauge( const struct FDisPlayerStatus_Mana& _rMana );              // 2013 rva 0x797f80
	void UpdateEquipmentInfo( const struct FDisPlayerStatus_Equipment& _rEquipment ); // 2013 rva 0x7981f0

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x7af370 (2012 0x811620). Body in disgfxmovieplayerhud.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );

// ---- agent HF (PHASE22 HF): the location banner, the objective popup and the tutorial line ----
// Bodies in disgfxmovieplayerhud.cpp; the 2013 rvas are the bodies ported, the 2012 PDB names the functions.
public:
	void ShowLocationDiscovery( const FString& _rLocationName, UBOOL _bPlaySound );         // 2013 rva 0x795160
	void OnObjectiveAdded( class UDishonoredObjective* _pObjective );                        // 2013 rva 0x7b2480
	void OnTaskAdded( class UDishonoredTask_Base* _pTask );                                  // 2013 rva 0x7b27c0
	INT ShowTutorial( UObject* _pLocObject, const TCHAR* _pLocProperty, FLOAT _fDuration,
		UBOOL _bShowOnce, UBOOL _bAllowCycling, UBOOL _bFromKismet, UBOOL _bForceDisplay ); // 2013 rva 0x7ac5a0
	struct FDisTutorialInfo* FindTutorialInfo( INT _ID );                                    // 2013 rva 0x79eef0
	INT FindTutorial( INT _ID ) const;                                                       // 2013 rva 0x7988e0
	void RemoveTutorialAt( INT _Index );                                                     // 2013 rva 0x7a7390
	void OnCrosshairActorSet( class IDisInteractableInterface* _pInteractable );            // 2013 rva 0x7af540

protected:
	void Tick_GameMessages( INT _ShowFlags );                                                // 2013 rva 0x78d3b0
	void Tick_Tutorials( FLOAT _fDeltaTime, INT _ShowFlags );                                // 2013 rva 0x7afa60
	void Tick_ObjectivePopup( FLOAT _fDeltaTime );                                           // 2013 rva 0x7afe70
	void ShowObjectivePopup( const struct FDisObjectivePopupInfo& _rPopup );                 // 2013 rva 0x798a40
	INT InsertTutorial( const struct FDisTutorialInfo& _rTutorialInfo );                     // 2013 rva 0x7a6c10
	void SetTutorialMessage( const struct FDisTutorialInfo& _rTutorialInfo );                // 2013 rva 0x7ad060
	void ClearTutorialMessage();                                                             // 2013 rva 0x798960
	void ShowTutorialWindow( const struct FDisTutorialInfo& _rTutorialInfo );                // 2013 rva 0x7ad2f0
	void HideTutorialWindow();                                                               // 2013 rva 0x78d6d0
	void ResolveMarkupString( FString& _rText ) const;                                       // 2013 rva 0x7acdc0
	void Tick_Interaction( class IDisInteractableInterface* _pInteractable );               // 2013 rva 0x7acb10
	void UpdateCrosshairFocusText( const FString& _rText );                                  // 2013 rva 0x7984f0
	void UpdateCrosshairInteractions( const FString& _rName, const struct FDisUIInteractionGroup& _rGroup ); // 2013 rva 0x7a6150
	// agent JH (PHASE24 JH): the other three groups of the interaction context
	void UpdateContextInteractions( const struct FDisUIInteractionGroup& _rGroup );                  // 2013 rva 0x7a63c0 (2012 0x809a40)
	void UpdateSpecialInteractions( const struct FDisUIInteractionGroup& _rGroup );                  // 2013 rva 0x7a6630 (2012 0x809c10)
	void UpdateQTEInteractions( const struct FDisUIInteractionGroup& _rGroup );                      // 2013 rva 0x7a6920 (new in 2013)

// ---- agent HI (PHASE22 HI): the rest of the objective notifications. Bodies in disgfxmovieplayerhud.cpp ----
public:
	void HandleObjectiveEvents( UBOOL _bHandle );                                            // 2013 rva 0x787ec0
	void OnObjectiveCompleted( class UDishonoredObjective* _pObjective );                    // 2013 rva 0x7b2500
	void OnObjectiveFailed( class UDishonoredObjective* _pObjective );                       // 2013 rva 0x7b2660
	void OnTaskUpdated( class UDishonoredTask_Base* _pTask );                                // 2013 rva 0x7b28e0
	void OnTaskCompleted( class UDishonoredTask_Base* _pTask );                              // 2013 rva 0x7b2a00
	void OnTaskFailed( class UDishonoredTask_Base* _pTask );                                 // 2013 rva 0x7b2b20

// ---- agent HI (PHASE22 HI): the task markers. Bodies in disgfxmovieplayerhud.cpp ----
protected:
	void Tick_Markers( FLOAT _fDeltaTime );                                                  // 2013 rva 0x7b00c0
	void UpdateTaskMarkerInstances();                                                        // 2013 rva 0x7ad550
	void PreRender_Markers( UCanvas* _pCanvas );                                             // 2013 rva 0x7a0890
	void SortMarkers( struct FDisHUDMarkerPosInfo* _pPosInfos, UBOOL _bSortNewMarkers );     // 2013 rva 0x799480
	void ClearMarkerInstances();                                                             // 2013 rva 0x7a0f00
public:

// ---- agent ID (PHASE23 ID): the subtitle line. Bodies in disgfxmovieplayerhud.cpp; 2013 rvas (2012 in the bodies) ----
public:
	void OnMainSubtitleChanged( const TCHAR* _pText, TArray<FLOAT>* _pTextPartsRatio );                // 2013 rva 0x7af5d0
	void OnSecondarySubtitleChanged( const TCHAR* _pText, TArray<FLOAT>* _pTextPartsRatio );           // 2013 rva 0x7af6e0
	void ShowNextMainSubtitlePart( INT _Part );                                                         // 2013 rva 0x79f210
	void ShowNextSecondarySubtitlePart( INT _Part );                                                    // 2013 rva 0x79f2d0
protected:
	void Tick_Subtitles( INT _ShowFlags );                                                              // 2013 rva 0x79fe20
	void SplitSubtitleText( const TCHAR* _pText, TArray<FString>& _rPartsOut, TArray<FLOAT>& _rRatiosOut ); // 2013 rva 0x7adf80
	void ShowSubtitleText( const TCHAR* _pText, INT _SubtitleType );                                    // 2013 rva 0x799690
