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
  foreach(m IN ITEMS GameFramework IpDrv WinDrv)
    if(TARGET ${m})
      list(APPEND launch_modules ${m})
    endif()
  endforeach()
  target_link_libraries(DishonoredGame PRIVATE ${launch_modules})
  target_compile_definitions(DishonoredGame PRIVATE
    DISHONORED_HAVE_GAMEFRAMEWORK=$<BOOL:$<TARGET_EXISTS:GameFramework>>
    DISHONORED_HAVE_IPDRV=$<BOOL:$<TARGET_EXISTS:IpDrv>>
    DISHONORED_HAVE_WINDRV=$<BOOL:$<TARGET_EXISTS:WinDrv>>)
  dishonored_apply_defines(DishonoredGame)
  target_compile_options(DishonoredGame PRIVATE ${DISHONORED_MSVC_WARNINGS})
  # UnVcWin32.cpp's stack walker needs symbols; keep the PDB next to the exe (Ninja default) and
  # make the map file available for link-error analysis.
  target_link_options(DishonoredGame PRIVATE /MAP:$<TARGET_FILE_DIR:DishonoredGame>/DishonoredGame.map)
  # Engine/Src/UnPNG.cpp carries #pragma comment(lib, "libpng15.lib") for the reference's own libpng
  # 1.5 build; ours is Dishonored::libPNG (FetchContent 1.6.43), so drop the injected name.
  target_link_options(DishonoredGame PRIVATE /NODEFAULTLIB:libpng15.lib)
  # Engine's UnTexCompress.cpp imports nvtt.dll (reference Development/External/nvtt, a VC8 DLL
  # that also needs cudart/MSVCR80); retail ships no nvtt.dll. Delay-load it so the exe starts
  # without it: nothing before package cooking calls into nvtt.
  target_link_options(DishonoredGame PRIVATE /DELAYLOAD:nvtt.dll)
  target_link_libraries(DishonoredGame PRIVATE delayimp)
else()
  add_executable(DishonoredGame WIN32 Src/DishonoredLaunchStub.cpp)
endif()
set_target_properties(DishonoredGame PROPERTIES OUTPUT_NAME "DishonoredGame" FOLDER "Game")
