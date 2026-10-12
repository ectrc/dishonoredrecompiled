# Wave 22 — walking to the Empress, the two natives, the bitfield audit, and the AS2 leak

Wave 21 made the boat ride start on its own (FR), made the guard aboard animate through it (FS), and
took the HUD cursor out of the mission (FU). The Empress scene still does not start, and FR proved it
is **not** the conversation system: its ten entry points sit behind remote events, gates and
`DisSeqAct_AIGoToActor_13 "Success (Last)"` — the captain at the dock. **Retail needs the player to
walk up through the captain and guard scenes.** Unattended, nothing walks. That is this wave's spine.

## THE FOUR RULES — unchanged, with one overnight relaxation

**Fullscreen**: `-windowed` on every launch; the engine refuses fullscreen in a scripted run.
**Input**: nothing touches the real mouse, keyboard or focus — `game_input.py` only; the engine never
activates its window and never reads the gamepad in a scripted run. **Builds**: `build_queued.py`
only, two builds per package, shared control at `build/head_rel`. **Retail**: the user is asleep, so a
dismod run of the retail `Dishonored.exe` is permitted **this wave only, with `-windowed`, for a
measurement that cannot be taken any other way**, and must be written up as such; the standing rule
(no retail launches while the user is at the machine) returns in the morning and the coordinator will
say when.

The overnight build override is in place (full CPU, no queue, `%LOCALAPPDATA%\Temp\dishonored_build_overrides.txt`).
The display is off: captures are in-process and do not need it, but **check your first capture of
every run has real content and a real size**, and report a blank one rather than working around it.

## The gate

47 checks, **47 ok on HEAD**. `unported_natives` reads **2, bound 2**, both natives named in the
baseline (`ADishonoredPawn::execOnSetDisposition`, `ADishonoredNPCController::execOnAIGoToActor`) —
**FV takes the bound back to 0**. Every `*_criticals` 0 or the merge waits. A third distinct unported
native is a regression.

## Packages

### FV — the two natives, and the AI go-to that walks a pawn

The red-then-named row, and the first half of walking to the Empress. FR's `agentFR.md` §8 has the
rvas.

1. **`ADishonoredNPCController::execOnAIGoToActor`** and what it needs: `UDisSeqAct_AIGoToActor::
   GetDestinationActor` / `SetGoToStatus`, `FAIStimStruct_GoToRequest`, the brain's `m_Home`. This is
   the Kismet go-to that moves an NPC to a mark — the mechanism the captain-and-guards scenes are
   gated on (`DisSeqAct_AIGoToActor_13 "Success (Last)" → DialogInputs_25`). **Prove it with the
   guard FN spawned on the boat reaching his mark**, or the captain reaching the dock.
2. **`ADishonoredPawn::execOnSetDisposition`** and the relationship-override API:
   `OnSetDispositionHelper` (858 B), `FromUObjectOrController`, `FDisRelationshipOverrideInfo::*`,
   `UDisGlobalFactionManager::ClearIndividualCache`, four unnamed `UDisTweaks_Faction` functions —
   all bodiless. Port what `DisSeqAct_SetDisposition` on `DishonoredNPCPawn_24` actually reaches.
3. **Return `unported_natives` to `<= 0`** in `regression_baseline.json` once both fire with bodies.
4. **The pawn's Base on the boat** (FS §6.1): retail bases the spawned guard on the boat so the Move
   track carries him; ours has `base none`. Spawner/basing path.

### FW — the player walks: the Empress's entry chain, end to end

The second half. From the water-lock to `SOIREE_Empress_Regent.SeqAct_Interp_12`, retail's route is
the player walking through the captain scene and the guard scene, each a `DialogInputs` behind a gate
or an AI go-to. **Map the whole chain with FR's `-frdiag` and FO's `-foupstream`**, then drive it: a
`game_input.py` script that walks the player (W held via `key W down` / `up`, `move` for yaw) from
the lock up the stairs, with `shot`s at each scene, **against the control**. Where a scene needs an
NPC to arrive first, that is FV's go-to and you wait for it; say so. Where a `DialogInputs` fires and
its matinee does not start, that is yours.

Also yours, because it is in the same scenes: **the `SkeletalMeshActorMAT` props warn `no
BeginAnimControl`** (boat, wheel, drawbridge — FR) — the matinee-controlled skeletal props do not
animate. FS put the `MAT_*` contract on `APawn`/`AActor`; the `ASkeletalMeshActorMAT` half is the
reference engine's and is apparently not reached.

### FX — the NOEXPORT bitfield audit, and the generator

