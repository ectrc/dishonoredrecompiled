// FDisConvSaveData cpptext: included inside the generated struct body (DishonoredGameGlobalEnumsClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The per-speaker record of which trees, conversations and labels have fired.
// 2013 rvas FindDialogData 0x8a2f30, FindConvData 0x8a30a0, FindLabelData 2012 0x8f2520. Bodies in disconvsavedata.cpp.
	struct FDisConvSaveData_DialogTree* FindDialogData( const class UDisDialogTree* _pTree, UBOOL _bAddIfNeeded );
	struct FDisConvSaveData_Conversation* FindConvData( const class UDisDialogTree* _pTree, const class UDisConversation* _pConv, UBOOL _bAddIfNeeded );
	struct FDisConvSaveData_Label* FindLabelData( const class UDisDialogTree* _pTree, const class UDisConversation* _pConv, const FName& _rLabel, UBOOL _bAddIfNeeded );
