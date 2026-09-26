/*=============================================================================
	NxPhysics.h: the PhysX 2.8.4 public entry points, reconstructed for Dishonored.

	DISHONORED(retail): the ten PhysXLoader.dll exports are undecorated C names (dumpbin /exports on
	the shipped DLL); the retail 2013 exe imports six of them - NxCreatePhysicsSDK, NxGetCookingLib,
	NxGetPhysicsSDK, NxGetPhysicsSDKAllocator, NxGetUtilLib, NxReleasePhysicsSDK
	(resources/docs/symbols/imports_2013.csv) - and reaches everything else through the vtables in
	NxGenerated.h. InitGameRBPhys (2013 rva 0x3d5710) calls
	NxCreatePhysicsSDK(0x02080400, allocator, outputStream, desc), i.e. the four-argument form.
=============================================================================*/

#ifndef NX_PHYSICS_H
#define NX_PHYSICS_H

#include "NxVersionNumber.h"
#include "NxGenerated.h"
#include "NxActorDesc.h"

// DISHONORED(layout): PDB NxContactStreamIterator, sizeof 60, the member offsets below. The stream
// format itself is PhysX-internal (only NxContactStreamIterator::goNextPoint is compiled into
// PhysXCore.dll, rva 0x1a50; the constructor and the rest are inline in the SDK header and are not
// in any shipped binary).
// DISHONORED(bringup): the iterator reports an empty stream, so FNxContactReport::onContactNotify
// (Engine/Src/UnNovodexSupport.cpp) sees no contact points. Static and dynamic collision do not go
// through it; per-contact gameplay feedback (impact sounds, contact damage) does. Reconstructing the
// stream layout is a follow-up - see resources/docs/agents/agentAL.md.
class NxContactStreamIterator
{
public:
	NxU32 numPairs;					// @0
	NxShape* shapes[2];				// @4
	NxU16 shapeFlags;				// @12
	NxU16 numPatches;				// @14
	const NxVec3* patchNormal;		// @16
	NxU32 numPoints;				// @20
	const NxVec3* point;			// @24
	NxF32 separation;				// @28
	NxU32 featureIndex0;			// @32
	NxU32 featureIndex1;			// @36
	NxU32 numPairsRemaining;		// @40
	NxU32 numPatchesRemaining;		// @44
	NxU32 numPointsRemaining;		// @48
	const NxF32* pointNormalForce;	// @52
	const NxU32* stream;			// @56

	NX_INLINE NxContactStreamIterator(const NxU32* contactStream)
	{
		memset(this, 0, sizeof(*this));
		stream = contactStream;
	}
	NX_INLINE bool goNextPair() { return false; }
	NX_INLINE bool goNextPatch() { return false; }
	NX_INLINE bool goNextPoint() { return false; }
	NX_INLINE NxShape* getShape(NxU32 index) const { return shapes[index]; }
	NX_INLINE NxU32 getNumPairs() const { return 0; }
	NX_INLINE NxU32 getNumPatches() const { return 0; }
	NX_INLINE NxU32 getNumPoints() const { return 0; }
	NX_INLINE const NxVec3& getPatchNormal() const { return *patchNormal; }
	NX_INLINE const NxVec3& getPoint() const { return *point; }
	NX_INLINE NxReal getSeparation() const { return separation; }
	NX_INLINE NxReal getPointNormalForce() const { return 0.0f; }
	NX_INLINE NxU32 getFeatureIndex0() const { return featureIndex0; }
	NX_INLINE NxU32 getFeatureIndex1() const { return featureIndex1; }
	NX_INLINE bool isDeletedShape(NxU32 index) const { NX_UNUSED(index); return false; }
};

NX_C_EXPORT NXPHYSXLOADER_API NxPhysicsSDK* NX_CALL_CONV NxCreatePhysicsSDK(
	NxU32 sdkVersion,
	NxUserAllocator* allocator,
	NxUserOutputStream* outputStream,
	const NxPhysicsSDKDesc& desc,
	NxSDKCreateError* errorCode);

NX_C_EXPORT NXPHYSXLOADER_API NxPhysicsSDK* NX_CALL_CONV NxCreatePhysicsSDKWithID(
	NxU32 sdkVersion,
	NxU32 sdkID,
	NxUserAllocator* allocator,
	NxUserOutputStream* outputStream,
	const NxPhysicsSDKDesc& desc,
	NxSDKCreateError* errorCode);

NX_C_EXPORT NXPHYSXLOADER_API void NX_CALL_CONV NxReleasePhysicsSDK(NxPhysicsSDK* sdk);
NX_C_EXPORT NXPHYSXLOADER_API NxPhysicsSDK* NX_CALL_CONV NxGetPhysicsSDK();
NX_C_EXPORT NXPHYSXLOADER_API NxUtilLib* NX_CALL_CONV NxGetUtilLib();
NX_C_EXPORT NXPHYSXLOADER_API NxFoundationSDK* NX_CALL_CONV NxGetFoundationSDK();
NX_C_EXPORT NXPHYSXLOADER_API void* NX_CALL_CONV NxCreateModule(NxU32 sdkVersion, const char* moduleName);

// DISHONORED(written): the four-argument and three-argument spellings the engine uses
// (Engine/Src/UnPhysLevel.cpp); the SDK header gives these parameters default values.
NX_INLINE NxPhysicsSDK* NxCreatePhysicsSDK(NxU32 sdkVersion, NxUserAllocator* allocator,
	NxUserOutputStream* outputStream, const NxPhysicsSDKDesc& desc)
{
	return NxCreatePhysicsSDK(sdkVersion, allocator, outputStream, desc, 0);
}
NX_INLINE NxPhysicsSDK* NxCreatePhysicsSDK(NxU32 sdkVersion, NxUserAllocator* allocator,
	NxUserOutputStream* outputStream)
{
	return NxCreatePhysicsSDK(sdkVersion, allocator, outputStream, NxPhysicsSDKDesc(), 0);
}
NX_INLINE NxPhysicsSDK* NxCreatePhysicsSDK(NxU32 sdkVersion)
{
	return NxCreatePhysicsSDK(sdkVersion, 0, 0, NxPhysicsSDKDesc(), 0);
}

#endif // NX_PHYSICS_H
