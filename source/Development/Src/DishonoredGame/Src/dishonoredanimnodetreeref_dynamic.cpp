// DishonoredGame/src/dishonoredanimnodetreeref_dynamic.cpp
// A tree-ref whose referenced tree is chosen at run time out of m_PossibleTemplates by name. This is the node the
// state picker drives: SetActiveTreeReference picks the template, instancing it or taking it out of the map's
// UDisAnimStatePool, and ClearActiveTreeReference hands it back.
// PDB functions attributed to this file (12), 2012 rvas:
//   0x6d93e0  UDishonoredAnimNodeTreeRef_Dynamic::InitAnim
//   0x6d94b0  UDishonoredAnimNodeTreeRef_Dynamic::PostAnimNodeInstance
//   0x6d9680  UDishonoredAnimNodeTreeRef_Dynamic::BuildParentNodesArray
//   0x6d9700  UDishonoredAnimNodeTreeRef_Dynamic::BuildTickArray
//   0x6d97c0  UDishonoredAnimNodeTreeRef_Dynamic::CheckForReleaseToPool
//   0x6e49a0  UDishonoredAnimNodeTreeRef_Dynamic::ClearActiveTreeReference
//   0x6ed730  UDishonoredAnimNodeTreeRef_Dynamic::CleanupForAnimStates
//   0x6ed810  UDishonoredAnimNodeTreeRef_Dynamic::OnCeaseRelevant
//   0x6eee20  UDishonoredAnimNodeTreeRef_Dynamic::SetActiveTreeReference

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

/** The map's shared state-tree pool; NULL until the map info exists (menu map, commandlets). */
static UDisAnimStatePool* DisGetAnimStatePool()
{
	UDishonoredMapInfo* MapInfo = DishonoredGetMapInfo();
	return MapInfo ? MapInfo->m_pAnimStateTreePool : NULL;
}

// DISHONORED(written): 2013 rva 0x68a060 (2012 0x6d93e0): in game the pooled instances are initialised directly, in the
// editor the base class owns the single instance
void UDishonoredAnimNodeTreeRef_Dynamic::InitAnim( USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent )
{
	if( !GIsGame )
	{
		Super::InitAnim( MeshComp, Parent );
		return;
	}

	UAnimNode::InitAnim( MeshComp, Parent );

	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		UDishonoredAnimTree* Instance = m_PossibleTemplates(TemplateIdx).m_pAnimTree_Instance;
		if( Instance )
		{
			Instance->InitAnimTree( MeshComp );
			Instance->m_pParentAnimTree_ForRef = m_pParentAnimTree;
		}
	}
}

// DISHONORED(written): 2013 rva 0x6926f0 (2012 0x6d94b0): a pooled template registers its pool settings and keeps its
// instance slot empty, a non-pooled one gets its own copy of the tree right away
void UDishonoredAnimNodeTreeRef_Dynamic::PostAnimNodeInstance( UAnimNode* SourceNode, TMap<UAnimNode*,UAnimNode*>& SrcToDestNodeMap )
{
	m_bInitializedSpawnedPoolTrees = TRUE;

	INT DefaultIndex = INDEX_NONE;
	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		FDynamicTreeTemplate& Template = m_PossibleTemplates(TemplateIdx);
		UDisAnimStatePool* Pool = DisGetAnimStatePool();

		if( Template.m_bUseAnimTreePooling )
		{
			if( Pool )
			{
				Pool->UpdatePoolSettings( Template );
			}
			if( Template.m_PoolSettings.m_bFetchAtSpawnTime )
			{
				m_bInitializedSpawnedPoolTrees = FALSE;
			}
		}
		else if( m_bPoolAllTreesAsTemporary )
		{
			FDynamicTreeTemplate TempPoolSettings = Template;
			TempPoolSettings.m_bUseAnimTreePooling = TRUE;
			TempPoolSettings.m_PoolSettings.m_TypicalSimultaneousActiveTrees = 1;
			TempPoolSettings.m_PoolSettings.m_bReturnToPoolNonRelevant = TRUE;
			TempPoolSettings.m_PoolSettings.m_bFetchAtSpawnTime = FALSE;
			if( Pool )
			{
				Pool->UpdatePoolSettings( TempPoolSettings );
			}
		}

		if( !Template.m_pAnimTree_Template || Template.m_bUseAnimTreePooling || m_bPoolAllTreesAsTemporary )
		{
			Template.m_pAnimTree_Instance = NULL;
		}
		else
		{
			Template.m_pAnimTree_Instance = Cast<UDishonoredAnimTree>( Template.m_pAnimTree_Template->CopyAnimTree( SourceNode->GetOuter() ) );
		}

		if( Template.m_pAnimTree_Template == m_pAnimTree )
		{
			DefaultIndex = TemplateIdx;
		}
	}

	if( DefaultIndex == INDEX_NONE )
	{
		m_pAnimTree = NULL;
		m_pAnimTree_Instance = NULL;
		m_CurRef = NAME_None;
		m_iCurRefIndex = INDEX_NONE;
	}
	else
	{
		m_pAnimTree_Instance = m_PossibleTemplates(DefaultIndex).m_pAnimTree_Instance;
		m_CurRef = m_PossibleTemplates(DefaultIndex).m_AnimTree_Name;
		m_iCurRefIndex = DefaultIndex;
	}
}

