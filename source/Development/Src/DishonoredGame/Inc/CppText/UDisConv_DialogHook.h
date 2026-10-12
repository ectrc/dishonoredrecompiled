// UDisConv_DialogHook cpptext: included inside the generated class body (DishonoredGameConversationClasses.h).
// DISHONORED(port): agent FR (PHASE17 FR). The node a dialog tree keys by hook enum (UDisDialogTree::m_DialogHooks).
// InitNode 2012 rva 0x8f8860, PlayNode_Derived 2013 0x8a77d0 (returns the running instance's hook branch index),
// EvaluateForFiring (vtable slot 85 in 2013; the base is an ICF-folded `return 0`, UDisConv_Hook_KismetActivated
// overrides it at 0x8a9200). Bodies in disconv_dialoghook.cpp.
public:
	virtual void InitNode( class UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode );
	virtual INT PlayNode_Derived( UBOOL& _rbNotFinished, struct FDisDialogRunningInstance* _pRunningInstance ) const;
	virtual INT EvaluateForFiring( struct FDisDialogRunningInstance* _pRunningInstance ) const { return 0; }
