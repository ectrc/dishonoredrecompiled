/*=============================================================================
	NxExtensionQuickLoad.h: the PhysX 2.8.4 QuickLoad extension, reconstructed for Dishonored.

	DISHONORED(layout): PhysXExtensions.pdb NxExtensionQuickLoad - base NxExtension,
	~NxExtensionQuickLoad at vt[2], wrapAllocator vt[3], initialize vt[4], cookConvexMesh vt[5],
	createConvexMesh vt[6], releaseConvexMesh vt[7]; enum NxExtensionQuickLoad::Mode =
	AllowSDKToReleaseQLMesh 0, RemoveSDKReferencesToQLMesh 1, RequireUserReleaseOfQLMesh 2.
	Retail does not use it (no PhysXExtensions.dll import), so USE_QUICKLOAD_CONVEX is 0 here.
=============================================================================*/

#ifndef NX_EXTENSION_QUICK_LOAD_H
#define NX_EXTENSION_QUICK_LOAD_H

#include "NxExtensions.h"

class NxExtensionQuickLoad : public NxExtension
{
public:
	enum Mode
	{
		AllowSDKToReleaseQLMesh = 0,
		RemoveSDKReferencesToQLMesh = 1,
		RequireUserReleaseOfQLMesh = 2
	};

	virtual ~NxExtensionQuickLoad() {}									// vt[2]
	virtual NxUserAllocator* wrapAllocator(NxUserAllocator& allocator, Mode mode, NxU32 pageSize) = 0;	// vt[3]
	virtual bool initialize(NxPhysicsSDK& sdk, NxU32 flags) = 0;		// vt[4]
	virtual bool cookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream, NxCookingInterface& cooker) = 0;	// vt[5]
	virtual NxConvexMesh* createConvexMesh(NxStream& stream) = 0;		// vt[6]
	virtual void releaseConvexMesh(NxConvexMesh& mesh) = 0;				// vt[7]
};

#endif // NX_EXTENSION_QUICK_LOAD_H
