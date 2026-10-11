# Agent FP — the HUD on every screen, one owner for every movie player, and the second letterbox

Wave 20, PHASE16 package FP. Base `15e8a10`, then main's `b759fd5` (the scripted-input rule) merged into the
worktree by `patch -p1` the moment the coordinator asked; main moved on to `e14b934` (FO, FQ) while the gate
ran, and none of those files are this package's. Worktree `build/agentFP_wt`, build dir
`build/agentFP_wt/build/fpreg` for the probes and `.../build/fpclean` for the gate, every game run through
`resources/tools/game_input.py` (never a real key, mouse or focus) with this agent's own exe name, log name,
ini dir and `-apshotname` tag. Evidence in main's `build/agentFP/` (probe.py, `fp_m*.log` pointers, the
captures under `shots/`, the build and regression logs).

> **The headline.** The user's report reproduces on the untouched HEAD executable through the scripted driver
> (`shots/before_menu.png`: the health and mana gauge top-left of the main menu) and is gone on this package's
> (`shots/fpm9_menu.png`, `fpm9_diff.png`, `fpm9_bright.png`: main menu, difficulty, brightness, **no HUD**;
> `fpm9_mission.png`: the whaleboat **with** the HUD and the letterbox; `fpm9_confirm.png` the message box with
> the blur and the global letterbox behind it). All captures 1600x900 read off the file, taken in-process by
> `game_input.py`'s `shot`, on the final build; `fpm8_*` are the same route one build earlier. But the brief's diagnosis was only a third of the cause, and the thing that actually
> opens the main menu in retail turned out not to exist in this build at all. §1.

## 0. What landed

| | |
|---|---|
| `UDisGlobalUIManager::RefreshGlobalUIState` | the whole of 2013 `0x847070`: HUD `fVisible`/`fUpdate`, the FX movie's, `m_pGlobal`'s stripes and cursor bits, `EnableControllerInput`, the loading-menu blur seed; agent FA's blur arm kept verbatim |
| `OnMovieStackChanged` | the thunk, called from `Start` (the Started seam) and `Close` (a new Closed seam) of every Dishonored movie player |
| `UDisGlobalUIManager::Init` + `FindCurrentMovieSet` (2013 shape) + `CreateMoviePlayer`, `LoadSFTweaksPackage`, `InitItemIconPackages` | the one owner: constructs pHUD, pPowerWheel, pJournal, pNote, pPauseMenu, pMissionStats (or pBrief/pResultsMenu) under the manager, starts pGlobal and the HUD, from `ADishonoredGameInfo::InitGlobalManagers` |
| `UDisGlobalUIManager::PreCommitMapChange` / `PostCommitMapChange` and the two `AGameInfo` C++ virtuals they hang off | the HUD and the global movie come back after New Game's map change — without these the mission had no HUD at all (§1.4) |
| `UGFxAction_OpenMovie::Activated`, `UGFxAction_CloseMovie::Activated` | **compiled for the first time**: retail's owner of the main menu, which this tree had as an empty virtual (§1.2) |
| `UDisGFxMoviePlayerBase::execStart/execClose/execAdvance/execPreLoad` | were `DISHONORED_NATIVE_STUB`s, so the Kismet open answered FALSE (§1.3) |
| `UDisGFxMoviePlayerBase::LoadMoviePackage`, the PreLoad prologue (tweaks off the Kismet action, movie off the tweaks) | how pGlobal gets `UI_Global.Global` with Init never touching its `MovieInfo` |
| `UDisGFxMoviePlayerGlobal::PreAdvance`'s stripes arm | the second letterbox, one `SetBlackStripes` argument, off `m_bRequestBlackStripes` |
| `FGFxEngine::RenderUI` | **one condition fixed**: `fVisible` gates first, as retail's `0x58dd30` (§1.1) |
| `DisHUDBringUp` and the `-gfxuimenu` bring-up | **deleted**, both; `-gfxuimenu` is accepted and means nothing, `-nodishud` still keeps the calibrated stages movie-free |
| `ADishonoredPlayerController::EnableControllerInput` | the one-line mask write, 2013 `0x6a0b00` |

## 1. Where the brief was wrong

Six premises, each measured.

