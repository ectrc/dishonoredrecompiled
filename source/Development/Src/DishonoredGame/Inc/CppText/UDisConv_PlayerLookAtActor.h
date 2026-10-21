// UDisConv_PlayerLookAtActor cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// agent JE (PHASE24 JE): PlayNode_Derived 2012 0x9077b0 / 2013 0x8acac0 and the TickNode retail folds with
// UDisConv_PlayerLookAtSpeaker's (2012 0x8f8a50 / 2013 0x8a7980, vtable slot 73 of both). Body in disconv_playerlookatactor.cpp.
public:
	virtual INT TickNode( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const;
protected:
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
public:
