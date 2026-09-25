# Agent AB — GameFramework / IpDrv / OSS bases, pending asserts, hygiene (2026-09-25)

Build dir `build\agentAB` (working tree; Ninja, Debug, `cmake\toolchain-x86.cmake`, `-DDISHONORED_REAL_LAUNCH=ON`, the four
module options ON, `-DDISHONORED_LAYOUT_CHECKS=ON`): `build\agentAB_configure.cmd`, `build\agentAB_full.cmd <targets>`,
`build\agentAB_probe.cmd`. IDA copies `resources/docs/idb/shipping2012_agentAB.i64`, `retail2013_agentAB.i64`; decompiles in
`build\agentAB\dec2012`, `dec2013`. Patch scripts `build\agentAB\*.py` (scratch). **Every number below is from `build\agentAB`**;
"retail SDK" = `retail_sdk_layout.json` (the dump of the 2013 exe), "2012 PDB" = `types.json`, "2013 rva" = retail exe.

## Result

| Check | Before (coordinator, HEAD) | After (`build\agentAB`) |
|---|---|---|
| `xcheck_sdk_layout.py build/agentAB/layout_probe_final.txt` (9-module probe) | 4 rows (2,314 types) | **0 rows**: types=3515 exact=2625 mismatching=0 contract=0 |
| `gen_layout_probe.py compare build/agentAB/layout_probe_final.txt` | 0 contract | probed=4337 exact=4086 **contract_mismatches=0** |
| `verify_phase2.py retail build/agentAB/layout_probe_final.txt` (new) | — | **2/2 PASS** |
| native sizes vs `native_class_sizes.csv` (every probed class) | — | 3 differ: `UNavigationMeshBase` 688/464, `UShaderCache` 132/128 (not mine, see pending), `UMCPBase` fixed 76 → 60 |
| DishonoredGame SDK asserts (`DishonoredGameLayouts.h`) | 12,137 + 354 pending | **12,502 + 0 pending** (`UDisGameCrowdPopulationManager` generated again) |
| OSS / GFxUI / AkAudio SDK asserts | 48+84 / 133+0 / 36+10 | **132+0 / 133+0 / 46+0** |
| `DishonoredLayouts.h` Core / Engine / D3D9Drv | 277+13 / 763+190 / 8+5 | **279+11 / 806+150 / 8+5** (reasons below) |
| `cmake --build build\agentAB --target DishonoredGame CoreSmoke LayoutProbe` | — | **0 errors** (`build\agentAB\build10.log`, `build11.log`), exe links |
| `CoreSmoke.exe` (repo root) | 99/99 | **99 passed, 0 failed** (`build\agentAB\coresmoke.log`) |
| isolated smoke (command below) | Edge-animation abort | passes the Edge point; aborts later in `RefShaderCache-PC-D3D-SM3.upk` (`Bad name index 1048704/1320`) — **identical to agent Y's snapshot run without any AB file** (`DishonoredGame\Logs\agentY.log`, 20:37): in-flight shader-cache work of the shared tree, not an AB regression |

Smoke command: `python resources\tools\build_and_smoke.py --build-dir build\agentAB --no-build --exe-name DishonoredGame_AB.exe
--log-name agentAB.log --ini-dir build/agentAB/config --rhi null --milestone "objects as part of root set" --skip-native
OnlineSubsystemPC --extra-args "-NoLoadStartupPackages -allowunboundnatives"` (`build\agentAB\smoke1.txt`,
`DishonoredGame\Logs\agentAB.log`). The 1320-name package was identified with `build\agentAB\namecount.py`.

## 1. The four SDK rows

