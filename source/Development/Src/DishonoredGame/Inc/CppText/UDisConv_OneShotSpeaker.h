// UDisConv_OneShotSpeaker cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). 2012 rvas GetSpeakerTree 0x8f8ba0, BindSpeaker_Derived 0x8fc290: the
// m_iOneShotSpeaker-th "other speaker" of the ADisDialogOneShot the tree is bound to, resolved through that speaker's
// IDisConvOneShotOwnerInterface (a spawner hands out one of its live pawns). Bodies in disconv_speakers.cpp.
public:
	virtual const class UDisDialogTree* GetSpeakerTree() const;
	// DISHONORED(port): agent IE. GetSpeakerName 2013 0x8b6380 (2012 0x8fc0e0), vtable slot 86.
	virtual void GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const;
protected:
	virtual class IDisConvSpeakerInterface* BindSpeaker_Derived( struct FDisDialogRunningInstance* _pRunningInstance ) const;
