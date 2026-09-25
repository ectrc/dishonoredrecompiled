# Agent R report — Engine convergence: Anim / Material / Particle / Light / Mesh / Texture / SpeedTree (2026-09-27)

Build dir: `build\agentR` (Ninja, Debug, `cmake\toolchain-x86.cmake`, `-DDISHONORED_REAL_LAUNCH=ON`,
**`-DDISHONORED_LAYOUT_CHECKS=OFF`** since the second Engine build: `Engine/Inc/DishonoredLayouts.h:419` asserts
`FOnlinePlayerScore` = 16 and agent S's in-flight change to that struct made every Engine unit fail with it; the
coordinator regenerates the header after the merge).
Logs: `build\agentR\probe_build*.log`, `engine_build*.log`, `smoke*.log`. Helper scripts (not repo tools, kept in
`build\agentR`): `patch_hand1..5.py` / `patch_deprecated*.py` (the hand edits as exact-string patches),
`check_bits.py` (bitfield mask order vs the dump), `size_check.py` (sizeof vs `size_2013` for my headers),
`shim_uses.py` → `shim_uses.txt` / `shim_table.md`, `tail_members.py`.

**Target = retail 2013**: reflected offsets from the CodeRed SDK dump (`retail_sdk_layout.json`, "retail SDK @"),
sizes from `native_class_sizes.csv` column `size_2013` ("retail sizeof"). The **2012 Shipping PDB** ("2012 PDB @")
only names the native-only members that sit in the dump's gaps and gives the base lists. Every number below
says which build it comes from.

## Result

| Check | Before (coord probe, 2026-09-27) | After (`build\agentR\layout_probe.txt`) |
|---|---:|---:|
| `xcheck_sdk_layout.py --header EngineAnimClasses.h` | 39 of 101 differ | **0** (exit 0) |
| `--header EngineMaterialClasses.h` | 15 of 34 | **0** |
| `--header EngineParticleClasses.h` | 9 of 132 | **0** |
| `--header EngineLightClasses.h` | 10 of 30 | **0** |
| `--header EngineMeshClasses.h` | 2 of 10 | **0** |
| `--header EngineTextureClasses.h` | 1 of 14 | **0** |
| `--header EngineSpeedTreeClasses.h` | 2 of 6 | **0** |
| `--header UnActorComponent.h` (`ULightComponent`) | 1 of 5 | **0** |
| sizeof vs `size_2013` for every class declared in the 7 headers + `ULightComponent` (`size_check.py`) | 95 mismatches | **0** |
| `gen_layout_probe.py compare` | 1639 exact, 0 contract mismatches | **1661 exact, 0 contract mismatches** (the tool rewrites `reference_layout_delta.md`; I restored the file and kept my copy in `build\agentR\reference_layout_delta.agentR.md`) |
| `DishonoredGame` (Engine + Launch) | | **links** (snapshot build, see below) |
| `build_and_smoke.py --no-build` milestone 1 | | **exit 0** (`Init: Object subsystem initialized`) |

The contract types in my headers (`UTexture`, `UTexture2D`, `UMaterial*`, `UAnimSequence`, `UAnimSet`,
`UStaticMesh`, `USkeletalMesh`, `USkeletalMeshComponent`) were not touched and stay exact.

## Method

1. Base classes first, with `sdk_props.py` (retail offsets; 2012 PDB names for the gaps): `UAnimNode`,
   `UMaterialExpression`, `ULightComponent` (hand, native header), `UParticleModuleRequired`, `UParticleSystem`,
   `UParticleSystemComponent`; then every child the delta listed. The tool's "converge the base first" notes were
   only about verification: its blocks use absolute retail offsets, so all blocks were regenerated in one pass and
   verified together on the rebuilt probe.
