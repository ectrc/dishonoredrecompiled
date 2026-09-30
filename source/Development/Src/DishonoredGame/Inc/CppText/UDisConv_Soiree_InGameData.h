// UDisConv_Soiree_InGameData cpptext: included inside the generated class body
// (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent ER (PHASE14 ER), 2013 rvas 0x8a94e0 / 0x8a9540 (2012 0x8f9cb0 / 0x8f9d10) - the
// in-game state of a soiree node: its own script properties (the UDisAttentionInfo_Base ICF fold that
// UDisConv_Node_InGameData already carries), then which running instance of which dialog-tree binding the
// matinee this node drives was playing. Bodies in dissavegame.cpp.
public:
	virtual void GameSave( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
