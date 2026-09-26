# Agent AM — our own Steamworks bindings, `WITH_STEAMWORKS=1`, the last three OSS natives (2026-09-26)

Wave-5 package AM. Target is the retail 2013 exe: every "2013 rva" is `retail2013_agentAM.i64` (copy of
`retail2013_named.i64`), "2012 rva" is `shipping2012_agentAM.i64`, readable version only. Decompiles in
`resources/reference/decomp/agentAM/` (2013) and `resources/reference/decomp/agentAM2012/` (2012), both
git-ignored. **No Steamworks SDK was downloaded**: the whole flat surface is reconstructed from the shipped
`steam_api.dll`, its interface-version strings and the retail/2012 decompiles.

## Result

| Step | State | Evidence |
|---|---|---|
| 1. Our own `steam_api` headers | **done**: `source/Development/Src/External/SteamworksFlat/steam/` — 10 headers, every type and every vtable slot tagged `DISHONORED(layout\|written\|port)` with a DLL export, an interface version string, a 2013 rva or a 2012 rva | 11 slots of `ISteamFriends011`, 2 of `ISteamUser016`, 2 of `ISteamUtils005`, 3 of `ISteamRemoteStorage006`, 1 of `ISteamUserStats010`, 1 of `ISteamApps005` and 1 of `ISteamNetworking005` are pinned by retail call sites (table below). Callback packing measured, not assumed |
| 2. Import library the Bink way | **done**: `cmake/Steamworks.cmake` builds `build/<dir>/steamworks/stub/steam_api.dll` from `steam_api_stub.cpp`; its import library makes the exe import `steam_api.dll` by the same undecorated cdecl names retail does, and `/DELAYLOAD:steam_api.dll` keeps a Steam-less run working | `dumpbin /dependents /imports:steam_api.dll` on our exe: `steam_api.dll` under "delay load dependencies", `SteamAPI_Init`, `SteamAPI_RegisterCallback`, `SteamAPI_Shutdown`, `SteamAPI_RestartAppIfNecessary`, `SteamUtils`, `SteamAPI_RunCallbacks`, `SteamUser`, `SteamFriends`, `SteamUserStats`, `SteamApps`, … The shipped DLL is never copied or staged |
| 3. `WITH_STEAMWORKS=1` | **done**: `DISHONORED_WITH_STEAMWORKS` (default ON when the headers are present) drives `WITH_STEAMWORKS` for every target and links `Dishonored::steamworks`. `WITH_STEAMWORKS_SOCKETS` stays 0 on evidence. Module grew 4 → 6 compile units | Snapshot build: 791 units, **0 errors**; `-- Steamworks: WITH_STEAMWORKS=1, import library from steam_api_stub, /DELAYLOAD:steam_api.dll`; CoreSmoke **99 passed, 0 failed**; `xcheck_sdk_layout.py` **2314 types, 0 mismatching, 0 contract mismatches** (the switch is layout-neutral) |
| 4. The three natives | **done**: `execReadFriendsList` / `execReadProfileSettings` / `execReadAchievements` plus the three member functions they dispatch to, both branches (Steam and offline) | `-strictnatives` run: no `UOnlineSubsystemSteamworks` native and no `Engine (no C++ body) native` left on the path |
| Offline path still works | **done**: `Initial startup: 6.6 s`, `Committed map change via DishonoredEngine`, exit 0 with `-nosteam` | commands below |
| Real bindings exercised against the shipped DLL | **done**: a run *without* `-nosteam` delay-loads `steam_api.dll` and calls the real `SteamAPI_Init`, which fails because no Steam client is running; the subsystem then takes the `OSCS_ServiceUnavailable` branch and the run still reaches both milestones | `agentAM_steam.log:19` `DevOnline: SteamAPI_Init: failed`, then `Initial startup: 9.76s` and `Committed map change via DishonoredEngine` |

**Acceptance**

