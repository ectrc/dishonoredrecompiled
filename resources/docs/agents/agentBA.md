# Agent BA report — the world in the right place (2026-09-27)

Package **BA** of wave 6 (`PHASE8.md`), the lead package and the user's defect: *"the map is disjointed,
nothing is in the right place other than terrain and some buildings"*. Build dir `build\agentBA_rel`
(Release, `set BUILD_DIR=build\agentBA_rel` + `resources\build-release.cmd`), built and run on the **shared
working tree** — five other packages were in flight (BB/BC in `External/GFx3`, BD in the post-process units,
BE in `GFxUI`, BF in the DishonoredGame attributes units); none of them touches a file of mine, the tree
compiled throughout and no snapshot was needed. IDA copies `resources\docs\idb\retail2013_agentBA.i64` and
`shipping2012_agentBA.i64`, headless `run.py` + `decompile_funcs.py` only, decompiles in
`build\agentBA\dec` (2012) and `build\agentBA\dec13` (2013). Patch scripts `build\agentBA_patch1.py`
(the census), `_patch2.py` (**the fix**), `_patch3.py` (the far-geometry counter). No commits, nothing
staged. Status rows: `agentBA_status.csv`.

## Result

**`AStaticMeshCollectionActor::UpdateComponentsInternal` (2013 rva `0x35f160`, 2012 rva `0x381800`,
`unstaticmesh.cpp:4045`) was missing from the tree, so every merged static mesh in every cooked map was
attached at the collection actor's own transform instead of its own.**

The cooker merges thousands of `StaticMeshActor`s per level into a handful of `AStaticMeshCollectionActor`s
(retail 608 bytes). The per-instance world matrix of each merged mesh does not live in the component's
`Translation`/`Rotation`/`Scale3D` — those stay at identity — it lives in `CachedParentToWorld`
(`UPrimitiveComponent` @336), which `AStaticMeshCollectionActor::Serialize` (2013 rva `0x3715a0`, already
ported) reads out of the package after the actor's own data. Retail then **overrides**
`UpdateComponentsInternal` to hand each `UStaticMeshComponent` that matrix:

```
v8 = (const FMatrix *)(v6 + 84);                     // component + 336 == CachedParentToWorld
UActorComponent::UpdateComponent(v6, GWorld->Scene, this, v8, 0);
```
— the ternary against `AActor::LocalToWorld()` for non-static-mesh components is the only other branch, and
`bCollisionUpdate` is ignored and passed on as `FALSE`. Without the override the base
`AActor::UpdateComponentsInternal` (2013 `0x17f940`) hands every component the collection actor's
`LocalToWorld()`, and `UActorComponent::ConditionalAttach` (2013 `0x17aaa0`) unconditionally calls
`SetParentToWorld` (2013 `0x12a090`), whose whole body is `CachedParentToWorld = ParentToWorld`. The loaded
matrix is overwritten before it is ever used. Every collection actor in a cooked Dishonored map sits at the
origin with an identity rotation, so **every merged prop collapsed onto (0,0,0)** while terrain, BSP and the
individually-placed large meshes — which are not merged — stayed exactly where they belong. That is the
user's sentence, word for word.

The composition itself was never at fault. `UPrimitiveComponent::SetTransformedToWorld` (2013 `0x12aaf0`,
958 bytes) matches retail exactly, including the order
`FScaleRotationTranslationMatrix(Scale * Scale3D, Rotation, Translation) * CachedParentToWorld`, the
`AbsoluteTranslation` row-3 zeroing, the `AbsoluteScale` normalise and the `AbsoluteRotation` axis-length
replacement, with masks `0x2000000/0x4000000/0x8000000` at @280 agreeing with the retail SDK. `AActor::LocalToWorld`
(2013 `0x16d7e0`) is the same matrix term for term. `UActorComponent::UpdateComponent` (2013 `0x17ab00`) and
`ConditionalUpdateTransform` (2013 `0x169750`) match. The input was wrong, not the arithmetic.

## 1. Measuring it first: the `-displace` placement census

Nothing was changed until the defect had a number. `-displace` (off by default, a `ParseParam` cached in a
function-local static; `DishonoredPlacementCensus` in `Engine/Src/UnLevel.cpp`, called once per frame from
`UWorld::UpdateLevelStreaming` in `UnWorld.cpp`) emits, every 10 s and once per world:

