// GFxUI/src/gfxuiallocator.cpp
// DISHONORED(port): the bodies of the allocator seam declared in Inc/gfxuiallocator.h.
// FGFxAllocator::Alloc 2013 0x574d90 (2012 0x5b98c0), Free 2013 0x574dc0 (2012 0x5b98f0).
// Retail routes both through the engine's appMalloc; here they go through the CRT so the seam links
// on its own, and they keep the two counters the retail layout has (m_TotalAlloc @8,
// m_FrameAllocPeak @12).
#include "gfxuiallocator.h"

#include <stdlib.h>

void FGFxAllocator::GetInfo(GSysAllocPaged::Info* Info) const
{
    if (Info == 0) return;
    // A paged allocator describes its granularity to GMemoryHeap; 64 KiB is what the Win32
    // allocator reports in retail.
    Info->MinAlign = 16;
    Info->MaxAlign = 16;
    Info->Granularity = 64 * 1024;
    Info->SysDirectThreshold = 0;
    Info->MaxHeapGranularity = 0;
    Info->HasRealloc = false;
}

void* FGFxAllocator::Alloc(unsigned int Size, unsigned int Align)
{
    void* p = Align > 16 ? _aligned_malloc(Size, Align) : malloc(Size);
    if (p)
    {
        m_TotalAlloc += Size;
        if (m_TotalAlloc > m_FrameAllocPeak) m_FrameAllocPeak = m_TotalAlloc;
    }
    return p;
}

bool FGFxAllocator::Free(void* Ptr, unsigned int Size, unsigned int Align)
{
    if (Ptr == 0) return true;
    if (Align > 16) _aligned_free(Ptr);
    else free(Ptr);
    m_TotalAlloc = Size <= m_TotalAlloc ? m_TotalAlloc - Size : 0;
    return true;
}

unsigned int FGFxAllocator::GetFootprint() const { return m_FrameAllocPeak; }
unsigned int FGFxAllocator::GetUsedSpace() const { return m_TotalAlloc; }