```
smoke:  python resources\tools\build_and_smoke.py --build-dir build\agentAM_snap --no-build
        --exe-name DishonoredGame_AM.exe --log-name agentAM.log --ini-dir build\agentAM\config --rhi null
        --skip-native OnlineSubsystemPC --milestone "Initializing Engine..." --expect "Initial startup"
        --expect "Committed map change via DishonoredEngine" --forbid "UOnlineSubsystemSteamworks::exec"
        --timeout 120 "--extra-args=-forcelogflush -noscenerender"
        -> exit 0 (5,229 log lines, milestone reached, all three checks ok)

strict: same, --log-name agentAM_strict.log, no --skip-native, --expect "Initial startup"
        --forbid "UOnlineSubsystemSteamworks::exec" --forbid "Engine (no C++ body) native not ported"
        "--extra-args=-forcelogflush -noscenerender -strictnatives"
        -> exit 0

steam:  DishonoredGame_AM.exe -log -unattended -LOG=agentAM_steam.log -ENGINEINI=... -nullrhi
        -forcelogflush -noscenerender          (no -nosteam: the real SteamAPI_Init path)
```

**No Steam client is running on this machine** (`tasklist` has no `steam*` process) and our generated
`DishonoredEngine.ini` has no `bEnableSteam` key, so without `-nosteam` `appIsSteamEnabled()` returns TRUE,
the DLL is loaded and `SteamAPI_Init()` returns false. That exercises the import library, the delay-load
thunk and one flat entry point for real, but **no interface vtable call could be executed**: every
`GSteam*` pointer stays NULL, `GSteamworksInitialized` stays FALSE and `InitSteamworks` takes its failure
branch. The vtable slots are therefore validated against the retail exe's own call sites (below), not
against a live client. With a Steam client running and Dishonored's app id 205100 in a `steam_appid.txt`,
the paths that would newly execute are `InitSteamworks` (the nine accessors, `GetAppID`,
`SetWarningMessageHook`, the cloud-quota trio, `BLoggedOn`, `GetPersonaName`, `GetSteamID`), the callback
bridge registrations and `ReadAchievements`' `RequestCurrentStats`.

### Which build the numbers come from (read this)

The shared working tree does not build right now: agent AL's in-flight `cmake/PhysX.cmake` +
`source/Development/Src/External/PhysX284` turn `WITH_NOVODEX` on and `NxGenerated.h` does not compile
(`error C3646: 'driveType': unknown override specifier`, ~40 errors in `ForceFieldShape.cpp`,
`NxForceFieldTornado.cpp`, …). Everything above therefore comes from the snapshot:

* `build/agentAM_wt` — detached worktree of HEAD `493a9ed` plus my 28 files (`resources/tools/make_snapshot.py AM`,
  file list in `build/agentAM/snapshot_files.txt`), with my two edits to the shared `cmake/Dependencies.cmake`
  and `cmake/DishonoredDefines.cmake` re-applied on top of their HEAD versions so AL's PhysX edits stay out.
* `build/agentAM_snap` — the build directory of that snapshot (`build/agentAM_snap_build.cmd [target]`).
  `build/agentAM` still holds the earlier shared-tree build and the smoke `config/`.