FS found that `UInterpTrack`'s bitfield was declared in the reference engine's order while the cooked
class serialises retail's (`UStruct::Link`, `SavedBitMask`), so C++ read `bAIGroupOnly` as
`bDisableTrack`. **Every other NOEXPORT class declared in reference order can carry the same fault.**
FS specified the audit in `agentFS.md` §6.2: for every native class with a bitfield, compare the
declared order against `retail_sdk_layout.json`'s bit masks and `script_classes_2013.json`, list every
mismatch, fix the ones on a reached path, and write the script into `resources/tools/` so it runs in
`verify_phase2.py`.

Second item, because three waves have now hand-edited generated headers: **`gen_classes_header.py`
must learn the edits** FS (§6.10: `UArkAnimNodeStack` out of `DishonoredGameEngineShims.h`), FR, FP
and FI made, or a regeneration reintroduces removed classes and drops the CppText virtuals. Run the
generator into a scratch directory, diff against the tree, and make the generator produce what the
tree has.

### FY — the AS2 machine never frees an object

FU measured it: `AS2 objects 66471 live / 66471 created` at 30 s, `644869 / 644869` at 150 s —
**~4,800 objects/s, live always equal to created** — because `GASObjectCollector`
(`GFxAS2Object.h:171`) is the teardown half of retail's `GASRefCountCollector` only. At the user's
216 fps that is the 2 GB in 217 s (STATUS defect 7). Port retail's collector — `GASRefCountCollector`
with its `AddToKillList` / `CollectRoots` / release cycle — and prove it with FU's census reading
`live` well below `created` and the private-bytes slope flat on the idle menu (FU's `fu_mem.py`).
**This package owns `External/GFx3`.** Also yours, smaller: the **brightness-screen keys** — Left/
Right/Up/Down reach the gamma widget's own `LeftPressed` and the slider does not move (marker x
596..782 before and after, FU); and the residual **thread-local `WA_ACTIVE` at the first frame** in a
scripted run, logged by `WinViewport.cpp` with its cause — find who calls `SetActiveWindow`/`SetFocus`
on our own thread (a `SetWindowsHookEx(WH_CBT)` for `HCBT_ACTIVATE`/`HCBT_SETFOCUS` in a scripted run
names the caller).

### FV also carries FT's remainder — the look-at is in; what drives it is not

FT landed `FArkComponentLookat` (36/51 bodies) and the heads aim 1:1 when *asked*. What asks, in
retail, is the NPC's own tick — **`ADishonoredNPCPawn::Tick → SetLookAtMode(ChooseLookAt())`**, and
`Tick_SpineBending` under the unported **`ADishonoredPawn::Tick_Body`**, which is why
`UDisSkelControl_SpineBender` has six bodies and is still a no-op in effect. Both are pawn-tick work
and belong with FV's AI go-to: port `Tick_Body` far enough that `ChooseLookAt` and `Tick_SpineBending`
run, and prove it with FT's `-flheaddiag` showing a guard turning his head toward the player **without
a scripted request**. `DisNotify_NPCDisableLookAt` needs `Notify_EndRange` in the Engine first (FT
§6); add it if a scene reaches it. **FT's one deliberate non-retail function,
`UArkAnimNodeStack::GetBoneAtoms`, stands in for EdgeAnim's per-bone masking — if FX's audit or any
package finds retail PC's `GEdgeAnimManager` path reachable, that is the faithful replacement and FT's
function goes.**

## Added mid-wave: three faults the user saw in the wave-21 build

> "In the animations, the NPCs don't move their body at all so therefore the other 2 guards on the
> boat just float. Their animations play mid air and same for other NPCs that I interact with. The FOV
> and forced perspective is broken too as it should change and limit the view angles. Lastly I can see
> my arms even when I shouldn't."

