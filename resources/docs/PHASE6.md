# Phase 3 wave 4 — milestone 5 on the null RHI, the rendered world on D3D9 (PHASE6.md)

Written 2026-09-25 after wave 3 (`PHASE5.md`) landed on main (43343f6..63758d5). Plan of record: `PLAN.md`. Read
`STATUS.md` first (incident note). Letters continue wave 3 (W–AC used): packages **AD–AK**.

## Goal of this wave

Wave 3 left us with an exe that starts like retail, loads `Startup.upk`, runs `GEngine->Init()`, loads
`DishonoredGameFull_P` to `Initial startup: 5.2s` and ticks. About 0.5 s later it dies while the main menu
streams in. This wave turns that into **a player in a mission map**:

1. **Milestone 5 (PLAN.md Phase 6), null RHI**: the main-menu map change commits
   (`Committed map change via DishonoredEngine`, golden :365). Then the prologue mission `L_Tower_P` streams in
   the way the menu's New Game does it (golden :389). A `DishonoredPlayerPawn` is possessed by the
   `DishonoredPlayerController`, and keyboard/mouse input moves and turns it.
2. **Milestone 4, rendered-world half, D3D9**: the scene renderer runs on the retail cooked caches (global and
   material) with no bring-up skip. The first world frame of `Dishonored_MainMenu_Env` / `L_Tower_P` is presented
   in a `-windowed` run. The Scaleform main menu stays gated on the user's GFx decision (`middleware.md` 2.3).
3. **Engine convergence wave 2**: the ~140 retail script natives without a C++ body, what agent AA left open,
   the UEngine/UWorld retail virtuals, and the DishonoredGame infrastructure the map needs (tweaks, FSM, inventory).

The retail 2013 exe is the target. Every port cites a 2013 rva (named db `retail2013_named.i64`) and uses the
2012 decompile only as the readable version of the same function.

## Facts fixed while planning (2026-09-25, from the wave-3 reports)

