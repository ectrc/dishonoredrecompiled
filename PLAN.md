# Dishonored (UE3) Recompilation Project Plan

Written 2026-09-24, revised same day. Working root: `D:\RecompileDishonored\Recompile\`.

## 1. Goal

Rebuild Dishonored's native executable from reconstructed C++ source so that a self-compiled
`DishonoredGame-Shipping.exe` loads the retail cooked content, plays the full game plus DLC,
and can be modified freely (offline play, multiplayer via `dismod`, no dependence on any dead
online service).

This is a **functional** rebuild, not a matching one. Nothing in this plan tries to reproduce
the original machine code. That decision drives everything below:

* Modern toolchain (MSVC 2022, C++17). No VS2008, no original compiler flags, no asm diffing.
* Decompiler output is a *reference*, never the deliverable. Code gets rewritten into clean,
  readable C++ as it is understood. Function boundaries, inlining and file placement are free.
* Middleware is replaced with open-source equivalents wherever the *data* stays compatible.
  Only the cooked content format and the UnrealScript bytecode contract are sacred.
* Behavior, not bytes, is the test: same log output, same packages load, same saves load,
  same gameplay.

Working assumption on "server shut off": the retail exe has no HTTP endpoints in its strings
besides driver-download URLs. Its online-facing pieces are Steam (`steam_api.dll`), Steam
leaderboards for DLC05 (Dunwall City Trials), and a `libcurl.dll` import that only exists in the
2013 build. Phase 8 pins down exactly what that curl usage is and removes it. If the "server"
means something else, adjust Phase 8 only.

## 2. What we have (verified inventory)

### 2.1 `Dishonored_Latest2026\` — retail 2013 build, no symbols

| Item | Value |
|---|---|
| `Binaries\Win32\Dishonored.exe` | 18,041,856 bytes, Shipping config, **no PDB** |
| Build | engineVersion 9411, changelist 334700, 2013-08-20 |
| Branch (from RSDS path) | `...\Dishonored_Campfire_8790\` (Perforce workspace of aarbona) |
| Imports | libcurl, steam_api, dinput8, xinput1_3, d3d9, wsock32, binkw32, MSVCR90/MSVCP90 |
| Source-path strings in exe | 66 unique files (assert/check paths) across Core, Engine, D3D9Drv, Launch, DishonoredGame |
| Content | `CookedPCConsole` 1463 files / 5.4 GB, INT only; `DLC\PCConsole\DLC05,06,07` |
| Config | `DefaultEngine.ini` already edited: `bEnableSteam=false` |
| Extra exes | `DishonoredGame-Shipping.exe` here is a byte-identical copy of the 2012 one (same PDB GUID `a172ba00...`). `Dishonored_Speedrun.exe` is from branch `UnrealEngine3DLC07`. `Dishonored_dump*.exe` are unpacked dumps. |
| RE state | `Dishonored.exe.i64` (183 MB) and `Dishonored_dump_SCY.exe.i64` (304 MB) IDA databases already exist |
| Mod hook | `_dinput8.dll` = `dismod` build (imgui overlay, Steam bypass, spawn panel) |

### 2.2 `Dishonored_Debug2012\` — leaked QA build, full symbols

Not actually a Debug-config build. These are Release / Shipping / Profile configs shipped with
their PDBs. No true `DishonoredGame-Debug.exe` exists (only its `.exe.config`).

| Exe (Win32) | Size | Config | PDB | GUID match |
|---|---|---|---|---|
| `DishonoredGame.exe` | 61.9 MB | Release, editor+game (links wx, nvtt, EasyHook, mscoree) | 280 MB | yes |
| `DishonoredGame-Shipping.exe` | 16.5 MB | Shipping | 98 MB | yes |
| `DishonoredGame-Shipping-DC.exe` | 17.4 MB | Shipping + debug console | 110 MB | yes |
| `DishonoredGame-ArkProfile.exe` | 27.6 MB | Profiling | 163 MB | yes |

* Build: engineVersion 9014, changelist 254295, 2012-06-20, branch `//dishonored/UnrealEngine3QATest`.
* Original source root: `V:\dishonored\UnrealEngine3QATest\Development\Src\`.
* Win64 exes for all four configs exist but have **no PDBs**.
* UE3 tool binaries: UnrealFrontend, CookerSync, UnrealConsole, MemoryProfiler2, StatsViewer,
  UE3ShaderCompileWorker, UnSetup. Managed editor DLLs (`Binaries\Win32\Editor\`) are **missing**,
  so `DishonoredGame.exe` editor mode will not start as-is.
* Content: `CookedPCConsole` 5408 files / 9.6 GB, seven languages. No DLC.
* A previous run (2026-03-05, from `D:\DishonoredDebug\`) died with
  `Failed to find file for package Core for async preloading` — a `Paths=` / working-directory
  problem in the generated `DishonoredEngine.ini`, not a binary problem. Fix in Phase 0 so the
  symbolized build runs as a behavioral reference.

PDB module inventory (from `DishonoredGame.pdb`, 3733 unique source files, saved to
`docs/pdb_srcfiles.txt`):

| Runtime modules (linked into Shipping, verified in Phase 1) | Not in Shipping (editor-only or other configurations) |
|---|---|
| Core, Engine, GameFramework, IpDrv, Launch, WinDrv, D3D9Drv, AkAudio, GFxUI, OnlineSubsystemSteamworks, DishonoredGame, DisJobs, Edge (18 functions, PS3 leftovers) | UnrealEd, UnrealEdCLR, DishonoredEditor, GFxUIEditor, UnrealSwarm, EdgeTool, XAudio2, OnlineSubsystemPC |

External libs referenced by the PDB: FaceFX, FCollada 3.05b, PhysX (2.8.x + APEX), FBX,
wxWidgets, Wwise, Scaleform GFx, Steamworks, Tootle 2.2, LZOPro, libpng, zlib, TinyXML,
PathEngine, EasyHook, NVTT 2.0.6, NVAPI, Bink, SpeedTree, MG remote debugger, libogg, libvorbis,
ConvexDecomposition, NvTriStrip.

### 2.3 Related assets outside this folder

* `D:\DishonoredMapMaking\` — DFSDK (community modding kit): UDK 2010-08 editor (engine 7026)
  plus UE Explorer-decompiled UnrealScript: `DishonoredGame` 228 `.uc`, `Engine` 1377 `.uc`,
  `Core` 9 `.uc`. Documents class properties, enums and native function signatures.
* `D:\Christmas\github\dismod\` — own dinput8 proxy (CMake, ~850 lines: engine object access,
  Steam hook, spawn/world mods, imgui render). References
  `CodeRedModding/UnrealEngine3` SDK headers for UE3 object layouts.

### 2.4 Toolchain on this machine

Present and sufficient: VS 2022 (MSVC v143), clang-cl (LLVM, also gives `llvm-pdbutil`), CMake,
Python 3.13, git, IDA 8.3, IDA 9.1 (idalib MCP available), FModel MCP.
Missing but wanted: ninja (faster builds), vcpkg (open-source deps). No GNU `strings` (use Python).
No legacy compiler is needed.

## 3. Strategy

x86 Win32 has no mature static recompiler, and a translated binary would be unmodifiable anyway,
so this is **decompilation-guided reimplementation**. The 2012 Shipping exe with full PDB is the
primary source of truth because every function has a name, a source file and a line number, and
types are recoverable. The 2013 retail exe is the final content target; its behavioral delta from
2012 is ported in Phase 7.

Principles that follow from "no matching":

1. **Prefer existing code over decompiling.** Open-source middleware (zlib, libpng, libogg,
   libvorbis, TinyXML, LZO, EasyHook) goes in as-is. Public UE3 header knowledge (UDK 2010,
   CodeRedModding SDK) seeds the class layouts.
2. **Decompile to understand, then write.** Raw Hex-Rays output lives in `reference/` (gitignored
   or a separate private branch). `Development/Src` only contains code someone has read,
   understood and rewritten with real types and names.
3. **Compatibility contracts are data, not code.** What must be exact:
   * `FArchive` serialization of every `UObject` subclass and `Serialize()` override (cooked
     packages, `.tfc`, savegames, `.bin` shader caches).
   * `UProperty` offsets and sizes of native classes as verified by `UStruct::Link` against the
     C++ layout — so member order and sizes in native classes must match what the cooked script
     classes expect. Padding/alignment must match too, but that comes from the type layout, not
     from the compiler version.
   * The `exec*` native function table and `FName` hardcoded name indices (`UnNames.h`).
   * Floating-point behavior in physics/animation only to the degree that saves and scripted
     sequences still work (`/fp:precise`, no `/fp:fast`).
   Everything else (memory allocator, threading, renderer internals, string handling, container
   implementations) may be modernized or replaced.
4. **Replace, wrap or stub proprietary static code by cost.** Scaleform GFx (UI), FaceFX
   (facial anim), PathEngine (navmesh), SpeedTree — each gets a decision in Phase 4: decompile
   because the cooked data needs it, reimplement a subset, or keep the original exe's code via
   a hybrid link during bring-up.
5. **Modern toolchain from day one.** MSVC 2022, `/std:c++17`, `/W4`, ASan available for
   bring-up. Win32 x86 first (fewest layout surprises). x64 becomes possible later since
   `UStruct::Link` recomputes offsets at load and the Win64 2012 exes prove the content loads
   under 64-bit pointers.

Fallback / incremental value: `dismod`-style hooking against the retail exe stays viable at
every stage and is how features get prototyped before the rebuilt exe can run them. Each finished
module (Core, then Engine) is directly usable as a typed SDK for `dismod` long before the game
links.

## 4. Phases

### Phase 0 — Project setup (days)

- [ ] `git init` in `Recompile\`. `.gitignore` everything under `../Dishonored_*`, all `.exe`,
      `.dll`, `.pdb`, `.i64`, `.id0-2`, `.nam`, `.til`, `.upk`, `.pck`, `.tfc`, and `reference/`.
      Game binaries, PDBs, cooked content and raw decompiles never enter the repo.
- [ ] Layout:
  ```
  Recompile/
    PLAN.md
    docs/            symbol dumps, module maps, decisions, progress
    tools/           Python + IDAPython scripts
    reference/       raw decompiler output per module (gitignored)
    external/        open-source deps (vcpkg manifest or submodules)
    Development/Src/<Module>/{Inc,Src,Classes}   rewritten source
    cmake/  CMakeLists.txt  CMakePresets.json
  ```
- [ ] Record SHA-256 of every exe/pdb in `docs/binaries.md`.
- [ ] Get the symbolized 2012 build running as a reference: fix `Paths=` in
      `Dishonored_Debug2012\DishonoredGame\Config\DishonoredEngine.ini` (or delete generated
      `Dishonored*.ini` and relaunch from `Binaries\Win32\`) until
      `DishonoredGame-Shipping-DC.exe` reaches the main menu. Keep its `Launch.log` as the
      golden log. Also capture `Launch.log` from the 2013 retail exe.
- [ ] Install ninja and vcpkg. Bootstrap an empty CMake project that builds a Win32 `hello`
      with the flags we intend to use (`/std:c++17 /fp:precise /W4 /permissive-`).

### Phase 1 — Symbol and type database — DONE 2026-09-25

Tracker: `docs/PHASE1.md`. Regeneration commands and caveats: `docs/symbols/README.md`.
Exit check: `python tools/symbols/verify_phase1.py`.

- [x] IDA database verified (`docs/idb/shipping2012_v1.i64`, PDB names on 99.6 % of 66,394
      functions, 63,011 local types) — `docs/symbols/db_verify.txt`.
- [x] `docs/symbols/functions.csv` (address, size, names, file, line, module, origin, compiland),
      `globals.csv`, `imports.csv`, `segments.csv`.
- [x] `docs/types/sizes.csv` (+ regenerable `types.json`, `all_types.h`), `vtables.csv` (regenerable).
- [x] Line info and module attribution via the DIA SDK (`tools/pdb/dia_dump.py`; llvm-pdbutil
      cannot read these PDBs). 100 % of functions attributed through section contributions.
- [x] `docs/module_map.md`: per-module and per-library sizes (Scaleform 10.4 %, Wwise ~4 %).
- [x] `docs/symbols/natives.csv`: 2,165 `exec*` natives, 1,992 with their GNatives index
      (numbered ones recovered from `GRegisterNative` call sites and the inlined Core stores);
      `classes.csv` (1,036 `StaticClass`); cross-check against DFSDK `.uc` in `natives_xcheck.md`.
- [x] `docs/symbols/hardcoded_names.csv`: 499 hardcoded FNames (index 0 = `None`, sparse up to 1300).
- [x] Golden logs in `docs/golden/` (2012 ArkProfile build reaches the main menu; Shipping
      configs write no log at all).

### Phase 2 — Source skeleton and type headers (1–2 weeks)

- [ ] Create `Development/Src/<Module>/Inc` and `Src` for each runtime module. Use
      `docs/pdb_srcfiles.txt` as a *guide* to file naming, not a mandate; merge or split files
      as makes sense.
- [ ] Copy DFSDK `.uc` files into `Development/Src/<Module>/Classes/` for reference (cooked
      packages already contain bytecode; these are not compiled).
- [ ] Pull public UE3 reference material into `reference/` (not compiled): CodeRedModding
      UnrealEngine3 SDK headers, UDK 2010 headers if obtainable. Record in `docs/legal.md`
      which references are acceptable to consult. Leaked engine source, if any, stays out of the repo.
- [ ] Write per-module `Inc/*.h` from the Phase 1 type export. Core types (`UObject`, `UClass`,
      `UProperty`, `FName`, `FArchive`, `TArray`, `FString`, `TMap`) get modern, readable
      implementations whose *layout* matches the exported types where cooked data or script
      classes depend on it, and whose *implementation* is free (e.g. `TArray` may be a thin
      wrapper with the same 12-byte header, but its growth policy is ours).
- [ ] Add a `static_assert(sizeof(X) == N)` for every native class the script side references,
      generated from `docs/types/`. This is the mechanical guard that replaces byte matching.

### Phase 3 — Decompile-and-rewrite pipeline (largest phase, months)

Module order (dependency order; Shipping-only, editor modules out of scope):

Sizes from `docs/module_map.md` (Phase 1). The Shipping exe is the `Win32-OSSSteamworks`
configuration: there is no OnlineSubsystemPC and no XAudio2 module in it; audio is Wwise via AkAudio.

1. Core — 6,577 functions, 1.19 MB
2. Engine — 22,892 functions, 4.72 MB
3. GameFramework — 517 functions
4. IpDrv, OnlineSubsystemSteamworks — 375 functions
5. WinDrv, D3D9Drv — 355 functions
6. AkAudio — 216 functions (+ the Wwise static libraries behind it)
7. GFxUI — 1,041 functions (+ the 5,635-function Scaleform runtime behind it)
8. DishonoredGame, DisJobs — 21,799 functions, 2.8 MB
9. Launch — 67 functions

Per module:

- [ ] `tools/decomp_module.py <Module>`: batch Hex-Rays decompile every function whose PDB
      source file belongs to the module into `reference/<Module>/<File>.cpp`, header comment
      `// 0x<addr> <name>`. This runs once and is regenerated on demand; it is never edited.
- [ ] Rewrite into `Development/Src/<Module>/Src/*.cpp` in priority order: constructors and
      `StaticConstructor`, `Serialize`, `exec*` natives, virtuals in vtable order, then the rest.
      Use real UE3 API names and types; drop decompiler artifacts; simplify where the intent
      is clear. Where the original does something only the old compiler needed (manual
      inlining, hand-unrolled loops, SSE intrinsics that MSVC 2022 auto-vectorizes), write the
      plain version.
- [ ] Compile each module as a static lib as soon as it parses; link errors drive the next
      function to write. Track progress in `docs/progress.md` (functions rewritten / total per
      module, plus "stubbed" count so stubs are not mistaken for done).
- [ ] Third-party code inside the exe: identify first with FLIRT/Lumina and known-library
      signatures (zlib, libpng, vorbis, LZO, TinyXML, PhysX inline headers). Anything matched is
      replaced by the open-source version, never rewritten by hand.

### Phase 4 — Third-party dependencies

Decision per library, recorded in `docs/deps.md` once Phase 1 gives sizes:

| Library | In Shipping exe? | Plan |
|---|---|---|
| PhysX 2.8.x / APEX | DLLs shipped | Link to shipped DLLs via import libs generated from exports; use PhysX 2.8.4 SDK headers. Later option: port to PhysX 3/4/5 if cooked collision data can be re-cooked — not for bring-up. |
| Bink | `binkw32.dll` shipped | Import lib from DLL exports; header reconstructed from usage (small API surface). Later option: replace with FFmpeg + re-encoded movies. |
| Steamworks | `steam_api.dll` shipped | Steamworks SDK of the matching interface version (check `SteamClient0xx` strings). Offline path is the default. |
| libcurl | `libcurl.dll` shipped (2013 only) | Remove entirely (Phase 8). |
| zlib, libpng, libogg, libvorbis, TinyXML, LZO (replaces LZOPro), EasyHook | static | vcpkg. LZO must decompress the same streams as LZOPro (it does; LZO1X format). |
| Scaleform GFx 3.x (`libgfx`, `libgfx_ime`) | static, **10.4 % of code** (5,635 functions, 1.18 MB) | Largest proprietary block, sized in Phase 1. Decide: (a) rewrite from decompile; (b) hybrid-link the original code during bring-up and replace later; (c) reimplement a subset sufficient for Dishonored's `.gfx` UI. No open alternative loads the shipped assets as-is. |
| FaceFX (`facefx`, `fxsdk_unreal`) | static, 1.3 % (1,478 functions) | Runtime part only (editor plugins irrelevant). Rewrite from decompile; the data format is fixed by cooked animsets. |
| Wwise (`aksoundengine`, `akmusicengine`, `akstreammgr`, `ak*fx`, `akvorbisdecoder`) | static, **~4 % (≈3,300 functions)** — it *is* linked into Shipping; `AkAudio` is the engine-side module | Wwise SDK is licensed; the shipped banks (`.bnk`/`.pck`) need the matching runtime. Options: rewrite from decompile, or hybrid-link during bring-up. XAudio2 is still used for movies/voice. |
| PhysX double-buffered scene (`libnxdoublebuffered_release`) | static, 0.9 % | Part of the PhysX 2.8 SDK (statically linked helper); comes with the SDK headers. |
| PathEngine | **not present in Shipping** (only referenced by the Release/editor PDB) | Nothing to do at runtime; navmesh queries are engine code. |
| SpeedTree | not present as a library in Shipping | Stub unless maps contain SpeedTree actors; check cooked packages first with FModel. |
| LZOPro (`lzopro`), zlib, libpng | static | Replace with open LZO / zlib / libpng (data compatible). |
| DirectX 9 / XAudio2 / XInput | system | Windows SDK headers where available; June 2010 DirectX SDK for D3DX9. Later option: D3D11 RHI. |

### Phase 5 — Build system

- [ ] CMake + ninja, MSVC 2022, `/std:c++17`, Win32 x86 for bring-up. x64 as a second preset
      once Core/Engine link, gated on all `static_assert` layout checks passing under 64-bit.
- [ ] Configs: Debug (checks, logging, ASan optional), Release (checks on), Shipping.
- [ ] Output `Binaries\Win32\DishonoredGame-Shipping.exe` into a staging copy of
      `Dishonored_Latest2026\` (junction the content folders; never build into the pristine tree).
- [ ] `tools/build_and_smoke.py`: build, launch with `-log`, wait for a milestone string, diff
      `Launch.log` against the golden log (normalized for timestamps/addresses).

### Phase 6 — Bring-up milestones (behavioral, not binary)

1. Links. Runs to `Init: Object subsystem initialized`.
2. Loads `Core.upk`, `Engine.upk`, `DishonoredGame.upk`, `Startup.upk`; UnrealScript VM runs
   `defaultproperties` and class construction without asserts. Test: batch-load all 471 `.upk`
   and every `.pck` with a commandlet-style `-loadall` switch and compare object counts with the
   reference build.
3. D3D9 device created; startup Bink movie and Scaleform main menu render.
4. `open` a mission map from console; player spawns; input works.
5. Retail savegames load; save/load round-trip inside the rebuilt exe.
6. Full campaign playable end to end.
7. DLC05/06/07 content (needs Phase 7).
8. Test suite: golden-log diffs for milestones 1–4, package load-all, save load-all, plus a
   scripted `-benchmark` flythrough on two maps to catch physics/animation drift.

### Phase 7 — Port 2012 → 2013 delta

- [ ] Diaphora or BinDiff between `DishonoredGame-Shipping.exe` (2012, named) and
      `Dishonored.exe` (2013). Propagate names into the existing `Dishonored.exe.i64`.
- [ ] Classify functions: identical / changed / new. Expect DLC additions
      (`DisDLC05MoviePlayerLeaderboard`, `Req_DLC05_*` natives), engine fixes, and the curl use.
- [ ] Decompile only changed/new functions from the 2013 binary into `reference/2013/` and fold
      the behavior into the single source tree. One tree targets the 2013 content; the 2012
      content stays loadable only as a test fixture.
- [ ] Use the 66 source-path strings in the 2013 exe as anchors (they name file and line).

### Phase 8 — Online / "server" removal

- [ ] In the 2013 IDA database: xrefs to every `curl_*` import → what is fetched, when, and what
      happens on failure. Delete the feature in the rewrite; nothing in the game should wait on
      the network.
- [ ] OnlineSubsystem: `OnlineSubsystemPC` is the default (already `bEnableSteam=false` in the
      current `DefaultEngine.ini`). Leaderboard reads return empty; achievements log locally.
- [ ] Keep `IpDrv` + `IpNetDriver` intact and compiling: this is the built-in UE3 client/server
      path that `dismod` multiplayer will use. Once the game compiles, `dismod` features move
      from hooks into real source changes.

### Phase 9 — Automation with Claude / IDA MCP

- [ ] `tools/ida/`: IDAPython scripts for export (Phase 1) and batch decompile (Phase 3), runnable
      headless via idalib so the MCP server can drive them.
- [ ] Per-module task files in `docs/tasks/<Module>.md` listing functions with status
      (todo / stubbed / rewritten / verified), so work can be parallelized across sessions and
      agents without re-deriving state.

## 5. Risks and open questions

* **Legal**: the 2012 build and its PDBs are leaked. UE3 is Epic-licensed; Scaleform, Bink,
  PhysX, FaceFX, PathEngine are licensed middleware. Keep the repo private; decide the
  distribution model (patch against owned retail files, `dismod`-style DLL, or source that
  requires the user's own content) before publishing anything.
* **Scale**: 66,394 functions / 11.4 MB of code in Shipping, 2,383 source files with line info
  (Phase 1 measurement). Engine 41 %, DishonoredGame 25 %, Core 10 %. Rewriting rather than
  transcribing makes each function slower but the result smaller and maintainable. Module
  ordering ensures each finished module is useful on its own.
* **Silent layout drift**: without byte matching, the only guard against a wrong struct layout
  is the generated `static_assert` set plus the load-all package test. Both must exist before
  Engine work starts, not after.
* **Behavioral drift from modernization**: replacing allocator, containers or math with
  different implementations can change iteration order, float rounding, or timing. Rules:
  `/fp:precise`, deterministic containers where script or save data depends on order (`TMap`
  iteration order is observable to UnrealScript), and the Phase 6 benchmark flythrough as
  regression test.
* **Scaleform GFx** is the biggest block of proprietary code: 10.4 % of all code (Phase 1
  measurement, see `docs/module_map.md`). Wwise is another 4 %. Pick the Phase 4 option for both
  before touching GFxUI / AkAudio.
* **Win64**: the leaked Win64 exes have no PDBs; they are only evidence that 64-bit works, not a source.
* **"Server"** definition — see Section 1 assumption.

## 6. First concrete steps (this week)

1. Phase 0 repo layout, `.gitignore`, `docs/binaries.md` hashes, CMake hello-world with the
   intended flags, ninja + vcpkg installed.
2. Fix the 2012 build's config paths; capture golden `Launch.log` from
   `DishonoredGame-Shipping-DC.exe` and from the 2013 retail exe.
3. Open `DishonoredGame-Shipping.exe.i64` in IDA 9.1; confirm PDB symbols are applied; run the
   Phase 1 export scripts and commit `docs/functions.csv`, `docs/types/`, `docs/module_map.md`.
4. Generate the `static_assert` layout header from `docs/types/`.
5. From `module_map.md`, size Core and GFxUI; start Phase 3 on Core.