1. **"Port the rest of RefreshGlobalUIState and the HUD will hide."** Ported, it would still have drawn. This
   tree's `FGFxEngine::RenderUI` tested `(fVisible && bHudEnabled) || bDisplayWithHudOff`; retail's (2013 rva
   `0x58dd30`, 845 bytes, named) tests `fVisible` first and only then `bHudEnabled || bDisplayWithHudOff`. The
   HUD's cooked default is `bDisplayWithHudOff = 1` (table in §2), so `fVisible = FALSE` was a no-op for exactly the
   movie it is meant for. One condition in `gfxuiengine.cpp`.
2. **"Retail has one owner for every movie player: UDisGlobalUIManager::Init."** Not for the main menu. Agent FB's
   retail object dump (`build/agentFB/retail/ue.log:11223`) has the live menu at
   `Dishonored_MainMenu.TheWorld.PersistentLevel.Main_Sequence.DisGFxActionOpenMovie.DisGFxMoviePlayerMainMenu`
   — it is opened by the menu map's **Kismet**, `UGFxAction_OpenMovie::Activated` (2013 `0x59d3a0`, 1147 bytes,
   pinned by its `AddCaptureKey` call and three UTF-16 variable names; 2012 `0x5e51a0`, named). In this tree that
   body lived in `ScaleformInteraction.cpp` under `WITH_GFx`, which `cmake/DishonoredDefines.cmake` sets to 0
   everywhere, **and** that unit is on `GFxUI/Sources.cmake`'s exclude list: the class had no `Activated` override
   at all. `-diskismet` counted `DisGFxActionOpenMovie_0` as activated on every run while it dispatched into
   `USequenceAction::Activated` and opened nothing. That is the measured gap agent DC wrote `-gfxuimenu` for
   ("never constructs a UGFxMoviePlayer"). Fixed with `Src/gfxuikismet.cpp` and two CppText declarations; the
   `-gfxuimenu` bring-up could then go. The manager's Init constructs the other six and starts two; it never
   touches `m_pMainMenu`, which `UDisGFxMoviePlayerMainMenu::PostStart` (2012 `0x821e00`, first line) sets and
   `PreClose` (`0x7f7030`) clears.
3. **The Kismet open then failed anyway**: `DISHONORED(bringup): DishonoredGame native not ported:
   UDisGFxMoviePlayerBase::execStart (parameters consumed, result zeroed)`. `DisGFxMoviePlayerBase` redeclares
   `Start`, `Close`, `Advance` and `PreLoad` as script natives over `GFxMoviePlayer`'s, and all four were stubs in
   `DishonoredGameNativeStubs.cpp`. Neither symbol table names a body for them (they fold onto
   `UGFxMoviePlayer::execStart`, 2012 `0x5b5b10` / 2013 `0x571120`); they are those four bodies through the
   virtuals. `fp_m5.log:1063` is the FALSE, `fp_m6.log:1063` the first `opened UI_MainMenu.MainMenu through
   DisGFxMoviePlayerMainMenu` from Kismet.
4. **"Init runs on every map, so the HUD is always open."** Init runs once per game info, and New Game is a
   `CommitMapChange` on the same persistent world, not a `LoadMap`: `fp_m7.log` reached `l_tower_p` with every
   movie closed by agent FI's `NotifyGameSessionEnded` and **no HUD** (`shots/fpm7_mission2.png`). Retail reopens
   it from `UDisGlobalUIManager::PostCommitMapChange` (2013 `0x83daa0`: `m_pGlobal->Start(FALSE)` then
   `StartHUD`, unconditional; 2012 `0x8aedc0` only restarted a closed HUD) and closes it first from
   `PreCommitMapChange` (2013 `0x841120`), both reached from `ADishonoredGameInfo`'s **C++ virtuals** (2012
   vtable slots 246 / 247; 2013 `0x605c50` and `0x607370`, pinned by `ClearAllRelationOverrides` /
   `Term_GlobalMusic` and by the `Init_GlobalMusic` + manager call pair). This tree's `UGameEngine::CommitMapChange`
   raised only the reference engine's *script* events, guarded on a function 2013's cook does not have, so
   nothing on the game info was ever told. Two empty virtuals on `AGameInfo`, two calls in `UnGame.cpp`, the
   overrides on `ADishonoredGameInfo` with the UI manager's share written and the rest named.
