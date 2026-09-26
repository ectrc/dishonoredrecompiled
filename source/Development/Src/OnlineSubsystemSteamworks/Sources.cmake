# OnlineSubsystemSteamworks module sources. The generated registrant units
# (OnlineSubsystemSteamworksRegistrants.cpp / OnlineSubsystemSteamworksNativeStubs.cpp, from
# resources/tools/symbols/gen_classes_header.py --sdk) register the 2 native classes of the retail
# OnlineSubsystemSteamworks.upk with the retail layout; OnlineSubsystemSteamworksOffline.cpp,
# OnlineSubsystemSteamworksReads.cpp, OnlineSubsystemSteamworksClient.cpp and
# onlinesubsystemsteamworksbridge.cpp are ours (agents X and AM).
#
# The units below stay excluded because they are Epic's later-generation UE3 Steamworks OSS, not
# Dishonored's: the 2012 PDB's OnlineSubsystemSteamworks module is onlinesubsystemsteamworks.cpp,
# onlinesubsystemsteamworksbridge.cpp and onlinesubsystemsteamworkspackage.cpp only
# (resources/docs/symbols/sourcefiles.txt), and neither build has a single FOnlineAsyncTaskManagerSteam,
# FVoiceInterfaceSteam, UOnlineGameInterfaceSteamworks, UOnlineAuthInterfaceSteamworks,
# UnSocketSteamworks or UnNetSteamworks function (functions.csv / functions_2013.csv). Retail reaches the
# same script API through one big onlinesubsystemsteamworks.cpp plus the callback bridge, so the ports go
# into our own units instead. OnlineSubsystemSteamworksPackage.cpp is the reference's registrant unit,
# replaced by the generated one.
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
)
set(OnlineSubsystemSteamworks_NOT_IN_PDB
)
