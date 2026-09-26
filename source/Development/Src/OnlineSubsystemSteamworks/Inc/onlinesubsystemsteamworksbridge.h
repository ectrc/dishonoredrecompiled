// onlinesubsystemsteamworksbridge.h - SteamCallbackBridge, the object retail's UOnlineSubsystemSteamworks
// keeps in its CallbackBridge member (FPointer at +200) and which owns every Steam callback registration.
//
// This is Arkane's own class, not Epic's: the reference OnlineSubsystemSteamworks of this tree is a later
// UE3 generation with FOnlineAsyncTaskManagerSteam and per-task CCallbacks, while the 2012 PDB shows
// Dishonored's module is only onlinesubsystemsteamworks.cpp + onlinesubsystemsteamworksbridge.cpp +
// onlinesubsystemsteamworkspackage.cpp (resources/docs/symbols/sourcefiles.txt), with all callbacks in
// this one bridge. Reconstructed from:
//   * the 2012 PDB's member and method names (35 SteamCallbackBridge entries in functions.csv, the
//     CCallback<SteamCallbackBridge,...> instantiations naming every registered callback struct)
//   * the 2013 constructor, rva 0x5ab400: 12 CCallback members at +4, +24, ... +224 after the Subsystem
//     pointer at +0, which is the sizeof 244 that retail's InitSteamworks allocates (appMalloc(244, 8),
//     2013 rva 0x5ac1d0)
//   * the 2012 constructor, rva 0x5f13f0, for the two user-stats callbacks 2013 dropped (see below).
#ifndef INC_DISHONORED_ONLINESUBSYSTEMSTEAMWORKSBRIDGE
#define INC_DISHONORED_ONLINESUBSYSTEMSTEAMWORKSBRIDGE

#if WITH_UE3_NETWORKING && WITH_STEAMWORKS

#include "steam/steam_api.h"

class UOnlineSubsystemSteamworks;

class SteamCallbackBridge
{
public:
	SteamCallbackBridge(UOnlineSubsystemSteamworks* InSubsystem);

private:
	void OnUserStatsReceived(UserStatsReceived_t* CallbackData);
	void OnUserStatsStored(UserStatsStored_t* CallbackData);
	void OnGameOverlayActivated(GameOverlayActivated_t* CallbackData);
	void OnDLCInstalled(DlcInstalled_t* CallbackData);
	void OnSteamShutdown(SteamShutdown_t* CallbackData);
	void OnSteamServersConnected(SteamServersConnected_t* CallbackData);
	void OnSteamServersDisconnected(SteamServersDisconnected_t* CallbackData);

	// DISHONORED(layout): +0 in retail (2013 ctor rva 0x5ab400 writes it first).
	UOnlineSubsystemSteamworks* Subsystem;

	// DISHONORED(port): the client-side callbacks retail's 2013 bridge registers, in its order:
	// GameOverlayActivated_t (+4), DlcInstalled_t (+24), SteamShutdown_t (+44),
	// GameServerChangeRequested_t (+64), GameRichPresenceJoinRequested_t (+84),
	// SteamServersConnected_t (+104), SteamServersDisconnected_t (+124), AvatarImageLoaded_t (+144).
	CCallback<SteamCallbackBridge, GameOverlayActivated_t, false> GameOverlayActivatedCallback;
	CCallback<SteamCallbackBridge, DlcInstalled_t, false> DLCInstalledCallback;
	CCallback<SteamCallbackBridge, SteamShutdown_t, false> SteamShutdownCallback;
	CCallback<SteamCallbackBridge, SteamServersConnected_t, false> SteamServersConnectedCallback;
	CCallback<SteamCallbackBridge, SteamServersDisconnected_t, false> SteamServersDisconnectedCallback;

	// DISHONORED(port): UserStatsReceived_t / UserStatsStored_t, 2012 bridge members
	// (2012 ctor rva 0x5f13f0, handlers 0x5f1260 / 0x5f1270 forwarding to
	// UOnlineSubsystemSteamworks::OnUserStatsReceived 0x5f07d0 / OnUserStatsStored 0x5f0840). The 2013
	// bridge no longer has them - with them gone, retail 2013's UserStatsReceivedState can never leave
	// OERS_InProgress once ReadAchievements (0x5aaf90) has asked for the stats, so its achievement-read
	// completion delegate never fires. They are kept here because the ported ReadAchievements is the
	// caller of RequestCurrentStats and has to be able to complete.
	CCallback<SteamCallbackBridge, UserStatsReceived_t, false> UserStatsReceivedCallback;
	CCallback<SteamCallbackBridge, UserStatsStored_t, false> UserStatsStoredCallback;

	// DISHONORED(written): retail also carries GameServerChangeRequested_t, GameRichPresenceJoinRequested_t
	// and AvatarImageLoaded_t (client) plus GSPolicyResponse_t, GSClientAchievementStatus_t and the two
	// game-server SteamServers* slots. Four of those have m_Func == NULL in retail's own constructor and
	// the rest need UOnlineSubsystemSteamworks::OnGameServerChangeRequested (0x5a89c0),
	// OnGameJoinRequested (0x5a8b60) and OnAvatarImageLoaded (0x5aa8c0), none of which is ported yet.
	// They are left out rather than registered with a handler that does nothing.
};

#endif // WITH_UE3_NETWORKING && WITH_STEAMWORKS

#endif // INC_DISHONORED_ONLINESUBSYSTEMSTEAMWORKSBRIDGE
