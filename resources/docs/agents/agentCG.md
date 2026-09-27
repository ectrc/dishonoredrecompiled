# Agent CG — The AI brain root: an NPC exists, has a mind and thinks (2026-09-27)

Package CG of `resources/docs/PHASE9.md`. Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentCG.i64`, a
copy of `retail2013_named.i64`); every "2013 rva" is a headless decompile in `build/agentCG_decomp/r13`, and the "2012"
rva in brackets is `shipping2012_agentCG.i64` (`build/agentCG_decomp/s12`, 758 functions), which carries the symbol
names and is the readable version of the same function. Status rows: `agentCG_status.csv` (260 rows).

**Verified against HEAD `844afbe`** (wave 8's tracker commit), not only against the `2cdd7b3` this package started
from: the snapshot worktree was rebased onto it and the whole package rebuilt (0 errors, 0 unresolved symbols,
`build/agentCG_build26.log`) and re-measured there — **31 checks ok, 0 failed, 0 skipped**, and the census below is the
run on that build. Note commit `aedbdaa` ("Back out agent CG's in-flight component work from EngineArkaneClasses.h"):
that back-out is correct and this package's own `EngineArkaneClasses.h` edit is in its file list, so the merge puts it
back with the rest of the component container.

**Read this first if you inherit the package.** The first mission map holds **41 `DishonoredSpawner` actors and zero
placed NPC pawns**. Nobody had measured that before, and it is why every earlier attempt at the AI stopped: porting the
brain, the behaviours and the sub-states does nothing at all until the spawner runs, because there is no NPC for them to
belong to. The census that found it is the single most useful thing in this package.

## Result against the accept line

| Acceptance | State |
|---|---|
| ≥ 120 natives ported | **not met — 34.** The measured ceiling today is 81 of the 236 remaining stubs, and the reason the rest cannot honestly be ported is one further root, named and costed below. The classification is data, not opinion: `build/agentCG_work/classify3.py` |
| **an NPC exists, thinks and moves**, evidenced by a census | **exists and thinks: met and measured. Moves: not met**, and the two things it needs are named precisely below. On `L_Tower_P`: 0 → **26 NPC pawns, 26 controllers, 26 brains, all 26 initialized**, 26 behaviour activations, 684 sub-state entries, 1.06 M sub-state ticks, 104 sub-processes begun, 34 stims raised and processed |
| no new warn-once line on the walking path | **met.** `unported_natives` stayed **0** — but only after the NPCs found it: the metric went 0 → 1 the moment they existed (see "The metric earning its keep") |
| `run_regression.py --no-build` green, 31 checks | **met: 31 ok, 0 failed, 0 skipped.** `probe_natives` also came **down from 7 to 6** |
| 0 criticals | **met** on every run: 180 s d3d9 on `L_Tower_P`, 150 s null RHI on `L_Tower_P` and on `L_Pub_Day_P`, and all four harness stages |

## The census, before and after

`-disai` is the measurement (free when the switch is absent, hung off the same per-frame script call as agent AU's
`-dispickup`, so no engine file is touched for it). It reports the NPC population, how many of them have a mind, what
each mind is doing, how far each NPC has moved and turned, and the flow through the AI — stims, behaviour activations,
sub-state entries and ticks, sub-processes, move requests.

**Before** (HEAD `2cdd7b3`, this package's own baseline build, `build/agentCG_build0.log`):

```
disai census: 0 NPC pawns, 0 controllers (0 with a brain, 0 initialized), 41 spawners; ...
disai no NPC pawn in l_tower_p; top actor classes: TargetPoint=91 Emitter=75 StaticMeshActor=60
    DishonoredSpawner=41 DishonoredNavPoint=31 StaticMeshCollectionActor=30 InterpActor=28 ...
```

**After** (180 s d3d9, `L_Tower_P`, `agentCG.log`):

```
disai census: 26 NPC pawns, 26 controllers (26 with a brain, 26 initialized), 41 spawners;
  moved 0.0 uu this second (0 moving), turned 0; brain ticks 1061372, stims 34/34,
  behaviors 684 init 26 activations, substates 684 enters 1061346 ticks, subprocesses 104,
  callbacks 0, move requests 0
