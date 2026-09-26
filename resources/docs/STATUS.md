# Project status — 2026-09-26 (milestone 5 done: the world renders and the player walks in it)

Read this first when resuming. Plan of record: `PLAN.md`. Trackers: `PHASE1.md` (done), `PHASE2.md`
(done), `PHASE3.md` (wave 1, done), `PHASE4.md` (wave 2, done), `PHASE5.md` (wave 3, done — read its
"Wave result"), `PHASE6.md` (wave 4, current). Decisions and fixes: `porting_notes.md`.
Per-function status: `progress.md` + `function_status.csv`. Agent reports: `agents/agent<A..AC>.md`.

## Incident 2026-09-25 (read before running anything)

The retail install `D:\RecompileDishonored\Dishonored_Latest2026` once lost `Engine/`, `DishonoredGame/CookedPCConsole`,
`DLC`, `Localization`, `Movies` during the wave-2 merge: `git worktree remove --force` on an agent worktree whose
`build/stage` junctioned those folders followed the junctions (restored since through Steam). Rules since: no junctions
into the retail or reference trees, ever; `stage_retail.py` copies our exe into the retail `Binaries\Win32` (as
`DishonoredGame_<X>.exe` per agent) and refuses stub exes; never recursively delete a directory that may contain a
junction (`resources/tools/unlink_junctions.py` removes links only); each agent runs with its own `--log-name`,
`--ini-dir` and `--exe-name` (`build_and_smoke.py`).

## Target

**Retail 2013 build** (`Dishonored_Latest2026`, engine 9411, DLC05–07) is what we rebuild. The 2012
symbolized build is a helping hand only: names, types, decompiles. Retail truth in hand:
native class sizes (`types/native_class_sizes.csv`, 2,857 classes), package member lists
(`types/script_classes_2013.json`, 3,043 classes), runtime member offsets of every reflected member
(`types/retail_sdk_layout.json` from the CodeRed dump `Dishonored_DumpedSDK_Retail`, `sdk_dump.md`),
and a named retail IDA database (`idb/retail2013_named.i64`, 82.8 % of the 2012 names propagated;
mangled names, e.g. `?Possess@AController@@UAEXPAVAPawn@@@Z`; addresses are VAs, image base 0x400000).
Every layout assert and cross-check uses the retail numbers first and the 2012 PDB only where retail
has no data.

## Where we are (HEAD after the wave-3 commits 43343f6 … 63758d5 + docs)

| Milestone (PLAN.md Phase 6) | State |
|---|---|
| 1. Core+Engine+Launch compile and link | done (wave 1) |
| 2. `Init: Object subsystem initialized` | done (wave 1/2), null RHI and d3d9 |
| 3. Load `Core.upk` … `Startup.upk`, `GEngine->Init()` | **done (wave 3 X/Z)**: retail seek-free path, `Startup.upk` 63,718 objects, `Initializing Engine...`, `LoadMap: DishonoredGameFull_P`, `Bringing World … up for play`, `Finished loading level`, `Initial startup: 5.2s`; the tick loop runs (null RHI, `--skip-native OnlineSubsystemPC`) |
| 4. D3D9 device, Bink movie, Scaleform menu | **done bar the menu**: the first mission map with all 8 sub-levels streamed in renders **20,370 frames under d3d9** windowed at 1280x720 over 90 s with **0 critical errors**, and 37,470 under the null RHI; the cooked caches load whole (127 global records, 2,580 material maps, 0 mismatches). The Scaleform main menu still waits on the GFx decision in `middleware.md`; long runs use `resourcesuild-release.cmd` |
| 5. Mission map, player spawns, input | **done 2026-09-26**: the first mission map renders (20,370 frames, d3d9 windowed 1280x720, 90 s, 0 criticals) and the pawn stands on it in `PHYS_Walking` and walks (`-inputtest moved` 1030.1, peak speed 500.7, still walking at 115 s with all seven always-loaded sub-levels visible). Two defects found by playing it are the lead packages of wave 5 (`PHASE7.md`): textures are corrupt on large surfaces, and touch notifications never fire so triggers, volumes and pickups are inert |

