# Agent DO report — the powers reach the frame, the tallboy's two passes, and what is really tilting the heads (2026-09-28)

Package **DO** of wave 9, written against **HEAD `9a04708`**. It takes agent DE's hand-over 1
(`ADishonoredPlayerController::ModifyPostProcessSettings`, the last unported channel of the post-process settings
path) and agent DI's hand-overs 1 and 2 (the tallboy's own appearance pass, and the NPC heads that sit tilted back).
Status rows: `agentDO_status.csv`. Build dir `build/agentDO`, snapshot worktree `build/agentDO_wt`, decompiles in
`build/agentDO_decomp/{r13,s12}`, IDA copies `resources/docs/idb/{retail2013,shipping2012}_agentDO.i64`. No commits,
nothing staged.

**Read this first: HEAD `9a04708` does not compile.** `GFxUI/Src/gfxuiengine.cpp:2288` calls the four-argument
`FGFxEngine::InputKey` from the free function `DishonoredGFxInputKey`, and at HEAD that overload is private:
`error C2248: 'FGFxEngine::InputKey': cannot access private member declared in class 'FGFxEngine'`. GFxUI is agent
DM's module, so the one-line repair (make that overload public) is applied to `build/agentDO_wt` **only**
(`build/agentDO_work/head_repair.py`) and is not part of this package. Nothing else is needed to build HEAD.

## Commands

```
python resources/tools/make_snapshot.py DO --sync                 # refresh the snapshot overlay
python build/agentDO_work/head_repair.py                          # HEAD does not compile; snapshot-only repair
python build/agentDO_work/snapshot_shims.py                       # this package's one line into HEAD's shims header
cmd /c build\agentDO_release.cmd                                  # Release into build/agentDO (+ CoreSmoke, LayoutProbe)
cmd /c build\agentDO_clean.cmd                                    # a clean full Release build into build/agentDO_clean
python resources/tools/stage_retail.py --build-dir build/agentDO --exe-name DishonoredGame_DO.exe
sh build/agentDO_work/runall.sh                                   # the seven measured runs
sh build/agentDO_work/runtallboy.sh                               # the three tallboy probes
python resources/tools/run_regression.py --build-dir build/agentDO --no-build
```

One measured run in full:

```
python build/agentDO_work/run.py pp_after_a -disdarkvisionpp -dispowerdbg -apshottime=25 --timeout=55
```

which is `DishonoredGame_DO.exe L_Tower_P?Name=Corvo?Team=255 -log -nosteam -unattended -LOG=agentDO_pp_after_a.log
-<kind>INI=build/agentDO/config/Dishonored<kind>.ini -benchmark -fps=30 -forcelogflush -windowed -ResX=1280
-ResY=720 -nomovie -nogfxui -skipnativepkgs=OnlineSubsystemPC -disdarkvisionpp -dispowerdbg -apshottime=25`.
Drop `-disdarkvisionpp` for the other leg.

## Result

| Accept | State |
|---|---|
| 1. the powers visibly change the frame | **done**: dark vision, one binary, one switch - **signed mean -49.81 of 765 over 99.98 % of pixels** against a **byte-identical** off-leg floor and a **-0.81** signed on-leg floor; the post-process controllers tick 5,668 times a run against 0 before (section 4.1) |
| 2. a guard looking at something | **done, and the cause was not what the hand-over said**: the guards' heads are craned back because the anim tree's head-aim blender applies `ADD_Empty_AimHeadIdle_UpLeft` at weight 1.00 for ever. `head_jnt` **57 degrees off its bind pose -> 1**, `neck_jnt` **33 -> 0**, and with `-dislookatplayer` the weights blend `Center 0.59 / Down 0.41` and the guard's head follows the player (sections 3, 4.2, 4.3) |
| 3. a tallboy with its stilts | **an account, not a screenshot**: `L_Tower_P` loads no tallboy content at all, and the two maps that carry it both die at 5.85 s / 7.43 s on an Engine skeletal-mesh assert that fires before either pass runs (section 4.4). Both passes are ported |
| 4. `run_regression.py` green + a clean full build | **31 ok, 0 failed, 0 skipped, 271 s** on this package's own `--build-dir`, plus a clean full Release build of the snapshot worktree, **964 steps, 0 errors** (section 5) |

## 1. The ACTOR feed: `ADishonoredPlayerController::ModifyPostProcessSettings` (2013 rva `0x6adeb0`)

1,498 bytes, and it does two things.

**It finds twenty-six post-process nodes by name and caches them** on `UDisPostProcessManager::m_PpBridge`
(`UnderWat_BendTime_Blink`, `UnderWaterVectors`, `BendTimeVectors`, `BendTimeUnderWaterVectors` and its switch,
`BendTimeSwitch`, `BlinkVectors`, `BlinkVectors2`, `BlinkSwitch`, `BlinkOnlySwitch`, `AdrenalineVectors` and its
switch, `PlagueVectors` and its switch, `PossessionVectors`, `PossessionIN`, `PossessionOUT`, `PossessionSwitch`,
`Antialiasing`, `BackupLastFrame`, `BlendLastFrame`, `KOVectors`, `KOSwitch`, `LensVectors`, `LensCompose`,
`LensSwitch`). Only when **all** of them resolve does it raise `m_PpBridge.m_bInitDone`, and that one bit is the gate
on the manager's whole effect drive: until this function has run once, no power can show at all.

