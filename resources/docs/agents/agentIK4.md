# Agent IK4: the stair foot, falling NPCs, FaceTo on by default (wave 26, PHASE23 package IK4)

Worktree `build/agentIK4_wt` at `788cd16`: IK3's merge `cace044` plus docs, with IL (`94d928d`) in it. Build dir `build/agentIK4`
(RelWithDebInfo). No commits. Two builds, both 0 errors and 0 C4263/C4264:
- build 1 (`DishonoredGame_IK4b1.exe`, from scratch): the diagnostics below;
- build 2 (`DishonoredGame_IK4.exe` = `DishonoredGame_IK4b2.exe`): adds the gap census, `-ik4traceall`, and FaceTo on by
  default.

Runs (`build/agentIK4/ik4_run.sh`):

| run | build | switches | shots |
|---|---|---|---|
| `CC_IK4a` | 1 | default | - |
| `CC_IK4f1` | 1 | `-icfaceto` | world 50..175 |
| `CC_IK4f2` | 1 | `-icfaceto` | - |
| `CC_IK4f3` | 1 | `-icfaceto` | - |
| `CC_IK4c` | 2 | FaceTo default | world 50..175 |
| `CC_IK4t` | 2 | `-ik4traceall` | - |

## 1. The stair foot (task 1)

The captain walks the boat deck (FaceTo on: 800..820 uu straight in every run) to X ~10327, Y 23053. There the floor sweep finds
nothing. `-ikdiag probe` traces from the raised start (Z 2551) down 600 uu at that point. **Every one of them hits nothing**:

| probe | flags | extent | result |
|---|---|---|---|
| retail NPC | 0x100030DF | cylinder | nothing |
| NPC without the NPC-only bit | 0x30DF | cylinder | nothing |
| player | 0x200030DF | cylinder | nothing |
| all blocking | `TRACE_AllBlocking` | zero | nothing |
| all blocking + player | `TRACE_AllBlocking` + player bit | zero | nothing |
| `MultiLineCheck` | `TRACE_AllBlocking \| TRACE_Visible` | zero | no hit at all |

With `-ik4traceall`, every primitive answers every movement trace whatever its `m_CollisionTraceTypes`. The result is the same:
the captain falls at the same spot, and the probes return nothing (IK4t).

So **the trace-type mask is not the cause**. Under the stair foot there is no collision primitive at all in our world within
600 uu. The navmesh (built where collision existed) has a poly there at Z 2415. The rendered stairs start there
(`build/cmp/IK3f/ours/w0154`). Retail.mp4 has the captain stand on them at world 154.

The suspect is now a stair/landing mesh that renders but has no collision in our runtime. Candidates:
- its component is not in the collision octree;
- its `UStaticMesh` collision data (kDOP tree / simple-collision `BodySetup`) did not load;
- it lives in a `StaticMeshCollectionActor`, whose components are not in `AllComponents`.

The last one is why build 2's actor census (`ikdiag census`, actors' `AllComponents` within 250 uu) could not name it. The census
also fired at the first floor miss, by the player at X 9834, not at the gap.

Next step: walk `StaticMeshCollectionActor::StaticMeshComponents` and the level's BSP/model at 10327/23053, and run each
component's own `LineCheck`. That separates "not in the octree" from "no collision data".

## 2. A falling NPC never lands (task 2)

`ikdiag falling` (once a second while PHYS_Falling): the captain drops from Z 2511 into `DishonoredWaterVolume_0` (bWaterVolume
1, gravity -1500). He slows to Z velocity ~ -1.6 at Z 1922, with nothing below within 200 uu. He stays PHYS_Falling for the rest
of the run: the engine's physFalling in a water volume decelerates the pawn but never lands it.

Retail's NPC side, read off the 2012 PDB:
- `ADishonoredNPCPawn::setPhysics` (2012 `0x7adaa0`, 264 B) requests the NPC master FSM's `UStateNPCMasterFalling` on
  PHYS_Falling and `UStateNPCMasterWalk` on PHYS_Walking.
- `UStateNPCMasterFalling::OnEnterState` is 2012 `0x6c9730`.
- The NPC master FSM and its states are skeletons in this tree (`statenpcmasterfalling.cpp` is a 9-line stub).
- The water entry (`ADishonoredPawn::OnEnterWaterVolume` / `Tick_Water`, IF's partial port) is not wired to `ADishonoredWaterVolume`.

These are a package of their own (the NPC master FSM, the water volume). They are **not ported**. With the stair-foot collision
in place the captain would not fall; whether retail drowns or swims an NPC that falls into the slip is open.

## 3. FaceTo on by default (task 3)

Three ride runs with `-icfaceto` on build 1, plus IK4c with the new default:

| run | BlendNow | "Failed, with animsequence" | Critical | captain straight / path |
|---|---|---|---|---|
| IK4a (switch off) | 21 | 0 | 0 | 257 / 690 uu (no turn: walks the start heading, falls at the same bow tip) |
| IK4f1 | 20 | 0 | 0 | 805 / 1017 uu |
| IK4f2 | 20 | 0 | 0 | 801 / 998 uu |
| IK4f3 | 20 | 0 | 0 | 819 / 1133 uu |
| IK4c (build 2, default on) | 20 | 0 | 3 at exit (below) | same walk |

Worker B (`_3`) is unchanged in all runs: against the wheel object, 0 uu. The switch is removed: SetConfig always creates the
component (retail 2012 `0x584010`), and `DisICFaceToEnabled` / `-icfaceto` are gone from `arkcomponentfaceto.cpp`, `icdiag.h`,
`arkcomponentlocomotion.cpp` and the comments in `arkcomponentlocomotiondynamic.cpp` / `dishonorednpcpawn_rotation.cpp`.

## 4. Avoidance (task 4)
No build left. `UNavigationHandle::PointReachableZeroExtent` (2012 `0x2a9ba0`) and `DetectPreAvoidanceCollision` (2012 `0x584f10`)
remain unported.

## 5. Acceptance

| check | result |
|---|---|
| captain at the top of the dock stairs within 25 s of world 150.6 | **not met.** He walks the deck to the stair foot (world ~153.7, as retail w0152..0154) and falls: no collision primitive at the foot (§1) |
| no NPC left PHYS_Falling at run end | **not met**: the captain (§2) |
| Worker B / walkers unchanged | Worker B unchanged; gate walkers see §6 |
| ride SSIM >= .72 at the agreed offset | **0.726** (IK4c, FaceTo default, offset -42.711, MAD 10.60); IK4f1 0.736 |

IK4c and IK4t (both build 2) end with `Critical: Pure virtual function being called` after `appRequestExit`. The call is in
`UAkAudioDevice::FinishDestroy -> AkAudioLibTerm -> CAkBankMgr::Term -> CAkBankMgr::UnloadAll` (`AkBankMgr.cpp:2296`, IH's Wwise
runtime), at shutdown. It is not in the locomotion code, and the build-1 runs on the same base exited clean. Reported, not
investigated: `External/Wwise2012` is outside this package.

## 6. Gate
`run_regression.py --build-dir build/agentIK4 --no-build --exe-name DishonoredGame_IK4.exe --only d3d9,inputtest,newgame`
(`build/agentIK4/gate1.txt`, `gate2.txt`, build 2). Both runs **23 ok, 0 failed**:

| | run 1 | run 2 |
|---|---|---|
| `unported_natives` / `probe_natives` | 0 / 3 | 0 / 3 |
| criticals | 0 | 0 |
| `d3d9_frames` | 1710 | 2010 |
| `inputtest_moved` | 1045.1 | 1047.8 |
| `newgame_on_mission` / `newgame_pawns` | 120 / 29 | 120 / 29 |

## 7. Where the brief was wrong
1. The stair foot is not a trace-mask divergence: with every primitive forced to answer every movement trace, nothing is under
   it. No primitive there collides at all.
2. A falling NPC is held by physFalling in the slip's water volume. Landing would need the NPC master FSM and the water volume,
   both unported; no locomotion-side landing code exists in retail to port.
3. FaceTo on by default changes nothing measurable in the walkers or the blend counts; the captain then turns onto his path.
