// Scaleform GFx 3.3.89 - the few kernel bodies the reconstructed headers need to be a library
// rather than a pile of declarations: GArray's growth, GLock over a Win32 critical section, and the
// one non-pure virtual of GFxLogBase.
//
// Everything else in this directory is either header-inline (GFxValue, the geometry) or pure
// virtual (the runtime's own interfaces). DISHONORED(written): resources/docs/agents/agentBB.md.
#include "GTypes.h"
#include "GFx3Gen.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

// DISHONORED(bringup, agent DC): the log hook. See the note at GFxLogHook in GTypes.h - this directory
// has no engine header, so a host that wants the AS2 machine's diagnostics installs a hook.
GFxLogHookFn GFxLogHook = 0;

void GFxLogf(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (GFxLogHook)
        GFxLogHook(buf);
    else
        printf("%s\n", buf);
}

// The Windows headers insist on the default 8-byte packing, and the engine compiles every unit with
// UE3's /Zp4 (cmake/DishonoredDefines.cmake), so they go inside a pack(8) window - the same thing
// WinDrv's PreWindowsApi.h / PostWindowsApi.h do for the rest of the tree.
#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

// GArray's allocator. The runtime routes this through GMemory/GMemoryHeap; until the allocator seam
// is live (FGFxAllocator, GFxUI/Inc/gfxuiallocator.h) the CRT heap is the right bring-up answer,
// because nothing in the reconstruction hands one of these arrays to retail code.
void* GFx3ArrayAlloc(unsigned int bytes, int statId)
{
    (void)statId;
    return bytes ? malloc(bytes) : 0;
}

void GFx3ArrayFree(void* p)
{
    if (p) free(p);
}

// GLock: PDB layout is a _RTL_CRITICAL_SECTION by value at @0, sizeof 24. csStorage is that
// critical section; the static assertion keeps the two in step.
static_assert(sizeof(CRITICAL_SECTION) <= sizeof(void*) * 6,
              "GLock::csStorage is too small for a CRITICAL_SECTION");

GLock::GLock()
{
    InitializeCriticalSection((CRITICAL_SECTION*)csStorage);
}

GLock::~GLock()
{
    DeleteCriticalSection((CRITICAL_SECTION*)csStorage);
}

void GLock::DoLock()
{
    EnterCriticalSection((CRITICAL_SECTION*)csStorage);
}

void GLock::Unlock()
{
    LeaveCriticalSection((CRITICAL_SECTION*)csStorage);
}

template<>
bool GFxLogBase<GFxLog>::IsVerboseActionErrors() const
{
    return false;
}

// DISHONORED(port, agent FA): 2013 0x9b08c0 GRenderer::MakeViewAndPersp3D, the one place a stage
// rectangle and a field of view become the pair of matrices the 3D display path projects through.
// The eye sits one focal length in front of the middle of the rectangle looking at it, so the pair
// maps the rectangle onto clip space exactly as the 2D viewport matrix does - which is what lets a
// clip with an identity 3D matrix draw in the same pixels it drew in before.
void GRenderer::MakeViewAndPersp3D(const GRect<float>& FrameRect, GMatrix3D& View, GMatrix3D& Persp,
                                   float FovYDegrees, bool bInvertY)
{
    const float Height = (float)fabs(FrameRect.Bottom - FrameRect.Top);
    const float Width = (float)fabs(FrameRect.Right - FrameRect.Left);
    const float CenterX = (FrameRect.Right + FrameRect.Left) * 0.5f;
    const float CenterY = (FrameRect.Bottom + FrameRect.Top) * 0.5f;
    const float Focal = (Width * 0.5f) / (float)tan(0.5 * (double)FovYDegrees
                                                    * 3.14159265358979323846 / 180.0);
    float EyeZ = -Focal;
    if (EyeZ < -100000.f)
        EyeZ = -100000.f;
    const GPoint3<float> Eye(CenterX, CenterY, EyeZ);
    const GPoint3<float> At(CenterX, CenterY, 0.f);
    const GPoint3<float> Up(0.f, bInvertY ? 1.f : -1.f, 0.f);
    if (bInvertY)
    {
        View.ViewLH(Eye, At, Up);
        Persp.PerspectiveFocalLengthLH(Focal, Width, Height, 1.f, 100000.f);
    }
    else
    {
        View.ViewRH(Eye, At, Up);
        Persp.PerspectiveFocalLengthRH(Focal, Width, Height, 1.f, 100000.f);
    }
}
