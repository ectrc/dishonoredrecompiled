// OnlineSubsystemSteamworksClient.cpp - the Steam client side of UOnlineSubsystemSteamworks: the cached
// interface pointers, InitSteamworks, the per-frame callback pump and the two user-stats handlers.
//
// Retail keeps all of this in onlinesubsystemsteamworks.cpp, which in this tree is Epic's later-generation
// reference implementation and stays out of the build (Sources.cmake): Dishonored's module has no
// FOnlineAsyncTaskManagerSteam, no Steam sockets and no game/auth/lobby interface units at all (the 2012
// PDB's OnlineSubsystemSteamworks is three source files, resources/docs/symbols/sourcefiles.txt). What is
// here is ported from the retail decompiles named per function.
//
// The flat Steamworks surface is ours: source/Development/Src/External/SteamworksFlat/steam.
#include "OnlineSubsystemSteamworks.h"

#if WITH_UE3_NETWORKING && WITH_STEAMWORKS

#include "OnlineSubsystemUtilities.h"
#include "onlinesubsystemsteamworksbridge.h"

// DISHONORED(retail): the cached interface pointers, one 4-byte global each, in the order the retail
// globals sit at (2013 0x105b150 GSteamUtils, 0x105b154 GSteamUser, 0x105b158 GSteamFriends,
// 0x105b15c GSteamRemoteStorage, then GSteamUserStats, GSteamMatchmakingServers, GSteamGameServer,
// GSteamApps, GSteamGameServerStats, GSteamMatchmaking, GSteamNetworking, GSteamGameServerNetworking,
// GSteamGameServerUtils, GSteamAppID, GIsSteamServerReady - the 2012 PDB names them all).
// InitSteamworks (2013 rva 0x5ac1d0) reads each one out of the matching steam_api.dll accessor.
ISteamUtils* GSteamUtils = NULL;
ISteamUser* GSteamUser = NULL;
ISteamFriends* GSteamFriends = NULL;
ISteamRemoteStorage* GSteamRemoteStorage = NULL;
ISteamUserStats* GSteamUserStats = NULL;
ISteamMatchmakingServers* GSteamMatchmakingServers = NULL;
ISteamApps* GSteamApps = NULL;
ISteamMatchmaking* GSteamMatchmaking = NULL;
ISteamNetworking* GSteamNetworking = NULL;
uint32 GSteamAppID = 0;

/** DISHONORED(port): the warning hook InitSteamworks installs (2013 rva 0x5ac1d0, ISteamUtils slot 16) */
static void SteamworksWarningMessageHook(int Severity, const char* Message)
{
	debugf(NAME_DevOnline, TEXT("Steamworks(%d): %s"), Severity, ANSI_TO_TCHAR(Message));
}

// DISHONORED(port): UOnlineSubsystemSteamworks::InitSteamworks, 2013 rva 0x5ac1d0 (2012 rva 0x5f1c00).
// Fetches every interface through the exported accessors, clears GSteamworksInitialized if any is NULL,
// and on success: GSteamAppID = GSteamUtils->GetAppID(), the warning hook, UserStatsReceivedState reset,
// the callback bridge, the cloud-quota query, then - when ISteamUser::BLoggedOn() - the logged-in player
// name/id/number, LS_LoggedIn and the connection-status / login-change delegates. On failure it fires the
// connection-status delegates with OSCS_ServiceUnavailable and still returns TRUE, which is the branch the
// offline path takes (see OnlineSubsystemSteamworksOffline.cpp).
// Not ported: the low-violence flag retail caches from ISteamApps::BIsLowViolence (no consumer in this
// tree), the game-server half, and ISteamNetworking::AllowP2PPacketRelay (Steam sockets stay off,
// cmake/Steamworks.cmake).
UBOOL UOnlineSubsystemSteamworks::InitSteamworks()
{
	if (GSteamworksInitialized)
	{
		GSteamUtils = SteamUtils();
		GSteamUser = SteamUser();
		GSteamFriends = SteamFriends();
		GSteamRemoteStorage = SteamRemoteStorage();
		GSteamUserStats = SteamUserStats();
		GSteamMatchmakingServers = SteamMatchmakingServers();
		GSteamApps = SteamApps();
		GSteamNetworking = SteamNetworking();
		GSteamMatchmaking = SteamMatchmaking();

		if (GSteamUtils == NULL || GSteamUser == NULL || GSteamFriends == NULL || GSteamRemoteStorage == NULL ||
			GSteamUserStats == NULL || GSteamMatchmakingServers == NULL || GSteamApps == NULL ||
			GSteamNetworking == NULL || GSteamMatchmaking == NULL)
		{
			debugf(NAME_DevOnline, TEXT("Steamworks: an interface accessor returned NULL, disabling Steam"));
			GSteamworksInitialized = FALSE;
		}
	}

	if (!GSteamworksInitialized)
	{
		OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms ConnectionParms(EC_EventParm);
		ConnectionParms.ConnectionStatus = OSCS_ServiceUnavailable;
		TriggerOnlineDelegates(this, ConnectionStatusChangeDelegates, &ConnectionParms);
		return TRUE;
	}

	GSteamAppID = GSteamUtils->GetAppID();
	GSteamUtils->SetWarningMessageHook(SteamworksWarningMessageHook);
	UserStatsReceivedState = OERS_NotStarted;
	CallbackBridge = new SteamCallbackBridge(this);

	if (GSteamRemoteStorage->IsCloudEnabledForAccount() && GSteamRemoteStorage->IsCloudEnabledForApp())
	{
		int32 TotalBytes = -1;
		int32 AvailableBytes = -1;
		GSteamRemoteStorage->GetQuota(&TotalBytes, &AvailableBytes);
		debugf(NAME_DevOnline, TEXT("Steam cloud quota: %d of %d bytes available"), (INT)AvailableBytes, (INT)TotalBytes);
	}

	if (GSteamUser->BLoggedOn())
	{
		LoggedInPlayerName = UTF8_TO_TCHAR(GSteamFriends->GetPersonaName());
		LoggedInPlayerNum = 0;
		LoggedInPlayerId.Uid = GSteamUser->GetSteamID().ConvertToUint64();
		LoggedInStatus = LS_LoggedIn;

		OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms ConnectionParms(EC_EventParm);
		ConnectionParms.ConnectionStatus = OSCS_Connected;
		TriggerOnlineDelegates(this, ConnectionStatusChangeDelegates, &ConnectionParms);

		OnlineSubsystemSteamworks_eventOnLoginChange_Parms LoginParms(EC_EventParm);
		LoginParms.LocalUserNum = LoggedInPlayerNum;
		TriggerOnlineDelegates(this, LoginChangeDelegates, &LoginParms);
	}

	return TRUE;
}

