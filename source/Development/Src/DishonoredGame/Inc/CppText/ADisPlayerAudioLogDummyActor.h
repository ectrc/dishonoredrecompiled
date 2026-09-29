// ADisPlayerAudioLogDummyActor cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x644220 (GameLoad) and 0x644280 (PostGameLoad). GameSave
// (0x6441d0) is retail's own body on the base ADisDialogInanimateDummy and is not ported, for the reason agent
// ED gave: the writing half of the object layer does not exist here. Bodies in dissavegame.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	/** retail's IsSaveable here is the real body it shares with UDisNPCTravelManager (2013 rva 0x6bfcc0) */
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return Location == SLL_FILE; }
