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

**The retail game.** Do not launch the real `Dishonored.exe` (a dismod run) while the user is at the
machine. It is the one executable we cannot patch: it uses the user's own display settings and
activates its own window, and a 15-second probe is 15 seconds of the user's foreground. Agent FT ran
one on a line in its brief — that line was wrong and is withdrawn. Use agent FB's existing retail
captures in `build/agentFB/retail/` and the shared control; write any measurement that genuinely
needs retail into your report as *not taken* and why, so it can be run when the machine is free.

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

## Outcome

### Coordinator — `e60c5c6`: a scripted run can no longer go fullscreen, take the foreground, or read the gamepad

Three faults the user hit while this wave ran on the machine they were using, all closed under the one
condition `DishonoredInputGrabDisabled()`, all measured before and after.

- **Fullscreen.** `game_input.py launch` did not pass `-windowed`; the retail ini says `Fullscreen=True`,
  so an agent's plain release build took the display. The launcher adds `-windowed`; the engine refuses
  fullscreen in `UGameEngine::Init` and on every `FWindowsViewport::Resize`, logging `fullscreen
  refused`. Verified on the ini-fullscreen path at 5.5 s.
- **Focus.** The no-grab mode showed the window `SW_SHOWNOACTIVATE` and lost three times over, found
  by **sampling the foreground every 3 s**: the splash (`SW_SHOW`, t=3 s), the `SetWindowPos` in
  `Resize` on the first map load (no `SWP_NOACTIVATE`, t=33 s), and an agent's run taking it from ours
  3 s later. Fixed: splash `SW_SHOWNOACTIVATE`, `SWP_NOACTIVATE` in `Resize`, no-activate `SC_RESTORE`,
  no `SetFocus` on a mouse message, `D3DCREATE_NOWINDOWCHANGES` on the D3D9 device. **After: 0 of 50
  foreground samples across three 60–90 s runs were the game**, with the user browsing throughout.
- **Gamepad.** The joystick poll in `ProcessInput` was gated on `HasFocus()` alone; once the window
  had focus, the user's controller drove the run (FU's Options stage quit at 15 s by
  `XboxTypeS_DPad_Left` → QUIT GAME). Poll and force-feedback now also gated on the scripted-run
  condition. **0 `XboxTypeS_` keys in 60 s with the controller attached.**
- **Residual, named as unexplained**: `WM_ACTIVATE` now logs every activation with its cause, and each
  run still records one `WA_ACTIVE` at the first rendered frame — with `-nosplash`, with the D3D flag
  confirmed applied (`0x00000952`), with nothing in the log at that instant. It never coincides with a
  foreground change: thread-local activation, which does not take the user's focus. Source not found.

### FU — merged `14cad0f`; FU's own gate 37/4 of 41 (a live gamepad — see `e60c5c6`), re-gated below

Eighteen files; sync list checked against `git diff --name-only d7d083a` + untracked: identical.

**The mouse cursor is gone from the mission, in retail's shape.** `UDisGFxMoviePlayerGlobal::PreRender`
(2013 `0x79eb90`), `ConditionalShowMouseCursor` (`0x78c790`), `ConditionalHideMouseCursor` (`0x7945b0`),
`MoveMouseCursor` (`0x78c8c0`), 2013's `PostStart`/`PreClose`/`PreCommitMapChange`,
`ComputeMovieSpaceInfo` (`0x787860`); `m_pMouseCursor` at +444; `UDishonoredViewportClient::Draw` calls
`PreRender` then the engine's `Draw` (retail `0x5ea330`). GFxUI's cursor glue deleted; one seam
`GGFxMoviePlayerPreCloseHook`. **Where FP's premise was wrong**: `m_bMouseCursorAllowed` read 0 on the
**menu** too — retail's `RefreshGlobalUIState` reads `FGFxEngine+40`, the per-player **FocusMovies**
array (`ReevaluateFocus` `0x586b60`), not `OpenMovies(0)`, which `InsertMovieIntoList` (`0x585350`)
keeps lowest-priority-first. "One seam" was two. Menu now `allowed 1 | clip attached`, mission
`allowed 0 NOT attached`.

**Enter was never broken — STATUS defect 5 does not reproduce.** Through `game_input.py` on HEAD,
`key Enter` on NEW GAME → `APressed () called` → `TransitionTo (NewGameScreen)`; `Down` moves
NORMAL→HARD; `Escape` → `BPressed`. Agent FI's four silent Enters were measured with the retired
`drive_input.py` against the `-gfxuimenu` bring-up movie (`canInput 0`) that FP has since replaced.
**Still open, brightness only**: Left/Right/Up/Down reach the movie — the gamma widget's own
`LeftPressed` is invoked — and the slider does not move (marker x 596..782 before and after).

**The inert `-gfxuimenu`** removed from `DISHONORED_PLAY_DEFAULTS`; README, `build-play.cmd`'s header
(which also still claimed `-startmap=Dishonored_MainMenu`), `dismod_harness.md` and a harness comment
corrected.

**The 2 GB: the AS2 machine never frees an object.** Private bytes +0.63 MB/s on the idle menu, flat
with `-nogfxui`; the new census reads `AS2 objects 66471 live / 66471 created` at 30 s and
`644869 / 644869` at 150 s — ~4,800 objects/s, **live always equal to created**, because
`GASObjectCollector` (`GFxAS2Object.h:171`) is the teardown half of retail's `GASRefCountCollector`
only. At the user's 216 fps that is 2 GB in 217 s. **GFx3 runtime work; next wave.**

**Where the brief was wrong**: defect 5 does not reproduce; FP's cursor bit was wrong, not just
unreadable; 2013's `PostStart`/`PreClose` differ from the 2012 skeleton comments; "Up/Down consumed"
holds for brightness only; the 2 GB is the AS2 heap, not the renderer; `build/head_rel` was not built
when the package needed a control.

**Gate on main for `14cad0f` (FU on top of `e60c5c6`): 47 ok / 0 failed / 0 skipped (2545 s).** The 37/41 FU measured on its own was the controller, not the code.

### FS — merged `d24f1bf`; FS's own gate **47 ok / 0 failed / 0 skipped** (2,654 s)

Twenty-one files; sync list checked against `git diff --name-only d7d083a` + untracked, every file in
the copy list or named as merged-from-main.

**Do pawns move and animate through the Boat scene? Half, measured.** Control `build/head_rel`
(`FSBASE3.log`) vs this (`FS5.log`) on the inputtest route: the spawned Elite guard's
`MatineeStackNode` now plays **`BoatDriver_SC_Tower_Idle` at weight 1.00**, sequence time advancing and
looping for the whole 58 s scene; on the control nothing drove it. The rats animate too. **He does not
move**: `base none`, `vel 0`, position constant while the boat sails off on its Move track — retail
carries him by **basing him on the boat**, and our spawned pawn has no Base (spawner/basing path, spec
§6.1). The Boat matinee has no player group; the player groups are on the soirée matinees FR's dialog
system starts, so the player half was not measurable this wave.

**The finding in nobody's brief, and the biggest: `UInterpTrack`'s bitfield was read in the wrong
order.** `UnInterpolation.h` declared it in the reference engine's order, but a native class keeps the
bit masks **serialized from the cooked script class** (`UStruct::Link`, `SavedBitMask`), so C++
`bDisableTrack` read retail's `bAIGroupOnly` and `bIsAnimControlTrack` read `bPlayerGroupOnly`. Every
non-player `StretchAnimControl` group said "no anim-control track" (`FS4.log`): no anim sets, no
weights. Re-declared in retail's order with the seven missing bits. **Every other NOEXPORT class
declared in reference order can carry the same fault — a scripted audit is specified in `agentFS.md`
§6.2 and is wave 22's.**

**Ported, in FO's order.** The five-pass `TrackUpdatePass` loop in `UInterpGroup::UpdateGroup` (retail
`UpdateGroupInst` 2012 `0x25b4e0`, 2013 `0x22fed0`), new virtual `UInterpTrackInst::UpdateTrackInst`,
the 0.1 s skip-mode re-step, `UpdateAnimWeights` for Stretch tracks, `GetGroupPawn` /
`SetActorRootMotion` / `SetActorCollision` / `SetMeshTranslationMode`. **The whole `StretchAnimControl`
runtime, 18 functions** (`UpdateTrackInst` `0x543f40`, `GetAnimForTime` `0x543a00` — the soirée-marker
stretching and looping). **`UArkAnimNodeStack`'s five bodies, moved into the Engine** because retail's
`APawn::SlotNodes` is `TArray<UArkAnimNodeStack*>` where this tree had `UAnimNodeSlot*` and the array
was empty on every Dishonored pawn; `CacheAnimNodes`, `MAT_SetAnimPosition/Weights/FinishAnimControl`,
`PostInitAnimTree`, `UpdateNodeSlot`. `UInterpGroupInstAI::InitPawn` (2013 `0x22bbc0`) replacing FN's
guard; the `MAT_*` contract as virtuals with **16 Dishonored pawn bodies** — `MAT_BeginAIGroup` /
`MAT_FinishAIGroup` partial, eleven callees absent (LOD manager, mesh offset, blend-pose node,
`MatineeControllerAdd`, the soirée AI stimulus, look-at).

