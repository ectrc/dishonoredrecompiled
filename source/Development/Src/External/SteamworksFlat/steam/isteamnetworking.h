// isteamnetworking.h - ISteamNetworking, the "SteamNetworking005" interface the shipped steam_api.dll
// serves (version string at steam_api.dll offset 0x1357c; reached through the exported SteamNetworking()
// accessor).
//
// Retail's networking is not in this package's scope: only the slots up to the one retail's
// InitSteamworks touches are declared. WITH_STEAMWORKS_SOCKETS stays 0 (see cmake/Steamworks.cmake:
// neither the 2012 PDB nor the retail 2013 exe has a single UnSocketSteamworks / UnNetSteamworks
// function, so retail built the OSS without the Steam socket subsystem).
#ifndef INC_DISHONORED_STEAM_ISTEAMNETWORKING
#define INC_DISHONORED_STEAM_ISTEAMNETWORKING

#include "steamclientpublic.h"

#pragma pack(push, 8)

enum EP2PSend
{
	k_EP2PSendUnreliable = 0,
	k_EP2PSendUnreliableNoDelay = 1,
	k_EP2PSendReliable = 2,
	k_EP2PSendReliableWithBuffering = 3
};

struct P2PSessionState_t
{
	uint8 m_bConnectionActive;
	uint8 m_bConnecting;
	uint8 m_eP2PSessionError;
	uint8 m_bUsingRelay;
	int32 m_nBytesQueuedForSend;
	int32 m_nPacketsQueuedForSend;
	uint32 m_nRemoteIP;
	uint16 m_nRemotePort;
};

class ISteamNetworking
{
public:
	virtual bool SendP2PPacket(CSteamID steamIDRemote, const void* pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel) = 0;
	virtual bool IsP2PPacketAvailable(uint32* pcubMsgSize, int nChannel) = 0;
	virtual bool ReadP2PPacket(void* pubDest, uint32 cubDest, uint32* pcubMsgSize, CSteamID* psteamIDRemote, int nChannel) = 0;
	virtual bool AcceptP2PSessionWithUser(CSteamID steamIDRemote) = 0;
	virtual bool CloseP2PSessionWithUser(CSteamID steamIDRemote) = 0;
	virtual bool CloseP2PChannelWithUser(CSteamID steamIDRemote, int nChannel) = 0;
	virtual bool GetP2PSessionState(CSteamID steamIDRemote, P2PSessionState_t* pConnectionState) = 0;
	// DISHONORED(layout): slot 7 = vtable +28, retail InitSteamworks 2013 rva 0x5ac1d0 ends with
	// `AllowP2PPacketRelay(false)`.
	virtual bool AllowP2PPacketRelay(bool bAllow) = 0;

	// DISHONORED(written): slots 8.. (the legacy listen-socket block) are not declared.
};

#define STEAMNETWORKING_INTERFACE_VERSION "SteamNetworking005"

#pragma pack(pop)

#endif // INC_DISHONORED_STEAM_ISTEAMNETWORKING
