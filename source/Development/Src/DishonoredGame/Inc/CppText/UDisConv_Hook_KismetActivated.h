// UDisConv_Hook_KismetActivated cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The hook node DDH_KISMET_ACTIVATED reaches: its output links are named after
// the Kismet input links of the DisSeqAct_DialogInputs that fires them, and PlayNode_Derived picks the one whose name
// the speaker's FDisHookParameters::m_KismetDialogHook carries. 2013 rvas InitNode 0x8bbbb0*, PlayNode_Derived 0x8ac350*,
// EvaluateForFiring 0x8a9200* (2012 0x907010 / 0x8fb740 / 0x8f9c00, MatchHooksToOutputLinks 0x903270; * = by vtable
// slot, unnamed in the 2013 database). Bodies in disconv_hook_kismetactivated.cpp.
public:
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT EvaluateForFiring( struct FDisDialogRunningInstance* _pRunningInstance ) const;
protected:
	void MatchHooksToOutputLinks( UBOOL _bForEditor );
