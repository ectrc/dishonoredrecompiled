# Agent IO: NPCs face one fixed direction while walking; odd routes (wave 26, PHASE23 package IO)

Worktree `build/agentIO_wt` at `242fb05` (= main `b08193f` + the PHASE23 launch-4 lines). Build dir `build/agentIO` (RelWithDebInfo).
No commits. Two builds, both 0 errors and 0 C4263/C4264:
- build 1 (`DishonoredGame_IO1.exe`; first invocation stopped on one compile error, a 34-argument `debugf`, fixed and resumed:
  `build/agentIO_build1.log`, `agentIO_build1b.log`): UpdateFaceTo and the rest of §2 except the path builder, `-iodiag`, and two A/B
  switches (`-iooldface`: no UpdateFaceTo / lean / retail IsLocoFaceToRequestActive = the b08193f behaviour; `-iooldmod`: the pre-IO
  SetModifier);
- build 2 (`DishonoredGame_IO2.exe` = `DishonoredGame_IO.exe`, `agentIO_build2.log`): adds retail's path builder (§3); the A/B
  switches are removed.

Runs (all `game_input.py launch`, `-windowed -nomovie`, never real input): ride route `build/agentIO/io_run.sh` (cutscene_compare,
`-finewgame=50 -fosoiree`, `-iodiag -ibdiag -ikdiag`), L_Tower_P spawn route `build/agentIO/io_tower.sh` (`-startmap=L_Tower_P
-startmapopen -distouchprobe`, 150 / 240 s). Tables: `build/agentIO/io_measure.py <log>` (per NPC, `-paths`), `io_table.py tag=log ...`.

| run | exe | switches | route |
|---|---|---|---|
| IO1c | IO1 | `-iooldface -iooldmod` (= b08193f behaviour) | ride, world 50..175 shots |
| IO1b | IO1 | same | ride, 140..175 shots |
| IO1a | IO1 | - (face fix, old path builder) | ride, 50..175 |
| IO1t | IO1 | - | L_Tower_P spawn, 150 s |
| IO2a, IO2b | IO2 | - | ride (50..175, 140..175) |
| IO2t | IO2 | - | L_Tower_P spawn, 240 s |

## 1. The measurement (before the fix)

`-iodiag` logs once a second per NPC moving or holding a computed path: `Rotation.Yaw`, velocity yaw and their difference, the
path-move yaw, `m_CurMoveYaw` / `m_TargetMoveYaw`, the move rotation speeds, the face-to request count / the locomotion's
`m_FaceToRequestID` / priority / `m_CurBodyYawDelta` / angle / target, the mesh's relative rotation, `RootMotionMode` /
`RootMotionRotationMode`, the root state and the path index; and every new path's points (`iodiag path`).

IO1c (the b08193f behaviour): **every walker's face-to request count is 0** (req% 0 for all NPCs). The face-to component exists
(IK4 made it default) but nothing asks it to turn the body: retail's `FArkComponentLocomotion::UpdateFaceTo` (2013 VA 0x943190,
2012 0x584130, 753 B) was never ported (`arkcomponentlocomotion.cpp` said "UpdateFaceTo / UpdateLookat stay out (their components
are not here)"). FullMovePawn writes `Rotation.Yaw + m_CurBodyYawDelta`, the delta is 0, so the body keeps its spawn yaw:
`_9` walks 53 samples at a constant 27.5 deg off its velocity, `_3` (Worker B) 90 deg, the captain `_26` 36 deg mean.
Before IK4 a pawn without the face-to component took `m_CurMoveYaw` directly in FullMovePawn - that is why the defect appeared with
IK4's "FaceTo on by default".

The mesh is not the cause: `Mesh->Rotation` is 0/0/0 on every walker in every run (the mesh-offset component's external rotation),
and stays at the lean values (pitch/roll a few hundred units, yaw 0) after the fix.

The same stand-in made `IsLocoFaceToRequestActive` answer "no face-to component" - FALSE for every NPC since IK4 - which silently
switched off the static, start-path and path turns.

Odd routes: `iodiag path` shows every multi-edge path zigzagging between the two ends of its navmesh edges, e.g. IO1a `_9` at 141.09:
`13163/23163 12641/22875 12454/22701 12421/23365 12203/22742 12169/23365 11927/22541 11367/23963 10850/23613 ...` (Y alternating
22.7k / 23.4k), the captain `... 10390/22412 11385/22104 10756/22465 11440/22143 11455/23496 11927/23591 12169/22742 12203/23365
12421/22742 12454/23396 ...`. The cause is the path builder, not avoidance (§3).

## 2. The body yaw (builds 1 and 2)

| function | retail | |
|---|---|---|
| `FArkComponentLocomotion::UpdateFaceTo` | 2013 VA 0x943190 / 2012 0x584130 (753 B) | whole: AllowRootMotion(disabled/deleted) / AllowRootMotion(FALSE) while a specific anim plays / DisableRootMotion; one yaw request at `m_FaceToPriority` towards `m_CurPathMove` while a path is computed and the pawn moves, speed `m_fMaxMoveRotationSpeed` (x 1.5 x the multiplier unless it is the emergency one, floor `m_fFaceToDefaultSpeed`), acceleration `m_fRotationAccelMultiplier` x speed, keep-snapped; else StopFaceTo; ForceBodySnapOnMovement when the angle is 0 and the speed is above `m_fMaxMultiDirSpeedForAngle`; SetRealVelocity(the velocity before ComputeDynamic) |
| `PreAsyncWorkTick` tail | 2013 VA 0x94caa0 | velocity captured before ComputeDynamic; unless PHYS_RigidBody: ComputeTargetPitchAndRoll, UpdatePitchAndRoll, UpdateMesh; then UpdateAnimNode, UpdateFaceTo |
| `ComputeTargetPitchAndRoll`, `UpdatePitchAndRoll`, `UpdateMesh` | 2013 VA 0x93e9e0 / 0x93eb30 / 0x93de20 (2012 0x57fca0 / 0x57fde0 / 0x57ef70) | the lean into turns; UpdateMesh writes the mesh-offset component's external rotation (pitch, 0, roll) |
| `SetModifier` | 2013 VA 0x941b70 / 2012 0x582b40 (702 B) | whole: the modifier's acceleration / deceleration are distances to reach / leave the fastest speed (rates v / (d / (v/2))), `m_MaxMultiDirSpeedIdx`, `m_bHaveStopAnim` (GK wrote "no writer in either database": this function writes it) |
| `UArkComponentLocomotionConfig::ComputeSpeedFromSpeedModeAndAngle` | 2012 0x575d10 (300 B) | new (CppText declaration; generator 79/79 byte-identical, `build/agentIO/regen`) |
| `ComputeDynamic` head | 2012 0x58ca10 | `m_fMaxMultiDirSpeedForAngle` from the face-to angle / out-of-shape ratio; the third out of UpdateMoveYaw is `m_fMaxMoveRotSpeedMultiplier` (DN wrote `m_fMaxMultiDirSpeedForAngle` there); the anim road no longer zeroes it |
| `UpdateMoveYaw` | 2012 0x582300 (1,739 B) | retail: max rotation speed from the most relevant modifier slot's two speed modes (or GetSpeedModeFromForwardSpeed), ArkSmoothFixAngleRotation with acceleration |
| `HandleArrival` end speed | 2012 0x589560 | ComputeSpeedFromSpeedModeAndAngle(EndSpeedIdx, modifier, angle) |
| `IsLocoFaceToRequestActive` | 2012 0x57fa80 (70 B) | retail: no request, or the active one is the locomotion's, or its priority <= `m_FaceToPriority` |
| `EvaluateStaticTurn`, `EvaluatePathTurn` | 2012 0x58c040 / 0x58c3d0 | the static turn towards the face-to target yaw (was skipped), the path turn's IsActiveRequestSnapped test |

Named deviations: UpdateFaceTo / the head of ComputeDynamic guard a NULL face-to component or anim node (retail has neither NULL);
UpdateMoveYaw's wall test is not ported - it needs `UNavigationHandle::PointReachableZeroExtent` (2012 0x2a9ba0, 1,781 B, absent),
so the multiplier stays 1 (retail's value under `m_fMinAngleToTestWalls` or when both probes pass); UpdateLookat is still out.

## 3. The routes (build 2)

`arkpathbuildutils.cpp`: `FindFirstEdgeIndex` (2012 0xb4d10), `AdjustPathFindingEdges` (0xb54d0), `MinimizeStraightPath_FunnelAlgorithm`
(0xb5cf0), `AdjustStraightPath` (0xb5fe0) ported whole. Read against the decompiles:
- DN's left/right edge winding was the mirror of retail's (retail: left is P0 when the cross about the shared poly's centre is <= 0;
  DN swapped there), while DN's funnel step `ApplyFunnelAlgorithm_FindTurn` is retail's sign for sign - so the funnel saw every edge
  backwards and emitted alternate edge ends: the zigzag.
- FindFirstEdgeIndex was a stand-in (always edge 0, its own winding); retail skips the edges the start point has already crossed.
- The funnel's end test was wrong (DN took the right corner for left <= 0 && right >= 0; retail: both <= 0 right corner, both >= 0
  left corner) and AdjustStraightPath only re-derived Z; retail inserts a corner the pulled-in distance away on the bisector when
  the path arrives at / leaves a pulled-in corner at more than 135 degrees.
- Kept: IsEdgeBorderExtentReachable asks the reference `PointReachable`, not `PointReachableZeroExtent` (named); a NULL shared poly
  uses the edge's P0 as the centre (retail would read a NULL poly).

After: the same searches give straight funnels, e.g. IO2a `_9` 141.0: `13163/23163 12641/23224 12169/23365 10750/23962 10559/24037
10529/24090 10429/24148 10408/24195 10404/24681`; the captain 150.7: `10058/23022 10075/23020 10339/23007 10387/22947 10412/22412
10467/22352 10603/22391 11431/22488 12535/22885`.

Avoidance (`DetectPreAvoidanceCollision`, `PointReachableZeroExtent`) was **not** the cause of the odd routes and is not ported.

## 4. Before / after per NPC (ride route)

|Rotation.Yaw - velocity yaw| in degrees over the once-a-second samples moving faster than the config's start speed (60 u/s), PHYS_Walking.
"steered" = root state NONE with a computed path (where the locomotion's face-to request drives the body); "all" also has the turn /
start / stop animations (root motion) and NPCs walking without a path.

| NPC | IO1c (b08193f behaviour) all: n / mean / <15 | steered | IO2b (build 2) all: n / mean / <15 | steered |
|---|---|---|---|---|
| _3 Worker B | 11 / 89.6 / 0% | 11 / 89.6 / 0% | 37 / 49.3 / 45% | 8 / 23.9 / 75% |
| _4 | 12 / 49.5 / 25% | 11 / 54.0 / 18% | 9 / 10.9 / 77% | 4 / 0.3 / 100% |
| _8 | 4 / 86.3 / 0% | - | 4 / 86.5 / 0% | - |
| _9 | 53 / 27.5 / 0% | 53 / 27.5 / 0% | 41 / 0.2 / 100% | 19 / 0.3 / 100% |
| _12 | 10 / 11.2 / 100% | 9 / 10.9 / 100% | 124 / 53.8 / 37% | 10 / 0.5 / 100% |
| _15 | 2 / 54.4 / 0% | 2 / 54.4 / 0% | 2 / 0.9 / 100% | 2 / 0.9 / 100% |
| _16 | 4 / 84.8 / 0% | - | 4 / 94.7 / 25% | - |
| _19 | 5 / 81.6 / 0% | - | 5 / 80.5 / 0% | - |
| _25 | - | - | 1 / 0.0 / 100% | 1 / 0.0 / 100% |
| _26 captain | 23 / 35.7 / 26% | 21 / 39.1 / 19% | 34 / 5.3 / 97% | 21 / 8.5 / 95% |

(IO1a, build 1 with the old path builder: `_9` 19.5 / 82%, `_26` 24.8 / 71%; IO2a, the other build-2 ride run: `_9` 0.1 / 100%,
`_26` 5.0 / 97%, `_3` 53.4 / 47%, `_12` 66.4 / 44%. `build/agentIO/table_ride.md`.)

L_Tower_P spawn route (`build/agentIO/table_tower.md`): only `_12`, `_15`, `_25` and the three no-path walkers move; steered `_12`
0.2 / 100%, `_15` 1.4 / 100%, `_25` 0.0 / 100%; all-samples `_12` 68.9 / 41% (the turn loop below).

**Acceptance (< 15 deg for every walker on both routes): met for the locomotion-steered walking (every NPC, steered column, except
Worker B's 8 samples at 24 deg mean); not met for all samples**, for three reasons that are not the face-to request:

1. **Turn animations spin (new with build 1's retail IsLocoFaceToRequestActive): `_3` and `_12`.** In TURN (root 3/4/5) the
   body yaw changes by 1,000..1,800 units per sample (~6..10 deg/frame), passes the target and ends 160 deg off, and the static turn
   fires again: `_12` loops for the whole run (124 samples, positions wander inside 17300..17780 / 22900..23440), Worker B after
   each leg. Cause (read, **not built**): our `USkeletalMeshComponent::ProcessRootMotion` (`Engine/Src/UnSkeletalComponent.cpp:3611`,
   stock UE3) moves the actor by the root rotation whenever `RootMotionRotationMode == RMRM_RotateActor`, in every root-motion mode;
   retail's (2012 0x353c80, called from UpdateSkelPoseEnd 0x354c10) enters that MoveActor block only for `RMM_Translate`, or
   `RMM_Ignore` right after `RMM_Translate` - for RMM_Accel / RMM_Velocity the rotation goes through the face-to component
   (`m_CurBodyYawDelta` = the root rotation, applied by FullMovePawn). The turn animations run RMM_Accel + RMRM_RotateActor (UpdateTurn
   2012 0x58c520), so ours turns the body twice. The fix is the condition: `(RootMotionMode == RMM_Translate || (RootMotionMode ==
   RMM_Ignore && PreviousRMM == RMM_Translate))`, dropping the `|| RootMotionRotationMode == RMRM_RotateActor` term (rotation inside
   stays gated by RMRM_RotateActor; retail also takes the translation from its own accumulator at +832, a separate detail). No build
   was left; **this should go with the sync** (package IO2) - with build 2 alone Worker B's return leg drifts (IO2a: 10988/22152 at
   121 -> 11240/22558 at 158) where b08193f returned (IO1c 122..125).
2. **No-path walkers `_8`, `_16`, `_19`** (world 51..55, around -1000/33700 and -4300/35900, Z -1420..-1520): they are "walking
   back to the mesh" (`m_bTryToReturnOnNavMesh`, no computed path) at 157 u/s for ~4 s, oscillating around one point, then stop
   (`m_fMaxDurationToReturnOnNavMesh`). Retail's UpdateFaceTo also only turns a body on a computed path, so the yaw is retail's; that
   these NPCs are off the navmesh at the soirée start is the defect (navmesh/pylon of that sublevel, as IK's L_Tower_P +249 X) - not
   investigated.
3. Root-motion start / stop states (root 6..9, 10..14): the face-to component is in root motion there (retail), the body turns only
   with the animation's root rotation.

## 5. The captain, Worker B, the ride

- **Captain**: faces his walking direction (steered 8.5 deg mean, 95% < 15). He reaches the dock floor (base WorldInfo, Z 2500) and his
  go-to point 12535/22884 (IO2b: on the dock floor by 180.2, arrived 192.5) - **but 20 s later than IN (157.76)**: from 156.0 to ~174 he
  stands at 10433/22844/2530 on the bow, `ikdiag floor ... no floor within 3 x MaxStepHeight`, FullMovePawn refusing every move, until
  FilterPathingFail re-searches (pfail 136) bring him down the bridge (base SkeletalMeshActor_2). He arrives there 40 uu east of the
  new straight segment 10387/22947 -> 10412/22412 while the start animation (root 7/8, `Empty_Walk_Start`) carries him; the old
  zigzag path happened to run 30..50 uu further west (corner 10370/22522). Retail's wall test in UpdateMoveYaw (the emergency
  rotation multiplier next to a wall) and DetectPreAvoidanceCollision are the unported parts that act exactly here. Reproduced
  twice (IO2a, IO2b).
- **Worker B (`_3`)**: ride route only - on the L_Tower_P spawn route he never moves in 240 s (no go-to fires there; the
  `-distouchprobe` walk triggers nothing for the walkers). Route: 10845/22461 -> 10988/22152 (two-point path, 342 uu) at 116.8,
  123 u/s; back at 120.8; repeated at 144.0 / 177.3 / 181.7 / 185.8. Speed while walking 123 u/s; mean 2D speed over moving samples
  80.8 (the turns in place count).
- **Ride SSIM at the constant offset** (IK's rule: -42.831 + (56.40 - own soirée ENTER), `build/agentIK/rescore.py`): IO2a **0.648**
  (MAD 14.1, zoom 1.00), IO1a 0.631, and **IO1c - the same binary with the b08193f NPC behaviour - 0.645**. So the < .72 is not this
  package: on b08193f the ride camera framing differs from IK4c (0.726): the player camera sits lower / pitched (e.g. w85: captain's
  head 40 px higher than IK4c and retail). IP (2898de5, the out-of-breath head layer at weight 1) is the likely cause; it is not in
  this tree.

## 6. Gate

`run_regression.py --build-dir build/agentIO --no-build --exe-name DishonoredGame_IO.exe --only d3d9,inputtest,newgame`:

| | run 1 (`gate1.txt`) | run 2 (`gate2.txt`) |
|---|---|---|
| result | **23 ok, 0 failed** | 21 ok, 2 failed: `inputtest_moved` 404.8 / 403.5 (load) |
| `unported_natives` / `probe_natives` | 0 / 3 | 0 / 3 |
| criticals (d3d9 / inputtest / newgame) | 0 / 0 / 0 | 0 / 0 / 0 |
| `inputtest_moved` | 1018.9 | 404.8, retry 403.5 |
| `newgame_on_mission` / `newgame_pawns` | 119 / 29 | 107 / 29 |
| `d3d9_frames` | 1530 | 960 |

`inputtest_moved` is load-sensitive (HL measured 402.7 the same way): run 2 ran beside four `DishonoredGame_IRb1` and one IH4 game.
Inputtest alone again: `gate3.txt` 404.7 / 217.3 (same load), then `gate4.txt` **1030.3** with the reference `DishonoredGame_IH4.exe`
at 1008.7 in the next slot (`gate4ref.txt`). The player's walk does not go through any file of this package.

## 7. Where the brief was wrong, with the measurement

1. "FullMovePawn whole incl. the rotation write ... FaceTo on by default" was not the whole chain: the write is there, but the
   request that makes the delta non-zero (UpdateFaceTo) was never ported - every walker's face-to request count was 0 (IO1c).
2. "A mesh-vs-actor rotation offset is a classic cause": `Mesh->Rotation` is 0/0/0 on every walker in every run.
3. "Port the avoidance helpers if the odd routes come from avoidance": they come from the path builder - DN's mirrored left/right
   edge winding fed into retail's funnel step (§3).
4. "ComputeMoveYaw/ComputeTargetMoveYaw ported" - yes, but UpdateMoveYaw was still a stand-in that also wrote its third out into
   the wrong member (`m_fMaxMultiDirSpeedForAngle` instead of `m_fMaxMoveRotSpeedMultiplier`), and SetModifier used the modifier's
   distances as rates and never set `m_MaxMultiDirSpeedIdx` / `m_bHaveStopAnim`.
5. "Worker B (_3) on the L_Tower_P spawn with -distouchprobe's go-to": he does not walk on that route (240 s, 0 uu); his route is the
   ride route's Kismet go-to at 116.8.
6. "The turn animations' RMRM_RotateActor root rotation" is applied twice in our engine (stock ProcessRootMotion + the face-to root
   branch); retail's ProcessRootMotion only rotates the actor in RMM_Translate (§4.1).
7. "cutscene_compare ride SSIM >= .72": the b08193f tree itself reads 0.645 with this package switched off (IO1c).

## 8. Files
`source/Development/Src/DishonoredGame/`: `Inc/arkcomponentlocomotion.h`, `Inc/ibdiag.h`, `Inc/CppText/UArkComponentLocomotionConfig.h`,
`Src/arkanimnodelocomotion.cpp`, `Src/arkcomponentlocomotion.cpp`, `Src/arkcomponentlocomotiondynamic.cpp`,
`Src/arkcomponentlocomotionrootmove.cpp`, `Src/arkpathbuildutils.cpp`, `Src/dishonorednpcpawn_locomotion.cpp`. Sync:
`build/agentIO_sync.py` (base 4ff8c11, main HEAD at hand-back). IDA only on `build/agentIO/idb/` copies; never fmodel.
