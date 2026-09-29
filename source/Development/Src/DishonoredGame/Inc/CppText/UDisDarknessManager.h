// UDisDarknessManager cpptext: included inside the generated class body (DishonoredGameDarknessClasses.h).
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rva 0x852ca0 - the chaos score and the deed counters. RETAIL
// folds GameSave and GameLoad onto this one body; the 2012 build has two (0x8c13a0 / 0x8c13d0), so vtables.csv
// reports a pair that retail does not have. Retail wins. Body in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
