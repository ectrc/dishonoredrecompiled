# Project status — 2026-09-25 (Phase 3 wave 2 landed; milestone 2 reached for the native packages)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `PHASE1.md` (done), `PHASE2.md`
(done), `PHASE3.md` (wave 1, done), `PHASE4.md` (wave 2, done). Decisions and fixes: `porting_notes.md`.
Per-function status: `progress.md` + `function_status.csv`. Agent reports: `agents/agent<A..V>.md`.

## Incident 2026-09-25 (read before running anything)

The retail install `D:\RecompileDishonored\Dishonored_Latest2026` lost `Engine/`, `DishonoredGame/CookedPCConsole`,
`DLC`, `Localization`, `Movies` during the wave-2 merge: `git worktree remove --force` on an agent worktree whose
`build/stage` junctioned those folders followed the junctions. `Binaries/Win32` and `DishonoredGame/Config`
survived; the 2012 tree is intact. **Restore with Steam "Verify integrity of game files"** (the folder is a
Steam install). All junctions under `build/` were removed (link-only, `resources/tools/unlink_junctions.py`);
`stage_retail.py` no longer creates any (it copies our exe into the retail `Binaries\Win32`); never
recursively delete a directory that may contain a junction. Until the content is restored, package tests in
CoreSmoke skip (45 pass + 2 skip instead of 99) and no smoke run past milestone 1 is possible.

## Target

**Retail 2013 build** (`Dishonored_Latest2026`, engine 9411, DLC05–07) is what we rebuild. The 2012
symbolized build is a helping hand only: names, types, decompiles. Retail truth now in hand:
native class sizes (`types/native_class_sizes.csv`, 2,857 classes), package member lists
(`types/script_classes_2013.json`, 3,043 classes), runtime member offsets of every reflected member
(`types/retail_sdk_layout.json` from the CodeRed dump `Dishonored_DumpedSDK_Retail`, `sdk_dump.md`),
and a named retail IDA database (`idb/retail2013_named.i64`, 82.8 % of the 2012 names propagated).
Every layout assert and cross-check uses the retail numbers first and the 2012 PDB only where retail
has no data.

## Where we are (HEAD `38141fb`)

| Milestone (PLAN.md Phase 6) | State |
|---|---|
| 1. Core+Engine+Launch compile and link | done (wave 1) |
| 2. `Init: Object subsystem initialized` | done 2026-09-26, null RHI; `--rhi d3d9` also creates the D3D9 RHI (wave 2 P) |
| 3. Load `Core.upk` … `Startup.upk` | **partial**: Core, Engine, GameFramework, IpDrv load end to end through our `ULinkerLoad` (LZO chunks, `Link`, `CreateExport`, `Preload`, `PostLoad`): `24107 objects as part of root set` (agent O, with `--skip-native GFxUI,AkAudio,OnlineSubsystemPC,OnlineSubsystemSteamworks,DishonoredGame -NoLoadStartupPackages -allowunboundnatives`). The game packages need the generated registrants (agent T, options `DISHONORED_ENABLE_*`) — the combined exe builds and links but its milestone-2 run is pending the content restore |
| 4. D3D9 device, Bink movie, Scaleform menu | not started (Bink import lib + header exist behind `DISHONORED_WITH_BINK`; Scaleform decision in `middleware.md`) |

| Area | State |
|---|---|
| Modules building | Core, Engine, GameFramework, IpDrv, WinDrv, D3D9Drv (on by default); GFxUI, AkAudio, OnlineSubsystemSteamworks, DishonoredGameModule behind `DISHONORED_ENABLE_{GFXUI,AKAUDIO,OSS,DISHONOREDGAME}` (generated registrants/headers, natives are `appErrorf` stubs); real `Launch` behind `DISHONORED_REAL_LAUNCH=ON`. Clean HEAD worktree with everything on and `DISHONORED_LAYOUT_CHECKS=ON`: 677 units, 0 errors, 64 MB exe |
| Layouts vs retail | `xcheck_sdk_layout.py`: 2,314 dump types probed, 1,677 exact, **4 rows** left (`ADisDoor`, `ADisGameCrowdAgentSkeletalRat`, `ADishonoredPlayerController`, `UOnlineSubsystemSteamworks`: shifts from GameFramework/IpDrv bases still to converge). `gen_layout_probe.py compare`: 2,341 types, 2,259 exact, 0 contract mismatches. Asserts: Core 277 + 13 pending, Engine 763 + 190 pending, D3D9Drv 8 + 5, DishonoredGame 12,137 SDK asserts (354 pending on the same bases) |
| Engine headers | all `*Classes.h` regenerated on the retail SDK offsets (agents Q/R/S: 216 PROPS blocks), `EShowFlags` is a QWORD with Dishonored's bits (V), interface bases per retail. Reference-only members live on as `DISHONORED_SHIM_STATIC` shims (storage-less); every shim use in `Engine/Src` is a porting TODO (tables in agentQ/R/S.md) |
| Core | UClass 436 (no `NetFields`/`m_DropdownCategory`), UObject without `NetIndex`, FPackageInfo 68, FAsyncIORequest with `Event` + `LoadDataWithEvent`; 26 serialization functions ported from the 2012 decompile and re-checked in the 2013 db (`function_status.csv`) |
| Engine bring-up ports (agent O) | `FSystemSettings` on retail's design (GEngineIni, 107 keys, no checkf, HKCU override), retail `RHIInit`, no shader compiler / no `.usf` hashing (retail has none), retail native package lists (2013 rva 0x5def10/0x5dfb50), GC token streams of ULevel/UWorld/UStaticMesh, five loader `Serialize` deltas |
| Switched off | `WITH_FACEFX=0`, `WITH_APEX=0` (retail never linked APEX), `WITH_STEAMWORKS=0`, `WITH_GFx=0`, `USE_UNIT_TESTS=0`, `WITH_REFERENCE_LIBPNG=0`; `WITH_LZO=1` (lzokay), `WITH_EDITORONLY_DATA=1` (retail keeps the editor-only members). PCH off. `/Zp4` per target |
| Middleware (`middleware.md`) | GFx 3.3.89, Wwise 2012.1 (bank v65), FaceFX SDK 1.7.3.1, PhysX SDK 2.8.4 (DLLs 2.8.4.6), Bink 1.9p, steam_api 1.30.50.46 (SDK 1.18/1.19 interfaces), libcurl 7.77.0; per-library plan and the SDKs the user must obtain |
| Versions pinned | `UnObjVer.cpp`: engine 9411, package 801, licensee 30, cooked content 133. `UnNames.h`: 499 hardcoded names + 69 reference-only |
| Tools | `resources/tools/sdk/` (`parse_codered_sdk.py`, `sdk_props.py`, `sdk_show.py`, `xcheck_sdk_layout.py`), `symbols/gen_layout_probe.py` (generate/compare/props [--sdk]/show/pdb-fix/structs), `gen_layout_asserts.py`, `gen_classes_header.py --sdk`, `ida/decompile_funcs.py` (headless batch decompile by name/rva), `ida/match_functions.py`, `pdb/read_package_classes.py`, `binaries/scan_versions.py`, `build_and_smoke.py --rhi/--expect/--skip-native/--milestone`, `stage_retail.py` (in place, no junctions), `unlink_junctions.py` |
| Bytecode / natives | opcodes match the reference; Core numbered natives match; `natives_2013.csv` for the retail table |

## First combined run (2026-09-25, content restored)

CoreSmoke 99/99 again. `DishonoredGame.exe -log -nosteam -seekfreeloadingpcconsole -unattended -nullrhi -NoLoadStartupPackages -allowunboundnatives -skipnativepkgs=OnlineSubsystemPC` with all four module options: every native script package loads, including `DishonoredGame.upk` (334 log lines, 276 localization warnings from the CDO `ImportText` pass), only four classes still go through the bridge (`Engine.ArkHealthInterface`, `Engine.ArkSettingsListenerInterface`, `Engine.DebugCameraController`, `DishonoredGame.DisGameCrowdPopulationManager`). It dies at `AnimationEncodingFormat.cpp:613` "7: unknown or unsupported translation compression": retail's `AnimationCompressionFormat` has `ACF_EdgeAnim = 7` (Sony Edge animation codec, the `Edge`/`DisJobs` modules of the 2012 PDB) which our tree lacks. A plain `DishonoredGame.exe -log` (no `-seekfreeloadingpcconsole`) exits silently after `Object subsystem initialized` with a 2-line log next to the exe: retail forces seek-free loading, ours does not yet.

## Next

1. `ACF_EdgeAnim` (7): port or bypass the Edge animation codec (`UAnimSequence` translation/rotation decompression;
   bring-up option: treat as identity/zero pose behind a `DISHONORED(bringup)` switch); register the four bridged
   classes; force seek-free loading like retail. Then, from a build with the four module options on:
   `python resources/tools/build_and_smoke.py --build-dir <dir> --no-build --milestone "objects as part of root set" --skip-native OnlineSubsystemPC --extra-args "-allowunboundnatives"`
   — load DishonoredGame/GFxUI/AkAudio/OSS with T's registrants, then drop `-NoLoadStartupPackages`
   (`Startup.upk`), then `StaticLoadClass(GameEngine)` + `GEngine->Init()` (milestone 3 proper).
2. Converge the GameFramework/IpDrv bases behind the 4 remaining SDK rows (`AGameCrowdAgentSkeletal`,
   `UOnlineSubsystemCommonImpl`, …) and the pending assert rows; `FSceneViewFamily::CurrentBendTime`,
   `SHOW_DefaultGame |= Selection|Portals`, `FAsyncIORequest` 2013 76-byte layout, `FSystemSettings`
   1088-byte struct convergence.
3. Start the per-function convergence of Engine (`progress.md`: Core 26 ported, Engine 0) with the
   shim tables as the work list; DishonoredGame natives from the named 2013 decompiles.
4. Phase 4: obtain PhysX 2.8.4 SDK, Wwise 2012.1 SDK, Steamworks 1.18/1.19 (user); decide Scaleform
   per `middleware.md` (main menu gates milestone 4).

## Known pitfalls

- Never open one IDA database from two processes; agents copy `shipping2012_v1.i64` / `retail2013_named.i64`.
- The Bash tool collapses `\\` and `\n` in heredocs: write patch scripts with the Write tool.
- CMake `file(GLOB)` is `CONFIGURE_DEPENDS`; new files need a reconfigure. `cmd` splits `-DX=Y` script
  arguments at `=`: put cmake flags inside the `.cmd` or in an environment variable.
- Junctions: see the incident above. `git worktree remove`, `rm -rf` and `Remove-Item -Recurse` may follow them.
- The shared working tree is edited by every agent at once; verify merges on a clean `git worktree add
  --detach build/head_wt HEAD` build (`build/head_wt_build.cmd`), never on the working tree.
