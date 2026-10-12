# Wave 21 — the conversation system, and the seam that moves a pawn in a scene

The gate is green at **47 of 47** (`80cc219`). The matinee *runtime* works: a forced cutscene
initialises and plays (FK), the Boat matinee runs to the end with its spawned guard as its AI actor
(FN), the HUD and letterbox appear exactly where retail puts them (FP). **What does not happen is the
thing the user has asked for twice: the opening cutscene starting on its own.** Agent FO proved why:

> `SOIREE_Empress_Regent.SeqAct_Interp_12` has five inputs with zero links and exactly one referencer
> in the process. Retail starts it from a **dialog tree**, by GUID. All 31 soirée nodes in the level
> match a matinee by GUID. The whole scripted-scene layer hangs off ten `DisDialogOneShot` actors.
> **66 conversation/dialog units are on the exclude list, ~60 KB, 179 retail functions, ~10 KB on
> the direct path.**

So wave 21 is a dialog-system wave. Behind it, in FO's and FN's hand-overs, are the three things a
scene needs once it starts: the track seam that moves pawns, the anim node retail actually drives, and
the AI contract that hands a pawn to the scene.

## THE THREE RULES — from the user, all absolute

**Fullscreen.** A run from this wave went fullscreen on the user's display. **Every game launch passes
`-windowed`.** `game_input.py launch` adds it; any other launch — the harness, your own runner,
`-startmap`/`-finewgame` by hand — carries it on the command line. The retail ini defaults to
fullscreen, so a launch without it takes the whole screen. The engine now also **refuses fullscreen**
under `-unattended`, `-inputcmd` or `-noinputgrab`, at startup (`UnGame.cpp`) and on every runtime
resize (`FWindowsViewport::Resize` — Alt+Enter, Options → Graphics, `SetRes`), logging
`fullscreen refused`. **Do not drive the display-mode option in any run.**

**Input.** Nothing you run may move the user's real mouse, press their real keys, or take window
focus. Drive the game only through `resources/tools/game_input.py` (`launch`, then `hover X Y`,
`click`, `key NAME`, `wait S`, `shot TAG`, fractions of the viewport). `drive_input.py` is retired and
refuses to run. Never call `SendInput`, `SetCursorPos`, `SetForegroundWindow`, `AttachThreadInput`,
`BringWindowToTop` or `SetFocus`; never click through a dialog. Under `-inputcmd` the window is never
activated and the real mouse never read; the game logs `DISHONORED(inputcmd): t=<world time> <cmd>`
and `idle` — correlate against those.

**Builds.** Every build goes through `resources/tools/build_queued.py` — never `cmake --build` or
`ninja` directly, not in your own scripts either. It queues one build at a time machine-wide and pins
each to half the cores. `build-release.cmd`, `build-play.cmd`, `run_regression.py`'s in-harness build
and `verify_phase2.py` already use it. **A waiting build is not a hung build**: look for
`build_queued: waiting for another build to finish`.

**And the build count is cut, because last wave a package waited longer than it worked.** Each package
now does **two** builds, not four: its own working build, and the gate's in-harness build. **Drop the
separate "clean full build, directory deleted first"** — the gate's build on a clean checkout is that.
**Drop the per-agent HEAD control build** — the coordinator provides one shared control at
`build/head_rel` (HEAD `80cc219`, built before launch; the exe is
`build/head_rel/Binaries/Win32/DishonoredGame.exe`). Measure before/after against that.

## The gate

`run_regression.py` is **47 checks**, and HEAD reads **47 ok, 0 failed, 0 skipped**. **Every
package's acceptance is 47/47.** Every `*_criticals` row reads 0 or the merge waits. If a row goes red
on your tree and you believe it is not yours, say which row, show it green on the shared control, and
say so — do not relax a bound.

Two harness facts: `--newgame-at` is 50 (FK measured `-finewgame` on the world clock landing inside an
async `LoadMap` at 22); and `CRITICAL_RE` matches the structured-exception banner, so a crash in a
stage is a red row, not an invisible one.

## Packages

### FR — the dialog system's direct path: a `DisConv_Soiree` node starts its matinee

The user-visible goal: **New Game → the boat → the Empress scene plays on its own.** FO's `agentFO.md`
§1 and §9 are the specification, in call order:

1. `UDisSeqAct_DialogInputs::Activated` (2013 rva `0x79bf00`, 578 B) — the one that already fires on
   our route (`SOIREE_Boat.DisSeqAct_DialogInputs_10`, input `Intro`) and dies on a comment-only
   skeleton.
2. `IDisConvSpeakerInterface::GetHookParameters` / `FireDialogHook` (`0x8a7360`) →
   `UDisConversationComponent::FireDialogHook` / `FireDialogHookNow`.
3. `UDisDialogTree_InGameBind::FireDialogHook` (`0x8a5150`, 602 B) / `EvaluateDialogHook`
   (`0x8a4690`, 479 B).
4. `FDisDialogRunningInstance::StartInstance` (`0x88bda0`) / `StartDialog`.
5. `UDisConv_Soiree::PlayNode_Derived` (`0x8a7a60`) → `PlayNodeMatinee` → **`FindMatinee`
   (`0x8b5de0`, 328 B)** → **`USeqAct_Interp::StartFromDialog` (`0x215020`, 77 B)**.

**Port the direct path (~10 KB) and prove it with one measurement**: `SeqAct_Interp_1` (the boat
ride) and then `SeqAct_Interp_12` (the Empress) go from `activations 0` to activated on a
`-finewgame` route, with the `-fosoiree` census and `-fkdiag` showing the groups instancing. **Do not
port the other ~50 KB** — the dialog *UI*, choices, subtitles, voice, the `UDisDialogTree` editor
surface — unless the direct path demands a piece; inventory what you leave with sizes and addresses.
The 1,053-placeholder mistake is in this tree's history and a 66-unit system is where it would recur.

Expect FN's and FO's warnings when the scene starts: `MAT_BeginAIGroup` is guarded and logs, and
every AI group will say so. That is FS's. **Coordinate with FS** on who touches
`UInterpGroupInstAI::InitGroupInst` / `InitPawn` (FS does).

### FS — the seam that moves a pawn, and the node retail actually animates

FO's §5–§7, in FO's order:

1. **The `UpdateTrackInst` seam is a five-pass loop, not a virtual.** Retail
   `UInterpGroupInst::UpdateGroupInst` (2012 `0x25b4e0`, 858 B) runs five passes on
   `UInterpTrack::TrackUpdatePass` (a byte this tree already declares at +105), with a 0.1 s re-step
   of passes 2–3 plus `FArkComponentLocomotion::MovePawn` in skip mode; the base
   `UInterpTrackInst::UpdateTrackInst` (2013 `0x212570`, 36 B) is `Track->UpdateTrack`. Build the
   pass loop in `UInterpGroup::UpdateGroup`, the virtual beside `NeedsSynchronizing`, then the
   bodies — **`StretchAnimControl` first** (`0x543f40`, 512 B; self-contained at ~4.2 KB with
   `GetAnimForTime`, `CalcChannelIndex`, `FindAnimSequenceFromName`): it is **267 of the level's 694
   tracks and on all six player groups**.
2. **`UArkAnimNodeStack`** (2012 `0x576570` `MAT_SetAnimPosition`, 536 B) — what retail's
   `APawn::MAT_SetAnimPosition` calls. Ours calls `UAnimNodeSlot`'s and `UArkAnimNodeStack` here is a
   `UAnimNodeBlendBase` with **zero bodies**; without it the seam animates nothing on any Dishonored
   pawn. ~1 KB, belongs with the seam.
3. **`ADishonoredNPCPawn::MAT_BeginAIGroup`** (2013 `0x765080`, 720 B) and retail's `InitPawn`
   (`0x22bbc0`, 373 B) — replacing FN's `FindFunction` guard with the native call plus the
   Gesture/Posture/Distraction/Face anim-set call. Then the rest of the NPC `MAT_*` (22) as the
   Boat/Empress scenes reach them, **and only as they reach them**.
4. `Locomotion` (five unported helpers; the component exists), `FaceTo` (`FArkComponentFaceTo`, 0
   bodies), `Dialog` (FR's). `LookAt` is FT's.

**Proof**: the Boat matinee's AI actor (FN's spawned Elite guard) and the player group's pawn **move
and animate** through the scene — `-fkdiag blend` already reports the player blend; add the same for
the AI group's pawn. A side-by-side against retail at the same world time if you can reach it.

### FT — the look-at component, so a scene's pawns look at things

Agent FL levelled the heads (wave 19) and said plainly the look-at **magnitude** cannot be proven until
`FArkComponentLookat` exists. **51 functions, inventoried with 2013 rvas in `agentFL.md` §6**;
`UArkComponentLookatConfig` 5. **Port what the Boat and Empress scenes and a standing guard reach;
inventory the rest.** FL's trap: when `m_pConfig` becomes non-NULL, `UpdateNodeWeights` fed a raw
(0,0) puts **1.00 on the grid centre, worth 60° of head** — `-flheaddiag` prints the assertion to
keep. Measure with FL's two-delta diagnostic (tree output vs renderer input, per bone) before and
after, and against retail with dismod if you can.

