# Agent D — P2.12 CoreSmoke (2026-09-25)

Console program that links `Core.lib` and exercises it: `source/Tests/CoreSmoke/` (`CMakeLists.txt`,
`CoreSmoke.cpp`, `stubs.cpp`). Built in `build\agentD` (Ninja, Debug, `cmake\toolchain-x86.cmake`);
logs in `build\agentD\smoke_build*.log`, run output in `build\agentD\smoke_run*.log`.
Run from the repo root (`build\agentD\Binaries\Win32\CoreSmoke.exe`); it reads
`resources/docs/symbols/hardcoded_names.csv` relative to the cwd. Exit code = number of failures
(100 = Core called `appError`/a `check()` failed; the message and Core's own stack walk are printed).

## Stubs (`stubs.cpp`)

Core.lib has 55 unresolved externals when linked alone. Each stub carries a one-line comment; the
groups below follow the reference module that owns the real definition.

| Symbol(s) | Reference owner | Why Core needs it |
|---|---|---|
| `GCreateMalloc()` | `Launch/Src/LaunchEngineLoop.cpp` | `UnAnsi.cpp appMalloc` calls it on the first allocation. Stub: `FMallocAnsi` wrapped in `FMallocThreadSafeProxy` (the reference picks `FMallocDebug`/`FMallocBinned`). |
| `extern "C" HINSTANCE hInstance`, `extern "C" TCHAR GPackage[64]` | `LaunchEngineLoop.cpp` | `UnVcWin32.cpp` window class registration; `UnOutputDevices.cpp`/`UnVcWin32.cpp` log header and crash report package name. |
| `INT GGameIcon` | `LaunchEngineLoop.cpp` | `appShowGameWindow` window icon. |
| `UBOOL GAlwaysReportCrash` | `Launch.cpp` | crash handling in `UnVcWin32.cpp`. |
| `TCHAR MiniDumpFilenameW[1024]` | `LaunchMisc.cpp` | minidump target in `UnMisc.cpp`/`UnVcWin32.cpp`. |
| `GError`, `GWarn`, a stdout device on `GLog` (`InstallSmokeOutputDevices`) | `LaunchEngineLoop.cpp` statics `Error`/`GameWarn`/`Log` | `Core.cpp` leaves `GError`/`GWarn` NULL; `appFailAssertFunc` does `GError->Logf(...)`, so any failed `check()` is an access violation until the app installs a device. |
| `appSocketInit(UBOOL)`, `UploadHardwareSurveyIfNecessary()` | `IpDrv/Src/UnSocketWin.cpp`, `HardwareSurvey.cpp` | `UnMisc.cpp appInit`. |
| `FDebugServer* GDebugChannel`, `FDebugServer::SendText` | `IpDrv/Src/IpDrv.cpp`, `FDebugServer.cpp` | `UnVcWin32.cpp appOutputDebugString` mirrors to the remote debug channel. Class declared minimally in the stub file (Core.h does not reach `FDebugServer.h`). |
| `UEngine* GEngine`, `FDynamicRHI* GDynamicRHI`, `UBOOL GAllowFullRHIReset` | `Engine/Src/UnEngine.cpp`, `DynamicRHI.cpp`, `RHI.cpp` | `UnMisc.cpp`, `Database.cpp`, `UnStatsNotifyProviders.cpp`, `UnVcWin32.cpp`. |
| `EShaderPlatform GRHIShaderPlatform`, `ShaderPlatformToText(...)` | `Engine/Src/ShaderManager.cpp`, `ShaderCompiler.cpp` | `appPlatformPostInit` logs the RHI name. Enum declared minimally (`W4EShaderPlatform` mangling only needs the name). |
| `IsInRenderingThread()`, `FlushRenderingCommands()`, `FlushDeferredDeletion()` | `Engine/Src/RenderingThread.cpp` | Core threading/async loading. `IsInRenderingThread` returns TRUE (no rendering thread exists). |
| `FName ENGINE_GetPlayerViewPoint` | Engine's generated names | `EngineControllerClasses.h eventGetPlayerViewPoint` is inlined into the Core units that include `Engine.h` (`UnMisc.cpp`, `UnVcWin32.cpp`, `UnStatsNotifyProviders.cpp`). |
| `GetMapNameStatic()` | `Engine/Src/UnWorld.cpp` | crash/log context in `UnMisc.cpp`. |
| `UObject::IsAPrefabArchetype`, `UObject::IsInPrefabInstance` | `Engine/Src/UnPrefab.cpp` | UObject virtuals declared in `UnObjBas.h`, needed by every Core unit with a UObject vtable. |
| `operator<<(FArchive&, FTextureAllocations&)`, `operator<<(FArchive&, FTextureAllocations::FTextureType&)`, `FTextureType::FTextureType()`, `FTextureAllocations::CancelRemainingAllocations`, `ULinkerLoad::StartTextureAllocation` | `Engine/Src/Texture2D.cpp` | `FPackageFileSummary` serialization (`UnLinker.cpp`) and `ULinkerLoad::VerifyImportInner`. The stub keeps the loading path of the reference (`Ar << TextureTypes`). |
| `TAccumulator<FLOAT/DWORD>::TAccumulator(const TCHAR*,DWORD,DWORD)`, `TCounter<FLOAT/DWORD>::TCounter(...)` | `Core/Inc/UnStats.h` itself | **Core defect** (see below); the deferred definitions are copied verbatim and explicitly instantiated. |
| `GGameWindow`, `GGameWindowUsingStartupWindowProc`, `GGameWindowStyle`, `GGameWindowPosX/PosY/Width/Height`, `GPrimaryMonitorWidth/Height`, `GPrimaryMonitorWorkRect`, `GVirtualScreenRect` | `WinDrv/Src/WinViewport.cpp` | `UnVcWin32.cpp appShowGameWindow`/`appPlatformInit`. |
| `UWindowsClient::StaticWndProc` | `WinDrv/Src/WinClient.cpp` | `appShowGameWindow` swaps the startup window procedure. Class declared minimally. |
| `GetCompatibilityLevelWindows()`, `SetCompatibilityLevelWindows(...)` | `D3D9Drv/Src/CompatibilityEvaluator.cpp` | `appGetCompatibilityLevel`/`appSetCompatibilityLevel`. |
| `dbghelp.lib`, `winmm.lib` (CMake, not a stub) | `UE3BuildWin32.cs` adds `dbghelp.lib` to every Win32 link | `Sym*`/`StackWalk64` in the Core stack walker, `timeBeginPeriod` in `appPlatformInit`. |

Not stubbed on purpose: `FName::StaticInit` is *not* called by the test unconditionally. Core's
`FName` constructors initialize the table on first use (`UnName.cpp` 527/652) and the reference
never calls `StaticInit` explicitly (`appInit` only calls `UObject::StaticInit`); a static FName in
Core.lib or the stubs has already initialized it before `main`, and `StaticInit` asserts against a
second call. The test checks `FName::GetInitialized()` and only calls `StaticInit` when needed.

## Core defects found

1. **`UnStats.h`: `TAccumulator`/`TCounter` 3-argument constructors declared but never defined under
   MSVC.** The P2.4 fix switched the in-class definitions (lines 499–514, 605–618) to the deferred
   "gcc path" declarations, but the deferred definitions at line 2233 are still guarded by
   `#if __GNUC__ || NGP`. Any exe that instantiates a stat (all of Core does) fails to link with
   `?? 0?$TAccumulator@M@@AAE@PB_WKK@Z` etc. Fix: change the guard at `UnStats.h:2233` to match the
   declaration side (e.g. `#if 1 // DISHONORED: MSVC 2022 two-phase lookup`), then drop the copy in
   `stubs.cpp`.
2. **`Core.lib` does not carry its system libraries.** `UnVcWin32.cpp` pulls `dbghelp` (stack
   walking) and `winmm` (`timeBeginPeriod`) but only `Crypt32`/`Iphlpapi` have `#pragma comment(lib)`.
   Suggest `target_link_libraries(Core PUBLIC dbghelp winmm)` in the root `CMakeLists.txt` (or
   `Core/Inc/UnVcWin32.h` pragmas) so consumers stop repeating it.
3. **`Core/Inc/DishonoredLayouts.h` is never included.** No Core header or source includes it, so its
   `static_assert`s (e.g. `sizeof(FArchive) == 136`, `sizeof(FMemoryReader) == 144`) never run; both
   would fail today (compiled 132/140, PDB 136/144, the missing `FArchive::ArIsDisSaveLoad`
   from `reference_layout_delta.md`). Include it from `Core.h` (end) once P2.5 lands so the checks bite.
4. **Stale 1999 headers still in `Core/Inc`:** `FOutputDeviceAnsiError.h` and `FOutputDeviceStdout.h`
   define their classes fully inline (`printf("%s")` with `TCHAR`), while `UnOutputDevices.cpp`
   defines `FOutputDeviceAnsiError::*` out of line; nothing includes either header. Same category as
   the deleted `Src/CorePrivate.h`; delete or replace them before Launch uses `FOutputDeviceAnsiError`.
5. **Layout sizes measured by the exe (Debug, same defines as Core):** `sizeof(UObject) == 72`
   (PDB 56; `reference_layout_delta.md` reported 60 from an earlier header state), `FArchive == 132`
   (PDB 136), `FMemoryReader == 140` (PDB 144). `FName == 8`, `FString == 12`,
   `FPackageFileSummary == 164` match. These are the P2.5 items; the test keeps them as FAILs so the
   run turns green when the headers converge.
6. **Concurrent edits (informational):** while this test was being linked, `UnObjBas.h`/`UnClass.h`
   were changed in the working tree (`NetIndex` and `UPackage::FileName` removed, `ClassGroupNames`
   renamed) without the matching `.cpp` updates, so `Core.lib` stopped compiling (`UnObj.cpp`,
   `UnCoreNet.cpp`, `UnClass.cpp`). The results below are from the last state in which Core compiled.

## Test results

(Filled in from `build\agentD\smoke_run_final.log`; see the end of this file.)
