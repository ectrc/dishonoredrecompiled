/*=============================================================================
	NxExtensions.h: the PhysX 2.8.4 extension API, reconstructed for Dishonored.

	DISHONORED(layout|retail): PhysXExtensions.dll exports exactly two functions, NxCreateExtension
	and NxReleaseExtension (dumpbin /exports on the shipped DLL), and PhysXExtensions.pdb gives
	NxExtension's vtable (getExtensionType vt[0], getExtensionVersion vt[1]) and the NxExtensionType
	values (NX_EXT_SCREEN_SURFACE 0, NX_EXT_PERFORMANCE_IDENTIFIER 1, NX_EXT_QUICK_LOAD 2).
	The retail 2013 exe imports NOTHING from PhysXExtensions.dll (imports_2013.csv), i.e. Arkane
	shipped with the QuickLoad convex path off; cmake/PhysX.cmake therefore compiles with
	USE_QUICKLOAD_CONVEX=0 and builds no import library for this DLL. The declarations exist so the
	reference code that mentions the types still compiles.
=============================================================================*/

#ifndef NX_EXTENSIONS_H
#define NX_EXTENSIONS_H

#include "NxPhysics.h"

// DISHONORED(layout): PhysXExtensions.pdb enum NxExtensionType. It is not in NxGenEnums.h because
// that header is generated from PhysXCore.pdb and PhysXCooking.pdb only.
enum NxExtensionType
{
	NX_EXT_SCREEN_SURFACE = 0,
	NX_EXT_PERFORMANCE_IDENTIFIER = 1,
	NX_EXT_QUICK_LOAD = 2,
	NX_EXT_LAST = 3
};

class NxExtension
{
public:
	virtual NxExtensionType getExtensionType() const = 0;	// vt[0]
	virtual NxU32 getExtensionVersion() const = 0;			// vt[1]
};

NX_C_EXPORT NXEXTENSIONS_API NxExtension* NX_CALL_CONV NxCreateExtension(NxExtensionType type);
NX_C_EXPORT NXEXTENSIONS_API void NX_CALL_CONV NxReleaseExtension(NxExtension& extension);

#endif // NX_EXTENSIONS_H
