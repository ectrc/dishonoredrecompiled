// DishonoredGame/src/dishonoredanimnodetreeref.cpp
// A leaf node that delegates to a whole nested UDishonoredAnimTree instance (m_pAnimTree_Instance). The instance is
// ticked and evaluated as a sub-tree through UAnimTree::TickTree / UAnimTree::GetBoneAtoms, and its nodes join the
// parent's node list so FindAnimNode and the relevancy bookkeeping reach them.
// PDB functions attributed to this file (15), 2012 rvas:
//   0x6d0020  UDishonoredAnimNodeTreeRef::InitializePrivateStaticClass   (generated)
//   0x6d0040  UDishonoredAnimNodeTreeRef::InitTreeInstance
//   0x6d0070  UDishonoredAnimNodeTreeRef::InitAnim
//   0x6d00d0  UDishonoredAnimNodeTreeRef::GetBoneAtoms
//   0x6d0110  UDishonoredAnimNodeTreeRef::BuildEdgeAnimTree              (not ported: no whole-tree Edge path here)
//   0x6d0140  UDishonoredAnimNodeTreeRef::CallDeferredInitAnim
//   0x6d4ae0  UDishonoredAnimNodeTreeRef::GetConnectionLocation          (editor only, not ported)
//   0x6d4b10  UDishonoredAnimNodeTreeRef::PostAnimNodeInstance
//   0x6d4b50  UDishonoredAnimNodeTreeRef::TickAnim
//   0x6d9370  UDishonoredAnimNodeTreeRef::GetNodesInternal
//   0x6e4900  UDishonoredAnimNodeTreeRef::GetNodeTitle                   (editor only, not ported)
//   0x6f31c0  UDishonoredAnimNodeTreeRef::DrawAnimNode                   (editor only, not ported)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x67dfd0 (2012 0x6d0040)
void UDishonoredAnimNodeTreeRef::InitTreeInstance( UDishonoredAnimTree* Tree, UAnimTree* ParentTree, USkeletalMeshComponent* MeshComp )
{
	if( Tree )
	{
		Tree->InitAnimTree( MeshComp );
		Tree->m_pParentAnimTree_ForRef = ParentTree;
	}
}

// DISHONORED(written): 2013 rva 0x67e000 (2012 0x6d0070)
void UDishonoredAnimNodeTreeRef::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	UAnimNode::InitAnim( MeshComp, Parent );

	UDishonoredAnimTree* Instance = ( GIsGame || GIsEditor ) ? m_pAnimTree_Instance : NULL;
	if( Instance )
	{
		Instance->InitAnimTree( MeshComp );
		Instance->m_pParentAnimTree_ForRef = m_pParentAnimTree;
	}
}

// DISHONORED(written): 2013 rva 0x682930 (2012 0x6d4b50): the sub-tree is ticked with this node's total weight as the
// weight of its own root
void UDishonoredAnimNodeTreeRef::TickAnim( FLOAT DeltaSeconds )
{
	if( GIsGame || GIsEditor )
	{
		if( m_pAnimTree_Instance )
		{
			m_pAnimTree_Instance->TickTree( DeltaSeconds, NodeTotalWeight );
		}
	}
}

// DISHONORED(written): 2013 rva 0x67e090 (2012 0x6d00d0)
void UDishonoredAnimNodeTreeRef::GetBoneAtoms( FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones, FBoneAtom& RootMotionDelta, INT& bHasRootMotion, FCurveKeyArray& CurveKeys )
{
	if( ( GIsGame || GIsEditor ) && m_pAnimTree_Instance )
	{
		m_pAnimTree_Instance->GetBoneAtoms( Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
	}
	else
	{
		UAnimNode::GetBoneAtoms( Atoms, DesiredBones, RootMotionDelta, bHasRootMotion, CurveKeys );
	}
}

// DISHONORED(written): 2012 rva 0x6d9370 (unnamed in the 2013 db)
void UDishonoredAnimNodeTreeRef::GetNodesInternal( TArray<UAnimNode*>& Nodes )
{
	if( SearchTag != UAnimNode::CurrentSearchTag )
	{
		SearchTag = UAnimNode::CurrentSearchTag;
		Nodes.AddItem( this );
		if( m_pAnimTree_Instance )
		{
			const INT SubNodeCount = m_pAnimTree_Instance->AnimTickArray.Num();
			if( Nodes.GetSlack() < SubNodeCount )
			{
				Nodes.Reserve( Nodes.Num() + SubNodeCount );
			}
			m_pAnimTree_Instance->GetNodesInternal( Nodes );
		}
	}
}

// DISHONORED(written): 2013 rva 0x67e100 (2012 0x6d0140)
void UDishonoredAnimNodeTreeRef::CallDeferredInitAnim()
{
	UAnimNode::CallDeferredInitAnim();

	if( GIsGame || GIsEditor )
	{
		if( m_pAnimTree_Instance )
		{
			m_pAnimTree_Instance->CallDeferredInitAnim();
		}
	}
}

// DISHONORED(written): 2013 rva 0x6926a0 (2012 0x6d4b10): an instanced tree-ref gets its own copy of the referenced tree
void UDishonoredAnimNodeTreeRef::PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap )
{
	if( m_pAnimTree && GIsGame )
	{
		m_pAnimTree_Instance = Cast<UDishonoredAnimTree>( m_pAnimTree->CopyAnimTree( SourceNode->GetOuter() ) );
	}
	else
	{
		m_pAnimTree_Instance = NULL;
	}
}
