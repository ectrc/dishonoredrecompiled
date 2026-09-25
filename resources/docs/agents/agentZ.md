# Agent Z — Map load: `DishonoredGameFull_P` up for play (no PhysX) (2026-09-25)

Package Z of `resources/docs/PHASE5.md`, steps 1–3. **Every number below comes from the build `build\agentZ`**
(Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`, the four module options ON) built from the snapshot worktree
`build\agentZ_wt` (see "How the runs were made"). "2013 rva" = retail `Dishonored_Latest2026` exe (named db, private copy
`resources/docs/idb/retail2013_agentZ.i64`), "2012 rva" = 2012 Shipping build (evidence only, `shipping2012_agentZ.i64`).
Decompiles: `resources/reference/decomp/agentZ/{2012,2013,2012_ser,2013_ser,2012nav}` (git-ignored).

## Result

With agent Z's snapshot (HEAD + the shared working tree at 21:40 + agent Z's files + the local-only stand-ins listed below)
and `-nullrhi`, the golden sequence runs up to the main-menu map change:

| Golden milestone | Reached | Our line (`DishonoredGame\Logs\agentZ.log`) |
|---|---|---|
| `LoadMap: DishonoredGameFull_P?Name=Corvo?Team=255` (golden :87) | yes | identical |
| `Game class is 'DishonoredGameInfo'` (:90) | **yes** | identical |
| `Bringing World DishonoredGameFull_P.TheWorld up for play` (:93) | **yes** | identical (no PhysX lines: `WITH_NOVODEX=0`, `RBPhysScene == NULL`, no guard needed) |
| `Bringing up level for play took` (:97) | **yes** | `0.004930` |
| `########### Finished loading level` (:129) | **yes** | `0.299536 seconds` |
| `DevDlc: Looking for DLC...` (:130) | yes | DLC05/06/07 found and merged |
| `SaveGameList done` (:183) | **no** | not logged; the `FDisAsyncSaveGameLister` completion line belongs to `UDishonoredEngine` (agent AC) |
| `Initializing Engine Completed` (:355) | **yes** | identical |
| `Initial startup` (:356) | **yes** | `5.20s` (golden 4.21 s) |
| first `Committed map change via DishonoredEngine` (:365) | **no** | `UGameEngine::CommitMapChange` runs (PreCommitMapChange, `CleanUpBeforeLevelTransition`, actor destruction) and streams `Dishonored_MainMenu_Env`; it dies in that package's nav mesh |

Accept command (exit 0):

```
python resources\tools\build_and_smoke.py --build-dir build\agentZ --no-build --exe-name DishonoredGame_Z.exe ^
  --log-name agentZ.log --ini-dir build\agentZ\config --rhi null ^
  --milestone "Bringing World DishonoredGameFull_P.TheWorld up for play" "--extra-args=-allowunboundnatives -forcelogflush"
```

The same command with `--milestone "Finished loading level"`, `"Initializing Engine Completed"` and `"Initial startup"`
also exits 0 (`build\agentZ\last_smoke.txt`). `-forcelogflush` only keeps the log intact through the crash that follows.

**Exact next blocker: `UNavigationMeshBase::Serialize`, 2013 rva 0x2909e0** (2012 rva 0x2b0000, 1915 bytes). While
`CommitMapChange` streams in the main menu, `Dishonored_MainMenu_Env.upk` →
`TheWorld:PersistentLevel.DisPylon_0.NavigationMeshBase_4469` misreads (`Bad export index 1065353215/6389`, file offset
7786610). Retail format: `NavMeshVersionNum`; `VersionAtGenerationTime` (>= 11, raises `FPathBuilder::LoadedPathVersionNum`);
`Verts`, `EdgeStorageData`, `Polys` (`operator<<(FArchive&, FNavMeshPolyBase&)` 2013 0x2780f0, 2012 0x2928c0, licensee-27
branch); a dummy object ref below 7; `LocalToWorld`/`WorldToLocal` (>= 8); `BorderEdgeSegments` (>= 9, only when the
outer `APylon` bit @288 is clear); `ConstructLoadedEdges`; `BuildBounds` below 12. Edges: `FNavMeshEdgeBase::Serialize`
2013 0x279d40, `SerializeEdgeVerts` 0x279eb0. Our `UNavigationMeshBase` is 688 bytes vs retail 464 (agent AB's size table),
so this is a navmesh layout convergence plus the serializer port.

## Step 1 — call graph (2013 rvas; 2012 decompiles are the readable copies)

```
UGameEngine::Init 0x2357c0 (X)                       ── Browse(DefaultURL)
 UGameEngine::Browse 0x2375d0 (2012 0x248650, same size)
  UDishonoredEngine::LoadMap 0x615210 (2012 0x64cf10)   CancelAllPendingAutosaves, deny pending mission-start save,
  │                                                    reset save/autosave refcounts, ActorVisibilityHistory.SetStates(0),
  │                                                    UAkAudioDevice::StopAllSounds, Super::LoadMap, PostCommitMapChange
  └ UGameEngine::LoadMap 0x240b40 (2012 0x250ff0, 4846 vs 5169 bytes)
     ├ CancelPendingMapChange / FSeamlessTravelHandler::CancelTravel, PreLoadMap callback
     ├ UEngine::PlayLoadMapMovie → UDishonoredEngine::PlayLoadMapMovie 0x601b80 (2012 0x648310)
     │    FindMapConfig 0x5fbdc0 (m_MapConfig, DefaultEngine.ini:147), rich-presence chapter,
     │    UDisBinkOverlayManager::OnPlaySaveNotificationMovie / OnPlayLoadingMovie, UEngine::PlayLoadMapMovie 0x2097d0
     ├ old world: FlushLevelStreaming 0x39cc10, TermWorldRBPhys, CleanupWorld, UAkAudioDevice::Flush, CollectGarbage
     ├ LoadPackage(map) → UWorld::Serialize 0x390b90 → ULevel::Serialize 0x259f30 (ported, below)
     ├ UWorld::Init 0x3945f0 → UWorld::UpdateWorldInfoCache 0x38cac0 (ported)
     ├ UWorld::SetGameInfo 0x38bfa0 (ported): GAME= option, AGameInfo.DefaultMapPrefixes, eventSetGameType,
     │    LoadPackagesFully 0x22a0a0 (PreLoadClass/PostLoadClass/LoadForAllGameTypes), SpawnActor(DishonoredGameInfo)
     │    → ADishonoredGameInfo::ADishonoredGameInfo 0x614660
     ├ UWorld::BeginPlay 0x394f10 (2012 0x3b37a0): UpdateComponents 0x394cf0, InitWorldRBPhys 0x40baf0 (NULL scene),
     │    InitLevelBSPPhysMesh, IncrementalInitActorsRBPhys, InitializeActors, AGameInfo::eventInitGame 0x38bc40
     │    (script DishonoredGameInfo.InitGame), ULevel::RouteBeginPlay 0x24eb30 → ADishonoredGameInfo::PostBeginPlay
     │    0x6155d0 → InitGlobalManagers 0x5e9bd0; GameSequence BeginPlay; RoutePostSequenceInit
     ├ ULocalPlayer::SpawnPlayActor → UWorld::SpawnPlayActor → AGameInfo::eventLogin (script)
     │    → GameInfo.SpawnPlayerController   NATIVE in retail: exec 0x1c60c0, body 0x2d13a0
     │    → Controller.Possess                NATIVE in retail: body 0x1cb120
     ├ FURL::SaveURLConfig, RedrawViewports, GStreamingManager, "Finished loading level", PostLoadMap callback
     └ UDishonoredEngine::PostCommitMapChange (2012 0x65d6b0, 2013 unmatched)
FEngineLoop::Init tail 0x5e11b0 → "Initializing Engine Completed", "Initial startup" (no GameInfo.OnEngineHasLoaded in retail)
UGameEngine::Tick 0x232860 → ConditionalCommitMapChange 0x232770 → CommitMapChange 0x22d570 (2012 0x242900)
     ├ UDishonoredEngine::PreCommitMapChange 0x616150, ADishonoredGameInfo::PreCommitMapChange 0x605c50
     ├ UWorld::CleanUpBeforeLevelTransition → DestroyActor → UIDataStore unregistration (ported, below)
     └ level streaming: UWorld::UpdateLevelStreaming 0x39c3d0, AddToWorld 0x39b900 → Dishonored_MainMenu_Env
         → UNavigationMeshBase::Serialize 0x2909e0  ← next blocker
AWorldInfo.PrepareMapChange: exec 0x1f9de0 → AWorldInfo::PrepareMapChange 0x3845f0 → UGameEngine::PrepareMapChange
     (2012 0x23fa60, 2013 unmatched); CommitMapChange: exec 0x1c6d80 → AWorldInfo::CommitMapChange 0x384680
```

DishonoredGame natives on this graph (status in the main tree at 22:20): `ADishonoredGameInfo` — `execSpawnPlayer` 0x5ed060,
`execCanStartMatch` 0x5ed020, `execPreventDeath_Native` 0x5ed140, `execGetChangelist` 0xcf20, `execGetDishonoredEngineVersion`
ported by AC; **`execGameEnding` 0x5ecfe0 still a stub**. `UDishonoredEngine` — all 15 natives (`execPlayLoadMapMovie` 0x5eee70
on UEngine, `execPushDisableSave` 0x5ecdd0, `execPopDisableSave` 0x5ece30, `execPushIgnoreAutosave` 0x5f6f70,
`execPopIgnoreAutosave` 0x5ecec0, `execDis_Save` 0x5ecd10, `execDis_Load` 0x5ecd70, `execOnControllerChanged` 0x5ecf20,
`execPublishRichPresence` 0x5ecf90, `execUpdateRichPresenceChapter` 0x5fb1b0, `execUpdateRichPresenceChaos` 0x5fb230, ...)
ported by AC; `UDishonoredViewportClient::execPostRender_Native` 0x5ed500 ported by AC. **No DishonoredGame or GFxUI
warn-once stub fired during the run above**; the stub lines that fired are OSS (`execReadFriendsList`, `execReadProfileSettings`,
`execReadAchievements`: agent X) and the Engine-level gaps below. C++ virtuals of the graph that are still AC's: the
`UDishonoredEngine::LoadMap` override 0x615210 and `PostCommitMapChange`, `ADishonoredGameInfo::PostBeginPlay` 0x6155d0 /
`InitGlobalManagers` 0x5e9bd0 / `PreCommitMapChange` 0x605c50 / `PostCommitMapChange` / `Tick` 0x605ab0, `FindMapConfig`
0x5fbdc0, `RefreshSaveGameList` 0x609870 (the `SaveGameList done` line).

`m_MapConfig` is a `UDishonoredEngine` config array (`[DishonoredGame.DishonoredEngine]`), not an `AWorldInfo` member; the
`ImportText (m_MapConfig)` errors are retail behaviour (golden :71-75, `m_bShowMapNameOnlyOnXboxNoHDD` is not in the struct).

### Engine-level gap found on the way: 147 retail natives without an exec body

The retail script packages declare 147 Core/Engine/GameFramework functions `native` that the reference Engine implements in
script (or not at all). `UFunction::Bind` finds no exec for them and leaves `Func == NULL`; the first call jumps to address 0
(that was the `eventLogin` crash). This needs `DECLARE_FUNCTION` + `MAP_NATIVE` entries in the Engine/GameFramework headers
and bodies ported from the 2013 exe — one owner (AA/AB or the coordinator), not per-agent. Hit on the map path so far:
`GameInfo:SpawnPlayerController`, `Controller:Possess`, `Camera:UpdateCamera`, `Controller/PlayerController:GetPlayerViewPoint`,
`Pawn:Died`, `Camera:ClearCameraLensEffects`, `InterpActor:SetShadowParentOnAllAttachedComponents`,
`DownloadableContentManager:BackupDLCList/RemoveUnavailableDLC/UninstallDLCs`. Full list (2013 exec rva from
`match_2012_2013.csv` where matched, else the 2012 rva; unverified matches), generated by a local diagnostic in `UFunction::Bind`:

- `Core.Commandlet`: Main (0x34270); `Core.HelpCommandlet`: Main; `Core.Object`: RSmerp (0xad80), VSmerp (0x9af0)
- `Engine.Actor`: ActivateOcclusion (0x1c12e0), CheckHitInfo (0x1c1800), DoKismetAttachment (0x1df6c0), FindEventsOfClass (0x1fcc40), PlayActorFaceFXAnim (0x1e76c0), PostAkEvent (0x1c1050), PostTrigger (0x1c1270), SetRTPCValue (0x1c10b0), SetState (0x1c1150), SetSwitch (0x1c11e0), StopActorFaceFXAnim, TakeDamage (0x1c1640), VolumeBasedDestroy (0x1c03a0)
- `Engine.AnimNodeSlot`: AddToSynchGroup (0x1dc970); `Engine.AutoTestManager`: DoSentinelActionBeforeExit_Native, DoSentinelActionPerLoadedLevel_Native
- `Engine.Camera`: AddCameraLensEffect (2012 0x1e4a80), ClearCameraLensEffects (0x1d03c0), FindCameraLensEffect (2012 0x1e49f0), GetCameraViewPoint (0x1cfec0), GetFOVAngle (0x1cfe60), RemoveCameraLensEffect (2012 0x632920), UpdateCamera (0x1cffd0)
- `Engine.CheatManager`: SetTargetedActor (0x1cc2e0), ShowActor (2012 0x20b390)
- `Engine.Controller`: GetPlayerViewPoint (0x1d1430), Possess (body 0x1cb120), UnPossess (2012 0x63c7e0)
- `Engine.DecalManager`: CanSpawnDecals, GetPooledComponent (0x1d4410), OnDecalFinished (2012 0x1d94f0), SetDecalParameters (0x1d40f0), SpawnDecal (0x1d4450)
- `Engine.DownloadableContentEnumerator`: CleanLaunchedDLC, UninstallDLC; `Engine.DownloadableContentManager`: BackupDLCList, RemoveUnavailableDLC, UninstallDLC, UninstallDLCs
- `Engine.EmitterCameraLensEffectBase`: ActivateLensEffect (2012 0x1cc9c0), NotifyRetriggered, RegisterCamera
- `Engine.Engine`: OnControllerDisconnected, OpenContentUnavailableMenu, OpenControllerConnectionMenu (0x1bee40), OpenPauseMenu, WaitMovie (2012 0x1f82a0); `Engine.GameEngine`: GetDLCManagementBridge (2012 0x22bc60)
- `Engine.GameInfo`: ReduceDamage (0x1c6170), SpawnPlayerController (0x1c60c0)
- `Engine.HUD`: DisplayConsoleMessages (0x557ba0?), ShouldDisplayDebug (0x1c3c50), ShowDebug (0x1c3bd0)
- `Engine.InterpActor`: SetShadowParentOnAllAttachedComponents; `Engine.KActor`: ApplyImpulse (0x1cfb20), TakeDamage; `Engine.KAsset`: TakeDamage; `Engine.LocalPlayer`: ZeroOverridePPDeltaSettings
- `Engine.OnlinePlayerStorage`: Get/SetRangedProfileSettingValueFloat/Int (2012 0x1e10c0, 0x1e1210; 2013 0x1cab50; 2012 0x1e0f30)
- `Engine.Pawn`: AddVelocity (0x1dade0), Died (0x1dafd0), FaceRotation (0x1daf30), GetNavigationHandle (2012 0x1f1090), HandleMomentum (0x1dac90), InitNavigationHandle (2012 0x638ce0), IsRagdoll (2012 0x1f1150), IsValidTargetFor, PlayHit (0x1db0a0), TakeDamage, UnPossessed (0x1daac0)
- `Engine.PlayerController`: CleanOutSavedMoves (0x1d3310), CleanUpBeforeLevelTransition (0x1d3af0), GetFOVAngle (0x1d3670), GetPlayerViewPoint, HandleWalking (0x1d35c0), IsLookInputIgnored (2012 0x1e6f00), IsMoveInputIgnored (2012 0x1e6e50), LimitViewRotation (0x1d3930), PlayerMove_Walking (0x1d3a00), ProcessViewRotation (0x1d3840), ResetTimeMargin (0x1d32e0), Sentinel_TakeScreenshot (0x1ee6a0), Sentinel_TakeScreenshotEnabled (2012 0x1e72f0), SetControllerTiltDesiredIfAvailable (2012 0x1e63d0), UpdateRotation (0x1d37e0)
- `Engine.PrimitiveComponent`: PutRigidBodyToSleep_Debug (0x3a27e0), RigidBodyIsAwake_Debug (0x3a3460), SetRBPosition_Debug (0x3a31a0), SetRBRotation_Debug (0x3a32a0), ShouldComponentAddToPrimitiveOctree (2012 0x12d4d0)
- `Engine.SeqEvent_TakeDamage`: HandleDamage (0x1d9e60), IsValidDamageType (0x1d9e00); `Engine.Settings`: Get/SetSettingsDataString (0x1e8e40, 0x1e8d30)
- `Engine.SkeletalMeshActor`: PostBeginPlaySkeletalMeshIsHidden_Native, TakeDamage; `Engine.SkeletalMeshActorMAT`: ClearAnimNodes (0x1db540), UpdateAnimSetList; `Engine.SkeletalMeshComponent`: GetBoneMatrixLocal (0x318810), PlayParticleEffect (0x348d50)
- `Engine.UIDataProvider`: GetProviderFieldType (0x1e9f10), ParseArrayDelimiter (0x1ea010); `UIDataProvider_MenuItem`: IsFiltered (2012 0x1dc8b0); `UIDataProvider_OnlinePlayerStorage`: OnReadStorageComplete_Native (2012 0x636200); `UIDataStore`: OnCommit
- `Engine.UIDataStore_DynamicResource` / `UIDataStore_GameResource`: FindProviderIndexByFieldValue (0x1f4dc0 / 0x1f5050), GenerateProviderAccessTag (0x1c70b0 / 0x1c7280), GetProviderCount (— / 2012 0x1db0e0), GetProviderFieldValue (0x1f4c60 / 0x1f4ef0), GetResourceProviderFields (2012 0x2106f0 / 0x210910)
- `Engine.UIDataStore_MenuItems`: AppendToSet (0x1c7410), ClearSet (2012 0x1db370), GetSet (2012 0x210a20); `UIDataStore_OnlinePlayerData`: OnSettingProviderChanged (0x1c74b0); `UIDynamicDataProvider`: Bind/UnbindProviderInstance (0x1c8490, 0x1caca0); `UIRoot`: Get/SetDataStoreFieldValue (2012 0x2103c0, 0x210280); `Engine.WorldInfo`: GetGlobalGravityZ (0x1c69a0)
- `GameFramework.GameCrowdAgent`: FellOutOfWorld, InitializeAgent (0x5577a0?), OnDestroyedByKismet (2012 0x638c60), OutsideWorldBounds, TakeDamage, VolumeBasedDestroy; `GameCrowdAgentSkeletal`: OnAnimEnd; `GameCrowdDestination`: AllowableDestinationFor, Decrement/IncrementCustomerCount, PickNewDestinationFor, ReachedDestination; `GameCrowdDestinationQueuePoint`: ActuallyAdvance; `GameCrowdInteractionPoint`: SetEnabled; `GamePlayerController`: CrowdFocus, CrowdToggle (0x1d20b0)

Reference-only script events our C++ still raises (the retail packages have no such function; `FindFunctionChecked`
aborts): `PlayerController.PreRender` (from `UGameViewportClient::Draw`), `PlayerController.PlayerTick`,
`PlayerController.OnEngineInitialTick`, `PlayerController.ServerUpdateLevelVisibility`, `GameInfo.OnEngineHasLoaded`
(`FEngineLoop::Init` tail; retail 0x5e11b0 has none), `GameInfo.PreCommitMapChange`, `AnalyticEventsBase.Init`,
`CloudStorageBase.Init` (the last two X removed at 2013 0x2357c0).

## What was ported (shared tree; all compiled and exercised in the `build\agentZ` runs above)

| Function | File | Evidence |
|---|---|---|
| `ULevel::Serialize` | `Engine/Src/UnLevel.cpp` | 2013 0x259f30 = 2012 0x2737b0 (1068 bytes both): no cover lists (licensee < 27 only), no 798 cover GUID refs (that read broke `DishonoredGameFull_P`: "Bad export index 9727/1597"), Arkane `m_CrossLevelReferencedActors` (>= 780), APEX blob skipped, visibility thresholds 734/739/757/799, no volume distance field |
| `UWorld::Serialize`, new `UWorld::UpdateWorldInfoCache` | `Engine/Src/UnWorld.cpp`, `Engine/Inc/UnWorld.h` (declaration) | 2013 0x390b90 / 0x38cac0: world-info cache refreshed after `PersistentLevel`, `SaveGameSummary` below licensee 27 only, GC sees `m_pAudioSystem` |
| `UWorld::SetGameInfo` | `Engine/Src/UnWorld.cpp` | 2013 0x38bfa0: no `IsServer` gate, no `DefaultGameType` start, `AGameInfo.DefaultMapPrefixes` picks the DLC game type (`DefaultGame.ini:12-14`), full map name to `SetGameType` |
| `USeqAct_PrepareMapChange::UpdateStatus` | `Engine/Src/UnSequence.cpp` | 2012 0x2f69d0 (inlined into `PostLoad` 2013 0x301ce0): no `MakeSafeLevelName` (its `ensure(!GIsRoutingPostLoad)` aborted the map load) |
| `UUIDataStore_OnlinePlayerData::InitializeDataStore` / `OnRegister` / `OnUnregister` | `Engine/Src/UnUIDataStores.cpp` | 2013 0x3efce0 / 0x3d45c0 / 0x3cf210: no friend-messages / party-chat providers (they lived in storage-less `DISHONORED_SHIM_STATIC` members, were garbage-collected and crashed `CommitMapChange` → `CleanUpBeforeLevelTransition` → `UnregisterDataStore`); profile/storage providers of the fixed retail classes; cached profile/storage bound |
| `UDisConv_Blurb::Serialize` | `DishonoredGame/src/disconv_blurb.cpp`, `DishonoredGame/Inc/CppText/UDisConv_Blurb.h` | 2013 0x8988a0 (2012 0x8e7d10): per-language `FDisBlurbLangInfo` block after the tagged properties ("Serial size mismatch: Got 394, Expected 3021" before) |
| `UDisConv_PlayerChoice::Serialize` | `DishonoredGame/src/disconv_playerchoice.cpp`, `CppText/UDisConv_PlayerChoice.h` | 2013 0x8a1850 (2012 0x8f1590): per-language choice blocks, pre-version `m_Choices` moved into `m_Choices_Static` |

Supporting edits in generated files (idempotent script `build\agentZ\apply_z_edits.py`, what the generator emits once the
CppText files exist): `DishonoredGame/Sources.cmake` drops `Src/disconv_blurb.cpp` and `Src/disconv_playerchoice.cpp` from
`DishonoredGame_EXCLUDE`; `DishonoredGame/Inc/DishonoredGameConversationClasses.h` gets the two
`#include "CppText/UDisConv_*.h"` lines. Of the 47 DishonoredGame `Serialize` overrides in the 2012 PDB only these two read
extra bytes from disk; the rest are save/GC/editor-only (agent X listed them for AC).
`function_status.csv`: 10 `ported`/`written` rows and 8 `needed` rows (SpawnPlayerController, Possess, UpdateCamera,
GetPlayerViewPoint, ClearCameraLensEffects, the three navmesh serializers), all tagged `agent Z`.

## How the runs were made (local-only stand-ins, never committed)

The shared tree was mid-edit (W, X, AA, AB, AC, Y all active). `build\agentZ_wt` is a `git worktree add --detach` of HEAD
`da24f65`; `build\agentZ\refresh_snapshot.py` copies every modified/untracked file of the shared `source/`, `cmake/`,
`CMakeLists.txt` over it (done once at 21:40, which brought in W's parity edits, AA's `UStaticMesh::Serialize`, AB's layouts,
X's `UGameEngine::Init` work, AC's natives); `build\agentZ\sync.py` copies agent Z's files; `build\agentZ\iter.sh` = sync +
patches + build + smoke. The snapshot's in-flight `Core/Inc/UnIOBase.h` + `Core/Src/UnAsyncLoading.cpp` (the 76-byte
`FAsyncIORequest` with `NormalizedFileName`) were reset to HEAD: with them the async IO thread asserted in
`FindCachedFileHandle` (`ArrayMax>=ArrayNum`) — worth a look by their owner. `build\agentZ\wt_others_patch.py` applies these
stand-ins to the snapshot only; each is a hand-over to the named owner:

| Stand-in | Owner | Why / retail evidence |
|---|---|---|
| `FSkeletalMeshLODInfo`: `bDisableCompression`/`bHasBeenSimplified` bits become storage-less shims | AA/O (`Engine/Inc/UnSkeletalMesh.h`) | retail SDK struct span 56 (`TriangleSortSettings` @44); ours 60 → every `LODInfo(i>0)` misindexed (`USkeletalMesh::PostLoad` assert) |
| `FMaterial::InitShaderMap`: no abort for special engine materials under `-nullrhi` | X/Y | every cooked material logs `Missing cached shader map`; `LevelColorationLitMaterial` would abort |
| `StaticAllocateObject` +256 bytes slack | X/AB | same stand-in as agent X; C++ classes larger than their retail size overran the heap (first seen as `FMallocDebug` RefCount asserts and a hang in `UStaticMesh::Serialize`) |
| `FParticleEmitterInstance::Resize`: no `GEngine` dereference when NULL | X (now in X's tree too) | startup packages load before `GEngine` (2013 `FEngineLoop::Init` 0x5e11b0 calls `LoadStartupPackages` 0x5e0e10 first) |
| `DisGameCrowdPopulationManager` gets the UObject constructor in the bridge (HEAD only) | X/AB (done in the shared tree) | the bridge gave the 5,444-byte UObject an Actor vtable (`AActor::PostLoad` on it) |
| unbound natives: `CallFunction` skips the parameters (`SkipFunction`) and zeroes the result, `ProcessEvent` skips the call, warn once | coordinator / Engine natives owner | the 147 natives above |
| `AGameInfo::execSpawnPlayerController` (header `DECLARE_FUNCTION` + `MAP_NATIVE`, body in `UnGame.cpp`) | Engine natives owner | 2013 exec 0x1c60c0 / body 0x2d13a0: `GWorld->SpawnActor(PlayerControllerClass, PlayerControllerClass->GetFName(), Location, Rotation)` |
| `FindFunctionChecked` warns once, `ProcessEvent(NULL)` returns | X/AA | the reference-only events listed above |
| `BeginRenderingViewFamily` and `UGameEngine::RedrawViewports` return under `GUsingNullRHI` | Y | the scene pass asserted on `FDownsampleSceneDepthPixelShader` (not in the cooked global cache); `UGameViewportClient::Draw` raises `PreRender` |
| `UOnlineSubsystem::Tick` does not assert | X (OSS) | `UOnlineSubsystemSteamworks` has no offline `Tick` yet |
| no `GameInfo.OnEngineHasLoaded` in `FEngineLoop::Init` | X | not in retail 0x5e11b0 |
| HEAD-only (before the refresh): W's parity edits (`case 7: break;`, raw blob copy, `PostLoad` gate), retail `UStaticMesh::Serialize`, analytics / cloud-storage singletons skipped | W / AA / X | all three are in the shared tree now |

Diagnostics (snapshot only, `build\agentZ\wt_diag_patch.py`): post-tag check of every constructed/serialized object's
`FMallocDebug` block, `ZDIAG` lines for a bad `AActor::Attached` array, for the object being serialized when
`IndexToObject` gets a bad export index, and for every unbound native in `UFunction::Bind`. Debug helpers:
`build\agentZ\zdbg.py` (agent O's debug loop + `--hang=N` all-thread stack dump + `--attach=PID`), `build\agentZ\dbg.py`,
`build\agentZ\hang.sh`.

## Follow-ups outside my files

1. **Next blocker (navmesh owner / AA)**: `UNavigationMeshBase` layout (688 vs retail 464) + `Serialize` 2013 0x2909e0,
   `operator<<(FNavMeshPolyBase&)` 0x2780f0, `FNavMeshEdgeBase::Serialize` 0x279d40 / `SerializeEdgeVerts` 0x279eb0.
2. **Engine natives (coordinator / AA / AB)**: the 147-function list; a generic fallback like the stand-in above in
   `UFunction::Bind`/`CallFunction` would turn today's jump-to-0 into the same warn-once contract as `DISHONORED_NATIVE_STUB`.
3. **AC**: `SaveGameList done` (`FDisAsyncSaveGameLister`, `RefreshSaveGameList` 0x609870), `execGameEnding` 0x5ecfe0,
   `UDishonoredEngine::LoadMap` 0x615210 / `PostCommitMapChange`, `ADishonoredGameInfo::PreCommitMapChange` 0x605c50.
4. **X**: reference-only events (`PlayerTick`, `OnEngineInitialTick`, `ServerUpdateLevelVisibility`, `OnEngineHasLoaded`),
   OSS `Tick`/`ReadFriendsList`/`ReadProfileSettings`/`ReadAchievements`; call `UWorld::UpdateWorldInfoCache` in
   `UGameEngine::LoadMap` after `UWorld::Init` like retail (UnGame.cpp is being edited by X; I did not touch it).
5. **Y**: `UGameViewportClient::Draw` → `PreRender`, the null-RHI scene pass (`FDownsampleSceneDepthPixelShader`).
6. **AA/O**: `FSkeletalMeshLODInfo` 60 → 56; material shader maps of the cooked packages are never found
   (`Missing cached shader map` for every material).
7. Retail spawns the game info / player controller with the class `FName` (golden: `PersistentLevel.DishonoredPlayerPawn`
   without suffix): `UWorld::SpawnActor` naming needs the 2013 body before `SetGameInfo` passes the name.

## Summary

Step 1 call graph with 2013 rvas is above; step 2 ported `ULevel`/`UWorld` serialization, the world-info cache,
`SetGameInfo`, `PrepareMapChange::UpdateStatus` and the online-player-data store; the two DishonoredGame serializers that read
extra data (`DisConv_Blurb`, `DisConv_PlayerChoice`) are written. Physics needed no guard (`WITH_NOVODEX=0`). In agent Z's
snapshot (other agents' in-flight work + local stand-ins), `-nullrhi` reaches `Game class is 'DishonoredGameInfo'`,
`Bringing World DishonoredGameFull_P.TheWorld up for play`, `Bringing up level for play took`, `Finished loading level`,
`Initializing Engine Completed` and `Initial startup: 5.20s`; `SaveGameList done` (AC) and the first `Committed map change`
are not reached. Next blocker: `UNavigationMeshBase::Serialize` 2013 rva 0x2909e0 while streaming `Dishonored_MainMenu_Env`.
No commits, no `git add`.