**Then, every frame, it folds four feeds into the `FArkPpConfig` the view carries**: the camera's post-process
targets, the water volume's override, the player's health effects and the dark-vision power.

Ported with it: `DisGetPpManager` (`0x7bf290`), `UDisPostProcessManager::{IsEffectRequired, StartEffect, StopEffect}`
(`0x7e7da0`, `0x7e7dc0`, `0x7e7df0`), `UDisActivePowerComponent_DarkVision::IsPpActive` (`0x7e7000`),
`ADishonoredPlayerCamera::ApplyCameraPostProcess` (`0x6ca780`), `ADishonoredPlayerPawn::ApplyHealthEffectsPost`
(`0x6ac910`), `ADishonoredPlayerController::ApplyWaterPostProcessSettings` (`0x6a6750`),
`ApplyDarkVisionPostProcessSettings` (`0x6a68d0`), `ApplyPossessionPostProcessSettings` (`0x6a6c30`) and
`ADishonoredPlayerController::Tick` (`0x6b6e80`).

### Defect 1 — retail does **not** tick the post-process controllers from `ModifyPostProcessSettings`

Agent DE's defect 7 said it did, and that is why the three controllers DE ported
(`UDisOpacityParameterPpController`, `UDisDarkVisionPpController`, `UDisBlindedPpController`) were left dormant.
`ModifyPostProcessSettings` has no such call in either build. The only call site of `UArkPpNodeController::Tick` in
either executable is **`ADishonoredPlayerController::Tick`** (2013 rva `0x6b6e80`, 2012 `0x6ef510`), which walks the
first local player's `UPostProcessChain::m_AllNodes` and ticks every node's `m_Controller` with the frame's delta.

Found by scanning, not by reading. The controller's `Tick` is vtable slot **+304 in retail 2013** and **+300 in the
2012 build**: retail's `UDisDarkVisionPpController` vtable starts at `0xd67408` with `Update` at +292, `IsShown` at
+300 and `Tick` at +304, against `0xd5a0e0` / +288 / +296 / +300 in 2012 — retail's `UObject` carries one virtual
more. Every `mov r32,[reg+disp]` and `call [reg+disp]` at those displacements in the whole `.text` of both
executables was enumerated (`build/agentDO_work/vslot2r_304.txt`, `vslot300.txt`, `vslot296.txt`). The scan was
validated first against a slot with known callers: at +296 in the 2012 build it finds exactly the three functions
that call `UArkPpNode::IsShownInConfig` inline (`UArkPpNodeBlur::CreateSceneProxy`, `UArkPpNodeMaterial::IsValid`,
`UArkPpNodeMaterial::CreateSceneProxy`), and the same three appear at +300 on retail — which is the tell that the
displacement has to come from retail, not from the 2012 PDB.

With `ADishonoredPlayerController::Tick` ported, the controllers tick and agent DE's three bodies are live for the
first time.

### Defect 2 — retail 2013 backs the BendTime node's uber parameters up inside the init block; the 2012 build does not

Retail copies `m_BendTimeVectorsPp->m_UberParameters` into `m_PpBridge.m_BendTimeVectorsUberBackup` immediately after
finding the node, unguarded. The 2012 build has no such copy at all. Ported as retail 2013 has it, with a null guard,
because a missing node is exactly the case the tail of the block tests for.

### Defect 3 — retail's node cache retries for ever when one node is missing

The init block is a straight run of twenty-six lookups with no test until the end, where all twenty-six are tested
at once and `m_bInitDone` is only raised if every one of them resolved. A chain missing one
node therefore re-runs the whole lookup every frame, for ever, and says nothing. Kept exactly as retail has it; the
census counts the misses so that state is visible instead of silent (`bridgeInit 0 (misses N)`).

### Defect 4 — `DisGetPpManager` returns a reference and faults when there is no game info

`if (GWorld && (GameInfo = GetGameInfo()) != 0) return GameInfo->m_pPpManager; else return *(UDisPostProcessManager*)0x3D8;`
— the else branch is a null dereference at the caller. Every caller here tests for NULL instead.

### Defect 5 — the possession intro divides by the intro duration with no guard

`ApplyPossessionPostProcessSettings` stage 1 ramps at `1 / m_IntroDuration * target * dt`. A zero intro duration
divides by zero exactly as retail does; left as retail has it and said at the site.

### What is not ported, and exactly what each one needs

Five of retail's calls are named at the call site rather than skipped in silence, each with its rva and its blocker:

* **`UpdateSunBlindingEffect`** (`0x6a6450`, 390 bytes) — walks `AWorldInfo::m_SunMeshesAndMaterials` @600 and pushes
  `pow(dot, m_fBloomExponent) * m_fBloomScale + m_SunMICInitialBloomValue` into each entry's `m_SunDynamicMIC`
  through `SetScalarParameterValue` with a **hardcoded FName index (1300, 0)**. That index is retail's name table,
  not this tree's, and guessing a material parameter name is the class of unverified change this project keeps
  paying for. It is the sun glare, not a power.
