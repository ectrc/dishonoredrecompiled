# Agent FU — the mouse cursor is the global movie's, Enter already works, and the menu leaks AS2 objects

Wave 21, PHASE17 package FU. Base `d7d083a`, worktree `build/agentFU_wt`, working build
`build/agentFU_wt/build/fureg`, the gate's in-harness build `build/agentFU_wt/build/fuclean`. Every game run
through `resources/tools/game_input.py` (never a real key, mouse or focus), every launch windowed
(`-playdefaults` appends `-windowed`, and from the coordinator's rule on it is also passed by hand), every build
through `build_queued.py`. Evidence in main's `build/agentFU/` (`fu_*.log` are in the retail
`DishonoredGame/Logs`, the captures under `shots/`, the memory series `fu_h*_mem.txt`, the scripts
`fu_ctrl1.txt`, `fu_idle.txt`, `fu_route.txt`, `fu_mem.py`).

> **The headline.** The cursor is now retail's: `UDisGFxMoviePlayerGlobal::PreRender` from
> `UDishonoredViewportClient::Draw`, gated on `m_bMouseCursorAllowed`, and it is gone from the mission
> (`shots/fur1_fu_mission.png`, `fu_r1.log`: `mouse cursor: allowed 0 ... NOT attached | map l_tower_p`, 31 lines,
> against the control's `clip 32x32 visible 1` on the same route). **Defect 5 does not reproduce**: on the
> untouched HEAD control, through the sanctioned driver, Enter activates the highlighted row, Down moves the
> difficulty selection, Escape goes back (§2). **And the 2 GB exhaustion has a mechanism**: the AS2 runtime never
> frees an object - `AS2 objects 66471 live / 66471 created` at 30 s, `644869 / 644869` at 150 s on the idle menu,
> with the interface off the process is flat (§4).

## 0. What landed

| | |
|---|---|
| `UDisGFxMoviePlayerGlobal::PreRender`, `ConditionalShowMouseCursor`, `ConditionalHideMouseCursor`, `MoveMouseCursor` | the four retail bodies (2013 `0x79eb90`, `0x78c790`, `0x7945b0`, `0x78c8c0`), with `m_pMouseCursor` (@444) as the heap `GFxValue` retail keeps there |
| `UDisGFxMoviePlayerGlobal::PostStart`, `PreClose`, `PreCommitMapChange` | 2013's shapes (`0x7aa430`, `0x7aa4d0`, `0x79eb30`): the value's life from Start to Close, the stripes latch and the priority-0 boxes dropped on close, the hide before a map change |
| `UDisGFxMoviePlayerBase::ComputeMovieSpaceInfo` | 2013 `0x787860`, the real member; GFxUI's and the HUD's private copies were inlines of it |
| `UDishonoredViewportClient::Draw` | retail's two statements (2013 `0x5ea330`): the open global movie's PreRender, then the engine's Draw |
| `GGFxMoviePlayerPreCloseHook` | the one new seam in GFxUI: `UDisGFxMoviePlayerBase::Close` calls the virtual `PreClose(bUnload)` before the engine's Close, and this tree's shared `UGFxMoviePlayer::Close` now offers that point |
| `DishonoredGFxUpdateMouseCursor`, `DishonoredGFxReleaseMouseCursor`, GFxUI's `FDisMovieSpaceInfo` copy | **deleted**, with the Engine call and the two externs |
| `UDisGlobalUIManager::RefreshGlobalUIState` | **one expression**: the "top" movie is the focus movie, not `OpenMovies(0)` (§1.2) |
| `UDisGlobalUIManager::PreCommitMapChange` | the global movie's own PreCommitMapChange first, retail's order |
| `DISHONORED_PLAY_DEFAULTS` | the `-gfxuimenu` row removed; README, `build-play.cmd`, `dismod_harness.md` corrected |
| the GFx UI census | one more pair on the "machine" line: `AS2 objects %u live / %u created` |

## 1. Where the brief was wrong

1. **"Enter does not activate a main-menu row" (defect 5).** It does, on HEAD `80cc219`, through `game_input.py`:
   `fu_h2.log:1456-1473` - `key Enter` → `watch INVOKE 'InputDelegate'` → `AS2 trace: > APressed () called` →
   `TransitionTo (NewGameScreen)`; `shots/fuh2_enter1.png` is the difficulty screen. Then `key Down` moves NORMAL →
   HARD (`fuh2_down1.png`), `key Up` traces `UpPressed () ... _curLine : 2`, `key Escape` traces `BPressed` /
   `BackToMainMenu` (`fuh2_esc1.png`). Agent FI's four silent Enters were measured with the retired
   `drive_input.py` against the `-gfxuimenu` bring-up movie, which agent FB's probe showed as
   `canFocus 0 | canInput 0`; agent FP replaced that movie with the Kismet-opened one (cooked `allowFocus 1,
   allowInput 1`) and nobody re-measured. The key path itself (`FGFxEngine::InitKeyMap` → `GFxKey::Return` = 13 →
   the movie's `InputManager` → `InputDelegate("A")`) was never broken. README and the defect text corrected;
   STATUS is the coordinator's.
2. **"`m_bMouseCursorAllowed` is computed exactly as retail does" (agent FP, PHASE17 §FU).** It read 0 on the menu
   too: every `UI state` line of every run on HEAD ends `cursor 0`, and FP's menu capture showed a cursor only
   because the old glue drew it whenever a Global movie was open. Retail's `RefreshGlobalUIState` (`0x847070`)
   reads `FGFxEngine+40`, which is **not** `OpenMovies` (+16) but the per-player `FocusMovies` array that
   `ReevaluateFocus` (`0x586b60`) fills with the topmost open movie that can receive focus; `OpenMovies` is sorted
   ascending (`InsertMovieIntoList`, `0x585350`: insert before the first entry of higher priority), so
   `OpenMovies(0)` is the lowest-priority movie, `UI_HUDFX`, which never takes input. Ported as the focused movie
   of player 0, else the topmost `bCanReceiveFocus` movie - what retail's list holds.
3. **"One seam, named by FP."** Two: the bit itself was wrong (above), and the glue that could not read it. With
   only the glue moved, my first route run read `allowed 0` on the menu and the mission alike (`fu_r1.log`).
4. **"Retail's `ConditionalHideMouseCursor` is 2013 `0x7945b0`"** - right, and `ConditionalShowMouseCursor`
   `0x78c790`, `MoveMouseCursor` `0x78c8c0`, `PreRender` `0x79eb90`. The 2013 `PostStart` (`0x7aa430`) and
   `PreClose` (`0x7aa4d0`) are **not** the 2012 bodies the skeleton comments carry: 2013's PreClose also clears
   `m_bHasBlackStripes` (which is why agent FP saw one `SetBlackStripes(0) NOT FOUND` on the reopened movie),
   `m_bMsgBoxSet` and `m_bTripleScreenCinematicMode` and removes every queued box of priority 0; 2013's PostStart
   advances the view once (`GFxMovieView::Advance(0, 2)`) and re-shows the box at the head of the queue.
5. **"Up/Down on the difficulty and brightness screens consume the key without moving the selection."** Half.
   Difficulty: Down moves the row (§2). Brightness: Left, Right, Up and Down each reach the movie
   (`watch CALL 'InputDelegate'` at pc 8043 / 8086 / 7957 / 8000) and **the slider does not move** - the white
   marker spans x 596..782 on the slider row before and after the four keys (`fur1_fu_bright.png`,
   `fur1_fu_bright2.png`, read off the files). Mouse drag moves it (agent FD). Open; §6.
6. **The 2 GB item's framing ("what allocates per frame at the menu").** Right question, and the answer is not an
   allocation per frame of the renderer: with `-gfxuinodisplay` (no display walk) the growth is unchanged, with
   `-gfxuinotext` it halves, with `-nogfxui` it is zero. It is the ActionScript machine (§4).

## 2. Defect 5 on the control, measured

`DishonoredGame_fuhead.exe` is `build/gate_rel` (HEAD `80cc219`, the coordinator's gate build of it; the brief's
`build/head_rel` was still building when this package needed a control), staged under this package's name. The
command line is the user's plus the harness isolation: `-nosteam -skipnativepkgs=OnlineSubsystemPC -ENGINEINI=...
-GAMEINI=... -INPUTINI=... -UIINI=... -playdefaults -fmmenu -dismousecursor -gfxuiwatch=InputDelegate`, script
`fu_ctrl1.txt`:

| step | log | capture |
|---|---|---|
| `key Enter` on the start screen | `main menu: start screen dismissed by Enter`, `TransitionTo (MainMenuScreen)` | `fuh2_menu1.png` |
| `key Enter` on NEW GAME | `APressed () called`, `Close : _level0.mainMenu_mc`, `TransitionTo (NewGameScreen)` | `fuh2_enter1.png` (EASY/NORMAL/HARD/VERY HARD, NORMAL lit) |
| `key Down` | `InputDelegate` at pc 8000, no trace line (DownPressed does not trace) | `fuh2_down1.png` (HARD lit) |
| `key Up` | `!>>>>>UpPressed () [object Object]_curLine : 2 / _nbLines-1 : 3` | - |
| `key Escape` | `BPressed () called`, `fscommand (FromNewGameScreen)`, `BackToMainMenu` | `fuh2_esc1.png` |

All captures 1600x900 read off the file, in-process. `grep -c FocusLost` 0 apart from the quit line.

## 3. The cursor, measured

`fu_r1.log` + `shots/fur1_*.png` on the working build (`DishonoredGame_fu.exe`), script `fu_route.txt`: start
screen, main menu, difficulty (Down), brightness (Left/Right/Up/Down), the confirmation box, YES, 50 s, the boat.

- Mission: `mouse cursor: allowed 0 closing 0 | clip NOT attached ... | map l_tower_p`, every second from 48.26 s
  to the quit; `fur1_fu_mission.png` and `fur1_fu_mission2.png` have no cursor where the control's
  `fuh2_*` route and FP's `fpm9_mission.png` had one left of centre. 0 criticals, `Exit: Exiting` at 100.24 s.
- Menu on that build: `allowed 0` as well (§1.2) - the one-expression fix is in the gate's build, whose exe the
  last section measures.
- The `-finewgame` crash path agent FP fixed (a cursor handle kept across the global movie's close and reopen) is
  structurally gone: the handle lives in `m_pMouseCursor`, released and deleted by `PreClose` before the view, and
  `PostStart` makes a new one. The gate's `newgame` stage runs that path.

## 4. The 2 GB exhaustion: the AS2 machine never frees an object

External sampler (`fu_mem.py`, `GetProcessMemoryInfo` on the pid every 5 s - it touches nothing in the game),
HEAD control idle on the main menu after one Enter, 150 s, private bytes:

| run | switch | 45 s | 95 s | 150 s | per second |
|---|---|---|---|---|---|
| `fu_h3` | - | 721.6 MB | 751.4 | 787.8 | +0.63 MB/s |
| `fu_h4` | `-nogfxui` | 606.8 | 606.5 | 606.3 | **flat** |
| `fu_h5` | `-gfxuinotext` | 720.3 | 738.5 | 758.3 | +0.36 MB/s |
| `fu_h6` | `-gfxuinodisplay` | 721.1 | 750.5 | 791.4 | +0.67 MB/s |
| `fu_r2` | this package, `-gfxuicensus` | 731.6 | 763.4 | 800.1 | +0.65 MB/s |

So the growth is the interface's, not the renderer's (the display walk off changes nothing) and only partly text.
The census pair added to the "machine" line reads, on `fu_r2census.log`: 30 s `AS2 objects 66471 live / 66471
created`, 90 s `351947 / 351947`, 150 s `644869 / 644869` - **about 4,800 objects a second, 16 per movie advance,
and the live count never drops below the created count**. `External/GFx3/GFxAS2Object.h:171` says why:
`GASObjectCollector` is "the teardown half of retail's garbage collector ... and nothing else"; every object the
content creates is chained and freed only when its movie closes. At ~150 bytes each that is the 0.65 MB/s, and at
the user's 216 fps (`scene rendered (46890 so far)` at 217.73 s) 2 GB in the time STATUS records. Retail's
`GASRefCountCollector` (heap id 323 in the demangled names) is the missing piece; it is a GFx3-runtime package, not
this one, and nothing here changes the runtime beyond the census line. The `-gfxuicensus` line also shows
`sprites created` climbing 8/s and `display objects placed` 21/s on an idle menu - the tweens attach and the
objects behind them are what accumulates.

## 5. Addresses, by hand, both databases

Private copies `scratchpad/idb/fu2013.i64` / `fu2012.i64`; 2013 rvas are VA − `0x400000`, 2012 rvas have image base 0.

| function | 2013 rva (size) | 2012 rva (size) | pinned by |
|---|---|---|---|
| `UDisGFxMoviePlayerGlobal::ConditionalShowMouseCursor` | `0x78c790` (295), named | `0x7f6070` (295) | `_root.mouseCursor_mc`, `attachMovie("mouseCursor","mouseCursorInst")` |
| `ConditionalHideMouseCursor` | `0x7945b0` (252), named | `0x7fa780` (252) | `removeMovieClip`; callers PreRender, PreClose, PreCommitMapChange |
| `MoveMouseCursor` | `0x78c8c0` (89), named | `0x7f61a0` (89) | `SetDisplayInfo` with flags 3 |
| `PreRender` | `0x79eb90` (200), named | `0x802c60` (200) | the only caller of Show; `+440 & 4`, `+380 & 0x200` |
| `PreCommitMapChange` | `0x79eb30` (87) | `0x802c50` (5) | Hide + the message-box queue `Shrink` on `0x106e660` |
| `PostStart` | `0x7aa430` (153) | `0x7f4640` (103) | `appMalloc(0x10, 8)` into +444, the Register delegate name, `ShowMessageBox` on the queue head |
| `PreClose` | `0x7aa4d0` (305) | `0x802e60` (110) | Hide, `operator delete` of +444, `RemoveMessageBox` (`0x7aa230`) over the queue |
| `UDisGFxMoviePlayerBase::ComputeMovieSpaceInfo` | `0x787860` (208), named | `0x7f4390` (208) | ratio 1.000 bytes |
| `UDishonoredViewportClient::Draw` | `0x5ea330` (64), named | `0x62ff50` (64) | `DisGetGlobalUIManager`, `+724`, `+196 & 1`, PreRender |
| `UDisGFxMoviePlayerBase::Close` | `0x793a90` (188) | `0x7fa080` (188) | agent FP's pin |
| `UDisGlobalUIManager::RefreshGlobalUIState` | `0x847070` (901) | `0x8b75e0` (776) | agent FP's pin; `GGFxEngine+40` read at `0xc470ad` |
| `FGFxEngine::ReevaluateFocus` | `0x586b60` (1962) | `0x5ca660` | walks `+16` from the top, fills `+40` per `LocalPlayerOwnerIndex`, raises `OnFocusLost`/`OnFocusGained` |
| `FGFxEngine::InsertMovieIntoList` | `0x585350` (229) | `0x5c9160` | insert before the first entry whose `+262` (Priority) is greater |
| `FGFxEngine::InsertMovie` | `0x58d4a0` (112) | `0x5ce820` | `AllMovies` at +28, `OpenMovies` +16, `DPGOpenMovies` +80 |
| `UDisGFxMoviePlayerMainMenu::FilterButtonInput` | - | `0x822590` (2012, named) | the start-screen arm this tree has; retail falls through to `MenuBase` (`0x813790`) and the base (`0x7f45a0`), which pass every key but BackSpace and the thumbsticks |

`m_pMouseCursor` is `FPointer` at 444 (`DishonoredGameLayouts.h:9583`); `bDisplayingUIMouseCursor` is the
`UGameViewportClient` bit at 164 mask 2 that Show sets and Hide clears, on `UGameEngine::GameViewport`.

## 6. Left open, on purpose

- **The brightness slider by keyboard** (§1.5): the keys reach `InputDelegate("left"/"right")`; the handler
  behind the gamma screen's entry never traces and the slider stays. `handleAnalogStickInput` /
  `UpdateAnalogInputForAS` (2012 `0x81d5b0`, unported) is the suspect: the AS2 `InputDelegate` routes the four
  directions through `InputsArray[id].dirX/dirY`, which the native analogue path writes. One `-gfxuiwatch` run
  per candidate name would settle it; not reached.
- **The AS2 collector** (§4) is the fix for the 2 GB; it is the runtime's, sized by retail's
  `GASRefCountCollector`, and it is not in this package.
- The 2013 `FGFxEngine` member at +40 is reconstructed by meaning (the focus movie), not by moving
  `PlayerStates`; this tree's `FGFxEngine` layout is not retail's and nothing here depends on it being so.
- `RegisterOnlineDelegates` / `UnregisterOnlineDelegates` are raised as script events when the cook has them
  (it does); with `-nosteam` their bodies log and do nothing.
- `-dismousecursor` stays as the one-line-a-second probe, now reporting `allowed`, `closing`, attached, position
  and map.

## 7. The gate, as the coordinator re-scoped it mid-package

Two builds: `build/fureg` (this package's working build, `fu_build1.log`, 0 errors) and the harness's own build of
`DishonoredGame` into `build/fuclean` (`fuclean/regression/build_DishonoredGame.log`: **0 errors, 0 C4263, 0
C4264**). The harness's CoreSmoke and LayoutProbe targets were then queued behind two other packages' builds and
the coordinator asked for them to be cancelled as the "clean build" PHASE17 dropped; they were, and the seven
stages ran with `--no-build` against the gate-built `DishonoredGame.exe` with HEAD's `CoreSmoke.exe` and
`LayoutProbe.exe` copied from `build/gate_rel` (no reflected member moved in this package, and Core's one change
is a command-line table CoreSmoke does not drive). That is **41 checks**, the six build rows being the ones
`--no-build` omits; the result is at the end of `build/agentFU/fu_regression.log` and in the hand-back.

**Result** (`build/agentFU/fu_regression.log`, 893 s): **37 ok, 4 failed, 0 skipped** of 41. The four:
`d3d9/unported_natives 1`, `options/options_log_lines 1371`, `options/options_steps 0`, `options/options_alive -1`.
Every one is the controller of §7a: the d3d9 stage's native is `ADishonoredPlayerController::
execDis_WheelShortcuts_GamepadSelect` (a pad-only native; a re-run read eight, all `*_Gamepad*`,
`LeanWithAnalogStick`, `SneakOrSlide`, `PlayerChoice_RequestSkip`, with `XboxTypeS` lines in the log), and the
options stage's log ends at 15.5 s with `start screen dismissed by XboxTypeS_DPad_Left` → `Do you want to quit the
game?` → `OnQuitGameConfirm: 'exit'` → `appRequestExit(0)`. **The same two stages on the shared control
`build/head_rel`** (`fu_regression_control.log`, `--no-build --only d3d9,options`): `unported_natives 0` on a run
with no pad line, and `options_steps 5 (wanted >= 6)` with `start screen dismissed by XboxTypeS_RightTrigger` -
the pad dismissed the start screen before the script's `start` step, which then reads `NOT FOUND`. Two further
re-runs of this package's exe (`fu_regression_rerun.log`, `fu_regression_rerun2.log`) went the same way with
`XboxTypeS_A` and `XboxTypeS_B`. The 41 rows are green on both executables exactly where the controller was quiet
and red on both where it was not; no bound was relaxed, and the merge gate on a quiet machine is the measurement
this package could not take.

## 7a. The gate's exe, and a hole in rule 1

`build/agentFU_wt/build/fuclean/Binaries/Win32/DishonoredGame.exe`, built by the harness, staged as
`DishonoredGame_fugate.exe` and driven with `fu_route.txt` (`fu_g1.log`): on `Dishonored_MainMenu` the probe reads
`mouse cursor: allowed 1 closing 0 | clip attached at 640.0,360.0 visible 1` once a second (14 lines, 10.3 s to
23.5 s), on `l_tower_p` `allowed 0 closing 0 | clip NOT attached` (74 lines), 0 criticals, `Exit: Exiting` at
99.82 s. That is the bit, the attach and the hide all behaving as retail's PreRender orders them.

**What that run also shows, and the coordinator should know**: it reached the mission before this package's
first scripted key. `fu_g1.log:799` `GFx input probe: first key 'XboxTypeS_DPad_Left'`, `:1098 start screen
dismissed by XboxTypeS_DPad_Left`, then six `APressed` from the pad walked QUIT GAME → NO → NEW GAME → ACCEPT →
ACCEPT → YES; `fu_g2.log` the same from `XboxTypeS_A`. **A connected XInput controller is read under `-inputcmd`**:
`FWindowsViewport::ProcessInput`'s joystick poll (`WinViewport.cpp:1182-1218`, `XInputGetState`) is not behind
`DishonoredInputGrabDisabled()`, which gates the keyboard, the mouse and the window activation only. Every run
before 03:19 was clean (no `XboxTypeS` line in `fu_h2`, `fu_r1`, `fu_r4`), so the pad went live on the user's side
during this package; the two gate-exe captures (`fug1_fug_main.png`, `fug2_menu.png`) are therefore the loading
screen and the boat, not the menu. The menu-cursor evidence on the gate exe is the log line above; the menu
captures with the cursor drawn are the control's (`fuh2_menu1.png`, the old glue) and would be one pad-free run
of `fu_menu_g.txt` away. Not patched here: `WinViewport.cpp` is the coordinator's file this wave (the
fullscreen guard), and this package's two builds are spent.

## 8. Files, and every function touched

`build/agentFU_sync.py`'s docstring is the per-function list; sixteen source and doc files, two new docs.