2. The offset delta rows are not the whole story: `xcheck` cannot see bitfields or members our header lacks under
   another name, and it does not compare `sizeof`. Two extra sweeps found 33 more classes in my headers:
   `size_check.py` (sizeof vs `size_2013`: 95 classes, 33 of them not in the delta rows, e.g.
   `UParticleModuleLocationBoneSocket` 116 vs 112, `UMaterialInstanceTimeVarying` 276 vs 264, `ULightFunction`
   76 vs 72) and `check_bits.py` (bitfield mask order vs the dump: `FRandomAnimInfo`, `FParameterValueOverTime`,
   `USkelControlBase`, `UParticleModuleTypeDataMesh` were missing a retail bit or had reference-only bits before
   a retail one). All fixed; what `check_bits.py` still lists is name-only (below).
3. Probe: `gen_layout_probe.py generate Engine` is **not** needed and must not be run (it regenerates the shared
   `source/Tests/LayoutProbe/*` for the current `types.json`; I reverted that). `cmake --build build\agentR --target
   LayoutProbe` recompiles the checked-in units against the edited headers.

## Per class: what changed

### Tool-regenerated `//## BEGIN PROPS` blocks (`sdk_props.py Engine <Class>`, 91 blocks)

- `EngineAnimClasses.h` (36): ASkeletalMeshActor, ASkeletalMeshActorBasedOnExtremeContent, ASkeletalMeshActorMAT,
  UAnimationCompressionAlgorithm_RemoveLinearKeys, UAnimationCompressionAlgorithm_PerTrackCompression,
  UAnimNotify_PlayParticleEffect, UAnimNotify_Trails, UAnimNode, UAnimNodeBlendBase, UAnimNode_MultiBlendPerBone,
  UAnimNodeAimOffset, UAnimNodeBlend, UAnimNodeAdditiveBlending, UAnimNodeBlendPerBone, UAnimNodeCrossfader,
  UAnimNodeSequence, UAnimNodePlayCustomAnim, UAnimNodeBlendDirectional, UAnimNodeBlendList, UAnimNodeBlendByBase,
  UAnimNodeBlendByProperty, UAnimNodeBlendBySpeed, UAnimNodeRandom, UAnimNodeBlendMultiBone, UAnimNodeMirror,
  UAnimNodeScalePlayRate, UAnimNodeScaleRateBySpeed, UAnimNodeSlot, UAnimNodeSynch, UAnimTree,
  UAnimNodeSequenceBlendBase, UAnimNodeSequenceBlendByAim, USkelControlBase, USkelControlLimb,
  USkelControlFootPlacement, USkelControlLookAt.
