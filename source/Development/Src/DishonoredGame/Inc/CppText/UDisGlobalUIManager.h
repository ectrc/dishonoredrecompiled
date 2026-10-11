// UDisGlobalUIManager cpptext: included inside the generated class body.

// ---- agent EB (PHASE11 EB): the object layer's GameLoad ----
public:
	// DISHONORED(port): agent EB, 2013 rva 0x856f70. Body in dissavegame.cpp.
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent EB. Retail declares IsSaveable inline here as `return TRUE`; all 59 such bodies
	// are ICF-folded onto UObject::IsRefSaveable's, which is why the PDB names only the 11 with a body of
	// their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }

// ---- agent EI (PHASE12 EI): the message box, whose queue and movie the manager owns ----
public:
	// DISHONORED(port): agent EI, 2013 rva 0x83dc00. Forwards to m_pGlobal->AddMessageBox and answers with the
	// new id. Bodies in disglobaluimanager.cpp.
	INT ShowMessageBox( const struct FDisMsgBoxInfo& _rInfo, UINT _Priority = 0 );
	// DISHONORED(port): agent EI, 2013 rva 0x83dc50
	void HideMessageBox( INT _ID );
	// DISHONORED(port): agent EI, 2013 rva 0x83dc30
	void AddMessageBoxTimer( INT _ID, FLOAT _fDuration );

// ---- agent FA (PHASE12 FA): the blur behind a modal ----
public:
	// DISHONORED(port): 2013 rva 0x84cf80. An open movie whose attributes changed re-reads the whole stack.
	void OnMovieAttributesChanged( class UDisGFxMoviePlayerBase* _pMovie );
	// DISHONORED(port): 2013 rva 0x847070 (2012 0x8b75e0, where the PDB names it): the blur, the black stripes, the
	// HUD's visibility and pause, the controller input mask and the mouse cursor, recomputed from the open movie
	// stack. Agent FA ported the blur arm, agent FP the rest. Body in disglobaluimanager.cpp.
	void RefreshGlobalUIState();

// ---- agent FP (PHASE16 FP): one owner for every movie player ----
public:
	// DISHONORED(port): 2013 rva 0x84cf70 (2012 0x8bc570), a jmp thunk onto RefreshGlobalUIState. Reached from
	// UDisGFxMoviePlayerBase::Start, Close and LoadMoviePackageAsync.
	void OnMovieStackChanged();
	// DISHONORED(port): 2013 rva 0x85fdb0 (2012 0x8c45d0), 1428 bytes. Constructs pHUD, pPowerWheel, pJournal, pNote,
	// pPauseMenu and pMissionStats (or pBrief and pResultsMenu under a challenge game info) from the current movie
	// set, then starts the global movie and the HUD. Called from ADishonoredGameInfo::InitGlobalManagers.
	void Init();
	// DISHONORED(port): 2013 rva 0x841120 (301 bytes; 2012 0x8aed60) and 0x83daa0 (29 bytes; 2012 0x8aedc0), reached
	// from ADishonoredGameInfo's two map-change virtuals. The map change closes every movie with
	// bCloseOnLevelChange; these close the HUD and the menu before it and reopen the global movie and the HUD after.
	void PreCommitMapChange();
	void PostCommitMapChange();
private:
	// DISHONORED(port): 2013 rva 0x851970 (2012 0x8c0020). 2013's: the default set, then every non-empty field of
	// the first DLC set whose m_MapFilter is a substring of the persistent map's name.
	void FindCurrentMovieSet( struct FDisUIMovieSet& _rMovieSet ) const;
	// DISHONORED(port): 2013 rva 0x846e70, the body Init calls six or seven times: the class by config name or the
	// default, the object under this manager, its tweaks out of the SF tweaks package, and its movie.
	class UDisGFxMoviePlayerBase* CreateMoviePlayer( UClass* _pDefaultClass, const TCHAR* _pName, const FString& _rClassName,
		const FString& _rMoviePath, UBOOL _bLoadMovieNow, UBOOL _bOverrideTweaks );
	// DISHONORED(port): 2013 rva 0x846d00: the seek-free package of SF tweaks the movie set names, into m_pSFTweaksPackage
	void LoadSFTweaksPackage( const FString& _rPackageName );
	// DISHONORED(port): 2013 rva 0x85dbb0: m_pItemSmallIconPackage and the DLC icon-package suffix
	void InitItemIconPackages( UBOOL _bHasDLCItemIconPackages, const FString& _rMapFilter );

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x841250 (new in 2013; the 2012 body at 0x8aef30 is 35 bytes against 152).
	// Body in disglobaluimanager.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