// DISHONORED(written): 2013 rva 0x68a130 (2012 0x6d9680)
void UDishonoredAnimNodeTreeRef_Dynamic::BuildParentNodesArray()
{
	UAnimNode::BuildParentNodesArray();

	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		UDishonoredAnimTree* Instance = m_PossibleTemplates(TemplateIdx).m_pAnimTree_Instance;
		if( Instance )
		{
			Instance->BuildParentNodesArray();
		}
	}
}

// DISHONORED(written): 2013 rva 0x68a1b0 (2012 0x6d9700): the templates flagged m_bFetchAtSpawnTime take a tree out of
// the pool once, when the owner's tick array is built. Deliberately no Super call: a tree-ref's sub-tree is ticked
// through UAnimTree::TickTree, not through the owner's tick array.
void UDishonoredAnimNodeTreeRef_Dynamic::BuildTickArray( TArray<UAnimNode*>& OutTickArray )
{
	if( !GIsGame || m_bInitializedSpawnedPoolTrees )
	{
		return;
	}

	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		FDynamicTreeTemplate& Template = m_PossibleTemplates(TemplateIdx);
		if( Template.m_bUseAnimTreePooling && Template.m_PoolSettings.m_bFetchAtSpawnTime && !Template.m_pAnimTree_Instance )
		{
			UDisAnimStatePool* Pool = DisGetAnimStatePool();
			if( Pool )
			{
				Template.m_pAnimTree_Instance = Pool->GetPooledTree( Template.m_pAnimTree_Template, SkelComponent, this );
			}
		}
	}
	m_bInitializedSpawnedPoolTrees = TRUE;
}

// DISHONORED(written): 2013 rva 0x68a270 (2012 0x6d97c0)
void UDishonoredAnimNodeTreeRef_Dynamic::CheckForReleaseToPool( INT Index )
{
	FDynamicTreeTemplate& Template = m_PossibleTemplates(Index);
	const UBOOL bPooled = Template.m_bUseAnimTreePooling || m_bPoolAllTreesAsTemporary;
	const UBOOL bReturnWhenIdle = Template.m_PoolSettings.m_bReturnToPoolNonRelevant || m_bPoolAllTreesAsTemporary;
	if( bPooled && Template.m_pAnimTree_Instance && bReturnWhenIdle )
	{
		UDisAnimStatePool* Pool = DisGetAnimStatePool();
		if( Pool )
		{
			Pool->ReturnToPool( Template.m_pAnimTree_Template, Template.m_pAnimTree_Instance );
		}
		Template.m_pAnimTree_Instance = NULL;
	}
}

// DISHONORED(written): 2013 rva 0x6928c0 (2012 0x6e49a0): the sub-tree's nodes lose their relevancy, so the state tree
// starts from scratch the next time it is picked
void UDishonoredAnimNodeTreeRef_Dynamic::ClearActiveTreeReference( UBOOL bCeaseRelevant )
{
	if( m_iCurRefIndex == INDEX_NONE )
	{
		return;
	}

	CheckForReleaseToPool( m_iCurRefIndex );

	if( bCeaseRelevant && m_pAnimTree_Instance )
	{
		UDishonoredAnimTree* Instance = m_pAnimTree_Instance;
		for( INT NodeIdx = 0; NodeIdx < Instance->AnimTickArray.Num(); NodeIdx++ )
		{
			UAnimNode* Node = Instance->AnimTickArray(NodeIdx);
			if( NodeIdx < Instance->AnimTickRelevancyArray.Num() )
			{
				Instance->AnimTickRelevancyArray(NodeIdx) = 0;
			}
			if( NodeIdx < Instance->AnimTickWeightsArray.Num() )
			{
				Instance->AnimTickWeightsArray(NodeIdx) = 0.f;
			}
			if( Node )
			{
				Node->OnCeaseRelevant();
			}
		}
	}

	m_iCurRefIndex = INDEX_NONE;
	m_CurRef = NAME_None;
	m_pAnimTree = NULL;
	m_pAnimTree_Instance = NULL;
}

