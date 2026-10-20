# Agent IO2: turn loops, the bow stall, the avoidance half, the off-mesh NPCs (wave 26, PHASE23 package IO2)

Worktree `build/agentIO2_wt` at `d947504` (main with IO + the ProcessRootMotion rule merged). Build dir `build/agentIO2`
(RelWithDebInfo). No commits. Two builds, both 0 errors and 0 C4263/C4264 (`build/agentIO2_build1.log`, `agentIO2_build2.log`):
- build 1 (`DishonoredGame_IO2b1.exe`): PointReachableZeroExtent and its helpers, UpdateMoveYaw's wall test, ComputeTargetMoveYaw's
  avoidance half, IsEdgeBorderExtentReachable on the zero-extent walk, `-io2watch=<pawn>` for `-ikdiag`;
- build 2 (`DishonoredGame_IO2.exe`): adds retail's non-uniform-scale box path in `FKAggregateGeom::LineCheck` (§3).
The merged tree was measured with `head_rel`'s exe (built from `d947504`), staged as `DishonoredGame_IO2m.exe`.

Runs (`game_input.py launch`, `-windowed -nomovie`; scripts `build/agentIO2/io_run.sh`, `io_tower.sh`; tables `io_table.py`, which
now splits off the samples inside a turn animation, root states 1..5):

| run | exe | route |
|---|---|---|
| IO2m / IO2mt | merged tree | ride (50..175 shots) / L_Tower_P spawn, 200 s |
| IO2a | build 1 | ride, `-io2watch=DishonoredNPCPawn_8` |
| IO2probe | build 1 | L_Tower_P spawn, `-indiag`, `INPROBE -246 33039 580 400` |
| IO2b, IO2c | build 2 | ride |
| IO2t | build 2 | L_Tower_P spawn, 200 s |

## 1. The merged tree (step 1)

The turn loops are gone with the ProcessRootMotion rule. |Rotation.Yaw - velocity yaw| over samples faster than 60 u/s:

| NPC | IO2m all: n / mean / <15 | outside turns | IO2mt (tower) all | outside turns |
|---|---|---|---|---|
| _3 Worker B | 9 / 10.1 / 88% | 8 / 0.1 / 100% | - | - |
| _4 | 13 / 2.6 / 84% | 10 / 0.0 / 100% | - | - |
| _8 | 4 / 84.7 / 0% | 4 / 84.7 / 0% | 4 / 90.6 / 0% | same |
| _9 | 40 / 0.1 / 100% | 40 / 0.1 / 100% | - | - |
| _12 | 13 / 0.0 / 100% | 12 / 0.0 / 100% | 28 / 3.7 / 96% | 23 / 4.3 / 95% |
| _15 | 2 / 11.2 / 50% | same | 2 / 4.7 / 100% | same |
| _16 | 4 / 91.7 / 25% | same | 4 / 86.0 / 0% | same |
| _19 | 5 / 83.5 / 0% | same | 5 / 77.0 / 0% | same |
| _26 captain | 23 / 8.9 / 91% | 22 / 9.2 / 90% | - | - |

Worker B: straight legs again, 10845/22461 -> 10983/22164 (118..120) and back (122..124), again 150.7..153.8, 123 u/s.
Captain: on the dock floor (base WorldInfo) at 157.95, arrived 176.4 - **the 156..174 bow stall did not happen on the merged
tree**: it came from the double root rotation (the start animation carried him off the line), not from a missing wall test.
Ride SSIM at the constant offset (-42.831 + (56.40 - ENTER)): **0.839**.

## 2. The avoidance half (step 2, build 1)

| function | retail | |
|---|---|---|
| `UNavigationHandle::PointReachableZeroExtent` | 2012 0x2a9ba0 (1,781 B) | whole: start poly (cache / GetPylonAndPolyFromPos), poly-to-poly walk along the 2D segment, blocked-edge (CostFor < 1 or >= BLOCKEDPATHCOST) and border hits, the final 2D + height-band test; `Engine/Src/UnNavigationHandle.cpp`, declared in `EngineAIClasses.h` |
| `FNavMeshCheckResult` | 2012 PDB, 32 B | `Engine/Inc/arknavmeshbuildutils.h` (the flag bit's name is ours: `m_bHit`) |
| `ArkNavMeshComputeHitPropertiesFromEdge` / `FromPoly` | 0x53d5f0 / 0x53d840 | whole |
| `SegmentIntersect2D`, `DistanceToLine` | 0xd730 / 0xd670 (Core unmath.cpp) | `Core/Src/UnMath.cpp`; IO's file-static copy of DistanceToLine removed |
| `FArkComponentLocomotion::UpdateMoveYaw` wall test | 0x582300 | front probe along the floor-projected move yaw, lateral probe to the turn side; emergency / rotation-speed multipliers |
| `ComputeTargetMoveYaw` avoidance half | 0x588700 | unstuck distance, move-hit-normal swing by +-m_fAvoidanceAngle on the side a corner probe reaches, else the pre-avoidance side |
| `IsEdgeBorderExtentReachable` | 0xacd50 | now on PointReachableZeroExtent (IO's named deviation closed) |
| `DetectPreAvoidanceCollision` | 0x584f10 | **FALSE**: its data is FArkComponentAvoidance's avoidable pairs, and that component (2012 0x574750..0x578ec0) is a comment-only stub in this tree - retail's "no avoidance component" exit |

Named deviations: the single-affector branch of ComputeTargetMoveYaw reads the avoidable owner's tweaks through an unresolved
folded accessor; it is unreachable while no pawn has an affector (avoidance unported). PointReachableZeroExtent's "override start
more than 150 uu away" test compares the override with itself in retail (the start was replaced first) and never fires - kept.

Measured (IO2a): the wall test fires (multiplier 2.95 / 3.00 / 5.50 on 9 of 1,428 samples, 1.00 elsewhere); the captain is on the
dock floor at ~157.5 and arrives at 174.8; ride SSIM 0.830.

## 3. The off-mesh NPCs _8 / _16 / _19 (step 3, build 2)

`-ikdiag -io2watch=DishonoredNPCPawn_8` (IO2a): `_8` is spawned by `DishonoredSpawner_24` (L_Tower_Script, GuardCaptain tweaks) at
-246/33039/580, i.e. on the navmesh (nearest poly Z 585.7) - but **nothing collides under it** (all five downward probes: nothing).
It falls 2,100 uu onto `StaticMeshCollectionActor_22` at Z -1617, stays off the mesh, and "walks back to the mesh" for 5 s at 157 u/s
around a point (the 84..93 deg samples). `_16`, `_19` are the same (spawned at Z 471 / -714, end at -1419 / -1499).

What should be there (cooked packages, `build/agentIO2/pkgbounds.py`): L_Tower_Script's hidden collision platforms -
`StaticMeshCollectionActor.StaticMeshActor_SMC_2` and `StaticMeshActor_2`, `metal_platforms_01.platform_small_01`, HiddenGame,
Scale3D (7.4, 9.0, 1.0), box covering X -943..1457, Y 32725..37431, Z 565..583 - the deck the guards stand on next to the whale ship
(`whaleship_main_01` itself has CollideActors 0). `INPROBE` (IO2probe): the component is attached, in the octree (3 nodes), collide /
block 1, masks np/npc/pl 1/1/1, ShouldCollide 1, its body is **one box element**, and its own traces: zero extent **miss**, NPC
extent **miss**, complex **HIT z 584**.

Cause: the stock `FKAggregateGeom::LineCheck` tests box / sphere / capsule elements only under a uniform Scale3D. Retail's
(2012 0x3dad50, 5,656 B) keeps a non-uniform branch: every box element whose own TM is unrotated and unscaled is tested with its
extents scaled by |Scale3D| and its offset by Scale3D, at scale 1. Ported (`Engine/Src/UnPhysCollision.cpp`). Not ported: the same
question for `FKAggregateGeom::PointCheck` (not read).

After (IO2b / IO2c): `_8` stands at -246/33039/676 on the platform, gets a path and walks it (19 samples, 0.0 deg); `_16` at
-4319/35889/471 and `_19` at -1022/33702/-714 stand (speed 0, no off-mesh walk).

## 4. Acceptance (build 2)

| NPC | IO2b all: n / mean / <15 | outside turns | IO2c all | outside turns |
|---|---|---|---|---|
| _3 Worker B | 9 / 1.9 / 100% | 6 / 0.0 / 100% | 8 / 9.5 / 87% | 7 / 0.1 / 100% |
| _4 | 12 / 1.0 / 100% | 11 / 1.0 / 100% | 14 / 0.4 / 100% | 12 / 0.4 / 100% |
| _8 | 19 / 0.0 / 100% | 19 / 0.0 / 100% | 19 / 0.0 / 100% | 19 / 0.0 / 100% |
| _9 | 22 / 0.2 / 100% | 22 / 0.2 / 100% | 21 / 0.4 / 100% | 21 / 0.4 / 100% |
| _12 | 23 / 6.1 / 86% | 16 / 4.1 / 93% | 13 / 7.5 / 76% | 12 / 4.9 / 83% |
| _15 | 2 / 18.6 / 50% | same | 2 / 10.7 / 50% | same |
| _25 | - | - | 1 / 0.0 / 100% | same |
| _26 captain | 20 / 1.1 / 100% | 20 / 1.1 / 100% | 24 / 5.2 / 91% | 22 / 2.0 / 95% |

Tower route (IO2t): `_12` 10 / 1.5 / 90%, `_15` 2 / 20.2 / 50%; no off-mesh walkers.

**Under 15 deg for all samples outside turns: not met for 6 samples of 3 NPCs** (both ride runs). Every one is a transient the
locomotion produces in retail's states too: the first second of a walk while the face-to request turns the body at its rotation
speed (`_15` 52.06 / 52.12: 21 / 37 deg, then 0), START_END / STOP_BEGIN root-motion states where the body is the animation's (`_12`
root 9 / 12: 15 / 41 deg), and two corners (`_12` 68.11 39 deg, `_26` 154.16 17 deg, the body still turning at the corner).
Turn-animation samples (root 1..5) separately: `_3` 3 samples, `_12` 7 / 1, `_26` 2 (IO2b / IO2c).

- **Captain**: dock floor at 157.50 (IO2c) / by 159.6 (IO2b; on the bridge at 156.4), arrived 172..175; facing his way (1.1 / 2.0 deg
  mean outside turns).
- **Worker B**: 10857/22450 -> 10988/22152 at 123 u/s (118.8..119.9), back 122..125, again 151..155; straight legs.
- **Ride SSIM** at the constant offset: **IO2c 0.826** (MAD in `build/cmp/IO2c_const`); IO2a (build 1) 0.830; merged tree 0.839.
  IO2b read 0.181: its captures from world ~55 on are the lower half black (`build/cmp/IO2b_const/pairs/pair_w0089.016.png`) - a
  capture defect of that run, not the game; IO2c on the same exe is the measurement.

## 5. Gate

`run_regression.py --build-dir build/agentIO2 --no-build --exe-name DishonoredGame_IO2.exe --only d3d9,inputtest,newgame`, twice
(`build/agentIO2/gate1.txt`, `gate2.txt`), beside other agents' games:

| | run 1 | run 2 |
|---|---|---|
| `unported_natives` / `probe_natives` | 0 / 3 | 0 / 3 |
| criticals (d3d9 / inputtest / newgame) | 0 / 0 / 0 | 0 / 0 / 0 |
| `inputtest_moved` (first / retry) | 403.6 / 397.1 FAIL | 217.4 / 397.1 FAIL |
| `d3d9_frames` | 690 / 900 (low) | 1050 |
| `newgame_on_mission` / `newgame_pawns` | 111 / 29 | 89 / 29 |

`inputtest_moved` is the load-sensitive metric (HL and IO measured ~400 the same way): the failing runs' W hold lasts as long in world
time but the pawn covers ~200..400 uu. A/B in one quieter slot (`ab_IO2b1.txt`, `ab_IO2.txt`): build 1 (without the LineCheck change)
**1013.2**, build 2 **1015.1** - the collision change does not touch the player's walk. Both runs are reported as measured; the
load-sensitive metric did not pass in either full gate run.

## 6. Where the brief was wrong, with the measurement
1. "The captain's bow stall at 156-174 (the straight path steps 40 uu east)": it does not happen on the merged tree (IO2m: dock floor
   157.95) - it was the start animation's double root rotation, fixed by the ProcessRootMotion rule; the wall test changes nothing
   measurable there.
2. "DetectPreAvoidanceCollision (0x584f10) - the avoidance half": it is pawn-vs-pawn prediction over FArkComponentAvoidance's pairs,
   a stub in this tree; it cannot act on static geometry, and without the component retail's own exit is FALSE.
3. "Off-mesh NPCs: spawn point vs navmesh, a teleport-on-spawn or a pylon transform": the spawners stand on the navmesh; the floor
   that carries them is a hidden, non-uniformly scaled collision platform whose box element our LineCheck skipped (retail tests it).
4. "Ride SSIM >= .80": met (0.826 .. 0.839) except one run with broken captures.

## 7. Files
`Core/Inc/UnMath.h`, `Core/Src/UnMath.cpp`, `Engine/Inc/EngineAIClasses.h`, `Engine/Inc/arknavmeshbuildutils.h`,
`Engine/Src/arknavmeshbuildutils.cpp`, `Engine/Src/UnNavigationHandle.cpp`, `Engine/Src/UnPhysCollision.cpp`,
`DishonoredGame/Inc/arkcomponentlocomotion.h`, `DishonoredGame/Src/arkcomponentlocomotiondynamic.cpp`,
`DishonoredGame/Src/arkpathbuildutils.cpp`, `DishonoredGame/Src/dishonorednpcpawn_locomotion.cpp`. No generated files.
bitfield_audit: mismatches 0. IDA only on `build/agentIO2/idb/` copies (`build/agentIO2/decomp_va.py`, headless); never fmodel.
