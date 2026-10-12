// DishonoredGame/src/disconv_node.cpp
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas (2012 in parentheses): InitNode 0x88c740 (0x8dd750, 208),
// PlayNode (0x8da990, 14), GetClassVars (0x8d97c0, 34), GetDialogTree 0x88b7a0 (0x8da9c0, 26).
// Not ported (editor; agentFR.md inventory): SearchNode 0x898c00, ExtractText / ExtractText_Derived 0x88af20,
// OnDelete 2012 0x8da9e0, ShouldApplyGlobally 0x8dd820.
#include "DishonoredGame.h"

void UDisConv_Node::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	m_pOwner = _pOwner;
	if( _bIsNewNode )
	{
		m_iSpeaker = -1;
	}
	else
	{
		const UDisConv_Node* MyRoot = this;
		for( UDisConversation* Owner = _pOwner; Owner; Owner = Owner->m_pOwner )
		{
			MyRoot = Owner;
		}
		for( INT i = 0; i < m_OutputLinks.Num(); i++ )
		{
			UDisConv_Node* Link = m_OutputLinks(i).m_pLink;
			if( Link )
			{
				const UDisConv_Node* LinkRoot = Link;
				for( UDisConversation* Owner = Link->m_pOwner; Owner; Owner = Owner->m_pOwner )
				{
					LinkRoot = Owner;
				}
				if( LinkRoot != MyRoot )
				{
					m_OutputLinks(i).m_pLink = NULL;
				}
			}
		}
	}
	if( _bForEditor )
	{
		SetFlags( RF_Transactional );
	}
}

INT UDisConv_Node::PlayNode( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	return PlayNode_Derived( _rbNotFinished, _pRunningInstance );
}

void UDisConv_Node::GetClassVars( FDisConvClassVars& _rVarsOut ) const
{
	appMemzero( &_rVarsOut, sizeof(_rVarsOut) );
	GetClassVars_Derived( _rVarsOut );
}

UDisDialogTree* UDisConv_Node::GetDialogTree()
{
	UDisConv_Node* Root = this;
	for( UDisConversation* Owner = m_pOwner; Owner; Owner = Owner->m_pOwner )
	{
		Root = Owner;
	}
	return Cast<UDisDialogTree>( Root );
}
