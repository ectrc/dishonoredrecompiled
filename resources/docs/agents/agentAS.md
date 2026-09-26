# Agent AS — touch, triggers and volumes: the overlap gates read the trace mask, and the world reacts (2026-09-26)

Package AS of `PHASE7.md`. Agent AQ converted the primitive-octree and encroachment gates to Arkane's 7-bit
`FDisPrimTraceMask m_CollisionTraceTypes` (@320) and the pawn started standing and walking; the **overlap and touch
gates were left reading the dead `BlockZeroExtent` / `BlockNonZeroExtent` booleans**, which are FALSE on every primitive
in every cooked package, so nothing ever stayed touching anything. This package converts them, puts retail's
`CollisionType` mapping the right way round, fixes `execSetTraceBlocking`, and measures the result.

Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentAS.i64`, a copy of `retail2013_named.i64`); every
"2013 rva" is from a headless decompile in `build/agentAS_decomp`.

## Result

| Acceptance | State |
|---|---|
| a `-distouch` census showing non-zero touch notifications and at least one volume entry | **done**. `l_tower_p`, 107 s: `touch begin 250 end 240 (volume 100, trigger 150), volume entered 50 left 50, PhysicsVolume changes 50, SeqEvent_Touch fired 46`. Before the fix, on the same build with only the four gates reverted: `touch begin 4558 end 4558 (volume 4558, trigger 0), volume entered 0 left 0, PhysicsVolume changes 0, SeqEvent_Touch fired 0` |
| walking into a trigger volume fires `SeqEvent_Touch` (logged) | **done**, 46 activations, named in the log with their Kismet path, e.g. `L_Tower_Script...SOIREE_Campbell_Sokolov.SeqEvent_Touch_1 fired on TriggerVolume_14 touched by DishonoredPlayerPawn (TriggerCount 1)`. **0** in the baseline |
| a pickup picked up end to end, or the exact native that blocks it named | **named**. Pickups are now *touched* (`L_Pub_Day_P`: `DisAbstractItemPickup`, `DisAbstractItemPickupNote`, `DisStatPickup` all enter the pawn's `Touching` list, `CollisionType 4` / mask `111 1111`), but they cannot be *consumed*: `DishonoredGame/Src/dispickup_base.cpp` is a 46-line comment stub — **none of `ADisPickup_Base`'s 42 functions is ported**. Blockers with rvas in the table below; `DisPickup_Base` declares no script `Touch`, so picking up is entirely that C++ focus path |
| 0 critical errors | **done**, `--forbid Critical` satisfied on every run (120 s d3d9 `l_tower_p`, 120 s d3d9 `L_Pub_Day_P`, plain startup, baseline) |
| agent AQ's collision numbers unchanged | **done**: `PhysX 895 actors, 1312 static shapes, 59 dynamic shapes; SDK 413 convex meshes, 9 triangle meshes`, `primitives 3425, attached 2928, CollideActors 1229`, **0** `Could not create new Shape` |
| no walking regression | **done**: `inputtest moved 1023.2`, `physics 1` (agent AQ: 972.2 … 1030; baseline build: 1026.0) |

**Agent AT: the touch path is sound.** `SeqEvent_Touch` fires with the right originator and instigator and `TriggerCount`
increments, `USeqEvent_Touch::CheckTouchActivate` / `CheckUnTouchActivate` are reached from `TouchTo` / `EndTouch`, and 46
distinct Kismet touch events in `L_Tower_Script` / `L_Tower_Audio` activated in one run. Nothing in `UnSequence.cpp` was
touched, so AT can merge on top.

## The three gates, against their decompiles

`BlockNonZeroExtent` is FALSE on all 3,425 primitives of `l_tower_p` (agent AQ's `-distrace` inventory, still true after
this package — retail never writes those booleans). Every gate below was `&& <component>->BlockNonZeroExtent`, so all of
them evaluated FALSE for every component in the game.

### `AActor::IsOverlapping` — **2013 rva 0x173920**

Unnamed in the idb; it is `AActor` vtable slot +456, reached through `AActor::execIsOverlapping` (0x2cd6c0). Three
distinct gates:

1. **The outer per-component loop** (`UnActor.cpp:2783`) drops the blocking test altogether. Retail:
   `if (!v64 || (*((_BYTE *)v64 + 76) & 1) == 0 || (*((_BYTE *)v64 + 280) & 0x10) == 0) continue;` — attached and
   `CollideActors`, nothing else.
2. **The inner loop** (`2793`) gains a *symmetric OR*, which is new behaviour rather than a substitution:
   ```c
   || !FDisPrimTraceMask::MatchesTraceFlags((FDisPrimTraceMask *)(v33 + 80), a3, 0)     // MyPrimComp, source = Other
   && !FDisPrimTraceMask::MatchesTraceFlags((... )((char *)v32 + 320), v6, 0)           // OtherPrimComp, source = this
   ```
   i.e. *either* side's mask stopping a movement trace by the other actor is enough. The reference required both sides
   to carry `BlockNonZeroExtent`.
3. **The box-overlap helper** (`2885`, `2899`) does the same with the box / primitive actors:
   `PrimComp->m_CollisionTraceTypes.MatchesTraceFlags(BoxActor, 0) || BoxActor->CollisionComponent->m_CollisionTraceTypes.MatchesTraceFlags(PrimitiveActor, 0)`.
   The `OtherPrimitiveComponent != NULL && PrimitiveActor == Other` branch only runs when `this` *is* the box actor,
   which is why retail passes `v6` (= `this`) to both of its tests there; ported literally.

`TraceFlags` is the literal `0` at all five call sites, so `MatchesTraceFlags` falls through to its source-actor test
(non-pawn → bit 0, pawn that is not the player → bit 1, the player pawn → bit 2).

This is the function that mattered most. `UWorld::MoveActor` begins touches from its `MultiLineCheck` results (which
already went through agent AQ's octree gates) and then calls `AActor::UnTouchActors`, which asks `IsOverlapping`
whether each touch still holds. With `IsOverlapping` always FALSE, **every touch was ended the same frame it began** —
measured as exactly 4,558 begins and 4,558 ends over 85 s in the baseline, all of them volumes, none of them triggers.

### `AActor::FindTouchingActors` — **2013 rva 0x189870**

Also unnamed; reached from `AActor::SetCollisionType` (0x18a390). The gate becomes
`Test->Component->m_CollisionTraceTypes.MatchesTraceFlags(this, 0)`, and the whole `bIsZeroExtent` notion disappears —
retail still calls `GetBoundingCylinder` at vtable +324 and discards both outputs, which is preserved.

The encroachment check is issued with `2105503` = `0x20209f` = `TRACE_AllColliding | 0x200000`. **0x200000 is an Arkane
trace flag the reference does not have**; found its readers by scanning the whole binary for that immediate inside
functions whose names mention Trace / Check / Overlap / Touch / Line / Point / Encroach / Move:

| function | 2013 rva | role |
|---|---|---|
| `AActor::FindTouchingActors` | 0x189870 | writes it |
| `UWorld::MoveActor` | store at 0x24cf23 | writes it (its encroachment pass) |
| `ADishonoredUsableObject::ShouldTrace` | 0x6494f0 | `a2 != this[189] \|\| (a4 & 0x200000)` |
| `ADisDefenceTower::ShouldTrace` | test at 0x6181f4 | same shape |
| `ADisWhaleOilReceptacle::ShouldTrace` | test at 0x865cd4 | same shape |
| `ADisDetectionEye::ShouldTrace` | test at 0x8bfdd4 | same shape |

So the bit means "this check is a touch / overlap sweep, not a movement trace", and the four DishonoredGame overrides
use it to expose a component they hide from real movement traces. Added as `TRACE_DisTouchOverlap = 0x200000` in
`Engine/Inc/UnLevel.h`, with its readers named in the comment. `AActor::ShouldTrace` (0x16cf40) and
`AActor::IsConsideredTransparent` (0x16cee0) do **not** test it.

### `UPrimitiveComponent::execSetTraceBlocking` — **2013 rva 0x12a230**

As agent AQ predicted, the whole body is the two setters. The decompiled store fills bits 0..2 from the *second* script
parameter (`NewBlockNonZeroExtent`) and bits 3..6 from the first (`NewBlockZeroExtent`), preserving bits 7+; the
setters do the same. Until now any script call to `SetTraceBlocking` changed nothing.

## `CollisionType`: retail's enum is a different enum

`AActor::SetDefaultCollisionType` (**0x179a00**) and `AActor::SetCollisionFromCollisionType` (**0x17e260**) only make
sense once you notice that **Arkane replaced `ECollisionType` wholesale**. `script_classes_2013.json`,
`Engine.Actor.ECollisionType`:

| value | reference name | retail name | mask it means |
|---|---|---|---|
| 0 | `COLLIDE_CustomDefault` | `COLLIDE_CustomDefault` | whatever the class default holds |
| 1 | `COLLIDE_NoCollision` | `COLLIDE_NoCollision` | nothing |
| 2 | `COLLIDE_BlockAll` | `COLLIDE_BlockAll` | all 7 bits, blocking |
| 3 | `COLLIDE_BlockWeapons` | **`COLLIDE_BlockGameplay`** | the 4 gameplay bits, no move, blocking |
| 4 | `COLLIDE_TouchAll` | `COLLIDE_TouchAll` | all 7 bits, not blocking |
| 5 | `COLLIDE_TouchWeapons` | **`COLLIDE_BlockMovement`** | the 3 move bits, blocking |
| 6 | `COLLIDE_BlockAllButWeapons` | **`COLLIDE_TouchMovement`** | the 3 move bits, not blocking |
| 7 | `COLLIDE_TouchAllButWeapons` | **`COLLIDE_BlockNPCVision`** | `m_bTraceForGameplay_VisionLOS` only, blocking |
| 8 | `COLLIDE_BlockWeaponsKickable` | **`COLLIDE_BlockMovement_PlayerOnly`** | `m_bTraceForMove_Player` only, blocking |

Nine values in both, same order, so nothing on disk moves — but four of the five weapon-flavoured names described the
*dead booleans* and meant the wrong thing. Renamed in `Engine/Inc/EngineClasses.h`; the only sites outside this package
were two editor-only lines in `UnFracturedStaticMesh.cpp` (`PostEditChange`'s "is this a touching type" test, now
`COLLIDE_TouchAll || COLLIDE_TouchMovement`) and `UnActor.cpp:1798`'s load fixup (value 3, name only).
`USeqAct_ChangeCollision` uses only the three names that did not change.

With the mapping the right way round, the census reads the collision types the cook intended:

```
DisTrigger_1          (DisTrigger)          CollisionType 4 (TouchAll)     collide 1 block 0 move 111 gameplay 1111
DishonoredWaterVolume_0                     CollisionType 4 (TouchAll)     collide 1 block 0 move 111 gameplay 1111
DishonoredAudioVolume_1                     CollisionType 6 (TouchMovement) collide 1 block 0 move 111 gameplay 0000
DisPossessionVolume_1 (L_Pub_Day_P)         CollisionType 6 (TouchMovement) collide 1 block 0 move 111 gameplay 0000
BlockingVolume_1                            CollisionType 5 (BlockMovement) collide 1 block 1 move 111 gameplay 0000
LightmassImportanceVolume_1                 CollisionType 1 (NoCollision)  collide 0 block 0 move 000 gameplay 0000
CullDistanceVolume_0                        CollisionType 1 (NoCollision)  collide 0 block 0 move 000 gameplay 0000
DisAbstractItemPickup_9 (L_Pub_Day_P)       CollisionType 4 (TouchAll)     collide 1 block 1 move 111 gameplay 1111
```

`SetCollisionFromCollisionType` has three deviations from the reference that are retail's, not mine:
`COLLIDE_NoCollision` does **not** clear `bBlockActors`; `COLLIDE_BlockAll` does **not** call `SetBlockRigidBody`; and
the tail sets `bPathColliding = m_bTraceForMove_NonPlayerPawn ? bStatic : FALSE` (@296 mask 0x1000000 from @288 mask
0x1) before mirroring `BlockRigidBody`, which the reference omitted entirely. `COLLIDE_CustomDefault` copies the whole
mask word off the class default's collision component.

## `-distouch`, the instrumentation

Two switches, both off by default and free when off.

**`-distouch`** — the census, once per second per world, re-armed for each new world (so the mission map is measured and
not the menu map):

* the counters: touch begin / end notifications, how many of the begins had an `AVolume` or an `ATrigger` on either
  side, `APhysicsVolume` entries and departures, `PhysicsVolume` reassignments, and `SeqEvent_Touch` activations;
* the world: actor count, how many have `bCollideActors`, volumes, triggers, `USeqEvent_Touch` instances, and how many
  actors are touching how many others *right now* — the last pair is what separates "touches fire" from "touches fire
  and are immediately undone";
* the pawn: location, `Physics`, its `PhysicsVolume` by name, and the names and classes of everything it is touching;
* the first 80 distinct begin-touch pairs, named with both classes, so a run's log shows what was actually walked into;
* every `SeqEvent_Touch` activation, named by Kismet path, detected by snapshotting `USequenceEvent::TriggerCount`
  around `CheckTouchActivate` — the only observable that says the event really fired;
* a one-shot inventory (up to 200) of every volume, trigger, pickup and `SeqEvent_Touch` carrier with its
  `CollisionType`, `bCollideActors`, touch-event count, distance from the pawn and its component's seven mask bits. An
  actor that carries a `SeqEvent_Touch` and reads `CollisionType 0` with an all-zero mask is one the octree can never
  hand a touch to; that is the line to look at first.

The counters live in `UnActor.cpp` next to the notifications; `AActor::SetZone` and `APawn::SetZone` in `UnLevAct.cpp`
bump the three volume counters, and `UWorld::TickWorldRBPhys` in `UnPhysLevel.cpp` calls the report next to agent AQ's
`DishonoredWorldTrace()`.

**`-distouchprobe`** — the walk-in. `-inputtest` only moves the pawn about a thousand units and `l_tower_p`'s triggers
are 20,000 units from the spawn anchor, so after the `-inputtest` walk has finished (14 s in) the probe moves the pawn
to one volume / trigger / pickup per second through `UWorld::FarMoveActor`, which is the same code path a real walk-in
takes (it calls `AActor::FindTouchingActors` and `AActor::SetZone` itself). It aims at the collision component's
`Bounds.Origin`, not `Location`, because a brush volume's `Location` is its pivot and usually outside the brush.

## Runs

Build: `set BUILD_DIR=build\agentAS` + `resources\build-release.cmd` (via `build/agentAS_release.cmd`, which only sets
the variable), 805 units, **0 errors, 0 link errors** (`build/agentAS_build{1..5}.log`). Built in the shared working
tree; agent AR's and AX's in-flight edits were in other files and never broke it.

```
python resources\tools\build_and_smoke.py --build-dir build/agentAS --no-build --exe-name DishonoredGame_AS.exe ^
  --log-name agentAS_final2.log --ini-dir build/agentAS/config --rhi d3d9 --timeout 120 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "inputtest moved" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -distouch -distouchprobe -distrace -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

