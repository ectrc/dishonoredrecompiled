# agent DN — the NPCs walk: the locomotion component and the four things the AI wants from the navigation mesh

Package: `FArkComponentLocomotion` and the nav-mesh queries it is the AI's only route into. Base HEAD `9a04708`.
Worktree `build/agentDN_wt`, build dir `build/agentDN`, build script `build/agentDN_release.cmd`, IDA copies
`build/agentDN_ida/`. No commits, no `git add`. `resources/play.cmd` and `resources/build-release.cmd` untouched.

Agent DF handed this over with the counters already saying what was wanted: **26 open locomotion requests and nothing to
service them** — `loco new/update/stop 26/0/0`, `faceto 0`, `moved 0.0 uu`. Agent CG had scoped the nav-mesh side to
"four things and no more". Both were right, and the first measurement changed the shape of the package.

## The measurement that came first, and what it found

Before writing a line of the component: **is there a navigation mesh in `L_Tower_P` at all, and is anyone standing on
it?** The answer decided everything else.

```
disai navmesh: 1 pylons (1 enabled, 1 with a mesh), 668 polys, 1437 verts, 866 edges
disai navmesh: 26 NPCs, 15 standing on a poly, 8 with a poly within 150uu
```

The cooked mesh loads, deserialises and answers static queries. Agent AD's reference nav-mesh runtime — `UNavMesh`,
`UNavigationHandle`'s poly A\*, the edge and pylon classes, 28,000 lines of it — was live the whole time and had never
been asked anything. That is the thirteenth instance of this project's standing pattern, and the widest yet by the ratio
of authored content to the thing that was missing:

> **DEFECT 1 — `APawn::SetupPathfindingParams` was an empty inline stub, and it is the gate on the entire nav-mesh
> runtime.** `Engine/Inc/EnginePawnClasses.h`, `virtual void SetupPathfindingParams( FNavMeshPathParams& ) {}` with the
> comment `// DISHONORED: stub, port rva 0x1e05d0`. `UNavigationHandle::FindPath` calls it through
> `IInterface_NavigationHandle` to fill `CachedPathParams` and then **returns FALSE at its second line** unless
> `CachedPathParams.bAbleToSearch` is set — and that function is the only thing that sets it. So every path search any
> pawn had ever made returned FALSE before it started, on a map with a 668-poly mesh and 26 guards standing on it.
> `APawn::GetEdgeZAdjust` (2013 rva 0x1caf50) was the same. Both are now ported (2013 rvas 0x1caf50 / 0x1cafa0), and
> `ADishonoredNPCPawn` overrides the second one as retail does (0x74ad20).

## The census, before and after

Same map, same build script, same switches; `L_Tower_P`, null RHI, `-disai`, **0 `Critical` in every run**.
The "after" column adds `-dislocowalk`, which is explained under "What asks them to walk" below.

| | before (HEAD `9a04708`, own build) | after |
|---|---|---|
| NPC pawns / controllers / brains | 26 / 26 / 26 initialized | unchanged |
| sub-state enters / transitions / slot 0 | 684+26 / 26 / `DisBehaviorIdle`=26 | unchanged |
| navigation mesh | not measurable — nothing asked | 1 pylon, **668 polys, 1,437 verts, 866 edges** |
| NPCs on a nav-mesh poly | 15 of 26 | **21 of 26** (the teleport recovers six) |
| locomotion components | **0** | **26** |
| loco requests new/update/stop | **26 / 0 / 0** | **50 / 24 / 24** |
| faceto new | 0 (correct then) | **12** |
| paths searched / failed | 0 / 0 | **103 / 4** |
| path points the funnel produced | 0 | **651** |
| arrivals reported to the AI | 0 | **25** |
| **2D distance moved** | **0.0 uu** | **26,224.4 uu** |
| **NPCs that moved** | **0 of 26** | **14 of 26**, farthest 2,995 uu |
| yaw turned (census) | 0 | non-zero every second: the NPCs face where they walk |

Per NPC, from the same line:

```
disai locomoved: 14 of 26 NPCs moved: DishonoredNPCPawn_23=2995 _14=2834 _3=2810 _22=2014 _9=1746
                 _25=684 _5=498 _7=439 _15=359 _1=236 _24=59 _12=36
```

and the component's own line, which is the one the accept asks for:

```
disai loco: 26 components; requests 50 start 24 update 24 stop; paths 103 built 4 failed, 651 path points;
            25 arrivals; 3528512 MovePawn ticks, 26224.4 uu moved; 53 teleports onto the mesh
disai desires: 86 set calls; faceto 12 new 0 update 0 stop; loco 50/24/24; lookat 0/0/0; body intentions 0
```

