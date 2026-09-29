// UDishonoredTask_Base cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rva 0x6cc250 (IsSaveable) and 2012 0x727060, which
	// retail uses for both GameSave and GameLoad. Body in dissavegame.cpp.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const;
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
