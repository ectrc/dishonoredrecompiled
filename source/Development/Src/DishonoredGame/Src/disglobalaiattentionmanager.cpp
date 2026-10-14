// DishonoredGame/src/disglobalaiattentionmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7): the three below, DisGlobalAIAttentionManager::AttnInfoFreeListGetNext (0x888440, the
// free-list link accessor) and the three class-registration helpers.
//
// ---- agent GI ports (PHASE20 GI): the pool of simple attention infos ----

#include "DishonoredGame.h"
#include "disattentioninfo.h"

void DishonoredLinkUDisGlobalAIAttentionManager()
{
}

// DISHONORED(port): 2012 rva 0x890e30 / 2013 0x846070 (198 B): the head of the free list, or a new object under this manager
// (the transient package when the manager is the sentinel). The debug counter is marked dirty either way.
UDisAttentionInfo_Simple* UDisGlobalAIAttentionManager::GetSimpleAttnInfoFromPool()
{
	m_bDebug_AllocNumDirty = 1;
	UDisAttentionInfo_Simple* Info = m_pSimpleAttnInfoFreeList;
	if( Info )
	{
		m_pSimpleAttnInfoFreeList = Info->m_pSimpleAttnInfoFreeList_Next;
		Info->m_pSimpleAttnInfoFreeList_Next = NULL;
		return Info;
	}
	return ConstructObject<UDisAttentionInfo_Simple>( UDisAttentionInfo_Simple::StaticClass(), this );
}

// DISHONORED(port): 2012 rva 0x890f00 / 2013 0x846140 (30 B)
void UDisGlobalAIAttentionManager::ReleaseSimpleAttnInfoToPool( UDisAttentionInfo_Simple* _pReleaseMe )
{
	if( !_pReleaseMe )
	{
		return;
	}
	_pReleaseMe->m_pSimpleAttnInfoFreeList_Next = m_pSimpleAttnInfoFreeList;
	m_pSimpleAttnInfoFreeList = _pReleaseMe;
	m_bDebug_AllocNumDirty = 1;
}

// DISHONORED(port): 2012 rva 0x88b7f0 (12 B): a map change drops the pool.
void UDisGlobalAIAttentionManager::PostCommitMapChange()
{
	m_pSimpleAttnInfoFreeList = NULL;
}

void UDisGlobalAIAttentionManager::FinishDestroy()
{
	if( DisGIDiagEnabled() )
	{
		debugf( TEXT("gidiag destroy %s at 0x%08x outer %s"), *GetFullName(), (DWORD)(PTRINT)this, GetOuter() ? *GetOuter()->GetName() : TEXT("none") );
	}
	Super::FinishDestroy();
}
