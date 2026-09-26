// OnlineSubsystemSteamworksOffline.cpp - UOnlineSubsystemSteamworks natives on the retail path that runs without Steam (the
// Steam DLL is never loaded in this build: WITH_STEAMWORKS=0, GSteamworksInitialized stays FALSE, as with a failed SteamAPI_Init
// or -nosteam in retail).
#include "OnlineSubsystemSteamworks.h"

// DISHONORED(port): UOnlineSubsystemSteamworks::Init, 2013 rva 0x5ad3d0 (2012 rva 0x5f2b50, onlinesubsystemsteamworks.cpp:1526),
// reached from script through UOnlineSubsystem::execInit (2013 rva 0x1c5da0: P_FINISH, *Result = this->Init()). The generated class
// declares its own execInit, so the thunk carries the body. Offline branches as decompiled:
// - InitSteamworks (2013 rva 0x5ac1d0): no Steam interfaces -> ConnectionStatusChangeDelegates(OSCS_ServiceUnavailable), TRUE
// - SignInLocally (2013 rva 0x5aab40): not logged in to Steam -> the class default LocalProfileName, player 0, LS_UsingLocalProfile,
//   LoginChangeDelegates(0)
// Not ported (Steam-only effects): the virtual SetNetworkNotificationPosition(CurrentNotificationPosition) call (overlay position
// through ISteamUtils), SetRichPresence("0","Dud") (GSteamFriends NULL), and the 140-byte leaderboard helper stored in
// pLeaderboardHelper (Steam CCallResult holders for callbacks 1104-1106, 2013 ctor rva 0x5ab620); pLeaderboardHelper stays NULL.
void UOnlineSubsystemSteamworks::execInit( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;

	UOnlineSubsystem::Init();
	m_paEnumeratedDLCs = new TArray<BYTE>();
	bLastHasConnection = GSocketSubsystem->HasNetworkDevice();
	LoggedInPlayerName = LocalProfileName;
	LoggedInPlayerNum = -1;
	LoggedInStatus = LS_NotLoggedIn;
	appMemzero(&LoggedInPlayerId, sizeof(LoggedInPlayerId));

	// DISHONORED(port): InitSteamworks (2013 rva 0x5ac1d0, OnlineSubsystemSteamworksClient.cpp). Without
	// Steam it does exactly what this file used to do inline - fire the connection-status delegates with
	// OSCS_ServiceUnavailable and return TRUE - and with Steam up it caches the interfaces and signs the
	// Steam user in. Agent AM.
#if WITH_UE3_NETWORKING && WITH_STEAMWORKS
	InitSteamworks();
#else
	OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms ConnectionParms(EC_EventParm);
	ConnectionParms.ConnectionStatus = OSCS_ServiceUnavailable;
	TriggerOnlineDelegates(this, ConnectionStatusChangeDelegates, &ConnectionParms);
#endif

	eventSetAccountInterface(this);
	eventSetPlayerInterface(this);
	eventSetPlayerInterfaceEx(this);
	eventSetStatsInterface(this);
	eventSetSystemInterface(this);
	if (ProfileDataDirectory.Len() == 0)
	{
		ProfileDataDirectory = TEXT(".\\");
	}

	// DISHONORED(port): SignInLocally (2013 rva 0x5aab40) only runs when the Steam user is not logged on
	// (its first test is GSteamworksInitialized && GSteamUser->BLoggedOn()), so a real Steam sign-in from
	// InitSteamworks above is not overwritten. Agent AM.
	if (LoggedInStatus == LS_NotLoggedIn)
	{
		LoggedInPlayerName = CastChecked<UOnlineSubsystemSteamworks>(GetClass()->GetDefaultObject())->LocalProfileName;
		LoggedInPlayerNum = 0;
		LoggedInStatus = LS_UsingLocalProfile;
		OnlineSubsystemSteamworks_eventOnLoginChange_Parms LoginParms(EC_EventParm);
		LoginParms.LocalUserNum = 0;
		TriggerOnlineDelegates(this, LoginChangeDelegates, &LoginParms);
	}

	*(UBOOL*)Result = TRUE;
}

// DISHONORED(port): execGetLoginStatus 2013 rva 0x5a4270 -> virtual GetLoginStatus 2013 rva 0x5a52c0 (2012 rva 0x5e9fc0): the Steam
// user's logon state for LoggedInPlayerNum while GSteamworksInitialized, LS_NotLoggedIn otherwise (the local-profile sign-in above
// does not change it)
void UOnlineSubsystemSteamworks::execGetLoginStatus( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(LocalUserNum);
	P_FINISH;
	*(BYTE*)Result = LS_NotLoggedIn;
}

// DISHONORED(port): execIsControllerConnected 2013 rva 0x5a4670 -> vtable slot +396 of UOnlineSubsystemSteamworks = 2013 rva 0x5ea9d0
// (COMDAT-folded `return 1`): every controller counts as connected on PC
void UOnlineSubsystemSteamworks::execIsControllerConnected( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(ControllerId);
	P_FINISH;
	*(UBOOL*)Result = TRUE;
}

// DISHONORED(port): UOnlineSubsystemSteamworks::Tick, offline branches only (the Steam client/server, voice and game-interface
// ticks need the Steamworks SDK). Keeps the local sign-in for a not-logged-in user and the connection-status change delegates.
void UOnlineSubsystemSteamworks::Tick(FLOAT DeltaTime)
{
	// DISHONORED(port): retail's Tick pumps the Steam callbacks first (2013 rva 0x5ac4e0 ->
	// TickSteamworksTasks 0x5aa980); with Steam off it is a no-op. Agent AM.
#if WITH_UE3_NETWORKING && WITH_STEAMWORKS
	TickSteamworksTasks(DeltaTime);
#endif

	if (LoggedInStatus == LS_NotLoggedIn)
	{
		LoggedInPlayerName = CastChecked<UOnlineSubsystemSteamworks>(GetClass()->GetDefaultObject())->LocalProfileName;
		LoggedInPlayerNum = 0;
		LoggedInStatus = LS_UsingLocalProfile;
		OnlineSubsystemSteamworks_eventOnLoginChange_Parms LoginParms(EC_EventParm);
		LoginParms.LocalUserNum = 0;
		TriggerOnlineDelegates(this, LoginChangeDelegates, &LoginParms);
	}

	const UBOOL bHasConnection = GSocketSubsystem ? GSocketSubsystem->HasNetworkDevice() : FALSE;
	if (bHasConnection != bLastHasConnection)
	{
		bLastHasConnection = bHasConnection;
		OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms ConnectionParms(EC_EventParm);
		ConnectionParms.ConnectionStatus = bHasConnection ? OSCS_Connected : OSCS_ConnectionDropped;
		TriggerOnlineDelegates(this, ConnectionStatusChangeDelegates, &ConnectionParms);
	}
}