// DISHONORED(written): 2013 rva 0x69b030 (2012 0x6ed730): map teardown; m_bKeepUntilFullDestroy trees survive a level
// transition and only go back on a full destroy
void UDishonoredAnimNodeTreeRef_Dynamic::CleanupForAnimStates( UBOOL bFullDestroy )
{
	if( !GIsGame )
	{
		return;
	}
	UDisAnimStatePool* Pool = DisGetAnimStatePool();
	if( !Pool )
	{
		return;
	}

	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		FDynamicTreeTemplate& Template = m_PossibleTemplates(TemplateIdx);
		const UBOOL bPooled = Template.m_bUseAnimTreePooling || m_bPoolAllTreesAsTemporary;
		if( bPooled && Template.m_pAnimTree_Instance && ( !Template.m_PoolSettings.m_bKeepUntilFullDestroy || bFullDestroy ) )
		{
			if( m_iCurRefIndex == TemplateIdx )
			{
				ClearActiveTreeReference( FALSE );
			}
			else
			{
				Pool->ReturnToPool( Template.m_pAnimTree_Template, Template.m_pAnimTree_Instance );
				Template.m_pAnimTree_Instance = NULL;
			}
		}
	}
}

// DISHONORED(written): 2013 rva 0x69b110 (2012 0x6ed810)
void UDishonoredAnimNodeTreeRef_Dynamic::OnCeaseRelevant()
{
	if( GIsGame )
	{
		for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
		{
			if( TemplateIdx == m_iCurRefIndex )
			{
				ClearActiveTreeReference( TRUE );
			}
			else
			{
				CheckForReleaseToPool( TemplateIdx );
			}
		}
	}
	UAnimNode::OnCeaseRelevant();
}

// DISHONORED(written): 2013 rva 0x6928c0-adjacent (2012 0x6eee20, unnamed in the 2013 db): pick the named template, take
// its tree out of the pool if it is pooled, release the previous one and clear the relevancy of every node under it
UDishonoredAnimTree* UDishonoredAnimNodeTreeRef_Dynamic::SetActiveTreeReference( FName TreeRefName )
{
	if( ( !GIsGame && !GIsEditor ) || m_CurRef == TreeRefName )
	{
		return m_pAnimTree_Instance;
	}

	const INT OldRefIndex = m_iCurRefIndex;
	for( INT TemplateIdx = 0; TemplateIdx < m_PossibleTemplates.Num(); TemplateIdx++ )
	{
		FDynamicTreeTemplate& Template = m_PossibleTemplates(TemplateIdx);
		if( Template.m_AnimTree_Name != TreeRefName )
		{
			continue;
		}

		if( !Template.m_pAnimTree_Instance && ( Template.m_bUseAnimTreePooling || m_bPoolAllTreesAsTemporary ) )
		{
			UDisAnimStatePool* Pool = DisGetAnimStatePool();
			if( Pool )
			{
				Template.m_pAnimTree_Instance = Pool->GetPooledTree( Template.m_pAnimTree_Template, SkelComponent, this );
			}
		}

		m_CurRef = TreeRefName;
		m_iCurRefIndex = TemplateIdx;
		m_pAnimTree = Template.m_pAnimTree_Template;
		m_pAnimTree_Instance = Template.m_pAnimTree_Instance;

		if( OldRefIndex != INDEX_NONE )
		{
			CheckForReleaseToPool( OldRefIndex );
		}

		TArray<UAnimNode*> Nodes;
		GetNodes( Nodes );
		for( INT NodeIdx = 0; NodeIdx < Nodes.Num(); NodeIdx++ )
		{
			UAnimNode* Node = Nodes(NodeIdx);
			if( !Node )
			{
				continue;
			}
			Node->bRelevant = FALSE;
			UAnimTree* NodeTree = Node->m_pParentAnimTree;
			if( NodeTree && Node->TickArrayIndex >= 0 && Node->TickArrayIndex < NodeTree->AnimTickRelevancyArray.Num() )
			{
				NodeTree->AnimTickRelevancyArray(Node->TickArrayIndex) = 0;
			}
		}
		break;
	}

	return m_pAnimTree_Instance;
}
