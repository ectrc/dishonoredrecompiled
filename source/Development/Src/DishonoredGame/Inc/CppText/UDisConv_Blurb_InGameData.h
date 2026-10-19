// UDisConv_Blurb_InGameData cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent ID (PHASE23 ID). The four per-speaker slots of a blurb that several speakers can say at once.
// GetBlurbSpeakerData 2013 rva 0x8946c0 (2012 0x8e0cc0, 148 B), StopBlurbSpeakerData 0x88c550 (0x8dd3f0, 64 B). Bodies in disconv_blurb.cpp.
public:
	struct FDisBlurbPerSpeakerInstance& GetBlurbSpeakerData( class IDisConvSpeakerInterface* _pSpeaker );
	void StopBlurbSpeakerData( class IDisConvSpeakerInterface* _pSpeaker );