**Not reached, specified**: the pawn's Base on the boat; `Locomotion`, `FaceTo` (component 0 bodies),
`Dialog` (FR), `LookAt` (FT, call site marked); the other fourteen NPC `MAT_*` and the player five; the
2013 refinements of the seam; the stack's child-0 weight (a flagged deviation).

**Six premises corrected**: the bitfield; the stack's "zero bodies" was a third of the hole (wrong
`SlotNodes` type, nothing caching the slots); **the Boat matinee does not start on `-finewgame=50`**
(control `activations 0` at 60/155 s) — only `-distouchprobe`'s teleports fire `StartSpawn`; "vtable
slot 74" is 2012's, 2013's seam is slot 76 (`0x505c70`); `MAT_BeginAIGroup` cannot yet be "the native
call" — a third of its body exists; `UInterpTrackInst::Track` (Arkane's +56) was never set by the
reference instances and the first seam call crashed on it — **minidump opened**, fixed.

**Rules**: all runs through `game_input.py`, FocusLost 0, no retail exe. **One run went fullscreen —
`FSBASE1`, the first control run, before the rule existed**; every launch after passes `-windowed`.
**Three generated headers hand-edited and the `UArkAnimNodeStack` shim block removed from
`DishonoredGameEngineShims.h` — the generator must learn this or a regeneration reintroduces the class
and the build fails loudly (§6.10).**

**Gate on main for `d24f1bf`: 47 ok / 0 failed / 0 skipped (2145 s, one load-sensitive retry of `d3d9_frames` with sixteen compilers running).**

### FR — merged `2ff7daa` (+ `d1f282c`); FR's own gate 46 / 1, the one row decided below

Seventy-nine files, 32 units; the sync script's own cross-check against the worktree's changed files
reports none unlisted. Addresses hand-resolved in both databases; decompiles in `build/agentFR/`.

**The boat ride starts on its own — the first Dishonored scripted scene to do so.** On `-finewgame=50`
with no input, three runs in a row: `SOIREE_Boat.DisSeqAct_DialogInputs_10 "Intro"` → hook →
component → `DisDialogTree_InGameBind` → running instance → `DisConversation_3.DisConv_Soiree_1` →
`USeqAct_Interp::StartFromDialog` → **`SOIREE_Boat.SeqAct_Interp_1` 0 → 1 activations, plays 94.23 s
and ends through the node**; its Completed reaches `Boat_LiftedUp` and `SOIREE_Workers.SeqAct_Interp_3`
goes 0 → 1. Control `build/head_rel`: both 0. Captures in-process: the control at the starting pier,
this build **inside the water-lock hall at the end of the ride**. No Critical, no minidump. What
animates during the ride is FS's runtime (AI groups bind, 0 `MAT_BeginAIGroup` warnings; the player
`MAT_*` warn once each; the boat moves by its Move track, skeletal props do not animate).

**The Empress does not start, and it is not the conversation system.** `SeqAct_Interp_12` stays at 0 at
330 s. `-foupstream` over all ten `SOIREE_Empress_Regent` `DialogInputs`: their entries are
`SeqEvent_RemoteEvent_34 (enabled 0) → Gate_14 → DialogInputs_8`, `RemoteEvent_86/87 → Gate_1 →
DialogInputs_1`, and **`DisSeqAct_AIGoToActor_13 "Success (Last)" → DialogInputs_25 "3Days_Forced"`** —
the captain at the dock. Between the lock and the Empress retail needs **the player to walk up through
the captain and guard scenes**; unattended nothing walks, and the AI-goto/trigger half is another
package. Retail-only questions recorded as not taken.

**Ported**: `IDisConvSpeakerInterface` (30-slot CppText), `IDisConvOneShotOwnerInterface`,
`ADisDialogOneShot`, `UDisDialogVoiceData`, `UDisDialogTree` (table-driven `InitHookInfo`, **126** rows
from 2013 `0x88d820`), `UDisDialogTree_InGameBind` + `FDisDialogRunningInstance`,
`UDisConversationComponent` (2013 shape), **`UDisConvGlobalMan`** (queue, 13 fire groups, ticked from a
new `ADishonoredGameInfo::Tick`), `UDisConversation`/`_InGameData`, `UDisConv_Node`/`_Soiree`
(`FindMatinee` by GUID, `TickNodeMatinee`), the hooks, speakers, actions, conditions, save data,
`UDisSeqAct_DialogInputs::Activated`, `StartFromDialog`/`ClearDialogConvNode`, `-frdiag`. CSV: **135
ported / 13 partial / 148 inventoried / 68 editor-only.**