* **`ApplyAdrenalineProcessSettings`** (`0x6a65e0`, 367 bytes) — `m_fAdrenalineWeight` @1780 rises and falls against
  the player-pawn combat tweaks and lands in `UDisPostProcessManager::m_AdrenalineParams`; needs
  `ADishonoredPlayerPawn`'s slow-motion predicate (`0x6a21e0`) and `FDisZoneTracker::IsInZone`, both unported.
* **`ApplyMusicalOverseerPostProcessSettings`** (`0x6a6970`, 691 bytes) — gated on
  `ADishonoredPawn::ArePowersInhibited`; needs a second tweaks class and the heart's proximity curve.
* **`UDisPostProcessManager::ApplyKismetPostProcessSettings`** (`0x7ef9b0`) and **`ApplyUIPostProcessSettings`**
  (`0x7efca0`) — driven by `SetKismetPPParams` / `SetUIPPParams`, which nothing in this tree calls yet.
* **`UDisPostProcessManager::Tick`** (2012 `0x857430`, 2,548 bytes, called from `ADishonoredPlayerPawn::Tick`) — the
  node-visibility and material drive for underwater, bend time, blink, adrenaline, plague, the knock-out and the two
  possession stages: it raises `m_bShowInGame` on the nodes and `m_Selection` on the switches and pushes the vector
  parameters. It is gated on `m_PpBridge.m_bInitDone`, which this package now raises, so it is the single largest
  remaining piece of the powers and it is a package of its own. **Dark vision does not go through it**, which is why
  dark vision is the one power this package can show end to end: its node is driven by its own
  `UDisDarkVisionPpController`, and `Epp_DarkVision` is not an index the manager's Tick reads.

Two calls inside `ApplyWaterPostProcessSettings` are named for the same reason:
`FSceneInterface::SetBelowWaterTranslucentSort` (retail vtable slot 1; this tree's `FSceneInterface` has no such
virtual and adding one at slot 1 would shift every slot after it) and the whole `UDisFogComponent` block — that class
has **no declaration at all** in this tree, only forward uses, so its two floats cannot be written. Retail closes the
function with `DisGetGameInfo()` and one call on it that the retail build has identical-code-folded with
`UDishonoredTask_Base::OnAdded_Impl`; the fold makes the real callee unidentifiable and it is not guessed at.

## 2. The tallboy: two passes, and agent DI's hand-over named the wrong one

DI's hand-over 1 called `ADisTallboyNPCPawn::PostBeginPlay_Body` (`0x781270`) "the stilts mesh". It is not.

* **`PostBeginPlay_Body`** (`0x781270`, 2012 `0x7f24d0`, 295 bytes) is the **searchlight**: it spawns
  `UDisTweaks_TallboyNPCPawn::m_pLightToSpawn` at the pawn's location with no rotation, writes `m_fLightRadius` /
  `m_fLightOuterConeAngle` / `m_fLightBrightness` into the spawned light's component (`UPointLightComponent::Radius`
  @428, `USpotLightComponent::OuterConeAngle` @552, `ULightComponent::Brightness` @256), reattaches it, seeds
  `m_SpotlightManager` from it (`FDisSpotlightManager::Init`, `0x840860`), starts the light's particle system
  (`CreateLightParticleSystem`, `0x76eff0`) and posts the ambient Ak event.
* **The stilts are `ADisTallboyNPCPawn::ApplyTweakChanges_Derived`** (2013 rva **`0x77daa0`**, 2012 `0x7f2440`,
  140 bytes) — the only function in the executable that ever puts a `USkeletalMesh` on `m_pStiltsMesh`, out of
  `UDisTweaks_TallboyNPCPawn::m_pStiltsSkeletalMesh` @940, followed by `SetParentAnimComponent(Mesh)`. **It is
  agent DI's own head defect one pass over**: a second skeletal mesh on the same skeleton, on a component nothing
  ever wrote. Found by following the callers of `UDisTweaks_TallboyNPCPawn::StaticClassNoInline` in retail, because
  retail's function is unnamed and its 2012 neighbour ordering does not carry over.

Both are ported, with `CreateLightParticleSystem` and `FDisSpotlightManager::Init` (spelled as the file-local
`DisSpotlightManagerInit`, as agent DI spelled `ApplyMaterialVariationToMesh`, because `FDisSpotlightManager` is a
generated struct and a method on one costs a full regeneration).

### Defect 6 — retail 2013 moved `PostAkEvent` inside the restore guard

2012 tests the light class and the "state is going to be restored" flag together and posts the ambient Ak event
**unconditionally**; retail 2013 puts the whole body, `PostAkEvent` included, inside
`if (!DisIsObjectStateGoingToBeRestored(this))`. A tallboy restored from a save therefore does not re-post its
ambient sound in retail and does in 2012. Ported as retail 2013 has it.

### `ADisPossessionProxyPawn::PostBeginPlay_Body` calls its *grand*parent

`0x7e7910`, 19 bytes: it calls `ADishonoredPawn::PostBeginPlay_Body` (`0x76a130`), **not**
`ADishonoredNPCPawn::PostBeginPlay_Body` (`0x77f8a0`) — it skips the head, the accessories and the material
variations of the NPC pass, because the proxy is never drawn. Then `SetIsVisible(FALSE)`, which is named as a
bring-up gap: it starts and stops `FDisComponentObservable`, the ark component the AI sight query walks, and this
tree holds `ADishonoredPawn::m_pCpntObservable` as an opaque `FPointer` with no class behind it. That component is
the AI package's.