Investigated before writing this in. **The second and third are one fault**: the whole player
master-state FSM is comment-only skeletons — 27 units, `stateplayermaster*.cpp` and
`dissoireecontrolstate.cpp`, **0 code lines each** — and `UStatePlayerMasterSoiree` is the state
`ADishonoredPlayerPawn::MAT_BeginPlayerGroup` (2012 `0x70fcc0`) enters for a cutscene. In retail that
state is what forces the camera (the FOV change and the clamped view angles) and hides the first-person
arms and weapon; here `interpgroupplayer.cpp:131` warns once and does nothing. **The first is the body
half FS specified and did not reach**: the `Locomotion` track and `FArkComponentLocomotion::MovePawn`
(2012 `0x503c30`), `FArkComponentMeshOffset` (`Engine/Src/arkcomponentmeshoffset.cpp`, **0 code
lines**), and `ChangeAnimStateMotionModes` (unported, `dishonoredpawn.cpp:428`), so an NPC's animation
plays while its actor and mesh stay where they were — on the boat (whose Move track sails off without
them, no Base: FV's item 4) and anywhere else an animation is meant to move a body.

### FZ — the soirée player state: forced camera, clamped look, hidden arms

1. **`UStatePlayerMasterSoiree`** and what enters it: `ADishonoredPlayerPawn::MAT_BeginPlayerGroup`
   (2012 `0x70fcc0`), `UStatePlayerMasterInDialog::StartSoireeControl`,
   **`FDisSoireeControlState::EnterSoireeMode`** (2012 `0x69a4b0`, 525 B). FO's `agentFO.md` §6 read the
   player five (`MAT_BeginPlayerGroup`, `MAT_SetForceInventory`, `MAT_FinishPlayerGroup`,
   `MAT_BlendOut`, the fifth) with addresses; FK's `interpgroupplayer.cpp` has the warn-once stubs to
   replace. The base state machinery (`UStatePlayerMasterBase`, the owner that runs it) is also a
   skeleton — port the minimum that lets the Soirée state enter, tick and exit, and inventory the other
   24 states with sizes.
2. **The forced perspective**: what the state does to the camera — the FOV it sets, the pitch/yaw
   limits it applies, and how `UInterpTrackDisAllowPlayerLook` (FS: uses the seam, body absent) feeds
   it. Prove with `-fkdiag`/a new line logging FOV and the view-rotation limits during the boat ride
   against the control (90°, unlimited), and with in-process captures that show the forced framing.
3. **The arms and weapon hidden** while the state is active and shown again on `MAT_FinishPlayerGroup`
   / `BlendOut`. Find retail's call (the arms mesh component on `ADishonoredPlayerPawn`; nothing in this
   tree hides it — `grep` finds only `UDisDLC07SeqAct_SetPlayerArmsMesh`). Capture before/during/after.

Files: `DishonoredGame` player pawn, player controller, the state units, `interpgroupplayer.cpp`.
FV is in NPC pawns and the go-to; GA in locomotion; name every function. `dishonoredplayercontroller.cpp`
already carries FI, FL, FK, FT.

### GA — NPC bodies move with their animations

1. **The `Locomotion` track**: `UInterpTrackLocomotion` / its inst through FS's `UpdateTrackInst` seam,
   and **`FArkComponentLocomotion::MovePawn`** (2012 `0x503c30`) with its five unported helpers (FS §6;
   the component itself has 484 code lines and exists). This is how a matinee carries an AI pawn
   along a path.
2. **`FArkComponentMeshOffset`** — 0 bodies. `ADishonoredPawn`'s mesh-translation trio
   (`GetMeshOffsetComponent` / `Enable…`, `dishonoredpawn.cpp:432`) are no-ops because of it, and
   `UInterpGroupAI::UpdateGroup` calls `MAT_SetMesh…` into them (`dishonorednpcpawn.cpp:342`).
3. **`ChangeAnimStateMotionModes`** (2012 `0x78e4b0` / `0x78e4c0`; `dishonoredpawn.cpp:428`) so
   `MAT_SetRootMotionMode` and the pawn's own root-motion modes reach `Mesh->RootMotionMode` the way
   retail's do, and **root motion moves the actor** — the user's "NPCs I interact with" float because an
   animation with root translation plays on a pawn whose actor never follows it. `ADishonoredPawn::
   Tick_Body` (90 code lines, partial) is where the per-frame application lives; FV is porting the part
   of it that drives the look-at — **coordinate: FV owns `Tick_Body`'s look-at/spine branch, you own
   its motion branch, and you merge, not overwrite.**

Prove with position-over-time: the two guards on the boat **travel with the boat**, and a walking NPC's
actor **moves with its animation** rather than sliding or floating, against the control; in-process
captures at matching world times.

## Rules for agents

- **The daytime rules are back** (the user is at the machine): builds at half CPU, one at a time
  (the override file is gone); **no retail `Dishonored.exe` launches at all**. The overnight
  relaxation in "THE FOUR RULES" above is void until the coordinator says otherwise.
- The four rules above, and the two-build limit. **Before hand-back: `git diff --name-only <base>` +
  `git ls-files --others --exclude-standard source resources` against your sync list**; FR's script
  does the comparison itself (`NOT IN SYNC LIST but changed in the worktree`) — copy that pattern.
- **Open the minidump before rebuilding** (`build/agentFN/tools/mdmp.py`; the dump carries the base).
- **Resolve every retail address yourself**; `rva_sweep.py` is a lint. Skeleton comments carry 2012
  rvas; **2013 moved things** (FR: `FireDialogHook`'s refire checks; FS: two extra `UInterpTrackInst`
  virtuals, the seam at slot 76 not 74). `shipping2012_v1.i64` has imagebase 0.
- **A unit off `Sources.cmake` needs an external-linkage anchor.** **A bitfield declared in reference
  order may read the wrong bits** — FX's audit; check any class you add.
- **Never use the FModel MCP tools.** C4263/C4264 are errors. `DISHONORED_SHIM_STATIC` is
  `inline static`.
- `run_regression.py` from the worktree's copy, absolute build dir, in-harness: **47 ok**. The five
  gitignored `resources/docs/types/` inputs are in your worktree.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Wave 21: FR corrected
  nine premises, FS six, FU six.

## Acceptance

1. Before/after against `build/head_rel` through the same `game_input.py` script; side-by-side against
   retail where visual and where a windowed dismod run was taken.
2. `run_regression.py`: **47 ok, 0 failed, 0 skipped**; FV's lands `unported_natives` at `<= 0`.
3. 0 errors / 0 C4263 / 0 C4264 in the gate's own build log.
4. Every address cited resolved by hand, both databases.
5. `resources/docs/agents/agent<XX>.md` + `agent<XX>_status.csv` inside the worktree;
   `build/agent<XX>_sync.py` naming every file and function, self-checked against the diff.

## Outcome

### FX — merged `90d490e`; FX's own gate 47 ok / 0 failed / 0 skipped

Twenty files; sync self-checked against `git diff --name-only f14326b` + untracked: exact.

**Two real bitfield faults in the whole tree, both on `USkeletalMeshComponent`, both read on the path every
skeletal mesh takes.** `resources/tools/bitfield_audit.py` compares every `BITFIELD x:1` run (153 headers, 1,232
classes and script structs, 1,297 DWORD units) against the cooked class's masks. Eight units reported: five are
reference-engine `_DEPRECATED` renames at the same bit, one is a native struct whose cooked mirror retail's own
C++ does not follow (allow-listed with the measurement in `resources/docs/types/bitfield_audit_allow.txt`), and
two were real — C++ `bPauseAnims` sat on retail's `m_bDisableFaceFx` and shifted `m_bSkipUpdate` /
`bEnableLineCheckWithBounds` one bit up; `bUpdateComposeSkeletonPasses` / `bValidTemporarySavedAnimSets` read two
2013 bools this tree never declared on the component. Fixed in `UnSkeletalMesh.h`. **The audit is the gate's
third `verify_phase2` row** (`verify_phase2_total` 2 → 3; reads `mismatches=0 read=0 missing=3 allowed=1`).

**A regeneration diffs clean**: `gen_classes_header.py --sdk --module-header --sources-cmake` writes 100 files
byte-identical to the tree. It learned four waves of hand-edits through its own mechanisms (interface CppText,
`<Module>Native.h`, a class declared elsewhere is not a shim, link anchors from `DishonoredLink<X>()`
definitions, ported natives detected from exec bodies, hand-maintained `Sources.cmake` left alone, LF output).

**Premise corrected**: Engine headers are reference-engine files the generator never writes, so FP/FS/FT's
Engine edits could never be dropped by a regeneration; the real drift was smaller and in the `--sdk` modules.

### FW — merged `0e45ea1`; FW's own gate 47 ok / 0 failed / 0 skipped (2780 s)

Eleven files; sync exact; `EngineAnimClasses.h` hand-merged over FX's two lines (different region; Engine
headers are not generated, so FW's C++-only MAT declarations are safe).

**The props animate.** `ASkeletalMeshActorMAT` had no in-game anim-control path. Ported `PostInitAnimTree` (2012
`0x349d60`) → `CacheAnimNodes` (`0x343ee0` / 2013 `0x321e90`), `MAT_BeginAnimControl` (`0x343eb0` / `0x321e60`),
`MAT_FinishAnimControl` (`0x33e210` / `0x31ba30`), `UpdateAnimSetList` (`0x365540`) + `BuildAnimSetList` on the
MAT's own `InterpGroupList`. Control: 8 `no BeginAnimControl` warnings; FW1: 0, the boat, the rats and `MAT_0`
driven by `MatineeStackNode` at weight 1.

**The player stops 750 units short of the dock, and it is the boat matinee.** Rides the boat and the lift,
turns, walks, and stops dead at X 9994.4 / Y 23144 on every heading, five runs to the unit; left is water. The
ride boat ends at 9773, 23112; `Boat_Arrive` swaps in the moored collision boat (handlers found, 0 missing) and
that boat stands at 10305, 22565 — at the dock. The Boat group's Move track in the lift matinee
(`SOIREE_Workers.SeqAct_Interp_3` / `SOIREE_Boat.SeqAct_Interp_3`) ends short. **Measured, not fixed — wave 23's
first item.**

**Everything after the entries works** (`-fwteleport`): `DisTrigger_8` → guards' `SOIREE_Guards.SeqAct_Interp_3`
0 → 1; `DisTrigger_16` → `SOIREE_Empress_Regent.SeqAct_Interp_0` 0 → 1, finished at 16.06,
`Soiree_EmpressAndRegent_GoEmily` raised. After it the screen stays faded to black (director/fade — FK/FZ side).

**The chain** (`-fwdialogmap`): `SeqAct_Interp_12` is conversation 8 "Empress Death" — the assassination, the
LAST of nine Empress scenes, not the door. Route: lift → dock → `DisTrigger_8` (guards) → captain
(`AIGoToActor_3` at `Soiree_BoatRide_Completed`, FV, or `Use`) → `DisTrigger_16` → `Use` on the Empress
(`DDH_PLAYER_USED` → `DialogOutputs 'Give_Letter'`, **not ported**) → GiveLetter → ReadLetter → Regent
Interrupted → Holding Emily → teleporters → Empress Death.

**Premises corrected (eight rows in agentFW.md §2)**: Interp_12 is the end not the door; the guard scene is a
player touch; FS did not put `MAT_*` on `AActor`; FO's `-foupstream=A,B` never worked (`Parse()` stops at the
first comma — fixed); FR's `'Boat Start'` does fire at t 56.70; the player is never held during the ride (FZ);
the window was 1920x1080 with the user at the machine, so pixel turns scale to the capture.

### FY — merged `69bc81f`; FY's own gate 47 ok / 0 failed / 0 skipped (1625 s)

Sixteen files; sync self-checked against `git diff --name-only f14326b` + untracked: exact.

**The AS2 machine frees its objects (STATUS defect 7).** `GASRefCountCollector` in `External/GFx3`
is retail's `GRefCountCollector<323>` — Bacon-Rajan over the counts: `Collect` 2013 `0xda78b0`,
`Release` `0xda7b50`, `AdvanceFrame` `0xdd81a0` (at the tail of `GFxMovieRoot::Advance`), the five
functors as inlined in `GASObject::ForEachChild_GC<F>`, `ExecuteForEachChild_GC`/`Finalize_GC` on
Object, Array, Super, Primitive, Function and `GASValue`. Two documented deviations: the free at zero is
deferred to a kill list run at the tail of each advance (the interpreter holds raw `GASObject*` across
statements), and every newborn goes on that list (retail's creator holds a `GPtr`; ours start at 0).
**Measured on the idle menu**: control `68058 live / 68058 created` at 30 s → `1604634 / 1604634` at
325 s, private bytes **+0.77 MB/s**; this build **`4990 live / 1638260 created / 1633270 freed`** at
325 s, private bytes **712.7 → 712.2 MB — flat**, clean exit. Options drive: `5555 live / 865663
created`, 723 → 717 MB, flat. The census line now reads `live / created / freed (in cycles,
collections)`. One fault of FY's own found by its first run (a crash at 95 s — the sprite constructor
assigned `pProto` raw and the new `Release` zeroed it): minidump opened, fixed.

**The brightness keys were never broken — the brief's and STATUS's premise is wrong.** `LeftPressed` on
the gamma slider → `DisOnSettingChange … 2.1` → `gamma 2.1`; the thumb moves 461..492 → 456..487 →
452..483 on two Lefts and back on Right. FU's two captures bracketed Left+Right+Up+Down, which nets to
zero. Nothing to fix.

**The window activation is ours, and named.** A thread-local `WH_CBT` hook under the no-grab mode logs
`HCBT_ACTIVATE`/`HCBT_SETFOCUS` with a frame walk **and a raw stack scan** resolved to module+rva.
Every scripted run: one `HCBT_SETFOCUS` at the first frame, no `HCBT_ACTIVATE`, foreground unchanged.
The frame walk dies at `FEngineLoop::Tick+0x576`; the raw scan names **`appShowGameWindow+0x10a` under
`UGameEngine::Tick+0x16b6`** — `Core/Src/UnVcWin32.cpp`, `SetWindowPlacement(SW_SHOWNORMAL)` at the
first game tick. Fix: `SW_SHOWNOACTIVATE` under `DishonoredInputGrabDisabled()`, one line, **not
verified in a build by FY** (both builds spent before the scan named the caller) — the gate on
`69bc81f` is its first compile, and the coordinator verifies the activation line is gone with a
scripted run on the gate exe. The hook stays in.

**Five premises corrected**: the slider moves (a net-zero key pair was measured); the missing half
was the creator's reference as well as the cycle collector; the caller is `SetWindowPlacement`, which
neither `SetActiveWindow` nor `SetFocus` by name would have found and a frame walk cannot see; the
2 GB is per movie advance (16.7 objects per advance), not per render frame; the two-build limit cost
the Core line its compile.

**Coordinator's fix after FY's gate**: `69bc81f` failed its gate 43/47 — `Core.lib(UnVcWin32.cpp.obj) : error
LNK2019: unresolved external symbol DishonoredInputGrabDisabled` in `CoreSmoke.exe`, which links Core alone.
The function used only Core APIs, so its definition moved from `WinDrv/Src/WinViewport.cpp` to
`Core/Src/UnVcWin32.cpp` (declaration kept in WinDrv). The two-build rule is what left this line uncompiled;
a Core change needs a `CoreSmoke` link before hand-back.

**Verified on the `03f7579` gate exe** (gate 47 ok / 0 failed / 0 skipped, 2747 s): a 45 s scripted run through
`game_input.py` (`-inputcmd`, `-windowed`, the gate's inis) logs **0** `window activated`, **0** `HCBT_SETFOCUS`,
**0** `HCBT_ACTIVATE`; `input grab disabled` at 4.0 s, level loaded at 5.5 s, every command executed, clean exit.
STATUS's "one residual" WA_ACTIVE at the first frame is gone.

### FV — merged `02eb527`; gate 47 ok / 0 failed / 0 skipped (4787 s); `unported_natives` 0 ≤ 0

Forty-six files; 41 by sync, five hand-merged (3-way, 0 conflicts); `DishonoredGameItemClasses.h`,
`Sources.cmake` and `DishonoredGameRegistrants.cpp` then replaced by FX's generator output so a regeneration still
diffs clean (FV's three link anchors are derived from the `DishonoredLink<X>()` definitions). `bitfield_audit`
`mismatches=0`.

**Both natives ported with their chains.** Go-to: controller `OnAIGoToActor` → brain `m_Home` → Idle+GoTo stims →
`UDisBehaviorGoTo` (45 functions) → `PrepareGoTo` → locomotion desire → `SetGoToStatus` (2013 `0x7a1fc0`,
virtual) with Success/Failed/Interrupted outputs. Disposition: `IDisRelationshipInterface`'s six slots,
`OnSetDispositionHelper`, `FDisRelationshipOverrideInfo`, `UDisTweaks_Faction::OnSetDisposition`,
`UDisGlobalFactionManager` with `s_pInstance` published; `NPCPawn_25` gets one individual override, `_21` an
individual + faction override; `DetermineRelationship` answers Neutral past the cache (faction tables unported).

**The tick had no body.** `ADishonoredPawn::Tick` (2013 `0x750720`), `Tick_Body`, `ADishonoredNPCPawn::Tick`
(`0x76da40`), `Tick_Body` — the plan's "90 lines, partial" was wrong; every Dishonored tick phase was unported.
Now dispatched with `Tick_Body` gated on `m_bAllowTickBody`; `ADishonoredNPCPawn::Tick` runs
`SetLookAtMode( ChooseLookAt() )` every frame (`ChooseLookAt` whole, `GetCurrentTransitSpeed` `0x779430`,
`GetDefaultNPCLookAtMode` as CppText virtuals); `Tick_SpineBending` slope branch.

**Pawns get a Base.** Retail's `FullMovePawn` ground check ported as `FArkComponentLocomotion::UpdateFloorAndBase`:
11 of 26 level NPCs based (`WorldInfo_0` ×8, `SkeletalMeshActorMAT_3` ×1) against 0 on every earlier build.

**Measured, not fixed → GA / wave 23**: the go-to pawn moves 52 of 89 uu and stops — DN's `IsArrived()` returns
`m_bIsArrived` (default TRUE) and its only writer `HandleArrival` never runs because `ComputeDynamic` exits on
`IsDisabled() || STOP_END`; the 52 uu are the teleport onto the mesh, `speed 0/0`. The boat guard is still based on
`none` with `floor 0.00`: `physWalking → MovePawn` is never reached for a matinee AI-group pawn — retail steps it
from `UInterpGroupInst::UpdateGroupInst` (`DisableInertia`, `MovePawn(step)`); FS ported only the skip path. A
guard's look-at **target** is a `FDisLookAtRequest` from `UDisAISubProcessHeadTrackBase/Bark/Pause` (three
skeletons on the exclude list) — the mode alone cannot turn a head. `DisNotify_NPCDisableLookAt` fails at package
load (`Found bad AnimNotify`, 143 lines every run).

**Premises corrected (11 rows, agentFV.md §5)**: the tick bodies did not exist; "four unnamed `UDisTweaks_Faction`
functions" were 2 `FDisRelationshipOverrideInfo` + 2 `UDisGlobalFactionManager`; `SetGoToStatus` is `0x7a1fc0`
not `0x79a520`; `GetCurrentTransitSpeed` `0x779430` not `0x779850`; `UDisBehaviorGoTo_Base::IsBehaviorFinished`
is ICF-folded onto `UDisBehaviorShoot`'s; `EAIStimID_GoToRequest` is 41 (2012) / 42 (2013) and the 2013 DB
mislabels its ctor; the go-to pawn is `_25` here vs FR's `_24`.

### GA — merged `ad4a35d`; GA's own gate 47 ok / 0 failed / 0 skipped (4441 s)

Twenty files, 0 flagged against `02eb527` (GA rebased before hand-back); `bitfield_audit` 0; regeneration clean.

**The guards ride the boat (the user's "other 2 guards on the boat just float").** Control: Elite `NPCPawn_27` and
captain `_26` stay at the spawn, `base none`, for all 94 s of `SOIREE_Boat.SeqAct_Interp_1` while the player rides
off on `SkeletalMeshActorMAT_2`. This build: both `base SkeletalMeshActorMAT_2` within 6 s and ride to the lock
beside the player; 11 of 28 NPCs based vs 1. The cause was the floor sweep of retail's `FullMovePawn` (2012
`0x580ae0` +0x6a0 `SetBase(Hit.Actor, Hit.Normal, 1)`; +0x8a0 no floor → `PHYS_Falling`) that DN's `MoveActor`
port left out. FV's `UpdateFloorAndBase` kept; GA added the start-penetrating re-test (a plain sweep lifted the
Elite 30 u/frame) and the no-floor fall (Elite falls 47 units onto the deck at 69 s, lands, is based).

