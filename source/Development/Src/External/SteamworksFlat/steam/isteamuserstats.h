// isteamuserstats.h - ISteamUserStats, the "STEAMUSERSTATS_INTERFACE_VERSION010" interface the shipped
// steam_api.dll serves (version string at steam_api.dll offset 0x135b0; reached through the exported
// SteamUserStats() accessor).
//
// Slots 0..13 are declared: the achievement block the ported natives need. Slot 0 is pinned by retail,
// the rest follow the order this interface generation documents; the leaderboard and global-stats tail
// (slots 14..) is deliberately not declared.
#ifndef INC_DISHONORED_STEAM_ISTEAMUSERSTATS
#define INC_DISHONORED_STEAM_ISTEAMUSERSTATS

#include "steamclientpublic.h"

#pragma pack(push, 8)

class ISteamUserStats
{
public:
	// DISHONORED(layout): slot 0 = vtable +0, retail ReadAchievements 2013 rva 0x5aaf90 calls it as
	// `(**GSteamUserStats)(GSteamUserStats)` right after setting UserStatsReceivedState to OERS_InProgress.
	virtual bool RequestCurrentStats() = 0;
	virtual bool GetStat(const char* pchName, int32* pData) = 0;
	virtual bool GetStat(const char* pchName, float* pData) = 0;
	virtual bool SetStat(const char* pchName, int32 nData) = 0;
	virtual bool SetStat(const char* pchName, float fData) = 0;
	virtual bool UpdateAvgRateStat(const char* pchName, float flCountThisSession, double dSessionLength) = 0;
	// DISHONORED(layout): slot 6 = vtable +24, the reference UnlockAchievement / LoadAchievementDetails
	// pair (Src/OnlineSubsystemSteamworks.cpp) reads an achievement's unlocked flag here.
	virtual bool GetAchievement(const char* pchName, bool* pbAchieved) = 0;
	virtual bool SetAchievement(const char* pchName) = 0;
	virtual bool ClearAchievement(const char* pchName) = 0;
	virtual bool GetAchievementAndUnlockTime(const char* pchName, bool* pbAchieved, uint32* punUnlockTime) = 0;
	virtual bool StoreStats() = 0;
	virtual int GetAchievementIcon(const char* pchName) = 0;
	virtual const char* GetAchievementDisplayAttribute(const char* pchName, const char* pchKey) = 0;
	virtual bool IndicateAchievementProgress(const char* pchName, uint32 nCurProgress, uint32 nMaxProgress) = 0;

	// DISHONORED(written): slots 14.. (RequestUserStats, GetUserStat/GetUserAchievement, ResetAllStats,
	// the leaderboard block, GetNumberOfCurrentPlayers, the global-stats block) are not declared. Retail
	// reaches the leaderboard ones through its own helper object (2013 ctor rva 0x5ab620, CCallResults for
	// callbacks 1104/1105/1106) which is out of this package's scope.
};

#define STEAMUSERSTATS_INTERFACE_VERSION "STEAMUSERSTATS_INTERFACE_VERSION010"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMUSERSTATS
