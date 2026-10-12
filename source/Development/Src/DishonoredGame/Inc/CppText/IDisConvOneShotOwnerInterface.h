// IDisConvOneShotOwnerInterface cpptext: included inside the generated interface body (DishonoredGameClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The owner of one or more one-shot speakers - a spawner, the player pawn
// (through its audio-log dummy), an inanimate dummy. Retail vtable order (2012 ??_7ADishonoredSpawner@@6BIDisConvOneShotOwnerInterface@@@
// at 0xcf9c20): 1 GetUObjectInterface, 2 AddDialogOneShot, 3 RemoveDialogOneShot, 4 GetSpeakerFromOneShotOwner. All pure
// in retail; defaults here because three implementing classes are still comment-only units (agentFR.md inventory).
public:
	virtual UObject* GetUObjectInterfaceDisConvOneShotOwnerInterface() { return NULL; }
	virtual void AddDialogOneShot( class UDisDialogVoiceData* _pOneShot, UBOOL _bIsOwner ) {}
	virtual void RemoveDialogOneShot( class UDisDialogVoiceData* _pOneShot ) {}
	virtual class IDisConvSpeakerInterface* GetSpeakerFromOneShotOwner( class IDisConvSpeakerInterface* _pTreeOwner ) { return NULL; }
