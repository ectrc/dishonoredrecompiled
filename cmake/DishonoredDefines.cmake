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
  WITH_STEAMWORKS=1
  WITH_STEAMWORKS_SOCKETS=0
  WITH_GFx=1
  WITH_GFx_IME=1
  WITH_NOVODEX=1
  WITH_APEX=1
  WITH_PHYSX_COOKING=1
  WITH_FACEFX=1
  # The Shipping exe links LZOPro (0.4 % of code) but every cooked package uses zlib
  # (package_summary.md); LZO stays off until Phase 4 decides on a replacement library.
  WITH_LZO=0
  WITH_OGGVORBIS=1
  # Not present in the Shipping exe
  WITH_TTS=0
  WITH_SPEECH_RECOGNITION=0
  WITH_FBX=0
  WITH_SPEEDTREE=0
  WITH_SPEEDTREE_MANGLE=0
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
