// DishonoredGame/src/disstimmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x764e50  public: static void __cdecl UDisStimManager::InitializePrivateStaticClassUDisStimManager(void)
//   0x764e70  private: void * __thiscall UDisStimManager::AllocateBlock_Common(struct FDisStimHeader *, int)
//   0x764ed0  public: virtual int __thiscall UDisStimManager::GetResourceSize(void)
//   0x767a90  public: void __thiscall UDisStimManager::InitStimManager(int, int)
//   0x767b20  public: void __thiscall UDisStimManager::SaveStim(class FArchive &, struct FAIStimStruct const *)
//   0x767ba0  public: void __thiscall UDisStimManager::LoadStim(class FArchive &, struct FAIStimStruct const * &)
//   0x767c80  private: void * __thiscall UDisStimManager::AllocateBlock(int, class FName const &)
//   0x767ca0  private: void __thiscall UDisStimManager::ReleaseBlock(void *)
//   0x76fc10  public: static class UClass * __cdecl UDisStimManager::GetPrivateStaticClassUDisStimManager(wchar_t const *)
//   0x772fd0  public: static class UClass * __cdecl UDisStimManager::StaticClassNoInline(void)
//   0x777080  public: void __thiscall UDisStimManager::TermStimManager(void)

#include "DishonoredGame.h"

/**
 * DISHONORED(layout): 2012 PDB FDisStimHeader, 8 bytes. It is what a FREE block holds in its first eight bytes; a live
 * block holds the FAIStimStruct instead, starting with its vtable pointer. Retail declares it beside this unit; ours keeps
 * it file-local because the stim structs themselves (aistimstruct.cpp) are not ported and nothing outside this unit needs
 * the type.
 */
struct FDisStimHeader
{
	FDisStimHeader*	m_pPrevFreeBlock;
	FDisStimHeader*	m_pNextFreeBlock;
};

/**
 * DISHONORED(written): retail ends a live block's life by calling slot 0 of its own vtable with the "do not free" flag
 * (`(**(void(__thiscall***)(char*,DWORD))pBlock)(pBlock, 0)` in TermStimManager), i.e. the scalar deleting destructor of
 * whatever FAIStimStruct subclass was constructed there. Calling it through this one-slot stand-in produces the same call
 * on the MSVC x86 ABI, and is how the pool can be torn down while the stim structs themselves are still unported.
 */
class FDisStimVirtualBase
{
public:
	virtual ~FDisStimVirtualBase() {}
};

// DISHONORED(port): 2013 rva 0x724990 (2012 0x767a90): the block size is rounded up to 8, one allocation holds every
// block, and the free list is threaded through the pool in order so that the first allocation comes off the front.
void UDisStimManager::InitStimManager( INT _MaxStimSize_bytes, INT _MaxNumStims )
{
	m_MaxStimSize_bytes = ( _MaxStimSize_bytes + 7 ) & ~7;
	m_MaxNumStims = _MaxNumStims;
	m_AllocSize = _MaxNumStims * m_MaxStimSize_bytes;

	BYTE* pStimData = (BYTE*)appMalloc( m_AllocSize );
	m_pStimData = (FPointer)pStimData;

	for( INT i = 0; i < _MaxNumStims; ++i )
	{
		FDisStimHeader* pHeader = (FDisStimHeader*)( pStimData + i * m_MaxStimSize_bytes );
		pHeader->m_pPrevFreeBlock = ( i > 0 ) ? (FDisStimHeader*)( pStimData + ( i - 1 ) * m_MaxStimSize_bytes ) : NULL;
		pHeader->m_pNextFreeBlock = ( i < _MaxNumStims - 1 ) ? (FDisStimHeader*)( pStimData + ( i + 1 ) * m_MaxStimSize_bytes ) : NULL;
	}
	m_pFirstFreeBlock = m_pStimData;
}

// DISHONORED(port): 2013 rva 0x73d3f0 (2012 0x777080): every block that is NOT on the free list still holds a live stim, so
// it is marked, and the unmarked ones are destroyed through their own vtable before the pool is released.
// DISHONORED(written): retail collects the marks in a TMemStackArray<UBOOL>; TMemStackArray is an unported header
// (Engine/Inc/arkutils_memstackarray.h), so this uses a TArray - same behaviour, different allocator.
void UDisStimManager::TermStimManager()
{
	BYTE* pStimData = (BYTE*)m_pStimData;
	if( !pStimData )
	{
		return;
	}

	TArray<UBOOL> bFreeBlocks;
	bFreeBlocks.AddZeroed( m_MaxNumStims );
	for( FDisStimHeader* pBlock = (FDisStimHeader*)m_pFirstFreeBlock; pBlock; pBlock = pBlock->m_pNextFreeBlock )
	{
		const INT BlockIndex = (INT)( ( (BYTE*)pBlock - pStimData ) / m_MaxStimSize_bytes );
		bFreeBlocks( BlockIndex ) = TRUE;
	}

	for( INT i = 0; i < bFreeBlocks.Num(); ++i )
	{
		if( !bFreeBlocks(i) )
		{
			FDisStimVirtualBase* pLiveStim = (FDisStimVirtualBase*)( pStimData + i * m_MaxStimSize_bytes );
			pLiveStim->~FDisStimVirtualBase();
			--m_NumAllocatedStims;
		}
	}

	appFree( pStimData );
	m_pStimData = NULL;
}

