// UDishonoredVisibilityComponent cpptext: included inside the generated class body.
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rva 0x6c64d0 (2012 0x7270b0) - retail folds GameSave and GameLoad
// onto one body, which is nothing but SaveLoadCommon (0x6c1950, 2012 0x722070). Bodies in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	void SaveLoadCommon( FArchive& _rArchive );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
