// steamtypes.h - our own reconstruction of the Steamworks SDK scalar types, handles and id classes.
// Nothing here comes from Valve's SDK: every type is derived from what the shipped
// D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\steam_api.dll (1.30.50.46, PE timestamp
// 2012-02-03) exports and from the retail 2013 Dishonored.exe / 2012 PDB that call into it.
// See resources/docs/middleware.md section 2.6 and resources/docs/agents/agentAM.md.
#ifndef INC_DISHONORED_STEAM_STEAMTYPES
#define INC_DISHONORED_STEAM_STEAMTYPES

// DISHONORED(layout): 8-byte packing, measured, not assumed. The retail exe's
// CCallback<SteamCallbackBridge,GSClientAchievementStatus_t,1>::GetCallbackSizeBytes returns 144
// (2012 rva 0x5ead40) for `uint64 + char[128] + bool` = 137 payload bytes: only 8-byte alignment rounds
// 137 up to 144 (4-byte alignment would give 140). The SDK's steamtypes.h opens VALVE_CALLBACK_PACK_LARGE
// (#pragma pack(push,8)) on Windows and every isteam*.h / callback struct is compiled inside it, and the
// engine here builds with /Zp4 (cmake/DishonoredDefines.cmake), so the pack has to be forced in these
// headers or every callback struct and every interface argument with a 64-bit member would be laid out
// differently from the shipped DLL.
#pragma pack(push, 8)

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef int64 lint64;
typedef uint64 ulint64;

typedef uint32 AppId_t;
const AppId_t k_uAppIdInvalid = 0;

typedef uint32 DepotId_t;
typedef uint32 AccountID_t;

// DISHONORED(layout): SteamAPI_RegisterCallResult / SteamAPI_UnregisterCallResult take the handle as two
// dwords (2012 CCallResult::Set rva 0x5ea610), i.e. a 64-bit by-value handle.
typedef uint64 SteamAPICall_t;
const SteamAPICall_t k_uAPICallInvalid = 0;

typedef int32 HSteamPipe;
typedef int32 HSteamUser;

typedef uint64 SteamLeaderboard_t;
typedef uint64 UGCHandle_t;
const UGCHandle_t k_UGCHandleInvalid = 0xffffffffffffffffull;

typedef void (*SteamAPIWarningMessageHook_t)(int, const char*);

// DISHONORED(layout): EUniverse / EAccountType are only needed for the CSteamID constructors the engine
// side uses; the values are the ones the DLL's own steamclientpublic.h strings imply and retail never
// compares against anything but k_EAccountTypeIndividual.
enum EUniverse
{
	k_EUniverseInvalid = 0,
	k_EUniversePublic = 1,
	k_EUniverseBeta = 2,
	k_EUniverseInternal = 3,
	k_EUniverseDev = 4,
	k_EUniverseMax
};

enum EAccountType
{
	k_EAccountTypeInvalid = 0,
	k_EAccountTypeIndividual = 1,
	k_EAccountTypeMultiseat = 2,
	k_EAccountTypeGameServer = 3,
	k_EAccountTypeAnonGameServer = 4,
	k_EAccountTypePending = 5,
	k_EAccountTypeContentServer = 6,
	k_EAccountTypeClan = 7,
	k_EAccountTypeChat = 8,
	k_EAccountTypeConsoleUser = 9,
	k_EAccountTypeAnonUser = 10,
	k_EAccountTypeMax
};

// DISHONORED(layout): 8 bytes passed and returned by value. Retail passes it as two dwords into
// ISteamFriends::GetFriendRelationship (2013 rva 0x5a5920: `GSteamFriends + 20` called with (a3, a4))
// and receives it from ISteamUser::GetSteamID through a hidden return pointer (2013 rva 0x5ac1d0:
// `(*(GSteamUser + 8))(GSteamUser, v17)` then two dwords copied into LoggedInPlayerId at +240/+244),
// which is exactly how MSVC treats an 8-byte class with a user-declared constructor.
class CSteamID
{
public:
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_unAccountInstance = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeInvalid;
		m_steamid.m_comp.m_EUniverse = k_EUniverseInvalid;
	}

	explicit CSteamID(uint64 ulSteamID) { m_steamid.m_unAll64Bits = ulSteamID; }

	CSteamID(AccountID_t unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_unAccountInstance = (eAccountType == k_EAccountTypeClan || eAccountType == k_EAccountTypeGameServer) ? 0 : 1;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;
	}

	uint64 ConvertToUint64() const { return m_steamid.m_unAll64Bits; }
	void SetFromUint64(uint64 ulSteamID) { m_steamid.m_unAll64Bits = ulSteamID; }
	AccountID_t GetAccountID() const { return m_steamid.m_comp.m_unAccountID; }
	EAccountType GetEAccountType() const { return (EAccountType)m_steamid.m_comp.m_EAccountType; }
	EUniverse GetEUniverse() const { return (EUniverse)m_steamid.m_comp.m_EUniverse; }
	bool IsValid() const { return m_steamid.m_comp.m_EAccountType != k_EAccountTypeInvalid && m_steamid.m_comp.m_EUniverse != k_EUniverseInvalid; }

	bool operator==(const CSteamID& Other) const { return m_steamid.m_unAll64Bits == Other.m_steamid.m_unAll64Bits; }
	bool operator!=(const CSteamID& Other) const { return m_steamid.m_unAll64Bits != Other.m_steamid.m_unAll64Bits; }

private:
	union SteamID_t
	{
		struct SteamIDComponent_t
		{
			uint32 m_unAccountID : 32;
			unsigned int m_unAccountInstance : 20;
			unsigned int m_EAccountType : 4;
			EUniverse m_EUniverse : 8;
		} m_comp;
		uint64 m_unAll64Bits;
	} m_steamid;
};

// DISHONORED(layout): 8 bytes, the low 24 bits are the AppId. Retail compares the AppId of a friend's
// CGameID against GSteamAppID in GetFriendsList (2013 rva 0x5adaa0, the `dword_145B184` compare).
class CGameID
{
public:
	CGameID() { m_ulGameID = 0; }
	explicit CGameID(uint64 ulGameID) { m_ulGameID = ulGameID; }
	explicit CGameID(AppId_t nAppID) { m_ulGameID = 0; m_gameID.m_nAppID = nAppID; }

	uint64 ToUint64() const { return m_ulGameID; }
	AppId_t AppID() const { return m_gameID.m_nAppID; }
	bool IsValid() const { return m_gameID.m_nAppID != k_uAppIdInvalid; }

	bool operator==(const CGameID& Other) const { return m_ulGameID == Other.m_ulGameID; }

private:
	enum EGameIDType
	{
		k_EGameIDTypeApp = 0,
		k_EGameIDTypeGameMod = 1,
		k_EGameIDTypeShortcut = 2,
		k_EGameIDTypeP2P = 3
	};

	struct GameID_t
	{
		unsigned int m_nAppID : 24;
		unsigned int m_nType : 8;
		unsigned int m_nModID : 32;
	};

	union
	{
		uint64 m_ulGameID;
		GameID_t m_gameID;
	};
};

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_STEAMTYPES
