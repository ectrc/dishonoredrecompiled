# Agent FL — the heads at the sky, and the 396 AnimNotify errors

Wave 19 (`PHASE15.md`), package FL. Worktree `build/agentFL_wt`, base `17a7c32`, **rebased onto `98e22d4`**
mid-package at the coordinator's instruction. Four changed source files plus this document and
`agentFL_status.csv`; no new source file. Three of the four are in `DishonoredGame`, one in `Engine`.
**Nothing in `Engine/Src/UnInterpolation.cpp`, the `UInterpTrack` family or `External/GFx3`** — FK and FJ
own those and this package opened neither.

## 1. The short version

**Both halves of fault 4 are answered, and the answer to each is not the one the brief expected.**

| the brief said | what is true |
|---|---|
| "`FArkComponentLookat` has fifty-one functions and not one is written" — implying the absent component is why the heads are up | **It is not.** The heads were up because **`UArkAnimNodeLookAt` had no `InitAnim` and no `GetBoneAtoms`**, two functions on a *different* class. `FArkComponentLookat` is still 100 % absent after this package and the heads are level. The component is what puts a NON-ZERO aim on the node; an idle NPC in retail has none, and retail's rest state is all eighteen pose weights at **zero** |
| "396 `Found bad AnimNotify` errors … an off-by-one in ownership … something in our loader attaches notifies to the wrong `AnimSequence`" | **Not our loader.** Measured: 5,637 AnimNotify objects load, 396 (7.0 %) arrive outered to another `AnimSequence`, **396 of 396 are repaired** by stock UE3's `Rename` in `UAnimSequence::PostLoad`. The linker's own export table is what says so — the cooked file's `OuterIndex` for each bad notify points at the other sequence, and we resolve it exactly as written. It is an **asset artefact**: several of these notify objects are **shared by two sequences**, and a shared object can only be outered to one of them |
| "`USkelControlLookAt` itself IS implemented, so the bone controller runs every frame with nothing setting its target" | **There is no look-at skel control in this anim tree.** `NPC_Human_at` has five `SkelControlLists`: `spine_0_jnt:DisSkelControl_SpineBender`, `eye_L_jnt`/`eye_R_jnt:SkelControlSingleBone`, `foot_R_jnt`/`foot_L_jnt:DisSkelControl_FootPlacement`. The head look-at is done entirely by the additive blend node |
| "`FArkComponentLookat::OnComposeSkeleton` would override exactly this bone [`head_jnt`]" (agent DO's note in `dishonoredplayercontroller.cpp:1522`) | **It is the eyes, not the head.** `OnComposeSkeleton` (2013 rva `0x552640`) loops twice over a pair of bone indices and clamps against `MIN_EYE_YAW` / `MAX_EYE_YAW` / `MIN_EYE_PITCH` / `MAX_EYE_PITCH`. The head is driven by `UpdateHeadAndTorso` → `UArkAnimNodeLookAt::SetAim`, which is the node, not the skeleton |
| "a standing guard also exists in `l_tower_p` reachable with `-startmap=l_tower_p -startmapopen` … that may be a cheaper control than the full cutscene" | **It would have measured the wrong pawn.** `-dislookat` already reported *a* guard and reported it as healthy (`head_jnt` 2° from bind). The pawn it picks is the first `ADishonoredNPCPawn` with both meshes, which in `l_tower_p` is one of the twenty-odd nobody is looking at. The two the user sees are the two the opening scene **spawns**, and they are the last two by iteration order. The measurement had to be camera-relative |

**The headline frames** are `build/agentFL/shots/fl_heads_three_way.png` (before / after / retail, cropped
to the two heads) and `build/agentFL/shots/fl_heads_before_after.png` (the full 1600x900 pair).

## 2. The measurement that decided it

`-dislookat` could not answer the question, so this package added **`-flheaddiag`**, which reports the four
NPC pawns **nearest the camera** and, per bone, **two** deltas rather than one:

* `l` — `LocalAtoms(i)` against `RefSkeleton(i).BonePos.Orientation`, in degrees. **The animation tree's
  own output.**
* `c` — `SpaceBases(i)` against the reference pose composed down the same chain, in degrees. **What the
  renderer uses**, so it includes the skel controllers and `ComposeSkeleton`.

A bone with a large `c` and a zero `l` was moved after the tree. A bone with a large `l` was moved by the
tree. That one distinction is the whole diagnosis.

**Before** (`98e22d4` + the diagnostics only, `FL1.log:2905`, `DishonoredNPCPawn_27`, 249 units from the
camera, `lastRender` == now):

```
root0_jnt l0/c0; Root_jnt l7/c7; spine_0_jnt l7/c1; spine_1_jnt l0/c1; spine_2_jnt l1/c2;
spine_3_jnt l0/c2; Torso_jnt l0/c2; neck_jnt l34/c32; head_jnt l60/c84; head_end_jnt l0/c84;
camera_jnt l0/c84; jaw_jnt l0/c84;
lookatnode LookatBlender w1.00 aim(0.00,0.00) config NULL | nonzero: 4:ADD_Empty_AimHeadIdle_Center=1.00
head Skm_EliteGuard_Head: ... root0_jnt l0/c0; ... neck_jnt l0/c0; head_jnt l0/c0;   (every bone 0/0)
```

**84 degrees of head, 34 of neck, and the entire spine inside 2.** The head mesh's own bones are all
0/0 and the renderer does not use them anyway (`ParentBoneMap` 118 of 118). So it is the tree, at exactly
two bones, and the only thing in that tree that touches exactly those two bones is `LookatBlender`.

**After** (same pawn, same driver, same `-apshottime` mark, `FL3.log`):

```
... spine_3_jnt l0/c3; Torso_jnt l0/c3; neck_jnt l2/c4; head_jnt l4/c0; head_end_jnt l0/c0;
camera_jnt l0/c0; jaw_jnt l0/c0;
lookatnode LookatBlender w1.00 aim(0.00,0.00) config NULL | nonzero: all zero
```

**`head_jnt` 84° → 0°.** `DishonoredNPCPawn_26` moves identically. The final tree, measured again after
the three `IsValidIndex` guards of §4 went in (`FL5.log`), reads `neck_jnt l0/c1; head_jnt l1/c0` — the
1–4° spread across the three after-runs is the idle animation's own breathing, not the look-at.

### The A/B that killed the obvious explanation

Agent DO's stand-in writes the grid **centre** pose at weight 1.00 every tick; its comment in
`Inc/CppText/UArkAnimNodeLookAt.h` says "at (0,0) it is exactly the Center pose — which is what retail's
own rest state is". Run as a pair:

| | head grid | `neck_jnt` | `head_jnt` |
|---|---|---|---|
| `-nodislookataim` (the cook's own weights) | `0:ADD_Empty_AimHeadIdle_UpLeft=1.00` | l34 / c32 | **l62 / c84** |
| default (DO's stand-in, centre) | `4:ADD_Empty_AimHeadIdle_Center=1.00` | l34 / c32 | **l60 / c84** |

**The centre of the aim grid is a pose, not a rest state.** It is worth as much head rotation as the
corner the cook left behind. Correcting *which* cell carries the weight cannot fix this; the weight itself
has to go.

## 3. What retail does, and why it needed two functions and not one

Three functions of `UArkAnimNodeLookAt`, read out of the 2013 binary:

* **`InitAnim`** (2013 rva `0x5138d0`, 189 bytes) — its first act after `Super::InitAnim` is
  `for(i=0;i<Anims.Num();i++) Anims(i).Weight = 0;`. The cook's defaults never survive into play. It then
  constructs `m_LookAtModeBlendingNode` in the transient package.
* **`TickAnim`** (2013 rva `0x526320`, 111 bytes) — the **whole body**, `UAnimNodeSequence::TickAnim`
  included, is behind `m_pConfig && m_pConfig->m_LookAtModes.Num()` (retail tests the `Num` word at
  `UArkComponentLookatConfig+96`; `m_LookAtModes` is at +92 in `retail_sdk_layout.json`, so the two agree).
  `m_pConfig` is written by `FArkComponentLookat::Starting`, which is unported — measured as **`config
  NULL` on every pawn in every run**. So nothing writes a weight back and the zeroes stand.
* **`GetBoneAtoms`** (indexed, 2013 rva `0x51b420`, 2,141 bytes) — when a set's total weight is at or
  below `ZERO_ANIMWEIGHT_THRESH` it fills every atom with **`FBoneAtom::Identity`**.

That third one is why **`InitAnim` alone would have made the pose worse.** Stock
`UAnimNodeSequenceBlendBase::GetBoneAtoms` (`UnAnimPlay.cpp:2201`) reacts to weights that do not sum to 1
by `check(FALSE)` and `FillWithRefPose`, and a **reference pose applied as an additive delta is not the
identity**. The two land together or not at all. This is the single most useful thing in this report for
the next package: *zeroing an additive node's weights is only safe once its `GetBoneAtoms` knows that zero
means identity.*

## 4. What was ported

All in `DishonoredGame/Src/dishonoredplayercontroller.cpp` under `agentFL:lookatnode`, replacing agent DO's
`agentDO:lookataim` stand-in, and declared in `Inc/CppText/UArkAnimNodeLookAt.h`. They live in
`dishonoredplayercontroller.cpp` rather than in `Engine/Src/arkanimnodelookat.cpp` for DO's reason —
`UArkAnimNodeLookAt` is declared in `DishonoredGameEngineShims.h` and `Engine` cannot see it — and for a
second one: a unit whose only link reference is `IMPLEMENT_CLASS`'s folded vtable COMDAT can be dropped
whole (agent FI), and these are virtuals that must dispatch. `dishonoredplayercontroller.cpp` is always
linked and DO's stand-in demonstrably dispatched from it.

| function | 2013 rva | 2013 size | 2012 rva | 2012 size | state |
|---|---|---|---|---|---|
| `InitAnim` | `0x5138d0` | 189 | `0x567330` | 189 | **ported, exact** |
| `TickAnim` | `0x526320` | 111 | `0x5672c0` | 111 | **ported**, with its config branch named rather than filled (§6) |
| `GetBoneAtoms(Atoms,…)` | `0x51bc80` | 3,213 | `0x55c310` | 3,258 | **ported**, `m_fLookAtModeBlendingRemainingTime <= 0` branch only (§6) |
| `GetBoneAtoms(INT,Atoms,…)` | `0x51b420` | 2,141 | `0x55bab0` | 2,131 | **ported, all three branches** |
| `GetAnimInfoTotalWeight(INT)` | `0x513990` | 124 | `0x553fc0` | 124 | **ported, exact** |
| `UpdateNodeWeights` | `0x512bf0` | 3,294 | `0x5532e0` | 3,294 | **ported, exact** (replaces DO's stand-in) |
| `SetAim` | `0x50a0b0` | 62 | `0x54a940` | 62 | **ported, exact** |
| `AutoInitConfigInAnimTreeEd` | `0x50a720` | 200 | `0x54afb0` | 200 | **ported**: the `GIsEditor && !GIsGame` guard; the editor half is named, not written (§6) |

**One deliberate divergence.** Retail declares the indexed `GetBoneAtoms(INT, …)` **virtual**; here it is a
private non-virtual `GetBoneAtomsForSet`. A second virtual `GetBoneAtoms` overload that overrides no base
virtual is **C4263**, which this tree treats as an error. Nothing in the 2012 PDB overrides the indexed
slot — it is the only symbol of that signature in the whole binary — and its only caller is the four-argument
overload in the same class, so the vtable slot is unobservable. `using
UAnimNodeSequenceBlendBase::GetAnimInfoTotalWeight;` keeps the base's zero-argument overload visible beside
the new `(INT)` one.

### `UpdateNodeWeights`: the grid, read off the decompile

Two sets of nine; `index = column*3 + row`, column `1 + sign(X)`, row `1 - sign(Y)`; the four cells the aim
falls between take the bilinear weights and the other five are zeroed. Index 4 is the centre, index 0
`UpLeft`, index 8 `DownRight` — which is exactly the order of the names in the cooked array, an independent
confirmation. Agent DO's stand-in had arrived at the same grid by inspection; this is retail's own, branch
by branch, including that retail writes the four cells before it zeroes the other five.

### Two measurements that validate the ported branches

`-flaimx=<f> -flaimy=<f>` (new, this package) pins the head grid so the two branches of
`GetBoneAtomsForSet` can be compared on one pose. (It is two switches, not one: UE3's `Parse(FString&)`
stops on a comma.)

| aim | weights | branch | `head_jnt` |
|---|---|---|---|
| `-1, 1` | `0:UpLeft=1.00` | **shortcut** (one cell at 1.0) | l2 / c2 |
| `-0.99, 0.99` | `0:UpLeft=0.98 1:CenterLeft=0.01 3:Up=0.01` | **accumulate** (three cells) | l2 / c2 |

The two agree to the measurement's resolution, so the weighted accumulation is not installed untested.
`-dislookatplayer` (agent DO's, rewired through retail's `SetAim` + `UpdateNodeWeights`) gives
`1:CenterLeft=0.83 2:DownLeft=0.17 13:AimTorsoIdle_Center=1.00` for an aim of (-1.00, -0.17) — the
bilinear mapping, with each set summing to 1.

**A caveat I will not hide**: in all three of those the head moves only 2°, where a single head-grid cell at
weight 1.00 moved it 84° *before* the override existed. The explanation is that the head grid and the torso
grid are deltas from a **shared** base and are meant to be driven together — retail's
`ComputeHeadAndTorsoAim` produces both aims from one look-at direction — whereas `-flaimx/-flaimy` drives
the head grid alone with the torso pinned at its centre, which is a combination retail never produces. So
these runs prove the two branches agree with each other; they do **not** prove the node turns a head by the
right amount, and nothing can until `FArkComponentLookat` exists to drive both grids. Said plainly: this
package makes the rest pose right and leaves the moving pose unmeasured.

## 5. Fault 4(b): the AnimNotify ownership is the asset's, and it is already repaired

`-flnotify` instruments the stock fix-up loop in `UAnimSequence::PostLoad`
(`Engine/Src/UnSkeletalAnim.cpp`, byte-identical to the reference engine's `UnSkeletalAnim.cpp:425`) with
three counters and, for each bad notify, the **linker's own answer**.

```
flnotify totals: 5637 AnimNotify objects loaded, 396 arrived outered to another AnimSequence,
                 396 repaired by PostLoad

flnotify Npc_SmallRat_as...:AnimSequence_4[0]  AnimNotify_AkEvent_0:
         notify export 16799, this export 16848, file OuterIndex 16821 -> ...:AnimSequence_1
flnotify Npc_SmallRat_as...:AnimSequence_15[1] AnimNotify_AkEvent_0:
         notify export 16810, this export 16826, file OuterIndex 16849 -> ...:AnimSequence_4
flnotify Npc_SmallRat_as...:AnimSequence_31[0] AnimNotify_AkEvent_553:
         notify export 16809, this export 16839, file OuterIndex 16839 -> ...:AnimSequence_30
```

Four things follow, and together they close the question:

1. **The ownership comes out of the file.** `FObjectExport::OuterIndex` for each bad notify points at the
   other sequence. Our `ULinkerLoad` resolved it exactly as written; there is no off-by-one.
2. **It is 7.0 %, not 100 %.** A systematic loader off-by-one would mis-outer every object in every
   package, not 396 of 5,637.
3. **The objects are shared.** Export 16799 is listed by both `AnimSequence_4` and `AnimSequence_15`;
   export 16807 by both `AnimSequence_13` and `AnimSequence_32`. **A notify object referenced from two
   sequences' `Notifies` arrays can only be outered to one of them**, so the error is unavoidable for the
   second one and is a content-authoring artefact retail shares. (`AnimNotify_AkEvent_0` appears twice with
   *different* export numbers, 16799 and 16810 — two distinct objects of the same name under different
   Outers, which is only possible if the Outers genuinely differ in the file.)
4. **It is already fixed, every time.** 396 of 396 repaired. And a notify fires out of its sequence's
   `Notifies` array regardless of its `Outer`, so even before the repair it was never a behavioural fault.

So the apparent "off-by-one" the brief saw in `_39`←`_38`, `_9`←`_7`, `_76`←`_71` is a shared object being
claimed by whichever sequence loaded first, and the neighbours in export order are simply the sequences the
editor duplicated from.

**`DisNotify_NPCDisableLookAt` is still unported** (`DishonoredGame/Src/disnotify_npcdisablelookat.cpp` is a
comment-only skeleton; `Notify` 2013 `0x2dea00`/2012 `0x6dea00`, 91 bytes, and `Notify_EndRange`
2012 `0x6dea60`, 91 bytes). It is 143 of the 396 only because the rat and guard anim sets carry a lot of
them. It is **not** a contributor to the sky-gaze: it would switch the look-at **off** during a scripted
animation, and the look-at is already off — there is nothing for it to switch. It becomes live the moment
`FArkComponentLookat` lands, and it is two 91-byte functions. Named, not written.

## 6. Inventoried, not ported

`FArkComponentLookat` — **51 functions, 0 written, unchanged by this package.** The ones the next package
needs first, in dependency order, all 2013 rvas resolved by mangled name in `retail2013_named.i64`:

| | 2013 rva | size | what it is |
|---|---|---|---|
| `FArkComponentLookat()` | `0x555d60` | 1,147 | the component |
| `Starting` / `Stopping` | unnamed in the 2013 db (2012 `0x596ee0` / `0x597260`) | | **the gate**: `Starting` is what sets `UArkAnimNodeLookAt::m_pConfig`, and until it exists `TickAnim` is inert by design |
| `PreAsyncWorkTick` | `0x555b40` | 162 | the per-frame entry |
| `UpdateHeadAndTorso` | `0x553050` | 2,341 | **what drives the node**: decays `m_fHeadRange`/`m_fTorsoRange`, and at rest calls `SetAim(0,0)` with both ranges at 0 |
| `UpdateEyes` | `0x54e960` | 1,200 | the eye bones |
| `UpdateBlink` | `0x552ec0` | 388 | |
| `OnComposeSkeleton` | `0x552640` | 814 | **the eyes, not the head** — clamps `eye_L_jnt`/`eye_R_jnt` against `MIN/MAX_EYE_YAW` and `MIN/MAX_EYE_PITCH` |
| `UpdateVariablesFromRequest` | `0x553fb0` | 2,668 | |
| `GetLookAtPosition` | `0x552bd0` | 751 | |
| `GetLookEyesPosition` | `0x54d380` | 961 | |
| `GetLookAtNeutralDirection` | `0x54d8b0` | 310 | |
| `GetCurrentLookAtDirection` | `0x54d760` | 329 | |
| `GetHeadAt` | `0x54d750` | 7 | |
| the six `Start*` entry points | `0x554a80`–`0x5549b0` | 130–175 each | what `FDisLookAtRequest::DoRequest` calls; the six warnings in `disdesirestructs.cpp:1265` name them |
| `StopLookAt` / `StopAllLookAtFromAsker` | `0x555710` / `0x555750` | 50 / 12 | |
| `ProceduralLookAt` | `0x552970` | 604 | |
| `DynamicInterpolator<float>::Update` | `0x54dff0` | 589 | |
| `DynamicInterpolator<FVector2D>::Update` | `0x54e240` | 926 | |
| `ManageReferences` | `0x555760` | 182 | GC |
| `GetLookatComponent` | `0x554a60` | 27 | |

`UArkAnimNodeLookAt` — the eight ported above, and **not** ported:

| | 2013 rva | size | why not |
|---|---|---|---|
| the `m_fLookAtModeBlendingRemainingTime > 0` branch of `GetBoneAtoms` | inside `0x51bc80` | | the look-at-mode crossfade. Runs `m_LookAtModeBlendingNode` on a smoothstep of `1 - 2*t`. Nothing in this build changes the look-at mode, so `t` is never set |
| `ComputeHeadAndTorsoAim` | `0x5176f0` | 1,111 | reads `m_HeadBlendInfos` / `m_TorsoBlendInfos`, built from `m_pConfig` |
| `BlendInfos::{UpdateRanges,UpdateDirection,Map,GetP1P2,IsOutside,MapIntoNormalizedSpace,staticInit}` | in `arkanimnodelookat.cpp`'s skeleton | | the triangle mapping behind it |
| `HandleBlendingBetweenLookatNodes` | `0x50c650` | 897 | |
| `HandleProceduralLookatInAnimTreeEd` | `0x526000` | 660 | |
| `GetMaxAim` | `0x50c5b0` | 160 | called by `UpdateHeadAndTorso` |
| `SetLookAtMode` | `0x50c410` | 48 | |
| `SetUseHeadRange` / `SetUseTorsoRange` / `SetUseTorsoDeadZone` | `0x50c440` / `0x50c480` / `0x50a110` | 61 / 61 / 37 | one clamped store each; they are the component's contract, left for the package that writes the caller rather than installed with no caller |
| `BeginDestroy` | `0x50a820` | 38 | deletes the two `BlendInfos` allocations; both are NULL without a config, and `BlendInfos` is not a ported type |
| the editor half of `AutoInitConfigInAnimTreeEd` | inside `0x50a720` | | needs `FDynInterpFVector2D::Init`; `GIsEditor && !GIsGame` is never true in the shipped game |
| `CheckAnimsUpToDate`, `BuildEdgeAnimTree`, `Render`, `GetSlider*`, `HandleSliderMove`, `GetSliderDrawValue` | | | editor and AnimTree Ed |

`UArkComponentLookatConfig` — 5 functions, 0 written. `InitProceduralLookAt` 2012 `0x57cb30`,
`UpdateProceduralLookAt` 2012 `0x57cd10`. Reached only from the component.

`UDisNotify_NPCDisableLookAt::Notify` / `Notify_EndRange` — 91 bytes each, §5.

`UDisSkelControl_SpineBender` — 9 functions, 0 written, `CalculateNewBoneTransforms` 2012 `0x6daf30`. It is
**live on every NPC** (`spine_0_jnt`, `ControlStrength` 1.00) and it is a no-op today because
`USkelControlBase::CalculateNewBoneTransforms` returns an empty array. Found while ruling it out as the
cause of the craning; it is not this package's and it is not a defect today, but it is a hole on the path
anyone will walk next.

## 7. The diagnostics added

All four are off by default, read on **first use** (never a file-scope static — agent CA's rule), and
`DISHONORED(bringup)`-tagged so `normalize_log.py` drops them from the golden diff.

| switch | what it prints |
|---|---|
| `-flheaddiag` | every 3 s: the four NPC pawns nearest the camera, with `l`/`c` per bone for the first 14 bones of **both** meshes, the active skel controls with strength, the sequences carrying weight with their `bIsAdditive`, and each `UArkAnimNodeLookAt`'s weight / `m_Aim` / `m_CurrentBlendedAim` / `m_pConfig` / ranges / non-zero pose weights |
| `-flnotify` | the three AnimNotify counters, plus one line per bad notify with the linker's export indices and the file's `OuterIndex` |
| `-flaimx=<f> -flaimy=<f>` | force the head grid's normalised aim, to exercise both branches of `GetBoneAtomsForSet` |
| `-nodislookataim` | repurposed (agent DO's switch, same name): the **A leg** — skip `InitAnim`'s zeroing and leave the cook's eighteen weights alone. This is how the before/after pair is taken on one binary |

## 8. Acceptance

1. **Before/after against the untouched HEAD executable through the same driver**: `FL1.log` vs `FL3.log`,
   both `-finewgame=20 -apshottime=85`, same pawns, same world time. `head_jnt` 84° → 0°.
   `build/agentFL/shots/fl_heads_three_way.png` adds retail's own frame of the same scene (agent FI's
   `ref5_retail.png`). Both runs have `FocusLost` 1, so they are comparable.
2. `run_regression.py` from this worktree's own copy with an absolute build dir
   (`--build-dir build/agentFL_reg --exe-name DishonoredGame_FLREG.exe --log-prefix FLREG`, so it
   collides with no other package's exe or log): **37 ok, 0 failed, 0 skipped, 970 s**,
   `build/agentFL_regression.log`. Re-run on the final tree after the three `IsValidIndex` guards of §4
   went in — **37 ok, 0 failed, 0 skipped, 328 s**, `build/agentFL_regression2.log`. Nothing in this package moves a harness metric — every stage
   drives `L_Tower_P` with `-startmap`, where the NPC heads were wrong all along and nothing measured
   them. That is the gate's blind spot in miniature, and it is FM's package, not this one's.
3. Clean full Release build into an empty directory, with `DISHONORED_SDK_LAYOUT_CHECKS` at its default
   `ON`: the harness's own build stage into `build/agentFL_reg`, which did not exist when it started —
   `build_DishonoredGame_errors 0`, `build_CoreSmoke_errors 0`, `build_LayoutProbe_errors 0`, all exit 0,
   and `layout_types 2314 / layout_mismatching 0 / layout_contract 0 / layout_probed 2341`.
   **0 C4263 and 0 C4264** across all eight of this package's build logs
   (`build/agentFL_build0.log` … `build/agentFL_build7.log`).
4. `rva_sweep.py` over `DishonoredGame/Src`: **0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013** from this
   package (the one remaining suspect is pre-existing, `disbehaviorpatrol.cpp:319`). Every address above
   was then resolved **by hand** by mangled name in `retail2013_named.i64` with its start and size, and
   cross-checked against `functions.csv` and `functions_2013.csv` — the table in §4 is that check.
   `0x51bc80` is the one address not named in the 2013 database: it is placed by two independent
   arguments, that `0x51b420 + 0x85d = 0x51bc7d` makes it the immediately following function, and that
   `match_2012_2013.csv` maps 2012 `0x55c310` to it by vtable at ratio 0.829; its body then confirms it by
   calling the indexed overload twice with 0 and 1.
5. This file and `agentFL_status.csv`, in the worktree.
6. `build/agentFL_sync.py`, naming every file and every function.

## 9. A trap worth recording

**`rva_sweep.py` passed eight VAs silently.** The first draft of the comments wrote
`2013 rva 0x5138d0 (VA 0x9138d0, …)`. The sweep classified `0x9138d0` as `ok-2013` — because `0x9138d0`
*also* happens to be a valid RVA somewhere else in the image — and flagged only `0x90a720`, which happened
not to be. That is the same failure STATUS records for agent FH's `0xdf0a00`, in the opposite direction:
**a VA written beside its own RVA still passes.** Every address in the tree is now an RVA, and the VA form
appears nowhere.

## 10. Files changed

| file | functions |
|---|---|
| `Engine/Src/UnSkeletalAnim.cpp` | **`UAnimSequence::PostLoad`** — three counters and one guarded diagnostic inside the existing stock notify fix-up loop; the loop's behaviour is unchanged. New file-scope: `GDisAnimNotifiesSeen`, `GDisAnimNotifiesBad`, `GDisAnimNotifiesRepaired`, `DisAnimNotifyOwnerDiagEnabled()` |
| `DishonoredGame/Src/dishonoredplayercontroller.cpp` | **removed** `UArkAnimNodeLookAt::DisSetNormalisedAim` and agent DO's stand-in `UArkAnimNodeLookAt::TickAnim`. **Added** `UArkAnimNodeLookAt::{InitAnim, GetAnimInfoTotalWeight(INT), SetAim, AutoInitConfigInAnimTreeEd, UpdateNodeWeights, GetBoneAtomsForSet, GetBoneAtoms, TickAnim}`, `DisFLHeadDiagEnabled`, `DisFLHeadReport`, `DisFLReportPawn`, `DisFLReportMeshBones`, `DisFLRefSpaceBases`, `DisFLQuatDegrees`, `DisLookAtReadForcedAim`. **Kept, reworded** `DisLookAtAimStandInEnabled`, `DisLookAtPlayerEnabled`. **One line added** inside `ADishonoredPlayerController::Tick` (the `DisFLHeadReport` call, beside the existing census calls) |
| `DishonoredGame/Inc/dispowercensus.h` | two declarations appended: `DisFLHeadDiagEnabled`, `DisFLHeadReport` |
| `DishonoredGame/Inc/CppText/UArkAnimNodeLookAt.h` | rewritten: the eight declarations above plus `using UAnimNodeSequenceBlendBase::GetAnimInfoTotalWeight;` |

**Collision surface.** `dishonoredplayercontroller.cpp` is shared. FK's package is
`Engine/Src/UnInterpolation.cpp`, the `UInterpTrack` family and `ADishonoredNPCPawn::MAT_*` in
`dishonorednpcpawn.cpp` — no overlap with the functions above. FJ is `External/GFx3`. The one function in
this file that another package might also touch is `ADishonoredPlayerController::Tick`, where this package
adds **three lines** next to the existing `DisPowerCensusEnabled` / `DisLookAtCensusEnabled` /
`DisTallboyCensusEnabled` blocks; a textual merge of that hunk is safe, and `build/agentFL_sync.py` flags it
rather than copying if main has moved.

## 11. For whoever ports `FArkComponentLookat`

Four things from this package that will save that work time:

1. **The node is ready for you.** `SetAim`, `UpdateNodeWeights`, `GetAnimInfoTotalWeight`, both
   `GetBoneAtoms` and `InitAnim` are written and retail-faithful. What you have to supply is `m_pConfig`,
   and the moment `Starting` sets it, `TickAnim`'s config branch becomes reachable — it is named in the
   source at the exact line where it goes.
2. **Three functions must land in the same commit as `Starting`**: `ComputeHeadAndTorsoAim`
   (2013 rva `0x5176f0`), the `BlendInfos` triangle mapping it reads, and
   `HandleBlendingBetweenLookatNodes` (2013 rva `0x50c650`). `TickAnim` calls all three the moment the
   config exists.
3. **Check the rest state again the day `m_pConfig` becomes non-NULL.** `UpdateHeadAndTorso`'s own rest
   path is verified from the decompile: with both ranges at or below zero it calls `SetAim(0,0)` and jumps
   past the whole aim computation. But the grid weights are written by the **node**, not by the component,
   and `UpdateNodeWeights` fed a raw (0,0) puts weight **1.00 on the grid centre** — a pose worth 60
   degrees of head rotation in this content. What `ComputeHeadAndTorsoAim` and `BlendInfos::Map` make of a
   zero aim at zero range is not something this package can predict, because neither is ported. The
   assertion to keep is the one `-flheaddiag` already prints: an idle NPC must read `nonzero: all zero`
   and `head_jnt l0/c0`. If it reads `4:ADD_Empty_AimHeadIdle_Center=1.00`, the sky-gaze has come back
   through a different door.
4. **Drive both grids.** `-flaimx/-flaimy` drives only the head grid and the head barely moves (§4); the
   head and torso poses are deltas from a shared base and `ComputeHeadAndTorsoAim` produces both aims from
   one direction. A test that moves one grid alone proves nothing about magnitude.