An earlier shared-tree build (before AL's PhysX landed) produced the same results, including the
`dumpbin` import dump above.

## What the DLL gave us, and what it did not

`D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\steam_api.dll` (1.30.50.46, PE timestamp
2012-02-03) has **56 exports** and, being this generation, exports the `SteamUser()`-style accessors —
there is **no `SteamInternal_CreateInterface` and no `SteamAPI_ISteam*` flat wrapper**, so the engine calls
the interface vtables directly and the vtable order is load-bearing. Interface version strings inside the
DLL at 0x13480..0x139f4: `SteamUtils005`, `STEAMHTTP_INTERFACE_VERSION001`,
`STEAMSCREENSHOTS_INTERFACE_VERSION001`, `STEAMREMOTESTORAGE_INTERFACE_VERSION006`, `SteamNetworking005`,
`STEAMAPPS_INTERFACE_VERSION005`, `STEAMUSERSTATS_INTERFACE_VERSION010`, `SteamMatchMakingServers002`,
`SteamMatchMaking009`, `SteamFriends011`, `SteamUser016`, `SteamClient012`, `SteamContentServer002`,
`SteamGameServerStats001`, `SteamGameServer011`.

The retail exe imports 18 of the 56 (`resources/docs/symbols/imports_2013.csv`), all **undecorated cdecl**
(`__imp__SteamUser`, …) — unlike Bink's stdcall `_BinkOpen@8`. `SteamClient` is *not* imported, so
`ISteamClient012` is never needed and is left as an incomplete type.

### The vtable slots pinned by retail

Each row is a retail call site that fixes a slot; any inserted or dropped method between two of them would
have moved the later one, so these anchors validate the whole declared run of each interface.

| Interface | Slot (+byte) | Method | Retail evidence |
|---|---|---|---|
| ISteamUser016 | 1 (+4) | `BLoggedOn` | `InitSteamworks` 2013 0x5ac1d0; `SignInLocally` 0x5aab40 |
| ISteamUser016 | 2 (+8) | `GetSteamID` | `InitSteamworks` 0x5ac1d0 → `LoggedInPlayerId` (+240/+244) |
| ISteamFriends011 | 0 (+0) | `GetPersonaName` | `InitSteamworks` 0x5ac1d0 → `LoggedInPlayerName` |
| ISteamFriends011 | 3 (+12) | `GetFriendCount` | `GetFriendsList` 0x5adaa0, argument 4 = `k_EFriendFlagImmediate` |
| ISteamFriends011 | 4 (+16) | `GetFriendByIndex` | `GetFriendsList` 0x5adaa0 (hidden return pointer) |
| ISteamFriends011 | 5 (+20) | `GetFriendRelationship` | `IsFriend` 0x5a5920, `== 3` = `k_EFriendRelationshipFriend` |
| ISteamFriends011 | 6 (+24) | `GetFriendPersonaState` | `GetFriendsList` 0x5adaa0 |
| ISteamFriends011 | 7 (+28) | `GetFriendPersonaName` | `GetFriendsList` 0x5adaa0 |
| ISteamFriends011 | 8 (+32) | `GetFriendGamePlayed` | `GetFriendsList` 0x5adaa0 |
| ISteamFriends011 | 27/28/29 (+108/+112/+116) | `Get{Small,Medium,Large}FriendAvatar` | `GetOnlineAvatar` 0x5a92d0 picks by requested size (< 64, < 184, else) |
| ISteamFriends011 | 36 (+144) | `SetRichPresence` | `UOnlineSubsystemSteamworks::Init` 0x5ad3d0 (`"0"`, `"Dud"`) |
| ISteamUtils005 | 9 (+36) | `GetAppID` | `InitSteamworks` 0x5ac1d0 → `GSteamAppID` (2013 global 0x105b184) |
| ISteamUtils005 | 16 (+64) | `SetWarningMessageHook` | `InitSteamworks` 0x5ac1d0 |
| ISteamUserStats010 | 0 (+0) | `RequestCurrentStats` | `ReadAchievements` 0x5aaf90 |
| ISteamApps005 | 1 (+4) | `BIsLowViolence` | `InitSteamworks` 0x5ac1d0 |
| ISteamRemoteStorage006 | 13/14/15 (+52/+56/+60) | `GetQuota` / `IsCloudEnabledForAccount` / `IsCloudEnabledForApp` | `InitSteamworks` 0x5ac1d0 calls the two predicates and only then `GetQuota` |
| ISteamNetworking005 | 7 (+28) | `AllowP2PPacketRelay` | `InitSteamworks` 0x5ac1d0, last call |

Interfaces are declared only up to the highest slot anything calls, and each header says so explicitly.
`ISteamClient`, `ISteamMatchmaking`, `ISteamMatchmakingServers`, `ISteamGameServer`,
`ISteamGameServerStats`, `ISteamHTTP` and `ISteamScreenshots` are incomplete types: retail only NULL-checks
those pointers, so declaring unverified slots would have been guesswork.

### Callback and call-result plumbing, and the packing measurement

The 2012 PDB kept the SDK's own `steam_api.h` line numbers for the template bodies, which gives the exact
shape of `CCallbackBase` / `CCallback<T,P,bGameServer>` / `CCallResult<T,P>`:

* three virtuals in this order — `Run(void*)`, `Run(void*, bool, SteamAPICall_t)`, `GetCallbackSizeBytes()`
  (2012 rvas 0x5ead30 / 0x5ead50 / 0x5ead20; the `CCallResult` pair is 0x5ea660 / 0x5ea680);
* `uint8 m_nCallbackFlags` then `int m_iCallback`, with `k_ECallbackFlagsRegistered = 0x01`
  (2012 `CCallback::Register` 0x5ead70: `if ((m_nCallbackFlags & 1) != 0) SteamAPI_UnregisterCallback(this)`
  then `SteamAPI_RegisterCallback(this, 1101)`);
* `SteamAPI_RegisterCallback(base, iCallback)`, `SteamAPI_UnregisterCallback(base)`,
  `SteamAPI_RegisterCallResult(base, hAPICall)`, `SteamAPI_UnregisterCallResult(base, hAPICall)`;
* `sizeof(CCallback<...>) == 20`, which is what retail's bridge measures: `appMalloc(244, 8)` for one
  pointer plus 12 CCallbacks (`InitSteamworks` 0x5ac1d0; ctor 0x5ab400 writes members at +4, +24 … +224).

Callback ids come from two numbers readable straight out of the exe: `UserStatsReceived_t` = **1101**
(the literal `Register` passes) and `NumberOfCurrentPlayers_t` = **1107** (2012 bridge ctor 0x5f13f0), plus
the leaderboard helper's 1104 / 1105 / 1106 (2013 ctor 0x5ab620). That fixes the user-stats base at 1100
and the ids run in the interface's documented order.

**Packing is measured, not assumed.** `CCallback<SteamCallbackBridge,GSClientAchievementStatus_t,1>::GetCallbackSizeBytes`
returns **144** (2012 rva 0x5ead40) for a `uint64 + char[128] + bool` payload = 137 bytes: only 8-byte
alignment rounds 137 to 144 (4-byte alignment gives 140). So the headers force `#pragma pack(push, 8)`,
which matters because the engine compiles with `/Zp4`. Two more sizes confirm the reconstructed structs:
`GameRichPresenceJoinRequested_t` = 264 = `sizeof(CSteamID) + 256` (0x5ead20) and
`GameServerChangeRequested_t` = 128 (0x66add0).

## Retail's OnlineSubsystemSteamworks is not Epic's — why the reference units stay excluded

`resources/docs/symbols/sourcefiles.txt` (2012 PDB) lists exactly three sources for the module:
`onlinesubsystemsteamworks.cpp`, `onlinesubsystemsteamworksbridge.cpp`, `onlinesubsystemsteamworkspackage.cpp`
(plus the SDK headers `steam_api.h`, `steamclientpublic.h`, `matchmakingtypes.h`). Neither build has a
single `FOnlineAsyncTaskManagerSteam`, `FVoiceInterfaceSteam`, `UOnlineGameInterfaceSteamworks`,
`UOnlineAuthInterfaceSteamworks`, `UnSocketSteamworks` or `UnNetSteamworks` function
(`functions.csv`, `functions_2013.csv`). Arkane shipped an **earlier UE3 generation** whose async plumbing
is one `SteamCallbackBridge` (35 functions in 2012, 17 in 2013) rather than Epic's task manager, and whose
global is `GSteamworksInitialized`, not the reference's `GSteamworksClientInitialized`.

So the reference `Src/OnlineSubsystemSteamworks.cpp` (9,310 lines), `OnlineAsyncTaskManagerSteam.cpp`,
`VoiceInterfaceSteamworks.cpp`, `UOnline{Auth,Game,Lobby}InterfaceSteamworks.cpp`, `UnNetSteamworks.cpp`
and `UnSocketSteamworks.cpp` **stay in the `Sources.cmake` EXCLUDE list**: they describe a module retail
does not have, they would need the whole SDK surface (matchmaking, matchmaking-servers, networking,
game-server, ~40 more callback structs) and they are the multiplayer path, which this package puts out of
scope. `onlinesubsystemsteamworksbridge.cpp` came *out* of the list and is now the real bridge; the ports
that retail keeps in its one big cpp went into our own `OnlineSubsystemSteamworksClient.cpp` and
`OnlineSubsystemSteamworksReads.cpp`. `OnlineSubsystemSteamworksPackage.cpp` stays excluded because the
generated registrant unit replaces it.

**`WITH_STEAMWORKS_SOCKETS` is 0 in every configuration**, on the same evidence: no Steam socket or net
function in either build, so retail's UE3 net driver is the plain IpDrv one. Turning the switch on would
add members to `UNetConnection` / `UNetDriver` / `UWorld` that retail does not have (`UnChan.h:666`,
`UnConn.h:285`, `UnNet.h:39`, `UnNetDrv.h:174`, `UnWorld.h:80`).

## The three natives

Signatures from the CodeRed dump's exec parameter structs (`src/OnlineSubsystemSteamworks_parameters.hpp`)
and the exec decompiles; bodies from the member functions the exec thunks dispatch to through the vtable.

| Native | exec (2013) | member (2013) | Behaviour |
|---|---|---|---|
| `ReadFriendsList(byte, int Count, int StartingAt)` | 0x5a4550 | 0x5aaf30 (slot 95 = +380) | `S_OK` only while `GSteamworksInitialized` and the caller is `LoggedInPlayerNum`; `ReadFriendsDelegates` fire with that result **either way** (retail: "always trigger the delegate immediately and again as friends are added") |
| `ReadProfileSettings(byte, OnlineProfileSettings, bool bForceRead)` | 0x5a4370 | 0x5ad8f0 (slot 91 = +364) | Offline / other player: `eventSetToDefaults()` on the passed-in object, `S_OK`, `ProfileCache.ReadDelegates` fire successful. Online: caches the object, empties it, defaults it. `bForceRead` is Arkane's third parameter, absent from the 2012 build (2012 member 0x5f3040 takes two) |
| `ReadAchievements(byte, int TitleId, bool bShouldReadText, bool bShouldReadImages)` | 0x5a4e50 | 0x5aaf90 (slot 121 = +484) | FALSE without Steam or for another player. With Steam: `UserStatsReceivedState` not in {`OERS_InProgress`, `OERS_Done`} → set `OERS_InProgress` + `ISteamUserStats::RequestCurrentStats()`; otherwise `AchievementReadDelegates` fire straight away |

The async completion `ReadAchievements` needs is `UserStatsReceived_t` → `OnUserStatsReceived`
(2012 rva 0x5f07d0): when the callback's `m_nGameID` equals `CGameID(GSteamAppID)`, `UserStatsReceivedState`
becomes `OERS_Done` (or `OERS_Failed`) and `AchievementReadDelegates` fire with `TitleId = 0`.

**A retail 2013 regression, worked around deliberately.** The 2013 `SteamCallbackBridge` (ctor 0x5ab400,
sizeof 244, 12 CCallbacks) no longer registers `UserStatsReceived_t` / `UserStatsStored_t`, which the 2012
one did (ctor 0x5f13f0). Nothing else in 2013 writes `UserStatsReceivedState`, so retail's
`ReadAchievements` asks for the stats and its completion delegate can never fire. Our bridge keeps the two
2012 registrations (tagged in `Inc/onlinesubsystemsteamworksbridge.h`) so the async read completes.

## Files

**New**

| File | What |
|---|---|
| `source/Development/Src/External/SteamworksFlat/steam/steamtypes.h` | scalars, handles, `SteamAPICall_t`, `CSteamID`, `CGameID`, the measured `#pragma pack(push, 8)` |
| `.../steam/steamclientpublic.h` | `EResult`, `EPersonaState`, `EFriendRelationship`, `EFriendFlags`, `ENotificationPosition`, the string limits |
| `.../steam/isteamuser.h`, `isteamfriends.h`, `isteamutils.h`, `isteamuserstats.h`, `isteamapps.h`, `isteamremotestorage.h`, `isteamnetworking.h` | the seven interfaces we call, per-slot tagged |
| `.../steam/steam_api.h` | the flat exports, `CCallbackBase` / `CCallback` / `CCallResult`, the 13 callback structs, the incomplete types |
| `.../steam_api_stub.cpp` | the throw-away stub DLL that yields the import library |
| `cmake/Steamworks.cmake` | `DISHONORED_WITH_STEAMWORKS`, `steam_api_stub`, `Dishonored::steamworks`, `/DELAYLOAD:steam_api.dll` |
| `source/Development/Src/Core/Src/UnSteamworks.cpp` | `GSteamworksInitialized`, `appIsSteamEnabled` (0x5a8840), `appSteamInit` (2012 0x5eebb0), `appSteamShutdown` (2012 0x5e9ea0), `appSteamHandleCmdLine` (0x5a50f0), `appGetSteamworksAppId` |
| `source/Development/Src/OnlineSubsystemSteamworks/Src/OnlineSubsystemSteamworksClient.cpp` | the `GSteam*` globals, `InitSteamworks` (0x5ac1d0), `TickSteamworksTasks` (0x5aa980), `OnUserStatsReceived` / `OnUserStatsStored` |
| `.../Src/OnlineSubsystemSteamworksReads.cpp` | the three natives and their member functions |
| `.../OnlineSubsystemSteamworksNativeStubs.ported.agentAM.txt` | the three exec names the generator must skip |

**Changed**

| File | Change |
|---|---|
| `cmake/Dependencies.cmake` | `include(cmake/Steamworks.cmake)` after `Bink.cmake` (which declares `DISHONORED_RETAIL_DIR`) |
| `cmake/DishonoredDefines.cmake` | `WITH_STEAMWORKS=$<BOOL:${DISHONORED_WITH_STEAMWORKS}>`, `WITH_STEAMWORKS_SOCKETS=0` with the evidence, and `Dishonored::steamworks` linked from `dishonored_apply_defines()` |
| `source/Development/Src/Core/Inc/UnFile.h` | declares `GSteamworksInitialized` (unconditionally — the offline branch of the ported natives needs it), `appIsSteamEnabled`, `appSteamInit`, `appSteamShutdown`, `appSteamHandleCmdLine`, `DISHONORED_STEAM_APPID 205100` |
| `source/Development/Src/Core/Src/UnMisc.cpp` | dropped the reference's `#include "OnlineSubsystemSteamworks.h"`: Core would have pulled in Engine's and the module's generated headers, and our static-library layering forbids the cycle (CoreSmoke / LayoutProbe link Core alone). The declarations moved to `UnFile.h` |
| `.../OnlineSubsystemSteamworks/Sources.cmake` | `onlinesubsystemsteamworksbridge.cpp` out of EXCLUDE; the comment records why the other reference units stay in |
| `.../Inc/onlinesubsystemsteamworksbridge.h`, `Src/onlinesubsystemsteamworksbridge.cpp` | were `import_reference.py` stubs, now the real `SteamCallbackBridge` (7 client callbacks) |
| `.../Inc/CppText/UOnlineSubsystemSteamworks.h` | declares the three read members plus `InitSteamworks`, `TickSteamworksTasks`, `OnUserStats{Received,Stored}` |
| `.../Src/OnlineSubsystemSteamworksOffline.cpp` (agent X's) | `execInit` calls `InitSteamworks()` instead of firing `OSCS_ServiceUnavailable` inline (identical without Steam); the local sign-in is guarded by `LoggedInStatus == LS_NotLoggedIn` so a real Steam sign-in is not overwritten (retail `SignInLocally` 0x5aab40 tests the same thing); `Tick` calls `TickSteamworksTasks` first |
| `.../Src/OnlineSubsystemSteamworksNativeStubs.cpp` | the three stub bodies removed |
| `source/Development/Src/Engine/Inc/arksettings.h`, `Src/arksettings.cpp` | `ArkSettings::EChangeReason`, `FindListeners` (0x539580), `OnSettingsChanged` (0x53b7e0) — see the note below |
| `source/Development/Src/Engine/Inc/EngineUIPrivateClasses.h`, `Src/UnUIDataStores.cpp` | `UUIDataProvider_OnlinePlayerStorage::OnReadStorageComplete_Native` (exec 0x5efc80, body 0x3e1660) declared, registered and implemented — see the note below |

## The one thing the ported `ReadProfileSettings` newly exposed

`ReadProfileSettings` really fires `ProfileCache.ReadDelegates` now, and the UI data provider subscribed to
them calls a native retail has and we did not: `Engine.UIDataProvider_OnlinePlayerStorage:OnReadStorageComplete_Native`.
It was unbound, so `-strictnatives` started aborting there at 6.0 s (before `Initial startup`), a regression
of the strict run. It is 53 bytes of exec (2013 rva 0x5efc80, `P_FINISH` then one virtual) whose body
(0x3e1660, `void()`, vtable slot 87 = +348) is three lines: `ArkSettings::FindListeners`, then
`ArkSettings::OnSettingsChanged(Profile, Listeners, ECR_ReadFromStorage)`. Both ArkSettings statics are
ported (2013 rvas above; `FindListeners` iterates every live `UObject` implementing
`Engine.ArkSettingsListenerInterface`, `OnSettingsChanged` rereads `ArkSettingsParameters` from the storage
object and calls `ApplyGameSettings` on each listener).

**The listener notification is gated behind `-arksettings`, off by default** (`DISHONORED(bringup)` in
`UnUIDataStores.cpp`). Applying the settings is what retail does, but every DishonoredGame listener's
`ApplyGameSettings` is still a generated `IArkSettingsListenerInterface` shim that `appErrorf`s, and the
first one kills the run — measured: with the gate open the run dies at 6.2 s on
`DishonoredGame native not ported: UDisPostProcessManager::ApplyGameSettings`. The shims are
`ADishonoredPlayerController`, `ADishonoredPlayerPawn`, `ADishonoredPlayerCamera`, `ADishonoredGameInfo`,
`UDisPostProcessManager`, `UDishonoredPlayerInput`, `UDisGFxMoviePlayerHUD`,
`UDisItemContext_AimAssistAttack` and more (`grep -rn ApplyGameSettings source/Development/Src/DishonoredGame/Inc`).
Retail's `GEnableForceFeedback = Parameters.m_bGamepadVibration` tail has no equivalent global in this tree.

## What is left

* **Hand-over (DishonoredGame, agent AJ/AF territory):** port the `ApplyGameSettings` implementations above,
  then drop `-arksettings` from `OnReadStorageComplete_Native`. That is what stands between here and the
  game actually applying its saved graphics / input / audio settings.
* **Still unported in the module** (`agentAM_status.csv` rows marked `needed`): `GetFriendsList` 0x5adaa0,
  `GetAchievements` 0x5ade40 (needs `LoadAchievementDetails` over `ISteamUserStats` slots 6/11/12),
  `GetOnlineAvatar` 0x5a92d0 and `OnAvatarImageLoaded` 0x5aa8c0, `OnGameServerChangeRequested` 0x5a89c0,
  `OnGameJoinRequested` 0x5a8b60, `CheckDLCOwnership` 0x5a8960 (`ISteamApps::BIsDlcInstalled`, the
  `Req_DLC05_*` checks), the leaderboard helper 0x5ab620. The headers already declare every slot the first
  five of those need; the leaderboard helper needs `ISteamUserStats` slots 14.., deliberately not declared.
* **Steam-only halves left out on purpose:** the game-server path (`SteamGameServer*`,
  `GIsSteamServerReady`, `GSClientAchievementStatus_t`, `GSPolicyResponse_t`), `ISteamNetworking`'s
  `AllowP2PPacketRelay`, `ISteamApps::BIsLowViolence`, the avatar request queue in `TickSteamworksTasks`,
  and `ReadProfileSettings`' cloud read plus its `WriteProfileSettings` follow-up.
* **Not verified against a live Steam client** (none installed/running here): every interface vtable call.
  The slots are pinned by retail call sites, the packing by a retail `GetCallbackSizeBytes`, but a run with
  the client up is the only way to prove `InitSteamworks` and the callback bridge end to end. Anyone with
  Steam installed can do it with `DishonoredGame_<X>.exe` (no `-nosteam`) plus a `steam_appid.txt`
  containing `205100`.
* **Coordinator:** the shared working tree does not build (agent AL's `External/PhysX284`); the merge of
  this package needs that fixed first, or my files merged on top of a working HEAD. Nothing of mine touches
  PhysX. `cmake/Dependencies.cmake` and `cmake/DishonoredDefines.cmake` are shared with AL — both of us
  added a block; the two are independent and merge cleanly by concatenation. Two shared docs are now
  out of date and I left them to you: `middleware.md` section 2.6 ("the Steamworks SDK is free with a
  partner account ... must be obtained" and the 1.18/1.19 SDK in the user-blocker list at the end) and
  `STATUS.md` ("Switched off: WITH_STEAMWORKS=0", the Phase 4 SDK list, and the natives row).

## Commands

```
snapshot: python resources\tools\make_snapshot.py AM --list <file list>     (28 files)
          then re-apply my cmake/Dependencies.cmake + cmake/DishonoredDefines.cmake edits on the
          snapshot's HEAD copies (AL's PhysX edits must not come along)
build:    build\agentAM_snap_build.cmd [DishonoredGame|CoreSmoke|LayoutProbe]     -> build\agentAM_snap
          (shared tree, when it builds: set BUILD_DIR=build\agentAM then resources\build-game.cmd)
checks:   CoreSmoke.exe from the repo root                       -> 99 passed, 0 failed
          LayoutProbe.exe > build\agentAM_snap\layout_probe.txt
          python resources\tools\sdk\xcheck_sdk_layout.py build\agentAM_snap\layout_probe.txt
                                                                 -> 2314 types, 0 mismatching, 0 contract
imports:  dumpbin /dependents /imports:steam_api.dll build\agentAM_snap\Binaries\Win32\DishonoredGame.exe
decomp:   python resources\tools\ida\run.py resources\tools\ida\decompile_funcs.py
              resources\docs\idb\retail2013_agentAM.i64 resources\reference\decomp\agentAM rva:0x...
```

Scratch (not repo tools): `build\agentAM_snap_build.cmd`, `build\agentAM_wt` (snapshot worktree — holds no
stage and no junction; remove it with `git worktree remove` only after checking that), IDA copies
`resources\docs\idb\{shipping2012,retail2013}_agentAM.i64`, decompiles under
`resources\reference\decomp\agentAM{,2012}`.

## Summary

The Steamworks flat API is ours: ten headers reconstructed from the shipped `steam_api.dll`'s 56 exports,
its interface version strings and 21 retail call sites that pin every vtable slot and every callback size we
rely on — no SDK downloaded, packing measured rather than guessed. A throw-away stub DLL gives the import
library, so the exe delay-imports `steam_api.dll` by exactly the names retail imports and still runs with no
Steam client and no DLL on the path. `WITH_STEAMWORKS=1` is on for every target behind
`DISHONORED_WITH_STEAMWORKS`, `WITH_STEAMWORKS_SOCKETS` stays 0 on evidence (retail has no Steam socket code
at all), the module builds 6 units instead of 4, and the switch is layout-neutral (2,314 types, 0
mismatches; CoreSmoke 99/0). The three natives that were the last bodies missing on the startup and map path
are ported from their 2013 decompiles with both branches, plus the callback bridge and the user-stats
completion they need — and the retail 2013 bridge's own missing `UserStatsReceived_t` registration is
restored from 2012 so the achievement read can complete. With `-nosteam` the run still reaches
`Initial startup: 6.6s` and `Committed map change via DishonoredEngine`; without it the real `SteamAPI_Init`
is called through our import library and fails cleanly into the offline path. `-strictnatives` no longer
aborts on any `UOnlineSubsystemSteamworks` native, nor on the Engine UI native the port newly exposed.
