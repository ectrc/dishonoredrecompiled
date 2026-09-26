// UnSteamworks.cpp - the Steam client API entry points Core itself calls: appIsSteamEnabled,
// appSteamInit (appInit, UnMisc.cpp) and appSteamShutdown (appPreExit, UnMisc.cpp), plus
// GSteamworksInitialized, which every Steam code path in the OnlineSubsystemSteamworks module gates on.
//
// Retail keeps these four in onlinesubsystemsteamworks.cpp (2012 rvas 0x5eeac0 / 0x5eebb0 / 0x5e9ea0,
// file:line 142 / 175 / 220; 2013 appIsSteamEnabled is rva 0x5a8840). Here they live in Core because
// Core is the caller and our modules are static libraries: Core cannot depend on the
// OnlineSubsystemSteamworks library without a cycle, and CoreSmoke / LayoutProbe link Core alone.
// Nothing else moved - the subsystem-side init (InitSteamworks, 2013 rva 0x5ac1d0) and the interface
// pointers stay in the module.
//
// The flat surface is ours: source/Development/Src/External/SteamworksFlat/steam (cmake/Steamworks.cmake).
#include "CorePrivate.h"

// DISHONORED(retail): 2013 global 0x101b298 (?GSteamworksInitialized@@3IA). FALSE until SteamAPI_Init
// succeeds, and back to FALSE on SteamAPI_Shutdown or the SteamShutdown_t callback. Defined in every
// configuration (see UnFile.h) so the offline branch of the ported natives compiles with WITH_STEAMWORKS=0.
UBOOL GSteamworksInitialized = FALSE;

#if WITH_STEAMWORKS

#include "steam/steam_api.h"

/** steam_api.dll is delay-loaded (cmake/Steamworks.cmake), so nothing may touch an export before this
 *  returns TRUE: a build with the bindings on has to keep running with no Steam client installed. */
static UBOOL appIsSteamApiDllLoaded()
{
	// DISHONORED(bringup): retail always has steam_api.dll next to the exe and never checks. We do,
	// because the smoke runs launch from a build directory and because -nosteam must not need the DLL.
	static INT DllState = -1;
	if (DllState == -1)
	{
		void* Handle = appGetDllHandle(TEXT("steam_api.dll"));
		DllState = Handle != NULL ? 1 : 0;
		if (DllState == 0)
		{
			debugf(NAME_DevOnline, TEXT("DISHONORED(bringup): steam_api.dll not found, the Steam client API stays off"));
		}
	}
	return DllState == 1;
}

// DISHONORED(port): appIsSteamEnabled, 2013 rva 0x5a8840 (2012 rva 0x5eeac0, byte-identical match,
// onlinesubsystemsteamworks.cpp:142). Caches its answer in the `SteamEnabledCheck` global (2013 rva
// 0xee0ab4) which starts at -1: -NOSTEAM, then [OnlineSubsystemSteamworks.OnlineSubsystemSteamworks]
// bEnableSteam in GEngineIni (false in the retail install's DefaultEngine.ini), then the three
// cook/editor commandlet tokens.
UBOOL appIsSteamEnabled()
{
	if (GSteamworksInitialized)
	{
		return TRUE;
	}

	static INT SteamEnabledCheck = -1;
	if (SteamEnabledCheck == -1)
	{
		UBOOL bEnabled = !ParseParam(appCmdLine(), TEXT("NOSTEAM"));
		if (bEnabled)
		{
			GConfig->GetBool(TEXT("OnlineSubsystemSteamworks.OnlineSubsystemSteamworks"), TEXT("bEnableSteam"), bEnabled, GEngineIni);
			if (bEnabled)
			{
				// DISHONORED(port): retail tests these three with its own HasCmdLineToken (a first-token
				// test for a commandlet name), which Core does not have; ParseParam is equivalent for a
				// game launch, where none of them is on the command line.
				bEnabled = !ParseParam(appCmdLine(), TEXT("EDITOR"))
					&& !ParseParam(appCmdLine(), TEXT("MAKE"))
					&& !ParseParam(appCmdLine(), TEXT("MAKECOMMANDLET"))
					&& !ParseParam(appCmdLine(), TEXT("COOKPACKAGES"));
			}
		}
		if (bEnabled && !appIsSteamApiDllLoaded())
		{
			bEnabled = FALSE;
		}
		SteamEnabledCheck = bEnabled ? 1 : 0;
	}

	return SteamEnabledCheck == 1;
}

// DISHONORED(port): appSteamInit, 2012 rva 0x5eebb0 (onlinesubsystemsteamworks.cpp:175), called from
// appInit (Core/Src/UnMisc.cpp). bRelaunchInSteam comes from the same ini section, and the app id passed
// to SteamAPI_RestartAppIfNecessary is Dishonored's 205100 (the literal in the 2012 decompile).
void appSteamInit()
{
	if (!appIsSteamEnabled())
	{
		return;
	}

	UBOOL bRelaunchInSteam = FALSE;
	GConfig->GetBool(TEXT("OnlineSubsystemSteamworks.OnlineSubsystemSteamworks"), TEXT("bRelaunchInSteam"), bRelaunchInSteam, GEngineIni);
	if (bRelaunchInSteam && SteamAPI_RestartAppIfNecessary(DISHONORED_STEAM_APPID))
	{
		debugf(NAME_DevOnline, TEXT("Steam is relaunching the game through the client, exiting"));
		appRequestExit(FALSE);
		return;
	}

	GSteamworksInitialized = SteamAPI_Init() ? TRUE : FALSE;
	debugf(NAME_DevOnline, TEXT("SteamAPI_Init: %s"), GSteamworksInitialized ? TEXT("initialized") : TEXT("failed"));
}

// DISHONORED(port): appSteamShutdown, 2012 rva 0x5e9ea0 (onlinesubsystemsteamworks.cpp:220), called from
// appPreExit (Core/Src/UnMisc.cpp). The game-server half of the retail body (SteamGameServer()->LogOff,
// SteamGameServer_Shutdown, GIsSteamServerReady) is not ported: retail's dedicated-server path is out of
// scope and calling SteamGameServer() would load the delay-loaded DLL on every exit.
void appSteamShutdown()
{
	if (GSteamworksInitialized)
	{
		SteamAPI_Shutdown();
		GSteamworksInitialized = FALSE;
	}
}

// DISHONORED(port): appSteamHandleCmdLine, 2013 rva 0x5a50f0 (2012 rva 0x5e9e50, byte-identical match),
// called from GuardedMain (Launch/Src/Launch.cpp): when the Steam client launches the game to join a
// server it prefixes the command line with "+connect ", which is skipped here so the rest of the engine
// sees the plain URL.
void appSteamHandleCmdLine(const TCHAR** CmdLine)
{
	const TCHAR* ConnectToken = TEXT("+connect ");
	const INT TokenLen = appStrlen(ConnectToken);
	if (appStrnicmp(*CmdLine, ConnectToken, TokenLen) == 0)
	{
		*CmdLine += TokenLen;
	}
}

// DISHONORED(written): retail has no appGetSteamworksAppId of its own (no such function in
// functions_2013.csv or the 2012 PDB); appGetTitleId's WITH_STEAMWORKS branch needs one. The constant is
// the app id the 2012 appSteamInit passes to SteamAPI_RestartAppIfNecessary.
INT appGetSteamworksAppId()
{
	if (GSteamworksInitialized && SteamUtils() != NULL)
	{
		return (INT)SteamUtils()->GetAppID();
	}
	return DISHONORED_STEAM_APPID;
}

#endif // WITH_STEAMWORKS
