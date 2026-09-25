# Agent P — D3D9Drv as a module + build follow-ups (2026-09-25)

Build dir `build\agentP` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`). Logs: `build\agentP\build_*.log`,
`probe_build*.log`, smoke logs in `build\agentP\smoke\` (`Launch_null.log`, `Launch_d3d9.log`). IDA copies
`resources/docs/idb/shipping2012_agentP.i64`, `retail2013_agentP.i64`; decompiles in `build\agentP\decomp\`.
Numbers are from the **2012 Shipping PDB** unless they say retail 2013.

## Result

- **`dishonored_module(D3D9Drv)`** builds (20 compile units, 4 mechanical fixes) behind `DISHONORED_ENABLE_D3D9DRV`
  (ON) and links `Dishonored::DirectX9 + Dishonored::nvapi`. **`DishonoredGame.exe` links `D3D9Drv.lib`**: the map file
  resolves `D3D9CreateRHI` (`D3D9Drv:D3D9Device.cpp.obj`), `appBeginDrawEvent` (`D3D9Util.cpp.obj`),
  `D3D9BeginCompileShader` (`D3D9ShaderCompiler.cpp.obj`), `GetCompatibilityLevelWindows`
  (`CompatibilityEvaluator.cpp.obj`), `SetDefaultResolutionForDevice` (`D3D9HardwareSurvey.cpp.obj`).
- `build_and_smoke.py --rhi null`: milestone 1 reached, golden diff unchanged from agent N's.
- **`--rhi d3d9`: `RHIInit` reached, the D3D9 RHI is created** (`Direct3DCreate9`, `D3DXCheckVersion`, caps, adapter
  display mode, float-RT checks) and the run continues past it, no crash or assert in D3D9Drv:
  ```
  [0007.16] Warning: Warning, Command line -d3d11 set, but D3D11 is not supported on this machine.  Will fallback to older API.
  [0010.27] Log: Shader platform (RHI): PC-D3D-SM3
  ```
  (`DynamicRHI.cpp` hardcodes `bForceD3D11 = TRUE`; `IsDirect3D11Supported` is the D3D11Drv stub, so the warning is
  expected and the fallback picks `D3D9CreateRHI`.) The IDirect3DDevice9 itself is created on the first viewport
  (`UpdateD3DDeviceFromViewports`), i.e. after `GEngine->Init` — beyond milestone 2. The D3D9 path takes ~3 s
  between the warning and the platform line (nvapi stereo updater + debug-runtime device enumeration); the null RHI
  takes 0.1 s.
- Both RHIs then stop at the same **next blocker, not D3D9-related** (below).
- Layout probe for D3D9Drv: 14 PDB types probed, **9 exact** after two cheap fixes, 5 pending;
  `D3D9Drv/Inc/DishonoredLayouts.h` generated (8 asserts) and included at the end of `D3D9Drv.h`.
- Follow-ups from agentN.md done: libpng pragma guarded, nvtt out of the game exe (no `nvtt`/`delayimp` in the map),
  `USE_UNIT_TESTS=0`, `STATGROUP_TexturePool` anchor confirmed on the map.

## Snapshot build (why `build\agentP` is configured from `build\agentP\snap\src`)

The working tree's `Engine/Inc/*Classes.h` are mid-edit by Q/R/S (first D3D9Drv build: 366 × `AKActorFromStatic`
static_assert, 95 × `FDistractionLoopOverride` undeclared from `EngineSequenceClasses.h:2098`), so, like agent N,
the exe is built from a snapshot: `build\agentP\sync_snapshot.py` extracts `git archive HEAD` (source, cmake,
CMakeLists.txt) once and copies agent P's own files over it on every build (`OWNED` list: root `CMakeLists.txt`,
`cmake/`, `D3D9Drv/`, `Launch/CMakeLists.txt`, `Launch/Src/DishonoredStubs.cpp`, `Engine/Src/{UnPNG,NullRHI,
UnTexCompress,UnLightMap,UnShadowMap}.cpp`, `Core|Engine/Sources.cmake`). `build\agentP_configure.cmd` /
`agentP_build.cmd <target> [log]` wrap VsDevCmd + sync + cmake (`-DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/
UnrealEngine3`, the reference dir is relative to the source dir otherwise). `build/agentP` is otherwise a normal
build dir (`build_and_smoke.py --build-dir build/agentP`, map at `build/agentP/Binaries/Win32/DishonoredGame.map`).
The snapshot has its own `resources/tools/symbols` + `resources/docs/types` copies so `gen_layout_probe.py generate`
runs against the snapshot's Engine headers (the working-tree probe units reference types HEAD does not declare).

**Snapshot-only measurement bridges** (never in the working tree; `sync_snapshot.py bridge_*`), needed because both
asserts fire in `PreInit` *before* `RHIInit` (`LaunchEngineLoop.cpp:2325/2382` vs `:2371`):

| Bridge | Owner | Retail evidence |
|---|---|---|
| `SystemSettings.cpp:531` `checkf(bFound)` → `warnf` tagged `DISHONORED(bringup)` | O step 2 | PHASE4.md: retail deleted the checkf |
| `SystemSettings.cpp:716` `verify(GetInt MinTextureResidentMipCount)` → default 7 | O step 3 | PHASE4.md: retail `GetInt` with default `GMinTextureResidentMipCount = 7` |
| `ShaderCompiler.cpp:450-483` ten `verify(GConfig->Get*("DevOptions.Shaders", …))` in `FShaderCompilingThreadManager()` → `(void)` (ctor defaults kept) | O / Engine ini | retail `DishonoredEngine.ini` has no `[DevOptions.Shaders]`; the ctor runs for both RHIs (`LaunchEngineLoop.cpp:2382`), so this is O's next ini blocker after steps 2/3 |
| `Launch/CMakeLists.txt`: drop `DishonoredGame` from the module foreach, `DISHONORED_HAVE_DISHONOREDGAME=0` | T | T's in-flight edit lists the `DishonoredGame` *module* next to the *exe* of the same name: `if(TARGET DishonoredGame)` is always true → CMake "Target DishonoredGame of type EXECUTABLE may not be linked into another target", and the define would be 1 without the module. **T: rename the module target** (e.g. `DishonoredGameModule` with `OUTPUT_NAME DishonoredGame`) |

## What changed (every reference edit carries `// DISHONORED(build|layout)`)

### CMake
| File | Change |
|---|---|
| `CMakeLists.txt` | `option(DISHONORED_ENABLE_D3D9DRV ON)` + `dishonored_module(D3D9Drv)` linking `Dishonored::DirectX9 Dishonored::nvapi`; Engine no longer links `Dishonored::nvtt` (editor-only now; the target stays declared in `ReferenceExternals.cmake`) |
| `cmake/DishonoredDefines.cmake` | `USE_UNIT_TESTS=0` (no `FUnitTestFramework` function in the 2012 PDB nor in `functions_2013.csv`; `UnBuild.h` would default it to `!FINAL_RELEASE && !SHIPPING_PC_GAME`), `WITH_REFERENCE_LIBPNG=0` (guard for `UnPNG.cpp`) |
| `Launch/CMakeLists.txt` | `D3D9Drv` in the module list, `DISHONORED_HAVE_D3D9DRV=$<TARGET_EXISTS:D3D9Drv>`; removed `/NODEFAULTLIB:libpng15.lib`, `/DELAYLOAD:nvtt.dll` and `delayimp` |
| `resources/tools/symbols/gen_layout_probe.py` | `MODULE_INCLUDES["D3D9Drv"] = [Engine.h, D3D9Drv.h, D3D9HardwareSurvey.h, HardwareID.h, VideoDevice.h, BaseDevice.h]` (D3D9DrvPrivate.h order; the survey/compat types live in headers `D3D9Drv.h` does not reach) |

### D3D9Drv compile fixes (4, all mechanical)
| File | Error | Fix |
|---|---|---|
| `Inc/D3D9Drv.h:32` | `checkAtCompileTime(D3DX_SDK_VERSION == 43)`: the reference `External/DirectX9` is `D3DX_SDK_VERSION 39` (`d3dx9core.h`; `dxsdkver.h` 9.24.1400 = August 2008 SDK) | `REQUIRED_D3DX_SDK_VERSION = 39`. Retail 2013 imports `d3d9` only (`imports_2013.csv`: 4 functions, no `d3dx9_*.dll`), so no D3DX version is retail truth; our exe imports `d3dx9_39.dll` (present in SysWOW64 here) for `D3DXCheckVersion` / the shader compiler / `D3D9MeshUtils` |
| `Src/D3D9Commands.cpp:11` | `#include <xnamath.h>` (February 2010+ SDK) | `<DirectXMath.h>` + `using namespace DirectX` (same `XMFLOAT4A`/`XMLoadFloat4A`/`XMVectorGet*Ptr` API); `SetVertexShaderFloatArray` compiles unchanged |
| `Src/D3D9RenderTarget.cpp:28` | C2445 `FD3D9Texture2D*` vs `TRefCountPtr<FD3D9Texture2D>` in `?:` | `.GetReference()` on the second operand |
| `Src/D3D9MeshUtils.cpp:699,892` | C2668 `CheckVA(HRESULT)` ambiguous (`warnf("%u", Result)`) | `(DWORD)Result` |

### Launch
`DishonoredStubs.cpp`: the D3D9 block (`D3D9CreateRHI` → null RHI, PIX markers, `D3D9BeginCompileShader`,
compatibility level, `SetDefaultResolutionForDevice`) is now `#if !DISHONORED_HAVE_D3D9DRV` so
`DISHONORED_ENABLE_D3D9DRV=OFF` still links; D3D11 / OpenGL stubs untouched.

### Layout (2012 PDB; D3D9Drv has no reflected classes, so no retail SDK rows and no `native_class_sizes.csv` rows)
`build\agentP\compare_d3d9.py` (probe vs `sizes.csv`/`types.json`), probe `build\agentP\layout_probe.txt`:

| Type | ours | 2012 | Members exact | Note |
|---|---:|---:|---|---|
| FD3D9BufferedGPUTiming | 48 | 48 | 7/7 | |
| FD3D9DepthState | 20 | 20 | 3/3 | |
| FD3D9EventQuery | 28 | 28 | 2/2 | |
| FD3D9HardwareSurveyData | 1096 | 1096 | 16/16 | not reachable from `D3D9Drv.h`, not asserted |
| FD3D9OcclusionQuery | 20 | 20 | 2/2 | |
| FD3D9StencilState | 60 | 60 | 13/13 | |
| FD3D9Surface | 28 | 28 | 4/4 | |
| FD3D9VertexDeclarationCache | 64 | 64 | 2/2 | |
| FD3D9Viewport | 32 (was 28) | 32 | 6/6 | **fixed**: `bWantsVSync` @28 added (`// DISHONORED(layout)`, Arkane vsync flag, unused until the 2013 vsync path is ported) |
| FD3D9BlendState | 56 | 52 | 11/11 | pending: reference-only trailing `BlendFactor` (used by `D3D9State.cpp`/`D3D9Commands.cpp`); a `DISHONORED_SHIM_STATIC` would share one value across states, so left as is |
| FD3D9RasterizerState | 28 | 24 | 4/4 | pending: reference-only trailing `bAllowMSAA` (same reasoning) |
| FD3D9SamplerState | 40 | 36 | 7/7 | pending: reference-only trailing `BorderColor` |
| FD3D9BoundShaderState | 64 | 52 | 1/4 | pending, **Engine-owned**: `FCachedBoundShaderStateLink` (`BoundShaderStateCache.h`) is 44 here vs 32 in 2012; the three `TRefCountPtr` members follow it (+12) |
| FD3D9DynamicRHI | 4984 (was 4980) | 6968 | 0/33 | `bIsVSyncDevice` @2128 added (**fixed the member set**, 0 missing); offsets still differ because 2012 embeds **`ArkIDirect3DDevice9 Direct3DDevice` (2080 bytes, vtable 300 = Arkane's state-caching device wrapper with `StreamSourceCache`/`SamplerStatesCache`, `D3D9Drv.h:801` in the 2012 tree)** instead of `TRefCountPtr<IDirect3DDevice9>`, has none of the reference perf-event members (`CurrentEventNodeFrame`, `CurrentEventNode`, `bTrackingEvents`, `bLatchedGProfilingGPU`, `bOriginalGEmitDrawEvents`, `AdapterIndex`, `DeviceType`: 28 bytes; `FD3D9EventNode*`/`FD3D9DisjointTimeStampQuery` do not exist in the PDB) and `FD3D9DynamicRHI::FD3D9Stream` is 12 (no `Offset`; ours 16 × 16 streams = +64). Porting `ArkIDirect3DDevice9` is a real port (retail `VideoDeviceEtAl` unity), not layout work |

Not probed: `FD3D9DisjointTimeStampQuery`, `FD3D9EventNode`, `FD3D9EventNodeFrame`, `FD3D9EventNodeStats`,
`FD3D9Stream`, `FKey` (no PDB UDT of that name). Arkane's `BaseDevice` (3100), `VideoDevice` (4536),
`HardwareID` (288), `ArkIDirect3DDevice9` (2080) are outside the `[UAF]`-name convention of the tools.

`resources/docs/function_status.csv`: 269 `d3d9drv` rows appended, status `reference` (module compiled from the
reference; none converged against the decompile).

### Build follow-ups (agentN.md "Changes wanted in Core / Engine")
| Item | Done |
|---|---|
| `Engine/Src/UnPNG.cpp:26` `#pragma comment(lib, "libpng15.lib")` | under `#if WITH_REFERENCE_LIBPNG` (=0 in `DishonoredDefines.cmake`); `/NODEFAULTLIB:libpng15.lib` dropped from Launch |
| `Engine/Src/NullRHI.cpp` PIX stubs | unchanged: `D3D9Util.cpp` provides `appBeginDrawEvent`/`appEndDrawEvent`/`appSetCounterValue` when D3D9Drv is linked; with the option OFF the guarded block in `DishonoredStubs.cpp` provides them |
| `Core/Src/UnitTest.cpp` vs `UEngine::Exec` | `USE_UNIT_TESTS=0` (matches both PDBs); the `FUnitTestFramework` stub block in `DishonoredStubs.cpp` is now dead and can go (not removed: O owns the rest of that file) |
| nvtt delay-load | removed. **nvtt users guarded with `WITH_EDITOR`**: every `#if _MSC_VER && !CONSOLE && !UE3_LEAN_AND_MEAN && !DEDICATED_SERVER` in **`Engine/Src/UnTexCompress.cpp`** (8: the `nvtt/nvtt.h` include, `NVTTCompress`/`DXTCompress`/`FAsyncDXTCompress`/`FAsyncPVRTCCompressor`, mip generation, `UTexture2D::Compress`/`ResizeTexture` bodies), **`UnLightMap.cpp`** (6: `FLightMapPendingTexture`, `AllocateLightMap`/`EncodeTextures` bodies) and **`UnShadowMap.cpp`** (3: `FShadowMapPendingTexture`, `UShadowMap2D` ctor body, `EncodeTextures` with the `NVTTCompress` calls) became `WITH_EDITOR && …`. Evidence (`build\agentP\decomp`): 2012 `UTexture2D::Compress` is 59 bytes = only the post-block tail (`CALLBACK_RefreshContentBrowser` + `GenerateTextureFileCacheGUID`), `FLightMap2D::AllocateLightMap` / `FInstancedLightMap2D::AllocateLightMap` 18 bytes (empty), `FinalizeEncoding` 35, `UShadowMap2D::UShadowMap2D` 126 (member init only); the PDB has no `FLightMapPendingTexture`/`FShadowMapPendingTexture`/`FAsyncDXTCompress`/`NVTTCompress` function; retail 2013 imports no `nvtt.dll`. Engine no longer links `Dishonored::nvtt`; the map has no nvtt symbol |
| `STATGROUP_TexturePool` anchor | checked on `build/agentP/Binaries/Win32/DishonoredGame.map`: `GroupFactory_STATGROUP_TexturePool` comes from `Core:BestFitAllocator.cpp.obj` only through the `/include` pragma (nothing in D3D9Drv references the allocator: the texture pool is console-only), so the anchor stays |

## Link-error categories resolved
| Category (agentN.md) | Before | Now |
|---|---:|---|
| D3D9Drv entry points (RHI creation, PIX events, shader compiler, compat level, survey) | 9 symbols stubbed | real, from `D3D9Drv.lib` |
| `libpng15.lib` LNK1104 | `/NODEFAULTLIB` | pragma guarded, no link option |
| `nvtt.dll` at start (`0xC0000135`) | `/DELAYLOAD` | not imported |
| `UnitTest.cpp` (`FUnitTestFramework`) | 8 stubs | no reference (`USE_UNIT_TESTS=0`) |
| transient: nvtt users after the guard (`FAsyncDXTCompress::*`, `NVTTCompress`, `GUseCUDAAcceleration`, `FAsyncPVRTCCompressor::*` from `UnLightMap.cpp`/`UnShadowMap.cpp`) | 12 unresolved | guarded in those two files too |

## Next blocker (both RHIs, not D3D9)

With the three ini bridges the run gets past `RHIInit` and stops in `AutoInitializeRegistrantsEngine`:
```
Assertion failed: (InOffset & ~0xFFFFF) == 0 [Core/Inc/UnObjGC.h] [Line: 47]
FGCReferenceInfo::FGCReferenceInfo() <- UClass::EmitObjectArrayReference() (UnObjGC.cpp:1533)
<- ULevel::StaticConstructor() (UnLevel.cpp:266) <- UClass::GetDefaultObject() <- UClass::Register()
<- ULevel::InitializePrivateStaticClassULevel() <- AutoInitializeRegistrantsEngine() (UnEngine.cpp:256)
```
A `ULevel` member offset above 1 MB reaches the GC token stream (`UnLevel.cpp:266` emits an object-array
reference): a `ULevel` layout problem in HEAD's `UnLevel.h` (S owns `UnLevel.h`). Identical with `-nullrhi`, so it is
the first thing O's smoke will hit after steps 2/3 and the `[DevOptions.Shaders]` verifies.

## Follow-ups outside my files
- **T**: `Launch/CMakeLists.txt` / root `CMakeLists.txt` module named `DishonoredGame` clashes with the exe (see bridge table).
- **O**: `ShaderCompiler.cpp:450-483` verifies on `[DevOptions.Shaders]` (10 keys) are the next ini blocker after
  `SystemSettings`; the `FUnitTestFramework` stub block in `DishonoredStubs.cpp` can be deleted.
- **S**: `ULevel` offset assert above; `FCachedBoundShaderStateLink` 44 vs 32 (2012) in `BoundShaderStateCache.h`
  (blocks `FD3D9BoundShaderState` 64 vs 52).
- `resources/tools/stage_retail.py` still copies `nvtt.dll`/`cudart.dll` from the reference tree (harmless, no longer needed).
- `resources/docs/types/reference_layout_delta.md` was not regenerated (`compare` would rewrite the Engine section from
  the snapshot's HEAD headers); D3D9Drv numbers are in `build\agentP\compare_d3d9.py` output above.
- The working-tree `source/Tests/LayoutProbe` was regenerated with `generate Core Engine D3D9Drv`
  (`probe_D3D9Drv.cpp` + `CMakeLists.txt` entry); `generate` deletes units for modules not listed, so the coordinator
  should always pass all three.
- D3DX: retail has no D3DX import at all, so `D3D9ShaderCompiler.cpp`, `D3D9MeshUtils.cpp` (`!UE3_LEAN_AND_MEAN`,
  no PDB functions) and the `D3DXCheckVersion` call in `D3D9Device.cpp:136` are editor/cook-time in Arkane's tree;
  a later pass can guard them like the nvtt users and drop `d3dx9.lib`.
- `Sources.cmake` `D3D9Drv_NOT_IN_PDB` (`D3D9Drv.cpp`, `D3D9MeshUtils.cpp`, `D3D9ShaderCompiler.cpp`) is consistent
  with that: retail compiled neither the D3DX mesh utils nor the shader compiler.
