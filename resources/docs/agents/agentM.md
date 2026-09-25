# Agent M report — Engine layout probe and 2012 convergence (2026-09-25)

Build dir: `build\agentM` (Ninja, Debug, `cmake\toolchain-x86.cmake`, configured with
`-DCMAKE_CXX_FLAGS="/FI build/agentM/force_editoronly_data.h"`, see "WITH_EDITORONLY_DATA" below).
Logs: `build\agentM\probe_build1..18.log`, `engine_build1..2.log`, `pdb_fix_all3.log`.
Every number below comes from the **2012 Shipping PDB** unless the column says 2013 (agent H's
`resources/docs/types/native_class_sizes.csv`).

## Result

| Item | State |
|---|---|
| `gen_layout_probe.py generate Core Engine` | works end to end: Engine.h plus 24 further `Engine*Classes.h` are included per probe unit; 1,899 types probed (Core 304 + Engine 1,595); 291 Engine types in `probe_skip_Engine.txt` (not reachable from the headers, guarded out, or PDB-only) |
| `LayoutProbe` | builds and runs post-build without linking Core/Engine (`build\agentM\layout_probe.txt`, 9,921 lines) |
| `resources/docs/types/reference_layout_delta.md` | Engine section: 1,374 / 1,899 types exact; **contract: 68 probed, 1 mismatching (`FPackageInfo`, Core-owned)** |
| Engine contract (32 types from PHASE3.md) | **29 probed types exact in size and every probed member offset**; `UNetDriver` / `UNetConnection` have no UDT in the 2012 PDB (not probeable, see "Remaining"); `FPackageInfo` is Core's (68 vs 72) |
| `Engine/Inc/DishonoredLayouts.h` | generated with `--probe`: 669 size asserts, 284 pending, 627 types listed as not reachable from `Engine.h`; included from the end of `Engine.h` under `DISHONORED_LAYOUT_CHECKS` |
| `Engine` target | **compiles with the asserts on** (`engine_build4.log`, 0 errors, 419 units); needed 8 one-token Src fixes (`this`/`GWorld`/`NewWorld` -> `*...` at the `FNetworkNotify` sites in `UnGame.cpp` 2463/2471/2478 and `UnWorld.cpp` 2625/4627/5203/5900/5906, each tagged `// DISHONORED(layout)`; both files also carry uncommitted edits of another agent, mine are single lines) |

## Contract table (2012 result per type)

"ours" is the probe's `sizeof` after convergence; "2013" is agent H's retail size. Arkane members were
synthesized from `types.json`; reference-only members became *shims* (see below), never storage. The full
member lists are in `build\agentM\report_tables.md` and in the headers (`// DISHONORED(layout)` lines).

