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

## THE CAPTURE RULE — unchanged, and the dialogs are gone

Take every screenshot **inside the game process** with `-apshottime=<seconds>` and
**`-apshotname=<your tag>`**, then glob your own tag. Never a desktop or window capture.

**Real mouse input works again.** The "Windows Security Alert" prompts were ours — three dev-only
listeners bound a port on every launch — and `98e22d4` made them opt-in. `grep -c FocusLost <log>`
is still the validity flag; a concurrent agent's game taking the foreground kills the menu
permanently, and that run should be discarded rather than averaged in.

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
