// steam_api.h - the flat surface of the shipped steam_api.dll, reconstructed for this tree.
//
// Evidence base:
//   * the DLL's own export table (56 names, `dumpbin /exports`, build/agentU/steam_api_exports.txt).
//     This generation still exports the SteamUser()-style accessors, so no SteamInternal_CreateInterface
//     and no SteamAPI_ISteam* flat wrapper exists: the engine calls the interface vtables directly.
//   * the 18 names the retail 2013 exe imports (resources/docs/symbols/imports_2013.csv): they are
//     undecorated cdecl (`__imp__SteamUser`, ...), so a plain extern "C" declaration matches.
//   * the CCallback / CCallResult plumbing pinned by the 2012 PDB, which kept the SDK's own
//     external\steamworks\sdk\public\steam\steam_api.h line numbers for the template bodies.
//
// Nothing here is copied from Valve's SDK.
#ifndef INC_DISHONORED_STEAM_STEAM_API
#define INC_DISHONORED_STEAM_STEAM_API

#include "steamclientpublic.h"
#include "isteamuser.h"
#include "isteamfriends.h"
#include "isteamutils.h"
#include "isteamuserstats.h"
#include "isteamapps.h"
#include "isteamremotestorage.h"
#include "isteamnetworking.h"

#pragma pack(push, 8)

// DISHONORED(written): the interfaces this tree never calls into. Retail fetches the pointers in
// InitSteamworks (2013 rva 0x5ac1d0) and only checks them for NULL, so an incomplete type is enough and
// keeps us from inventing unverified vtable slots.
class ISteamClient;
class ISteamMatchmaking;
class ISteamMatchmakingServers;
class ISteamGameServer;
class ISteamGameServerStats;
class ISteamHTTP;
class ISteamScreenshots;

#ifdef __cplusplus
extern "C" {
#endif

// DISHONORED(layout): the exported flat entry points, all cdecl and undecorated. The subset below is the
// 18 the retail exe imports plus SteamAPI_RestartAppIfNecessary, which the 2012 exe imports as well and
// which appSteamInit still calls (2012 rva 0x5eebb0).
bool __cdecl SteamAPI_Init();
bool __cdecl SteamAPI_InitSafe();
void __cdecl SteamAPI_Shutdown();
bool __cdecl SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID);
void __cdecl SteamAPI_RunCallbacks();
bool __cdecl SteamAPI_IsSteamRunning();
HSteamPipe __cdecl SteamAPI_GetHSteamPipe();
HSteamUser __cdecl SteamAPI_GetHSteamUser();
const char* __cdecl SteamAPI_GetSteamInstallPath();
void __cdecl SteamAPI_SetMiniDumpComment(const char* pchMsg);
void __cdecl SteamAPI_WriteMiniDump(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID);

// The interface accessors. Every one of these is an export of the shipped DLL and an import of the
// retail exe; ISteamUserStats & co. come back as the vtable layouts in the isteam*.h next door.
ISteamUser* __cdecl SteamUser();
ISteamFriends* __cdecl SteamFriends();
ISteamUtils* __cdecl SteamUtils();
ISteamUserStats* __cdecl SteamUserStats();
ISteamApps* __cdecl SteamApps();
ISteamRemoteStorage* __cdecl SteamRemoteStorage();
ISteamNetworking* __cdecl SteamNetworking();
ISteamMatchmaking* __cdecl SteamMatchmaking();
ISteamMatchmakingServers* __cdecl SteamMatchmakingServers();
ISteamGameServer* __cdecl SteamGameServer();
void __cdecl SteamGameServer_Shutdown();

#ifdef __cplusplus
}
#endif