**The 24 updates are the point of the second pass.** An *actor* target needs no update as it moves — the component
re-resolves it every tick, which is retail's design and why the first attempt measured `50/0/24`. What takes the update
branch is a change of the request's *parameters*, so `-dislocowalk` re-states every order ten seconds later at the RUN
speed index: `FDisDesireRequest::GetRequestStatus` answers Unchanged for the target, `FDisLocoRequest::SetParams` raises
`DTDRS_UpdateRequestNeeded` because the speed index differs, and the order goes down
`FArkComponentLocomotion::UpdateLocoToActor` → `FArkRequestManager::UpdateRequestByIdx`. That is what a behaviour
escalating from a walk to a chase does. Six of the 24 orders are actor targets on another NPC, so the actor path and the
location path are both exercised.

`faceto 12` is the change agent DF predicted: `bAlwaysStrafe` is FALSE, so `EnsureProperRotation` withholds the facing
until arrival — and arrival could not happen before. Twelve NPCs have now arrived somewhere and asked to face something.

### The pictures

Three frames of the Tower Waterlock courtyard from **one fixed camera**, at **+1 s, +5 s and +12 s after the order**:
`build/agentDN/walk_before.png`, `walk_mid.png`, `walk_after.png` (raw bitmaps beside them). Three separate runs whose
command lines differ only in `-dislocoshot=<seconds>`. At +1 s two guards stand where the spawner put them; at +5 s both
have walked in under the arch and a third is visible behind them; at +12 s the courtyard is empty. The camera is the same
in all three — its position is logged in each run, and `-dislocowatch` freezes the player's pawn
(`setPhysics( PHYS_None )`, `bCollideWorld = FALSE`) precisely so that it is, because a walking pawn drifts and a pair
taken ten world-seconds apart would otherwise compare two different places rather than two positions of the same guard.

**The screenshot is raised by the census, not by `-apshottime`, and that is a finding rather than a preference.**
`-apshottime` keys on `FSceneViewFamily::CurrentWorldTime`, which is per-world and restarts at zero when the game
travels: on a loaded machine the same command line captured the tower once and the **startup map's boat ride** the next
time, because the startup map's own clock reached the mark before the travel. `-dislocoshot` keys off the `-disai`
census, which only ever runs on the world that holds the NPCs, so it cannot pick the wrong map; it logs the map name with
the request. This is `PHASE10`'s "measure the map you mean", arriving from a third direction — after a one-shot probe and
after a colour grade, now after a screenshot.

## What is faithful, what is bring-up, and what is absent

This is the honest split, because the package could not be finished at retail fidelity in one pass: retail's locomotion
is 130 functions in six Engine units plus `arkpathbuildutils.cpp`, about 67 KB of code.

**Faithful ports** (transcribed from the decompile, rva at every site):

* `FArkRequestManager<T>` — the whole priority queue, `Engine/Inc/arkrequestmanager.h` (retail's own home for it;
  2012 PDB attributes all 25 instantiated bodies to that header). `Initialize` / `AddRequest` / `UpdateRequestByIdx` /
  `RemoveRequestByIdx` / `RemoveAllRequestsFromAsker` / `GetRequestIdxByID` / `Update`.
* The component's **616-byte layout**, all 104 members, from the 2012 PDB, asserted at compile time along with the
  fourteen structs it is built from (`checkAtCompileTime( sizeof( FArkComponentLocomotion ) == 616 )`).
* The public request surface: `StartLocoToLocation` / `StartLocoToActor` / `UpdateLocoToLocation` / `UpdateLocoToActor` /
  `StopLoco` / `StopAllLocoFromAsker` / `GetActiveRequestTargetedLocation` / `GetAskedTargetLocOfActiveRequest` /
  `GetActiveRequestMaxSpeedIdx` / `ComputeHasRequestWithPathComputedFlag` / `OnRequestManagerEvent`.
* Lifetime and configuration: `Starting`, `Stopping`, `Enable`, `Disable`, `IsDisabled`, `OnEnable`, `OnDisable`,
  `SetConfig`, `IsConfigValid`, `GetSpeedMode`, `GetForwardSpeedOfSpeedMode`, `GetFinalSpeedMultiplier`, `SetModifier`,
  `GetLocomotionComponent`, `GetPawnGroundLocation`, `HandleTeleport`, `AllowToFall`.
* **The nav-mesh four**: `FindNearestLocationOnNavMesh` (0x545660), `UpdateStartLocAndVerifyIfOnValidPoly` (0x53ed10),
  `CanNPCPathfindFromLocation` (0x546770), and the path chain `UpdatePathFindingEdges` (0x5468f0) →
  `UNavigationHandle::FindPath` → `DefaultPathBuilder` (0x545910) → `BuildStraightPath`.
* **All eight of `arkpathbuildutils.cpp`**: `AdjustPathFindingEdges`, `AddAdjustedPathFindingEdge`,
  `IsEdgeBorderExtentReachable`, `MinimizeStraightPath_FunnelAlgorithm`, `ApplyFunnelAlgorithm_FindTurn`,
  `AdjustStraightPath`, `BuildStraightPath`, `FindFirstEdgeIndex` — Arkane's funnel, transcribed including its literals
  (the 0.45 narrow-edge fraction, the 0.1 shared-vertex epsilon, the 0.0001 same-point epsilon).
* The AI's half of the contract: `ADishonoredNPCPawn::SetupPathfindingParams` (0x74ad20),
  `SetupPathGoalsAndConstraints` (0x75e490), `UDishonoredAIBrain::GetPathGoalsAndConstraintsFromBehavior` (0x700d40),
  `UDishonoredAIBehavior::GetDefaultPathGoals` / `GetDefaultPathConstraints` / `CallGetPathGoals` /
  `CallGetPathConstraints` (0x6eabc0 / 0x6eaca0 / 0x6eac30 / 0x6ead10), `ADishonoredNPCPawn::Spawned`'s locomotion half
  (0x752270), `physWalking` (0x74e8a0), `ADishonoredNPCController::GetMoveTargetLocation` (0x74e720),
  `DisGetPawnFeet` (0x7baff0).
* The move: `MovePawn` (0x543d00), `UpdateMoveSpeed` (0x53e220), `ComputeTargetMoveSpeed`'s braking-distance rule
  (0x548050), `ComputeIsArrivedFlagAndSq2DDistToPathEnd` (0x544390), `ComputePathMove`'s corner blend (0x548410),
  `ApplySteering`'s inertia blend (0x54a630), `ClampMove` (0x544490), `HandleArrival`'s event bookkeeping (0x5487b0),
  `IsThereAnotherForceThanPath` (0x541a00), `UpdateSharedProperties` (0x546260), `AddSteeringForce`, `IsRootMotion`,
  `ComputeMoveYaw`, `GetDirectionToNextPathPoint`.

**Bring-up reconstructions** — retail's structure and call order, simpler arithmetic, each named at the site:

* `ComputeDynamic` (0x54bec0, 3,026 b). The call order is retail's, read off the decompile; the calls it makes into the
  unported animation and avoidance systems are left out.
* `FullMovePawn` (0x53fb00, 4,707 b). Retail rolls its own swept move out of `UWorld::MultiPointCheck` plus a step-up,
  a wall slide and its own touch and bump events; this uses `UWorld::MoveActor`, which is UE3's own swept move and
  already does all four. What is lost is named at the site.
* `ComputeTargetMoveYaw` (0x547950) and `UpdateMoveYaw` (0x541330), 3.5 KB between them of multi-directional speed
  blending against the anim node's strafe set: with no anim node there is no strafe set, so the target yaw is the path's
  yaw and the turn is at the speed mode's own maximum rotation speed without retail's acceleration curve.
* `UpdatePathProperties` (0x541ee0) — the corner advance, the two path directions and the end flag are retail's; the
  turn-ahead bookkeeping the root-motion system needs is not.
* `ModerateMovePawn` (0x540d80) declines every frame, so every frame takes the full move. Retail's cheap path writes
  `Location` directly and takes Z from the start poly's plane, which is only sound while the multi-point check that this
  package leaves to `MoveActor` keeps it sound.
* `ComputeMostRelevantSpeedIdx` (0x53f090) answers 0, which is retail's own fall-through when there is no anim node.
* `IsConfigValid` (0x53f4c0): the five structural checks are retail's; the skeletal-mesh-name and per-foot bone checks
  are not, because they need the anim sets resolved against the pawn's mesh.

**Not ported, named with addresses** — every one of these needs a system this package does not own:

| what | 2013 rva | blocked on |
|---|---|---|
| `arkcomponentlocomotionrootmove.cpp`, all 15 (`UpdateTurn` 0x54b9d0, `UpdateStartMove` 0x546f60, `UpdateStopMove` 0x547300, `RequestTurn` 0x54af40, `EvaluateStaticTurn` 0x54b4f0, `EvaluatePathTurn` 0x54b880, `EvaluateStartPathTurn` 0x54b780, `ResetTurn` 0x545a10, `ResetStartMove` 0x545ae0, `ResetStopMove` 0x545b70, `SetRootState` 0x5459a0, `IsReadyToTurn` 0x53e7a0, `IsSpeedModeCanDoTurn` 0x54ae10, `IsTurnNotTooCloseFromPathEnd` 0x542b20, `IsLocoFaceToRequestActive` 0x53e750) | — | `UArkAnimNodeLocomotion` |
| `ComputeDynamic`'s avoidance half: `DetectPreAvoidanceCollision` 0x543f70, `HandlePushPeriod` 0x548b90, `ComputePriorityOfNPCsFromSharedProps` 0x5491b0, `HaveAvoidanceAffector` 0x53e2f0 | | `FArkComponentAvoidance`, `UArkAvoidable` methods |
| `arkcomponentlocomotionfollow.cpp`: `UpdateFollow` 0x544da0, `ComputeFollowLocation` 0x5448a0, `InitFollowProperties` 0x544780 | | nothing in the tower follows |
| `UpdateFaceTo` 0x543190, `UpdateLookat` 0x543490, `UpdateMesh` 0x53de20, `UpdateAnimNode` 0x53dea0, `UpdatePitchAndRoll` 0x53eb30, `ComputeTargetPitchAndRoll` 0x53e9e0, `ComputeLookatSpeedMultiplier` 0x543da0 | | `FArkComponentFaceTo`, `FArkComponentLookat`, `FArkComponentMeshOffset`, `UArkAnimNodeLocomotion` |
| `CanBeTeleportedToLocationWithoutBeingSeen` 0x53e310, `FindValidLocation` 0x53ed70, `SendTouchAndBumpEvents` 0x53e0d0, `UnTouchActors` 0x53fa60 | | vision / the hand-rolled move |
| `FArkComponentManager` and its six `FArkComponentPolicy` instantiations (2012 PDB, 180 bytes) | | nothing creates one; `UWorld::m_pComponentManager` is a forward declaration |
| `UDisTweaks_NPCPawn`'s own pathfinding extent, the two floats at +452/+456 the decompile reads | | the tweaks member names for those offsets are unresolved; the pawn's cylinder is used, which is never larger |

## The four other defects this package found

**DEFECT 2 — `FPathStore::EdgeList` is a `TArrayNoInit`, and the component is heap-allocated with a plain `new`.**
`Src/arkcomponentlocomotion.cpp`. `FPathStore` is a reflected script struct, so its array's constructor deliberately
leaves the count, the capacity and the allocation pointer as whatever was on the heap. The first
`m_RawPathFindingEdges.EdgeList = handle->PathCache.EdgeList` then frees a garbage pointer. It is an access violation on
the first frame any NPC asks for a path, and it was diagnosed rather than guessed: a set of once-only
`DISHONORED(bringup): loco trace` lines through the spine reached `pf_findpath_done: found 1 edges 0` and died before the
next statement. The traces are kept, because they are the tool that will find the next failure here.
**Worth carrying forward: any non-`UObject` that owns a reflected struct by value has to zero it itself.** Retail gets
away with it because Arkane's component allocator memsets; ours is `new`.

**DEFECT 3 — the give-up clock and the thing that resets it were chasing each other.** `ResetPathProperties` (faithfully)
zeroes `m_fTryToReturnOnNavMeshDuration`, which is the clock the "stop trying to get back on the mesh" decision is
measured against. Calling retail's `HandleTeleport` from the teleport branch — which this package did at first, and
retail does not — put the clock back to zero on every successful teleport. Measured: **191,404 teleports and 20,411
failed searches in 46 seconds**, which slowed the whole game to 4 % of real time. Three separate latches fixed it, and
each is a one-line clause with the measurement in the comment: the teleport is raised once per stretch off the mesh, a
request whose start poly cannot be found gives up until the request changes, and a search that already failed is not
retried until the destination moves. 20,411 failures became **2**.

**DEFECT 4 — an obstacle forced a fresh A\* every frame.** `FullMovePawn` raises `m_bForcePathComputation` when the move
is blocked, which is right; doing it every frame a pawn is wedged is not. **76,098 searches in 57 seconds from 26 NPCs.**
Retail rate-limits this through the push period (`HandlePushPeriod`, 0x548b90), which is not ported; ours fires once per
request.

**DEFECT 5 — `HEAD 9a04708` does not compile with `DISHONORED_ENABLE_GFXUI=ON`.** Not this package, and not this
package's file: `GFxUI/Src/gfxuiengine.cpp:2288` calls the **private** four-argument `FGFxEngine::InputKey` from the free
function `DishonoredGFxInputKey`, which is `error C2248`. `GFxUI` belongs to agent DM this wave, so this worktree carries
the smallest possible local fix — a forward declaration and a `friend` line in `GFxUI/Inc/gfxuiengine.h`, both tagged
`DISHONORED(bringup): agent DN, local build fix, NOT part of this package` — so that the package could be built and
measured at all. **Coordinator: this is DM's to resolve; drop my two lines if DM's own fix lands first.**

## Retail-versus-tree differences worth carrying forward

1. **`EPathFindingError` is not the reference enum.** The 2012 PDB has six entries (`STARTPOLYNOTFOUND`,
   `GOALPOLYNOTFOUND`, `ANCHORPYLONNOTFOUND`, `MAXIMUMVISITEXCEEDED`, `NOPATHFOUND`=4, `INVALIDNAVMESH`=5);
   `EngineClasses.h` has the reference's seven (`NOPATHFOUND`=3, `COMPUTEVALIDFINALDEST_FAIL`,
   `GETNEXTMOVELOCATION_FAIL`, `MOVETIMEOUT`). **The values collide**, so a saved or logged error code means different
   things in the two. This package uses the tree's names and says so at the site rather than renumbering an enum the
   whole nav-mesh runtime reads.
2. **`UNavigationHandle::FindPath` has a third parameter in retail.** 2013 rva 0x295b40:
   `FindPath(AActor**, INT*, FPathStore* out_PathCache)`, and so do `GeneratePath` and `PathCache_Empty`; passing NULL
   means "use `this->PathCache`", which the tree's two-parameter signature already does. Because `PathCache` is already
   an `FPathStore` in the tree, calling the reference `FindPath()` and copying `PathCache` out is behaviourally retail's
   NULL case. Changing the signature means touching `GeneratePath` and `AddSuccessorEdgesForPoly` in the 22,000-line
   `UnNavigationMesh.cpp`, which this package does not need.
3. **`APawn::SetupPathfindingParams` and `GetEdgeZAdjust` are `const` in retail** (`?SetupPathfindingParams@APawn@@UBE…`)
   and non-`const` in the tree's `IInterface_NavigationHandle`. Matching retail means changing the interface and every
   implementor — `AController`, `ACrowdAgentBase`, `AGameCrowdAgent`, `APylon`, `ADishonoredGameInfo`,
   `ADisDLC07GravehoundSpawner`. Left as the tree has it, named here.
4. **`DisGetPawnFeet` takes the cylinder's *bounds*, not `Location` minus `CollisionHeight`** — so a pawn whose cylinder
   has moved (crouching, possessed) answers where its cylinder really is.

## Agent DP's shim audit, checked against this package's actual path

DP flagged `ANavigationPoint::PathList` as its number one "most likely to bite next" and named this package. Checked
rather than assumed, and the answer is specific:

* **`ANavigationPoint::PathList` is not on this package's path.** In the whole nav-mesh runtime it is read from exactly
  one place at run time: `UNavMeshGoal_ClosestActorInList::SeedWorkingSet` (`UnNavigationConstraintsAndGoals.cpp:674`,
  through `UNavigationHandle::DoesPylonAHaveAPathToPylonB`). This package's searches use `UNavMeshGoal_At` and
  `UNavMeshPath_Toward`, which is what retail's `GetDefaultPathGoals` / `GetDefaultPathConstraints` supply
  (0x6eabc0 / 0x6eaca0). The other `PathList` uses in `UnNavigationMesh.cpp` (9861, 12792, 6849) are pylon *build* code.
  **So the exposure for AI pathfinding is one goal evaluator, not the runtime** — worth knowing, because it also says
  which evaluator must not be used until `PathList` is resolved.
* **`AController::MoveTarget` / `CurrentPath` / `NextRoutePath` / `MoveTimer` are not on it either**, and this package
  moves NPCs *off* the code that reads them: `ADishonoredNPCPawn::physWalking` hands the frame to the locomotion
  component instead of to `APawn::physWalking`, and it is `APawn`'s reference walking path (`setMoveTimer`,
  `moveToward`, the serpentine code in `UnPawn.cpp`) that touches all four. An NPC with a locomotion component never
  enters it. `ADishonoredNPCController::GetMoveTargetLocation` reads the component, not `MoveTarget`.
* **`UNavigationHandle::Breadcrumbs` and `BestUnfinishedPathPoint` are inert here.** They are read by
  `GetNextMoveLocation` / `HandleNotOnPath` / `GetBestUnfinishedPathPoint` — the reference *move* path. This package
  consumes `PathCache` directly and calls none of them.
* **One shim was real, and it is fixed.** `FNavMeshPathParams::SearchLaneMultiplier` is `DISHONORED_SHIM_STATIC`, so it
  is one FLOAT for the whole process, and it is read **inside the A\***, at `UnNavigationMesh.cpp:10752`, to offset
  successor edges into a lane. Neither of this package's two `SetupPathfindingParams` bodies wrote it, so every search
  ran with whatever the last `AController` or `ACrowdAgentBase` had left there. Retail's `FNavMeshPathParams` has no such
  member; every writer in the tree writes 0. Both bodies now write 0 explicitly, with the reason at the site. It is a
  `B-shared-write` in DP's ranking, not an `A-fatal`, and nothing observable changed — which is the point: it was a
  cross-instance dependency inside the search that nobody would have found from a symptom.

