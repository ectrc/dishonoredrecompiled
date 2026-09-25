# Agent S report — Engine convergence: EngineClasses.h core / UI / AI / GameEngine / Controller / Client (2026-09-25)

Build dir `build\agentS` (Ninja, Debug, `cmake\toolchain-x86.cmake`, `-DDISHONORED_REAL_LAUNCH=ON`). Logs
`build\agentS\probe0..6.log`, `game1..2.log`, `smoke*.log`. Helper scripts in `build\agentS\` (`patch1..4.py`, `bitcheck.py`,
`sizecheck.py`, `tail_of.py`, `shim_uses.py`), not repo tools. The **retail 2013 build is the target**: every offset below
that says "retail SDK" comes from `retail_sdk_layout.json` (the CodeRed dump taken in the running retail exe), every
"2012 PDB" number from `types.json` (Shipping 2012 PDB) and is evidence only.

## Result

| Check | State |
|---|---|
| `xcheck_sdk_layout.py build/agentS/layout_probe.txt --header <h>` | `EngineUIPrivateClasses.h` 0 / 41, `EngineUserInterfaceClasses.h` 0 / 22, `EngineAIClasses.h` 0 / 32, `EngineGameEngineClasses.h` 0 / 28, `EngineControllerClasses.h` 0 / 6, `EnginePawnClasses.h` 0 / 6, `UnClient.h` 0 / 1, `UnLevel.h` 0 / 3, `UnCorObj.h` 0 / 4, `UnActorComponent.h` 0 / 5, `PrimitiveComponent.h` 0 / 13 — all exit 0. `EngineClasses.h` **1 / 239** (`UGameViewportClient`, root cause `ShowFlags.h`, not my file — see "Remaining") |
| sizes vs `native_class_sizes.csv` size_2013 (`build\agentS\sizecheck.py`, 257 classes of my headers with a retail size) | 2 mismatches left: `UGameViewportClient` 292 vs 284 (same root cause), `UArrowComponent` 480 vs 464 (`PrimitiveComponent.h`, hand-written class outside my rows: follow-up) |
| `gen_layout_probe.py compare` | probed 1,920, exact 1,666, **0 contract mismatches** (`buildgentS\probe8.log`, final probe) (`AActor`, `APawn`, `AController`, `APlayerController`, `AWorldInfo`, `UEngine`, `UGameEngine`, `UPlayer`, `ULocalPlayer`, `UActorComponent`, `UPrimitiveComponent`, `UMeshComponent`, `ULevel`, `UWorld` untouched except the bitfield rows below) |
| Bitfields (`build\agentS\bitcheck.py`: header `BITFIELD` order vs the dump's offset + mask order, 403 types) | 0 differences in my headers; the remaining reports are `UArrowComponent`/`UBrushComponent`/`UDraw*Component` (`PrimitiveComponent.h`, outside my rows) and two false negatives of the script (`APlayerController`, `UUIDataProvider_Settings`: verified by hand, the script picked a `class APlayerController* PC,` parameter line as the declaration) |
| `UnknownDataNN` placeholders | **0** left in my headers (every SDK gap was filled from the 2012 PDB by relative position) |
| `cmake --build build\agentS --target DishonoredGame` | see "Build" below |
| `build_and_smoke.py --build-dir build/agentS --no-build` | **exit 0**, `Init: Object subsystem initialized` reached (`buildgentS\smoke1.log`; the golden diff is the known ini/WinSock/-nullrhi noise) |

Rows went from 40 + 29 + 7 + 9 + 1 + 1 + 1 + 1 + 1 (+ `ULightComponent`, agent R's) = 90 to 1.

## Method

1. `sdk_props.py Engine <all 82 classes with a PROPS block in my headers>` in one run (the tool works from the retail
   offsets, so bases and children can be regenerated together; the probe check is what needs the base first). It
   filled every native gap from the 2012 PDB (no `UnknownData` left), moved reference-only members to
   `DISHONORED_SHIM_STATIC` blocks and reported interface vtables and native tails.
2. Then the bitfield-only classes the delta cannot see (`bitcheck.py`): `AHUD`, `APhysicsVolume`, `AStaticMeshActor`,
   `ATriggerVolume`, `AVolume`, `ULevelStreamingAlwaysLoaded`, `UNavMeshPathGoalEvaluator` (tool), and the too-large
   classes the delta does not flag (`sizecheck.py`): `UFont` (`ScalingFactor` reference-only, made `UMultiFont` +4),
   `UBookMark`, `USpriteComponent`, `UNavMeshPath_MinDistBetweenSpecsOfType` (tool).
3. Hand edits (`build\agentS\patch1..4.py`), listed per class below.
4. Probe rebuilt after each round; the last three probe links needed `/FORCE:UNRESOLVED` (`build\agentS_probe_force.cmd`,
   flag removed again afterwards) because agent R's in-progress headers shim members the 2012 PDB still lists
   (`UParticleModuleTypeDataMesh::CameraFacingUpAxisOption_DEPRECATED`, `USkelControlBase::ControlPosX/Y_DEPRECATED`):
   the probe references such a static by name and cannot link it. Those three are R's classes; the forced link only
   affects their rows.

## Per class

### Tool-regenerated PROPS blocks (retail offsets; native gaps from the 2012 PDB)

`EngineClasses.h` (48): `AAutoTestManager` (+MaxTravelPoints @684, fSentinelTimeScale @712, CamActor @748, CamActors @752,
CameraIdx @764), `ADynamicSMActor` (+ReplicatedMaterial @596; ReplicatedMaterial0/1 shimmed), `AGameInfo` (+7 Arkane
members @936..956 and bNewOnlineSessionOnTravel; 15 reference-only shimmed), `AHUD` (+bForceHideDebugInfo; 4 bits
shimmed), `AInterpActor` (8 reference-only shimmed), `ANavigationPoint` (bPathsChanged, bShouldSaveForCheckpoint,
PathList, InventoryCache shimmed: the -12 that shifted every child), `APhysicsVolume` (bNoInventory), `APlayerStart`
(bBestStart, Score, SelectionIndex), `APortalMarker`, `APylon` (+m_LastNavMeshGeneratedImportedMeshOffset,
m_ImportedMeshOffset; 12 reference-only shimmed), `AStaticMeshActor` (5 shimmed), `ATeleporter` (+m_bDisableTouch),
`ATriggerVolume` (+m_bTriggerUnTouchEventOnDeath), `AVolume` (bPawnsOnly), `AVolumePathNode`, `UActorFactory`
(+AlternateMenuPriority @76 — the 2012 PDB spells it `AlternateMenuPriority_DEPRECATED`; NewActorClassName,
bShowInEditorQuickMenu shimmed: the +12 of the 11 factories), `UActorFactoryActor/Archetype/DynamicSM/Emitter/LensFlare/
PhysicsAsset/RigidBody/SkeletalMesh/StaticMesh`, `UBookMark` (HiddenLevels), `UCheatManager` (+DebugCameraControllerRef,
DebugCameraControllerClass, m_pTargetedActor), `UDamageType` (+5 bits incl. retail-only bCausesFracture; the 2012
`bCausesFracture_DEPRECATED` spelling dropped), `UFaceFXAsset` (+ReferencedAkEvents @100; PreviewMorphSets,
ReferencedSoundCues shimmed), `UFont` (+CharRemap in the gap @80; ScalingFactor shimmed), `UGameViewportClient` (+4 bits;
see Remaining), `UIniLocPatcher`, `ULevelStreaming` (+DuplicateNames @112; LevelTransform shimmed: the 224 → 160),
`ULevelStreamingAlwaysLoaded` (+m_bConsiderForPartialSaves), `ULevelStreamingDistance`, `UMeshComponentFactory`,
`UMultiFont`, `UOnlineSubsystem` (+Sessions @140; see hand edits), `UPostProcessChain` (+m_GraphRoot, m_AllNodes,
m_Switches; Effects shimmed — 47 uses), `UPrimitiveComponentFactory` (+m_CollisionTraceTypes; BlockZero/NonZeroExtent
shimmed), `USceneCaptureReflectComponent` (+ReflectionChannels, m_bIsActiveReflection), `USettings`
(+LocalizedSettingsMappings @80, PropertyMappings @92 from the PDB, two delegates), `USpriteComponent`
(SpriteCategoryName), `UStaticMeshComponentFactory`.

`EngineUIPrivateClasses.h` (28): `UUIDataProvider` (+WriteAccessType @56, ProviderChangedNotifies @60,
`__OnDataProviderPropertyChange__Delegate` @72: the 68 → 84 that shifted all 27 children), `UUIDataStore`, every
`UUIDataStore_*` / `UUIDataProvider_*` / `UUIPropertyDataProvider` / `UUIResource*Provider` of the delta,
`UGameUISceneClient` (+bSynchronizePlayers, m_bBinkPause, m_bDebugMenuPause). Notable adds: `UUIDataStore_OnlinePlayerData`
+NumNewDownloads/NumTotalDownloads (10 reference-only shimmed), `UUIDataStore_Registry` +RegistryDataProvider (RegistryData
shimmed), `UUIDataProvider_Settings` +SettingsArrayProviders, `UUIDataProvider_OnlinePlayerStorageArray` +PlayerStorageName,
`UUIDataStore_DynamicResource` +ResourceProviderDefinitions, `UUIResourceDataProvider` +bDataBindingPropertiesOnly.

`EngineUserInterfaceClasses.h` (8): `UUIRoot` (BadCapsLocContexts shimmed: 68 → 56, shifts UInteraction, UDataStoreClient,
UUISceneClient and their children), `UInteraction`, `UInput` (+BaseBindings, m_PCBindings, m_PadBindingSet1..4 @116..176,
NameToPtr TMap @212 from the PDB; 5 reference-only shimmed), `UPlayerInput` (+aTurn_BeforeClear @348, aLookUp_BeforeClear
@352; 8 WiiU/PS3 members shimmed: the 604 → 444), `UConsole`, `UDataStoreClient`, `UUIInteraction` (+AxisEmulationDefinitions
TMap @204, CanvasScene @344), `UUISceneClient`.

`EngineAIClasses.h` (9): `UNavigationHandle` (+m_LocationOfStartPoly @56, m_pStartPoly @68; BestUnfinishedPathPoint,
bVisualPathDebugging, bDebug_Breadcrumbs, Breadcrumbs[10], BreadCrumbMostRecentIdx, BreadCrumbDistanceInterval shimmed:
the 336 → 228), `UNavMeshPathGoalEvaluator` (bDoPartialAStar, MaxOpenListSize shimmed: the +4 of its children),
`UNavMeshGoal_At/ClosestActorInList/GenericFilterContainer/Null/PolyEncompassesAI`, `UNavMeshPath_Toward`,
`UNavMeshPath_MinDistBetweenSpecsOfType` (Penalty).

`EngineGameEngineClasses.h`: `UDownloadableContentManager` (+InstalledDLCBackup @80; QueuedFullyLoadPackageInis shimmed).
`EnginePawnClasses.h`: `AScout` (DefaultReachSpecClass, EdgePathColors shimmed; 1328 → 1312).

### Hand edits (evidence in the `// DISHONORED(layout)` comments)