5. **"FindCurrentMovieSet is `sub_C51970`, medium confidence."** Confirmed exactly: it reads
   `GWorld->PersistentLevel`'s outermost name, walks `m_DLCUI` with a 160-byte stride, `appStristr`s each
   `m_MapFilter` into the map name and copies the non-empty `FDisUIMovieSet` properties through the struct's
   `UProperty` list. Called once, from `0xc5fe03` inside Init. High confidence now.
6. **`m_bShowHUDWhileActive`'s defaults** are right and do reach our objects — §2 — so the expression was never
   the problem; items 1–4 were. The brief's instinct to check them first was still correct: it cost one log line.
7. **And the gate found one more**, not in the brief: the first full run read 44/3, the two extra rows being the
   `newgame` stage dying 2.0 s into `l_tower_p`, deterministic on `-finewgame` (fp_ng1..ng4) and never on the
   clicked route (fp_m7, m8). Resolved with agent FJ's ASLR method (`build/agentFP/resolve_crash.py`: runtime
   base `0xcd0000`, crash `0x1c0f1d3` → map VA `0x133f1d3` = `GASEnvironment::FindTarget+0x43`). The cause was
   this tree's, not retail's: `DishonoredGFxUpdateMouseCursor` kept a `static GFxValue` for the attached cursor
   clip across frames and never dropped it when the global movie closed; with the manager now closing and
   reopening `UI_Global` on every map change, the next `SetDisplayInfo` went through a released view. Retail
   drops the clip in `UDisGFxMoviePlayerGlobal::ConditionalHideMouseCursor` (2013 `0x7945b0`) from `PreClose`
   and `PreCommitMapChange`; `UGFxMoviePlayer::Close` now calls `DishonoredGFxReleaseMouseCursor` before the
   view goes, and the update voids a handle whose movie is not the one on the stack. `fp_ng5.log`: 0 criticals,
   29 pawns. Why it never showed on the clicked route is the usual reason a use-after-free hides: the freed
   memory had not been reused yet.

And one correction to the tree rather than the brief: `dishonoredgameinfo.cpp` said `UDisGlobalUIManager::Init`
was 2013 rva `0x8b8d10`; it is `0x85fdb0` (FM's number), and `0x8b8d10` is not a function start in either
database. The comment is corrected at the site.

## 2. The cooked defaults, read out of the cook

Logged by Init from each class default object — those objects are deserialised from the cooked
`DishonoredGame.u` this build loads, which is the cook speaking (`fp_m8.log`, lines 454–461 of any run):

| class | showHUD | blur | stripes | pauseHUD | cursor | allowFocus | allowInput | captureInput | hudOff | closeOnLevelChange | priority | tweaks |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| DisGFxMoviePlayerGlobal | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 1 | 6 | Twk_InGameUI.Twk_GFxMoviePlayerGlobal |
| DisGFxMoviePlayerHUD | 1 | 0 | 0 | 0 | 1 | 0 | 1 | 0 | 0 | 1 | 2 | Twk_InGameUI.Twk_GFxMoviePlayerHUD |
| DisGFxMoviePlayerMainMenu | **0** | 0 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 4 | none (the Kismet action's `m_pTweaks`) |
| DisGFxMoviePlayerGamma | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 1 | 5 | Twk_InGameUI.Twk_GFxMoviePlayerGamma |
| DisGFxMoviePlayerPauseMenu | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 4 | Twk_InGameUI.Twk_GFxMoviePlayerPauseMenu |
| DisGFxMoviePlayerJournal | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 4 | Twk_UI_Journal.Twk_GFxMoviePlayerJournal |
| DisGFxMoviePlayerNote | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 5 | Twk_InGameUI.Twk_GFxMoviePlayerNote |
| DisGFxMoviePlayerPowerWheel | 1 | 0 | 0 | 0 | 0 | 1 | 1 | 0 | 1 | 1 | 3 | Twk_InGameUI.Twk_GFxMoviePlayerPowerWheel |

So the AND across the open stack is `MainMenu = 0` on the front end and `1` everywhere else; the brightness screen
(Gamma, `showHUD 1`) opens on top of the main menu, which stays open, so the AND is still 0 there. The HUD's own
player has `bDisplayWithHudOff = 0` in the cook — the old bring-up had forced it to 1 by hand, which is one more
reason the RenderUI precedence mattered. The manager's `RefreshGlobalUIState` census line reads, with the menu
open: `HUD visible 0 update 1, stripes 1, cursor 0, input blocked 0`; on `l_tower_p`: `HUD visible 1`.

Note `neverDisplayHUD 1` on the start-up world `DishonoredGameFull_P` and on the menu map: `AWorldInfo
+752 mask 0x400` is set there, so the HUD is off on the front end by two independent terms. The one in the
brief (`m_bShowHUDWhileActive`) and this one.

## 3. The route, measured

`build/agentFP/probe.py` is retired by the input rule; every run below is
`game_input.py launch ... -- -nosteam -skipnativepkgs=OnlineSubsystemPC -ENGINEINI=... -playdefaults -fmmenu
-fmdrive=8:start`, i.e. the user's command line plus the isolation, and the script
`wait 22 shot menu / hover 0.497 0.80, click, shot diff / hover 0.297 0.57, click, shot bright / key Enter, shot
confirm / key Enter, wait 50, shot mission`. Coordinates are viewport fractions read off the first capture.

| run | exe | what it showed |
|---|---|---|
| `fp_before.log`, `shots/before_menu.png`, `before_diff.png` | main's `build/gate_rel` (HEAD) | the user's report: the health and mana gauge on the main menu and on the difficulty screen |
| `fp_m2`–`fp_m5` | this package, in steps | §1.2–1.3: Init opens the HUD and the global movie at 5 s, `HUD visible 0`, and **no menu at all** until the Kismet action compiled and its natives answered |
| `fp_m6.log` | | the menu from Kismet (`opened UI_MainMenu.MainMenu through DisGFxMoviePlayerMainMenu (focus 1 input 1 capture 1 hudoff 1)`), `UI state: 4 open ... MainMenu(showHUD 0 ...) -> HUD visible 0`, `Global: SetBlackStripes(1) ok` |
| `fp_m7.log` | | the whole route; the mission reached with **no HUD** — §1.4 |
| `fp_m8.log`, `shots/fpm8_*.png` | one build before the final | menu / difficulty / brightness with no HUD; the confirmation box with the blur and the global letterbox behind it; `HUD: reopened UI_HUD.HUD after the map change`, `UI state ... -> HUD visible 1`, `HUD: SetBlackStripes(1) ok`, and the whaleboat with the gauge and the bars |
| `fp_ng1`–`fp_ng4` | | the `newgame` stage's crash reproduced on `-finewgame=22` four times, with and without `-fidiag`; `-fpadvdiag` placed it after the frame's last advance (§1.7) |
| `fp_ng5.log` | the final build | the same route, 0 criticals, 29 pawns on `l_tower_p` |
| `fp_m9.log`, `shots/fpm9_*.png` | the final build | the clicked route again, the five acceptance captures, 0 criticals |

`grep -c FocusLost` is 1 on every run of this package and it is the `execOnFocusLost` stub line at quit, not a
focus event — the scripted window never has focus. The coordinator's warning about `ReceivedFocus` did not bite:
nothing in `RefreshGlobalUIState` depends on a focus event; it runs off `Start` and `Close`.

The global letterbox: `Global: SetBlackStripes(1) ok` the moment the menu opens (its `m_bDrawBlackStripesWhileActive`
is 1), and `SetBlackStripes(0) NOT FOUND` once, on the first frame after the map change, when the reopened
`UI_Global` movie has not yet instantiated `_root.blackStripes_mc`; the latch then matches the fresh movie's
authored state and the next change invokes normally. The triple-screen geometry and the two save-icon timers of
the same 1241 bytes are not ported and say so at the site.

## 4. The gate

`run_regression.py` from the worktree's own copy, absolute build dir
`D:/RecompileDishonored/Recompile/build/agentFP_wt/build/fpclean`, **the directory deleted first and all three
targets built inside the harness** (`DISHONORED_LAYOUT_CHECKS` on, its cmake default). `build/agentFP/fp_regression2.log`,
2689 s:

```
46 ok, 1 failed, 0 skipped
  REGRESSION inputtest/inputtest_criticals: 1 (wanted == 0)
```

**The one failure is `inputtest_criticals`, package FN's known spawn crash** (`DisSeqAct_StartSpawn_0` off
`DisTrigger_9`, PHASE16 §FN); it is the row PHASE16 says to expect until FN lands. Everything else:
`build_{DishonoredGame,CoreSmoke,LayoutProbe}_errors` 0, **0 C4263 and 0 C4264 in all three build logs**,
`coresmoke 99 / 0`, `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`,
`d3d9_frames 8910`, `texture_census 16323`, `inputtest_moved 1030.2`, `newgame_menu_open_seconds 21.9`,
`newgame_on_mission 1`, `newgame_pawns 29`, `newgame_criticals 0`, `options_steps 6`, `options_alive 233.2`,
`options_as2_errors 39`, `options_criticals 0`.

The three calibrated stages still pass `-nodishud` and read what they read before: `Init` leaves the global
movie and the HUD closed under it (and `PostCommitMapChange` too), so no calibrated threshold moved. The
`newgame` and `options` stages run the shipped command line, where `-gfxuimenu` is now inert and the menu
comes from Kismet; both pass, `options_as2_errors` at 39 against a bound of 41.

The first full run of this package read **44/3**: the two extra rows were the §1.7 crash, found by the gate and
fixed before this run. `build/agentFP/fp_regression.log` is that run.

## 5. Addresses, by hand, both databases

Every one is a function start with its size, read out of private copies (`scratchpad/idb/fp2013.i64`,
`fp2012.i64`); 2013 numbers are rvas (VA − `0x400000`), 2012's are rvas with image base 0.

| function | 2013 rva (size) | 2012 rva (size) | how the 2013 one is pinned |
|---|---|---|---|
| `UDisGlobalUIManager::RefreshGlobalUIState` | `0x847070` (901) | `0x8b75e0` (776) | the named `OnMovieStackChanged` thunk jumps to it |
| `OnMovieStackChanged` | `0x84cf70` (5), named | `0x8bc570` (5) | — |
| `OnMovieAttributesChanged` | `0x84cf80` (24), named | `0x8bc580` (24) | — |
| `Init` | `0x85fdb0` (1428) | `0x8c45d0` (511) | FName("pHUD") etc.; the one caller is InitGlobalManagers `0x5e9c38` |
| `FindCurrentMovieSet` | `0x851970` (545) | `0x8c0020` (507) | the one caller is Init `0xc5fe03`; body §1.5 |
| `CreateMoviePlayer` (Init's helper) | `0x846e70` (508) | — (inlined in 2012's Init) | `L"DishonoredGame.%s"`, `L"Twk_%s"`, `StaticLoadClass` under `UDisGFxMoviePlayerBase` |
| `LoadSFTweaksPackage` | `0x846d00` (355) | — | `_SF`, `GPackageFileCache`, `StaticFindObject(UPackage)` into +688 |
| `InitItemIconPackages` | `0x85dbb0` (352) | — | `StaticFindObjectChecked(UPackage, m_ItemSmallIconPackageName)` into +692 |
| `PreCommitMapChange` | `0x841120` (301) | `0x8aed60` (88) | called from `0x605c50` at `0xa05d11`; calls CloseHUD `0x787e30` |
| `PostCommitMapChange` | `0x83daa0` (29) | `0x8aedc0` (28) | called from `0x607370` at `0xa0744b`; `Start` vcall +296 then StartHUD `0x78cd00` |
| `ADishonoredGameInfo::PreCommitMapChange` | `0x605c50` (689) | `0x661260` (489) | `Term_GlobalMusic`, `ClearAllRelationOverrides` (both named) |
| `ADishonoredGameInfo::PostCommitMapChange` | `0x607370` (961) | `0x65f400` (248) | the second caller of the named `Init_GlobalMusic` |
| `ADishonoredGameInfo::InitGlobalManagers` | `0x5e9bd0` (148) | `0x62f8b0` (148) | — |
| `UDisGFxMoviePlayerBase::Start` | `0x7a3f40` (139) | `0x7f4200` (132) | calls the named OnMovieStackChanged |
| `UDisGFxMoviePlayerBase::Close` | `0x793a90` (188) | `0x7fa080` (188) | calls the named OnMovieStackChanged; ratio 0.830 |
| `UDisGFxMoviePlayerBase::LoadMoviePackage` | `0x7a3fd0` (666) | `0x81d0c0` (594) | called from CreateMoviePlayer `0xc4703d` |
| `HasFinishedAsyncLoading` | `0x787720` (15), vtable +436 | `0x7f42b0` (15), vtable +432 | `(this[95] & 0x100) == 0`; 2013 gained one slot below it |
| `UDisGFxMoviePlayerBase::PreLoad` | — | `0x822820` (315) | the prologue ported into the PreLoad seam |
| `UDisGFxMoviePlayerGlobal::PreAdvance` | `0x7aa610` (1241) | — | `_root.blackStripes_mc`, one-argument `SetBlackStripes`; FM's pin stands |
| `UDisGFxMoviePlayerMainMenu::PostStart` / `PreClose` | — | `0x821e00` (1141) / `0x7f7030` (233) | `m_pMainMenu = this` / `= 0` |
| `UGFxAction_OpenMovie::Activated` | `0x59d3a0` (1147) | `0x5e51a0` (1166) | calls the named `AddCaptureKey` `0x59ad80`; §1.2 |
| `UGFxAction_CloseMovie::Activated` | `0x5874f0` (296), named | `0x5e5630` (296) | — |
| `FGFxEngine::RenderUI` | `0x58dd30` (845), named | `0x5ceb00` (845) | §1.1 |
| `ADishonoredPlayerController::EnableControllerInput` | `0x6a0b00` (39), named | `0x6d1340` (39) | `m_InputEnableMask[Level]`, offset 1644 |

Not provable and said so: which of the two unnamed game-info classes 2013's Init tests (`0x1072040`,
`0x10721f8` static-class globals) is DLC06 and which DLC07. Both tests only pick the tweaks slot for a hit in
the SF tweaks package, which only the DLC06 and DLC07 movie sets name; the campaign never reaches it.
`Init` carries the medium-confidence note.

