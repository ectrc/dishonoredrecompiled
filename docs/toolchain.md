# Toolchain

Recorded 2026-09-24 (P0.3). No legacy compiler is used; this is a functional, non-matching rebuild.

| Tool | Version | Notes |
|---|---|---|
| MSVC | 19.44.35225 (VS 2022 Community, toolset 14.44.35207) | x86 target via `VsDevCmd -arch=x86`; older toolsets 14.16 / 14.29 also installed |
| CMake | 4.4.0-rc2 | presets in `CMakePresets.json` |
| Ninja | 1.13.2 (pip) | |
| vcpkg | 6ade29bb (2026-09-24), `external/vcpkg` | manifest `vcpkg.json`, no deps yet |
| Python | 3.13.14 | scripts under `tools/` |
| comtypes | 1.4.17 | DIA SDK access for `tools/pdb/dia_dump.py` |
| DIA SDK | `msdia140.dll` from VS 2022 Community | loaded register-free |
| IDA Pro | 9.1 (`C:\Program Files\IDA Professional 9.1`) | `idapro` Python module 0.0.9 activated for headless runs; IDA 8.3 also installed |
| clang-cl / llvm-pdbutil | LLVM in `C:\Program Files\LLVM` | `llvm-pdbutil` cannot read the game PDBs ("Too many directory blocks"), DIA is used instead |
| git | 2.45.1 | |

Build: `run-vcvars.cmd x86-debug` (configure + build) or `cmake --preset x86-debug` inside a
VS x86 developer prompt.
