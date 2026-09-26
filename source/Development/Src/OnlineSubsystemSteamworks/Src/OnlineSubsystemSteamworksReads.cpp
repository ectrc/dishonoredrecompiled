// OnlineSubsystemSteamworksReads.cpp - the three UOnlineSubsystemSteamworks async reads script calls on
// the startup and map path: ReadFriendsList, ReadProfileSettings and ReadAchievements, each with its exec
// thunk. They were the last natives on that path without a body (resources/docs/PHASE6.md, wave result).
//
// Every one is ported from its retail 2013 decompile and takes the same two branches retail does: the
// Steam branch when GSteamworksInitialized is set, and the offline branch otherwise. The offline branch
// is what a -nosteam run (or the retail install's bEnableSteam=false) takes, and it is fully determined:
// the reads complete immediately through their delegates. The Steam branch is compiled only with
// WITH_STEAMWORKS=1 (cmake/Steamworks.cmake), so this unit builds either way.
#include "OnlineSubsystemSteamworks.h"
#include "OnlineSubsystemUtilities.h"

#if WITH_STEAMWORKS
#include "steam/steam_api.h"
extern ISteamUserStats* GSteamUserStats;
#endif

// DISHONORED(port): UOnlineSubsystemSteamworks::ReadFriendsList, 2013 rva 0x5aaf30 (2012 rva 0x5f0db0,
// vtable slot 95 = +380). S_OK only while Steam is up and the caller is the logged-in player, and the
// read-friends delegates fire either way - retail's comment in the reference body is "always trigger the
// delegate immediately and again as friends are added".
UBOOL UOnlineSubsystemSteamworks::ReadFriendsList(BYTE LocalUserNum, INT Count, INT StartingAt)
{
	DWORD Return = E_FAIL;
	if (GSteamworksInitialized && LocalUserNum == LoggedInPlayerNum)
	{
		Return = S_OK;
	}

	FAsyncTaskDelegateResults Params(Return);
	TriggerOnlineDelegates(this, ReadFriendsDelegates, &Params);
	return Return == S_OK;
}

// DISHONORED(port): execReadFriendsList, 2013 rva 0x5a4550. Parameters per the retail script signature
// (CodeRed SDK dump UOnlineSubsystemSteamworks_execReadFriendsList_Params: LocalUserNum, optional Count,
// optional StartingAt, bool return).
void UOnlineSubsystemSteamworks::execReadFriendsList( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(LocalUserNum);
	P_GET_INT_OPTX(Count, 0);
	P_GET_INT_OPTX(StartingAt, 0);
	P_FINISH;
	*(UBOOL*)Result = ReadFriendsList(LocalUserNum, Count, StartingAt);
}

// DISHONORED(port): UOnlineSubsystemSteamworks::ReadProfileSettings, 2013 rva 0x5ad8f0 (vtable slot 91 =
// +364; the 2012 function at rva 0x5f3040 has no bForceRead parameter, so this is the 2013 body).
// Offline (or for another player) it puts the passed-in settings object back to its defaults, reports
// success and fires the profile-read delegates. Online it caches the object, empties it, and - when the
// player is logged in and a cloud profile exists - hands the read to the engine; the first-time case
// falls back to defaults plus a write.
// Not ported, all three Steam-only and all three behind code that is still missing: the cloud read itself
// (retail goes through the engine virtual at +324 once DoesProfileExist() says there is a file), the
// WriteProfileSettings follow-up (still a generated stub), and retail's `GEngine && (GEngine@700 & 0x20)`
// gate around the whole caching branch - without that flag retail returns E_FAIL after resetting the
// passed-in object, while this port reports success.
UBOOL UOnlineSubsystemSteamworks::ReadProfileSettings(BYTE LocalUserNum, UOnlineProfileSettings* ProfileSettings, UBOOL bForceRead)
{
	DWORD Return = E_FAIL;

	if (!GSteamworksInitialized || LocalUserNum != LoggedInPlayerNum)
	{
		// DISHONORED(bringup): retail dereferences ProfileSettings unconditionally here; the parameter is
		// script-supplied and optional in practice, so it is checked.
		if (ProfileSettings != NULL)
		{
			ProfileSettings->eventSetToDefaults();
		}
		Return = S_OK;
	}
	else if (CachedProfile == NULL || bForceRead)
	{
		if (ProfileSettings != NULL)
		{
			CachedProfile = ProfileSettings;
			CachedProfile->AsyncState = OPAS_Read;
			CachedProfile->ProfileSettings.Empty();
			CachedProfile->eventSetToDefaults();
			Return = S_OK;
		}
	}
	else if (CachedProfile->AsyncState != OPAS_Read)
	{
		if (CachedProfile != ProfileSettings && ProfileSettings != NULL)
		{
			ProfileSettings->ProfileSettings = CachedProfile->ProfileSettings;
			CachedProfile = ProfileSettings;
		}
		Return = S_OK;
	}

	if (CachedProfile != NULL && LocalUserNum == LoggedInPlayerNum)
	{
		CachedProfile->AsyncState = OPAS_Finished;
	}

	OnlineSubsystemSteamworks_eventOnReadProfileSettingsComplete_Parms Parms(EC_EventParm);
	Parms.LocalUserNum = LocalUserNum;
	Parms.bWasSuccessful = Return == S_OK ? FIRST_BITFIELD : 0;
	TriggerOnlineDelegates(this, ProfileCache.ReadDelegates, &Parms);

	return Return == S_OK;
}

