// UDisConv_MeSpeaker cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas GetSpeakerTree 0x8a7ba0, BindSpeaker_Derived 2012 0x8f9d90: the
// tree's own bound speaker. Bodies in disconv_speakers.cpp.
public:
	virtual const class UDisDialogTree* GetSpeakerTree() const;
	// DISHONORED(port): agent IE. GetSpeakerName 2013 0x8b1cc0 (2012 0x8ff210), vtable slot 86.
	virtual void GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const;
protected:
	virtual class IDisConvSpeakerInterface* BindSpeaker_Derived( struct FDisDialogRunningInstance* _pRunningInstance ) const;
