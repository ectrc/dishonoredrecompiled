cmake_minimum_required(VERSION 3.25)
project(Dishonored LANGUAGES CXX)

if(NOT MSVC)
  message(FATAL_ERROR "Dishonored builds with MSVC only")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Configure inside a VS x86 environment (vcvars32); pointer size must be 4")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_compile_options(/W4 /permissive- /Zc:__cplusplus /fp:precise /EHsc /utf-8 /MP)
# /Zp4 (4-byte struct packing, UnrealBuildTool VCToolChain.cs) is applied per engine target in
# dishonored_apply_defines(); globally it breaks third-party code (libpng) and the Windows SDK asserts.
# WIN32_LEAN_AND_MEAN / NOMINMAX are set by WinDrv/Inc/MinWindows.h; defining them here too
# would only produce C4005 redefinition warnings in every unit.
add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)

include(cmake/Dependencies.cmake)
include(cmake/ReferenceExternals.cmake)
# Declared before cmake/Wwise.cmake, which needs to know whether the AkAudio module is in the build: the
# Wwise glue (the low-level IO hook, UAkAudioDevice, the AK allocation hooks) lives in that module, and
# Engine's akbank.cpp / akevent.cpp / UnActor.cpp call into it. The module section below uses this value.
option(DISHONORED_ENABLE_AKAUDIO "Build the AkAudio module (retail classes plus the Wwise 2012.1 glue when DISHONORED_WITH_WWISE)" OFF)
include(cmake/Wwise.cmake)
# DISHONORED(bringup): UE3 overrides the global operator new/delete (Core/Inc/UnFile.h) but not C++17's aligned overloads.
# Over-aligned render types (the GPU-skin vertex factories hold MS_ALIGN(16) members) are placement-newed into the
# engine heap and then deleted, so with /Zc:alignedNew the CRT aligned free receives an appMalloc pointer and the
# process aborts during skeletal mesh cleanup (agent AF's B4 hang, root-caused by agent AO). MSVC 2010, which this
# code targets, has no aligned new, so turning it off restores the intended behaviour.
add_compile_options(/Zc:alignedNew-)

include(cmake/DishonoredDefines.cmake)
include(cmake/DishonoredModule.cmake)

set(DISHONORED_BINARIES_DIR "${CMAKE_BINARY_DIR}/Binaries/Win32")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DISHONORED_BINARIES_DIR}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DISHONORED_MODULE_LIST "" CACHE INTERNAL "modules declared so far, in order" FORCE)

# Engine modules in dependency order (PLAN.md Phase 3). Each one is added when it compiles;
# modules still being brought up sit behind an option so the default configure stays green.
dishonored_module(Core)
# dbghelp: stack walker (UnVcWin32.cpp); winmm: timeBeginPeriod. Core.lib needs them in every exe.
target_link_libraries(Core PUBLIC Dishonored::zlib Dishonored::lzokay Dishonored::DirectX9 Dishonored::libPNG dbghelp winmm)

option(DISHONORED_ENABLE_ENGINE "Build the Engine module" ON)
if(DISHONORED_ENABLE_ENGINE)
  dishonored_module(Engine)
  # nvtt (UnTexCompress.cpp DXT compression) is editor-only since agent P (WITH_EDITOR guards in UnTexCompress.cpp,
  # UnLightMap.cpp, UnShadowMap.cpp): the retail exe imports no nvtt.dll. Dishonored::nvtt stays declared in
  # cmake/ReferenceExternals.cmake for a future editor build.
  target_link_libraries(Engine PUBLIC Dishonored::nvapi)
endif()

# Milestone 1 modules (agent N): the real Launch needs their registrants and the Core hooks they
# own (WinSock init, game window globals, hardware survey). Each is default ON since it compiles.
option(DISHONORED_ENABLE_GAMEFRAMEWORK "Build the GameFramework module" ON)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_GAMEFRAMEWORK)
  dishonored_module(GameFramework)
endif()
option(DISHONORED_ENABLE_IPDRV "Build the IpDrv module (WinSock, UDP/TCP net driver, debug server)" ON)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_IPDRV)
  dishonored_module(IpDrv)
  # UnSocketWin.cpp / IpDrv.cpp: WinSock 2 and the WinInet HTTP downloader
  target_link_libraries(IpDrv PUBLIC ws2_32 wininet)
endif()
option(DISHONORED_ENABLE_WINDRV "Build the WinDrv module (Windows client, viewport, DirectInput/XInput)" ON)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_WINDRV)
  dishonored_module(WinDrv)
  # WinClient.cpp: DirectInput 8 joystick enumeration; XnaForceFeedbackManager.cpp: XInput
  target_link_libraries(WinDrv PUBLIC dinput8 dxguid xinput)
  # WintabSupport.h needs Development/External/wintab (tablet pressure, editor only), absent from the reference tree
  target_compile_definitions(WinDrv PRIVATE WITH_WINTAB=0)
