// FDisDialogRunningObjs cpptext: included inside the generated struct body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The SDK dump left m_Conversations as a 12-byte gap; retail's bodies
// (BuildFallbackChain 2012 rva 0x90a010, GetInGameNode 0x902b80) read it as the TArray of the tree's per-conversation
// in-game data, one UDisConversation_InGameData per UDisDialogTree::m_Conversations entry. Same 12 bytes, typed here.
	TArrayNoInit<class UDisConversation_InGameData*>& Conversations() { return *(TArrayNoInit<class UDisConversation_InGameData*>*)m_Conversations; }
	const TArrayNoInit<class UDisConversation_InGameData*>& Conversations() const { return *(const TArrayNoInit<class UDisConversation_InGameData*>*)m_Conversations; }
