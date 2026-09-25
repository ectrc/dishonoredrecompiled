# Agent Q report — Engine convergence: Interpolation / Sequence / Camera / Decal / LensFlare / Physics (2026-09-25)

Build dir `build\agentQ` (Ninja, Debug, `cmake\toolchain-x86.cmake`, `-DDISHONORED_REAL_LAUNCH=ON`,
`DISHONORED_LAYOUT_CHECKS` left ON). Logs: `build\agentQ\probe_build*.log`, `engine_build*.log`,
`build\agentQ\smoke\`. Patch scripts: `build\agentQ\patch1.py`, `patch2.py`; shim table:
`build\agentQ\shim_table.py` -> `shim_table.txt`.

**Target = retail 2013** (`retail_sdk_layout.json` offsets via `sdk_show.py` / `sdk_props.py`,
`native_class_sizes.csv` sizes). The 2012 Shipping PDB (`types.json`) is evidence only; every number
below says which build it comes from.

## Result

| Check | Before (build/coord, 2026-09-25 05:07) | After |
|---|---|---|
| `xcheck_sdk_layout.py --header` (6 headers) | 40 rows | **1 row** (`AKActor`, root cause outside my files, see "Remaining") |
| `gen_layout_probe.py compare` contract mismatches | 0 | 0 (probed 1905, exact 1565) |
| Engine + Launch build | — | `DishonoredGame.exe` links (`build\agentQ\snap_build3.log`, 0 errors) |
| `build_and_smoke.py --build-dir build/agentQ/snap --no-build` | milestone 1 | **exit 0**, `Init: Object subsystem initialized` reached (`build\agentQ\smoke1.log`) |

Measured on **`build\agentQ\snap`**: a `git archive HEAD` snapshot (agents P and S committed) plus my six
headers (`build\agentQ\sync_snapshot.py`, same approach as agents N/P). Reason: the working tree could
not produce a `DishonoredGame.exe` or a `LayoutProbe` while I ran — `Launch/Src/LaunchEngineLoop.cpp`
(agent O, uncommitted) fails on `Core.h(739)` (`Logf<EPlatformType>`), `GFxUI/Inc/GFxUIClasses.h` (agent T)
was mid-rewrite, and agent T's regenerated `source/Tests/LayoutProbe/CMakeLists.txt` references the
`DishonoredGame`/`GFxUI`/`AkAudio`/`OnlineSubsystemSteamworks` targets that are OFF in `build\agentQ`.
The snapshot probe (`build\agentQ\snap\layout_probe.txt`, copied to `build\agentQ\layout_probe.txt`) is
what the numbers above come from; the last working-tree probe (`probe_build5.log`, 05:45) agreed on my
headers before the final round of fixes. `DISHONORED_LAYOUT_CHECKS=OFF` in both configs: the existing
`Engine/Inc/DishonoredLayouts.h` assert `FOnlinePlayerScore == 16` (agent S's row) broke every Engine TU with
it on (`full_build1.log`, 500 hits).

Note for the coordinator: the snapshot's `src/external` is its own FetchContent cache. A first attempt with a
junction to the repo's `external/` rewrote the shared `*-subbuild` CMakeCache paths (`probe_build7.log`); I
deleted the four `external/*-subbuild` dirs so the next configure of any build dir recreates them — the
`*-src` clones and `*-build` outputs are untouched.

## Method

1. Baseline: `xcheck_sdk_layout.py build/agentQ/layout_probe.txt --header <6 headers>` -> 40 rows
   (same as `build/coord`).
2. `sdk_props.py Engine <39 classes>` regenerated every `//## BEGIN PROPS` block of the delta from the
   retail offsets (bases first is irrelevant for the text; it matters for the probe result only, and the
   only bases in my set — `UInterpGroup`, `UInterpGroupInst`, `AKActor`, `ACamera`, `URB_BodyInstance` —
   were regenerated in the same run). Native gaps were filled from the 2012 PDB by relative position.
3. Hand edits (`patch1.py` .. `patch5.py`), listed per class below.
4. Probe rebuild, `xcheck_sdk_layout.py --header`, `gen_layout_probe.py compare`. The xcheck only flags
   members off their retail offset or a span not reached, so the classes whose sizeof merely *exceeds* retail
   came from `compare` (`reference_layout_delta.md` rows of my headers): `USeqAct_Latent`, `FTViewTarget`,
   `FDirectorTrackCut`, `FFaceFXSoundCueKey`, `UInterpTrackInstBoolProp`, `UInterpTrackInstDirector`,
   `UInterpTrackInstFloatProp`, `USeqAct_PlayFaceFXAnim` — regenerated / hand-fixed the same way.
5. Engine + Launch build, `build_and_smoke.py --no-build` at milestone 1.

## Per class

Column "how": `tool` = `sdk_props.py` block regeneration only; `tool+hand` = block plus a hand edit
described in the notes. "ours before" is the `build/coord` probe (2026-09-25), "retail" is
`native_class_sizes.csv` size_2013; the reflected member offsets are the SDK dump's.

### EngineInterpolationClasses.h (12 rows)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| UInterpGroup | 100 -> 112 | 112 | tool | already inherited `FInterpEdInputInterface` (vptr @56 in retail and 2012); added `GroupAnimSetsPawn` @96 (retail SDK = 2012 PDB) and the five Arkane bits `bBecomeDynamic..bBackupTransformOnLoop` @108 (retail masks 0x20..0x200) |
| UInterpGroupInst | 80 -> 112 | 112 | tool+hand | +`m_pGroupPawn` @64, `BackupCollisionType` @80, `m_InterpGroupType` @81, `m_CurrentRootMotionMode` @82 (retail name; the 2012 PDB calls the byte `RootMotionMode`), `m_ActorSavedPosition` @84, `m_ActorSavedRotation` @96, `m_bHasAnimControlTrack` @108 mask 0x1, `m_bShouldBackupTransform` mask 0x2 (retail-only). Reference-only `CachedCamOverridePostProcess` shimmed |
| UInterpGroupAI | 116 -> 128 | 128 | tool | +`LockRequests`, `CanInterruptMatinee`, `bDisableHitReact`, `bShouldFakeDeath` (retail-only), `bDisableJiggleBones`, `bShouldBeDetached` (retail-only) @120, `LookatRequestsPriority` @124; 3-byte tail = alignment. Six reference-only members shimmed |
| UInterpGroupDirector | 100 -> 112 | 112 | tool (base) | no own members |
| UInterpGroupInstAI | 100 -> 148 | 148 | tool | +`GroupAnimSetsPawn` @112, `m_MeshTranslationMode` @137, `bAlreadyTerminated..bHaveMoveTrack` @140, `m_iLODRequestID` @144 (retail = 2012); `bSavedCollideActors`/`bSavedBlockActors` shimmed |
| UInterpGroupInstDirector | 80 -> 112 | 112 | tool (base) | no own members |
| UInterpTrackAnimControl | 180 -> 168 | 168 | tool | reference-only `AnimSets`, `bSkipAnimNotifiers` shimmed |
| UInterpTrackFloatMaterialParam | 172 -> 168 | 168 | tool | `Material_DEPRECATED` shimmed |
| UInterpTrackInstAnimControl | 92 -> 112 | 112 | tool | +`OffsetForLoopWhileSyncing` @68, `bAnimIsLooping` @72, `LoopStart` @76, `LoopEnd` @80, `m_ChannelIndex` @84 (retail = 2012) |
| UInterpTrackMove | 192 -> 196 | 196 | tool | +`fLODDistance` @188 (retail = 2012); `bUseRawActorTMforRelativeToInitial` shimmed; 2-byte tail = alignment |
| UInterpTrackToggle | 140 -> 144 | 144 | tool | +`BYTE TermInstAction` @136 (retail = 2012); `bActivateWithJustAttachedFlag` shimmed |
| UInterpTrackVectorMaterialParam | 172 -> 168 | 168 | tool | `Material_DEPRECATED` shimmed |
| UInterpTrackInstBoolProp | 84 -> 80 | 80 | tool (compare row) | reference-only `INT BitMask` between `BoolProp` @72 and `ResetBool` @76 (mask 0x1) shimmed; retail = 2012 |
| UInterpTrackInstDirector | 72 -> 68 | 68 | tool (compare row) | reference-only `FRenderingPerformanceOverrides OldRenderingOverrides` shimmed; retail span ends at `OldViewTarget` @64+4 |
| UInterpTrackInstFloatProp | 84 -> 80 | 80 | tool (compare row) | reference-only `FPointer DistributionProp` shimmed |
| FDirectorTrackCut | 20 -> 16 | 16 (2012 PDB; script struct) | hand | reference-only `INT ShotNumber` after `TargetCamGroup` @8 removed and shimmed (retail SDK span 0..16) |
| FFaceFXSoundCueKey | 4 -> 4 | 4 | hand | member is `class UAkEvent* FaceFXAkEvent` @0 in retail SDK and 2012 PDB; the reference `USoundCue* FaceFXSoundCue` is shimmed (3 uses in UnInterpolation.cpp) |

### EngineSequenceClasses.h (15 rows)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| USeqAct_ActorFactory | 340 -> 332 | 332 | tool | `CurrentSpawnIdx` shimmed |
| USeqAct_Delay | 288 -> 284 | 284 | tool | reference `bDelayActive`/`bStartWillRestart` DWORD kept; offsets only |
| USeqAct_GetLocationAndRotation | 292 -> 280 | 280 | tool | `Rotation` (FVector) shimmed; retail has `Location` @248, `RotationVector` @260, `SocketOrBoneName` @272 |
| USeqAct_Interp | 472 -> 520 | 520 | tool+hand | +28 members (retail = 2012 for all but `m_bShouldNotBeInterruptedByDialog` mask 0x40000, retail-only): the two native `TMap`s `SavedActorTransforms` @264 / `SavedActorVisibilities` @324 were placed into the SDK gap from the 2012 PDB, `DeltaTime` @396, 16 Arkane bits @400, `BlendOutTime` @404, `BlendOutTimeOverride` @408, `m_SkipMode` @412, `m_AIBehaviorPriority` @413, `m_FadeTime` @416, `m_SavedGroupInstData` @436, `m_pDialogTree_RunningInst` @468, `m_MatineeGUID` @472, `ActivatedLinks` @488, `m_OverrideDistractionLoop` @500, `m_pLinkedVolume` @512, `m_ConversationNodePointer` @516. Hand: declared `struct FDistractionLoopOverride` (`Engine.SeqAct_Interp.DistractionLoopOverride`: `LoopName` @0, `LoopCount` @8, 12 bytes in retail SDK and 2012 PDB). Eight reference-only members shimmed |
| USeqAct_Latent | 268 -> 264 | 264 | tool (compare row) | reference-only `FLOAT LatentActivationTime` after `bAborted` @260 shimmed (3 uses UnSequence.cpp:3127-3159); base of every `USeqAct_*` above, so their offsets moved with it |
| USeqAct_LevelStreaming | 288 -> 284 | 284 | tool | |
| USeqAct_LevelVisibility | 284 -> 280 | 280 | tool | |
| USeqAct_MultiLevelStreaming | 288 -> 284 | 284 | tool | |
| USeqAct_PrepareMapChange | 292 -> 288 | 288 | tool | |
| USeqAct_SetDOFParams | 356 -> 352 | 352 | tool | +`FColor ModulateBlurColor` @280, `OldModulateBlurColor` @328 (retail = 2012); `MinBlurAmount`/`OldMinBlurAmount` shimmed |
| USeqAct_SetMatInstScalarParam | 264 -> 272 | 272 | tool | +`m_pMaterialOwnerPawn` @264, `m_iMaterialIndex` @268: retail-only (2012 PDB sizeof 264; `retail_reconciliation.md`) |
| USeqAct_SetMotionBlurParams | 284 -> 280 | 280 | tool | |
| USeqAct_StreamInTextures | 320 -> 308 | 308 | tool | `bLocationBased` @264 mask 0x1 is the retail name of the 2012 bit `bLocationBased_DEPRECATED` (shimmed with `bHasTriggeredAllLoaded`, `StreamingDistanceMultiplier`, `NumWantingResourcesID`) |
| USeqAct_WaitForLevelsVisible | 284 -> 280 | 280 | tool | |
| USeqCond_SwitchObject | 236 -> 240 | 240 | tool | +`UClass* MetaClass` @236 (retail = 2012) |
| USeqAct_PlayFaceFXAnim | 280 -> 280 | 280 | tool (compare row) | `class UAkEvent* AkEventToPlay` @276 (retail = 2012) replaces the reference `USoundCue* SoundCueToPlay` (shimmed; 10 uses UnSkeletalComponent.cpp) |
| USeqEvent_Touch | 312 -> 336 | 336 | tool | +`ProximityTweaks` @296, `IgnoredProximityTweaks` @308 (`TArray<UDisEngineTweaksBase*>`), `m_bIgnorePossessingPawn`, `m_bIgnorePossessedPawn`, `m_bIgnoreBlink` (masks 0x1..0x4), `m_bOnlyDeadOrSleepingPawns` (0x20) @320; `bUseInstigator` shimmed |

### EngineCameraClasses.h (2 rows)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| ACamera | 1328 -> 1040 | 1040 | tool | +`FArkPpConfig m_CamPostProcessSettings` @636 (132 bytes, retail = 2012; struct declared by agent M in EngineClasses.h). The 288 extra bytes were reference-only storage: `FPostProcessSettings CamPostProcessSettings`, `FRenderingPerformanceOverrides RenderingOverrides`, `UCameraAnimInst* AnimInstPool[8]`, `ActiveAnims`, `FreeAnims`, `AnimCameraActor` + 5 bits — all shimmed (none exists in the retail dump or the 2012 PDB) |
| FTViewTarget | 44 -> 40 | 40 (2012 PDB; script struct) | hand | reference-only `APlayerReplicationInfo* PRI` after `AspectRatio` @36 removed and shimmed (retail SDK span 0..40); it sits inside `ACamera::ViewTarget` / `PendingViewTarget`, which is why those two and everything after them were still 4 bytes late after the block regeneration (8 uses UnCamera.cpp:147-180 `CheckViewTarget`) |
| ACameraActor | 944 -> 752 | 752 | tool | `bCamOverridePostProcess` @584 mask 0x2 is the retail name of the 2012 bit `bCamOverridePostProcess_DEPRECATED` (shimmed); +`FArkPpConfig m_CamOverridePostProcess` @600; reference `FPostProcessSettings CamOverridePostProcess` shimmed; 12-byte tail = 16-alignment (2012 PDB has no member after `MeshComp` @736) |

### EngineDecalClasses.h (4 rows)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| ADecalActorBase | 592 -> 592 | 592 | tool+hand | retail SDK has `Decal` @584 (directly after AActor's `LatentActions` 572..584) with no `VfTable_` entry, and the 2012 PDB lists bases `['AActor']` only — so the reference `IEditorLinkSelectionInterface` base (its vptr occupied 584 and pushed `Decal` to 588) is gone, with its `GetUObjectInterfaceEditorLinkSelectionInterface`. `ADecalActor` / `ADecalActorMovable` inherit the fix (592, `Decal` @584). No Engine/Src use of the interface (editor-only) |
| ADecalManager | 640 -> 656 | 656 | tool | +`TArray<UMaterialInstanceConstant*> PoolMICs` @600 (retail = 2012); 12-byte tail = alignment |
| UActorFactoryDecal | 104 -> 92 | 92 | tool (base) | `DecalMaterial` @88 once agent S's `UActorFactory` (+12) lands; with the unconverged base the probe still reports the old offset — see "Remaining" |
| UDecalComponent | 800 -> 752 | 752 | tool | +`OriginalParentRelativeLocation` @724, `OriginalParentRelativeOrientationVec` @736 (retail = 2012); `bDecalMaterialSetAtRunTime`, `StreamingDistanceMultiplier`, `FMatrix ParentRelLocRotMatrix` shimmed (the matrix was the bulk of the 48 extra bytes); the 2-byte gap @674 is the `SCRIPT_ALIGN` after the two enums |

### EngineLensFlareClasses.h (2 rows + 1 struct)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| FLensFlareElement | 324 -> 320 | 320 (2012 PDB; no retail native size: script struct) | hand | reference-only `BITFIELD bOrientTowardsSource` + `SCRIPT_ALIGN` between `Rotation` @124 and `Color` @152 removed (retail SDK and 2012 PDB both have `Color` directly at 152); shimmed inside the struct. `LensFlare.h` keeps its own copy in the native `FLensFlareElementValues` mirror (not reflected, untouched) |
| ULensFlare | 508 -> 500 | 500 | tool | `bUseTrueConeCalculation`, `MinStrength` shimmed |
| ULensFlareComponent | 528 -> 528 | 528 | tool | offsets only (`SourceColor` 488 -> 492 ...); `bHasSeparateTranslucency`, `bUseTrueConeCalculation`, `MinStrength`, `NextTraceTime` shimmed; 8-byte tail = 16-alignment |

### EnginePhysicsClasses.h (4 rows)

| Class | ours before -> after | retail | how | notes |
|---|---:|---:|---|---|
| AKActor | 832 -> 816 | 816 | tool+hand | reference-only `ImpactSoundComponent`, `ImpactSoundComponent2`, `SlideSoundComponent` shimmed. The tool left `BYTE UnknownData00[8]` for the SDK gap @696: removed by hand — it is the 16-byte alignment padding before `FRigidBodyState RBState` @704 (`FQuat` member); the 2012 PDB has no member there and also places `RBState` @704 |
| AKActorFromStatic | 832 -> 832 | 832 | tool | +`m_bInitialized` @816 (retail = 2012); offsets follow the base |
| UPhysicalMaterial | 156 -> 140 | 140 | tool | reference-only `ImpactSound`, `SlideSound`, `FractureSoundExplosion`, `FractureSoundSingle` (`USoundCue*`) shimmed |
| URB_BodyInstance | 152 -> 224 | 224 | tool | +5 Arkane bits @104 (`m_bFrozen` 0x1000 .. `m_bSeveredLimb` 0x10000; `m_bIgnoreNextWakeupEvent` 0x4000 is retail-only), `FQuat m_CurrentRotation` @160, `m_CurrentPosition` @176, `m_ActiveTransformIndex` @188, `m_RealLinearVelocity` @192, `m_RealAngularVelocity` @204 (retail = 2012); the 8-byte gap @152 and the 8-byte tail are the 16-alignment of the `FQuat` |

### `VERIFY_CLASS_OFFSET_NODIE` lines

Eight `VERIFY_CLASS_OFFSET_NODIE(...)` lines in the `#ifdef VERIFY_CLASS_SIZES` blocks named members that are
shims now (`STRUCT_OFFSET` of a static member does not compile once `UnEngine.cpp` defines
`VERIFY_CLASS_SIZES`): commented out with a `// DISHONORED(layout)` note — `UInterpGroupAI::PreviewPawnClass`,
`UInterpGroupInst::CachedCamOverridePostProcess`, `UInterpTrackAnimControl::AnimSets`,
`USeqAct_Latent::LatentActivationTime`, `USeqAct_Interp::ConstantCameraAnimRate`, `ACamera::AnimCameraActor`,
`UDecalComponent::ParentRelLocRotMatrix`, `ULensFlareComponent::NextTraceTime`.

## UnknownData placeholders

None left in the six headers. The only one the tool emitted (`AKActor` @696, 8 bytes) was alignment
padding and is documented in place instead.

## Shimmed reference-only members (`DISHONORED_SHIM_STATIC`) and their Engine/Src uses

Storage-less inline statics: the reference code compiles, reads yield zero, writes are global.
Every use is a porting TODO for the owner of the .cpp. Counts from `build\agentQ\shim_table.txt`
(word matches in `Engine/Src/*.cpp`, restricted to files naming the class for generic names).

| Header | Class | Shim | Engine/Src uses | Files |
|---|---|---|---:|---|
| EngineInterpolationClasses.h | InterpGroupAI | `PreviewPawnClass` | 11 | UnInterpolation.cpp:11 |
| EngineInterpolationClasses.h | InterpGroupAI | `bNoEncroachmentCheck` | 5 | UnInterpolation.cpp:5 |
| EngineInterpolationClasses.h | InterpGroupAI | `bDisableWorldCollision` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpGroupAI | `bIgnoreLegacyHeightAdjust` | 2 | UnInterpolation.cpp:2 |
| EngineInterpolationClasses.h | InterpGroupAI | `bRecreatePreviewPawn` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpGroupAI | `bRefreshStageMarkGroup` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpGroupInst | `CachedCamOverridePostProcess` | 8 | UnInterpolation.cpp:8 |
| EngineInterpolationClasses.h | InterpGroupInstAI | `bSavedCollideActors` | 2 | UnInterpolation.cpp:2 |
| EngineInterpolationClasses.h | InterpGroupInstAI | `bSavedBlockActors` | 2 | UnInterpolation.cpp:2 |
| EngineInterpolationClasses.h | InterpTrackAnimControl | `AnimSets` | 10 | UnEngine.cpp:2 UnInterpolation.cpp:8 |
| EngineInterpolationClasses.h | InterpTrackAnimControl | `bSkipAnimNotifiers` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpTrackFloatMaterialParam | `Material_DEPRECATED` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpTrackMove | `bUseRawActorTMforRelativeToInitial` | 2 | UnInterpolation.cpp:2 |
| EngineInterpolationClasses.h | InterpTrackToggle | `bActivateWithJustAttachedFlag` | 4 | UnInterpolation.cpp:4 |
| EngineInterpolationClasses.h | InterpTrackVectorMaterialParam | `Material_DEPRECATED` | 4 | UnInterpolation.cpp:4 |
| EngineSequenceClasses.h | SeqAct_GetLocationAndRotation | `Rotation` | 16 | UnSequence.cpp:16 |
| EngineSequenceClasses.h | SeqAct_Latent | `LatentActivationTime` | 3 | UnSequence.cpp:3 |
| EngineSequenceClasses.h | SeqAct_ActorFactory | `CurrentSpawnIdx` | 3 | UnSequence.cpp:3 |
| EngineSequenceClasses.h | SeqAct_Interp | `bDisableRadioFilter` | 1 | UnInterpolation.cpp:1 |
| EngineSequenceClasses.h | SeqAct_Interp | `bShouldShowGore` | 12 | UnInterpolation.cpp:10 UnPhysic.cpp:2 |
| EngineSequenceClasses.h | SeqAct_Interp | `LinkedCover` | 2 | UnInterpolation.cpp:2 |
| EngineSequenceClasses.h | SeqAct_Interp | `ReplicatedActorClass` | 4 | UnInterpolation.cpp:4 |
| EngineSequenceClasses.h | SeqAct_Interp | `ReplicatedActor` | 33 | UnActor.cpp:3 UnInterpolation.cpp:27 UnVehicle.cpp:3 |
| EngineSequenceClasses.h | SeqAct_Interp | `RenderingOverrides` | 11 | UnGame.cpp:1 UnInterpolation.cpp:4 UnPlayer.cpp:6 |
| EngineSequenceClasses.h | SeqAct_Interp | `ConstantCameraAnim` | 4 | UnInterpolation.cpp:4 |
| EngineSequenceClasses.h | SeqAct_Interp | `ConstantCameraAnimRate` | 1 | UnInterpolation.cpp:1 |
| EngineSequenceClasses.h | SeqAct_SetDOFParams | `MinBlurAmount` | 9 | DOFAndBloomEffect.cpp:4 UnPlayer.cpp:1 UnPostProcess.cpp:1 UnSequence.cpp:3 |
| EngineSequenceClasses.h | SeqAct_SetDOFParams | `OldMinBlurAmount` | 2 | UnSequence.cpp:2 |
| EngineSequenceClasses.h | SeqAct_StreamInTextures | `bLocationBased_DEPRECATED` | 1 | UnSequence.cpp:1 |
| EngineSequenceClasses.h | SeqAct_StreamInTextures | `bHasTriggeredAllLoaded` | 2 | UnSequence.cpp:2 |
| EngineSequenceClasses.h | SeqAct_StreamInTextures | `StreamingDistanceMultiplier` | 1 | UnSequence.cpp:1 |
| EngineSequenceClasses.h | SeqAct_StreamInTextures | `NumWantingResourcesID` | 4 | UnSequence.cpp:4 |
| EngineSequenceClasses.h | SeqEvent_Touch | `bUseInstigator` | 2 | UnSequence.cpp:2 |
| EngineCameraClasses.h | Camera | `bFadeAudio` | 2 | UnSequence.cpp:2 |
| EngineCameraClasses.h | Camera | `bForceDisableTemporalAA` | 1 | UnPlayer.cpp:1 |
| EngineCameraClasses.h | Camera | `bUseClientSideCameraUpdates` | 0 |  |
| EngineCameraClasses.h | Camera | `bDebugClientSideCamera` | 0 |  |
| EngineCameraClasses.h | Camera | `bShouldSendClientSideCameraUpdate` | 0 |  |
| EngineCameraClasses.h | Camera | `CamPostProcessSettings` | 1 | UnPlayer.cpp:1 |
| EngineCameraClasses.h | Camera | `RenderingOverrides` | 23 | AmbientOcclusionRendering.cpp:1 DirectionalLightComponent.cpp:1 LightRendering.cpp:1 LightShaftRendering.cpp:2 Scene.cpp:3 SceneRendering.cpp:2 ShadowSetup.cpp:1 UberPostProcessEffect.cpp:1 UnGame.cpp:1 UnInterpolation.cpp:4 UnPlayer.cpp:6 |
| EngineCameraClasses.h | Camera | `AnimInstPool` | 0 |  |
| EngineCameraClasses.h | Camera | `ActiveAnims` | 16 | UnCamera.cpp:16 |
| EngineCameraClasses.h | Camera | `FreeAnims` | 3 | UnCamera.cpp:3 |
| EngineCameraClasses.h | Camera | `AnimCameraActor` | 7 | UnCamera.cpp:7 |
| EngineCameraClasses.h | CameraActor | `bCamOverridePostProcess_DEPRECATED` | 2 | UnCamera.cpp:2 |
| EngineCameraClasses.h | CameraActor | `CamOverridePostProcess` | 8 | UnCamera.cpp:3 UnInterpolation.cpp:5 |
| EngineDecalClasses.h | DecalComponent | `bDecalMaterialSetAtRunTime` | 1 | DecalComponent.cpp:1 |
| EngineDecalClasses.h | DecalComponent | `StreamingDistanceMultiplier` | 7 | DecalComponent.cpp:2 LandscapeRender.cpp:1 UnSkeletalComponent.cpp:2 UnSkeletalMesh.cpp:2 |
| EngineDecalClasses.h | DecalComponent | `ParentRelLocRotMatrix` | 3 | DecalComponent.cpp:3 |
| EngineLensFlareClasses.h | LensFlare | `bUseTrueConeCalculation` | 7 | LensFlare.cpp:4 LensFlareRendering.cpp:3 |
| EngineLensFlareClasses.h | LensFlare | `MinStrength` | 11 | LensFlare.cpp:4 LensFlareRendering.cpp:7 |
| EngineLensFlareClasses.h | LensFlareComponent | `bHasSeparateTranslucency` | 2 | LensFlareRendering.cpp:2 |
| EngineLensFlareClasses.h | LensFlareComponent | `bUseTrueConeCalculation` | 7 | LensFlare.cpp:4 LensFlareRendering.cpp:3 |
| EngineLensFlareClasses.h | LensFlareComponent | `MinStrength` | 11 | LensFlare.cpp:4 LensFlareRendering.cpp:7 |
| EngineLensFlareClasses.h | LensFlareComponent | `NextTraceTime` | 5 | LensFlare.cpp:5 |
| EngineLensFlareClasses.h | FLensFlareElement | `bOrientTowardsSource` | 5 | LensFlare.cpp:2 LensFlareRendering.cpp:3 |
| EnginePhysicsClasses.h | KActor | `ImpactSoundComponent` | 7 | UnPhysActor.cpp:7 |
| EnginePhysicsClasses.h | KActor | `ImpactSoundComponent2` | 7 | UnPhysActor.cpp:7 |
| EnginePhysicsClasses.h | KActor | `SlideSoundComponent` | 13 | UnPhysActor.cpp:13 |
| EnginePhysicsClasses.h | PhysicalMaterial | `ImpactSound` | 1 | UnPhysAsset.cpp:1 |
| EnginePhysicsClasses.h | PhysicalMaterial | `SlideSound` | 1 | UnPhysAsset.cpp:1 |
| EnginePhysicsClasses.h | PhysicalMaterial | `FractureSoundExplosion` | 1 | ProcBuilding.cpp:1 |
| EnginePhysicsClasses.h | PhysicalMaterial | `FractureSoundSingle` | 1 | ProcBuilding.cpp:1 |

Plus `FTViewTarget::PRI` (8 uses UnCamera.cpp:147-180) and `FDirectorTrackCut::ShotNumber` (6, UnInterpolation.cpp),
`FFaceFXSoundCueKey::FaceFXSoundCue` (3, UnInterpolation.cpp), `UInterpTrackInstBoolProp::BitMask` (7,
UnInterpolation.cpp), `UInterpTrackInstDirector::OldRenderingOverrides` (2), `UInterpTrackInstFloatProp::DistributionProp`
(5), `USeqAct_PlayFaceFXAnim::SoundCueToPlay` (10, UnSkeletalComponent.cpp) from the compare-row pass.
`AnimInstPool`, `bUseClientSideCameraUpdates`, `bDebugClientSideCamera`, `bShouldSendClientSideCameraUpdate` have no
Engine/Src use (script/GameFramework only).

## Remaining rows

| Row | Why | Owner |
|---|---|---|
| `AKActor` (xcheck: `SlideEffectComponent` 664→668 and everything after it +4, sizeof 816 ok) | `struct FPhysEffectInfo` in **EngineClasses.h:1588** (agent S's committed file, off-limits for me) still has the reference-only `class USoundCue* Sound` after `Effect`: retail SDK span 0..12 (`Threshold` @0, `ReFireDelay` @4, `Effect` @8) and 2012 PDB sizeof 12, ours 16. `AKActor::ImpactEffectInfo` @652 and `SlideEffectInfo` @672 are two of them, so every member from `SlideEffectComponent` on is 4 late and the two extra words eat the 8-byte alignment gap @696. Fix: drop `Sound` (shim; uses: UnFracturedStaticMesh.cpp 2298/2737/3015/3141/3160 `PartImpactEffect.Sound`, all on `UFracturedStaticMeshComponent`). Once it is 12 bytes `AKActor` and `AKActorFromStatic` are exact without further edits | coordinator / agent S |
| `UInterpGroupInst` compare note `RootMotionMode` | not a mismatch: the 2012 PDB name of the byte @82 that retail reflects as `m_CurrentRootMotionMode` (header uses the retail name, comment records the 2012 one) | — |
| `UActorFactoryDecal` | now exact (92, `DecalMaterial` @88) with agent S's `UActorFactory` from HEAD | done |

## Follow-ups (outside my files)

- `UnCamera.cpp:903-913` `ACameraActor::PostLoad` converts `bCamOverridePostProcess_DEPRECATED` (now a
  shim) — the retail bit is `bCamOverridePostProcess`; the conversion is dead and should read the retail
  bit when Camera is ported.
- `UnSequence.cpp` `USeqAct_StreamInTextures::PostLoad` converts `bLocationBased_DEPRECATED` (shim) into
  `bLocationBased` — same pattern.
- `UnInterpolation.cpp:3360-3420` `UInterpGroupInst::{HasPPS,CreatePPS,CachePPS,RestorePPS,DestroyPPS}`
  operate on the shimmed `CachedCamOverridePostProcess` (a global now); retail keeps the camera override
  in `ACameraActor::m_CamOverridePostProcess` (`FArkPpConfig`) — port when Matinee is ported.
- `UnCamera.cpp` camera-anim pool (`ActiveAnims`, `FreeAnims`, `AnimCameraActor`, `AnimInstPool`) is
  reference-only: retail `ACamera` has no camera-anim members at all (the SDK dump ends at
  `CameraShakeCamModClass` @1036), so `ACamera::PlayCameraAnim` & co. must be re-derived from the 2013
  decompile, not ported from the reference.
- `USeqAct_Interp::ReplicatedActor` (33 uses in UnActor.cpp / UnInterpolation.cpp / UnVehicle.cpp) and
  `RenderingOverrides` (ACamera + USeqAct_Interp, 11 rendering files) are the largest shim clusters.
- `DishonoredLayouts.h` (coordinator regenerates): the pending rows for `ACamera` (1040), `ACameraActor`
  (752), `AKActor` (816), `FLensFlareElement` (320), `ULensFlare` (500), `URB_BodyInstance` (224),
  `UPhysicalMaterial` (140) can become asserts.
- `UActorFactoryDecal` depends on agent S's `UActorFactory` (+12).