| Area | State |
|---|---|
| Modules building | Core, Engine, GameFramework, IpDrv, WinDrv, D3D9Drv, Launch (real) and, with `DISHONORED_ENABLE_{GFXUI,AKAUDIO,OSS,DISHONOREDGAME}=ON`, GFxUI, AkAudio, OnlineSubsystemSteamworks (no-Steam path), DishonoredGameModule (289 natives ported, 685 warn-once stubs). `resources\build-game.cmd [target]` configures + builds `build\game` with every option on. 764 units, 0 errors |
| Runs | `python resources/tools/build_and_smoke.py --build-dir build/game --no-build --exe-name DishonoredGame_C.exe --log-name coord.log --ini-dir build/coord_config --rhi null --milestone "Initializing Engine..." --expect "Finished loading level" --expect "Initial startup" --skip-native OnlineSubsystemPC` passes; `--rhi d3d9 --extra-args "-windowed -ResX=1280 -ResY=720 -nomovie"` reaches the material load and stops (above) |
| Natives | Script natives without a C++ body bind to `UObject::execDishonoredUnboundNative` (consumes parameters, zeroes the result, warns once; `-strictnatives` aborts): ~140 Engine/GameFramework ones remain (`agents/agentZ.md` list); generated module stubs use `DISHONORED_NATIVE_STUB`. Still stubbed on the map path: `Camera.UpdateCamera`, `HUD.DisplayConsoleMessages`, `DownloadableContentManager.*DLC*`, `Pawn.Died`, `Camera.ClearCameraLensEffects`, `InterpActor.SetShadowParentOnAllAttachedComponents`, OSS `Read*` |
| Animation | ACF_EdgeAnim (113,232 of 113,242 cooked sequences) decodes through the ported per-sequence evaluator (`Engine/Src/EdgeAnimEvaluate.cpp`, bit-exact on 2,743 sequences); `-edgerefpose` restores the reference-pose gate; the whole-tree Edge path (blend tree evaluation) is not ported (`edgeanim.md`) |
| Layouts vs retail | `xcheck_sdk_layout.py build/game/layout_probe.txt`: 2,314 types, **0 rows**; `gen_layout_probe.py compare`: 2,341 types, 0 contract mismatches; `verify_phase2.py retail` 2/2; DishonoredGame 12,502 layout asserts, 0 pending |
| Engine convergence | 143 functions of the startup assets checked against 2013 (87 identical, 49 ported, 5 written; `serialization_delta_engine.md`); `progress.md` Engine 60 ported / 30 written / 87 verified |
| Renderer | `renderer.md`: D3D9 RHI, cooked global shader cache (VER_MIN_SHADER 786, SF_Pixel = 1); material shader caches (786/23, 798/23 gates) and 143 Arkane shader types not declared; under the null RHI the scene render is skipped (`RenderViewFamily_RenderThread`) because the global cache lacks e.g. `FDownsampleSceneDepthPixelShader` |
| Audio | **deferred to Phase 10 as polish** (the user's call, 2026-09-26): the silent backend loads the real banks and resolves the real event ids, which is all bring-up needs. Do not open an audio package until the game plays |
| Middleware, ours | **PhysX 2.8.4, Steamworks and Wwise 2012.1 bindings are written by us** from the shipped DLLs and their PDBs, no vendor SDK downloaded and nothing redistributed: `source/Development/Src/External/{PhysX284,SteamworksFlat,Wwise2012}`, `cmake/{PhysX,Steamworks,Wwise}.cmake`, stub-DLL import libraries as for Bink. `WITH_NOVODEX=1` (scene created, convex meshes cooked, 235 rigid bodies in the streamed levels), `WITH_STEAMWORKS=1` (the last 3 unported natives ported), Wwise with a silent backend (65 file packages, 39 banks, 463 of 468 events resolved; real audio needs the licensed SDK, which is then one cmake switch). Still off: `WITH_GFx` (Scaleform decision), `WITH_FACEFX`, `WITH_APEX` (retail never linked it), `WITH_OGGVORBIS` |
| Middleware (`middleware.md`) | GFx 3.3.89, Wwise 2012.1 (bank v65), FaceFX 1.7.3.1, PhysX 2.8.4, Bink 1.9p (import lib in use), steam_api 1.30.50.46, libcurl 7.77.0; SDKs the user must obtain |
| Versions pinned | `UnObjVer.cpp`: engine 9411, package 801, licensee 30, cooked content 133. `UnNames.h`: 499 hardcoded names + 69 reference-only |
| Tools | `resources/tools/sdk/` (`parse_codered_sdk.py`, `sdk_props.py`, `sdk_show.py`, `xcheck_sdk_layout.py`), `symbols/gen_layout_probe.py`, `gen_layout_asserts.py`, `gen_classes_header.py --sdk` (+ `Inc/CppText/<Class>.h` hook, `<Module>NativeStubs.ported.txt` skip list), `ida/decompile_funcs.py`, `build_and_smoke.py`, `stage_retail.py`, `build/head_wt_build.cmd` (clean-worktree verification), `resources/build-game.cmd` |

## Next

Wave 5 is planned in `PHASE7.md`, built around the two defects the user found by running the game: **AR**
textures, **AS** touch/triggers/volumes, then **AT** Kismet (which also fixes the menu teardown and the retail
New Game route), **AU** the DishonoredGame AI-brain foundation and pickups, **AV** the Arkane animation nodes,
**AW** the Scaleform decision, **AX** the load-all sweep and a regression harness. Audio stays out (Phase 10).

Wave 4 is fully merged (`PHASE6.md` tracker and "Wave result" have the numbers and the commits `571bd0e`,
`8af9425`, `03f5335`, `f8bad4f`, `ad3f9ae`, `f8dfe78`, `6e9d1ad`, `f184f60` plus the bridge commit). With
`-noscenerender` the merged tree reaches the map change and keeps ticking with no critical error; with the
scene renderer on it asserts at ~7 s on a mesh batch whose index range exceeds its index buffer, which is the
next blocker. Then wave 5 from the follow-ups in `PHASE6.md`: Arkane anim
nodes (the tweak anim tree is gated behind `-distweakanimtree` until they exist), the 275 DishonoredGame stubs
behind the AI brain / sub-process / item-context classes (triage in `agents/agentAJ.md`), the Arkane and GFx
post-process shader families, a Release or `FMallocBinned` build for long d3d9 runs, `UShaderCache` 132 -> 128,
the ~100 remaining Engine/GameFramework shim classes, the retail nav-mesh runtime, and the whole-tree Edge
path. Also the load-all test over all 471 `.upk` (milestone 3 exit check), now that AD's `-loadall` exists,
Phase 4 is done: the PhysX, Steamworks and Wwise bindings are ours, written from the shipped DLLs and
their PDBs, so nothing there waits on a download. What is left of the middleware is Scaleform GFx, which
gates the main menu (`middleware.md`), and **audio, which is deliberately last — PLAN.md Phase 10, polish**.

## Known pitfalls

- `/Zc:alignedNew-` is required (set in `CMakeLists.txt`): UE3 overrides the global `operator new`/`delete` but
  not C++17's aligned overloads, so over-aligned render types placement-newed into the engine heap were freed
  through the CRT's aligned free and the process aborted during skeletal mesh cleanup.
- A serializer that writes a member back unconditionally corrupts it, because `Serialize` is also walked by the
  GC's reference collector (that is what destroyed every skeletal mesh's triangle count; see `agents/agentAO.md`).
