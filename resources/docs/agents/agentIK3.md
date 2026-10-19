# Agent IK3: the captain walks his path (wave 26, PHASE23 package IK3, continues IK2)

Worktree `build/agentIK3_wt` at `8254e4e`. That is the PHASE23 commit after IK2's `82e4f2d`, so `UnPhysic.cpp` is in it.
Build dir `build/agentIK3` (RelWithDebInfo). No commits.

Two builds, both 0 errors, 0 C4263/C4264:
- **build 1** (`DishonoredGame_IK3b1.exe`): retail's whole FullMovePawn, SendTouchAndBumpEvents and UnTouchActors. The
  warm-up of the untouched tree ran in the same invocation.
- **build 2** (`DishonoredGame_IK3.exe` = `DishonoredGame_IK3b2.exe`): adds the `-ikdiag floor` line, one per FullMovePawn.
  Same behaviour as build 1.

The merged tree itself was measured with `head_rel`'s exe, which is main with IK2 and `UnPhysic.cpp`, staged as
`DishonoredGame_IK3m.exe`.

Runs (`build/agentIK3/ik3_run.sh`, i.e. cutscene_compare -> `game_input.py launch`, `-windowed -nomovie`):
- `CC_IK3m`: the merged tree.
- `CC_IK3a`: build 1.
- `CC_IK3b`: build 2, with world 50..175 shots.
- `CC_IK3f`: build 2 with `-icfaceto`.

## 1. Result (window world 150.6..170.6)

| | IK3m: merged tree (IK2 + UnPhysic) | IK3a / IK3b: retail FullMovePawn | IK3f: + `-icfaceto` |
|---|---|---|---|
| start root motion after the turn | **read again**: 177 of 237 START frames non-zero (the RMM_Accel pair is gone) | 93/112, 136/169 | (no turn: FaceTo turns the body) |
| FullMovePawn moves, asked / got | 1118.8 / **8.5 uu**: 226 of 228 blocked, 221 start-penetrating in `EmpressBoat_anim_PA` (MoveActor) | 1062 / **794 uu** (IK3a); 732 / 690 (IK3b); 4 to 10 blocked, all by the player's cylinder | 406.5 / **406.5 uu**, 0 blocked |
| captain's path walked | 8.5 uu | 813 uu (IK3a), 684 uu (IK3b) | **1059 uu**, of which 781 uu straight-line |
| where he goes | stays at the bow | walks the boat to its bow tip; no FaceTo, so the start animation's heading is kept | **walks the boat toward the dock stairs like retail.mp4** (`build/cmp/IK3f/ours/w0153.005.png`, `w0154.014.png` vs `build/cmp/final/pairs/pair_w0152/0154`) |
| then | - | at X 10321 the floor sweep finds nothing within 3 x MaxStepHeight -> PHYS_Falling -> Z 1914, never lands | same at X 10321 (world 153.7) -> falls into the slip, Z 1919, PHYS_Falling to the end of the run |
| Worker B (`_3`) | against `DishonoredUsableObject_9`, 0 uu | same | same |

**Acceptance is half met.**
- With FaceTo on, the captain walks 1059 uu of his path in 3 s after the detach.
- Without FaceTo he walks 690..813 uu.
- He does not reach the stairs. Where the boat's bow ends (X ~10321, Y 23053), the path's third point is 10327/23057/2415, a
  navmesh poly 10 uu below the deck. The NPC floor sweep finds no collision there:
  - `ikdiag floor ... no floor within 3 x MaxStepHeight | hit none`;
  - the raised start is at centre Z 2551, and the sweep reaches centre Z 2461, so a floor at 2415 should be hit.
- The pawn then falls into the water. It never lands: PHYS_Falling for the rest of the run at Z 1919.

## 2. What was ported

| function | retail | |
|---|---|---|
| `FArkComponentLocomotion::FullMovePawn` | 2012 `0x580ae0` (4,707 B) / 2013 VA `0x93fb00` (4,729 B) | whole: body yaw, the move from Velocity, FindValidLocation and touches, the step-raised horizontal sweep with its slide, the floor sweep (3 x MaxStepHeight; steep-floor slope slide; skeletal floor only with bCanStepUpOn), the location / rotation write, base / Floor, components, untouch, zone, push body, the clamped Velocity, the root-motion consumption, the fall decision. No `UWorld::MoveActor` any more |
| `SendTouchAndBumpEvents` | 2012 `0x57f220` (275 B) | |
| `UnTouchActors` | 2012 `0x580a40` (155 B) | |
| `UpdateFloorAndBase` (agent FV's stand-in) | - | removed; it was a reconstruction of FullMovePawn's floor tail |

Named deviations:
- `UWorld::MoveAttachedActors` (2012 `0x25f970`, Arkane's 2,558 B) is the location write plus FarMoveActor for the attached
  actors: they follow the location, not the rotation.
- The pawn bit retail clears at the end (pawn +296, bit 0x1000) is not identified.
- `GetLedgeWalkMinFloorZ` is `WalkableFloorZ`; that is its whole body in 2012 (`0x2bd9b0`, 7 B).
- Retail tests "is a pawn" with `m_ActorTypeFlags` (byte @266); this tree never writes that byte, so `Cast<APawn>` is used.

Avoidance: `UNavigationHandle::PointReachableZeroExtent` (2012 `0x2a9ba0`, 1,781 B, Arkane navigation) and
`FArkComponentLocomotion::DetectPreAvoidanceCollision` (2012 `0x584f10`, 1,053 B) do exist under those names. They are plain
functions, but PointReachableZeroExtent is a navmesh walk with Arkane's `FNavMeshCheckResult`, and it did not fit in the build
that remained. They are named, not ported.

## 3. Still open, with the measurement
1. **The missing floor at the bow / stair foot.**
   - The navmesh has floor at 10327/23057/2415, but no collision answers the NPC's floor sweep there.
   - The sweep uses trace flags 0x100030DF, or 0x30DF after an NPC-only hit; both resolve to `m_bTraceForMove_NonPlayerPawn`
     (`FDisPrimTraceMask::MatchesTraceFlags`, 2013 rva 0x129990).
   - Next measurement: a downward zero-extent trace with the player's flags and with all blocking flags at the failing spot,
     naming what is there (gangway, stair foot, BlockingVolume) and its `m_CollisionTraceTypes`.
   - Retail.mp4 has him step onto the stairs at world 152..154, so in retail something under him answers the NPC's sweep.
2. **A falling NPC never lands.** He stays PHYS_Falling at Z 1919 after dropping into the slip, probably the water volume or
   NPC swim (IF's area). This was not looked at.
3. Without `-icfaceto` the start animation's heading is not corrected toward the path (IC's FaceTo is opt-in); the captain
   then walks the boat's axis.

## 4. Ride SSIM and gate
Ride SSIM depends on the offset at the ±0.01..0.02 level:

| offset | how it is chosen | SSIM | MAD |
|---|---|---|---|
| -42.821 | the agreed rule: IB2's -42.831 shifted by IK3b's own soiree ENTER 56.39 | **0.726** | 10.68 |
| -42.67 | IK2's choice | 0.735 | - |
| -42.75 | - | 0.747 | - |

The ride frames do not involve FullMovePawn; the boat NPCs are retargeted and PHYS_None. The spread is alignment, not the
move. The agreed rule gives 0.726, under the 0.73 bar.

Gate (`run_regression.py --build-dir build/agentIK3 --no-build --exe-name DishonoredGame_IK3.exe --only
d3d9,inputtest,newgame`, `build/agentIK3/gate1.txt`, `gate2.txt`). Both runs pass every check, **23 ok, 0 failed**:

| | run 1 | run 2 |
|---|---|---|
| `unported_natives` / `probe_natives` | 0 / 3 | 0 / 3 |
| criticals | 0 | 0 |
| `d3d9_frames` | 1860 | 1680 |
| `inputtest_moved` | 1059.1 | 1050.0 |
| `newgame_on_mission` / `newgame_pawns` | 119 / 29 | 119 / 29 |

The NPC walkers of the inputtest / newgame stages run through the new FullMovePawn without a critical.

All measurements are on `8254e4e` + this package. IL (`94d928d`: controller list and soirée stim on by default, Stop stim at
world 150.9) landed after both builds and is **not** in these runs.

## 5. Where the brief was wrong
1. "Does he move after the turn?" With the merged tree, root motion is back, but MoveActor stays inside the boat's physics
   asset: 8.5 uu. Retail's move fixes that; the step-raised sweep never touches the low parts of the hull.
2. "Reaches the dock stairs" depends on collision the NPC's floor sweep does not find at the stair foot, not on the move code.
3. The two avoidance functions exist in the PDB; the brief's "missing" was the tree's, not retail's.
4. Worker B was already stopped at the wheel on main; this package does not change him.
