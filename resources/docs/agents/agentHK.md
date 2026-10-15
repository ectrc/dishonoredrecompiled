# Agent HK: the start screen gives the first player its controller, and Bink is on by default (wave 25, PHASE22 package HK)

Opus 5.5. Worktree `build/agentHK_wt`, build dir `build/agentHK` (RelWithDebInfo), staged as
`Dishonored_Latest2026/Binaries/Win32/DishonoredGame_HK.exe`.

The worktree started at `ccdc25c`. It was then moved onto main's new commits by `git apply` of its diff:
`6ae222a` → `835dc2b` → `9da5dbc` → `d9be103` → `52cbde4`. None of those commits touched a file of this package. After each
move the generator re-ran byte-identical (79 files) and `bitfield_audit` reported 0 mismatches.

Three builds, not the two the plan allowed:

| build | base | configure | what ran on it |
|---|---|---|---|
| 1 | `ccdc25c` | `-DDISHONORED_WITH_BINK=ON` | the diagnosis runs (§3) |
| 2 | `835dc2b` | Bink switch unset | the five hand-route runs (§4) |
| 3, final | `9da5dbc` | Bink switch unset | the gate and run HKF1, ordered by the coordinator |

A build on `6ae222a` was stopped half-way when main moved.

Builds 2 and 3 configured with `-UDISHONORED_WITH_BINK -UDISHONORED_WITH_BINK_DEFAULT_ON`. That removes both cache entries, so
the cache came up `DISHONORED_WITH_BINK:BOOL=ON` from the new default alone. The last two moves (`d9be103` from HL, `52cbde4`
from HB) touched no GFx, input or engine-loop file, so they were not rebuilt.

## Result

**A player who goes through the menu by hand, with movies on, now reaches the boat using only the keyboard, as in retail.**
The run used no `-finewgame` and no `-nomovie`:

1. Space on the start screen sets `GamePlayers(0)->ControllerId = 0`. It does this through retail's own `SelectController`
   and the menu script's `SetControllerId`.
2. Enter four times: NEW GAME → NORMAL → brightness ACCEPT → YES.
3. The briefing page waits for a key. Space, Enter or a mouse click each leave it.
4. The clouds (the INTRO_LOC movie) play, and the boat ride starts.

Six clean hand-route runs, 0 Criticals. HG's `-finewgame` controller-0 bringup is removed.

## 1. What retail does (both dbs, by hand)

| function | 2013 (VA / rva) | 2012 rva | notes |
|---|---|---|---|
| `UDisGFxMoviePlayerMainMenu::FilterButtonInput` | VA 0xbe1f40 / rva 0x7e1f40 | 0x822590 | no csv match; found from the xrefs of `_root.startScreen_mc` |
| `UDisGFxMoviePlayerMainMenu::SelectController` | VA 0xbc1d20 / rva 0x7c1d20 | 0x7f4a70 | |
| `UGameViewportClient::InputKey` | VA 0x6b9430 / rva 0x2b9430 | 0x2cb720 | Arkane added a controller-selection arm |
| `UDishonoredEngine::StopMovie` | rva **0x5e4590** | 0x62bc30 | |
| `UDishonoredEngine::PreExit` | rva **0x5e4520** | 0x62bbc0 | |
| `FGFxEngine::ReevaluateFocus` | VA 0x986b60 | 0x5ca660 | same as the reference: a priority walk with an early-out. Ours matches it |

**`FilterButtonInput`.** On the start screen (login step 1), on key press, 2013 does this:
* calls `SelectController(ControllerId)`;
* fires the script event `ReadProfileSettings(0)` (its FName is at VA 0x145d970, set in `AutoGenerateNamesDishonoredGame` at 0x9e588c);
* sets `m_bShowTitleScreen` (+612 bit 0);
* hides the message box;
* closes the screen and moves to step 7 / screen 2.

2012's step-8 write `GEngine@700 |= 8` is gone in 2013.

**`SelectController`.** It calls `UnregisterControllerDelegates(old)`, `SetControllerId(id)` and
`RegisterControllerDelegates(id)`, then stores the id in `m_DelegatesControllerId`. 2013 also sets two engine bits:
`m_bAcceptControllerDisconnectionEvents` (0x10 @700) and `m_bAcceptProfileReading` (0x20 @700). When the id changes it calls
0x459d10. That is a one-byte `ret` that ICF folded with `UDishonoredTask_Base::OnAdded_Impl`, so there is nothing to port.

**`UGameViewportClient::InputKey`.** When `m_bWaitingControllerSelection` (0x8 @700) is set and the key is `XboxTypeS_A`, it
gives `GamePlayers(0)` that pad's controller id and clears the bit. A scan of the 2013 db finds no other native write to that
bit, so whatever sets it is script.

**`StopMovie`.** It sleeps until `m_dSaveNotificationMovieEndTime`, then calls `UEngine::StopMovie` (0x1d8900).

**`PreExit`.** In order:
1. 0x5e4100: frees the 2026 telemetry task and calls `curl_global_cleanup`. Left out, as `Init` already leaves out 0x601210.
2. `UDisBinkOverlayManager::CleanUp`.
3. Deletes the save lister and the save deleter.
4. `UGameEngine::PreExit`.
5. Frees `m_pGameState` with a plain `operator delete`.

On PC the keyboard is controller 0 for the start screen: the viewport sends keyboard and mouse keys with `ControllerId` 0, and
`SelectController` takes that id.

## 2. What landed

* **`UDisGFxMoviePlayerMainMenu::SelectController`**, a new member declared in `Inc/CppText/UDisGFxMoviePlayerMainMenu.h`, and
  the start-screen arm of `FilterButtonInput` as 2013 has it. `PreAdvance` clears `m_bShowTitleScreen` when the menu opens
  (2012 0x822280, 2013 login step 8). `-pbinkdiag` logs
  `start screen: SelectController(0) -> first local player controller id 0`.
* **`UGameViewportClient::InputKey`** now has the 2013 controller-selection arm.
* **The GFx hand-route moved.** The plan did not foresee this, and the rest only works because of it. Once the first player
  owned controller 0, the main menu stopped receiving keyboard keys (§3).
  * In retail, `GlobalInteractions` is `[Console, UGFxInteraction, UIInteraction, PlayerManagerInteraction]`.
    `GameViewportClient.Init` inserts `GetCustomInteractionClass` under `WITH_GFx`.
  * This tree has no `UGFxInteraction`. It offered GFx the key by hand, but only after the whole list.
  * `PlayerManagerInteraction` now finds a player for controller 0. It sends Enter to `DishonoredPlayerInput`
    (`Dis_PlayerChoice_RequestSkip`), which consumes it, so GFx never sees it.
  * The fix offers the key and the mouse axis to GFx at `UGFxInteraction`'s slot, right after the console (`UnPlayer.cpp`,
    marked `DISHONORED(bringup)`). With controller -1 nothing changes, because `PlayerManagerInteraction` routes nothing.
* **`UDishonoredEngine::StopMovie` and `UDishonoredEngine::PreExit`.** The overlay manager's `CleanUp` now runs at exit, and
  every run exits cleanly.
* **HG's `-finewgame` controller-0 assignment is removed.** The `-finewgame` log line stays, without the `-> 0`.
* **`cmake/Bink.cmake`.**
  * `DISHONORED_WITH_BINK` now defaults to ON.
  * `option()` keeps the cached OFF in every existing build dir (`head_rel`, `gate_wt`). A one-shot cache migration
    (`DISHONORED_WITH_BINK_DEFAULT_ON`) turns them ON on their next configure.
  * A missing retail `binkw32.dll` is now a WARNING, not a FATAL_ERROR.
  * `build-release.cmd` and `build-game.cmd` never passed the switch, so they are unchanged.
  * Measured with `DISHONORED_RETAIL_DIR` pointing at a missing tree: the configure prints the Bink warning and goes on
    (`build/agentHK/nodll_configure.log`). It then stops on PhysX's own FATAL_ERROR, which predates this package. With
    `-DDISHONORED_WITH_PHYSX=OFF` it configures to the end (`nodll_configure2.log`).

## 3. How the keyboard problem was found (measurements)

| run | what was done | what it showed |
|---|---|---|
| HK2 | Space assigns controller 0, then Enter ×4 on the menu | no AS2 trace; `Dis_PlayerChoice_RequestSkip` fires instead |
| HK4 | `exec set DisLocalPlayer ControllerId -1` at runtime, then Enter | `APressed`, then `TransitionTo (NewGameScreen)` |
| HK6 | `-gfxuicensus`, Enter at controller 0, then at -1 | 0 GFx key downs at controller 0; 1 at -1 |
| HK7, HK10 | `build/agentHK/tools/focus_peek.py` (`ReadProcessMemory` only) | `PlayerStates(0).FocusedMovie` is `UI_MainMenu.MainMenu` (focus 1, input 1, login step 8, screen 3) the whole time, so focus was correct |
| HK11 | `-gfxuikey`, straight into `FGFxEngine::InputKey(0, focused movie, ...)` | the key is handled at controller 0, so the key was lost before GFx was asked |
| `obj list class=Interaction` | list the interactions | Console, UIInteraction, PlayerManagerInteraction and the player's input; `UPlayerManagerInteraction::InputKey` (`UnPlayer.cpp:162`) routes keys by controller id |

## 4. Hand route

Switches: no `-finewgame`, no `-nomovie`; `-windowed -ResX=1920 -ResY=1080 -pbinkdiag`; ini isolation. Command:

`python build/agentHK/tools/hk_run.py --exe .../DishonoredGame_HK.exe --log HKR<n> --tag hkr<n> --menu build/agentHK/tools/menu_keys.txt --page-key <key>`

| run (build) | page key | controller after Space | key wait start → page left (s) | intro | boat | Criticals |
|---|---|---|---|---|---|---|
| HKR1 (2) | SpaceBar | 0 | 59.07 → 70.01 | INTRO_LOC played, skipped with Escape | EnterSoireeMode | 0 |
| HKR2 (2) | Enter | 0 | 98.08 → 104.39 | yes | yes | 0 |
| HKR3 (2) | click 0.5 0.5 | 0 | 76.74 → 82.43 | yes | yes | 0 |
| HKR4 (2) | SpaceBar | 0 | 99.91 → 107.28 | yes | yes | 0 |
| HKR5 (2) | SpaceBar | 0 | 83.61 → 91.09 | yes | yes | 0 |
| HKF1 (3, final exe) | SpaceBar | 0 | yes | yes | yes | 0 |

HKR3's click is logged as `movie key LeftMouseButton ... consumed 1`. Space and Enter are turned into Escape by the loading
movie's own key map (`FDisFullScreenMovieBink::InputKey`, as HG ported it). Mouse buttons are excluded from that map, and a
click still leaves the page.

**Page scores.** HG's `build/agentHG/measure.py` on the `-pbinkdiag` movie frames (`build/agentHK/shots/hkr*_page2_bink.png`),
SSIM median against retail t 4.0-5.5:

| run | map name | chapter title + description | PRESS ANY KEY |
|---|---|---|---|
| HKR1 | .829 | .895 | .891 |
| HKR2 | .892 | .928 | .905 |
| HKR3 | .934 | .951 | .927 |
| HKR4 | .877 | .921 | .902 |
| HKR5 | .888 | .926 | .904 |

The chapter title and PRESS ANY KEY areas are at least .89 on every run. The map name is at least .83 on every run and at
least .87 on 4 of 5.

**Clouds.** Against retail t 8.1 the clouds score only .72-.77, because the shot is late. The runner queues the `wait 2.4`
when the log shows the intro start; on this loaded machine the game read the command file 4-20 s into the 39 s movie.
Matched against HG's INTRO_LOC decode (`build/agentHG/movies/INTRO_LOC`, 2 frames per second), every shot is INTRO_LOC:

| run | best INTRO_LOC frame | SSIM |
|---|---|---|
| HKR1 | 14 | .971 |
| HKR2 | 11 | .939 |
| HKR3 | 45 | .923 |
| HKR4 | 47 | .898 |
| HKR5 | 10 | .988 |

The movie is right; the capture time is not retail's 2.4 s.

## 5. Gate (build 3, `9da5dbc`, Bink ON)

`run_regression.py --build-dir build/agentHK --no-build --exe-name DishonoredGame_HK.exe --log-prefix HKg_`:
**30 ok, 4 failed, 0 Criticals** in every stage (nullrhi, d3d9, inputtest, newgame, options). Other numbers:
`newgame_on_mission` 120, `newgame_pawns` 29, `options_steps` 6, `options_as2_errors` 39.

The four failures:
* **`coresmoke_passed` and `layout_types`**: the build cmd only builds the DishonoredGame target, the same as for HG.
* **`unported_natives` 4**: `ADishonoredPlayerPawn::execOnToggle{Tutorial,AchievementEval,Choke,Journal}`. They came in with
  HI and were fixed by HL in `d9be103`, which build 3 does not contain.
* **`inputtest_moved` 216** (392 on the first attempt), under load. Re-run alone with `--only inputtest`: 1051.0 on the HK
  exe against 1046.0 on HJ's exe, both stages 9 ok. It is noise, not this package.

The gate stages pass `-nomovie` through `-playdefaults`, so the startup movies never block an unattended run.

## 6. Where the plan was wrong (with the measurement)

1. **"Port the start screen's controller assignment and the page works."** Assigning the controller alone made the main menu
   stop receiving keyboard keys (HK2, HK6, HK11 in §3). The GFx hand-route had to move to `UGFxInteraction`'s slot.
2. **"2012 0x5e4520 / 0x5e4590."** Those are 2013 rvas. The 2012 functions are `PreExit` 0x62bbc0 and `StopMovie` 0x62bc30.
3. **"The 2012 start-screen filter sets `m_bWaitingControllerSelection`."** 2013's filter does not. The only native write to
   that bit in 2013 is the clear in `UGameViewportClient::InputKey`.
4. **"Drop `-DDISHONORED_WITH_BINK=ON` in the second build to prove the default."** That proves nothing: the cache keeps the
   ON from build 1, and every older build dir keeps its OFF. The default was proven by removing both cache entries with `-U`,
   and the migration makes the flip reach existing build dirs.
5. **"A build without the retail DLL configures."** True for Bink. `cmake/PhysX.cmake` still fails without the retail tree.
6. **"Click NEW GAME."** A scripted `click` on the main menu does nothing, on build 1 as well (HK1: no `Click` or `APressed`
   trace). The route uses Enter, as agent FU's did. A click on the briefing page does work.
7. **"Clouds SSIM ≥ .83 against retail."** That only holds if the capture lands at movie t ≈ 2.4 s. The clouds were measured
   against the movie's own frames instead (§4).

## 7. Open items

* `ReadProfileSettings(0)` now runs at the start screen, as in retail. With HJ's `SetToDefaults` it resets the profile. The
  options stage still passes; persisted profile reads are HJ's area.
* `m_bWaitingControllerSelection` has no writer in this tree. In retail it is set by script; not measured.
* Scripted mouse clicks on the main menu are not handled by the movie (§6, item 6).
* `UGFxInteraction` itself is still not in `GlobalInteractions`, because this tree builds with `WITH_GFx=0`. The slot
  emulation is a `DISHONORED(bringup)`. `InputChar` has no GFx route at all, as before.

## Files

* Engine: `source/Development/Src/Engine/Src/UnPlayer.cpp`.
* DishonoredGame sources: `Src/disgfxmovieplayermainmenu.cpp`, `Src/dishonoredengine.cpp`, `Src/dishonoredplayercontroller.cpp`.
* DishonoredGame headers: `Inc/CppText/UDishonoredEngine.h`, `Inc/CppText/UDisGFxMoviePlayerMainMenu.h` (new).
* Generated: `Inc/DishonoredGameUIClasses.h` (one CppText include line).
* Build and docs: `cmake/Bink.cmake`, `cmake/Dependencies.cmake`, `resources/docs/middleware.md`.
* Tools (not synced): `build/agentHK/tools/hk_run.py`, `focus_peek.py`, `hk_ida.py`, `menu_*.txt`.
* Sync script: `build/agentHK_sync.py`, base `52cbde4`.
