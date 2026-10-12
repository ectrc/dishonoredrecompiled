// UDisConv_OneShotSpeaker cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). 2012 rvas GetSpeakerTree 0x8f8ba0, BindSpeaker_Derived 0x8fc290: the
// m_iOneShotSpeaker-th "other speaker" of the ADisDialogOneShot the tree is bound to, resolved through that speaker's
// IDisConvOneShotOwnerInterface (a spawner hands out one of its live pawns). Bodies in disconv_speakers.cpp.
public:
	virtual const class UDisDialogTree* GetSpeakerTree() const;
protected:
	virtual class IDisConvSpeakerInterface* BindSpeaker_Derived( struct FDisDialogRunningInstance* _pRunningInstance ) const;
