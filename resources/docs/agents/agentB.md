# Agent B — Engine module compiles with MSVC 2022 (Phase 3 start)

Build dir: `build\agentB` (Ninja, Debug, x86, `-DDISHONORED_ENABLE_ENGINE=ON`). Logs and the
analysis script (`analyse.py`: errors grouped by code / message / file) live in `build\agentB\logs`.
Policy followed: mechanical fixes only, `// DISHONORED: <why>` on every source edit, no behavior
change; missing SDKs are switched off from `cmake/DishonoredDefines.cmake`, not in `UnBuild.h`.

## Result

**Every Engine compile unit builds with 0 errors** (499 units: 513 reference `Src/**/*.cpp`
minus 17 excluded, plus the 3 `Debugger/` units the reference `Engine.vcxproj` compiles).
Verified per object with `ninja -k 0 <all Engine objects>` after each pass.

Final `cmake --build build\agentB --target Engine` (pass 4, `DISHONORED_LAYOUT_CHECKS=OFF`):
**exit 2, 11 failed units, none of them mine** — 8 Core units and 3 Engine units
(`UnPenLev.cpp`, `UnConn.cpp`, `UnWorld.cpp`: `FPackageInfo::FileName` no longer exists after agent
A's `UnCoreNet.h` edit; they compiled in pass 3 before that edit). The `Engine.lib` archive step is
therefore blocked only by **agent A's uncommitted Core layout edits** in the shared tree (`UnObjBas.h` removed `UObject::NetIndex`, `UnClass.h` `ClassGroupNames`,
`bspatch/bzlib.h` rewrite, `DishonoredLayouts.h` static_asserts incl. a `/Zp4` requirement): those
fail `Core/Src/UnObj.cpp`, `UnCoreNet.cpp`, `UnClass.cpp`, `bspatch/*.cpp` and, with
`DISHONORED_LAYOUT_CHECKS=ON`, every unit. Engine was therefore verified with
`-DDISHONORED_LAYOUT_CHECKS=OFF` in my build dir. Heads-up for A: 7 Engine files still use
`NetIndex` (`Src/UnLevel.cpp`, `UnChan.cpp`, `UnEngine.cpp`, `UnScript.cpp`, `UnInterpolation.cpp`,
`UnParticleComponents.cpp`, `ParticleEmitterInstances.cpp`) and will need the same treatment once
the Core change lands.

## Error categories (first build: 711 errors in 26 files, 84 failed units)

| Category | Count | Fix |
|---|---:|---|
| Stale reference units not in `Engine.vcxproj` (`UnMaterial.cpp`, `UnScene.cpp`, `UnTex.cpp`, `UnStats.cpp`, `UnSHM.cpp`, `UnResource.cpp`, `UnRebuildTools.cpp`, `UnSequenceDraw.cpp`, `UnLevelVisibility.cpp`, `AGameStats.cpp`, `UnGameUtilities.cpp`, `UnEdLayer.cpp`, `UnEdCoordSystem.cpp`, `DecalVertexFactory.cpp`) — C2065/C2039/C2143/C4430/C2653/C3861/C2146/C2447/C2059/C2011... referencing `FRenderInterface`, `UShadowMap`, `EditorPrivate.h` etc. | ~650 (9 of the 10 worst files) | `Engine_EXCLUDE` in `Engine/Sources.cmake`. Checked against the reference `Engine.vcxproj` (`build\agentB\vcxproj_check.py`: 420 compiled entries vs 430 on disk); none of these has a function in the Shipping PDB. |
| C1083 `NxFoundation.h` / `NxCooking.h` (`UnNovodexSupport.h`) — 40+ PhysX units (`UnPhys*.cpp`, `NxForceField*.cpp`, `PhysXParticle*.cpp`, `NvApex*.cpp`, `UnLevel.cpp`, `UnWorld.cpp`, `UnLevAct.cpp`, ...) | 1 message, 40+ units | `WITH_NOVODEX=0`. The reference `Development/External/Novodex` is NovodeX SDK **2.1.2** (`NxVersionNumber.h`), not PhysX 2.8.4: no `NxCooking.h`, `NxSceneQuery.h`, `fluids/`, `Nxd`. Wiring it is pointless; Phase 4 must supply PhysX 2.8.4 (Shipping links `physx`/`physxloader`). |
| C1083 `vorbis/vorbisenc.h` (`UnAudioDecompress.h`) | 1 | `WITH_OGGVORBIS=0`. The Shipping PDB has no `FVorbisAudioInfo` / `UnAudioDecompress.cpp` function (audio is Wwise/AkAudio), so this matches the shipped exe. |
| C1083 `nvapi.h` (`ue3stereo.h`, included by `SceneRenderTargets.cpp`, `ue3stereo.cpp`, `D3D9Drv.h`) | 1 | `Dishonored::nvapi` via FetchContent of NVIDIA's public SDK (`github.com/NVIDIA/nvapi`, MIT, pinned commit `70d337db`), header + `x86/nvapi.lib`. Shipping links nvapi (10 `NvAPI_*` stubs, 7 `ue3stereo.cpp` functions in the PDB). The 2009 `ue3stereo.h` compiles unchanged against the R535 header. |
| C1083 `nvtt/nvtt.h` (`UnTexCompress.cpp`) | 1 | `Dishonored::nvtt` from the reference tree (`cmake/ReferenceExternals.cmake`, `nvtt/include` + `nvtt/lib/nvtt.lib`). |
| C1083 `streams.h` (DirectShow base classes: `AVIWriter.cpp`, `CapturePin.cpp`, `CaptureSource.cpp`) | 3 | Excluded. Epic's tree had `External/DirectShow`; the reference lacks it, and no `FAVIWriter`/`FCapturePin` function exists in the Shipping PDB. Note: `UnEngine.cpp`/`UnGame.cpp` still reference `FAVIWriter::GetInstance()`, so a Debug **link** will need the DirectShow base classes (Windows SDK samples) in Phase 4. |
| C1083 `..\..\UnrealEd\Inc\DebugToolExec.h` (`UnEngine.cpp`), `UnPatchCommandlets.h` (`UnPatchCommandlets.cpp`, 7 PDB functions), `UnEdTran.h` (`UnErrorChecking.cpp`) | 3 | Copied exactly these three headers from the reference `UnrealEd/Inc` into `source/Development/Src/UnrealEd/Inc/` (new dir; picked up by the flat include model). Only declarations are needed in a game build (`FDebugToolExec` is used under `WITH_EDITOR`). |
| C2653/C2065/C3861 `UDebuggerCore` (`UnGame.cpp`, `UnEngine.cpp`) | 10 | The import copied Inc/Src/Classes only, so `Engine/Debugger/*` and `Engine/Bink/Src/*` had become PDB **stubs** (`debugger/undebuggercore.h` etc.). Replaced them with the reference files (`Debugger/` 9 files, `Bink/Src/` 7 files) and added the 3 `Debugger/*.cpp` units via a new `Engine_EXTRA` list (`UDebuggerCore` has 89 functions in the PDB). |
| C4596 illegal qualified name in member declaration (`Debugger/UnDebuggerCore.h` 588–593, `FCallStack::GetNode` & co.) | 5 | Dropped the `FCallStack::` qualifier (same VS2010 leniency as `FFileManagerWindows.h` in Core). |
| C2665 `GenerateStrips` (`RawIndexBuffer.cpp`) | 2 | The reference nvTriStrip (`External/nvTriStrip` **and** the `Engine/Src/NvTriStrip.h` copy) is the stock 16-bit-index library; UE3 calls Epic's 32-bit fork (`GenerateStrips(const UINT*, ...)`). New Dishonored switch `WITH_NVTRISTRIP=0` added to the 9 guard lines of `RawIndexBuffer.cpp` (cook-time optimiser; no `CacheOptimize` function exists in the Shipping PDB). |
| C3861 `png_set_add_alpha` (`UnPNG.cpp`) | 1 | The reference `External/libPNG` is 1.2.5 (header and `.lib` both lack the symbol), but `UnPNG.cpp` targets libpng 1.5.13. `Dishonored::libPNG` now comes from FetchContent libpng v1.6.43 (`cmake/Dependencies.cmake`); zlib is declared with `OVERRIDE_FIND_PACKAGE` plus a `zlib-extra.cmake` providing `ZLIB::ZLIB` so libpng's `find_package(ZLIB)` uses our zlib build. Core recompiled fine against it. |
| C2011 type redefinition in `Core/Inc/UnLinker.h` | 11 | Side effect of the stale `UnTex.cpp` (excluded). Also deleted the stale 1999 `Engine/Src/EnginePrivate.h` that shadowed `Inc/EnginePrivate.h` (same landmine as Core). |

## Files excluded (`Engine/Sources.cmake`, `Engine_EXCLUDE`)

Stale, not in the reference `Engine.vcxproj`, no PDB functions: `Src/AGameStats.cpp`,
`Src/DecalVertexFactory.cpp`, `Src/UnEdCoordSystem.cpp`, `Src/UnEdLayer.cpp`, `Src/UnGameUtilities.cpp`,
`Src/UnLevelVisibility.cpp`, `Src/UnMaterial.cpp`, `Src/UnRebuildTools.cpp`, `Src/UnResource.cpp`,
`Src/UnSHM.cpp`, `Src/UnScene.cpp`, `Src/UnSequenceDraw.cpp`, `Src/UnStats.cpp`, `Src/UnTex.cpp`.
DirectShow (SDK missing, no PDB functions): `Src/AVIWriter.cpp`, `Src/CapturePin.cpp`, `Src/CaptureSource.cpp`.

Added (`Engine_EXTRA`, new list honoured by `dishonored_module`): `Debugger/UnDebuggerCore.cpp`,
`Debugger/UnDelphiInterface.cpp`, `Debugger/UnWTInterface.cpp`.

Note for whoever regenerates `Sources.cmake` with `import_reference.py`: the script does not know
`Engine_EXTRA` or the exclusion reasons; and it copies only Inc/Src/Classes, which is what produced
the Debugger/Bink stubs in the first place.

## Defines changed (`cmake/DishonoredDefines.cmake`)

| Define | Was | Now | Why |
|---|---|---|---|
| `WITH_NOVODEX` | 1 | 0 | reference has NovodeX 2.1.2, Engine needs PhysX 2.8.4 (Phase 4) |
| `WITH_OGGVORBIS` | 1 | 0 | no libvorbis in the reference; Shipping PDB has no vorbis decoder (Wwise) |
| `WITH_NVTRISTRIP` | — | 0 | new Dishonored-only switch, see `RawIndexBuffer.cpp` |

`WITH_PHYSX_COOKING` stays 1 because `UnBuild.h` defines it unconditionally (a 0 from CMake would
only produce C4005 and lose).

## CMake changes

- `cmake/Dependencies.cmake`: `dishonored_fetch` forwards extra args to `FetchContent_Declare` and
  exports `<name>_SOURCE_DIR/_BINARY_DIR` to the caller (they were function-local before, so
  `${zlib_SOURCE_DIR}` had been empty; it only worked because `zlibstatic` carries PUBLIC include
  dirs). zlib gets `OVERRIDE_FIND_PACKAGE` + `zlib-extra.cmake` (`ZLIB::ZLIB`). New: libpng v1.6.43
  (`PNG_SHARED/TESTS/TOOLS OFF`) as `Dishonored::libPNG`, nvapi (pinned commit) as `Dishonored::nvapi`.
- `cmake/ReferenceExternals.cmake`: reference libPNG block removed (moved to Dependencies); new
  `nvtt` SDK; comment on why nvTriStrip is not wired.
- `cmake/DishonoredModule.cmake`: `<Name>_EXTRA` compile-unit list.
- `CMakeLists.txt`: `Engine` links `Dishonored::nvtt Dishonored::nvapi`.
- `DISHONORED_MSVC_WARNINGS`: nothing added. The full Engine build emits 12 warnings in total
  (5 C4244 in libpng/zlib, 4 C4838, 1 C5205 `UnDebuggerCore.cpp:156` delete of abstract
  `UDebuggerInterface` without virtual dtor, 1 C4996, 1 C4127); all left visible.

## Source edits (all in Engine unless noted)

- `Src/EnginePrivate.h`: deleted (stale 1999 copy shadowing `Inc/EnginePrivate.h`).
- `Src/RawIndexBuffer.cpp`: 9 guard lines gained `&& WITH_NVTRISTRIP` (`// DISHONORED`).
- `Debugger/UnDebuggerCore.h`: 5 in-class declarations unqualified (`// DISHONORED`).
- `Inc/UnPNG.h`: comment only (points at `Dependencies.cmake`).
- `Debugger/*` (9 files) and `Bink/Src/*` (7 files): reference copies replacing the stubs. Git on
  Windows still tracks them under the old lowercase paths (`debugger/undebuggercore.cpp` shows as
  modified, `Debugger/UnDebuggerInterface.h` and the 3 Bink headers as untracked); the coordinator
  may want `git mv` to record the case change.
- New `source/Development/Src/UnrealEd/Inc/{DebugToolExec.h,UnPatchCommandlets.h,UnEdTran.h}`:
  verbatim reference copies.

## Core edits

None. (`Core/Inc/*` was not touched; the Core failures in the last passes are agent A's.)

## Open items for Phase 4 / the coordinator

- PhysX 2.8.4 SDK (WITH_NOVODEX), libvorbis (only if WITH_OGGVORBIS is ever wanted; Shipping has
  none), DirectShow base classes (AVIWriter, Debug link only), Epic's 32-bit nvTriStrip fork
  (WITH_NVTRISTRIP), FaceFX/APEX/GFx/Steamworks as before.
- `porting_notes.md`: the libPNG row and the "Stale `Src/<Module>Private.h`" row can be updated
  from this report (Engine's copy is gone; libpng is now FetchContent).
- `DishonoredLayouts.h` (agent A) demands `/Zp4`; that is a global compile option change and
  should be decided once, since it affects every module.
