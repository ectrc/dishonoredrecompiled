// UDishonoredObjective cpptext: included inside the generated class body.
public:
	// DISHONORED(port): agent ED (PHASE11 ED), 2013 rvas 0x6d0b50 / 0x6d4cd0 / 0x6d4d60. Body in
	// dissavegame.cpp.
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const;
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
