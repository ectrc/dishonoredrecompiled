// UDisConv_Speaker cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). A conversation's speaker slot. BindSpeaker 2012 rva 0x904d10 resolves the
// slot to a live IDisConvSpeakerInterface through the derived BindSpeaker_Derived, then CheckLOSAndRadius 0x9012a0 and
// the awareness filter. GetSpeakerTree is the tree a FDisConvSpeaker_InGame must be able to speak
// (IDisConvSpeakerInterface::SupportsDialogTree). Bodies in disconv_speakers.cpp.
public:
	class IDisConvSpeakerInterface* BindSpeaker( struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual const class UDisDialogTree* GetSpeakerTree() const { return NULL; }
	// DISHONORED(port): agent IE. GetSpeakerName 2013 0x8ad320 (2012 0x8fc460), vtable slot 86 of every speaker class.
	virtual void GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const;
protected:
	virtual class IDisConvSpeakerInterface* BindSpeaker_Derived( struct FDisDialogRunningInstance* _pRunningInstance ) const { return NULL; }
	UBOOL CheckLOSAndRadius( class IDisConvSpeakerInterface* _pMe, class IDisConvSpeakerInterface* _pBoundSpeaker ) const;
