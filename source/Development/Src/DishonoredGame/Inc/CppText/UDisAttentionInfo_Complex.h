// UDisAttentionInfo_Complex cpptext: included inside the generated class body (dishonoredgameclasses.h).
// DISHONORED(port): agent EJ (PHASE12 EJ) - GameLoad, 2013 rva 0x74ff80 (2012 0x793830), retail vtable slot 70
// (vftable rva 0xd40618). GameSave (0x74ff00, slot 69) is not ported, for agent ED's reason: the writing half
// of the DisSaveLoad object layer does not exist here.
//
// The body is in Src/dissavegame.cpp rather than in retail's own Src/disattentioninfo_complex.cpp, because that
// unit is still an import_reference.py skeleton on DishonoredGame_EXCLUDE.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return TRUE; }