| Row | Root cause | Fix |
|---|---|---|
| `ADisGameCrowdAgentSkeletalRat` +228 | `AGameCrowdAgent`/`AGameCrowdAgentSkeletal` at the reference layout | `sdk_props.py GameFramework` (all 24 tree-declared retail GameFramework classes); `AGameCrowdAgent` bases `ACrowdAgentBase` + `IArkHealthInterface` @588 (retail SDK VfTable, 2012 PDB bases; the reference `IInterface_RVO` is gone; `ArkGetCurHealth` = `Health` per 2012 rva 0x15d0, `ArkIsIncapacitated`/`ArkIsDeadOrDestroyed` = the interface defaults 2012 rva 0x1705a0/0x1705b0); `MySpawner` is `UGameCrowdSpawner*` @900 (the reference `TScriptInterface` was 8 bytes); `FSingleAgentAttractor` declared (retail SDK 0..28). 68 reference-only agent members and 24 skeletal-agent members are shims |
| `UOnlineSubsystemSteamworks` +4 | `UOnlineSubsystemCommonImpl` 204 vs 200 | `sdk_props.py IpDrv`: +`FPointer pLeaderboardHelper` @196; `GameInterfaceImpl`/`AuthInterfaceImpl` reference-only shims |
| `ADisDoor` −12 | `FPolyReference` 28 vs 24 | `CachedPoly` removed (retail SDK span 0..24, 2012 PDB 24); `FPolyReference::GetPoly` ported (2013 rva 0x2723f0 = 2012 rva 0x28cc60, identical bytes: no cache, no sub-mesh lookup) in `UnNavigationMesh.cpp` |
| `ADishonoredPlayerController` +8 | `AGamePlayerController` 1336 vs 1328 | `sdk_props.py`: `bWarnCrowdMembers`, `bDebugCrowdAwareness`, `CurrentSoundMode` shims |

Also from the same pass: `AGameCrowdDestination` bases + `IGameCrowdSpawnInterface` @592 / `IEditorLinkSelectionInterface` @596
(`UGameCrowdSpawnInterface`/`IGameCrowdSpawnInterface` moved from the DishonoredGame shims into `GameFrameworkClasses.h`,
`IMPLEMENT_CLASS` in `GameCrowd.cpp`, registrant), fixes `ADynamicGameCrowdDestination` +16; `UGameThirdPersonCamera(Mode)`,
`AGameCrowdDestinationQueuePoint`, `USeqAct_GameCrowdSpawner` (retail super `Engine.SequenceAction`, `Spawner` @248) converged;
`UMCPBase` rebased on `UObject` (retail super, flags 0x1, 60 bytes; the reference `UMcpServiceBase` made it 76).

## 2. `UGameCrowdPopulationManager`