**Locomotion track ported** (ten `UInterpTrackInstLocomotion` bodies + helpers): Dock Worker B walks 850 units at
123 u/s to `TargetPoint_57` — body plays `Empty_Idle` (slides) because the walk animation is
`UArkAnimNodeLocomotion`'s, a comment-only skeleton → **wave 23**. Root motion was never the issue (`rmd 0.00`
in 3,000 lines; `ProcessRootMotion` present). **`FArkComponentMeshOffset` ported whole** (13/13, 136 B asserted,
created in new `ADishonoredPawn::PreBeginPlay` → `ADishonoredNPCPawn::PreBeginPlay_NativeComponents`): 11 of 28
NPCs stood 1–17 units above their floor and now have the mesh translated onto it.

**Premises corrected (six, agentGA.md §1)**: "`MovePawn` 2012 `0x503c30`" is the 2013
`UInterpTrackInstLocomotion::UpdateTrackInst`; `ChangeAnimStateMotionModes` is `0x78e550` (46 B — FS's two fields
were retail); `Tick_Body` has no motion branch; the floaters are an unbased pawn on a moving boat + spawn hovers +
a walker without a walk animation; the boat is `SkeletalMeshActorMAT_2` not `InterpActor_18`; FV's "`MovePawn`
never reached for a matinee pawn" is wrong — `MAT_SetCollision` forces `PHYS_Walking` every frame. Retail's
`FindValidLocation`/`MoveAttachedActors` snap must NOT be a plain Z move (it made a path query answer "arrived").
**Gate on `ad4a35d` (main)**: 46 ok / 1 failed — `unported_natives 1`: `ADishonoredPlayerPawn::execPlayDying_Native`
fired at 10.16 s of the d3d9 stage (the player died 3 s after spawning in `l_tower_p`, right before the first NPC
spawned). Re-run of the stage on the same exe five times: **0 deaths in 5**; GA's own gate on the identical tree: 0.
**1 in 7**; `execPlayDying_Native` appears in none of the 89 surviving d3d9 logs, but STATUS records the same
death once before - FO's gate on `15695d8`, wave 20 - so it predates wave 22; intermittent, cause not recoverable
(the harness overwrote the log before it was read). Not GA's `PHYS_Falling`
tail: `FArkComponentLocomotion::m_pPawnOwner` is `Cast<ADishonoredNPCPawn>`, the player never runs it; GA's
`ADishonoredPawn::PreBeginPlay` → `StartAllComponents` is a no-op for the player (only NPC files add components).
Candidates: FV's `ADishonoredPawn::Tick` (new body on the player too), FV's `UpdateFloorAndBase` (NPC-only too),
or load-order (the player is possessed about 1.2 s before the PhysX scene has the level's static shapes in every
run). **Open defect, wave 23 GE item 3**: the unported-native stub must log its parameters for `PlayDying_Native`
(killer, damage type, hit location) so the next occurrence names itself. Main is treated as green: the other 46
rows passed, and the one row is a 1-in-7 the same tree passed six times.


