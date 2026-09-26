// isteamapps.h - ISteamApps, the "STEAMAPPS_INTERFACE_VERSION005" interface the shipped steam_api.dll
// serves (version string at steam_api.dll offset 0x13590; reached through the exported SteamApps()
// accessor).
#ifndef INC_DISHONORED_STEAM_ISTEAMAPPS
#define INC_DISHONORED_STEAM_ISTEAMAPPS

#include "steamclientpublic.h"

#pragma pack(push, 8)

class ISteamApps
{
public:
	virtual bool BIsSubscribed() = 0;
	// DISHONORED(layout): slot 1 = vtable +4, retail InitSteamworks 2013 rva 0x5ac1d0 caches the result
	// in a global right after creating the callback bridge.
	virtual bool BIsLowViolence() = 0;
	virtual bool BIsCybercafe() = 0;
	virtual bool BIsVACBanned() = 0;
	virtual const char* GetCurrentGameLanguage() = 0;
	virtual const char* GetAvailableGameLanguages() = 0;
	virtual bool BIsSubscribedApp(AppId_t appID) = 0;
	// DISHONORED(layout): slot 7 = vtable +28, the DLC ownership check retail's CheckDLCOwnership
	// (2013 rva 0x5a8960) is built on (`Req_DLC05_*`, resources/docs/middleware.md 2.6).
	virtual bool BIsDlcInstalled(AppId_t appID) = 0;
	virtual uint32 GetEarliestPurchaseUnixTime(AppId_t nAppID) = 0;
	virtual bool BIsSubscribedFromFreeWeekend() = 0;
	virtual int GetDLCCount() = 0;
	virtual bool BGetDLCDataByIndex(int iDLC, AppId_t* pAppID, bool* pbAvailable, char* pchName, int cchNameBufferSize) = 0;
	virtual void InstallDLC(AppId_t nAppID) = 0;
	virtual void UninstallDLC(AppId_t nAppID) = 0;
};

#define STEAMAPPS_INTERFACE_VERSION "STEAMAPPS_INTERFACE_VERSION005"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMAPPS
