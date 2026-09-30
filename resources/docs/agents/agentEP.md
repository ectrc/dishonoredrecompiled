# Agent EP (PHASE14 EP) — eight guards in the Tower were already being told to patrol, and nothing was listening

Worktree `build/agentEP_wt`, detached at `2d08c15`. Own build dirs `build/agentEP_rel` (every AI measurement),
`build/agentEP_clean` (the acceptance build, deleted first) and `build/agentEP_regwt` (the regression harness). Own IDA
copy `build/agentEP_ida/retail2013_agentEP.i64`, headless `idalib` only — **no IDA MCP tool and no FModel tool was
used**. Nothing committed, nothing staged, nothing written into the main checkout or another agent's worktree.

Thirty files, **every one inside `DishonoredGame`**, plus these two documents.

## The answer in ten lines

* **The brief's premise about `TickBrain_Senses` is wrong, and it is wrong in this tree's own comments.** The comment
  block in `dishonoredaibrain_senses.cpp` says retail's `TickBrain_Senses` "asks the controller's
  `FDisComponentVisionNPC` what it can see". It does not. Retail 2013's body (rva `0x717880`, 118 bytes, and I proved it
  is that function by decompiling its *caller*) does four things and none of them is vision: it rebuilds two inhibitor
  masks, checks Kismet events, runs a player-proximity attention pass and ticks one look-at request. **Vision is not
  ticked from the brain at all** — it is an Ark component on the time-sliced policy of `FArkComponentManager`. Section 2.
* **Nothing in this tree filled `ADishonoredNPCPawn::m_SpawnerInfo`, and that single gap was blocking the patrol, the
  guard post and the squad system at once.** `FSpawnNPCPawn_TweakObj::DoInit` (2013 `0x8856a0`, 702 bytes) copies the
  spawner into it; it was a one-line stub whose comment said the member "is not in the retail SDK dump",
  which is false — it is at offset 3204 and `DishonoredGameLayouts.h` has been asserting it all along. Before this
  package every NPC's squad was `None`, every guard's home post was the origin, and every
  `m_bPatrolUponStartup` was FALSE. Section 4.
* **Eight of L_Tower_P's spawners were already raising the patrol stim at HEAD and nothing was listening.** Measured with
  a stim histogram added for the purpose: `EAIStimID_PatrolRequest(71)=8` on the untouched HEAD executable.
  `UDishonoredAIBrain::ProcessOneStim` drops a stim no behaviour's evaluate mask covers without a log line, so "the
  patrol behaviour is unported" and "nothing asks for a patrol" looked identical from outside. Section 3.
* **Four guards now adopt a route and walk to it; one of them walked 265 uu and arrived.** After: `disai slot0:
  DisBehaviorIdle=26 DisBehaviorPatrol=8`, `DisAISubStateTakeActorPosition=4`, `DishonoredNPCPawn_15=265` uu to
  `DishonoredRoute_4`'s point 0 and `arrived` 26 uu short of it, and `DishonoredNPCPawn_12` holding a **7-point path at
  158/158 uu/s** along `DishonoredRoute_3`. Before: 26 brains, all `DisBehaviorIdle`/`DisAISubStateStand`, 0.0 uu moved,
  route list empty. Section 5.
* **The four that do not adopt a route are refusing it correctly**, which is the measurement that says the port is
  faithful rather than lucky: `DishonoredNPCPawn_13`/`_14` are squad `GuardsB` and the only route inside their 500 uu
  adoption range belongs to squad `Servant`; `_20` is second to `_19` on a route whose capacity is 1; `_25` is 1855 uu
  from the nearest route. Section 5.2.
* **Two latent defects in other agents' work, both found by making the data real rather than by reading.**
  `UDisActorFactoryNPCPawn::CreateActor` and `ADishonoredSpawner::OnSpawned` both gated possession on
  `m_bStraightToRagdoll` where retail tests **`m_bTreatAsKnockedOut`** — neighbouring bits of one bitfield word. It was
  invisible while `m_SpawnerInfo` was all zeroes; the first run with it filled reported "26 NPC pawns, **0 controllers**".
  And `UDisBehaviorPatrolSearch` inherits `UDisBehaviorPatrol`, so the moment the base answered stim 71 the subclass
  stole it from it — `disai slot0: DisBehaviorPatrolSearch=8` — until its three retail overrides (stim **72**) were
  ported. Sections 4.1 and 3.2.
* **Four mislabelled retail addresses in this tree, all of which `rva_sweep.py` passes as `ok-2013-mid`.**
  `ADishonoredNPCController::InitNPC` cited as `0x7549d0` (really `0x7632e0`), `FArkGameEventDispatcher::GetInstance` as
  `0x557160` (`0x54df00`), `FDisLookAtRequest::Initialize` as `0x74f180` (`0x8b2400`),
  `UDisAINoiseManager::RegisterListener` as `0x7431e0` (`0x851570`). All four are in files this package already owns and
  all four are corrected. `build/agentEP/addr_audit.py` is the check that found them. Section 8.
* **Perception and the behavioural reaction are handed over, not delivered, and they are handed over measured.** Both
  are one chain and it is far larger than the patrol: `FArkComponentManager` + six policies, `FDisComponentObservable`,
  `FDisComponentVision`, `FDisComponentVisionNPC`, the cone maths, and then
  `UDisAIBrainProcessAttention` — the *only* consumer of the `TargetSighted` stim, 61 named functions and 5,507 bytes.
  Sections 6 and 7 carry the complete member layout of all three component classes, both creation sites, the tick
  phases, and the cone algebra, in `build/agentEP/spec/vision_spec.md` and `build/agentEP/spec2/creation_spec.md`.
* **Two defects of my own, both in things I had inferred rather than resolved, and both caught by a check I wrote for
  the purpose.** `FindNextPoint`'s indirect call through the behaviour's vtable at offset 440 I first wrote as
  `OnBehaviorStart()`, which would have dropped the route `ChooseNewRoute` had just adopted; it is
  `UDisBehaviorPatrol::OnNewRouteChosen` (2013 `0x6ec9e0`), pinned twice. And I renamed `0x6fd350` to
  `OnPatrolPointReached` while rewriting its skeleton banner, where retail's own PDB calls it
  `UDisBehaviorPatrol::OnReachedDestination`. Both corrected. Section 8, items 9 and 10 — which also carry a **second
  truth-source correction**: `match_2012_2013.csv` names `0x6ec9e0` as `ScheduleRouteSwitchBark`, a 59-byte function,
  where `0x6ec9e0` is 38 bytes and tail-jumps to it.
* **Regression 37 ok, 0 failed, 0 skipped**, built inside the harness from the worktree's own copy; a clean full Release
  build with the directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**; `rva_sweep`
  over the 24 files this package owns: **332 citations, one flagged and that one deliberately**, plus a by-hand
  `ida_funcs.get_func` resolution of all **145** distinct 2013 addresses they cite. Section 9.

## 1. What the package is, in one table

