// DISHONORED(written): the one out-of-line piece of the reconstructed PhysX headers. NxArray's
// default allocator routes through the SDK allocator the loader hands out - which is exactly why the
// retail exe imports PhysXLoader.dll!NxGetPhysicsSDKAllocator (imports_2013.csv) although nothing in
// the engine calls it by name. Before the SDK exists (NxGetPhysicsSDKAllocator returns NULL) it falls
// back to the CRT heap, as the SDK's own NxAllocatorDefault does.

#include "NxArray.h"
#include "NxUserAllocator.h"
#include <stdlib.h>

void* NxAllocatorDefaultMalloc(size_t size)
{
	NxUserAllocator* allocator = NxGetPhysicsSDKAllocator();
	return allocator ? allocator->malloc(size) : ::malloc(size);
}

void NxAllocatorDefaultFree(void* mem)
{
	NxUserAllocator* allocator = NxGetPhysicsSDKAllocator();
	if (allocator)
	{
		allocator->free(mem);
	}
	else
	{
		::free(mem);
	}
}