| Fact | Consequence |
|---|---|
| The merged null-RHI smoke dies about 0.5 s after `Initial startup` with `appError called: Bad export index 1065353215/6389` right after `Flushing async loaders.`. Agent Z's snapshot died with the same numbers at the same point: `Dishonored_MainMenu_Env.upk` → `TheWorld:PersistentLevel.DisPylon_0.NavigationMeshBase_4469`, file offset 7786610. 1065353215 = 0x3F7FFFFF is a float read where an export index was expected. Retail `UNavigationMeshBase::Serialize` is 2013 rva 0x2909e0 (2012 0x2b0000, 1915 bytes). Our class is 688 bytes against retail 464. Arkane's nav mesh is a different design (2012 edge 52 bytes vs the reference's 112; agent AB, 10 pending rows in `UnPath.h`) | AD: nav mesh layout + serializers first, then a serialization sweep of every package the main menu and `L_Tower_P` stream |
| Content gaps agent Y saw on D3D9 while `Dishonored_MainMenu` streams: `StaticMeshComponent` of InterpActors over-reads 4 bytes (`Got 306, Expected 302`). Agent AA's `FStaticMeshComponentLODInfo` port (no `VertexColorPositions`) may already cover it. `ULevel::Serialize` still uses the reference gates 797 / 798 (`UnLevel.cpp:336/:448`) | AD re-measures after its nav mesh fix; every `Serial size mismatch` becomes a port |
| The golden run streams the prologue into the persistent world. After `DownUp` it logs `Committed map change via DishonoredEngine` a second time (:389, `l_tower_p.TheWorld`), then pawn/tweak lines (`PlayerArms_Tatoo_inst GetOutermost(): Twk_Pawn_Corvo`). Mission maps are not `open`ed; the GFx main menu starts them through a map change | AF finds the New Game path in the 2013 decompiles (menu natives → `DishonoredGameInfo` / `DishonoredEngine` / `WorldInfo.PrepareMapChange`/`CommitMapChange`). A `DISHONORED(bringup)` switch `-startmap=L_Tower_P` issues the same calls without GFx |
| The retail 2013 exe is Shipping and has no log strings (agent X: no `Initializing Engine` / `Initial startup` in the db). `SaveGameList done` and `DevDlc: Looking for DLC...` exist only in the 2012 ArkProfile golden log | acceptance lines after `Initial startup` are golden lines our reference `debugf`s print (`Committed map change via DishonoredEngine`) or `DISHONORED(bringup):` lines named per package |
| The 147 retail natives with no exec (agent Z's list) now bind to `UObject::execDishonoredUnboundNative`, which warns once and returns zero (`-strictnatives` aborts). Already ported: GameInfo.SpawnPlayerController, Controller.Possess/UnPossess/GetPlayerViewPoint, Pawn.UnPossessed + APawn::PossessedBy, PlayerController.GetPlayerViewPoint/GetFOVAngle, Camera.GetCameraViewPoint/GetFOVAngle. Still hit on the startup/map path: Camera:UpdateCamera, HUD:DisplayConsoleMessages, DownloadableContentManager:BackupDLCList/RemoveUnavailableDLC/UninstallDLCs, Pawn:Died, Camera:ClearCameraLensEffects, InterpActor:SetShadowParentOnAllAttachedComponents. Z's 2013 rvas come from `match_2012_2013.csv` and are marked unverified | AE ports them in dependency order, movement/camera first. Each rva is confirmed in the named db before porting |
| Under `-nullrhi` the coordinator skips the scene render in `RenderViewFamily_RenderThread` (`SceneRendering.cpp`). The first assert is `FDownsampleSceneDepthPixelShader` (`SceneRendering.cpp:3396`), whose cooked layout differs from ours. Cooked global cache: 263 records, 62 load, 58 layouts differ, 143 Arkane/GFx types undeclared, 144 reference-only types with no cooked record (`renderer.md` 2–5) | AH converges the global types the scene renderer binds, guards the reference-only passes and removes the skip |
| Material shader maps: retail skips maps below 786/23 (2013 rva 0x164a60) and constructs material shader types with 798/23 (0xb7ea00 / 0xb7ea40). With those gates every declared mesh-material type differs in its common part (`FMaterialShader` / `FMeshMaterialShader` / `FVertexFactoryParameterRef`). `TBasePassPixelShader` has a third template argument (`...FALSEFALSE`). `FMaterialShaderMap::Serialize` (2013 rva 0x40ef30) then stops in `FUniformExpressionSet::Serialize`. Retail `FMaterialShaderMap::IsComplete` (2013 rva 0x3ea7d0, 116 bytes) returns FALSE only while compiling. Without maps, D3D9 aborts in `FMaterial::InitShaderMap` (retail does the same) | AG: material shader maps load from the cooked caches. That is the precondition for any D3D9 world frame |
| `UFont::GetScalingFactor` returns the storage-less shim `ScalingFactor` = 0, so canvas text is invisible. Retail has no such member | AI: return 1 / drop the scale, as retail does |
| `UWorld::Init` (2013 rva 0x3945f0) creates the audio system and MapInfo from `[DishonoredMods] AudioSystemClass` / `MapInfoClass` and fills `m_pWorldInfoCheckStreamingPersistent`. Ours does neither. `UDishonoredAudioSystem::Init` is 2013 rva 0x7a3120 | AI (Engine side + a silent audio system); Wwise stays a stub until the user installs the SDK |
| UEngine lacks retail virtuals: `PlayLoadMapMovie(Map, Movie)` 0x2097d0, `OpenPauseMenu`, `OnControllerDisconnected`, `OpenControllerConnectionMenu`, `OpenContentUnavailableMenu`, `IsLoadingGame`, `StopMovie`, `IsDebugMenuVisible` (+392), `RenderDebugMenu` (+388). `UDishonoredEngine`'s ports cannot override them | AI declares them in retail vtable order; AF wires the `UDishonoredEngine` overrides |
| DishonoredGame: 289 of 974 natives are ported, 685 stubs are left. What blocks the rest is infrastructure: the tweak interface (`IDis(Engine)TweaksInterface` vtable, `FSpawnActor_TweakObj::DoInit` before `PostBeginPlay`), native FSM transitions, `UDishonoredInventory`, DisSaveLoad. Ten `Serialize` overrides without a CppText hook abort a load on a size mismatch (agent X's list) | AJ (infrastructure + serializers); AF (player controller/input/pawn/camera/HUD/engine units) |
| PhysX is off (`WITH_NOVODEX=0`, `RBPhysScene == NULL`). Pawn walking is UE3's own `PHYS_Walking` against BSP/kDOP collision, not PhysX. Rigid bodies are inert. Audio is silent. Physics needs no guard on the map path (agent Z) | milestone 5 is judged without physics objects or sound; PhysX 2.8.4 / Wwise 2012.1 SDKs stay user blockers |
| Parallel builds race on the shared `external/<name>-build` folders (`cmake/Dependencies.cmake:16`). Every wave-3 agent built from a snapshot with private dependency builds (`pnglibconf.h` C1083) | coordinator pre-wave: per-build-dir dependency `BINARY_DIR` |
| Wave-3 agents edited the shared `function_status.csv` and `DishonoredGameNativeStubs.ported.txt` concurrently | coordinator pre-wave: per-agent files merged at merge time |

## Coordinator, before spawning (≈½ day)

- [x] **Per-build dependency builds.** (done 2026-09-26: `BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/<name>-build`, an already fetched `external/<name>-src` is used through `FETCHCONTENT_SOURCE_DIR_<NAME>` so no download sub-build runs; two concurrent Core configures+builds passed while `build/game` rebuilt.) `cmake/Dependencies.cmake`: `BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/${name}-build"`; sources stay in the shared `external/<name>-src`, fetched once with `FETCHCONTENT_FULLY_DISCONNECTED=ON` after the first fetch. Verify two configures + builds in parallel (`build/coord`, `build/coord2`) without C1083.
- [x] **Per-agent ported lists.** (done: the generator folds `<Module>NativeStubs.ported.*.txt`.) `gen_classes_header.py` reads `<Module>NativeStubs.ported.txt` **and** every `<Module>NativeStubs.ported.*.txt` (agents write `DishonoredGameNativeStubs.ported.agent<X>.txt`). Agents regenerate generated DishonoredGame files **only in their snapshot**; the coordinator regenerates the shared tree at merge.
- [x] **Per-agent status rows.** (agents write them; the coordinator folds at merge.) Agents write `resources/docs/agents/agent<X>_status.csv` (same columns as `function_status.csv`). The coordinator folds them in at merge and regenerates `progress.md` (`module_map.py`).
- [x] **Baseline.** (STATUS.md / PLAN.md updated with the wave-3 docs commit 00e8482.) Record the merged smoke (below) and its `Bad export index` end in `STATUS.md` (STATUS still describes wave 2). Update `PLAN.md` Phase 6 milestone 3 → done, 4 → partial (device + frame + Bink done; menu gated on GFx).
      `python resources/tools/build_and_smoke.py --build-dir build/game --no-build --exe-name DishonoredGame_C.exe --log-name coord.log --ini-dir build/coord_config --rhi null --milestone "Initializing Engine..." --expect "Finished loading level" --expect "Initial startup" --skip-native OnlineSubsystemPC`
- [x] Commit this file, the pre-wave changes and STATUS/PLAN; spawn AD–AK in one message. Also added by the coordinator: a Debug-build unhandled-exception filter in `Launch.cpp` (an access violation on any thread now logs the stack through `CreateMiniDump` + `GError->HandleError()`; before it the process died silently) and the package/object names in the `ULinkerLoad::IndexToObject` index errors.

## Work packages (all parallel; dependencies and file ownership noted)

### AD — Streaming serialization: nav mesh, then every package the main menu and `L_Tower_P` stream (milestone blocker)
Owner: agent AD. Build dir `build\agentAD`. IDA copies `shipping2012_agentAD.i64`, `retail2013_agentAD.i64`.
Files: `Engine/Inc/UnPath.h` (nav mesh structs, `UNavigationMeshBase`), `Engine/Src/UnNavigationMesh.cpp`,
`Engine/Src/NavMeshRenderingComponent.cpp`, `Engine/Src/arknavmeshbuildutils.cpp`, `Engine/Src/UnLevel.cpp`
(797/798 gates), `Core/Inc/UnIOBase.h` + `Core/Src/UnAsyncLoading.cpp` (async-load fixes only),
`Engine/Src/UnStaticMesh.cpp` / `UnSkeletalMesh.cpp` / other `Serialize` bodies the sweep proves wrong (list them in the
report), new `Engine/Src/DishonoredLoadAll.cpp` + a hook of ≤ 10 lines in `Launch/Src/LaunchEngineLoop.cpp` (AD owns that file this wave).
- [ ] 1. `-loadall=<pkg+pkg|@listfile>` (`DISHONORED(bringup)`): after `Initial startup`, `LoadPackage` each package, log
      `DISHONORED(bringup): loadall <pkg>: <N> exports, <E> errors` per package (serial-size mismatches, bad indices caught
      and counted instead of aborting only under this switch), then exit. List files: `Dishonored_MainMenu*` and every
      streaming level of `L_Tower_P` (from its `ULevelStreaming` objects via `read_package_classes.py`).
- [ ] 2. `UNavigationMeshBase` to retail 464 bytes (retail SDK members, 2012 PDB for native members; `sdk_show.py`) and the nav
      structs of `UnPath.h` (`FNavMeshPolyBase`, `FNavMeshEdgeBase` 52 bytes in 2012, …). Port `UNavigationMeshBase::Serialize`
      2013 0x2909e0 (`NavMeshVersionNum`, `VersionAtGenerationTime` ≥ 11 → `FPathBuilder::LoadedPathVersionNum`, `Verts`,
      `EdgeStorageData`, `Polys`, dummy object ref below 7, `LocalToWorld`/`WorldToLocal` ≥ 8, `BorderEdgeSegments` ≥ 9 only when
      the outer `APylon` bit @288 is clear, `ConstructLoadedEdges`, `BuildBounds` below 12), `operator<<(FArchive&, FNavMeshPolyBase&)`
      0x2780f0 (licensee-27 branch), `FNavMeshEdgeBase::Serialize` 0x279d40, `SerializeEdgeVerts` 0x279eb0, and whatever
      `PostLoad`/edge construction the load runs. Runtime path finding is **not** in scope; stub what the load does not reach, with `DISHONORED(bringup)`.
- [ ] 3. `ULevel::Serialize` gates 797 (`VER_DYNAMICTEXTUREINSTANCES`) / 798 checked against 2013 0x259f30; the
      `StaticMeshComponent` 306/302 case re-measured; `FAsyncIORequest` 76 bytes (agent AB) re-checked against 2013
      `QueueIORequest` 0x519b0. X and Z both saw crashes with it in their snapshots (`~FAsyncIORequest`, `FindCachedFileHandle`
      `ArrayMax>=ArrayNum`); the 2013 handle cache keyed by `NormalizedFileName` (0x74130) is ported if the async streaming path needs it.
- [ ] 4. Sweep: every loadall error becomes a port (2013 rva cited). DishonoredGame serializers go to AJ as a hand-over, not edited here.
- **Accept:** `python resources/tools/build_and_smoke.py --build-dir build/agentAD --no-build --exe-name DishonoredGame_AD.exe --log-name agentAD.log --ini-dir build/agentAD/config --rhi null --timeout 120 --milestone "Committed map change via DishonoredEngine" --expect "Initial startup" --expect "Committed map change via DishonoredEngine" --skip-native OnlineSubsystemPC`
  exit 0 with no `Bad export index` / `Serial size mismatch` in `agentAD.log`; `--extra-args "-loadall=@build/agentAD/tower.txt"` logs 0 errors for every non-DishonoredGame export class (the report names the DishonoredGame ones handed to AJ).
- **Report:** `resources/docs/agents/agentAD.md` (nav mesh layout table retail/2012/ours, per-function table, loadall results per package).

### AE — Engine/GameFramework retail natives without a body (agent Z's list), movement and camera first
Owner: agent AE. Build dir `build\agentAE`. Starts immediately; runs use AD's snapshot for map-change paths until AD merges.
Files: `Engine/Src/UnCamera.cpp`, `UnController.cpp`, `UnPawn.cpp`, `UnPhysic.cpp`, `UnActor.cpp` (natives only; AA's
`Serialize`/`PostLoad` stay), `UnDecal*.cpp` / `UnHUD` code (wherever `AHUD` natives live), `DownloadableContent.cpp`,
`UnSequence.cpp` (SeqEvent_TakeDamage only), `UnUIDataStores.cpp` (the listed UIDataStore natives only), `KActor`/`SkeletalMeshActor`
units, `GameFramework/Src/GameCrowd*.cpp`; headers: the `DECLARE_FUNCTION` / natives-table / `MAP_NATIVE` lines of
`EngineCameraClasses.h`, `EngineControllerClasses.h`, `EnginePawnClasses.h`, `EngineClasses.h`, `EngineUIPrivateClasses.h`,
`GameFrameworkClasses.h` (no layout edits), plus `IArkHealthInterface` default bodies (`EnginePawnClasses.h`; 2012 rva 0x1705a0/0x1705b0).
- [ ] 1. Inventory: rerun Z's unbound-native diagnostic on the merged tree (the warn-once lines of `execDishonoredUnboundNative`
      under `-strictnatives` off). For each function, confirm the 2013 exec and body rvas in the named db (Z's matches are unverified).
- [ ] 2. Port in dependency order. Map path first: `Camera.UpdateCamera` (0x1cffd0) + `ClearCameraLensEffects` (0x1d03c0) /
      `AddCameraLensEffect` / `FindCameraLensEffect` / `RemoveCameraLensEffect`; `PlayerController.PlayerMove_Walking` (0x1d3a00),
      `ProcessViewRotation` (0x1d3840), `UpdateRotation` (0x1d37e0), `HandleWalking` (0x1d35c0), `LimitViewRotation` (0x1d3930),
      `IsLookInputIgnored` / `IsMoveInputIgnored` (0x1d3ab0 / 0x1d3a70), `CleanOutSavedMoves` (0x1d3310), `ResetTimeMargin` (0x1d32e0),
      `CleanUpBeforeLevelTransition` (0x1d3af0); `Pawn.Died` (0x1dafd0), `FaceRotation` (0x1daf30), `AddVelocity` (0x1dade0),
      `HandleMomentum` (0x1dac90), `PlayHit` (0x1db0a0), `TakeDamage`, `IsRagdoll`, `Get/InitNavigationHandle`; `Actor.TakeDamage`
      (0x1c1640), `SetState` (0x1c1150), `CheckHitInfo` (0x1c1800), `FindEventsOfClass` (0x1fcc40), `DoKismetAttachment` (0x1df6c0),
      `VolumeBasedDestroy` (0x1c03a0), `PostAkEvent` / `SetRTPCValue` / `SetSwitch` / `PostTrigger` / `ActivateOcclusion` (onto the silent
      `UAkAudioDevice`); `GameInfo.ReduceDamage` (0x1c6170); `HUD.DisplayConsoleMessages` / `ShouldDisplayDebug` (0x1c3c50) / `ShowDebug`
      (0x1c3bd0); `DownloadableContentManager.BackupDLCList` / `RemoveUnavailableDLC` / `UninstallDLC(s)`; `InterpActor.SetShadowParentOnAllAttachedComponents`;
      `WorldInfo.GetGlobalGravityZ` (0x1c69a0). Then the rest of the list (DecalManager, KActor/KAsset, SkeletalMeshActor(MAT),
      SeqEvent_TakeDamage, Settings, OnlinePlayerStorage, UIDataStore*/UIRoot, GameFramework crowd, `Object.RSmerp`/`VSmerp`).
      Commandlet/AutoTest/Sentinel/`*_Debug` entries get a documented `DISHONORED(bringup)` no-op when retail's body is empty in Shipping.
- [ ] 3. Script functions retail made native whose reference body lives in UnrealScript are **not** re-run from script: the C++ body is the retail one.
- **Accept:** ≥ 70 of the list with a retail body (`written`/`ported`, rvas in `agentAE_status.csv`); the AD-level smoke with
  `--extra-args "-strictnatives"` exits 0 through `Committed map change via DishonoredEngine` (on AD's snapshot until AD merges); no
  `native not ported` line from an Engine/GameFramework class in `agentAE.log`; CoreSmoke 99/99; `xcheck_sdk_layout.py` 0 rows.
- **Report:** `resources/docs/agents/agentAE.md` (table: class.function, 2013 exec rva, body rva, status; remaining entries with reasons).

### AF — Milestone 5 driver: `L_Tower_P` streamed in, player spawned and possessed, input moves the pawn
Owner: agent AF. Build dir `build\agentAF`. Depends on AD (map change) and uses AE/AI/AJ snapshots as they appear. Starts
on the New-Game path and the input chain immediately.
Files: `Engine/Src/UnGame.cpp` (LoadMap/PrepareMapChange/CommitMapChange, `UpdateWorldInfoCache` after `UWorld::Init` like retail),
`Engine/Src/UnPlayer.cpp`, `Engine/Src/UnIn.cpp`, `Engine/Src/UnInteraction.cpp`, `Engine/Src/UnLevTic.cpp`, `WinDrv/Src/WinViewport.cpp`,
`WinDrv/Src/WinClient.cpp`, `WinDrv/Inc/*`; DishonoredGame units `dishonoredengine.cpp`, `dishonoredviewportclient.cpp`,
`dishonoredplayercontroller.cpp`, `dishonoredplayerinput.cpp`, `dishonoredplayerpawn.cpp`, the player camera unit, `dishonoredhud.cpp`,
`disgfxmovieplayermainmenu.cpp` (New-Game natives only) and their `Inc/CppText/*.h`; `DishonoredGameNativeStubs.ported.agentAF.txt`.
- [ ] 1. New Game in the 2013 decompiles: the menu natives/script → `ADishonoredGameInfo` / `UDishonoredEngine` map change →
      `AWorldInfo::PrepareMapChange` (exec 0x1f9de0, body 0x3845f0) / `CommitMapChange` (exec 0x1c6d80, body 0x384680) →
      `UGameEngine::CommitMapChange` (0x22d570). Port `UDishonoredEngine::LoadMap` 0x615210, `PreCommitMapChange` 0x616150,
      `PostCommitMapChange` and `Tick` as far as the path needs. `-startmap=L_Tower_P` (`DISHONORED(bringup)`) issues the menu's calls
      after the main-menu commit. Golden target: :389 `Committed map change via DishonoredEngine` with `l_tower_p` levels.
- [ ] 2. Spawn: `SpawnPlayActor` → `Login` → `SpawnPlayerController` → `ADishonoredGameInfo::SpawnPlayer` (tweak pawn, AJ's `DoInit`) → `Possess`;
      retail naming with the class `FName` (`PersistentLevel.DishonoredPlayerPawn`, Z follow-up 7; `UWorld::SpawnActor` itself is AI's).
      Log `DISHONORED(bringup): possessed <pawn> in <map>`.
- [ ] 3. Input: `UWindowsClient` / `FWindowsViewport` key/mouse events → `UGameViewportClient::InputKey/InputAxis` → `ULocalPlayer` →
      `UDishonoredPlayerInput` (the four player-input stubs, retail `PlayerInput` tick/axis code in `UnIn.cpp` with the shimmed
      `CurrentTouches`/`aTilt` paths guarded) → `PlayerController.PlayerTick` (retail events only, no reference-only ones) → AE's
      `PlayerMove_Walking` / `ProcessViewRotation`. The player-controller stubs the tick hits (`HandleWalking`, `HandleHeldButtons`,
      `CalcPlayerSwimAccelRate`, input/wheel/item handlers) are ported from 2013. `-inputtest` (`DISHONORED(bringup)`) injects
      W held for 2 s and a mouse-X sweep through `FViewport::InputKey/InputAxis`, then logs
      `DISHONORED(bringup): inputtest moved <dist> turned <yaw>`.
- [ ] 4. UEngine overrides: once AI's virtuals are in, `UDishonoredEngine`'s `PlayLoadMapMovie` / menu / controller functions become real overrides (merge order below).
- [ ] 5. A `--rhi d3d9 -windowed` run with a human at the keyboard once AG/AH land: three `-dumpframes` shots (spawn, after walking, after turning).
- **Accept:** `python resources/tools/build_and_smoke.py --build-dir build/agentAF --no-build --exe-name DishonoredGame_AF.exe --log-name agentAF.log --ini-dir build/agentAF/config --rhi null --timeout 180 --milestone "Committed map change via DishonoredEngine" --expect "Initial startup" --expect "DISHONORED(bringup): possessed" --expect "DISHONORED(bringup): inputtest moved" --skip-native OnlineSubsystemPC --extra-args "-startmap=L_Tower_P -inputtest"`
  exits 0; the moved distance is > 0 and the yaw changed; the pawn and controller classes are `DishonoredPlayerPawn`/`DishonoredPlayerController`; 30 s of ticking in `L_Tower_P` without an assert.
- **Report:** `resources/docs/agents/agentAF.md` (call graph with 2013 rvas, natives ported, remaining stubs the tick hits).

### AG — Renderer A: material shader maps from the retail cooked caches
Owner: agent AG. Build dir `build\agentAG`. D3D9 runs with the windowed switches. Files: `Engine/Inc/MaterialShared.h`, `Engine/Src/MaterialShared.cpp`,
`Engine/Inc/MaterialShader.h`, `Engine/Src/MaterialShader.cpp`, `Engine/Inc/MeshMaterialShader.h`, `Engine/Src/MeshMaterialShader.cpp`,
`Engine/Inc/VertexFactory.h`, `Engine/Src/VertexFactory.cpp` and the vertex-factory units (`LocalVertexFactory`, `GPUSkinVertexFactory`,
particle/decal/terrain factories), `Engine/Src/BasePassRendering.{h,cpp}`, `LightRendering.{h,cpp}`, `DepthRendering.{h,cpp}`,
`HitProxyRendering`, `ShadowDepthRendering` (material shader classes only), `Engine/Src/Material.cpp` (`CacheResourceShaders` path).
Global shader types and `ShaderCache.cpp`/`ShaderManager.*` are AH's; send the edits you need there to AH.
- [ ] 1. Retail gates: maps below 786/23 skipped (2013 0x164a60); material shader types constructed with 798/23 (0xb7ea00 / 0xb7ea40).
- [ ] 2. The common part first: `FMaterialShader` / `FMeshMaterialShader` parameters, `FVertexFactoryParameterRef` / per-factory
      `FVertexFactoryShaderParameters` serializers from the 2013 decompiles, checked against the cooked history word counts
      (`build\agentY\parse_gsc.py` method on `RefShaderCache-PC-D3D-SM3.upk` and the per-package `UShaderCache`s). Then per type
      (49 layout diffs: `TLight*`, `TDepthOnly*`, `TLightMapDensity*`, `FHitProxy*`, `FHitMask*`, `FTextureDensity*`, `TDistortionMesh*`,
      `FLightFunctionPixelShader`, `FTranslucencyPostRenderDepthPixelShader`, …).
- [ ] 3. The 49 undeclared material types: `TBasePassPixelShader<Policy, SkyLight, X>` with Arkane's third template argument (names `…FALSEFALSE` … `…TRUETRUE`),
      `TBasePassVertexShader<Policy>`, `FModShadowMesh*`.
- [ ] 4. `FMaterialShaderMap::Serialize` (2013 0x40ef30) + `FUniformExpressionSet::Serialize` / `FMaterialUniformExpression` classes
      to Arkane's layout; `FMaterialShaderMap::IsComplete` as retail 0x3ea7d0. `UShaderCache` 132 → 128 (AB pending row).
- [ ] 5. Inventory line `DISHONORED(bringup): material shader maps: <N> loaded, <S> skipped, <T> undeclared types, <M> mismatches`.
- **Accept:** `python resources/tools/build_and_smoke.py --build-dir build/agentAG --no-build --exe-name DishonoredGame_AG.exe --log-name agentAG.log --ini-dir build/agentAG/config --rhi d3d9 --timeout 120 --milestone "Initializing Engine..." --expect "Initial startup" --expect "material shader maps:" --skip-native OnlineSubsystemPC --extra-args "-windowed -ResX=1280 -ResY=720 -nomovie"`
  exits 0 with **0 mismatches and 0 undeclared types** for the Startup/Engine material caches; no `Failed to find shader map for default material`; the `-agentYnomatshaders` bypass is not needed; the null-RHI baseline is unchanged.
- **Report:** `resources/docs/agents/agentAG.md` (per type: 2013 `Serialize` rva, history words cooked vs ours, status).

### AH — Renderer B: global shaders and the scene renderer on the retail caches, D3D9 world frame
Owner: agent AH. Build dir `build\agentAH`. Files: `Engine/Src/SceneRendering.{h,cpp}` (remove the null-RHI skip),
`ShadowRendering.{h,cpp}`, `BranchingPCFShadowRendering.{h,cpp}`, `ModShadowRendering` units, `LightShaftRendering.cpp`,
`DepthDependentHalo*`, `ShaderComplexityRendering`, `MLAA`/`FXAA` units, `ScenePostProcessing.cpp`, `UberPostProcessEffect.cpp`,
`SceneRenderTargets.cpp`, `Engine/Src/ShaderCache.cpp`, `ShaderManager.{h,cpp}`, `GlobalShader.{h,cpp}`, `Engine/Src/Scene.cpp`, `Engine/Inc/Scene.h`
(`FSceneView` members), `Engine/Src/RHI.cpp` (`GMobileTiledRenderer` FALSE on PC), `Engine/Src/DynamicRHI.cpp` (the spurious
`-d3d11` warning), `D3D9Drv/Src/*`.
- [ ] 1. `FDownsampleSceneDepthPixelShader` first, then every global type the scene renderer binds for a lit world: the 58 layout-diff
      types (shadow projection 3, `TModShadowProjectionPixelShader` 9, branching PCF 27 + quality types, `FShadowProjectionVertexShader`,
      `FModShadowProjectionVertexShader`, light shafts, `FDepthDependentHaloApplyPixelShader`, `FShaderComplexityApplyPixelShader`,
      `FMLAAVertexShader`, `FFXAAVertexShader`): `Serialize` + parameters + `SetParameters` from the 2013 decompiles.
- [ ] 2. The reference-only passes retail lacks (`renderer.md` §4/§5.5: uber post-process, temporal AA, SSAO, reference FXAA/MLAA,
      exponential/height fog, `FSimpleF32*`, …) are guarded so they are never reached (`DISHONORED(retail)`: retail has no such shader).
      The Arkane FArkPp graph, DisFog and Arkane bloom are **not** ported this wave: their passes are skipped with a named
      `DISHONORED(bringup)` gate, and the 143 types stay undeclared (listed as the wave-5 work).
- [ ] 3. Remove the coordinator's skip in `RenderViewFamily_RenderThread`; `-nullrhi` renders the scene every frame. `FSceneViewFamily(Context)` ctor
      takes `InCurrentBendTime` first (retail); the `bScreenCaptureRenderTarget` shim is read at `SceneRendering.cpp:2862` / `ScenePostProcessing.cpp:59` (AB follow-up).
- [ ] 4. D3D9 world frame with AG's materials (AG snapshot until it merges): `-firstframe` / `-dumpframes` shots of `Dishonored_MainMenu_Env` (and `L_Tower_P` with AF's `-startmap`).
- **Accept:** null: `python resources/tools/build_and_smoke.py --build-dir build/agentAH --no-build --exe-name DishonoredGame_AH.exe --log-name agentAH.log --ini-dir build/agentAH/config --rhi null --timeout 120 --milestone "Initializing Engine..." --expect "Initial startup" --expect "DISHONORED(bringup): scene rendered" --skip-native OnlineSubsystemPC`
  exits 0 with the skip removed and 30 s of frames. d3d9 (joint with AG): same command with `--rhi d3d9 --extra-args "-windowed -ResX=1280 -ResY=720 -nomovie -dumpframes=D:/RecompileDishonored/Recompile/build/agentAH/shots/frame"`
  exits 0, `global shader cache` line shows 0 mismatches for the bound types, and the report shows the world frames (geometry lit, no post-process). Visual differences against retail are listed.
- **Report:** `resources/docs/agents/agentAH.md` (+ `renderer.md` §5 updated: what is done, the remaining FArkPp/DisFog/bloom work).

### AI — Engine convergence wave 2 (AA's leftovers, UEngine/UWorld retail virtuals, Arkane Engine classes out of the DishonoredGame shims)
Owner: agent AI. Build dir `build\agentAI`. Files: `Engine/Src/UnEngine.cpp`, `Engine/Inc/EngineGameEngineClasses.h` (UEngine
virtuals) / `UnEngine.h`, `Engine/Src/UnWorld.cpp`, `Engine/Inc/UnWorld.h`, `Engine/Src/UnLevAct.cpp` (`UWorld::SpawnActor`), `Engine/Src/UnFont.cpp`,
`Engine/Src/UnInterpolation.cpp`, `Engine/Inc/EngineInterpolationClasses.h`, `Engine/Inc/EngineTextureClasses.h`,
`Engine/Src/TextureRenderTarget2D.cpp`, `Engine/Src/arksettings.cpp`, `GameFramework/Src/gamecrowdpopulationmanager.cpp`,
`resources/tools/symbols/gen_classes_header.py` (only AI edits it this wave, beyond the coordinator's pre-wave change),
`DishonoredGame/Inc/DishonoredGameEngineShims.h` (removals), new Engine units for the moved classes, `DishonoredGame/Src/dishonoredaudiosystem.cpp` + its CppText.
- [ ] 1. UEngine retail virtuals in 2013 vtable order (checked against the 2013 `UDishonoredEngine` vtable): `PlayLoadMapMovie(Map, Movie)`
      0x2097d0, `OpenPauseMenu`, `OnControllerDisconnected`, `OpenControllerConnectionMenu`, `OpenContentUnavailableMenu`, `IsLoadingGame`,
      `StopMovie`, `IsDebugMenuVisible` (+392), `RenderDebugMenu` (+388). Engine default bodies from 2013.
- [ ] 2. `UWorld::Init` 0x3945f0: audio system + MapInfo from `[DishonoredMods]`, `m_pWorldInfoCheckStreamingPersistent`; `UAudioSystem`
      virtual interface; `UDishonoredAudioSystem::Init` 0x7a3120 as a silent system (AkAudio stays a stub). `UWorld::SpawnActor` with retail's
      trailing init-callback parameter (tweak spawns, AC follow-up 5) and retail naming.
- [ ] 3. `UFont::GetScalingFactor` → retail (no scale member). `UTextureRenderTarget2D` `m_ResolutionType` @255 + `PostLoad`
      (2012 0x18d760; 2013 via match). `ArkSettings::ApplyCurrentSettings` 0x53b790, `UGameCrowdPopulationManager::ApplyGameSettings` 0x5620b0.
- [ ] 4. `UMatineeData`, the Arkane interp tracks (`FaceTo`/`LookAt`/`Locomotion`/`StretchAnimControl`), `UAkEvent`, `UAkBank`,
      `UArkComponentContainer`, `UUIDynamicFieldProvider` move from `DishonoredGameEngineShims.h` into Engine (generator emits them no
      more; layouts from the SDK). Their `Serialize`/`PostLoad` + `UInterpData::Serialize` (`m_Data` + `m_iMatineeDataVersion`) are ported,
      plus the track priority setters 0x4fc8a0 / 0x4fcc30 / 0x4fcc90.
- [ ] 5. AA's remaining list: `FStaticMeshRenderData` / `FStaticMeshComponentLODInfo` trimmed to retail, `USkeletalMesh::CalculateInvRefMatrices`
      (2013 2590 bytes), the shim-table TODOs on the map path (`ULevelStreaming.LevelTransform` 5 uses in `UnWorld.cpp`,
      `UPostProcessChain.Effects` users outside AH's files guarded).
- **Accept:** CoreSmoke 99/99; `xcheck_sdk_layout.py` 0 rows; `gen_layout_probe.py compare` 0 contract; `verify_phase2.py retail` 2/2;
  the baseline smoke plus `--expect "DISHONORED(bringup): audio system DishonoredAudioSystem"` (logged by the new `UWorld::Init`) exits 0;
  the first-frame text is visible in a d3d9 `-firstframe` dump; ≥ 40 Engine functions moved to `ported`/`written`/`verified` (`agentAI_status.csv`).
- **Report:** `resources/docs/agents/agentAI.md` (+ `serialization_delta_engine.md` rows updated).

### AJ — DishonoredGame infrastructure: tweaks, FSM, inventory, NPC/game-info natives, the missing serializers
Owner: agent AJ. Build dir `build\agentAJ`. Files: DishonoredGame units **not** owned by AF or AI: `dishonoredgameinfo.cpp`,
`distweaksbase.cpp` and the `DisTweaks*` units, `dishonorednativestatemachine.cpp`, `dishonoredpawn.cpp`, NPC pawn/controller units,
inventory/item units, AI behaviour/blackboard units, music manager, `DishonoredGlobalAIManager`, their `Inc/CppText/*.h`,
`DishonoredGameNativeStubs.ported.agentAJ.txt`.
- [ ] 1. Serializers without a CppText hook (agent X's list, 2013 rvas): `ADisMovableLimb` 0x642c30, `ADisWatchTower` 0x6195c0,
      `ADishonoredPawn` 0x748ee0, `UDisAIBlackboard` 0x72ffc0, `UDisDialogTree` 0x88b210, `UDisTweaks_DefenceTower` 0x6208a0,
      `UDisTweaks_SkeletalBreakable` 0x618c30, `UDisTweaks_StaticBreakable` 0x6209a0, `UDisTweaks_UsableObject` 0x64e170,
      `UDishonoredGlobalAIManager` 0x860430, plus AD's hand-overs.
- [ ] 2. Tweak interface: `IDisTweaksInterface` / `IDisEngineTweaksInterface` vtables from 2013; `FSpawnActor_TweakObj::DoInit` called
      before `PostBeginPlay` (with AI's `SpawnActor` parameter); `SetTweaks` on spawned pawns/items.
- [ ] 3. Native FSM: `InitFSM`, `RequestStateChange`, `DemandStateChange`, `DoStateChange`, `TickStateMachine` (2013 rvas); `UDishonoredInventory`
      core (add/equip/query, the natives the player pawn's spawn hits).
- [ ] 4. `ADishonoredGameInfo`: `GameEnding` 0x5ecfe0, `PostBeginPlay` 0x6155d0 → `InitGlobalManagers` 0x5e9bd0, `PreCommitMapChange`
      0x605c50, `PostCommitMapChange`, `Tick` 0x605ab0; the NPC pawn (19) / NPC controller (17) / pawn (17) stubs `L_Tower_P` hits. The save
      system (DisSaveLoad, `FGameState`) is **out of scope** (milestone 6); `m_pGameState` stays NULL with `DISHONORED(bringup)`.
- **Accept:** ≥ 120 more DishonoredGame natives `written` (`agentAJ_status.csv`); AD's `-loadall` over the `L_Tower_P` packages
  reports 0 DishonoredGame serializer errors; on AF's snapshot the `L_Tower_P` run has no warn-once line from AJ classes before
  `DISHONORED(bringup): possessed`; the module builds with `DISHONORED_SDK_LAYOUT_CHECKS=ON`.
- **Report:** `resources/docs/agents/agentAJ.md` (port table as AC's, remaining stubs per class that the map hits).

### AK — Build hygiene, debug tooling, Edge in-engine check and whole-tree decision memo
Owner: agent AK. Build dir `build\agentAK`. Files: `cmake/*` (after the coordinator's pre-wave change), root `CMakeLists.txt`,
`resources/tools/build_and_smoke.py`, `resources/tools/stage_retail.py`, `resources/tools/ida/decompile_funcs.py`,
`resources/tools/symbols/gen_layout_probe.py` (the `compare` side effect only), new `resources/tools/debug/` (promote W's
`stack_sample.py`, X's `dbg`/`dbgrun`, Z's `zdbg.py`), new `resources/tools/make_snapshot.py`, `resources/docs/edgeanim.md` §7, `resources/docs/toolchain.md`.
- [ ] 1. Verify the per-build dependency dirs under load (3 parallel builds); `make_snapshot.py <X> [files…]`: `git worktree add --detach build/agent<X>_wt HEAD`
      + overlay, **refusing** to run if the target contains a junction or a stage dir; removal via `os.rmdir` of links first (`unlink_junctions.py`), never a recursive delete through links.
- [ ] 2. `build_and_smoke.py`: drop `-seekfreeloadingpcconsole` (retail default since X); `--expect-count LINE=N` (the second
      `Committed map change`); `--forbid LINE` (fail when `Bad export index` / `Serial size mismatch` / `native not ported` appear).
- [ ] 3. `decompile_funcs.py` suffixes the rva to output names (overloads no longer overwrite each other, AA follow-up 5);
      `gen_layout_probe.py compare` writes `reference_layout_delta.md` only with `--write` (AA follow-up 6).
- [ ] 4. Debug tools under `resources/tools/debug/` with a README: appErrorf/AV stop, EBP-chain stacks symbolized through the `.map`, all-thread hang dump.
- [ ] 5. Edge: in-engine check once an NPC is in a loaded map (AD/AF snapshot): default vs `-edgerefpose` on the same NPC,
      pose finite, no W flip. Memo `edgeanim.md` §7: whole-tree `BuildEdgeAnimTree` path (+8–12 days) vs keeping Plan B, with the
      evidence (additive order, locomotion extraction, blend differences) and a recommendation for wave 5.
- [ ] 6. Optional: root set 63,718 vs golden 51,634 (agent X) explained; `jmarshall` leftovers (`UnMath.h`, `SplashScreen.cpp`, `UnAnimPlay.cpp`) listed.
- **Accept:** three concurrent `cmake --build` in `build\agentAK`, `build\agentAK2`, `build\agentAK3` succeed with no C1083; the baseline smoke passes with the new `build_and_smoke.py` (no seek-free switch, `--forbid "Bad export index"` fails as expected on the baseline); `make_snapshot.py` refuses a junction test dir; edgeanim.md §7 written.
- **Report:** `resources/docs/agents/agentAK.md`.

## Coordination and merge

- **File ownership (never edited by two packages):** `UnGame.cpp`, `UnPlayer.cpp`, `UnIn.cpp`, `WinDrv/*` → AF; `UnEngine.cpp`,
  `UnWorld.cpp`, `UnLevAct.cpp`, `UnFont.cpp`, `UnInterpolation.cpp`, `gen_classes_header.py` → AI; `UnLevel.cpp`, `UnPath.h`,
  `UnNavigationMesh.cpp`, `LaunchEngineLoop.cpp`, `Core` async IO → AD; `UnCamera.cpp`, `UnController.cpp`, `UnPawn.cpp`, `UnPhysic.cpp`,
  the natives in `UnActor.cpp` → AE; `SceneRendering.*`, `ShaderCache.cpp`, `ShaderManager.*`, `GlobalShader.*`, `Scene.h/.cpp`,
  `D3D9Drv` → AH; `MaterialShared.*`, `MaterialShader.*`, `MeshMaterialShader.*`, `VertexFactory.*`, `BasePassRendering.*`,
  `LightRendering.*`, `DepthRendering.*` → AG; `cmake/*`, `CMakeLists.txt`, `build_and_smoke.py` → AK. DishonoredGame units are split
  between AF (engine/viewport/player controller/input/pawn/camera/HUD/main-menu) and AJ (everything else the map hits); AI owns
  `dishonoredaudiosystem.cpp` and the shim removals.
- **Shared headers:** `EngineClasses.h`, `EngineControllerClasses.h`, `EnginePawnClasses.h`, `EngineCameraClasses.h` → AE edits only
  `DECLARE_FUNCTION`/natives-table lines (+ `IArkHealthInterface` bodies). `EngineGameEngineClasses.h` / `EngineInterpolationClasses.h` /
  `EngineTextureClasses.h` → AI. Any other edit a package needs outside its list goes into its report as a hand-over.
- **Generated DishonoredGame files** (`DishonoredGameNativeStubs.cpp`, class headers, `Sources.cmake`): regenerated by agents
  in their snapshots only; the coordinator regenerates the shared tree at each merge from the per-agent `ported.agent<X>.txt` lists.
- **Snapshots:** when others' in-flight edits break your build, `make_snapshot.py` (or `git worktree add --detach build\agent<X>_wt HEAD`)
  plus your files. A snapshot never contains a stage directory or a junction.
- **Merge order:** AK (tooling) → AD (blocker) → AI (UEngine virtuals, UWorld::Init, SpawnActor) → AE (natives) → AJ (DishonoredGame
  infrastructure) → AF (milestone 5, on top of all four) → AG → AH (joint D3D9 world-frame check after both). After each merge, on a clean
  `git worktree add --detach build/head_wt HEAD` build with layout checks on: CoreSmoke, `xcheck_sdk_layout.py`, `gen_layout_probe.py compare`,
  `verify_phase2.py retail`, the baseline smoke, then the merged package's accept command. One commit per agent; the AkAudio/DishonoredGame/DisJobs
  index-case fix (agent AB's `git rm -r --cached` + `git add` of the three directories) goes in AK's commit.
- **End of wave:** this tracker, `STATUS.md`, `PLAN.md` Phase 6 milestone 5 (and milestone 4's world-frame half), `progress.md`, `renderer.md`, memory.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| C7 | coordinator | Per-build dependency dirs, per-agent ported/status files, baseline in STATUS/PLAN, this plan | done | 2026-09-26 | see the checklist above; Debug crash filter + linker index diagnostics added |
| AD | AD | Streaming serialization: nav mesh (0x2909e0), ULevel gates, async IO, `-loadall` sweep | done | 2026-09-26 | commit 8af9425: the blocker was the reference poly serializer reading a `TArray<FCoverReference>` retail lacks at licensee 30; class 688 -> retail 464; 5 serializers ported; `-loadall` 3 menu + 9 tower packages, 0 errors. `ULevel::Serialize` and `FAsyncIORequest` checked and left alone |
| AE | AE | Engine/GameFramework retail natives without a body (Z's list), movement/camera first | done | 2026-09-26 | commit f8bad4f: 109 natives given retail bodies (145 missing -> 29 real gaps); no `Engine/GameFramework native not ported` line left on the map path; 22 UI data-store natives left (off-path). Runs need `-forcelogflush` |
| AF | AF | Milestone 5: `L_Tower_P` streamed in, player possessed, input moves the pawn | done (milestone not claimed) | 2026-09-26 | commit f184f60: `LoadMap: L_Tower_P`, a `DishonoredPlayerPawn` possessed by a `DishonoredPlayerController` at the PlayerStart, 172 retail bindings, state `PlayerWalking`, W producing real movement acceleration and the mouse turning the view 13,328 yaw units — but the pawn does not move (spawns in a world with no collision, falls to the kill plane, freezes at `PHYS_None`), so AF rightly did not claim the milestone. Also fixed a wave regression: `UDishonoredEngine::PlayLoadMapMovie` recursed into itself. Found the retail New Game route (`m_NewGameCommand` = `ce ChangeLvl_StartNewGame`, `OnNewGameConfirm` 0x7c1ff0). Six switches, all off by default |
| AG | AG | Renderer A: material shader maps from the retail caches (786/798, uniform expressions) | done | 2026-09-26 | commit ad3f9ae: 0 maps -> **2580 loaded, 0 undeclared, 0 mismatches**, 142,902 shader references all consuming their exact cooked ranges; `IsComplete` was the culprit (retail 0x3ea7d0: FALSE only while compiling); retail registers only 17 vertex factory types |
| AH | AH | Renderer B: global shaders + scene renderer, null-RHI skip removed, D3D9 world frame | done | 2026-09-26 | commit f8dfe78: 62 -> **127 records, 58 -> 0 mismatches**, null-RHI skip gone, scene renderer runs every frame (>210 frames at 1920x1080); two shared parameter structs explained most of the 58; D3D9 world frame re-measured at merge |
| AI | AI | Engine convergence wave 2: UEngine virtuals, UWorld::Init audio/MapInfo, UFont, matinee classes into Engine | done | 2026-09-26 | commit 03f5335: UEngine virtuals in 2013 vtable order, `UWorld::Init` audio system + MapInfo (0x3945f0), `SpawnActor` init functor (0x256990), `UFont::GetScalingFactor` (text was invisible), 19 Arkane classes + 6 structs moved into Engine, 44 functions |
| AJ | AJ | DishonoredGame infrastructure: tweaks, FSM, inventory, game-info/NPC natives, 10 serializers | done | 2026-09-26 | commit 6e9d1ad: all 10 serializers, the full tweak interface + fallback chain through AI's init functor, native FSM (18), inventory core (12), 74 functions; module regenerated (119 -> 100 shim classes, 0 pending). 7 of 120 natives: the 275 left need the AI brain / item context first (triage in the report) |
| AK | AK | Build hygiene, debug tools, Edge in-engine check + whole-tree memo | done | 2026-09-26 | commit 571bd0e: 3 concurrent full builds with no C1083, `--forbid`/`--expect-count`, `resources/tools/debug/` (its stack gave AD the blocker chain), `make_snapshot.py`, edgeanim.md section 7 (keep the per-sequence evaluator for wave 5; in-engine check still pending) |

## Wave result (coordinator, 2026-09-26)

All eight packages are merged: AK `571bd0e`, AD `8af9425`, AI `03f5335`, AE `f8bad4f`, AG `ad3f9ae`,
AH `f8dfe78`, AJ `6e9d1ad` (with the module regeneration AI's class move needed, and a coordinator bridge
guarding the two reference-only `Actor.PostInitAnimTree` / `Actor.AnimTreeUpdated` events), AF `f184f60`,
and two more coordinator bridges from AF's snapshot-only repairs: the zero-length matinee guard in
`USeqAct_Interp::StepInterp` and a `-binnedmalloc` switch for `_DEBUG` builds.

**The wave's headline: the game boots, loads, renders and changes map with no crash anywhere.** On the merged
clean-worktree build (`build/head_wt`, layout checks on, all module options, 785 units, 0 errors):

| Check | Wave 3 | Wave 4 (7 packages) |
|---|---|---|
| null-RHI run | died 0.5 s after `Initial startup` on `Bad export index` | `Initial startup: 7.73s`, **`Committed map change via DishonoredEngine`**, no `Critical` line at all |
| scene rendering | skipped entirely under the null RHI | **runs every frame**, 180+ frames logged at 1920x1080 |
| cooked global shaders | 62 records, 58 parameter mismatches | **127 records, 0 mismatches** |
| cooked material shader maps | 0 loaded, d3d9 aborted on the default material | **2580 loaded, 0 undeclared, 0 mismatches** |
| natives without a body on the path | ~140 | **4**: 3 Steamworks `Read*` (need the Steam SDK) and `ADishonoredPlayerPawn::execPlayDying_Native` (AF) |
| CoreSmoke / xcheck / verify_phase2 | 99/99 / 0 rows / 2/2 | 99/99 / **0 rows** (2,314 types) / **2/2** |

`function_status.csv` folded the six per-agent files: 1,200 rows (+307), Engine per-function 0.8 % -> 1.2 %.

**The matinee guard moved the end of the run, and exposed the next blocker.** Before it, the world tick
spun forever in `StepInterp` right after the map change; with it, on the merged tree (`build/game`, all
eight packages):

| Run | Result |
|---|---|
| null RHI, `-noscenerender` | **no critical error at all**: `Initial startup: 6.62s`, `Committed map change via DishonoredEngine` at 7.40 s, and the world keeps ticking to 33.7 s (the run is killed by the harness timeout, not by a fault) |
| null RHI, scene rendering on (default) | the renderer now draws real geometry and asserts at ~7 s on a mesh batch whose index range exceeds its index buffer (`DynamicIndexBuffer != NULL \|\| ... BatchElement.FirstIndex + ...`). This is the **next blocker**, newly exposed rather than newly caused: before the guard the tick never got this far |

Known follow-ups recorded for wave 5: Arkane anim nodes (the tweak anim tree asserts on a state picker with
zero child weights, so the assignment is gated behind `-distweakanimtree`), the 275 DishonoredGame stubs that
need the AI brain / sub-process / item-context classes (triage in `agentAJ.md`), the Arkane and GFx
post-process shader families (136 undeclared types), `UShaderCache` 132 -> 128 with
`FCompressedShaderCodeCache`, a Release or `FMallocBinned` build for long d3d9 runs (`_DEBUG` uses
`FMallocDebug` and the now-resident caches exhaust the 32-bit heap ~2 s in), the ~100 remaining
Engine/GameFramework shim classes, and the whole-tree Edge path. Added by AF's five documented blockers: the
mesh-batch index-range assert above; the sub-level association hang in `FSkeletalMeshObject` cleanup
(identical stacks under `FMallocDebug`, `FMallocBinned` and `-onethread`, so almost certainly one heap
corruption, and it is what stands between here and a walking pawn); world collision, without which the pawn
falls to the kill plane; `APlayerController::Possess`/`UnPossess` still being `import_reference.py` stubs; the
reference height-fog parameter assert on the first world frame; and the player's `UPlayerInput` never entering
`GlobalInteractions`. `UInterpData::InterpLength` should be set when `UMatineeData::m_Data` is ported, which
retires the guard.

## Rules for agents (wave 2/3 rules, repeated)

- Edit only the files your package names, in the shared working tree. When others' edits break your build, snapshot into
  `build\agent<X>_wt` (HEAD + your files). A snapshot never contains a stage directory or a junction.
- Own build dir `build\agent<X>`; own IDA copies `resources\docs\idb\shipping2012_agent<X>.i64` / `retail2013_agent<X>.i64` (copied from
  `shipping2012_v1.i64` / `retail2013_named.i64`); headless only: `python resources\tools\ida\run.py resources\tools\ida\decompile_funcs.py <db> <out> <name|rva:0x…>`;
  **never the IDA MCP tools**, never another agent's copy.
- Run the exe only through `build_and_smoke.py --exe-name DishonoredGame_<X>.exe --log-name agent<X>.log --ini-dir build\agent<X>\config`,
  or by hand with the same `-LOG=`/`-ENGINEINI=`/`-GAMEINI=`/`-INPUTINI=`/`-UIINI=` switches; never touch another agent's exe/log/ini.
- **Never create junctions or symlinks into the retail or reference trees, and never recursively delete a directory that may contain
  one** (`rm -rf`, `Remove-Item -Recurse`, `git worktree remove`). Links go with `resources\tools\unlink_junctions.py` only. Never
  delete anything under `Dishonored_Latest2026`.
- No commits, no `git add` (never `git add -A`). Report to `resources\docs\agents\agent<X>.md`: what changed, evidence per change,
  exact commands and which build each number comes from, what is left, follow-ups outside your files. Status rows go in `agent<X>_status.csv`.
- Every edit is tagged `// DISHONORED(port|written|layout|retail|bringup): <evidence>`. The retail 2013 exe is the target: cite the 2013 rva, and use the 2012
  decompile for readability only.
- Bash heredocs mangle backslashes, so write patch scripts with the Write tool. Sources are CRLF.
- **Never use the FModel MCP tools (`mcp__fmodel__*`).** FModel is UE4-only and does not read Dishonored's UE3 packages; the user
  stopped an agent for trying (2026-09-26). Package questions go through our tools (`resources/tools/**`, `read_package_classes.py`),
  the CodeRed SDK dump and the retail IDA db.