## The one placement compromise, and the generator rule that undoes it

`FArkComponentLocomotion` is declared and compiled in **DishonoredGame**, not in Engine where retail has it
(`Engine/Src/arkcomponentlocomotion*.cpp`). One reason: the component cannot compile without
`UArkComponentLocomotionConfig`, and that reflected **Engine-package** class is declared in
`DishonoredGame/Inc/DishonoredGameEngineShims.h` — one of **113 Engine-package classes** the generator puts in this
module because the Engine headers lack them — and `IMPLEMENT_CLASS`'d in `DishonoredGame/Src/DishonoredGameRegistrants.cpp`.
Engine cannot include a DishonoredGame header.

Moving the unit to its retail home is therefore one decision, not a rewrite: declare `UArkComponentLocomotionConfig`
(and `UArkAvoidable`, `UArkAnimNodeLocomotion`) in `Engine/Inc/EngineArkaneClasses.h`, at which point the generator's own
rule drops them from the shim header by itself — `gen_classes_header.py`, `sdk_select`:
`if r["package_2013"] in SHIM_PACKAGES and cpp not in d.declared`. The Engine stub files
(`Engine/{Inc,Src}/arkcomponentlocomotion*.cpp`, `arkpathbuildutils.cpp`, `arkrequestmanager.h`) keep their skeleton
banners and the unit names match retail's, so the move is a file move plus that declaration. `arkrequestmanager.h` is
already in Engine, because the template needs nothing but `FArkComponentBase`.

**Note the header shadowing**: `DishonoredGame/Inc/arkcomponentlocomotion.h` shadows the Engine skeleton of the same
name for DishonoredGame translation units, because `DishonoredGame/Inc` precedes `Engine/Inc` on the include path. The
Engine file is comment-only, so nothing is lost; it is stated here because it would otherwise be a surprise.

## What asks them to walk, and why a probe

The two things that make a retail guard walk are `DisBehaviorGuard`'s patrol and the Kismet AI actions
(`UDisSeqAct_AISetPatrol`, `UDisSeqAct_AIGoToActor`), and both are separate unported packages. Agent DF measured that the
only behaviour any tower NPC reaches is `DisBehaviorIdle`, whose `DisAISubStateStand` asks to stand **where it already
is** — so the tower's NPCs, correctly serviced, stand still. That is why the 26 requests arrive instantly and why the
"before" column of the census is honest rather than broken.

`-dislocowalk[=<seconds>]` therefore gives them somewhere to go, and it is careful about which half is the probe. It
reaches into each NPC's **live sub-state**, takes that sub-state's own `FDisLocoRequest` — agent DF's, the one the census
counts — and calls `RequestLocationTarget` on it with a poly centre from the mesh. Everything downstream is the real
path: `FDisLocoRequest::DoRequest` → `DisDesireStructs::StartLoco` → `FArkComponentLocomotion::StartLocoToLocation` → the
request queue → `UpdatePathOfHigherPriorityRequest` → `ADishonoredNPCPawn::SetupPathGoalsAndConstraints` →
`UNavigationHandle::FindPath` → `AdjustPathFindingEdges` → the funnel → `ComputeDynamic` → `MovePawn`. Only the
*initiator* is the probe. The sub-state does not fight it: `FDisDesireRequest::GetRequestStatus` compares the resolved
target against the one stored in the request, which is now the probe's, so it answers Unchanged.

`-dislocowatch=<n>` puts the camera behind `DishonoredNPCPawn_<n>` once, for the screenshots. It moves the player's own
pawn and nothing else; it is in the same family as `-apshottime`.

**A finding that came out of choosing the destination**, worth recording because it is a fact about the map and not about
the code: sending every NPC to the poly nearest the *player* produces **zero** paths and zero movement, because the
player spawns at the foot of the tower and every such destination is ~470 units below the NPC — reaching another floor
needs the stair edges, and the reference A\* did not find them from those starts. Farthest-poly-on-my-own-level produced
real paths and real walking; nearest-to-the-player produced none. The two runs are otherwise identical.

## Why 14 of 26 and not 26 of 26

Stated plainly, because it is the difference between this and retail:

* **11 NPCs are not on the navigation mesh when they spawn** — the spawners put them on stairs, balconies and ledges the
  cooked mesh does not cover. Retail's answer is a teleport, which is ported (`m_bTeleportRequired` /
  `m_TeleportLocation` / `UWorld::FarMoveActor`) minus its visibility test
  (`CanBeTeleportedToLocationWithoutBeingSeen`, 0x53e310), and it recovers six of them: 15 of 26 on a poly becomes 21.
  The rest report `CPNT_LOCO_EVENT_PATHFINDING_FAILED` once and stand, which is what retail's sub-state would be told.
* **Some walk into things and stop.** With no turn-in-place, no start or stop animation, no avoidance and no crowd
  give-way, a guard that meets a doorframe at an angle wedges. It reports `CPNT_LOCO_EVENT_HIT_OBSTACLE` to its
  sub-state, repaths once, and if that does not help it stays there. A guard that walks badly is more useful than one
  that stands still, and the census says which is which per NPC.
* **Two of 26 had no reachable target** within 1,500 units on their own level.

## Gates

* `build\agentDN_release.cmd` for `DishonoredGame`, `CoreSmoke` and `LayoutProbe`: **0 errors**
  (`build/agentDN_build21.log`, `agentDN_build_cs.log`, `agentDN_build_lp.log`).
* **A clean full release build from an empty directory**: `build\agentDN_clean.cmd` configures and builds
  `build/agentDN_wt` into a fresh `build/agentDN_clean`, all three targets, **0 errors**, 934 units
  (`build/agentDN_clean_build.log`, `agentDN_clean_cs.log`, `agentDN_clean_lp.log`). It emits 312 warnings and **0 of
  them come from this package's files** — they are the pre-existing C4316/C4100/C4335 of `External/GFx3`,
  `External/PhysX284` and the renderer, which an incremental build does not re-show.
* `python resources/tools/run_regression.py --build-dir build/agentDN --no-build` → **31 ok, 0 failed, 0 skipped**, 429 s
  (`build/agentDN/regression/summary.txt`). `unported_natives` **0**, `probe_natives` **6** (bound 12), all three
  `*_criticals` **0**.
* **0 `Critical`** in every game run: **420 s** null RHI on `L_Tower_P`, 169 s d3d9, and both screenshot runs.
* No new unported natives: this package adds no exec wrappers at all. Everything it ports is C++ the AI calls directly.

## Generated files — read this before merging

**One line of one generated file changed**: `DishonoredGame/Sources.cmake`, which is produced by
`gen_classes_header.py --sources-cmake`. `Src/dishonorednpcpawn_locomotion.cpp` came off the `DishonoredGame_EXCLUDE`
skeleton list because it now has code, and the skeleton count in the header comment moved **834 → 833**. That is exactly
what the regeneration produces — the generator's rule is `is_comment_only(p)` — so the coordinator's

```
python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake
```

reproduces it. **Nothing else this package touches is generated**: the five new units are new files the module's
`Src/**/*.cpp` glob picks up, and the three `Inc/CppText/` headers are hand-written hooks, not output. No regeneration
was run in this worktree.

## Files

New, `DishonoredGame` (2,943 lines): `Inc/arkcomponentlocomotion.h`, `Src/arkcomponentlocomotion.cpp`,
`Src/arkcomponentlocomotiongoto.cpp`, `Src/arkcomponentlocomotionpath.cpp`, `Src/arkcomponentlocomotiondynamic.cpp`,
`Src/arkpathbuildutils.cpp`.