* the **layout line** — the reflected offset of `Translation`, `Rotation`, `Scale`, `Scale3D` and
  `CachedParentToWorld` on `UPrimitiveComponent` next to the C++ `STRUCT_OFFSET`, agent AQ's `disbits`
  pattern. All five agree (`400 / 412 / 424 / 428 / 336`) and `sizeof(UStaticMeshComponent)` is 576, so this
  was never a layout bug and the second question — *is the data there* — was the right one;
* per level in `GWorld->Levels`: the actor count, the bounding box of the **actor locations**, the number of
  primitive components, how many are attached, the bounding box of their **composed `LocalToWorld` origins**,
  how many of those origins sit within a unit of their owner's own location (**collapsed**), and the same
  three figures restricted to `AStaticMeshCollectionActor` components, plus how many carry an all-zero
  `CachedParentToWorld`;
* on the first pass, for the first two collections in each level: the actor's location, rotation, draw scale
  and draw scale 3D, and for four of its components the component's own `Translation`, `Rotation`, `Scale`,
  `Scale3D`, its `CachedParentToWorld` origin, its composed `LocalToWorld` origin and its three `Absolute`
  flags — which separates "the actor is misplaced" from "the component is misplaced" from "both are right";
* how many attached primitives land outside `HALF_WORLD_MAX` (with the first four named on the first
  pass, which in practice only reports for levels already visible when that pass runs).

`-disnosmcaxform` reverts the collection actor to the base implementation, so the before and the after come
off the **same exe** and the pair is reproducible (agent AR's precedent). Both switches cost nothing when off.

**The census answered the first question immediately**: it is not actors of one sub-level against another, and
not the actor transforms. Actor-location boxes are identical before and after and agree between the sub-levels
of a map. It is components within an actor, and only within collection actors.

## 2. Before and after

d3d9, windowed 1280x720, `-benchmark -fps=30` (a fixed time step — without it the falling pawn puts the camera
somewhere different every run and no two shots compare), `-apshot=120`, same exe, the only difference being
`-disnosmcaxform`.

### `L_Tower_P` — the first mission, exterior

| Measure | Before | After |
|---|---:|---:|
| attached primitives, whole world | 2,926 | 2,926 |
| **of them collapsed onto their owner** | **2,915** | **405** |
| `L_Tower_Env` collection components | 2,439 | 2,439 |
| **of them collapsed** | **2,439** | **29** |
| `L_Tower_Env` collection bounds | `(0 0 0)..(0 0 0)` span 0 | `(-438391 -612020 -10438)..(575870 371100 19075)` |
| `L_Tower_Block` collection components collapsed | 46 of 46 | 0 of 46 |
| `L_Tower_Script` collection components collapsed | 18 of 18 | 0 of 18 |
| actor-location box, whole world | `(-310555 -76746 -1800)..(57400 392881 11886)` | identical |

Detail line for one component, the same component in both runs (`l_tower_p` persistent level,
`StaticMeshCollectionActor.StaticMeshActor_SMC_623`, `T (0 0 0) R (0 0 0) Scale 1.000 Scale3D (4.0 8.0 0.0)`):

```
before: cached (0.0 0.0 0.0)          ltw (0.0 0.0 0.0)
after:  cached (11844.3 21864.0 2401.9) ltw (11844.3 21864.0 2401.9)
```
The component's own translation and rotation really are identity, so `LocalToWorld == CachedParentToWorld`:
the whole placement of a merged mesh is that one serialized matrix, and it was being thrown away.

Screenshots `build\agentBA\shots\before_tower_f120.png` / `after_tower_f120.png` (scene frame 120, same
camera): before, the boat deck sits in front of a featureless grey slab and an empty skyline; after, the
harbour wall, the gantries, the rigging, the lamp standards, the flags and the Dunwall skyline are all there.

### `L_Pub_Day_P` — the hub, interior

| Measure | Before | After |
|---|---:|---:|
| attached primitives, whole world | 5,423 | 5,423 |
| **of them collapsed onto their owner** | **5,417** | **798** |
| `L_Pub_Day_Env` collection components | 4,422 | 4,422 |
| **of them collapsed** | **4,422** | **31** |
| `L_Pub_Day_Env` collection bounds | `(0 0 0)..(0 0 0)` span 0 | `(-22179 -22706 -3073)..(23311 8630 6137)` |
| `L_Pub_Day_Env` actor-location box | `(-12273 -12760 -2940)..(8004 6055 3054)` | identical |
| `L_Pub_Day_Light` collection components collapsed | 179 of 179 | 7 of 179 |
| `L_Pub_Day_Audio` collection components collapsed | 30 of 30 | 0 of 30 |

Screenshots `before_pub_f120.png` / `after_pub_f120.png`: before, a heap of columns, crates and beams piled at
the origin in front of an empty sea — exactly the "disjointed" picture the user described; after, the Hound
Pits pub interior, complete, with the Golden Cat handbill on the wall and the windows in the walls.

**Per-level bounds now agree between related sub-levels**, which is the acceptance test: in both maps the
composed component origins of every sub-level fall in the same coordinate region as that level's actor
locations, whereas before the collection components of every sub-level were a single point at the origin while
the actors were spread over tens of thousands of units.

The residual `collapsed` counts (405 of 2,926 and 798 of 5,423) are ordinary single-component actors — lights,
nav points, audio emitters, triggers, pickups — whose one component genuinely sits at the actor's location.
Only 31 and 40 of them respectively are collection components, and those are merged meshes the cook really did
place at the collection's origin.

## 3. The fix

Three files, plus the census in two more.

| File | Change | 2013 rva |
|---|---|---|
| `Engine/Src/UnStaticMesh.cpp` (+34) | `AStaticMeshCollectionActor::UpdateComponentsInternal`, the port. Hands each `UStaticMeshComponent` its own `CachedParentToWorld`, everything else the actor transform, `bCollisionUpdate` passed on as `FALSE` exactly as retail does. Carries `-disnosmcaxform`, which falls back to `Super::` so the defect can be reproduced on a fixed exe | **0x35f160** |
| `Engine/Classes/StaticMeshCollectionActor.uc` (+7) | the `cpptext` declaration | — |
| `Engine/Inc/EngineClasses.h` (+7) | the same declaration on the generated class | — |
| `Engine/Src/UnLevel.cpp` (+205) | `-displace`, the census of section 1. `DISHONORED(bringup)` only | bring-up |
| `Engine/Src/UnWorld.cpp` (+3) | the once-per-frame call from `UWorld::UpdateLevelStreaming`. `DISHONORED(bringup)` only | bring-up |

`UnStaticMesh.cpp`, `StaticMeshCollectionActor.uc` and `EngineClasses.h` are outside the file list in
`PHASE8.md`, which named `UnLevel.cpp`, `UnLevAct.cpp`, `PrimitiveComponent.cpp`, `UnActorComponent.cpp`,
`UnActorComponent.h` and `UnWorld.cpp`. The defect turned out to live in the collection actor rather than in
the component transform path, and no other package of this wave owns those three files. `UnLevAct.cpp`,
`PrimitiveComponent.cpp` and `UnActorComponent.cpp` needed no change — they already match retail.

## 4. The candidates that were killed, with the measurement that killed each

1. **The component-to-world composition order.** Killed by decompiling
   `UPrimitiveComponent::SetTransformedToWorld` (2013 `0x12aaf0`): ours is the same matrix product in the same
   order with the same `Absolute*` handling. `AActor::LocalToWorld` (2013 `0x16d7e0`) likewise — retail caches
   the result in `CachedLocalToWorld` behind an epsilon compare and returns a reference where we return a
   value, which is a speed difference and nothing else.
2. **A layout or reflection bug on the transform members.** Killed by the census layout line: reflected and
   C++ offsets agree on all five members.
3. **A per-level offset applied on association.** Killed twice. `ULevelStreaming::Offset` /
   `OldOffset` work and are zero in these maps; `ULevelStreaming::LevelTransform` and
   `ULevel::AppliedLevelTransform` are both `DISHONORED_SHIM_STATIC` — storage-less, shared, zero-initialised
   — so `bTransformActors` in `UWorld::AddToWorld` is always `FALSE` and the transform branch never runs. That
   is **correct**, not a latent bug: retail's `ULevelStreaming` has no `LevelTransform` member at all (the
   retail SDK span is 56..160, and the member is reference-only), so retail never transforms a streamed level
   either. The census confirms it empirically: every sub-level's geometry lands in its own map's coordinate
   region with no offset missing. Agent AP's `DISHONORED_SHIM_STATIC` sweep did not cover the placement path;
   it does now, and this is the only shim on it.
4. **Actor locations lost or reset by `PostLoad` / `ConditionalUpdateComponents`.** Killed by the census: the
   per-level actor-location boxes are bit-identical before and after the fix and match the level's geometry.