| | baseline (`agentAS_base.log`) | after (`agentAS_final2.log`) |
|---|---|---|
| touch begin / end | 4558 / 4558 | **250 / 240** |
| of the begins: volume / trigger | 4558 / **0** | 100 / **150** |
| volume entered / left | 0 / 0 | **50 / 50** |
| `PhysicsVolume` changes | 0 | **50** |
| `SeqEvent_Touch` fired | **0** | **46** |
| probes that touched their target | **0 of 39** | **45 of 48** |
| `inputtest moved` | 1026.0, physics 1 | 1023.2, physics 1 |
| PhysX actors / static shapes | 895 / 1312 | 895 / 1312 |
| `Could not create new Shape` | 0 | 0 |
| `Critical` | 0 | 0 |

The baseline is `build/agentASbase_wt`, a detached worktree of HEAD `a64f9b4` with all eight of this package's files
copied over it and then **only the four overlap gates reverted** to the boolean form (`build/agentAS_baselinepatch.py`),
so the census, the probe and the `CollisionType` port are identical on both sides and the delta is the gates alone.
Remove it with `python resources/tools/make_snapshot.py ASbase --remove`.

The baseline's 4,558 begins with 4,558 ends is the signature of the defect: about 54 begin-and-immediately-end pairs per
second, forever, and not one trigger among them. Fixing the gates therefore *removes* per-frame work as well as making
touch work.

