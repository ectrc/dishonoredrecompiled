# Agent AI report — Engine convergence wave 2: UEngine/UWorld retail virtuals, the Arkane Engine classes out of the shims (2026-09-26)

Package "### AI" of `PHASE6.md`. Build dir `build\agentAI`, snapshot `build\agentAI_wt`, IDA copies
`resources/docs/idb/retail2013_agentAI.i64` / `shipping2012_agentAI.i64` (headless only), decompiles in
`build\agentAI\dec2013` and `dec2012` (54 functions), vtable dumps `build\agentAI\vtables_2013*.txt`
(`build\agentAI\dump_vtables.py`, a new headless IDA script: slot, target rva, size, demangled name, first bytes).
Patch scripts: `build\agentAI\patch1_engine.py`, `patch1b_engine.py`, `patch2_arkane.py`, `patch3_world.py`,
`patchlib.py`, `extract_shims.py`, `overlay_shared.py`. **No commits, nothing staged.**

**The session was cut off by a power cut and restarted; the edits below were re-applied from the patch scripts afterwards, so every
one of them is in the shared working tree now.** `resources/docs/agents/agentAI_status.csv` has 51 per-function rows (18 `ported`, 24 `written`, 2 `verified`, 6 `bringup`, 1 `layout`: **44 functions moved to ported/written/verified**, the accept criterion was >= 40)
(`rva,status,note`, the rva column is the 2012 one where a match exists and the note always carries the 2013 rva).

## What changed, by area

### 1. UEngine: the retail virtuals in 2013 vtable order (`EngineGameEngineClasses.h`, `UnEngine.cpp`)

The reference `UEngine` had none of Arkane's virtuals, so `UDishonoredEngine`'s ports (agent AC) could not override anything.
They are declared now in the order of the 2013 tables (`UEngine` vftable rva 0xc1edc0, `UGameEngine` 0xc1f0a8,
`UDishonoredEngine` 0xcdb390, all three dumped and compared slot by slot):

| Slot (+offset) | Virtual | 2013 rva of the engine body |
|---|---|---|
| 74 (+296) | `PlayLoadMapMovie(MapName, MovieName)` | 0x2097d0 (ported) |
| 75 (+300) | `OpenPauseMenu` | empty; `UDishonoredEngine` 0x605150 |
| 76 (+304) | `OnControllerDisconnected(ControllerId)` | empty everywhere |
| 77, 78 (+308, +312) | `OpenControllerConnectionMenu`, `OpenContentUnavailableMenu` | empty; `UDishonoredEngine` 0x5e4270 / 0x5e42a0 |
| 79 (+316) | `StopMovie` | 0x1d8900 (our body already matched) |
| 80 (+320) | `Init` | 0x1fe910 |
| 81, 82 (+324, +328) | `LoadProfile`, `SaveProfile` | pure in the engine (0x209530 / 0x209550 log "Pure virtual not implemented"); `UDishonoredEngine` 0x5e41b0 / 0x6050b0 |
| 83 (+332) | `PreExit` | 0x1d8850 (Wwise `StopAllSounds`; empty with the AkAudio stub) |
| 84 (+336) | `TickDisconnectedController` | pure (0x209570) |
| 96 (+384) | `NotifyActorDestroyed(Actor)` | empty; `UDishonoredEngine` 0x5fbd40 |
| 97, 98 (+388, +392) | `RenderDebugMenu`, `IsDebugMenuVisible` | compiled out of the shipping exe (empty / FALSE) |
| 100, 101 (+400, +404) | `PreCommitMapChange`, `PostCommitMapChange` | 0x1e3330 (written); `UDishonoredEngine` 0x616150 / 0x6152f0 |
| 102 (+408) | `ShouldStopMovieAtEndOfLoadMap` | 0x1d88d0 (written) |
| 103, 104 (+412, +416) | `IsLoadingGame`, `IsLoadingLevelState` | FALSE; `UDishonoredEngine` 0x5e4500 / 0x5e4510 |
| 120 (+480) | `IsLevelInGameState(LevelName)` | FALSE; `UDishonoredEngine` 0x601a80 (`FGameState::findLevelIndex`) |

Three reference virtuals are **not** 2013 virtuals and lost their `virtual` (no slot exists between their neighbours):
`DumpFrameTimesToStatsLog`, `GetSpriteCategoryIndex`, and `ConstructNetDriver` — the last one keeps `virtual` because
`UnPenLev.cpp`/`UnWorld.cpp` call it through `GEngine`, with a `DISHONORED(retail)` note.

Natives: `execPlayLoadMapMovie` takes `MapName`/`MovieName` in 2013 (exec 0x1eee70, the reference took none), and
`execOpenPauseMenu` (0x1c8910), `execOnControllerDisconnected` (0x1d47c0), `execOpenContentUnavailableMenu` (0x1c5e90),
`execWaitMovie` (0x1e2600) are retail natives the reference lacks — declared, defined and added to the natives table.
`UEngine::PlayLoadMapMovie` itself is ported from 0x2097d0 (per-map intro movie from `[FullScreenMovie]`, dropped when the
level is already in the save game or a game is loading; `PlayOnce` entries collected as retail does).
`UEngine::ApplyGameSettings` (0x1d87b0) is ported: client gamma + listener, then the two subtitle bits from `m_SubtitlesMode`.

### 2. `UWorld::Init` (2013 rva 0x3945f0): audio system and MapInfo (`UnWorld.cpp`)

The world now loads `[DishonoredMods] AudioSystemClass` (`DishonoredGame.DishonoredAudioSystem`) with
`StaticLoadClass`, constructs it with the world as outer, calls `Init()` through the first `UAudioSystem` virtual and logs
`DISHONORED(bringup): audio system DishonoredAudioSystem`; when the world info brought no `UMapInfo` one of
`[DishonoredMods] MapInfoClass` is constructed on it. `m_pWorldInfoCheckStreamingPersistent` is filled by
`UWorld::UpdateWorldInfoCache` (2013 0x38cac0), which `UWorld::Serialize` already calls on every load — that is retail's own
mechanism, and `UWorld::SpawnActor` calls it too (below). Retail's `FArkComponentManager` allocation (@712) is not ported:
the component manager is not in the tree.

### 3. `UWorld::SpawnActor` (0x256990) and `UWorld::DestroyActor` (0x252cf0) (`UnLevAct.cpp`, `UnWorld.h`)

`SpawnActor` takes retail's twelfth parameter, `FSpawnActorInitFunctor*` (2012 PDB: one vtable pointer, sizeof 4, declared in
`UnWorld.h`), and calls `DoInit(Actor)` after the transform and before `ConditionalForceUpdateComponents` and the begin-play
chain — the hook `FSpawnActor_TweakObj::DoInit` (2013 0x885650) needs to give a spawned actor its tweak object before
`PostBeginPlay` (agent AC follow-up 5). Agent AJ had already written the same struct in
`DishonoredGame/Inc/DishonoredGameNative.h` behind `#ifndef DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR`; `UnWorld.h` defines that
macro now, so AJ's copy switches itself off and both sides use Engine's declaration.
`DestroyActor` notifies the engine (`GEngine->NotifyActorDestroyed`) where retail does, after the owner is cleared and before
the actor leaves the level list.

### 4. The Arkane Engine-package classes moved out of the DishonoredGame shims

19 classes and 6 structs that `native_class_sizes.csv` marks `package_2013 = Engine` lived in
`DishonoredGameEngineShims.h` and were registered by DishonoredGame, although `Engine.upk`'s and every level's exports use
them. They are Engine's now, with the retail SDK layouts copied verbatim from the generated shim blocks
(`build\agentAI\extract_shims.py`) and asserted with `static_assert` on every size plus the load-bearing offsets:

- **new `Engine/Inc/EngineArkaneClasses.h`** (included from `Engine.h` after `EngineTextureClasses.h`): `UAudioSystem`,
  `UAkBaseSoundObject`, `UAkEvent`, `UAkBank`, `UArkComponentContainer`, and the registrant macro
  `AUTO_INITIALIZE_REGISTRANTS_ENGINE_ARKANE` (called from `AutoInitializeRegistrantsEngine`).
  `UAudioSystem` gets its eight retail virtuals in 2013 vtable order (`Init`, the listener-cell getter, `GetCellAtPoint`,
  `Update`, `Register`/`UnregisterAmbientSound`, `SuspendUpdate`, `ResumeUpdate`; slots 73..80 of vftable 0xc5d2d8, each one
  overridden by `UDishonoredAudioSystem` 0xd5c4b0).