**Where the brief was wrong**: the direct path is **~45 KB, not 10**; `DialogInputs::Activated` fired
with a NULL target because **`ADishonoredSpawner::OnSpawned` never raised `DisSeqEvent_Spawned`** —
retail does (2012 `0x6a1010`) — `FireSpawnedKismetEvent` added; 2013 moved `FireDialogHook`'s refire
checks (the plan's addresses were 2012 shapes); the recorded `0x8b2e80` is a `TArray` helper; 126 hook
rows not 122; `UDisConversation`'s flag bits shifted; retail `UDisDialogTree::PostLoad` calls
`InitNode(NULL,0,0)` — `m_iConvNodeIndex` is not serialized, and without it every node had no in-game
data; a relative `--cmd-file` is never read (the game's cwd differs) — **`game_input.py` should
absolutise it.**

**The one red row and the decision.** FR's gate: `unported_natives 2` —
`ADishonoredPawn::execOnSetDisposition` and `ADishonoredNPCController::execOnAIGoToActor` now fire at
7 s because the faithful Spawned event reaches the spawner's Kismet. They need the AI go-to package and
the relationship-override API (`OnSetDispositionHelper` 858 B, `FDisRelationshipOverrideInfo`,
`UDisGlobalFactionManager::ClearIndividualCache`, four unnamed `UDisTweaks_Faction` functions), all
bodiless; rvas in `agentFR.md` §8. **Coordinator's decision (`d1f282c`): the bound moves to 2 with
both natives named in the baseline, a third distinct native is a regression, and porting the two is
wave 22's first item.** Reverting the Spawned event would put the boat back to 0 activations; holding
the first self-starting cutscene behind two other packages was the worse trade.

**Gate on main for `d1f282c` (FR + the named baseline): 47 ok / 0 failed / 0 skipped (1317 s)** - `unported_natives 2 (wanted <= 2)`, both named.

### FT — merged `c1c4945`; FT's own gate **47 ok / 0 failed / 0 skipped** (1311 s)

Twenty-two files (15 changed, 7 new); every file differing from main HEAD confirmed on the sync list.

**`FArkComponentLookat` exists and runs.** Created on every NPC in `ADishonoredNPCPawn::Spawned`,
configured from `UDisTweaks_NPCPawn::m_pComponentLookatConfig`; `Starting()` finds `camera_jnt`,
`head_jnt`, both eyes, the eye controls, the blink blender, the look-at blender and the spine bender
on every pawn. `FDisLookAtRequest` now calls the six retail `Start*` entry points, `StopLookAt` and
`GetTimeLeft` on the real component instead of counting a desire it could not execute.

**Heads aim 1:1.** FL's two-delta diagnostic against `build/head_rel`: scene pawns at rest sit in
retail's two-centre state, `head_jnt` l1/c1 vs the control's l2/c2 — level, and **FL's 60° trap does
not bite**. A civilian given the AI's procedural patrol look-at: **requested 17.8° → composed 17°;
26.8° → 26°**; the control reads 3° and never moves. The retail magnitude comparison was withdrawn
(no retail launches while the user is at the machine) and is recorded as not taken.

**The big finding, from an unexplained frame — a coat drawn over the boat view, a canopy over the
guards: every NPC bone's local scale was 2.000**, 2^depth at the fingers, bounds radius ~12,000. The
look-at's `UpdateBlink` fades the Blink channel of `BlinkBlender` to weight 1 over a Source already at
weight 1; Blink is a non-additive eyelid-only anim, and this tree's stock
`UAnimNodeBlendBase::GetBoneAtoms` **sums** its children — two full poses, scale 1 + 1. Retail writes
the same weights, but **retail PC never evaluates skeletons through `GetBoneAtoms`**:
`UpdateSkelPoseBegin/UpdateSkelPose/End` (2013 `0x754510`/`0x754f1b`/`0x747239`) go through
`GEdgeAnimManager`, and the only reader of `m_bDontUseRefPoseOnNonDefinedBones` in either binary is
`BuildEdgeAnimTreeLeaf` (2013 `0x59871f`) — the Edge leaf masks bones a channel does not define.
Two Engine functions: **`UAnimNodeBlendBase::ChildrenHasChanged`** (retail's last vtable slot, 2012
`0x19f260`, called at the end of `UAnimNodeSequence::SetAnim`; this tree's `SetAnim` lacked it), and
**`UArkAnimNodeStack::GetBoneAtoms` — NOT a retail function**, the one deliberate deviation,
documented: Source is the base, each channel lerped over it on the bones its sequence defines,
additive channels accumulating. After: zero `flhead SUSPECT` lines on both routes, frames normal,
heads still turn.

**Ported**: `FArkComponentLookat` 36/51 bodies (the rest macros and the request template);
`UArkAnimNodeLookAt`'s config half (`SetConfig`, `CheckAnimsUpToDate` — **the anim selector, not editor
code** — `ComputeHeadAndTorsoAim`, `BlendInfos`, `HandleBlendingBetweenLookatNodes`, the config
branch of `TickAnim`, the `GetBoneAtoms` crossfade); `UArkComponentLookatConfig::Init/
UpdateProceduralLookAt`; `UDisSkelControl_SpineBender` (6 bodies, off the exclude list — **still a
no-op in effect**: its driver `Tick_SpineBending` is under the unported `ADishonoredPawn::Tick_Body`).
Inventoried with rvas: the component manager, the NPC Tick's `SetLookAtMode(ChooseLookAt())`,
`Tick_SpineBending` + helpers, `DisNotify_NPCDisableLookAt` (needs `Notify_EndRange` in the Engine
first), `FArkComponentFaceTo`, the track and `MAT_` callers.

**Where the brief was wrong**: FL's trap did not bite; `CheckAnimsUpToDate` is the pose selector;
`OnComposeSkeleton` is eyes-only and only while FaceFX plays (compiled out here);
`DisNotify_NPCDisableLookAt` cannot land without `Notify_EndRange`; `SetConfig` is the ICF-folded
`OverwriteExactRotation` at 2012 `0x58df40`; `Starting`/`Stopping` are 2013 `0x556660`/`0x556a00`
by vtable; the retail comparison was withdrawn. FT's dismod probe files remain in the user's dismod
repository (`src/mods/ftlookat.*`), inert unless `DISMOD_FTLOOKAT=1`.

## Wave result

**Five of five merged, each gated 47 ok / 0 failed / 0 skipped on a clean checkout of main**
(FU re-gated on top of the coordinator's guards; FR with one named row). Plus four coordinator commits
the user asked for during the wave: `e60c5c6` (no fullscreen, no focus theft, no gamepad in a scripted
run), `edea361` (the overnight build override), `4258e09` (no retail launches at the machine),
`080b322` (absolute command-file path).

| | | gate |
|---|---|---|
| `14cad0f` FU | the cursor leaves the mission; Enter was never broken; the 2 GB is the AS2 heap | 47/47 |
| `d24f1bf` FS | the five-pass track seam; `UArkAnimNodeStack`; **`UInterpTrack`'s bitfield read in the wrong order** | 47/47 |
| `2ff7daa` FR | **the boat ride starts on its own** — the conversation system's direct path, 32 units | 47/47 (`d1f282c`, named row) |
| `c1c4945` FT | heads aim 1:1; **every NPC bone was at scale 2.0** | 47/47 |

**Of the user's asks**: the opening *boat ride* now starts and plays without being forced — the first
Dishonored scripted scene to do so; the Empress does **not**, and the reason is now exact: retail needs
the player to walk up through the captain and guard scenes, and nothing walks unattended (wave 22's
spine). The HUD, letterbox and cursor are where retail puts them. Heads are level and aim. The
gamepad, fullscreen and focus theft that cost the user their evening are fixed and measured.

**Found in nobody's brief this wave**: the `UInterpTrack` bitfield order (FS — and every NOEXPORT
class may share it); retail PC evaluating skeletons through EdgeAnim, which is why a stock
`GetBoneAtoms` doubled every bone (FT); the spawner never raising `DisSeqEvent_Spawned` (FR); the
cursor-allowed bit reading the wrong movie (FU); the AS2 machine never freeing an object (FU).
**FR corrected nine premises of its brief, FS six, FT seven, FU six.**

**Process, this wave**: a sync-list omission was caught by the clean-checkout gate (FP, wave 20) and
every package since was checked against `git diff --name-only <base>` before merge — FR's script now
does that check itself; four packages' worth of builds ran at half CPU behind one lock until the user
went to sleep; and three of the user's rules were written in the middle of the wave because the
no-grab mode had holes a 3-second foreground sampler found that one-shot checks had missed.

**Gate on main for `c1c4945` (the full wave-21 tree): 47 ok / 0 failed / 0 skipped (1479 s).**
