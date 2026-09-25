# Agent X — Milestone 3 driver: seek-free default, bridged classes, `Startup.upk`, `GEngine->Init()` (2026-09-25)

Package X of `resources/docs/PHASE5.md`. The retail 2013 exe is the target: every "2013 rva" is `retail2013_agentX.i64`
(copy of `retail2013_named.i64`), "2012 rva" is `shipping2012_agentX.i64` / `functions.csv`, readable version only.
Decompiles: `resources/reference/decomp/agentX/` (git-ignored).

## Result

| Step | State | Evidence (run log) |
|---|---|---|
| 1. Seek-free like retail | **done**: a plain launch resolves `CookedPCConsole` without `-seekfreeloadingpcconsole` | `build/agentX/plain_final_plain.log`: root-set line, `Initializing Engine...`, `LoadMap:` |
| 2. Four bridged classes registered | **done**: no `unbound native class` line without `-allowunboundnatives` | `build/agentX/smoke_step2.txt` exit 0 (`-NoLoadStartupPackages`), 0 bridge warnings |
| 3. `Startup.upk` + startup packages | **done** (on the snapshot below): `Log: 63718 objects as part of root set at end of initial load.` (golden 51634) | `build/agentX/smoke_final.log:2366` |
| 4. `UEngine::Init` → `UGameEngine::Init` → `Browse` | **done**: `Log: Initializing Engine...`, then the golden :87 line `Log: LoadMap: DishonoredGameFull_P?Name=Corvo?Team=255` (byte-identical), no native stub hit, also with `-strictnatives` and no skip list | `smoke_final.txt`, `smoke_strict.txt` exit 0 |
| 5. `FEngineLoop::Init` tail + 5 s of `EngineTick` | **blocked by Z**: `LoadMap` aborts in `ULevel::Serialize` (`Bad export index 9727/1597`), so `Browse` never returns and the tick loop is not reached. The tail itself was compared with 2013 rva 0x5e11b0 (below) | — |
| 6. Hand-over notes | below | — |

**Acceptance** (PHASE5 X): `build_and_smoke.py ... --milestone "Initializing Engine..." --expect "LoadMap: DishonoredGameFull_P"`
exit 0 with `-nullrhi`, no `-NoLoadStartupPackages`, no `-allowunboundnatives`; the same run with no skip list and
`-strictnatives` exits 0; a plain `DishonoredGame_X.exe -log` (no switches but the isolation ones) reaches the same
`LoadMap:` line, with the null RHI and with the D3D9 RHI.

### Which build the numbers come from (read this)

The shared working tree does not build on its own today (other agents mid-edit), and the shared `external/libpng-build`
is rewritten by every agent's build (`pnglibconf.h` disappears mid-build: my first two builds of the working tree failed
on it). All numbers above come from **`build/agentX`** = snapshot worktree `build/agentX_wt` (HEAD `da24f65`) with its
own FetchContent binary dirs, assembled by `build/agentX_pullsel.py`:

1. my files (`build/agentX_sync.py`, list below);
2. other agents' in-flight work the path needs, copied from the working tree as of this afternoon:
   W (animation parity: `EngineAnimClasses.h`, `AnimSequence.uc`, `AnimationEncodingFormat*.{h,cpp}`,
   `UnSkeletalAnim.cpp`, `UnAnimPlay.cpp`, `EdgeAnim.h`, `EdgeAnimEvaluate.cpp`), AA (`UnStaticMesh.{h,cpp}`,
   `InstancedStaticMesh.cpp`, `UnSkeletalMesh.{h,cpp}`, `UnActor.cpp`), AB (`GameFramework*Classes.h`, `GameCrowd.cpp`,
   `IpDrvClasses.h`, `OnlineSubsystemSteamworksLayouts.h`, `Core/Inc/DishonoredLayouts.h`, `EngineClasses.h` +
   `UnNavigationMesh.cpp` for `FPolyReference`), AC (the whole `DishonoredGame` module, regenerated in the snapshot with
   AC's generator for the `CppText` hooks). **Not** taken: AB's `FAsyncIORequest` 76-byte change (`UnIOBase.h`,
   `UnAsyncLoading.cpp`: crashes in `FAsyncIORequest::~FAsyncIORequest` from `QueueIORequest`), Y's shader-cache files
   (see Y below);
3. **snapshot-only bring-up patches** (`build/agentX_localfix.py`, never in the working tree; each one is a hand-over):
   - `UObject::StaticAllocateObject`: +512 bytes per allocation. Without it `FMallocDebug` asserts
     (`GIsCriticalError||Ptr->RefCount==1`) while `DishonoredGame.upk` CDOs are built: native classes whose C++ is larger
     than the retail script size (probe `build/head_wt/build/wt/layout_probe.txt` vs `native_class_sizes.csv`):
     `ADisGameCrowdAgentSkeletalRat` +224, `ADishonoredPlayerController` +16, `ADynamicGameCrowdDestination` +16,
     `UOnlineSubsystemSteamworks` +4 (AB).
   - `FSkeletalMeshLODInfo`: the reference `bDisableCompression`/`bHasBeenSimplified` bits make it 60 bytes, retail is 56
     (retail SDK span 0..56: DisplayFactor, LODHysteresis, LODMaterialMap @8, bEnableShadowCasting @20, TriangleSorting @32,
     TriangleSortSettings @44) → `USkeletalMesh::PostLoad` reads garbage from `LODInfo(1)` on (`ArrayNum>=0` assert). Made
     the two bits storage-less shims (AA, `UnSkeletalMesh.h`).
   - `FMaterial::InitShaderMap`: "Failed to find shader map for default material %s" is a warning instead of `appErrorf`
     (HEAD's `VER_MIN_SHADER` 836 rejects every cooked shader; see Y).
   - `FOutputDeviceWindowsError::Serialize`: `DebugBreak()` when a debugger is attached (stack of every `appErrorf` under
     `build/agentX_dbg.py`).

W's parity edits: I first applied the three edits locally (`build/agentX_wparity.py`), then replaced them with W's
finished files (agentW.md part 1). None of the above is in the working tree.

## What changed in the working tree (my files)

| File | Change | Evidence |
|---|---|---|
| `Launch/Src/LaunchEngineLoop.cpp` | seek-free flags as retail: `GUseSeekFreeLoading = !EDITOR`; `GIsSeekFreePCServer = -SEEKFREELOADINGSERVER`; `GIsSeekFreePCConsole = !EDITOR && (-SEEKFREELOADINGPCCONSOLE \|\| !-SEEKFREELOADING)`; no `NOSEEKFREELOADING`, no Content-folder probe. `-seekfreeloadingpcconsole` still accepted | `DISHONORED(port)`: `FEngineLoop::PreInit` 2013 rva 0x5e1910 (2012 0x629460). `appGetPlatformType` 2013 rva 0x14d10 returns 64 (`PLATFORM_WindowsConsole`) on the flag, so `UnMisc.cpp:3867` rewrites `\CookedPC` → `\CookedPCConsole`; `UnMisc.cpp` needed no change |
| `Engine/Src/arkhealthinterface.cpp`, `arksettings.cpp` | `IMPLEMENT_CLASS(UArkHealthInterface)`, `IMPLEMENT_CLASS(UArkSettingsListenerInterface)` | `DISHONORED(port)`: 2013 GetPrivateStaticClass 0x533a40 / 0x533b00 (56 bytes, flags 0x4001 / 0x10004001) |
| `Engine/Inc/DebugCameraController.h` (new), `Engine/Src/debugcameracontroller.cpp` | `Engine.DebugCameraController` on `APlayerController`, retail SDK members (span 1320..1368, `Oryginal*` spelling), `static_assert(sizeof == 1376)`, natives table `GEngineADebugCameraControllerNatives`, bodies moved from GameFramework (identical to 2013 0xe9020 / 0xdcea0 / 0xe9160 / 0xed9b0 / 0xd92d0) | `DISHONORED(retail)`: `script_classes_2013.json` super `Engine.PlayerController`, config Input; `native_class_sizes.csv` 1376, super `APlayerController` |
| `GameFramework/Src/DebugCameraController.cpp` | deleted (moved to Engine) | — |
| `GameFramework/Inc/GameFrameworkClasses.h` (patch `build/agentX_patch_gf.py`, idempotent; AB kept it) | `ADebugCameraController` removed (decl, `AUTOGENERATE_FUNCTION`s, registrant, natives table, verify lines); `FGameCrowdAttractor` + `UGameCrowdPopulationManager` (UObject + `IArkSettingsListenerInterface`, 144 bytes) added and registered instead of the reference Actor; `VERIFY_CLASS_SIZE_NODIE(UGameCrowdPopulationManager)` | `DISHONORED(layout)`: `native_class_sizes.csv` 144, flags 0x0, super UObject; 2012 PDB bases UObject @0, interface @56; retail SDK members @60..144 |
| `GameFramework/Src/gamecrowdpopulationmanager.cpp` | `IMPLEMENT_CLASS(UGameCrowdPopulationManager)`, size assert, empty `ApplyGameSettings` (`DISHONORED(bringup)`, 2013 0x5620b0 not ported) | — |
| `Engine/Src/UnEngine.cpp` | `AutoInitializeRegistrantsEngine` registers the two interfaces and `ADebugCameraController` (+ natives lookup); `InitializeObjectReferences`: retail texture set (+`DefaultBlackCubemapTexture`, no screen-door / image-grain / landscape-hole / APEX loads — the "Failed to load 'Texture2D None.'"/"PhysicalMaterial None." warnings are gone), +`ShadedLevelColorationTranslucentUnlitMaterial` | `DISHONORED(retail)` / `DISHONORED(port)`: 2013 rva 0x1f62a0 (2012 0x20c740) load list mapped to retail SDK `UEngine` offsets |
| `Engine/Src/UnGame.cpp` | `UGameEngine::Init`: no analytics singleton / `StartSession`; DLC class names forced to `Engine.DownloadableContentEnumerator` / `Engine.DownloadableContentManager` / `Engine.ArkDLCManagementBridge` before the DLC objects; `InitGameSingletonObjects` = retail `InitDLCObjects` (enumerator, manager + `eventInit`, `ArkDLCManagementBridge` loaded through `UObject` because the class is a DishonoredGame-registered Engine shim), no warnings, no cloud singleton | `DISHONORED(port)`: 2013 rva 0x2357c0 (disassembly: `lea ecx,[esi+688h]` / `[esi+69Ch]` stores), `InitDLCObjects` 2013 0x214250 (2012 0x22bdc0) |
| `Engine/Src/ParticleEmitterInstances.cpp` | `FParticleEmitterInstance::Resize`: the `MaxParticleResize` limit only when `GEngine` exists, no warning (startup packages init particle systems from `UParticleSystemComponent::PostLoad` before `GEngine`) | `DISHONORED(port)`: 2013 rva 0x46cd90 (2012 0x495d60). Outside my file list; small and blocking — AA please keep it |
| `WinDrv/Src/WinClient.cpp` | DirectInput mouse setup: HRESULTs checked in sequence, one `DISHONORED(bringup)` warning on failure instead of `verify`, `EnumDevices` only with a device | retail 2013 0x5c9d50 ignores the HRESULTs (Shipping `verify`) |
| `OnlineSubsystemSteamworks/Src/OnlineSubsystemSteamworksOffline.cpp` (new) | `execInit` = retail `UOnlineSubsystemSteamworks::Init` offline path: `UOnlineSubsystem::Init`, `m_paEnumeratedDLCs`, `bLastHasConnection`, logged-in fields reset, `ConnectionStatusChangeDelegates(OSCS_ServiceUnavailable)`, the five `Set*Interface(self)` events, `ProfileDataDirectory` default `.\`, local-profile sign-in (CDO `LocalProfileName`, player 0, `LS_UsingLocalProfile`, `LoginChangeDelegates(0)`), TRUE; `execGetLoginStatus` (LS_NotLoggedIn without Steam), `execIsControllerConnected` (TRUE) | `DISHONORED(port)`: Init 2013 0x5ad3d0 (2012 0x5f2b50), InitSteamworks 0x5ac1d0, SignInLocally 0x5aab40, execGetLoginStatus 0x5a4270 → GetLoginStatus 0x5a52c0, execIsControllerConnected 0x5a4670 → vtable +396 = 0x5ea9d0 (`return 1`). Not ported (Steam-only): overlay notification position, `SetRichPresence`, the leaderboard helper in `pLeaderboardHelper` (stays NULL) |
| `OnlineSubsystemSteamworks/Src/OnlineSubsystemSteamworksNativeStubs.cpp`, `OnlineSubsystemSteamworksNativeStubs.ported.txt` (new) | the three stubs removed; the generator skips them | — |
| `resources/docs/function_status.csv` | 28 rows (18 ported/written/reference, 10 needed) | — |

`DisGameCrowdPopulationManager` is generated again: AC's 19:56 regeneration picked it up from the header above
(`UDisGameCrowdPopulationManager : UGameCrowdPopulationManager`, 5444-byte asserts pass); I did not touch `DishonoredGame/*`.
`Core/Src/UnMisc.cpp` needed no change. The `-skipnativepkgs`/`-allowunboundnatives` switches stay (opt-in,
`DISHONORED(bringup)`); neither is needed any more: `OnlineSubsystemPC` is not in the runtime package list
(`appGetGameNativeScriptPackageNames` loads only `OnlineSubsystemSteamworks` outside cooking).

## Commands (all from `D:\RecompileDishonored\Recompile`, build `build/agentX`)

```
build\agentX_configure.cmd          (snapshot configure + build; build\agentX_build.cmd = sync + build)
python build\agentX_pullsel.py      (re-assemble the snapshot: selected in-flight files + mine + GF patch + local fixes)

step 2: python resources\tools\build_and_smoke.py --build-dir build\agentX --no-build --exe-name DishonoredGame_X.exe
        --log-name agentX.log --ini-dir build\agentX\config --rhi null --milestone "objects as part of root set at end of initial load"
        --expect "objects as part of root set" --skip-native OnlineSubsystemPC "--extra-args=-NoLoadStartupPackages"
        -> exit 0, 55567 root-set objects, 0 "unbound native class" lines (build/agentX/smoke_step2.txt/.log)
accept: python resources\tools\build_and_smoke.py --build-dir build\agentX --no-build --exe-name DishonoredGame_X.exe
        --log-name agentX.log --ini-dir build\agentX\config --rhi null --milestone "Initializing Engine..."
        --expect "LoadMap: DishonoredGameFull_P" --skip-native OnlineSubsystemPC
        -> exit 0 (build/agentX/smoke_final.txt/.log); with "--extra-args=-strictnatives" and no --skip-native: exit 0 (smoke_strict.txt)
plain:  python build\agentX_plain.py final_plain            (DishonoredGame_X.exe -log -unattended + isolation switches only)
        -> LoadMap reached (build/agentX/plain_final_plain.log); with -nullrhi: plain_plain_nullrhi.log; D3D9: plain_plain_d3d9.log
debug:  python build\agentX_dbgrun.py <tag> <hang_s> [args]  (build/agentX_dbg.py: stops on AV / appErrorf / hang, EBP-chain
        stack symbolized from the .map; the WOW64 breakpoint 0x4000001F after start-up is treated as a stop)
```

`build_and_smoke.py` always passes `-seekfreeloadingpcconsole`; since step 1 it is redundant (coordinator may drop it).

## Natives the path hits

With the snapshot build every `DISHONORED(bringup): ... native not ported` line up to `LoadMap:` is gone (`-strictnatives`
passes). The ones my runs produced and I ported (blocking, small): `UOnlineSubsystemSteamworks::execInit` (2013 0x5ad3d0
via 0x1c5da0), `execGetLoginStatus` (0x5a4270), `execIsControllerConnected` (0x5a4670). Everything else on the
`DishonoredEngine`/`DishonoredViewportClient`/`DisLocalPlayer` path is AC's (`UDishonoredEngine::Init`, viewport client,
player input, `DisConv_Blurb`/`DisConv_PlayerChoice`/`DisTweaksBase`/`DishonoredGameInfo` serializers in the snapshot).
Handed to AC as `needed` in `function_status.csv`: the remaining DishonoredGame `Serialize` overrides without a `CppText`
yet — `ADisMovableLimb` 0x642c30, `ADisWatchTower` 0x6195c0, `ADishonoredPawn` 0x748ee0, `UDisAIBlackboard` 0x72ffc0,
`UDisDialogTree` 0x88b210, `UDisTweaks_DefenceTower` 0x6208a0, `UDisTweaks_SkeletalBreakable` 0x618c30,
`UDisTweaks_StaticBreakable` 0x6209a0, `UDisTweaks_UsableObject` 0x64e170, `UDishonoredGlobalAIManager` 0x860430
(2013 rvas; a size mismatch in any of them aborts the load, `Serial size mismatch` like `DisConv_Blurb` did: Got 394,
Expected 3021). Handed to AB: `UGameCrowdPopulationManager::ApplyGameSettings` 0x5620b0.

## Step 5 — `FEngineLoop::Init` tail vs 2013 rva 0x5e11b0

Retail after `GEngine->Init()`: timing init, `SECONDS=` / `FPS=` (after `Init`, ours parses them before), `EXEC=`,
`GIsRunning = 1`, `FObjectPropagator::Unpause`, `GFullScreenMovie->GameThreadStopMovie`, the `movietest` block. The
retail 2013 exe is Shipping and has **no** log strings at all (`Initializing Engine`, `Initial startup` are not in it:
UTF-16 byte search in the 2013 db) — the golden log is the 2012 ArkProfile build, so our reference `debugf`s stay.
`CheckNativeClassSizes` / `eventOnEngineHasLoaded` are left as they are (the former is a no-op without
`-CHECK_NATIVE_CLASS_SIZES`). `-SECONDS=5` exists in retail (benchmark exit) and is the planned step-5 check once
`LoadMap` returns.

## Hand-over

### Z (map load)
`LoadMap: DishonoredGameFull_P?Name=Corvo?Team=255` is logged; then `UGameEngine::LoadMap` (2013 0x240b40) →
`LoadPackage` → `ULinkerLoad::Preload(ULevel)` → `ULevel::Serialize` (2013 0x259f30, 2012 0x2737b0, 1068 bytes) → the
`Actors` array → `operator<<(UObject*&)` → `appErrorf("Bad export index 9727/1597")`: `ULevel::Serialize` reads out of
phase before `Actors` (first delta of the level format). Stack in `build/agentX/dbg_s4a.txt`. `Browse` → `LoadMap` is
reached 3.2–4.4 s after start (golden 3.44 s).

### Y (window / device / shaders)
- `-nullrhi`: `UWindowsClient::Init` (DirectInput now warn-and-continue), `DishonoredViewportClient` construction,
  `CreateViewportFrame`, `SetViewportFrame`, the startup-movie call and `eventInit` all pass; no crash before `LoadMap`.
  The D3D9 RHI run (no `-nullrhi`) reaches the same `LoadMap` line (`plain_plain_d3d9.log`), with a spurious
  "Command line -d3d11 set, but D3D11 is not supported" warning at RHI init although no `-d3d11` is on the command line.
- `UWindowsClient::GetPlayWorldViewport` (PIE helper) runs in game and looks up `UnrealEd.PlayInEditor_RHI_F`
  (a `LocalizationWarning` the golden log does not have).
- Your `VER_MIN_SHADER 786` (`ShaderManager.h`) makes `FMaterial::InitShaderMap` → `GetLocalShaderCache` →
  `UShaderCache::Load` deserialize the cooked material shaders, which crashes in
  `TLightPixelShader<FSpotLightPolicy,FNoStaticShadowingPolicy>::Serialize` → `FVertexFactoryPSParameterRef` (`Bad name
  index 1048704/1320`), and with HEAD's `ShaderCache.cpp`/`MaterialShader.cpp` in `FMaterialShaderMap::Serialize` →
  `FUniformExpressionSet` → `FMaterialUniformExpression` (`dbg_s3e.txt`, `dbg_s3f.txt`). With 836 every shader is
  rejected and `Startup.upk`'s special engine materials abort ("Failed to find shader map for default material
  LevelColorationLitMaterial") — my snapshot turns that into a warning. The material-shader cache format is the next
  blocker of a real run.

### AA
`FSkeletalMeshLODInfo` 56 bytes (above); the `Resize` port in `ParticleEmitterInstances.cpp`; `UParticleSystemComponent`
`PostLoad`/`InitializeSystem` 2013 0x4b1b60 / 0x4a9af0 differ slightly (no DLE `AddRef`, no delay block, `IsTemplate(0x600)`).

### AB
The four C++-larger-than-retail classes (heap corruption, above); `FAsyncIORequest` 76-byte change crashes in its
destructor from `QueueIORequest` (`dbg_s3g.txt`); `Engine.OnlineRecentPlayersList` is requested by script but retail's
Engine package has no such class (warning before `LoadMap`). You own the retirement of `AGameCrowdPopulationManager`
(your note in agentAB.md).

### Coordinator
Root set: ours 63,718 vs golden 51,634 (2012 ArkProfile, which also loads the DLC tweaks) — not investigated;
`build_and_smoke.py` can drop `-seekfreeloadingpcconsole`; shared `external/*-build` dirs break parallel builds
(`pnglibconf.h`) — snapshots should get their own FetchContent binary dirs by default.

## Files

Working tree: `Launch/Src/LaunchEngineLoop.cpp`, `Engine/Src/{UnEngine,UnGame,arkhealthinterface,arksettings,
debugcameracontroller,ParticleEmitterInstances}.cpp`, `Engine/Inc/DebugCameraController.h` (new),
`GameFramework/Inc/GameFrameworkClasses.h` (shared with AB), `GameFramework/Src/gamecrowdpopulationmanager.cpp`,
`GameFramework/Src/DebugCameraController.cpp` (deleted), `WinDrv/Src/WinClient.cpp`,
`OnlineSubsystemSteamworks/Src/{OnlineSubsystemSteamworksOffline.cpp (new), OnlineSubsystemSteamworksNativeStubs.cpp}`,
`OnlineSubsystemSteamworks/OnlineSubsystemSteamworksNativeStubs.ported.txt` (new), `resources/docs/function_status.csv`,
this report. Scratch (not repo tools): `build/agentX_*.{cmd,py}`, `build/agentX_wt` (snapshot worktree — contains no
stage and no junction; remove it with `git worktree remove` only after checking that), IDA copies
`resources/docs/idb/{shipping2012,retail2013}_agentX.i64`.

## Summary

Seek-free resolution is retail's by default, the four bridged classes are registered for real (the bridge switches are no
longer needed), `Startup.upk` and all startup packages load (63,718 root-set objects), `UEngine::Init` and
`UGameEngine::Init` run through the Dishonored engine/viewport classes and the offline Steamworks subsystem to the golden
`LoadMap: DishonoredGameFull_P` line with zero unported natives (`-strictnatives`), plain launch included. That result
needs, besides my files, other agents' in-flight work and four snapshot-only bring-up patches (listed above); the next
blockers are Z's `ULevel::Serialize` and Y's material shader cache format, and the tick loop (step 5) waits for `LoadMap`.
