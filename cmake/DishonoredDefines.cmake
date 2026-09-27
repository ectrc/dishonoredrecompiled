# Preprocessor definitions for every engine module. Values reproduce what UnrealBuildTool
# (../UnrealEngine3/Development/Src/UnrealBuildTool/Configuration/UE3BuildWin32.cs,
# UE3BuildTarget.cs) and Core/Inc/UnBuild.h set for a Win32 game build, adjusted to what the
# Dishonored Shipping exe actually links (resources/docs/module_map.md). UnBuild.h itself is
# never edited; every switch is overridden from here.

set(DISHONORED_DEFINES
  # UE3BuildWin32.cs: platform
  _WINDOWS=1
  WIN32=1
  _WIN32_WINNT=0x0502
  WINVER=0x0502
  UNICODE
  _UNICODE
  # LaunchGames.h assigns 2..8 to Epic's games; Dishonored takes the next free slot
  GAMENAME=DISHONOREDGAME
  DISHONOREDGAME=9
  IS_DISHONOREDGAME=1
  # UE3BuildTarget.cs: game (non-editor) configuration
  WITH_EDITOR=0
  # The 2012 PDB keeps the editor-only members (EditorIconColor, SourceFilePath, LightingGuid, ...)
  # in the shipping layouts; UE3BuildTarget.cs only sets 0 for script-patch exes (agent M).
  WITH_EDITORONLY_DATA=1
  WITH_MANAGED_CODE=0
  # UnBuild.h switches, set to what Shipping links (Steamworks OSS, Scaleform, PhysX+APEX, FaceFX, LZO)
  WITH_UE3_NETWORKING=1
  # Shipping uses the Steamworks OSS and Scaleform GFx 4 (10.4 % of code). Steamworks is ours now:
  # source/Development/Src/External/SteamworksFlat reconstructs the flat surface of the shipped
  # steam_api.dll and cmake/Steamworks.cmake provides Dishonored::steamworks, so WITH_STEAMWORKS
  # follows DISHONORED_WITH_STEAMWORKS (default ON when those headers are present). GFx has no
  # runtime yet and stays off; Engine.h would pull its headers into every module otherwise.
  # WITH_STEAMWORKS_SOCKETS stays 0 in every configuration: neither the 2012 PDB nor the retail 2013
  # exe has one UnSocketSteamworks / UnNetSteamworks function, so retail's net driver is plain IpDrv
  # (evidence in cmake/Steamworks.cmake).
  WITH_STEAMWORKS=$<BOOL:${DISHONORED_WITH_STEAMWORKS}>
  WITH_STEAMWORKS_SOCKETS=0
  WITH_GFx=0
  WITH_GFx_IME=0
  # WITH_NOVODEX / WITH_PHYSX_COOKING / NX_DISABLE_FLUIDS are set per target by
  # dishonored_apply_defines() from DISHONORED_WITH_PHYSX (cmake/PhysX.cmake), because they must be
  # identical in every module and they depend on the reconstructed PhysX 2.8.4 headers being present.
  # APEX stays off permanently: the retail exe imports nothing from any APEX_*.dll and the 2012 PDB
  # has no NxApex/physx::apex function at all (resources/docs/middleware.md 1, row APEX).
  WITH_APEX=0
  # FaceFX is linked into Shipping (1.3 %), but its SDK is not in the reference tree; off until
  # Phase 4 provides a runtime. Engine class layouts that embed FaceFX members are checked by the
  # layout probe regardless.
  WITH_FACEFX=0
  # Every cooked package is PKG_StoreCompressed with CompressionFlags=2 = COMPRESS_LZO (Dishonored
  # links LZO Pro, lzopro_lzo1x_decompress_safe; format-compatible with LZO1X). The codec is lzokay
  # (cmake/Dependencies.cmake), wired into Core/Src/UnMisc.cpp appCompressMemoryLZO /
  # appUncompressMemoryLZO. WITH_LZO also selects COMPRESS_DefaultPC = COMPRESS_LZO in UnFile.h, so
  # GBaseCompressionMethod defaults to 2 like the retail exe (serialization_delta_core.md).
  WITH_LZO=1
  # The Shipping PDB has no FVorbisAudioInfo / UnAudioDecompress.cpp functions (audio is Wwise,
  # AkAudio); libvorbis is not in the reference tree either (UnAudioDecompress.h includes
  # vorbis/vorbisenc.h). Matches the shipped exe.
  WITH_OGGVORBIS=0
  # Dishonored-only switch (RawIndexBuffer.cpp): the reference nvTriStrip is the stock 16-bit-index
  # library while UE3 calls Epic's 32-bit fork. The strip/cache optimiser is cook-time only and no
  # CacheOptimize function exists in the Shipping PDB; on until Phase 4 ports the 32-bit fork.
  WITH_NVTRISTRIP=0
  # Not present in the Shipping exe
  WITH_TTS=0
  WITH_SPEECH_RECOGNITION=0
  WITH_FBX=0
  WITH_SPEEDTREE=0
  # WITH_SPEEDTREE_MANGLE is derived inside UnBuild.h (EPIC_INTERNAL), do not redefine it here
  WITH_PANORAMA=0
  WITH_GAMESPY=0
  WITH_GAMECENTER=0
  WITH_SUBSTANCE_AIR=0
  WITH_APSALAR=0
  WITH_SWRVE=0
  WITH_TESTTRACK=0
  WITH_OPEN_AUTOMATE=0
  WITH_DATABASE_SUPPORT=0
  WITH_ACTORX=0
  WITH_IME=1
  USE_NULL_RHI=0
  # UnBuild.h defaults USE_UNIT_TESTS to !FINAL_RELEASE && !SHIPPING_PC_GAME, which makes UEngine::Exec("UNITTEST")
  # reference Core/Src/UnitTest.cpp (excluded, Core/Sources.cmake). Neither the 2012 PDB nor the retail 2013 exe
  # has an FUnitTestFramework function (functions.csv / functions_2013.csv), so the harness is off in every config.
  USE_UNIT_TESTS=0
  # Engine/Src/UnPNG.cpp: #pragma comment(lib, "libpng15.lib") is for the reference's own libpng 1.5.13 build;
  # this tree links Dishonored::libPNG (FetchContent 1.6.43, cmake/Dependencies.cmake).
  WITH_REFERENCE_LIBPNG=0
  SUPPORTS_SCRIPTPATCH_CREATION=0
  XDKINSTALLED=0
  EPIC_INTERNAL=0
)

