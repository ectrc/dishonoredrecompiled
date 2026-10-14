# Agent GK (PHASE20 GK) — the captain's start poly, the stop-animation system, and the push clamp that was never retail's

Worktree `build/agentGK_wt` at `4b889ff` (main merged at hand-back: see §7). Working build `build/agentGK_rel` (`build/agentGK/gk_build.cmd`
through `build_queued.py`, one pass `gk_build0.log`: 0 errors, 0 C4263, 0 C4264); the gate's in-harness build `build/agentGK_reg` is the
package's second and last build, and the measured run GK2 is on that exe. IDA: private copies `build/agentGK/idb/GK_retail2013.i64` /
`GK_shipping2012.i64`, read headlessly through `resources/tools/ida/run.py` with `build/agentGK/gk_decomp.py` / `gk_xrefs.py` /
`gk_callees.py` (decompiles in `build/agentGK/decomp2012`, `decomp2013`; the MCP worker pool was full). Every address below was resolved
by hand in both databases (2013 rva = VA − 0x400000; 2012 image base 0). No FModel tool. Every launch through `game_input.py launch`
(`build/agentGK/gk_run.py`, `-windowed`, `-inputcmd`, `-unattended`, private inis under `build/agentGK/config/<run>/`); no retail
`Dishonored.exe` launch; nothing real touched. Control: the coordinator's CO1 (`head_rel` at `4b889ff`, the same `gb2_steps.txt` route,
`DishonoredGame/Logs/CO1.log`).

## 0. The answer

1. **The captain is the thing the player walks into, and he is where he is because of a teleport.** On CO1 and on GK1 alike,
   `fwdiag ahead ... blocked at 0 of 200 by DishonoredNPCPawn_26 (CollisionCylinder)` (19 and 13 lines): the player's W from the
   `Player` socket (9826, 23064) stops at 10023, 23034 against the captain's cylinder at 10092, 23057. The Elite (`_27`, 9994, 23174)
   is 110 uu off the line and has no go-to in either log. The captain stands at 10092 because agent DN's teleport put him there: the
   ride leaves him at 10130, 23055, 2503 (based on the boat), `AIGoToActor_3` fires at log 208 s, the start-poly test fails, and
   `FindNearestLocationOnNavMesh` + `FarMoveActor` moves him to the "nearest" mesh point - three times on GK1, 0.558 uu further in
   +Y each time (`gadiag teleport DishonoredNPCPawn_26 from X=10129.755 Y=23055.264 Z=2502.980 to X=10092.222 Y=23056.334
   Z=2414.988 -> 1`, then `23056.893`, `23057.451`). §2.
2. **The "0.56 uu off the mesh" of GC/GD is not a distance off the mesh.** `FindNearestLocationOnNavMesh` (retail's body, DN's port is
   exact) projects the pawn's **centre** (ground + CollisionHeight 96) onto the poly's plane; on a poly tilted by normal
   (-0.001, -0.006, 1) that projection lands 96 × 0.006 = 0.56 uu from the pawn's own XY, every time, on every tick. The captain's
   ground point is ON the mesh: the 2D side test against the poly's world-space edges reads `-0.059 -5757 -4783 -10132` for the poly
   under him (all ≤ 0: inside, on the edge shared with the next poly, whose test reads `... 0.000 ...`). What refused it is the
   containment test, twice over: the reference engine's `GetPolyFromPoint` → `ContainsPoint` (a 3D box with a one-unit epsilon
   - Worker A's floor trace is 1.6 uu above his poly - then a strict crossing count that excludes an edge point), and, after this
   package put retail's test in, the candidate query itself (§2.1). §1, §2.
