# Phase 0 + Phase 1 tracker

Status values: `todo`, `in-progress`, `blocked`, `done`. Dates are completion dates.
Acceptance rules are re-checked by `python tools/symbols/verify_phase1.py`.

## Phase 0 — Project setup

| ID | Task | Deliverable | Status | Date | Notes |
|---|---|---|---|---|---|
| P0.1 | Repo init and layout | `.gitignore`, `README.md`, dir tree, first commit | done | 2026-09-24 | commit `5e99803` |
| P0.2 | Binary manifest | `docs/binaries.md` via `tools/hash_binaries.py` | done | 2026-09-24 | 4 Win32 2012 exes GUID-match their PDBs; `Debug2012/Win32/Dishonored.exe` is the Shipping-DC build |
| P0.3 | Toolchain | ninja, comtypes, vcpkg, idalib activation, `docs/toolchain.md` | done | 2026-09-24 | |
| P0.4 | CMake bootstrap | `CMakeLists.txt`, `CMakePresets.json`, `run-vcvars.cmd`, `Launch.cpp` | done | 2026-09-24 | 32-bit exe builds and writes `Init: Object subsystem initialized` |
| P0.5 | Reference builds and golden logs | `docs/golden/*.log`, `tools/normalize_log.py` | in-progress | | Shipping-DC wrote no log at all; ArkProfile build used instead. Retail 2013 log is savegame lines only (Shipping logging compiled out) |

## Phase 1 — Symbol and type database

| ID | Task | Deliverable | Status | Date | Notes |
|---|---|---|---|---|---|
| P1.1 | Verify and snapshot IDA db | `docs/idb/shipping2012_v1.i64`, `docs/symbols/db_verify.txt` | in-progress | | |
| P1.2 | Function export | `docs/symbols/functions.csv`, `segments.csv` | todo | | |
| P1.3 | Line info via DIA | `lines.csv`, `pdb_functions.csv`, `compilands.csv`, `sourcefiles.txt` | in-progress | | |
| P1.4 | Join → module attribution | `functions.csv` (+file,line,module,origin), `functions_nofile.csv`, `functions_nofile_summary.md` | todo | | |
| P1.5 | Globals and imports | `globals.csv`, `imports.csv` | todo | | |
| P1.6 | Types export | `docs/types/all_types.h`, `types.json`, `sizes.csv`, `xcheck_sdk.md` | todo | | |
| P1.7 | Vtables | `vtables.csv` | todo | | |
| P1.8 | Native function table | `natives.csv`, `classes.csv`, `natives_xcheck.md` | todo | | |
| P1.9 | Hardcoded FName table | `hardcoded_names.csv` | todo | | |
| P1.10 | Module map | `docs/module_map.md` | todo | | |
| P1.11 | 2013 database sanity | `docs/symbols/db_verify_2013.txt` | todo | | |
| P1.12 | Tracking documents | this file, `docs/progress.md`, `tools/symbols/verify_phase1.py` | in-progress | | |

## Acceptance rules

- P0.1: `git status` clean after commit; game binaries ignored.
- P0.2: manifest lists the 4 Win32 2012 exes with PDB-matching GUIDs and `Dishonored.exe` 2013 with GUID `5c4d3821...`.
- P0.3: `import idapro` works; `ninja --version`; `cmake --version` ≥ 3.25.
- P0.4: `run-vcvars.cmd x86-debug` produces a PE with machine 0x14c that writes the milestone line.
- P0.5: 2012 log contains `Init: Object subsystem initialized` and a map load; normalized diff pasted in `docs/golden/README.md`.
- P1.1: `sub_` share ≤ 5 %; `UObject`, `UClass`, `UProperty`, `FName`, `FString`, `FArchive` present in local types.
- P1.2: row count equals IDA function count; `FEngineLoop::Init`, `UObject::Serialize`, `appInit`, `FName::StaticInit`, `UObject::StaticConstructObject` present.
- P1.3: DIA function count within 2 % of IDA's; every runtime module appears in `sourcefiles.txt`; `FEngineLoop::Init` has line records.
- P1.4: ≥ 90 % of non-CRT functions attributed to a module.
- P1.5: `GObjObjects`, `GNames`/`FName::Names`, `GEngine`, `GWorld`, `GNatives`, `GMalloc` present; import DLL list matches the survey.
- P1.6: `FName=8`, `FString=12`, `TArray<...>=12`, `UObject`, `UClass`, `UProperty`, `FArchive` in `sizes.csv`; `types.json` parses.
- P1.7: `UObject` vtable has `Serialize`; distinct classes ≥ `StaticClass` count.
- P1.8: no `execReq_DLC05_*`; core natives carry `native_index`; exe-only share < 10 %.
- P1.9: index 0 = `None`; contiguous; count ≥ 1000.
- P1.10: percentages sum to 100 ± 1; Core, Engine, DishonoredGame, GFxUI rows present.
- P1.11: file exists with a function count.
- P1.12: `verify_phase1.py` exits 0.