## 6. What this package did not do, on purpose

- **The mouse cursor stays drawn in the mission** (`fpm9_mission.png`, left of centre). `RefreshGlobalUIState`
  computes `m_pGlobal->m_bMouseCursorAllowed` (0 on the mission, 1 on the menu) exactly as retail does, but the
  GFxUI cursor glue (`DishonoredGFxUpdateMouseCursor`, agent EH's) draws the clip whenever a `Global` movie is
  open and cannot see the DishonoredGame bit; retail's `ConditionalShowMouseCursor` / `ConditionalHideMouseCursor`
  (2013 `0x78c790` / `0x7945b0`) are the consumers. Pre-existing, visible, one small seam away; named here rather
  than reached for at the end of a long package.

- `LoadTexturePackageAsync` / the texture-package cache (`m_TexturePackages`), so the DLC item-icon suffix is left
  empty with a warning; the campaign's answer anyway.
- `UDisGFxMoviePlayerGlobal::ConditionalHideMouseCursor` (in retail's PreCommitMapChange) and the power wheel's
  close; neither body exists in this tree.
- The main menu's PreCommitMapChange is `Close(TRUE)`, which runs through the engine's Close and the new Closed
  seam; `UDisGFxMoviePlayerMainMenu::PreClose`'s other work (texture package release, online delegates) is not here.
- `-gfxuimenu=<movie>` / `-gfxuimenuclass=` / `-gfxuinoglobal` are accepted and do nothing; the play-defaults table
  in `Core/Src/UnMisc.cpp` still appends `-gfxuimenu` and that token is now inert. Removing it from the table is
  the coordinator's call (the newgame stage's log echo would change).
- `STATUS.md`'s "Where we are" and open-defect 3 still describe `DisHUDBringUp` and the latched guard; both are
  gone. Coordinator's file.

## 7. Files, and every function touched

`build/agentFP_sync.py`'s docstring is the per-function list. Shared files touched: `dishonoredplayercontroller.cpp`
(one function appended at the end), `DishonoredGameNativeStubs.cpp` (four stubs deleted), `EngineClasses.h` (two
empty virtuals on `AGameInfo`), `UnGame.cpp` (two calls in `CommitMapChange`), `GFxUIUISequenceClasses.h` (two
`#include "CppText/..."` lines where the generator emits them). No reflected member moved; no header regenerated.
