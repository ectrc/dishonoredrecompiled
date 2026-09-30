# Agent EL — Corvo in the Tower (2026-09-30)

Agent EI, handing this over:

> *"The player is never put into the level the map change loads. This is now the single thing between the
> front end and a picture of Corvo in the Tower, and it is one function."*

| | |
|---|---|
| Package | the path from YES on the New Game confirmation to the player standing in Dunwall Tower |
| Starts at | HEAD `0cbe3fe`, worktree `build/agentEL_wt` |
| Files | **20**, every one of them under `source/Development/Src/DishonoredGame` (7 new), plus these two documents |
| Regression | `37 ok, 0 failed, 0 skipped, 954s` on `build/agentEL_wt/build/agentEL_wtreg`, **built inside the harness** (§6) |
| Clean build | the worktree, build directory deleted first, every target, `DISHONORED_LAYOUT_CHECKS=ON`: **1002 edges, 0 errors, 0 C4263, 0 C4264** (§6) |

**`build/agentEL/el_tower_t08000003.png` is Corvo standing on the boat landing at Dunwall Tower**, reached
from the main menu by five real key presses and nothing else — no `-startmap`, no console command, no forced
teleport. The same five keys on the untouched HEAD executable through the same driver give a black frame
(`build/agentEL/el_head2_t08000002.png`, mean brightness **0.64**, 0.04 % of pixels above black, against
**80.7** and 92.4 % for this package).

## 1. Result

| Accept | State |
|---|---|
| a screenshot of Corvo in the Tower, reached by real input from the menu | **done.** §3 has the key schedule, the log and the two pictures |
| a before/after against the untouched HEAD executable through the same driver | §4 |
| `run_regression.py`, worktree copy, absolute build dir, built in the harness, 37 checks | §6 |
| a clean full release build, build dir deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | §6 |
| `rva_sweep.py` plus by-hand resolution of every cited address | §6 |
| report + `agentEL_status.csv` | this file; 22 rows |

## 2. The brief was wrong about the size of this, and the correction is the report

> *"it is one function: `UDisSeqAct_SetPlayerTravelDestination::Activated`, 2013 rva `0x78a0a0`"*

The address is right — §5.1 re-derives it — and the function is necessary. It is not sufficient. What
actually stood between the map change and the picture is **seven functions in five units, plus one existing
function in this tree whose body was wrong**, and none of that was guessable: it came out of the Kismet graph
of `L_Tower_Script` read at runtime.

Retail's arrival is a chain of Kismet ops, and every link of it had to be alive:

```
menu:  DishonoredGameFull_P.Main_Sequence.GoToTowerEmpress
         SeqEvent_Console 'ChangeLvl_StartNewGame'      <- OnNewGameConfirm's `ce`
           DisSeqAct_SetStoryFlag_0                     <- NOT ported: arms the Tower's gate     0x78fce0
           DisSeqAct_SetPlayerTravelDestination_1       <- NOT ported: records the destination   0x78a0a0
           SeqAct_PrepareMapChange_1 / SeqAct_CommitMapChange_1

tower: L_Tower_Script.Main_Sequence
         SeqEvent_LevelLoaded -> ... -> DisSeqCond_IsSentinel_4       <- NOT ported               0x78f070
           -'False'->            DisSeqCond_CheckStoryFlag_4          <- NOT ported               0x78e9d0
           -'True'->             SeqAct_Gate_1                        (engine, fine)
           ->                    DisSeqAct_ShowLocationDiscovery_0    (not ported, passes through)
           ->                    SeqAct_Gate_8                        (engine, fine)
           ->                    DisSeqAct_GotoPlayerTravelDestination_0  <- NOT ported           0x79a940
                                   -> DisTeleportPlayer                                           0x7bf830
```

Three things about that chain are worth stating plainly, because each cost a measurement:

1. **A `USequenceCondition` that is not ported stops everything downstream of it dead.** It does not
   auto-activate its output links (that is the point of a condition), so with no `Activated` body neither
   'True' nor 'False' ever carries an impulse. `DisSeqCond_IsSentinel_4` had `ActivateCount == 1` and both its
   outputs at `bHasImpulse == 0`: it *ran*, and it produced nothing. A plain `USequenceAction` behaves the
   opposite way — `DisSeqAct_ShowLocationDiscovery` is unported and the chain still passes through it, because
   `USequenceOp::DeActivated` fires the outputs — which is why the title card is the only thing missing from
   the arrival and the arrival still happens.
2. **The gate is a story flag, and a story flag is two halves.** A `UDisStoryFlagSet` asset declares a GUID;
   the *value* lives in `ADishonoredPlayerPawn::m_StoryFlagInstances`, which is `CPF_Transient`, so a new game
   starts with every flag false. The menu's `GoToTowerEmpress` sequence sets the flag that
   `L_Tower_Script`'s `DisSeqCond_CheckStoryFlag_4` reads. Neither side existed, so porting only the reader
   would have been useless.
3. **`DisGetCurrentLevel` was wrong in this tree, and the Goto action cannot work until it is right.** §5.2.

## 3. What five real key presses now do

`build/agentEL/tower_log.txt`, driven by **`resources/tools/drive_input.py`** (the shared driver, `--exe
DishonoredGame_EL.exe`), schedule keyed to `-gfxuimenu: opened UI_MainMenu`:

```
2:key:20,1.5    SPACE   dismiss the start screen
8:key:0d,0.3    ENTER   NEW GAME
16:key:0d,0.3   ENTER   difficulty
24:key:0d,0.3   ENTER   continue past the brightness screen -> the confirmation box
34:key:0d,0.3   ENTER   YES
```

`build/agentEL/tower_drive.txt` shows all five delivered to the one window, and the log:

```
[0056.95] message box 1 up: 'Creating a new game will overwrite previous autosaves. Continue?' [YES|NO|]
[0067.29] message box 1 answered with button 0
[0067.48] OnNewGameConfirm(difficulty 1): 'ce ChangeLvl_StartNewGame'
[0067.77] SetPlayerTravelDestination: destination 'PlayerStart_NewGameEmpress' from level 'Dishonored_MainMenu'
[0068.77] Committed map change via DishonoredEngine                      <- 1.29 s after the key
[0074.42] GotoPlayerTravelDestination: 'l_tower_p' -> PlayerStart_3 at X=-3900.598 Y=36639.262 Z=-215.850 (moved 1)
```

and 0 `Critical error` over the whole 160 s run. The pawn, which every earlier run left at the menu world's
spawn `X=0 Y=500 Z=-209`, is at `X=-3900.598 Y=36639.262 Z=-225.200` — `l_tower_p`'s
`PlayerStart_3`, whose `Tag` is the `PlayerStart_NewGameEmpress` the menu asked for.

**`build/agentEL/el_tower_t08000003.png`** (also `_t06500002`, `_t10000004`, `_t12000005`) is the frame: the
pier, the ferry, the steps up to the Tower, the Dunwall skyline.

## 4. Before and after, same driver, same five keys

| | HEAD `0cbe3fe` (`DishonoredGame_ELB.exe`) | this package (`DishonoredGame_EL.exe`) |
|---|---|---|
| the box is answered | yes, `answered with button 0` at 64.70 (agent EI's work is in HEAD) | yes, at 67.29 |
| `OnNewGameConfirm` -> `ce ChangeLvl_StartNewGame` | yes | yes |
| `SetPlayerTravelDestination` | **no line at all**: the class is registered but `Activated` is not overridden | `destination 'PlayerStart_NewGameEmpress' from level 'Dishonored_MainMenu'` |
| the map change commits | yes, 65.87 | yes, 68.77 |
| `GotoPlayerTravelDestination` | **never**: the story-flag gate upstream of it never opens | `-> PlayerStart_3 ... (moved 1)` |
| the frame 15 s later | **black**: mean brightness **0.64**, **0.04 %** of pixels above black (`build/agentEL/el_head2_t08000002.png`) | **the Tower**: mean **80.7**, **92.4 %** above black (`build/agentEL/el_tower_t08000003.png`) |
| the scene census at the end of the run | 2009 primitives processed, 64 occluded, 435 visible, 923/6506 draw lists | 2672 processed, 550 occluded, 260 visible, 671/6506 — a different camera, inside the level rather than under it |

The HEAD executable is built from a `git archive 0cbe3fe` of the repository into `build/agentEL_head`, so the
main checkout was never read as a build source and never touched.

Both runs staged their own executable from their own build directory (`--build-dir build/agentEL_headrel`
for the HEAD one), so `stage_retail.py` copied the right binary each time; the trap agent EI recorded is
avoided by never using `--no-stage` rather than by trusting a stale staged file.

**One run struck from this report**: `head` (the first HEAD attempt). Its drive log shows the `+8` step
firing at `+16.8 s` and the `+16` step at `+17.1 s`, 0.3 s apart, so the menu never reached the confirmation
before the last key; the box came up at 42.01 and was never answered. That is a driver slip under load, not a
property of HEAD, and `head2` — same schedule, steps at +2.6/+8.0/+16.0/+24.0/+34.0 — is the measurement §4
quotes. No security dialog appeared in any run.

## 5. The two corrections that mattered

### 5.1 `0x78a0a0` re-derived, and the matcher is wrong about the other one

Agent EI read `UDisSeqAct_SetPlayerTravelDestination::Activated` out of a vtable slot rather than a symbol and
asked for it to be re-derived. It is right, and here is the derivation that does not depend on EI's:

* `?InternalConstructor@UDisSeqAct_SetPlayerTravelDestination@@SAXPAX@Z` (2013 `0x7b82c0`) writes
  `0xd4ebe8` into the object at +18. That *is* the class's primary vtable, by construction.
* `0x78a0a0` appears **exactly once** in the whole of `.rdata`, at `0xd4ebe8 + 372`.
* `+372` is the `Activated` slot: `+376` is `USequenceOp::DeActivated` and `+368` is
  `UObject::IsRefSaveable`, retail's inserted save-layer virtual.
* Its body writes `this+248` (`m_Tag`, the class's only property) into `s_pInstance + 1700`
  (`m_PlayerTravelLocationName`) and the current level's package name into `+1708`
  (`m_PlayerTravelOriginLevelName`), then tail-calls `USequenceAction::Activated`. Nothing else in the binary
  can be that function.

A footnote on the brief: it describes the body as writing "two words onto `s_pInstance` (+1700 and +1704) and
the current level's package name". +1700 and +1704 are the two halves of the one `FName`
`m_PlayerTravelLocationName`; the package name is a second `FName` at +1708.

**`UDisSeqAct_GotoPlayerTravelDestination` is where the truth sources are wrong.**
`match_2012_2013.csv` maps the 2012 vtable `0xd420e0` to 2013 `0xd52cf0` ("global-table", 1.000) and the 2012
`InternalConstructor` `0x7e5a20` to `0x7b7fa0` ("neighbours", 1.000 — a ratio computed over 27 identical
bytes, so it carries no information), and `retail2013_named.i64` and `functions_2013.csv` carry both names.
Both are wrong:

* `0xd52cf0 + 372` is `0x78f600`, whose body loops over `Targets` and calls
  `ADishonoredPawn::SetMinimumScriptedHeatlh(this+248)` — that is `UDisSeqAct_LimitPawnMinHealth::Activated`,
  and `UDisSeqAct_GotoPlayerTravelDestination` has no int at 248, it has a one-bit
  `m_bUseDestinationTargetRotation`.
* The real vtable is **`0xd4e738`**. Three independent facts pin it: `0x7b7e70` (not `0x7b7fa0`) writes it;
  it is 1200 bytes long and ends **exactly** where `0xd4ebe8` begins, so the two sibling classes' tables are
  adjacent; and the function at `0x7853f0 + 0x20` references it, the same +0x20 relation
  `InitializePrivateStaticClassUDisSeqAct_SetPlayerTravelDestination` (`0x7854c0`) has to `0x7854e0` and
  `0xd4ebe8`.
* `0xd4e738 + 372` is **`0x79a940`**, 410 bytes, and its body tests
  `m_PlayerTravelOriginLevelName != DisGetCurrentLevel()->GetOutermost()->GetFName()`, searches
  `FActorIterator` for an `ANavigationPoint` whose `Tag` is `m_PlayerTravelLocationName`, calls
  `DisTeleportPlayer`, clears the destination and copies the pawn's location into
  `ADishonoredPlayerPawn::m_PreviousTravelLocation`. That is the function.

`rva_sweep.py` passes `0x7b7fa0` and `0xd52cf0` without complaint, because both are real 2013 addresses. This
is the second wave running in which the sweep is necessary and not sufficient.

### 5.2 `DisGetCurrentLevel` was `GWorld->CurrentLevel`, and both its body and its rva were wrong

`dissavegame.cpp` carried:

```c
// DISHONORED(port): 2013 rva 0x7ef0c0 (2012 0x82f1c0)
ULevel* DisGetCurrentLevel()  { return GWorld ? GWorld->CurrentLevel : NULL; }
```

* `0x7ef0c0` is **304 bytes inside `UDisPostProcessManager::TickMaskOn`**. The function matched to
  2012 `0x82f1c0` is **`0x7ec3d0`**, and it is **275 bytes**, not two.
* `GWorld->CurrentLevel` also cannot be what retail means. Outside the editor it *is* `PersistentLevel` —
  `UnLevAct.cpp:371` asserts exactly that — so `DisGetCurrentLevel` would answer `DishonoredGameFull_P` for
  every mission, and `UDisSeqAct_GotoPlayerTravelDestination`, whose whole test is "am I in a level other than
  the one the destination was set in", could never fire. Measured with the old body: the origin recorded was
  `DishonoredGameFull_P` and so was the level at arrival, so the test compared a name with itself.
* Retail returns the **mission's own** persistent level: `GWorld->m_pWorldInfo->StreamingLevels(0)->LoadedLevel`,
  and only when `StreamingLevels(0)` is a `ULevelStreamingPersistent` — which is exactly what
  `CommitMapChange` makes it. Measured with the fix: `Dishonored_MainMenu` before the map change,
  `l_tower_p` after it, and the Goto action fires.

The one deviation: retail reads `GWorld->m_pWorldInfo` with no test. This tree calls `DisGetCurrentLevel`
from paths that run before the world info exists, so the pointer is tested.

## 6. Verification

* **Regression**, the worktree's own copy, an absolute build dir, built inside the harness so the six
  build-stage checks count: `python resources\tools\run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEL_wt/build/agentEL_wtreg --exe-name DishonoredGame_ELR.exe
  --log-prefix ELR` -> **`37 ok, 0 failed, 0 skipped, 954s`** (`build/agentEL/regression1.log`,
  `build/agentEL_wt/build/agentEL_wtreg/regression/summary.txt`). The five gitignored
  `resources/docs/types` inputs were copied into the worktree first, so the layout stage measured
  2314 types / 2341 probes rather than recording -1.
* **Clean full release build** of the worktree, build directory deleted first, every target, with
  `DISHONORED_LAYOUT_CHECKS=ON`: **1002 edges, 0 errors, 0 `C4263`, 0 `C4264`**, all four targets linked
  (`DishonoredGame`, `CoreSmoke`, `EdgeAnimSmoke`, `LayoutProbe`) — `build/agentEL/buildclean.log`.
* **Addresses.** Every retail address this package cites was resolved by hand against
  `build/agentEL_ida/retail2013_agentEL.i64`, my own copy of `resources/docs/idb/retail2013_named.i64`, opened
  headlessly through `resources/tools/ida/run.py` (`build/agentEL/q17.py` prints
  `ida_funcs.get_func` start/size/name for each). **No IDA MCP tool and no FModel tool was used.** All 21 of
  the 2013 addresses are function **starts**; the one that is not (`0x7ef0c0`, `MID(+304)`) is quoted only to
  say that it is wrong.
* **`rva_sweep.py`** over `source/Development/Src/DishonoredGame` in the worktree:
  **3667 citations, 2203 `ok-2013`, 1454 `ok-2013-mid`, 10 `ok-2012-labelled`, 0 `MISLABELLED-2012`,
  0 `UNKNOWN-CLAIMED-2013`** (`build/agentEL/sweep_all.csv`). Filtered to the 19 source files this package
  touches: **289 citations, 181 `ok-2013`, 108 `ok-2013-mid`, 0 mislabelled, 0 fabricated**; every
  `ok-2013-mid` on a line this package wrote is the *2012* address quoted beside a 2013 one, which is what
  the bucket means.
* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` (all three flags) is
  required and was run**, from the worktree, so `SDK_SOURCE` resolved to the worktree. Its entire effect is
  seven `#include "CppText/..."` lines in `Inc/dishonoredgameclasses.h` and `Inc/dishonoredgamekismetclasses.h`
  and the `Sources.cmake` change below. The generator writes CRLF and this tree keeps LF, so the regenerated
  files were normalised afterwards.
* **`Sources.cmake` changed**: the count in its header comment (827 -> 821) and **six** units leaving
  `DishonoredGame_EXCLUDE`, because none of them is comment-only any more: `dishonoredkismet.cpp`,
  `dishonoredutilities_math.cpp`, `disseqcond_issentinel.cpp`, `disseqcond_checkstoryflag.cpp`,
  `disseqact_setstoryflag.cpp`, `disstoryflagset.cpp`. It is generated, so a
  `gen_classes_header.py --sdk --sources-cmake` run reproduces exactly this.
* **Files outside `DishonoredGame`: none in the source tree.** The two documents in
  `resources/docs/agents/` are the only other files. `Src/dishonoredplayercontroller.cpp` carried a temporary
  one-shot Kismet probe (`-eltravelprobe`, §7.6) during the investigation and was reverted with
  `git checkout --`; `git status` shows it clean.
* No commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`, and the main
  checkout and the other agents' worktrees were never written to.

## 7. Deviations, stated once

1. **`DisTeleportPlayer` does not call `ADishonoredPlayerController::OnTeleport_Native(NULL)`** (2013
   `0x6a5f40`, the PC vtable's `+1272` slot, which is where retail's last statement goes). The override is not
   declared in this tree at all — only its `DISHONORED_NATIVE_STUB` script thunk — and its body is camera
   bookkeeping: `ADishonoredPlayerCamera::OnTeleport` (`0x6ca370`) sets a flag and forwards to every
   `UDishonoredCameraInfluenceGroup`, then `ADishonoredPlayerPawn::m_LastSecondLocation` is reset.
   `dishonoredplayercamera.cpp` is still a comment-only skeleton, so the call is left out rather than
   half-ported. The move and the rotation, which are what put the player in the level, are there; the
   screenshot is the evidence that nothing visible depends on the rest.
2. **`UDisSeqAct_ShowLocationDiscovery::Activated` (`0x7998a0`) is deliberately not ported.** It sits between
   `SeqAct_Gate_1` and `SeqAct_Gate_8` on the arrival chain, and because a plain `USequenceAction` activates
   its outputs from `DeActivated`, the impulse passes through it without a body. What is missing is the
   "Dunwall Tower" title card, not the arrival.
3. **`ADishonoredPlayerPawn::s_pInstance` is tested in `UDisSeqAct_GotoPlayerTravelDestination::Activated`
   where retail dereferences it unguarded**, because the New Game path reaches that action before anything in
   this tree guarantees the pointer. It is *not* tested in `UDisSeqCond_CheckStoryFlag::Activated`, where
   retail also dereferences it unguarded, because the only path that reaches that dereference has already
   found the flag in a loaded set.
4. **`GWorld->m_pWorldInfo` is tested in `DisGetCurrentLevel`** where retail does not (§5.2).
5. **Two one-line `debugf`s are new and permanent**, one in each travel action. They are what turned "the map
   changed and the frame is still black" into "the destination is `PlayerStart_NewGameEmpress` and nothing
   ever read it", and they fire once per map change.
6. **A temporary `-eltravelprobe` was used and is gone.** It printed the streaming levels, the pawn, the
   destination, every `UDisSeqAct_GotoPlayerTravelDestination` in the world with the *upstream* producers of
   its input link to depth 5, every `USeqEvent_Console` with its name, and the contents of the sequence the
   `SetPlayerTravelDestination` that fired belongs to. Every fact in §2 came out of it. It lived in
   `dishonoredkismet.cpp` and was called from `ADishonoredPlayerController::Tick`; both are reverted, and
   `git diff` on `dishonoredplayercontroller.cpp` is empty. If the next package needs it,
   `build/agentEL/ng3_log.txt` and `ng4_log.txt` are its output.

## 8. Hand-overs

1. **The title card.** `UDisSeqAct_ShowLocationDiscovery::Activated`, 2013 `0x7998a0`, 42 bytes:
   `DisGetGFxHUD()->ShowLocationDiscovery( m_Text, (bitfield & 1) == 0 )` then
   `USequenceAction::Activated`. It is on the arrival chain and it is the one visible thing the arrival is
   still missing. `UDisGFxMoviePlayerHUD::ShowLocationDiscovery` is the work, not the action.
2. **The story-flag store is now real, and nothing saves it.** `m_StoryFlagInstances` is `CPF_Transient` and
   `ADishonoredPlayerPawn::GameSave`/`GameLoad` are not ported, so flags do not survive a save. Every
   `DisSeqCond_CheckStoryFlag` in the game now answers correctly *within one session*; the moment saves work,
   this array has to be in the stream.
3. **`UDisSeqCond_*` is a class of gap worth sweeping.** A condition with no `Activated` body silently ends
   every chain it is on, and there are dozens of them (`DisSeqCond_IsDoorOpen`, `DisSeqCond_PawnIsPossessed`,
   `DisSeqCond_CompareBoolExtended`, ...). Unlike an unported *action*, an unported *condition* produces no
   log line and no warning: `ActivateCount` goes up and nothing happens. A one-off census of
   `USequenceCondition` objects with `ActivateCount > 0` and every output at `bHasImpulse == 0` would list
   them all in one run.
4. **The camera does not know it was teleported** (deviation 1). Once `dishonoredplayercamera.cpp` is a real
   unit, `ADishonoredPlayerController::OnTeleport_Native` (`0x6a5f40`) is three lines and belongs at the end
   of `DisTeleportPlayer`.
5. **`match_2012_2013.csv` is wrong about at least one class and the sweep cannot see it** (§5.1). The
   failure mode is a 27-byte `InternalConstructor` matched by neighbour order: every one of them is identical
   except for the vtable address it writes, so the ratio is 1.000 for the wrong answer too, and the wrong
   answer then propagates to the class's vtable through `global-table` votes. Any package that reads a body
   out of a vtable should check that the vtable's own `InternalConstructor` is the one the matcher names, or
   pin the vtable a second way — adjacency and the `InitializePrivateStaticClass + 0x20` reference both work.
6. **`DisGetCurrentLevel`'s old body is under three waves of save-layer reasoning.** §5.2 changed it under
   `dissavegame.cpp`'s own callers (`DisIsLevelShared`, the neighbours of `DisFindLevelFromName`,
   `dishonoredengine.cpp:1294`). The regression is clean and the milestone map still loads, but nobody has
   re-read the save layer with the new meaning of "current level", which is now "the mission's persistent
   level" rather than "the hub".
7. **The player pawn is the menu's pawn, moved.** `GotoPlayerTravelDestination` does exactly what retail does
   — `FarMoveActor` on the existing `ADishonoredPlayerPawn`, which lives in
   `DishonoredGameFull_P.PersistentLevel` — so the pawn that walks the Tower is the one that was spawned for
   the menu world. That is retail's own shape (the hub persistent level owns the pawn and missions stream
   under it) and is recorded here only because it surprises anyone reading the object names in a log.

## 9. Files

Mine (20, seven new), all under `source/Development/Src/DishonoredGame`:

* **new**: `Inc/CppText/UDisSeqAct_SetPlayerTravelDestination.h`,
  `Inc/CppText/UDisSeqAct_GotoPlayerTravelDestination.h`, `Inc/CppText/UDisSeqAct_SetStoryFlag.h`,
  `Inc/CppText/UDisSeqCond_CheckStoryFlag.h`, `Inc/CppText/UDisSeqCond_IsSentinel.h`,
  `Inc/CppText/UDisStoryFlagSet.h`, `Inc/CppText/FDisStoryFlagInstance.h`
* **edited**: `Inc/CppText/ADishonoredPlayerPawn.h`, `Inc/dishonoredutilities_math.h`,
  `Src/dishonoredkismet.cpp`, `Src/dishonoredutilities_math.cpp`, `Src/disseqcond_issentinel.cpp`,
  `Src/disseqcond_checkstoryflag.cpp`, `Src/disseqact_setstoryflag.cpp`, `Src/disstoryflagset.cpp`,
  `Src/dishonoredplayerpawn.cpp`, `Src/dissavegame.cpp`
* **regenerated**: `Inc/dishonoredgameclasses.h`, `Inc/dishonoredgamekismetclasses.h`, `Sources.cmake`

Outside the module: `resources/docs/agents/agentEL.md` and `agentEL_status.csv` (this file and its table),
written in the worktree because the main checkout is not mine to edit. **Total 22.**

`build/agentEL_sync.py` is the authoritative copy list and `--check` prints what differs.

Scratch, not repo tools: `build/agentEL/` — `go.py` (the wrapper around `resources/tools/drive_input.py`),
`q1.py`..`q17.py` and `isfunc.py` / `vt.py` / `dec.py` / `disasm.py` / `names.py` / `xrefs.py` (the headless
IDA queries), the run logs `ng1`..`ng5`, `tower`, `head`, `head2`, the build logs, `sweep_all.csv` and the
screenshots; `build/agentEL_build.cmd`, `build/agentEL_run.py`, `build/agentEL_sync.py`; worktree
`build/agentEL_wt`; build directories `build/agentEL_wtrel` (the play build the runs are driven on),
`build/agentEL_wt/build/agentEL_wtreg` (the harness's own), `build/agentEL_clean` (the clean gate) and
`build/agentEL_head` + `build/agentEL_headrel` (the untouched HEAD tree, extracted with
`git archive 0cbe3fe`, and its build). IDA: **own copy only**,
`build/agentEL_ida/retail2013_agentEL.i64`.
