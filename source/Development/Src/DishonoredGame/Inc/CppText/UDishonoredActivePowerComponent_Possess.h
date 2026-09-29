// UDishonoredActivePowerComponent_Possess cpptext: included inside the generated class body.
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x7ed930 (GameSave), 0x7edb00 (GameLoad) and 0x7fb8e0
// (PostGameLoad). Bodies in dissavegame.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
