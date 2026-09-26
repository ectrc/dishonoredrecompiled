// steam_api_stub.cpp - the throw-away stub DLL cmake/Steamworks.cmake builds only to get an import
// library for the shipped steam_api.dll, exactly the way cmake/Bink.cmake does it for binkw32.dll
// (resources/docs/agents/agentU.md: lib.exe /def cannot produce the right x86 import names, a dllexport
// build can). This DLL is never shipped, never staged and never loaded: the runtime is
// D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\steam_api.dll.
//
// Every entry point below is one the retail 2013 exe imports (resources/docs/symbols/imports_2013.csv,
// 18 names) plus SteamAPI_RestartAppIfNecessary, which the 2012 exe imports and appSteamInit still calls.
// extern "C" + __cdecl makes MSVC export the undecorated name, which is what the shipped DLL exports and
// what the retail import table names (`__imp__SteamUser`, ...).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STEAM_STUB extern "C" __declspec(dllexport)

STEAM_STUB bool __cdecl SteamAPI_Init() { return false; }
STEAM_STUB bool __cdecl SteamAPI_InitSafe() { return false; }
STEAM_STUB void __cdecl SteamAPI_Shutdown() {}
STEAM_STUB bool __cdecl SteamAPI_RestartAppIfNecessary(unsigned int) { return false; }
STEAM_STUB void __cdecl SteamAPI_RunCallbacks() {}
STEAM_STUB bool __cdecl SteamAPI_IsSteamRunning() { return false; }
STEAM_STUB int __cdecl SteamAPI_GetHSteamPipe() { return 0; }
STEAM_STUB int __cdecl SteamAPI_GetHSteamUser() { return 0; }
STEAM_STUB const char* __cdecl SteamAPI_GetSteamInstallPath() { return ""; }
STEAM_STUB void __cdecl SteamAPI_SetMiniDumpComment(const char*) {}
STEAM_STUB void __cdecl SteamAPI_WriteMiniDump(unsigned int, void*, unsigned int) {}

STEAM_STUB void __cdecl SteamAPI_RegisterCallback(void*, int) {}
STEAM_STUB void __cdecl SteamAPI_UnregisterCallback(void*) {}
STEAM_STUB void __cdecl SteamAPI_RegisterCallResult(void*, unsigned __int64) {}
STEAM_STUB void __cdecl SteamAPI_UnregisterCallResult(void*, unsigned __int64) {}

STEAM_STUB void* __cdecl SteamUser() { return NULL; }
STEAM_STUB void* __cdecl SteamFriends() { return NULL; }
STEAM_STUB void* __cdecl SteamUtils() { return NULL; }
STEAM_STUB void* __cdecl SteamUserStats() { return NULL; }
STEAM_STUB void* __cdecl SteamApps() { return NULL; }
STEAM_STUB void* __cdecl SteamRemoteStorage() { return NULL; }
STEAM_STUB void* __cdecl SteamNetworking() { return NULL; }
STEAM_STUB void* __cdecl SteamMatchmaking() { return NULL; }
STEAM_STUB void* __cdecl SteamMatchmakingServers() { return NULL; }
STEAM_STUB void* __cdecl SteamGameServer() { return NULL; }
STEAM_STUB void __cdecl SteamGameServer_Shutdown() {}

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
	return TRUE;
}
