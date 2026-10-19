// UDisConv_InitiatorSpeaker cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas GetSpeakerTree 0x61a090 (m_pInitiatorDialogTree), BindSpeaker_Derived
// 2012 0x8f9db0: the speaker that fired the hook. Bodies in disconv_speakers.cpp.
public:
	virtual const class UDisDialogTree* GetSpeakerTree() const;
	// DISHONORED(port): agent IE. GetSpeakerName 2013 0x8b1e80 (2012 0x8ff3d0), vtable slot 86.
	virtual void GetSpeakerName( FString& _rNameOut, UBOOL _bUseDecorations ) const;
protected:
	virtual class IDisConvSpeakerInterface* BindSpeaker_Derived( struct FDisDialogRunningInstance* _pRunningInstance ) const;
