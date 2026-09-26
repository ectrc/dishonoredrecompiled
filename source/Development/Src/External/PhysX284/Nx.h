/*=============================================================================
	Nx.h: part of the PhysX 2.8.4 API reconstructed for Dishonored.

	DISHONORED(written): the macros the reconstructed headers need. Calling conventions come from the
	shipped binaries: the ten PhysXLoader.dll exports are undecorated names (dumpbin /exports, i.e.
	extern "C" __cdecl) and every interface method in the PDBs is a plain __thiscall member.
=============================================================================*/

#ifndef NX_H
#define NX_H

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include "NxSimpleTypes.h"

#define NX_CALL_CONV				__cdecl
#define NX_C_EXPORT					extern "C"
#define NX_INLINE					inline
#define NX_FORCE_INLINE				__forceinline
#define NX_COMPILE_TIME_ASSERT(e)	typedef char NxCompileTimeAssert_[(e) ? 1 : -1]
#define NX_UNUSED(x)				((void)(x))

#ifndef NULL
#define NULL 0
#endif

#define NX_ASSERT(x)				((void)0)
#define NX_ALWAYS_ASSERT()			((void)0)
#define NX_DELETE(x)				delete (x)
#define NX_DELETE_SINGLE(x)			{ delete (x); (x) = 0; }
#define NX_DELETE_ARRAY(x)			{ delete [] (x); (x) = 0; }
#define NX_ARRAY_SIZE(a)			(sizeof(a) / sizeof((a)[0]))

// DISHONORED(written): the SDK's default sleep counter, the default argument of wakeUp() and of
// NxBodyDesc::wakeUpCounter - and NxBodyDesc::setToDefault in the shipped PhysXExtensions.dll
// (rva 0x1540) stores 0.4f into wakeUpCounter @88, i.e. 20.0f * 0.02f.
#define NX_SLEEP_INTERVAL			(20.0f * 0.02f)

// DISHONORED(layout): PhysX 2.8.4 has no NxSoftBodyAttachmentFlag in the shipped PDBs (the DLLs
// never name it), but Engine/Src/UnPhysSkelComponent.cpp uses its two values; they mirror
// NxClothAttachmentFlag, which the PDB does have (NX_CLOTH_ATTACHMENT_TWOWAY 1, _TEARABLE 2).
enum NxSoftBodyAttachmentFlag
{
	NX_SOFTBODY_ATTACHMENT_TWOWAY = 1,
	NX_SOFTBODY_ATTACHMENT_TEARABLE = 2
};

// DISHONORED(retail): the shipped SDK is the DLL build (PhysXLoader.dll resolves PhysXCore.dll and
// PhysXCooking.dll), so the per-library API macros are empty: the ten loader entry points arrive
// through our own import library (cmake/PhysX.cmake) and everything else through the vtables.
#define NXPHYSICSSDK_API
#define NXFOUNDATION_API
#define NXCOOKING_API
#define NXCHARACTER_API
#define NXEXTENSIONS_API
#define NXPHYSXLOADER_API

#endif // NX_H
