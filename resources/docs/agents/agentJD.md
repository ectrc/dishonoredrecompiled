# Agent JD: NPC walking - path properties, Arkane PointReachable, avoidance, moderate move (wave 27, PHASE24 package JD)

Worktree `build/agentJD_wt` at `96cdf21` (main moved on with JA `810f13a`, JB `d7123aa`; none of them touches a file of this
package). Build dir `build/agentJD` (RelWithDebInfo). No commits. Two builds, 0 errors, 0 C4263/C4264:
- build 1 (`DishonoredGame_JD1.exe`; the first invocation stopped on one C2661 - a 30-argument `debugf` - fixed and resumed:
  `build/agentJD_build1.log`, `agentJD_build1b.log`): everything below except §6, with the A/B switches `-jdoldpath` (DN's
  UpdatePathProperties / ComputePathYawFromDist / corner blend), `-jdnoavoid`, `-jdoldreach`;
- build 2 (`DishonoredGame_JD.exe` = `DishonoredGame_JD2.exe`, `agentJD_build2.log`): build-1 switches removed, ModerateMovePawn
  (switch `-jdnomoderate`), bJustTeleported. **One edit after build 2 is unbuilt: MovePawn's conditions (§6).**

Runs (all `game_input.py launch`, `-windowed -nomovie`, never real input): ride route `build/agentJD/jd_run.sh` (cutscene_compare
`-finewgame=50 -fosoiree`, `-iodiag -ibdiag -ikdiag -jddiag`), L_Tower_P spawn route `build/agentJD/jd_tower.sh` (200 s).
Tables: `build/agentJD/jd_measure.py`, `jd_dev.py` (deviation from the route polyline, heading rate), IO2's `io_table.py`.
Decompiles in `build/agentJD/d12`, `d13` (private IDA copies `build/agentJD/idb/`); config dump `build/agentJD/tools/lococonfig.py`.

| run | exe | switches | route |
|---|---|---|---|
| JD0a, JD0c (JD0b: captures 144x73, void) | head_rel (`2127b56`, staged `DishonoredGame_JD0.exe`) | - | ride 50..200 |
| JD1a | JD1 | - | ride 50..200 |
| JD1b | JD1 | `-jdoldpath -jdnoavoid -jdoldreach` | ride 140..200 |
| JD1t | JD1 | `-jdwatch=_12` | tower |
| JD2a | JD2 | - (moderate move on) | ride |
| JD2b | JD2 | `-jdnomoderate` (= the hand-back tree's behaviour for walking NPCs, §6) | ride |
| JD2t | JD2 | `-jdnomoderate -jdwatch=_12` | tower |

## 0. Pylon census (`-jddiag census`, JD1a) - decides JI

```
census pylon DisPylon_2 class DisPylon level TheWorld at 249/0/0 imported 1 disabled 0 polys 668 box -4902/19204/-938..23186/37680/3821 obstacle polys 2532
census 1 pylons, 0 dynamic
```
One pylon in the loaded world (L_Tower_P's imported mesh, IK's identity-transform pylon), **no AArkDynamicPylon**. Every point of every
route of the captain and Worker B lies on DisPylon_2 (`jddiag route ... DisPylon_2(DisPylon)` for all points). **JI is not needed.**

## 1. What was ported

| function | retail | |
|---|---|---|
| `UpdatePathProperties` | 2012 0x582eb0 / 2013 VA 0x941ee0 (2,499 B both) | whole. The current path direction is the SEGMENT's (prev corner -> corner), not pawn -> corner; a corner is passed within m_fIntermediatePointValidDist, or when it is behind the pawn along the segment, or when the next corner is visible through the corner's navmesh edge (cross tests against the edge ends from the pawn); after one corner, further corners are passed while the corner after is `PointReachable` with the pawn's extent; m_fDistBetweenPathPoints is 3D |
| `ComputePathYawFromDist` | 2012 0x583880 / 2013 VA 0x9428b0 (614 B) | whole: m_CurPathYaw at the path end or while the distance stays in the first stretch, else the 2D yaw of the segment it ends in |
| `ComputePathMove` | 2012 0x5891c0 / 2013 VA 0x948410 (913 B) | whole. The security-radius blend towards the corner applies only on the LAST stretch (path-at-end bit 0x20, both dbs) and writes m_CurMoveYaw; DN blended at every intermediate corner |
| `UNavigationHandle::PointReachable` (Arkane, out-list) | 2012 0x2af580 (1,194 B) | whole: the extent box's four corners walked with PointReachableZeroExtent (the rearmost along the move skipped), crossed edges collected; the reference virtual stays as the script / reference entry and answers with it. The scheduler's straight-line case now copies the crossed edges into the raw edges as retail |
| `FArkComponentAvoidance` | 2012 0x5772e0 ctor, 0x5773d0 dtor, 0x575340/0x575370 Starting/Stopping, 0x5753b0, 0x577430 GetAvoidanceComponent, 0x577450/0x577470 Register/Unregister, 0x578ec0 PreAsyncWorkTick (1,406 B) | whole (constants 80 / 3 / 300 / 500 / 10 / 100 from 2012 0xe33308..). Class in `Engine/Inc/arkcomponentavoidance.h`, defined in `DishonoredGame/Src/arkavoidance.cpp` |
| `UArkAvoidable` methods | 2012 0x526c80..0x529d00 | whole; `Inc/CppText/UArkAvoidable.h` (shim header regenerated: one include line, 79/79 byte-identical) |
| `AGameInfo::PostBeginPlay` | 2012 0x2ec070 (31 B) | adds and starts the avoidance component (`jddiag gameinfo DishonoredGameInfo container yes components 1`) |
| `DetectPreAvoidanceCollision` | 2012 0x584f10 (1,053 B) | whole (IO2 had retail's no-component exit only) |
| `ComputePriorityOfNPCsFromSharedProps` | 2013 rva 0x5491b0 = 2012 0x589e00 (1,737 B) | whole; component 0 runs it once a frame |
| `HandlePushPeriod` | 2013 rva 0x548b90 = 2012 0x589940 (1,216 B) | whole (the brief's "push period"); `FArkCpntLocoGoToLocationProp::InitAllByDefaultExcept` 2012 0x582e00 |
| `ApplySteering` | 2012 0x58b110 (1,044 B) | whole (DN's was a reconstruction without the avoidance force) |
| `UpdateSharedProperties` | 2012 0x586f60 | whole (DN wrote other fields into the row) |
| `ComputeTargetMoveYaw` single-affector branch | 2012 0x588700 | IO2's unresolved folded accessors are GetStrongerColAvoidable (+84) / GetPropertyFlags (+116) |
| `ComputeDynamic` | 2012 0x58ca10 | corrections: the stuck test only with a move hit normal and no avoidance force; the push clamp uses m_fCurAcceleration; m_fSteeringMultiplier zeroed on the animation road; m_PreviousSteeringForce rolled over |
| `PreAsyncWorkTick` | 2012 0x58d5f0 | m_bSomeoneWantsToPushMe; priorities; **m_bStartingNewPath cleared after ComputeDynamic** (it stayed raised for the whole path, so EvaluateStartPathTurn could fire anywhere on it) |
| NPC `UpdateLocomotion`, `RegisterAvoidable`, `UpdateAvoidableCollisionGroupFlags` | 2012 0x7b84a0 / 0x7c1ab0 / 0x7c0450 | whole; registered in PostBeginPlay, unregistered in ShutPawnDown_Derived (`jddiag avoidable ... registered 1 radius 10.0 force 1.00 groups 2`) |
| `ModerateMovePawn` | 2012 0x581d50 / 2013 VA 0x940d80 (1,447 B) | whole (SendTouchAndBumpEvents / UnTouchActors are IK3's), `-jdnomoderate` |
| `FullMovePawn` tail | 2012 0x580ae0 | IK3's unidentified "pawn +296 bit 0x1000" is `AActor::bJustTeleported` (2012 PDB); cleared |

Named deviations: FArkComponentAvoidance / UArkAvoidable are defined in DishonoredGame (retail: Engine) and the game info creates the
component by id; NPC "in combat" is the brain's awareness (the pawn virtual IsInCombat is not declared here); the push game event
(FArkGameEventDispatcher type 1) is not raised; `UWorld::MoveAttachedActors` (0x25f970, 2,558 B: the reference MoveActor's
attached-actor loop refactored) is still FarMoveActor of the attached actors; `CanBeTeleportedToLocationWithoutBeingSeen` (0x57f460,
769 B) not ported (0 teleports on both routes); UpdateAvoidableCollisionGroupFlags' other retail callers (behaviour start/resume,
relationship change) are outside this package; the player's avoidable (HandleAvoidance) is unported, so property flag 8 / push periods
never start.

## 2. Measurements

Captain `_26` after the detach (world 150.7..175), heading from displacement over >= 0.1 s, samples faster than 60 u/s:

| run | straight share (heading rate < 5 deg/s) | p95 rate | total heading change |
|---|---|---|---|
| JD0a / JD0c (head) | 70% / 71% | 125 / 136 deg/s | 625 / 638 deg |
| JD1b (build 1, old path switches) | 71% | 135 | 641 |
| **JD1a (build 1)** | **80%** | 124 | **457** |
| **JD2b (build 2, hand-back behaviour)** | **77%** | 126 | **479** |

Head: at the corner 10412/22412 -> 10467/22352 -> 10603/22391 the target yaw swung -9881 -> 8975 -> 1402 (the pawn aimed at each funnel
corner it was pulled into, overshot and came back). JD1a: the move yaw goes -16380 -> 1940 in 1.3 s and holds 1940 to the next corner.
Worker B `_3`: straight legs in every run (92-98% straight; deviation from his two-point route p95 10-17 uu). Tower `_12` (JD1t, JD2t):
100% straight, deviation p95 10 uu, 0 turn states.

Body vs velocity (`io_table.py`, once a second): `_26` 95% < 15 deg in JD0a, JD1a, JD2b alike (5.8..6.7 deg mean); with
`jd_measure.py` per frame after the detach: JD1a 97%, JD2b 95%. `_3` 100% outside turn animations. **Met.**

Static turns per corner: `_26` 1 static turn on the route (at 153.5), `_3` 1-2 per run (one per leg end). **<= 1 per corner: met.**

Avoidance (`-jddiag avoid`): every NPC registers (radius 10 from the tweaks, force 1, group 2). On the ride route no two walking NPCs cross:
the closest pass is the captain and `_4` at 74.2 uu centre distance (avoidance max distance 80; strength 0.10, forces applied
164.49..164.87) and `_22` at 79 uu; the 1-s sampled head run had no pair under 289 uu except the two boat-carried pawns. The
"two NPCs crossing at the dock" situation does not occur on either route, so the separation >= cylinder sum is met only in the
trivial sense (74 uu > the two cylinders).

FilterPathingFail: 0 lines in every run. Criticals: 0 in every run.

Ride SSIM at the constant offset (a = 13.57 - soiree ENTER): head JD0a **0.846**, JD0c **0.842**; JD1a 0.828, JD2b 0.833, JD2a 0.829.
The per-frame drop (median -0.015 vs JD0c) is spread over the whole boat ride, where no NPC uses this package's code (both boat pawns
are retargeted); the pairs show the same framing with the boat at a slightly different point of the ride (`build/cmp/JD2b_const/pairs/
pair_w0078.011.png` vs `JD0c_const/pair_w0078.004.png`). Not attributed; **below the .84 bar as measured**.

Gate (`run_regression.py --build-dir build/agentJD --no-build --exe-name DishonoredGame_JD.exe --only d3d9,inputtest,newgame
--game-extra-args=-jdnomoderate`, twice, `build/agentJD/gate1.txt`, `gate2.txt`): **23 ok, 0 failed both**; criticals 0/0/0;
unported_natives 0; probe_natives 3; inputtest_moved 1007.9 / 1007.6; newgame_pawns 29; d3d9_frames 1380 / 1500.

## 3. Turns at corners: retail's data says arcs, not turns in place

Every path-turn request at a corner was refused by RequestTurn (23 / 24 `path-refused` lines per run). The captain's config
(`L_Tower_Script.EliteGuardLocomotionConfig`, cooked package dump `tools/lococonfig.py`) gives the walk speed mode `EmptyWalk`
(Empty_WalkN 157.7 u/s) **two** turn animations only, `Empty_Walk_TurnLeft180/Right180`; a two-animation set accepts only swings
within 45 deg of a half turn (IsSpeedModeCanDoTurn, 2012 0x58b960). The corners on these routes are 35..64 deg, so retail cannot play
a turn animation there either: the move yaw turns at the speed mode's m_fRotationMaxSpeed (EmptyWalk 90 deg/s; measured peak 110
deg/s with the walk/run blend), an arc of about v/w = 100 uu radius. Only the idle mode (16 animations) turns in place - the static turns.

## 4. Where the brief was wrong, with the measurement

1. "UpdatePathProperties' turn-ahead bookkeeping (m_bMustTestPathAfterTurn, m_fBeforeTurnSquaredDist, m_fBeforeTurnDuration)": none of
   these is in UpdatePathProperties in either database (2012 0x582eb0, 2013 0x941ee0, 2,499 B both). m_bMustTestPathAfterTurn is the
   scheduler's (IK2 ported it), the other two RequestTurn's / UpdateTurn's (GK). What was wrong in UpdatePathProperties was the corner
   advance and the meaning of m_CurPathDir/Yaw (§1) - and, bigger, ComputePathMove blending at every corner.
2. "ComputePathYawFromDist 2012 0x583880 / 2013 0x5428b0": 0x5428b0 is the 2013 rva (VA 0x9428b0); ok. "0x548b90 push period" and
   "0x5491b0 priorities" are 2013 rvas (2012 0x589940 / 0x589e00), not 2012 addresses.
3. "fMinDistanceFromWalls from the config, not appFrand": retail randomises too - 2012 0x5875e0 is radius x (rand() x (multiplier-1)/32767
   + 1) x 1.65, the same distribution as appFrand(). No change.
4. "turn in place at corners as retail" / "the yaw rate at corners equals the turn-animation rate": the walk speed mode has only 180-degree
   turn animations (§3); retail arcs through these corners at the rotation speed.
5. "DetectPreAvoidanceCollision dynamic.cpp:286 returns FALSE": it was IO2's port of retail's no-component exit; now whole.
6. "Two NPCs crossing at the dock": no such pair on either route (§2).
7. ModerateMovePawn: DN's MovePawn had the inertia test inverted - retail (2012 0x584ca0) takes the moderate move only for an
   inertia-DISABLED pawn (and never just after a teleport, and with a BlockRigidBody base, not BlockActors). With the moderate move
   ported and DN's condition, every walking NPC took it (JD2a: 16,527 moderate / 8,023 full moves) and the captain's feet went onto the
   navmesh plane 45 uu inside the boat's deck at world 152 (Z 2500 -> 2451).

## 5. Files
`DishonoredGame/Src/`: arkcomponentlocomotionpath.cpp, arkcomponentlocomotiondynamic.cpp, arkcomponentlocomotion.cpp,
arkcomponentlocomotionrootmove.cpp, arkavoidance.cpp (new), dishonorednpcpawn_locomotion.cpp, dishonorednpcpawn.cpp (two calls);
`DishonoredGame/Inc/`: arkcomponentlocomotion.h, jddiag.h (new), CppText/UArkAvoidable.h (new), CppText/ADishonoredNPCPawn.h,
DishonoredGameEngineShims.h (generated, one line); `Engine/`: Inc/arkcomponentavoidance.h, Inc/EngineAIClasses.h, Inc/EngineClasses.h,
Src/UnNavigationHandle.cpp, Src/UnGame.cpp. Generator 79/79 byte-identical (`build/agentJD/regen`), bitfield_audit mismatches 0.
Sync `build/agentJD_sync.py`. `-jddiag[=census,walk,turn,avoid]`, `-jdwatch=<pawn>`, `-jdnomoderate`.

## 6. The unbuilt edit
`FArkComponentLocomotion::MovePawn` (2012 0x584ca0) after build 2: moderate move only when `!bJustTeleported && m_bInertiaDisabled`
with a BlockRigidBody (or collision-less) base. For every inertia-enabled NPC this is exactly build 2 with `-jdnomoderate` (JD2b, the
gate runs); the BlockRigidBody base test also changes FullMovePawn's `_bOnValidBase` argument on bases that block actors but not rigid
bodies - unmeasured. The coordinator's merge build is its first compile.
