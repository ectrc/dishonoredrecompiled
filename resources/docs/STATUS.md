# Project status — 2026-09-25 (Phase 3 wave 2 landed: milestone 2 for the native packages; see the incident note)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `resources/docs/PHASE1.md`
(done), `resources/docs/PHASE2.md` (done), `resources/docs/PHASE3.md` (wave 1 done, wave 2 next). Decisions and fixes: `resources/docs/porting_notes.md`.

## Target

**Retail 2013 build** (`Dishonored_Latest2026`, engine 9411, DLC05–07) is what we rebuild. The
2012 symbolized build is a helping hand only: names, types, decompiles. Many structs differ in
size and member set between 2012 and 2013; everything derived from the 2012 PDB (the current
Core layouts, `sizes.csv`, `types.json`, the contract-type asserts) is **provisional** until
checked against the retail exe / retail packages (PLAN.md Phase 2b).

## Where we are

Phase 2 is complete and **Phase 3 wave 1 has landed** (`resources/docs/PHASE3.md`): **milestone 1 is
reached** — our own `DishonoredGame.exe` (real `Launch` + Core + Engine + GameFramework + IpDrv +
WinDrv, null RHI, `-DDISHONORED_REAL_LAUNCH=ON`) runs against the staged retail content and logs
`Init: Object subsystem initialized` (`python resources/tools/build_and_smoke.py`). CoreSmoke is
99/99 (incl. LZO-decompressing a retail package chunk). Retail truth is in hand: native class
sizes for 2,857 retail classes, member lists for 3,043 retail script classes, and a named retail
database (`resources/docs/idb/retail2013_named.i64`, 82.8% of 2012 names propagated). **The Core and
Engine contract types now match retail** (`types/retail_reconciliation.md`: UClass 436 without
`NetFields`/`m_DropdownCategory`, UTexture2D 372, USkeletalMeshComponent 1088); the layout tools
(`gen_layout_probe.py compare`, `gen_layout_asserts.py`) take retail sizes from
`native_class_sizes.csv` and fall back to the 2012 PDB only where a class has no retail descriptor.
A CodeRed dump of the **running retail exe** (`D:\RecompileDishonored\Dishonored_DumpedSDK_Retail`,
`resources/docs/sdk_dump.md`) supplies the runtime offset of every reflected member;
`resources/tools/sdk/xcheck_sdk_layout.py` checks our probe against it (`types/retail_sdk_delta.md`:
1,132 Core+Engine types, 666 exact, 234 to converge, 0 contract mismatches after the APawn fix).
Next blocker on the way to milestone 2: `SystemSettings.cpp:532` assert on the retail ini.

| Area | State |
|---|---|
| Reference tree | `../UnrealEngine3` (UE3 build 10897). Modules imported into `source/Development/Src/`: Core, Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, GFxUI, OnlineSubsystemSteamworks, Launch. Skeletons for DishonoredGame (1,043 files), AkAudio, DisJobs. |
| Build | `resources\run-vcvars.cmd x86-debug` / `x86-release`. Core, Engine, GameFramework, IpDrv, WinDrv build by default; `Launch` is the real `LaunchEngineLoop` behind `-DDISHONORED_REAL_LAUNCH=ON` (null RHI via stubs until D3D9Drv is a target, wave 2 P). `LayoutProbe`, `CoreSmoke` (99/99) and `build_and_smoke.py` (milestone 1) green. `/Zp4` per target. |
| Switched off for now | `WITH_FACEFX=0`, `WITH_APEX=0`, `WITH_STEAMWORKS=0`, `WITH_GFx=0`, `WITH_LZO=0` (missing SDKs, Phase 4; **LZO is needed for package loading**: all packages are COMPRESS_LZO). PCH off. DirectX 9 + libpng come from the reference tree (`cmake/ReferenceExternals.cmake`), zlib via FetchContent. |
| Versions pinned | `Core/Src/UnObjVer.cpp`: engine 9411, package 801, licensee 30, cooked content 133. `Core/Inc/UnNames.h` regenerated from the 499 hardcoded names (+69 reference-only names ≥ 1301). |
| Symbol data | `resources/docs/symbols/*` (functions, natives with all folded indices, hardcoded names, licensee branches, opcodes, package summaries), `resources/docs/types/*` (sizes, member delta, retail sizes `native_class_sizes.csv`, retail offsets `retail_sdk_layout.json` from the SDK dump), `resources/docs/reference_xref.csv` (Core 89 % / Engine 52 % of functions have a reference definition). |
| Layout probe | `reference_layout_delta.md`: 1,899 Core+Engine types probed, 1,374 exact, 0 contract mismatches **against retail sizes**. `DishonoredLayouts.h`: Core 278 asserts + 12 pending, Engine 669 + 284 pending. Nine non-contract Engine classes still differ from retail (`retail_reconciliation.md`). |
| Opcodes / natives | Dishonored's bytecode opcodes match the reference (differences are COMDAT folding artifacts). Core numbered natives: no real mismatches. |
| Serialization | `serialization_delta_core.md`: 98 Core functions vs reference, 24 to port (UClass::Serialize, UScriptStruct::SerializeBin, CreateLoader+ArkBsPatch, LZO package chunks, ...). `function_status_seed.csv` seeds Phase 3 status. |
| Arkane Core code | bzip2 decompressor + bspatch + TPool implemented from decompile and verified (`agents/agentC.md`). |
| Class headers | `gen_classes_header.py` emits UE3-style `*Classes.h` from the PDB; `dishonoredgame_class_inventory.md`: 1,485 DishonoredGame classes, 220 with DFSDK `.uc`. |