// DISHONORED(layout): CCallbackBase is what SteamAPI_RegisterCallback / SteamAPI_RegisterCallResult take.
// Its shape is pinned by the 2012 PDB's CCallback<SteamCallbackBridge,...> instantiations:
//   * three virtuals in this order - Run(void*), Run(void*, bool, SteamAPICall_t), GetCallbackSizeBytes()
//     (2012 rvas 0x5ead30 / 0x5ead50 / 0x5ead20; the CCallResult pair is 0x5ea660 / 0x5ea680)
//   * the two protected fields m_nCallbackFlags (uint8) and m_iCallback (int), in that order, and the
//     k_ECallbackFlagsRegistered = 0x01 bit that Register / the destructor test
//     (2012 CCallback::Register rva 0x5ead70 `if ((m_nCallbackFlags & 1) != 0) SteamAPI_UnregisterCallback(this)`)
//   * CCallback then adds m_pObj and m_Func, so sizeof(CCallback<...>) is 20 with this packing - which is
//     what retail's SteamCallbackBridge measures: appMalloc(244) for 1 pointer + 12 CCallbacks
//     (2013 InitSteamworks rva 0x5ac1d0, ctor rva 0x5ab400 writing members at +4, +24, ... +224).
class CCallbackBase
{
public:
	CCallbackBase()
	{
		m_nCallbackFlags = 0;
		m_iCallback = 0;
	}

	virtual void Run(void* pvParam) = 0;
	virtual void Run(void* pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall) = 0;
	virtual int GetCallbackSizeBytes() = 0;

	int GetICallback() const { return m_iCallback; }

	enum { k_ECallbackFlagsRegistered = 0x01, k_ECallbackFlagsGameServer = 0x02 };

protected:
	uint8 m_nCallbackFlags;
	int m_iCallback;
};

#ifdef __cplusplus
extern "C" {
#endif

// DISHONORED(layout): the four callback-manager entry points, with the argument order the 2012 decompiles
// show: RegisterCallback(base, iCallback), UnregisterCallback(base),
// RegisterCallResult(base, hAPICall), UnregisterCallResult(base, hAPICall).
void __cdecl SteamAPI_RegisterCallback(CCallbackBase* pCallback, int iCallback);
void __cdecl SteamAPI_UnregisterCallback(CCallbackBase* pCallback);
void __cdecl SteamAPI_RegisterCallResult(CCallbackBase* pCallback, SteamAPICall_t hAPICall);
void __cdecl SteamAPI_UnregisterCallResult(CCallbackBase* pCallback, SteamAPICall_t hAPICall);

#ifdef __cplusplus
}
#endif

// DISHONORED(port): CCallback<T, P, bGameServer>, the 2012 shape (steam_api.h:275 Register,
// :301 Run(void*), :305 Run(void*, bool, SteamAPICall_t), :309 GetCallbackSizeBytes per the PDB line
// numbers of 0x5ead70 / 0x5ead30 / 0x5ead50 / 0x5ead20). The Register body is the decompile of 0x5ead70.
template<class T, class P, bool bGameServer>
class CCallback : public CCallbackBase
{
public:
	typedef void (T::*func_t)(P*);

	CCallback()
	{
		m_pObj = NULL;
		m_Func = NULL;
		if (bGameServer)
		{
			m_nCallbackFlags |= k_ECallbackFlagsGameServer;
		}
	}

	CCallback(T* pObj, func_t func)
	{
		m_pObj = NULL;
		m_Func = NULL;
		if (bGameServer)
		{
			m_nCallbackFlags |= k_ECallbackFlagsGameServer;
		}
		Register(pObj, func);
	}

	~CCallback() { Unregister(); }

	void Register(T* pObj, func_t func)
	{
		if (pObj == NULL || func == NULL)
		{
			return;
		}
		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
		{
			SteamAPI_UnregisterCallback(this);
		}
		m_pObj = pObj;
		m_Func = func;
		SteamAPI_RegisterCallback(this, P::k_iCallback);
	}

