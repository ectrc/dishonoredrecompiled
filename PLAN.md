# Dishonored (UE3) Recompilation Project Plan

Written 2026-09-24, last revised 2026-09-25 after Phase 3 wave 2 (`resources/docs/PHASE4.md`). Progress
summary: `resources/docs/STATUS.md`.
Working root: `D:\RecompileDishonored\Recompile\`.

## 1. Goal

Rebuild Dishonored's native executable from C++ source so that a self-compiled
`DishonoredGame-Shipping.exe` loads the retail cooked content, plays the full game plus DLC,
and can be modified freely (offline play, multiplayer via `dismod`, no dependence on any dead
online service).

**The target is the RETAIL 2013 build** (`Dishonored_Latest2026\Binaries\Win32\Dishonored.exe`,
engine 9411, changelist 334700, with DLC05–07). Everything the rebuilt exe does must match that
binary: class and struct layouts, member sets, serialization, native tables, behavior. The
symbolized 2012 QA build is only a **helping hand**: it donates names, types and readable
decompiles because it has a PDB, but a large number of its structs have different sizes and
different members than the 2013 retail build (see `resources/docs/types/xcheck_sdk.md`,
`dishonoredgame_class_inventory.md` enum/member deltas). Whenever 2012 and 2013 disagree, 2013
wins; nothing derived from the 2012 PDB is final until it has been checked against the retail exe
or the retail cooked packages.

This is a **functional** rebuild on top of a **real Unreal Engine 3 source tree**, not a
matching one:

* The base is the leaked UE3 build 10897 source at `D:\RecompileDishonored\UnrealEngine3`
  (CodeRedModding/UnrealEngine3, Feb 2013). Dishonored's engine is 9014 (2012) / 9411 (2013):
  the same generation, ~8 months apart, so the reference already contains the object system,
  linker, script VM, renderer, PhysX glue and tools. Engine work is *convergence* (make the
  reference behave like Dishonored's branch), not reconstruction.
* Only what the reference lacks is written from the decompile: the DishonoredGame module
  (21.8k functions), Arkane's engine modifications, AkAudio, Edge/DisJobs, and Arkane's GFxUI
  extensions. Decompiles come from the 2012 build (named) and are then re-checked against the
  2013 retail exe (unnamed, diffed function by function, Phase 7) before they count as done.
* Modern toolchain (MSVC 2022, C++17, CMake). No VS2008, no UnrealBuildTool, no byte matching.
  Decompiler output is a reference, never the deliverable.
* Only the cooked content format and the UnrealScript bytecode contract are sacred. Behavior,
  not bytes, is the test.

Working assumption on "server shut off": the retail exe has no HTTP endpoints besides
driver-download URLs. Its online-facing pieces are Steam, Steam leaderboards for DLC05, and a
`libcurl.dll` import that only exists in the 2013 build. Phase 8 removes them.

## 2. What we have (verified inventory)

### 2.1 `Dishonored_Latest2026\` — retail 2013 build, no symbols

| Item | Value |
|---|---|
| `Binaries\Win32\Dishonored.exe` | 18,041,856 bytes, Shipping config, **no PDB** |
| Build | engineVersion 9411, changelist 334700, 2013-08-20, branch `Dishonored_Campfire` |
| Imports | libcurl, steam_api, dinput8, xinput1_3, d3d9, wsock32, binkw32, MSVCR90/MSVCP90 |
| Content | `CookedPCConsole` 1463 files / 5.4 GB, INT only; `DLC\PCConsole\DLC05,06,07` |
| Config | `DefaultEngine.ini` already edited: `bEnableSteam=false` |
| RE state | `resources/docs/idb/retail2013_named.i64`: 65,841 functions, 82.8 % of the 2012 names propagated (agent J, own matcher); the original `Dishonored.exe.i64` was a bare auto-analysis |
| Mod hook | `_dinput8.dll` = `dismod` build (imgui overlay, Steam bypass, spawn panel) |
| Content state | **2026-09-25: `Engine/`, `CookedPCConsole`, `DLC`, `Localization`, `Movies` were deleted by a recursive delete through staging junctions (STATUS.md incident); restore via Steam "Verify integrity"** |

### 2.2 `Dishonored_Debug2012\` — leaked QA build, full symbols

Release / Shipping / Shipping-DC / ArkProfile configs with GUID-matching PDBs (not a Debug
config). Build 9014, changelist 254295, 2012-06-20, branch `//dishonored/UnrealEngine3QATest`,
original source root `V:\dishonored\UnrealEngine3QATest\Development\Src\`. Win64 exes without
PDBs. `CookedPCConsole` 9.6 GB, seven languages, no DLC. Only the ArkProfile exe writes a
`Launch.log`; it reaches the main menu with `-seekfreeloadingpcconsole` (`resources/docs/golden/`).

Shipping exe measured in Phase 1 (`resources/docs/module_map.md`): 66,394 functions, 11.4 MB of code.

| Runtime modules (linked into Shipping) | Not in Shipping |
|---|---|
| Core (10 %), Engine (41 %), GameFramework, IpDrv, Launch, WinDrv, D3D9Drv, AkAudio, GFxUI, OnlineSubsystemSteamworks, DishonoredGame (25 %), DisJobs, Edge (18 fns) | UnrealEd, UnrealEdCLR, DishonoredEditor, GFxUIEditor, UnrealSwarm, EdgeTool, XAudio2, OnlineSubsystemPC, PathEngine |

Static libraries inside the exe: Scaleform `libgfx` (10.4 %), Wwise `ak*` (~4 %), FaceFX
(1.3 %), PhysX helper, libpng, zlib, LZOPro.

### 2.3 `D:\RecompileDishonored\UnrealEngine3\` — reference engine source (build 10897)

See `resources/docs/engine_reference.md` for the full comparison. Key numbers: 651 of the 1,884 Shipping
PDB source files exist in the reference (Core 112/118, Engine 467/577, D3D9Drv/IpDrv/WinDrv/Launch
complete); 66 % of Core and 41 % of Engine functions have a same-named definition. It ships
PhysX 2.8 (Novodex) headers, DirectX9, libpng, libogg, nvtt, zlib. It does **not** ship
Scaleform, Wwise, FaceFX, Bink or Steamworks SDKs, nor anything of Arkane's.

### 2.4 Related assets

* `D:\DishonoredMapMaking\` — DFSDK: UDK 2010 editor plus UE Explorer-decompiled UnrealScript
  for `DishonoredGame` (228 `.uc`). Its Engine/Core `.uc` are stock UDK 2010, not Dishonored's.
* `D:\Christmas\github\dismod\` — own dinput8 proxy (CMake, imgui, Steam hook, spawn/world mods).
* `D:\RecompileDishonored\Dishonored_DumpedSDK_Retail\` — CodeRed-Generator dump taken inside the
  running **retail** exe: 3,038 classes / 943 structs / 4,548 functions with the runtime
  `UProperty::Offset`, sizes, flags and exec-parameter layouts of the 2013 build. The only
  offset-level retail source; parsed by `resources/tools/sdk/parse_codered_sdk.py`, used by
  `xcheck_sdk_layout.py`. What it can and cannot tell: `resources/docs/sdk_dump.md`.

### 2.5 Toolchain

VS 2022 (MSVC 19.44), CMake 4.4, ninja, FetchContent, Python 3.13 + comtypes (DIA SDK), IDA 9.1 with
idalib (`idapro` module) and the IDA MCP, git. `resources/docs/toolchain.md`.

## 3. Strategy

**Port the reference engine, then converge it onto Dishonored using the symbolized 2012 build.**

1. **Reference first.** Every engine module starts as a copy of the 10897 source, trimmed to
   what Shipping links. The first milestone is that this code *compiles and links* under MSVC
   2022 with our CMake build. That is compile-fixing, not writing.
2. **Converge by PDB.** For every function in `resources/docs/symbols/functions.csv`:
   * same-named definition in the reference → diff its Hex-Rays decompile against the
     reference source; port the difference (Arkane change or Epic drift between 9014 and 10897)
     only when it affects behavior, data layout or serialization.
   * no definition → write it from the decompile into the module.
   The per-function status (`reference`, `ported`, `written`, `stubbed`, `verified`) lives in
   `resources/docs/progress.md`.
3. **Layout and serialization are the contract — the RETAIL 2013 layout.**
   The 2012 PDB (`resources/docs/types/sizes.csv`) gives named layouts and is the first approximation;
   the authoritative sizes and member sets are the retail ones, recovered from (a) the 2013
   cooked packages (member list, order and types of every script class; offsets are not
   serialized, `UStruct::Link` recomputes them), (b) the 2013 exe (class sizes in constructors/`StaticClass` registration,
   member accesses in decompiled functions after Phase 7 name propagation), (c) the runtime
   dump of the retail exe (`Dishonored_DumpedSDK_Retail`, `resources/docs/sdk_dump.md`): the
   offset of every reflected member as retail's `UStruct::Link` computed it. `static_assert`s guard every native class against
   the **2013** numbers once known; the 2012 numbers are a stepping stone.
   Package version constants (`UnObjVer.cpp`) are pinned to Dishonored's cooked packages, not
   10897's; `Serialize()` overrides are diffed function-by-function because that is where
   Arkane's licensee changes hide.
4. **DishonoredGame is written, not ported.** 21.8k functions, 1,058 source files, none in the
   reference. UnrealScript-side class layouts come from the PDB types (exact) and the DFSDK
   `.uc` (readable); the native side is decompile-guided rewriting, in dependency order inside
   the module.
5. **Prefer existing code over decompiling** for middleware: PhysX 2.8 headers (reference
   tree), DirectX SDK, libpng/zlib/libogg/libvorbis/LZO (FetchContent), Steamworks SDK. Scaleform,
   Wwise and FaceFX have no source; Phase 4 decides per library between rewrite-from-decompile,
   hybrid-link during bring-up, or subset reimplementation.
6. **Modern toolchain from day one.** MSVC 2022, `/std:c++17`, `/fp:precise`, Win32 x86 for
   bring-up (pointer size matches the cooked script layouts); x64 later, as the reference's
   Win64 target and `UStruct::Link` offset recomputation allow.

Fallback / incremental value: `dismod`-style hooking against the retail exe stays viable at
every stage; a compiling Core+Engine from the reference is immediately a typed SDK for it.

## 4. Phases

### Phase 0 — Project setup — DONE 2026-09-24

Repo, `.gitignore`, `resources/docs/binaries.md`, toolchain, CMake x86 preset (`resources/run-vcvars.cmd`), golden
logs. Tracker: `resources/docs/PHASE1.md`.

### Phase 1 — Symbol and type database — DONE 2026-09-25

`resources/docs/symbols/` (functions with file/line/module, globals, imports, natives with GNatives
indices, hardcoded FNames, vtables), `resources/docs/types/` (sizes, layouts, header), `resources/docs/module_map.md`.
Regeneration: `resources/docs/symbols/README.md`. Exit check: `resources/tools/symbols/verify_phase1.py`.

### Phase 2 — Reference import and layout convergence — DONE 2026-09-25

Tracker `resources/docs/PHASE2.md`; exit check `resources/tools/symbols/verify_phase2.py` (18/18). Original task list, all done:

- [x] `resources/tools/symbols/xref_reference.py`: for each PDB function, look up a same-named definition
      in the reference; for each PDB source file, whether it exists there. Writes
      `resources/docs/reference_xref.csv` and refreshes the tables in `resources/docs/engine_reference.md`.
- [x] Copy the Shipping runtime modules from the reference into `source/Development/Src/`: Core,
      Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, GFxUI, OnlineSubsystemSteamworks, Launch,
      zlib. Keep Epic's `Inc/Src/Classes` layout. Drop editor-only, console, mobile and
      D3D11/OpenGL code paths behind CMake options rather than deleting them.
- [x] Add empty `DishonoredGame`, `AkAudio`, `DisJobs` modules with the file list from
      `resources/docs/symbols/sourcefiles.txt`.
- [x] Generate `source/Development/Src/Core/Inc/DishonoredLayouts.h` from `resources/docs/types/sizes.csv`:
      `static_assert(sizeof(X) == N)` for every native class the cooked script side references.
- [x] (`gen_layout_probe.py generate/compare`) PDB member offsets vs the reference headers
      (parse `types.json`; compile-time probe of the reference via a generated `offsetof`
      program). Output `resources/docs/types/reference_layout_delta.md`: the exact list of Arkane layout
      changes per class. Fix headers first; nothing else compiles correctly until they match.
- [x] Pin `UnObjVer.cpp` / `UnNames.h` to Dishonored: package file version and licensee version
      read from `Core.upk`; `hardcoded_names.csv` replaces the reference name list (indices
      must match, names 0–1300 sparse).
- [x] Native registration: regenerate the `IMPLEMENT_FUNCTION` / `AutoInitializeRegistrants`
      lists from `resources/docs/symbols/natives.csv` and `classes.csv`.
- **Exit:** Core compiles as a static lib under the CMake build; `DishonoredLayouts.h` passes
      for Core types; `reference_layout_delta.md` exists for Engine and DishonoredGame types.

### Phase 2b — Retail (2013) layout truth — DONE for Core/Engine 2026-09-25

Trackers: `resources/docs/PHASE3.md` (H, I, J, C1–C3) and `PHASE4.md` (Q, R, S, T, V). Reconciliation
notes: `resources/docs/types/retail_reconciliation.md`; the SDK dump: `resources/docs/sdk_dump.md`.

- [x] (package I, 2026-09-26) `resources/tools/pdb/read_package_classes.py`: parse the 2013 cooked packages (LZO chunks,
      name/import/export tables, `UClass`/`UStruct`/`UProperty`/`UEnum`/`UFunction` exports).
      UE3 does not serialize property offsets or `PropertiesSize` (`UStruct::Link` recomputes
      them), so this yields the exact 2013 **member list, order and types** of every script
      class, its enums and native function indices — from which offsets follow deterministically.
      Do the same for the 2012 packages and diff.
- [x] 2013 native class sizes: every `InitializePrivateStaticClass<X>` calls
      `UClass::UClass(ENativeConstructor, sizeof(X), …, L"<Name>", L"<Package>", …)`; the size is an
      immediate next to the class-name string xref, readable in the unnamed retail exe →
      `native_class_sizes.csv` (2012 vs 2013). DONE 2026-09-26 (package H).
- [x] Retail member **offsets**: `resources/tools/sdk/parse_codered_sdk.py` parses the CodeRed dump of
      the running retail exe (`Dishonored_DumpedSDK_Retail`) into `retail_sdk_layout.json`;
      `resources/tools/sdk/xcheck_sdk_layout.py build/<dir>/layout_probe.txt` checks every probed
      member offset and class span against it → `retail_sdk_delta.md` (first run 1,132 types / 666 exact / 234 to converge; after wave 2:
      2,314 types / 1,677 exact / 4 rows). This replaces the 2012-offset check wherever retail differs.
- [x] Regenerate `DishonoredLayouts.h` and the layout probe against the **2013** sizes (retail sizes
      from `native_class_sizes.csv`, script-struct sizes and member offsets from the SDK dump); every
      Engine `*Classes.h` regenerated with `sdk_props.py` (wave 2 Q/R/S), `EShowFlags` QWORD (V).
- [x] Exit (Core/Engine): 0 contract mismatches in `gen_layout_probe.py compare` and
      `xcheck_sdk_layout.py`; DishonoredGame: 12,137 generated SDK asserts pass, 4 SDK rows and 354
      pending asserts wait on GameFramework/IpDrv bases. `verify_phase2.py` `retail` section: not written
      (the two cross-check tools are the exit check).

### Phase 3 — Module convergence and DishonoredGame rewrite (months)

Wave 1 (`resources/docs/PHASE3.md`, done 2026-09-26): retail truth, Core/Engine contract types
reconciled with retail, milestone 1. Wave 2 (`resources/docs/PHASE4.md`, done 2026-09-25 machine
date): milestone 2 for the four native packages (O), D3D9Drv target (P), every Engine header on
the retail SDK offsets (Q/R/S/V), DishonoredGame/GFxUI/AkAudio/OSS registrants + headers generated
from the SDK dump (T), middleware versions + Phase 4 memo (U). Wave 3 (`resources/docs/PHASE5.md`,
planned 2026-09-25): Edge-animation bypass (W), milestone 3 driver to `GEngine->Init()` and the tick
loop (X), first frame + Bink movies (Y), `DishonoredGameFull_P` up for play without PhysX (Z), Engine
per-function convergence wave 1 (AA), GameFramework/IpDrv bases + pending asserts (AB), DishonoredGame
natives on the startup path (AC). Wave 4: the Edge animation evaluator port.

Order (sizes from `resources/docs/module_map.md`; Shipping has no OnlineSubsystemPC/XAudio2):

1. Core — 6,577 functions, 66 % in reference
2. Engine — 22,892 functions, 41 % in reference
3. GameFramework — 517
4. IpDrv, OnlineSubsystemSteamworks — 375
5. WinDrv, D3D9Drv — 355
6. AkAudio — 216 (+ Wwise decision, Phase 4)
7. GFxUI — 1,041 (+ Scaleform decision, Phase 4)
8. DishonoredGame, DisJobs — 21,799 functions, 2.8 MB, written from decompile
9. Launch — 67

Per module:

- [x] Make the reference copy compile and link with MSVC 2022 (`/permissive-`, C++17): Core, Engine,
      GameFramework, IpDrv, WinDrv, D3D9Drv, Launch; GFxUI/AkAudio/OSS/DishonoredGame as generated
      registrant units (their glue/natives are stubs until Phase 4 / the rewrite).
- [x] Batch Hex-Rays decompile: `resources/tools/ida/decompile_funcs.py <db> <out> <name|re:|rva:> …`
      (headless, per pattern or list file; per-module sweeps go to `resources/reference/decomp/`, gitignored).
- [ ] Convergence pass in priority order: `Serialize`, constructors / `StaticConstructor`,
      `exec*` natives, virtuals in vtable order, then the rest. **Started**: Core 26 ported (L),
      Engine bring-up ports by O (SystemSettings, RHIInit, shader system, loader `Serialize` deltas,
      GC token streams), all re-checked in the 2013 db; the `DISHONORED_SHIM_STATIC` tables in
      `agents/agentQ/R/S.md` are the Engine work list. For `reference` functions diff
      decompile vs source and port behavior differences; for `missing` functions write them.
      Record status per function in `resources/docs/progress.md`.
- [ ] Third-party code inside the exe: identify with FLIRT/Lumina first; anything matched is
      replaced by the library, never rewritten.

DishonoredGame specifics — headers, registrants, names and native stubs are **generated** (wave 2 T,
`gen_classes_header.py --sdk`, 1,822/1,823 native classes, 73 group headers, a `UStruct::Link`
emulator that reproduces all 9,342 SDK offsets); what remains is the native code. Original plan:
generate the class declarations (`DishonoredGameClasses.h` equivalent)
from the retail SDK dump (`retail_sdk_layout.json`: 1,870 classes / 665 structs with retail offsets,
sizes, flags and `UnknownData` gaps for the native-only members) with `resources/docs/types/types.json`
(2012 PDB) supplying the names and types of what sits in those gaps, cross-referenced with the
DFSDK `.uc` for comments; exec-parameter structs and `FunctionFlags` for the `event`/`exec`
wrappers come from the dump's `*_parameters.hpp`; then rewrite natives and native classes in dependency order (`DisGlobalEnums`, items,
pawns, AI brain processes, powers, UI last).

### Phase 4 — Third-party dependencies — **DONE for PhysX, Steamworks, Wwise and Bink 2026-09-26**

Nothing had to be obtained: the retail install ships the runtime DLLs and the 2012 tree ships their PDBs, so
the bindings are **written by us** (`source/Development/Src/External/`, one `cmake/<Name>.cmake` each, a
throw-away stub DLL per library purely for its import lib, the shipped DLL as the runtime, nothing copied or
redistributed). See `agents/agentAL.md` (PhysX 2.8.4), `agentAM.md` (Steamworks), `agentAN.md` (Wwise 2012.1)
and `middleware.md`.

* **PhysX 2.8.4** — `WITH_NOVODEX=1`: 68 interfaces at their PDB vtable slots, 109 descriptors, 33 default sets
  decoded out of the DLLs' compiled inline code, the SDK descriptor from retail's own construction. Creates its
  scene, cooks convex meshes from the packages' precooked data, runs rigid-body init for 235 components.
  Narrower than retail and tagged: empty contact stream, fluids off, no double buffering, no character controller.
* **Steamworks** — `WITH_STEAMWORKS=1`: 10 headers from the shipped DLL's exports, its version strings and 21
  retail call sites pinning the vtable slots; the three remaining unported natives ported. Sockets stay off on
  evidence (retail's networking is plain IpDrv). No Steam client here, so only the import and init path is
  proven live; with Steam installed, `steam_appid.txt` = 205100 and no `-nosteam` finishes the proof.
* **Wwise 2012.1** — headers from the 2012 PDB with a `static_assert` per struct size, plus a silent backend
  that really reads the shipped banks: 65 file packages, 39 banks loaded, 463 of 468 events resolved, and the
  Wwise name hash verified as FNV-1 over all 981 shipped containers. **This is where audio stops until Phase 10**
  (the user's call, 2026-09-26: audio is polish). Real audio then needs either the licensed SDK, after which
  `DISHONORED_WWISE_SDK=<path>` is the whole switch, or an evaluator for the bank logic.
* **Still open**: Scaleform GFx (the main menu, `middleware.md` decision) and FaceFX. APEX stays off
  permanently — retail ships the DLLs but never linked it.

### Phase 5 — Build system

- [x] CMake + ninja, MSVC 2022, Win32 x86 presets. `dishonored_module(<Name>)` per module, options
      `DISHONORED_ENABLE_*`, `DISHONORED_REAL_LAUNCH`, `DISHONORED_LAYOUT_CHECKS`, `DISHONORED_SDK_LAYOUT_CHECKS`, `DISHONORED_WITH_BINK`.
- [x] Configs: presets `x86-debug`, `x86-release`, `x86-shipping` (`DISHONORED_SHIPPING`).
- [x] Staging: `stage_retail.py` copies our `DishonoredGame.exe` into the retail `Binaries\Win32` next to
      `Dishonored.exe` (no junctions; the earlier junctioned stage caused the 2026-09-25 content loss).
- [x] `resources/tools/build_and_smoke.py`: build, stage, run with `-log -nosteam -seekfreeloadingpcconsole -unattended`
      (+ `--rhi null|d3d9`, `--expect`, `--skip-native`), golden-log diff cut at `--milestone`.

### Phase 6 — Bring-up milestones (behavioral)

1. Reference Core + Engine + Launch compile and link with MSVC 2022. **DONE 2026-09-26.**
2. Runs to `Init: Object subsystem initialized` with Dishonored's names/versions. **DONE 2026-09-26** (null RHI).
3. Loads `Core.upk`, `Engine.upk`, `DishonoredGame.upk`, `Startup.upk`; script VM runs
   `defaultproperties` without asserts. **DONE 2026-09-25 (wave 3 X/Z)**: retail seek-free path, `Startup.upk`
   63,718 objects, `Initializing Engine...`, `LoadMap: DishonoredGameFull_P` up for play, `Initial startup: 5.2s`
   on the null RHI (`PHASE5.md`, wave result). Still to do: the load-all test over all 471 `.upk` and every
   `.pck` with object counts compared with the reference build.
4. D3D9 device up; Bink startup movie and Scaleform main menu render. **MOSTLY DONE 2026-09-26 (waves 3 Y,
   4 AG/AH)**: D3D9 device and viewport, the 8 Bink startup movies, and the cooked shader caches now load
   whole (127 global records and 2,580 material maps, 0 mismatches, 0 undeclared); the scene renderer runs
   every frame instead of being skipped. Left: a Release or `FMallocBinned` build for long d3d9 runs (the
   debug allocator exhausts the 32-bit heap with the caches resident), the Arkane/GFx post-process shader
   families, and the Scaleform main menu pending the `middleware.md` decision.
5. `open` a mission map; player spawns; input works. **NEARLY DONE 2026-09-26 (wave 4)**: the null-RHI run
   reaches `Initial startup` in 7.7 s, renders frames, and commits the map change into the mission map
   (`Committed map change via DishonoredEngine`) with no critical error anywhere; the player controller
   possesses its pawn and the tweak chain applies the pawn's own tweak set. The possessed/input-moved
   evidence is package AF, in flight at the time of writing.
6. Retail savegames load; save/load round-trip.
7. Full campaign; then DLC05/06/07 (needs Phase 7).
8. Test suite: golden-log diffs (milestones 2–5), package load-all, save load-all, scripted
   flythrough on two maps to catch physics/animation drift.

### Phase 7 — Port 2012 → 2013 delta (runs alongside Phase 3, not after it)

The 2013 exe is the target, so this is not a final polish step: every function ported from the
2012 decompile is diffed against its 2013 counterpart before it is marked `verified`.

- [x] Name propagation (wave 1 J, own matcher `resources/tools/ida/match_functions.py`; Diaphora too slow):
      82.8 % matched → `resources/docs/idb/retail2013_named.i64`, `symbols/match_2012_2013.csv`, `functions_2013.csv`.
- [~] Classify identical / changed / new: `symbols/match_2012_2013.md` (matched / unmatched), package delta
      `types/script_delta_2012_2013.md` (401 classes added, 98 removed, 258 changed), `native_class_sizes.md`
      (274 size changes, 345 2013-only classes); per-function identical/changed classification still to do.
- [~] Decompile changed/new functions and fold the behavior in: done for everything ported so far
      (every wave-1/2 port cites a 2013 rva); the systematic pass is Phase 3's per-function work.

### Phase 8 — Online / "server" removal

- [ ] Find every `curl_*` call site in the 2013 database; delete the feature.
- [ ] Offline `OnlineSubsystemSteamworks` path by default; leaderboards return empty;
      achievements log locally.
- [ ] Keep `IpDrv` / `IpNetDriver` compiling: the built-in UE3 client/server path that `dismod`
      multiplayer will use.

### Phase 9 — Automation

- [x] `resources/tools/ida/`: export scripts + headless batch decompile (`decompile_funcs.py`) + matcher.
- [x] Per-function status: `resources/docs/function_status.csv` + `progress.md`; per-agent packages and
      reports in `PHASE<n>.md` / `agents/agent<X>.md` (the parallel-session workflow used since Phase 2).

### Phase 10 — Polish, late (audio, cosmetics)

Deliberately last, by the user's decision on 2026-09-26: **audio is polish and is not needed to get the game
playable.** The Wwise bindings and the silent backend already in the tree are enough for bring-up — they load
the real banks, resolve the real event ids and keep the audio calls traceable — so nothing here blocks a
milestone. Do not spend a wave on it until the game plays.

- [ ] Real audio, either route (`agents/agentAN.md` sections 1, 3 and 7 have the detail):
      (a) install the licensed Wwise 2012.1 SDK and set `DISHONORED_WWISE_SDK=<path>`, then restore the nine
      plug-in and codec factories, give the sink the viewport window handle, and check the CRT mismatch; or
      (b) write an evaluator for the bank logic (action lists, attenuation and RTPC curves, state and switch
      containers, the music hierarchy) on top of the AKPK and BKHD readers that already work.
- [ ] Tick the audio device (`UClient::GetAkAudioElement` slot, `UWindowsClient::Tick`) and fill
      `UDishonoredAudioSystem::RegisterAmbientSound` / `Update` / `ConsumeEndOfEventNotifies` /
      `PostAkEventAtPoint`, all decompiled; `UAkAudioDevice::ApplyGameSettings`; the matinee Ak tracks.
      Until then `PostEvent` is never reached, which is why the silent backend logs banks but no events.
- [ ] FaceFX (`WITH_FACEFX=0`): facial animation, same character — cosmetic, and its SDK is not in the tree.
- [ ] Cosmetic rendering left over from wave 4: the Arkane and GFx post-process shader families
      (136 undeclared types), Arkane bloom, DisFog.

## 5. Risks and open questions

* **2012 ≠ 2013 layouts** — largely retired: Core/Engine headers are on the retail sizes and offsets
  (0 contract mismatches, 4 SDK rows left in GameFramework/IpDrv bases). Still provisional: native-only
  members whose position is inferred from the 2012 PDB inside retail gaps, and every 2012 decompile
  not yet diffed against its 2013 counterpart.
* **Shared working tree + junctions.** Agents edit one tree; merges are verified on a clean worktree.
  A recursive delete through staging junctions destroyed the retail content once (2026-09-25); staging
  no longer uses links and nothing may recursively delete a directory that could contain one.

* **Legal**: the 2012 build, its PDBs and the reference engine source are all leaked material;
  UE3 is Epic-licensed and Scaleform/Bink/PhysX/FaceFX/Wwise are licensed middleware. Keep the
  repo private; decide the distribution model before publishing anything.
* **Version drift 9014 ↔ 10897**: the reference is newer than Dishonored's engine. Serialization
  paths (`Ver() < VER_*`), name tables, native indices and class layouts must be pinned to
  Dishonored's values, not the reference's. Phase 2's layout delta and the per-function
  `Serialize` diff are the guards.
* **Scale**: DishonoredGame (25 % of code) is still written from scratch; Engine convergence is
  bounded by the 59 % of functions without a same-named reference definition, many of which are
  script thunks.
* **Silent layout drift**: generated `static_assert`s plus the load-all test must exist before
  Engine convergence starts.
* **Behavioral drift from modernization**: `/fp:precise`; deterministic containers where script
  observes order; flythrough regression test.
* **Scaleform (10.4 %) and Wwise (4 %)** have no source anywhere; their Phase 4 decision gates
  GFxUI and AkAudio.

## 6. Next concrete steps (2026-09-25, wave 3 landed)

1. Finish wave 4: package AF (milestone 5 evidence) and its `ADishonoredPlayerPawn::execPlayDying_Native`.
2. Wave 5, from the follow-ups in `PHASE6.md` "Wave result": Arkane anim nodes (the tweak anim tree is gated
   behind `-distweakanimtree` until they exist), the 275 DishonoredGame stubs behind the AI brain /
   sub-process / item-context classes, the Arkane and GFx post-process shader families, a Release or
   `FMallocBinned` build for long d3d9 runs, the ~100 remaining Engine/GameFramework shim classes, the
   retail nav-mesh runtime, and the whole-tree Edge path.
3. Load-all test over all 471 `.upk` / every `.pck` (milestone 3 exit check), now that `-loadall` exists.
4. Phase 4: obtain PhysX 2.8.4 / Wwise 2012.1 / Steamworks 1.18 SDKs (the last 3 unported natives on the
   path are Steamworks `Read*`); decide Scaleform (`middleware.md`).
