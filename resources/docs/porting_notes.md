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
| LZO | `WITH_LZO=1` via lzokay (MIT, FetchContent), `COMPRESS_DefaultPC = COMPRESS_LZO` | cooked packages are `PKG_StoreCompressed` with `CompressionFlags=2` = `COMPRESS_LZO`; Dishonored links LZO Pro's `lzopro_lzo1x_decompress_safe`, LZO1X-compatible (wave 1 K) |
| Excluded units | `Core/Src/UnitTest.cpp`; `USE_UNIT_TESTS=0` | neither exe has an `FUnitTestFramework` function (wave 2 P) |
| Launch | real `Launch.cpp`/`LaunchEngineLoop.cpp` behind `DISHONORED_REAL_LAUNCH=ON` (`GAMENAME=DISHONOREDGAME` branches, `DISHONORED_HAVE_<MODULE>` registrant hooks, remaining stubs in `DishonoredStubs.cpp`); `DishonoredLaunchStub.cpp` only when OFF | wave 1 N, wave 2 O/P/T |

| DirectX 9 SDK | reference `Development/External/DirectX9`, mirrored into the build tree **without `rpcsal.h`** (`cmake/ReferenceExternals.cmake`) | its 2010-era `rpcsal.h` shadowed the Windows Kit's and broke `objidl.h` (101 × C2061) |
| FaceFX | `WITH_FACEFX=0` for now | SDK not available; Engine headers include `../../../External/FaceFX/FxSDK/Inc/FxSDK.h` |
| APEX | `WITH_APEX=0` — final: retail ships the APEX DLLs but imports nothing from them and has no `NxApex` symbol (`middleware.md`) | wave 2 U |
| Steamworks / Scaleform / Wwise | `WITH_STEAMWORKS=0`, `WITH_GFx=0`; the OSS/GFxUI/AkAudio script classes are registered from generated units (`DISHONORED_ENABLE_*`) with `appErrorf` natives | SDKs per `middleware.md` (Steam 1.18/1.19, GFx 3.3.89 has none, Wwise 2012.1) |
| libpng | reference `Development/External/libPNG`, mirrored with `"../../zlib/zlib.h"` rewritten to `<zlib.h>` | |
| Struct packing | `/Zp4` on every engine/test target (`dishonored_apply_defines`), **not** global | UnrealBuildTool `VCToolChain.cs:30`; PDB proves it (`UProperty::PropertyFlags` at 68, `UField` 60). Global `/Zp4` breaks libpng and the Windows SDK `C_ASSERT`s; Windows headers are wrapped by `PreWindowsApi.h` (pack 8) |
| libpng | FetchContent 1.6.43 (reference copy is 1.2.5, too old for `UnPNG.cpp`); zlib exported as `ZLIB::ZLIB` via `OVERRIDE_FIND_PACKAGE` | agent B |
| nvapi / nvtt | nvapi via FetchContent (D3D9Drv `ue3stereo.h`); **nvtt users are `WITH_EDITOR` only** (retail imports no nvtt.dll; 2012 decompiles of `UTexture2D::Compress` etc. are post-nvtt tails), no delay-load | wave 2 P |
| PhysX | `WITH_NOVODEX=0`: the reference `External/Novodex` is NovodeX 2.1.2 (2004); retail uses PhysX SDK 2.8.4 (DLLs 2.8.4.6) | Phase 4 must supply the 2.8.4 SDK (`middleware.md`) |
| Editor-only data | `WITH_EDITORONLY_DATA=1` | retail keeps the editor-only members (SDK dump: `AMatineePawn::PreviewMesh` @1184, `CPF_EditorOnly` flags) |
| Show flags | `typedef QWORD EShowFlags` with Dishonored's bit assignments (`Engine/Inc/ShowFlags.h`, `Scene.h`) | 2012 PDB `unsigned __int64` @96/@24, retail SDK `FQWord`; 62 bits witnessed in decompiles, 22 reference flags → 0 (wave 2 V) |
| System settings | `FSystemSettings` reads `[SystemSettings]` from `GEngineIni` with retail's 107 keys; no `checkf` on missing keys; HKCU `Software\Arkane\Dishonored` override | retail has no `SystemSettings.ini` string; 2013 rva 0x1806c0 (wave 2 O) |
| Shaders | no shader compiler, no `.usf` source hashing; `VerifyGlobalShaders` = retail body; cooked global shader cache only | neither exe has `FShaderCompilingThreadManager` or `.usf` strings (wave 2 O) |
| Native package list | hardcoded from the 2013 exe (0x5def10 / 0x5dfb50); bring-up switches `-skipnativepkgs=` and `-allowunboundnatives` | retail inis have no `[Engine.ScriptPackages]` (wave 2 O) |
| Reference-only members | `DISHONORED_SHIM_STATIC` (inline static, no storage) after the PROPS block; the layout probe reports them MISSING; every use in `Engine/Src` is a porting TODO | agents M, Q, R, S |
| Staging | our exe is copied into the retail `Binaries\Win32`; **no junctions/symlinks into the retail or reference trees, ever** | 2026-09-25 content-loss incident (STATUS.md) |
| Engine stale units | 14 units the reference `Engine.vcxproj` itself does not compile + DirectShow AVI writer excluded (`Engine/Sources.cmake`); `Engine_EXTRA` adds `Debugger/*.cpp` | agent B |
| Stale `Core/Inc/FOutputDeviceAnsiError.h`, `FOutputDeviceStdout.h` | deleted (1999 inline copies conflicting with `UnOutputDevices.cpp`) | agent D |
| Precompiled headers | off (`DISHONORED_USE_PCH=OFF`) | CMake's `/FI` force-include double-includes guard-less UE3 private headers |
| Stale `Src/<Module>Private.h` | `Core/Src/CorePrivate.h` deleted (1999 copy; the project uses `Inc/CorePrivate.h`, but same-directory lookup found the stale one first) | Engine, WinDrv, D3D9Drv have the same pair; check each before compiling that module |
| Two-phase lookup | `UnStats.h`: use the deferred (`gcc`) constructor definitions instead of the in-class ones, and enable their `#if __GNUC__` definitions for MSVC too (otherwise unresolved `TAccumulator<T>::TAccumulator` at link) | `TAccumulator`/`TCounter` referenced `FStatGroup`/`GStatManager` before their declaration |

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
