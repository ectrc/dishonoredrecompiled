# Project status — 2026-09-25 (evening: Phase 2 complete)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `resources/docs/PHASE1.md`
(done), `resources/docs/PHASE2.md` (in progress). Decisions and fixes: `resources/docs/porting_notes.md`.

## Target

**Retail 2013 build** (`Dishonored_Latest2026`, engine 9411, DLC05–07) is what we rebuild. The
2012 symbolized build is a helping hand only: names, types, decompiles. Many structs differ in
size and member set between 2012 and 2013; everything derived from the 2012 PDB (the current
Core layouts, `sizes.csv`, `types.json`, the contract-type asserts) is **provisional** until
checked against the retail exe / retail packages (PLAN.md Phase 2b).

## Where we are

Phase 2 (reference import and layout convergence) is **complete** (`verify_phase2.py` 18/18, `CoreSmoke` 67/67). Engine compile (Phase 3 start) is in progress.

| Area | State |
|---|---|
| Reference tree | `../UnrealEngine3` (UE3 build 10897). Modules imported into `source/Development/Src/`: Core, Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, GFxUI, OnlineSubsystemSteamworks, Launch. Skeletons for DishonoredGame (1,043 files), AkAudio, DisJobs. |
| Build | `resources\run-vcvars.cmd x86-debug` / `x86-release`. **Core.lib compiles in both presets**, `LayoutProbe` and `CoreSmoke` run green. `Engine` builds behind `-DDISHONORED_ENABLE_ENGINE=ON` (499 units; NetIndex fallout being fixed). `/Zp4` per target. `Launch` is a stub exe. |
| Switched off for now | `WITH_FACEFX=0`, `WITH_APEX=0`, `WITH_STEAMWORKS=0`, `WITH_GFx=0`, `WITH_LZO=0` (missing SDKs, Phase 4; **LZO is needed for package loading**: all packages are COMPRESS_LZO). PCH off. DirectX 9 + libpng come from the reference tree (`cmake/ReferenceExternals.cmake`), zlib via FetchContent. |
| Versions pinned | `Core/Src/UnObjVer.cpp`: engine 9411, package 801, licensee 30, cooked content 133. `Core/Inc/UnNames.h` regenerated from the 499 hardcoded names (+69 reference-only names ≥ 1301). |
| Symbol data | `resources/docs/symbols/*` (functions, natives with all folded indices, hardcoded names, licensee branches, opcodes, package summaries), `resources/docs/types/*` (sizes, member delta), `resources/docs/reference_xref.csv` (Core 89 % / Engine 52 % of functions have a reference definition). |
| Layout probe | `reference_layout_delta.md`: 304 Core types, 269 exact, all 38 contract types exact. Arkane layout deltas applied to Core headers (see PHASE2 P2.5). `DishonoredLayouts.h` (277 asserts) active in Core.h. |
| Opcodes / natives | Dishonored's bytecode opcodes match the reference (differences are COMDAT folding artifacts). Core numbered natives: no real mismatches. |
| Serialization | `serialization_delta_core.md`: 98 Core functions vs reference, 24 to port (UClass::Serialize, UScriptStruct::SerializeBin, CreateLoader+ArkBsPatch, LZO package chunks, ...). `function_status_seed.csv` seeds Phase 3 status. |
| Arkane Core code | bzip2 decompressor + bspatch + TPool implemented from decompile and verified (`agents/agentC.md`). |
| Class headers | `gen_classes_header.py` emits UE3-style `*Classes.h` from the PDB; `dishonoredgame_class_inventory.md`: 1,485 DishonoredGame classes, 220 with DFSDK `.uc`. |

## Next (Phase 3)

- Phase 2b first: recover the 2013 retail layouts (script property offsets and class sizes from
  the 2013 cooked packages; native sizes from the 2013 exe) and re-verify the Core contract types
  against them before Engine convergence.
- Then the Engine layout probe (`gen_layout_probe.py generate Core Engine`) against 2013 numbers
  and Engine contract types (AActor, UWorld, ULevel, USkeletalMesh..., see `reference_member_delta.md`).
- Port the 24 Core serialization functions from `serialization_delta_core.md` (UClass::Serialize
  first). Bring in an LZO1X decompressor (lzokay/LZO) and set `WITH_LZO=1`.
- Milestone 1: real `Launch` linking Core+Engine (+ stubs for missing modules) to reach
  `Init: Object subsystem initialized`.

## Known pitfalls

- Never open one IDA database from two processes (`resources/docs/idb/README` rule): headless scripts use
  `shipping2012_work.i64`; the MCP session uses `shipping2012_v1.i64`. Agents get their own copies.
- The Bash tool collapses `\\` in heredocs: write Python patch scripts to files, or use the Edit tool.
- CMake's `file(GLOB)` for module sources is `CONFIGURE_DEPENDS`; new files need a reconfigure.
- Build logs: `%TEMP%\claude\...\scratchpad\core_build*.log`, `probe_build*.log`.