3. **Retail's `UNavigationHandle::GetPylonAndPolyFromPos` is not the reference's.** 2012 `0x2a8830` / 2013 `0x28e260` (unnamed
   `sub_68E260`, 1,014 B): a vertical segment Pos ± `NavMeshGen_MaxPolyHeight / 2` with **zero** XY extent through each enabled pylon's
   mesh (`GetIntersectingPolys`), the 2D edge-side test `ArkNavMeshPolyContainsPoint2DAlreadyInBound` (edges inclusive, no
   `MinWalkableFloorZ`), and with several polys the one whose plane passes nearest Pos.Z. Ported into the Engine unit (hand-edited
   reference file) with the three Arkane helpers of `arknavmeshbuildutils.cpp`. §2.1.
4. **The stop-animation system is ported** - `arkcomponentlocomotionrootmove.cpp`, 15 functions, 2012 and 2013 resolved (§3) - and it
   runs: Dock Worker B turns in place before his walk (`Empty_TurnToWalkLeft135` requested at 174.2 s, `RootRotationScale` from the 135°
   swing, then `Empty_WalkN` at 123 u/s from 178.9 s). GD's two stand-ins are removed. But GK1 also measured him **oscillating on his
   mark** (±20 uu, `Empty_Walk_TurnRight180` requested at the end, 185.5-200 s) where GC1 stood still - and the cause was neither
   stand-in: agent DN's `IsThereAnotherForceThanPath` answered `IsRootMotion()`, whose DN body reads the mesh's `RootMotionMode`, which
   is `RMM_Velocity` on every NPC (GA's `rmm 1/x`), so the push-speed clamp held every lone pawn's move to one acceleration step above
   its real speed - GD2's "moves zeroed by the push clamp" crawl, which GD's stand-ins papered over. Both are retail's bodies now
   (§3.2) and the gate exe is measured in §4.
5. Premises corrected: §1 (six).
6. Gate: §5. Sync: `build/agentGK_sync.py`, 0 flagged (§7).

## 1. Premises of the brief, corrected with the measurement

| the brief / earlier report said | measured |
|---|---|
| "`NPCPawn_26` ... 0.56 uu from a navigation-mesh point the start-poly test refuses ... GC's edge-point bounds problem" (GD §5, PHASE20) | the 0.56 is `GetClosestPointOnPoly( pawn centre )` on a tilted poly, not a gap: the ground point is inside the poly (side test ≤ 0 on every edge, GK1 `candidate poly 0/4`). Retail's `FindNearestLocationOnNavMesh` has the same arithmetic (2012 `0x586560`, DN's port is exact) - it is the reference's containment, and the teleport it provoked, that moved him |
| "retail 2012 `0x57f8a0` (UpdateStartLocAndVerifyIfOnValidPoly) is DN's body line for line - so the difference is upstream: the point handed in" | the point is the same (the trace hit under the pawn's XY, Z 2318.995 on both); what differs is `GetPylonAndPolyFromPos` itself, which the brief treated as shared engine code: retail replaced the reference body (2013 `0x28e260`, 1,014 B against the reference's pylon query + `GetPolyFromPoint`) |
| "Worker A (`NPCPawn_4`, same symptom, nearest 0.13)" | Worker A's floor trace lands 1.6 uu **above** his poly (`Z=2404.000` vs the poly's `2402.4`): the reference's `ContainsPoint` box test (`DEFAULT_EPSILON` 1.0) refuses the height, retail's ± `NavMeshGen_MaxPolyHeight / 2` (80 uu) window accepts it; his 0.09 uu creep per teleport is the same centre projection (normal tilt 0.001) |
| "the Elite walks off ... the Elite's go-to and the captain's" (coordinator) | the Elite (`_27`) receives no `OnAIGoToActor` on CO1 or GK1 (the four go-tos of the route are `_25`, `_26`, `_8`, `_9`); the blocker is the captain's cylinder in both logs (`blocked ... by DishonoredNPCPawn_26`, 19 / 13 lines), 110 uu from the Elite |
| "the plank matinee (SeqAct_Interp_6 'Plank0')" | `Main_Sequence.SeqAct_Interp_6` moves `InterpActor_13` at 3509, 24926 (the start pier), playing from 100 s on both logs - not the dock; `SOIREE_Guards.SeqAct_Interp_6` is a different op, active from load |
| GD §5.2: the landing is "what retail lands the last unit with (`m_bHandleStop` → end speed = walk)" | `m_bHaveStopAnim` has no writer in either database (every function of the component and the constructor were decompiled and searched for the bit at +584), so `UpdateStopMove` never raises `m_bHandleStop`; retail lands an accurate or non-animated stop with `HandleArrival`'s overshoot clamp alone - which works once the push clamp stops firing (item 4 of §0) |