| Type | File | Change |
|---|---|---|
| `UOnlineSubsystem`, `UIniLocPatcher`, `UUIDataStore_OnlineStats` | EngineClasses.h, EngineUIPrivateClasses.h | the SDK dumps each `TScriptInterface` as an `X_Object`/`X_Interface` pointer pair; the tool emitted those pairs and shimmed the real members. Replaced by one `TScriptInterface<IInterface> X;` each (2012 PDB spelling, @60..108 / @68 / @212, @220). `UOnlineSubsystem` keeps 9 reference-only interface shims (GameInterface, VoiceInterface, …) |
| `FASwitch` | EngineClasses.h | new script struct (`Engine.PostProcessChain.ASwitch`, retail 16: m_Name @0, m_Value @12; 2012 PDB 16) for `UPostProcessChain::m_Switches` |
| `FIniLocFileEntry` | EngineClasses.h | retail Filename @0, ReadState @12 (16); DLName, HashCode, bIsUnicode reference-only → shims (44 → 16) |
| `FKismetDrawTextInfo` | EngineClasses.h | AppendedText reference-only → shim (52 → 40); `UnInterpolation.cpp:8259` writes the shim |
| `FLocalizedSubtitle` | EngineClasses.h | LanguageExt, bSingleLine reference-only → shims (28 → 16); `UnAudioNodes.cpp` 1153/1159/1177/1188 read them (porting TODO: the language lookup no longer sees per-entry extensions) |
| `FOnlineContent` | EngineClasses.h | +`TArrayNoInit<FString> DLCMainFolder` @76 (retail TArray; the 2012 PDB has an FString there: retail wins) (76 → 88) |
| `FOnlinePlayerScore` | EngineClasses.h | retail PlayerID @0, Score @8, Rank @12, playerName @16 (span 28; sizeof 32 with the QWORD in FUniqueNetId); the 2012 TeamID @8 is gone (no Src use) |
| `FPlayerStorageArrayProvider` | EngineUIPrivateClasses.h | +`FName PlayerStorageName` @4 (8 → 16) |
| `FSettingsArrayProvider` | EngineUIPrivateClasses.h | new script struct (retail SettingsId @0, SettingsName @4, Provider @12; 2012 PDB 16) for `UUIDataProvider_Settings::SettingsArrayProviders` |
| `FNavMeshPathParams` | EngineAIClasses.h | SearchLaneMultiplier reference-only → shim (48 → 44); written in UnController.cpp 3266/3285, UnCrowd.cpp:21, UnNavigationHandle.cpp:1020, read in UnNavigationMesh.cpp:10791 |
| `FPathStore` | EngineAIClasses.h | +`FVector m_vComputedDestination` @12 (12 → 24) |
| `FKeyBind` | EngineUserInterfaceClasses.h | +m_bLMouseHeld, m_bRMouseHeld (after Alt), m_bIgnoreLMouse, m_bIgnoreRMouse (after bIgnoreAlt): retail @20 masks 0x8/0x10/0x100/0x200, 2012 PDB bits 3/4/8/9 (the legacy copy in `UnInteraction.h` is included by nothing and was left) |
| `FDebugTextInfo` | EngineControllerClasses.h | bKeepAttachedToActor reference-only → shim (retail @52 has one bit) |
| `UUIListElementProvider` / `IUIListElementProvider`, `UUIListElementCellProvider` / `IUIListElementCellProvider` | EngineUIPrivateClasses.h (+ `IMPLEMENT_CLASS` in `UnUIDataStores.cpp`, registrant macro) | the reference headers dropped both UI interfaces; retail registers them (SDK: UInterface children) and 11 providers/stores carry their vtables (`VfTable_IUIListElementProvider` etc.). Declared with the vptr only (no methods ported: `DISHONORED(port)`), inherited by `UUIDataProvider_OnlineFriends` (Cell @88), `UUIDataProvider_PlayerAchievements` (Cell @88), `UUIDataProvider_OnlinePlayerStorageArray` (@84/@88), `UUIDataProvider_SettingsArray` (@84/@88), `UUIResourceCombinationProvider` (@84/@88), `UUIResourceDataProvider` (@108/@112), `UUIDataStore_DynamicResource` (@116), `UUIDataStore_GameResource` (@116), `UUIDataStore_OnlinePlayerData` (@116), `UUIDataStore_OnlineStats` (@116/@120), `UUIDynamicDataProvider` (@108). 2012 PDB bases agree at every offset |
| `UUIDynamicDataProvider` | EngineUIPrivateClasses.h (+ `IMPLEMENT_CLASS`, registrant) | new: retail base of `UUIDataProvider_Settings` (script_classes_2013.json super `Engine.UIDynamicDataProvider`; abstract, transient, native, implements UIListElementCellProvider; retail span 108..120: VfTable @108, DataClass @112, DataSource @116; 2012 PDB identical). `UUIDataProvider_Settings` now derives from it (`DECLARE_CLASS` super updated). Its natives BindProviderInstance/UnbindProviderInstance are not ported |
| `UUIDataStore_InputAlias` | EngineUIPrivateClasses.h | `TMap<FName,INT> InputAliasLookupMap` re-added after the block as the native tail (2012 PDB @128, 60 bytes; retail sizeof 188 = 128 + 60). **Tool caveat**: `sdk_props.py` reports no native tail when `sizeof == span_end`, although the SDK span covers unreflected trailing members; it had shimmed this one |
| `APylon` | EngineClasses.h | base `IInterface_NavigationHandle` dropped: 2012 PDB bases are ANavigationPoint + IEditorLinkSelectionInterface @784 only; retail SDK VfTable_IEditorLinkSelectionInterface @784, NavMeshPtr @788 (1008 → 928) |
| `AActor` (contract) | EngineClasses.h | +`m_bForceUpdateComponentsOnPostGameLoad:1` (retail SDK @300 mask 0x100, retail-only) |
| `AWorldInfo` (contract) | EngineGameEngineClasses.h | bit renamed `bUseProcBuildingRulesetOverride` (retail SDK @748 mask 0x2000000; 2012 PDB spelling `_DEPRECATED`); its shim removed |
| `UEngine` (contract) | EngineGameEngineClasses.h | DWORD @700 in retail order: m_bRequestOpenPauseMenu_FocusLost, _NoController (the 2012 PDB has one m_bRequestOpenPauseMenu), m_bPauseForDisconnectedController, m_bWaitingControllerSelection, m_bAcceptControllerDisconnectionEvents, +m_bAcceptProfileReading, m_bInitControllerToZero, +m_bCheckDLCAccessibility, +m_bStorageDeviceChangedEventReceived, +m_bOptionsMenuRequestedStorageDeviceChanged (retail-only bits; no Src use of the old name) |
| `UArkSettingsListenerInterface` / `IArkSettingsListenerInterface` | EngineClasses.h / UnClient.h (from EngineGameEngineClasses.h) | moved so `UClient` can inherit the interface: `Engine.h` includes `UnClient.h` (:390) before the class section of `EngineClasses.h` (:412, the :348 include is `ENUMS_ONLY`), so the C++ interface lives in `UnClient.h` and the UInterface class in `EngineClasses.h` |
| `UClient` | UnClient.h | bases `UObject, FExec, IArkSettingsListenerInterface` (2012 PDB @56/@60), so MinDesiredFrameRate lands @64 (retail SDK @64), sizeof 80. The two pure virtuals get inline bodies (`return this` / empty, `DISHONORED(port)`; the 2012 exe folds UClient's implementation) so `UWindowsClient` stays concrete |
| `ULineBatchComponent` | UnLevel.h | +`BITFIELD m_bSoulRendering:1` @460 (retail SDK mask 0x1; FPrimitiveDrawInterface vtable @452, View @456), BatchedLines @464 |
| `USystem` | Core/Inc/UnCorObj.h | +`FString ScreenShotPath` @116, +`TArray<FString> MobileScriptPaths` @176 (retail SDK and 2012 PDB) (236 → 260) |
| `UActorComponent` (contract) | UnActorComponent.h | +`bIsSkeletalMeshComponent:1` (retail SDK @76 mask 0x20, retail-only) |
| `UPrimitiveComponent` (contract) | PrimitiveComponent.h | bits renamed `BlockZeroExtent` / `BlockNonZeroExtent` (retail SDK @280 masks 0x80/0x100; the 2012 PDB spells them `_DEPRECATED`); their two shims removed, so the 60 Src uses now hit the real bits |
| `UMeshComponent` | — | bitfields already in retail order, nothing to do |

Engine/Src edits: `UnUIDataStores.cpp` (`IMPLEMENT_CLASS` for the three new classes; the `InputAliasLookupMap` code is
unchanged in the end).

## Shimmed reference-only members with Engine/Src uses (porting TODOs)

Identifier counts from `build\agentS\shim_uses.py` (name matches, so `PathList`, `GoalActor`, `Score` include other classes'
members of the same name); members with 0 uses are omitted (`build\agentS\shim_uses.txt` has all 126).

| Class.member | uses |
|---|---|
| `ANavigationPoint.PathList` | 176 (UnNavigationPoint.cpp 99, UnRoute.cpp 31, UnPath.cpp 13, UnVehicle.cpp 7) — the reference path network |
| `ANavigationPoint.bPathsChanged` / `InventoryCache` | 27 / 9 |
| `UPostProcessChain.Effects` | 47 (UnSequence.cpp 20, UnAudio.cpp 8, SceneRendering.cpp 6, UnPlayer.cpp 5) — retail post-process is the Arkane graph (m_GraphRoot / m_AllNodes / m_Switches) |
| `UPrimitiveComponentFactory.BlockZeroExtent` / `BlockNonZeroExtent` | 27 / 33 (mostly `UPrimitiveComponent`'s real bits of the same name) |
| `UFaceFXAsset.ReferencedSoundCues` / `PreviewMorphSets` | 22 / 4 (UnFaceFXAsset.cpp, UnFaceFXAnimSet.cpp) — retail has `ReferencedAkEvents` |
| `APylon.NavMeshGenerator` / `VoxelFilterBounds` / `VoxelFilterTM` / `bAllowRecastGenerator` / `OnBuild_*CollisionForThese` / `MaxPolyHeight_Optional` / `bPylonInHighLevelPath` / `bSolidObstaclesInGame` / `bUseRecast` / `DebugPath*` | 16 / 6 / 4 / 5 / 4+4 / 4 / 3 / 2 / 1 / 1+1 (UnNavigationMesh.cpp, NavMeshRenderingComponent.cpp) |
| `UNavigationHandle.Breadcrumbs` / `BreadCrumbMostRecentIdx` / `BreadCrumbDistanceInterval` / `bVisualPathDebugging` / `BestUnfinishedPathPoint` | 5 / 9 / 2 / 5 / 2 (UnNavigationHandle.cpp) |
| `UNavMeshGoal_At.bGoalInSamePolyAsAnchor` / `PartialDistSq` / `bWeightPartialByDist`, `UNavMeshGoal_GenericFilterContainer.SeedLocations`, `UNavMeshPath_Toward.bBiasAgainstHighLevelPath` / `OutOfHighLevelPathBias` / `GoalActor` | 3 / 2 / 1, 2, 1 / 1 / 153 (GoalActor is mostly APawn's) |
| `UInput.CurrentTouches` / `CachedInputEvents` / `CachedAnalogInputEvents` / `CachedTouchInputEvents` / `CurrentControllerId` | 10 / 5 / 5 / 5 / 4 (UnIn.cpp, UnSequence.cpp) |
| `UPlayerInput.aTilt` / `aRotationRate` / `aGravity` / `aAcceleration` / `aTouch` / `aBackTouch` | 1 each, 2 (UnIn.cpp) |
| `UUIDataStore_OnlinePlayerData.*Provider*` (10 members) | 2..7 each (UnUIDataStores.cpp) |
| `ULevelStreaming.LevelTransform` | 5 (UnWorld.cpp) |
| `UGameViewportClient.bDisplayHardwareMouseCursor` / `bCapturedWorldRendering` | 3 / 3 (UnPlayer.cpp) |
| `AGameInfo.JoinInProgressStandbyWaitTime` / `StreamingPauseIcon` / `bIsStandbyCheckingOn` / `AnimTreePoolSize` | 5 / 2 / 1 / 1 |
| `UDownloadableContentManager.QueuedFullyLoadPackageInis` | 4 (DownloadableContent.cpp) |
| `UActorFactory.NewActorClassName` | 4 (UnActorFactory.cpp) |
| `AScout.EdgePathColors`, `ADynamicSMActor.ReplicatedMaterial0/1`, `UOnlineSubsystem.VoiceInterface` / `AuthInterface`, `APlayerStart.Score` | 4, 1+1, 1 / 1, 14 (mostly other `Score`s) |
| struct shims: `FLocalizedSubtitle.LanguageExt` / `bSingleLine`, `FNavMeshPathParams.SearchLaneMultiplier`, `FKismetDrawTextInfo.AppendedText` | see the hand-edit table |

## Remaining

- `UGameViewportClient` 292 vs 284 (`EngineClasses.h`, LoadingMessage @112 vs 104): `EShowFlags ShowFlags` @96 is
  `TStaticBitArray<128>` (16 bytes) in `ShowFlags.h:88` (the `CONSOLE && FINAL_RELEASE` branch `FShippingShowFlags` is
  16 bytes too), while the 2012 PDB types `UGameViewportClient::ShowFlags` as `unsigned __int64` (8 bytes @96) and the
  retail SDK dumps it as `FQWord` @96. Arkane's show flags are a 64-bit word: `ShowFlags.h` (not in my package) and every
  `SHOW_*` constant / `FSceneViewFamily::ShowFlags` user are one engine-wide change — coordinator decision.
- `UArrowComponent` 480 vs 464 and the `UArrowComponent`/`UBrushComponent`/`UDraw{Box,Capsule,Cylinder,Sphere}Component`
  bitfield rows (`PrimitiveComponent.h`, hand-written classes outside my rows): retail adds `bTreatAsASprite` @460
  (Arrow), `m_bTwoSided` @532 (Brush) and has no `bDrawOnlyIfSelected` in the four draw components.
- Layout asserts: `Engine/Inc/DishonoredLayouts.h:419` still asserts `sizeof(FOnlinePlayerScore) == 16` (2012 PDB); retail
  is 28 reflected (32 with the QWORD alignment). My build dir runs with `-DDISHONORED_LAYOUT_CHECKS=OFF` for the Engine
  build; the coordinator regenerates the header after merging.

## Follow-ups outside my files

1. `ShowFlags.h`: 64-bit `EShowFlags` (evidence above).
2. `PrimitiveComponent.h`: the five hand-written component classes above.
3. `sdk_props.py`: (a) `TScriptInterface` members arrive as `X_Object`/`X_Interface` pairs and the real member is shimmed —
   fold the pair back into one `TScriptInterface<IInterface>` member; (b) when `sizeof == span_end` the tool does not
   report the native tail although the dump's span covers unreflected trailing members (`UUIDataStore_InputAlias`);
   (c) it does not list retail interface bases that the reference headers lack (`IUIListElementProvider`) as missing
   declarations, only as "must inherit"; (d) a shim of a member the 2012 PDB still has (retail-removed) cannot link in
   the probe (`DISHONORED_SHIM_STATIC static`), so such members must be deleted, not shimmed (`FOnlinePlayerScore::TeamID`,
   `UActorFactory::AlternateMenuPriority_DEPRECATED`; agent R's `*_DEPRECATED` cases are the same).
4. Porting TODOs of the shim table; the UI list-element interfaces and `UUIDynamicDataProvider` have no methods.
5. `UnInteraction.h` (legacy `UInput`/`UInteraction`/`FKeyBind`) is included by nothing — delete when convenient.
