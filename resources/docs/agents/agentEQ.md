# Agent EQ (PHASE13 EQ) — the nine `ApplyGameSettings` bodies, and what a saved setting can actually reach

Branched from HEAD `2d08c15`, own worktree `build/agentEQ_wt`, own build dirs
`build/agentEQ_wt/build/agentEQ_rel` (this package), `build/agentEQ_headsrc/build/agentEQ_headrel` (the
untouched HEAD baseline, built from a `git archive` export), `build/agentEQ_wt/build/agentEQ_reg` (the
regression harness), `build/agentEQ_wt/build/agentEQ_clean` (the gate build). Own IDA copy
`build/agentEQ_ida/retail2013_agentEQ.i64`, headless only. **No IDA MCP tool and no FModel tool was used.**
No commits, nothing staged, nothing written into the main checkout.

**This package touches no file in `External/GFx3` and no file in `GFxUI`.**

## The answer in nine lines

* **The nine `ApplyGameSettings` bodies are ported and both `-arksettings` gates are gone.** Measured, by
  real input, from the start screen: three slider moves, each republishing to **22 of 22 listeners**, no
  abort; and the startup profile read publishing to **21 of 21**. Beside it, the untouched HEAD executable
  with `-arksettings` **aborts at 3.16 s**, before the menu exists. Section 3.
* **Two of the nine are not bodies.** `ADishonoredPlayerController::ApplyGameSettings` is **empty in
  retail**, in both builds — its interface vtable slot points at a three-byte `ret 8` that `/OPT:ICF` shares
  with every other empty two-argument virtual. And `ADishonoredGameInfo::ApplyGameSettings` has **no 2012
  body to read at all**: 2012's `DishonoredGameInfo` does not implement
  `Engine.ArkSettingsListenerInterface`. Section 2.
* **A tenth listener had to be ported for the nine to mean anything.** `UWindowsClient::ApplyGameSettings`
  was `UClient`'s empty `{}`, so `UEngine::ApplyGameSettings`' own `Client->ApplyGameSettings` went nowhere
  and no graphics setting could reach the viewport. It and `FWindowsViewport::ApplyGameSettings` are ported
  — **two files in `WinDrv`**, which is more than the brief expected this package to touch. Section 4.
* **`ArkSettings::SaveSettings` is ported, is measured being called, and cannot persist anything here — and
  that is retail's behaviour, not a gap in the port.** `ArkSettings::SaveSettings: SaveProfileData -> 0` is
  in the log the first time the player leaves the options screen with something changed. Everything
  downstream of that event is inside `if (GSteamworksInitialized && LocalUserNum == LoggedInPlayerNum)`;
  offline, retail's own `ReadProfileSettings` (2013 `0x5ad8f0`) sets the profile back to its defaults and
  reports success. Section 5.
* **`ArkSettings::GetSettingProvider` and `PCResolutionSettingProvider` are ported and measured**: 21
  display modes off the RHI, filtered at 800x600, formatted `%4d x %4d`, with the current row resolved
  against `GSystemSettings` — `21 modes, current 11`, and index 11 is `1600 x  900`, the mode in force.
  Section 6.
* **The brief is wrong about where the resolution row is.** It is not "the one row in the options tree with
  no value and no value list": `FillSettingsCategoryList` never carries id 115 at all. The row lives on the
  **video sub-screen**, which `UDisGFxMoviePlayerMenuBase::Req_VideoSettingsScreen` (2013 `0x7dc260`)
  builds out of ids 116, 117 and 115 — an unported native until this package. Section 6.
* **The last acceptance item is not reached, and the reason is measured and is not the port.** A row on the
  options screen can be selected and changed from the keyboard (measured: DOWN then RIGHT changes
  `PSI_Gameplay_KillCamMode`), and the GRAPHICS **category tab responds to a mouse click** (measured:
  `OnTabSelected (2, r)`, `_subCategoriesList.length : 0`). No **key** switches the tab — E, Q, TAB, PAGE UP
  and UP produce no AS2 response, which is agent EO's hand-over 7 — and the follow-up click that would open
  the video sub-screen was lost in five runs out of five to **three other agents' game windows taking the
  foreground**, which every drive log records by handle. Section 7.
* **Two corrections to the truth sources.** The 2013 database names `0x612f00`
  `ADishonoredGameInfo::GetUObjectInterfaceInterface_NavigationHandle`; it is the **ArkSettings** getter in
  2013, and the name is a 2012 ICF fold carried across by the matcher. And agent EO's open judgement about
  `ArkSettingsParameters` member 46 is now a **measurement**: retail's `FWindowsViewport::ApplyGameSettings`
  writes it into `GSystemSettings.bAllowLightShafts`, so it is `m_bLightShaftEnable`. Sections 8 and 2.