### The volume path, end to end

`l_tower_p` (`agentAS_final2.log`, 48 probes, 45 hits):

* `DefaultPhysicsVolume_1` is the pawn's `PhysicsVolume` from the first tick, with `eventActorEnteredVolume` firing
  (50 entries / 50 departures over the run, from `AActor::SetZone` and `APawn::SetZone`);
* `DishonoredWaterVolume_0` — probe 8: `PhysicsVolume DishonoredWaterVolume_0, touching 1: DishonoredWaterVolume_0`.
  The swim path follows immediately: `ADishonoredPlayerController::execCalcPlayerSwimAccelRate` starts firing (a
  still-stubbed DishonoredGame native, agent AU's);
* `TriggerVolume` (11 instances) and `DisTrigger` (28) — every one touched, with its Kismet `SeqEvent_Touch` activating;
* `DishonoredAudioVolume` (6) — all six touched;
* `BlockingVolume_0/1` are the two probes that correctly produce **no** touch: `CollisionType 5`
  (`COLLIDE_BlockMovement`, `BlockActors 1`), so `AActor::IsBlockedBy` excludes them from the touch set, which is
  right. The third miss, `TriggerVolume_3`, has an L-shaped brush whose bounds origin is outside it.

`L_Pub_Day_P` (`agentAS_pub.log`, 186 probes, 184 hits) covers the classes `l_tower_p` has none of:

* `DisPossessionVolume_1/2` — `CollisionType 6`, both touched;
* `DishonoredWaterVolume` ×6 — `DishonoredWaterVolume_15` becomes the pawn's `PhysicsVolume`;
* 116 pickups (`DisAbstractItemPickup` ×81, `DisAbstractItemPickupNote` ×21, `DisStatPickup` ×14) — touched, e.g.
  `touching 4: DishonoredWaterVolume_15 DisAbstractItemPickup_14 DisAbstractItemPickup_9 DishonoredAudioVolume_50`;
* census after 115 s: `touch begin 418 end 354 (volume 346, trigger 8), volume entered 191 left 191, PhysicsVolume
  changes 191, SeqEvent_Touch fired 2`; 0 `Critical`.

Neither map contains a `DisStealthVolume` or a `DynamicPhysicsVolume` instance, so those two of the brief's list are
untested by observation; both are plain `AVolume` / `APhysicsVolume` subclasses (`script_classes_2013.json`) and go
through the identical `IsOverlapping` box path and `SetZone`, which `DisPossessionVolume` and `DishonoredWaterVolume`
exercise.

Plain-startup regression (`agentAS_plain.log`, no `-startmap`, null RHI): `Finished loading level`, `Initial startup`,
no `Critical`.

## Pickups: the hand-over to agent AU

The touch half works — pickups enter the pawn's `Touching` list on both maps. The pick-up itself cannot work, and it is
not a touch problem: **`DisPickup_Base` declares no script `Touch` event** (`script_classes_2013.json`: `OnToggleHidden`
and the native `BaseChange`, nothing else), so in retail a pickup is taken through the C++ focus / interact path — and
`DishonoredGame/Src/dispickup_base.cpp` is a 46-line file of comments with **none of its 42 attributed functions
ported**. In order of what blocks what:

| 2013 rva | function | why it blocks a pickup |
|---|---|---|
| 0x680a00 | `ADisPickup_Base::PostBeginPlay` | nothing sets the pickup up. `DisStatPickup_0` in `l_tower_p` loads with `bCollideActors 0`, its `StaticMeshComponent`'s `CollideActors 0` and an all-zero mask, so neither a touch nor a crosshair trace can reach it (the 116 pickups of `L_Pub_Day_P` are authored on and *are* reachable) |
| 0x66c900 | `ADisPickup_Base::ShouldTrace` | the crosshair / focus gate; without it the pickup is not selectable |
| 0x682c50 | `ADisPickup_Base::GetCrosshairStatus` | no "can pick up" prompt |
| 0x682ca0 | `ADisPickup_Base::AttemptInteract_Derived` | the interact itself |
| 0x66c840 / 0x680650 / 0x6800d0 | `ConsumePickup` / `Steal` / `StartPickupTravel` | the consume, the theft and the fly-to-hand |
| 0x6805c0 / 0x6705f0 | `Attach` / `Detach` | attaching to the pawn |
| 0x8529e0 | `UDishonoredInventory::ConsumeStatPickup(ADisStatPickup*, INT*)` | the inventory side (`dishonoredinventory.cpp` lists it, no body) |
| — | `ADisPickup_Base::execBaseChange` | the one pickup *script* native still a `DISHONORED_NATIVE_STUB` (`DishonoredGame/Src/DishonoredGameNativeStubs.cpp:1681`) |

Other DishonoredGame natives that this package's touches newly reach, all agent AU's:
`ADishonoredPlayerController::execCalcPlayerSwimAccelRate` (a water volume), `ADishonoredPawn::execOnTeleport_Native`
and `ADishonoredPlayerController::execOnTeleport_Native`, `ADishonoredPawn::execTakeDamage`,
`ADishonoredSpawner::execOnStartSpawn`, `ADishonoredPlayerPawn::execLanded_Native` and `execTakeFallingDamage_Native`.

## What changed, per file

| File | Change | Evidence |
|---|---|---|
| `Engine/Inc/UnLevel.h` (+7) | `TRACE_DisTouchOverlap = 0x200000` | 2013 0x189870, 0x24cf23, 0x6494f0 |
| `Engine/Inc/UnActorComponent.h` (+22) | `TracesForAnyMove`, `TracesForAllGameplay`, `TracesForAnyGameplay`, `ClearAllTrace` on agent AQ's `FDisPrimTraceMask` — nothing redefined | 2013 0x179a00, 0x17e260 |
| `Engine/Inc/EngineClasses.h` (+13/-12) | `ECollisionType` spelt as retail's script package declares it | `script_classes_2013.json` |
| `Engine/Src/PrimitiveComponent.cpp` (+7/-2) | `execSetTraceBlocking` writes the mask | 2013 0x12a230 |
| `Engine/Src/UnActor.cpp` (+415/-51) | the five overlap / touch gates, `SetDefaultCollisionType`, `SetCollisionFromCollisionType`, and the `-distouch` / `-distouchprobe` instrumentation | 2013 0x173920, 0x189870, 0x179a00, 0x17e260 |
| `Engine/Src/UnFracturedStaticMesh.cpp` (+2/-2) | the editor-only touching-type test uses retail's two touching values | `script_classes_2013.json` |
| `Engine/Src/UnLevAct.cpp` (+33) | the three volume counters in `AActor::SetZone` / `APawn::SetZone`. `DISHONORED(bringup)` only | — |
| `Engine/Src/UnPhysLevel.cpp` (+4) | one call to the census, next to agent AQ's `DishonoredWorldTrace()`. `DISHONORED(bringup)` only | — |

Four files are outside the list the package brief named, flagged here for the coordinator. None belongs to another
wave-5 package (`PHASE7.md`'s ownership table gives AR the D3D9 and texture units, AT `UnSequence.cpp` and
`UnLevel.cpp`, AU the DishonoredGame units, AV the anim units, AX the tools):

* `Engine/Inc/EngineClasses.h` — the `ECollisionType` rename. Unavoidable: the retail mapping cannot be written with
  names that mean the opposite of what the values do. 13 lines, no layout change.
* `Engine/Src/UnFracturedStaticMesh.cpp` — two editor-only lines that used the three removed names.
* `Engine/Src/UnLevAct.cpp` and `Engine/Src/UnPhysLevel.cpp` — agent AQ's files from wave 4, already merged and
  unowned this wave; `DISHONORED(bringup)` counters and one call, nothing else.

`Engine/Inc/UnLevel.h` is the one LF file in the set; its line endings were preserved (every other file is CRLF).

## Follow-ups outside this package

1. **`UWorld::MoveActor`'s `TRACE_DisTouchOverlap`** (2013 store at 0x24cf23). Retail initialises the encroachment
   pass's trace flags to `0x200000` alone; ours passes 0. `UnLevAct.cpp` is agent AQ's file and I only added counters
   to it, so this is left for the coordinator or whoever next owns that file. Consequence today: usable objects,
   defence towers, whale-oil receptacles and detection eyes hide their special component from `MoveActor`'s
   encroachment sweep, so they will not begin a touch from movement — `FindTouchingActors` (which does carry the flag)
   still finds them.
2. **Agent AQ's remaining dead-boolean readers** that this package did not need: `UnPhysic.cpp:1728`
   (`AProjectile::processHitWall`'s `bSwitchToZeroCollision`), `UnNavigationMesh.cpp:8553` (nav-mesh `bPathColliding`
   filter), `GameCrowd.cpp:1451` (the crowd agent's encroachment filter, a copy of `FindTouchingActors`'s loop with the
   same gate — it should get the same substitution). The render-only uses in `UnStaticMeshRender.cpp`,
   `UnBrushComponent.cpp` and `SpeedTreeComponent.cpp` only affect `SHOW_Collision*` debug drawing and can stay.
3. **`AActor::m_ActorTypeFlags`** (BYTE @266) is still never written, so `MatchesTraceFlags` keeps using agent AQ's
   `GetAPawn()` / `IsPlayerPawn()` equivalents.
4. **The pawn lands 1,200 units below the intro boat.** In every run the pawn spawns at Z=-215, falls, and comes to
   rest walking at Z≈-1,440 with a valid floor; agent AQ saw the same (`distrace 115.3s pawn at Z=-1418.098 physics 1`).
   It walks, so it is not this package's defect, but it means `-inputtest` alone can never reach a trigger — which is
   why `-distouchprobe` exists. Whoever owns the boat's collision next should look at it.
5. **`AActor::IsConsideredTransparent`** (0x16cee0) is now decompiled (`build/agentAS_decomp`): it is
   `(TraceFlags & TRACE_DisGameplay_VisionLOS) && (m_bOverrideDefaultVisionTransparency ? m_bTransparentForVisionChecks
   : CollisionComponent's StaticMesh's own flag)` (`AActor` @300 masks 0x40 and 0x20). It is the leading guard of
   `AActor::ShouldTrace` (0x16cf40) and is still unported; it can only reject more candidates.

## Files

Mine (8 source + 2 docs): `Engine/Inc/{UnLevel.h,UnActorComponent.h,EngineClasses.h}`,
`Engine/Src/{UnActor.cpp,PrimitiveComponent.cpp,UnFracturedStaticMesh.cpp,UnLevAct.cpp,UnPhysLevel.cpp}`, plus this
report and `agentAS_status.csv`.

Scratch (not repo tools): `build/agentAS_patch{1..6}.py`, `build/agentAS_patch3b.py`,
`build/agentAS_baselinepatch.py`, `build/agentAS_release.cmd`, `build/agentASbase_release.cmd`,
`build/agentAS_{find,vtab,imm,dis}.py` (headless idalib helpers beside `resources/tools/ida/decompile_funcs.py`:
name search, vtable slot dump, immediate-operand search, and a disassembly window — promote `agentAS_imm.py` to
`resources/tools/ida/` if anyone else wants to find who reads a flag), `build/agentAS_build{1..5}.log`,
`build/agentASbase_build1.log`, `build/agentAS_decomp/` (12 decompiles), snapshot `build/agentASbase_wt` +
`build/agentASbase` (no stage directory, no junction), IDA copy `resources/docs/idb/retail2013_agentAS.i64`. Logs in
the retail `DishonoredGame/Logs`: `agentAS_t1.log`, `agentAS_t2.log`, `agentAS_t3.log`, `agentAS_final.log`,
`agentAS_final2.log`, `agentAS_base.log`, `agentAS_pub.log`, `agentAS_plain.log`.

`resources/play.cmd` and `resources/build-release.cmd` were not touched. No commits, no `git add`.