Rewritten: `Engine/Inc/arkrequestmanager.h` (skeleton → the template),
`DishonoredGame/Src/dishonorednpcpawn_locomotion.cpp` (skeleton → the pawn's half).

Edited (1,222 insertions, 62 deletions across 18 files): `Engine/Inc/EnginePawnClasses.h`, `Engine/Src/UnPawn.cpp`,
`DishonoredGame/Inc/CppText/{ADishonoredNPCPawn,UDishonoredAIBehavior,UDishonoredAIBrain}.h`,
`DishonoredGame/Inc/{disaicensus,disdesirestructs,dishonoredutilities}.h`,
`DishonoredGame/Src/{disaicensus,disdesirestructs,dishonoredaibehavior,dishonoredaibrain,dishonorednpccontroller,dishonoredutilities_accessors}.cpp`,
`DishonoredGame/Sources.cmake`, and `GFxUI/Inc/gfxuiengine.h` (DEFECT 5, not this package).

`agentDN_status.csv` is generated from the tags in the tree (`build/agentDN_work/mkstatus.py`), so it cannot drift from
it: **509 tagged sites, 318 with a 2013 rva** — 310 `port`, 123 `bringup`, 43 `layout`, 29 `written`, 4 `retail`.

Tooling left behind, all in `build/agentDN_work/`: `ty.py` (dump a 2012-PDB type's members), `xrefs.py` (who calls this
rva — this is what settled that `physWalking`, not the component tick, is where retail moves an NPC), `resolve.py`,
`crlf.py`, `mkstatus.py`, and the eight idempotent patch scripts `p01..p08`. Decompiles in `build/agentDN_decomp12/`
(127 locomotion functions) and `build/agentDN_decomp12b/` (the nav and pawn side).

## What this package did NOT get to measure

This snapshot is `9a04708`. `ee2e413`, `a96b8e4` and **`1924264`** landed after it, and the third is the one that makes
`L_Boyle_Ext_P` and `L_Streets1_P` run. **Locomotion has therefore only ever been measured on `L_Tower_P`**, and the two
questions the other maps answer are worth asking on the merged tree, in this order:

1. **Does the cooked navigation mesh exist there too?** `-disai` prints the answer in one line as soon as a world with
   NPCs exists — `disai navmesh: N pylons (… with a mesh), P polys, V verts, E edges` — and if it does, `-dislocowalk`
   needs nothing else.
2. **How many of that map's NPCs spawn off the mesh?** On the tower it is 11 of 26, and that single number is what
   decides how much of the "why 14 of 26" section applies anywhere else. If a street map spawns its guards on the mesh,
   the same code walks more of them.

## Hand-overs

1. **`UArkAnimNodeLocomotion` is the next package and it is the one that makes the walk look right.** Everything left in
   `arkcomponentlocomotionrootmove.cpp` — turn in place, start and stop animations, the whole root state machine — is
   blocked on it, and so are `UpdateMesh`, `UpdateAnimNode` and `ComputeMostRelevantSpeedIdx`. The component already
   carries `m_bCanManageAnim` and `m_pAnimNodeLoco` and reads them defensively everywhere, which is a path retail itself
   has, so landing the anim node turns those on without touching the rest.
2. **`FArkComponentFaceTo` and `FArkComponentLookat` are now the shortest path to a visible improvement.** The census
   already says the AI is asking: `faceto 12 new`. Agent DF's `DisDesireStructs` seam has the same shape for all three
   kinds; this package converted the loco third of it (`IsLocoRequestStarted` / `StartLoco` / `UpdateLoco` / `StopLoco`)
   and left the other two exactly as DF wrote them, so the same four functions per kind is the whole job.
3. **`FArkComponentAvoidance` / `UArkAvoidable`'s methods.** 26 guards walking with no give-way rule is why some of them
   wedge. `ComputePriorityOfNPCsFromSharedProps` (0x5491b0) already has its data structure in the tree — the static
   `ms_LocoCpntSharedProps` table is populated every frame by `UpdateSharedProperties`; nothing reads it yet.
4. **Move the component to Engine**, per "The one placement compromise" above. It is one declaration plus a file move,
   and it is what lets `interptracklocomotion.cpp` (matinee-driven locomotion, agent DM's area) use it.
5. **The nav mesh works. Anything that wants a path can now have one.** `FDisAIMonitorPawnReachability` (agent CG's
   hand-over 3, 6 blocked natives) needs `UNavigationHandle::PointReachable` and `SetupPathGoalsAndConstraints( dest,
   bForReachability=TRUE )` — both live, and the `bForReachability` parameter is already threaded through.
6. **The `EPathFindingError` collision (difference 1) should be settled before anything serialises a path error.**
7. **Agent DF's hand-over 1 — make C4263 and C4264 errors — is still open, and this package did not do it.** It is a
   change to the whole tree's warning set, and turning it on while three agents have unmerged work is how a build breaks
   for someone who did not touch anything. What this package did instead is verify its own six new overrides by
   *measurement*: the once-only `DISHONORED(bringup): loco trace` lines through the spine fire in order —
   `spawned_enter`, `spawned_tweaks`, `spawned_navhandle`, `spawned_component`, `spawned_setconfig`, `starting_enter`,
   `phys_enter`, `tick_enter`, `dyn_enter`, `sched_enter`, `pf_enter`, `goals_enter`, `pf_findpath`, `pb_enter`,
   `move_enter` — so every override in the chain is reached, which is the thing C4263 would have caught. They are kept
   deliberately: they are what turned "it crashes somewhere in locomotion" into "it dies between `pf_findpath_done` and
   the next statement" in one run, and they cost one branch per call after the first.
8. **`-dislocowalk`, `-dislocowatch` and `-dislocoshot` are measurement switches, not gameplay**, and they should be the
   first thing deleted once a behaviour asks an NPC to go somewhere by itself. `DisBehaviorGuard`'s patrol and
   `UDisSeqAct_AISetPatrol` / `AIGoToActor` are what replaces them, and neither needs anything from this package that is
   not already in place.
