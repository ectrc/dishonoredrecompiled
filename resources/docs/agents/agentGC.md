# Agent GC — the NPC walks: `UArkAnimNodeLocomotion`

Wave 23 (`PHASE19.md`, package GC). Worktree `build/agentGC_wt`, base `9f214c5`, with main `785e91d` (FZ) and `62f3682`
(the FIFO build queue) merged by `git merge`. Working build `build/agentGC_rel` (Release, `DISHONORED_LAYOUT_CHECKS=ON`,
`build/agentGC/gc_build.cmd` through `build_queued.py`, log `gc_build0.log`); the gate's in-harness build is
`build/agentGC_reg`. Sync `build/agentGC_sync.py`. Control `build/head_rel` (rebuilt at `9f214c5` = `ad4a35d`'s code,
20:58 today, staged as `DishonoredGame_GCHEAD.exe`); this package staged as `DishonoredGame_GC.exe`. No retail
`Dishonored.exe` launch; every launch `game_input.py launch` with `-windowed` (`presented frame (1920x1080, windowed)` in
all three logs); every address below resolved by hand in both databases (`build/agentGC/idb/GC_*.i64`, copies).
2013 rvas are VA − 0x400000 as every earlier report writes them; the 2012 database has image base 0.

## 0. The answer in six lines

1. **`UArkAnimNodeLocomotion` is ported** (DishonoredGame, `arkanimnodelocomotion.cpp`; 34 of retail's 52 functions, the
   editor / foot-placement 18 named and left) and `FArkComponentLocomotion` drives it as retail does (`SetConfig` finds the
   `LocoBlender`, `Initialize`s it; `PreAsyncWorkTick` step 10 feeds the speed; `SetModifier`, `OnEnable`, `HandleTeleport`,
   `ComputeMostRelevantSpeedIdx` are retail's bodies). §2.
2. **Dock Worker B's legs are driven by the node**: on GC1 the node reads `speed 123.0 smoothed 123.0 ... S1 mode 1 w 1.00
   dirs 4:1.00 | children 1:Empty_WalkN w 1.00 sync` for every second of the 850-unit walk, against `0:Empty_Idle w 1.00`
   standing, and the group rate scale is 1.000 because the walk animation's foot speed is the 123 u/s the dynamics chose.
   The control plays `Empty_Idle` through the whole walk (GCC1, `gadiag pawn ... anim Empty_Idle w 1.00`). §3.
3. **The captures do not show a walking man, on either build**: Worker B is the tall thin dark silhouette of FZ's `fz6b/c`
   (`gcc1_gc_a`, `gc1_gc_a`, standing; `gcc1_gc_b`, `gc1_gc_b`, walking). Measured with `-flheaddiag` (GC2): no bone at
   scale 2, no NaN, bounds normal (`flhead SUSPECT` 0) — it is not FT's doubling. What is on him: two
   `DisSkelControl_FootPlacement` controls at strength 1.00, `on1`, on `foot_L/R_jnt`, and that class is a comment-only
   skeleton running the **reference** `USkelControlFootPlacement` (a two-bone limb IK to a traced floor) over the cooked
   Dishonored properties; GA measured the floor trace under these pawns reading the riverbed (1,000+ units). Named, not
   fixed: the two builds were spent before the measurement came in. §4.
4. **Worker A's path fails on a bounds test, not reachability**: `gadiag pathfail DishonoredNPCPawn_4: no start poly at
   11554.745/22720.404/2404 (nearest 11554.832/22720.500/2402.4 found 1, 5.01 s of 5.00)` every 5 s; the nearest mesh point
   is **0.13 units** away in XY and 1.6 below, the teleport puts him exactly on it, and the next tick's
   `GetPylonAndPolyFromPos` still says no poly — a point on the poly's edge fails the containment test. Unit named in §5,
   with the fix that needs the next build.
5. Premises corrected: six (§1), among them that the node's *direction* is `FArkComponentFaceTo`'s, not the locomotion
   component's; that DN's 2013 rvas for the component are right but his `PreAsyncWorkTick`/`SetConfig` ones named other
   functions in the VA column; and that the captures the plan asked for cannot be taken from the route's camera.
6. Gate: §7. Build logs: 0 errors, 0 C4263, 0 C4264.

## 1. Where the brief was wrong, with the measurement

| the brief / earlier report said | what is measured |
|---|---|
| "the walk animation is `UArkAnimNodeLocomotion`'s, fed by the locomotion component's speed/direction" | the component feeds only the **speed** (`UpdateAnimNode`, 2012 `0x57eff0`: `SetSpeed(\|Velocity.xy\| scaled, m_bInertiaDisabled)` + `SetSpeedMultiplier`) and the **modifier** (`SetModifier`'s tail `0x582df2`). The **angle** has exactly three callers in both binaries: `FArkComponentFaceTo::PreAsyncWorkTick` (`0x57af4e`), `FArkComponentFaceTo::Starting` (`0x57dc8b`, angle 0) and the debug controller. FaceTo is unported, so the angle stays 0 = forward, which is right for a pawn whose body yaw follows his path |
| "the speed → playrate/blend mapping" as one thing | two: `UpdateModifierSlot` picks the two **speed modes** around the smoothed speed (weights linear between their foot speeds, snapped past 0.98), and `UpdateMovementChildren` sets **one group rate scale** = sequence length / (Σ weight × anim speed × length / speed) × modifier multiplier × look-at multiplier on `LocoGroup`; the children themselves play at rate ±1 (−1 for a left-of-forward strafe in a multi-direction mode) |
| DN's banner: "`PreAsyncWorkTick` 2013 rva `0x54caa0`", "`SetConfig` 2013 rva `0x543060`" | both numbers are RVAs and are right as RVAs (`0x94caa0` = `sub_94CAA0`, 1,363 B; `0x943060` = `sub_943060`, the `SetConfig` body); read as VAs they land on `FSplineMeshVertexFactoryShaderParameters::Serialize` and a `TArray` copy constructor. Every 2013 number in this report is an RVA |
| GA §6: "a locomotion-driven NPC here slides in `Empty_Idle` at 123 u/s ... because the walk animation is the node's" | true, and the mechanism is retail's own fallback: an **uninitialised** node (no `Initialize`) puts child 0 at weight 1 and the rest at 0 (`TickAnim`'s else branch, `0x55ba04`), and child 0 is `Empty_Idle`. With `Initialize` the eight phases run and child 1 (`Empty_WalkN`) takes the walk |
| "a capture series of Worker B mid-walk" from GA's route | the Workers scene starts at log 125.5 s (world 116.5) while the player is still on the boat at Z −782 and Worker B walks on the dock at Z 2511, 2,000 units away and 3,300 up: the route's camera never sees him. The captures use FW's `-fwteleport=TargetPoint_57@112` (the player lands at 10988/22152/2407, 340 units from the worker) and a `movepx` turn to yaw 23388; the same script on both builds |
| GA: "Worker A's path fails every 5 s (DN's A*)" | the A* never runs for him: the request dies in step 2 of `UpdatePathOfHigherPriorityRequest` (`no start poly`), before `FindPath`. Not DN's A*, the poly containment test (§5) |
| FT: "`UArkAnimNodeStack::GetBoneAtoms` is the one deliberate non-retail stand-in" (kept, as asked) | kept; this node adds nothing to the evaluation path. But the stretched bodies are not a stack or blend-weight effect either — §4 |

## 2. What was ported

### 2.1 `UArkAnimNodeLocomotion` (`DishonoredGame/Src/arkanimnodelocomotion.cpp`, `Inc/CppText/UArkAnimNodeLocomotion.h`)

The class is a `DishonoredGameEngineShims.h` shim (FT's reason for `UArkAnimNodeLookAt`), so the bodies live in
DishonoredGame; retail's unit is `Engine/Src/arkanimnodelocomotion.cpp`. The CppText is included by the generated shim
body through the generator (`gen_classes_header.py --out-dir` run, the two `#include "CppText/..."` lines and the
`DishonoredLinkArkAnimNodeLocomotion` anchor copied from its output; the regeneration of the merged tree is byte-identical).

| function | 2013 rva (size) | 2012 rva (size) | notes |
|---|---|---|---|
| `TickAnim` | `0x51b280` (413) | `0x55b910` (413) | the eight phases on an initialised node whose nine children are sequences; else child 0 = 1. `UpdateGridMove` (editor) not called |
| `Initialize` | `0x51d4c0` (550, unnamed in 2013: `sub_91D4C0`, the one callee of `SetConfig`) | `0x55e050` (550) | slots reset, children checked as `UAnimNodeSequence`, `m_bFootOnGround` sized to the feet |
| `GetAnimNodeLocomotion` | `0x515930` (72) | `0x54c370` (63) | `LocoBlender` by name; 2013 adds the `Cast` (`sub_9150B0`); `FindAnimNode` for retail's `FindAnimNodeFast` |
| `BlendSpeedAndAngleProperties` | `0x50bc00` (427) | `0x54c3b0` (427) | the (speed, angle) velocity eased at `m_fVelocityMaxBlendSpeed × ratio` |
| `HandleTransition` | `0x511230` (355) | `0x551920` (355) | the modifier-to-modifier transition animation as a specific anim (type 3) |
| `UpdateSpecificAnim` | `0x50bdb0` (669) | `0x54c560` (669) | mode weights in/out over `m_fBlendDuration`; the move-in gate on the most relevant child |
| `UpdateMovementAnims` | `0x516770` (472) | `0x556c40` (472) | the two modifier slots' crossfade, overwrite under `m_fModifierBlendOverwriteMaxPct`, out-of-shape ratio, `UpdateModifierSlot(i, 0.98)` |
| `UpdateModifierSlot` | `0x5105b0` (946) | `0x550ca0` (946) | the two speed modes around the blended speed, their two direction animations each |
| `UpdateMovementChild` | `0x510970` (800) | `0x551060` (800) | weight = modifier × speed × angle × mode[0]; `bSynchronize` = !static; `bReverseSync` = left strafe; returns weight × anim speed × length |
| `UpdateMovementChildren` | `0x516950` (2,799) | `0x556e20` (2,799) | kept / cleared / newly assigned children, the forced group position, the group rate scale |
| `UpdateSpecificChild` | `0x517440` (319) | `0x557910` (319) | child 8 |
| `UpdateFeetPropeties` | `0x5114a0` (793) | `0x551b90` (793) | `m_bFootOnGround` from the group position; one guard added (§6) |
| `UpdateProgressVariables` | `0x5113a0` (246) | `0x551a90` (246) | |
| `ResetModifierSlot`, `FindFastestSpeed`, `ComputeBlendingOutOfShapeRatio`, `GetFinalSpeedMultiplier` | `0x510c90` `0x510cf0` `0x510e30` `0x5118b0` | `0x551380` `0x5513e0` `0x551520` `0x551fa0` | the chord table `SegmentPoints` read off the 2012 local static init (`0xf4a730`) |
| `OnBecomeRelevant`, `OnCeaseRelevant` | `0x515ec0` `0x509c00` | `0x556390` `0x54a490` | |
| `SetAngle`, `SetSpeed`, `GetSpeed`, `SetSpeedMultiplier`, `SetModifier`, `GetMovementAnimPrevAndCurProgress`, `IsSpecificAnim*`, `GetTypeOfCurSpecificAnim`, `IsPlayingOnlySpecificAnim`, `AbortSpecificAnimRequest`, `GetMostRelevantModifierSlot`, `GetOutOfShapeRatio`, `GetDeltaTime` | `0x509a40`..`0x509c30` | `0x54a2d0`..`0x54a4c0` | |
| `GetDurationBeforePlayingTurnAnim`, `RequestTurnAnim`, `RequestStartAnim`, `RequestStopAnim` | `0x510060` `0x5101d0` `0x510340` `0x510470` | `0x550750` `0x5508c0` `0x550a30` `0x550b60` | the root-move state machines' requests (their callers, `UpdateTurn` / `UpdateStartMove` / `UpdateStopMove`, are DN's unported `arkcomponentlocomotionrootmove.cpp`) |
| `GetFootProperties` | `0x515980` | `0x555e50` | returns −1 until `GetFootIndexFromBoneIndex` is ported |
| the two names | dyn. init `0xb83210` / `0xb83230` | `0xb9f990` / `0xb9f9b0` | `"LocoBlender"`, `"LocoGroup"` (2012 .rdata `0xca8c8c` / `0xca8c98`) |

Bitfield reads, checked against the retail SDK layout (`m_bInitialized` 0x1 .. `m_bPreviewOrientOnSlope` 0x100 at 568):
the `*((_DWORD*)this+142)` masks in every decompile. `UAnimNodeSequence` bits 0x4000 / 0x8000 at 216 are `bSynchronize` /
`bReverseSync` in this tree's declaration order; `Rate` 212, `CurrentTime` 220, `PreviousTime` 224, `AnimSeq` 228.
`FAnimGroup` +12 `SynchMaster`, +28 `RateScale`, +32 `SynchPctPosition`.

**Not ported, named**: `GetNextFootOnGroundProperties` (2013 `0x515a00`, 885 B), `GetDefaultLocalRootFootDelta` (`0x515d80`),
`GetFootIndexFromBoneIndex` (`0x5117c0`) — the foot-placement readers (their consumer is the unported Dishonored foot
placement, §4); `Render` (`0x509c40`), `GetSliderPosition` / `HandleSliderMove` / `GetSliderDrawValue` (`0x515f70`,
`0x516170`, `0x5163f0`), `UpdateGridMove` (`0x511910`) — the AnimTree editor.

### 2.2 `UArkComponentLocomotionConfig` (same unit, `Inc/CppText/UArkComponentLocomotionConfig.h`)

`ComputeAnimDirAndWeightFromAngleAndDirCount` (2013 `0x5331d0` / 2012 `0x574b20`, 217 B), `GetSpeedModeFromForwardSpeed`
(`0x5343a0` / `0x575c30`, 224 B), `GetForwardSpeedOfSpeedMode(INT)` (`0x5342a0` / `0x575b30`: move animation
`4 / (8 / count)`, the forward one) and the `(speed, modifier)` overload (`0x534380` / `0x575c10`).
`FArkComponentLocomotion::GetForwardSpeedOfSpeedMode` now forwards to it as retail does (DN's stand-in took the fastest
animation of the mode; identical on single-direction modes, which every mode the tower's workers use is).

### 2.3 The component (`arkcomponentlocomotion.cpp`, `.h`)

`SetConfig` (2013 `0x543060` / 2012 `0x584010`): `GetAnimNodeLocomotion` → `m_bCanManageAnim` → `Initialize`. `SetModifier`
(`0x541b70` / `0x582b40`): the node's modifier. `OnEnable` (`0x53def0` / `0x57f040`): speed 0 and the multiplier.
`UpdateAnimNode` (`0x53dea0` / `0x57eff0`, new). `HandleTeleport` (`0x546550` / `0x587240`): `AbortSpecificAnimRequest(TRUE)`.
`ComputeMostRelevantSpeedIdx` (`0x53f090` / `0x5802a0`): retail's body in place of DN's `return 0`. `PreAsyncWorkTick`
(`0x54caa0` / `0x58d5f0`): step 10's node feed after `ComputeDynamic` — `|Velocity.xy normalised × m_fRealScaledSpeed|`
read off the disassembly at `0x58daad..0x58dae2` (`[esi+0F8h]` = sub-object +248 = `m_fRealScaledSpeed` @264,
`[esi+188h]` = `m_CurModifierIdx` @408; the `this` of that decompile is the `IArkComponentPreAsyncWork` sub-object at +16).
`UpdateFaceTo` / `UpdateLookat` stay out (their components are not here).

## 3. The runs

All through `build/agentGC/gc_run.py` (GA's runner, `game_input.py launch`, `-inputcmd -unattended -windowed`), route
`-finewgame=50 -fosoiree -fkdiag -frdiag -diskismet -gadiag -fwdiag -fwteleport=TargetPoint_57@112`, steps
`build/agentGC/gc_steps.txt`: at wall 118 s two `movepx` turns, then captures every 2 s (`gc_a..gc_f`) and `gc_g` 10 s
later, quit. The teleport fires at world 112 (log 121.9): the player stands at 10988/22152/2407 facing yaw 23388; Worker B
(`DishonoredNPCPawn_3`) stands 340 units away at 10845/22461 until the scene's key 0 (log 125.1 / 125.5) sends him to
11634/22103 at 123 u/s. Captures in-process, 1920x1080, 6,220,854 B each, thumbnails `build/agentGC/captures/*.png`, all
viewed. No `Critical`, no new minidump, `FocusLost` 0 (the one `execOnFocusLost` line is the front end's unported native,
in every run of every package).

| run | exe | log | what it measured |
|---|---|---|---|
| GCC1 | control `DishonoredGame_GCHEAD.exe` (`head_rel`, `9f214c5`) | `GCC1.log`, `gcc1_run.txt` | Worker B walks 850 units in `Empty_Idle w 1.00` (log 125.1 → 133.2); `gcc1_gc_a` (world 115.8, standing) and `gcc1_gc_b` (117.9, walking) show him as a tall thin dark silhouette; `gcc1_gc_c..g` he has left the view |
| GC1 | this package, `DishonoredGame_GC.exe` (`agentGC_rel`) | `GC1.log`, `gc1_run.txt` | `gadiag loconode DishonoredNPCPawn_3`: standing `speed 0.0 ... M0 mod 0 w 1.00 S0 mode 0 w 1.00 static dirs 4:1.00 \| children 0:Empty_Idle w 1.00`; from log 125.9 `vel 123.0 realscaled 123.0 ... mostrelevant 1 ... S1 mode 1 w 1.00 dirs -1:0.00 4:1.00 \| children 1:Empty_WalkN w 1.00 t 0.85 sync`, rate 1.000, every second to 133; `ComputeMostRelevantSpeedIdx` 1 while walking. Same positions and timing as the control to the unit. Captures: the same silhouette (`gc1_gc_a`, `gc1_gc_b`) |
| GC2 | this package + `-flheaddiag` | `GC2.log`, `gc2_run.txt` | FT's sweep on Worker B at 341 units: `bones 110 required 110`, no `SUSPECT`, local/composed deltas of the idle (`spine_3_jnt l29/c16`, `neck_jnt l25/c40`); controls: `foot_R_jnt:DisSkelControl_FootPlacement(s1.00 b0.20 on1); foot_L_jnt:... on1`, spine bender off, eyes off |

The node's weights change with speed as the acceptance asks: the speed slot flips from `S0 mode 0 static` (idle) to `mode 1`
(walk) the second `m_fRealScaledSpeed` becomes 123, the two modifier slots stay `M0 mod 0 w 1.00 / M1 mod -1 w 0.00`
(one modifier in the config), and the most relevant child changes from 0 to 1. With one speed between idle and walk there
is no intermediate blend to show on this route; `UpdateModifierSlot`'s linear weight is exercised by the same code path
with two modes bracketing the speed (the control's `S0/S1` alternation on GC1 is the retail `>` test flipping on a speed
exactly equal to the mode's foot speed, both slots at 1.00).

## 4. The silhouette (FZ's `fz6b/c`, `fz7b`), measured on the dock

Both builds draw Worker B as a man two to three times his height, dark, thin, arms up (`gcc1_gc_a`, `gc1_gc_a`), and he
walks away in that shape (`gcc1_gc_b`, `gc1_gc_b`). What GC2 rules out: the anim stack / blend weights (the LocoBlender's
nine children are accounted for above; FT's `flhead SUSPECT` sweep finds no bone at scale 2, no NaN, no bounds over 2,000
on him, so this is not the `GetBoneAtoms` doubling of wave 21). What it finds: two active `UDisSkelControl_FootPlacement`
controls at strength 1.00 on the feet. `disskelcontrol_footplacement.cpp` is on the exclude list (comment-only), so the
object runs the **reference** `USkelControlFootPlacement::CalculateNewBoneTransforms` — a limb IK that traces for a floor
under the foot and pulls the leg chain to it — over cooked Dishonored properties the reference code does not know
(`UDisSkelControl_FootPlacement`'s own members; GA §6 item 4: `InitializeFootProperties`, 2012 `0x6da640`, is what fills
`m_FeetProps`). GA's `-gadiag ground` measured the trace under a pawn on a skeletal-mesh floor reading the riverbed 1,000+
units down, which is exactly where FZ saw the silhouettes (pawns on the matinee-moved boat) and the dock is a mesh too.
Prediction to test with the next build: disable the two controls (`StrengthTarget 0`) or port `UDisSkelControl_FootPlacement` (its rvas are not resolved
here; FT's `disskelcontrol_spinebender.cpp` is the precedent) and the silhouette becomes a man. Not done here: both builds were spent.

## 5. Worker A: the path failure, named

`DishonoredNPCPawn_4` at 11555/22720/2498 (cylinder 96, ground trace hits 2404). Every 5 s the track re-sends him to
`TargetPoint_24` (`request 1 event 1` then `event 2` = `CPNT_LOCO_EVENT_PATHFINDING_FAILED`). The new `gadiag pathfail`
line (both failure points of `UpdatePathOfHigherPriorityRequest` / `UpdatePathFindingEdges`) reads, every time:

    gadiag pathfail DishonoredNPCPawn_4: no start poly at X=11554.745 Y=22720.404 Z=2404.000 (nearest X=11554.832 Y=22720.500 Z=2402.417 found 1, 5.01 s of 5.00), destination X=11471.734 Y=22733.715 Z=2405.000

So: `UpdateStartLocAndVerifyIfOnValidPoly` (retail 2012 `0x57f8a0`, 474 B, DN's port is that body line for line - the
floor trace from +10 to −2h−10, `GetPylonAndPolyFromPos(ground, 0.f)`, the step-height and cylinder-top tests) fails at
`GetPylonAndPolyFromPos`; `FindNearestLocationOnNavMesh` (150 units) **does** find a poly, 0.13 units away in XY and 1.6
below; the teleport branch puts him on that point (he creeps +0.09 units per attempt: 11554.745 → .832 → .919 → 5.093 →
5.267 across the five attempts logged), and the next containment test fails again. A point returned by
`FNavMeshPolyBase::GetClosestPointOnPoly` lies **on the poly's edge**, and `UNavigationHandle::GetPylonAndPolyFromPos`
(Engine, reference `UnNavigationHandle.cpp:4076`, a 5×5×1024 box query then the per-poly containment) does not count an
edge point as inside. It is a bounds test; the reachability of `TargetPoint_24` is never asked (the A* does not run). The
fix is one of two lines and needs a build this package no longer has: nudge the nearest point one unit toward the poly's
centre in `FindNearestLocationOnNavMesh` before it becomes `m_TeleportLocation`, or give `UpdateStartLocAndVerifyIfOnValidPoly`
the XY tolerance retail's engine has in `GetPolyFromPoint` (to be read in the 2013 database: the reference's edge test is
strict, retail's may not be). The same line names 10 other pawns at spawn (`_10`, `_11`, `_19`, `_26`, `_27`: no poly within
150 units — spawned off the mesh, GA's 11 of 28; `_2`, `_21`: `FindPath` error 1 over 109–123 units, a real A* failure).

## 6. Deviations, named

- `UpdateFeetPropeties`: retail divides the foot-on-ground range by the weight sum whenever it is under 1 and gets infinity
  for a sum of 0; the sum is tested (`> 0.00001`) first. Everything else in the node is retail's control flow.
- `GetAnimNodeLocomotion` walks the tree (`FindAnimNode`) because `UAnimTree::FindAnimNodeFast`'s table is not ported.
- `GetFootProperties` returns −1 (no `GetFootIndexFromBoneIndex`).
- The node's angle is never set (no `FArkComponentFaceTo`): forward animations only.
- `-gcview=X,Y,Z,Yaw@seconds` added to `fwdiag.cpp` (measurement only, unused in the runs above: `-fwteleport` + a
  `movepx` turn gave a view both builds could take identically).

## 7. Acceptance

| | |
|---|---|
| `run_regression.py` | see `build/agentGC/gc_regression.log` and `build/agentGC_reg/regression/summary.txt`: the worktree's copy, absolute build dir, in-harness build (the package's second build), `--newgame-at 50 --game-extra-args=-windowed`: **47 ok, 0 failed, 0 skipped**, 4,823 s; `unported_natives 0 (<= 0)`, `coresmoke_failed 0`, `nullrhi_criticals 0`, `d3d9_criticals 0`, `inputtest_criticals 0`, `newgame_criticals 0`, `options_criticals 0`, `verify_phase2_total 3`, `newgame_pawns 29`, `inputtest_moved 950.3`, `d3d9_startup_seconds 5.3`; the d3d9 stage's log has 0 `execPlayDying_Native` lines; its build log 0 errors / 0 C4263 / 0 C4264 |
| the builds | `build/agentGC_rel` (one pass, `gc_build0.log`: 0 errors, 0 C4263, 0 C4264, 314 warnings all pre-existing) and the gate's `build/agentGC_reg` |
| generator | `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir` before the merge (only the two CppText include lines and the anchor differed; copied) and after it (byte-identical); no hand edit of `dishonoredgame*classes.h`, `DishonoredGameRegistrants.cpp`, `Sources.cmake` |
| `build/agentGC_sync.py` | 11 files, every function named, 0 flagged against main, the list equal to `git diff --name-only <base>` + `ls-files --others` |
| the merge | main `785e91d` (FZ, 53 files) and `62f3682` merged with `git merge`, no conflicts (FZ's and GC's generated-file edits are disjoint and the regeneration is byte-identical) |
| before/after | §3: GCC1 (control) vs GC1 vs GC2, one route, one script, same world times |
| the rules | every launch `game_input.py launch` + `-windowed`; no real input, no focus change; builds through `build_queued.py` only (queued behind GB, the gate, GD, GF for 66 minutes); no retail `Dishonored.exe` launch |
