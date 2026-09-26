// isteamremotestorage.h - ISteamRemoteStorage, the "STEAMREMOTESTORAGE_INTERFACE_VERSION006" interface
// the shipped steam_api.dll serves (version string at steam_api.dll offset 0x13554; reached through the
// exported SteamRemoteStorage() accessor).
//
// Slots 0..15 are declared, three of them pinned by retail; the UGC tail is not declared.
#ifndef INC_DISHONORED_STEAM_ISTEAMREMOTESTORAGE
#define INC_DISHONORED_STEAM_ISTEAMREMOTESTORAGE

#include "steamclientpublic.h"

#pragma pack(push, 8)

enum ERemoteStoragePlatform
{
	k_ERemoteStoragePlatformNone = 0,
	k_ERemoteStoragePlatformWindows = (1 << 0),
	k_ERemoteStoragePlatformOSX = (1 << 1),
	k_ERemoteStoragePlatformPS3 = (1 << 2),
	k_ERemoteStoragePlatformReserved1 = (1 << 3),
	k_ERemoteStoragePlatformReserved2 = (1 << 4),
	k_ERemoteStoragePlatformAll = 0xffffffff
};

class ISteamRemoteStorage
{
public:
	virtual bool FileWrite(const char* pchFile, const void* pvData, int32 cubData) = 0;
	virtual int32 FileRead(const char* pchFile, void* pvData, int32 cubDataToRead) = 0;
	virtual bool FileForget(const char* pchFile) = 0;
	virtual bool FileDelete(const char* pchFile) = 0;
	virtual SteamAPICall_t FileShare(const char* pchFile) = 0;
	virtual bool SetSyncPlatforms(const char* pchFile, ERemoteStoragePlatform eRemoteStoragePlatform) = 0;
	virtual bool FileExists(const char* pchFile) = 0;
	virtual bool FilePersisted(const char* pchFile) = 0;
	virtual int32 GetFileSize(const char* pchFile) = 0;
	virtual int64 GetFileTimestamp(const char* pchFile) = 0;
	virtual ERemoteStoragePlatform GetSyncPlatforms(const char* pchFile) = 0;
	virtual int32 GetFileCount() = 0;
	virtual const char* GetFileNameAndSize(int iFile, int32* pnFileSizeInBytes) = 0;
	// DISHONORED(layout): slots 13/14/15 = vtable +52/+56/+60, retail InitSteamworks 2013 rva 0x5ac1d0
	// calls IsCloudEnabledForAccount() && IsCloudEnabledForApp() and only then GetQuota(&total, &avail).
	virtual bool GetQuota(int32* pnTotalBytes, int32* puAvailableBytes) = 0;
	virtual bool IsCloudEnabledForAccount() = 0;
	virtual bool IsCloudEnabledForApp() = 0;

	// DISHONORED(written): slots 16.. (SetCloudEnabledForApp, the UGC download block, the published-file
	// block) are not declared.
};

#define STEAMREMOTESTORAGE_INTERFACE_VERSION "STEAMREMOTESTORAGE_INTERFACE_VERSION006"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMREMOTESTORAGE
