# PhysX 2.8.4 (NVIDIA / Epic's UE3 build). Retail ships PhysXLoader.dll, PhysXCore.dll,
# PhysXCooking.dll, NxCharacter.dll and PhysXExtensions.dll (all 2.8.4.6, InternalName *_FC6_GPU) and
# imports six undecorated C entry points from PhysXLoader.dll: NxCreatePhysicsSDK, NxGetCookingLib,
# NxGetPhysicsSDK, NxGetPhysicsSDKAllocator, NxGetUtilLib, NxReleasePhysicsSDK
# (resources/docs/symbols/imports_2013.csv). Everything else goes through the SDK vtables.
#
# There is no NVIDIA SDK in this tree and none is needed. source/Development/Src/External/PhysX284
# is our own reconstruction of the 2.8.4 API: every enum value, member offset and vtable slot is read
# out of the PDBs that ship next to the DLLs in Dishonored_Debug2012\Binaries\Win32 (the DLLs are
# byte-identical to the retail 2013 ones, resources/docs/binaries.md), and every descriptor default
# is decoded from the DLLs' own compiled copy of the inline setToDefault(). See
# resources/docs/middleware.md 2.2 and resources/docs/agents/agentAL.md.
#
# With DISHONORED_WITH_PHYSX=ON:
#   * External/PhysX284/PhysXLoader_stub.cpp is linked into a throw-away
#     build/<dir>/physx/stub/PhysXLoader.dll whose import library (build/<dir>/physx/PhysXLoader.lib)
#     makes our exe import PhysXLoader.dll!NxCreatePhysicsSDK by that exact name, the same trick
#     cmake/Bink.cmake uses for binkw32.dll. The stub DLL is never copied next to the exe; the retail
#     DLL in DISHONORED_RETAIL_DIR/Binaries/Win32 is the runtime.
#   * dishonored_apply_defines() sets WITH_NOVODEX=1, WITH_PHYSX_COOKING=1, NX_DISABLE_FLUIDS=1,
#     USE_QUICKLOAD_CONVEX=0 and SUPPORT_DOUBLE_BUFFERING=0 on every target, adds the header
#     directory and links Dishonored::physx.
#
# Off by default only when the reconstruction is missing; it lives in the tree, so ON is the default.

set(DISHONORED_PHYSX_DIR "${CMAKE_SOURCE_DIR}/source/Development/Src/External/PhysX284")
if(EXISTS "${DISHONORED_PHYSX_DIR}/NxPhysics.h")
  set(_dishonored_physx_default ON)
else()
  set(_dishonored_physx_default OFF)
endif()
option(DISHONORED_WITH_PHYSX "Compile the engine's PhysX path (WITH_NOVODEX=1) against the reconstructed PhysX 2.8.4 headers and link the PhysXLoader.dll import library" ${_dishonored_physx_default})

if(DISHONORED_WITH_PHYSX)
  set(physx_loader_dll "${DISHONORED_RETAIL_DIR}/Binaries/Win32/PhysXLoader.dll")
  if(NOT EXISTS "${physx_loader_dll}")
    message(FATAL_ERROR "DISHONORED_WITH_PHYSX: ${physx_loader_dll} not found; set DISHONORED_RETAIL_DIR")
  endif()
  add_library(physxloader_stub SHARED "${DISHONORED_PHYSX_DIR}/PhysXLoader_stub.cpp")
  set_target_properties(physxloader_stub PROPERTIES
    OUTPUT_NAME PhysXLoader
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/physx/stub"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/physx"
    FOLDER "External")

  # The default NxArray allocator (NxAllocatorDefault.cpp) is the only translation unit of the
  # reconstruction; everything else is header-only.
  add_library(physx284 STATIC "${DISHONORED_PHYSX_DIR}/NxAllocatorDefault.cpp")
  target_include_directories(physx284 PUBLIC "${DISHONORED_PHYSX_DIR}")
  target_link_libraries(physx284 PUBLIC physxloader_stub)
  target_compile_options(physx284 PRIVATE /Zp4)
  set_target_properties(physx284 PROPERTIES FOLDER "External")

  add_library(Dishonored::physx INTERFACE IMPORTED)
  target_link_libraries(Dishonored::physx INTERFACE physx284)
  message(STATUS "PhysX: WITH_NOVODEX=1 against the reconstructed 2.8.4 headers (runtime ${physx_loader_dll})")
endif()