| Type | 2012 PDB | ours (probe) | 2013 (agent H) | header | Arkane members added | reference-only members shimmed |
|---|---:|---:|---:|---|---|---|
| AActor | 592 | 592 | 592 | EngineClasses.h | 28: CachedLocalToWorld, CachedLocation, CachedRotation, CachedDrawScale, CachedDrawScale3D, CachedPrePivot, m_ActorTypeFlags, bPathCannotStepUpOn, ... (+20 more) | 10: bSkipAttachedMoves, bProjectileMoveSingleBlocking, bForceOctreeMNFilter, bHiddenEdGroup_DEPRECATED, bHiddenEdLayer, bHiddenEdScene, bDebugEffectIsRelevant, SkelMeshCompTickTag, Layer, Group_DEPRECATED |
| APawn | 1184 | 1184 | 1184 | EnginePawnClasses.h | 10: m_bCanRunOffLedges, m_bFellOutOfWorld, m_bThrownWhileBlink, m_NavigationHandleClass, m_NavigationHandle, m_ComponentContainer, m_BackedUpPhysicsBoneIndexes, m_BackedUpPhysicsBoneAtoms, m_BackedUpPhysicsPreviousSlot, m_fBackedUpPhysicsDeltaTime | 27: bScriptTickSpecial, bNoWeaponFiring, bPathfindsAsVehicle, bPrevBypassSimulatedClientPhysics, bUsedByMatinee, bFastAttachedMove, FlashCount, FiringMode, ... (+19 more) |
| AController | 896 | 896 | 896 | EngineControllerClasses.h | 4: bAffectedByHitEffects, ViewX, ViewY, ViewZ | 20: PlayerReplicationInfo, bOverrideSearchStart, bAdvancedTactics, bCanDoSpecial, bAdjusting, bPreparingMove, bForceStrafe, bEarlyOutOfSighTestsForSameType, ... (+12 more) |
| APlayerController | 1328 | 1328 | 1328 | EngineControllerClasses.h | 0:  | 22: bCameraCut, bInteractiveMode, bShowKismetDrawText, bDebugCameraAnims, bBlockCameraAnimsFromOverridingPostProcess, bLogHearSoundOverflow, RealViewTarget, mySecondaryHUD, ... (+14 more) |
| AWorldInfo | 1904 | 1904 | 1904 | EngineGameEngineClasses.h | 38: m_SunMeshesAndMaterials, m_ArkDefaultPpSettings, bMapHasDLEsOutsideOfImportanceVolume, Paused, bUseProcBuildingRulesetOverride_DEPRECATED, bAllowLightEnvSphericalHarmonicLights, bAllowModulateBetterShadows, bIncreaseFogNearPrecision, ... (+30 more) | 45: DefaultPostProcessSettings, bUseGammaCorrection, bSuspendAI, bMinimizeBSPSections, bNoMobileMapWarnings, bUseProcBuildingRulesetOverride, bInteractiveMode, bPhysicsIgnoreDeltaTime, ... (+37 more) |
| UWorld | 716 | 716 | 716 | UnWorld.h | 6: PurgeTriggered, m_pAudioSystem, m_ActorsThatCareAboutOtherActors, m_pWorldInfo, m_pWorldInfoCheckStreamingPersistent, m_pComponentManager | 7: SaveGameSummary_DEPRECATED, DemoRecDriver, PeerNetDriver, RedirectNetDriver, AnimTreePool, bDoDelayedUpdateCullDistanceVolumes, Observers |
| ULevel | 796 | 796 | 796 | UnLevel.h | 1: m_CrossLevelReferencedActors | 5: CrossLevelCoverGuidRefs, CoverLinkRefs, CoverIndexPairs, PrecomputedVolumeDistanceField, AppliedLevelTransform |
| UActorComponent | 84 | 84 | 84 | UnActorComponent.h | 1: bIsPrimitiveComponent | 0:  |
| UPrimitiveComponent | 464 | 464 | 464 | PrimitiveComponent.h | 0:  | 12: ReplacementPrimitive, FogVolumeComponent, OverrideLightComponent, MotionBlurInstanceScale, bUsePerInstanceHitProxies, bCastStaticShadow, bNoModSelfShadow, BlockZeroExtent, BlockNonZeroExtent, bBlockFootPlacement, bSupportedOnMobile, ScriptRigidBodyCollisionThreshold |
| UMeshComponent | 480 | 480 | 480 | PrimitiveComponent.h | 0:  | 0:  |
| UStaticMeshComponent | 576 | 576 | 576 | UnStaticMesh.h | 1: bDrawAfterFog | 3: StreamingDistanceMultiplier, bCanHighlightSelectedSections, VertexPositionVersionNumber |
| USkeletalMeshComponent | 1056 | 1056 | 1088 | UnSkeletalMesh.h | 11: m_bReallyInheritTransformFromAnimParent, m_bDisableFaceFx, m_bSkipUpdate, m_bDontUpdateKinematic, RootRotationScale, AdditionalRootRotation, m_pFaceFXAsset, m_fFaceFxTickTime, m_pFaceFxAudioHandler, m_pEdgeAnimData, m_TickData | 108: AnimTickArray, AnimAlwaysTickArray, AnimTickRelevancyArray, AnimTickWeightsArray, ApexClothing, StreamingDistanceMultiplier, MorphSets, ActiveMorphs, ... (+100 more) |
| UStaticMesh | 312 | 312 | 312 | UnStaticMesh.h | 1: m_bTransparentForVisionChecks | 11: LegacykDOPTree, bHasBeenSimplified, bIsMeshProxy, VertexPositionVersionNumber, ConsolePreallocateInstanceCount, bRemoveDegenerates, bStripkDOPForConsole, bPerLODStaticLightingForInstancing, FoliageDefaultSettings, SourceData, OptimizationSettings |
| USkeletalMesh | 528 | 528 | 528 | UnSkeletalMesh.h | 7: m_UserBounds, m_MaterialsToBodyParts, m_OriginTransform, m_EdgeSkeleton, bUsePackedPosition, EditorOnlyInfo, m_CachedPathName | 92: ClothingAssets, ClothingLodMap, SourceData, OptimizationSettings, bHasBeenSimplified, BoundsPreviewAsset, PreviewMorphSets, SourceFilePath, ... (+84 more) |
| UTexture | 236 | 236 | 236 | EngineTextureClasses.h | 3: bForceNoQuality, BlendNormalToNeutral, KuwaharaFilterSettings | 1: CachedLODGroup |
| UTexture2D | 368 | 368 | 372 | EngineTextureClasses.h | 0:  | 8: CachedATITCMips, CachedETCMips, CachedFlashMipsMaxResolution, CachedFlashMips, bIsEditorOnly, bIsCompositingSource, bHasBeenPaintedInEditor, MipsToRemoveOnCompress |
| UMaterial | 912 | 912 | 912 | EngineMaterialClasses.h | 8: BloomColor, bAllowFog_DEPRECATED, bAllowDisFog, bUsedWithFogVolumes_DEPRECATED, bUsedWithFracturedMeshes_DEPRECATED, bUsedWithFoliage, bUsedWithArkPostProcess, EditorCompounds | 22: ShadowDepthBias, D3D11TessellationMode, WorldDisplacement, TessellationMultiplier, SubsurfaceInscatteringColor, SubsurfaceAbsorptionColor, SubsurfaceScatteringRadius, EnableSubsurfaceScattering, ... (+14 more) |
| UMaterialInstance | 208 | 208 | 208 | EngineMaterialClasses.h | 0:  | 0:  |
| UMaterialInstanceConstant | 256 | 256 | 256 | EngineMaterialClasses.h | 0:  | 0:  |
| UAnimSequence | 332 | 332 | 332 | EngineAnimClasses.h | 3: m_NotifiesAtAnimStart, m_NotifiesAtAnimEnd, AnimTags | 1: bWasCompressedWithoutTranslations |
| UAnimSet | 268 | 268 | 268 | EngineAnimClasses.h | 1: EditorOnlyInfo | 3: bAnimRotationOnly, PreviewSkelMeshName, BestRatioSkelMeshName |
| UPhysicsAsset | 160 | 160 | 160 | EnginePhysicsClasses.h | 0:  | 0:  |
| UEngine | 1480 | 1480 | 1480 | EngineGameEngineClasses.h | 22: DefaultBlackCubemapTexture, DefaultBlackCubemapTextureName, ShadedLevelColorationTranslucentUnlitMaterial, ShadedLevelColorationTranslucentUnlitMaterialName, bHideSecondarySubtitles, HACK_UseTickFrequency, bRenderTerrainCollisionAsOverlay, m_bShowDebugMatineeInfos, ... (+14 more) | 29: MobileEmulationMasterMaterial, MobileEmulationMasterMaterialName, bScreenshotRequested, bCheckForMultiplePawnsSpawnedInAFrame, bUseRecastNavMesh, bUseNormalMapsForSimpleLightMaps, bStartWithMatineeCapture, bCompressMatineeCapture, ... (+21 more) |
| UGameEngine | 1804 | 1804 | 1804 | EngineGameEngineClasses.h | 2: DLCManagementBridge, DLCManagementBridgeClassName | 9: bCheckForMovieCapture, bTriggerPostLoadMap, bStartedLoadMapMovie, bEnableSecondaryDisplay, bEnableSecondaryViewport, SecondaryViewportClientClassName, SecondaryViewportClients, SecondaryViewportFrames, AnimTags |
| UPlayer | 92 | 92 | 92 | EngineClasses.h | 0:  | 0:  |
| ULocalPlayer | 612 | 612 | 612 | EngineClasses.h | 9: bOverridePostProcessSettings, bRecoveryFromPostProcessOverride, m_LevelArkPpSettings, m_CurrentArkPpSettings, OverridePPRecoveryTime, OverridePPStartTime, OverridePPEndTime, OverridePPOpacity, m_ArkPpSettingsOverride | 6: ViewState2, CurrentPPInfo, LevelPPInfo, ActivePPOverrides, AspectRatioAxisConstraint, CachedAuthInt |
| UNetDriver | - | not probed | - | UnNetDrv.h | 0:  | 0:  |
| UNetConnection | - | not probed | - | UnConn.h | 0:  | 0:  |
| FPackageInfo | 68 | 72 (**mismatch**) | - | - | 0:  | 0:  |
| USequence | 328 | 328 | 328 | EngineSequenceClasses.h | 1: m_AllEvents | 1: DelayedLatentOps |
| USequenceOp | 224 | 224 | 224 | EngineSequenceClasses.h | 1: m_bAlwaysOutOfBendTime | 0:  |
| UInterpData | 156 | 156 | 156 | EngineSequenceClasses.h | 2: m_Data, m_iMatineeDataVersion | 12: InterpLength, PathBuildTime, InterpGroups, CurveEdSetup, InterpFilters, SelectedFilter, DefaultFilters, EdSectionStart, EdSectionEnd, bShouldBakeAndPrune, BakeAndPruneStatus, CachedDirectorGroup |

