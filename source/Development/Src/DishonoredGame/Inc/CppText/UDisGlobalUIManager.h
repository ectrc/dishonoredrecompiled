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
