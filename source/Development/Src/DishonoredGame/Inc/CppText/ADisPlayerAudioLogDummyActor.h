// ADisPlayerAudioLogDummyActor cpptext: included inside the generated class body (DishonoredGameClasses.h).
// DISHONORED(port): agent EF (PHASE11 EF), 2013 rvas 0x644220 (GameLoad) and 0x644280 (PostGameLoad). GameSave
// (0x6441d0) is retail's own body on the base ADisDialogInanimateDummy and is not ported, for the reason agent
// ED gave: the writing half of the object layer does not exist here. Bodies in dissavegame.cpp.
public:
	virtual void GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location );
	virtual void PostGameLoad( ESaveLoadLocation _Location );
	/** retail's IsSaveable here is the real body it shares with UDisNPCTravelManager (2013 rva 0x6bfcc0) */
	virtual UBOOL IsSaveable( ESaveLoadLocation Location ) const { return Location == SLL_FILE; }

// ---- agent FR (PHASE17 FR): the player's speaker proxy, displayeraudiologdummyactor.cpp ----
// DISHONORED(port): the actor the player pawn speaks through. GetSpeakerLocInfo 2012 rva 0x69fdb0, AddDialogOneShot 0x699670,
// RemoveDialogOneShot 0x691f60, GetSpeakerFromOneShotOwner (ADisDialogInanimateDummy's 0x691c00: the actor's own speaker
// interface), GetConversationComponent_Derived 0x568210 (m_pConvComponent). FireDialogHook 0x691fb0, StartDialog 0x692010,
// EndDialog 0x6920f0, PlayDialogBlurb 0x692090, Tick 0x6a8ee0 (the audio-log / heart-gadget side) are inventoried in agentFR.md.
public:
	virtual UObject* GetUObjectInterfaceDisConvSpeakerInterface() { return this; }
	virtual UObject* GetUObjectInterfaceDisConvOneShotOwnerInterface() { return this; }
	virtual class UDisConversationComponent* GetConversationComponent_Derived() { return m_pConvComponent; }
	virtual void GetSpeakerLocInfo( TArray<struct FDisConvSpeakerLocInfo>& _rLocInfoOut ) const;
	virtual void AddDialogOneShot( class UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner );
	virtual void RemoveDialogOneShot( class UDisDialogVoiceData* _pOneShot );
	virtual class IDisConvSpeakerInterface* GetSpeakerFromOneShotOwner( class IDisConvSpeakerInterface* _pTreeOwner ) { return const_cast<ADisPlayerAudioLogDummyActor*>( this ); }
