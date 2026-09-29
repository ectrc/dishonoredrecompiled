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
#include "aistimstruct.h"	// the stim type registry and the dispatch table LoadStim reconstructs through
#include "dishonoredutilities_saveload.h"	// DisStopRestore, for the one branch retail cannot reach

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
	DISHONORED(port): agent EJ (PHASE12 EJ) - LoadStim, the one thing UDishonoredAIBrain::GameLoad needs that
	this tree did not have. SaveStim (2013 rva 0x724a00, 2012 0x767b20) stays out for agent ED's reason: the
	writing half of the DisSaveLoad object layer does not exist here. Its retail body, so that the pair reads
	as one format:

	  SaveStim( FArchive& Ar, const FAIStimStruct* pStim )
	      INT StimTypeID = pStim->m_StimID;        Ar << StimTypeID;
	      INT BlockIndex = IsInPool(pStim) ? ((BYTE*)pStim - (BYTE*)m_pStimData) / m_MaxStimSize_bytes : INDEX_NONE;
	      Ar << BlockIndex;
	      pStim->GetScriptStruct()->SerializeBin( Ar, (BYTE*)pStim, 0 );
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x704f60 (2012 0x767ba0). Three things in the stream: the stim's type id, the
// index of the pool block it lived in when the save was written, and then the stim's own script properties,
// binary and untagged.
//
// The block index is a hint and not a promise. Retail looks the named block up in THIS session's pool and
// uses it only while it is still on the free list; a block that is not free (or a pool whose free list is
// empty) is left alone and the properties are read into whatever is already there, through that object's own
// script struct. On a level restore the pool is freshly initialised and every block is free, which is the
// only path a save file exercises.
//
// DISHONORED(bringup): two deviations, both named because neither reads a stream byte.
//   1. m_pVTable. Retail keeps its dispatch table in the object, so the placement-new the creator does
//      installs it; this port keeps it beside the object (see aistimstruct.h) and therefore has to set it,
//      exactly as DisNewStim does.
//   2. m_pStimManager. It is a reflected member of FAIStimStruct with no CPF_Transient, so it is in the
//      binary walk below and the save carries it - which is why retail does not assign it here. It is
//      assigned anyway when the walk left it NULL, because a pooled stim whose manager is NULL never returns
//      its block to the pool (DisStimRefRelease's own rule).
void UDisStimManager::LoadStim( FArchive& _rArchive, const FAIStimStruct*& _rpOutStim )
{
	INT StimTypeID = INDEX_NONE;
	_rArchive << StimTypeID;
	INT BlockIndex = INDEX_NONE;
	_rArchive << BlockIndex;

	FAIStimStruct* pStim = NULL;
	if( BlockIndex >= 0 )
	{
		pStim = (FAIStimStruct*)( (BYTE*)m_pStimData + BlockIndex * m_MaxStimSize_bytes );

		FDisStimHeader* pFreeBlock = (FDisStimHeader*)m_pFirstFreeBlock;
		while( pFreeBlock != NULL && (void*)pFreeBlock != (void*)pStim )
		{
			pFreeBlock = pFreeBlock->m_pNextFreeBlock;
		}

		if( pFreeBlock != NULL )
		{
			const FAIStimTypeInfo* pTypeInfo = DisGetStimTypeInfo( (BYTE)StimTypeID );
			if( pTypeInfo == NULL || pTypeInfo->m_pCreatorFn == NULL )
			{
				// DISHONORED(written): retail indexes g_StimTypeInfos with the saved id and does not check it.
				// A tree whose stim table is incomplete would construct through a NULL creator here, and the
				// crash would say nothing about which body was reading; the properties are still read below,
				// into the block as it stands, so the stream stays in step either way.
				debugf( NAME_Warning, TEXT("DISHONORED(bringup): UDisStimManager::LoadStim: the save names stim type %d, which this tree has no table row for"), StimTypeID );
			}
			else
			{
				pStim = pTypeInfo->m_pCreatorFn( AllocateBlock_Common( pStim, pTypeInfo->m_StimSize_bytes ) );
				DisStimSetVTable( pStim, pTypeInfo->m_pVTable );
			}
		}
	}

	const UScriptStruct* pStruct = ( pStim != NULL ) ? DisStimGetScriptStruct( pStim ) : NULL;
	if( pStruct == NULL )
	{
		// retail dereferences the block from here on with no test at all; with no block, no table row and no
		// script struct there is nothing to read the properties into, and reading none of them would leave the
		// stream one stim short.
		DisStopRestore( _rArchive, FString::Printf( TEXT("UDisStimManager::LoadStim (2013 rva 0x704f60): the save names stim type %d in pool block %d, and this session cannot reconstruct it"), StimTypeID, BlockIndex ) );
		_rpOutStim = NULL;
		return;
	}

	pStruct->SerializeBin( _rArchive, (BYTE*)pStim, 0 );

	if( pStim->m_pStimManager == NULL )
	{
		pStim->m_pStimManager = this;
	}
	_rpOutStim = pStim;
}
