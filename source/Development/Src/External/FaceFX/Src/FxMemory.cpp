/*=============================================================================
	FxMemory.cpp - FaceFX 1.7.3.1 memory system (fxsdk\src\fxmemory.cpp, compiland FxMemory.obj).

	DISHONORED(port): agent FX1 (wave 26). The allocation policy picks the free store (operator new/delete),
	the CRT heap or the user's functions; with bUseMemoryPools sixteen fixed-size allocators (8..128 bytes
	in steps of 8, chunks of up to 255 blocks of a 4 KB page) serve every request they fit. Retail is built
	without the allocation tracker, so the statistics and the allocation walk return 0 (fxmemory.cpp:827-978).
	The pool chunk entry the PDB names FxMemoryChunkEntry is {FxMemoryChunk chunk; next} (16 bytes).
=============================================================================*/
#include "FxMemory.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb071e0. */
FxMemoryAllocationPolicy::FxMemoryAllocationPolicy()
	: userAllocate(NULL)
	, userAllocateDebug(NULL)
	, userFree(NULL)
	, allocationType(MAT_FreeStore)
	, bUseMemoryPools(FxFalse)
{
}

/** 2012 0xb07220. */
FxMemoryAllocationPolicy::FxMemoryAllocationPolicy(FxMemoryAllocationType inAllocationType, FxBool inbUseMemoryPools,
	void* (*inAllocate)(FxSize), void* (*inAllocateDebug)(FxSize, const FxChar*), void (*inFree)(void*, FxSize))
	: userAllocate(inAllocate)
	, userAllocateDebug(inAllocateDebug)
	, userFree(inFree)
	, allocationType(inAllocationType)
	, bUseMemoryPools(inbUseMemoryPools)
{
}

/** 2012 0xb07260. */
FxMemoryAllocationPolicy::FxMemoryAllocationPolicy(const FxMemoryAllocationPolicy& other)
	: userAllocate(other.userAllocate)
	, userAllocateDebug(other.userAllocateDebug)
	, userFree(other.userFree)
	, allocationType(other.allocationType)
	, bUseMemoryPools(other.bUseMemoryPools)
{
}

/** 2012 0xb072b0. */
FxMemoryAllocationPolicy& FxMemoryAllocationPolicy::operator=(const FxMemoryAllocationPolicy& other)
{
	if( this != &other )
	{
		userAllocate = other.userAllocate;
		userAllocateDebug = other.userAllocateDebug;
		userFree = other.userFree;
		allocationType = other.allocationType;
		bUseMemoryPools = other.bUseMemoryPools;
	}
	return *this;
}

/** 2012 0xb07310. */
FxMemoryAllocationPolicy::~FxMemoryAllocationPolicy()
{
}

/** 2012 0xb07320. */
static void* FxHeapAllocate(FxSize numBytes)
{
	return malloc(numBytes);
}

/** 2012 0xb07340. */
static void* FxHeapAllocateDebug(FxSize numBytes, const FxChar*)
{
	return malloc(numBytes);
}

/** 2012 0xb07360. */
static void FxHeapFree(void* ptr, FxSize)
{
	free(ptr);
}

/** 2012 0xb07380. */
static void* FxFreeStoreAllocate(FxSize numBytes)
{
	return ::operator new(numBytes);
}

/** 2012 0xb073a0. */
static void* FxFreeStoreAllocateDebug(FxSize numBytes, const FxChar*)
{
	return ::operator new(numBytes);
}

/** 2012 0xb073c0. */
static void FxFreeStoreFree(void* ptr, FxSize)
{
	::operator delete(ptr);
}

/** fxmemory.cpp:103. */
static FxMemoryAllocationPolicy gAllocationPolicy;

class FxFixedSizeMemoryAllocator
{
public:
	struct FxMemoryChunk
	{
		void Initialize(FxSize blockSize, FxUInt8 numBlocks);
		void Destroy();
		void Reset(FxSize blockSize, FxUInt8 numBlocks);
		void* Allocate(FxSize blockSize);
		void Free(void* ptr, FxSize blockSize);

		FxUInt8* pMemory;
		FxSize memorySize;
		FxUInt8 firstAvailableBlock;
		FxUInt8 numBlocksAvailable;
	};

	struct FxMemoryChunkEntry
	{
		FxMemoryChunk chunk;
		FxMemoryChunkEntry* pNext;
	};

	FxFixedSizeMemoryAllocator();
	~FxFixedSizeMemoryAllocator();

	void Initialize(FxSize blockSize);
	void Destroy();
	void Trim();
	void* Allocate();
	void Free(void* ptr);
	FxFixedSizeAllocatorInfo GetInfo() const;

	FxSize GetBlockSize() const
	{
		return _blockSize;
	}

private:
	FxMemoryChunkEntry* FindOwnerChunk(void* ptr);

	FxMemoryChunkEntry* _pChunks;
	FxSize _blockSize;
	FxUInt8 _numBlocks;
};

static_assert(sizeof(FxMemoryAllocationPolicy) == 20, "FxMemoryAllocationPolicy: 2012 PDB sizeof 20");
static_assert(sizeof(FxFixedSizeMemoryAllocator) == 12, "FxFixedSizeMemoryAllocator: 2012 PDB sizeof 12");
static_assert(sizeof(FxFixedSizeMemoryAllocator::FxMemoryChunkEntry) == 16, "FxMemoryChunkEntry: allocated as 0x10");

/** 2012 0xb073e0. */
FxFixedSizeMemoryAllocator::FxFixedSizeMemoryAllocator()
	: _pChunks(NULL)
	, _blockSize(0)
	, _numBlocks(0)
{
}

/** 2012 0xb07410. */
FxFixedSizeMemoryAllocator::~FxFixedSizeMemoryAllocator()
{
}

/** 2012 0xb07420. */
void FxFixedSizeMemoryAllocator::Initialize(FxSize blockSize)
{
	_blockSize = blockSize;
	FxSize numBlocks = 4096 / _blockSize;
	if( numBlocks > 255 )
	{
		numBlocks = 255;
	}
	else if( !numBlocks )
	{
		numBlocks = 8 * _blockSize;
	}
	_numBlocks = static_cast<FxUInt8>(numBlocks);
	_pChunks = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocate(sizeof(FxMemoryChunkEntry)));
	_pChunks->pNext = NULL;
	_pChunks->chunk.Initialize(_blockSize, _numBlocks);
}

/** 2012 0xb074b0. */
void FxFixedSizeMemoryAllocator::Destroy()
{
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		pChunkEntry->chunk.Destroy();
		FxMemoryChunkEntry* pTemp = pChunkEntry;
		pChunkEntry = pChunkEntry->pNext;
		gAllocationPolicy.userFree(pTemp, sizeof(FxMemoryChunkEntry));
	}
	_pChunks = NULL;
	_blockSize = 0;
	_numBlocks = 0;
}

/** 2012 0xb07510: frees every empty chunk but the first one found; the list links are left as retail leaves them. */
void FxFixedSizeMemoryAllocator::Trim()
{
	FxBool bFoundFirstEmptyChunk = FxFalse;
	FxMemoryChunkEntry* pChunkEntry = _pChunks;
	while( pChunkEntry )
	{
		if( _numBlocks == pChunkEntry->chunk.numBlocksAvailable )
		{
			if( bFoundFirstEmptyChunk )
			{
				pChunkEntry->chunk.Destroy();
				FxMemoryChunkEntry* pTemp = pChunkEntry;
				pChunkEntry = pChunkEntry->pNext;
				gAllocationPolicy.userFree(pTemp, sizeof(FxMemoryChunkEntry));
			}
			else
			{
				bFoundFirstEmptyChunk = FxTrue;
				pChunkEntry = pChunkEntry->pNext;
			}
		}
		else
		{
			pChunkEntry = pChunkEntry->pNext;
		}
	}
}

/** 2012 0xb07bd0. */
void* FxFixedSizeMemoryAllocator::Allocate()
{
	FxMemoryChunkEntry* pPreviousChunkEntry = NULL;
	for( FxMemoryChunkEntry* pChunkEntry = _pChunks; pChunkEntry; pChunkEntry = pChunkEntry->pNext )
	{
		if( pChunkEntry->chunk.numBlocksAvailable )
		{
			return pChunkEntry->chunk.Allocate(_blockSize);
		}
		pPreviousChunkEntry = pChunkEntry;
	}
	FxMemoryChunkEntry* pNewEntry = static_cast<FxMemoryChunkEntry*>(gAllocationPolicy.userAllocate(sizeof(FxMemoryChunkEntry)));
	pNewEntry->pNext = NULL;
	pPreviousChunkEntry->pNext = pNewEntry;
	pNewEntry->chunk.Initialize(_blockSize, _numBlocks);
	return pNewEntry->chunk.Allocate(_blockSize);
}

/** 2012 0xb07d70. */
void FxFixedSizeMemoryAllocator::Free(void* ptr)
{
	FindOwnerChunk(ptr)->chunk.Free(ptr, _blockSize);
}