endif()
# D3D9Drv (wave 2 P): the retail renderer (module_map.md: d3d9drv; retail imports d3d9). Launch links
# the real D3D9CreateRHI / PIX markers / shader compiler entry points instead of the null-RHI stubs.
option(DISHONORED_ENABLE_D3D9DRV "Build the D3D9Drv module (Direct3D 9 RHI, hardware survey, compatibility evaluator)" ON)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_D3D9DRV)
  dishonored_module(D3D9Drv)
  # d3d9 + d3dx9 (device; D3DX shader compiler / mesh utils / D3DXCheckVersion, editor-time paths the retail exe does
  # not import: imports_2013.csv lists d3d9 only) and nvapi (ue3stereo.h stereo updater)
  target_link_libraries(D3D9Drv PUBLIC Dishonored::DirectX9 Dishonored::nvapi)
endif()

# Retail script-package modules (wave 2 T, resources/docs/agents/agentT.md): class declarations and
# registrants generated from the retail SDK dump (gen_classes_header.py --sdk) so UClass::Bind finds a
# constructor for every native class export of GFxUI.upk, AkAudio.upk, OnlineSubsystemSteamworks.upk
# and DishonoredGame.upk. Native function bodies are appErrorf stubs until ported. Default OFF until
# the coordinator merges them into the smoke run; DishonoredGame needs the other three (its classes
# derive from GFxMoviePlayer, AkComponent, OnlineSubsystemCommonImpl).
# <Module>Layouts.h: static_assert of every retail sizeof and reflected member offset of the generated
# classes (independent of the Core/Engine PDB asserts behind DISHONORED_LAYOUT_CHECKS).
option(DISHONORED_SDK_LAYOUT_CHECKS "Compile the retail-SDK layout static_asserts of the generated script-package modules" ON)
option(DISHONORED_ENABLE_GFXUI "Build the GFxUI module (retail class registrants only, no Scaleform glue)" OFF)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_GFXUI)
  dishonored_module(GFxUI)
  target_compile_definitions(GFxUI PRIVATE DISHONORED_SDK_LAYOUT_CHECKS=$<BOOL:${DISHONORED_SDK_LAYOUT_CHECKS}>)
endif()
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_AKAUDIO)
  dishonored_module(AkAudio)
  target_compile_definitions(AkAudio PRIVATE DISHONORED_SDK_LAYOUT_CHECKS=$<BOOL:${DISHONORED_SDK_LAYOUT_CHECKS}>)
endif()
option(DISHONORED_ENABLE_OSS "Build the OnlineSubsystemSteamworks module (retail class registrants only, no Steamworks SDK)" OFF)
if(DISHONORED_ENABLE_ENGINE AND DISHONORED_ENABLE_IPDRV AND DISHONORED_ENABLE_OSS)
  dishonored_module(OnlineSubsystemSteamworks)
  target_compile_definitions(OnlineSubsystemSteamworks PRIVATE DISHONORED_SDK_LAYOUT_CHECKS=$<BOOL:${DISHONORED_SDK_LAYOUT_CHECKS}>)
endif()
option(DISHONORED_ENABLE_DISHONOREDGAME "Build the DishonoredGame module (retail class registrants + Engine/GameFramework shims, native stubs)" OFF)
if(DISHONORED_ENABLE_DISHONOREDGAME)
  foreach(dep IN ITEMS GFxUI AkAudio OnlineSubsystemSteamworks GameFramework IpDrv)
    if(NOT TARGET ${dep})
      message(FATAL_ERROR "DISHONORED_ENABLE_DISHONOREDGAME needs the ${dep} module (DISHONORED_ENABLE_GFXUI/AKAUDIO/OSS/GAMEFRAMEWORK/IPDRV=ON)")
    endif()
  endforeach()
  # the exe (Launch) owns the name "DishonoredGame", so the library target is DishonoredGameModule
  dishonored_module(DishonoredGame TARGET DishonoredGameModule)
  # DishonoredGameRegistrants.cpp: ~1,950 IMPLEMENT_CLASS expansions and the whole-module header set in one unit
  target_compile_options(DishonoredGameModule PRIVATE /bigobj)
  target_compile_definitions(DishonoredGameModule PRIVATE DISHONORED_SDK_LAYOUT_CHECKS=$<BOOL:${DISHONORED_SDK_LAYOUT_CHECKS}>)
endif()

add_subdirectory(source/Development/Src/Launch)

# Tests and probes
if(EXISTS "${CMAKE_SOURCE_DIR}/source/Tests/LayoutProbe/CMakeLists.txt")
  add_subdirectory(source/Tests/LayoutProbe)   # generated by resources/tools/symbols/gen_layout_probe.py
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/source/Tests/CoreSmoke/CMakeLists.txt")
  add_subdirectory(source/Tests/CoreSmoke)
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/source/Tests/EdgeAnimSmoke/CMakeLists.txt")
  add_subdirectory(source/Tests/EdgeAnimSmoke)   # DISHONORED(written): agent W, Edge evaluator vs the retail one
endif()