## 3. The tilted heads: measured, and it is neither the head mesh nor `FArkComponentLookat`

Agent DI's hand-over 2 read the tilt as "the body's animation pose reaching the head through the now-correct
`ParentAnimComponent`" and pointed at `FArkComponentLookat` / `FDisLookAtRequest::Initialize` as the gap. The
measurement (`-dislookat`) says something much more specific, and it took four passes to get there.

**First: the head mesh's own bone transforms are never drawn.** `UpdateRefToLocalMatrices`
(`Engine/Src/UnSkeletalRender.cpp:165`) takes a parent-animated component's matrices from `ParentComp->SpaceBases`
through `ParentBoneMap` whenever the map is complete, and it is (118 of 118). The head component's own `SpaceBases`
— which the census shows frozen at the reference pose, `head_jnt` at a constant `(P1109 Y-16384 R32768)` — never
reach the screen. What is drawn is the **body** mesh's `head_jnt`, which moves every frame.

**Second: on the body mesh, the head and the neck are the only bones far from their bind pose.** Angle between the
animated `LocalAtoms` rotation and `RefSkeleton(i).BonePos.Orientation`, on `Skm_EliteGuard_Body`:

```
root0_jnt 0; Root_jnt 7; spine_0_jnt 7; spine_1_jnt 0; spine_2_jnt 1; spine_3_jnt 0; Torso_jnt 0;
neck_jnt 34; head_jnt 63; head_end_jnt 0; camera_jnt 0; jaw_jnt 0; (every face bone 0)
```

Thirty-four degrees at the neck and sixty-three at the head, stacked, with the whole spine inside seven. That is not
a pose; that is one channel.

**Third: no skel control touches either bone.** The tree has five control lists and they are
`spine_0_jnt:DisSkelControl_SpineBender`, `eye_L_jnt` and `eye_R_jnt:SkelControlSingleBone`, and `foot_R_jnt` /
`foot_L_jnt:DisSkelControl_FootPlacement`. Retail's head-look is not a skel control.

**Fourth, and this is the defect.** The NPC anim tree `NPC_Human_at` has a node called **`LookatBlender`**, of class
**`UArkAnimNodeLookAt`**, at full weight, feeding an `AnimNodeAdditiveBlending` whose additive input is at weight
1.00. It is a `UAnimNodeSequenceBlendBase`, and its `Anims` array holds **eighteen additive aim poses** — nine head
and nine torso, on a 3x3 grid. The weights read:

```
ADD_Empty_AimHeadIdle_UpLeft=1.00  ADD_Empty_AimHeadIdle_CenterLeft=0.00  ADD_Empty_AimHeadIdle_DownLeft=0.00
ADD_Empty_AimHeadIdle_Up=0.00      ADD_Empty_AimHeadIdle_Center=0.00      ADD_Empty_AimHeadIdle_Down=0.00
ADD_Empty_AimHeadIdle_UpRight=0.00 ADD_Empty_AimHeadIdle_CenterRight=0.00 ADD_Empty_AimHeadIdle_DownRight=0.00
(and the nine ADD_Empty_AimTorsoIdle_* the same)
```

**Entry 0 at weight 1.00 — "aim head idle, up and to the left" — applied additively at full strength, for ever.**
That is the tilted head, exactly. It is the cook's default array: the only thing in retail that ever writes those
eighteen weights is `UArkAnimNodeLookAt::UpdateNodeWeights` (2012 rva `0x5532e0`, 3,294 bytes), reached from
`TickAnim` (`0x5672c0`) through `ComputeHeadAndTorsoAim` (`0x557b80`) and a `BlendInfos` triangle mapping, and only
while `m_pConfig` is set — which is `FArkComponentLookat::Starting`'s job. `Engine/Src/arkanimnodelookat.cpp` is a
comment-only stub of 43 functions, and `arkcomponentlookat.cpp` of 51.

This is the thirteenth instance of the standing pattern and the first where the placeholder is a **cooked data
default** rather than a storage-less member: the content is fully authored (eighteen aim poses on a grid), the code
that would read the grid is absent, and the array's first cell is applied at full weight.

### What this package does about it, and what it deliberately does not

`UArkAnimNodeLookAt::TickAnim` here is a **stand-in, not a port**, switched off by `-nodislookataim`: it drives the
same eighteen weights from a normalised aim by bilinear interpolation on the same grid, so the rest state (aim 0,0)
is the `Center` pair — which is the rest state the node is authored around — instead of a corner. The units are
**not** retail's: retail's `m_Aim` is in degrees mapped through `UArkComponentLookatConfig`'s per-mode ranges, and
this takes [-1,1]. Whoever ports `UArkAnimNodeLookAt` deletes it. With `-dislookatplayer` the aim is taken from the
direction to the player, which is what makes a guard turn its head and follow.

**The AI census reading `lookat 0` is a separate thing and it is not a defect.** `FDisLookAtRequest` is already
ported (agent DF; `Initialize` is 2013 rva **`0x8b2400`**, not the `0x74f180` the brief carried) and `DoRequest`
already counts all six `FArkComponentLookat` entry points. The count is zero because no *idle* behaviour asks for a
look-at: retail's `GetDesiresLookAtRequest` exists on `UDisAISubProcessPersonalSpace`, `UDisAISubStateFirePistol`,
`UDisAISubStateInvestigate`, `UDisAISubStateMeleeEngage`, `UDisAISubStateStareAtUnreachable`,
`UDisAISubStateWHCombatShortDistance`, `UDisBehaviorNotice`, `UDisBehaviorSearch` and `UDisBehaviorStationarySearch`
— nine classes, none of them the idle loop the NPCs of `L_Tower_P` are in. A guard that has *noticed* something is
what moves that counter, and that is the AI package's side.


