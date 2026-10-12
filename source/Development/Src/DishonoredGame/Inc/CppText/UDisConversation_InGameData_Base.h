// UDisConversation_InGameData_Base cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The per-conversation array of per-node in-game data objects.
// InitForConvNode_Derived 2012 rva 0x8e3640, GetInGameDataForNode 2013 0x88d070. Bodies in disconversation_ingamedata.cpp.
public:
	class UDisConv_Node_InGameData* GetInGameDataForNode( const class UDisConv_Node* _pNode ) const;
protected:
	virtual void InitForConvNode_Derived( const class UDisConv_Node* _pNode );