* **Regression 37 ok, 0 failed, 0 skipped**; a clean full Release build with the build directory deleted
  first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**; `rva_sweep` over the whole tree:
  **7138 citations, 0 unknown, 0 mislabelled in any file this package touched**. Section 9.

## 1. The nine, body by body

`resources/docs/agents/agentEQ_status.csv` has every address with its verdict and how it was identified.
Every one of the 55 retail 2013 addresses this package cites was resolved by hand in the 2013 database with
`ida_funcs.get_func` (start-or-mid, containing function, name) — `build/agentEQ/resolve_2013.txt`, all 55 on
a function **START**.

| listener | 2013 rva | what the body is |
|---|---|---|
| `ADishonoredPlayerCamera` | `0x6c01f0` | `m_fDefaultFOVSettings` and `ACamera::DefaultFOV` from `m_FOV`; `m_RollAmount` and `m_BobAmount` from `m_fHeadBobAmount` |
| `ADishonoredGameInfo` | `0x5e9db0` | `SetDifficulty(m_Difficulty)`, through the virtual at retail vtable +1052 |
| `ADishonoredPlayerController` | *(folded onto `0x128ad0`)* | **empty**, in both builds |
| `ADishonoredPlayerPawn` | `0x6a1a80` | one bit: `m_bCameraRelativeClimbing` |
| `UDisPostProcessManager` | `0x7e7e30` | `m_PCAntialiasingType`, then `GSystemSettings.iType_AntiAlias`, then `m_PpBridge.m_PpNodeAA->m_Type` |
| `UDishonoredPlayerInput` | `0x6bd610` | the eleven mouse/pad values, plus the binding rebuild for reasons 0 and 3 |
| `UDisItemContext_AimAssistAttack` | `0x7ff710` | five members: the two auto-aim flags and strengths and the elixir flag |
| `UDisGFxMoviePlayerHUD` | `0x7af370` | the fifteen `FDisHUDSettings` members, skipped entirely for `ASLI_ModifiedByUser` |
| `UDisGlobalUIManager` | `0x841250` | `m_bEnableAutoSaveInMenus`, `m_bEnableTutorials`, `m_bEnableBaseTutorials` |

Two more were ported with them because they belong to the same mechanism and would otherwise be wrong:

| | 2013 rva | why |
|---|---|---|
| `UDisItemContext_ProjectileAttack::ApplyGameSettings` | `0x7ffc10` | it **overrides** the aim-assist body in retail (the same five writes spelled out again plus `m_KillCamSettings`); giving the base a body without it would have silently dropped the kill-cam mode |
| `UDisGFxMoviePlayerMenuBase::Req_VideoSettingsScreen` | `0x7dc260` | the only place the resolution row exists (section 6) |

Every offset in every body was read against the retail SDK layout, not guessed. Three of them cross-check
each other and the struct they read:

* `UDishonoredAudioSystem::ApplyGameSettings` (already ported) reads `a1[49..52]` as the four volumes —
  `ArkSettingsParameters` members 49..52 are exactly `m_GlobalVolume`, `m_MusicVolume`, `m_SFXVolume`,
  `m_VoicesVolume`.
* `UDisGFxMoviePlayerHUD::ApplyGameSettings` copies members **17..27** into `FDisHUDSettings`' eleven show
  flags, which is agent EK's eleventh flag confirmed a second time.
* `ADisDLC06GameInfo::ApplyGameSettings` (`0x8bf190`) is `ADishonoredGameInfo`'s body with `+136` in place
  of `+132` — agent EO's `m_Difficulty` and `m_DifficultyDLC06` confirmed from a body neither of us had read.

## 2. The two that were not bodies, and how each was pinned

**`ADishonoredPlayerController::ApplyGameSettings` is empty in retail.** `vtables.csv` (the 2012 table) has
`ADishonoredPlayerController{for IArkSettingsListenerInterface}` at `0xd16598` with slot 1 =
`0xa26ea0` = `UGameViewportClient::SetOnlyUseControllerTiltInput`. That is not a mis-attribution: it is
`/OPT:ICF` folding every empty two-argument virtual onto one three-byte `ret 8`, and the PDB names the fold
after whichever symbol survived. The 2013 build agrees independently: the labelled vftable
`const ADishonoredPlayerController::'vftable'{for 'IArkSettingsListenerInterface'}` at `0xd18730` has slot 1 =
`0x128ad0`, whose whole body decompiles to `;`. **Two builds, two vtables, one empty body.** This tree now
has an empty body with that provenance in its comment, not an `appErrorf`.

**`ADishonoredGameInfo::ApplyGameSettings` is new in 2013 and had to be found from scratch.**
`script_classes_2012.json` gives `DishonoredGameInfo` one interface, `Engine.Interface_NavigationHandle`;
`script_classes_2013.json` gives it two, with `Engine.ArkSettingsListenerInterface` first. So there is no
2012 body, no PDB name, and `match_2012_2013.csv` marks the 2013 function `new`. It was pinned twice:

1. **From the vtable.** `PCResolutionSettingProvider`-style interface tables for this class are two slots:
   the `GetUObjectInterface…` adjustor and `ApplyGameSettings`. A byte search for `lea eax,[ecx-972]; ret`
   (972 is the retail SDK's `VfTable_IArkSettingsListenerInterface` offset in `ADishonoredGameInfo`) finds
   exactly one function, `0x612f00`, and exactly two `.rdata` sites hold it, each followed by a body and a
   null terminator: `0xda20f4` → `0x5e9db0` and `0xdb6204` → `0x8bf190`.
2. **From what the body calls.** `0x5e9db0` is `object->vtable[+1052](Parameters + 132)`. Reading slot
   +1052 out of `ADishonoredGameInfo`'s own primary vftable (`0xcdb838 + 0x41c = 0xcdbc54`) gives
   `0x5e9f60` = `ADishonoredGameInfo::SetDifficulty(EDifficulty)` — a name the 2013 database carries from
   the 2012 PDB. `+132` is `m_Difficulty`. The other site, `0x8bf190`, is the same body reading `+136`
   (`m_DifficultyDLC06`) and sits sixteen bytes after `ADisDLC06GameInfo`'s
   `InitializePrivateStaticClass`, which is how the two DLC subclasses were told apart.

`SetDifficulty` itself is ported with one named gap: retail dispatches game event 9 with the old and the new
value through `FArkGameEventDispatcher::GetInstance()`, and that class is unported in this tree (the same
gap `CppText/ADishonoredPawn.h` already records).

## 3. The gate, and the measurement on both sides of it

Agent EO gated retail's republish loop behind `-arksettings` in `DisGFxMoviePlayerMenuBase`, and agent AM
had already gated `UnUIDataStores.cpp`'s on the same switch. **Both gates are gone.** `DisRepublishSettings`
is now three lines and `UUIDataProvider_OnlinePlayerStorage::OnReadStorageComplete_Native` calls
`ArkSettings::OnSettingsChanged` unconditionally, as retail does.

`ArkSettings::OnSettingsChanged`' loop has no early exit, so one line after it is the record that every
listener was reached. That census line is the measurement.

**Before — the untouched HEAD `2d08c15` executable, `-arksettings`, same schedule, same machine**
(`build/agentEQ/befA_log.txt`):

```
[0003.16] Critical: appError called: DishonoredGame native not ported: UDisPostProcessManager::ApplyGameSettings
```

It never reaches the menu, let alone a slider: at HEAD the **startup profile read** is already enough,
because `-arksettings` opens `UnUIDataStores.cpp`'s gate first. The drive schedule's own log records that
the mark it waits for, `-gfxuimenu: opened UI_MainMenu`, never appeared.

**After — this package, no `-arksettings`, real `SendInput` keys from the start screen, nothing forced**
(`build/agentEQ/aftD_log.txt`):

```
[0022.98] ArkSettings::GetSettingProvider(115): 21 modes, current 11, [ 800 x  600, 1024 x  768, ... 2560 x 1600]
[0023.04] ArkSettings::OnSettingsChanged: reason 1 -> 21/21 listeners, gamma 2.2000, res 1600x900, AA 1
[0095.24] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 2.1000 -> rounded 2, changed 1
[0095.24] ArkSettings::OnSettingsChanged: reason 2 -> 22/22 listeners, gamma 2.1000, res 1600x900, AA 1
[0110.58] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 2.0000 -> rounded 2, changed 1
[0110.58] ArkSettings::OnSettingsChanged: reason 2 -> 22/22 listeners, gamma 2.0000, res 1600x900, AA 1
[0125.25] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 1.9000 -> rounded 2, changed 1
[0125.25] ArkSettings::OnSettingsChanged: reason 2 -> 22/22 listeners, gamma 1.9000, res 1600x900, AA 1
```

reason 1 is `ASLI_ReadProfileFromStorage` (the startup read, 21 listeners exist then) and reason 2 is
`ASLI_ModifiedByUser` (the slider, 22 by then — the options menu's own listener array is the twenty-second).
No `Critical:` line appears anywhere in the run.

**And the whole options screen, end to end** (`build/agentEQ/optA_log.txt`) — Options opened from the main
menu, one row selected and changed, then Escape:

```
[0003.20] ArkSettings::OnSettingsChanged: reason 1 -> 21/21 listeners, ...
[0042.12] AS2 trace: >> TransitionTo (OptionsScreen)
[0092.15] DisOnSettingChange: id 105 (PSI_Gameplay_KillCamMode) mapping 3 data 1 value 2.0000 -> rounded 2, changed 1
[0092.15] ArkSettings::OnSettingsChanged: reason 2 -> 22/22 listeners, ...
[0106.97] AS2 trace: > BPressed () called
[0106.97] ArkSettings::OnSettingsChanged: reason 3 -> 22/22 listeners, ...
[0106.97] ArkSettings::SaveSettings: SaveProfileData -> 0
```

That is the full retail sequence: a **mapping-type-3 row** changed on the options screen (not just the
brightness slider), republished as `ASLI_ModifiedByUser`, then `OnLeaveOptions` republishing as
`ASLI_ValidatedByUser` and calling `ArkSettings::SaveSettings`. The `-> 0` is section 5.

## 4. The tenth listener: `WinDrv`

`UEngine::ApplyGameSettings` (2013 `0x1d87b0`, already ported) hands the parameters to
`Client->ApplyGameSettings`. `UClient::ApplyGameSettings` in `UnClient.h` is `{}`, and `UWindowsClient` had
no override — so the endpoint that carries **every graphics setting that is not a profile value** was a
no-op. Retail has two bodies there and both are ported:

* `UWindowsClient::ApplyGameSettings` (`0x5c4040`, 99 bytes): the loop over `UWindowsClient::Viewports`.
* `FWindowsViewport::ApplyGameSettings` (`0x5c3210`, 260 bytes): for `ASLI_ValidatedByUser` — the reason
  `OnLeaveOptions` publishes — it resizes the viewport when the resolution, fullscreen or vsync differ from
  what is in force, and `FWindowsViewport::Resize` ends in `GSystemSettings.SetResolution`, which **writes
  `ResX`/`ResY`/`bFullscreen` to `GEngineIni`**. Unconditionally it writes four more `GSystemSettings`
  members: `TextureForcedLODBias` from `m_TextureDetails`, `SkeletalLODDistanceFactorMultiplier` from
  `m_ModelDetails`, `bAllowLightShafts` from member 46 and `bAllowRatsShadow` from `m_bRatShadows`.

Every one of those was identified by its byte offset from `GSystemSettings` itself (2013 rva `0x1042f20`).
`FSystemSettings` has a vfptr, so an `FSystemSettingsData` member at data offset *N* is at object offset
*N+4*: `+128` `TextureForcedLODBias`, `+120` `SkeletalLODDistanceFactorMultiplier`, `+76`
`bAllowLightShafts`, `+80` `bAllowRatsShadow`, `+132` `iType_AntiAlias` (which
`UDisPostProcessManager::ApplyGameSettings` writes), `+904`/`+908`/`+912` `ResX`/`ResY`/`bFullscreen`,
`+892` `bUseVSync`.

**Two deviations, both stated in the comment.** Retail's 2013 `FWindowsViewport::Resize` takes the vsync
flag as a parameter and this tree's five-parameter `Resize` does not, so the flag is put into
`GSystemSettings` directly; and retail passes its own stored window position where this passes the `-1/-1`
default, which re-centres a windowed viewport it resizes.

**This is the part of the brief's file list that was wrong.** The brief expected `Engine/Inc/arksettings.h`,
`Engine/Src/arksettings.cpp` and `Engine/Src/UOnlinePlayerStorage.cpp` outside `DishonoredGame`. This
package does not touch `UOnlinePlayerStorage.cpp` at all and does touch `Engine/Src/UnUIDataStores.cpp`
(the second gate) and three `WinDrv` files. Section 10 has the full list.

## 5. `ArkSettings::SaveSettings`, and why nothing survives a restart

`ArkSettings::SaveSettings` (2013 `0x5334e0`) is sixty-six bytes and all of them are one script event:
`FindFunctionChecked(SaveProfileData)` plus `ProcessEvent` on `PC->OnlinePlayerData`, with the event's
four-byte `bool` return as the parameter block. It is ported, it is wired into `OnLeaveOptions` where retail
calls it, and it is measured being called (section 3). `ArkSettings::UpdateSettingsFromSystemSettings`
(`0x53b7b0`) is ported beside it because it is four lines and the same one-line reach through
`OnlinePlayerData->ProfileProvider->Profile`.

**It returns `0`, and it returns `0` in retail too.** `UIDataStore_OnlinePlayerData.SaveProfileData` reaches
`UOnlineSubsystemSteamworks::WriteProfileSettings`, and the whole of that body is behind
`IsSteamClientAvailable() && LocalUserNum == LoggedInPlayerNum`; its write is
`GSteamRemoteStorage->FileWrite`. The read side says the same thing even more plainly — retail 2013's
`UOnlineSubsystemSteamworks::ReadProfileSettings` (`0x5ad8f0`) opens with

```
if ( !GSteamworksInitialized || (unsigned __int8)a2 != *((_DWORD *)this + 62) )
{ ... FindFunctionChecked(a3, ENGINE_SetToDefaults, ...); ProcessEvent(...); return 1; }
```

— offline it puts the profile object back to its defaults and reports success. There is no local-file
fallback anywhere in it: the cloud read is inside the Steam branch, behind `DoesProfileExist()` and a
`GEngine` flag.

**So the brief's second deliverable rests on a premise retail does not meet.** "A setting the player chooses
should survive a restart" is true of retail *with a Steam client*, through Steam Cloud's local disk cache,
and of nothing else. Every run in this tree is `-nosteam` (it is on the command line of every agent's runs,
this package's included), so `GSteamworksInitialized` is FALSE and the write has nowhere to go. Porting more
of the chain would not change that; `execWriteProfileSettings` is still a generated stub, and giving it a
body would still hit `IsSteamClientAvailable()`.

**The one thing retail persists without Steam is the resolution**, and it persists it through
`GSystemSettings.SetResolution` → `GEngineIni`, not through the profile — which is why section 4's
`FWindowsViewport::ApplyGameSettings` is in this package at all. The mechanism is ported and in place; what
stops it being *measured* is section 7, and it is not a settings problem.

## 6. The setting provider, and where the resolution row actually is

`ArkSettings::SettingProvider` is a seven-slot interface, recovered slot for slot by reading the two vtables
retail has for it side by side — the base's at `0xca3f08` (which `match_2012_2013.csv` names
`InvalidDynamicSettingProvider`) and `PCResolutionSettingProvider`'s at `0xca44f4`:

| slot | offset | base | `PCResolutionSettingProvider` |
|---|---|---|---|
| 0 | +0 | scalar deleting destructor `0x533530` | vector deleting destructor `0x5375e0` |
| 1 | +4 | empty | `GetDynamicValueNames` `0x537460` |
| 2 | +8 | returns 0 | `GetCurrentValueIndex` `0x57b5f0` (`return this[4]`) |
| 3 | +12 | returns 0 | `GetInnerValue` `0x537260` |
| 4 | +16 | empty | `SetCurrentValueIndex` `0x7b99b0` (`this[4] = v`) |
| 5 | +20 | empty | `ReadFromSystemSettings` `0x537350` |
| 6 | +24 | empty | `Refresh` `0x537520` |

Slots 2 and 4 are four and thirteen bytes and both are ICF folds, which is why the 2013 database has no name
for either; they were identified by the two call sites that use them — `OnSettingChange` reads slot 2 as the
old value and writes slot 4 with the new one, and `CreateGFxSetting` reads slot 1 then slot 2.

`ArkSettings::GetSettingProvider` (`0x5396a0`) is two function-statics: the resolution provider for id 115,
`Refresh()`ed on every call, and the base for everything else. Retail **inlines the whole body into
`ArkSettingsParameters::Read`** as well — the same `_S5_9 & 1` guard and the same
`GetSettingProvider::5::Provider` atexit appear in both — which is how one function-static is reached from
two functions and why `Read`'s resolution arm is not what it looked like. Retail's arm is:

```
SettingProvider& Provider = GetSettingProvider( 115 );          // Refresh() inside
if ( bOverrideStorageSettingsWithSystemSettings ) { Provider.ReadFromSystemSettings(); m_ResX = GSystemSettings.ResX; m_ResY = GSystemSettings.ResY; }
else                                             { m_ResX = Provider.GetInnerValue( Provider.GetCurrentValueIndex(), 0 ); m_ResY = ...1 ); }
```

Agent EO's note said "retail asks the provider's current value" in the non-override case and it is right;
what it could not know is that the override case *also* touches the provider, through slot 5. That is the
call that makes the row point at the mode in force, and `ReadFromSystemSettings` has **no other caller in
the whole binary**.

Measured (`build/agentEQ/aftD_log.txt`, on retail's own path, nothing forced):

```
ArkSettings::GetSettingProvider(115): 21 modes, current 11, [ 800 x  600, 1024 x  768, 1152 x  864,
 1280 x  720, 1280 x  768, 1280 x  800, 1280 x  960, 1280 x 1024, 1360 x  768, 1366 x  768, 1440 x 1080,
 1600 x  900, 1600 x 1024, 1600 x 1200, 1680 x 1050, 1920 x 1080, 1920 x 1200, 1920 x 1440, 2048 x 1536,
 2560 x 1440, 2560 x 1600]
```

Index 11 is `1600 x  900`, which is what the run is at. **That is the resolution row's value and its value
list**, produced by the code the options screen asks for.

**The row is not in the options tree.** The brief calls it "the one row in the options tree with no value
and no value list". `FillSettingsCategoryList` — retail's, and the port of it agent EO wrote — never carries
id 115 anywhere: its GRAPHICS runs are the video sub-screen row (id −1), the gamma (112), 118..123, 126..130
and 133. Ids 114..117 live on the **video sub-screen**, and the native that builds it,
`UDisGFxMoviePlayerMenuBase::Req_VideoSettingsScreen` (`0x7dc260`), was a `DISHONORED_NATIVE_STUB`. It is
ported: three settings built by hand — 116 `PSI_GraphicsPC_FullScreen`, 117 `PSI_GraphicsPC_VSync`, 115
`PSI_GraphicsPC_Resolution`, in that order, with `m_bDropList` set for 115 alone (retail sets bit 4 of the
flag byte when the id is 115) — handed to `_root.optionsMenu_mc.FillVideoSettings`.

`CreateGFxSetting`'s and `OnSettingChange`'s dynamic arms are ported with it, so a row with no value mappings
of its own now takes its list and its index from the provider and writes the player's choice back into it.

## 7. What the options screen accepts from a PC keyboard, measured

This is the one thing standing between this package and the last two acceptance items, and it is worth
recording precisely because a guess here would be cheap and wrong.

| input on the options screen | result |
|---|---|
| DOWN, then RIGHT | **works** — `DisOnSettingChange: id 105 (PSI_Gameplay_KillCamMode) ... changed 1` (`optA_log.txt`) |
| Escape | **works** — `> BPressed () called`, `OnLeaveOptions`, republish reason 3, `SaveSettings` |
| **mouse click on the GRAPHICS tab** | **works** — `OnTabSelected (2, r)`, `UpdateSubTabs`, `_subCategoriesList.length : 0` (`vidD_log.txt`) |
| RIGHT before any DOWN | nothing |
| UP from the first row (×3), then RIGHT, then ENTER | nothing — the selection does not leave the row list for the tab strip (`optB_log.txt`) |
| E, Q, TAB, PAGE UP as a tab switch | nothing at all (`vidA_log.txt`, `vidB_log.txt`, `optC_log.txt`, `vidE_log.txt`) |

So rows can be selected and changed, the screen can be left, and the category tab **can** be switched — but
only with the mouse. No key reaches it: the tabs are drawn with pad `Y` glyphs (agent EO's
`optD_options.png` shows them) and `FGFxEngine::InitKeyMap` maps no PC key to any pad button except
`XboxTypeS_A`/`XboxTypeS_B`, which it folds onto Return and Escape. That is agent EO's hand-over 7 verbatim,
it is in `GFxUI`, and `GFxUI` is not this package.

**What actually stopped the last two measurements is the desktop, not the code.** Three other agents run
their own copies of this game on this machine, and the driver records the foreground window handle beside
every step it sends. In five consecutive runs the click that would have activated the VIDEO SETTINGS row
was sent while another agent's window held the foreground — `foreground 0x115b2` and `0x515e2` against this
run's own `0x1e14aa`/`0x2014aa`, in `vidC_drive.txt`, `vidD_drive.txt`, `vidE_drive.txt` and
`vidF_drive.txt`. The one click that did land, in `vidD`, is the GRAPHICS tab, and it worked. The driver
already re-takes the foreground immediately before each press (agent EH's fix); it cannot hold it.

**Consequence, stated plainly:** acceptance item 3 is met for the row's *content* (section 6) and not for
its *appearance on screen*; acceptance item 2 is not met, for the reason in section 5 and for this one.
Neither is a defect in the bodies this package ported. Whoever repeats these two measurements should do it
on a desktop with one game window on it, and the sequence is: click the GRAPHICS tab at (0.585, 0.176) of
the client area, click the first row, then LEFT or RIGHT on the resolution row and Escape.

## 8. Corrections

**To the brief.**

1. "Nine DishonoredGame listeners still carry the generated `appErrorf`" — true, but one of the nine
   (`ADishonoredPlayerController`) has an **empty** retail body and one (`ADishonoredGameInfo`) has **no
   2012 body at all**, so neither is a body to port in the ordinary sense. Section 2.
2. "the resolution row is the one row in the options tree with no value and no value list" — id 115 is
   **not in the options tree**. Section 6.
3. "A setting the player chooses should survive a restart" — retail does not do this without a Steam client.
   Section 5.
4. "every file outside `DishonoredGame` — expect `Engine/Inc/arksettings.h`, `Engine/Src/arksettings.cpp`
   and `Engine/Src/UOnlinePlayerStorage.cpp`" — `UOnlinePlayerStorage.cpp` is **not** touched;
   `Engine/Src/UnUIDataStores.cpp` and three `WinDrv` files are. Section 10.
5. The brief says the nine are "the biggest hand-over"; the tenth listener, `UWindowsClient`, was not in the
   list and without it the nine deliver nothing to the viewport. Section 4.

**To the truth sources.**

6. **`0x612f00` is mislabelled in the 2013 database.** It is named
   `ADishonoredGameInfo::GetUObjectInterfaceInterface_NavigationHandle` and its body is
   `lea eax,[ecx-972]; ret`. In 2013 that class's `Interface_NavigationHandle` vfptr is at **976** (its
   getter is `0x612f10`, `lea eax,[ecx-976]`); **972** is the `IArkSettingsListenerInterface` vfptr, which
   2012 did not have. The name is a 2012 ICF fold carried across by the matcher onto a function that means
   something else in 2013. Recorded in the status table; nothing in this tree cites it.
7. **`vtables.csv` is usable for this work** despite being the 2012 table — the interface vtables it lists
   are how `ADishonoredPlayerController`'s empty body was found — but only because every address in it was
   re-derived in the 2013 database before it was believed. The 2013 database also carries `'vftable'` labels
   for eight `{for IArkSettingsListenerInterface}` tables, and those were used the same way: as a lead, then
   confirmed from the class's own constructor (`UDisGlobalUIManager`, `0x85bf30`, writes `0xd88790` into the
   object at +56) or from a body the table points at.
8. **Agent EO's open judgement is settled.** EO wrote that calling `ArkSettingsParameters` member 46
   `m_bLightShaftEnable` rather than the 2012 PDB's `m_PostProcessQuality` "is a judgement, not a
   measurement". `FWindowsViewport::ApplyGameSettings` writes member 46 into
   `GSystemSettings.bAllowLightShafts` and member 48 into `bAllowRatsShadow`. It is a measurement now.

## 9. Gates

* **Regression:** `python resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEQ_wt/build/agentEQ_reg` — the worktree's own copy, an
  absolute build dir, built inside the harness (no `--no-build`), after copying the five gitignored layout
  inputs (`all_types.h`, `retail_sdk_layout.json`, `script_classes_2012.json`, `script_classes_2013.json`,
  `types.json`) into the worktree. **37 ok, 0 failed, 0 skipped.** `build/agentEQ/reg1.log`.
* **Clean gate build:** the build directory deleted first, full Release, `DISHONORED_LAYOUT_CHECKS=ON`, all
  three targets: **0 errors, 0 C4263, 0 C4264.** `build/agentEQ/buildclean.log`.
* **`rva_sweep.py`** over the whole worktree: **7138 citations, 4116 `ok-2013`, 2998 `ok-2013-mid`, 22
  `ok-2012-labelled`, 0 unknown, 0 `UNKNOWN-CLAIMED-2013`, 2 `MISLABELLED-2012`**. Both of the two are the
  pre-existing pair agent EO reported, in `GFxUI/Src/gfxuirenderer.cpp:1669`, in a line that does say
  "2012"; nothing in any file this package touched is flagged. `build/agentEQ/sweep.csv`.
* **By hand, not by sweep:** all 55 retail 2013 addresses this package cites were resolved in the 2013
  database with `ida_funcs.get_func`, start-or-mid and name. All 55 land on a function **START**:
  `build/agentEQ/resolve_2013.txt`. The ones with no name in the 2013 database — `0x6c01f0`, `0x5e9db0`,
  `0x8bf190`, `0x7e7e30`, `0x7af370`, `0x841250`, `0x5c3210`, `0x537460`, `0x537520`, `0x57b5f0`,
  `0x539730`, `0x7cb870`, `0x7db5f0`, `0x8bf170`, `0x8aff10`, `0x7bbc00`, `0x8a8240` — were each identified
  by their callers, their callees or the vtable slot that holds them, and each identification is stated in
  `agentEQ_status.csv`. Six of the addresses cited are data, not code — `0xca3f08`, `0xca44f4`, `0xd18730`,
  `0xd88790` and `0xcdb838` are vtables and `0x1042f20` is `GSystemSettings` — and each was read out of the
  2013 database dword by dword (`build/agentEQ/dump1.txt`, `dump2.txt`, `dump4.txt`, `dump6.txt`) rather
  than taken from a comment.

## 10. Merging

* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` IS REQUIRED and WAS run**,
  twice (once for the ten `Inc/CppText` declarations, once more after `Req_VideoSettingsScreen` was ported).
  Unlike agent EO's package it has content-bearing output beyond one file, because a cpptext declaration is
  what stops the generator emitting the `appErrorf` pure-virtual stub:
  * `Sources.cmake` — **three units leave the exclude list, 818 → 815**: `dispostprocessmanager.cpp`,
    `disitemcontext_aimassistattack.cpp`, `disitemcontext_projectileattack.cpp`. They need no external
    registration of their own (agent EN's rule): each defines a virtual the class's vftable references, and
    the vftable is emitted in `DishonoredGameRegistrants.cpp`, so the linker pulls the object in. Measured:
    the clean build links.
  * `DishonoredGameCameraClasses.h`, `dishonoredgameclasses.h`, `DishonoredGameItemClasses.h`,
    `DishonoredGameUIClasses.h`, `DishonoredGameWeaponRangedClasses.h` — the nine `appErrorf` stubs removed
    and one cpptext include added.
  * `Src/DishonoredGameNativeStubs.cpp` — `execReq_VideoSettingsScreen` removed.
  * `DishonoredGame.h`, `DishonoredGameRegistrants.cpp` and the other seventy-odd generated headers came
    back byte-identical apart from line endings and are deliberately not in the copy list.
* **Files outside `DishonoredGame`: six.**
  * `Engine/Inc/arksettings.h`, `Engine/Src/arksettings.cpp` — **agents EK and EO both touched these;
    expect to merge all three packages' changes together.**
  * `Engine/Src/UnUIDataStores.cpp` — agent AM's `-arksettings` gate removed.
  * `WinDrv/Inc/WinDrv.h`, `WinDrv/Src/WinClient.cpp`, `WinDrv/Src/WinViewport.cpp` — the tenth listener
    (section 4). Additive: two new function bodies and two declarations, nothing existing changed.
* **Total: 37 files** — 35 source plus this report and its status table. The 35: **3** in `Engine`, **3**
  in `WinDrv`, **29** in `DishonoredGame` (10 `Inc/CppText`, two of them new; 1 new
  `DishonoredGameNativeStubs.ported.agentEQ.txt`; 7 generated — `Sources.cmake`, five class headers and
  `Src/DishonoredGameNativeStubs.cpp`; 11 `Src` bodies). `build/agentEQ_sync.py` is the authoritative copy list and runs worktree → main;
  `--check` reports without copying. Nothing is committed and the worktree is left dirty.

## 11. Hand-over

1. **The options screen's category tabs have no PC key** — the mouse switches them, no key does (section
   7). One entry in `FGFxEngine::InitKeyMap` — or honouring `[GFxUI.KeyMap]` and `FullKeyboard=1` in
   `DishonoredInput.ini`, which it ignores — makes the video sub-screen reachable from the keyboard, and
   with it the resolution row on screen and the only Steam-free persistence this game has. Everything on the
   C++ side of it is now in place. `GFxUI`, agent EO's hand-over 7.
1b. **Two measurements are owed and both need a quiet desktop**, not a code change (section 7): the
   resolution row drawn on the video sub-screen, and a resolution change surviving a restart through
   `GEngineIni`. The click sequence is written down at the end of section 7.
2. **The profile is not persisted and cannot be without Steam** (section 5). If a session is ever wanted
   that survives a restart offline, it is a *deliberate deviation from retail*, not a port: something would
   have to write `UOnlinePlayerStorage::ProfileSettings` to a file. `execWriteProfileSettings` is still a
   generated stub and `UOnlineSubsystemSteamworks::WriteProfileSettings` (reference body, present) is behind
   `IsSteamClientAvailable()`.
3. **`UDisGFxMoviePlayerHUD::ApplyGameSettings`' five follow-up calls** are still not ported (agent EK's
   gap, unchanged): `0x78cf30` (take the player info down), `0x7ad010` (the tutorials), `0x79f0d0` (the
   pickup log), `0x79f170` (the marker instances) and the heart's highlight re-arm. A HUD setting changed at
   runtime is obeyed by whatever reads `m_Settings` on its next tick and by nothing else.
4. **`UDishonoredPlayerInput::ApplyGameSettings`' binding half** is agent DO's `BuildBindings()` stand-in.
   Retail's `ReadPCBindingsFromProfile` (`0x6b8240`), `TranslateBaseBindings` (`0x6baa80`) and
   `InitGameActionBindings` (`0x6bac10`) chain and its `FGFxEngine` call for the five movement/use glyph
   keys are not ported, so a rebound key does not reach the bindings and the menus' key glyphs are not fed.
5. **`ADishonoredGameInfo::SetDifficulty` does not dispatch its game event** — `FArkGameEventDispatcher` is
   unported. Anything that wants to hear a difficulty change hears nothing.
6. **`ArkSettings::ResetSettings` (`0x53c070`) and `ResetAllSettings` (`0x53c3d0`)** are still not ported,
   which is why `OnResetOptions` sets the changed flag and redraws without resetting anything.
7. **`UDisGFxMoviePlayerMenuBase::OnApplyVideoSettings` (2012 `0x7f7290`)** is still a stub — the video
   sub-screen's own Apply. `Req_VideoSettingsScreen` fills the screen; this is what commits it.
8. **`ADisDLC06GameInfo::ApplyGameSettings` (`0x8bf190`)** is read but not ported, along with the rest of
   the DLC06 game info. It is two lines whenever that class is reached.

## 12. Scratch

`build/agentEQ/` — the decompiles (`dec2013/`), the IDA scripts (`dec.py`, `dis.py`, `dump.py`, `hunt.py`,
`names.py`, `scan.py`, `scantext.py`, `strs.py`, `resolve.py`, `callers.py`), the build logs, the drive
logs, the run logs, the sweep CSV, `resolve_2013.txt`, and `measure.py`, which starts the game and the
driver together so a run's schedule and its log are kept side by side. `build/agentEQ_run.py` is the runner,
`build/agentEQ_sync.py` the copy list, `build/agentEQ_ida/` the IDA copy, `build/agentEQ_headsrc/` the
`git archive` export of HEAD `2d08c15` and its build. None of it is part of the package.