- **`EngineInterpolationClasses.h`**: `UMatineeData`, `UInterpTrackKeyProperties`, the four Arkane tracks
  (`FaceTo`, `LookAt`, `Locomotion`, `StretchAnimControl`), their key-properties classes and their track instances, plus the
  six structs they embed (`FRuntimeMatineeData`, `FEditorMatineeData`, the four track keys).
- Bodies went into the retail source files of the 2012 PDB, which existed as comment-only stubs:
  `akevent.cpp`, `akbank.cpp`, `arkcomponentcontainer.cpp`, `matineedata.cpp`, `interptrack{faceto,lookat,locomotion,stretchanimcontrol}.cpp`;
  `IMPLEMENT_CLASS(UAudioSystem)` went into `UnWorld.cpp`, which is where the 2012 PDB attributes its class registration
  (rva 0x3a23a0), and `IMPLEMENT_CLASS(UInterpTrackKeyProperties)` into `UnInterpolation.cpp`.
- The generator needs no change for the move: `sdk_select` skips a `SHIM_PACKAGES` class that the tree declares, so
  regenerating DishonoredGame drops all 19 (**119 → 100 shim classes, 12,396 layout asserts, 0 pending**). Regenerated in the
  snapshot only; the coordinator regenerates the shared tree at merge (the shared tree has both declarations until then, so
  **the shared tree does not compile until that regeneration** — this is the one merge step my package needs).