## 2. The start poly

### 2.1 `UNavigationHandle::GetPylonAndPolyFromPos` (Engine/Src/UnNavigationHandle.cpp, hand-edited reference unit)

Retail (2012 `0x2a8830`, 998 B; 2013 `sub_68E260` rva `0x28e260`, 1,014 B, found through the only two callers of
`ArkNavMeshPolyContainsPoint2DAlreadyInBound` + `GetIntersectingPolys` in the 2013 database): pylon octree box = Pos with extent
(0, 0, `NavMeshGen_MaxPolyHeight` × 0.5); per pylon with `NavMeshPtr` and not `bDisabled` (+788 / +860 bit 0x200 = `APylon::IsValid()`):
`NavMeshPtr->GetIntersectingPolys( Pos, Extent, Polys )`, then every new poly not passing `ArkNavMeshPolyContainsPoint2DAlreadyInBound`
removed; one poly → it; several → the one with the smallest |planeZ(Pos) − Pos.Z| (plane from `PolyCenter` / `PolyNormal`, |N.Z| ≥ 1e-4).
`MinWalkableFloorZ` is never read. The three helpers (`arknavmeshbuildutils.cpp`, PDB list kept): `ArkNavMeshPolyContainsPoint2DAlreadyInBound`
2012 `0x5409e0` / 2013 `0x501880` (365 B both: `(Vj.Y−Vi.Y)(P.X−Vi.X) − (Vj.X−Vi.X)(P.Y−Vi.Y) > 0` on any edge → outside),
`ArkNavMeshPolyContainsPoint2D` `0x544270` / `0x5062a0` (66 B, the bounds first, inclusive), `ArkNavMeshPolyContainsPoint3D` `0x5442c0` /
`0x5062f0` (249 B, the height over the plane within [−extent, PolyHeight + extent)).

**One named deviation**: the candidate query is padded by 1 uu in XY (`CandidateExtent( 1, 1, Extent.Z )`). Measured on GK1: the captain's
ground point lies on the edge shared by two polys (sides `-0.059` and `0.000`); the reference `GetIntersectingPolys` this tree runs
transforms the zero-extent box into the mesh's local frame (`bNeedsTransform`: the L_Tower_P mesh is an imported mesh with a
`LocalToWorld` translation, its `BoxBounds` are local - `bounds X=9747..9843` against a world centre of `10044`) and neither poly came
back, while a 50 × 50 × 200 query returned both. Retail reads world-space data with no transform and needs no pad. The containment
stays retail's exact test, so the pad widens the candidates and never the answer. GK2 (§4) measures the padded query.

### 2.2 What the diagnostic measured (GK1, `-gadiag`, the new `gadiag teleport` and `pathfail ... candidate poly` lines)

| pawn | ground point | candidates (50 × 50 × 200) | side test | what refused it |
|---|---|---|---|---|
| `_26` captain, 213.3 s | 10092.222, 23057.451, 2318.995 | 4 | poly 0: `-0.059 -5757 -4783 -10132` (inside, on an edge); poly 2: `... 0.000 ...` (on the same edge) | the zero-extent candidate query (§2.1) |
| `_4` Worker A, 107.8 s and every 5 s | 11554.832, 22720.500, **2404.000**; nearest 1.58 below | — | — | the reference's one-unit box (GC's build); 34 teleports of +0.087 uu on GK1 (the same centre projection), §4 for the gate exe |
| `_8`, `_16`, `_17` | on polys (`contains2D 1`, every side ≤ 0) | 1 | inside | the same candidate query: three more spawn-point pawns the reference test and the first port both refused |

The 2012 `UpdatePathOfHigherPriorityRequest` (`0x58a4d0`, 3,132 B) was read whole for the teleport: retail teleports only a pawn that
`m_bHasBeenOnNavMesh` (bit 0x100 at +252) and adds `CollisionHeight` to the mesh point; DN's port teleports any pawn (his deliberate
deviation, documented at the site, and the one that put the captain on the dock edge). Left as DN had it: the guards spawn off the
mesh and would never path at all under retail's gate - retail carries them to the dock on the boat's sockets by `FDisComponentRetargeting`
(GB's residue, §6), whose `Captain` socket GK1 measured at **10042.7, 23044.4, 2337.3** (`gbdiag bones ... Captain: socket`), 50 uu
from the captain's teleport point and on the same dock polys.

## 3. The stop-animation system (`DishonoredGame/Src/arkcomponentlocomotionrootmove.cpp`)

| function | 2013 rva | 2012 rva (B) | how the 2013 one was found |
|---|---|---|---|
| `UpdateStopMove` | `0x547300` (named) | `0x587fe0` (1,515) | diffed: same control flow on a layout 8 bytes longer |
| `UpdateStartMove` | `0x546f60` (`sub_946F60`) | `0x587c40` (916) | callees of `ComputeDynamic` 2013 `0x54bec0` in retail's order |
| `UpdateTurn` | `0x54b9d0` (`sub_94B9D0`) | `0x58c520` (1,260) | same |
| `RequestTurn` | `0x54af40` (`sub_94AF40`) | `0x58ba90` (1,451) | callee of `UpdateTurn` 2013 |
| `EvaluateStaticTurn` / `EvaluateStartPathTurn` / `EvaluatePathTurn` | `0x54b4f0` / `0x54b780` / `0x54b880` | `0x58c040` / `0x58c2d0` / `0x58c3d0` (651 / 241 / 330) | the first three callees of `UpdateTurn` |
| `IsSpeedModeCanDoTurn` | `0x54ae10` (named) | `0x58b960` (299) | |
| `ResetTurn` / `ResetStartMove` / `ResetStopMove` | `0x545a10` / `0x545ae0` / `0x545b70` | `0x5868d0` / `0x5869a0` / `0x586a30` (195 / 136 / 136) | callees of the three Update functions |
| `SetRootState` | `0x5459a0` | `0x586860` (100) | the one callee that calls `ComputeIsArrivedFlagAndSq2DDistToPathEnd` `0x544390` |
| `IsReadyToTurn` / `IsLocoFaceToRequestActive` / `IsTurnNotTooCloseFromPathEnd` | `0x53e7a0` / `0x53e750` / `0x542b20` | `0x57fad0` / `0x57fa80` / `0x583af0` (113 / 70 / 143) | callees of `EvaluateStaticTurn` 2013 `0x94b4f0` in order |

Transcribed from the 2012 decompiles; the yaw arithmetic (the `(WORD)(b − a)` shortest signed delta retail inlines) is one file
static, as are the two `UArkAvoidable` readers (retail's `GetAffectorCount` / `GetForce` are `m_AffectorCount` / `m_Force`; the
component's `m_pAvoidable` may be NULL here). `FArkComponentFaceTo` is absent: `IsLocoFaceToRequestActive` answers retail's first clause
(request count 0 → TRUE), `EvaluateStaticTurn`'s turn towards a face-to target with no path is skipped (named), `EvaluatePathTurn` never
fires (`m_FaceToRequestID` stays −1, retail's own first test). The turn animation's root rotation is scaled from the authored
45 × (4 − pair) degrees to the swing (`RootRotationScale`), `RootMotionMode = 3` (`RMM_Accel` in this tree's enum), `RotateActor`.

### 3.1 The callers

`ComputeDynamic` takes retail's first road (`IsDisabled || STOP_END || TURN || START || (m_bCanManageAnim && IsPlayingOnlySpecificAnim)`)
and runs the three machines there and in every 0.04 s sub-step (retail refreshes `m_TargetMoveYaw` before them on a new path or a stale
polyline and again after, `0x58cb8d..0x58ceea`); `ComputeTargetMoveSpeed` has retail's root-state switch (turn-wait states hold
`m_TurnProps.m_fTurnStartSpeed`, stop-approach states the walk speed, the animation-owned states 0, only NONE / TURN_END / START_END
brake) and the walk-back-onto-the-mesh speed for a request without a path; `OnDisable` resets the three machines; `HandleArrival` keeps
only retail's overshoot clamp (GD's landing test removed) and the push clamp loses GD's `!m_bIsArrived`.

### 3.2 Two DN bodies that were not retail's (measured on GK1, fixed for the gate exe)

| function | 2013 / 2012 | DN's body | retail's body |
|---|---|---|---|
| `IsThereAnotherForceThanPath` | `0x541a00` / `0x5829d0` (249 B) | steering force, or the push flags, or **`IsRootMotion()`** | steering force (any component ≥ 1e-4), or - unless `ms_LocoCpntSharedProps( m_LocoCpntIdx ).m_bMustIgnoreAvoidance` - an avoidance force ≥ 0.05 |
| `IsRootMotion` | `0x53ed30` / `0x57ff70` (60 B) | `Mesh->RootMotionMode != RMM_Ignore` (TRUE on every NPC, `rmm 1/x`) | disabled, or the pawn `bDeleteMe`, or STOP_END, or the node playing only a specific animation |

With DN's pair every lone NPC passed through the push-speed clamp every frame (speed held to one acceleration step above the real
speed, zeroed under `m_fVelocityToStopMoving`), which is GD2's 216 s crawl over the last unit and GK1's Worker B jitter on his mark
once the stand-ins were gone (`gadiag pawn DishonoredNPCPawn_3 ... 11636/22102 vel 60 ... 11614/22115 vel 35 ... 11636/22101 ...` from
185.5 s, `Empty_Walk_TurnRight180` requested by `EvaluateStartPathTurn` on each overshoot, and he drifted 40 uu off the mesh by 214 s).
Also retail's per-frame clear of `m_bStuckedInAWall` at the top of `ComputeDynamic` (`0x58ca10+0x6b`), which DN left out (the flag stayed
up until the next full move and would have fired `UpdateTurn`'s wait-for-good-foot at once).

## 4. The runs

All through `build/agentGK/gk_run.py` (GB's runner: `game_input.py launch`, `-windowed -inputcmd -unattended`, gate-style inis
under `build/agentGK/config/<run>/`), GB's `gb2_steps.txt` route as `build/agentGK/gk2_steps.txt` (the ride, two turns to +X, W 14 s
with captures `gb2_a..f`, then three more captures `g..i` after a second W), switches `-finewgame=50 -fosoiree -fkdiag -frdiag
-fwdiag -fwnear -fwkismettrace -diskismet -gbdiag -gadiag -fvdiag`. Logs `build/agentGK/GK1.log`, `GK2.log`; captures (BMP + 640x360
PNG, every one checked: 2,248-7,663 distinct sampled colours) in `build/agentGK/captures/GK1`, `GK2`; the measured lines
`build/agentGK/gk1_run.txt`, `gk2_run.txt`. 0 `Critical` in both, no `execPlayDying`, `presented frame (1920x1080, windowed)`.
Control: CO1 (coordinator, `head_rel` at `4b889ff`, same route and script).