	void Unregister()
	{
		if (m_nCallbackFlags & k_ECallbackFlagsRegistered)
		{
			SteamAPI_UnregisterCallback(this);
		}
	}

protected:
	virtual void Run(void* pvParam) { (m_pObj->*m_Func)((P*)pvParam); }
	virtual void Run(void* pvParam, bool /*bIOFailure*/, SteamAPICall_t /*hSteamAPICall*/) { (m_pObj->*m_Func)((P*)pvParam); }
	virtual int GetCallbackSizeBytes() { return sizeof(P); }

	T* m_pObj;
	func_t m_Func;
};

// DISHONORED(port): CCallResult<T, P>, the 2012 shape (Set 0x5ea610 = steam_api.h:185, Run(void*)
// 0x5ea660 = :220, Run(void*, bool, SteamAPICall_t) 0x5ea680 = :225). The constructor sets m_iCallback to
// P::k_iCallback, as retail's bridge shows (NumberOfCurrentPlayersCallback.m_iCallback = 1107, 2012 ctor
// rva 0x5f13f0).
template<class T, class P>
class CCallResult : public CCallbackBase
{
public:
	typedef void (T::*func_t)(P*, bool);

	CCallResult()
	{
		m_hAPICall = k_uAPICallInvalid;
		m_pObj = NULL;
		m_Func = NULL;
		m_iCallback = P::k_iCallback;
	}

	~CCallResult() { Cancel(); }

	void Set(SteamAPICall_t hAPICall, T* p, func_t func)
	{
		if (m_hAPICall != k_uAPICallInvalid)
		{
			SteamAPI_UnregisterCallResult(this, m_hAPICall);
		}
		m_pObj = p;
		m_Func = func;
		m_hAPICall = hAPICall;
		if (hAPICall != k_uAPICallInvalid)
		{
			SteamAPI_RegisterCallResult(this, hAPICall);
		}
	}

	bool IsActive() const { return m_hAPICall != k_uAPICallInvalid; }

	void Cancel()
	{
		if (m_hAPICall != k_uAPICallInvalid)
		{
			SteamAPI_UnregisterCallResult(this, m_hAPICall);
			m_hAPICall = k_uAPICallInvalid;
		}
	}

private:
	virtual void Run(void* pvParam)
	{
		m_hAPICall = k_uAPICallInvalid;
		(m_pObj->*m_Func)((P*)pvParam, false);
	}

	virtual void Run(void* pvParam, bool bIOFailure, SteamAPICall_t hSteamAPICall)
	{
		if (hSteamAPICall == m_hAPICall)
		{
			m_hAPICall = k_uAPICallInvalid;
			(m_pObj->*m_Func)((P*)pvParam, bIOFailure);
		}
	}

	virtual int GetCallbackSizeBytes() { return sizeof(P); }

	SteamAPICall_t m_hAPICall;
	T* m_pObj;
	func_t m_Func;
};

// DISHONORED(layout): the callback ids. The bases are fixed by the two retail numbers we can read
// straight out of the exe - UserStatsReceived_t = 1101 (2012 CCallback::Register rva 0x5ead70 passes
// 1101) and NumberOfCurrentPlayers_t = 1107 (2012 bridge ctor rva 0x5f13f0 / 2013 leaderboard helper
// 0x5ab620 which sets 1104, 1105, 1106 for the three leaderboard call results) - so the user-stats base
// is 1100 and the ids run in the interface's documented order.
enum
{
	k_iSteamUserCallbacks = 100,
	k_iSteamGameServerCallbacks = 200,
	k_iSteamFriendsCallbacks = 300,
	k_iSteamUtilsCallbacks = 700,
	k_iSteamAppsCallbacks = 1000,
	k_iSteamUserStatsCallbacks = 1100,
	k_iSteamNetworkingCallbacks = 1200,
	k_iSteamRemoteStorageCallbacks = 1300
};

// ISteamUser callbacks (base 100), the two retail's bridge registers.
struct SteamServersConnected_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 1 };
};

struct SteamServersDisconnected_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 3 };
	EResult m_eResult;
};