Ported with them: `UAkEvent::PostLoad`/`PostRename`/`FixRequiredBank`/`GetMaxRadius`/`IsAudible`, `UAkBank::PostLoad`/`BeginDestroy`,
`UArkComponentContainer::Serialize`/`BeginDestroy`/`AddReferencedObjects`, `UMatineeData::Serialize`/`FindGroupByName`/`GetInterpGroup`,
the four track `PostLoad`s (each resolves its keys' target group through the owning `UMatineeData`) and the three interp-track
priority setters (0x4fc8a0 / 0x4fcc30 / 0x4fcc90, agent AC follow-up 3). Wwise-blocked parts (`ComputeAkID`, `ComputeMaxRadius`,
`UAkBank::Load`/`Unload`) carry `DISHONORED(bringup)` with the retail behaviour written out.

### 5. `UInterpData::Serialize`, `UFont`, `UTextureRenderTarget2D` — agent AA's three open items

- `UInterpData::Serialize` (0x2128f0) is written now that `UMatineeData` is in Engine: the sequence variable tracks
  `m_Data->m_iDataVersion` and marks the package dirty. Declaration added to `EngineSequenceClasses.h` (one line, the only
  file I touched outside my list).
- `UFont::GetScalingFactor` returns **1** (2013 slot 74 is the folded `fld1; ret 4`): retail has no `ScalingFactor` member and
  the storage-less shim returned 0, which made all canvas text invisible. `UMultiFont::GetScalingFactor` is ported from
  0x216b40 (`HeightTest / ResolutionTestTable(Index)`, 1 past the table).
- `UTextureRenderTarget2D`: `BYTE m_ResolutionType` @255 replaces the `SCRIPT_ALIGN` after `AddressY`, and `PostLoad`
  (0x17d330) resizes the target from `GSceneRenderTargets.GetBufferSizeX/Y() >> {0,0,1,2}[type]`.

### 6. `ArkSettings` (new `Engine/Inc/arksettings.h`, `Engine/Src/arksettings.cpp`)

`ArkSettingsParameters` (the 2012 PDB struct, 208 bytes, 52 members) with its constructor (0x53b450) and `Read` (0x539730:
the profile setting ids 65..130 through `UOnlinePlayerStorage`, with the fullscreen/vsync/speaker ids written back from
`GSystemSettings` under the override), plus `ArkSettings::GetParameters` (0x53b730) and `ApplyCurrentSettings` (0x53b790).
This is what `UDishonoredAudioSystem::Init` and `UEngine`/`UGameCrowdPopulationManager::ApplyGameSettings` need, and it closes
agent AC follow-up 3's `ArkSettings::ApplyCurrentSettings` item. The resolution provider, `FindListeners`, `OnSettingsChanged`,
`SaveSettings` and `Reset*` are not ported.

### 7. `UDishonoredAudioSystem` (`dishonoredaudiosystem.cpp` + new `Inc/CppText/UDishonoredAudioSystem.h`)

`Init` (0x7a3120) does the occluding-door cosine (a unit global in retail) and `ArkSettings::ApplyCurrentSettings(this)`;
`SuspendUpdate`/`ResumeUpdate` (0x7873d0 / 0x7873e0) toggle the suspend bit; `ApplyGameSettings` (0x7872b0) is reached but
silent. The fourteen Wwise id lookups, the end-of-event notify queue and the volume RTPCs need the Wwise 2012.1 SDK.
`UGameCrowdPopulationManager::ApplyGameSettings` (0x5620b0) stores `m_bAllowShadow`; its two propagation loops are bring-up.

### 8. Generator (`gen_classes_header.py`, one hunk)

`pure_virtual_stubs` now also treats a declaration in `Inc/CppText/<Class>.h` as one of the class's own methods, so a ported
interface override (here `UDishonoredAudioSystem::ApplyGameSettings`) is no longer duplicated by a generated
`appErrorf` stub. This is the generator's only change from me beyond the coordinator's pre-wave one.

## Verification (all numbers from `build\agentAI`, built from the snapshot `build\agentAI_wt`)

Because the shared tree cannot compile until DishonoredGame is regenerated, the verification build is the snapshot:
HEAD + my 27 files (`build\agentAI\snapshot_files.txt`) + the other agents' in-flight sources copied from the shared tree
(`build\agentAI\overlay_shared.py`: 130 files copied, the 3 generated DishonoredGame outputs others had touched skipped) + a DishonoredGame
regeneration. The `resources/docs/{types,symbols}` data the generator reads is git-ignored, so 9 files were copied into the worktree.

| Check | Result |
|---|---|
| `build\agentAI_wt_build.cmd DishonoredGame` (Debug, four module options, `DISHONORED_REFERENCE_DIR`) | **0 errors**, exe links (`build\agentAI\build6.log`, 589 steps). The earlier failures were not mine: 15 unresolved externals of agent AE's declarations before the overlay, AE's mid-edit `UnCamera.cpp`, and agent AJ's duplicate `FSpawnActorInitFunctor` |
| `CoreSmoke.exe` (repo root, `build\agentAI\coresmoke.txt`) | **99 passed, 0 failed, 0 skipped** |
| `xcheck_sdk_layout.py build/agentAI/layout_probe.txt` | types=2314 exact=1681 mismatching=0 **contract_mismatches=0** (0 rows, the same 2,314 types as the STATUS baseline; `retail_sdk_delta.md` unchanged) |
| `gen_layout_probe.py compare build/agentAI/layout_probe.txt` | probed=2341 exact=2316 **contract_mismatches=0** (no `--write`, so the delta doc is untouched) |
| `verify_phase2.py retail build/agentAI/layout_probe.txt` | **2/2 PASS** |
| DishonoredGame regeneration in the snapshot | 1823 classes, **119 -> 100 shim classes**, 12,396 layout asserts, **0 pending**, link emulation 9,289 members 0 differ |
| `build_and_smoke.py --build-dir build/agentAI --no-build --exe-name DishonoredGame_AI.exe --log-name agentAI.log --ini-dir build/agentAI/config --rhi null --timeout 300 --milestone "Initializing Engine..." --expect "Initial startup" --expect "DISHONORED(bringup): audio system DishonoredAudioSystem" --skip-native OnlineSubsystemPC` | **exit 0**: `Initial startup: 21.81s` and `DISHONORED(bringup): audio system DishonoredAudioSystem` at 20.49 s (`build\agentAI\smoke1.txt`, log `agentAI.log`, 5,191 lines). No `native not ported` line from an Engine class of mine, no layout assert fired |

CoreSmoke and LayoutProbe were built with `build\agentAI\verify.cmd`; the probe output was written from bash
(`LayoutProbe.exe > build/agentAI/layout_probe.txt`), never from PowerShell (BOM).

The d3d9 half of the accept line (first-frame text visible in a `-firstframe` dump) could **not** be run: the render thread
still aborts in `FMaterialInstanceResource::GetMaterial` from `FScene::AddPrimitive`, which is agent AG's material shader maps.
The font fix is argued from the retail slot instead: `UFont::GetScalingFactor` returned the storage-less shim `ScalingFactor`
(0), so every `DrawText` scaled to nothing; it returns 1 now, which is exactly what the folded retail slot (`fld1; ret 4`) does.

## What is left

1. `UUIDynamicFieldProvider` stays a DishonoredGame shim: it derives from `UUIDataProvider`, which lives in
   `EngineUIPrivateClasses.h`, and `Engine.h` does not include that header (only `UnController.cpp`,
   `DownloadableContent.cpp`, `GFxUI`, `IpDrv` and `DishonoredGame` do). Moving it means either moving it into
   `EngineUIPrivateClasses.h` (agent AE's lines this wave) or giving Engine a second Arkane UI header — a wave-5 item, with its
   `Serialize` (2013 0x40fa70) and its 19 natives.
2. `AInOutVolume`, `ADisFog`, `UArkPpNode*`, `UArkProfileSettings`, `UGameCrowdSpawner` and the other 100 remaining shim
   classes are the same kind of move; `UArkProfileSettings` in particular is what `ArkSettingsParameters::Read` wants
   (it is found by name today).
3. `FArkComponentBase` / `FArkComponentManager` are not ported, so `UArkComponentContainer`'s per-component GC and memory
   passes and `UWorld::Init`'s component manager are stubs.
4. AA's wave-2 list that I did not reach: `FStaticMeshRenderData` / `FStaticMeshComponentLODInfo` trimmed to retail,
   `USkeletalMesh::CalculateInvRefMatrices` (2013 differs, 2590 bytes), the `ULevelStreaming.LevelTransform` shim uses in
   `UnWorld.cpp` (5 sites, all in the streaming-transform block, which retail does not have) and the `UPostProcessChain.Effects`
   users outside AH's files.
5. The Wwise-blocked bodies of §4/§7 and the `UAudioSystem` virtuals' real signatures (`Update` takes
   `TSet<UAkComponent*>&` and `GetCellAtPoint`/`GetListenerCell` return `AInOutVolume*`/`ADishonoredAudioVolume*` in retail;
   Engine cannot name those types yet, so the slots use `void*` and `AVolume*`).
6. `UEngine::Init` (0x1fe910) itself is still the reference one; `UGameEngine::Init` (0x2357c0) and the `UDishonoredEngine`
   overrides of the new slots are agent AF's.

## Hand-overs

1. **Coordinator (merge):** regenerate DishonoredGame in the shared tree (`gen_classes_header.py DishonoredGame --sdk
   --module-header --sources-cmake`) as part of my merge — without it the tree has each of the 19 moved classes twice.
   `DishonoredGameLayouts.h` keeps 0 pending asserts after the regeneration.
2. **Agent AJ:** `DISHONORED_HAVE_FSPAWNACTORINITFUNCTOR` is defined in `UnWorld.h` now, so the copy in
   `DishonoredGameNative.h` compiles out; `UWorld::SpawnActor`'s twelfth parameter is in place for
   `UDisTweaksBase::SpawnActor_Derived` to pass `FSpawnActor_TweakObj(this)` (the call site still passes nothing).
3. **Agent AF:** the `UDishonoredEngine` overrides are real overrides now (`PlayLoadMapMovie`, `OpenPauseMenu`,
   `OnControllerDisconnected`, `OpenControllerConnectionMenu`, `OpenContentUnavailableMenu`, `IsLoadingGame`, `StopMovie`,
   `PreCommitMapChange`, `PostCommitMapChange`, `NotifyActorDestroyed`, `IsLoadingLevelState`, `IsLevelInGameState`); the
   `CppText/UDishonoredEngine.h` comment that says Engine cannot reach them is stale.
4. **Agent AE:** `EngineGameEngineClasses.h` carries both your DLC/gravity natives and my `UEngine` virtuals — one file, two
   packages' edits, no overlap in the hunks.
5. **Agent AG/AH:** `UTextureRenderTarget2D::PostLoad` now reads `GSceneRenderTargets` during `PostLoad`
   (`SceneRenderTargets.h` included from `TextureRenderTarget2D.cpp`); a render target whose `m_ResolutionType` is non-zero
   resizes itself to the scene buffers.
6. **`UEngine::Init`'s callers:** `LoadProfile`, `SaveProfile` and `TickDisconnectedController` are `PURE_VIRTUAL` now, so any
   Engine-only build that instantiates a bare `UEngine` and calls them aborts like retail does (it logs
   "Pure virtual not implemented").
