// DishonoredGame/src/disconv_dialoghook.cpp
// DISHONORED(port): agent FR (PHASE17 FR). InitNode 2012 rva 0x8f8860 (89 B), PlayNode_Derived 2013 0x8a77d0 (2012
// 0x8f88b0, 25 B). OnDelete_Derived 0x8fb150 is editor-only (agentFR.md inventory).
#include "DishonoredGame.h"

void UDisConv_DialogHook::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	UDisConv_Node::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	if( m_HookEnum == DDH_INVALID )
	{
		m_HookEnum = (BYTE)m_iHookIndex;
		if( UDisDialogTree* Tree = GetDialogTree() )
		{
			Tree->m_bNeedsResaving = TRUE;
		}
	}
}

INT UDisConv_DialogHook::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	return _pRunningInstance->m_iDialogHookBranchIndex;
}