| retail 2013 | what | where |
|---|---|---|
| `0x8856a0` | `FSpawnNPCPawn_TweakObj::DoInit` — all fifteen `FDisSpawnerInfo` members from the spawner | `Src/distweaksbase.cpp` |
| `0x771bc0` | `UDisTweaks_NPCPawn::SpawnActor_WithSpawner` — the spawn that carries the spawner | `Src/distweaks_npcpawn.cpp` |
| `0x64bb50` and 14 more | `ADishonoredRoute` — registration, adoption, the neglect clock, the route geometry | `Src/dishonoredroute.cpp` |
| `0x841590` and 3 more | `UDisPatrolManager` — the route list and which route an NPC takes | `Src/dispatrolmanager.cpp` |
| `0x64bab0` | `ADishonoredNavPoint::IsGuardPoint` | `Src/dishonorednavpoint.cpp` |
| `0x5fe910` | `IDisSquadInterface::IsSquadSupported` | `Src/dissquadinterface.cpp` |
| `0x840790` | `FDisNonRepeatINTRandomHelper::GetNonRepeatingRandomValue` | `Src/disglobalenums_utilities.cpp` |
| `0x6fa6e0` and 22 more | `UDisBehaviorPatrol` — the whole behaviour | `Src/disbehaviorpatrol.cpp` |
| `0x6ef220` and 3 more | `UDisBehaviorPatrolSearch` — the three overrides that keep it off its base's stim | `Src/disbehaviorpatrolsearch.cpp` |
| `0x72b860`, `0x7289b0`, `0x728980` | the three `UDisAISubProcessWatchPoints` accessors the patrol reads at a post | `Src/disaisubprocesswatchpoints.cpp` |

`resources/docs/agents/agentEP_status.csv` carries the same list with every note, every "not ported" with its reason,
and the hand-over set: **88 rows — 54 ported, 7 corrected, 8 deliberately not ported, 1 measured finding and 18 handed
over** — and the per-file counts are 23 ported bodies in `disbehaviorpatrol.cpp`, 15 in `dishonoredroute.cpp`, 4 in
`dispatrolmanager.cpp`, 4 in `disbehaviorpatrolsearch.cpp`, 3 in `disaisubprocesswatchpoints.cpp` and one each in
`dishonorednavpoint.cpp`, `dissquadinterface.cpp`, `disglobalenums_utilities.cpp`, `distweaks_npcpawn.cpp` and
`distweaksbase.cpp`.

## 2. The first correction: retail's senses pass has nothing to do with vision

`dishonoredaibrain_senses.cpp` in this tree says:

> Retail asks the controller's `FDisComponentVisionNPC` what it can see, hands each sighting to the attention process
> (`UDisAIBrainProcessAttention`) and raises the `TargetSighted` / `TargetUnsighted` stims from the difference.

That is an invention, and it is load-bearing: it is why the file is a documented no-op and why the brief's area reads as
"the vision component is the thing `TickBrain_Senses` needs".

**`TickBrain_Senses` is not named in retail 2013**, so I identified it from the other side: `UDishonoredAIBrain::TickBrain`
(2013 `0x724a10`, named) calls, in order, `sub_B24340` (= `0x724340`, the address this tree already cites for
`TickBrain_Stealth`), `TickBrain_Stims` (named), `TickBrain_Processes` (named), **`sub_B17880`**, `sub_B17100`
(= `ProcessAllStims`), then the behaviour tick and `TickBrain_Steering` (named). `sub_B17880` is rva `0x717880`, 118
bytes, which is the slot `TickBrain_Senses` occupies and the size of 2012's `TickBrain_Senses` (`0x7562d0`, also 118).
So the address this tree cites is right and its description is wrong. Its whole body:

```
sub_B00F70( this )                              ; rebuild brain+132 and brain+136 (the two final sense masks) by OR-ing
                                                ;   the five mask words at +112..+128; force bits 0xE when the pawn is
                                                ;   frozen by Bend Time; force +136 to 1 when flag bit 3 of +60 is set
if( !DisIsActorFrozenByBendTimePower( pawn ) ) {
    UDishonoredAIBrain::CheckForImportantKismetEvents()
    sub_B174E0( this )                          ; 923 bytes: the player-proximity attention pass - reads
                                                ;   ADishonoredPlayerPawn::s_pInstance, compares the squared distance
                                                ;   against UDisConvGlobalMan's radius, and clears or sets the pawn's
                                                ;   minimum-attention limits 2..9 through pawn+1228's vtable slot 8
    if( brain+176 ) { brain+184 -= dt; if( brain+184 < 0 ) brain+176 = brain+180 = 0; }
    FDisLookAtRequest::TickRequest( brain+388, dt )
}
```

**Consequence for whoever ports perception**: do not go looking for a vision call site in the brain. Vision is
`FDisComponentVision::TimeSlicedTick`, reached from `FArkComponentTimeSlicedTickPolicy::Tick` (2013 `0x536e70`), reached
from `FArkComponentManager::Tick` phase 1, reached from `UWorld::TickAW`. The brain only ever *receives* the result,
through `ADishonoredNPCController::StartSeeingVisibleThing` → `UDishonoredAIBrain::HandleTargetSighted`.

## 3. What HEAD actually does in the Tower, and the counter that says why

Every number in this section is `L_Tower_P?Name=Corvo?Team=255 -nullrhi -disai`, the untouched HEAD build
(`build/agentEP_rel` before any edit, staged as `DishonoredGame_EPH.exe`), `build/agentEP/base_nullrhi_log.txt`:

```
disai census: 26 NPC pawns, 26 controllers (26 with a brain, 26 initialized), 41 spawners; moved 0.0 uu this second
              (0 moving), turned 0; brain ticks 310778, stims 72/72, behaviors 684 init 26 activations,
              substates 710 enters 310726 ticks, subprocesses 104, callbacks 0, move requests 0
disai substates: 26 transitions; entered DisAISubStateInit=684 DisAISubStateStand=26
disai slot0: DisBehaviorIdle=26
disai loco: 26 components; requests 26 start 0 update 0 stop; paths 12 built 2 failed, 24 path points;
            12 arrivals; 0.0 uu moved; 12 teleports onto the mesh; last path error GoalPolyNotFound
```

The brain layer is complete and idle. The three things this package added to the census are what made it diagnosable:

```
disai stims: 72 offered; EAIStimID_BrainInit(12)=26 EAIStimID_DestinationReached(24)=12
             EAIStimID_PathingFail(69)=14 EAIStimID_PathingSuccess(70)=12 EAIStimID_PatrolRequest(71)=8
disai patrol: map info yes, patrol manager yes (route list empty); 6 routes (6 active, 19 points),
              31 nav points (4 guard); 41 spawners, 8 ask for a patrol
disai routes: [DishonoredRoute_2 type 1 2 points active 1 cap 1 range 500 squads 1] ... x6
```

Three facts fell out of that one line and each of them changed what the package had to be:

1. **`EAIStimID_PatrolRequest(71)=8`.** `ADishonoredSpawner::OnSpawned` already raises the patrol stim — agent CG ported
   that — for every spawner whose `m_bPatrolUponStartup` is set. Eight of L_Tower_P's 41 spawners set it. The stim
   reaches the brain, and `ProcessOneStim` drops it because no behaviour's evaluate mask covers id 71. So no Kismet, no
   cheat and no forcing is needed to make a guard patrol: the level asks for it eight times at load.
2. **the patrol manager exists and its route list is empty.** `UDishonoredMapInfo::m_pPatrolManager` is a default
   sub-object of the map info, so it is created by the cooked defaults with no code at all; what was missing was
   `ADishonoredRoute::PostBeginPlay`, the four lines that thread a route onto it.
3. **the level has real patrol data**: six `ADishonoredRoute` actors, all active, 19 route points between them, four
   guard posts, and each route has a squad filter and a capacity of 1.

### 3.2 The defect this package caused, measured, and fixed

With `UDisBehaviorPatrol`'s mask in and nothing else changed, the census read

```
disai slot0: DisBehaviorIdle=26 DisBehaviorPatrolSearch=8
```

`UDisBehaviorPatrolSearch` **derives from** `UDisBehaviorPatrol`, so it inherited the evaluate mask and the evaluate
delegate for stim 71, and because it comes earlier in the brain's `m_BehaviorArray` it took the slot for all eight NPCs —
where it then did nothing, because `RequestSubStateChange<UDisTweaks_AIBehavior_Patrol, …>` cannot find
`UDisTweaks_AIBehavior_Patrol` on an object whose tweaks are `UDisTweaks_AIBehavior_PatrolSearch`.

Retail's three overrides (2013 `0x6ef220`, `0x6fd5e0`, `0x6e9ae0`) are each a single `if( id == 72 )` —
`EAIStimID_PatrolSearchRequest` — and their three bodies are ICF-folded onto `UDisBehaviorPatrol`'s, which is why
`0x6ebbf0`, the folded evaluate thunk, resolves to `UDisBehaviorPatrolSearch::EvaluatePatrolSearchRequest` when you
disassemble it. With them ported the slot goes to `DisBehaviorPatrol`.

**The general lesson is not about patrol**: any behaviour whose subclass is also in the brain's list will be shadowed by
it the moment the base answers a stim, and the shadow is silent — no warning, no log line, and the shadowing behaviour
holds the slot and does nothing. I counted how often that can happen rather than guessing: of the **25 distinct
behaviour classes** `DisAIBrain` constructs, there is **exactly one** base/subclass pair, and it is this one
(`build/agentEP/behs.txt` is the list the census printed, and the parent of every class comes from the generated
headers). So the hazard is real but it is not widespread, and the one place it exists is now closed.

## 4. The spawner info, which is what made any of it reachable

With the mask and the manager in, `ADishonoredRoute::CanAdopt` still refused every route. The probe says why in one line:

```
{nearest DishonoredRoute_2 active 1 points 2 null 0 squad 'GuardsB' pawnsquad 'None' squadok 0 cap 0/1 range 143/500}
```

A route 143 uu away, active, with two good points, adoption range 500, capacity free — refused because the route's squad
filter says `GuardsB` and the pawn's squad is `None`. `CanAdopt` reads `pawn->m_SpawnerInfo.m_Squad` (retail reads
`pawn+3220`, which is `m_SpawnerInfo` at 3204 plus `m_Squad` at 16), and **nothing in this tree ever wrote any field of
`m_SpawnerInfo`** — I grepped every write before believing it.

Retail fills it in `FSpawnNPCPawn_TweakObj::DoInit` (2013 `0x8856a0`, 702 bytes), the spawn-time init functor
`UWorld::SpawnActor` calls between the actor's transform and `PostBeginPlay`. It writes **every one of
`FDisSpawnerInfo`'s fifteen members**, each from a named spawner member. The bitfield is the part worth writing down, because the whole of section 4.1 comes out of it —
`ADishonoredSpawner`'s eleven booleans share the word at 1116, in declaration order:

| bit | spawner member | goes to |
|---:|---|---|
| 5 | `m_bSpawnDead` | `m_SpawnerInfo.m_bSpawnDead` (bit 0 of +76), **only when bit 7 is clear** |
| 7 | `m_bTreatAsKnockedOut` | `m_SpawnerInfo.m_bTreatAsKnockedOut` (+92) |
| 8 | `m_bPatrolUponStartup` | `m_SpawnerInfo.m_bPatrolUponStartup` (+40) |
| 10 | `m_bCapableOfFleeing` | `m_SpawnerInfo.m_bCapableOfFleeing` (+24) |

and `m_bStraightToRagdoll` (bit 1 of +76) is not copied from anything: it is computed, `!m_pDeadPoseAnimSet ||
m_DeadPoseAnimName == NAME_None` — "spawned dead with no pose animation, so there is nothing to pose it with".
`m_Position` is the **spawner's** transform, not the pawn's, which is what makes `UDishonoredAIBrain::m_Home` — the post
`UDisBehaviorGuard` walks back to — a real place instead of the origin.

Reaching that functor needed one more retail body: `UDisTweaks_NPCPawn::SpawnActor_WithSpawner` (`0x771bc0`), which is
`UDisTweaksBase::SpawnActor_Derived` with `FSpawnNPCPawn_TweakObj` instead of `FSpawnActor_TweakObj` and two extra
preconditions of its own (`m_pSkeletalMesh` and `m_pAnimTreeTemplate` must both be set). `CreateNPCPawn` now calls it, as
retail's `0x74e4b0` does, and passes `bNoCollisionFail = spawner->m_bSpawnDead` where this tree passed a constant TRUE.

### 4.1 The latent defect it exposed: two neighbouring bits

The first run with `m_SpawnerInfo` filled read **"26 NPC pawns, 0 controllers (0 with a brain)"** — every NPC lost its
mind. Both places that decide whether a spawned pawn gets a controller tested
`m_SpawnerInfo.m_bSpawnDead || m_SpawnerInfo.m_bStraightToRagdoll`, and `m_bStraightToRagdoll` is now TRUE for every NPC
in the level (none of them has a dead-pose animation). Retail tests a different pair, and it is unambiguous in both
places:

```
0x75dbb0  UDisActorFactoryNPCPawn::CreateActor:   if( (pawn+3280 & 1) == 0 && (pawn+3296 & 1) == 0 )
0x6590e0  ADishonoredSpawner::OnSpawned:          if( (pawn+3280 & 1) != 0 || (pawn+3296 & 1) != 0 ) return
```

3280 bit 0 is `m_bSpawnDead`; **3296 is `m_bTreatAsKnockedOut`**, not 3280 bit 1. Both are corrected. This is a defect
that could not be seen by reading either file: it needed the data to be real first, and it would have fired on the first
level with a spawner marked "spawn dead".

## 5. Four guards on patrol, measured

`build/agentEP/patrol13_log.txt` — the tree exactly as handed over, the same command line as section 3, a 266-second run
on `build/agentEP_rel`:

| | HEAD `2d08c15` | agent EP |
|---|---:|---:|
| NPC pawns / controllers / brains / initialized | 26 / 26 / 26 / 26 | **26 / 26 / 26 / 26** |
| behaviour activations | 26 | **34** |
| slot-0 behaviours | `DisBehaviorIdle=26` | **`DisBehaviorIdle=26 DisBehaviorPatrol=8`** |
| sub-states entered | `Init=684 Stand=26` | **`Init=684 Stand=22 TakeActorPosition=4`** |
| patrol manager's route list | empty | **`DishonoredRoute_6`** (all six registered) |
| NPC squad | `None` on all 26 | **`GuardsB` / `Servant` / `WorkersBoat`** from the spawner |
| routes adopted | 0 | **4** |
| uu moved (total) | 0.0 | **315.4** |
| navigation-mesh paths built | 12 | **13** |
| the farthest walker | — | **`DishonoredNPCPawn_15` = 265 uu, `arrived`, 26 uu short of its route point** |
| a live patrol walk | — | **`DishonoredNPCPawn_12`: 7-point path, 158/158 uu/s, 1625 uu to go** |
| stims offered | 72 | 71 |

The one number that goes *down* is the stim total, and it is the expected direction: `EAIStimID_DestinationReached` falls
from 12 to 11 because one of the NPCs that was standing in `DisAISubStateStand` at HEAD — and therefore arriving at its
own position — is now walking a route instead. Every other stim count is identical, including the eight patrol requests.

The one line that is the deliverable, from `disai patrolstate`:

```
{nearest DishonoredRoute_3 active 1 points 2 null 0 squad 'GuardsB' pawnsquad 'GuardsB' squadok 1 cap 1/1 range 146/500}
(destreached 0 rotreached 0 rotfocus 0 stoptype 0 rottarget 0; loco desired 1 paused 0 id 0 cpnt 1
 dest X=17588.707 Y=23150.305 Z=3017.461; otherstate none cpnt 0 id -999)
<loco req 1 path 1/7 speed 158/158 dist 1625>
[DishonoredNPCPawn_12 DisBehaviorPatrol route DishonoredRoute_3 idx 1/1 rep 0 dir 0 guard none
 substate 1/DisAISubStateTakeActorPosition nearestroute 146 canadopt 0]
```

A guard, in `DisBehaviorPatrol`, holding `DishonoredRoute_3`, at route index 1, in the `TakeActorPosition` sub-state of
slot 1, with a seven-point navigation-mesh path and the walk speed its tweaks asked for. Reached with five key presses'
worth of nothing: the level's own spawner raised the stim, the level's own route was adopted, and no switch forced any
of it.

### 5.2 The four that refuse, and why that is the result rather than a shortfall

```
_11 GuardsB     route DishonoredRoute_2   nearest 143 uu   walking (no path: goal off the loaded navmesh)
_12 GuardsB     route DishonoredRoute_3   nearest 146 uu   WALKING, 7-point path, 158 uu/s
_13 GuardsB     none                      nearest 1389 uu  nearest route is squad 'Servant'  -> refused
_14 GuardsB     none                      nearest 1394 uu  nearest route is squad 'Servant'  -> refused
_15 Servant     route DishonoredRoute_4   nearest 306 uu   WALKED 265 uu, arrived (26 uu short)
_19 WorkersBoat route DishonoredRoute_1   nearest 197 uu   walking (no path)
_20 WorkersBoat none                      nearest 194 uu   DishonoredRoute_1 capacity 1, _19 holds it -> refused
_25 Servant     none                      nearest 1855 uu  outside every route's 500 uu adoption range -> refused
```

Each refusal is one of `CanAdopt`'s five tests answering for the level's own data. A port that accepted all eight would
be the wrong answer.

### 5.3 What still stops a patrol completing a lap, measured precisely

Two separate things, neither of them in this package, and both worth a hand-over:

**(a) Three of the four route destinations are not on the loaded navigation mesh.** `last path error GoalPolyNotFound`,
and the navmesh census says `1 pylons (1 enabled, 1 with a mesh), 668 polys` with `21 of 26 NPCs standing on a poly`.
668 polys is not the whole Dunwall Tower exterior; `L_Tower_Nav` is one of the eight levels `L_Tower_P` streams and the
patrol points sit outside what is loaded at the boat landing. `DishonoredNPCPawn_12`'s seven-point path is the proof
that a route point *inside* the loaded mesh paths fine.

**(b) The arrival never becomes the sub-state's own `DestinationReached`, so `FindNextPoint` is never reached.**
`DishonoredNPCPawn_15` shows `arrived`, `dist 26`, and `destreached 0` at the same time. Two counters localise it
exactly, and neither moved from HEAD:

```
disai loco:    requests 26 start 0 update 0 stop        (GDisLocoRequestsStarted / Updated / Stopped)
disai desires: 41 set calls; ... loco 26/0/0            (GDisDesireRequests[DDK_Loco] = StartLoco calls)
```

Four patrol sub-states called `SetLocoLocationDesire` with a live destination (the probe prints
`loco desired 1 paused 0 dest X=17588.707 …`) and `DisDesireStructs::StartLoco` was still called exactly 26 times — once
per NPC, for the Idle behaviour's `Stand` sub-state and never again. So `FDisDesireRequest::GetRequestStatus` answered
`DTDRS_Unchanged` for every one of them and `FDisLocoRequest::DoRequest` returned at its first line. (Whether the
component's live request is still the one the `Stand` sub-state made, and therefore whether
`FArkComponentLocomotion::HandleArrival`'s `m_bDestinationAlreadyReached` latch is what swallows the second arrival, I
did **not** measure — it follows from the counters but nothing here reads that flag.)

**The anomaly to chase is in the probe's own output, and I eliminated the two obvious explanations.** The probe reads,
on all four patrol sub-states, simultaneously:

```
loco desired 1   paused 0   id 0   cpnt 1
```

and the global counters read `GDisDesireRequests[DDK_Loco] = 26`, `GDisDesireUpdates[DDK_Loco] = 0`,
`GDisDesireStops[DDK_Loco] = 0`.

* `cpnt 1` kills the first candidate: `m_pLocoComponent` **is** bound, so
  `IDisDesiresInterface::InitializeDesires` did run for these sub-states and did not take its
  `GetDesiresOwningPawn()` early-out.
* `0 stops` kills the second: `InitializeDesires` calls `PauseRequest()` on every request it binds, and `PauseRequest`
  calls `StopRequest` → `DisDesireStructs::StopLoco` whenever `m_RequestID != INDEX_NONE`. Zero stops in 260 seconds
  means every request's `m_RequestID` was already `INDEX_NONE` at that moment — so a virgin request is **not**
  zero-valued, and `FDisLocoRequest::Initialize` not writing the field is not the bug (retail's `0x8b4280` does not
  write it either; it clears the paused bit and writes five pointers).
* which leaves `paused 0, id 0` on a request that `StartLoco` was never called for, and those three readings cannot all
  be true under `disdesirestructs.cpp` as it stands. Something writes `m_RequestID = 0` and `m_bPaused = FALSE` on these
  four requests without going through `DoRequest`.

