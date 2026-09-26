// isteamfriends.h - ISteamFriends, the "SteamFriends011" interface the shipped steam_api.dll serves
// (version string at steam_api.dll offset 0x13604; reached through the exported SteamFriends() accessor).
//
// Slots 0..36 are declared because retail calls slot 36 (SetRichPresence). Eleven of the 37 slots are
// pinned directly by retail call sites (see the per-slot tags); an inserted or dropped method anywhere in
// between would move those, so the whole run is validated by them.
#ifndef INC_DISHONORED_STEAM_ISTEAMFRIENDS
#define INC_DISHONORED_STEAM_ISTEAMFRIENDS

#include "steamclientpublic.h"

#pragma pack(push, 8)

// DISHONORED(layout): 24 bytes; ISteamFriends::GetFriendGamePlayed fills it. Retail's GetFriendsList
// (2013 rva 0x5adaa0) reads m_gameID's AppId (compared against GSteamAppID), m_unGameIP, m_usGamePort
// and m_usQueryPort out of it.
struct FriendGameInfo_t
{
	CGameID m_gameID;
	uint32 m_unGameIP;
	uint16 m_usGamePort;
	uint16 m_usQueryPort;
	CSteamID m_steamIDLobby;
};

class ISteamFriends
{
public:
	// DISHONORED(layout): slot 0 = vtable +0, retail InitSteamworks 2013 rva 0x5ac1d0
	// (`(**GSteamFriends)(GSteamFriends)` -> LoggedInPlayerName).
	virtual const char* GetPersonaName() = 0;
	virtual void SetPersonaName(const char* pchPersonaName) = 0;
	virtual EPersonaState GetPersonaState() = 0;
	// DISHONORED(layout): slot 3 = vtable +12, retail GetFriendsList 2013 rva 0x5adaa0
	// (`GetFriendCount(k_EFriendFlagImmediate)`).
	virtual int GetFriendCount(int iFriendFlags) = 0;
	// DISHONORED(layout): slot 4 = vtable +16, retail GetFriendsList 2013 rva 0x5adaa0
	// (`GetFriendByIndex(SteamFriendIndex, k_EFriendFlagImmediate)`, hidden return pointer).
	virtual CSteamID GetFriendByIndex(int iFriend, int iFriendFlags) = 0;
	// DISHONORED(layout): slot 5 = vtable +20, retail IsFriend 2013 rva 0x5a5920
	// (`GetFriendRelationship(Id) == k_EFriendRelationshipFriend`).
	virtual EFriendRelationship GetFriendRelationship(CSteamID steamIDFriend) = 0;
	// DISHONORED(layout): slot 6 = vtable +24, retail GetFriendsList 2013 rva 0x5adaa0.
	virtual EPersonaState GetFriendPersonaState(CSteamID steamIDFriend) = 0;
	// DISHONORED(layout): slot 7 = vtable +28, retail GetFriendsList 2013 rva 0x5adaa0.
	virtual const char* GetFriendPersonaName(CSteamID steamIDFriend) = 0;
	// DISHONORED(layout): slot 8 = vtable +32, retail GetFriendsList 2013 rva 0x5adaa0.
	virtual bool GetFriendGamePlayed(CSteamID steamIDFriend, FriendGameInfo_t* pFriendGameInfo) = 0;
	virtual const char* GetFriendPersonaNameHistory(CSteamID steamIDFriend, int iPersonaName) = 0;
	virtual bool HasFriend(CSteamID steamIDFriend, int iFriendFlags) = 0;
	virtual int GetClanCount() = 0;
	virtual CSteamID GetClanByIndex(int iClan) = 0;
	virtual const char* GetClanName(CSteamID steamIDClan) = 0;
	virtual const char* GetClanTag(CSteamID steamIDClan) = 0;
	virtual bool GetClanActivityCounts(CSteamID steamIDClan, int* pnOnline, int* pnInGame, int* pnChatting) = 0;
	virtual SteamAPICall_t DownloadClanActivityCounts(CSteamID* psteamIDClans, int cClansToRequest) = 0;
	virtual int GetFriendCountFromSource(CSteamID steamIDSource) = 0;
	virtual CSteamID GetFriendFromSourceByIndex(CSteamID steamIDSource, int iFriend) = 0;
	virtual bool IsUserInSource(CSteamID steamIDUser, CSteamID steamIDSource) = 0;
	virtual void SetInGameVoiceSpeaking(CSteamID steamIDUser, bool bSpeaking) = 0;
	virtual void ActivateGameOverlay(const char* pchDialog) = 0;
	virtual void ActivateGameOverlayToUser(const char* pchDialog, CSteamID steamID) = 0;
	virtual void ActivateGameOverlayToWebPage(const char* pchURL) = 0;
	virtual void ActivateGameOverlayToStore(AppId_t nAppID) = 0;
	virtual void SetPlayedWith(CSteamID steamIDUserPlayedWith) = 0;
	virtual void ActivateGameOverlayInviteDialog(CSteamID steamIDLobby) = 0;
	// DISHONORED(layout): slots 27/28/29 = vtable +108/+112/+116, retail GetOnlineAvatar 2013 rva
	// 0x5a92d0 picks small (< 64 px), medium (< 184 px) or large by requested size.
	virtual int GetSmallFriendAvatar(CSteamID steamIDFriend) = 0;
	virtual int GetMediumFriendAvatar(CSteamID steamIDFriend) = 0;
	virtual int GetLargeFriendAvatar(CSteamID steamIDFriend) = 0;
	virtual bool RequestUserInformation(CSteamID steamIDUser, bool bRequireNameOnly) = 0;
	virtual SteamAPICall_t RequestClanOfficerList(CSteamID steamIDClan) = 0;
	virtual CSteamID GetClanOwner(CSteamID steamIDClan) = 0;
	virtual int GetClanOfficerCount(CSteamID steamIDClan) = 0;
	virtual CSteamID GetClanOfficerByIndex(CSteamID steamIDClan, int iOfficer) = 0;
	virtual uint32 GetUserRestrictions() = 0;
	// DISHONORED(layout): slot 36 = vtable +144, retail UOnlineSubsystemSteamworks::Init 2013 rva
	// 0x5ad3d0 (`SetRichPresence("0","Dud")`).
	virtual bool SetRichPresence(const char* pchKey, const char* pchValue) = 0;

	// DISHONORED(written): slots 37.. (ClearRichPresence, GetFriendRichPresence*, InviteUserToGame, the
	// coplay block) are not declared - no retail call site in this tree pins them.
};

#define STEAMFRIENDS_INTERFACE_VERSION "SteamFriends011"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMFRIENDS
