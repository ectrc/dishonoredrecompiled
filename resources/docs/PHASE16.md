# Wave 20 — the red row, the cutscene's activation, one owner for the HUD, and the unload list

Wave 19 closed with the gate **red on purpose**: 47 checks, 46 ok, and `inputtest_criticals == 1` is
a real crash that the harness had been unable to see for nineteen waves. This wave starts there.

The user's standing instruction is unchanged — *"get the UI as perfect to the original as
possible"* — and the user's last report was that the opening cutscene does not play. Wave 19 found
out why, and it was not what the plan said: **the first Dishonored cutscene to reach
`UInterpGroupInst::InitGroupInst` killed the process every time**, on a script event retail never
raises. That is fixed. What is left is that **the opening cutscene is never activated** (`activations
0` across a whole new-game route), and that is now the only *known* blocker to the thing the user
asked for.

## THE INPUT RULE — new this wave, from the user, and absolute

**Nothing you run may move the user's real mouse, press their real keys, or take window focus.** The
user works on this machine while you run, and agents driving the game with real input kept stealing
the foreground from them. `resources/tools/drive_input.py` (SendInput + SetForegroundWindow) is
**retired and refuses to run**. Do not reimplement it. Do not call `SendInput`, `SetCursorPos`,
`SetForegroundWindow`, `AttachThreadInput`, `BringWindowToTop` or `SetFocus` from any script, and do
not click through any dialog.

**Drive the game through `resources/tools/game_input.py` instead:**

```
python resources/tools/game_input.py launch --exe <exe> --log <name>.log --tag <xx> [-- more game args]
python resources/tools/game_input.py --cmd-file <printed path> hover 0.419 0.216 wait 1 shot tabs click key Enter
```

`launch` starts the game with `-inputcmd=<file> -unattended -apshotname=<tag>`. Under `-inputcmd` the
engine **forces the no-grab mode**: the window is shown `SW_SHOWNOACTIVATE`, never calls
`SetActiveWindow`, never captures or clips the cursor, and ignores every real key and mouse message.
Commands are injected **inside the process, after DirectInput**, through the same
`UGameViewportClient::InputKey/InputAxis` entry points real input uses, and
`FWindowsViewport::GetMousePos` returns the scripted pointer so Scaleform follows it. Coordinates are
**fractions of the viewport** (0..1), so the window's pixel size does not matter. The game logs every
command as `DISHONORED(inputcmd): t=<world time> <command>` and `... idle` when done — **correlate
against those, not against sleeps.** `shot TAG` writes `apshottime_<TAG>NNNNN.bmp` into
`DishonoredGame/Screenshots/Win32Console/`.

Verified before this wave resumed: the foreground HWND is identical before, during and after a
scripted run; a scripted click advanced the start screen (`TransitionTo (MainMenuScreen)`,
`OnMouseUp`); a scripted `key Escape` was delivered; a script executes exactly once.

**`-finewgame`, `-disdifficulty`, `-fmmenu`, `-fkdiag` and the other in-process instruments remain
fine** — they never touched real input. Launch everything with `-unattended` at minimum.

## THE CAPTURE RULE — unchanged

Take every screenshot **inside the game process** with `-apshottime=<seconds>` or `game_input.py`'s
`shot TAG`, with **your own tag**, then glob your own tag. Never a desktop or window capture.
`grep -c FocusLost <log>` is still a validity flag, though with no run ever taking focus it should now
read 0 everywhere.

Still true: the desktop resolution flaps between 1024x768 and 6144x1728, so **read every capture's
real size off the file**, and **a pixel coordinate off a capture is not a pixel coordinate for the
input driver** — agent FJ saw a 1280x720 client and a 1600x900 bitmap on the same command line.

## The gate

`run_regression.py` is **47 checks**. HEAD reports **46 ok, 1 failed**. The failing row is real and
it is **package FN's**. Every other package should expect **46/47 until FN lands**, and must say in
its report which row failed and that it is the known one. **Nobody relaxes the bound.**

Three harness facts from wave 19 that every package needs:

- **`CRITICAL_RE` now matches `=== Critical error: ===` as well as `Critical: `.** Before `cd5fadd`
  an access violation inside a stage was invisible, so a "37/37" from an earlier wave does not mean
  the build was crash-free then. Do not cite one as if it did.
- **`-finewgame=<secs>` is on the world clock, which starts before the first map loads.** Under load
  `LoadMap` began at 27.6 s, so `-finewgame=22` fired into an async load and asserted in
  `StaticAllocateObject`. If you drive a new game, give it room, and FN raises the harness's
  `--newgame-at`.
- **Every run without `-nodishud` now has the HUD movie open.** If numbers moved, that is why.

## Packages

### FN — the red row, and the spawner that is "connected to one Group"

**The crash.** At ~38–46 s of the `inputtest` stage, `-distouchprobe` walks the pawn onto
`DisTrigger_9`, which fires `SOIREE_Guards.SeqEvent_Touch_0`, which runs `DisSeqAct_StartSpawn_0`,
and **the spawn kills the process**. Every metric the stage measures is emitted just before, which is
why it passed for nineteen waves. Intermittent: present in three independent gate runs of HEAD, absent
from two others. The one credible frame is **`UArkComponentContainer::AddNewComponent_Internal+0x2d`**;
nine of the ten others resolve thousands of bytes past a public symbol and should not be believed.

**Two leads, from two packages that were not looking for it.** Agent FK: the line immediately before
the fault is `an Actor may only be connected to one Group! [DishonoredSpawner_11]`, and the pawn that
dies is one `DisSeqAct_StartSpawn_0` produced **one frame earlier**. FK's own hand-over item 2 is
**`UInterpGroupInstAI::InitGroupInst` with a spawner as the group actor** — a matinee AI group whose
actor is the `DishonoredSpawner`, not the pawn it spawns. **These may be one fault.** Agent FI ported
the spawn path a wave earlier (`OnStartSpawn`, `DoSpawnNow`, `UDisSeqAct_StartSpawn::*`), and it may
be that spawning *now works well enough to reach a crash it could not reach before*.

**Open the minidumps first.** There is one in `DishonoredGame/Logs/unreal-v9411-*.dmp` for every
instance of this crash. Agent FJ's method for the ASLR base is in `agentFJ.md` — the exe is
`/DYNAMICBASE`, so do **not** subtract `0x400000`. Then read retail's `UInterpGroupInstAI::InitGroupInst`
rather than guessing what it does with a spawner. `rva_sweep` does not scan banner comment blocks and
passes VAs beside RVAs in both directions; resolve every address by hand.

**Also yours, small**: raise `--newgame-at` in `run_regression.py` per FK's measurement, and the
**intermittent assertion on the front end's own `PrepareMapChange`** (FK, 2 of 10 runs, undiagnosed)
— diagnose it or specify it, do not leave it unnamed.

**Acceptance**: the red row goes green on a clean checkout, **and** you can say what the fault was
and why it was intermittent. A green row without a named cause is not acceptance — the crash was
intermittent before too.

### FO — the cutscene's activation, then its tracks

Agent FK's hand-over, in FK's order, and **sequenced because it will not all fit**:

1. **What activates `L_Tower_Script…SOIREE_Empress_Regent.SeqAct_Interp_12`.** 38 groups, eight
   `InterpGroupAI` (Empress, Emily, Lord_Regent, Campbell, Elite_A/B, Daud, assassin), a `PlayerGroup`,
   a `DirGroup`. `activations 0` across a whole new-game route. Trace the Kismet upstream of it; an
   unported `USequenceCondition` kills a chain **silently** (STATUS, agent EL), and FK counted eight
   conditions blocked on a helper in one excluded unit. This is the only *known* blocker to the thing
   the user asked for, so it is first.
2. **The `UpdateTrackInst(FLOAT, UBOOL)` seam.** A virtual that exists nowhere in this tree, used by
   `FaceTo`, `Locomotion`, `StretchAnimControl` and `Dialog` — and **`StretchAnimControl` is 267 of
   the level's 694 tracks and sits on all six player groups**. `LookAt` uses a static. Only
   `AIControl`, `Attachment` and `DisAllowPlayerLook` use `UpdateTrack`. Build the seam, then the nine
   bodies.
3. **The `MAT_*` contract**: 22 on `ADishonoredNPCPawn`, 8 on `ADishonoredPawn`, 5 on
   `ADishonoredPlayerPawn`. The player five first — `MAT_BeginPlayerGroup` requests a state change
   into `UStatePlayerMasterSoiree`, an **excluded unit**, so the player FSM is the prerequisite.
   `m_pMatineeBlender` and `UInterpGroup::HasMoveTrack` do not exist here at all.
4. `AActor::MAT_SetPhysics` / `BackupCollisionType`; `MAT_BeginAnimControl` as an `AActor` virtual;
   `NeedWaitToStart`'s caller; `InitInterp`'s three retail differences (`CanPlay`,
   `FilterObjectDuplicate`, the nav-list bracket).

**Coordinate with FN** on `UInterpGroupInstAI::InitGroupInst` — it is on both your paths. FN owns
the crash; if you need that function fixed to proceed, say so and wait rather than fixing it twice.

FK's instruments: `-fkdiag`, `-fkdiagat=<secs>`, `-fknoplayergroup` (the control). Its report §9 is
the specification; read it in full before starting.

### FP — the HUD on every screen, one owner for the HUD, and the second letterbox

**The user's newest report, and it is a wave-19 regression in effect: the health bars are on every
screen, including the menu.** Wave 19 flipped `-dishud` after proving retail opens the HUD movie
**unconditionally** — which is right — but retail then *hides* it on every screen that asks for that,
and the hiding is unported. Retail's `UDisGlobalUIManager::RefreshGlobalUIState` (2013 rva
`0x847070`, 901 bytes; 2012 `0x8b75e0`) recomputes on every movie-stack change:

```
bShowHUD  = !(GWorld->PersistentWorldInfo->m_bNeverDisplayHUD)        // AWorldInfo+752, mask 0x400
for each open UDisGFxMoviePlayerBase m:
    bBlurGame         |= m->m_bBlurGameWhileActive                  // +380 bit 1
    bDrawBlackStripes |= m->m_bDrawBlackStripesWhileActive          // bit 2
    bShowHUD          &= m->m_bShowHUDWhileActive                   // bit 8   <- AND, not OR
    bPauseHUD         |= m->m_bPauseHUDWhileActive                  // bit 0x10
m_pHUD->pMovie->fVisible = bShowHUD;   m_pHUD->pMovie->fUpdate = !bPauseHUD;
m_pHudFX->pMovie->fVisible = 1;        m_pHudFX->pMovie->fUpdate = !(bPauseHUD | bBlurGameSeed);
m_pGlobal->m_bRequestBlackStripes = bDrawBlackStripes;  m_pGlobal->m_bMouseCursorAllowed = bAllowMouseCursor;
EnableControllerInput(0x7FFFF, !bBlockInput, DisInputMaskLevel_BlockedUI);
```

**Ours (`disglobaluimanager.cpp:120`) has only the blur arm.** Agent FA ported that; nothing else in
the function exists. So the HUD is opened (now) and never hidden. Before wave 19 the missing arm had no
symptom because the movie that it hides was behind a switch — **a feature that was measured only in an
instrumented run had a regression hiding behind it too.** Agent FM's helper decoded the whole
function against `retail_sdk_layout.json`, member by member, in `agentFM.md`.

**This is first**, because it is what the user sees, and it is small: port the rest of
`RefreshGlobalUIState`, confirm with `-apshottime` that the menu, the difficulty screen and the
brightness screen have **no HUD** and the mission does, and check **who calls it** — retail reaches it
from `OnMovieStackChanged` (a 5-byte `jmp` thunk, 2013 `0x84cf70`) and `OnMovieAttributesChanged`,
so it must actually run when a movie opens or closes here. One input you cannot read from any ini:
**`m_bShowHUDWhileActive`'s default on each movie player lives in cooked script defaults.** The menu
movies must have it FALSE and the HUD's own player TRUE (the seed is AND-ed across every open movie,
the HUD's own player included); if the cooked defaults do not reach our objects, the expression is
wrong regardless of the port. **Read them out of the cook, or out of retail with dismod, before
concluding anything else is wrong.**

Then the ownership, which is why the HUD was behind a switch in the first place. The flip alone
crashed the menu in 2 of 3 runs, because **retail has one owner for every movie player and this tree
has two racing bring-ups** (`DisHUDBringUp`, `DisGFxAutoOpenMenu`). Agent FM latched one behind the
other as a guard and named the real fix:

2. **Port `UDisGlobalUIManager::Init`** (2013 rva `0x85fdb0`, 1428 bytes; 2012 `0x8c45d0`) and
   `FindCurrentMovieSet` (2012 `0x8c0020`; 2013 `sub_C51970`, **medium confidence** — inferred from
   call position only, pin it), and **delete both bring-ups**. FM's helper left the member layout
   (`m_pGlobal` 724, `m_pHUD` 732, …) and the config section (`[DishonoredGame.DisGlobalUIManager]`,
   `m_DefaultUI`, `m_DLCUI`) in `agentFM.md`. Note **`FDisUIMovieSet` changed shape between builds** —
   2012 is 60 bytes keyed on `m_MapName`, 2013 is 160 bytes keyed on `m_MapFilter` with per-slot
   class-name overrides.
3. **The `UI_Global` letterbox.** The bars exist in **two** movies: `UI_HUD`'s (driven by
   `Tick_Cinematic`, two `SetBlackStripes` arguments — ported) and `UI_Global`'s (driven by
   `UDisGFxMoviePlayerGlobal::PreAdvance`, 2013 rva `0x7aa610`, 1241 bytes, **one** argument —
   **absent**). The global one is what lets a UI screen letterbox while the HUD is hidden, and it is
   what `m_bDrawBlackStripesWhileActive` across open movies feeds.
4. `m_bShowHUDWhileActive`'s default is covered under item 1; if item 1 found it wrong in the cooked
   defaults, that finding belongs here with the fix.

### FQ — the unload list, then the guard, then the mouse

Agent FJ fixed the Options crash in the dump and **deliberately did not** write the second guard the
same instruction needs, because it would dereference the pointer it protects. In FJ's order:

1. **Retail's movie-root unload list.** `GFxSprite::ExecuteEvent` (2012 `0x9f4a10` + `0xb9`) links
   every unloaded sprite onto a list at `movieroot+0x27fc` with an explicit `inc [esi+4]`. That is
   what makes any raw `GFxASCharacter*` safe for the rest of an advance in retail — including
   `DragState::pCharacter`, which retail also copies as a bare dword with no `AddRef`, as we do.
2. **Then the `GASFunctionObject::pDeclTarget` guard** (`GFxAS2Interp.cpp:443`; retail's is in
   `GASAsFunctionObject::Invoke`, 2012 `0x9f3540` / 2013 `0x9e9d10`, falling back to the caller's env).
3. **The re-entrant path.** The user's actual crash came through **GFx's own mouse handler** —
   `AS2 trace: OnMouseUp (0, undefined)` is the line before the fatal `OnTabSelected` — re-entrant
   with the runtime's advance/display state. Both FJ's and FM's reproductions enter through
   `GFxMovieView::Invoke`, which is a clean entry point, and **neither reproduced the death**. Drive
   the switch through a real mouse event (input works now) with the scroll bar dragged to the bottom
   first (`OnLifterPressed` → `ratioScroll` 0.999964, as the user had it), and find out whether
   re-entrant `Display` has a second defect. PHASE15's first candidate — `ctx.MaskLevel` across a
   re-entrant `Display` — is still open with FM's measurement in its favour.
4. **Our VM stringifies a `MovieClip` as `undefined`.** A trace printed `undefined` for `this` and the
   next instruction succeeded on the same object. Retail stringifies a clip to its target path. **Every
   "on undefined" in our AS2 error text is untrustworthy until this is fixed**, which makes it worth
   more than its size.
5. `UpdateSettings` — the one genuine missing prototype method of the three agent FF raised, not seen
   on any screen FG's driver reaches. And `TArray<GFxValue>` is unsafe (destructor plus uninitialised
   `TArray::Add`), per FM.

This package owns `External/GFx3`.

## Deferred, and said so

- **`FArkComponentLookat`** (51 functions, inventoried with 2013 rvas in `agentFL.md`) and
  **`UDisSkelControl_SpineBender`** (9 functions, live on every NPC, a silent no-op). The heads are
  level without them; the look-at *magnitude* cannot be proven until the component exists, and a
  cutscene's pawns are meant to look at things. **Wave 21**, after the cutscene activates — there is
  no point aiming heads in a scene that does not play. FL's trap stands: when `m_pConfig` becomes
  non-NULL, `UpdateNodeWeights` fed a raw (0,0) puts 1.00 on the grid centre, worth 60° of head.
- The rest of the UI list: Enter not activating a main-menu row (STATUS defect 5), the menu camera
  transform and haze parameters (both captured from retail in `build/agentFB/retail/`), text clarity
  (never measured), the 2 GB exhaustion after ~3.5 min on the menu.
- **Wave 15** (ET perception, EU patrol blockers, EV the last 82 KB of the save, EW five layout
  defects) stays paused.

## Rules for agents

- **Every build goes through `resources/tools/build_queued.py`. Never call `cmake --build` or `ninja`
  directly — not in your own helper scripts either.** The user asked for this after four agents
  building at once pinned all sixteen cores: the wrapper takes a machine-wide lock so builds run one
  at a time (a waiter prints a line every 30 s naming whose build it is waiting on), and it pins the
  build to **half the cores by affinity mask** with `-j 8` and below-normal priority — a hard cap that
  cl.exe inherits, verified by reading the mask back from the OS. `build-release.cmd`,
  `build-play.cmd`, `build-game.cmd`, `run_regression.py`'s in-harness build and `verify_phase2.py`
  all already use it, so going through those is enough. **A build that waits is not a build that
  hung**: look for the `build_queued: waiting for another build` line before concluding anything.
  Expect your clean full build to take longer than last wave's did.
- **Capture in-process only, with your own `-apshotname` tag.**
- Work **only** in your own worktree. Your sync script runs **worktree → main**, **flags rather than
  copies** anything changed in main since your base, and **names every function you touch per file**.
  Three packages now share `dishonoredplayercontroller.cpp` (FI, FL, FK); wave 18's one semantic
  collision survived a three-way textual merge and was caught only by a build.
- **Open the dump before rebuilding anything.** There is a minidump for every crash in
  `DishonoredGame/Logs/`. The exe is `/DYNAMICBASE`; `agentFJ.md` has the base-recovery method.
  A two-identical-frame stack means the walker failed, not that the fault is one frame deep.
- **Prefer a measurement in the real game over an argument about it.** `dismod_harness.md`.
- **Resolve every retail address yourself.** `rva_sweep.py` is a lint, not a gate: it passed a VA for
  an RVA in both directions and does not scan banner comment blocks. `vtables.csv` is the 2012 table;
  `match_2012_2013.csv` has propagated a wrong vtable; skeleton comments carry 2012 rvas; remember
  `+ 0x400000` **only** for the 2013 database — `shipping2012_v1.i64` has imagebase 0.
- **A unit taken off `Sources.cmake` needs an external-linkage anchor** or MSVC drops the object and
  the symptom is a virtual that will not dispatch.
- **A feature measured only in an instrumented run is not a feature the user has.** Check your claim
  against the way the user launches the game — the staged executable, `-LOG= -forcelogflush`, and the
  play defaults. The bars were "working" for a whole wave while being unreachable.
- **Never use the FModel MCP tools.**
- `run_regression.py` from **the worktree's own copy** with an **absolute** build dir, built inside
  the harness; **47 checks, expect 46/47 until FN lands**. Copy the five gitignored
  `resources/docs/types/` inputs in first.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** In wave 19, FJ
  corrected six premises of its brief, FL six, FK five and FM five, and every one of the four found
  the cause somewhere the plan had not pointed.

## Acceptance, common to all four

1. For anything claimed fixed: a before/after against the untouched HEAD executable through the same
   driver, and a side-by-side against retail where the fault is visual.
2. `run_regression.py`: 47 checks, **46 ok with `inputtest_criticals` the one failure** — or 47 ok
   once FN has landed. Name the failing row.
3. A clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: 0 errors,
   0 C4263, 0 C4264.
4. Every address cited resolved by hand with its function start and size in both databases.
5. `resources/docs/agents/agent<XX>.md` and `agent<XX>_status.csv`, written **inside the worktree**.
6. `build/agent<XX>_sync.py`, naming every file and every function.

## Outcome

### FO — merged `15695d8`; gate on main **45 ok / 2 failed** (see below); FO's own gate 46 / 1

Two files plus the two documents; one new diagnostic unit (`fokismetupstream.cpp`) and one line in
FK's `fkmatineediag.cpp`. **Nothing behavioural, deliberately.** `UnInterpolation.cpp`, the
`UInterpGroupInst*` family and every generated header untouched.

**The opening cutscene is not started by Kismet.** `SOIREE_Empress_Regent.SeqAct_Interp_12` has five
inputs with **zero links**, 20 of 21 outputs wired to remote events, and **exactly one referencer in
the whole process** (an `FArchiveFindCulprit` scan of every UObject): its owning sequence's
`SequenceObjects`. There is no chain for an unported condition to kill. **This plan's premise was
wrong in kind, not degree.**

Retail starts it from the **conversation system**: `UDisSeqAct_DialogInputs::Activated` (2013
`0x79bf00`) → `IDisConvSpeakerInterface::FireDialogHook` (`0x8a7360`) →
`UDisConversationComponent::FireDialogHook[Now]` → `UDisDialogTree_InGameBind::FireDialogHook`
(`0x8a5150`) / `EvaluateDialogHook` (`0x8a4690`) → `FDisDialogRunningInstance::StartInstance`
(`0x88bda0`) → `UDisConv_Soiree::PlayNode_Derived` (`0x8a7a60`) → `PlayNodeMatinee` →
**`FindMatinee` (`0x8b5de0`: `TObjectIterator<USeqAct_Interp>`, `Interp->m_MatineeGUID` at +472 ==
node's `m_SoireeGUID`)** → **`USeqAct_Interp::StartFromDialog` (`0x215020`: `ForceActivateInput(0)`,
`Position = 0`)**. **Proved in data** with the new `-fosoiree`: 31 `UDisConv_Soiree` nodes load with
`l_tower_p`, **all 31 match a `USeqAct_Interp` by GUID**; `SeqAct_Interp_12` is
`DisDialogOneShot_0.DisDialogTree_OneShot_0.DisConversation_8.DisConv_Soiree_1`. The mission's whole
scripted-scene layer — 13 Empress/Regent, 7 Campbell/Sokolov, 6 Emily, 3 Guards, Boat, Workers — is
driven by **ten `DisDialogOneShot` actors**.

**The first link already fires on our route and dies at once**: `-diskismet` counts
`DisSeqAct_DialogInputs x1` every run — `SOIREE_Boat.DisSeqAct_DialogInputs_10`, input `Intro`,
one-shot `DisDialogOneShot_8`, whose node names the boat ride. Its native `Activated` is a
comment-only skeleton, so `USequenceAction::Activated` passes the impulse through and the scene layer
stays dark all the way to the Empress.

**Size of the port: 66 conversation/dialog units on `DishonoredGame/Sources.cmake`'s exclude list,
~60 KB / 179 non-boilerplate retail functions** (`UDisDialogTree` 21 KB, `UDisConversationComponent`
10 KB), ~10 KB on the direct path, specified in call order in `agentFO.md` §9. **Wave 21 is a
dialog-system wave.**

**Not reached, specified.** The `UpdateTrackInst` seam is **not a single virtual**: retail's
`UInterpGroupInst::UpdateGroupInst` (2012 `0x25b4e0`, 858 B) is **five passes on
`UInterpTrack::TrackUpdatePass`** (a byte this tree already declares at +105) with a 0.1 s re-step of
passes 2–3 plus `FArkComponentLocomotion::MovePawn` in skip mode. `StretchAnimControl` (`0x543f40`)
is self-contained at ~4.2 KB and goes first. **A gap no brief named**: retail
`APawn::MAT_SetAnimPosition` calls **`UArkAnimNodeStack::MAT_SetAnimPosition`** (2012 `0x576570`,
536 B); ours calls `UAnimNodeSlot`'s, and `UArkAnimNodeStack` here has **zero bodies** — without it
the seam animates nothing on any Dishonored pawn. The `MAT_*` player five are read; the player FSM
(`UStatePlayerMasterSoiree`, `FDisSoireeControlState::EnterSoireeMode` 2012 `0x69a4b0`) is the
prerequisite.

**FN coordination**: `UInterpGroupInstAI::InitGroupInst` (2013 `0x2304f0`) read in full — **no actor
swap**; a NULL actor is bound lazily every frame by `UpdateGroupInst` via
`FindUnusedGroupLinkedVariable`. Consistent with FN's `UDisSeqVar_SpawnerPawn` finding. FN's
`UInterpData`-walk crash is the next thing to fire once the dialog path is ported, because
`Tower_EmpressRegent` is a shared asset.

**Five premises corrected**: no Kismet upstream exists; **`m_pMatineeBlender` and
`UInterpGroup::HasMoveTrack` both exist here** (`dishonoredgameclasses.h:2481`,
`UnInterpolation.cpp:3164`), contrary to this plan; the seam is a pass loop; `InitGroupInst` was never
the spawner fault; and "the only known blocker" had three more behind it.

**The gate on main read 45 ok / 2 failed.** `inputtest_criticals 1` is the known FN row. The second,
`unported_natives 1`, is **`ADishonoredPlayerPawn::execPlayDying_Native` at 10.5 s of the d3d9 stage —
the player pawn died ten seconds into the map.** FO's own gate of the same code had **zero** such
lines, and FO's merge is diagnostics only, so this is non-deterministic and not FO's. The log shows
**no fall, no damage, no late streaming line** before the death, so the cause is **unknown** and is
recorded as such rather than attributed to machine load. **The row stays red; `15695d8` is to be
re-gated on a quiet machine.** A pawn that can die ten seconds into a map with nothing in the log is a
defect in its own right.

### FQ — merged `e14b934`; FQ's own gate 46 / 1 (the known row)

Ten files, all in `External/GFx3`. Every address resolved by hand in both databases with matching
sizes; `rva_sweep` over GFx3 728 citations, 0 mislabelled, 0 unknown.

**The movie-root unload list is in.** `GFxSprite::OnEventUnload` links the sprite onto
`GFxMovieRoot::pUnloadListHead` with a reference on the call that raises its unloaded bit (retail
`GFxSprite::ExecuteEvent` 2012 `0x9f4a10+0xb9` / 2013 `0x9eb1f0`); `ProcessUnloadQueue` (2013
`0x9f80b0`) walks and releases it after each of `Advance`'s three drains; the destructor sweeps it. A
new `GFxASCharacter::OnEventUnload` (2013 `0x9c61d0`) clears the drag, dispatches `onUnload`, raises
the bit and calls `pHandle->ChangeCharacter(0)`. Measured: the list is the last holder of a clip 8
times in the first 10 s of the menu.

**The user's exact crash path, driven end to end through GFx's own mouse handler with
`game_input.py`** — lifter dragged to `ratioScroll 0.999964105364522`, the user's value, sub-tab
toggles, CONTROLS, giving `OnMouseUp (0, undefined)` then `OnTabSelected (1, r)` inside the mouse
dispatch — **three cycles × two runs, 0 criticals, clean exit.** Not claimed as "fixed"; it is the
path that killed the user, run to completion. `ctx.MaskLevel` across a re-entrant `Display` stays
unmeasured: nothing on this path reaches `Display` re-entrantly, and scripted events come from
`UGameViewportClient::Tick`, not the Win32 pump.

**Found on the way, user-visible on HEAD with a real mouse: after a scroll, every click on the tabs
landed on a masked-out list row.** `GFxSprite::GetTopMostMouseEntity` never tested the sprite's
`setMask` mask; retail does so **first thing** (2013 `0x9f0e80`). Ported; hit tests at the tab points
went from `widget5.btn`/`widget2.btn` to `tab0.btn`/`tab1.btn`, 100 %. **This, not `MoveTo_End`, is
why FM could not complete the user's path.**

**A `MovieClip` stringified as `undefined`** because `ToString` fell through `valueOf` to
`Object.prototype.valueOf`, which answered undefined for a clip. Retail's `ToStringImpl`,
`GASObjectProto::ToString`/`ValueOf`, `GetCharacterNamePath` and `ToPrimitive` ported (2013
`0x9c26f0`, `0x9d1340`, `0x9d1410`, `0x9c13b0`, `0x9c3120`). **Re-attribution, correcting this plan:
the "on undefined" error text never came from stringification** — it is literally `self == 0` — but
the receiver is now *named*, and on the user's path **10 of 104 "on undefined" change meaning**: 8
are on a removed clip (`…_options_mc.scrollBar`, FJ's `removeMovieClip`, a no-op in retail too), 2
are on null, **94 are genuine**.

**Six premises of this plan corrected.** (1) The `pDeclTarget` guard **never depended on the list**:
retail's `GASAsFunctionObject` holds the declaring clip as a `GFxCharacterHandle` whose `pCharacter`
is zeroed at unload — FJ's hand-over and this plan both had it wrong; ported as a handle with retail's
unloaded fallback (`Invoke` 2013 `0x9e9d10`), 0 fallbacks fired. (2) "Drive with a real mouse" —
forbidden mid-package, and FQ's first eight real-input runs went into a window launched with
`-unattended`, which disables input grab; only the log said so. (3) "On undefined" is unrelated to
stringification. (4) The user's path could not be completed on HEAD because of the mask hit test, not
`MoveTo_End`. (5) **`UpdateSettings` is not a defect**: the receiver is `NewGameMenu extends
_common.GenericMenu` and `MainMenu.gfx` carries no `UpdateSettings` string — undefined in retail's
cook too. That closes the last of agent FF's three "missing methods": all three were never library
gaps. (6) FQ's own: its first "control" was a `sed` of the build script that missed the `cd` line, so
runs c1–c8 measured the package; every control number is from the true export, c9.

**Found and left**: `ProcessUnloadQueue`'s parent notification (vtable +0x90) not reproduced;
`GetHitArea` arm absent; `DragState::pCharacter` stays bare, as retail's is — the list is what makes
it safe. `TArray<GFxValue>`: nothing left; FM had already fixed its site.

All five `options_*` rows green; `options_as2_errors` 41 = bound — the stringification fix changes
the **text** of the errors, not their count.

**Gate on main: 46 ok / 1 failed / 0 skipped (1361 s)** - `inputtest_criticals` the known row; `unported_natives` green, so the pawn death in FO's gate was non-deterministic.