**The next instrument is the status, not the fields**: one counter per `EDisDesireRequestStatus` value returned by
`FDisDesireRequest::GetRequestStatus`, and one per `DoRequest` entry, in `disdesirestructs.cpp`. That is agent DN's and
agent DF's layer; a package that narrows it to "four requests reach `RequestLocationTarget` and `GetRequestStatus`
answers `Unchanged` for all four, with the component bound and nothing paused" is worth more than a package that patches
a field it does not understand.

## 6. Perception, handed over with its whole layout

This is the part of the brief I did not deliver, and the honest reason is size: the chain from "a guard's eye" to "the
guard knows the player is there" is **five classes and about 14 KB of retail code**, against 5 KB for the whole patrol.
What I did instead of half-porting it is measure it completely, so the next package starts from a layout rather than a
decompile. `build/agentEP/spec/vision_spec.md` (1026 lines) and `build/agentEP/spec2/creation_spec.md` are the artefacts;
this is the shape of it.

**The chain, with every 2013 address resolved by hand** (`build/agentEP/resolve.py`; five of them are unnamed in 2013 and
each was identified a second way — `0x3945f0` and `0x536e70` and `0x755a10` by their 2012 mangled names through
`match_2012_2013.csv` plus the store into `UWorld+712` / the single call site / the `PreBeginPlay_NativeComponents`
vtable slot, and `0x8856a0` and `0x6ec9e0` as section 8 describes):

```
UWorld::Init (0x3945f0)  appMalloc(180) -> FArkComponentManager::FArkComponentManager (0x53a5e0) -> UWorld+712
UWorld::TickAW           FArkComponentManager::Tick (0x53a6b0) phase 1
                           -> policy at manager+0x38
                           -> FArkComponentTimeSlicedTickPolicy::Tick (0x536e70, unnamed in 2013; 0.9 ms budget)
                                -> FDisComponentVisionNPC::TimeSlicedTick (0x878fc0)
                                     -> FDisComponentVision::TimeSlicedTick (0x8788e0, 1753 B)
                                          -> FindObservablesInVisionCone (0x8754c0) walks the OBSERVABLE policy
                                             list at manager+0x70 and tests DisIsPointInVisionCone (0x7f1580)
                                          -> CheckPointInLOS (0x867520) -> DisLineProbe  (PORTED already)
                                          -> vtable+56 VisionStatusChanged (0x864500)
                                               -> ADishonoredNPCController::Start/StopSeeingVisibleThing
                                                  (0x7591a0 / 0x759270)  <- NOT PORTED
                                                    -> UDishonoredAIBrain::HandleTargetSighted (0x723040)
                                                         -> raises FAIStimStruct_TargetSighted (id 45)
ADishonoredNPCController::PostBeginPlay (0x74e690)  adds FDisComponentVisionNPC (432 B) and stores it at ctrl+904
ADishonoredPawn::PreBeginPlay_NativeComponents (0x755a10)  adds FDisComponentObservable (80 B) at pawn+1920,
    config = UDisTweaks_Pawn::m_pObservableComponentTweaks (+240) + 140
FDisComponentVision::Starting (0x8673c0)     registers component+0x10 with manager+0x8C, vtable slot 0
FDisComponentObservable::Starting (0x86a2f0) registers component+0x10 with manager+0x70 AND component+0x14 with manager+0
```

The tick phases are worth carrying over verbatim, because two of them are dead: `FArkComponentManager::Tick`'s switch
handles **0, 1, 2 and 5 only**, `UWorld::TickPreAW` ticks phase **5 before phase 0**, and `UWorld::TickPostUpdate`
(`0x25da60`) calls it with 3 and then 4, both of which hit the default arm and return. Do not invent policies for 3 and 4.

A policy is 28 bytes — vptr, a registered `TArray`, a scratch `TArray` — and its vtable has exactly **two** slots,
`Register` at +0 and `Unregister` at +4. The time-sliced one is 40 bytes (a third array at +28). The manager is 180.

**What is absent in this tree and has to come with it**: `Engine/Src/arkcomponentmanager.cpp` (8-line banner),
`Engine/Inc/arkcomponentpolicy.h` + `.cpp` (10 and 7 lines), all six `IArkComponent*` interfaces, all of
`Src/discomponentvision.cpp`, `discomponentvisionnpc.cpp`, `discomponentobservable.cpp` and
`dishonoredutilities_vision.cpp` (every one on `DishonoredGame_EXCLUDE`), and `ArkRemoveRoll` in
`Engine/Src/arkutils.cpp` (9-line banner), which `UpdateVisionConeMatrix` needs. **What is already present and correct**:
`UDisTweaks_Vision` with all twelve fields at the right offsets, `FDisComponentObservableConfig`, `DisLineProbe`,
`UArkComponentContainer`, `FArkComponentBase`, `FArkGameEventDispatcher`, and the whole of
`Start/StopSeeingVisibleThing`'s downstream (`HandleTargetSighted` is three lines over machinery that exists).

The one thing `vision_spec.md` flags that a porter must not paper over: `FDisComponentVision` has **eight unexplained
bytes at offset 24..31** and `FDisComponentVisionNPC` twelve at **404..415**, neither read nor written by any of the 61 `FDisComponentVision*` /
`FDisComponentObservable*` bodies in `build/agentEP/dec2013/`. Reserve them; do not invent members there.

## 7. The reaction, and why it is behind one more class

"A guard reacting to being seen" I read as the behavioural consequence of a sighting, and it has exactly one door.
`FAIStimStruct_TargetSighted` has **one** consumer in all of retail 2013 — I searched every `DisDelegate` thunk in the
image for it:

```
0x7415f0  DisDelegate<uint,FAIStimStruct>::PrivateDelegatorMemFn<UDisAIBrainProcessAttention,
              FAIStimStruct_TargetSighted, {UDisAIBrainProcessAttention::FilterTargetSighted}>
0x7372a0  ... the same for FAIStimStruct_TargetUnsighted
```

No behaviour's evaluate mask covers `TargetSighted`; it is a **brain-process filter**, and the process turns it into
attention, which then raises the `NoticeBegin` / `SearchRequest` / combat stims that behaviours *do* answer. So the
reaction is `UDisAIBrainProcessAttention`: 61 named functions, 5,507 bytes, plus `UDisAttentionInfo_Base` / `_Simple` /
`_Complex`, `disattentiondefs.cpp` and `disattentiontargetcomponent.cpp` — five more skeleton units.

Two things about it are already standing and should shorten that package: `FDisAttentionProxy` is ported, and
`UDishonoredAIBrain::GetAttentionProxyInfo` / `GetTopEnemyProxy` / `ClearAllMinAttention` / `MaxOutAttention` are all
already call sites waiting for it. `UDisBehaviorNotice`'s own three callbacks are ported too (agent DF), so once the
attention process raises `NoticeBegin` a guard's head turns without any further work.

## 8. Deviations and defects, stated plainly

