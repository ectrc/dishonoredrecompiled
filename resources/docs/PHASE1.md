# Phase 0 + Phase 1 tracker

Status values: `todo`, `in-progress`, `blocked`, `done`. Dates are completion dates.
Acceptance rules are re-checked by `python resources/tools/symbols/verify_phase1.py` (exit 0 = phase complete).

## Phase 0 — Project setup

| ID | Task | Deliverable | Status | Date | Notes |
|---|---|---|---|---|---|
| P0.1 | Repo init and layout | `.gitignore`, `README.md`, dir tree, first commit | done | 2026-09-24 | commit `5e99803` |
| P0.2 | Binary manifest | `resources/docs/binaries.md` via `resources/tools/hash_binaries.py` | done | 2026-09-24 | 4 Win32 2012 exes GUID-match their PDBs; `Debug2012/Win32/Dishonored.exe` is the Shipping-DC build |
| P0.3 | Toolchain | ninja, comtypes, idalib activation, `resources/docs/toolchain.md` | done | 2026-09-24 | |
| P0.4 | CMake bootstrap | `CMakeLists.txt`, `CMakePresets.json`, `resources/run-vcvars.cmd`, `Launch.cpp` | done | 2026-09-24 | 32-bit exe builds and writes `Init: Object subsystem initialized` |
| P0.5 | Reference builds and golden logs | `resources/docs/golden/*.log`, `resources/docs/golden/README.md`, `resources/tools/normalize_log.py` | done | 2026-09-24 | Shipping/Shipping-DC write no log; ArkProfile build with `-seekfreeloadingpcconsole` reaches the main menu and is the golden. Retail 2013 log is savegame lines only |

## Phase 1 — Symbol and type database

| ID | Task | Deliverable | Status | Date | Notes |
|---|---|---|---|---|---|
| P1.1 | Verify and snapshot IDA db | `resources/docs/idb/shipping2012_v1.i64`, `resources/docs/symbols/db_verify.txt` | done | 2026-09-24 | PDB already applied: 66,394 functions, 0.38 % unnamed, 63,011 local types. Database is rebased to 0 (`va == rva`) |
| P1.2 | Function export | `resources/docs/symbols/functions.csv`, `segments.csv` | done | 2026-09-24 | |
| P1.3 | Line info via DIA | `lines.csv`, `pdb_functions.csv`, `compilands.csv`, `compiland_of.csv`, `sourcefiles.txt` | done | 2026-09-25 | llvm-pdbutil cannot read the PDB; DIA (`msdia140.dll`) used. 371,684 line records, 2,383 source files, 88,653 section contributions |
| P1.4 | Join → module attribution | `functions.csv` (+file,line,module,origin,compiland), `functions_nofile.csv`, `functions_nofile_summary.md` | done | 2026-09-25 | 100 % attributed (line records first, section contributions as fallback). 13,272 functions have no line records: static libraries (Scaleform, Wwise, PhysX) and compiler-generated code |
| P1.5 | Globals and imports | `globals.csv` (53,026), `imports.csv` (522) | done | 2026-09-24 | |
| P1.6 | Types export | `resources/docs/types/sizes.csv`, `types.json`, `all_types.h`, `xcheck_sdk.md` | done | 2026-09-24 | 39,964 UDTs, 20,051 enums. `FName=8`, `FString=12`, `UObject=56`, `UClass=456`. CodeRed 2013 dump only covers 6 classes; offsets differ (expected 2013 delta) |
| P1.7 | Vtables | `vtables.csv` | done | 2026-09-24 | 5,349 vftables, 271,589 slots. COMDAT folding makes some slot names misleading (see `resources/docs/symbols/README.md`) |
| P1.8 | Native function table | `natives.csv`, `classes.csv`, `natives_xcheck.md` | done | 2026-09-25 | 2,165 `exec*`, 1,993 with index (277 numbered incl. Core opcodes, rest `-1` name-bound), 1,036 `StaticClass`. DishonoredGame classes: 6.5 % exe-only vs DFSDK `.uc`; Engine/Core `.uc` in DFSDK are UDK 2010, not comparable |
| P1.9 | Hardcoded FName table | `hardcoded_names.csv` | done | 2026-09-25 | 499 names from `FName::StaticInit` (471 `AllocateNameEntry` calls + 28 inlined entries). Indices sparse 0–1300 by design |
| P1.10 | Module map | `resources/docs/module_map.md`, `resources/docs/progress.md` | done | 2026-09-25 | Engine 41 %, DishonoredGame 25 %, Core 10 %, Scaleform 10.4 %, Wwise ~4 %. No XAudio2 / OnlineSubsystemPC / PathEngine in Shipping |
| P1.11 | 2013 database sanity | `resources/docs/symbols/db_verify_2013.txt` | done | 2026-09-24 | Plain auto-analysis: 34,059 functions, 93 % unnamed, 18 types. Phase 7 work |
| P1.12 | Tracking documents | this file, `resources/docs/progress.md`, `resources/tools/symbols/verify_phase1.py` | done | 2026-09-25 | |

## Acceptance rules

- P0.1: `git status` clean after commit; game binaries ignored.
- P0.2: manifest lists the 4 Win32 2012 exes with PDB-matching GUIDs and `Dishonored.exe` 2013 with GUID `5c4d3821...`.
- P0.3: `import idapro` works; `ninja --version`; `cmake --version` ≥ 3.25.
- P0.4: `resources/run-vcvars.cmd x86-debug` produces a PE with machine 0x14c that writes the milestone line.
- P0.5: 2012 log contains `Init: Object subsystem initialized` and a map load.
- P1.1: `sub_` share ≤ 5 %; `UObject`, `UClass`, `UProperty`, `FName`, `FString`, `FArchive` present in local types.
- P1.2: row count equals IDA function count; `FEngineLoop::Init`, `UObject::Serialize`, `appInit`, `FName::StaticInit`, `UObject::StaticConstructObject` present.
- P1.3: every DIA function RVA is an IDA function; every Shipping runtime module appears in `sourcefiles.txt`.
- P1.4: ≥ 90 % of non-CRT functions attributed to a module.
- P1.5: `GObjObjects`, `GEngine`, `GWorld`, `GNatives`, `GMalloc` present; `steam_api`, `d3d9`, `binkw32`, `dinput8` imported.
- P1.6: `FName=8`, `FString=12`, a 12-byte `TArray<...>`, `UObject`, `UClass`, `UProperty`, `FArchive` in `sizes.csv`.
- P1.7: `UObject` vtable has `Serialize`; distinct vftable classes ≥ `StaticClass` count.
- P1.8: no `execReq_DLC05_*`; natives carry indices; DishonoredGame-class exe-only share < 10 %.
- P1.9: index 0 = `None`; ≥ 400 unique names incl. `Core`, `Engine`.
- P1.10: core/engine/dishonoredgame/gfxui rows present.
- P1.11: file exists with a function count.
- P1.12: `verify_phase1.py` exits 0.

## Follow-ups handed to later phases

- Phase 2: import the reference engine source (`../UnrealEngine3`, build 10897, see
  `resources/docs/engine_reference.md`) module by module; `resources/tools/symbols/xref_reference.py` for per-function
  reference status.
- Phase 2: generate the `static_assert` layout header from `resources/docs/types/sizes.csv` / `types.json`
  and diff it against the reference headers.
- Phase 4: decide Scaleform (10.4 %) and Wwise (~4 %) strategy before GFxUI / AkAudio work.
- Phase 7: name propagation into the 2013 database; `resources/docs/types/xcheck_sdk.md` lists the first known layout deltas.
