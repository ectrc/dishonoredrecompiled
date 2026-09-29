// UDishonoredMapInfo cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x611780 / 0x60ba70. Body in dissavegame.cpp
	// (retail has it in dishonoredmapinfo.cpp). The per-squad live counts and the objective task targets.
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );

	// DISHONORED(port): agent ED (PHASE11 ED). Retail declares IsSaveable inline here as `return TRUE`; all
	// 59 such bodies are ICF-folded onto UObject::IsRefSaveable's (2012 rva 0x66a860), which is why the PDB
	// names only the 11 overrides that have a body of their own. vtables.csv slot 67 is the record.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
