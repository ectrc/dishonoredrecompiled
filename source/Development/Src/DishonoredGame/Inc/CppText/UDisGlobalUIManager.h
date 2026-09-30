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
	// DISHONORED(port, partial): 2013 rva 0x847070 (2012 0x8b75e0, where the PDB names it) is the whole of it - the blur, the
	// black stripes, the HUD, the controller input mask and the mouse cursor. Only the blur half is ported;
	// the others are named at the site. Body in disglobaluimanager.cpp.
	void RefreshGlobalUIState();

// ---- agent EQ (PHASE13 EQ): the IArkSettingsListenerInterface override the settings republish calls ----
public:
	// DISHONORED(port): 2013 rva 0x841250 (new in 2013; the 2012 body at 0x8aef30 is 35 bytes against 152).
	// Body in disglobaluimanager.cpp.
	virtual void ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason );
