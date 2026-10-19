# Agent IK2: the captain walks his path (wave 26, PHASE23 package IK2, continues IK)

Worktree `build/agentIK2_wt`: made at `f90db27` (= IK's merge `8dcbd03` + docs), fast-forwarded to `b83f983` (IC) before
the first build as the coordinator asked. Build dir `build/agentIK2` (RelWithDebInfo). No commits.

Two builds, both 0 errors and 0 C4263/C4264:
- **build 1** (`DishonoredGame_IK2b1.exe`): the scheduler state machine, the PreAsyncWorkTick bit, ResetPathProperties, the
  teleport, FullMovePawn's body yaw and hit handling, and `ArkIsPointOnNavMesh`. The first link stopped on a missing include in
  `arkutils.cpp`; the resume is the same build.
- **build 2** (`DishonoredGame_IK2.exe` = `DishonoredGame_IK2b2.exe`): adds `FindValidLocation`, removes the reference's
  RMM_Accel physics tick in `ProcessRootMotion`, and logs path points.

**The hand-back tree is build 2 plus one unbuilt edit** (`UnPhysic.cpp`, §3). Do not merge it without a third build.

Runs:
- `CC_IK2a`: build 1, ride + post-ride.
- `CC_IK2f`: build 1, `-icfaceto`.
- `CC_IK2c`: build 2, ride + post-ride.
- `CC_IK2d`: build 2, `-icfaceto`.

All runs used `build/agentIK2/ik2_run.sh` (cutscene_compare -> `game_input.py launch`, `-windowed -nomovie`). Tables come from
`build/agentIK/ik_measure.py`; the per-exit scheduler lines are `ikdiag sched`.

## 1. Result

| | IK HEAD (`8dcbd03`) | IK2a: build 1 | IK2c: build 2 |
|---|---|---|---|
| captain's go-to request | `arrived nopath` for 20 s: the old request's FindPath failure burnt `m_bPathfindAlreadyFailed`, and nothing re-searched | **path found at world 150.53**: 16 points, 2420 uu, 10112/23023 -> 10400/22947 -> ... -> 12535/22885 (the dock stairs side); `PATHFINDING_SUCCEED` told to the go-to | same path |
| root states after the detach | 0 | turn (TurnToWalkRight180, he faced -X after walking back toward the mesh before the detach), then the start-move loop 6/7/8/9 | turn, then root 0 with velocity 157 u/s along the path yaw (-2672) |
| root motion in START | - | **0 on all 296 frames**: the turn left the mesh in RMM_Accel, and the reference's "tick physics right away" ran FullMovePawn inside the animation update, which zeroed the delta | no START (the turn ends in TURN_END / NONE) |
| FullMovePawn moves (world 150.6..170.6) | - | 71, blocked 71, penetrating 67 (boat physics asset) | **0: physics never runs.** The mesh stays RMM_Accel and the reference's `APawn::performPhysics` skips physics in RMM_Accel (§3) |
| captain moved | 0 | 0.4 uu | 0 uu |
| `_3` (Worker B) | 10845/22461 -> 10976/22182 and back at 123 u/s, then against `DishonoredUsableObject_9` | same | same; speed 123 u/s unchanged |
| ride SSIM / MAD at the constant offset | 0.735 / 10.23 | **0.737 / 10.55** (offset -42.67) | **0.733 / 10.73** (offset -42.67) |

With `-icfaceto` (IK2f / IK2d) there is no turn animation, so the mesh stays RMM_Velocity. The start animation's root motion is
then read again (261 of 347 frames non-zero). But the root-motion move runs along the pawn's -X facing into the boat's physics
asset and is refused (348 of 348 blocked, 345 penetrating). So with FaceTo on, the captain plays the start loop in place.

**Acceptance is not met: the captain does not move.** The scheduler, which was this package's goal, works: he gets his path and
heads along it.

What stops him is two pieces of the reference engine that retail does not have, plus retail's own move:
1. **The RMM_Accel physics pair.**
   - The reference has two halves. `USkeletalMeshComponent::ProcessRootMotion` ticks the owner's physics right after the
     animation when RootMotionMode is RMM_Accel. `APawn::performPhysics` skips its regular physics in RMM_Accel.
   - Retail has neither half: 2012 `0x2bd2a0` / 2013 VA `0x6a2230` (517 B both) start with `CheckStillInWorld`, and 2012
     `0x353c80` / 2013 VA `0x746280` never call `performPhysics`.
   - Build 2 removed the ProcessRootMotion half. The performPhysics half is removed in the tree but **not built** (§3).
   - Retail's turn leaves the mesh in RMM_Accel too (`UpdateTurn` 2012 `0x58c520`; nothing resets the mode in `ResetTurn`,
     `SetRootState` or `UpdateMesh`). So without the pair every NPC that ever turned stops getting physics (IK2c) or loses its
     root motion (IK2a).
2. **Retail's FullMovePawn move itself** (2012 `0x580ae0`, decompiled).
   - The horizontal sweep starts MaxStepHeight above the cylinder. The floor is found within 3 x MaxStepHeight below.
   - Ours sweeps `UWorld::MoveActor` from the cylinder itself. Moving off the bow, it is refused by low parts of
     `EmpressBoat_anim_PA`: start-penetrating, with the step-raised `FindValidLocation` box finding nothing to push out of.
   - Retail's raised sweep clears those parts.
   - This is the next port: FullMovePawn's horizontal + vertical half, about 3 KB of the 4.7 KB function.

## 2. What was ported