## 4. The measurements

Every run is `build/agentDO_work/run.py`: the map as the command line's **first token** (agent DE's route, so the
mission is the very first `LoadMap` and no one-shot probe latches on the startup map), `-benchmark -fps=30` for a
fixed frame step, `-windowed -ResX=1280 -ResY=720 -nomovie`, `-nogfxui`, and **`-apshottime`, never `-apshot`**.

`-nogfxui` is in every run and it is not cosmetic: at HEAD the main menu comes up over the running game on some runs
and not others, and a frame with `NEW GAME | MISSIONS | OPTIONS | QUIT GAME` across it is not a measurement of a
colour grade. That is what a first pass of this package measured before the switch was added, and it is worth
knowing for anyone else taking frames at HEAD.

### 4.0 The measured pair was re-taken on the final binary

Everything below was first measured on `build/agentDO_build13`; the last edit of the package (a guard in
`ApplyWaterPostProcessSettings`, in a branch `L_Tower_P` never enters) produced `build14`, and the pair was taken
again on it. The **off** leg is **byte-identical across the two builds** (md5 `be666db0...`), and the pair on the
final binary reads \|mean\| 152.53, **signed mean -49.81**, 99.98 % of pixels, max 752 - the same signal to within
0.06 % of the signed mean. The tables below are the build-13 numbers; `pp_before_c` / `pp_after_c` are the final
binary's.

### 4.1 The powers: dark vision, one binary, one switch

`-disdarkvisionpp` holds `UDisActivePowerComponent_DarkVision::IsPpActive` true, which is the single bit the power
itself sets. Everything after it is the ported path: `ApplyDarkVisionPostProcessSettings` puts the bit on
`UDisDarkVisionPpController::m_bIsActive`, `ADishonoredPlayerController::Tick` ticks the controller so
`m_EyeLidTime` and then `m_PowerTime` rise, the controller's `IsShown` answers TRUE, and its `Update` hands the node
the power material and pushes dark vision's static and dynamic uber adjustments into the config.

| pair | \|mean\| of 765 | signed mean | % of pixels > 2 | max |
|---|---:|---:|---:|---:|
| `pp_before_a` vs `pp_after_a` (the switch) | **178.26** | **-49.84** | **99.99 %** | 763 |
| `pp_before_b` vs `pp_after_b` (the same pair, second run of each) | 182.92 | -50.65 | 100.00 % | 765 |
| `pp_before_a` vs `pp_before_b` (floor, off) | 0.00 | 0.00 | 0.00 % | 0 (**byte-identical**, md5 `be666db0...`) |
| `pp_after_a` vs `pp_after_b` (floor, on) | 54.96 | **-0.81** | 74.07 % | 581 |

The two floors are the reason the signed mean is the figure to read. With the effect **off** the frame is
byte-identical run to run. With it **on** it is not, and it should not be: dark vision's pass re-draws its film
grain and its two runtime materials every frame, so three quarters of the pixels move a little and the absolute
mean of the floor is 54.96 — while its **signed** mean is -0.81, because the movement has no direction. The signal's
signed mean is -49.84, **61 times** the floor's, and it is the same to within 1.6 % on the independent second pair.

Frame statistics: luma **95.99 -> 46.15**, R **85.93 -> 71.83**, G **97.79 -> 64.24**, B **104.25 -> 2.38**, median
78 -> 30. The blue channel collapsing to near zero while red holds is Dishonored's dark vision exactly, and
`build/agentDO/darkvision_after.png` is the picture of it beside `darkvision_before.png`.

The controllers are demonstrably ticking, which is what defect 1 bought:

```
dispower census: manager 1 bridgeInit 1 (misses 0) modifyCalls 1417 | chain nodes 42, controllers 4, shown 3,
  controller ticks 5668
dispower darkvision: node ArkPpNodeMaterial_9 showInGame 1 controller DisDarkVisionPpController active 1 debug 0
  eyeLid 1.000 power 1.000 shown 1 material TEST_PPG_EyeLid
dispower effects: 3:req1/state2 | tallboy stilts 0 lights 0
```

`bridgeInit 1 (misses 0)` — all twenty-six nodes resolved on the first pass, so the gate on every other effect is
open for the first time. `controller ticks 5668` over the run against **0** before this package. `eyeLid 1.000
power 1.000` is `UDisDarkVisionPpController::Tick` having run the two stages to completion; `shown 2 -> 3` is its
`IsShown` answering TRUE; `3:req1/state2` is `Epp_DarkVision` requested through the ported `StartEffect`. 0
`Critical` in either leg.

### 4.2 The head: one binary, one switch

