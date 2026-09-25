# Dishonored (UE3) Recompilation Project Plan

Written 2026-09-24, revised 2026-09-25 (Phase 0/1 complete; reference engine source added).
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
| RE state | `Dishonored.exe.i64` exists but is a plain auto-analysis (34,059 functions, 93 % unnamed) |
| Mod hook | `_dinput8.dll` = `dismod` build (imgui overlay, Steam bypass, spawn panel) |

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
   cooked packages (every script property carries its exact offset, every class its
   PropertiesSize), (b) the 2013 exe (class sizes in constructors/`StaticClass` registration,
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

Tracker `resources/docs/PHASE2.md`; exit check `resources/tools/symbols/verify_phase2.py` (18/18); status `resources/docs/STATUS.md`. Original task list kept below for reference.

- [ ] `resources/tools/symbols/xref_reference.py`: for each PDB function, look up a same-named definition
      in the reference; for each PDB source file, whether it exists there. Writes
      `resources/docs/reference_xref.csv` and refreshes the tables in `resources/docs/engine_reference.md`.
- [ ] Copy the Shipping runtime modules from the reference into `source/Development/Src/`: Core,
      Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, GFxUI, OnlineSubsystemSteamworks, Launch,
      zlib. Keep Epic's `Inc/Src/Classes` layout. Drop editor-only, console, mobile and
      D3D11/OpenGL code paths behind CMake options rather than deleting them.
- [ ] Add empty `DishonoredGame`, `AkAudio`, `DisJobs` modules with the file list from
      `resources/docs/symbols/sourcefiles.txt`.
- [ ] Generate `source/Development/Src/Core/Inc/DishonoredLayouts.h` from `resources/docs/types/sizes.csv`:
      `static_assert(sizeof(X) == N)` for every native class the cooked script side references.
- [ ] `resources/tools/symbols/xcheck_reference_types.py`: PDB member offsets vs the reference headers
      (parse `types.json`; compile-time probe of the reference via a generated `offsetof`
      program). Output `resources/docs/types/reference_layout_delta.md`: the exact list of Arkane layout
      changes per class. Fix headers first; nothing else compiles correctly until they match.
- [ ] Pin `UnObjVer.cpp` / `UnNames.h` to Dishonored: package file version and licensee version
      read from `Core.upk`; `hardcoded_names.csv` replaces the reference name list (indices
      must match, names 0–1300 sparse).
- [ ] Native registration: regenerate the `IMPLEMENT_FUNCTION` / `AutoInitializeRegistrants`
      lists from `resources/docs/symbols/natives.csv` and `classes.csv`.
- **Exit:** Core compiles as a static lib under the CMake build; `DishonoredLayouts.h` passes
      for Core types; `reference_layout_delta.md` exists for Engine and DishonoredGame types.

### Phase 2b — Retail (2013) layout truth (before Engine convergence)

Detailed plan and tracker: `resources/docs/PHASE3.md` (work packages H–N).

- [ ] `resources/tools/pdb/read_package_classes.py`: parse the 2013 cooked packages (LZO chunks,
      name/import/export tables, `UClass`/`UStruct`/`UProperty`/`UEnum`/`UFunction` exports).
      UE3 does not serialize property offsets or `PropertiesSize` (`UStruct::Link` recomputes
      them), so this yields the exact 2013 **member list, order and types** of every script
      class, its enums and native function indices — from which offsets follow deterministically.
      Do the same for the 2012 packages and diff.
- [ ] 2013 native class sizes: every `InitializePrivateStaticClass<X>` calls
      `UClass::UClass(ENativeConstructor, sizeof(X), …, L"<Name>", L"<Package>", …)`; the size is an
      immediate next to the class-name string xref, readable in the unnamed retail exe →
      `native_class_sizes.csv` (2012 vs 2013). DONE 2026-09-26 (package H).
- [x] Retail member **offsets**: `resources/tools/sdk/parse_codered_sdk.py` parses the CodeRed dump of
      the running retail exe (`Dishonored_DumpedSDK_Retail`) into `retail_sdk_layout.json`;
      `resources/tools/sdk/xcheck_sdk_layout.py build/<dir>/layout_probe.txt` checks every probed
      member offset and class span against it → `retail_sdk_delta.md` (2026-09-27: 1,132 types,
      666 exact, 234 to converge). This replaces the 2012-offset check wherever retail differs.
- [ ] Regenerate `DishonoredLayouts.h` and the layout probe against the **2013** sizes; fix the
      headers where 2012 and 2013 differ, citing the retail evidence in the
      `// DISHONORED(layout)` comment (`retail:` prefix).
- [ ] Exit: every Core contract type and every Engine/DishonoredGame script class matches the
      2013 numbers (`gen_layout_probe.py compare` sizes **and** `xcheck_sdk_layout.py` offsets with
      0 contract mismatches); `verify_phase2.py` gains a `retail` section.

### Phase 3 — Module convergence and DishonoredGame rewrite (months)

Wave 1 (`resources/docs/PHASE3.md`, done 2026-09-27): retail truth, Core/Engine contract types
reconciled with retail, milestone 1. Wave 2 (`resources/docs/PHASE4.md`, started 2026-09-27):
milestone 2 (startup packages), D3D9Drv target, Engine headers converged on the retail SDK
offsets (agents Q/R/S), DishonoredGame/GFxUI/AkAudio/OSS registrants + headers from the SDK dump
(T), middleware versions + Phase 4 memo (U).

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

- [ ] Make the reference copy compile and link with MSVC 2022 (`/permissive-`, C++17). Fix
      compile errors mechanically; do not change behavior in this step.
- [ ] `resources/tools/decomp_module.py <Module>`: batch Hex-Rays decompile of every PDB function in the
      module into `resources/reference/<Module>/<File>.cpp` (gitignored, regenerated on demand).
- [ ] Convergence pass in priority order: `Serialize`, constructors / `StaticConstructor`,
      `exec*` natives, virtuals in vtable order, then the rest. For `reference` functions diff
      decompile vs source and port behavior differences; for `missing` functions write them.
      Record status per function in `resources/docs/progress.md`.
- [ ] Third-party code inside the exe: identify with FLIRT/Lumina first; anything matched is
      replaced by the library, never rewritten.

DishonoredGame specifics: generate the class declarations (`DishonoredGameClasses.h` equivalent)
from the retail SDK dump (`retail_sdk_layout.json`: 1,870 classes / 665 structs with retail offsets,
sizes, flags and `UnknownData` gaps for the native-only members) with `resources/docs/types/types.json`
(2012 PDB) supplying the names and types of what sits in those gaps, cross-referenced with the
DFSDK `.uc` for comments; exec-parameter structs and `FunctionFlags` for the `event`/`exec`
wrappers come from the dump's `*_parameters.hpp`; then rewrite natives and native classes in dependency order (`DisGlobalEnums`, items,
pawns, AI brain processes, powers, UI last).

### Phase 4 — Third-party dependencies

| Library | Status | Plan |
|---|---|---|
| PhysX 2.8.x / APEX | DLLs shipped; the reference tree only has **NovodeX 2.1.2** (`Development/External/Novodex`, no `NxCooking.h`), which cannot compile `UnNovodexSupport.h` | PhysX 2.8.4 SDK headers must come from elsewhere (wave-2 agent U decides); link to the shipped DLLs via import libs. |
| DirectX 9 | headers in the reference tree | June 2010 DirectX SDK for D3DX9 at build time. |
| LZO1X decompressor (replaces LZOPro) | **required for milestone 3**: every cooked package is `COMPRESS_LZO`; Dishonored calls `lzopro_lzo1x_decompress_safe` (LZO1X-compatible) | lzokay (MIT) or LZO 2.10 (GPL) via FetchContent; `WITH_LZO=1` |
| zlib, libpng, libogg, libvorbis, TinyXML | zlib/libpng already via FetchContent; libogg/libvorbis not linked in Shipping (audio is Wwise) | Data compatible; no decompile. |
| Steamworks | `steam_api.dll` shipped; reference `OnlineSubsystemSteamworks` source | Steamworks SDK of the matching interface version; offline path default. |
| Bink | `binkw32.dll` shipped | Import lib from DLL exports; small reconstructed header. |
| libcurl | 2013 exe only | Removed (Phase 8). |
| Scaleform GFx 3.x (`libgfx`, `libgfx_ime`) | static, **10.4 %** (5,635 fns); reference has only GFx-4 GFxUI glue (our GFxUI folder mixes it with the Arkane GFx-3 PDB stubs) | Decide (wave-2 agent U, `resources/docs/middleware.md`): rewrite from decompile / hybrid-link during bring-up / subset reimplementation. Biggest single decision. |
| Wwise (`ak*`) | static, **~4 %** (≈3,300 fns); reference has no Wwise | Same three options; shipped `.bnk`/`.pck` banks need the matching runtime. |
| FaceFX (`facefx`, `fxsdk_unreal`) | static, 1.3 %; reference has `Engine/FaceFX` glue only | Rewrite runtime from decompile; data format fixed by cooked animsets. |
| PathEngine, SpeedTree | not in Shipping | Nothing to do. |

### Phase 5 — Build system

- [ ] CMake + ninja, MSVC 2022, Win32 x86 preset (exists). One `add_library` per module
      mirroring `source/Development/Src/<Module>`; options for editor/console code paths (off).
- [ ] Configs: Debug (checks, logging, ASan optional), Release (checks on), Shipping.
- [ ] Output into a staging copy of `Dishonored_Latest2026\` (content junctioned; the pristine
      tree is never built into).
- [ ] `resources/tools/build_and_smoke.py`: build, launch with `-log`, wait for a milestone string, diff the
      normalized `Launch.log` against `resources/docs/golden/2012_arkprofile_launch.norm.log`.

### Phase 6 — Bring-up milestones (behavioral)

1. Reference Core + Engine + Launch compile and link with MSVC 2022 (UDK-style empty game).
2. Runs to `Init: Object subsystem initialized` with Dishonored's names/versions. **DONE 2026-09-26** (null RHI).
3. Loads `Core.upk`, `Engine.upk`, `DishonoredGame.upk`, `Startup.upk`; script VM runs
   `defaultproperties` without asserts. (Wave 2, `PHASE4.md` package O; needs T's registrants:
   `UClass::Bind` aborts on any native class without a registrant.) Test: load-all over all 471 `.upk` and every `.pck`,
   object counts compared with the reference build.
4. D3D9 device up; Bink startup movie and Scaleform main menu render.
5. `open` a mission map; player spawns; input works.
6. Retail savegames load; save/load round-trip.
7. Full campaign; then DLC05/06/07 (needs Phase 7).
8. Test suite: golden-log diffs (milestones 2–5), package load-all, save load-all, scripted
   flythrough on two maps to catch physics/animation drift.

### Phase 7 — Port 2012 → 2013 delta (runs alongside Phase 3, not after it)

The 2013 exe is the target, so this is not a final polish step: every function ported from the
2012 decompile is diffed against its 2013 counterpart before it is marked `verified`.

- [ ] Diaphora/BinDiff `DishonoredGame-Shipping.exe` (2012, named) vs `Dishonored.exe` (2013);
      propagate names into `Dishonored.exe.i64`.
- [ ] Classify identical / changed / new; expect DLC natives (`Req_DLC05_*`), engine fixes, curl.
- [ ] Decompile changed/new functions into `resources/reference/2013/` and fold the behavior into the
      single source tree.

### Phase 8 — Online / "server" removal

- [ ] Find every `curl_*` call site in the 2013 database; delete the feature.
- [ ] Offline `OnlineSubsystemSteamworks` path by default; leaderboards return empty;
      achievements log locally.
- [ ] Keep `IpDrv` / `IpNetDriver` compiling: the built-in UE3 client/server path that `dismod`
      multiplayer will use.

### Phase 9 — Automation

- [ ] `resources/tools/ida/`: export scripts exist; add batch decompile (`decomp_module.py`) runnable
      headless via idalib.
- [ ] `resources/docs/tasks/<Module>.md`: per-function status lists so work parallelizes across sessions.

## 5. Risks and open questions

* **2012 ≠ 2013 layouts.** Phase 2 converged Core on the 2012 PDB because that is the only
  build with symbols. Many structs differ in size and members between the two builds (DLC05–07
  natives, enum growth, `m_bShowMapNameOnlyOnXboxNoHDD`-style additions, the `xcheck_sdk.md`
  offset mismatches). Every layout must be re-verified against the retail exe / retail packages
  (Phase 2b) before Engine and DishonoredGame convergence relies on it; the 2012-derived
  headers are provisional until then.

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

## 6. Next concrete steps

1. `resources/tools/symbols/xref_reference.py` → `resources/docs/reference_xref.csv` (function and file status).
2. Copy Core from the reference; CMake `add_library(Core)`; make it compile with MSVC 2022.
3. `DishonoredLayouts.h` from `sizes.csv`; run it against the reference Core headers; fix.
4. Pin `UnObjVer.cpp` and `UnNames.h` to Dishonored's values.
5. Repeat 2–3 for Engine; then start milestone 1.
