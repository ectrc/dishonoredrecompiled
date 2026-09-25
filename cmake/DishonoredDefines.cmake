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
  WITH_EDITORONLY_DATA=0
  WITH_MANAGED_CODE=0
  # UnBuild.h switches, set to what Shipping links (Steamworks OSS, Scaleform, PhysX+APEX, FaceFX, LZO)
  WITH_UE3_NETWORKING=1
  # Shipping uses the Steamworks OSS and Scaleform GFx 4 (10.4 % of code), but neither SDK is in
  # the reference tree (steam/steam_api.h, Kernel/SF_Types.h). Both are off until Phase 4 provides
  # them; Engine.h pulls their headers into every module otherwise.
  WITH_STEAMWORKS=0
  WITH_STEAMWORKS_SOCKETS=0
  WITH_GFx=0
  WITH_GFx_IME=0
  # Engine's UnNovodexSupport.h needs PhysX 2.8.4 (NxCooking.h, NxSceneQuery.h, fluids/, Nxd),
  # but the reference tree only ships Development/External/Novodex = NovodeX SDK 2.1.2 (2004,
  # NxVersionNumber.h), which has neither. Off until Phase 4 provides PhysX 2.8.4; Shipping links
  # it (physx/physxloader in module_map.md). UnBuild.h then sets NX_DISABLE_FLUIDS.
  WITH_NOVODEX=0
  # APEX headers in the reference need the PhysX 3 foundation (foundation/PxSimpleTypes.h), which
  # the Novodex 2.8 SDK in the reference tree lacks; off until Phase 4 (Dishonored ships APEX DLLs).
  WITH_APEX=0
  WITH_PHYSX_COOKING=1
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
endfunction()
