# Agent EZ — the two functional bugs the user hit (2026-09-30)

| | |
|---|---|
| Package | `PHASE12.md` EZ: 1. New Game does not start the mission from the shipped executable. 2. Options, back, Options gives an empty screen. 3. the exit teardown fault, only with room left |
| Base | started at `e2d7e5d`, **rebased onto `8a04f5f`** (agent EX) mid-package at the coordinator's instruction; every measurement quoted below is on `8a04f5f` |
| Worktree | `build/agentEZ_wt` |
| Files | **four** source files (one Core, three `External/GFx3`) plus these two documents. No new unit, no `Sources.cmake` change, no generator run |
| Both bugs | **fixed and measured by real input on the staged shipped executable**, before and after, same driver, same key schedule |
| Item 3 | **not taken** — §7. Its premise is wrong and I say how |

## 1. Result

| Accept | State |
|---|---|
| New Game starts the mission from the staged shipped executable, by real input, at more than one difficulty, with the log lines and a screenshot | **done**, §3. Difficulty 1 and difficulty 3 (VERY HARD, the user's case) |
| Options -> back -> Options shows the Options screen, with the before measurement of the empty one | **done**, §4 |
| a before/after against the untouched HEAD executable through the same driver | §3.3 and §4.3 — HEAD is `8a04f5f`, extracted with `git archive` and built into its own directory |
| `run_regression.py` 37 ok, 0 failed, 0 skipped, worktree copy, absolute build dir, built in the harness | §6 |
| a clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | §6 |
| `rva_sweep.py` plus a by-hand resolution of every cited address | §6 |
| report + `agentEZ_status.csv` | this file; 15 rows |

## 2. The brief was wrong about both causes, and neither was where it pointed

**Bug 1 was not the difficulty, not the console-event dispatch and not a `USequenceCondition`.**
It was a bringup switch: `DISHONORED_PLAY_DEFAULTS` appended `-startmap=Dishonored_MainMenu
-startmapopen`, which throws away the world that owns the console event. §3.1.

The brief's first candidate — *"the difficulty differs, 3 in the user's run, 1 in EL's"* — is dead on
the first measurement. My reproduction of the fault on the untouched HEAD executable logs
**difficulty 1** and fails exactly as the user's difficulty-3 run did
(`build/agentEZ/rb3_log.txt:2502`, and `b1_log.txt:2322` on the pre-rebase base). `OnNewGameConfirm`
does one thing with the difficulty — `ADishonoredGameInfo::m_Difficulty = (BYTE)_Difficulty` — and
nothing on the path reads it.

The brief's third candidate — an unported `USequenceCondition` silently ending the chain — is a real
shape and agent EL's finding stands, but it is not this. The chain never starts: the console event
has no listener at all.

**Why agent EL's run completed and the user's did not** is the whole answer, and it is visible in the
two command lines rather than in the two builds. EL drove the game with **`-startmap=`** (empty), and
`appStrfind` then finds the `startmap` token already present and appends no default
(`build/agentEL/tower_log.txt:7`). The user launched the shipped executable with no such switch, so
the default applied. Same code, different world.

**Bug 2 was not a movie removed on close and not recreated, and not a state flag left set** — it was
`removeMovieClip` succeeding where Flash and GFx both refuse it. §4.1.

**And the premise I brought myself was wrong too.** I read the AS2 fault as a stale
`GFxCharacterHandle`, implemented retail's missing `ResolveCharacter` re-resolution, measured it, and
it did **not** fix anything: the clip is not merely unreachable, it is gone from the display list, so
there is nothing to re-resolve to. That work is reverted and handed over as §8.2 rather than
reported as the fix.

## 3. Bug 1 — New Game starts the mission from the shipped executable

### 3.1 The cause, measured

`UnMisc.cpp`'s play-defaults table carried

```c
{ TEXT("startmap"), TEXT(" -startmap=Dishonored_MainMenu -startmapopen") },
```

`DishonoredTickStartMap` (`UnGame.cpp`) reads `-startmapopen` and runs `UGameEngine::Exec("OPEN
Dishonored_MainMenu")` as soon as the tick loop runs. `OPEN` is `LoadMap`, so the world
`DishonoredGameFull_P` — loaded three seconds earlier — is **replaced**, not streamed under. The menu
still draws, because `-gfxuimenu` opens `UI_MainMenu.MainMenu` over whatever world is up. What goes
with the old world is `DishonoredGameFull_P.TheWorld:PersistentLevel.Main_Sequence`, and that
sequence owns every one of the hub's `SeqEvent_Console` nodes.

The run's own Kismet inventory, one line apart from the `OPEN`, prints what is thrown away
(`build/agentEZ/b1_log.txt:489`):

```
kismet inventory: classes present: SeqAct_ActivateRemoteEvent x235, ... SeqAct_CommitMapChange x82,
  DisSeqAct_SetStoryFlag x74, DisSeqAct_SetPlayerTravelDestination x64, SeqEvent_Console x53, ...
[0004.00] startmap: 'OPEN Dishonored_MainMenu' after the None commit
[0004.04] LoadMap: Dishonored_MainMenu?Name=Corvo?Team=255
```

and 40 s later, with `-diskismet` on, the console command lands in a world that has none of them:

```
[0043.88] OnNewGameConfirm(difficulty 1): 'ce ChangeLvl_StartNewGame'
[0043.88] kismet census: script FindSeqObjectsByClass(SeqEvent_Console, recursive 1)
          on Dishonored_MainMenu.TheWorld:PersistentLevel.Main_Sequence -> 0 objects
[0043.88] kismet census: script FindSeqObjectsByClass(SeqEvent_Console, recursive 1)
          on Dishonored_MainMenu.TheWorld:PersistentLevel.Main_Sequence -> 0 objects
```

`build/agentEZ/b1_log.txt:2322-2324`. **Zero objects.** That is the whole of "the command is issued
and nothing follows": `PlayerController.CauseEvent` looks for the event, finds nothing, and returns.

The fix is to delete the row. Nothing replaces it, because `DishonoredGameFull_P`'s own Kismet
commits the map change into `Dishonored_MainMenu` by itself — which is retail's shape, and which
leaves the hub's sequence alive as the persistent level under a `ULevelStreamingPersistent`. Measured
on the fixed build: `Committed map change via DishonoredEngine` at 4.65 s, `-gfxuimenu: opened
UI_MainMenu.MainMenu` right after, so the menu is not slower.

`-startmap=<map>` and `-startmap=<map> -startmapopen` still work when given on the command line, and
`run_regression.py` passes both explicitly on three of its stages, so the harness is untouched (and
is not built with the play defaults anyway).

### 3.2 What the keys now do

Driven with the shared `resources/tools/drive_input.py` through `build/agentEZ/go.py`, `--exe
DishonoredGame_EZ.exe`, keyboard only, schedule keyed to `-gfxuimenu: opened UI_MainMenu`:

| difficulty | keys | run |
|---|---|---|
| 1 (NORMAL, the default) | SPACE, ENTER, ENTER, ENTER, ENTER | `r3b` |
| 3 (VERY HARD, the user's) | SPACE, ENTER, **DOWN, DOWN**, ENTER, ENTER, ENTER | `r2` |

```
[0072.46] OnNewGameConfirm(difficulty 1): 'ce ChangeLvl_StartNewGame'
[0072.48] SetPlayerTravelDestination: destination 'PlayerStart_NewGameEmpress' from level 'Dishonored_MainMenu'
[0072.89] Committed map change via DishonoredEngine              <- 0.43 s after the key
[0077.52] GotoPlayerTravelDestination: 'l_tower_p' -> PlayerStart_3 at X=-3900.598 Y=36639.262 Z=-215.850 (moved 1)
```

`build/agentEZ/r3b_log.txt`, and the same four lines with `difficulty 3` in `r2_log.txt:1593`.
0 `Critical error` in either run. The frames are
**`build/agentEZ/ez_r3b_t12000002.png`** (difficulty 1) and **`ez_r2_t14000004.png`** (difficulty 3):
the boat landing at Dunwall Tower, mean brightness 80.8 and 80.7, 94.8 % of pixels above black.

### 3.3 Before and after, same driver, same keys

| | HEAD `8a04f5f` (`DishonoredGame_EZB.exe`) | this package (`DishonoredGame_EZ.exe`) |
|---|---|---|
| the world after four seconds | `OPEN Dishonored_MainMenu` -> `LoadMap: Dishonored_MainMenu` | `DishonoredGameFull_P`, its own Kismet commits the menu map change |
| `OnNewGameConfirm` -> `ce ChangeLvl_StartNewGame` | yes, difficulty 1 (`rb3`) and difficulty 3 (`rb2`) | yes, both |
| `SeqEvent_Console` found by the `ce` | **0 objects** | the hub's 53 are alive |
| `SetPlayerTravelDestination` | **no line at all**, either difficulty | `destination 'PlayerStart_NewGameEmpress'` |
| `Committed map change` after the key | **never** (0 in the whole run) | 0.43 s (d1) / 0.96 s (d3) later |
| `GotoPlayerTravelDestination` | **never** | `-> PlayerStart_3 ... (moved 1)` |
| the frame 60 s after the key | the menu's own camera over Dunwall, mean 89.2 (`ez_rb2_t14000004.png`) — *"you are just there forever"* | the Tower landing, mean 80.7 (`ez_r2_t14000004.png`) |
| the AS2 error 200 ms later | `removeMovieClip ... on undefined`, the user's log line | **gone** (§4) |

The HEAD executable is a `git archive 8a04f5f` of the repository into `build/agentEZ_head`, built
into `build/agentEZ_headrel2`, so the main checkout was never read as a build source. Each run staged
its own executable from its own build directory; `--no-stage` was used only to re-run against an
executable staged moments earlier in the same session, never across a rebuild.

## 4. Bug 2 — Options, back, Options

### 4.1 The cause, measured

`MCRemoveMovieClip` (`GFxAS2Lib.cpp`) was

```c
GFxSprite* s = ThisSprite(fn);
if (s == 0 || s->GetParent() == 0) return;
GFxSprite* parent = s->GetParent()->ToSprite();
if (parent) parent->RemoveDisplayObject(s->GetDepth(), s->GetId());
```

with no test of **what created the clip**. Flash and GFx both refuse `removeMovieClip` on a clip the
timeline placed; only `attachMovie` / `createEmptyMovieClip` / `duplicateMovieClip` clips can be
removed. The Dishonored menu's teardown calls it on `_root.optionsMenu_mc`, which is one of the root
timeline's own children of `UI_MainMenu.MainMenu`, and we obeyed.

The measurement is the root's display list at the moment the second entry fails, printed by a
temporary probe (`build/agentEZ/o12_log.txt:2308`):

```
EZPROBE resolve 'optionsMenu_mc' FAILED under _level0, 10 entries
  entry 0 depth 1  name 'mainMenu_mc'        entry 5 depth 95 name 'videoSettings_mc'
  entry 1 depth 50 name 'newGame_mc'         entry 6 depth 96 name 'gamepadMapping_mc'
  entry 2 depth 68 name 'startScreen_mc'     entry 7 depth 97 name 'gammaSetting_mc'
  entry 3 depth 72 name 'missions_mc'        entry 8 depth 98 name 'help'
  entry 4 depth 93 name 'loadGame_mc'        entry 9 depth 99 name 'vignette_mc'
```

Every other screen is still there. `optionsMenu_mc` is not, and nothing puts it back.

The user-visible failure two steps later is the root calling a method on the reference it still
holds. `-gfxuiwatch=SetMenu` names it exactly (`build/agentEZ/o10_log.txt`):

```
[0026.62] watch CALL 'SetMenu' receiver type 7, interface 25EDF1E8 (pc 7752 of 8251)   <- first entry, works
[0056.65] watch CALL 'SetMenu' receiver type 7, interface 00000000 (pc 7752 of 8251)   <- second entry
[0056.65] AS2 error: call of a value that is not a function: 'SetMenu' ... on undefined
```

Type 7 is `GASValue::CHARACTER`; the handle is the same, its `pCharacter` is now null. The AS2 then
transitions the root to `OptionsScreen` with no options screen in the movie, which is the empty
screen the user saw.

The fix is the missing test:

```c
if (s == 0 || s->GetParent() == 0 || !s->bScriptCreated) return;
```

with `GFxCharacter::bScriptCreated` set by `GFxSprite::AttachMovie` and
`GFxSprite::CreateEmptyMovieClip`.

**Deviation, stated once.** Retail does not carry a flag: it tells the two apart by depth, because
`attachMovie` places a clip at its AS depth **+ 16384**, above every timeline depth, and `_depth` /
`getDepth()` subtract it again. This tree does not have that convention — `MCAttachMovie` passes the
AS depth through unchanged, `MCGetDepth` returns the raw internal depth, and `GASop_DuplicateClip`
*subtracts* 16384 instead of adding it — so `GetDepth() >= 16384` is not usable here and would have
made `removeMovieClip` a no-op for every clip. Repairing the depth convention moves script-created
clips above timeline ones in z-order, which is a rendering change in agent EX's area; it is §8.1.

### 4.2 What the keys now do

Keyboard only, so no click coordinate is involved (§5): SPACE, RIGHT, RIGHT, ENTER (Options), ESC
(back), ENTER (Options again).

```
[0071.93] AS2 trace: > APressed () called
[0072.09] AS2 trace: >> TransitionTo (OptionsScreen)     <- first entry
[0089.90] AS2 trace: > BPressed () called                <- ESC, back to the main menu
[0105.90] AS2 trace: > APressed () called
[0106.06] AS2 trace: >> TransitionTo (OptionsScreen)     <- second entry, no error at all
```

`build/agentEZ/r1_log.txt`, 0 `Critical error`. The frame is
**`build/agentEZ/ez_r1_t07000006.png`**: OPTIONS, the four category tabs, GAMEPLAY / USER INTERFACE
and the six rows — the same screen the first entry draws (`ez_o14_t04000003.png` is the first entry
of an earlier run of the same schedule, for comparison).

### 4.3 Before and after, same driver, same keys

| | HEAD `8a04f5f` (`rb1`) | this package (`r1`) |
|---|---|---|
| first Options entry | `TransitionTo (OptionsScreen)`, screen draws | the same |
| ESC leaves Options | yes, main menu back with OPTIONS highlighted | the same |
| second Options entry | `AS2 error: 'SetMenu' ... on undefined`, then `TransitionTo (OptionsScreen)` | **no error** |
| the frame after it | **empty**: the 3D scene, no menu of any kind (`ez_rb1_t07000006.png`) | the Options screen (`ez_r1_t07000006.png`) |

## 5. One driver correction that cost two runs, and is worth recording

The coordinator's warning is right and I hit it twice. `build/agentEZ/a2_drive.txt` and
`f3_drive.txt` both show a `hoverclick` step with **`foreground 0x3b05c4`** and **`0x70a64`** — a
window that is not the game's. In `a2` the click landed anyway and the difficulty did change to 3; in
`f3` it did not and the run produced no `OnNewGameConfirm` at all. Neither of those runs is quoted as
evidence.

Everything in §3 and §4 is driven by **keys only**, which the drive logs show delivered to the game's
own window handle on every step. The difficulty screen turned out to be reachable that way after all:
**DOWN moves the difficulty selection**, so SPACE / ENTER / DOWN / DOWN / ENTER / ENTER / ENTER
reaches VERY HARD with no pointer at all. Agent EX's DPI-awareness change therefore cannot have moved
anything under these measurements, and they were in any case all re-taken on `8a04f5f` after the
rebase.

A second driver note: with four agents building and driving at once, the schedule bunches. One run
(`o13`) delivered its SPACE at +12.3 s and its two RIGHTs 0.2 s apart at +16.3 and +16.5, so the menu
was never where the schedule assumed; it is struck and re-run as `o14`. Read the drive log first,
every time.

## 6. Verification

* **Regression**, the worktree's own copy, an absolute build dir, **built inside the harness** so the
  six build-stage checks count: `python resources\tools\run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEZ_wt/build/agentEZ_wtreg --exe-name
  DishonoredGame_EZR.exe --log-prefix EZR` -> **`37 ok, 0 failed, 0 skipped, 1654s`** (`build/agentEZ/regression1.log`).
  The five gitignored `resources/docs/types` inputs were copied into the worktree first, twice — once
  at the start and again after the rebase checkout — so the layout stage measures rather than
  recording `-1`.
* **Clean full release build** of the worktree, build directory deleted first, every target, with
  `DISHONORED_LAYOUT_CHECKS=ON`: **1017 edges, 0 errors, 0 `C4263`, 0 `C4264`, 0 `FAILED:`**, all four targets linked (`DishonoredGame`, `CoreSmoke`, `EdgeAnimSmoke`, `LayoutProbe`) (`build/agentEZ/buildclean.log`).
* **`rva_sweep.py`**, the worktree's own copy, over the whole tree: **7302 citations, 4218 `ok-2013`,
  3059 `ok-2013-mid`, 22 `ok-2012-labelled`, 2 `MISLABELLED-2012`, 1 `UNKNOWN-CLAIMED-2013`**
  (`build/agentEZ/sweep_all.csv`). **All three suspects are pre-existing and in files this package
  does not touch** — `GFxUI/Src/gfxuirenderer.cpp:1679` (two 2012 style addresses quoted as 2013, in
  agent EX's area) and `DishonoredGame/Src/disbehaviorpatrol.cpp:319`. Filtered to my four files:
  **89 citations, 42 `ok-2013`, 45 `ok-2013-mid`, 2 `ok-2012-labelled`, 0 mislabelled, 0 fabricated**,
  and **this package adds no address citation of its own** — `git diff | grep 0x` is empty.
* **Addresses resolved by hand.** `build/agentEZ/isfunc.py` through `resources/tools/ida/run.py` on
  **my own copy** `build/agentEZ_ida/retail2013_agentEZ.i64`. No IDA MCP tool and no FModel tool was
  used. All fourteen addresses this report and the status CSV name are function **starts**.
  **My own first attempt was wrong and is worth recording**: I resolved them without adding the image
  base and every one came back mid-function in an unrelated `UUIDataProvider` — `0x7bcad0` "inside
  `UPrimitiveComponent::SetPhysMaterialOverride`", and so on. The database holds **VAs**; a tree rva
  needs `+ 0x400000`. With the base added, `0x7bcad0` is a 188-byte function start, `0x7bc9d0` is
  `UDisGFxMoviePlayerMenuBase::CloseOptions`, and agent EO's six addresses are all correct.
* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is NOT required and
  was NOT run.** This package adds no class, no native and no unit. **`Sources.cmake` is unchanged**
  (0 units in or out of `DishonoredGame_EXCLUDE`).
* **Files outside `DishonoredGame`: all four.** `Core/Src/UnMisc.cpp` and three in
  `External/GFx3`. **Total 6** with the two documents.
* No commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`, and the main
  checkout and the other agents' worktrees were never written to.

## 7. Item 3, the exit teardown fault: not taken, and its premise is wrong

I did not take it. I say so plainly, and I correct the brief on the way past, because the correction
matters more than the item.

The brief and `STATUS.md` both describe it as *"a critical error **after** `Exit: Game engine shut
down` and `Exit: Windows client shut down`, with thirteen unsymbolised frames"*. **The user's log has
no `Exit:` line at all.** What it has, at `Launch720b.log:6091`, is

```
[0217.73] Critical: appError called: Ran out of virtual memory. To prevent this condition, you must
          free up more space on your primary hard disk.
[0217.73] Critical: Windows GetLastError:  (2)
```

**while the game is still rendering the menu** — the frame before it is `scene rendered (46890 so
far, d3d9, 1 views, 1280x720)` at 217.67. Nothing was shutting down. This is a 32-bit process running
out of address space after three and a half minutes sitting on the main menu (`Init: Memory total:
... Virtual=2.0GB`), which is also why the user, stuck at a menu whose New Game did nothing, ended up
with a crash: they waited.

The "thirteen unsymbolised frames" are not frames. Decoded as UTF-16 they are the error message
itself — `0x200074 0x680074 0x730069` is `" t" "ht" "si"` — so the walker was handed a buffer, not a
stack. Any investigation that starts from those addresses is chasing text.

That is a package of its own and I did not have room for it after 1 and 2. What it should start from:
what allocates per frame at the menu, not what happens at `appExit`.

## 8. Hand-overs

1. **The GFx depth convention, for agent EX.** Retail places a script-created clip at its AS depth
   **+ 16384** and subtracts it again in `_depth` / `getDepth()`. Here `MCAttachMovie` applies no
   offset, `MCGetDepth` returns the raw depth, and `GASop_DuplicateClip` (`GFxAS2Interp.cpp`)
   **subtracts** 16384 where it should add it. Two consequences: `removeMovieClip` cannot use
   retail's own test (which is why §4.1 records a flag instead), and a clip `attachMovie`'d at AS
   depth 5 currently draws *below* a timeline clip at depth 50 where Flash would draw it above.
   Repairing it lets `bScriptCreated` be deleted.
2. **`GFxCharacterHandle::ResolveCharacter` is a stub, for agent EX.** `GFxPlayer.h`:
   `{ (void)root; return pCharacter; }`, four lines under a comment that says the indirection exists
   so a `GFxValue` naming a removed clip "degrades to undefined instead of dangling" — which is only
   half of it. Retail re-resolves the name through the parent's display list, so a clip the timeline
   rebuilds is found again. I implemented it (a refcounted parent handle, so re-resolution never
   reads a destroyed `pParent`, then `GFxDisplayList::GetCharacterByName`), measured it, and it fixed
   nothing here because the clip was genuinely gone; it is reverted. It is still a real gap and the
   next clip the timeline rebuilds will need it.
3. **`MovieClip.setMask` is not implemented, for agent EX.** The AS2 `MovieClip` prototype
   (`GFxAS2Lib.cpp`) registers thirteen methods and `setMask` is not among them. Every entry into
   Options raises `call of a value that is not a function: 'setMask' ... on Sprite
   _level0.optionsMenu_mc._options_mc._list_mc` — a real sprite asking for a real builtin, twice per
   entry.
4. **The gamepad glyph clips do not exist on PC, for agents EX and EY.** `_LShoulder_mc`,
   `_RShoulder_mc`, `_LTrigger_mc`, `_RTrigger_mc` and their `_glow_mc` children resolve to undefined,
   which is where the `tweenTo` / `tweenEnd` / `gotoAndStop` / `gotoAndPlay` "on undefined" errors on
   the Options screen come from, and it is the same fault as the `GotoLabeledFrame: no frame named
   'PC'` warnings. Cosmetic, but it is eleven AS2 errors per Options entry and it hides real ones.
5. **Two files in the MAIN checkout are CRLF on disk**, `External/GFx3/GFxAS2Lib.cpp` (1228 CRLF, 0
   LF) and `External/GFx3/GFxPlayerSprite.cpp` (1594 CRLF), although `.gitattributes` says
   `* text=auto eol=lf` and both blobs are LF. Git normalises on read so `git status` is clean and
   nothing shows it. It is invisible until a tool byte-compares — `build/agentEZ_sync.py` had to
   normalise line endings before it could tell "changed" from "checked out differently", and a sync
   script that does not will flag or overwrite files at random. The worktree's copies are LF.
6. **The exit fault is an address-space leak at the menu**, §7, not a teardown fault.
7. **`-startmap=<map>` on its own (without `-startmapopen`) is now the only supported way to jump
   straight to a map**, and it goes through `STREAMMAP` after the menu's commit, which is the same
   engine entry the Kismet action uses. Anything that wanted the old default behaviour should pass
   `-startmap=Dishonored_MainMenu -startmapopen` explicitly; `run_regression.py` already does.

## 8a. Merging, and what moved under me while I was verifying

The coordinator moved me from `e2d7e5d` to **`8a04f5f`** (agent EX) mid-package and I re-took every
measurement there; §3 and §4 are all on `8a04f5f`. **While the regression and the clean build were
running, main advanced again to `a677a21`** (agent FA, the menu in three dimensions), and FA's commit
touches **all three** of the GFx3 files this package touches.

* `build/agentEZ/ez.patch` is the four-file change against **`8a04f5f`** — the form that was measured,
  regression-gated and clean-built.
* `build/agentEZ/ez_on_a677a21.patch` is the same change **rebased onto `a677a21`**, with the one
  conflict resolved. Three of the four files rebase with no conflict (`GFxPlayer.h` needs a two-line
  fuzz because FA adds members next to mine). The conflict is in
  `GFxPlayerSprite.cpp`, `GFxCharacter::GFxCharacter`, and it is textual, not semantic: FA adds
  `pMatrix3D(0), pPerspective3D(0), pView3D(0), PerspectiveFOV(0.f)` to the constructor initialiser
  list and I add `bScriptCreated(false)` to the same list. The resolution is the one line

  ```c
  RollOverCnt(0), bAcceptAnimMoves(true), bScriptCreated(false), pWeakProxy(0), pMatrix3D(0), pPerspective3D(0),
  ```

* **`a677a21` + this change builds and both fixes still hold on it.** The build is
  `build/agentEZ/build_merge.log` (963 edges, 0 errors, 0 `FAILED:`), and it was driven with the same
  two keyboard schedules as §3 and §4:
  **`m1`** — second Options entry at 70.23 s, no `SetMenu` error, `build/agentEZ/ez_m1_t07000006.png`
  is the Options screen; **`m2`** — `OnNewGameConfirm(difficulty 3)` at 83.37 ->
  `SetPlayerTravelDestination` -> `Committed map change` at 85.00 -> `GotoPlayerTravelDestination` at
  91.13, `ez_m2_t14000003.png` is the Tower landing, mean 80.8. 0 `Critical error` in either.
  What `a677a21` has **not** had is the 37-check regression and the clean layout build: those ran on
  `8a04f5f`. If the gate is wanted on `a677a21` it has to be re-run there, and I say so rather than
  implying the numbers carry over.

**`Binaries\Win32\DishonoredGame-Win64-Shipping.exe` in the retail tree — the executable the user
actually launches — was deliberately NOT overwritten.** It is shared, and it is the coordinator's to
stage. It still carries the build the user hit both bugs on: once this merges, it has to be re-staged
from the merged build before the user tries again, or they will see the same two faults.

`gen_classes_header.py` is not required and was not run. `Sources.cmake` is unchanged. Every file
outside `DishonoredGame` is listed in §9 — all four of them, because this package has nothing inside
`DishonoredGame` at all. **Total 6** files with the two documents. Nothing is committed.

## 9. Files

Mine, four source files and two documents:

* `source/Development/Src/Core/Src/UnMisc.cpp` — the play-defaults row removed, with why
* `source/Development/Src/External/GFx3/GFxAS2Lib.cpp` — `MCRemoveMovieClip`'s missing test
* `source/Development/Src/External/GFx3/GFxPlayer.h` — `GFxCharacter::bScriptCreated`
* `source/Development/Src/External/GFx3/GFxPlayerSprite.cpp` — set it in the two creators, init it
* `resources/docs/agents/agentEZ.md`, `agentEZ_status.csv`

**Agent EX owns `External/GFx3` and `GFxUI` for wave 16.** The three GFx3 files do not overlap EX's
four (`GFxDisplay.cpp`, `GFxTextDocView.cpp`, `gfxuirenderer.cpp`, `Launch.cpp`) and the patch applied
to `8a04f5f` with no conflict, but `build/agentEZ_sync.py` **flags rather than copies** them: they are
EX's to take. `UnMisc.cpp` is the only file it will copy.

Scratch, not repo tools: `build/agentEZ/` — `go.py`, `isfunc.py`, the run logs and drive logs
(`b1`, `o1`..`o14`, `f1`..`f4`, `g1`, `g2`, `r1`..`r3b`, `rb1`..`rb3`, `hb1`, `m1`, `m2`, `p1`,
`p2`), the build logs, `sweep_all.csv`, `ez.patch`, `ez_on_a677a21.patch` and the screenshots; `build/agentEZ_build.cmd`,
`build/agentEZ_run.py`, `build/agentEZ_sync.py`; worktree `build/agentEZ_wt`; build directories
`build/agentEZ_wtrel` (the play build the runs are driven on), `build/agentEZ_wt/build/agentEZ_wtreg`
(the harness's own), `build/agentEZ_clean` (the clean gate) and `build/agentEZ_head` +
`build/agentEZ_headrel2` (the untouched `8a04f5f` tree and its build; `agentEZ_headrel` is the
pre-rebase `e2d7e5d` one) and `build/agentEZ_mergebase` + `agentEZ_mergecheck` + `agentEZ_mergerel`
(an `a677a21` extraction, the same with this change applied, and its build - the tree §8a measures). IDA: **own copy only**, `build/agentEZ_ida/retail2013_agentEZ.i64`.