`-nodislookataim` is the A leg — it leaves the eighteen weights exactly as the cook left them, which is what this
tree did before this package. Same binary, same framing (agent DI's `-disheadcam`), same `-apshottime`.

| | `-nodislookataim` (before) | default (after) |
|---|---|---|
| the weight the blender carries | `ADD_Empty_AimHeadIdle_UpLeft = 1.00`, every other entry 0.00 | `ADD_Empty_AimHeadIdle_Center = 1.00`, every other entry 0.00 |
| `head_jnt` from its bind pose | **57 degrees** | **1 degree** |
| `neck_jnt` from its bind pose | **33 degrees** | **0 degrees** |
| the frame | `build/agentDO/head_before.png` - the craned-back head of agent DI's screenshots, reproduced | `build/agentDO/head_after.png` - **the guard's head is level and facing forward** |
| the two frames | \|mean\| 1.51 of 765, signed +0.12, 1.24 % of pixels, max 705 | |

The image figure is small and it should be: one NPC's head is about a percent of a 1008x567 frame, and the maximum
of 705 says that where it changed, it changed almost completely. The measurement that matters here is the bone one -
57 degrees to 1 - and it is the same two numbers that identified the defect in the first place.

### 4.3 A guard looking at something

`-dislookatplayer` takes the aim from the direction to the player. The weights stop being one-hot and blend, which
is the whole point of a 3x3 grid of aim poses:

```
ADD_Empty_AimHeadIdle_Center=0.59  ADD_Empty_AimHeadIdle_Down=0.41   (every other entry 0.00)
head_jnt 8 degrees from bind, neck_jnt 7
```

The player's view point sits below the guard's head, so the blend is between centre and down and the guard's head
is angled down at him - `build/agentDO/head_look.png` against `head_after.png` (level) and `head_before.png`
(craned back). \|mean\| 1.36 of 765, signed +0.10, 4.66 % of pixels, max 725 between the level head and the
looking one.

This is the stand-in, not `FArkComponentLookat`. What it proves is what the eighteen cooked poses are for and that
driving them is all the anim tree needs; what it does not do is take that aim from a look-at request, which is the
port (hand-over 2).

### 4.4 The tallboy: where the content is, and why no tallboy walks today

`-distallboy` counts loaded tallboy pawns, archetypes and tweak objects; `-distallboyspawn=<seconds>` spawns one
from the first loaded archetype. Three maps:

| map | result |
|---|---|
| `L_Tower_P` (the only map this tree runs) | `0 tallboy pawns (0 archetypes), stilts mesh set 0, attached light 0; tweak objects 0` — **no tallboy content is loaded at all**, so there is nothing to spawn from and nothing for either pass to do |
| `L_Boyle_Ext_P` | `0 tallboy pawns (0 archetypes) ... tweak objects 2 Pwn_Tallboy_Wood_SearchLight(stilts Skm_TallBoyStilts) Pwn_Tallboy_Wood(stilts Skm_TallBoyStilts)` — **the content is there and it names the stilts mesh**, but the map dies at 5.85 s |
| `L_Streets1_P` | `tweak objects 0` at 4.90 s, and the map dies at 7.43 s |

**Both of those maps die on the same Engine assert, and it is not this package's**:

```
Critical: appError called: Assertion failed: BoneVisibilityStates.Num() == SkeletalMesh->RefSkeleton.Num()
  [File: Engine/Src/UnSkeletalComponent.cpp] [Line: 3357]
```

The census line printed a second and a half earlier says `ApplyTweakChanges 0, light spawns 0` — neither tallboy
pass has run when it fires, so neither can be the cause. It is `UpdateRequiredBones`'s check that a component's
`BoneVisibilityStates` is as long as its mesh's skeleton, and it means **`L_Tower_P` is the only mission map this
tree survives past seven seconds today.** That is a blocker for anyone who needs content the first mission does not
carry — tallboys, masked NPCs for agent DI's accessories and material variations, weepers — and it is the reason
this accept is an account rather than a screenshot.

So: both tallboy passes are ported and exercised by nothing, the same way agent DI's accessory path is. What proves
them is a map with a tallboy spawner that runs for more than seven seconds, and the assert above is what stands
between here and that.

## 5. The harness and the builds

`python resources/tools/run_regression.py --build-dir build/agentDO --no-build`: **31 ok, 0 failed, 0 skipped,
271 s** (`build/agentDO/regression1.txt`, `build/agentDO/regression/summary.txt`). `layout_types 2314`,
`layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`, `coresmoke_passed 99`, `verify_phase2 2/2`,
`nullrhi_criticals 0`, `d3d9_frames 2430`, `d3d9_criticals 0`, `d3d9_draw_elements 6506`, `unported_natives 0`,
`touch_census 308`, `sequence_census 6078`, `inputtest_moved 1019.5`, `inputtest_peak_speed 500.0`,
`physics_actors 1369`, `physics_static_shapes 1312`, `probe_natives 6`, `inputtest_criticals 0`.

`inputtest_moved` is 1019.5 against 1029.6 at HEAD and `probe_natives` 6 against 7, both inside their bounds:
nothing this package touches is on the walking path.

| build | result |
|---|---|
| `build/agentDO_release.cmd` (DishonoredGame, CoreSmoke, LayoutProbe into `build/agentDO`) | 0 errors, 0 unresolved symbols (`build/agentDO_build14.log`, `build15.log`) |
| `build/agentDO_clean.cmd` (a CLEAN full Release build of the same worktree into `build/agentDO_clean`) | **964 steps, 0 errors**, all three binaries (`build/agentDO_clean.log`) |

The measured logs: `pp_before_a/b/c.log`, `pp_after_a/b/c.log`, `head_before.log`, `head_after.log`,
`head_look.log`, `tb_tower.log`, `tb_streets1.log`, `tb_boyle.log` in `build/agentDO/`. 0 `Critical` in every
`L_Tower_P` run; the same 577 `Warning:`/`Error:` lines in the off leg, the on leg and the head runs, so the port
adds no new warning.

## 6. Files

Mine (20, `build/agentDO/snapshot_files.txt`):

| File | Change |
|---|---|
| `DishonoredGame/Src/dishonoredplayercontroller.cpp` | `ModifyPostProcessSettings`, `Tick` (the node-controller drive), `ApplyWaterPostProcessSettings`, `ApplyDarkVisionPostProcessSettings`, `ApplyPossessionPostProcessSettings`, the `UArkAnimNodeLookAt` aim stand-in, and the three censuses |
| `DishonoredGame/Src/dishonoredplayercamera.cpp` | `ApplyCameraPostProcess` (was a comment-only skeleton) |
| `DishonoredGame/Src/dishonoredplayerpawn_combat.cpp` | `ApplyHealthEffectsPost` (was a comment-only skeleton) |
| `DishonoredGame/Src/distallboynpcpawn.cpp` | `ApplyTweakChanges_Derived` (the stilts), `PostBeginPlay_Body` (the searchlight), `CreateLightParticleSystem`, `DisSpotlightManagerInit` (was a comment-only skeleton) |
| `DishonoredGame/Src/dispossessionproxypawn.cpp` | `PostBeginPlay_Body` (was a comment-only skeleton) |
| `DishonoredGame/Src/dishonoredutilities_accessors.cpp` | `DisGetPpManager` |
| `DishonoredGame/Inc/dispowercensus.h` | new: the three censuses and the five switches |
| `DishonoredGame/Inc/dishonoredutilities.h` | one declaration |
| `DishonoredGame/Inc/CppText/` | new: `ADisTallboyNPCPawn.h`, `ADisPossessionProxyPawn.h`, `ADishonoredPlayerCamera.h`, `UDisPostProcessManager.h`, `UDisActivePowerComponent_DarkVision.h`, `UArkAnimNodeLookAt.h`; appended: `ADishonoredPlayerController.h`, `ADishonoredPlayerPawn.h` |
| `DishonoredGame/Inc/dishonoredgameclasses.h`, `DishonoredGamePowerClasses.h`, `DishonoredGameCameraClasses.h`, `DishonoredGameEngineShims.h` | **generated**: one `#include "CppText/<Class>.h"` line each (three in the first), which is exactly what the generator emits once the CppText file exists |
| `DishonoredGame/Sources.cmake` | **generated**: four units off the skeleton-exclude list (`dishonoredplayercamera`, `dishonoredplayerpawn_combat`, `distallboynpcpawn`, `dispossessionproxypawn`), which is what the generator does once a skeleton unit has a body in it - agent DE's defect 9 |

**The generated files are edited, so the coordinator must regenerate at merge with all three flags**
(`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`) — and **I have not regenerated**,
per the rule. Every one of those edits is what the generator produces by itself from the files this package adds.

Not mine, and not part of this package: `GFxUI/Inc/GFxUIEngine.h`, repaired in `build/agentDO_wt` only so HEAD
compiles (see the top of this report).

Helpers in `build/agentDO_work/` (not repo tools): `write_tallboy.py`, `write_pp.py`, `write_pp2.py`, `fix1.py`,
`fix_water.py`, `write_lookat_census.py`, `write_lookat2.py`..`write_lookat6.py`, `write_lookataim.py`,
`write_tallboy_probe.py` (every edit re-applied idempotently), `head_repair.py`, `snapshot_shims.py`, `run.py`,
`runall.sh`, `runtallboy.sh`, `stats.py`, `xref.py`, `vslot.py`, `vslot2.py`, `frange.py`.

## 7. Bring-up switches

| Switch | What it does |
|---|---|
| `-dispowerdbg` | one census line a second: the manager, the bridge's init state and misses, the chain's node and controller counts, how many controllers say their node is shown, the controller tick count, the dark-vision controller's whole state, and every effect with a non-zero request or state |
| `-disdarkvisionpp` | holds `UDisActivePowerComponent_DarkVision::IsPpActive` true - the one bit the power itself sets, and the switch of the accept pair |
| `-nodismodifypp` | `ModifyPostProcessSettings` returns at once: the whole ACTOR feed off, which is exactly what this tree delivered before this package |
| `-dislookat` | the head-look measurement: both meshes' `head_jnt` and `neck_jnt` in component and local space, the body's per-bone angle from its bind pose, the tree's skel-control lists, its aim-offset nodes, its node table, every blend's children and every `UAnimNodeSequenceBlendBase`'s `Anims` array with weights |
| `-nodislookataim` | turns the head-aim stand-in off, leaving the cook's default weights: the A leg of the head pair |
| `-dislookatplayer` | aims every NPC's head at the player |
| `-distallboy`, `-distallboyspawn=<seconds>` | the tallboy census, and a spawn from the first loaded archetype in front of the player |

All seven are read on first use, never in a file-scope static (agent CA's finding, PHASE10 rule).

## 8. Hand-overs

1. **`UDisPostProcessManager::Tick` is now the single largest remaining piece of the powers** (2012 rva `0x857430`,
   2,548 bytes, called from `ADishonoredPlayerPawn::Tick`, itself unported). It is gated on
   `m_PpBridge.m_bInitDone`, which this package raises for the first time, so it is unblocked: it raises
   `m_bShowInGame` on the nodes and `m_Selection` on the switches and pushes the vector parameters for underwater,
   bend time, blink, adrenaline, plague, the knock-out and the two possession stages. Everything this package wrote
   into `m_PossessionParams` is read by its `TickPossession` (`0x84c1e0`) and by nothing else today. With it,
   `ApplyAdrenalineProcessSettings` (`0x6a65e0`) and `ApplyMusicalOverseerPostProcessSettings` (`0x6a6970`) are the
   two feeds left, and `UpdateSunBlindingEffect` (`0x6a6450`) needs one retail name-table index resolved.
2. **Head-look is `UArkAnimNodeLookAt` first, `FArkComponentLookat` second, and the order matters.** The node
   (`Engine/Src/arkanimnodelookat.cpp`, 43 functions: `TickAnim` `0x5672c0`, `ComputeHeadAndTorsoAim` `0x557b80`,
   `UpdateNodeWeights` `0x5532e0` at 3,294 bytes, `BlendInfos::{UpdateDirection, UpdateRanges, Map, GetP1P2,
   IsOutside, MapIntoNormalizedSpace}`, `GetBoneAtoms` x2, `InitAnim`, `HandleBlendingBetweenLookatNodes`, plus
   `UArkComponentLookatConfig::UpdateProceduralLookAt`) is what turns an aim into the eighteen weights, and its
   `m_pConfig` comes from `FArkComponentLookat::Starting`. The component
   (`Engine/Src/arkcomponentlookat.cpp`, 51 functions: `OnComposeSkeleton` `0x552640`, `UpdateHeadAndTorso`
   `0x553050`, `UpdateVariablesFromRequest` `0x553fb0`, `UpdateEyes` `0x54e960`, `UpdateBlink` `0x552ec0`,
   `ProceduralLookAt` `0x552970`, `PreAsyncWorkTick` `0x555b40`, the six `Start*` entry points at `0x554a80`..
   `0x5559b0`, `FLookatRequestData::*`) is what turns a request into that aim. Delete this package's stand-in
   (`Inc/CppText/UArkAnimNodeLookAt.h` and the `agentDO:lookataim` block) when the node lands. Everything above it is
   already in place: `FDisLookAtRequest` is ported and counted, and the nine AI classes that would issue a request
   are named in section 3.
3. **`L_Tower_P` is the only mission map this tree survives.** `L_Boyle_Ext_P` dies at 5.85 s and `L_Streets1_P` at
   7.43 s on `check(BoneVisibilityStates.Num() == SkeletalMesh->RefSkeleton.Num())`
   (`Engine/Src/UnSkeletalComponent.cpp:3357`), before any NPC spawner fires and before either tallboy pass runs.
   Until that is fixed, the tallboy passes, agent DI's accessories and material variations and every other piece of
   content the first mission does not carry cannot be exercised at all. It is the cheapest unblocking left in the
   character area.
4. **Agent DE's defect 7 is corrected and the correction has a method.** The controller vtable slot moved between
   the two builds (+300 in 2012, +304 in retail), which is why reading the 2012 PDB's slot numbering answered the
   wrong question. `build/agentDO_work/vslot2.py` enumerates every `mov`/`call`/`jmp` through `[reg+disp]` across a
   whole `.text` and `vslot.py` does it per function; validating the displacement against a slot with known callers
   before trusting a "nothing calls this" answer is what turned the claim around.
5. **The retail tree is shared and it is not a quiet machine.** Three agents were running the game during these
   measurements; another agent's screenshots land in the same `Screenshots/Win32Console` (one early figure of this
   package picked up a file called `dmclick90000001.bmp`), the same `Logs` directory is opened by every run, and a
   run can be killed from outside (`exit 4294967295`). `build/agentDO_work/run.py` now takes a per-run log name,
   filters new screenshots to `apshottime*` and retries; anyone measuring frames here should do the same, and should
   pass `-nogfxui` so the main menu does not land on top of the frame.

## 9. The screenshots

| file | what |
|---|---|
| `build/agentDO/darkvision_after.png` | **the accept shot for the powers**: `L_Tower_P` through dark vision - the world desaturated into Dishonored's yellow-green with the vignette, blue collapsed to 2.38 of 255 |
| `build/agentDO/darkvision_before.png` | the same frame, same binary, `-disdarkvisionpp` absent |
| `build/agentDO/diff_darkvision.png` | the two amplified x4 |
| `build/agentDO/head_before.png` | **agent DI's tilted head, reproduced by one switch** (`-nodislookataim`) |
| `build/agentDO/head_after.png` | **the accept shot for the head**: the same guard, same binary, head level and facing forward |
| `build/agentDO/head_look.png` | the same guard with `-dislookatplayer`: head angled down at the player |
| `build/agentDO/diff_head.png`, `diff_headlook.png`, `diff_after_floor.png`, `diff_darkvision_b.png`, `diff_darkvision_final.png` | the difference figures behind the tables |
