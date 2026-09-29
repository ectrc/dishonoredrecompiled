// UDisSeqAct_AutoSave cpptext: included inside the generated class body (DishonoredGameKismetClasses.h).
public:
	// DISHONORED(written): 2013 rva 0x785f20 (2012 0x7cb450, same bytes): m_AutoSaveStatus @272
	void SetAutoSaveDenied() { m_AutoSaveStatus = DASAS_Denied; }

	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x785f30 (2012 0x7cb460). Reads no stream bytes:
	// an autosave action that was mid-save when the slot was written is resolved once the level is back.
	virtual void PostGameLoad( ESaveLoadLocation Location );
