# Toolchain

Recorded 2026-09-24 (P0.3). No legacy compiler is used; this is a functional, non-matching rebuild.

| Tool | Version | Notes |
|---|---|---|
| MSVC | 19.44.35225 (VS 2022 Community, toolset 14.44.35207) | x86 target via `VsDevCmd -arch=x86`; older toolsets 14.16 / 14.29 also installed |
| CMake | 4.4.0-rc2 | presets in `CMakePresets.json` |
| Ninja | 1.13.2 (pip) | |
| Python | 3.13.14 | scripts under `resources/tools/` |
| comtypes | 1.4.17 | DIA SDK access for `resources/tools/pdb/dia_dump.py` |
| DIA SDK | `msdia140.dll` from VS 2022 Community | loaded register-free |
| IDA Pro | 9.1 (`C:\Program Files\IDA Professional 9.1`) | `idapro` Python module 0.0.9 activated for headless runs; IDA 8.3 also installed |
| clang-cl / llvm-pdbutil | LLVM in `C:\Program Files\LLVM` | `llvm-pdbutil` cannot read the game PDBs ("Too many directory blocks"), DIA is used instead |
| git | 2.45.1 | |

Build: `resources/run-vcvars.cmd x86-debug` (configure + build) or `cmake --preset x86-debug` inside a
VS x86 developer prompt. Full game exe: `resources\build-game.cmd [target]` (`build\game`, every module option on;
`set BUILD_DIR=build\agentX` for another directory).

**Never use the FModel MCP tools (`mcp__fmodel__*`).** FModel is UE4-only, it does not read Dishonored's UE3 packages, and
the user has forbidden it (2026-09-26; also in `PHASE6.md` rules, `STATUS.md` pitfalls, `agents/README.md`). Package data
comes from our own readers (`resources/tools/pdb/extract_edgeanim.py`, `read_package_classes.py`, the `-loadall` switch).

## Repository build and debug tooling (wave 4, agent AK)

| Tool | Purpose |
|---|---|
| `cmake/Dependencies.cmake` | FetchContent sources shared under `external/<name>-src` (fetched once, `FETCHCONTENT_SOURCE_DIR_<NAME>` reuses them without a download sub-build); every build directory compiles its own copy under `<build>/_deps/<name>-build`, so any number of build directories build concurrently (verified with three full builds, `agents/agentAK.md`) |
| `resources/tools/make_snapshot.py <X> [files…]` | HEAD snapshot worktree `build/agent<X>_wt` + your files overlaid (`--sync` re-copies the recorded list, `--remove` removes it only when it holds no junction); writes `build/agent<X>_wt_build.cmd`. Refuses a target containing a reparse point or a stage directory |
| `resources/tools/unlink_junctions.py [dir] --apply` | lists/removes junctions and symlinks (the link only, never the target); the only sanctioned way to remove a link |
| `resources/tools/build_and_smoke.py` | build + stage + run + golden diff; `--expect`, `--expect-count LINE=N`, `--forbid LINE`, per-agent `--exe-name/--log-name/--ini-dir`; no `-seekfreeloadingpcconsole` (retail default) |
| `resources/tools/stage_retail.py` | copies the exe (+ .pdb/.map) into the retail `Binaries\Win32` as `DishonoredGame_<X>.exe`; refuses stub exes and the retail exe name |
| `resources/tools/debug/` (`dbg.py`, `dbgrun.py`, `stack_sample.py`, README) | crash / `appErrorf` / hang stacks symbolized through the linker `.map`, non-invasive thread sampling; `dbgrun.py` takes the smoke tool's switches |
| `resources/tools/ida/decompile_funcs.py` | headless Hex-Rays decompiles, one `<name>_<rva>.c` per function (overloads never collide) |
| `resources/tools/symbols/gen_layout_probe.py compare <probe> [--write]` | layout verification; the shared `reference_layout_delta.md` is rewritten only with `--write` |
