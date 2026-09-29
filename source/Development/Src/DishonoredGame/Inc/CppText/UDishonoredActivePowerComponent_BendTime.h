// UDishonoredActivePowerComponent_BendTime cpptext: included inside the generated class body.
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x7e7150 (GameSave), 0x7f8980 (GameLoad) and 0x7e71d0
// (PostGameLoad). The bend-time clock is saved as an elapsed time relative to now, so PostGameLoad is what turns
// it back into an absolute m_fTimeAtStart. Bodies in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