1. **Four mislabelled addresses in this tree, all pre-existing, all corrected here.** Each one passes `rva_sweep.py` as
   `ok-2013-mid` — it is inside *a* function, just not the one it names. The check that catches it is
   `build/agentEP/addr_audit.py`: it resolves every address the package's files label as 2013 with `ida_funcs.get_func`
   and prints `start` or `MID`. The first run reported **six** `MID`; each of the six was then looked up by NAME in
   `functions_2013.csv`, which named four of them elsewhere in the image — `0x7549d0` for
   `ADishonoredNPCController::InitNPC` (really `0x7632e0`, 60 KB away), `0x557160` for
   `FArkGameEventDispatcher::GetInstance` (`0x54df00`), `0x74f180` for `FDisLookAtRequest::Initialize` (`0x8b2400`) and
   `0x7431e0` for `UDisAINoiseManager::RegisterListener` (`0x851570`). The other two
   (`UDisSteeringInfluence_EnemyPush::SetEnemyRange`, `ADishonoredSpawner::execOnStartSpawn`) have no 2013 name at all,
   so no name check is possible for them; they are left as their authors wrote them and flagged here. The audit now
   reports **142 starts and those 2 mid** out of 144.
2. **`UDisBehaviorPatrol::OnPostGameLoad` is not ported.** Retail's `UDishonoredAIBehavior` has an
   `OnPostGameLoad(UBOOL,UBOOL)` virtual that this tree does not, and adding one for a single behaviour would move a
   vtable the whole AI save stack (agents EJ, EN) was measured against. A patrol restored from a save does not re-adopt
   its route; the spawner re-requests it instead. Same reason for `UDisBehaviorPatrolSearch::OnPostGameLoad`.
3. **`ADishonoredRoute::GameSave` and `GameLoad` are not ported**, for agent ED's reason (no writing half) plus the save
   class list. A restored route comes back with its default `m_bIsActive` and no adopters, which is the state a freshly
   begun level is in anyway.
4. **`UDisAISubProcessWatchPoints` is three accessors, not the class.** `StartWatchingPoints` (`0x73a9d0`),
   `IncrementWatchPoint` (`0x73aaf0`) and `TickSubProcess_Derived` (`0x73e440`) need
   `ADisGuardWatchPoint::GetWatchDuration` and the look-at target desire. The measurable cost: `IsWatchingPoints` always
   answers FALSE, so `SetGuardPoint` takes retail's own no-watch-points branch and a guard post is held for
   `m_fGuardDuration` (or forever) without the head sweeping between its watch points.
5. **`UDisBehaviorPatrol::OnBehaviorPause` is an empty body**, as retail's is minus one call:
   `FDisComponentAnimPlayer::GetPlayingAnimOnChannel`/`StopAnim` are not ported, so there is no induced idle animation
   to stop — and `OnEnterCallback_Stand` does not induce one either (`UDisAISubProcessAmbientAnims` is a skeleton). It
   logs once when it would have.
6. **`ResolveRouteIndex` keeps retail's two unreachable switch arms.** Retail's switch has five; `ERouteType` has three
   values, so arms 3 and 4 cannot be reached with legal data. They are kept because retail's own *grouping* is what
   documents the other three: `ERT_Circle` shares its arm with 3 and `ERT_Loop` with 4, which is how you know
   `ERT_Loop` is a there-and-back and `ERT_Circle` a wrap.
7. **`CanAdopt`'s second parameter is a one-map exception and I ported it as one.** 2013 added a `UBOOL` whose only
   effect is to skip the adoption-range test when the current level's package is `L_Isl_LowChaos_P`. It is TRUE only
   from `OnPostGameLoad`, which is not ported, so in this tree the parameter is always FALSE. The map name is retail's
   own string constant.
8. **The corpse block of `FSpawnNPCPawn_TweakObj::DoInit` is not ported.** Retail's first block, for a spawner that says
   spawn-dead or treat-as-knocked-out, is four calls on `ADishonoredNPCPawn + 2340` (`0x76e240`, `0x76e250`, and
   `0x76e210` / `0x76e1a0` for the already-handled case). That is the corpse path, not the spawn path. A spawner marked
   "spawn dead" produces a live NPC — which is what it already did before this package, and now the flags at least say
   so, so `CreateActor` correctly declines to give it a controller.
