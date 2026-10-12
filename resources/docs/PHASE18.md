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
