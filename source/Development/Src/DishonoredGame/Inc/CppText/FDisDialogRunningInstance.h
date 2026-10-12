// FDisDialogRunningInstance cpptext: included inside the generated struct body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). One of the two running conversations a UDisDialogTree_InGameBind can hold.
// Retail keeps the bodies in disdialogtree_ingamebind.cpp; 2013 rvas StartInstance 0x88bda0, StopInstance 0x8a4410*,
// StartDialog 0x897100*, GetRunningObjects 0x892380, GetInGameNode 0x89a3a0*, GetInGameConv 0x89ae10,
// FiredFromHookName 0x89ada0, NotifyWhenDialogStarts 0x893b80 (* = found through the caller). The SDK names the
// bitfield at +24 m_bStopWhenDialogStarts; every retail body reads it as the check-only flag of the instance
// (StartInstance's last parameter), so that is what it means here.
	void StartInstance( eDisDialogHook _Hook, class IDisConvSpeakerInterface* _pInitiator, INT _iHookSubIndex, UBOOL _bCheckOnly );
	void StopInstance( DisConvoEndReasonEnum _ConvoEndReason );
	void StartDialog( class UDisConversation_InGameData* _pConv );
	UBOOL HasDialogStarted() const { return m_pCurPlayingConversation != NULL; }
	UBOOL IsCheckOnly() const { return m_bStopWhenDialogStarts; }
	FDisDialogRunningObjs& GetRunningObjects();
	class UDisConv_Node_InGameData* GetInGameNode( const class UDisConv_Node* _pNode, eDisConvInGameDataType _DataType );
	class UDisConversation_InGameData* GetInGameConv( const class UDisConv_Node* _pNode );
	const class UDisDialogTree* GetDialogTree() const;
	void FiredFromHookName( FString& _rNameOut ) const;
	void NotifyWhenDialogStarts( class UDisConv_Node* _pNode ) { m_NodesToNotifyDialogStarted.AddUniqueItem( _pNode ); }