## Incident 2026-09-25 (read before running anything)

The retail install `D:\RecompileDishonored\Dishonored_Latest2026` lost `Engine/`, `DishonoredGame/CookedPCConsole`,
`DLC`, `Localization`, `Movies` during the wave-2 merge: `git worktree remove --force` on an agent worktree whose
`build/stage` junctioned those folders followed the junctions. `Binaries/Win32` and `DishonoredGame/Config`
survived. **Restore with Steam "Verify integrity of game files"** (the folder is a Steam install). All junctions
under `build/` have been removed; `stage_retail.py` no longer creates any (it copies our exe into the retail
`Binaries\Win32`); never recursively delete a directory that may contain a junction.

## Wave 2 result (2026-09-25)

All packages O–V of `resources/docs/PHASE4.md` are merged (HEAD `0c9bd4d`): D3D9Drv is a module; every Engine
header is on the retail runtime layout (SDK delta: 2,314 dump types, 1,677 exact, 4 rows, all in
GameFramework/IpDrv bases); `EShowFlags` is a QWORD; DishonoredGame (1,822 native classes), GFxUI, AkAudio and
OnlineSubsystemSteamworks have generated registrants/headers behind `DISHONORED_ENABLE_*`; our exe loads
Core/Engine/GameFramework/IpDrv end to end (`24107 objects as part of root set`, agent O); middleware versions
are pinned in `middleware.md`. Next: load the game packages with T's registrants (drop the skip list), then
`Startup.upk`, `GEngine->Init()`, milestone 3; converge the remaining GameFramework/IpDrv bases; Phase 4
SDK decisions per `middleware.md`.

## Next (Phase 3 wave 2)

Detailed plan with parallel work packages O–U: `resources/docs/PHASE4.md` (wave 1, H–N, is done:
`resources/docs/PHASE3.md`). Tooling for the wave is in place: `resources/tools/sdk/sdk_props.py`
(PROPS blocks from retail offsets), `sdk_show.py`, `xcheck_sdk_layout.py --header`,
`build_and_smoke.py --rhi/--expect/--skip-native`. First base-class fix with it (`UInterpTrackInst`
56 → 64) took the SDK delta from 232 to 209 rows.

- O: milestone 2 — `FSystemSettings` from `GEngineIni` with retail's 107 keys, `-nullrhi`, port
  `FAsyncIORequest::Event`/`LoadDataWithEvent`, retail's hardcoded native package list, load
  Core/Engine/GameFramework/IpDrv, then Startup once T's registrants exist.
- P: D3D9Drv module target + build follow-ups. Q/R/S: Engine headers converged on
  `retail_sdk_delta.md` by base-class family. T: DishonoredGame/GFxUI/AkAudio/OSS registrants and
  headers generated from the SDK dump. U: middleware versions + `middleware.md`.

### Wave 1 items (done)

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