disai thoughts: [DishonoredNPCPawn_0 ctrl DishonoredNPCController_0 brain init
  behavior DisBehaviorIdle substate DisAISubStateInit moved 0.0 turned 0] [DishonoredNPCPawn_1 ...
```

Read that line left to right and it is the whole chain: the spawner makes a pawn, the actor factory gives it a
controller, the controller builds a brain, the brain builds 26 behaviours each with its own sub-state machine, a
BrainInit stim is raised and processed, the idle behaviour answers it and takes slot 0, and its sub-state machine enters
`DisAISubStateInit` and ticks. `moved 0.0` is honest: see "What moving still needs".

`L_Pub_Day_P` reports 0 NPC pawns and **0 spawners** — the hub's population is not spawner-driven (a scan of the cooked
packages finds `DishonoredSpawner` only in `L_Tower_Script.upk`), so the hub needs `DisGameCrowd` or the Soiree
sub-level, neither of which is this package's. The first mission is the evidence.

## What is ported (260 status rows across 8 subsystems)

**1. `UDishonoredAIBrain` (48)** — `InitBrain` 2013 rva **0x7252a0**, `TickBrain` **0x724a10**, `TerminateBrain`
**0x726720**, `RefreshBrainThoughts` **0x720390**, `InitBrain_Processes` **0x70edf0**, `InitBrain_Steering`
**0x711cf0**, `TickBrain_Processes` **0x720320**, `RefreshBrain_Processes` **0x70b4a0**, `TickBrain_Stims` **0x70b510**,
`ProcessAllStims` **0x717100**, `FlushStimQueue` **0x716eb0**, `ProcessOneStim` **0x711960**, `EnqueueStim`
**0x7148c0**, `AddBehaviorFromTweak` **0x70b420**, `GetAIBehaviorTweakForSlot` **0x70ed10**, `SupportsBehavior`
**0x70b580**, `IsBehaviorOnStack` **0x703ca0**, `IsCurrentBehavior` **0x700d80**, `GetBrainProcess` **0x70b710**,
`Forget` **0x726920**, `RefreshRelationshipStatus` **0x723a90**, `OnDifficultyChange` **0x722ad0**, the flag, suspicion
and sense accessors, and the attention accessors `FDisAttentionProxy` reads.

**Three ways retail 2013 differs from the 2012 build, all of which a 2012-faithful port would have got wrong:**
* `m_BrainInhibitors` (2012 @224) and `AddBrainInhibitor` / `RemoveBrainInhibitor` / `IsBrainInhibited` **do not exist
  in retail 2013**. None of the three has a match in `match_2012_2013.csv`, the member is absent from the 2013 reflected
  layout, 2013's `InitBrain` has no inhibitor clear where 2012's has one, and 2013's `TickBrain` has no
  `m_BrainInhibitors.Num() <= 0` gate where 2012's does. **Reintroducing that gate stops every brain from ticking.**
* `m_ActiveBehaviorStack` has **19** entries in 2013 and 18 in 2012; every loop over it runs 0..18 (2013's
  `ProcessAllStims` ends `while ( v8 < 19 )`).
* `m_MandatoryBehaviors` (2013 @484) is new, and `InitBrain` builds a behaviour from each of its entries *as well as*
  one per behaviour slot of the tweaks.

**2. `UDishonoredAIBehavior` (26)** — `CallInitBehavior` **0x6f7030** (the whole construction: the sub-state machine,
one `UDisAISubStateInit` plus one sub-state per tweak entry, one sub-process per tweak entry, the four stim masks),
`CallTickBehavior` **0x6eaaa0**, `CallRefreshThoughts` **0x6ea9b0**, `CallOnBehaviorPause` **0x6ea8f0**,
`CallOnBehaviorResume` **0x6f7670**, `CallFilterAIStim` **0x6efdf0**, `CallShouldFinishWhileDormant` **0x6eff50**,
`OnBecomeDormant` **0x6f0020**, `RegisterCallbacks` **0x6eaf30**, `GetSubProcess` **0x6eae90**, `SetTweaks_Derived`
**0x6ea740**, the action-target and sub-state accessors.

**3. `UDisAISubProcess` (15), `UDisAISubState` (22), `UDisAISubStateMachine` (14)** — the two roots the package brief
named after the brain, complete. The part that matters most for the next package is
`UDisAISubState::RegisterDelegate_OnEnterCallback` / `_OnResetCallback` / `_OnExitCallback` / `_TickCallback` /
`_RefreshCallback` / `_RequestStateExitCallback` (2013 rvas 0x705a00, 0x705ca0, 0x705bc0, 0x705ae0, 0x70d350, 0x70d490):
**the six callbacks are ordinary UE3 script delegates**, already generated on `UDisAISubState` as
`__OnEnterCallback__Delegate` and friends. `UDishonoredAIBehavior::RegisterCallbacks` composes the function name from the
sub-state's `m_StateSuffix` (flag bits 1 OnEnter, 2 OnReset, 4 OnExit, 8 Tick, 0x10 Refresh, 0x20 RequestStateExit), so
the bound function is `OnEnterCallback_<Suffix>` on the owning behaviour — which is exactly the shape of the ~109
`UDisBehavior*::exec<Kind>Callback_<SubState>` natives that remain. **They are reachable now; what they still need is
below.**

**4. `UDisAIBrainProcess` (7)** — the always-on half of a brain (`InitBrainProcess` **0x77e3d0**, `TickBrainProcess`,
`RefreshBrainProcess`, `TermBrainProcess`, `FilterAIStim_BrainProcess` **0x78d1e0**, which gets first refusal on every
stim).

**5. The AI stimulus system** — `FAIStimStruct` with its six-slot dispatch table, the ~110 concrete stim structs
registered by `DIS_IMPLEMENT_STIM`, `DisDelegate<R,FAIStimStruct>` with the two static binding thunks and
`s_NullDelegate`, `DisNewStim`, `DisHandleAIStim_Internal` / `DisHandleAIStim` / the Multi broadcast form, and the
`FDisStimRef` reference counting. `UDisStimManager` (7): the block pool every stim is allocated from
(`InitStimManager` **0x724990**, `AllocateBlock` **0x724b80**, `AllocateBlock_Common` **0x7275b0**, `ReleaseBlock`
**0x724ba0**, `TermStimManager` **0x73d3f0**).

**6. The Ark component and event infrastructure** — `FArkComponentBase` and its creator registry,
`UArkComponentContainer` (9: `AddNewComponentByID` **0x534100**, `StartAllComponents` **0x5362e0**,
`StopAllComponents` **0x536310**, `RemoveAllComponents` **0x538990**, `Serialize` **0x5364b0**, …) and
`FArkGameEventDispatcher` (6: `GetInstance`, `CreateInstance` **0x5572d0**, `ProcessEvent`, the deferred
registration/unregistration lists and `CheckLeaks`).

**7. The NPC spawn path** — this is what turned 0 NPCs into 26. `ADishonoredSpawner` (10: `PostBeginPlay` **0x65c030**,
`Tick` **0x658d30**, `SpawnOnePawn` **0x658b30**, `DoSpawnNow` **0x65e790**, `OnSpawned` **0x6590e0**,
`ClearPendingSpawns` **0x658c20**, `IsMinDelaySinceLastSpawnElapsed` **0x64b300**, `GetTweaks_Derived` **0x6433b0**) and
`UDisActorFactoryNPCPawn` (4: `CreateActor` **0x75dbb0**, `CreateNPCPawn` **0x74e4b0**, `GetDefaultActor`,
`CanCreateActor`). `ADishonoredNPCController` (29: `PostBeginPlay` **0x74e0b0**, `InitNPC` **0x7549d0**, `Tick`
**0x754ac0**, `ClearComponents` **0x74a850**, `UnPossess` **0x74a900**, `IsDead` **0x74e800**, plus the Kismet AI ops).
`ADishonoredGameInfo::InitGlobalManagers` **0x5e9bd0** and `PostBeginPlay` **0x6155d0** — without them the global AI
manager never initialises its stim pool and no brain can raise a stim at all.

**8. `FDisAttentionProxy` (17)** and `IDisAttentionTargetInterface` (2) — the (brain, target) pair every AI query about
"something I am paying attention to" goes through. Cheap (18 tiny methods, all byte-identical between the builds) and it
unblocked 27 instances of the natives sweep on its own.

**34 natives** in `DishonoredGameNativeStubs.ported.agentCG.txt`, the most useful of which are the eleven
`ADishonoredNPCController::OnAI*` Kismet ops: they are the bridge from a level designer's Kismet graph into the AI, and
they now raise real stims into a real behaviour stack (`OnAIAmbush` **0x7635f0**, `OnAIDoSearch` **0x763770**,
`OnAIGuard` **0x763a80**, `OnAISetPatrol` **0x763bd0**, `OnAIRingAlarm` **0x763fa0**, `OnAISetSenses` **0x74aaf0**,
`OnAISetSuspicionLevel` **0x74e770**, …).

## The four defects this package found, and how

Every one was found by instrumenting before changing, which is the project's standing rule; three of them produced code
that ran and did nothing, which is exactly the failure mode the brief warned about.

**1. The map has no NPCs — it has spawners.** The `-disai` census printed `0 NPC pawns, 41 spawners` and, because it
also dumps the ten most common actor classes when it finds no NPC, it named `DishonoredSpawner=41` in the same line.
Without that the obvious conclusion would have been that the brain port was broken.

**2. The per-subclass settings trap, in its sharpest form yet.** `ADishonoredSpawner::DoSpawnNow` reads the NPC tweaks
through the spawner's own `IDisTweaksInterface`. `ADishonoredSpawner::GetTweaks_Derived` is a **7-byte** function
returning `m_pPawnTweaks`; without it, `IDisTweaksInterface::GetTweaks` answers the all-zero class default of
`UDisTweaks_NPCPawn`, whose `m_pBrainTweak` is NULL, and retail's own `if( !Tweaks->m_pBrainTweak ) return;` refuses
**every spawn on every map, silently**. This is the third wave in a row this trap has appeared (147 pickups consumed for
nothing in wave 7, the pawn killed on every landing in wave 6) and it is worth stating as a rule: **in this codebase,
`GetTweaks_Derived` and `SetTweaks_Derived` are not boilerplate — they are the function.**

**3. The state machine was asked to enter a NULL state.** The brain initialised, built its behaviours, and the first
one crashed the process one second after the first NPC spawned. Tracing `CallInitBehavior` step by step put it between
"init substate constructed" and "fsm inited": `FDisAISubStateInit_Param` was default-constructed, so its
`m_pStateClass` was NULL and agent AJ's `InitFSM` was asked to change to a state class that is not in its map. Retail's
own `FDisAISubStateInit_Param` constructor sets it to `UDisAISubStateInit`.

**4. The stim queue was not holding references, so every stim was processed out of a freed block.** The trace reported
every processed stim with `m_StimID` **128** — which is not a valid `EAIStimID` (the enumeration ends at 121). 128 is the
low byte of a pointer. `m_StimQueue` holds `FDisStimRef` entries and `EnqueueStim` was filling one in by assigning its
two fields rather than through `DisStimRefInit`, so the queue entry never took a reference; the stack reference
`DisHandleAIStim_Internal` holds across the enqueue was therefore the last one, `UDisStimManager::ReleaseBlock` threaded
its free-list pointers through the block, and the queue was left pointing at a freed block whose first eight bytes are
now `m_pPrevFreeBlock` and `m_pNextFreeBlock` — exactly where the vtable pointer and `m_StimID` live. That is retail's
design and the reason `FDisStimRef` is a counted reference at all. Fixing it is what took the census from **0 behaviour
activations to 26**.
A second, smaller one came out of the same trace: a generated stim struct's `EC_EventParm` constructor zeroes
`m_StimID` and nothing sets it, so `DisNewStim` looked up the wrong type info. Retail sets the id in each stim's own
constructor (`FAIStimStruct_BrainInit::FAIStimStruct_BrainInit`, 2013 rva 0x7189e0, is the base constructor plus that one
store); every raise site now goes through a helper that does the same, and `ProcessOneStim` refuses an id outside the
enumeration rather than indexing a mask out of bounds with it.

## The metric earning its keep

`run_regression.py`'s `unported_natives` bound of 0 went **0 → 1** the moment NPCs existed:
`ADishonoredNPCPawn::execTakeDamage`. That is the metric doing precisely what agent BF built it for — an NPC that exists
gets damaged (the spawners on `L_Tower_P` put their NPCs above a drop, so they land), and nothing had ever reached that
native before. It is ported, and `ADishonoredSpawner::execOnStartSpawn` with it (its C++ body was ported but its exec
wrapper was not, so a Kismet StartSpawn still hit the generated stub). Both back to 0, and `probe_natives` 7 → **6**.

## Why 28 natives and not 120 — measured, not estimated

`build/agentCG_work/classify3.py` takes all 236 remaining DishonoredGame stubs with an identifiable retail body, extracts
each body from the decompile, and asks a mechanical question: **does every symbol this body calls exist in the tree?**
Availability is measured by scanning every source file for a definition or declaration, not guessed. The decompiler's
own intrinsics (`qmemcpy`, `LOBYTE`, `SHIDWORD`, …) and the generated class machinery (`StaticClassNoInline`,
`PrivateStaticClass`, a reflected struct's own constructor) are excluded, because counting them as blockers made 44
natives look gated when they are not.

| kind | rows | meaning |
|---|---|---|
| `empty` | 12 | the retail body is one line or a single flag write |
| `selfcontained` | 69 | every callee exists — **portable today** |
| `ownhelper` | 35 | the only missing callees are private members of the same behaviour class |
| `blocked` | 109 | calls a subsystem that is not there |
| absent in 2013 | 11 | the function does not exist in retail 2013 and must not be ported at all |

So the honest ceiling is **81**, and of those 34 are in this package's own classes — which is exactly what it ported. This package ported 34 and states
where it stopped rather than padding the number. The blockers, by how many natives each gates:

| blocker | natives | what it is |
|---|---|---|
| `FDisAISubState*_Param` constructors (17 types) | ~35 | the *request* half of a sub-state change. A behaviour callback whose job is "enter the TakePosition sub-state" builds one of these. Each is a small native struct with an `OnPending` that writes the target sub-state's own members through raw offsets — and **the sub-state classes themselves (`UDisAISubStateStand`, `TakePosition`, `Investigate`, `MeleeChase`, `FirePistol`, …, 17 of them) have no bodies**. Porting the params without the sub-states would make a callback request a state that does nothing, which is the trade agent AU's rule forbids |
| `IDisDesiresInterface` (`ClearFaceToDesire`, `ClearLookAtDesire`, `SetBodyIntentionDesire`, `SetLookAtProxyDesire`, …) | ~23 | how a behaviour expresses "look there", "face that", "hold this stance". Needs `FArkComponentLookat` and `FArkComponentFaceTo`, neither ported |
| `UDisAIBrainProcessAttention` (84 functions) | ~12 plus everything about seeing | the attention meter. Every `FDisAttentionProxy` query funnels into it, so a brain currently answers `DAL_Unaware` about everything |
| `FDisAIMonitorPawnReachability`, `UDisAISubProcessManageAttacks`, `ADisAlarmBell`, `UDisGlobalMusicManager` | ~20 | one small subsystem each |

**The next package is the 17 sub-state classes and their parameter structs.** That is one coherent piece of work, it is
where 35 of the remaining natives live, and with the sub-state machine and the callback delegates already in place it is
the last thing between a behaviour and a visible action.

## What moving still needs — named precisely, as the brief asked

The census reports `moved 0.0 uu, 0 moving` and that is correct rather than broken: **nothing in the tree can move an
NPC yet**, and it is two things, not one.

**1. `FArkComponentLocomotion` (117 functions, ~40 KB).** This is the pawn's move executor and it is the AI root this
package does not own. `ADishonoredNPCPawn::Spawned` (2013 rva 0x752e10) adds it to the pawn's component container and
gives it the tweaks' `UArkComponentLocomotionConfig`; `ADishonoredNPCController::GetMoveTargetLocation` reads its active
request. The expensive parts are `FullMovePawn` **0x53fb00** (4,707 b), `ComputeDynamic` **0x54bec0** (3,026 b),
`ComputeTargetMoveYaw` **0x547950** (1,790 b), `ComputePriorityOfNPCsFromSharedProps` **0x5491b0** (1,737 b),
`ComputeTargetMoveSpeed` **0x548050** (957 b), `HandleArrival` **0x5487b0** (977 b), `ApplySteering` **0x54a630**
(1,044 b) and `ClampMove` **0x544490** (752 b). The cheap public surface a behaviour actually calls is small:
`GetLocomotionComponent` **0x543110**, `Enable` / `Disable` **0x53e630** / **0x546520**, `AddSteeringForce`
**0x53dfe0**, `GetRequestsCount` **0x541b60**, `GetActiveRequestTargetedLocation` **0x5464d0**,
`ForceActiveRequestRepath` **0x53e620**, `HandleTeleport` **0x546550**.

**2. What the AI needs from the navigation-mesh runtime** — which agent AD deliberately left as the reference design in
wave 4, and which is *not* in this package. It is four things and no more:

* **A path between two points on the mesh.** `FArkComponentLocomotion::DefaultPathBuilder` **0x545910** is the AI's only
  entry into it; everything else in locomotion consumes the result as a polyline. Retail's path finder is
  `UNavigationHandle`'s poly-based A\* over `UNavMesh`, driven by the goal evaluators and constraints below.
* **The goal evaluators and constraints the AI supplies.** `UDishonoredAIBehavior::GetDefaultPathGoals` **0x6eabc0** /
  `GetDefaultPathConstraints` **0x6eaca0** and the per-behaviour `CallGetPathGoals` **0x6eac30** /
  `CallGetPathConstraints` **0x6ead10** fill `TArray<UNavMeshPathGoalEvaluator*>` and
  `TArray<UNavMeshPathConstraint*>`; `UDisAISubState::GetPathGoals` / `GetPathConstraints` let a sub-state add its own.
  `ADishonoredNPCPawn::SetupPathfindingParams` **0x74b8b0** fills the `FNavMeshPathParams`. All four of those are
  declared in this package and answer the base default, so the *shape* is in place — the runtime that consumes them is
  not.
* **"Where on the mesh is this point, and can I stand there?"**
  `FArkComponentLocomotion::FindNearestLocationOnNavMesh` **0x545660** and `CanNPCPathfindFromLocation` **0x546770**.
  `ADishonoredNPCPawn::MoveAtSafeLocation` (0x75b0f0) and the spawner's "is there room" test both need it.
* **Reachability as a query, not a path.** `FDisAIMonitorPawnReachability::IsPawnReachableBy` and
  `GetKnownHideoutThePlayerIsIn` gate 6 of the remaining natives and are what makes a guard give up rather than walk
  into a wall.

Nothing else in the AI touches the nav mesh. In particular the brain, the behaviour stack, the sub-state machine, the
stim system and the attention proxy are all complete without it — which is why this package could be measured at all.

## What is deliberately left as a documented gap

Each of these is one `DISHONORED(bringup)` line at the site naming the retail function and its rva, never a silent
no-op:

* **`TickBrain_Senses`** (2013 rva 0x717880) — needs `FDisComponentVisionNPC` and `UDisAIBrainProcessAttention`, so **an
  NPC cannot see**. Everything downstream of sight (suspicion escalation, the attention meter, combat engagement) is
  therefore never raised by vision; stims from Kismet, noise, damage and the brain-init path still flow, which is what
  makes the behaviour stack observable.
* **`TickBrain_Steering`** / `RefreshBrainThoughts_Steering` — the `UDisSteeringInfluence` classes are generated with
  their members but have no bodies, and there is no locomotion to hand a force to. `InitBrain_Steering` does wire
  `m_pSteeringInfluence_Combat` / `_EnemyPush` / `_Danger`, their owner and their fade speed, so the pass becomes a
  transcription the moment the influences get bodies.
* **`TickBrain_Stealth`** — would build the player's stealth indicator from an attention level that is always Unaware.
  Writing it would put a wrong value in front of the HUD rather than none.
* **The three `FArkComponentBase` components `InitBrain` adds to the brain's container** (`FDisAIMonitorReaction`,
  `FDisAIKnowledgeComponent` and, when the attention tweaks ask, `FDisMonitorNPCAttention`): all three unported, so the
  container stays empty and every `GetFirstComponent<T>` in the AI answers NULL — which is a path retail itself has, so
  the brain runs.
* **`FDisLookAtRequest::Initialize`** (0x74f180) — `FArkComponentLookat` is not ported, so the head-look request is never
  issued.
* **`UArkComponentContainer::Owner`** is `protected` in the reference engine with only `AActor` as a friend, and
  `EngineClasses.h` is not this package's file, so the brain's container is not given its controller as owner. Nothing
  reads it yet (the three components above are what would).
* **The suspecting-NPC melee draw** in `InitNPC` — needs `UDishonoredInventory::FindEquippableItem` (0x805cb0) and
  `ADishonoredPawn::EquipItemByType` (0x753260), the equip half agent AJ left for `UDisItemContext`.
* **`UDisActorFactoryNPCPawn::IsEnoughRoomToSpawnDisPawn`** (0x758db0) — its two trace-flag constants (8326 / 8351) are
  two of the `FDisPrimTraceMask` combinations agent AU's follow-up 2 says `Engine/Inc/UnLevel.h` still has no names for,
  so a spawner whose spawn point is blocked spawns anyway where retail would refuse.
* **The squad limits** (`UDishonoredMapInfo::FindSquadInfo`, 0x6f5760) — a squad never refuses a spawn, so a map that
  relied on the limit gets more NPCs than retail. The census reports the count, so the difference is visible.

## One generator hook (shared-tree plumbing, reproduced by regeneration)

`resources/tools/symbols/gen_classes_header.py` gained a **third** cpptext hook beside the two agent AJ added in wave 4:
a generated **struct** body now includes `Inc/CppText/<Struct>.h` when that file exists, the same rule classes and
interfaces already had. It is needed because `FDisAttentionProxy` is a reflected struct with eighteen methods of its own
and roughly thirty of the AI natives call them; without the hook they have nowhere to be declared. Applied idempotently
by `python build/agentCG_work/patch_gen.py`, tagged `DISHONORED(written)`, and reproduced by the coordinator's
regeneration at merge.

## Runs

Build: `cmd /c build\agentCG_release.cmd` (Release, snapshot worktree `build/agentCG_wt` → `build/agentCG`), **0 errors,
0 unresolved symbols** (`build/agentCG_build24.log`), plus `CoreSmoke` and `LayoutProbe` so the whole harness runs.

| log | what | result |
|---|---|---|
| `agentCG.log` | `L_Tower_P` 180 s d3d9, `-inputtest -disai -distouch` | the census line quoted above, **0 `Critical`**, 0 unported natives |
| `agentCG_final_tower.log` | `L_Tower_P` 150 s null RHI, `-inputtest -disai` | 26/26/26 brains, 684 sub-state enters, 0 `Critical` |
| `agentCG_final_pub.log` | `L_Pub_Day_P` 150 s null RHI, `-inputtest -disai` | 0 NPC pawns, **0 spawners** (the hub is not spawner-driven), 0 `Critical` |
| `build/agentCG/regression/summary.txt` | `run_regression.py --build-dir build/agentCG --no-build`, on the snapshot rebased onto HEAD `844afbe` | **31 ok, 0 failed, 0 skipped**, 443 s; `unported_natives` 0, `probe_natives` 7 (the bound is 12) |
| the same, on `2cdd7b3` | before the rebase | 31 ok, 0 failed, 422 s; `probe_natives` 6 |
| `agentCG_base.log` | the same command on the package's own baseline build of HEAD `2cdd7b3` | 0 NPC pawns, 41 spawners — the "before" half of the census |

## Hand-overs

1. **The 17 AI sub-state classes and their `_Param` structs — the next package, and the largest single block of
   remaining natives (~35).** The sub-state machine, the six callback delegates and `RegisterCallbacks` are all in
   place; what is missing is `UDisAISubStateStand`, `TakePosition`, `TakeActorPosition`, `GenericAction`, `Investigate`,
   `TrackTarget`, `Cower`, `Flee`, `Follow`, `MaintainDistance`, `Menace`, `MeleeChase`, `MeleeEngage`, `FirePistol`,
   `FindShootingPosition`, `LieInWait`, `StareAtUnreachable`, and one `FDis…_Param` per class whose `OnPending` writes
   that sub-state's members. Do them together: a param without its sub-state is the trap.
2. **`UDisAIBrainProcessAttention` (84 functions) is the single highest-leverage AI piece left.** Every
   `FDisAttentionProxy` query already routes to it through `UDishonoredAIBrain::GetAttentionLevel` /
   `GetAttentionProxyInfo`, which this package ported as the "no process" path. Landing it turns 26 unaware NPCs into 26
   NPCs that notice things, and unblocks `TickBrain_Senses` and `TickBrain_Stealth`.
3. **`FArkComponentLocomotion` + the four nav-mesh runtime queries** above, for movement. In that order: the nav queries
   first, because locomotion's path builder is the only consumer.
4. **`FArkGameEventDispatcher::CreateInstance` is never called in the tree.** The dispatcher is ported but no instance is
   created, so every `RegisterToEvent` this package would make is left out with a note (the brain's difficulty-change and
   push-by-avoidable subscriptions, the behaviour's and sub-process's actor-terminated subscriptions, the spawner's
   pawn-destroyed subscription). The retail call site is in the engine start-up path and belongs to whoever owns
   `Engine/Src/UnGame.cpp` — one line, and it turns all of those on at once.
5. **`m_ActorTypeFlags` is still never written** (agent AS follow-up 3, agent AU follow-up 6). Unchanged by this package,
   but the AI reads it in several unported bodies, so it will come up again.
6. **Coordinator, at merge:** the module must be regenerated for DishonoredGame (`gen_classes_header.py --sdk`) —
   22 new `Inc/CppText/` files, the new units, the struct-cpptext hook and
   `DishonoredGameNativeStubs.ported.agentCG.txt` all need it. `build/agentCG_work/sync_build.py` is exactly that with
   the output pointed at a worktree. Four of the units this package extends are already on the shared compile list
   (`dishonorednpccontroller.cpp`, `dishonorednpcpawn.cpp`, `dishonoredgameinfo.cpp`,
   `dishonoredutilities_accessors.cpp`) and 14 more come off the exclude list.

## Files

**Mine** (73, listed in `build/agentCG_work/files.txt`). Generated files were **not** touched in the shared tree; the
module is regenerated in the snapshot only.

* Engine: `Inc/arkcomponentbase.h`, `Inc/arkgameeventdispatcher.h`, `Inc/EngineArkaneClasses.h` (the
  `UArkComponentContainer` block only), `Src/arkcomponentcontainer.cpp`, `Src/arkgameeventdispatcher.cpp`
* DishonoredGame headers: `Inc/aistimstruct.h`, `Inc/disdelegate.h`, `Inc/disaisubstate.h`, `Inc/disaicensus.h`,
  `Inc/dishonoredutilities_ai.h`, `Inc/DishonoredGameNative.h` (the AI include block)
* New `Inc/CppText/`: `UDishonoredAIBrain.h`, `UDishonoredAIBehavior.h`, `UDisAISubProcess.h`, `UDisAISubState.h`,
  `UDisAISubStateMachine.h`, `UDisAIBrainProcess.h`, `UDisStimManager.h`, `FDisAttentionProxy.h`,
  `IDisAttentionTargetInterface.h`, `ADishonoredSpawner.h`, `UDisActorFactoryNPCPawn.h`, `UDisBehaviorIdle.h`,
  and nine `UDisBehavior*.h`; extended `ADishonoredNPCController.h`, `ADishonoredNPCPawn.h`, `ADishonoredGameInfo.h`,
  `UDishonoredGlobalAIManager.h`
* `Src/`: `dishonoredaibrain.cpp`, `dishonoredaibrain_senses.cpp`, `_stealth.cpp`, `_steering.cpp`,
  `dishonoredaibehavior.cpp`, `disaibrainprocess.cpp`, `disaisubprocess.cpp`, `disaisubstate.cpp`,
  `disaisubstatemachine.cpp`, `aistimstruct.cpp`, `disstimmanager.cpp`, `disattentionproxy.cpp`,
  `disattentiontargetinterface.cpp`, `dishonoredspawner.cpp`, `disactorfactorynpcpawn.cpp`,
  `dishonorednpccontroller.cpp`, `dishonorednpcpawn.cpp`, `dishonoredglobalaimanager.cpp`,
  `dishonoredutilities_accessors.cpp`, `dishonoredgameinfo.cpp`, `disbehavioridle.cpp`, nine `disbehavior*.cpp`,
  `disaicensus.cpp` (the census, not a retail unit), and the four-line census hook in `dishonoredplayercontroller.cpp`
* `DishonoredGameNativeStubs.ported.agentCG.txt` (**34 natives**), this report and `agentCG_status.csv` (260 rows)
* Shared-tree plumbing, reproduced by regeneration: `resources/tools/symbols/gen_classes_header.py` (the struct cpptext
  hook)

**Scratch** (not repo tools): `build/agentCG_work/` — `classify3.py` (the measured portability classification and the
blocker table), `cls.py` / `sym.py` / `off.py` / `off12.py` / `gap.py` / `cost.py` (analysis helpers; `off12.py` exists
because `retail_sdk_layout.json` has **no `ADishonoredNPCPawn`** and the 2012 PDB is the only layout for it),
`mklist.py` / `mk_unblock.py` / `mk_full.py` (decompile lists), `sync_build.py`, `crlf.py`, `mkstatus.py`, and
`fix1..fix12.py` / `write_*.py` which re-apply every edit idempotently. Decompiles in `build/agentCG_decomp/{r13,s12}`
(1,157 functions). Snapshot worktree `build/agentCG_wt`, build dir `build/agentCG`, build logs
`build/agentCG_build{0..25}.log`. IDA copies `resources/docs/idb/retail2013_agentCG.i64` and
`shipping2012_agentCG.i64`.

## Commands

```
python resources/tools/make_snapshot.py CG --list build/agentCG_work/files.txt   # refresh the snapshot overlay
python build/agentCG_work/sync_build.py                     # sync + regenerate the module in the snapshot only
cmd /c build\agentCG_release.cmd                            # Release build of the snapshot into build/agentCG
cmd /c build\agentCG_release.cmd CoreSmoke                  # and LayoutProbe, so all 31 harness checks run
python resources/tools/run_regression.py --build-dir build/agentCG --no-build    # 31 ok, 0 failed
python build/agentCG_work/patch_gen.py                      # the struct cpptext generator hook (idempotent)
python build/agentCG_work/classify3.py                      # the portability classification and the blocker table
python build/agentCG_work/mkstatus.py                       # regenerate agentCG_status.csv from the tagged edits
python resources/tools/ida/run.py resources/tools/ida/decompile_funcs.py resources/docs/idb/retail2013_agentCG.i64 build/agentCG_decomp/r13 rva:0x7252a0
```

The census run, in full:

```
python resources\tools\build_and_smoke.py --build-dir build/agentCG --no-build --exe-name DishonoredGame_CG.exe ^
  --log-name agentCG.log --ini-dir build/agentCG/config --rhi d3d9 --timeout 180 ^
  --skip-native OnlineSubsystemPC --milestone "Initializing Engine..." --expect "Initial startup" --forbid "Critical" ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -disai -distouch -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

`resources/play.cmd` and `resources/build-release.cmd` were not touched. No commits, no `git add`.
