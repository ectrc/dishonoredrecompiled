// UDisConv_Node cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The node virtuals of the conversation graph, in retail's vtable order
// (2013 ??_7UDisConv_Soiree@@6B@ at 0x119ed30): 73 InitNode 0x88c740, 74 TickNode, 75 OnDialogStarted (the slot
// FDisDialogRunningInstance::StartDialog calls on m_NodesToNotifyDialogStarted; an ICF-folded no-op in the base),
// 76 InterruptTickingNode, 77 IsTickingNodeInterruptible, 78 SearchNode 0x898c00 (editor), 79 GetClassVars_Derived,
// 80 PlayNode_Derived, 81 ExtractText_Derived 0x88af20. The non-virtual entry points are PlayNode (2012 0x8da990, 14 B),
// GetClassVars (2012 0x8d97c0) and GetDialogTree (2013 0x88b7a0). Bodies in disconv_node.cpp.
public:
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual INT TickNode( FLOAT _fDeltaTime, struct FDisDialogRunningInstance* _pRunningInstance ) const { return -1; }
	virtual void OnDialogStarted( struct FDisDialogRunningInstance* _pRunningInstance ) {}
	virtual void InterruptTickingNode( struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const {}
	virtual UBOOL IsTickingNodeInterruptible( struct FDisDialogRunningInstance* _pRunningInstance, DisConvoEndReasonEnum _InterruptReason ) const { return FALSE; }
	virtual void GetClassVars_Derived( struct FDisConvClassVars& _rVarsOut ) const {}
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const { return 0; }
	INT PlayNode( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	void GetClassVars( struct FDisConvClassVars& _rVarsOut ) const;
	class UDisDialogTree* GetDialogTree();
	const class UDisDialogTree* GetDialogTree() const { return const_cast<UDisConv_Node*>( this )->GetDialogTree(); }