// DISHONORED(layout): 128 bytes, two 64-byte buffers - the value
// CCallback<SteamCallbackBridge,GameServerChangeRequested_t,0>::GetCallbackSizeBytes returns
// (2012 rva 0x66add0).
struct GameServerChangeRequested_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 32 };
	// sizeof must stay 128 (2012 rva 0x66add0).
	char m_rgchServer[64];
	char m_rgchPassword[64];
};

// ISteamFriends callbacks (base 300).
struct PersonaStateChange_t
{
	enum { k_iCallback = k_iSteamFriendsCallbacks + 4 };
	uint64 m_ulSteamID;
	int m_nChangeFlags;
};

struct GameOverlayActivated_t
{
	enum { k_iCallback = k_iSteamFriendsCallbacks + 31 };
	uint8 m_bActive;
};

// DISHONORED(layout): sizeof == 264 = sizeof(CSteamID) + k_cchMaxRichPresenceValueLength, the value
// CCallback<SteamCallbackBridge,GameRichPresenceJoinRequested_t,0>::GetCallbackSizeBytes returns
// (2012 rva 0x5ead20).
struct GameRichPresenceJoinRequested_t
{
	enum { k_iCallback = k_iSteamFriendsCallbacks + 37 };
	CSteamID m_steamIDFriend;
	char m_rgchConnect[k_cchMaxRichPresenceValueLength];
};

struct AvatarImageLoaded_t
{
	enum { k_iCallback = k_iSteamFriendsCallbacks + 34 };
	CSteamID m_steamID;
	int m_iImage;
	int m_iWide;
	int m_iTall;
};

// ISteamUtils callbacks (base 700).
struct SteamShutdown_t
{
	enum { k_iCallback = k_iSteamUtilsCallbacks + 4 };
};

// ISteamApps callbacks (base 1000).
struct DlcInstalled_t
{
	enum { k_iCallback = k_iSteamAppsCallbacks + 5 };
	AppId_t m_nAppID;
};

// ISteamUserStats callbacks (base 1100).
struct UserStatsReceived_t
{
	enum { k_iCallback = k_iSteamUserStatsCallbacks + 1 };
	uint64 m_nGameID;
	EResult m_eResult;
	CSteamID m_steamIDUser;
};

struct UserStatsStored_t
{
	enum { k_iCallback = k_iSteamUserStatsCallbacks + 2 };
	uint64 m_nGameID;
	EResult m_eResult;
};

struct UserAchievementStored_t
{
	enum { k_iCallback = k_iSteamUserStatsCallbacks + 3 };
	uint64 m_nGameID;
	bool m_bGroupAchievement;
	char m_rgchAchievementName[k_cchStatNameMax];
	uint32 m_nCurProgress;
	uint32 m_nMaxProgress;
};

struct NumberOfCurrentPlayers_t
{
	enum { k_iCallback = k_iSteamUserStatsCallbacks + 7 };
	uint8 m_bSuccess;
	int32 m_cPlayers;
};

// DISHONORED(written): the game-server callbacks retail's bridge holds slots for but never registers
// (2013 ctor rva 0x5ab400 leaves m_Func == NULL for GSPolicyResponse_t, SteamServersConnected_t and
// SteamServersDisconnected_t on the game-server side). Declared so the bridge can mirror retail's member
// set. sizeof(GSClientAchievementStatus_t) must be 144, the value
// CCallback<SteamCallbackBridge,GSClientAchievementStatus_t,1>::GetCallbackSizeBytes returns
// (2012 rva 0x5ead40) - that is the measurement the 8-byte pack in steamtypes.h rests on.
struct GSPolicyResponse_t
{
	enum { k_iCallback = k_iSteamGameServerCallbacks + 15 };
	uint8 m_bSecure;
};

struct GSClientAchievementStatus_t
{
	enum { k_iCallback = k_iSteamGameServerCallbacks + 6 };
	uint64 m_SteamID;
	char m_pchAchievement[128];
	bool m_bUnlocked;
};

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_STEAM_API
