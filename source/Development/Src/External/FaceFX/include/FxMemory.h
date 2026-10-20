/*=============================================================================
	FxMemory.h - FaceFX 1.7.3.1 memory system (fxmemory.h).

	DISHONORED(port): agent FX1 (wave 26). Every SDK allocation goes through FxAllocate / FxFree with the
	size of the block (the fixed-size pools need it on free). FxUseAllocator gives SDK classes a class
	operator new/delete on top of them (0x330050 / 0x330060). The construct/destroy helpers are the header
	templates retail instantiates per type (fxmemory.h:132 FxConstruct, :142 FxDefaultConstruct, :148/157
	FxDestroy, :214 FxDelete); they placement-new through the global operator, because FxUseAllocator hides it
	inside SDK classes.
=============================================================================*/
#ifndef FxMemory_H__
#define FxMemory_H__

#include "FxPlatform.h"

namespace OC3Ent
{
namespace Face
{

enum FxMemoryAllocationType
{
	MAT_FreeStore = 0,
	MAT_Heap = 1,
	MAT_Custom = 2
};

struct FxMemoryAllocationPolicy
{
	FxMemoryAllocationPolicy();
	FxMemoryAllocationPolicy(FxMemoryAllocationType inAllocationType, FxBool inbUseMemoryPools,
		void* (*inAllocate)(FxSize), void* (*inAllocateDebug)(FxSize, const FxChar*), void (*inFree)(void*, FxSize));
	FxMemoryAllocationPolicy(const FxMemoryAllocationPolicy& other);
	FxMemoryAllocationPolicy& operator=(const FxMemoryAllocationPolicy& other);
	~FxMemoryAllocationPolicy();

	void* (*userAllocate)(FxSize numBytes);
	void* (*userAllocateDebug)(FxSize numBytes, const FxChar* system);
	void (*userFree)(void* ptr, FxSize n);
	FxMemoryAllocationType allocationType;
	FxBool bUseMemoryPools;
};

struct FxFixedSizeAllocatorInfo
{
	FxSize blockSize;
	FxSize numBlocksPerChunk;
	FxSize chunkSize;
	FxSize numChunks;
	FxSize numEmptyChunks;
	FxSize numFullChunks;
	FxSize numPartialChunks;
	FxSize avgNumBlocksInUsePerChunk;
	FxSize totalSize;
};

void FxMemoryStartup(const FxMemoryAllocationPolicy& allocationPolicy);
void FxMemoryShutdown();
FxMemoryAllocationPolicy FxGetMemoryAllocationPolicy();
void FxTrimMemoryPools();

void* FxAllocate(FxSize numBytes);
void* FxAllocateDebug(FxSize numBytes, const FxChar* system);
void FxFree(void* ptr, FxSize n);

FxSize FxGetCurrentNumAllocations();
FxSize FxGetPeakNumAllocations();
FxSize FxGetMinAllocationSize();
FxSize FxGetMaxAllocationSize();
FxSize FxGetMedianAllocationSize();
FxSize FxGetAvgAllocationSize();
FxSize FxGetCurrentBytesAllocated();
FxSize FxGetPeakBytesAllocated();
FxSize FxGetNumFixedSizeAllocators();
FxFixedSizeAllocatorInfo FxGetFixedSizeAllocatorInfo(FxSize index);
void* FxGetAllocStart();
void* FxGetAllocNext(void* const pAlloc);
void* FxGetAllocPrev(void* const pAlloc);
FxBool FxGetAllocInfo(void* const pAlloc, FxSize& allocNumber, FxSize& allocSize, FxChar*& allocSystem);

class FxUseAllocator
{
public:
	static void* operator new(FxSize size)
	{
		return FxAllocate(size);
	}

	static void operator delete(void* ptr, FxSize size)
	{
		FxFree(ptr, size);
	}
};

template<typename T, typename V> inline void FxConstruct(T* p, const V& value)
{
	::new (static_cast<void*>(p)) T(value);
}

template<typename T> inline void FxDefaultConstruct(T* p)
{
	::new (static_cast<void*>(p)) T();
}

template<typename T> inline void FxDestroy(T* p)
{
	if( p )
	{
		p->~T();
	}
}

template<typename T> inline void FxDestroy(T* first, T* last)
{
	while( first != last )
	{
		first->~T();
		++first;
	}
}

template<typename T> inline void FxDelete(T*& p)
{
	FxDestroy(p);
	FxFree(p, sizeof(T));
	p = NULL;
}

} // namespace Face
} // namespace OC3Ent

#endif