## provisional-2012 (every layout change; re-check against H's 2013 sizes and I's member lists)

All 2013 sizes in the table equal 2012 except **`USkeletalMeshComponent` 1056 -> 1088 (+32)** and
**`UTexture2D` 368 -> 372 (+4)**; those two need agent I's 2013 member list before the retail layout can be
written. Changed types, each marked `// DISHONORED(layout): 2012 PDB ...`:

- Script classes whose `//## BEGIN PROPS` block was regenerated from `types.json` (`gen_layout_probe.py props Engine ...`):
  `AActor`, `APawn`, `AController`, `APlayerController`, `AWorldInfo`, `UEngine`, `UGameEngine`, `ULocalPlayer`,
  `UTexture`, `UTexture2D`, `UMaterial`, `UAnimSequence`, `UAnimSet`, `USequence`, `USequenceOp`, `UInterpData`,
  and `UMaterialInterface` (base of the three material contract types, 116 bytes; 101 reference-only
  mobile/flatten members shimmed). `UPlayer`, `UMaterialInstance`, `UMaterialInstanceConstant`, `UPhysicsAsset`
  already matched the PDB member list and were left as they were.
- Hand-written (noexport) classes, members reordered / added / removed by name: `UWorld` (`UnWorld.h`), `ULevel`
  (`UnLevel.h`, `WITH_NOVODEX` guard around `SceneIndex..LevelConvexBSPActor` dropped), `UActorComponent`
  (`+bIsPrimitiveComponent`), `UPrimitiveComponent`, `UMeshComponent` (`PrimitiveComponent.h`), `UStaticMesh`,
  `UStaticMeshComponent` (`UnStaticMesh.h`), `USkeletalMesh`, `USkeletalMeshComponent` (`UnSkeletalMesh.h`, whole
  data block regenerated in PDB order).
- Base-class changes (from the PDB base lists): `AController` loses `IInterface_NavigationHandle`; `APawn` bases are
  `AActor, IInterface_NavigationHandle, IArkHealthInterface` (loses `IInterface_Speaker`; both interface vptrs sit in
  AActor's tail padding @584 / @588); `UWorld` loses `FNetworkNotify` (no `UWorld::Notify*` function exists in the
  PDB; the Notify members stay as plain functions); `ULocalPlayer` loses `FObserverInterface` (`AddObserver` /
  `RemoveObserver` are no-ops); `UEngine` gains `IArkSettingsListenerInterface` @60.
- New Arkane types declared: `UArkHealthInterface` / `IArkHealthInterface` (EnginePawnClasses.h),
  `UArkSettingsListenerInterface` / `IArkSettingsListenerInterface` (EngineGameEngineClasses.h),
  `FRenderingChannelContainer`, `FDisPrimTraceMask`, `EDisTranslucencySortPriority` (UnActorComponent.h),
  `FArkPpConfig` + 5 sub-structs (EngineClasses.h), `FArkSunGlareMeshParams`,
  `FWorldInfoNavMeshGen{Process,Base,Scout}Params` (EngineGameEngineClasses.h), `FAnimSet_EditorOnly`
  (EngineAnimClasses.h), `FUserBounds`, `FBodyPart`, `FSkeletalMesh_EditorOnly`, `USkeletalMeshComponent::FTickData`
  (UnSkeletalMesh.h). The registrations (`DECLARE_ABSTRACT_CLASS` of the two U*Interface classes) still need an
  `IMPLEMENT_CLASS` in Src when the module is linked.