| function | retail | notes |
|---|---|---|
| `UpdatePathOfHigherPriorityRequest` | 2012 `0x58a4d0` / 2013 VA `0x949890` | the whole `m_LastPFError` state machine (banner lists every rule): repath on stuck / target moved beyond `m_fRepathCoef` / path at end / the after-turn sideways test with `PointReachable`; START/ANCHOR (walk back, re-verify, give up after `m_fMaxDurationToReturnOnNavMesh`, else clear when the ground point moved); GOAL/NOPATH (clear when the target left `m_fEndLocationThreshold`, or a GOAL target is now on the mesh); MAX (verify, off-mesh nearest point / teleport rule); the search with retail's straight-line `PointReachable` short-circuit and budget; SUCCEED / FAILED once per request. Replaces `m_bPathfindAlreadyFailed` as a gate |
| `ArkIsPointOnNavMesh` | 2012 `0xacfa0` (45 B) | `Engine/Src/arkutils.cpp`, declared in `arknavmeshbuildutils.h` |
| `ResetPathProperties` | 2012 `0x586320` | flag word `& 0xFFFFFF01 | 0x20`: `m_bTryToReturnOnNavMesh` kept (HEAD cleared it), `m_bForcePathComputation` and `m_bMustTestPathAfterTurn` cleared |
| `PreAsyncWorkTick` path reset | 2012 `0x58d5f0` | tests +252 bit 0x40 (`m_bForcePathComputation`, the bit `ForceActiveRequestRepath` / `OnEnable` / `HandleTeleport` set); HEAD tested 0x8, the scheduler's own repath bit |
| teleport branch | 2012 `0x58d5f0` | the scheduler adds the half-height; the bit clears only when the move happened; no forced recompute |
| `OnRequestManagerEvent` FIRST_REQUEST_HAS_CHANGED | 2012 `0x587350` | no dirty bit (retail sets none) |
| `FullMovePawn` head | 2012 `0x580ae0` | body yaw: `(0, Normalize(Rotation.Yaw + FaceTo m_CurBodyYawDelta), 0)` with FaceTo, and a rotation change alone runs the move; hit normal only for non-pawn hits; HEAD's stuck flag / HIT_OBSTACLE / burnt "failed" bit removed (retail's stuck flag is the dynamics' velocity test) |
| `FindValidLocation` + its call | 2012 `0x57ffb0` (745 B) | the un-sticking step before the move (`move || !base`), check box raised by MaxStepHeight, XY x 1.001 (0.5 with inertia disabled) |
| `ProcessRootMotion` | 2012 `0x353c80` / 2013 VA `0x746280` | the reference's RMM_Accel immediate `performPhysics` removed |

Named deviations:
- `EPathFindingError` is the tree's enum: retail's goal group maps to {GOALPOLYNOTFOUND, NOPATHFOUND}.
- The reference `PointReachable` has no Arkane crossed-edge out-list, so straight-line paths reserve no edges.
- The radius doubling stops at 2400 uu.
- With FaceTo off (`-icfaceto` is off by default), the pawn faces its move yaw (HEAD's stand-in).
- `SendTouchAndBumpEvents` is not ported.
- `CanBeTeleportedToLocationWithoutBeingSeen` is not ported.
- The avoidance half of `ComputeTargetMoveYaw` was **not done**: it needs `UNavigationHandle::PointReachableZeroExtent` and
  `DetectPreAvoidanceCollision` (2013 `0x543f70`), neither of which exists in the tree.

## 3. The unbuilt edit in the hand-back tree
`Engine/Src/UnPhysic.cpp` `APawn::performPhysics`: the reference's RMM_Accel skip is removed (retail: 2012 `0x2bd2a0` / 2013 VA
`0x6a2230`). It is the second half of the pair whose first half build 2 removed.

Without it, build 2's tree regresses every NPC that has played a turn animation, because they never get physics again. The
coordinator has two choices:
- take a third build with this edit (expected: the captain walks the 16-point path as far as `MoveActor` lets him off the bow);
- or revert the `UnSkeletalComponent.cpp` hunk, which puts the tree back to build 1's root-motion behaviour.

## 4. Gate (build 2, before the unbuilt edit)
`run_regression.py --build-dir build/agentIK2 --no-build --exe-name DishonoredGame_IK2.exe --only d3d9,inputtest,newgame`, twice
under the usual load (`build/agentIK2/gate1.txt`, `gate2.txt`):

| | run 1 | run 2 |
|---|---|---|
| result | 21 ok, 2 failed | 22 ok, 1 failed |
| `unported_natives` / `probe_natives` | 0 / 3 | 0 / 3 |
| `d3d9` / `inputtest` / `newgame` criticals | 0 / 0 / 0 | 0 / 0 / 0 |
| `d3d9_frames` (>= 1000) | 870, re-run 660 | 990, re-run 840 |
| `inputtest_moved` (>= 800) | 592.7, re-run 369.7 | 398.9, re-run **1063.4** |
| `newgame_pawns` | 29 | 29 |

The two failing metrics are the load-sensitive ones (run 2's re-run passes `inputtest_moved` at 1063.4).

## 5. Where the brief was wrong
1. "Re-search when the destination changes" was the scheduler half. The captain then gets his path at once, but two reference
   engine features retail lacks (the RMM_Accel physics pair) and retail's own step-raised FullMovePawn sweep decide whether he
   walks it.
2. "Worker B unchanged (123 u/s)": `_3`'s speed is unchanged, but since IK's navmesh fix his route ends against the wheel
   usable object; this package does not change that.
3. The avoidance half of ComputeTargetMoveYaw depends on two navigation functions the tree does not have.
4. Our ResetPathProperties cleared `m_bTryToReturnOnNavMesh`, and PreAsyncWorkTick reset the path on the wrong bit.
