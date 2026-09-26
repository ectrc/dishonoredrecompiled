# Steamworks (Valve). Retail ships steam_api.dll 1.30.50.46 (PE timestamp 2012-02-03, 56 exports) and
# imports 18 undecorated cdecl entry points from it (resources/docs/symbols/imports_2013.csv:
# SteamAPI_Init/Shutdown/RunCallbacks/Register*/Unregister*, the Steam{Apps,Friends,GameServer,
# Matchmaking,MatchmakingServers,Networking,RemoteStorage,User,UserStats,Utils} accessors and
# SteamGameServer_Shutdown). There is no SDK in the tree and none is downloaded: the flat surface is
# reconstructed in source/Development/Src/External/SteamworksFlat/steam/ from the DLL's export table, the
# interface version strings inside the DLL (0x13480..0x139f4) and the retail 2013 / 2012 decompiles that
# pin every vtable slot and callback size we rely on (resources/docs/agents/agentAM.md).
#
# With DISHONORED_WITH_STEAMWORKS=ON:
#   * source/Development/Src/External/SteamworksFlat/steam_api_stub.cpp is linked into a throw-away
#     build/<dir>/steamworks/stub/steam_api.dll whose import library (build/<dir>/steamworks/steam_api.lib)
#     is exactly what Valve's dllexport build produces, so the exe imports steam_api.dll!SteamUser & co.
#     by the same names retail does. lib.exe /def cannot do this on x86 (agentU.md, same reason as Bink).
#   * Dishonored::steamworks carries that import library, the header directory and
#     /DELAYLOAD:steam_api.dll, so nothing in the DLL is touched until the first Steam call. The offline
#     path (-nosteam, or bEnableSteam=false, which is the retail DefaultEngine.ini state) never makes one,
#     so the exe still runs with no Steam client and with no steam_api.dll next to it.
#   * dishonored_apply_defines() sets WITH_STEAMWORKS=1 on every target and links Dishonored::steamworks.
#
# WITH_STEAMWORKS_SOCKETS stays 0 in every configuration, on evidence: the 2012 PDB's
# OnlineSubsystemSteamworks module consists of onlinesubsystemsteamworks.cpp, onlinesubsystemsteamworksbridge.cpp
# and onlinesubsystemsteamworkspackage.cpp only (resources/docs/symbols/sourcefiles.txt), and neither build
# has a single UnSocketSteamworks / UnNetSteamworks / FSocketSteamworks / SteamNetConnection function
# (functions.csv, functions_2013.csv). Retail's UE3 net driver is the plain IpDrv one, and turning the
# switch on would add members to UNetConnection / UNetDriver / UWorld that retail does not have.
#
# The shipped DLL stays in the retail tree (DISHONORED_RETAIL_DIR) and is never copied or staged.
# See resources/docs/middleware.md section 2.6.

option(DISHONORED_WITH_STEAMWORKS "Compile the Steamworks client path (WITH_STEAMWORKS=1) and link the steam_api.dll import library" ON)

set(DISHONORED_STEAMWORKS_DIR "${CMAKE_SOURCE_DIR}/source/Development/Src/External/SteamworksFlat")
if(DISHONORED_WITH_STEAMWORKS AND NOT EXISTS "${DISHONORED_STEAMWORKS_DIR}/steam/steam_api.h")
  message(STATUS "Steamworks: ${DISHONORED_STEAMWORKS_DIR}/steam/steam_api.h missing, WITH_STEAMWORKS stays 0")
  set(DISHONORED_WITH_STEAMWORKS OFF CACHE BOOL "" FORCE)
endif()

if(DISHONORED_WITH_STEAMWORKS)
  set(steam_dll "${DISHONORED_RETAIL_DIR}/Binaries/Win32/steam_api.dll")
  if(NOT EXISTS "${steam_dll}")
    message(STATUS "Steamworks: ${steam_dll} not found; the bindings still build (the DLL is delay-loaded and only needed at runtime with Steam enabled)")
  endif()
  add_library(steam_api_stub SHARED "${DISHONORED_STEAMWORKS_DIR}/steam_api_stub.cpp")
  set_target_properties(steam_api_stub PROPERTIES
    OUTPUT_NAME steam_api
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/steamworks/stub"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/steamworks"
    FOLDER "External")
  add_library(Dishonored::steamworks INTERFACE IMPORTED)
  target_include_directories(Dishonored::steamworks INTERFACE "${DISHONORED_STEAMWORKS_DIR}")
  # delayimp.lib carries __delayLoadHelper2, the thunk /DELAYLOAD generates a call to.
  target_link_libraries(Dishonored::steamworks INTERFACE steam_api_stub delayimp)
  # Delay-load so a build with the bindings on still starts without steam_api.dll on the search path;
  # appSteamInit (Core/Src/UnSteamworks.cpp) refuses to touch any export before the DLL loads.
  target_link_options(Dishonored::steamworks INTERFACE /DELAYLOAD:steam_api.dll)
  message(STATUS "Steamworks: WITH_STEAMWORKS=1, import library from steam_api_stub, /DELAYLOAD:steam_api.dll (runtime ${steam_dll})")
endif()