- Sub-struct / template fixes: `FApexModuleDestructibleSettings` (12: no `MaxShapeCount`, no override bit),
  `FLightmassWorldInfoSettings` (60), `FLightmassMaterialInterfaceSettings` (24), `FExpressionInput` (28, no
  `OutputIndex` / `InputName`, `MaterialShared.h`), `UStaticMesh::kDOPTreeType` = `TkDOPTree` (24, not
  `TkDOPTreeCompact`), no `LegacykDOPTree` member, `UMaterialInstance::Resources[2]`,
  `UMaterial::DefaultMaterialInstances[2]` (two material quality levels in 2012).
- Stubs to port (declared inline, rva of the 2012 body in the comment): `APawn::GetEdgeZAdjust` (0x1e0580),
  `APawn::SetupPathfindingParams` (0x1e05d0), `APawn::ArkGetCurHealth` (0x18db60), `UEngine::ApplyGameSettings`.
  `APawn::ArkIsIncapacitated` / `ArkIsDeadOrDestroyed` have no APawn symbol (COMDAT-folded) and return FALSE.

Script members added by Arkane are inside the regenerated PROPS blocks; agent I's package tables confirm
them (their names come straight from the PDB, their offsets follow from the list plus /Zp4).

## Shims (`DISHONORED_SHIM_STATIC`)

Reference-only members that the PDB proves absent are not deleted: the module port would need hundreds of
Src edits (`APawn::PlayerReplicationInfo` alone has 96 uses in 13 files, `UWorld::DemoRecDriver` 83,
`AController::MoveTarget` 81, `UInterpData::InterpGroups` is all of Matinee). They are kept after each member
block as `DISHONORED_SHIM_STATIC <decl>;`, which is `inline static` by default (Engine.h) so unported
reference code compiles unchanged, and plain `static` inside the probe (no constructor instantiation, the
probe links no module library). They are **not layout**; reading one yields the zero-initialised static,
writing one is a global write, and every use is a porting TODO. Bitfield shims are typed `BITFIELD` (a `UBOOL`
= `UINT` static made the replication `NEQ()` overloads ambiguous in UnActor.cpp). 568 shim lines in 23 blocks
(`grep -n DISHONORED_SHIM_STATIC source/Development/Src/Engine/Inc/*.h`).

## WITH_EDITORONLY_DATA

The 2012 PDB keeps the editor-only data (`AActor::EditorIconColor` @248, `UTexture::SourceFilePath` @164 and
`LightingGuid` @192, `UStaticMesh::SourceFilePath` @260, ...). The reference `UE3BuildTarget.cs:736` sets
`WITH_EDITORONLY_DATA=0` only for script-patching executables, so a Win32 game build has it at 1;
`cmake/DishonoredDefines.cmake` says 0 and is not my file. `build\agentM` forces it to 1 through
`/FI build/agentM/force_editoronly_data.h`; **the coordinator must set `WITH_EDITORONLY_DATA=1` in
`DishonoredDefines.cmake`** (agent H's 2013 sizes are identical to 2012 for these classes, so this holds for
retail too). The regenerated PROPS blocks and the converged noexport classes have their
`#if WITH_EDITORONLY_DATA` guards removed where the PDB has the member.

## Tool changes

`resources/tools/symbols/gen_layout_probe.py`
- `generate`: probe units are chunked (`CHUNK` = 100 types per `probe_<Module>_<n>.cpp`; cl.exe stops a TU at
  100 errors and a single 35k-line Engine TU is slow); `MODULE_INCLUDES["Engine"]` lists the 24 `Engine*Classes.h`
  that `Engine.h` does not reach (in the order Engine's own .cpp files include them); every unit predefines
  `DISHONORED_LAYOUT_CHECKS 0` and `DISHONORED_SHIM_STATIC static`; `probe_main.cpp` stubs `appFailAssertFunc`,
  `appFailAssertFuncDebug` and `GDynamicRHI` besides `appMalloc` / `appFree`. `skip-from-log` understands the
  chunked file names.
- `CONTRACT` += the 32 Engine types from PHASE3.md.
- `show` no longer crashes on bitfield members.
- New `props <Module> <Class>...`: rewrites a `//## BEGIN PROPS` block from `types.json` (reference lines reused
  by name with their access specifier, Arkane members synthesized with `gen_classes_header.fix_type`,
  `SCRIPT_ALIGN` after BYTE runs, reference-only members become the shim block). Leaves already-matching
  classes untouched.
- New `pdb-fix [Type...]`: **IDA's PDB import drops or truncates derived-class members that MSVC places in an
  over-aligned base's tail padding** (`UMeshComponent::Materials` @452 inside `sizeof(UPrimitiveComponent)` ==
  464, `UStaticMeshComponent::StaticMesh` @476, `USkeletalMeshComponent::SkeletalMesh` @468,
  `AController::Pawn` @584, `AWorldInfo::m_SunMeshesAndMaterials` became `_BYTE[4]`). `pdb-fix` re-reads every
  UDT through DIA (loader from `resources/tools/pdb/dia_dump.py`,
  `../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.pdb`) and replaces the data-member lists of
  `types.json` in place (277 types changed; bitfields normalised to IDA's byte + bit form; `sizes.csv` is
  unaffected). `types.json` is gitignored: **run `pdb-fix` after every `export_types.py`**, otherwise `props`,
  `show` and `compare` work on the truncated lists.