Agent X added the retail UObject (144, `IArkSettingsListenerInterface` @56) and its `IMPLEMENT_CLASS` in
`gamecrowdpopulationmanager.cpp` (X's file, kept). AB removed the reference Actor so no second "GameCrowdPopulationManager"
class can be created by `AGameCrowdPopulationManager::StaticClass()`: its declaration, event-parms structs, natives table and
`AUTOGENERATE_FUNCTION`s, the reference `USeqAct_GameCrowdPopulationManagerToggle` (retail has no such class), their 14 method
bodies and `IMPLEMENT_CLASS`es in `GameCrowd.cpp`, and the four `Cast<AGameCrowdPopulationManager>(WorldInfo->PopulationManager)`
sites (the member is a shim: retail `AGameInfo`/`AWorldInfo` have no population manager) — each `// DISHONORED(port)`.
Regenerated DishonoredGame (`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake --probe
build/agentAB/layout_probe.txt`): 1,823 classes (was 1,822), 119 shim classes (was 120), 0 pending; the four hand entries of
`DishonoredGameLayouts.pending.txt` removed (`FTViewTarget` 40, `FPolyReference` 24, `USeqAct_Interp` 520,
`USeqAct_MultiLevelStreaming` 284 all match retail now).

## 3. Pending asserts

`gen_layout_probe.retail_sizes()`: script-struct targets were the SDK span end (PropertiesSize, no tail padding: `FBox` 25,
`FIniLocFileEntry` 13, …); now `Align(span_end, MinAlignment)` (16 with a Matrix/Plane/Vector4/Quat member, else 4 —
`UStructProperty::Link`), which removed 14 false rows. New `RETAIL_NATIVE_STRUCT_SIZES` for non-reflected types whose 2013
size differs from 2012 (`FSystemSettings` 11628, `FSystemSettingsData` 1052, `FSystemSettingsDataWorldDetail` 144, evidence
in the tool). Converged: `FSystemSettings` family (below), `FTextureLODSettings`/`FExposedTextureLODSettings` 728
(`TEXTUREGROUP_MAX` 26 as in the retail enum, `FTextureLODGroup::KuwaharaFilterSettings` @24), `FParticleEmitterInstance` and
its 8 children (`bUseNxFluid` @328, 2012 PDB), `FConsoleMessage` (PRI shim), `FMaterialReferenceList`
(`AffectedPPChainMaterialRefs` shim), `FDebugTextInfo` (`OrigActorLocation`/`Font` shims), `AGameReplicationInfo` 640 and
`USkeletalMeshSocket` 120 (`sdk_props.py`; `FSkeletalMeshSocket_EditorOnly` declared), `UArrowComponent` 464.

**Engine: 150 pending**, every one named in `build\agentAB\pending_engine_final.txt`:

| Reason | Rows |
|---|---:|
| Rendering types, agent Y's area (not touched by rule): `UnParticleHelper.h` 36 (dynamic emitter data, scene proxies, vertex structs), `GlobalShader.h` 15, `MaterialShared.h` 10, `MaterialShader.h` 5, `ShaderManager.h` 4, `RHI.h` 4, `Scene.h` 4 (`FSceneView` 1280/1392, `FLightChannelAllocator`, `FPrimitiveDrawInterface`, `FSceneViewStateInterface`), particle/lens-flare vertex factories 10, `MeshMaterialShader.h` 2, `ShaderCache.h` 2 (`UShaderCache` 132/128), `UnSceneCapture.h` 2, `ShaderCompiler.h`, `UnTex.h` (`FTexture2DResource`), `UnRenderUtils.h`, `RawIndexBuffer.h`, `LensFlare.h`, `PrimitiveComponent.h` (`FPrimitiveSceneProxy`), `UnActorComponent.h` (`FWindSourceSceneProxy`), `UnCanvas.h`, `UnClient.h` (`FViewport`), `PreviewScene.h`, `FullScreenMovie(Fallback).h` (Bink) | 105 |
| Navigation mesh: `UnPath.h` edge/poly/world structs and `UNavigationMeshBase` 688/464 — Arkane's nav mesh is a different design (2012 edge 52 vs reference 112); needs a nav-mesh port, not a layout pass | 10 |
| Mesh data read by `Serialize` (AA/W area): `UnSkeletalMesh.h` 7 (`FStaticLODModel` 300/328, `FSkelMeshSection`, `FVertInfluence`, …), `UnStaticMesh.h` 3 (`FStaticMeshRenderData`, `FStaticMeshComponentLODInfo`, `FStaticMeshLODElement`) | 10 |
| Gameplay stats (`UGameplayEvents*`, `FGameplayEventsHeader`, `FPlayerInformation`, the four `*ClassEventData`: 2012 stores `FString` names, the reference `FName`/extra fields; serializers in UnGameplayEvents.cpp; dead at runtime) | 10 |
| Texture streaming internals (`UnContentStreaming.h` 5: `FStreamingManagerTexture` 560/2668) | 5 |
| Tick stats (`UnEngine.h` 3, agent X's file set) | 3 |
| Editor lighting build (`StaticLighting.h` 2, `LightingBuildOptions.h`) | 3 |
| `FAnimSlotInfo` 52/20 (2012 `m_SlotName`, `m_iNBChannel`, `m_ChannelWeights[10]`; anim, agent W) | 1 |
| `FParticleRibbonEmitterInstance` 644/628 (reference per-instance `CurrentSizes`/`HeadOnlyParticles` used by the ribbon code: a shim would share them across instances) | 1 |
| `FWaveModInfo` (UnAudio.h, audio), `FRBPhysScene` (PhysX off) | 2 |

Core: 11 pending (compressed-archive proxies, async IO system/handle, `FAsyncPackage`, `FCallbackEventObserver`, output device,
thread, ring buffer, gameplay profiler — none on this package's list); D3D9Drv: 5 (Y). `FAsyncIOSystemBase::FAsyncIORequest` is a
nested type the generator does not assert; it is 76 bytes now (2013 layout below).

## 4. Hygiene

| Item | Change |
|---|---|
| `FSceneViewFamily::CurrentBendTime` | `Scene.h`: `CurrentBendTime` @0, `bSkipForegroundRendering` @68, `bIsRenderingReflectionScene` @72, `GammaCorrection` @76 (2012 PDB 80; copy ctor 2012 rva 0x47fae0, `FSceneViewFamilyContext` ctor 2012 rva 0x2126d0 / 2013 rva 0x1fc2f0); `bScreenCaptureRenderTarget`, `bDrawBaseInfo` shims; `Scene.cpp` ctor initializes the new members (the retail ctor takes `InCurrentBendTime` first — the call sites are Y's) |
| `SHOW_DefaultGame` | `Scene.cpp`: `| SHOW_Selection | SHOW_Portals`; `SHOW_ViewMode_Lit | (SHOW_DefaultGame & ~SHOW_ViewMode_Mask)` = `0x04062BD217403362` = the retail ctor constant (2013 rva 0x2c53c0), checked by evaluating the Scene.h/Scene.cpp definitions |
| `FAsyncIORequest` 2013 | `UnIOBase.h`: `FString NormalizedFileName` @24 (76 bytes: Offset @36 … RequestType @68, bits @72, per 2013 `QueueIORequest` rva 0x519b0); `UnAsyncLoading.cpp` fills it like 2013 `sub_4470E0` (rva 0x470e0: leading `..\` pairs skipped, a leading `\` ensured). The 2013 handle cache keyed by it (rva 0x74130) is not ported |
| `FSystemSettings` | `SystemSettings.h`: ten `FSystemSettingsData*` structs at the 2013 offsets of agent O's table (WorldDetail 144 with `bAllowRatsShadow` @76 and `iType_AntiAlias` @128, …, Audio @1048), `FSystemSettingsData` 1052, `FSystemSettings : FExec, FSystemSettingsData` + `FRenderThreadSettings` @1056 (storage only) + `bIsEditor` @1100 + `CurrentSplitScreenLevel` @1104 + `Defaults[5][2]` @1108 = **11628** (2013 `Initialize` rva 0x1844e0: 0x41C copies, 2104 stride); 28 reference-only members + `bInit`, `SystemSettingName`, `NumberOfSystemSettings` are shims; O's table unchanged (ctor: `bInit` assigned in the body) |
| `UArrowComponent` 464 | `PrimitiveComponent.h`: `bTreatAsASprite` a bit (retail @460 mask 0x1), `SpriteCategoryName` shim; `UBrushComponent::m_bTwoSided` @532 mask 0x2; `bDrawOnlyIfSelected` of the four draw components is a shim (retail DWORDs have two bits) |
| `dishonored_module(<Name> TARGET <t>)` | `cmake/DishonoredModule.cmake`; root `CMakeLists.txt` uses `dishonored_module(DishonoredGame TARGET DishonoredGameModule)` (+ `/bigobj`, the SDK-checks define) instead of the inline copy; `gen_layout_probe.py` maps DishonoredGame → `DishonoredGameModule` for the probe's include dirs |
| `inc`/`Inc` case | on disk every module now uses `Inc`/`Src` (`DisJobs/inc` renamed to `Inc`; AkAudio/DishonoredGame already were); generator output paths and `Sources.cmake` already use `Inc`/`Src`. **The git index still spells 1,098 paths `inc/`/`src/`** (AkAudio, DishonoredGame, DisJobs; plus 32 lowercase file names, e.g. `inc/akaudioclasses.h` vs `Inc/AkAudioClasses.h`) — fixing it is a staging operation, left to the coordinator at commit time: `git rm -r -q --cached source/Development/Src/AkAudio source/Development/Src/DishonoredGame source/Development/Src/DisJobs` then `git add` the same three directories (`build\agentAB\case_check.py` lists the differences) |
| `verify_phase2.py retail <probe>` | new section: runs `gen_layout_probe.py compare` and `xcheck_sdk_layout.py` on the probe and fails on any contract mismatch of either |
| `DishonoredLayouts.h` | regenerated: `gen_layout_asserts.py --probe build/agentAB/layout_probe_final.txt Core Engine D3D9Drv` |
| probe tooling | `gen_layout_probe.py`: `MODULE_INCLUDES` for GameFramework/IpDrv (first probe of both), script-struct alignment, `RETAIL_NATIVE_STRUCT_SIZES`, `CMAKE_TARGET`; skip lists `probe_skip_GameFramework.txt`, `probe_skip_IpDrv.txt` (new), `probe_skip_Engine.txt` +10 (types in X's/W's new headers: `ADebugCameraController`, `FEdgeAnim*`, `FDisJob*`, `FLocomotionState*`) |

`source/Tests/LayoutProbe` is back at HEAD (the 9-module probe sources are in `build\agentAB\probe_full`; regenerate with
`gen_layout_probe.py generate Core Engine GameFramework IpDrv D3D9Drv GFxUI AkAudio OnlineSubsystemSteamworks DishonoredGame`).

## Files (AB)

GameFramework: `Inc/GameFrameworkClasses.h`, `Inc/GameFrameworkCameraClasses.h`, `Inc/GameFrameworkAnimClasses.h` (PROPS
comments), `Src/GameCrowd.cpp`. IpDrv: `Inc/IpDrvClasses.h`. Engine: `Inc/Scene.h`, `Src/Scene.cpp`, `Inc/SystemSettings.h`,
`Src/SystemSettings.cpp` (ctor), `Inc/PrimitiveComponent.h`, `Inc/EngineClasses.h` (`FPolyReference`, `FConsoleMessage`,
`FMaterialReferenceList`), `Src/UnNavigationMesh.cpp` (`GetPoly`), `Inc/EngineControllerClasses.h` (`FDebugTextInfo`),
`Inc/EngineReplicationInfoClasses.h`, `Inc/EngineSkeletalMeshClasses.h`, `Inc/ParticleEmitterInstances.h`,
`Inc/EngineTextureClasses.h`, `Inc/UnTex.h`, `Inc/DishonoredLayouts.h`. Core: `Inc/UnIOBase.h`, `Src/UnAsyncLoading.cpp`,
`Inc/DishonoredLayouts.h`. D3D9Drv `Inc/DishonoredLayouts.h`. Generated: DishonoredGame (headers, registrants, stubs,
`Sources.cmake`, `DishonoredGameLayouts.pending.txt`), OSS/GFxUI/AkAudio layouts + stub units. CMake: `cmake/DishonoredModule.cmake`,
`CMakeLists.txt`. Tools: `resources/tools/symbols/gen_layout_probe.py`, `verify_phase2.py`; `resources/docs/types/probe_skip_*`.
`DisJobs/inc` → `DisJobs/Inc` on disk.

## Follow-ups outside my files

1. **X / merge**: `build/agentX_patch_gf.py` is a no-op on the new `GameFrameworkClasses.h` (it returns when
   `UGameCrowdPopulationManager` is present); X's snapshot `build/agentX_wt` still carries the Actor, so the merge takes AB's
   `GameFrameworkClasses.h` / `GameCrowd.cpp` and X's `gamecrowdpopulationmanager.cpp`. The size rows X lists in agentX.md (`ADisGameCrowdAgentSkeletalRat`, `ADishonoredPlayerController`,
   `ADynamicGameCrowdDestination`, `UOnlineSubsystemSteamworks`) all match retail now, so the local allocation slack can go.
2. **Engine interface**: `IArkHealthInterface` (EnginePawnClasses.h) declares `ArkIsIncapacitated`/`ArkIsDeadOrDestroyed` pure;
   retail has default bodies (2012 rva 0x1705a0/0x1705b0) — give them bodies, then `AGameCrowdAgent`'s two overrides can go.
3. **Y**: the 105 rendering rows above; `FSceneViewFamily(Context)` ctor should take `InCurrentBendTime` first (retail); the
   `bScreenCaptureRenderTarget` shim is read in SceneRendering.cpp:2862 / ScenePostProcessing.cpp:59 and written in UnPlayer.cpp:1197.
4. **Coordinator**: `external\*-build` is shared by every build dir: concurrent builds regenerate `libpng-build\pnglibconf.h` and
   other agents' compiles fail with C1083 (hit here 3 times). `build\agentAB_build.cmd` appends a private copy to `INCLUDE`;
   a per-build-dir `BINARY_DIR` in `cmake/Dependencies.cmake` would fix it for everyone.
5. `VERIFY_CLASS_OFFSET_NODIE` lines in GameFramework/IpDrv headers still name members that are shims now (`MyGroup`,
   `CurrentSoundMode`, `AuthInterfaceImpl`, …); harmless while `AutoCheckNativeClassSizes*` is compiled out, fix when enabled.
6. `FTextureLODGroup::KuwaharaFilterSettings` is not read from the ini yet (2013 `FTextureLODSettings::ReadEntry`).
7. The git index case fix (hygiene row above).
