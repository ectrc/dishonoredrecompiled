// isteamutils.h - ISteamUtils, the "SteamUtils005" interface the shipped steam_api.dll serves
// (version string at steam_api.dll offset 0x13480; reached through the exported SteamUtils() accessor).
#ifndef INC_DISHONORED_STEAM_ISTEAMUTILS
#define INC_DISHONORED_STEAM_ISTEAMUTILS

#include "steamclientpublic.h"

#pragma pack(push, 8)

class ISteamUtils
{
public:
	virtual uint32 GetSecondsSinceAppActive() = 0;
	virtual uint32 GetSecondsSinceComputerActive() = 0;
	virtual EUniverse GetConnectedUniverse() = 0;
	virtual uint32 GetServerRealTime() = 0;
	virtual const char* GetIPCountry() = 0;
	// DISHONORED(layout): slots 5/6 = vtable +20/+24, the pair the avatar and achievement-icon loaders
	// use (reference LoadSteamImageToTexture2D, Src/OnlineSubsystemSteamworks.cpp).
	virtual bool GetImageSize(int iImage, uint32* pnWidth, uint32* pnHeight) = 0;
	virtual bool GetImageRGBA(int iImage, uint8* pubDest, int nDestBufferSize) = 0;
	virtual bool GetCSERIPPort(uint32* unIP, uint16* usPort) = 0;
	virtual uint8 GetCurrentBatteryPower() = 0;
	// DISHONORED(layout): slot 9 = vtable +36, retail InitSteamworks 2013 rva 0x5ac1d0 stores the result
	// in GSteamAppID (2013 global 0x105b184).
	virtual uint32 GetAppID() = 0;
	virtual void SetOverlayNotificationPosition(ENotificationPosition eNotificationPosition) = 0;
	// DISHONORED(layout): slots 11/12/13 = vtable +44/+48/+52, the polling form of a call result.
	virtual bool IsAPICallCompleted(SteamAPICall_t hSteamAPICall, bool* pbFailed) = 0;
	virtual int GetAPICallFailureReason(SteamAPICall_t hSteamAPICall) = 0;
	virtual bool GetAPICallResult(SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed) = 0;
	virtual void RunFrame() = 0;
	virtual uint32 GetIPCCallCount() = 0;
	// DISHONORED(layout): slot 16 = vtable +64, retail InitSteamworks 2013 rva 0x5ac1d0 installs the
	// warning hook there.
	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) = 0;
	virtual bool IsOverlayEnabled() = 0;
	virtual bool BOverlayNeedsPresent() = 0;
};

#define STEAMUTILS_INTERFACE_VERSION "SteamUtils005"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMUTILS
