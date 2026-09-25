# Porting notes — reference UE3 10897 → MSVC 2022 / C++17

Policy (PLAN.md §3, Phase 2 P2.4): mechanical fixes only, no behavior changes. Every non-trivial
edit to a file copied from the reference carries a `// DISHONORED: <why>` comment so
`import_reference.py --diff <Module>` stays readable. Layout changes proven by the PDB use
`// DISHONORED(layout): <PDB member / size>`.

## Build environment decisions

| Decision | Value | Source |
|---|---|---|
| Game slot | `GAMENAME=DISHONOREDGAME`, `DISHONOREDGAME=9` | `Launch/Inc/LaunchGames.h` assigns 2–8 to Epic's games |
| Windows target | `_WIN32_WINNT=0x0502`, `WINVER=0x0502` | `UE3BuildWin32.cs`; raised only if the Windows 10 SDK refuses it |
| Include model | every `source/Development/Src/*/Inc` is on every module's include path | UnrealBuildTool adds all module Inc dirs globally; Core's `UnVcWin32.h` includes WinDrv's `PreWindowsApi.h`, `UnFile.h` includes `../../Engine/Inc/UnConsoleTools.h` |
| Relative includes | kept as in the reference (`../../Engine/Inc/...`, `../../Launch/Resources/...`) | they resolve because the tree mirrors `Development/Src/<Module>` |
| zlib | FetchContent v1.3.1 (`cmake/Dependencies.cmake`), `UnMisc.cpp` includes `<zlib.h>` | reference used `Development/External/zlib` which the clone lacks |
| LZO | `WITH_LZO=0` for now — **milestone-3 blocker** | cooked packages are `PKG_StoreCompressed` with `CompressionFlags=2` = `COMPRESS_LZO` (earlier note wrongly read that as zlib); Dishonored links LZO Pro's `lzopro_lzo1x_decompress_safe`, LZO1X-compatible. Plan: lzokay (MIT) or LZO 2.10 via FetchContent, then `WITH_LZO=1` |
| Excluded units | `Core/Src/UnitTest.cpp` | test harness |
| Launch | reference `Launch.cpp`/`LaunchEngineLoop.cpp` imported but not compiled; `DishonoredLaunchStub.cpp` builds the exe until milestone 1 | |

| DirectX 9 SDK | reference `Development/External/DirectX9`, mirrored into the build tree **without `rpcsal.h`** (`cmake/ReferenceExternals.cmake`) | its 2010-era `rpcsal.h` shadowed the Windows Kit's and broke `objidl.h` (101 × C2061) |
| FaceFX | `WITH_FACEFX=0` for now | SDK not available; Engine headers include `../../../External/FaceFX/FxSDK/Inc/FxSDK.h` |
| APEX | headers need `foundation/PxSimpleTypes.h` (PhysX 3 foundation, not in the reference Novodex 2.8 SDK) | pending: `WITH_APEX=0` or an APEX SDK, decided when Engine compiles |
| Steamworks / Scaleform | `WITH_STEAMWORKS=0`, `WITH_GFx=0` for now | `Engine.h` includes `OnlineSubsystemSteamworks.h` → `steam/steam_api.h` and `ScaleformEngine.h` → `Kernel/SF_Types.h` (GFx 4 SDK); neither SDK is available yet (Phase 4) |
| libpng | reference `Development/External/libPNG`, mirrored with `"../../zlib/zlib.h"` rewritten to `<zlib.h>` | |
| Precompiled headers | off (`DISHONORED_USE_PCH=OFF`) | CMake's `/FI` force-include double-includes guard-less UE3 private headers |
| Stale `Src/<Module>Private.h` | `Core/Src/CorePrivate.h` deleted (1999 copy; the project uses `Inc/CorePrivate.h`, but same-directory lookup found the stale one first) | Engine, WinDrv, D3D9Drv have the same pair; check each before compiling that module |
| Two-phase lookup | `UnStats.h`: use the deferred (`gcc`) constructor definitions instead of the in-class ones | `TAccumulator`/`TCounter` referenced `FStatGroup`/`GStatManager` before their declaration |

## Compile error categories (Core)

Filled in during P2.4 from `core_build*.log`. One row per category: count at first sight, fix
applied, files touched.

| Category | Count | Fix | Notes |
|---|---:|---|---|
| C1083 missing include (`PreWindowsApi.h`, `d3dx9.h`, `../../Engine/Inc/...`, `zlib.h`, FaceFX, `OnlineSubsystemSteamworks.h`) | 6 distinct | flat include model; DirectX mirror; zlib FetchContent; import the sibling modules' headers; `WITH_FACEFX=0` | see decisions above |
| C2061 SAL macros in Windows Kit headers | 101 | exclude `rpcsal.h` from the DirectX mirror | |
| C2065/C3861 two-phase lookup in `UnStats.h` | 20 | gcc path | |
| C2011 type redefinition (`UnLinker.h`) | 704 | delete stale `Src/CorePrivate.h` | |
| C3240/C2838/C4596 qualified names in in-class declarations (`FFileManagerWindows.h`) | 8 | drop the `FFileManagerWindows::` qualifier on 3 declarations | reference code relied on VS2010 leniency |
| C1083 `..\..\..\External\libpng\png.h` via `Engine.h` → `UnPNG.h` | 1 | libPNG headers/libs from the reference tree (`Dishonored::libPNG`), `UnPNG.h` includes `<png.h>`/`<zlib.h>` | Core's `UnMisc.cpp`, `UnVcWin32.cpp`, `UnStatsNotifyProviders.cpp` include `Engine.h` (Epic's own layering violation) |

## Layout probe limitations

`source/Tests/LayoutProbe` (generated) prints `sizeof`/offsets for every PDB type the module
declares. `#define private public` before `Core.h` exposes explicitly `private:`/`protected:`
members, but members that are private *by default* (declared right after `class X {` with no
access specifier) stay private, and the SFINAE member detection then reports them as `MISSING`.
Sizes are always compared. If those offsets are ever needed, build the probe unit with
`clang-cl -fno-access-control`. Types the module's main header does not reach are listed in
`resources/docs/types/probe_skip_<Module>.txt`.

## Warnings (Core, first clean build: 987)

| Warning | Count | Action |
|---|---:|---|
| C4305 double→float literal truncation | 462 | silenced (`/wd4305`), pervasive UE3 style |
| C4595 inline non-member `operator new/delete` (`UnFile.h` 2147–2170) | 260 | silenced for now; the definitions must move into one compile unit before the exe links (ODR) |
| C4005 macro redefinition (`NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `WITH_SPEEDTREE_MANGLE`) | 195 | removed the duplicate definitions from CMake |
| C4471 forward-declared unscoped enum (`EPixelFormat`) | 65 | silenced (`/wd4471`) |
| C4838 narrowing in aggregate init (`UnVcWin32.cpp`) | 4 | left visible |

## Bytecode opcodes

`resources/docs/symbols/opcodes.md` (`xcheck_opcodes.py`): Dishonored's `EX_*` numbering (UObject
natives with GNatives index < 0x80, all indices per folded function) **matches the reference**
`UnStack.h` / `UnCorSc.cpp`. The 6 nominal differences are identical-COMDAT-folding artifacts
(`execFalse`/`execIntZero`/`execNoObject`, `EqualEqual_DelegateFunction`/`_DelegateDelegate`,
`EqualEqual_IntInt`/`_ObjectObject` share one body). No opcode regeneration needed; the numbered
natives ≥ 0x80 are compared per class in P2.7.
