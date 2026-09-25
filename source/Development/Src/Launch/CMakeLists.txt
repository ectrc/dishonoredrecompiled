# DishonoredGame.exe. Until Phase 3 milestone 1 links, the default is a stub entry point
# (DishonoredLaunchStub.cpp); DISHONORED_REAL_LAUNCH=ON compiles the reference Launch module
# (Launch.cpp, LaunchEngineLoop.cpp, LaunchMisc.cpp; PIB.cpp is the _WINDLL "plugin in browser"
# build and is not needed) plus DishonoredStubs.cpp, the empty registrant/name hooks and globals
# for the modules that are not built yet (see resources/docs/agents/agentN.md).
option(DISHONORED_REAL_LAUNCH "Compile the reference Launch sources instead of the stub entry point" OFF)

if(DISHONORED_REAL_LAUNCH)
  if(NOT TARGET Engine)
    message(FATAL_ERROR "DISHONORED_REAL_LAUNCH needs DISHONORED_ENABLE_ENGINE=ON")
  endif()
  add_executable(DishonoredGame WIN32
    Src/Launch.cpp
    Src/LaunchEngineLoop.cpp
    Src/LaunchMisc.cpp
    Src/DishonoredStubs.cpp
    Inc/LaunchApp.h
    Inc/LaunchEngineLoop.h
    Inc/LaunchGames.h
    Inc/LaunchPrivate.h)
  target_include_directories(DishonoredGame PRIVATE Inc Src)
  # Every module target links its predecessors PUBLICly, so the last one pulls in all of them
  set(launch_modules Core Engine)
  # agent T: the retail script-package modules (root CMakeLists.txt) replace their registrant stubs in DishonoredStubs.cpp
  foreach(m IN ITEMS GameFramework IpDrv WinDrv D3D9Drv GFxUI AkAudio OnlineSubsystemSteamworks DishonoredGameModule)
    if(TARGET ${m})
      list(APPEND launch_modules ${m})
    endif()
  endforeach()
  target_link_libraries(DishonoredGame PRIVATE ${launch_modules})
  # DISHONORED_HAVE_D3D9DRV: DishonoredStubs.cpp keeps the null-RHI stand-ins for D3D9CreateRHI & co. only
  # while D3D9Drv is not a target (DISHONORED_ENABLE_D3D9DRV=OFF).
  target_compile_definitions(DishonoredGame PRIVATE
    DISHONORED_HAVE_GAMEFRAMEWORK=$<BOOL:$<TARGET_EXISTS:GameFramework>>
    DISHONORED_HAVE_IPDRV=$<BOOL:$<TARGET_EXISTS:IpDrv>>
    DISHONORED_HAVE_WINDRV=$<BOOL:$<TARGET_EXISTS:WinDrv>>
    DISHONORED_HAVE_D3D9DRV=$<BOOL:$<TARGET_EXISTS:D3D9Drv>>
    DISHONORED_HAVE_GFXUI=$<BOOL:$<TARGET_EXISTS:GFxUI>>
    DISHONORED_HAVE_AKAUDIO=$<BOOL:$<TARGET_EXISTS:AkAudio>>
    DISHONORED_HAVE_OSS=$<BOOL:$<TARGET_EXISTS:OnlineSubsystemSteamworks>>
    DISHONORED_HAVE_DISHONOREDGAME=$<BOOL:$<TARGET_EXISTS:DishonoredGameModule>>)
  dishonored_apply_defines(DishonoredGame)
  target_compile_options(DishonoredGame PRIVATE ${DISHONORED_MSVC_WARNINGS})
  # UnVcWin32.cpp's stack walker needs symbols; keep the PDB next to the exe (Ninja default) and
  # make the map file available for link-error analysis.
  target_link_options(DishonoredGame PRIVATE /MAP:$<TARGET_FILE_DIR:DishonoredGame>/DishonoredGame.map)
else()
  add_executable(DishonoredGame WIN32 Src/DishonoredLaunchStub.cpp)
endif()
set_target_properties(DishonoredGame PROPERTIES OUTPUT_NAME "DishonoredGame" FOLDER "Game")