// DISHONORED(port): UOnlineSubsystemSteamworks::TickSteamworksTasks, 2013 rva 0x5aa980 (2012 rva
// 0x5f0930): pumps the Steam callbacks every frame. The avatar request queue retail also ages here needs
// GetOnlineAvatar (2013 rva 0x5a92d0), which is not ported, so QueuedAvatarRequests stays untouched.
void UOnlineSubsystemSteamworks::TickSteamworksTasks(FLOAT DeltaTime)
{
	if (GSteamworksInitialized)
	{
		SteamAPI_RunCallbacks();
	}
}

// DISHONORED(port): UOnlineSubsystemSteamworks::OnUserStatsReceived, 2012 rva 0x5f07d0
// (onlinesubsystemsteamworks.cpp:710). The game id test is CGameID(GSteamAppID), i.e. the app id in the
// low 24 bits.
void UOnlineSubsystemSteamworks::OnUserStatsReceived(UserStatsReceived_t* CallbackData)
{
	const CGameID GameID((AppId_t)GSteamAppID);
	if (CallbackData->m_nGameID != GameID.ToUint64())
	{
		return;
	}

	if (CallbackData->m_eResult == k_EResultOK)
	{
		UserStatsReceivedState = OERS_Done;

		OnlineSubsystemSteamworks_eventOnReadAchievementsComplete_Parms Parms(EC_EventParm);
		Parms.TitleId = 0;
		TriggerOnlineDelegates(this, AchievementReadDelegates, &Parms);
	}
	else
	{
		UserStatsReceivedState = OERS_Failed;
	}
}

// DISHONORED(port): UOnlineSubsystemSteamworks::OnUserStatsStored, 2012 rva 0x5f0840
// (onlinesubsystemsteamworks.cpp:744). The bClientStatsStorePending half fires
// FlushOnlineStatsDelegates, which no class in this tree has yet, so only the achievement half is ported.
void UOnlineSubsystemSteamworks::OnUserStatsStored(UserStatsStored_t* CallbackData)
{
	const CGameID GameID((AppId_t)GSteamAppID);
	if (CallbackData->m_nGameID != GameID.ToUint64())
	{
		return;
	}

	const DWORD Result = CallbackData->m_eResult == k_EResultOK ? S_OK : E_FAIL;
	if (bStoringAchievement)
	{
		FAsyncTaskDelegateResults Params(Result);
		TriggerOnlineDelegates(this, AchievementDelegates, &Params);
		bStoringAchievement = FALSE;
	}
	if (bClientStatsStorePending)
	{
		// DISHONORED(bringup): retail also triggers FlushOnlineStatsDelegates with an
		// FAsyncTaskDelegateResultsNamedSession(FName("Game"), Result) here; the delegate array does not
		// exist in this tree's UOnlineSubsystem yet.
		bClientStatsStorePending = FALSE;
	}
}

#endif // WITH_UE3_NETWORKING && WITH_STEAMWORKS
