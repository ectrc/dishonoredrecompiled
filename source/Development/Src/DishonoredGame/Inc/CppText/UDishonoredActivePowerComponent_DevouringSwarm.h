// UDishonoredActivePowerComponent_DevouringSwarm cpptext: included inside the generated class body.
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x7e75e0 (GameSave) and 0x7fe3c0 (GameLoad). The retail
// GameLoad address is NOT in match_2012_2013.csv (2012's 0x85dd90 is unmatched); it comes from retail's own
// vtable, slot 70 (build/agentEF/dump_vt2013.py). Bodies in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
