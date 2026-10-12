// UDisDialogVoiceData cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). InitForGame 2013 rva 0x88b2e0 (clears m_bMapsBuilt), BindToOneShot 0x8a4b00
// (the one-shot's tree becomes this voice data's tree; the StartEditing it then runs is editor-only and unported).
// Bodies in disdialogvoicedata.cpp.
public:
	void InitForGame();
	void BindToOneShot( class ADisDialogOneShot* _pOneShot );