| run | exe | what it measured |
|---|---|---|
| CO1 | `head_rel` `4b889ff` | the captain teleported to 10092, 23057 at 161 s (`base SkeletalMeshActorMAT_2` → `none`, Z 2503 → 2415), `no start poly` at 166 s, `pathing failed`; the player's W stops at 10023, 23034 `blocked at 0 of 200 by DishonoredNPCPawn_26` (19 lines); `AIGoToActor_3` → `Interrupted` at quit |
| GK1 | `agentGK_rel` (build 1: the first port, the root-move system, GD's stand-ins removed) | `gadiag teleport DishonoredNPCPawn_26` three times (+0.558 uu in Y each, §2); at 213.3 s the candidate dump: 4 polys within 50 uu, the ground point inside two of them by the side test, both missing from the zero-extent candidate query → `pathing failed`; the player blocked by `_26` (13 lines), then the script's later headings walk him off the stern into `DishonoredWaterVolume` (physics 3) - a script artefact, not a result. Dock Worker B turns in place (`Empty_TurnToWalkLeft135` at 174.2 s), walks at 123 u/s from 178.9 s, reaches 11636, 22102 at 185.5 s, then **oscillates** ±20 uu with `Empty_Walk_TurnRight180` requests, drifts 40 uu off the mesh by 214 s, 18 teleports (§3.2). Worker A: 34 teleports of +0.087 uu, `no start poly` every 5 s. Captures `gk1_gb2_a` (the stretched captain in the bow, GH's), `gk1_gb2_d` (the player's view inside his coat) |
| GK2 | `agentGK_reg` (the gate exe: + the padded candidate query, retail's `IsThereAnotherForceThanPath` / `IsRootMotion`, the stuck-flag clear) | **the captain still fails**: 2 teleports (195.2 s), `no start poly at 10092.222, 23056.986, 2318.992` at 200.2 s with the candidate dump reading, for the two polys under him, `inside2D 1 in zero-extent query 1` (polys 0 and 2, sides `-0.059 ...` / `... 0.000 ...`; `transform 1 origin X=249`) - so the candidate query and the containment both pass and the refusal is in the rest of `GetPylonAndPolyFromPos` (the pylon octree stage) or in `UpdateStartLocAndVerifyIfOnValidPoly`'s two height tests after it (§4.1); the player blocked by `_26` (35 lines, 163-202 s), `DisTrigger_8` never touched. **Worker B no longer oscillates and is never teleported** (0 against GK1's 18): walks 10845, 22461 → 11626, 22104 by 171 s, back through TargetPoint_57 (10988, 22152 at 176 s) to 10852, 22391 and holds there within ±20 uu with turn animations to the quit (38 `loconode` lines with a `Turn` child, 162-231 s). Worker A as GK1 (33 teleports). Captures `gk2_gb2_a..i` (the captain in the bow, the player against him) |

### 4.1 What the measurement leaves open, exactly

`UNavigationHandle::GetPylonAndPolyFromPos` as ported returns FALSE for a point its own candidate query returns two polys for and its own
2D test accepts (GK2, `_26` at 200.2 s; `_8` at 99.2 s, `inside2D 1 in zero-extent query 1`). Two stages are left between those two
facts and the answer: the pylon octree query (`FNavMeshWorld::GetPylonOctree()`, a box of extent (0, 0, 80) in world space against
`APylon::GetBounds( WORLD_SPACE )` - `NavMeshPtr->BoxBounds.TransformBy( LocalToWorld )` for this imported mesh, where the reference's
own query used (5, 5, 1024) and my diagnostic's `GetAllPolysFromPos` used (50, 50, 200) and found the pylon), and, after a poly is
returned, `UpdateStartLocAndVerifyIfOnValidPoly`'s `MaxStepHeight > ground.Z − GetClosestPointOnPoly( ground ).Z` - where the reference's
`GetClosestPointOnPoly` tests its world-space plane point with `ContainsPoint( ..., LOCAL_SPACE, 0 )` (UnNavigationMesh.cpp:19594), a
249-unit frame mismatch on this mesh that sends it to the edge loop. One `gadiag` line inside `GetPylonAndPolyFromPos` (pylons found,
polys before and after the 2D filter) decides between the two; both builds of this package were spent before the GK2 dump existed.
Either way the captain's own ground point is on the mesh by retail's test, which is the premise the brief had the other way round.

## 5. Gate

`run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGK_reg --newgame-at 50 --game-extra-args=-windowed` from the
worktree's copy, its own in-harness build (the package's second build): **47 ok, 0 failed, 0 skipped, 2967 s** (`build/agentGK/gk_gate.log`,
`build/agentGK_reg/regression/summary.txt`); `unported_natives 0 (<= 0)`; `build_DishonoredGame_errors 0`, `build_CoreSmoke_errors 0`,
`build_LayoutProbe_errors 0`; 0 `C4263` / `C4264` in `build_DishonoredGame.log`; `nullrhi_criticals 0`, `d3d9_criticals 0`,
`inputtest_criticals 0`, `newgame_criticals 0`, `options_criticals 0`; `d3d9_frames 1830` (>= 1000, no re-run needed), `d3d9_startup_seconds
26.5`, `inputtest_moved 1020.8`, `newgame_pawns 29`, `coresmoke_passed 99`, `verify_phase2 3/3`, `layout_mismatching 0`, `options_as2_errors
39`. The working build `gk_build0.log`: 0 errors, 0 C4263, 0 C4264, 311 warnings (pre-existing). No minidump was produced by any run.

## 6. What is left

1. **`FDisComponentRetargeting`** (GB's residue, not reached): 2012 ctor `0x8af8b0`, `StartRetargeting 0x8d2400` (569 B), `StopRetargeting
   0x8d0eb0` (563 B), `SetMasterActor 0x8d1100`, `Stopping 0x8d2330`, `PreAsyncWorkTick 0x8d23c0`, `OnStopRetargeting 0x8d10f0`,
   `GetMemoryFootprint 0x8af970`, and **`UpdateRetargeting 0x8b37e0` (4,308 B of SSE matrix work: the slave mesh is placed in the master
   bone/socket's frame each tick, `m_bKeepActorCloser`, the mesh translation locked by `eMeshOffsetDisable_ByRetargeting`)**. It needs
   `FArkGameEventDispatcher` (event 13) and the component manager. It is what puts the captain on the `Captain` socket (measured at
   10042.7, 23044.4, 2337.3 at the ride's end - on the dock polys) instead of GA's basing + DN's teleport.
2. `ASkeletalMeshActorMAT::MAT_SetPhysics` (2012 `0x3317c0`) + `InitGroupInst`'s `bBecomeDynamic`, `UInterpTrackInstAttachment::LoadData /
   SaveData` (GB §9.3): not reached.
3. The teleport's `CanBeTeleportedToLocationWithoutBeingSeen` (2012 `0x57f460`, 769 B; 2013 `0x53e310`) and retail's `m_bHasBeenOnNavMesh`
   gate (§2.2): DN's deviation stands until the retargeting lands the guards on the mesh by itself.
4. `FArkComponentFaceTo` (the turn towards a face-to target, the path corner turns, the node's angle - GC §1).

## 7. Merge, sync

Main `ece8e1b` (GH: the look-at blend, `gfarmsdiag`, `game_input.py exec`) merged into the worktree by `git merge` after GK2 (commit
`c09b38c`, no conflicts, nothing of GH's touches navigation or `arkcomponentlocomotion*`); the generator re-run on the merged tree
(`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --out-dir build/agentGK/regen`) diffs byte-identical
(Sources.cmake and every generated header and unit). The merged tree is **not rebuilt** (both builds spent), so GK2's captures still show
GH's stretched guards. `build/agentGK_sync.py`: 11 files, every function named, **0 flagged** against main `ece8e1b`, the list equal to
`git diff --name-only ece8e1b` + `ls-files --others` (self-checked both ways). Engine hand edits: `UnNavigationHandle.cpp` (one function,
one include), `arknavmeshbuildutils.cpp` (was a comment-only stub), `Inc/arknavmeshbuildutils.h` (new); no `dishonoredgame*classes.h`,
`DishonoredGameRegistrants.cpp` or `Sources.cmake` edit; the new DishonoredGame unit is picked up by the module glob like DN's.
