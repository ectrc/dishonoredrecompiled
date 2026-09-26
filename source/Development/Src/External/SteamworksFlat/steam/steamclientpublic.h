// steamclientpublic.h - our own reconstruction of the Steamworks enumerations the Dishonored engine side
// compares against. Evidence per enumerator; nothing copied from Valve's SDK.
// The DLL carries the interface version strings this generation serves (steam_api.dll offsets 0x13480 ..
// 0x139f4: SteamUtils005, STEAMREMOTESTORAGE_INTERFACE_VERSION006, SteamNetworking005,
// STEAMAPPS_INTERFACE_VERSION005, STEAMUSERSTATS_INTERFACE_VERSION010, SteamMatchMakingServers002,
// SteamMatchMaking009, SteamFriends011, SteamUser016, SteamClient012, SteamGameServer011).
#ifndef INC_DISHONORED_STEAM_STEAMCLIENTPUBLIC
#define INC_DISHONORED_STEAM_STEAMCLIENTPUBLIC

#include "steamtypes.h"

#pragma pack(push, 8)

// DISHONORED(layout): only the values the engine side uses are pinned; k_EResultOK == 1 is what every
// Steam callback's m_eResult is compared against.
enum EResult
{
	k_EResultOK = 1,
	k_EResultFail = 2,
	k_EResultNoConnection = 3,
	k_EResultInvalidPassword = 5,
	k_EResultLoggedInElsewhere = 6,
	k_EResultInvalidProtocolVer = 7,
	k_EResultInvalidParam = 8,
	k_EResultFileNotFound = 9,
	k_EResultBusy = 10,
	k_EResultInvalidState = 11,
	k_EResultInvalidName = 12,
	k_EResultInvalidEmail = 13,
	k_EResultDuplicateName = 14,
	k_EResultAccessDenied = 15,
	k_EResultTimeout = 16,
	k_EResultBanned = 17,
	k_EResultAccountNotFound = 18,
	k_EResultInvalidSteamID = 19,
	k_EResultServiceUnavailable = 20,
	k_EResultNotLoggedOn = 21,
	k_EResultPending = 22,
	k_EResultEncryptionFailure = 23,
	k_EResultInsufficientPrivilege = 24,
	k_EResultLimitExceeded = 25,
	k_EResultRevoked = 26,
	k_EResultExpired = 27
};

// DISHONORED(layout): retail's GetFriendsList (2013 rva 0x5adaa0) sets FOnlineFriend::bIsOnline from
// `GetFriendPersonaState(Id) > k_EPersonaStateOffline`, so offline must be 0.
enum EPersonaState
{
	k_EPersonaStateOffline = 0,
	k_EPersonaStateOnline = 1,
	k_EPersonaStateBusy = 2,
	k_EPersonaStateAway = 3,
	k_EPersonaStateSnooze = 4,
	k_EPersonaStateLookingToTrade = 5,
	k_EPersonaStateLookingToPlay = 6,
	k_EPersonaStateMax
};

// DISHONORED(layout): k_EFriendRelationshipFriend == 3 is pinned by retail's IsFriend
// (2013 rva 0x5a5920: `GetFriendRelationship(...) == 3`).
enum EFriendRelationship
{
	k_EFriendRelationshipNone = 0,
	k_EFriendRelationshipBlocked = 1,
	k_EFriendRelationshipRequestRecipient = 2,
	k_EFriendRelationshipFriend = 3,
	k_EFriendRelationshipRequestInitiator = 4,
	k_EFriendRelationshipIgnored = 5,
	k_EFriendRelationshipIgnoredFriend = 6,
	k_EFriendRelationshipSuggested = 7,
	k_EFriendRelationshipMax
};

// DISHONORED(layout): k_EFriendFlagImmediate == 4 is pinned by retail's GetFriendsList
// (2013 rva 0x5adaa0: `GetFriendCount(4)` and `GetFriendByIndex(i, 4)`).
enum EFriendFlags
{
	k_EFriendFlagNone = 0x00,
	k_EFriendFlagBlocked = 0x01,
	k_EFriendFlagFriendshipRequested = 0x02,
	k_EFriendFlagImmediate = 0x04,
	k_EFriendFlagClanMember = 0x08,
	k_EFriendFlagOnGameServer = 0x10,
	k_EFriendFlagRequestingFriendship = 0x80,
	k_EFriendFlagRequestingInfo = 0x100,
	k_EFriendFlagIgnored = 0x200,
	k_EFriendFlagIgnoredFriend = 0x400,
	k_EFriendFlagSuggested = 0x800,
	k_EFriendFlagAll = 0xFFFF
};

enum ENotificationPosition
{
	k_EPositionTopLeft = 0,
	k_EPositionTopRight = 1,
	k_EPositionBottomLeft = 2,
	k_EPositionBottomRight = 3
};

enum EAccountFlags
{
	k_EAccountFlagNormalUser = 0
};

// DISHONORED(layout): the string length limits the interfaces document; only the ones a reconstructed
// callback struct needs are kept. k_cchMaxRichPresenceValueLength = 256 is pinned by the retail
// callback size: CCallback<...,GameRichPresenceJoinRequested_t,0>::GetCallbackSizeBytes returns 264
// (2012 rva 0x5ead20) = sizeof(CSteamID) + 256.
enum { k_cchPersonaNameMax = 128 };
enum { k_cchMaxRichPresenceKeys = 20 };
enum { k_cchMaxRichPresenceKeyLength = 64 };
enum { k_cchMaxRichPresenceValueLength = 256 };
enum { k_cchStatNameMax = 128 };
enum { k_cchGameExtraInfoMax = 64 };

// DISHONORED(layout): the fixed-size buffer GameServerChangeRequested_t carries; the same
// GetCallbackSizeBytes family gives 128 for that callback (2012 rva 0x66add0, two 64-byte buffers).
enum { k_cbMaxGameServerGameData = 2048 };

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_STEAMCLIENTPUBLIC
