// onlinesubsystemsteamworksbridge.cpp - SteamCallbackBridge, the Steam callback owner of
// UOnlineSubsystemSteamworks (see Inc/onlinesubsystemsteamworksbridge.h for what the class is and where
// its shape comes from). Retail's file is
// v:\dishonored\unrealengine3qatest\development\src\onlinesubsystemsteamworks\src\onlinesubsystemsteamworksbridge.cpp
// and the 2012 PDB's file:line for each handler is quoted below.
#include "OnlineSubsystemSteamworks.h"

#if WITH_UE3_NETWORKING && WITH_STEAMWORKS

#include "OnlineSubsystemUtilities.h"
#include "onlinesubsystemsteamworksbridge.h"

// DISHONORED(port): SteamCallbackBridge::SteamCallbackBridge, 2013 rva 0x5ab400 (2012 rva 0x5f13f0):
// the Subsystem pointer, then one CCallback::Register per handler.
SteamCallbackBridge::SteamCallbackBridge(UOnlineSubsystemSteamworks* InSubsystem)
	: Subsystem(InSubsystem)
{
	GameOverlayActivatedCallback.Register(this, &SteamCallbackBridge::OnGameOverlayActivated);
	DLCInstalledCallback.Register(this, &SteamCallbackBridge::OnDLCInstalled);
	SteamShutdownCallback.Register(this, &SteamCallbackBridge::OnSteamShutdown);
	SteamServersConnectedCallback.Register(this, &SteamCallbackBridge::OnSteamServersConnected);
	SteamServersDisconnectedCallback.Register(this, &SteamCallbackBridge::OnSteamServersDisconnected);
	UserStatsReceivedCallback.Register(this, &SteamCallbackBridge::OnUserStatsReceived);
	UserStatsStoredCallback.Register(this, &SteamCallbackBridge::OnUserStatsStored);
}

// DISHONORED(port): 2012 rva 0x5f1260, onlinesubsystemsteamworksbridge.cpp:77
void SteamCallbackBridge::OnUserStatsReceived(UserStatsReceived_t* CallbackData)
{
	Subsystem->OnUserStatsReceived(CallbackData);
}

// DISHONORED(port): 2012 rva 0x5f1270, onlinesubsystemsteamworksbridge.cpp:82
void SteamCallbackBridge::OnUserStatsStored(UserStatsStored_t* CallbackData)
{
	Subsystem->OnUserStatsStored(CallbackData);
}

// DISHONORED(port): 2012 rva 0x5ea160, onlinesubsystemsteamworksbridge.cpp:38 - the Steam overlay pauses
// the game through the engine's lost-focus pause, exactly as a lost window focus does.
void SteamCallbackBridge::OnGameOverlayActivated(GameOverlayActivated_t* CallbackData)
{
	if (GEngine != NULL)
	{
		GEngine->OnLostFocusPause(CallbackData->m_bActive != 0);
	}
}

// DISHONORED(port): 2012 rva 0x5ea190, onlinesubsystemsteamworksbridge.cpp:44 - sets the subsystem's
// bDLCHasBeenInstalled bit so the next DLC enumeration rebuilds its list.
void SteamCallbackBridge::OnDLCInstalled(DlcInstalled_t* /*CallbackData*/)
{
	Subsystem->bDLCHasBeenInstalled = TRUE;
}

// DISHONORED(port): 2012 rva 0x5ea150, onlinesubsystemsteamworksbridge.cpp:33 - the Steam client is going
// down, so the game exits.
void SteamCallbackBridge::OnSteamShutdown(SteamShutdown_t* /*CallbackData*/)
{
	debugf(NAME_DevOnline, TEXT("Steam client is shutting down, requesting exit"));
	appRequestExit(FALSE);
}

// DISHONORED(port): the connection-status half of the retail handlers (2012 rvas 0x5f11e0 / 0x5f1220,
// onlinesubsystemsteamworksbridge.cpp:65/:71): the subsystem's connection-status change delegates, the
// same ones InitSteamworks fires (2013 rva 0x5ac1d0, delegates at +412).
void SteamCallbackBridge::OnSteamServersConnected(SteamServersConnected_t* /*CallbackData*/)
{
	OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms Parms(EC_EventParm);
	Parms.ConnectionStatus = OSCS_Connected;
	TriggerOnlineDelegates(Subsystem, Subsystem->ConnectionStatusChangeDelegates, &Parms);
}

void SteamCallbackBridge::OnSteamServersDisconnected(SteamServersDisconnected_t* /*CallbackData*/)
{
	OnlineSubsystemSteamworks_eventOnConnectionStatusChange_Parms Parms(EC_EventParm);
	Parms.ConnectionStatus = OSCS_ConnectionDropped;
	TriggerOnlineDelegates(Subsystem, Subsystem->ConnectionStatusChangeDelegates, &Parms);
}

#endif // WITH_UE3_NETWORKING && WITH_STEAMWORKS
