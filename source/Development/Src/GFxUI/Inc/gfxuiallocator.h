#pragma once
// GFxUI/inc/gfxuiallocator.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x5b98c0  public: virtual void * __thiscall FGFxAllocator::Alloc(unsigned int, unsigned int)
//   0x5b98f0  public: virtual bool __thiscall FGFxAllocator::Free(void *, unsigned int, unsigned int)
//   0x5b9910  public: virtual void __thiscall FGFxAllocator::GetInfo(struct GSysAllocPaged::Info *)const

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the allocator seam. GSysAllocPaged is a 15-slot interface and retail's
// FGFxAllocator overrides 5 of them (PDB sizeof 16: the singleton-support base plus
// "UINT m_TotalAlloc" @8 and "UINT m_FrameAllocPeak" @12).
//   FGFxAllocator::Alloc  2013 0x574d90  (2012 0x5b98c0)
//   FGFxAllocator::Free   2013 0x574dc0  (2012 0x5b98f0)
// The bodies go through the engine's appMalloc in retail; here they go through the CRT so the seam
// links on its own, and they keep the two counters retail keeps.
#include "GFx3.h"

class FGFxAllocator : public GSysAllocBase_SingletonSupport<FGFxAllocator, GSysAllocPaged>
{
public:
    unsigned int m_TotalAlloc;       // @8
    unsigned int m_FrameAllocPeak;   // @12

    FGFxAllocator() : m_TotalAlloc(0), m_FrameAllocPeak(0) {}
    virtual ~FGFxAllocator() {}                                                  // vt[0]

    virtual void GetInfo(GSysAllocPaged::Info* Info) const;                       // vt[3]
    virtual void* Alloc(unsigned int Size, unsigned int Align);                   // vt[4] 2013 0x574d90
    virtual bool Free(void* Ptr, unsigned int Size, unsigned int Align);          // vt[5] 2013 0x574dc0
    virtual unsigned int GetFootprint() const;                                    // vt[11]
    virtual unsigned int GetUsedSpace() const;                                    // vt[12]
};