# Per-configuration switches (UE3BuildTarget.cs: Debug / Release / Shipping)
set(DISHONORED_DEFINES_DEBUG   _DEBUG FINAL_RELEASE=0 SHIPPING_PC_GAME=0)
set(DISHONORED_DEFINES_RELEASE NDEBUG FINAL_RELEASE=0 SHIPPING_PC_GAME=0)
set(DISHONORED_DEFINES_SHIPPING NDEBUG FINAL_RELEASE=1 SHIPPING_PC_GAME=1 NO_LOGGING=1)

option(DISHONORED_SHIPPING "Build with the Shipping switches (FINAL_RELEASE, SHIPPING_PC_GAME, NO_LOGGING)" OFF)
option(DISHONORED_LAYOUT_CHECKS "Compile the PDB-derived static_assert layout checks" ON)

function(dishonored_apply_defines target)
  target_compile_definitions(${target} PRIVATE ${DISHONORED_DEFINES})
  # UE3 is built with 4-byte struct packing (UnrealBuildTool VCToolChain.cs); the PDB layouts and the
  # cooked packages assume it (UProperty::PropertyFlags QWORD at 68, ULinkerLoad::TickStartTime at 1628).
  # Windows API headers are wrapped by WinDrv's PreWindowsApi.h (pack 8) / PostWindowsApi.h.
  target_compile_options(${target} PRIVATE /Zp4)
  if(DISHONORED_SHIPPING)
    target_compile_definitions(${target} PRIVATE ${DISHONORED_DEFINES_SHIPPING})
  else()
    target_compile_definitions(${target} PRIVATE
      $<$<CONFIG:Debug>:${DISHONORED_DEFINES_DEBUG}>
      $<$<NOT:$<CONFIG:Debug>>:${DISHONORED_DEFINES_RELEASE}>)
  endif()
  if(DISHONORED_LAYOUT_CHECKS AND NOT DISHONORED_SHIPPING)
    target_compile_definitions(${target} PRIVATE DISHONORED_LAYOUT_CHECKS=1)
  else()
    target_compile_definitions(${target} PRIVATE DISHONORED_LAYOUT_CHECKS=0)
  endif()
  # USE_BINK_CODEC (UnBuild.h default EPIC_INTERNAL && !UE3_LEAN_AND_MEAN = 0): retail links Bink
  # (binkw32.dll, 20 imports) and UnCodecs.h declares UCodecMovieBink differently per value, so the
  # switch must be identical in every module. cmake/Bink.cmake provides Dishonored::bink.
  if(DISHONORED_WITH_BINK)
    target_compile_definitions(${target} PRIVATE USE_BINK_CODEC=1)
    target_link_libraries(${target} PUBLIC Dishonored::bink)
  endif()
  # PhysX 2.8.4 (cmake/PhysX.cmake). WITH_NOVODEX guards ~100 Engine files and changes class layouts
  # (UnPhysPublic.h, EnginePhysicsClasses.h), so it must be identical in every module.
  #  * WITH_PHYSX_COOKING=1 is what UnBuild.h sets on every platform and what retail does (the exe
  #    imports NxGetCookingLib and cooks the level BSP at load time, InitGameRBPhys 2013 rva 0x3d5710);
  #  * NX_DISABLE_FLUIDS=1 is what UnBuild.h derives when PhysX is off and what this bring-up keeps on
  #    purpose: the fluid/particle path is not on the collision path and would pull in another ~20
  #    files (resources/docs/agents/agentAL.md);
  #  * USE_QUICKLOAD_CONVEX=0 because the retail exe imports nothing from PhysXExtensions.dll
  #    (imports_2013.csv), i.e. Arkane shipped with the QuickLoad convex path off;
  #  * SUPPORT_DOUBLE_BUFFERING=0 because NxdScene lives in the SDK's static libnxdoublebuffered,
  #    which is not a DLL and which we do not have (middleware.md 2.2).
  if(DISHONORED_WITH_PHYSX)
    target_compile_definitions(${target} PRIVATE
      WITH_NOVODEX=1 WITH_PHYSX_COOKING=1 NX_DISABLE_FLUIDS=1
      USE_QUICKLOAD_CONVEX=0 SUPPORT_DOUBLE_BUFFERING=0
      DISHONORED_PHYSX_IMPORT_LIB=1)
    target_link_libraries(${target} PUBLIC Dishonored::physx)
  else()
    target_compile_definitions(${target} PRIVATE WITH_NOVODEX=0 WITH_PHYSX_COOKING=1)
  endif()
  # Scaleform GFx 3.3 (cmake/GFx.cmake). DISHONORED_WITH_GFX3 goes on every target because GFxUI and
  # DishonoredGame both name GFxValue/GFxMovieView/GRenderer, and the header directory comes with the
  # target. It changes no engine layout and nothing instantiates the runtime yet, so a build with it
  # on runs exactly as a build with it off (resources/docs/agents/agentBB.md).
  if(DISHONORED_WITH_GFX3)
    target_compile_definitions(${target} PRIVATE DISHONORED_WITH_GFX3=1)
    target_link_libraries(${target} PUBLIC Dishonored::gfx3)
  else()
    target_compile_definitions(${target} PRIVATE DISHONORED_WITH_GFX3=0)
  endif()
  # WITH_STEAMWORKS (above) changes FUniqueNetId's conversion guards in EngineClasses.h, so the switch
  # must be identical in every module; the import library and the header directory come with the target.
  if(DISHONORED_WITH_STEAMWORKS)
    target_link_libraries(${target} PUBLIC Dishonored::steamworks)
  endif()
  # Wwise 2012.1 (cmake/Wwise.cmake): our own AK headers plus the silent backend, or the installed SDK.
  # It goes on every module for the same reason Bink does: Engine's akbank.cpp / akevent.cpp / UnActor.cpp call
  # AK::SoundEngine as much as AkAudio does, and DishonoredGame's UDishonoredAudioSystem does too, so the
  # switch must have the same value in every unit. Without it those bodies stay DISHONORED(bringup) stubs.
  if(TARGET Dishonored::wwise)
    target_link_libraries(${target} PUBLIC Dishonored::wwise)
  else()
    target_compile_definitions(${target} PRIVATE DISHONORED_WITH_WWISE=0)
  endif()
endfunction()
