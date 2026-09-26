# Agent AQ — the pawn stands on the level and walks: Arkane's collision-trace mask, not the spawn order (2026-09-26)

Package AQ: make the player pawn stand on `L_Tower_P` and walk, which completes PLAN.md Phase 6 milestone 5.
Target is the retail 2013 exe (`resources/docs/idb/retail2013_agentAQ.i64`, a copy of `retail2013_named.i64`); every
"2013 rva" below is from a headless decompile in `build/agentAQ_decomp`.

## Result

| Acceptance | State |
|---|---|
| the pawn stands on the level instead of falling | **done**. `physics 1` (PHYS_Walking), `base SkeletalMeshActorMAT_2`, floor normal `(-0.178, 0.499, 0.848)`; resting at `X=-3800.456 Y=36440.234 Z=-179.682` under the null RHI, `X=-3483.612 Y=35761.023 Z=-66.506` under d3d9 |
| `-inputtest` reports a non-zero moved distance | **done**: `inputtest moved 972.2` under d3d9 (`218.8` … `222.8` under the null RHI, where the pawn starts one tick earlier and W is released sooner), peak 2D speed 500.4, `physics 1` at the end instead of `physics 0` |
| it walks under `-startmap=L_Tower_P` with the sub-levels visible | **done**: all seven `LevelStreamingAlwaysLoaded` sub-levels report `bIsVisible 1` and sit in `GWorld->Levels` at indices 1..7, d3d9 windowed 1280x720, 120 s, **0 critical errors**. At 115 s the pawn is still walking (`physics 1`, `base WorldInfo_0`, velocity 500) |
| shape-creation counts before / after | **235 `URB_BodyInstance::InitBody : Could not create new Shape` -> 0**. After the fix the scene holds 895 PhysX actors, 1312 static shapes, 59 dynamic shapes, 413 convex meshes and 9 triangle meshes (was 2 actors / 1 static shape / 1 convex mesh in agent AL's one-shot line) |

**Which of the two reported findings mattered: neither.** The real cause is a third thing, and the two findings are
answered below with measurements.

## The cause: Arkane replaced `BlockZeroExtent` / `BlockNonZeroExtent` with a 7-bit trace mask

`UPrimitiveComponent::BlockZeroExtent` and `BlockNonZeroExtent` are **0 on every primitive in every cooked package**.
Measured with the new `-distrace` switch on the unmodified tree:

```
[0003.48] distrace 0.0s primitives 3425, attached 2928, CollideActors 1229,
          BlockNonZeroExtent 0, BlockZeroExtent 0;
          static meshes 1147 (kDOP 1147, no kDOP 0, 309728 collision triangles)
[0003.48] distrace 0.0s probe down zero: clear | extent 49x88: clear | point at anchor: clear
```

1,229 attached primitives have `CollideActors`, all 1,147 static meshes have a full kDOP tree with 309,728 collision
triangles — and not one primitive has either blocking bit, so `FOctreeNode::ActorZeroExtentLineCheck`,
`ActorNonZeroExtentLineCheck` and `ActorPointCheck` rejected every candidate before ever touching the kDOP. A downward
line check from the PlayerStart through 20,000 units of level came back `clear`. **The world had no collision at all.**

The per-component dump (`-distrace` prints it once per world) says why:

```
disbits property CollideActors        offset 280 mask 0x00000010 owner PrimitiveComponent
disbits property BlockActors          offset 280 mask 0x00000040 owner PrimitiveComponent
disbits property BlockZeroExtent      offset 280 mask 0x00000080 owner PrimitiveComponent
disbits property BlockNonZeroExtent   offset 280 mask 0x00000100 owner PrimitiveComponent
disbits property m_CollisionTraceTypes offset 320

disbits BrushComponent0 (BrushComponent) owner DynamicBlockingVolume_1:
        raw @280 0x10c08658  @320 0x00000007
disbits BrushComponent0 C++ reads CollideActors 1 AlwaysCheckCollision 0 BlockActors 1
        BlockZeroExtent 0 BlockNonZeroExtent 0 CanBlockCamera 1 BlockRigidBody 1;
        trace move 1/1/1 gameplay 0/0/0/0
```

The reflected masks and the C++ bitfields agree exactly (0x658 = 0x8|0x10|0x40|0x200|0x400), so this is **not** a layout
or mask bug: the package really stores those two booleans as FALSE, and stores the blocking information in
`FDisPrimTraceMask m_CollisionTraceTypes` at offset 320 instead — `0x7` for the blocking volume (all three *movement*
traces, no gameplay trace), `0x7f` for the player's cylinder.

Retail confirms it. `FOctreeNode::ActorZeroExtentLineCheck` (**2013 rva 0x2b12c0**) is

```c
if ( (ChkTraceFlags & 0x100) != 0 )          // TRACE_ShadowCast: CastShadow / HasStaticShadowing / AffectsPrimitive
    ...
else if ( !FDisPrimTraceMask::MatchesTraceFlags((FDisPrimTraceMask *)(Prim + 320), ChkActor, ChkTraceFlags)
       || !UPrimitiveComponent::ShouldCollide(Prim) )
    continue;
```

and `ActorNonZeroExtentLineCheck` (**0x2b1880**) and `ActorPointCheck` (**0x2b1fa0**) have the identical pair. The two
booleans are dead weight in retail: nothing reads them, which is exactly why no cooking pass ever set them.

### `FDisPrimTraceMask::MatchesTraceFlags` (2013 rva 0x129990, 2012 0x12dd30, bit-identical)

Read straight out of the disassembly (`build/agentAQ_decomp/dis_129990.txt`):

| TraceFlags tested | returns |
|---|---|
| `& 0x0F000000` (any gameplay trace), then `0x08000000` | `m_bTraceForGameplay_VisionLOS` (bit 6) |
| … `0x01000000` | `m_bTraceForGameplay_Crosshair` (bit 3) |
| … `0x04000000` | `m_bTraceForGameplay_Melee` (bit 5) |
| … `0x02000000` | `m_bTraceForGameplay_Projectile` (bit 4) |
| `& 0x10000000` | `m_bTraceForMove_NonPlayerPawn` (bit 1) |
| `& 0x20000000` | `m_bTraceForMove_Player` (bit 2) |
| no Arkane flag, source actor is not a pawn | `m_bTraceForMove_NonPawn` (bit 0) |
| no Arkane flag, a pawn that is not the player | `m_bTraceForMove_NonPlayerPawn` (bit 1) |
| no Arkane flag, the player pawn | `m_bTraceForMove_Player` (bit 2) |

The last two are spelled in retail with `AActor::m_ActorTypeFlags` (BYTE @266, `retail_sdk_layout.json`): `mov al,[edx+10Ah] / and al,20h`
is "is a pawn" and `cmp byte ptr [eax+10Ah], 24h` is "is the player pawn". Nothing in this tree ever *writes* that byte
(the only reader is `DishonoredGame/Src/dishonoredinventory.cpp:230`, `m_ActorTypeFlags != 36`), so the port uses the
equivalent engine predicates `AActor::GetAPawn()` and `APawn::IsPlayerPawn()`. That is the one deliberate deviation in
this package; porting `m_ActorTypeFlags` itself would retire it.

The six new trace flags land in `ETraceFlags` (`Engine/Inc/UnLevel.h`); the reference enum stops at `0x100000`, so the
high nibble retail uses was free.

## Why the two reported findings are not the cause

**1. Spawn order / level streaming — measured, and it is fine.** With `-distrace` printing every `ULevelStreaming`
entry per tick, `l_tower_p`'s eight entries already have `LoadedLevel` on the very first tick, and the seven
`LevelStreamingAlwaysLoaded` ones reach `bIsVisible 1` and enter `GWorld->Levels` within the first second:

```
[0004.67] distrace 0.0s   level L_Tower_Audio  ... bIsVisible 1 LoadedLevel 1 pendingVis 0 worldIndex 1 actors 44
[0004.67] distrace 0.0s   level L_Tower_Env    ... bIsVisible 0 LoadedLevel 1 pendingVis 1 worldIndex 3 actors 102
...
[0007.69] (all seven)     ... bIsVisible 1 ... worldIndex 1..7
```

The `bShouldBeLoaded 0 bShouldBeVisible 0` that agents AF and AL reported is a red herring: those two *members* are
only advisory (they are what `ULevelStreamingKismet` and the replication path read). For an always-loaded level the
decision is taken by the virtuals, and `ULevelStreamingAlwaysLoaded::ShouldBeLoaded` returns TRUE unconditionally (only
an `AutoTestManager` makes it consult `bShouldBeLoaded`), with `ULevelStreaming::ShouldBeVisible` returning
`ShouldBeLoaded()` in game. `UWorld::UpdateLevelStreaming` uses the virtuals, not the members, so the flushes in
`LoadMap` are not the problem and **no change to `LoadMap` or to the streaming order was needed**. `L_Tower_Water` is a
`LevelStreamingKismet` and correctly stays unloaded until its Kismet action fires.

What made the earlier runs *look* like a spawn-order problem is that the pawn falls at the same time the sub-levels
arrive: with no collision anywhere it falls through the floor the instant the floor exists, reaches `l_tower_p`'s
`KillZ` of **-5000** (the menu map's is -240000, which is why the menu pawn never froze), and `Pawn.FellOutOfWorld`
sets `PHYS_None` — terminal, and the pawn is not destroyed because `Pawn.Died` is still a warn-once stub.

**2. The 619 log lines about collision shapes — a real but separate defect, now also fixed.** They are 235 pairs of
`FKAggregateGeom::InstanceNovodexGeom … Cannot 3D-Scale rigid-body primitives (sphere, box, sphyl).` +
`URB_BodyInstance::InitBody : Could not create new Shape`, 470 lines in total (the brief's 619 counted a longer run).
They come from `Engine/Src/UnPhysCollision.cpp:858`, not `UnPhysAsset.cpp`, and they are not the reference's guard
firing legitimately: **retail has no such guard and no such string.** `FKAggregateGeom::InstanceNovodexGeom`
(**2013 rva 0x3c8330**) begins

```c
if ( fabs(pScale.X - pScale.Y) >= 1e-4 || fabs(pScale.Y - pScale.Z) >= 1e-4 )   // i.e. !IsUniform()
  for ( i = 0; i < BoxElems.Num(); i++ )
    if ( (BoxElem->bNoRBCollision) == 0 && BoxElem->TM.IsUnrotatedAndUnscaled(1e-4) )
      { dimensions = 0.5f * Dim * fabs(pScale.Axis) + 0.025f;  ... shapes.pushBack(BoxDesc); }
else
  { sphere loop; box loop; sphyl loop; }                                         // the reference's uniform path
```

so Arkane *added* 3D-scaled box support (boxes only, and only when the element transform is unrotated). Every one of the
235 failures is a non-uniformly scaled `Env_Blockout.collisions.Collision_model` instance. Ported, together with the
`FMatrix::IsUnrotatedAndUnscaled` it needs (**2013 rva 0x3a77b0**). Agent AL's reading was right that PhysX is not on the
`PHYS_Walking` path — `Engine/Src/UnPhysic.cpp` has no `WITH_NOVODEX` at all and the kDOP trees were always intact — so
this fix changes rigid bodies, ragdolls and physics props, not walking.

## What changed, per file

| File | Change | Evidence |
|---|---|---|
| `Engine/Inc/UnLevel.h` (+13) | six `TRACE_DisGameplay_*` / `TRACE_DisMove_*` flags and the `TRACE_DisGameplay` combination | 2013 0x129990's flag tests |
| `Engine/Inc/UnActorComponent.h` (+23) | `FDisPrimTraceMask::MatchesTraceFlags` declaration + inline `SetAllMovementTrace`, `SetAllGameplayTrace`, `TracesForAllMove` | 2013 0x129990, 0x129000, 0x129030, 0x166310 |
| `Engine/Src/UnLevAct.cpp` (+49/-2) | `FDisPrimTraceMask::MatchesTraceFlags` body; both `CheckEncroachment` blocking filters | 2013 0x129990, 0x243e40 |
| `Engine/Src/UnOctree.cpp` (+19/-6) | the three octree gates; `ActorPointCheck` clears `ChkActor` | 2013 0x2b12c0, 0x2b1880, 0x2b1fa0, 0x2b3760 |
| `Core/Inc/UnMath.h` (+10) | `FMatrix::IsUnrotatedAndUnscaled(FLOAT)` | 2013 0x3a77b0 |
| `Engine/Src/UnPhysCollision.cpp` (+29/-8) | the non-uniform-scale box path in `InstanceNovodexGeom`; the "Cannot 3D-Scale" message is gone | 2013 0x3c8330 |
| `Engine/Src/UnPhysLevel.cpp` (+215) | **`-distrace`**, the measurement (below). `DISHONORED(bringup)` only | — |

Two files are outside the list the package brief suggested and are flagged here for the coordinator: `UnOctree.cpp`
(where the collision filters actually live — the brief's `UnPhysAsset.cpp` / `UnPhysic.cpp` have none) and
`UnPhysCollision.cpp` (where `InstanceNovodexGeom` lives, not `UnPhysAsset.cpp`). Neither is in agent AP's rendering
set. `Core/Inc/UnMath.h` gains one inline member function and nothing else.

## `-distrace`, the instrumentation

Added to `UWorld::TickWorldRBPhys` (`Engine/Src/UnPhysLevel.cpp`, next to agent AL's PhysX scene summary), re-armed for
each new world so the mission map is measured and not the menu. Once per second it logs, as `DISHONORED(bringup)` lines:

* the pawn: location, velocity, `Physics`, `Base`, `Floor`, `bCollideActors/bCollideWorld/bBlockActors`, cylinder extent;
* one line per `ULevelStreaming`: `bShouldBeLoaded`, `bShouldBeVisible`, `bShouldBlockOnLoad`, `bIsVisible`,
  `LoadedLevel`, `bHasVisibilityRequestPending`, its index in `GWorld->Levels`, its actor count;
* three collision probes at the spawn anchor: a zero-extent downward line check over 20,000 units, the same with the
  pawn's cylinder extent, and a point check — which is what separates "no floor" from "a floor that does not collide";
* the PhysX scene and SDK counters (actors, static/dynamic shapes, convex and triangle meshes);
* the world's collision inventory: primitives, attached, `CollideActors`, `BlockNonZeroExtent`, `BlockZeroExtent`,
  `TraceForMove_Player`, `TraceForMove_any`, `TraceForGameplay_any`, and how many static meshes carry a non-empty kDOP
  tree with how many collision triangles;
* once per world, the `disbits` dump: the reflected offset and bit mask of each collision bool, the raw DWORDs at 272,
  276, 280, 284 and 320 of the first four collidable primitives, the values the C++ bitfields read back, and the same for
  the class default object. That is the dump that identified the mask as correct and the data as empty.

It is off by default and costs nothing when off. Worth keeping: it is the fastest way to answer "why is this actor not
colliding".

## Runs

Baseline, unmodified tree (`agentAQ_base.log`, Release, null RHI):

```
[0057.60] possessed DishonoredPlayerPawn in l_tower_p at X=-3900.598 Y=36639.262 Z=-215.850
[0063.97] inputtest waiting for ground: 8.0s, physics 2, pawn at Z=-5330.034, floor none
[0066.06] inputtest waiting for ground: 10.0s, physics 0, pawn at Z=-5330.034, floor none
[0089.36] inputtest moved 0.0 turned 21060 (peak 2D speed 0.0, peak 2D accel 3303.0, physics 0)
          235 x "URB_BodyInstance::InitBody : Could not create new Shape"
```

After (`agentAQ_final.log`, Release snapshot, **d3d9 windowed 1280x720**, scene rendering on, 120 s, `--forbid Critical` satisfied):

```
python resources\tools\build_and_smoke.py --build-dir build/agentAQ_rel --no-build --exe-name DishonoredGame_AQ.exe ^
  --log-name agentAQ_final.log --ini-dir build/agentAQ/config --rhi d3d9 --timeout 120 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "inputtest moved" --forbid "Critical" ^
  --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -inputtest -distrace -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
```

```
[0004.38] distrace 0.0s PhysX 864 actors, 1297 static shapes, 2 dynamic shapes; SDK 406 convex meshes, 9 triangle meshes
[0004.69] inputtest start: pawn at X=-3900.598 Y=36639.262 Z=-380.650, physics 1, bindings 172
[0007.69] inputtest moved 972.2 turned -28096 (peak 2D speed 500.4, peak 2D accel 3303.0, physics 1,
          pawn at X=-3483.612 Y=35761.023 Z=-66.506)
[0119.65] distrace 115.3s pawn at X=-4324.041 Y=36339.559 Z=-1418.098 vel (215.6, 450.8, 0.0)
          physics 1 base WorldInfo_0 floor (0.013, -0.017, 1.000)
[0119.65] distrace 115.3s level L_Tower_{Audio,Block,Env,Fx,Light,Nav,Script} ... bIsVisible 1 ... worldIndex 1..7
[0119.65] distrace 115.3s PhysX 895 actors, 1312 static shapes, 59 dynamic shapes; SDK 413 convex meshes, 9 triangle meshes
          0 x "Could not create new Shape"
```

Null-RHI run for the same thing (`agentAQ_t5.log`): `inputtest moved 218.8`, `physics 1`, `base SkeletalMeshActorMAT_2`,
0 shape failures, exit 0. Plain-startup regression check (`agentAQ_base2.log`, no `-startmap`):
`Finished loading level`, `Initial startup`, no `Critical`.

Standing on `SkeletalMeshActorMAT_2` is correct for this map: `L_Tower_P` is the arrival-by-boat intro, and the pawn
starts on the boat before stepping onto the BSP (`base WorldInfo_0` later in the run).

## Which build the numbers come from

Both a snapshot and the shared tree, and they agree.

* **Snapshot** (used while agent AP's in-flight edit broke the shared tree with
  `SceneRendering.cpp(4763): error C2065: 'GScreenShotRequest'`):
  `python resources/tools/make_snapshot.py AQ <my 7 files>` = HEAD `ac079ad` + only agent AQ's files, Release-built with
  `build/agentAQ_wt_release.cmd` into `build/agentAQ_rel` — 805 units, **0 errors, 0 link errors**
  (`build/agentAQ_build6..8.log`). `agentAQ_final.log` and `agentAQ_t1..t5.log` come from it.
* **Shared working tree**, once AP's edits had cleared (`git status` then showed only agent AQ's seven files):
  `set BUILD_DIR=build\agentAQ_shared` + `resources\build-release.cmd` — 805 units, **0 errors, 0 link errors**
  (`build/agentAQ_build9.log`), and the same accept line gives `inputtest moved 1006.2`, `physics 1`,
  `base WorldInfo_0`, `PhysX 895 actors, 1312 static shapes, 59 dynamic shapes; SDK 413 convex meshes, 9 triangle
  meshes`, **0** shape failures, no `Critical` over 90 s of d3d9 (`agentAQ_shared.log`).

`build/agentAQ` holds the first shared-tree Release build, used for the baseline and the two instrumentation-only runs.
`resources/play.cmd` and `resources/build-release.cmd` were not touched.

## What is left (follow-ups outside this package)

1. **`UPrimitiveComponent::execSetTraceBlocking`** (`Engine/Src/PrimitiveComponent.cpp:1288`, agent AP's file this
   wave). Retail (**2013 rva 0x12a230**) writes the mask, not the booleans; the whole body reduces to
   ```cpp
   m_CollisionTraceTypes.SetAllMovementTrace(NewBlockNonZeroExtent);
   m_CollisionTraceTypes.SetAllGameplayTrace(NewBlockZeroExtent);
   ```
   (the decompile's masked store preserves bits 7+, which the two setters also do). Until then, any script call to
   `SetTraceBlocking` silently does nothing.
2. **The rest of the `BlockZeroExtent` / `BlockNonZeroExtent` readers.** They are all dead tests now. The ones that
   matter for gameplay, with the retail counterpart where it is known:
   * `Engine/Src/UnActor.cpp:2783/2793` and `2885/2899` — `AActor::IsOverlapping` / the box-overlap helper use
     `BlockNonZeroExtent` as "is blocking", so **touch and encroachment notifications never fire**: triggers and volumes
     are inert. Same substitution as the octree gates.
   * `Engine/Src/UnActor.cpp:2431` — `AActor::FindTouchingActors`.
   * `Engine/Src/UnActor.cpp:2059..2217` — `AActor::SetDefaultCollisionType` / `SetCollisionType` run the mapping the
     wrong way round: retail's `SetDefaultCollisionType` (**0x179a00**, decompile in `build/agentAQ_decomp`) derives
     `CollisionType` (@263) *from* `m_CollisionTraceTypes` via `TracesForAllMove()` and the gameplay-bit combinations,
     while ours derives the two booleans from `CollisionType`.
   * `Engine/Src/UnPhysic.cpp:1728` — `AProjectile::processHitWall`'s `bSwitchToZeroCollision` test; needs
     `TRACE_DisGameplay_Projectile` semantics, which I did not decompile.
   * `Engine/Src/UnNavigationMesh.cpp:8553` — nav-mesh generation's `bPathColliding` filter.
   * `GameFramework/Src/GameCrowd.cpp:1451` — the crowd agent's encroachment filter.
   * Render-only uses (`UnStaticMeshRender.cpp`, `UnBrushComponent.cpp`, `SpeedTreeComponent.cpp`) only affect
     `SHOW_Collision*` debug drawing and can stay.
   A tempting bring-up bridge is to *derive* the two booleans from the mask when a primitive is attached (retail never
   reads them, so it cannot regress retail behaviour) and leave the reference sites alone; the catch is
   `UnActor.cpp:1774`, which compares them against the class default to update the mask, so the bridge would have to be
   applied after that. Porting the sites is cleaner.
3. **`AActor::m_ActorTypeFlags`** (BYTE @266) is never written; once it is, `MatchesTraceFlags` should use it rather
   than `GetAPawn()` / `IsPlayerPawn()`.
4. **`RepresentConvexAsBox`** (**0x2c1ff0**), retail's uniform-scale convex-to-box shortcut inside
   `InstanceNovodexGeom`, is not ported. It is an optimisation.
5. **`AActor::IsConsideredTransparent`**, the leading guard of retail's `AActor::ShouldTrace` (**0x16cf40**), is not
   ported. It can only reject more candidates, so nothing depends on it yet.
6. Agent AF's **B2** (the retail `-startmap` route without `-startmapopen` still stops after
   `Committed map change via DishonoredEngine`) is untouched; `-startmapopen` is still how a mission map is reached.

## Coordination with agent AP

Nothing in this package touches `SceneRendering`, `BasePassRendering`, `MaterialShared`, `MaterialInstance`,
`PrimitiveComponent.cpp`, `UnStaticMesh`, `UnModelComponent` or `Scene`. Two notes for AP:

* the one-line `execSetTraceBlocking` fix above lives in `PrimitiveComponent.cpp`, AP's file — either AP applies it or
  the coordinator does after the merge;
* `-distrace`'s inventory line is also a quick renderer sanity check: it reports how many primitives are attached
  (2,928 of 3,425) with their kDOP triangle totals, which bounds how much of the level the renderer could be drawing.

## Files

Mine (7 source + 2 docs): `Core/Inc/UnMath.h`, `Engine/Inc/{UnLevel.h,UnActorComponent.h}`,
`Engine/Src/{UnLevAct.cpp,UnOctree.cpp,UnPhysCollision.cpp,UnPhysLevel.cpp}`, plus this report and
`agentAQ_status.csv`.

Scratch (not repo tools): `build/agentAQ_patch{1..6}.py`, `build/agentAQ_fixpatch{,3}.py` (the patch scripts, written
with the Write tool because the Bash heredoc eats backslashes), `build/agentAQ_dis.py` (a headless disassembly dump
beside `resources/tools/ida/decompile_funcs.py` — promote it to `resources/tools/ida/` if anyone else wants it),
`build/agentAQ_wt_release.cmd`, `build/agentAQ_patchdoc.py`, `build/agentAQ_build{1..9}.log`, `build/agentAQ_decomp/` (12 decompiles + the
`MatchesTraceFlags` disassembly), snapshot `build/agentAQ_wt` + `build/agentAQ_rel` (no stage directory, no junction),
IDA copy `resources/docs/idb/retail2013_agentAQ.i64`. Logs in the retail `DishonoredGame/Logs`:
`agentAQ_base.log`, `agentAQ_t1..t5.log`, `agentAQ_final.log`, `agentAQ_shared.log`, `agentAQ_base2.log`.

No commits, no `git add`.
