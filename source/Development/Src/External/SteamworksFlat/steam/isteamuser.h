// isteamuser.h - ISteamUser, the "SteamUser016" interface the shipped steam_api.dll serves
// (version string at steam_api.dll offset 0x13614; the exe reaches it through the exported SteamUser()
// accessor, import `SteamUser` in resources/docs/symbols/imports_2013.csv).
//
// Only the slots the ported engine code calls are declared. Declaring fewer slots than the interface
// really has is safe (the object is never constructed here, only called through), declaring them in the
// wrong order is not - so every slot below is pinned to a retail call site.
#ifndef INC_DISHONORED_STEAM_ISTEAMUSER
#define INC_DISHONORED_STEAM_ISTEAMUSER

#include "steamclientpublic.h"

#pragma pack(push, 8)

class ISteamUser
{
public:
	// DISHONORED(layout): slot 0.
	virtual HSteamUser GetHSteamUser() = 0;
	// DISHONORED(layout): slot 1 = vtable +4, retail InitSteamworks 2013 rva 0x5ac1d0
	// (`(*(GSteamUser + 4))(GSteamUser)` gates the logged-in branch) and SignInLocally 2013 rva 0x5aab40.
	virtual bool BLoggedOn() = 0;
	// DISHONORED(layout): slot 2 = vtable +8, retail InitSteamworks 2013 rva 0x5ac1d0 stores the two
	// returned dwords into LoggedInPlayerId (+240/+244).
	virtual CSteamID GetSteamID() = 0;

	// DISHONORED(written): slots 3.. (InitiateGameConnection, TerminateGameConnection, TrackAppUsageEvent,
	// GetUserDataFolder, the voice block, GetAuthSessionTicket/BeginAuthSession/EndAuthSession,
	// CancelAuthTicket, UserHasLicenseForApp, BIsBehindNAT, AdvertiseGame, RequestEncryptedAppTicket,
	// GetEncryptedAppTicket) are not declared: nothing in this tree calls them and no retail call site
	// pins their order. Add them from a retail decompile before calling any of them.
};

#define STEAMUSER_INTERFACE_VERSION "SteamUser016"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMUSER
