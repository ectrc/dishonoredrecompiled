# Agent IT — the newgame gate stage "fails on main tonight"

Wave 26, PHASE23 package IT. Worktree `build/agentIT_wt` at fd3f90b (code = 91352d7), RelWithDebInfo, two builds
(`build/agentIT_build1.log` = clean base, `build/agentIT_build2.log` = the fix). Sync: `build/agentIT_sync.py`, base f33df83.

## 1. Answer

Main does not fail the newgame stage. Agent IR's runs all used **IR's own Debug exe**, and that exe found a real leak that
RelWithDebInfo cannot see:

- **Why only IR's runs failed.** `build/agentIR/CMakeCache.txt` says `CMAKE_BUILD_TYPE=Debug`. `make_snapshot.py` writes
  `-DCMAKE_BUILD_TYPE=Debug` into `build/agent<X>_wt_build.cmd`, and the shared rule ("RelWithDebInfo, not the Debug
  default") was not applied. In Debug, `DO_GUARD_SLOW` turns on `VerifyLevelsGotRemovedByGC` (`Engine/Src/UnWorld.cpp:2043`).
  That check is what raises `appError("Streamed out levels were not completely garbage collected")`. The same build also
  prints `DevConfig` lines and about 4,846 `LocalizationWarning` lines per run. IR's are the only logs tonight that do.
- **What IR's two "controls" really ran.** `run_regression.py --no-build --exe-name N` still **stages**: `stage_retail.stage()`
  copies `<build-dir>/Binaries/Win32/DishonoredGame.exe` over `N`. So IR's "head_rel" control
  (`--exe-name DishonoredGame-Win64-Shipping.exe`) and its "build 1" control (`DishonoredGame_IRb1.exe`) both ran IR's final
  Debug exe. All three files have md5 `28923c2f…`. The original `DishonoredGame-Win64-Shipping.exe` was overwritten.
- **The leak itself is real, and it is on main in every configuration.** The divergence from retail: our
  `UDisGFxMoviePlayerMainMenu::PreClose` was half-ported. Retail's (2013 rva **0x7d2790**, 2012 **0x7f7030**) ends with
  - `UnregisterCommonDelegates` (only when the movie is exactly the main menu class and `m_pOnlineSubsystem` is set), then `m_pOnlineSubsystem = 0`
  - `UnregisterControllerDelegates(m_DelegatesControllerId)`, then `m_DelegatesControllerId = -1`

  Ours only did the manager's `m_pMainMenu` reset, in the Closed seam. HK's `SelectController` (2013 0x7c1d20, merged in
  wave 25) calls the script's `RegisterControllerDelegates` when the start screen is dismissed. Without the matching
  unregister, the menu stays in `OnlineSubsystemSteamworks_0.DeviceIdCache.DeviceSelectionDelegates`, and that keeps the
  whole `Dishonored_MainMenu` world alive after the map change. It has leaked since HK. Only a Debug exe reports it.
- **The Steamworks subsystem does not clear the cache on a map change.** No such clearing exists in retail: retail relies
  on the menu's PreClose to unregister. The fix therefore goes in PreClose, not in the online subsystem.

## 2. Bisection (newgame stage, `-playdefaults -finewgame=50.0 -fidiag -fmmenu -fmdrive=8:start`, d3d9, `-nosteam`)

Every exe was copied into a private `build/agentIT_runs/b_<tag>` directory and staged under a new name, so no other
agent's staged exe was touched. Summary: `build/agentIT_runs/bisect_summary.txt`. Logs: `ITb_<tag>__newgame.log`.

| exe (md5 / source) | build | criticals | pawns | on_mission | result |
|---|---|---|---|---|---|
| DishonoredGame_IR.exe (IR final, = IR's "head_rel" and "b1" controls) | Debug | 3 | -1 | 0 | FAIL (IR: 3 of 4, and the 4th, IRg2, hit the 300 s timeout mid-trace: its log has the same "didn't get garbage collected" chain) |
| build/head_rel exe of 02:46 (= main 24c7074; md5 = DishonoredGame_IO2m.exe) | RelWithDebInfo | 0 | 29 | 91 | pass |
| DishonoredGame_IS.exe | RelWithDebInfo | 0 | 29 | 90 | pass |
| DishonoredGame_IP.exe | RelWithDebInfo | 0 | 29 | 118 | pass |
| DishonoredGame_IO.exe | RelWithDebInfo | 0 | 29 | 118 | pass |
| **IT build 1 = main 91352d7, clean** | RelWithDebInfo | 0 | 29 | 119 | pass |

The environment was not a factor:
- Steam has been running since 2026-10-10 20:58, so it was up for every passing gate too.
- Every gate stage runs with `-nosteam`.
- `build/agentIR/config/*.ini` differs from IO's only by the `[WinDrv.Accessibility]` block that a crashed exit does not write back.
- No game held a save.

## 3. Leak measurement in RelWithDebInfo

Run with `game_input.py launch`, using the newgame arguments. At world t≈134 s, on l_tower_p, the probe queues
`exec obj garbage`, then `obj list class=DisGFxMoviePlayerMainMenu`, `class=World` and `class=Level`
(`build/agentIT_runs/probe.sh`, logs `ITp_base1.log` and `ITp_fix1.log`):

| | DisGFxMoviePlayerMainMenu | World Dishonored_MainMenu | Level Dishonored_MainMenu | Worlds |
|---|---|---|---|---|
| base (91352d7) | 1 (`DisGFxMoviePlayerMainMenu_0`) | alive | alive (7,172 B) | 10 |
| fix | 0 | gone | gone | 9 |

Script warnings are identical in both runs: 7, the same set.

## 4. The fix

The file is `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp`, function `DisGFxMoviePlayerPreClose`. It is the `GGFxMoviePlayerPreCloseHook`, called
from `UGFxMoviePlayer::Close` ahead of `CloseScene`, which is where retail's `UDisGFxMoviePlayerBase::Close` (0x793a90) calls
the virtual PreClose. The fix adds a main-menu branch with retail's delegate half, in retail's order.

Deviations, named:
- **The texture-package half of retail PreClose stays unported.** That is the `ReleaseTexturePackage` of
  `m_MapLargeImagePackage[0]` (2013 `sub_C5B0B0`), reached through `GWorld->GetGameInfo()+0x41c`. `UDisGlobalUIManager::ReleaseTexturePackage`
  has no body in this tree. The `m_pMainMenu = NULL` stays in the Closed seam (agent FP).
- **The `UnregisterCommonDelegates` branch is dead today.** Our PostStart (agent BE) skips retail's
  `m_pOnlineSubsystem = GameEngine->OnlineSubsystem` and its `RegisterCommonDelegates` call, so `m_pOnlineSubsystem` stays
  NULL. The branch is ported so that it pairs correctly once PostStart's half is ported.

## 5. Gate on the fix (`DishonoredGame_IT.exe`)

See `build/agentIT_runs/gate_newgame_{1,2,3}.txt` and `gate_d3d9_inputtest.txt` (`--log-prefix ITg<n>_`):

| run | criticals | pawns | on_mission | other |
|---|---|---|---|---|
| newgame 1 | 0 | 29 | 119 | rc 0 |
| newgame 2 | 0 | 29 | 118 | rc 0 |
| newgame 3 | 0 | 29 | 118 | rc 0 |
| d3d9 + inputtest | d3d9 0, inputtest 0 | | | unported_natives 0, probe_natives 3, inputtest_moved 1016.2; rc 0 |

3 of 3. Note: RelWithDebInfo passed before the fix as well (section 2). What the fix changes is the leak in section 3.
A Debug exe was not built to re-run the DO_GUARD_SLOW check, because the rules allow two builds and require RelWithDebInfo.

## 6. Where the brief is wrong

- **"Fails on head_rel (main's own exe)."** The exe IR ran was not main's. IR's control re-staged its own Debug build
  over `DishonoredGame-Win64-Shipping.exe` (md5 identical to `DishonoredGame_IR.exe`). Real head_rel (02:46) passes, and
  so does main 91352d7 built RelWithDebInfo.
- **"Something changed between yesterday and tonight."** Nothing in main changed the outcome. The leak has existed since HK.
  What changed was the build type: IR's Debug exe is the first build to run `DO_GUARD_SLOW`.
- **"The Steam client is now running."** It has been running since 10-10. Every gate run uses `-nosteam`.
- **"Bisect with `--no-build --exe-name` against the staged exes."** Doing that against another agent's build dir
  overwrites the staged exe with whatever that build dir holds right now, which is exactly how IR's controls went wrong.
  Copy the exe into a private build dir first.
- **"Retail clears DeviceSelectionDelegates / the device-id cache on map change."** It does not. Retail's menu
  unregisters in its own PreClose, so the fix belongs there.

## 7. Recommendations (not done here)

- `make_snapshot.py`: make RelWithDebInfo the default in `CMAKE_OPTIONS` (line 31), or refuse Debug without a flag.
- A periodic Debug gate (`DO_GUARD_SLOW`) would catch level leaks like this one. Gates do not run in Debug today.

## 8. Processes

- I started `DishonoredGame_ITbase.exe` pid 24760 at 03:41:29 by mistake: a bisect tag collision staged the old
  `DishonoredGame.exe` (09-29) under that name. I stopped its driver, but the auto-mode classifier refused the kill of the
  game process. It had exited on its own by 03:54. The staged `DishonoredGame_ITbase.exe` is that old exe;
  `DishonoredGame_ITITbase.exe` is the real base.
- Other agents' processes seen during the work: DishonoredGame_IH4, DishonoredGame_IO2, DishonoredGame_IO2b1 and
  DishonoredGame_IS (2). Still running at hand-back: DishonoredGame_IH4 (pid 25176) and DishonoredGame_IO2 (pid 22224).
  None of mine are left.