- New `structs <F...>`: prints gen_classes_header-style declarations for PDB structs a converged header needs.

`resources/tools/symbols/gen_layout_asserts.py`
- Asserts only types declared in headers reachable (transitive quoted includes) from `<Module>.h` and seen by
  the probe; the others are listed at the end of the file as "not reachable" (627 for Engine: they live in the
  `Engine*Classes.h` that the .cpp files include on their own, so they would be incomplete at the end of
  `Engine.h`). Core is unchanged by this (278 / 12, `--dry-run`).
- The packing probe struct is named per module (`FDishonoredPackingProbeEngine`): Core's copy is already in
  every Engine TU via `Core.h`, the redefinition broke all 419 units in the first Engine build.
- `--dry-run` prints the counts without writing.

Helper scripts used for the noexport classes (kept in `build\agentM`, not repo tools): `members_of.py` (header
member lines vs PDB order), `edit_noexport.py` + specs (named removals / inserts / unguards), `rewrite_members.py`
(regenerates a class's or struct's data block in PDB order).

`resources/docs/types/probe_skip_Engine.txt`: 291 entries, collected over 11 skip-from-log rounds.

## Remaining mismatches

- Contract: `FPackageInfo` 68 vs 72 (Core `UnCoreNet.h`; the reference `LoadingPhase` after `Extension` is not in
  the PDB). Agent A listed it as pending; Core is off limits for M.
- `UNetDriver`, `UNetConnection`: no UDT of that name in the 2012 PDB (`sizes.csv` / `types.json` /
  `all_types.h` only have `struct UNetDriver *` forward uses, 0 member functions in `functions.csv`) and no row
  in H's 2013 table: the networking classes are compiled out of the shipping exe. Nothing to converge against;
  `UnNet.h` is therefore not part of the probe includes.
- Non-contract Engine types: 524 rows with differences (471 size mismatches, 39 size-ok with `MISSING` members,
  14 offset-only). Largest: `FSystemSettings` 11584 vs 1088, `FStreamingManagerTexture` 560 vs 2676, `ACamera`
  1040 vs 1328, the material shader parameter structs, `UNavigationMeshBase` 464 vs 688. They are `// pending:`
  in `DishonoredLayouts.h` and belong to the per-module ports of Phase 3.

## For porting_notes.md (coordinator)

1. `WITH_EDITORONLY_DATA=1` for the game build (see above).
2. IDA drops tail-padding members of derived classes; `types.json` needs `gen_layout_probe.py pdb-fix` after export.
3. Shim convention `DISHONORED_SHIM_STATIC` (Engine.h) for reference-only members; every use is a porting TODO.
4. Interface bases per the 2012 PDB: `APawn` implements `IInterface_NavigationHandle` + `IArkHealthInterface`,
   `AController` does not implement `IInterface_NavigationHandle`, `UEngine` implements
   `IArkSettingsListenerInterface`; `UWorld` is not a `FNetworkNotify`, `ULocalPlayer` not a `FObserverInterface`.
5. The 2012 material system has two quality levels (`[2]` arrays), `FExpressionInput` has no
   `OutputIndex` / `InputName`, `UStaticMesh` uses the non-compact kDOP tree.
6. The module layout header must only assert types complete at the end of `<Module>.h`; per-module packing probe.