Also yours: **`UDisSkelControl_SpineBender`** (9 functions, 0 written, `CalculateNewBoneTransforms`
2012 `0x6daf30`), live on every NPC at `spine_0_jnt` with `ControlStrength` 1.00 and a silent no-op
today.

### FU — the mouse cursor in the mission, and the menu's remaining keys

The small UI package, because the user's standing instruction is still the interface:

1. **The mouse cursor stays drawn in the mission.** `m_bMouseCursorAllowed` is computed by
   `RefreshGlobalUIState` (FP) but the GFxUI cursor glue cannot read it — one seam, named by FP.
   Retail's `ConditionalHideMouseCursor` is 2013 `0x7945b0`.
2. **Enter does not activate a main-menu row** (STATUS defect 5). SPACE works and runs
   `TransitionTo(MainMenuScreen)`; four Enters produce **no AS2 trace at all** (FI). Find where the key
   is dropped — `FGFxEngine::InputKey`'s key map, the movie's `Key.isDown` listeners (FF counted 114
   sites), or the focus the probe reports as `focus NULL | canFocus 0 | canInput 0` (FB).
3. **The play-defaults table still appends the now-inert `-gfxuimenu`** (`Core/Src/UnMisc.cpp`,
   `DISHONORED_PLAY_DEFAULTS`). Remove it, and re-read the README's "Running it" against what the
   table now does.
4. If room remains: the 2 GB address-space exhaustion after ~3.5 minutes on the menu — open it as
   "what allocates per frame at the menu", **not** from the thirteen fake frames STATUS warns about.

**Proof for 1 and 2**: in-process captures and the AS2 trace, driven by `game_input.py`
(`key Enter`, `hover`, `shot`).

## Rules for agents

- **The two absolute rules above.** Input only through `game_input.py`; builds only through
  `build_queued.py`; two builds per package, the shared control at `build/head_rel`.
- Work **only** in your own worktree. Your sync script runs **worktree → main**, **flags rather than
  copies** anything changed in main since your base, and **names every function you touch per file**.
  **Before you hand back, run `git diff --name-only <base>` and `git ls-files --others
  --exclude-standard` in your worktree and check every file is in your sync list** — FP's missed a
  header and the merge failed to compile on a clean checkout.
- **Open the minidump before rebuilding anything** (`DishonoredGame/Logs/unreal-v9411-*.dmp`;
  `build/agentFN/tools/mdmp.py` reads it, and **the dump carries the module base** — do not subtract
  `0x400000`, do not recover ASLR). A two-identical-frame stack means the walker failed.
- **Prefer a measurement in the real game over an argument about it.** `dismod_harness.md`.
- **Resolve every retail address yourself.** `rva_sweep.py` is a lint: it passed VAs for RVAs in both
  directions and does not scan banner comment blocks. `vtables.csv` is the 2012 table;
  `match_2012_2013.csv` has propagated a wrong vtable; **skeleton comments carry 2012 rvas**;
  `shipping2012_v1.i64` has imagebase 0, `retail2013_named.i64` is `0x400000`.
- **A unit taken off `Sources.cmake` needs an external-linkage anchor** or MSVC drops the object.
- **A feature measured only in an instrumented run is not a feature the user has.** Check your claim
  against the staged shipped exe with the play defaults. The bars were "working" for a wave.
- **Never use the FModel MCP tools.**
- `run_regression.py` from **the worktree's own copy** with an **absolute** build dir, built inside
  the harness; **47 checks, 47 ok**. Copy the five gitignored `resources/docs/types/` inputs in first.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** In wave 20 every
  package corrected five to seven premises of its brief, and the two biggest findings were in nobody's
  brief.

## Acceptance, common to all four

1. For anything claimed fixed: a before/after against the shared control at `build/head_rel` through
   the same `game_input.py` script, and a side-by-side against retail where the fault is visual.
2. `run_regression.py`: **47 ok, 0 failed, 0 skipped**.
3. 0 errors, 0 C4263, 0 C4264 in the gate's own build log (the gate's clean-checkout build is the
   clean build).
4. Every address cited resolved by hand with its function start and size in both databases.
5. `resources/docs/agents/agent<XX>.md` and `agent<XX>_status.csv`, written **inside the worktree**.
6. `build/agent<XX>_sync.py`, naming every file and every function — **checked against `git diff
   --name-only <base>` before hand-back.**
