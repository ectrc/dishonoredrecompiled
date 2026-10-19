# Agent IK: the captain's start-move loop (wave 26, PHASE23 package IK)

Worktree `build/agentIK_wt`, made by `make_snapshot.py IK` at main `97349ae` (= `63c3c91` + PHASE23 docs, no source difference).
Build dir `build/agentIK` (RelWithDebInfo). No commits. Two builds, both 0 errors, 0 C4263/C4264:
- **build 1** (`DishonoredGame_IK1.exe`): `-ikdiag` + one runtime switch `-ikfix` (the candidate retail return-to-the-mesh rule).
  The warm-up compile of the untouched tree and one resume after a C2661 (too many `debugf` arguments) are the same build.
- **build 2** (`DishonoredGame_IK.exe` = `DishonoredGame_IK2.exe`): the hand-back tree, switch removed.

Every run went through `cutscene_compare.py run` (`game_input.py launch`, `-windowed -nomovie -finewgame=50 -fosoiree`) via
`build/agentIK/ik_run.sh`; tables by `build/agentIK/ik_measure.py <log>`; re-scoring at a constant offset by `build/agentIK/rescore.py`.
Runs: `CC_IK1a` (build 1, HEAD behaviour), `CC_IK1b` (build 1, `-ikfix`), `CC_IK2` (build 2, world 50..175 every 1 s), `CC_IK2b`
(build 2, `-fvdiag -diskismet`). Decompiles: `build/agentIK/decomp2012/`, `build/agentIK/decomp2013/` (private IDA copies `build/agentIK/idb/`).

## 1. The diagnosis table (window world 150.6..170.6)

| | IK1a: HEAD behaviour | IK1b: retail return-to-the-mesh (`-ikfix`) | IK2: + retail navmesh frame (build 2) |
|---|---|---|---|
| `_26` start poly at the detach (10120.7/23022.6/2496.8, on the bow) | fails: ground trace hits `SkeletalMeshActorMAT_2` at z 2404.4, nearest mesh point **10092.2/23023.7/2318.8** (29 uu behind, 86 uu below) | same failure, same nearest point | **passes** (no `startpoly` line, 0 teleports in the whole run) |
| `m_bHasBeenOnNavMesh` / teleport | 1 / **teleported** to 10092/23024/2415 (into the boat) | 0 / no teleport | 0 / no teleport |
| `_26` root states | 8:151 7:30 6:17 9:21 0:2 (the start-move loop) | 0 only (no path: walks to the mesh point) | 0 only |
| START frames ("animation owns the pawn"): root-motion delta read by `ComputeDynamic` | **non-zero on 140 of 167 frames, mean 6.59 uu/frame (max 24.5), XY**; mesh `RootMotionMode` 1 (Velocity); child 8 `Empty_Walk_Start` w 1.00, Edge sequence, root = Edge locomotion joint 0 | - | - |
| dynamics velocity 2D (mean / max) | 60 / 116 u/s | 157 / 158 u/s (speed mode 1) toward the nearest mesh point | 0 |
| `FullMovePawn` moves: asked / got | 166 moves, **1153.7 uu asked, 0.0 got** | 332 moves, 3124.7 asked, 9.0 got | none (no velocity) |
| blocked (`Hit.Time < 0.1`) / start-penetrating | **166 / 166** | 329 / 0 | - |
| blocker | `SkeletalMeshActorMAT_2.SkeletalMeshComponent0`, physics asset `EmpressBoat.EmpressBoat_anim_PA`, normal -1.00/0.07/0 | same component, normal +1.00/0/0 (the boat behind him) | - |
| `_26` moved | 28.5 uu (the teleport), then 0 | 7.8 uu | 0 (stands on the bow, `build/cmp/IK2/ours/w0152.022.png`) |
| go-to state | path 17 points / 2447 uu (after the teleport) | no path, return-to-mesh | **request "arrived nopath", path -1/0, stage 0 for 20 s** (§4) |
| `_3` (Worker B) | leg 864 uu, interior 122.4 u/s; then stuck against `tower_waterlock_int_metal_pillar_01` (146/147 blocked), start loop | leg 873 uu, interior 123.0 u/s; then stuck against `tower_waterlock_wall_03` (226/226) | route changed: 10845/22461 -> 10976/22182 and back at 123 u/s, then against `DishonoredUsableObject_9` (wheel_01_PA, 446/446) |
| `gadiag pathfail` / teleports (whole run) | 41 / 41 | - | **18 / 0** |
| Critical | 0 | 0 | 0 |

### Reading it
1. **Cause (a) is refuted.** The start animation's root motion reaches the pawn: 6.6 uu per frame on 140 of 167 START frames (about
   140 u/s at the run's 21 fps), horizontal after `ProcessRootMotion`'s mesh-to-world transform. HC was right (§4.3 of agentHC.md):
   every NPC skeleton has `locomotionJointIndex 0`, Plan B differentiates that joint, and a forward non-wrapping start animation needs
   no `FLocomotionState`. IB's "root-motion delta 0" was the once-a-second `-gadiag` stack sample, which lands on the frame after
   `ComputeDynamic` zeroed `RootMotionDelta`; the samples in between (`rm 1 0.00/0.00/4.07`) carry it.
2. **Cause (b) is real but is not "the bow".** Every move is refused because HEAD's teleport put the captain **inside**
   `EmpressBoat_anim_PA` (start-penetrating on 166 of 166 moves). Retail would not have teleported him at all: 2012 `0x58a4d0` /
   2013 VA `0x949890` teleport only when `m_bHasBeenOnNavMesh` (+252 bit 0x100, 2013 +260) is set, and that bit is raised only by a
   successful search. The captain never had one.
3. **The teleport and the "off the mesh" verdict were both caused by our navmesh being 249 uu off.** `DisPylon_2` (L_Tower_P's
   imported mesh) serializes `LocalToWorld` = translation (249, 0, 0) with world-space verts (NavMeshVersionNum >= 28, so the
   Arkane load-time bake does not run). Retail `APylon::PostBeginPlay` (2012 `0x279cd0`, 2013 VA `0x65f8b0`, 165 B both) sets
   the nav and obstacle meshes' `LocalToWorld`/`WorldToLocal` to **identity** for `bImportedMesh` (@860 bit 0), `PylonMoved` is
   octree remove/add only (26 B both), and none of retail's poly functions has a transform (`GetPolyCenter` 2013 = 4 B,
   `GetVertLocation` returns the raw vert, `GetClosestPointOnPoly(const FVector&)` 479 B / `ContainsPoint(const FVector&, float)`
   2,393 B identical in both dbs). The reference installed the pylon actor's transform there (`InitTransform`), so every world
   query read L_Tower_P's mesh shifted +249 X. On the corrected mesh the captain's ground point on the bow is on a poly (IK2: no
   start-poly failure, no teleport) - which is what retail.mp4 shows: he walks forward off the bow at world 151
   (`build/cmp/final/pairs/pair_w0151.001.png`), never down into it.
4. With IK1b's retail rule but the shifted mesh, the return-to-the-mesh walk heads for a point behind and below him and the boat
   stops it - the measurement that sent me to the mesh frame.

## 2. What was ported (build 2)

| function | retail | change |
|---|---|---|
| `APylon::PostBeginPlay` (`Engine/Src/UnNavigationMesh.cpp`) | 2012 `0x279cd0` / 2013 VA `0x65f8b0` | imported mesh: identity transforms on nav + obstacle mesh; the reference bit `bNeedsTransform` cleared (identity makes it a no-op); `-ikdiag` line (`ikdiag pylon DisPylon_2 imported: identity transform (the actor's would move it by X=249.000 Y=0.000 Z=0.000)`) |
| `APylon::PylonMoved` | 2012 `0x27a3b0` / 2013 VA `0x6602a0` (26 B) | octree remove + add only |
| `FArkComponentLocomotion::UpdatePathOfHigherPriorityRequest`, start-poly failure step | 2012 `0x58a4d0` / 2013 VA `0x949890` | nearest mesh point looked up once per stretch off the mesh (150, then 300 only with `m_bHasBeenOnNavMesh`); teleport only with `m_bHasBeenOnNavMesh` and not disabled; `m_bHasBeenOnNavMesh` raised by a successful search (was: by any valid start poly) |
| `ComputeMoveYaw` | 2013 VA `0x93e1f0` (2012 `0x57f340`, 35 B) | `|m_fCurMoveSpeed| < 0.1 ? -200000 : m_CurMoveYaw` (HEAD: the path yaw) |
| `ComputeTargetMoveYaw` | 2013 VA `0x947950` (2012 `0x588700`) | by root state: path point while a path is computed and not arrived, else `m_NearestLocationOnNavMesh` while returning to the mesh, else the current yaw; turn states -> turn start/end yaw (HEAD: the path yaw always) |

Named deviations (in the code banners): the avoidance half of `ComputeTargetMoveYaw` (±`m_fAvoidanceAngle` via
`PointReachableZeroExtent` / `DetectPreAvoidanceCollision`) is not ported; retail's search radius doubles without bound, ours
stops at 300; `CanBeTeleportedToLocationWithoutBeingSeen` (2012 `0x57f460`) still not ported; `FullMovePawn` is still
`UWorld::MoveActor` - and its banner's claim that MoveActor steps up was wrong (§3).

## 3. Retail's FullMovePawn, read for (b) (2012 `0x580ae0`, 2013 VA `0x93fb00`, decompiles in both dirs)
The horizontal sweep starts MaxStepHeight above the cylinder (trace flags `0x30DF`, or `0x100030DF`), the floor is found within
3 x MaxStepHeight below, a start-penetrating pawn is moved out by `FindValidLocation` when it has no base or moves, the hit normal is
kept only when the hit is not a pawn, and a downward hit on a `USkeletalMeshComponent` floor refuses the move unless bit 0x20000 at
+288 of the hit's actor is set (the decompiler's labels there are not resolved). None of that is in our `MoveActor` path; with the
mesh frame fixed none of it was needed to keep the captain out of the hull, so it stays a named follow-up.

## 4. Acceptance (build 2)

| check | result |
|---|---|
| captain > 200 uu along his path in the 20 s after world 150.6, up the dock stairs | **not met: 0 uu.** He now stands on the bow where retail.mp4 has him at world 150, instead of sunk in the hull, but the go-to request never gets a path (`fvdiag goto tick ... loco req 1 path -1/0 arrived nopath`, 20 s; no `FindPath ... failed`, no start-poly failure) |
| walkers unchanged | `_3`'s speed is 123 u/s as before, but its **route changed** (§1); Worker B's 860-uu leg no longer happens on this route. Retail frames w0150..154 show a worker at the left wheel, where `_3` now stops; not decided |
| ride SSIM >= .72 | **0.735, MAD 10.23, zoom 0.99** at IK2's own constant offset (retail_t = world - 42.64, from its soiree ENTER 56.21 vs IB2's 56.40). The tool's own fit gave 0.634 because it accepted a slope b = 1.094; IB2 re-scored the same way at -42.831: 0.728 (`build/cmp/IK2_const`, `build/cmp/IB2_const`) |
| gate d3d9+inputtest+newgame | §6 |
| generator / audit | `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentIK/regen`: 79 of 79 byte-identical; `bitfield_audit.py` mismatches 0 |

### The next step (a third build, reported, not taken)
Why the corrected-mesh go-to gets no search. Prime suspect, from the log: at log 154.29 (before the detach) the captain's earlier
request (to his ride-start point -3790/36315) fails `FindPath` (error 1), which sets `m_CurReqWorkData.m_bPathfindAlreadyFailed`;
our scheduler then skips every search while that flag holds (`bNeedNewSearch` needs `!m_bPathfindAlreadyFailed` unless forced, and
the start-poly branch returns silently on it). On HEAD the teleport's `m_bForcePathComputation = TRUE` masked this. Retail has no
such gate: its `m_LastPFError` state machine (2012 `0x58a4d0`, LABEL_59..LABEL_118) re-searches when the destination it asked for
moved (GOALPOLYNOTFOUND / NOPATHFOUND / MAXIMUMVISITEXCEEDED compare `ComputeTargetedLocationForPath` with
`m_EndLocationUsedToAskPath`) and when the pawn's ground point moved (STARTPOLYNOTFOUND). Build 3 = port that state machine whole
(it is fully decompiled) plus one `-ikdiag` line per scheduler exit.

## 5. `-ikdiag`
For `_26` and `_3`, world 100..200:
- `ikdiag dyn <pawn> world frame root road anim|full rmread x/y/z rmm rmv curmove vel speed yaw/target pathyaw return been computed nearest at base | spec <child 8: seq w t prev rate play rbo rro edge>` - every `ComputeDynamic`.
- `ikdiag move <pawn> world frame root move actual time hit <actor> comp <class> <name> normal at penetrating base` - every `FullMovePawn` that sweeps.
- `ikdiag blocker <actor> comp ... collide block zero nonzero rigid | actor collide block physics | physicsasset/staticmesh <path> bounds` - once per blocking component.
- `ikdiag startpoly <pawn> ... ground (trace hit) nearest found teleport been disabled return` - first frame of each stretch off the mesh.
- `ikdiag pylon <name> imported: identity transform (...)` - once per imported pylon.

## 6. Gate
`run_regression.py --build-dir build/agentIK --no-build --exe-name DishonoredGame_IK.exe --only d3d9,inputtest,newgame`, twice, while
other agents built at 100 % CPU (`build/agentIK/gate1.txt`, `gate2.txt`):

| | run 1 (`IKg_`) | run 2 (`IKg2_`) |
|---|---|---|
| result | 22 ok, 1 failed | 21 ok, 2 failed |
| `unported_natives` / `probe_natives` | 0 / 4 | 0 / 4 |
| `d3d9_criticals` / `inputtest_criticals` / `newgame_criticals` | 0 / 0 / 0 | 0 / 0 / 0 |
| `d3d9_frames` (>= 1000) | 1560 | **930, re-run 900** |
| `inputtest_moved` (>= 800) | **332.8, re-run 384.9** | **216.0, re-run 217.2** |
| `newgame_on_mission` / `newgame_pawns` | 85 / 29 | 29 pawns |

Both failing metrics are the load-sensitive ones (IB measured 403 / 218 and 330..600 frames for its own tree under the same load);
the player's walk does not touch the navigation mesh or the NPC locomotion this package changed. The quiet-machine gate decides.

## 7. Where the brief was wrong
1. "root-motion delta 0" / cause (a): the delta is there (§1.1); `FLocomotionState` is not needed for this animation.
2. "MoveActor blocked at the bow by the boat's collision": blocked because HEAD teleported him **into** the boat's physics asset
   (penetrating 166/166); retail does not teleport a pawn that never had a path.
3. The real defect under both is the navmesh frame: L_Tower_P's imported mesh read +249 X off (retail: identity, both dbs). This
   also moved every other NPC's paths (`_3`'s route changed when it was fixed).
4. `ComputeMoveYaw` / `ComputeTargetMoveYaw` were stand-ins returning the path yaw; retail aims at the path point / the nearest mesh point.
5. The captain's "MAT_Detach position" was not the problem: he is detached where retail.mp4 shows him (`pair_w0150`).
6. `cutscene_compare.py`'s alignment fit can accept a slope (b = 1.094 on IK2) and halve the ride SSIM; read it at the constant offset.
