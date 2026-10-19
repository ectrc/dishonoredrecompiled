// DishonoredGame/src/disconversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas (2012 in parentheses): InitNode (0x8ea770, 79), InitConvNodes
// (0x8e35b0, 131), UpdateOutputLinks 0x899e00 (0x8e83e0, 273), GetFiringPriority (0x8de3f0, 121), GetClassVars_Derived
// 0x895d70 (0x8f0800, 65).
// Not ported (editor; agentFR.md inventory): BreakAllLinks 0x88cd70, SearchNodes 2012 0x8de300, MarkWholeConvoPendingKill
// 2012 0x8de110.
// Agent IE (PHASE23 IE): GetSpeakerName 2013 0x895dc0 (2012 0x8e1390, 956 B) - UDisConv_Soiree::FindBlurbs matches a matinee
// group's name against it at run time. The "Me" slot without a speaker node reads the root tree's m_OwnerNickname and then its
// one-shot's m_MeSpeaker nickname; retail reads the root of the m_pOwner chain as a tree without a cast, GetDialogTree here.
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

void UDisConversation::GetSpeakerName( INT _iIndex, FString& _rNameOut, UBOOL _bUseDecorations ) const
{
	if( _iIndex == -1 || _iIndex >= m_Speakers.Num() || ( m_Speakers(_iIndex).m_Type == 3 && !m_Speakers(_iIndex).m_pLinkedNode ) )
	{
		if( _iIndex == -1 )
		{
			_rNameOut = _bUseDecorations ? TEXT("<None>") : TEXT("");
		}
		else
		{
			_rNameOut = _bUseDecorations ? TEXT("<???>") : TEXT("");
		}
		return;
	}
	const FDisConvSpeaker& Speaker = m_Speakers(_iIndex);
	switch( Speaker.m_Type )
	{
	case 0:
		if( Speaker.m_pLinkedNode )
		{
			Speaker.m_pLinkedNode->GetSpeakerName( _rNameOut, _bUseDecorations );
		}
		else
		{
			const UDisDialogTree* Tree = GetDialogTree();
			if( Tree->m_OwnerNickname.Len() > 0 )
			{
				_rNameOut = _bUseDecorations ? FString::Printf( TEXT("%s, <Me>"), *Tree->m_OwnerNickname ) : FString( Tree->m_OwnerNickname );
			}
			else if( Tree->m_pBoundToOneShot && Tree->m_pBoundToOneShot->m_MeSpeaker.m_SpeakerNickName != NAME_None )
			{
				const FString NickName = Tree->m_pBoundToOneShot->m_MeSpeaker.m_SpeakerNickName.GetNameString();
				_rNameOut = _bUseDecorations ? FString::Printf( TEXT("%s, <Me>"), *NickName ) : NickName;
			}
			else
			{
				_rNameOut = _bUseDecorations ? TEXT("<Me>") : TEXT("");
			}
		}
		break;
	case 1:
		if( IsA( UDisDialogTree::StaticClass() ) )
		{
			_rNameOut = _bUseDecorations ? TEXT("<Initiator>") : TEXT("");
		}
		else if( Speaker.m_pLinkedNode )
		{
			Speaker.m_pLinkedNode->GetSpeakerName( _rNameOut, _bUseDecorations );
		}
		else
		{
			_rNameOut = TEXT("<Initiator>");
		}
		break;
	case 2:
		_rNameOut = _bUseDecorations ? TEXT("<Player>") : TEXT("Player");
		break;
	case 3:
		Speaker.m_pLinkedNode->GetSpeakerName( _rNameOut, _bUseDecorations );
		break;
	}
}