/** 2012 0xb07db0. */
FxFixedSizeMemoryAllocator::FxMemoryChunkEntry* FxFixedSizeMemoryAllocator::FindOwnerChunk(void* ptr)
{
	FxUInt8* p = static_cast<FxUInt8*>(ptr);
	for( FxMemoryChunkEntry* pChunkEntry = _pChunks; pChunkEntry; pChunkEntry = pChunkEntry->pNext )
	{
		if( p >= pChunkEntry->chunk.pMemory && p < pChunkEntry->chunk.pMemory + _blockSize * _numBlocks )
		{
			return pChunkEntry;
		}
	}
	return NULL;
}

/** 2012 0xb075a0. */
FxFixedSizeAllocatorInfo FxFixedSizeMemoryAllocator::GetInfo() const
{
	FxFixedSizeAllocatorInfo info;
	info.blockSize = _blockSize;
	info.numBlocksPerChunk = _numBlocks;
	info.chunkSize = _blockSize * _numBlocks;
	info.numChunks = 0;
	info.numEmptyChunks = 0;
	info.numFullChunks = 0;
	info.numPartialChunks = 0;
	info.avgNumBlocksInUsePerChunk = 0;
	info.totalSize = 0;
	for( FxMemoryChunkEntry* pChunkEntry = _pChunks; pChunkEntry; pChunkEntry = pChunkEntry->pNext )
	{
		++info.numChunks;
		if( _numBlocks == pChunkEntry->chunk.numBlocksAvailable )
		{
			++info.numEmptyChunks;
		}
		if( !pChunkEntry->chunk.numBlocksAvailable )
		{
			++info.numFullChunks;
		}
		info.avgNumBlocksInUsePerChunk += _numBlocks - pChunkEntry->chunk.numBlocksAvailable;
	}
	info.numPartialChunks = info.numChunks - info.numEmptyChunks - info.numFullChunks;
	info.avgNumBlocksInUsePerChunk /= info.numChunks;
	info.totalSize = info.chunkSize * info.numChunks;
	return info;
}

/** 2012 0xb076a0. */
void FxFixedSizeMemoryAllocator::FxMemoryChunk::Initialize(FxSize blockSize, FxUInt8 numBlocks)
{
	memorySize = blockSize * numBlocks;
	pMemory = static_cast<FxUInt8*>(gAllocationPolicy.userAllocate(memorySize));
	Reset(blockSize, numBlocks);
}

/** 2012 0xb076f0. */
void FxFixedSizeMemoryAllocator::FxMemoryChunk::Destroy()
{
	gAllocationPolicy.userFree(pMemory, memorySize);
	pMemory = NULL;
	memorySize = 0;
	firstAvailableBlock = 0;
	numBlocksAvailable = 0;
}

/** 2012 0xb07b00: every free block stores the index of the next free one in its first byte. */
void FxFixedSizeMemoryAllocator::FxMemoryChunk::Reset(FxSize blockSize, FxUInt8 numBlocks)
{
	firstAvailableBlock = 0;
	numBlocksAvailable = numBlocks;
	FxUInt8* p = pMemory;
	for( FxUInt8 i = 0; i != numBlocks; p += blockSize )
	{
		*p = ++i;
	}
}

/** 2012 0xb07ca0. */
void* FxFixedSizeMemoryAllocator::FxMemoryChunk::Allocate(FxSize blockSize)
{
	if( !numBlocksAvailable )
	{
		return NULL;
	}
	FxUInt8* pResult = pMemory + blockSize * firstAvailableBlock;
	firstAvailableBlock = *pResult;
	--numBlocksAvailable;
	return pResult;
}

/** 2012 0xb07e20. */
void FxFixedSizeMemoryAllocator::FxMemoryChunk::Free(void* ptr, FxSize blockSize)
{
	FxUInt8* toRelease = static_cast<FxUInt8*>(ptr);
	*toRelease = firstAvailableBlock;
	firstAvailableBlock = static_cast<FxUInt8>((toRelease - pMemory) / blockSize);
	++numBlocksAvailable;
}

/** fxmemory.cpp:433. */
static const FxSize FX_NUM_MEMORY_POOLS = 16;
static FxFixedSizeMemoryAllocator gMemoryPool[FX_NUM_MEMORY_POOLS];