- `EngineMaterialClasses.h` (16): UMaterialExpression (+`Compound` @68, 2012 PDB name for the dump's 4-byte gap),
  …ComponentMask, …DynamicParameter, …FontSample, …FontSampleParameter, …Parameter, …ScalarParameter,
  …StaticSwitchParameter, …StaticComponentMaskParameter, …VectorParameter, …TextureCoordinate, …TextureSample,
  …TextureSampleParameter, …AntialiasedTextureMask, …TextureSampleParameterNormal, UMaterialInstanceTimeVarying.
- `EngineParticleClasses.h` (21): UParticleSystem, UParticleSystemComponent (+`ViewMBInfoArray` @508 from the 2012
  PDB for the dump's 12-byte gap, +`m_pTimeBoundActor` @688), UParticleEmitter, UParticleModule,
  UParticleModuleCollision, …EventReceiverSpawn, …KillBox, …KillHeight, …Location, …LocationBoneSocket,
  …LocationPrimitiveCylinder, …ParameterDynamic, …Required, …SizeMultiplyVelocity, …SpawnPerUnit, …SubUV,
  …SubUVMovie, …TypeDataMesh, …TypeDataMeshPhysX, …TypeDataRibbon, UPhysXParticleSystem.
- `EngineLightClasses.h` (11): UDirectionalLightComponent, UDominantDirectionalLightComponent, UPointLightComponent,
  UDominantPointLightComponent, USpotLightComponent, UDominantSpotLightComponent, USkyLightComponent,
  USphericalHarmonicLightComponent, UDynamicLightEnvironmentComponent, UParticleLightEnvironmentComponent,
  ULightFunction.
- `EngineMeshClasses.h` (3): UApexAsset, UApexGenericAsset, UInstancedStaticMeshComponent (+`PerInstanceData_DEPRECATED`
  @572, `PerInstanceSMData` @584, `CachedMappings` @604 from the 2012 PDB for the dump's gaps).
- `EngineTextureClasses.h` (2): UTexture2DComposite, UTextureMovie.
- `EngineSpeedTreeClasses.h` (2): USpeedTreeActorFactory, USpeedTreeComponentFactory (their rows depended on agent S's
  `UActorFactory` / `UPrimitiveComponentFactory` fix, which landed while I worked: both are exact now).

Native tails reported by the tool (retail sizeof > span end): all are 16-byte alignment padding of
`UAnimNode`-derived classes (`FBoneAtom CachedRootMotionDelta` @160 aligns the class to 16: 204→208, 221→224,
228→240, 276→288, …), of `ULightComponent` children (`FMatrix` members: 424→432, 456→464, 548→560, 580→592,
652→656, 764→768) and of `UMaterialExpressionAntialiasedTextureMask` (157→160); the 2012 PDB has no member past the
last reflected one in any of them, and the probe confirms every sizeof.

### Hand edits (`patch_hand1..5.py`)

| Type | Header | Change | Evidence |
|---|---|---|---|
| `ULightComponent` | `UnActorComponent.h` | data block rewritten in the order of `sdk_props.py --print`; added `LightEnv_BouncedLightBrightness` @268, `LightEnv_BouncedModulationColor` @272, `bOnlyAffectSameAndSpecifiedLevels` (mask 0x1000), `bUseVolumes` (0x4000), `OtherLevelsToAffect` @284, `InclusionVolumes` @300, `ExclusionVolumes` @312, `InclusionConvexVolumes` @324, `ExclusionConvexVolumes` @336 (`TArrayNoInit<FConvexVolume>`: the dump says `TArray<FPointer>`, the 2012 PDB names the element); reference-only `bUseImageReflectionSpecular`, `bExplicitlyAssignedLight`, `bAllowCompositingIntoDLE`, `ReflectionSpecularBrightness` shimmed | retail SDK span 81..424, sizeof 432 (2012 PDB 432, same offsets) |
| `FAnimBlendChild` | Anim | +`INT bHasRootMotion` @20, +`FBoneAtom RootMotion` @32, +`m_bIsAnimSeq` (mask 0x4), `DrawY` unconditional @68 | retail span 0..72 (2012 PDB 80, 16-aligned; ours 80) |
| `FAnimBlendInfo` / `FAnimInfo` | Anim | `FAnimInfo` +`FPointer m_pLocomotionState` @16 (2012 PDB `FLocomotionState*`), so `FAnimBlendInfo` grows to 32 | retail spans 0..20 / 0..32 |
| `FAnimNotifyEvent` | Anim | `Comment` no longer editor-only, +`FLOAT m_fRange` @20 | retail span 0..24, 2012 PDB 24 |
| `FTrailSample` / `FTrailSamplePoint` | Anim | `SecondEdgeSample` before `ControlPointSample` | retail @16/@28 and @28/@52, 2012 PDB same |
| `FRandomAnimInfo` | Anim | +`m_bPickAgainOnBecomeRelevant:1` (mask 0x2 @20) | retail dump, 2012 PDB @20 |
| `FAnimNodeForSearch`, `FAnimTree_EditorOnly` | Anim | new structs (12 / 136 bytes) for `UAnimTree::m_lAnimNodeFastSearch` @476 and `EditorOnlyInfo` @284; retail member names (`PreviewSkelMesh` … where the 2012 PDB says `*_DEPRECATED` at the same offsets) | retail dump structs, 2012 PDB sizes |
| `FParticleSysParam` | Particle | `Scalar_Low` / `Vector_Low` removed (shims) | retail span 0..40, 2012 PDB 40 |
| `FSourceTexture2DRegion` | Texture | `DestOffsetX` / `DestOffsetY` removed (shims) | retail span 0..20, 2012 PDB 20 |
| `FParameterValueOverTime` | Material | +`m_bUseRealTime:1` (mask 0x2 @44) | retail dump, 2012 PDB @44 |
| `UMaterialExpressionStaticSwitchParameter` | Material | base changed from the reference-only `UMaterialExpressionStaticBoolParameter` (in neither build) to `UMaterialExpressionParameter`; `InstanceOverride` typed `const FStaticSwitchParameter*` as in 2012; `SetStaticParameterOverrides` / `ClearStaticParameterOverrides` declared on the class and their bodies copied in `MaterialExpressions.cpp` (they were inherited from the removed base) | retail SDK super `UMaterialExpressionParameter`, span 120..184; 2012 PDB bases `['UMaterialExpressionParameter']`, 184 |
| `UMaterialInstanceTimeVarying` | Material | `ScalarParameterValues` / `TextureParameterValues` / `VectorParameterValues` kept as the native tail after `//## END PROPS` (the tool wanted to shim them); only `LinearColorParameterValues` shimmed | 2012 PDB @228/@240/@252, retail sizeof 264 = span end 228 + 36 |
| `USphericalHarmonicLightComponent`, `UDominantDirectionalLightComponent` | Light | the tool's `UnknownData00[8]` @424 / @456 removed: they are the 16-byte alignment padding before `FSHVectorRGB` (2012 PDB @432) / `FDominantShadowInfo` (FMatrix members, 2012 PDB @464) | 2012 PDB member offsets, probe |

### `*_DEPRECATED` renames (retail dropped the suffix, same offset)

The dump names these members without `_DEPRECATED`; the 2012 PDB / reference have the suffix at the same
offset. The tool emitted the retail name as storage and shimmed the old name, which the probe cannot link
(`static` in the probe). Storage keeps the retail name, the `PostLoad` conversion code uses it, shim dropped:
`UAnimNotify_Trails::SampleTimeStep` @100, `TrailSampleData` @104 (`UnSkeletalAnim.cpp:3031-3049`),
`UAnimTree::PrioritizedSkelBranches` @236 (`UnAnimTree.cpp:3548`), `USkelControlBase::ControlPosX/Y` @176/@180
(`UnSkelControl.cpp:369-374`, also the `VERIFY_CLASS_OFFSET_NODIE` line), `UParticleModuleTypeDataMesh::
CameraFacingUpAxisOption` @78 (`UnParticleModules.cpp:2569`), and the bitfields `ASkeletalMeshActor::
bCollideActors_OldValue` (`UnSkeletalMesh.cpp:2853`), `USkelControlBase::bEnableEaseInOut` (`UnSkelControl.cpp:352/357`),
`UParticleSystem::bLit` (`UnParticleComponents.cpp:3013`), `UParticleModuleRequired::bRequiresSorting`
(`UnParticleModules.cpp:993`). Each line carries `// DISHONORED(layout)`.

## Engine/Src edits (all minimal, tagged `// DISHONORED(layout)`)

`UnAnimTree.cpp` (1 line), `UnSkeletalAnim.cpp` (5), `UnSkelControl.cpp` (5), `UnParticleModules.cpp` (2),
`UnParticleComponents.cpp` (1), `UnSkeletalMesh.cpp` (1), `MaterialExpressions.cpp` (+18: the two override hooks of
`UMaterialExpressionStaticSwitchParameter`). No other Engine source needed a change: every other removed member is
a `DISHONORED_SHIM_STATIC`.

## UnknownData placeholders left

None. The two the tool emitted (`USphericalHarmonicLightComponent` @424, `UDominantDirectionalLightComponent` @456)
were alignment padding, not members.

## Remaining differences (name-only, not layout)

- `check_bits.py` still reports: `UMaterial` bits `bAllowFog_DEPRECATED` / `bUsedWithFogVolumes_DEPRECATED` /
  `bUsedWithFracturedMeshes_DEPRECATED` / `bIsFallbackMaterial_DEPRECATED` and `UTexture::CompressionNoMipmaps_DEPRECATED`
  sit at the retail mask positions of `bAllowFog` … `CompressionNoMipmaps` (contract types, agent M's names from the
  2012 PDB; not renamed to keep the contract headers untouched). `FTextureGroupContainer` has two reference-only bits
  (`TEXTUREGROUP_ImageBasedReflection`, `_Bokeh`, enum values 26/27 absent in retail) inside a 4-byte DWORD (no layout
  effect). `FLightingChannelContainer` / `FRenderingChannelContainer` differ only in the dump's numbering of duplicate
  names (`Unnamed01` vs `Unnamed_1`).
- `UMorphNodeBase`, `UMorphNodeWeightByBoneAngle`, `UMorphNodeWeightByBoneRotation`: the dump lists them with no
  members and `native_class_sizes.csv` has no retail size (not native in the retail exe); left as the reference has
  them. `UMaterialExpressionStaticBoolParameter` is reference-only too (in neither build) and still has an
  `IMPLEMENT_CLASS` in `MaterialExpressions.cpp:62`: a registrant retail does not have (follow-up for T / the
  Engine registrant list).
- Classes not reachable from the probe (in `probe_skip_Engine.txt`) were not checked; nothing was added to the skip
  list.

## Shimmed reference-only members (porting TODOs)

132 shims in my classes (`build\agentR\shim_uses.txt`, columns: header, class, member, use count, files), 120 of
them referenced from `Engine/Src`. The largest by use count (the counts are identifier hits, so common names such
as `Material`, `Group`, `Function`, `Materials`, `EndTime` are inflated by unrelated code):

| Class | Members | Real uses worth noting |
|---|---|---|
| `UAnimTree` | 27 (AnimTreeTemplate, bEnablePooling, bUseSavedPose, bBeingEdited, bRebuildAnimTickArray, RootMorphNodes, SavedPose, MorphConnDrawY, PreviewPlayRate, Preview*/Socket*_DEPRECATED, PreviewMeshList/Index, PreviewSocketList/Index, PreviewAnimSetList/ListIndex/Index, PreviewCam*, PreviewFloor*, AnimNodeFrames) | `UnAnimTree.cpp` (51 hits), `UnSkeletalComponent.cpp` (32: `AnimTreeTemplate`, `SavedPose`, `bRebuildAnimTickArray` pooling/saved-pose paths), `UnPawn.cpp`, `UnSkeletalMesh.cpp`, `UnInterpolation.cpp`. Retail keeps the editor data in `FAnimTree_EditorOnly EditorOnlyInfo` @284 instead. |
| `UParticleModuleRequired` | 13 (bAllowImageFlipping, bSquareImageFlipping, bEnable{Near,Far}ParticleCulling, bOverrideSystemMacroUV, bOrbitModuleAffectsVelocityAlignment, Near/Far Cull/Fade Distance, MacroUVPosition/Radius) | `UnParticleSystemRender.cpp` (distance culling, macro UV), `UnParticleModules.cpp`, `ParticleEmitterInstances.cpp` |
| `UAnimNodeSequence` | 11 (bLoopCameraAnim, bRandomizeCameraAnimLoopStartTime, bCheckForFinishAnimEarly, bBlendingOut, EndTime, CameraAnim, ActiveCameraAnimInstance, CameraAnimScale/PlayRate/BlendInTime/BlendOutTime) | `UnAnimPlay.cpp` (camera-anim-on-anim feature, 24+19 hits): retail has no camera anim on anim nodes |
| `UMaterialExpression` | 8 (EditorX/Y_DEPRECATED, bShowInputs, bShowOutputs, Material, Function, BorderColor, Outputs) | `MaterialExpressions.cpp`, `UnMaterial.cpp`, `UnLinkedObjDrawUtils.cpp` (editor drawing); `Material`/`Function` back-pointers are the 2013-removed owner links |
| `UParticleSystemComponent` | 7 (LightEnvironmentSharedInstigator, MaxLightEnvironmentPooledReuses, bHasBeenActivated, bSkipBoundsUpdate, WarmupTickRate, EditorDetailMode, AttractorCollisionEvents) | `UnParticleComponents.cpp`, `UnScript.cpp`, `ParticleEmitterInstances.cpp` |
| `ASkeletalMeshActor` | 7 (bShouldDoAnimNotifies, bShouldShadowParentAllAttachedActors, FacialAudioComp, ReplicatedMaterial0/1, InterpGroupList, …) | `UnSkeletalMesh.cpp`, `UnPawn.cpp`, `UnAnimPlay.cpp`; retail has one `ReplicatedMaterial` @600 and `InterpGroupList` only on `ASkeletalMeshActorMAT` @632 |
| `UParticleModuleCollision` | 6 (bApplyPhysics, bCollideOnlyIfVisible, bCollideWithWorld, bCollideWithWorldAttractors, MaxCollisionDistance, ParticleAttractorCollisionActions) | `ParticleModules_Collision.cpp`, `UnParticleComponents.cpp` |
| `USkelControlLookAt` / `USkelControlLimb` | 5 / 4 (ActorSpaceLookAtTarget, RotationAngleRange*, ControlBoneIndex / JointOffsetSpace, JointOffset, JointOffsetBoneName, bRotateJoint) | `UnSkelControl.cpp` (15 + 10 hits) |
| `UAnimNode` | 5 (bTickDuringPausedAnims, bEditorOnly, NodeEndEventTick, CachedCurveKeys, LastUpdatedAnimMorphKeys) | `UnAnimPlay.cpp`, `UnAnimTree.cpp` (curve keys / morph keys) |
| `ULightComponent` | 4 (bUseImageReflectionSpecular, bExplicitlyAssignedLight, bAllowCompositingIntoDLE, ReflectionSpecularBrightness) | `LightComponent.cpp`, `LightSceneInfo.cpp`, `DynamicLightEnvironmentComponent.cpp`, `BasePassRendering.cpp`, `PreviewScene.cpp`, `TranslucentRendering.cpp` |
| `UDynamicLightEnvironmentComponent` | 4 (VelocityUpdateTimeScale, bAffectedBySmallDynamicLights, bShadowFromEnvironment, bAlwaysInfluencedByDominantDirectionalLight) | `DynamicLightEnvironmentComponent.cpp` |
| `UApexAsset` / `UApexGenericAsset` | 4 / 1 (OriginalApexName, NamedReferences, SourceFilePath, SourceFileTimestamp / Materials) | `NvApex*.cpp`; note `UApexGenericAsset::MApexAsset` is `FPointer` @68 in retail (72 bytes) |
| `UParticleLightEnvironmentComponent` | 3 (NumPooledReuses, SharedInstigator, SharedParticleSystem) | `DynamicLightEnvironmentComponent.cpp`, `UnParticleComponents.cpp` |
| others | `UAnimNodeSlot` (3), `UPhysXParticleSystem` (2), `UParticleSystem` (WarmupTickRate), `UAnimNotify_Trails` (SampledSkeletalMesh, bPreviewForceExplicit), `UAnimNotify_PlayParticleEffect` (PSNonExtremeContentTemplate, BoneSocketModuleActorName), `FParticleSysParam` (Scalar_Low, Vector_Low: `UnParticleComponents.cpp:6920-7150`), `FSourceTexture2DRegion` (DestOffsetX/Y: `Texture2DComposite.cpp:378`), `UPointLightComponent::ShadowPlane`, `USpotLightComponent::LightShaftConeAngle`, `UParticleModuleTypeDataMeshPhysX::ZOffset`, `UMaterialExpression*Parameter::Group`, `UMaterialExpressionTextureSample::TextureObject`, `UAnimationCompressionAlgorithm_RemoveLinearKeys::EffectorDiffSocket`, `UMaterialInstanceTimeVarying::LinearColorParameterValues`, and the particle-module bits from the bitfield sweep (`UParticleModule::bSupportsRandomSeed/bRequiresLoopingNotification`, `UParticleModuleKillBox::bAxisAlignedAndFixedSize`, …) | see `shim_uses.txt` |

Full table: `build\agentR\shim_table.md` (132 rows).

## Build and smoke

- **Probe**: after the coordinator's generator fix (shims report `MISSING` instead of being addressed) I ran
  `gen_layout_probe.py generate Core Engine D3D9Drv` once as asked (24 files under `source/Tests/LayoutProbe` changed in
  the tree, left for the coordinator) and rebuilt `LayoutProbe` in `buildgentR` (`engine_build6.log`, probe written
  13:03): all checks above are from that probe.
- **Engine + Launch**: the working tree does not build at the moment for reasons outside my files: agent T's in-flight
  `GFxUI/Inc/GFxUIClasses.h` / `GFxUIEngineShims.h` break `UnPlayer.cpp`, `WinDrv/WinViewport.cpp` and
  `LaunchEngineLoop.cpp` (`engine_build3..6.log`: 116 of 119 Engine units compile, only those three fail, none with an
  error in my headers), and for a while T's `Launch/CMakeLists.txt` linked the `DishonoredGame` exe into itself (CMake
  error, `engine_build4.log`; fixed in HEAD since). So, like agents N and P, the exe was built from a **snapshot**:
  `buildgentR\sync_snapshot.py` extracts `git archive HEAD` (S's commit `921742f` is HEAD, so S's headers and its
  half of `UnActorComponent.h` are in) and copies my 15 owned files over it; `buildgentR_snap_build.cmd` configures
  `buildgentR_snap` from `buildgentR\snap\src` (`-DDISHONORED_LAYOUT_CHECKS=OFF`, see the header of this report)
  and builds `DishonoredGame`: `snap_build1.log`, 638 units, **0 errors, exe linked** (`buildgentR_snap\Binaries  Win32\DishonoredGame.exe`). No snapshot-only bridge was needed.
- **Smoke**: `python resources/tools/build_and_smoke.py --build-dir build/agentR_snap --no-build --stage build/agentR/stage`
  → **exit 0**, `Init: Object subsystem initialized` reached (`buildgentR\smoke1.log`; private stage dir so the shared
  `build\stage` is not touched). The golden diff is the known milestone-1 set (ini load order, `-nullrhi` command line,
  WinSock line), nothing new.
- Re-running the updated `sdk_props.py --dry-run` over all 91 regenerated classes (`buildgentRecheck_dry.txt`)
  gives no `TScriptInterface` folds and no "reference member the 2012 PDB has but retail lacks" flags in my classes;
  its two "2012 members past the last reflected one, delete rather than shim" notes are `USkelControlBase`
  (`*_DEPRECATED` names: already deleted, they are the retail-named storage) and `UMaterialInstanceTimeVarying`
  (kept as the native tail on purpose: retail sizeof 264 = 228 + the three 12-byte arrays the 2012 PDB puts at
  228/240/252; deleting them would make the class 228).

## Follow-ups (outside my files)

1. `DishonoredLayouts.h`: regenerate after the merge (my build has the asserts off because of the
   `FOnlinePlayerScore` assert, agent S's struct).
2. `UMaterialExpressionStaticBoolParameter` (`IMPLEMENT_CLASS` in `MaterialExpressions.cpp:62`) is a native class
   retail does not have; drop it from the Engine registrants when the material editor code goes, or keep it
   unregistered.
3. The `UAnimTree` editor state now lives in `FAnimTree_EditorOnly EditorOnlyInfo` (retail); `UnAnimTree.cpp` /
   `UnSkeletalComponent.cpp` still write the shimmed flat members (`AnimTreeTemplate`, `SavedPose`, pooling): the
   anim-tree port has to route them.
4. Camera-anim-on-`UAnimNodeSequence` (`UnAnimPlay.cpp`) is reference-only code with no retail storage; remove in the
   anim port.
5. `probe_skip_Engine.txt` untouched; `reference_layout_delta.md` restored to HEAD after the compare runs;
   `source/Tests/LayoutProbe/*` regenerated once with the fixed generator (coordinator's request).
6. `UnActorComponent.h` carries both agent S's (`UActorComponent` / `UPrimitiveComponent` / `UMeshComponent` bitfields)
   and my (`ULightComponent`) edits; commit it once for both.
