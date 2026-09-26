// DISHONORED(retail): source of the PhysXLoader.dll import library (cmake/PhysX.cmake,
// DISHONORED_WITH_PHYSX). The retail 2013 exe imports six of the DLL's ten exports by their
// undecorated C names (resources/docs/symbols/imports_2013.csv):
//   NxCreatePhysicsSDK, NxGetCookingLib, NxGetPhysicsSDK, NxGetPhysicsSDKAllocator,
//   NxGetUtilLib, NxReleasePhysicsSDK
// and the shipped DLL exports ten (dumpbin /exports, build/agentU/physxloader_exports.txt):
//    1 NxCreateModule            2 NxCreatePhysicsSDK       3 NxCreatePhysicsSDKWithID
//    4 NxGetCookingLib           5 NxGetCookingLibWithID    6 NxGetFoundationSDK
//    7 NxGetPhysicsSDK           8 NxGetPhysicsSDKAllocator 9 NxGetUtilLib
//   10 NxReleasePhysicsSDK
// This file is linked into a throw-away build/<dir>/physx/stub/PhysXLoader.dll purely for its import
// library, exactly the way cmake/Bink.cmake does it for binkw32.dll: our exe then imports
// PhysXLoader.dll!NxCreatePhysicsSDK like retail, and the retail DLL in
// DISHONORED_RETAIL_DIR/Binaries/Win32 is the runtime. The stub DLL is never copied next to the exe.
//
// The export names are undecorated, so these are extern "C" __cdecl; only the names matter to the
// import library, not the argument types (the real signatures live in NxPhysics.h / NxCooking.h).

#define PHYSX_STUB extern "C" __declspec(dllexport)

PHYSX_STUB void* __cdecl NxCreateModule(unsigned int, const char*) { return 0; }
PHYSX_STUB void* __cdecl NxCreatePhysicsSDK(unsigned int, void*, void*, const void*, int*) { return 0; }
PHYSX_STUB void* __cdecl NxCreatePhysicsSDKWithID(unsigned int, unsigned int, void*, void*, const void*, int*) { return 0; }
PHYSX_STUB void* __cdecl NxGetCookingLib(unsigned int) { return 0; }
PHYSX_STUB void* __cdecl NxGetCookingLibWithID(unsigned int, unsigned int) { return 0; }
PHYSX_STUB void* __cdecl NxGetFoundationSDK() { return 0; }
PHYSX_STUB void* __cdecl NxGetPhysicsSDK() { return 0; }
PHYSX_STUB void* __cdecl NxGetPhysicsSDKAllocator() { return 0; }
PHYSX_STUB void* __cdecl NxGetUtilLib() { return 0; }
PHYSX_STUB void __cdecl NxReleasePhysicsSDK(void*) {}
