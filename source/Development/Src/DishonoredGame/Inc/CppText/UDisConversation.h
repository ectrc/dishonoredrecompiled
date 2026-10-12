// UDisConversation cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). 2012 rvas InitNode 0x8ea770, InitConvNodes 0x8e35b0, UpdateOutputLinks
// 0x8e83e0 (2013 0x899e00), GetFiringPriority 0x8de3f0, GetClassVars_Derived 0x8f0800 (2013 0x895d70). Bodies in
// disconversation.cpp.
public:
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual void GetClassVars_Derived( struct FDisConvClassVars& _rVarsOut ) const;
	eDisConvPriority GetFiringPriority( const struct FDisDialogRunningInstance* _pRunningInst ) const;
protected:
	void InitConvNodes( TArrayNoInit<class UDisConv_Node*>& _rNodes, class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	void UpdateOutputLinks();