9. **One defect of my own, found by reading a vtable I had guessed at.** `FindNextPoint` ends, when the route has been
   patrolled `m_PatrolAttentionSpan` times and a new one has been adopted, with an indirect call through the behaviour's
   own vtable at offset 440. I first wrote that as `OnBehaviorStart()`, which would have called `ResetPatrol` and
   **dropped the route `ChooseNewRoute` had just adopted**. It is `UDisBehaviorPatrol::OnNewRouteChosen`, 2013 rva
   `0x6ec9e0`. I pinned the vtable twice as the brief requires: the constructor at `0x6f9160` stores
   `UDisBehaviorPatrol::vftable{for UObject}` (rva `0xd28340`) and that table's slot 110 is `sub_AEC9E0` = `0x6ec9e0`;
   and the **2012** table (`vtables.csv`) carries `UDisBehaviorPatrol::OnNewRouteChosen` at slot **109** — one slot
   earlier, because 2013 inserted a virtual at slot 104 — with slots 105..109 in exactly the order 2013's 106..110 have
   them. Retail's body is `GetSubProcess( UDisAISubProcessAmbientBarks )` and a tail jump to that sub-process's
   `ScheduleRouteSwitchBark` (2013 `0x7398a0`), which sets `m_bDoRouteSwitchBark` and loads `m_fBarkInhibitionTimer` from
   the tweaks' `m_fSilenceBeforeRouteSwitchBark`; that sub-process is a skeleton, so the ported body is a documented
   no-op that logs once. `UDisBehaviorPatrolSearch`
   overrides it in retail with an empty body (its own slot 110 is ICF-folded onto `UDishonoredTask_Base::OnAdded_Impl`,
   which is one byte, a `ret`), so inheriting this no-op is what retail does for that class anyway.
   **And `match_2012_2013.csv` names `0x6ec9e0` wrongly**, which is the second truth-source correction in this package:
   it maps it to 2012 `0x789900`, `UDisAISubProcessAmbientBarks::ScheduleRouteSwitchBark` (ratio 0.760, by neighbours),
   a 59-byte function where `0x6ec9e0` is 38. The disassembly says why the match went wrong and settles the identity at
   the same time — the whole of `0x6ec9e0` is `GetSubProcess( UDisAISubProcessAmbientBarks::StaticClass() )` followed by
   `jmp 0x7398a0`, and `0x7398a0` holds `ScheduleRouteSwitchBark`'s 59 bytes — the same 59 as 2012's — which IDA
   attributes as a tail chunk of `0x6ec9e0` rather than as a function of its own, so the tail's bytes dominated the
   neighbour match. Its last three instructions are `or dword ptr [esi+68h], 1` (`m_bDoRouteSwitchBark`) and
   `fstp dword ptr [esi+6Ch]` (`m_fBarkInhibitionTimer` ← the tweaks' `m_fSilenceBeforeRouteSwitchBark` at +144), which
   is what settles which of the two names belongs to which address.
   The caller is a vtable slot on a behaviour, and `GetSubProcess` is a `UDishonoredAIBehavior` method, so `this` cannot
   be the sub-process.
10. **A second defect of my own, in a name.** I called `0x6fd350` `OnPatrolPointReached`. Retail's own PDB calls it
   **`UDisBehaviorPatrol::OnReachedDestination(AActor* const, UBOOL)`** — it is on the skeleton banner
   `import_reference.py` generated, and I renamed it while rewriting that banner. Invented, and now corrected everywhere.
11. **Whether `ADishonoredRoute::Tick` is reached at all, I did not measure.** `ARoute` derives from `AInfo`, which in
    UE3 is normally a non-ticking actor, and the tick is the only thing that advances `m_NeglectTimer`. It matters only
    after a route is *released*: `PostBeginPlay` starts every route already neglected (`m_NeglectTimer =
    m_NeglectThreshold + 1`), which is why `AdoptNewRoute`'s neglected branch is the one that fires on a fresh level and
    why all four adoptions in section 5 happen. If the tick never runs, a route given back by one NPC is never preferred
    over a nearer un-neglected one again. One counter in `ADishonoredRoute::Tick` settles it.
12. **The census additions are instrumentation, not retail.** `DisAINoteStim`, `DisAIStimHistogram`,
   `DisAIPatrolReport`, `DisAIPatrolState`, `DisAINoteSighting` and `DisAISightingReport` live in
   `disaicensus.cpp`, which this tree already designates as the measurement file, and cost nothing without `-disai`.
   `DisAINoteSighting` is called from nowhere yet: it is the hook `FDisComponentVisionNPC::VisionStatusChanged` will call,
   and it reports `0 started, 0 stopped` — which is this package's honest answer to "does a guard perceive the player".
13. **One edit to `UDishonoredAIBrain::EnqueueStim`**: one line, `DisAINoteStim( _pAddMe->m_StimID )`. It is the whole
    reason section 3 exists.

## 9. Verification

* **Accept 1 — a guard walking a patrol**: section 5, `build/agentEP/patrol13_log.txt` on the tree as handed over
  (`patrol1` … `patrol12` are the sequence of measurements that got there, each with the build it was taken on). A guard perceiving the player and a guard reacting to
  being seen are **not delivered**; sections 6 and 7 are what they need, measured, with the two spec files.
* **Accept 2 — before/after against the untouched HEAD executable**: section 3 is HEAD, section 5 is after, same command
  line, same driver, same build directory recipe. HEAD was built from the worktree before any edit and staged as
  `DishonoredGame_EPH.exe`; `build/agentEP/base_nullrhi_log.txt` and `base_walk_log.txt` are its logs. The
  `-dislocowalk` control run was repeated on the tree as handed over (`build/agentEP/walkchk2_log.txt`, **28,171.9 uu
  moved over 13 of 26 NPCs, 49 paths built, 22 arrivals**, against 24,033.9 / 13 / 48 / 23 on an earlier build of this
  package and 20,335 on HEAD's own `base_walk_log.txt`) to show this package did not disturb the locomotion another agent
  measured — and to separate "the locomotion cannot move a pawn" from "nothing asks it to", which is what section 5.3 is
  about.
* **Accept 3 — regression**: `python build/agentEP_wt/resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEP_regwt --exe-name DishonoredGame_EPR.exe --log-prefix agentEP_reg`, the
  worktree's own copy, an absolute build dir, no `--no-build`. The five gitignored layout inputs
  (`resources/docs/types/{all_types.h, retail_sdk_layout.json, script_classes_2012.json, script_classes_2013.json,
  types.json}`) were copied into the worktree first. Result: section 10.
* **Accept 4 — clean full Release build**: `build/agentEP_clean_build.cmd` deletes `build/agentEP_clean` first,
  configures the worktree with `DISHONORED_LAYOUT_CHECKS=ON` and builds all three targets.
  On the tree as handed over: **exit 0, 999 ninja edges, 2,335 log lines, 0 errors, 0 C4263, 0 C4264**
  (`build/agentEP/cleanbuild3.log`). Two earlier clean builds, each of an earlier state of the package, are
  `cleanbuild.log` and `cleanbuild2.log`; all three were 0/0/0. `cleanbuild3` predates two comment-only edits made while
  writing this document, so the tree as handed over was also built incrementally afterwards
  (`build/agentEP_b13.log`, 0 errors, 0 C4263, 0 C4264) and the regression's own build stage compiled it from scratch in
  attempt 4.
* **Accept 5 — `rva_sweep.py` plus a by-hand resolution**: `build/agentEP/sweep_mine.py` runs the sweep over the whole
  worktree and then reports the 24 files this package owns separately: **332 citations, 191 `ok-2013`, 140
  `ok-2013-mid`, and exactly one flagged — `0x7398a0`, deliberately.** That one is not a function start in the 2013 PDB
  because it is the shared tail chunk of `0x6ec9e0`, which is the fact the comment beside it exists to record (section
  8.9); the sweep classifying it `UNKNOWN-CLAIMED-2013` is the sweep being right. The two `MISLABELLED-2012` elsewhere in
  the worktree are both pre-existing in `GFxUI/Src/gfxuirenderer.cpp:1669`, which is what agents EJ and EN also reported.
  `build/agentEP/addr_audit.py` then resolves all **145** distinct 2013 addresses those files cite with
  `ida_funcs.get_func`: **142 function starts and 3 mid**. One of the three is that same `0x7398a0`; the other two are
  the only citations whose targets have no 2013 name at all (`ADishonoredSpawner::execOnStartSpawn`,
  `UDisSteeringInfluence_EnemyPush::SetEnemyRange`), so no name check is possible for them. The four that were genuine
  mislabels are corrected. Section 8.1.
* **Layouts** — unchanged. This package adds no reflected member and changes no class's size: every new declaration is a
  member function or a static, and the layout stage's own numbers are in section 10.

## 10. The regression

`resources/tools/run_regression.py`, **37** checks over six stages, run from the worktree's own copy with an absolute
`--build-dir` and no `--no-build`, so all six build checks count. Transcript `build/agentEP/regression.txt`, summary
`build/agentEP_regwt/regression/summary.txt`.

**It took four attempts, and the three that did not pass are worth the paragraph**, because two of them are the brief's
"a d3d9 or inputtest failure is usually the machine" measured from the inside:

| attempt | tree | result | `d3d9_startup_seconds` | `cl.exe` running |
|---:|---|---|---:|---:|
| 1 | before the last two corrections | **37 ok, 0 failed, 0 skipped, 1245 s** | 3.6 | 0 |
| 2 | as handed over | 35 ok, **2 failed** (`d3d9_frames` 480, `inputtest_moved` 653.0) | 32.1 | 18 |
| 3 | as handed over | 36 ok, **1 failed** (`d3d9_frames` 570) | 30.0 | 18 |
| 4 | as handed over | **37 ok, 0 failed, 0 skipped, 427 s** | 3.4 | 0 |

The same executable and the same command line gave `d3d9_startup_seconds` 3.6 s on a quiet machine and 30-32 s while
another agent ran an eighteen-way `cl.exe` build; `d3d9_frames` is a count over a fixed 90-second wall clock, so it
collapsed from 2310 to 480/570 against a bound of 1000. `inputtest_moved` did the same once (653.0 against 800.0) and
read 1018.5 and 1035.4 on the two runs where the machine was free. Nothing in the failing runs was a criticals count, a
layout number or a count of anything this package changed: `unported_natives` was 0, `layout_mismatching` and
`layout_contract` 0, `physics_actors` 1369 and `coresmoke_passed` 99 in every one of the four.
Transcripts: `regression_first.txt`, `regression_second.txt`, `regression_third.txt` and `regression.txt`.

**37 ok, 0 failed, 0 skipped, 427 s**, on the fourth attempt, on the tree exactly as handed over, first run of the
harness on a quiet machine:

| stage | checks | the numbers that matter |
|---|---:|---|
| build | 6 | **0** errors and exit **0** for each of DishonoredGame, CoreSmoke and LayoutProbe |
| coresmoke | 2 | **99** coresmoke_passed, **0** coresmoke_failed |
| layout | 7 | **2314** layout_types, **0** layout_mismatching, **0** layout_contract, **2341** layout_probed |
| nullrhi | 4 | **2.8** startup_seconds, **0** nullrhi_criticals, **6692** nullrhi_log_lines |
| d3d9 | 9 | **3.4** d3d9_startup_seconds, **2370** d3d9_frames, **6506** d3d9_draw_elements, **452** d3d9_visible_prims, **0** unported_natives |
| inputtest | 9 | **1010.6** inputtest_moved, **500.0** inputtest_peak_speed, **1369** physics_actors, **1312** physics_static_shapes, **6** probe_natives |


## 11. Merging

**`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` (all three flags) IS required, and I ran
it in the worktree**, five times as the package grew. Its content diff is five generated files and no more:

* `Sources.cmake` — **nine** skeleton units come off `DishonoredGame_EXCLUDE`, **818 → 809**:
  `disaisubprocesswatchpoints.cpp`, `disbehaviorpatrol.cpp`, `disbehaviorpatrolsearch.cpp`,
  `disglobalenums_utilities.cpp`, `dishonorednavpoint.cpp`, `dishonoredroute.cpp`, `dispatrolmanager.cpp`,
  `dissquadinterface.cpp`, `distweaks_npcpawn.cpp`.
* `dishonoredgameclasses.h`, `DishonoredGameSearchClasses.h`, `DishonoredGameGlobalEnumsClasses.h` — the nine new
  `#include "CppText/…"` lines the generator inserts.
* `DishonoredGameNativeStubs.cpp` — the four `UDisBehaviorPatrol::exec*` stubs are gone, because
  `DishonoredGameNativeStubs.ported.agentEP.txt` names them.

It rewrites about eighty other generated headers with no content change (`git diff --numstat` reports 0/0 for each),
which is what agent EN also saw.

**`Sources.cmake` DOES change**, by nine units.

**Thirty files, and every source file is inside `DishonoredGame`.** Nothing in `Engine`, `Core`, `GFxUI`, `GameFramework`
or any other module; nothing in `resources/tools`. The two documents are the only files outside the module. Total
including them: **32**.

`build/agentEP_sync.py` is the authoritative list and performs the copy (`--apply`, **worktree → main**). Nothing is
committed and nothing is staged; the worktree is left dirty with exactly those 30 files modified or added plus the
~80 generated headers the generator rewrote without changing, and the five gitignored layout inputs copied into
`resources/docs/types/` (which `git status` does not show).

## 12. Hand-overs

1. **A sub-state that is not the brain's first never gets a locomotion order.** Section 5.3 is the measurement:
   `DisDesireStructs::StartLoco` is called exactly 26 times in a 260-second run — once per NPC, for the Idle behaviour's
   `Stand` sub-state — while four patrol sub-states sit in `DisAISubStateTakeActorPosition` holding a live destination,
   a bound locomotion component (`cpnt 1`), `m_bDesired` set and `m_bPaused` clear. Two candidate causes are eliminated
   there by measurement, and the third needs one counter per `EDisDesireRequestStatus` inside
   `FDisDesireRequest::GetRequestStatus` rather than another look at the request's fields. This is the one thing between
   "a guard walks to its first patrol point" and "a guard walks its route".
2. **Perception is `FArkComponentManager` + `FDisComponentObservable` + `FDisComponentVision` +
   `FDisComponentVisionNPC` + the cone maths**, about 14 KB and ~80 functions, and it is fully specified in
   `build/agentEP/spec/vision_spec.md` (member layout of all three classes with the evidence for every name, the vtable
   slot of every indirect call, the cone algebra) and `build/agentEP/spec2/creation_spec.md` (both creation sites, the
   tick phases with their call sites, which policy each `Starting` registers with, where the tweaks come from).
   `build/agentEP/dec2013/` holds 180 retail decompiles behind it, and `build/agentEP/spec2/dec/` another 12.
3. **The behavioural reaction is `UDisAIBrainProcessAttention`** and nothing else: `FAIStimStruct_TargetSighted` has
   exactly one consumer in the whole image. Section 7.
4. **`L_Tower_P`'s navigation mesh at the boat landing is 668 polys and does not cover three of the four adopted patrol
   routes** (`last path error GoalPolyNotFound`). Whoever works on streaming should know that the AI's reach is bounded
   by this and not by the AI.
5. **A base behaviour that answers a stim is silently shadowed by its subclass** — the subclass comes earlier in
   `m_BehaviorArray`, inherits the mask and the delegate, takes the slot and does nothing. Section 3.2. In `DisAIBrain`'s
   own 25 behaviour classes there is exactly one such pair and this package closed it, but check the subclass's
   `GetEvaluateStimDelegate` whenever you port a base's, and check the brain being used: a different
   `UDisTweaks_AIBrain` has a different list.
6. **`ADishonoredSpawner`'s eleven booleans share one bitfield word at 1116** and two of them are adjacent in meaning as
   well as in bits (`m_bSpawnDead` bit 5, `m_bTreatAsKnockedOut` bit 7, with `m_bSpawnDead_AlreadyHandled` bit 6 between
   them). Section 4.1 is what happened when that was read one bit out. `m_SpawnerInfo`'s own `m_bSpawnDead` and
   `m_bStraightToRagdoll` are bits 0 and 1 of the same word at +76, which is the other half of the confusion.
7. **`rva_sweep.py`'s `ok-2013-mid` verdict hides a mislabel.** Four of this tree's citations name a function they are
   not the start of, and one of them (`0x7549d0` for `InitNPC`) is 60 KB away from the truth. `addr_audit.py` is nine
   lines of IDAPython and it should probably be promoted into `resources/tools` beside the sweep.