// DISHONORED(port): execReadProfileSettings, 2013 rva 0x5a4370. Arkane's signature has a third parameter
// the 2012 build did not (CodeRed SDK dump UOnlineSubsystemSteamworks_execReadProfileSettings_Params:
// LocalUserNum, ProfileSettings, bForceRead, bool return).
void UOnlineSubsystemSteamworks::execReadProfileSettings( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(LocalUserNum);
	P_GET_OBJECT(UOnlineProfileSettings, ProfileSettings);
	P_GET_UBOOL(bForceRead);
	P_FINISH;
	*(UBOOL*)Result = ReadProfileSettings(LocalUserNum, ProfileSettings, bForceRead);
}

// DISHONORED(port): UOnlineSubsystemSteamworks::ReadAchievements, 2013 rva 0x5aaf90 (2012 rva 0x5f0e10,
// vtable slot 121 = +484). FALSE without Steam or for another player. With Steam: if the user stats are
// neither being read nor already read, mark them in progress and ask
// ISteamUserStats::RequestCurrentStats() (slot 0) - the answer arrives as UserStatsReceived_t and
// OnUserStatsReceived fires the achievement-read delegates; otherwise fire them straight away.
UBOOL UOnlineSubsystemSteamworks::ReadAchievements(BYTE LocalUserNum, INT TitleId, UBOOL bShouldReadText, UBOOL bShouldReadImages)
{
	if (!GSteamworksInitialized || LocalUserNum != LoggedInPlayerNum)
	{
		return FALSE;
	}

#if WITH_STEAMWORKS
	if (UserStatsReceivedState != OERS_InProgress && UserStatsReceivedState != OERS_Done)
	{
		UserStatsReceivedState = OERS_InProgress;
		GSteamUserStats->RequestCurrentStats();
		return TRUE;
	}
#endif

	OnlineSubsystemSteamworks_eventOnReadAchievementsComplete_Parms Parms(EC_EventParm);
	Parms.TitleId = TitleId;
	TriggerOnlineDelegates(this, AchievementReadDelegates, &Parms);
	return TRUE;
}

// DISHONORED(port): execReadAchievements, 2013 rva 0x5a4e50 (CodeRed SDK dump
// UOnlineSubsystemSteamworks_execReadAchievements_Params: LocalUserNum, optional TitleId, optional
// bShouldReadText, optional bShouldReadImages, bool return; the decompile shows bShouldReadText defaulting
// to TRUE).
void UOnlineSubsystemSteamworks::execReadAchievements( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(LocalUserNum);
	P_GET_INT_OPTX(TitleId, 0);
	P_GET_UBOOL_OPTX(bShouldReadText, TRUE);
	P_GET_UBOOL_OPTX(bShouldReadImages, FALSE);
	P_FINISH;
	*(UBOOL*)Result = ReadAchievements(LocalUserNum, TitleId, bShouldReadText, bShouldReadImages);
}
