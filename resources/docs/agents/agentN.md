# Agent N — Milestone 1: real Launch (2026-09-25)

Build dir `build\agentN` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`); logs and the error
grouper (`analyse.py`) in `build\agentN\logs`. Tools: `resources/tools/stage_retail.py`,
`resources/tools/build_and_smoke.py`. Every number below is from the **2012 Shipping PDB**
(`resources/docs/symbols/functions.csv`, `module_map.md`) unless it says retail 2013.

## Result

**Milestone 1 reached.** `build\stage\Binaries\Win32\DishonoredGame.exe -log -nosteam
-seekfreeloadingpcconsole -unattended` writes `Init: Object subsystem initialized` (line 23 of
`Launch.log`, 81 lines total) and goes on to `FSystemSettings::Initialize`, where it stops on an
Engine/retail-ini mismatch (see "Run"). `build_and_smoke.py` exits 0.

## What was done

1. **`DISHONORED_REAL_LAUNCH`** (`source/Development/Src/Launch/CMakeLists.txt`, default OFF).
   ON compiles the reference `Launch.cpp`, `LaunchEngineLoop.cpp`, `LaunchMisc.cpp` plus the new
   `Src/DishonoredStubs.cpp`; OFF keeps `DishonoredLaunchStub.cpp`. `PIB.cpp` is the `_WINDLL`
   "plugin in browser" build and is not compiled (PDB attributes no function to it). The exe links
   `Core Engine` plus whichever of `GameFramework IpDrv WinDrv` exist as targets, gets
   `dishonored_apply_defines` (`/Zp4`, the module defines), a `.map` file and
   `/NODEFAULTLIB:libpng15.lib` (see link-error table).
2. **`GAMENAME == DISHONOREDGAME` branches** in `LaunchEngineLoop.cpp` (every `#error Hook up your
   game name here` site, all marked `// DISHONORED`):
   - registrant/name externs and calls: `AutoInitializeRegistrantsDishonoredGame`,
     `AutoGenerateNamesDishonoredGame`, `AutoCheckNativeClassSizesDishonoredGame`; the audio slot
     calls `AutoInitializeRegistrantsAkAudio` / `AutoGenerateNamesAkAudio` instead of XAudio2
     (module_map.md: `akaudio`, no XAudio2 in the exe);
   - `appSetGameName()` → `GGameName = "Dishonored"` (retail tree is `DishonoredGame\`, inis are
     `DishonoredEngine.ini` …);
   - `GGameIcon`/`GEditorIcon` = the demo icon ids (PCLaunch.rc is not compiled; `LoadIcon` just
     returns NULL);
   - the OSS selection (`appGetOSSPackageName` and the registrant block) treats
     `IS_DISHONOREDGAME` like `WITH_STEAMWORKS`: retail links `OnlineSubsystemSteamworks`
     (module_map.md) and ships `OnlineSubsystemSteamworks.upk`, while `WITH_STEAMWORKS=0` only
     keeps the SDK headers out of the build. The registrants are stubs until that module builds.
   `LaunchGames.h` was left alone: `DISHONOREDGAME=9` comes from `cmake/DishonoredDefines.cmake`.
3. **Module targets** (root `CMakeLists.txt`, options default ON): `GameFramework` (17 units),
   `IpDrv` (28 units, links `ws2_32 wininet`), `WinDrv` (11 units, links `dinput8 dxguid xinput`,
   `WITH_WINTAB=0`). Compile fixes:

   | Module | Problem | Fix |
   |---|---|---|
   | IpDrv | `UdpLink.cpp`: `AUdpLink` undeclared (the reference `IpDrvClasses.h` has no such class); `UCompressCommandlet.cpp`: `FILECOPY_*` enums gone. Neither is in the reference `IpDrv.vcxproj`, neither has a PDB function | `IpDrv_EXCLUDE` in `IpDrv/Sources.cmake` |
   | WinDrv | `WintabSupport.h` includes `Development/External/wintab` (tablet pressure, editor), absent from the reference tree; `UnBuild.h` defaults `WITH_WINTAB 1` | `target_compile_definitions(WinDrv PRIVATE WITH_WINTAB=0)`; `WinClient.cpp` include moved under `#if WITH_WINTAB` like `WinViewport.cpp` already has (`// DISHONORED`). No `FWinTab` function in the PDB |
   | GameFramework | none | |

   D3D9Drv was **not** made a target: the exe links with Engine's null RHI (below).
4. **Stubs** `Launch/Src/DishonoredStubs.cpp` (owner and reason on every block):

   | Symbol(s) | Owner | Why referenced |
   |---|---|---|
   | `AutoInitializeRegistrantsDishonoredGame`, `AutoGenerateNamesDishonoredGame`, `AutoCheckNativeClassSizesDishonoredGame` | DishonoredGame (skeleton, 1,043 files) | the new GAMENAME branch |
   | `AutoInitializeRegistrantsAkAudio`, `AutoGenerateNamesAkAudio` | AkAudio (Wwise) | audio slot of the GAMENAME branch |
   | `AutoInitializeRegistrantsGFxUI` | GFxUI (`WITH_GFx=0`) | called unconditionally by `InitializeRegistrantsAndRegisterNames` |
   | `AutoInitializeRegistrantsOnlineSubsystemSteamworks`, `AutoGenerateNamesOnlineSubsystemSteamworks` | OnlineSubsystemSteamworks (`WITH_STEAMWORKS=0`) | OSS registrant block |
   | `D3D9CreateRHI` → `NullCreateRHI()` | D3D9Drv `D3D9Device.cpp` (PDB rva 0x60a180) | `DynamicRHI.cpp RHIInit`; Engine's `NullRHI.cpp` compiles under `USE_DYNAMIC_RHI`, so the null RHI stands in for the device |
   | `appBeginDrawEvent`, `appEndDrawEvent`, `appSetCounterValue` | D3D9Drv `D3D9Util.cpp` (PDB rva 0x5fd420) | PIX markers from `UnSceneUtils.h`, 32 Engine units; `NullRHI.cpp` only defines them under `USE_NULL_RHI` |
   | `D3D9BeginCompileShader`, `D3D9FinishCompilingShaderThroughWorker` | D3D9Drv `D3D9ShaderCompiler.cpp` | `ShaderCompiler.cpp`; return FALSE (cooked builds load the shader cache) |
   | `GetCompatibilityLevelWindows`, `SetCompatibilityLevelWindows`, `SetDefaultResolutionForDevice` | D3D9Drv `CompatibilityEvaluator.cpp`, `D3D9HardwareSurvey.cpp` | `UnVcWin32.cpp appGet/SetCompatibilityLevel`, `PreInit -firstinstall` |
   | `D3D11CreateRHI`, `IsDirect3D11Supported`, `D3D11BeginCompileShader`, `D3D11FinishCompilingShaderThroughWorker` | D3D11Drv (not imported: no d3d11drv in the retail exe) | `RHIInit`, `ShaderCompiler.cpp` reference them unconditionally |
   | `OpenGLCreateRHI`, `OpenGLBeginCompileShader`, `OpenGLFinishCompilingShaderThroughWorker`, `AddMaterialToOpenGLProgramCache` | OpenGLDrv (not imported) | `RHIInit`, `ShaderCompiler.cpp`, `Material.cpp` |
   | `FAVIWriter::GetInstance` → NULL | Engine `AVIWriter.cpp` (excluded, DirectShow; agentB.md) | `UnEngine.cpp`/`UnGame.cpp` movie capture; no PDB function |
   | `FUnitTestFramework::{GetInstance, ctor, dtor, ContainsTest, RunAllValidTests, RunTestByName, DumpUnitTestExecutionInfoToContext}`, `FUnitTestFeedbackContext::Serialize` | Core `UnitTest.cpp` (excluded test harness) | `UEngine::Exec("UNITTEST")`; no PDB function |
   | `#pragma comment(linker, "/include:?GroupFactory_STATGROUP_TexturePool@@...")` (link anchor, not a stub) | Core `BestFitAllocator.cpp` | nothing in a PC game build references the allocator, so the linker drops the object and its `DECLARE_STATS_GROUP`; `FStatManager::Init` then asserts `check(Group)` (`UnStats.cpp:118`) for Texture2D.cpp's `STAT_TexturePool_PackMipTailSavings`. First runtime crash, found with Core's own stack walk in `Launch.log`; verified with a map-file cross-check of all 689 `StatFactory_*` against the 51 `GroupFactory_*` that linked (only this group was missing) |

   Of agent D's 55 CoreSmoke stubs, everything else is now owned by a real target: Launch
   (`GCreateMalloc`, `hInstance`, `GPackage`, `GGameIcon`, `GAlwaysReportCrash`,
   `MiniDumpFilenameW`, the output devices), IpDrv (`appSocketInit`,
   `UploadHardwareSurveyIfNecessary`, `GDebugChannel`), Engine (`GEngine`, `GDynamicRHI`,
   RHI/rendering-thread hooks, `FTextureAllocations`, `UObject::IsAPrefabArchetype`…), WinDrv
   (`GGameWindow*`, `UWindowsClient::StaticWndProc`). `TAccumulator`/`TCounter` link from Core now.

## Link-error categories

| Category | Count (first link) | Resolution |
|---|---:|---|
| `LNK1104 cannot open file 'libpng15.lib'` | 1 | `Engine/Src/UnPNG.cpp` has `#pragma comment(lib, "libpng15.lib")` for the reference's libpng 1.5 (ours is FetchContent 1.6.43) → `/NODEFAULTLIB:libpng15.lib` on the exe. **Engine change to report:** guard or drop that pragma |
| D3D9Drv entry points (RHI creation, PIX events, shader compiler, compat level) | 9 symbols, 70 references | stubs (null RHI) |
| D3D11Drv / OpenGLDrv entry points referenced unconditionally by Engine | 8 symbols | stubs |
| Excluded units (`AVIWriter.cpp`, `UnitTest.cpp`) | 6 symbols | stubs |
| Registrant hooks of unbuilt modules | 8 symbols | stubs |
| `nvtt.dll` not found at start (`0xC0000135`): `Engine/Src/UnTexCompress.cpp` imports the reference's VC8 `nvtt.dll` (needs `cudart.dll`, `MSVCR80`); retail ships no nvtt | 1 | `/DELAYLOAD:nvtt.dll` + `delayimp` on the exe (nothing before cooking calls nvtt); `stage_retail.py` still copies `nvtt.dll`/`cudart.dll` from the reference tree |
| Transient: agent L's Core API change (`UObject::FlushAsyncLoading(FName)` -> `()`, `ProcessAsyncLoading(..., FName)`, `UObject::EndLoad(const TCHAR*)`, `FAsyncIOSystemBase::LoadData/LoadCompressedData(..., EAsyncIORequestType)`) before the Engine call sites were updated; agent M's `Engine/Inc/*Classes.h`, `PrimitiveComponent.h`, `UnActorComponent.h`, `UnLevel.h`... regeneration (`UGameEngine::SecondaryViewportClients`, `m_LevelArkPpSettings`, `FArkSunGlareMeshParams`...: 800+ errors in ~250 Engine units at the time of writing) | 4-5 symbols / 858 compile errors | none of mine. L's part is committed (`316a576`, `181ca20`) and links. M's headers are still mid-edit, so **the exe was built from a snapshot**: `build\agentN\snap\src` = working tree with `Engine/Inc` reset to `HEAD` (`git show HEAD:...`), own `external/` FetchContent dir, `build\agentN\snap\build.cmd`. Once M lands, `cmake --build build\agentN --target DishonoredGame` (or `build_and_smoke.py` without `--build-dir`) is the normal path |

`build\agentN\runlink.py` runs the lib/exe steps straight from `build.ninja` (ignoring
dependency state) so link errors could be collected while other agents' Core/Engine units were
broken; `linkonly.cmd` wraps it in the VS x86 environment.

## Tools

- `resources/tools/stage_retail.py [--build-dir build\agentN] [--retail …] [--stage build\stage]`:
  `Binaries\Win32` = our exe/pdb/map + every retail DLL (binkw32, PhysX*, APEX_*, steam_api, …,
  `Microsoft.VC90.CRT` junction); `DishonoredGame\` is a real directory with `Config\` **copied**
  (read-only attribute dropped: the engine writes `DishonoredEngine.ini` & co. there), `Logs\` real,
  `CookedPCConsole`/`DLC`/`Localization`/`Movies` junctions (`mklink /J`) and the `PCConsoleTOC*.txt`
  copied; `Engine\` is a junction. Nothing the engine writes can reach the retail tree; rerunning
  recreates the junctions and refreshes `Config\`.
- `resources/tools/build_and_smoke.py [--no-build] [--timeout 120] [--extra-args …]`: build the
  target (VsDevCmd x86), stage, run `DishonoredGame.exe -log -nosteam -seekfreeloadingpcconsole`
  from `build\stage\Binaries\Win32` with a kill timeout, normalize `DishonoredGame\Logs\Launch.log`
  and the golden log (same rules as `normalize_log.py`), cut the golden at the milestone line and
  print the unified diff; outputs in `build\agentN\smoke\` (`Launch.norm.log`,
  `golden_prefix.norm.log`, `smoke_diff.txt`, build/run logs). Exit 0 iff the milestone line is in
  the log.

## Changes wanted in Core / Engine (not made, reported)

- `Engine/Src/UnPNG.cpp:26` `#pragma comment(lib, "libpng15.lib")`: should be under a
  `WITH_REFERENCE_LIBPNG`-style guard or removed (CMake links `Dishonored::libPNG`).
- `Engine/Src/NullRHI.cpp` defines `appBeginDrawEvent`/`appEndDrawEvent`/`appSetCounterValue` only
  under `USE_NULL_RHI`; with `USE_NULL_RHI=0` and no D3D9Drv target they have to be stubbed. Fine
  as is once D3D9Drv is a target.
- `Core/Src/UnitTest.cpp` is excluded but `UEngine::Exec` references `FUnitTestFramework`; either
  compile it (it is small) or guard the `UNITTEST` exec in Engine.
- `Core/Src/BestFitAllocator.cpp` owns `STATGROUP_TexturePool` but is only linked when something
  references the allocator; either move the `DECLARE_STATS_GROUP` next to its users (Texture2D.cpp)
  or keep the `/include` anchor in Launch. The same static-library trap applies to every
  `IMPLEMENT_CLASS` / `AutoInitializeRegistrants*` object nothing references (the registrants are
  fine today because each module's `<Module>.cpp` is referenced by Launch); worth a map-file check
  once script packages load (milestone 2).
- `Engine/Src/UnTexCompress.cpp`: nvtt should not be an import dependency of the game exe (retail has
  none); delay-loaded from Launch for now.
- `Engine/Src/SystemSettings.cpp:532`: `FSystemSettings::LoadFromIni` asserts on settings missing
  from the retail `DishonoredSystemSettings.ini` (`StaticDecals` first). Next blocker after
  milestone 1 (see "Run").

## Run

`python resources/tools/build_and_smoke.py --no-build --build-dir build\agentN\snap\build --timeout 120`
(exe from the snapshot build, staged into `build\stage`, run with
`-log -nosteam -seekfreeloadingpcconsole -unattended`; outputs in `build\agentN\snap\build\smoke\`).
Exit code of the game: 3 (appError, see below). Normalized diff of the golden prefix (2012
ArkProfile, cut at the milestone line) against our `Launch.log` prefix:

```
--- golden(prefix)
+++ Launch.log
@@ -1,11 +1,23 @@
 Log: Log file open, <date> <time>
 Init: WinSock: version 1.1 (2.2), MaxSocks=32767, MaxUdp=65467
-Log: Deleting old log file Launch-backup-<stamp>.log
+DevConfig: GConfig::LoadFile associated file:  ..\..\DishonoredGame\Config\DishonoredSystemSettings.ini
+DevConfig: GConfig::LoadFile has loaded file:  ..\..\DishonoredGame\Config\DishonoredLightmass.ini
+DevConfig: GConfig::LoadFile has loaded file:  ..\..\DishonoredGame\Config\DishonoredEngine.ini
+DevConfig: GConfig::LoadFile has loaded file:  ..\..\DishonoredGame\Config\DishonoredGame.ini
+DevConfig: GConfig::LoadFile has loaded file:  ..\..\DishonoredGame\Config\DishonoredInput.ini
+DevConfig: GConfig::LoadFile has loaded file:  ..\..\DishonoredGame\Config\DishonoredUI.ini
+Init: Version: 9411
+Init: Epic Internal: 0
+Init: Compiled (32-bit): Sep 25 2026 <time>
+Init: Changelist: 334700
+Init: Command line: -log -nosteam -seekfreeloadingpcconsole -unattended
+Init: Base directory: <path>
 Init: Computer: <redacted>
 Init: User: <redacted>
 Init: CPU Page size=4096, Processors=16
 Init: High frequency timer resolution =10.000000 MHz
-Init: Memory total: Physical=<n> GB Pagefile=<n> GB Virtual=<n> GB
-Log: Steam Client API Disabled!
+Init: Memory total: Physical=<n> GB (<n> GB approx) Pagefile=<n> GB Virtual=<n> GB
+DevStats: GSecondsPerCycle 1.000000e-07
+Init: WinSock: I am DESKTOP-46V9CBT (172.26.80.1:0)
 Init: Presizing for 58555 objects not considered by GC, pre-allocating 0 bytes.
 Init: Object subsystem initialized
```

Differences, all expected:

| Line | Why |
|---|---|
| `Deleting old log file ...` | golden had a stale `Launch-backup-*.log`; ours runs in a fresh `Logs\` |
| `DevConfig: ...`, `DevStats: ...` | Dev* categories; the golden build suppresses them (`[Core.System] Suppress=` only applies after the inis are loaded, and the retail 2013 code logs less at this point) |
| `Init: Version / Epic Internal / Compiled / Changelist / Command line / Base directory` | reference `appInit` lines; absent from the 2012 ArkProfile log (Arkane build differences / `FINAL_RELEASE` trimming); same for `WinSock: I am ...` from IpDrv's `appSocketInit(FALSE)` |
| `Memory total: ... (189GB approx)` | reference 10897 format; the 2012 exe prints the older format without the `approx` field |
| `Steam Client API Disabled!` | Arkane's `-nosteam` handling in OnlineSubsystemSteamworks (not built, stubbed) |
| `Presizing for 58555 objects ...` | identical: `[Core.System] MaxObjectsNotConsideredByGC` from the retail `DefaultEngine.ini` |

After the milestone the log shows 18 `DevSave: Loading value for ... from [Core.System]` lines, the
`DevDataBase: Connection to "provider=sqloledb;Data Source=SQL01.ARKANE-STUDIOS.LAN..." failed` line
**identical to the golden log's line 12** (`FTaskPerfMemDatabase`, `LaunchEngineLoop.cpp` `PreInit`),
and then the first real blocker:

```
Critical: appError called: Assertion failed: SystemSettings[SettingIndex].bFound
  [File: Engine/Src/SystemSettings.cpp] [Line: 532]
Couldn't find system setting StaticDecals in Ini section SystemSettings in Ini file
  ..\..\DishonoredGame\Config\DishonoredSystemSettings.ini!
Stack: FSystemSettings::LoadFromIni() <- FSystemSettings::Initialize() <- FEngineLoop::PreInit()
  (LaunchEngineLoop.cpp:2328) <- EnginePreInit() <- GuardedMain() <- WinMain()
```

`FSystemSettings` in the reference `SystemSettings.cpp` requires every entry of its table
(`StaticDecals` & co.) to exist in the ini; the retail 2013 `BaseSystemSettings.ini` /
`DefaultSystemSettings.ini` do not carry Epic's full set. That is milestone-2 territory (Engine
convergence with the 2013 settings table, or making missing settings non-fatal as the retail exe
evidently does) and is reported for the Engine owner below; Launch does not touch it.

The tools ran the stub exe end to end as well (`--build-dir build\x86-debug`) before the real
one existed, so `stage_retail.py` / `build_and_smoke.py` work with any `DishonoredGame.exe`.
