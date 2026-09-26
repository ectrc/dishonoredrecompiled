/*=============================================================================
	NxCooking.h: the PhysX 2.8.4 cooking API, reconstructed for Dishonored.

	DISHONORED(retail): the retail exe imports NxGetCookingLib from PhysXLoader.dll and drives the
	cooker only through the returned NxCookingInterface vtable - InitGameRBPhys (2013 rva 0x3d5710)
	does `GNovodexCooking = NxGetCookingLib(0x02080400)` and the cook sites call
	`GNovodexCooking->NxCookTriangleMesh(...)`. It imports nothing from PhysXCooking.dll even though
	that DLL also exports the free functions (NxInitCooking, NxCookTriangleMesh, ...), so in the
	shipped configuration those free functions are header-inline wrappers over NxGetCookingLib, which
	is what they are below. NxCookingInterface's vtable order is in NxGenerated.h, from
	PhysXCooking.pdb: NxSetCookingParams vt[0], NxGetCookingParams vt[1], NxPlatformMismatch vt[2],
	NxInitCooking vt[3], NxCloseCooking vt[4], NxCookTriangleMesh vt[5], NxCookConvexMesh vt[6],
	NxCookClothMesh vt[7], NxCookSoftBodyMesh vt[8], NxCreatePMap vt[9], NxReleasePMap vt[10],
	NxScaleCookedConvexMesh vt[11], NxReportCooking vt[12], ~NxCookingInterface vt[13].
=============================================================================*/

#ifndef NX_COOKING_H
#define NX_COOKING_H

#include "NxPhysics.h"

NX_C_EXPORT NXPHYSXLOADER_API NxCookingInterface* NX_CALL_CONV NxGetCookingLib(NxU32 sdkVersionNumber);
NX_C_EXPORT NXPHYSXLOADER_API NxCookingInterface* NX_CALL_CONV NxGetCookingLibWithID(NxU32 sdkVersionNumber, NxU32 sdkID);

NX_INLINE NxCookingInterface* NxGetCookingInterface()
{
	return NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
}
NX_INLINE bool NxInitCooking(NxUserAllocator* allocator = 0, NxUserOutputStream* outputStream = 0)
{
	return NxGetCookingInterface()->NxInitCooking(allocator, outputStream);
}
NX_INLINE void NxCloseCooking()
{
	NxGetCookingInterface()->NxCloseCooking();
}
NX_INLINE bool NxCookTriangleMesh(const NxTriangleMeshDesc& desc, NxStream& stream)
{
	return NxGetCookingInterface()->NxCookTriangleMesh(desc, stream);
}
NX_INLINE bool NxCookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream)
{
	return NxGetCookingInterface()->NxCookConvexMesh(desc, stream);
}
NX_INLINE bool NxCookClothMesh(const NxClothMeshDesc& desc, NxStream& stream)
{
	return NxGetCookingInterface()->NxCookClothMesh(desc, stream);
}
NX_INLINE bool NxCookSoftBodyMesh(const NxSoftBodyMeshDesc& desc, NxStream& stream)
{
	return NxGetCookingInterface()->NxCookSoftBodyMesh(desc, stream);
}
NX_INLINE bool NxSetCookingParams(const NxCookingParams& params)
{
	return NxGetCookingInterface()->NxSetCookingParams(params);
}
NX_INLINE const NxCookingParams& NxGetCookingParams()
{
	return NxGetCookingInterface()->NxGetCookingParams();
}
NX_INLINE bool NxPlatformMismatch()
{
	return NxGetCookingInterface()->NxPlatformMismatch();
}
NX_INLINE bool NxCreatePMap(NxPMap& pmap, const NxTriangleMesh& mesh, NxU32 density, NxUserOutputStream* out = 0)
{
	return NxGetCookingInterface()->NxCreatePMap(pmap, mesh, density, out);
}
NX_INLINE bool NxReleasePMap(NxPMap& pmap)
{
	return NxGetCookingInterface()->NxReleasePMap(pmap);
}
NX_INLINE bool NxScaleCookedConvexMesh(const NxStream& in, NxF32 scale, NxStream& out)
{
	return NxGetCookingInterface()->NxScaleCookedConvexMesh(in, scale, out);
}

#endif // NX_COOKING_H
