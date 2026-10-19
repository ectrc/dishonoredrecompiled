// UDisDialogVoiceData cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). InitForGame 2013 rva 0x88b2e0 (clears m_bMapsBuilt), BindToOneShot 0x8a4b00
// (the one-shot's tree becomes this voice data's tree; the StartEditing it then runs is editor-only and unported).
// Bodies in disdialogvoicedata.cpp.
public:
	void InitForGame();
	void BindToOneShot( class ADisDialogOneShot* _pOneShot );

// DISHONORED(port): agent ID (PHASE23 ID). PostLoad 2013 rva 0x892450* (vtable 0x1199cf8 slot 6; 2012 0x8fd550),
// RebuildLookupMaps 0x8a1440 (0x90e1a0), FindTextBlurb 0x8a1550 (0x90e2a0). Bodies in disdialogvoicedata.cpp.
	virtual void PostLoad();
	struct FDisVoiceTextBlurb* FindTextBlurb( const class UDisDialogTree* _pTree, const FGuid& _rBlurbGUID );
protected:
	void RebuildLookupMaps();