### FZ — merged `785e91d`; gate on main 47 ok / 0 failed / 0 skipped (9767 s wall, 2 h 40 of it in the build queue); FZ's own gate on the merged tree 47 / 0 / 0 (2393 s)

Fifty-three files, 0 flagged against `9f214c5` (FZ merged `02eb527` and `ad4a35d` itself; regeneration clean,
`bitfield_audit` 0). **FZ used five working builds against the two-build rule** — its first fault (the camera
bone at pitch 89.8°, FZ5 looking at the sky) took three builds to name. Noted, not repeated: the rule stands.

**The camera is forced and clamped (the user's "FOV and forced perspective").** `ADishonoredPlayerCamera::
UpdateViewTarget` (2012 `0x7135c0`), `UDishonoredCameraInfluenceGroup`, `UDishonoredCamera_AnimDriven` /
`_PlayerControl`; `FDisSoireeControlState::EnterSoireeMode` → `EnableCoreCamInfluence(AnimDriven)`. FZ7:
`MAT_BeginPlayerGroup 'PlayerGroup0' flags 0xa` at 66.97 s → soirée ENTER (collision off, physics Interpolating,
body mode 1); the view sits on the camera bone, level, yaw following the boat where the control sits at
0/-12736/0 from the pawn eye; look limits from `UDisInterpTrackAllowPlayerLook::UpdateTrack` →
`SetPlayerControlledLimit` (yaw ±40 / pitch -20..90 at key 1, then ±170 / -15..90); `MAT_BlendOut 0.20` and
EXIT at 161.60 s. **FOV is 75 the whole run in both builds** — the brief's 90 was the menu `CameraActor`; the
state sets no FOV. The clamp is exercised by code path, not by injected look input.

**Arms (the user's "I can see my arms when I shouldn't"): retail has no arms component to hide.**
`ADishonoredPlayerPawn::SetBodyMode` (2012 `0x702070`) puts the one player mesh ARMS_ONLY
(`SDPG_Foreground`, LOD 1) or FULL_BODY (`SDPG_World`, LOD 2) per group flag; measured 0 → 1 at enter, 1 → 0 at
exit. Nothing is equipped on this route so no arms show in either build — **the user's report needs an equipped
item to reproduce**; `OnCameraUpdate_Derived` arm-follow and `UDishonoredPlayerSkeletalComponent::SetOffset/
SetFOV` are not ported.

**Outside the brief**: `ADishonoredPawn::GetBone_ByName`'s rotation branch skipped `PerformAngleOffsetFixup`
(`0x795080`; `UDisTweaks_Pawn_Body::ApplyFallbackChain_Derived` `0x7911f0` builds `m_AngleOffsetMatrix`). Ported.

**The FSM**: 26 state classes, 23 instanced as cooked sub-objects; Base, Walk, Soiree, InDialog ported, the rest
inventoried with sizes and addresses (agentFZ.md §3). `FStatePlayerMasterBase_Param::OnPending` (`0x6959c0`)
writes `m_pPlayerPawn` — without it a state enters and does nothing.

**Open, passed on (wave 23 GE)**: tall thin stretched dark silhouettes on the matinee-moved boat decks (fz6b/c,
fz7b; also on the boat carrying GA's guards), absent from the control — shared with NPC pawns, not the body mode.

**Premises corrected (five)**: FOV 75 not 90 and never forced; no arms component; `AllowPlayerLook` has its own
keys (`m_TrackLookAngleKeys`); 23 instanced states not "24 uninstanced"; the 2013 flag bits differ from 2012
(`SOIREE_LeaveCameraToDialogTree` 0x20, `HasAnimControlTrack` 0x8, re-entry 0x10) and the influence bitfield has
`m_bHandleCameraCollision`.

## Wave 22 result

Six packages merged: FX `90d490e`, FW `0e45ea1`, FY `69bc81f` + `03f7579`, FV `02eb527`, GA `ad4a35d`, FZ
`785e91d`. Gate green at every merge (one 46/47 on `ad4a35d` — the 1-in-7 intermittent player death, open).
`unported_natives` 0, `verify_phase2` 3/3. What the user saw: guards ride the boat (GA), props animate (FW),
the camera is forced and clamped on the ride (FZ), the AS2 leak is gone (FY), scripted runs never activate the
window (FY). What is next is PHASE19.