/** 2012 0xb07740. */
void FxMemoryStartup(const FxMemoryAllocationPolicy& allocationPolicy)
{
	gAllocationPolicy = allocationPolicy;
	if( gAllocationPolicy.allocationType == MAT_FreeStore )
	{
		gAllocationPolicy.userAllocate = FxFreeStoreAllocate;
		gAllocationPolicy.userAllocateDebug = FxFreeStoreAllocateDebug;
		gAllocationPolicy.userFree = FxFreeStoreFree;
	}
	else if( gAllocationPolicy.allocationType == MAT_Heap )
	{
		gAllocationPolicy.userAllocate = FxHeapAllocate;
		gAllocationPolicy.userAllocateDebug = FxHeapAllocateDebug;
		gAllocationPolicy.userFree = FxHeapFree;
	}
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_MEMORY_POOLS; ++i )
		{
			gMemoryPool[i].Initialize(8 * (i + 1));
		}
	}
}

/** 2012 0xb07880. */
void FxMemoryShutdown()
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_MEMORY_POOLS; ++i )
		{
			gMemoryPool[i].Destroy();
		}
	}
}

/** 2012 0xb078c0. */
FxMemoryAllocationPolicy FxGetMemoryAllocationPolicy()
{
	return FxMemoryAllocationPolicy(gAllocationPolicy);
}

/** 2012 0xb078f0. */
void FxTrimMemoryPools()
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_MEMORY_POOLS; ++i )
		{
			gMemoryPool[i].Trim();
		}
	}
}

/** 2012 0xb07b60. */
static void* FxPoolAllocate(FxSize numBytes)
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		for( FxSize i = 0; i < FX_NUM_MEMORY_POOLS; ++i )
		{
			if( gMemoryPool[i].GetBlockSize() >= numBytes )
			{
				return gMemoryPool[i].Allocate();
			}
		}
	}
	return gAllocationPolicy.userAllocate(numBytes);
}

/** 2012 0xb07cf0. */
static void FxPoolFree(void* ptr, FxSize n)
{
	if( ptr )
	{
		if( gAllocationPolicy.bUseMemoryPools )
		{
			for( FxSize i = 0; i < FX_NUM_MEMORY_POOLS; ++i )
			{
				if( gMemoryPool[i].GetBlockSize() >= n )
				{
					gMemoryPool[i].Free(ptr);
					return;
				}
			}
		}
		gAllocationPolicy.userFree(ptr, n);
	}
}

/** 2012 0xb07930. */
void* FxAllocateDebug(FxSize numBytes, const FxChar*)
{
	return FxPoolAllocate(numBytes);
}

/** 2012 0xb07950. */
void* FxAllocate(FxSize numBytes)
{
	return FxPoolAllocate(numBytes);
}

/** 2012 0xb07970. */
void FxFree(void* ptr, FxSize n)
{
	FxPoolFree(ptr, n);
}

/** 2012 0xb07990. */
FxSize FxGetCurrentNumAllocations()
{
	return 0;
}

/** 2012 0xb079a0. */
FxSize FxGetPeakNumAllocations()
{
	return 0;
}

/** 2012 0xb079b0. */
FxSize FxGetMinAllocationSize()
{
	return 0;
}

/** 2012 0xb079c0. */
FxSize FxGetMaxAllocationSize()
{
	return 0;
}

/** 2012 0xb079d0. */
FxSize FxGetMedianAllocationSize()
{
	return 0;
}

/** 2012 0xb079e0. */
FxSize FxGetAvgAllocationSize()
{
	return 0;
}

/** 2012 0xb079f0. */
FxSize FxGetCurrentBytesAllocated()
{
	return 0;
}

/** 2012 0xb07a00. */
FxSize FxGetPeakBytesAllocated()
{
	return 0;
}

/** 2012 0xb07a10. */
FxSize FxGetNumFixedSizeAllocators()
{
	return gAllocationPolicy.bUseMemoryPools ? FX_NUM_MEMORY_POOLS : 0;
}

/** 2012 0xb07a30. */
FxFixedSizeAllocatorInfo FxGetFixedSizeAllocatorInfo(FxSize index)
{
	if( gAllocationPolicy.bUseMemoryPools )
	{
		return gMemoryPool[index].GetInfo();
	}
	FxFixedSizeAllocatorInfo info;
	memset(&info, 0, sizeof(info));
	return info;
}

/** 2012 0xb07ac0. */
void* FxGetAllocStart()
{
	return NULL;
}

/** 2012 0xb07ad0. */
void* FxGetAllocNext(void* const)
{
	return NULL;
}

/** 2012 0xb07ae0. */
void* FxGetAllocPrev(void* const)
{
	return NULL;
}

/** 2012 0xb07af0. */
FxBool FxGetAllocInfo(void* const, FxSize&, FxSize&, FxChar*&)
{
	return FxFalse;
}

} // namespace Face
} // namespace OC3Ent