- MSVC lays a run of consecutive virtual overloads out in **reverse** declaration order. Any hand-written
  interface binding must be checked against the real vtable, not against declaration order.
- Smoke runs need `-forcelogflush` (`"--extra-args=-forcelogflush"`, the `=` form: argparse eats a bare
  `-switch`), otherwise the log truncates mid-line and a milestone that did happen never reaches the file.
- After a power cut, a build directory can fail every link with an access violation and a truncated exe: those
  are corrupted compiler PDBs (`C1051: obsolete format`). Delete the `*.pdb` inside that build directory only.
  A power cut can also zero-fill source files — scan before trusting a diff (one Engine source and a set of
  regenerable decompiles were zeroed on 2026-09-26).
- The Engine link is close to its limit (Engine.lib ~1.19 GB). Reference-only shims in widely included headers
  must be plain `static` with one definition in a `.cpp`, never inline static: as inline statics six of them
  added 77 MB of per-TU ctor/dtor/atexit and type info and broke the link.
- Never use the FModel MCP tools (`mcp__fmodel__*`): UE4-only, useless on these UE3 packages, and the user has forbidden them.
- Never open one IDA database from two processes; agents copy `retail2013_named.i64` (`retail2013_<agent>.i64`).
  The coordinator's copy is `retail2013_coord.i64` (idalib MCP session); names are MSVC-mangled, look them up as
  `?Name@Class@@...`; `lookup_funcs` on a VA (rva + 0x400000).
- The Bash tool collapses `\\` and `\n` in heredocs: write patch scripts with the Write tool. Sources are CRLF.
- CMake `file(GLOB)` is `CONFIGURE_DEPENDS`; new files need a reconfigure. `cmd` splits `-DX=Y` script
  arguments at `=`: put cmake flags inside the `.cmd` or in an environment variable. PowerShell `>` writes a BOM:
  write probe output from bash (`LayoutProbe.exe > build/game/layout_probe.txt`).
- Parallel agent builds race on the shared FetchContent `external/*-build` (pnglibconf.h): snapshot builds with
  their own external dirs until the per-build FetchContent package (wave 4) lands.
- Junctions: see the incident above. `git worktree remove`, `rm -rf` and `Remove-Item -Recurse` may follow them.
- The shared working tree is edited by every agent at once; verify merges on the clean `build/head_wt` worktree
  (`git -C build/head_wt checkout --detach main`, `build/head_wt_build.cmd DishonoredGame` with `LAYOUT_CHECKS=ON`
  and `EXTRA_CMAKE` holding the module options), never on the working tree. Never `git add -A`.
- A `DECLARE_FUNCTION(execX)` in the generated `*Classes.h` has no trailing semicolon when an inline body follows;
  insert new declarations (with `;`) before such a line, never between it and its `{`.
