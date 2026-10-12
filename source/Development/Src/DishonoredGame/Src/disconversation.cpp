// DishonoredGame/src/disconversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas (2012 in parentheses): InitNode (0x8ea770, 79), InitConvNodes
// (0x8e35b0, 131), UpdateOutputLinks 0x899e00 (0x8e83e0, 273), GetFiringPriority (0x8de3f0, 121), GetClassVars_Derived
// 0x895d70 (0x8f0800, 65).
// Not ported (editor; agentFR.md inventory): BreakAllLinks 0x88cd70, SearchNodes 2012 0x8de300, GetSpeakerName 0x895dc0,
// MarkWholeConvoPendingKill 2012 0x8de110.
#include "DishonoredGame.h"

void UDisConversation::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	UDisConv_Node::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	InitConvNodes( m_Nodes, _pOwner, _bForEditor, _bIsNewNode );
	InitConvNodes( m_EditorNodes_Background, _pOwner, _bForEditor, _bIsNewNode );
	UpdateOutputLinks();
}

void UDisConversation::InitConvNodes( TArrayNoInit<UDisConv_Node*>& _rNodes, UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	for( INT i = 0; i < _rNodes.Num(); i++ )
	{
		UDisConv_Node* Node = _rNodes(i);
		if( Node )
		{
			Node->m_iConvNodeIndex = i;
			Node->InitNode( this, _bForEditor, _bIsNewNode );
		}
		else
		{
			_rNodes.Remove( i-- );
		}
	}
}

void UDisConversation::UpdateOutputLinks()
{
	const INT NumLinks = m_bPlayerMustBeNear ? 3 : 2;
	if( m_OutputLinks.Num() > NumLinks )
	{
		m_OutputLinks.Remove( NumLinks, m_OutputLinks.Num() - NumLinks );
	}
	else if( m_OutputLinks.Num() < NumLinks )
	{
		m_OutputLinks.AddZeroed( NumLinks - m_OutputLinks.Num() );
	}
	m_OutputLinks(0).m_Description = TEXT("Start");
	m_OutputLinks(1).m_Description = TEXT("Interrupted");
	if( m_bPlayerMustBeNear )
	{
		m_OutputLinks(2).m_Description = TEXT("Player Walked Away");
	}
}

eDisConvPriority UDisConversation::GetFiringPriority( const FDisDialogRunningInstance* _pRunningInst ) const
{
	const UDisConversation* Root = this;
	for( UDisConversation* Owner = m_pOwner; Owner; Owner = Owner->m_pOwner )
	{
		Root = Owner;
	}
	if( Root != this )
	{
		return (eDisConvPriority)m_Priority;
	}
	const UDisDialogTree* Tree = ConstCast<UDisDialogTree>( this );
	const INT Hook = _pRunningInst->m_DialogHookThatFiredInst;
	if( Tree && Hook >= 0 && Hook < Tree->m_DialogHooks.Num() && Tree->m_DialogHooks( Hook ) )
	{
		return (eDisConvPriority)Tree->m_DialogHooks( Hook )->m_Priority;
	}
	return (eDisConvPriority)m_Priority;
}

void UDisConversation::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	_rVarsOut.m_InGameDataClasses[eDisConvInGameDataType_PerNode] = UDisConversation_InGameData::StaticClass();
	_rVarsOut.m_InGameDataClasses[eDisConvInGameDataType_PerNode_Persistent] = UDisConversation_InGameData_Base::StaticClass();
}
