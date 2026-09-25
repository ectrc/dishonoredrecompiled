# OnlineSubsystemSteamworks module sources (agent T, Phase 3 wave 2). Only the generated registrant
# units compile: OnlineSubsystemSteamworksRegistrants.cpp / OnlineSubsystemSteamworksNativeStubs.cpp
# (resources/tools/symbols/gen_classes_header.py --sdk) register the 2 native classes of the retail
# OnlineSubsystemSteamworks.upk with the retail layout. The Epic implementation needs the Steamworks SDK
# (WITH_STEAMWORKS=0; resources/docs/middleware.md) and stays out until Phase 4.
set(OnlineSubsystemSteamworks_EXCLUDE
  Src/OnlineAsyncTaskManagerSteam.cpp
  Src/OnlineSubsystemSteamworks.cpp
  Src/OnlineSubsystemSteamworksPackage.cpp
  Src/UOnlineAuthInterfaceSteamworks.cpp
  Src/UOnlineGameInterfaceSteamworks.cpp
  Src/UOnlineLobbyInterfaceSteamworks.cpp
  Src/UnNetSteamworks.cpp
  Src/UnSocketSteamworks.cpp
  Src/VoiceInterfaceSteamworks.cpp
  Src/onlinesubsystemsteamworksbridge.cpp
)
set(OnlineSubsystemSteamworks_NOT_IN_PDB
)