// DISHONORED(port): 2013 rva 0x724b80 (2012 0x767c80): a stim that does not fit a block, or an exhausted pool, is passed to
// AllocateBlock_Common as "no block", which makes it a plain heap allocation.
void* UDisStimManager::AllocateBlock( INT _StimSize_bytes, const FName& _rDebugStimName )
{
	FDisStimHeader* pFreeBlock = (FDisStimHeader*)m_pFirstFreeBlock;
	if( !pFreeBlock || _StimSize_bytes > m_MaxStimSize_bytes )
	{
		pFreeBlock = NULL;
	}
	return AllocateBlock_Common( pFreeBlock, _StimSize_bytes );
}

// DISHONORED(port): 2013 rva 0x7275b0 (2012 0x764e70): the block is unlinked from the free list (advancing the head first
// when it was the head) and the allocation count goes up either way.
void* UDisStimManager::AllocateBlock_Common( void* _pWhere, INT _StimSize_bytes )
{
	FDisStimHeader* pWhere = (FDisStimHeader*)_pWhere;
	if( !pWhere )
	{
		++m_NumAllocatedStims;
		return appMalloc( _StimSize_bytes );
	}

	if( (FPointer)pWhere == m_pFirstFreeBlock )
	{
		FDisStimHeader* pNext = pWhere->m_pNextFreeBlock;
		m_pFirstFreeBlock = (FPointer)pNext;
		if( pNext )
		{
			pNext->m_pPrevFreeBlock = NULL;
		}
	}
	if( pWhere->m_pNextFreeBlock )
	{
		pWhere->m_pNextFreeBlock->m_pPrevFreeBlock = pWhere->m_pPrevFreeBlock;
	}
	if( pWhere->m_pPrevFreeBlock )
	{
		pWhere->m_pPrevFreeBlock->m_pNextFreeBlock = pWhere->m_pNextFreeBlock;
	}
	++m_NumAllocatedStims;
	return pWhere;
}

// DISHONORED(port): 2013 rva 0x724ba0 (2012 0x767ca0): a block inside the pool goes back on the front of the free list, one
// outside it was a heap fallback and is freed.
void UDisStimManager::ReleaseBlock( void* _pReleaseMe )
{
	if( IsInPool( _pReleaseMe ) )
	{
		FDisStimHeader* pHeader = (FDisStimHeader*)_pReleaseMe;
		pHeader->m_pPrevFreeBlock = NULL;
		pHeader->m_pNextFreeBlock = (FDisStimHeader*)m_pFirstFreeBlock;
		m_pFirstFreeBlock = (FPointer)pHeader;
		if( pHeader->m_pNextFreeBlock )
		{
			pHeader->m_pNextFreeBlock->m_pPrevFreeBlock = pHeader;
		}
	}
	else
	{
		appFree( _pReleaseMe );
	}
	--m_NumAllocatedStims;
}

// DISHONORED(port): the address test retail inlines into ReleaseBlock and SaveStim. Retail's upper bound is
// m_pStimData + m_MaxStimSize_bytes * (m_MaxNumStims + 1), one block past the pool; it is kept as retail has it.
UBOOL UDisStimManager::IsInPool( const void* _pBlock ) const
{
	const BYTE* pStimData = (const BYTE*)m_pStimData;
	const BYTE* pBlock = (const BYTE*)_pBlock;
	return pBlock >= pStimData && pBlock < pStimData + m_MaxStimSize_bytes * ( m_MaxNumStims + 1 );
}

// DISHONORED(port): 2013 rva 0x727610 (2012 0x764ed0)
INT UDisStimManager::GetResourceSize()
{
	return m_pStimData ? m_AllocSize : 0;
}

/*-----------------------------------------------------------------------------
	DISHONORED(bringup): SaveStim / LoadStim (2012 rvas 0x767b20 / 0x767ba0) are NOT ported. Both need three things that do
	not exist in the tree: FAIStimStruct with its GetScriptStruct() virtual (Src/aistimstruct.cpp is still a comment-only
	skeleton), the static stim registry FAIStimStruct::g_StimTypeInfos (per stim id: m_StimSize_bytes and m_pCreatorFn,
	which is how a loaded stim is reconstructed), and the DisSaveLoad ESaveLoadLocation plumbing. The retail bodies are:

	  SaveStim( FArchive& Ar, const FAIStimStruct* pStim )
	      INT StimTypeID = pStim->m_StimID;        Ar << StimTypeID;
	      INT BlockIndex = IsInPool(pStim) ? (pStim - m_pStimData) / m_MaxStimSize_bytes : INDEX_NONE;
	      Ar << BlockIndex;
	      pStim->GetScriptStruct()->SerializeTaggedProperties( Ar, pStim, NULL );   // UStruct vtable +308

	  LoadStim( FArchive& Ar, const FAIStimStruct*& pOutStim )
	      INT StimTypeID = INDEX_NONE; Ar << StimTypeID;
	      INT BlockIndex = INDEX_NONE; Ar << BlockIndex;
	      the block the save names is looked up in the pool and, only when it is still ON THE FREE LIST, taken out of it
	      with the same unlink AllocateBlock_Common does (or heap-allocated from g_StimTypeInfos[StimTypeID].m_StimSize_bytes
	      when there is no block), then g_StimTypeInfos[StimTypeID].m_pCreatorFn constructs the stim in place and its
	      script struct reads the properties back.

	Note that a stim whose block is no longer free is skipped, so a save reloaded into a pool that already holds live stims
	drops them rather than overwriting - retail behaviour worth keeping when this lands.
-----------------------------------------------------------------------------*/
