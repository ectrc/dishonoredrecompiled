/*=============================================================================
	UnNovodexLibs.cpp: Novodex library imports
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

#if WITH_NOVODEX

// Novodex library imports

//#define	USE_PROFILE_NOVODEX

#include "UnNovodexSupport.h"

#if defined(XBOX)

#if (defined _DEBUG) && (defined USE_DEBUG_NOVODEX)
//	#pragma message("Linking Xenon DEBUG Novodex Libs")
	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCookingDEBUG.lib")
	#endif
	#pragma comment(lib, "PhysXCoreDEBUG.lib")
	#pragma comment(lib, "PhysXLoaderDEBUG.lib")
	#pragma comment(lib, "FoundationDEBUG.lib")
	#pragma comment(lib, "FrameworkDEBUG.lib")
	#pragma comment(lib, "OpcodeDEBUG.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensionsDEBUG.lib")
	#endif
#else
//	#pragma message("Linking Xenon RELEASE Novodex Libs")
	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCooking.lib")
	#endif
	#pragma comment(lib, "PhysXCore.lib")
	#pragma comment(lib, "PhysXLoader.lib")
	#pragma comment(lib, "Foundation.lib")
	#pragma comment(lib, "Framework.lib")
	#pragma comment(lib, "Opcode.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensions.lib")
	#endif
#endif

#elif defined(_WIN64) // WIN64

#if (defined _DEBUG) && (defined USE_DEBUG_NOVODEX)
	#pragma message("Linking Win64 DEBUG Novodex Libs")

	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCooking64DEBUG.lib")
	#endif
	#pragma comment(lib, "PhysXLoader64DEBUG.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensions64DEBUG.lib")
	#endif
	#if SUPPORT_DOUBLE_BUFFERING
		#pragma comment(lib, "libnxdoublebuffered_release.lib")
	#endif
#else
	#pragma message("Linking Win64 RELEASE Novodex Libs")

	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCooking64.lib")
	#endif
	#pragma comment(lib, "PhysXLoader64.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensions64.lib")
	#endif
	#if SUPPORT_DOUBLE_BUFFERING
		#pragma comment(lib, "libnxdoublebuffered_release.lib")
	#endif
#endif

	#pragma comment(lib, "DelayImp.lib")

#elif _WINDOWS // Win32

// DISHONORED(retail): cmake/PhysX.cmake puts our own import library for the retail
// PhysXLoader.dll on the link line and nothing else - the retail 2013 exe imports six functions
// from PhysXLoader.dll and nothing at all from PhysXCooking.dll or PhysXExtensions.dll
// (resources/docs/symbols/imports_2013.csv), and libnxdoublebuffered is a static SDK library we
// do not have (SUPPORT_DOUBLE_BUFFERING is 0). So the default-lib directives below, which name
// SDK libraries that do not exist in this tree, are skipped.
#if !DISHONORED_PHYSX_IMPORT_LIB

#if (defined _DEBUG) && (defined USE_DEBUG_NOVODEX)
//	#pragma message("Linking Win32 DEBUG Novodex Libs")
	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCookingDEBUG.lib")
	#endif
	#pragma comment(lib, "PhysXLoaderDEBUG.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensionsDEBUG.lib")
	#endif
	#if SUPPORT_DOUBLE_BUFFERING
		#pragma comment(lib, "libnxdoublebuffered_release.lib")
	#endif
#else
//	#pragma message("Linking Win32 RELEASE Novodex Libs")
	#if WITH_PHYSX_COOKING
		#pragma comment(lib, "PhysXCooking.lib")
	#endif
	#pragma comment(lib, "PhysXLoader.lib")
	#if USE_QUICKLOAD_CONVEX
		#pragma comment(lib, "PhysXExtensions.lib")
	#endif
	#if SUPPORT_DOUBLE_BUFFERING
		#pragma comment(lib, "libnxdoublebuffered_release.lib")
	#endif
#endif

	#pragma comment(lib, "DelayImp.lib")

#endif // !DISHONORED_PHYSX_IMPORT_LIB

#endif	//#if defined(XBOX)

NxPhysicsSDK*			GNovodexSDK = NULL;
// DISHONORED(bringup): set once UWorld::TickWorldRBPhys has logged the scene summary that shows
// the reconstructed PhysX bindings reaching the shipped SDK.
UBOOL					GDishonoredLoggedPhysXSceneSummary = FALSE;
NxExtensionQuickLoad *	GNovodeXQuickLoad = NULL;
#if WITH_PHYSX_COOKING
NxCookingInterface*		GNovodexCooking = NULL;
#endif

TMap<INT, NxScenePair>	GNovodexSceneMap;	// hardware scene support - using NxScenePair
TMap<INT, struct ForceFieldExcludeChannel*> GNovodexForceFieldExcludeChannelsMap;
TArray<NxActor*>		GNovodexPendingKillActor;
TArray<NxJoint*>		GNovodexPendingKillJoint;
TArray<NxConvexMesh*>	GNovodexPendingKillConvex;
TArray<NxTriangleMesh*>	GNovodexPendingKillTriMesh;
TArray<NxHeightField*>	GNovodexPendingKillHeightfield;
TArray<NxCCDSkeleton*>	GNovodexPendingKillCCDSkeletons;
TArray<class UserForceField*>				GNovodexPendingKillForceFields;
TArray<class UserForceFieldLinearKernel*>	GNovodexPendingKillForceFieldLinearKernels;
TArray<class UserForceFieldShapeGroup* >	GNovodexPendingKillForceFieldShapeGroups;
INT						GNumPhysXConvexMeshes = 0;
INT						GNumPhysXTriMeshes = 0;

/** Array of fluid emitters that need deleteing. */
#ifndef NX_DISABLE_FLUIDS
TArray<NxFluidEmitter*>	GNovodexPendingKillFluidEmitters;
#endif

#if !NX_DISABLE_CLOTH
TArray<NxCloth*>		GNovodexPendingKillCloths;
TArray<NxClothMesh*>	GNovodexPendingKillClothMesh;
#endif // !NX_DISABLE_CLOTH

#if !NX_DISABLE_SOFTBODY
TArray<NxSoftBodyMesh*>	GNovodexPendingKillSoftBodyMesh;
#endif // !NX_DISABLE_SOFTBODY

#endif // WITH_NOVODEX
