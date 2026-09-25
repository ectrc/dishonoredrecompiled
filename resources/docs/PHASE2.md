# Phase 2 tracker — Reference import and layout convergence

Status values: `todo`, `in-progress`, `blocked`, `done`. Exit check: `python resources/tools/symbols/verify_phase2.py`.
Plan of record: `PLAN.md` §4 Phase 2.

| ID | Task | Deliverable | Status | Date | Notes |
|---|---|---|---|---|---|
| P2.1 | Reference cross-reference | `resources/docs/reference_xref.csv`, xref tables in `engine_reference.md` | done | 2026-09-25 | Core 89 %, Engine 52 % of non-generated functions have a reference definition (brace-scanner index, 36.9k definitions) |
| P2.2 | Import Core from the reference | `source/Development/Src/Core/`, `reference_import.md`, stubs for `bspatch/*`, `Pool.h` | done | 2026-09-25 | Engine/IpDrv/Launch/WinDrv/GameFramework/D3D9Drv/GFxUI/OSS-Steamworks headers imported too (flat include model) |
| P2.3 | CMake module pattern and defines | `cmake/DishonoredDefines.cmake`, `cmake/DishonoredModule.cmake`, `cmake/ReferenceExternals.cmake`, Core target | done | 2026-09-25 | configure lists Core (69 units); PCH off; DirectX from the reference tree |
| P2.4 | Core compiles with MSVC 2022 | `Core.lib` in x86-debug and x86-release, `porting_notes.md` | in-progress | | iterating on `core_build*.log`; see porting_notes.md |
| P2.5 | Layout verification vs PDB | `DishonoredLayouts.h`, `LayoutProbe`, `types/reference_layout_delta.md` | todo | | |
| P2.5b | Textual member delta | `types/reference_member_delta.md` | done | 2026-09-25 | 2,804 classes compared, 1,038 differ (parser is name-based; compiled delta is authoritative) |
| P2.6 | Pin versions, names, package constants | `package_summary.md`, `UnObjVer.cpp`, regenerated `UnNames.h` | in-progress | | `package_summary.md` done: file 801 / licensee 30, engine 9411 (2013) / 9014 (2012), cooked content 133 / 132, zlib |
| P2.7 | Core class and native registration | `gen_classes_header.py`, native index check, `symbols/opcodes.md` | in-progress | | opcodes match the reference (folding artifacts only); Core numbered natives: 205 common, 0 real mismatches; class-header generator pending |
| P2.8 | Licensee serialization inventory | `symbols/licensee_branches.csv`, `.md` | done | 2026-09-25 | 1,723 functions scanned: 14 licensee comparisons (thresholds 0–30), 217 engine-version comparisons |
| P2.9 | Skeletons for non-reference modules | `DishonoredGame/`, `AkAudio/`, `DisJobs/` stubs | done | 2026-09-25 | 1,043 + 6 + 6 files (15 DishonoredGame entries were non-source paths) |
| P2.10 | Tracking and exit check | this file, `progress.md` per-function scheme, `verify_phase2.py` | in-progress | | |
| P2.12 | (stretch) Core smoke exe | `source/Tests/CoreSmoke` | todo | | |

## Acceptance rules

- P2.1: 66,394 rows; Core `reference` share ≥ 60 %; runs in < 1 min with a warm index.
- P2.2: Core under `source/`; `reference_import.md` lists the copied files, stubs and excluded units.
- P2.3: configure succeeds and lists target `Core`.
- P2.4: `--target Core` builds in both presets; `porting_notes.md` has the category table.
- P2.5: `DishonoredLayouts.h` compiles; delta empty for the Core contract types listed in the plan.
- P2.5b: file exists; Core section agrees with the compiled delta.
- P2.6: `package_summary.md` exists; `UnNames.h` index/name equality with `hardcoded_names.csv`.
- P2.7: Core native indices identical to `natives.csv`; generated `CoreClasses.h` compiles as a drop-in.
- P2.8: CSV non-empty; every licensee threshold ≤ 30.
- P2.9: DishonoredGame 1,058 / AkAudio 6 / DisJobs 6 skeleton files.
- P2.10: `verify_phase2.py` exits 0.
- P2.12: `CoreSmoke.exe` links and exits 0 (not required for exit).
