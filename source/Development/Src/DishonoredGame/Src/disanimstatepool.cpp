// DishonoredGame/src/disanimstatepool.cpp
// The map-wide pool of instanced anim-state trees (UDishonoredMapInfo::m_pAnimStateTreePool). One FDisAnimTreePool per
// template; a tree-ref takes a copy out of it when its state is picked and hands it back when the state ends, so a
// state tree is copied once per map instead of once per pawn.
// PDB functions attributed to this file (9), 2012 rvas:
//   0x8883a0  UDisAnimStatePool::InitializePrivateStaticClass   (generated)
//   0x890b20  UDisAnimStatePool::FindAnimTreePool
//   0x896840  UDisAnimStatePool::CallPostAnimNodeInstance
//   0x898af0  UDisAnimStatePool::AddTreesToPool
//   0x898bc0  UDisAnimStatePool::ReturnToPool
//   0x898c00  UDisAnimStatePool::GetPooledTree
//   0x89a560  UDisAnimStatePool::UpdatePoolSettings

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x845e60 (2012 0x890b20)
FDisAnimTreePool* UDisAnimStatePool::FindAnimTreePool( const UDishonoredAnimTree* Template )
{
	for( INT PoolIdx = 0; PoolIdx < m_AnimTreePools.Num(); PoolIdx++ )
	{
		if( m_AnimTreePools(PoolIdx).m_pPooledTemplate == Template )
		{
			return &m_AnimTreePools(PoolIdx);
		}
	}
	return NULL;
}

// DISHONORED(written): 2012 rva 0x896840 (unnamed in the 2013 db): PostAnimNodeInstance down the whole copied tree, in
// lockstep with the source tree. Retail passes a NULL node map; ours passes an empty one, because the reference
// signature takes it by reference (no ported override reads it for a pooled copy).
void UDisAnimStatePool::CallPostAnimNodeInstance( UAnimNode* SourceNode, UAnimNode* DestNode )
{
	if( !SourceNode || !DestNode )
	{
		return;
	}

	TMap<UAnimNode*,UAnimNode*> EmptyNodeMap;
	DestNode->ParentNodes.Empty();
	DestNode->NodeTickTag = 0;
	DestNode->PostAnimNodeInstance( SourceNode, EmptyNodeMap );

	UAnimNodeBlendBase* DestBlend = Cast<UAnimNodeBlendBase>( DestNode );
	UAnimNodeBlendBase* SourceBlend = Cast<UAnimNodeBlendBase>( SourceNode );
	if( !DestBlend || !SourceBlend )
	{
		return;
	}

	const INT ChildCount = Min( DestBlend->Children.Num(), SourceBlend->Children.Num() );
	for( INT ChildIdx = 0; ChildIdx < ChildCount; ChildIdx++ )
	{
		UAnimNode* SourceChild = SourceBlend->Children(ChildIdx).Anim;
		UAnimNode* DestChild = DestBlend->Children(ChildIdx).Anim;
		if( SourceChild && DestChild )
		{
			CallPostAnimNodeInstance( SourceChild, DestChild );
		}
	}
}

// DISHONORED(written): 2012 rva 0x898af0 (unnamed in the 2013 db): the copies live in the transient package
void UDisAnimStatePool::AddTreesToPool( FDisAnimTreePool* Pool, UDishonoredAnimTree* Template, INT NumNewTrees )
{
	if( !Pool || !Template || NumNewTrees <= 0 )
	{
		return;
	}

	Pool->m_PooledAnimTrees.Reserve( Pool->m_MaxPooledTrees + NumNewTrees );
	Pool->m_MaxPooledTrees += NumNewTrees;

	for( INT TreeIdx = 0; TreeIdx < NumNewTrees; TreeIdx++ )
	{
		UDishonoredAnimTree* NewTree = Cast<UDishonoredAnimTree>( Template->CopyAnimTree( UObject::GetTransientPackage() ) );
		if( !NewTree )
		{
			continue;
		}
		CallPostAnimNodeInstance( Template, NewTree );
		NewTree->bParentNodeArrayBuilt = FALSE;
		Pool->m_PooledAnimTrees.AddItem( NewTree );
	}
}

// DISHONORED(written): 2013 rva 0x85aeb0 (2012 0x89a560): grow the pool to the template's declared simultaneous count
void UDisAnimStatePool::UpdatePoolSettings( const FDynamicTreeTemplate& Settings )
{
	if( !Settings.m_pAnimTree_Template )
	{
		return;
	}

	FDisAnimTreePool* Pool = FindAnimTreePool( Settings.m_pAnimTree_Template );
	if( !Pool )
	{
		const INT PoolIdx = m_AnimTreePools.Add( 1 );
		appMemzero( &m_AnimTreePools(PoolIdx), sizeof(FDisAnimTreePool) );
		Pool = &m_AnimTreePools(PoolIdx);
		Pool->m_pPooledTemplate = Settings.m_pAnimTree_Template;
	}

	if( Pool->m_MaxPooledTrees < Settings.m_PoolSettings.m_TypicalSimultaneousActiveTrees )
	{
		AddTreesToPool( Pool, Settings.m_pAnimTree_Template, Settings.m_PoolSettings.m_TypicalSimultaneousActiveTrees - Pool->m_MaxPooledTrees );
	}
}

// DISHONORED(written): 2013 rva 0x858b80 (2012 0x898c00): an empty pool grows by one rather than failing
UDishonoredAnimTree* UDisAnimStatePool::GetPooledTree( const UDishonoredAnimTree* Template, USkeletalMeshComponent* SkelComp, UDishonoredAnimNodeTreeRef_Dynamic* ParentAnimNode )
{
	FDisAnimTreePool* Pool = FindAnimTreePool( Template );
	if( !Pool )
	{
		// DISHONORED(bringup): retail dereferences the result; a template that never registered its pool settings
		// (UpdatePoolSettings) would fault there
		debugf( TEXT("DISHONORED(bringup): DisAnimStatePool::GetPooledTree: no pool for %s"), Template ? *Template->GetPathName() : TEXT("NULL") );
		return NULL;
	}

	if( Pool->m_PooledAnimTrees.Num() == 0 )
	{
		AddTreesToPool( Pool, const_cast<UDishonoredAnimTree*>(Template), 1 );
		if( Pool->m_PooledAnimTrees.Num() == 0 )
		{
			return NULL;
		}
	}

	UDishonoredAnimTree* Tree = Pool->m_PooledAnimTrees( Pool->m_PooledAnimTrees.Num() - 1 );
	Pool->m_PooledAnimTrees.Remove( Pool->m_PooledAnimTrees.Num() - 1, 1 );
	UDishonoredAnimNodeTreeRef::InitTreeInstance( Tree, ParentAnimNode ? ParentAnimNode->m_pParentAnimTree : NULL, SkelComp );
	return Tree;
}

// DISHONORED(written): 2013 rva 0x8588-adjacent (2012 0x898bc0, unnamed in the 2013 db): the tree is reset against its
// template before it goes back, so the next user gets a clean copy
void UDisAnimStatePool::ReturnToPool( const UDishonoredAnimTree* Template, UDishonoredAnimTree* PooledTree )
{
	FDisAnimTreePool* Pool = FindAnimTreePool( Template );
	if( !Pool || !PooledTree )
	{
		return;
	}
	CallPostAnimNodeInstance( Pool->m_pPooledTemplate, PooledTree );
	PooledTree->bParentNodeArrayBuilt = FALSE;
	Pool->m_PooledAnimTrees.AddItem( PooledTree );
}
