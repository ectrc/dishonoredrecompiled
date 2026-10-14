# Wave 23 — reaching the dock, NPCs that walk, a go-to that arrives, and Use

Wave 22 ported the two natives and the AI go-to (FV), made the props and the guards ride the boat (FW, GA),
put a bitfield audit in the gate and made a regeneration diff clean (FX), made the AS2 machine free its
objects and took the last window activation out of scripted runs (FY), and gave the player the soirée
state (FZ — see its outcome in PHASE18). What the first mission still cannot do, each measured by a
wave-22 package and **not fixed because its two builds were spent**:

1. **The player cannot leave the boat** — it ends 750 units short of the dock (FW).
2. **A walking NPC slides** — the Locomotion track moves him, `UArkAnimNodeLocomotion` is a skeleton (GA).
3. **A go-to never arrives** — `HandleArrival` never runs, so `DisSeqAct_AIGoToActor` never fires
   `Success (Last)` and the captain-and-guards chain stays shut (FV).
4. **`Use` does nothing** — `DDH_PLAYER_USED → DialogOutputs 'Give_Letter'` is unported; the Empress chain
   needs it twice (FW).
5. **The screen stays black after the first Empress scene** (FW).
6. **No guard turns his head** — the look-at target comes from three skeleton AI sub-processes (FV).

The spine is 1 → 3 → 4: walk off the boat, the captain walks to the player, `Use` on him, and the Empress
scene starts without a teleport. Everything is measured against `build/head_rel` (HEAD of main) through
`game_input.py`.

## THE FOUR RULES — daytime, no relaxation

**Fullscreen**: `-windowed` on every launch; the engine refuses fullscreen in a scripted run. **Input**:
nothing touches the real mouse, keyboard or focus — `game_input.py` only. **Builds**: `build_queued.py`
only (half the cores, one build at a time on the machine), **two builds per package**, shared control at
`build/head_rel`; the lock is not fair, so start your gate build early and do everything else while it
waits. **Retail**: **no `Dishonored.exe` launch of any kind** — the user is at the machine.

## The gate

47 checks, **47 ok on HEAD**; `unported_natives` **0 ≤ 0** (FV) — any distinct native here is a
regression; `verify_phase2_total` 3 (FX's bitfield audit). Every `*_criticals` 0 or the merge waits.

## The generator rule (FX)

`gen_classes_header.py <Module> --sdk --module-header --sources-cmake --out-dir <scratch>` regenerates
the four `--sdk` modules byte-identical to the tree and **the coordinator runs it on every merge**. So:
never hand-edit `dishonoredgame*classes.h`, `DishonoredGameRegistrants.cpp` or `DishonoredGame/Sources.cmake`.
A C++-only declaration goes in `Inc/CppText/<Class>.h` (the generator includes it before `};`); a unit taken
off the exclude list whose functions are all virtuals gets a `void DishonoredLink<Class>()` definition
(the generator builds the anchor table from those). Engine/GameFramework headers are reference-engine
files the generator never writes — hand edits there are fine but **say so** (GA's `EngineInterpolationClasses.h`
Locomotion declarations and FW's MAT declarations are the precedent).

## Packages

### GB — the player reaches the dock: the boat matinee's end position

FW measured it five times to the unit: the ride boat `SkeletalMeshActorMAT_2` ends at **9773, 23112** and
the moored collision boat `Boat_Arrive` swaps in stands at **10305, 22565**, 750 units away at the dock;
the pawn stops at X 9994.4 / Y 23144 on every heading, with water beyond. The swap itself runs (handlers
found, 0 missing). So the Boat group's Move track in the lift matinee (`SOIREE_Workers.SeqAct_Interp_3` /
`SOIREE_Boat.SeqAct_Interp_3`, group `Boat_Mover`) is evaluated wrong, or the group is attached to the
wrong actor, or a second matinee/`Teleport` that retail runs at `Boat_Arrive` does not run here.

1. Dump the Move track's keys (`-fkmatineediag` / FS's `-fsmatinee`) and the actor's position at each key
   against the key values; compare the matinee's end against retail's **cooked** key data, not against
   the 2012 skeleton. FS/FN's `UInterpTrackMove` runtime is the suspect (`GetKeyframePosition`,
   `UInterpTrackInstMove::InitialTM`, the relative/world `MoveFrame` of the group).
2. The alternative FW named: `Boat_Arrive`'s Kismet may `Teleport` the player onto the moored boat, or
   the `ToggleHidden`/`ChangeCollision` pair is meant to run while the ride boat is already at the dock.
   Read the sequence (`-fokismetupstream`, fixed by FW for comma lists).
3. **Acceptance**: FW's `fw4_cmd.txt` (scaled to the capture width) walks the player off the boat onto the
   dock and into `DisTrigger_8` — the guards' `SOIREE_Guards.SeqAct_Interp_3` goes 0 → 1 **without
   `-fwteleport`**. Capture before/after.

### GC — the NPC walks: `UArkAnimNodeLocomotion`

GA: Dock Worker B walks 850 units at 123 u/s to `TargetPoint_57` with `Empty_Idle` playing. The walk
animation is `UArkAnimNodeLocomotion`'s — a comment-only skeleton — fed by the locomotion component's
speed/direction (FT's `UArkAnimNodeStack::GetBoneAtoms` stand-in is how the stack evaluates; keep to it).

1. Port `UArkAnimNodeLocomotion` (its `TickAnim`, `GetBoneAtoms`/blend selection, the speed →
   playrate/blend mapping; `GetCurrentTransitSpeed` is FV's) from the 2013 binary, 2012 names at the
   rvas. Retail PC evaluates skeletons through EdgeAnim — do not port the Edge path; the node's
   *selection and blend weights* are what matter and are plain C++.
2. Worker A: his path query fails every 5 s (DN's A*) — measure why (`-gadiag`), fix if it is a bounds or
   reachability test, otherwise name the unit.
3. **Acceptance**: a capture series of Worker B mid-walk with legs moving; `-gadiag` showing the node's
   blend weights change with speed; control shows `Empty_Idle`.

### GD — the go-to arrives, and `Use`

FV: `DisSeqAct_AIGoToActor_2` sends `NPCPawn_25` 89 uu; he moves 52 (the teleport onto the mesh) and sits
in stage 0 forever because `FArkComponentLocomotion::IsArrived()` returns `m_bIsArrived` (default TRUE)
and its only writer `HandleArrival` — also the only emitter of `DESTINATION_REACHED` — never runs:
`ComputeDynamic` exits on `IsDisabled() || STOP_END` first. GA then showed `physWalking → MovePawn` IS
reached (`MAT_SetCollision` forces `PHYS_Walking`), so the component's own state machine is the fault.

1. Walk retail's `FArkComponentLocomotionDynamic::ComputeDynamic` / `HandleArrival` / the stop-state
   transitions against DN's port; find where `STOP_END` is entered before the desire is consumed. Prove it
   with `-fvdiag`'s `goto tick` lines: `arrived 1`, `DESTINATION_REACHED` stim, `SetGoToStatus Success`.
2. **`Use`**: `DDH_PLAYER_USED` → the dialog hook → `DialogOutputs 'Give_Letter'`. FW's `-fwdialogmap`
   names the chain; the player-use path (`UDisPlayerUse*`, `DishonoredGamePlayerUseClasses.h`) is
   skeleton. Port what `Use` on the captain and on the Empress reaches, and the `Give_Letter` output.
3. **Acceptance**: `AIGoToActor_3 'Curnow GoTo 1'` fires `Success (Last)` after `Soiree_BoatRide_Completed`
   (the captain reaches the player); `Use` on him logs the dialog hook firing. If GB lands first, chain it:
   boat → dock → guards → captain → `DisTrigger_16` without a teleport.

### GE — the fade, the heads, and the intermittent death

1. FW: after `SOIREE_Empress_Regent.SeqAct_Interp_0` finishes (16.06 s, `GoEmily` raised) the screen stays
   black — `CameraFade_3` in, no fade out. Director track / `UDisSeqAct_CameraFade` / the player camera's
   fade state (FK's side; FZ's soirée state may own the camera now — read FZ's outcome first).
2. FV: the look-at **target** is a `FDisLookAtRequest` from `UDisAISubProcessHeadTrackBase` / `Bark` /
   `Pause`, three skeletons on the exclude list; and `DisNotify_NPCDisableLookAt` fails at package load
   (`Error, *** Found bad AnimNotify`, 143 lines at 5.7 s every run) — fix the class before anything
   reaches `Notify_EndRange`. Port the three sub-processes enough for a guard to look at the player when
   he walks past (`-flheaddiag`).
3. **The intermittent player death** (PHASE18, GA outcome): on `ad4a35d` the player died 3 s after spawning in the
   d3d9 stage once in seven runs — `ADishonoredPlayerPawn::execPlayDying_Native` fired; seen once before, in FO's gate
   on `15695d8` (wave 20), so it predates FV/GA. First: make the generated unported-native stub log its consumed parameters for this native (killer,
   damage type, hit location) and make `ADishonoredPawn::Died`/`TakeDamage` log the damage event under a switch,
   so the next occurrence names itself; then run the d3d9 stage in a loop (90 s each) until it reproduces or 20
   runs pass. Suspects: FV's new `ADishonoredPawn::Tick` body on the player, fall damage from a spawn before the
   level's static shapes exist (the PhysX scene has 4 static shapes when the player is possessed, every run).
4. **Acceptance**: capture after the Empress scene shows the room, not black; a guard's head follows the
   player in a capture pair; the `bad AnimNotify` lines are gone; the death is either reproduced and named, or
   20 clean d3d9 runs are on record.

### GF — FZ's residue: the arms with an item equipped, and the stretched silhouettes

FZ (PHASE18 outcome) found no arms component — `ADishonoredPlayerPawn::SetBodyMode` (2012 `0x702070`) is the
mechanism and it switches 0 → 1 → 0 correctly — but **nothing was equipped on the harness route**, so the user's
"arms when I shouldn't see them" was never reproduced; `OnCameraUpdate_Derived`'s arm-follow and
`UDishonoredPlayerSkeletalComponent::SetOffset/SetFOV` are unported. Also: tall, thin, stretched dark
silhouettes on NPC pawns standing on a matinee-moved boat (captures fz6b/c, fz7b; the guards' boat too), absent
from the control. GC owns the anim stack; this package owns the player side.

1. Equip the sword/hand before the ride (`-fzdiag` + the inventory path; `HasNothingEquipped`) and measure the
   arms through enter/exit of the soirée state against the control; port `OnCameraUpdate_Derived`'s arm-follow
   and the player skeletal component's `SetOffset/SetFOV` (2012 names at the rvas, 2013 checked).
2. The player's `Tick_Begin` / `PreBeginPlay_Actions` dispatch that FZ stood in for: port the real ones if the
   arms path needs them; otherwise name what is still a stand-in.
3. The silhouettes, player side: if GC shows they are the anim stack's, leave them to GC; if they are the mesh
   translation or `SetBodyMode`'s LOD/depth-group switch on a based pawn, fix here.
4. **Acceptance**: capture pairs (control vs this build) with an item equipped: arms hidden during the ride,
   back after; the silhouettes named or gone.

## Rules for agents

- The four rules above, the two-build limit, the generator rule. **Before hand-back: merge main's HEAD
  into your worktree and make your sync script read 0 flagged** (GA's precedent); `git diff --name-only
  <base>` + `git ls-files --others --exclude-standard source resources` must equal the sync list.
- **A Core change needs a `CoreSmoke` link before hand-back** (FY's one-line fix failed its gate on
  `LNK2019`: Core cannot call WinDrv).
- **Open the minidump before rebuilding** (`build/agentFN/tools/mdmp.py`).
- **Resolve every retail address yourself**; skeleton comments carry 2012 rvas and 2013 moved things
  (wave 22: three of FV's briefed addresses were wrong, two of GA's). `shipping2012_v1.i64` has imagebase 0.
- **Never use the FModel MCP tools.** C4263/C4264 are errors. A bitfield declared in reference order may
  read the wrong bits — the audit is in the gate and names the class.
- `run_regression.py` from the worktree's copy, absolute build dir, in-harness: **47 ok**.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Wave 22: FW corrected
  eight premises, FV eleven, GA six, FY five.

## Acceptance

1. Before/after against `build/head_rel` through the same `game_input.py` script, captures viewed.
2. `run_regression.py`: **47 ok, 0 failed, 0 skipped**; `unported_natives` stays 0.
3. 0 errors / 0 C4263 / 0 C4264 in the gate's own build log.
4. Every address cited resolved by hand, both databases.
5. `resources/docs/agents/agent<XX>.md` + `agent<XX>_status.csv` inside the worktree;
   `build/agent<XX>_sync.py` naming every file and function, self-checked, 0 flagged against main.

## Outcome

### GC — merged `d96a331`; gate on main 46/47 (`d3d9_frames` 750 and 960 against 1000 with several unqueued 16-job builds running - a load cliff; the stage alone re-read 9/9 on the same exe), GC's own gate 47 / 0 / 0 (4823 s)

Eleven files, 0 flagged against `c2dd580`; regeneration clean; `bitfield_audit` 0; 0 d3d9 deaths in its gate.

**`UArkAnimNodeLocomotion` is ported** (34 of retail's 52 functions; the 18 editor/foot-placement ones named with
rvas) and `FArkComponentLocomotion` drives it as retail does (`SetConfig` → `Initialize`; `PreAsyncWorkTick` step
10 feeds `|Velocity.xy| * m_fRealScaledSpeed`). **Measured**: standing `Empty_Idle w 1.00`; walking at 123 u/s
`speed mode 1 w 1.00 | child 1 Empty_WalkN w 1.00 sync`, group rate 1.000, for the whole 850-unit walk; the
control plays `Empty_Idle` throughout. The weights change with speed, as asked.

**But no walking man is visible on either build: the walker is FZ's tall thin dark silhouette, standing and
walking, on the dock as well as the boat.** Not the anim stack (`-flheaddiag`: 110/110 bones, no scale, no
suspect). On him run two `DisSkelControl_FootPlacement` controls at strength 1.00 — a **comment-only skeleton
class** whose instances run the reference engine's `USkelControlFootPlacement` limb IK over cooked Dishonored
properties, `InitializeFootProperties` unported, the floor trace under mesh floors reading the riverbed (GA §6.4).
**Handed to GF** (builds spent): port `UDisSkelControl_FootPlacement`, or prove it with strength 0.

**Worker A never walks because the A\* never runs**: `no start poly at 11554.745/22720.404/2404 (nearest 0.13
units away in XY)` every 5 s — the teleport puts him on `GetClosestPointOnPoly`'s edge point and
`UNavigationHandle::GetPylonAndPolyFromPos` does not contain an edge point. A bounds test (handed to GD).

**Premises corrected (six)**: the node's direction/angle is `FArkComponentFaceTo`'s (unported → forward only);
speed→playrate is one group-rate scale with children at ±1; the `Empty_Idle` slide was retail's own
uninitialised-node fallback; DN's 2013 numbers are RVAs; the route's camera cannot see the dock workers from the
boat (teleport needed); Worker A's failure is not the A\*. Deviations: `UpdateFeetPropeties` guards a zero weight
sum (retail: inf/NaN); `FindAnimNode` for `FindAnimNodeFast`; `GetFootProperties` returns -1.

### GB — merged; GB's own gate on the merged tree 47 ok / 0 failed / 0 skipped; 0 d3d9 deaths

Nineteen files, 0 flagged against `ff5de25`; regeneration clean; `bitfield_audit` 0.

**The boat's end is data, not a bug.** The 19-key Move track ends at 9713, 23115, 2253 and the actor is there; the
Move runtime matches retail in both databases (`agentGB.md` §2). The "moored boat 750 units away" is the dock's
dressing boat and its collision twin, swapped in place at `Boat_Arrive`. **What retail does instead: the Boat
group's four `InterpTrackAttachment` tracks** (a skeleton here) hard-attach the player to the boat's `Player`
socket for the ride and leave him there; the guards to `Captain`/`Driver`. **Ported**: `UInterpTrackAttachment`
(ten bodies) + `UInterpTrackInstAttachment`, `AActor/APawn/ASkeletalMeshActor::DoKismetAttachment`,
`AActor::MAT_Attach/MAT_Detach`, the NPC's (partial). Measured: the player rides attached and is left at
9826, 23064, 2412 facing the gangway.

**Acceptance not reached — a regression, named**: W gives speed 0 at the ride's end on `9f214c5` and after this
port; FW walked 260 units on `da77f0f`. GA's unconditional `MAT_StopRootMotion` leaves the **player** at
`RMM_Velocity` (`rmm 2/0 → 1/1` at 56 s) and `physWalking` (UnPhysic.cpp:1334) derives Velocity from the zero
root-motion delta. Retail's player overrides are unported: `ADishonoredPlayerPawn::StopRootMotionMode` (2012
`0x6facb0` / 2013 `0xaa1af0`) → `RMM_Ignore`, and `ADishonoredPlayerPawn::ChangeAnimStateMotionModes` (`0x6fae70` /
`0xaab1d0`). **GF ports both.**

**Premises corrected (seven)**: the boat's end is data; the far boat is the dock's; the lift matinee has no Move
track; the Move runtime is not the suspect; the player IS held in retail (by the Attachment track); the
`DoKismetAttachment` comment's 2012 rva was wrong; the silhouettes are on the control too.

### GE — merged; GE's own gate on the merged tree 47 ok / 0 failed / 0 skipped

Thirty files, 0 flagged against `f7bc754`; regeneration clean; `bitfield_audit` 0; CoreSmoke linked (Core change).

**The fade is fixed, and it was a dialog-hook bug.** `CameraFade_3` is undone by the director Fade track of
`SeqAct_Interp_26` "Player Overhearing Empress", started by `DialogInputs_26 'BlockPlayer'` on `DisDialogOneShot_27`
— the SECOND one-shot of `NPCPawn_9`. FR's `UDisDialogTree_InGameBind::PlayFromNode` answered TRUE for a hook node
that matched nothing, so `_7` swallowed the hook. Retail (2012 `0x905f40`) answers TRUE only once a conversation
plays — two lines. GE1: `Interp_26` activates at 45.58 s, the fade reads 0.343 → 0.000, captures show the gazebo.

**The 396 `bad AnimNotify` lines per run were the reference engine's owner check**; retail's `PostLoad` has none.
Gone. **Heads**: the three head-track sub-processes ported whole plus the brain's attention-target functions and
`FireDialogHook`, but **no head follows the player yet**: the look-at is raised by
`UDisAIBrainProcessAttention::HandleTopAttnTarget` — 223 functions on the exclude list (agentGE.md §3) → wave 24.

**The spawn death is named** (reproduced on loop run 3/20): the player is possessed at Z -215.85 **before the
level's static PhysX shapes exist** (0 at possession, 4 at +0.5 s) and falls; normal load lands with 57–69
`DmgType_Fell` at 1.4 s (**every d3d9 run loses ~60 health at spawn**); a slow load (startup 41.6 s) passes KillZ
at 2.78 s → 1000 damage, Killer None. Not FV, not GA. Fix = spawn order (possess after `InitWorldRBPhys` has the
static actors) → wave 24. Instruments: `-gedeathdiag`; the generator's `DISHONORED_NATIVE_STUB_LOGPARAMS` for natives
listed in `<Module>NativeStubs.logparams.txt`.

**Premises corrected (six)**: the fade-out is a dialog-started matinee's Fade track; the sub-processes never ask for
a look-at; `DisNotify_NPCDisableLookAt` was never broken; the death is the spawn fall, not wave-22 code;
`USeqAct_Switch::Activated` exists inline; the silhouettes are on static-ground guards in the control too.