5. **Other missing collection overrides.** A sweep of every retail override of `UpdateComponentsInternal`,
   `SetParentToWorld`, `SetTransformedToWorld`, `UpdateTransform` and `ClearComponents`
   (`resources/tools/ida/sym.py`) against the tree found exactly one missing: this one.
   `AStaticLightCollectionActor::UpdateComponentsInternal` (2013 `0x248ed0`) was already ported in
   `UnLight.cpp` and is the pattern this fix follows.

## 5. Accept

* **Screenshot pairs from the same camera, both maps**: `build\agentBA\shots\{before,after}_tower_f120.png`
  and `{before,after}_pub_f120.png`, same exe, same `-apshot=120 -benchmark -fps=30`, the "before" taken with
  `-disnosmcaxform`.
* **Placement census with per-level bounds that agree between related sub-levels**: section 2, and the raw
  lines in the retail `DishonoredGame/Logs`: `agentBA_before_tower.log`, `agentBA_after_tower.log`,
  `agentBA_before_pub.log`, `agentBA_after_pub.log`, `agentBA_far_tower.log`, `agentBA_far_pub.log`.
  Transcripts `build\agentBA\{before,after}_{tower,pub}.txt`, `far_{tower,pub}.txt`, all `EXIT=0` with
  `--forbid Critical` clean.
* **Regression harness green**: `python resources/tools/run_regression.py --build-dir build/agentBA_rel
  --no-build` -> **19 ok, 0 failed, 4 skipped, 421 s**
  (`build\agentBA_rel\regression\summary.txt`). `d3d9_draw_elements` 6,506 (HEAD 6,381),
  `d3d9_draws_per_frame` 1,167, `d3d9_visible_prims` 448, `inputtest_moved` 1,021.8, 0 criticals in all three
  stages.

## 6. What is outside this package

1. **A handful of merged meshes really do sit outside `HALF_WORLD_MAX`**: 11 of 2,926 attached primitives in
   `L_Tower_P` (9 in `L_Tower_Env`, 2 in `L_Tower_Script`) and 8 of 5,423 in `L_Pub_Day_P` (all in
   `L_Pub_Day_Light`), reaching about 600,000 units against UE3's 262,144. They are the distant Dunwall
   skyline and sky plates — one of them is a plane with `Scale3D (-3600, 6000, 6000)` at
   `(-39746, -241575, 149792)`. We compose the same matrix retail composes, byte for byte, so this is the
   cooked data and not a port defect; it is recorded because it is the only remaining anomaly the census
   reports and because a future world-bounds check would trip on it.
2. **The pawn still falls and the camera still ends up below the floor over a long run** (agent AP's
   follow-up 3, agent AQ's resting positions). That is why the screenshots are pinned to scene frame 120: the
   view is still on the boat there. Not a placement defect, and not this package.
3. **The lighting of the merged meshes is untested here.** `AStaticLightCollectionActor` was already correct,
   but nothing in this package checked that a lightmap follows its mesh now that the mesh moved. Package BD's
   before/after pair will show it.
4. **A regression check for this defect is one line and I did not add it**, because five other packages are
   running `run_regression.py` against the same file this wave and the accept bound is "still 19 checks". The
   check to add at merge: run the `d3d9` stage with `-displace` and require the `displace world` line's
   `collapsed` count to stay under ~1,000 for `L_Tower_P` (2,915 with the defect, 405 without).
   `-disnosmcaxform` makes it fail on demand, so the check can be proven to work.

## Follow-ups

1. Fold the check in point 6.4 into `run_regression.py`.
2. `-displace` and `-disnosmcaxform` are bring-up aids in the style of `-distrace` and `-distexfill`. They cost
   nothing when off and `-displace` is the fastest way to answer "why is this actor in the wrong place", so
   they are worth keeping until the world is trusted.
3. `AActor::LocalToWorld` returns by value where retail returns a cached reference behind an epsilon compare
   (2013 `0x16d7e0` writes `CachedLocation`, `CachedRotation`, `CachedDrawScale3D`, `CachedPrePivot`,
   `CachedDrawScale`, `CachedLocalToWorld`, all of which exist on our `AActor`). Porting the cache is a pure
   speed win on a function the component update calls once per actor per update; the arithmetic is already
   identical.
4. `UActorComponent::UpdateComponent` takes `const FMatrix&` where retail takes a nullable `const FMatrix*`
   whose `NULL` means "use the owner's transform, or identity if there is no owner". No caller in our tree can
   pass `NULL`, but a DishonoredGame caller ported later might expect to.
