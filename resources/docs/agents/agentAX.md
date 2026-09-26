# Agent AX report — the load-all sweep of the whole content tree and a one-command regression run (wave 5)

Package "### AX — Verification" of `PHASE7.md`. Build dirs: `build\agentAX` (shared working tree, used for the first
build and then abandoned for measurement — see §0) and `build\agentAX_wt\build\agentAXR` (the clean HEAD snapshot
`build\agentAX_wt`, made with `make_snapshot.py AX`, which is where every number below was measured). No IDA copy
was needed (no retail function was ported; the one retail-vs-ours question in §3 is answered from our own headers
and the retail SDK layout the tree already carries). Nothing committed, nothing `git add`ed.

## What changed

| File | Change | Evidence |
|---|---|---|
| `source/Development/Src/Engine/Src/DishonoredLoadAll.cpp` | **agent AD's file, additive only** (§1.1): `-loadallpurge`, continue past a non-fatal `LoadPackage` failure, load line logged **before** the collect plus a new `loadall purged <pkg>: <N> objects live` line after it | §1 |
| `resources/tools/debug/loadall_sweep.py` (new) | the milestone 3 exit check: `upk` sweeps every `.upk` of the retail cooked tree through `-loadall` and restarts after a crash, `pck` parses and bounds-checks every Wwise `AKPK` file package, `report` merges both into the committed baseline | §1 |
| `resources/docs/loadall_baseline.{md,csv}` (new) | the baseline: 471 `.upk` + 503 `.pck`, per-package object counts, load time, live-object count after the collect, and the triage of every error | §1, §3 |
| `resources/tools/run_regression.py` (new) | one command: build, CoreSmoke, LayoutProbe + `xcheck_sdk_layout` + `compare` + `verify_phase2 retail`, the null-RHI milestone run, the d3d9 render run with frame and census counts, the `-inputtest` distance, the PhysX scene counts, the unported-native count — 30 metrics, each against an expected value, exit non-zero on any regression | §2 |
| `resources/docs/regression_baseline.json` (new) | the expectations: `op` / `value` / `optional` / `note` per metric, so tightening a bound or adding a counter is a data change, not a code change | §2 |
| `resources/docs/agents/agentAX_status.csv` (new) | status rows, including the `needed` row for the `UMaterial::FinishDestroy` defect | — |

`build_and_smoke.py` needed **no change**: agent AK's `--forbid` / `--expect-count` / `prepare_run` are exactly what
`run_regression.py` and `loadall_sweep.py` drive, and both call it as a subprocess with `--no-build`, the `=` form for
extra args and always `-forcelogflush`. `resources\play.cmd` and `resources\build-release.cmd` were not touched.

## 0. Why every number here is from a snapshot, not from the shared tree

The first full measurement run was made on the shared working tree and was **wrong**, in a way worth recording:
it reported `texture census 13915`, `touch census 2026`, a 19.9 s d3d9 startup, 1,200 frames in 90 s and 656
draw-list elements per frame. Those are agents **AR's and AS's in-flight edits** (`D3D9Texture.cpp`, `UnTex.h`,
`Texture2D.cpp`, `UnContentStreaming.cpp` were modified in the tree while I built), not HEAD. A verification baseline
built from a tree five other agents are editing is not a baseline. Everything below was re-measured on
`build\agentAX_wt` = `git worktree add --detach HEAD` (`a64f9b4`) + my four files, Release.

The accident is also the proof that the optional-counter wiring works end to end: AR's and AS's census lines were
picked up and measured as soon as they existed, and they go back to `SKIP` at HEAD (§2.3).

## 1. The milestone 3 exit check (step 1)

Full result and the triage of every error: **`resources/docs/loadall_baseline.md`**, rows in
`loadall_baseline.csv`. Headline:

| kind | packages | objects / contained files | load errors | teardown crashes |
|---|---:|---:|---:|---:|
| `.upk`, `DishonoredGame/CookedPCConsole`, `UObject::LoadPackage` | **471** | **524,807** | **1** | **30** |
| `.pck`, CookedPCConsole + DLC05..07, AKPK header parse | **503** | **2,411** | **0** | n/a |

**Milestone 3 is exited**: every one of the 471 cooked packages deserializes without a linker abort — no
`Bad export index`, no `Bad name index`, no `Serial size mismatch`, no `Serialize` assertion, over 524,807 objects.
115 s wall for the whole sweep, 7.9 s of it inside the loads. `L_Tower_Env` loads 5,089 objects, identical to agent
AD's wave-4 number, which cross-checks the counter.

### 1.1 The additive changes to `DishonoredLoadAll.cpp` (agent AD's file)

Nothing was removed and no existing log line changed before its `errors` field, so AD's `loadall_driver.py` and its
accept command still parse. Three additions:

1. **`-loadallpurge`** collects with `RF_Native | RF_Marked` instead of AD's `RF_Native | RF_Marked | RF_Standalone`.
   Every cooked asset is `RF_Standalone`, so AD's keep flags never freed a swept package; the startup packages are in
   the root set and are kept whatever the keep flags are. Without the switch the 32-bit address space is gone after a
   few dozen packages; with it the live-object count after each of 471 collects stays inside **65,619..89,149**.
   AD's default is unchanged (`--no-purge` in the sweep tool restores it).
2. A `LoadPackage` that returns **NULL** (an unresolvable name) is counted and the sweep **continues**. Only a linker
   abort has to end the process, because only `appErrorf` runs `UObject::StaticShutdownAfterError`.
3. The per-package line is logged **before** the collect, and a new line
   `DISHONORED(bringup): loadall purged <pkg>: <N> objects live` after it. A package whose load line is present and
   whose purge line is missing crashed in the **teardown** of its own objects — which is 30 of the 471 (§3), and
   without this split their load results were simply lost (the first sweep attributed 29 packages to
   "died before the first loadall line").

### 1.2 Why `.pck` cannot go through `-loadall`

The 503 `.pck` files are **Wwise 2012.1 file packages** (magic `AKPK`), read by the AkAudio low-level IO, never by
`UObject::LoadPackage`. `loadall_sweep.py pck` parses the header the engine parses — version, the four section sizes,
and every soundbank / streamed-file / external lookup-table entry's `startBlock * blockSize + fileSize` against the
file length. All 503 parse, 2,411 contained files, 0 errors, and the engine's own log agrees line for line
(`Wwise: file package <name>.pck: AKPK v1, 1 banks, N streamed files`).

## 2. The regression run (steps 2 and 3)

```
python resources/tools/run_regression.py --build-dir build/agentAX                 # build + all six stages
python resources/tools/run_regression.py --build-dir build/agentAX --no-build
python resources/tools/run_regression.py --no-build --only d3d9,inputtest
python resources/tools/run_regression.py --no-build --update-baseline
python resources/tools/run_regression.py --list
```

From a snapshot, run **this tree's** copy against the snapshot's build directory with `--no-build`
(`--build-dir build/agentAX_wt/build/agentAXR --no-build`): a worktree has no `resources/docs/types` data (generated,
not committed) and `stage_retail.py` resolves the retail install relative to its repo, so a worktree's own copy of
the layout tools cannot run there. That is documented in the tool's docstring.

Six stages, 30 metrics. Expectations live in `resources/docs/regression_baseline.json`.

### 2.1 The passing run on HEAD — exit 0

`build\agentAX\regression_head_final2.txt`, Release build of the HEAD snapshot,
**27 ok, 0 failed, 3 skipped, 443 s** (`regression_head3.txt` is the same result with the earlier, tighter bounds):

| stage | metric | measured at HEAD | gate |
|---|---|---:|---|
| build | 3 targets (`DishonoredGame`, `CoreSmoke`, `LayoutProbe`) | 0 error lines, exit 0 each | `== 0` |
| coresmoke | passed / failed | **99 / 0** | `>= 99` / `== 0` |
| layout | `xcheck_sdk_layout` types / mismatching / contract | **2,314 / 0 / 0** | `== 2314` / `== 0` / `== 0` |
| layout | `gen_layout_probe compare` probed / contract | **2,341 / 0** | `>= 2341` / `== 0` |
| layout | `verify_phase2 retail` | **2 / 2** | `>= 2` / `== 2` |
| nullrhi | log lines, `Initial startup`, criticals | 7,991, **3.1 s** (2.7 s idle), **0** | `>= 500`, `<= 60 s`, `== 0` |
| d3d9 | log lines, `Initial startup`, criticals | 2,604, **3.9 s** (3.4 s idle), **0** | `>= 500`, `<= 60 s`, `== 0` |
| d3d9 | frames (`scene rendered`), 90 s run | **20,070** | `>= 1000` |
| d3d9 | static draw-list elements drawn / in the scene | **965 / 6,381** | `>= 400` / `>= 6000` |
| d3d9 | primitives surviving culling | **445** | `>= 200` |
| d3d9 | texture census (agent AR) | absent -> **SKIP** | `> 0`, optional |
| inputtest | `-inputtest moved` / peak 2D speed | **1,029.1 / 500.4** | `>= 800` / `>= 400` |
| inputtest | PhysX actors / static shapes | **895 / 1,312** | `>= 850` / `>= 1250` |
| inputtest | distinct unported natives that fire on the path | **4** | `<= 4` |
| inputtest | criticals | **0** | `== 0` |
| inputtest | touch census (AS) / sequence-op census (AT) | absent -> **SKIP** | `> 0`, optional |

The gates are deliberately far below the measured values wherever the metric is load sensitive, and tight wherever
it is not. Measured cost of machine load, same command, same exe, twice: 20,220 frames / 965 draws / 445 visible
prims / 2.7 s startup with the machine idle, and **4,380 frames / 588 draws / 279 visible prims / 25.2 s startup**
while four other agents were building (`regression_head_final.txt`). Correctness metrics do not move at all under
load — 2,314 / 0 / 0, 99 / 0, 895 / 1,312, `moved` 1,029.1..1,029.6, 0 criticals — so those are pinned, and the
frame, draw and startup gates only have to catch what every defect of waves 4 and 5 actually looked like: a counter
at zero, a crash, or a milestone that stopped being reached. The `_about` block of the baseline says so.

`PHASE7.md`'s "Where this wave starts" table reproduces, with **one correction**: the unported natives that fire on
the walking path are **four**, and not the three the table names. They are `ADishonoredGameInfo::execGameEnding`,
`ADishonoredPlayerController::execHandleHeldButtons`, `ADishonoredPlayerPawn::execLanded_Native` and
`ADishonoredPlayerPawn::execTakeFallingDamage_Native`. `Dis_Zoom` never fires on that path; the two Pawn ones do (the
pawn lands on the boat). Agent AU's step 3 should take all four. Everything else matches: startup 3.4 s vs 3.2 s,
20,070..20,220 frames vs 20,370 in 90 s with 0 criticals, `965 of 6,381` draw-list elements vs `979 of 6,381`,
`inputtest moved` 1,029.1 vs 1030 with peak 2D speed 500.4, and 895 physics actors with 1,312 static shapes exactly.
The drawn count is the one number that wanders (965 idle, 784 and 588 under load, because the census samples every
30th frame while the sub-levels are still streaming in), so its gate is `>= 400`.

### 2.2 The failing run — a deliberately broken counter, exit 1

`build\agentAX\regression_break_noscene.txt`: the same command with `--game-extra-args=-noscenerender`, which turns
the scene census off exactly as a future wave would by dropping the counters:

```
ok   d3d9_log_lines           1262 (wanted >= 500)
ok   d3d9_criticals           0 (wanted == 0)
FAIL d3d9_frames              -1 (wanted >= 1000)
FAIL d3d9_draws_per_frame     -1 (wanted >= 400)
FAIL d3d9_draw_elements       -1 (wanted >= 6000)
FAIL d3d9_visible_prims       -1 (wanted >= 200)
2 ok, 5 failed, 1 skipped      -> exit 1        (build\agentAX\regression_break_final.txt)
```

A metric the baseline requires and the log no longer carries is reported as `-1`, which fails `>=`, `<=` and `==`
alike; it is never silently skipped. `build\agentAX\regression_head2.txt` is the second kind of failure, a value that
moved: `FAIL unported_natives 4 (wanted <= 3)` against the pre-correction bound, exit 1.

Two guards exist because the first version of the tool was fooled by both:

* **`<stage>_log_lines`** is a metric. The very first snapshot run reported plausible numbers while no game ran at
  all (`stage_retail.py` resolved the retail tree relative to the worktree and refused), because the metrics were
  read from the **previous** run's `Launch.log`. `smoke()` now deletes the log before the run and 0 log lines fails.
* An **unparsable tool output fails**. The same run reported `SKIP layout_types` because
  `xcheck_sdk_layout.py` had thrown (no `retail_sdk_layout.json` in a worktree); a missing summary line is now `-1`.

`xcheck_sdk_layout.py` rewrites the shared `resources/docs/types/retail_sdk_delta.md` as a side effect. The layout
stage saves and restores it, so a verification run leaves no shared document dirty (pass `--write-deltas` to keep the
rewrite). `git status` after every run here shows only my own files.

### 2.3 The census counters of the other packages (step 3)

`texture_census`, `touch_census` and `sequence_census` are `optional` in the baseline: absent -> `SKIP`, present ->
measured and required to be `> 0` on every later run. The contract is what agent AP's scene census already does, and
it is written into the baseline and the tool: **`DISHONORED(bringup): <thing> census: <N> ...`**, first integer is
the headline count. The regex only matches such a line, never the command-line echo at the top of the log — the
first version matched `-distouch` in the echo and reported `touch census 2026` out of the retail path
`Dishonored_Latest2026`.

Measured on the shared working tree while AR and AS had their edits in it (§0, `build\agentAX\regression_run1.txt`):
`texture_census 13915` (AR's `texture census: 633 created (378 DXT1, ...)` and its upload line) and
`touch_census 2026`, i.e. both counters are already picked up. `sequence_census` (AT) stays `SKIP` until AT lands.
The switches a census needs live in the baseline's `_extra_args` (currently `-distouch` for the inputtest stage), so
adding one is a data change too. When a counter is in HEAD, flip `optional` to `false` and the metric can never be
dropped again without failing the run.

## 3. Triage of the sweep's errors (step 1's accept line)

Detail, stacks and the affected package list: `loadall_baseline.md`. Summary:

| error | count | assignment |
|---|---:|---|
| `Can't bind to native class OnlineSubsystemPC.OnlineSubsystemPC` | 1 | **benign, self-inflicted**: `OnlineSubsystemPC.upk` is a native script package whose registrant this build does not link, which is why every run passes `-skipnativepkgs=OnlineSubsystemPC`. Not a serializer failure, and nothing in the game loads it |
| teardown crash, `UMaterial::FinishDestroy+0x56`, read at `0xfffffcfd` | 29 | **one bug, one line**: `Material.cpp:1925` deletes `DefaultMaterialInstances[0..2]` but the member is `FDefaultMaterialInstance*[2]` (retail layout, `EngineMaterialClasses.h:3949`), so index 2 aliases `INT EditorX` and `delete (FDefaultMaterialInstance*)EditorX` faults — `0xfffffcfd` is `EditorX == -771`. Only packages carrying a material with a non-zero `EditorX` fault, hence 29 and not 471. **Not caused by `-loadallpurge`**: the same stack appears under `UObject::StaticExit` at ordinary process shutdown with the switch off. Fix: loop to `ARRAY_COUNT(DefaultMaterialInstances)` as the constructor already does for `MaterialResources`. `Material.cpp` belongs to no wave-5 package -> **coordinator** (or AR by extension); AX did not edit it |
| teardown crash, integer divide by zero in `FTexture2DResource::GetData+0xbd` <- `InitRHI` <- the rendering thread | 1 (`LEVEL`) | **agent AR**. `Texture2D.cpp:2590` divides by `GPixelFormats[EffectiveFormat].BlockSizeX`; the only row with `BlockSizeX == 0` is `PF_Unknown`, so a `UTexture2D` in `LEVEL.upk` reaches `InitRHI` with `Format`/`GetEffectivePixelFormat` == `PF_Unknown`. Same function family as AR's corrupt-texture defect, and AR's own census already reports `6 pitch mismatches` |

All three were found with agent AK's `dbgrun.py`, which stops on the first real exception and symbolizes the EBP
chain through the `.map` (`build\agentAX\dbg\purge1.txt`, `purge_L_PrsnSewer_P.txt`, `purge_L_Streets1_Light.txt`,
`purge_LEVEL.txt`, `nopurge_exit.txt`, and the HEAD re-checks under `build\agentAX_wt\build\agentAXR\dbg\`).

## 4. Commands

```
snapshot: python resources\tools\make_snapshot.py AX source/Development/Src/Engine/Src/DishonoredLoadAll.cpp
              resources/tools/run_regression.py resources/tools/debug/loadall_sweep.py resources/docs/regression_baseline.json
build:    cd build\agentAX_wt  &&  set BUILD_DIR=build\agentAXR  &&  resources\build-release.cmd [DishonoredGame|CoreSmoke|LayoutProbe]
          (shared tree: set BUILD_DIR=build\agentAX then resources\build-release.cmd)
sweep:    python resources\tools\debug\loadall_sweep.py upk    --build-dir build/agentAX_wt/build/agentAXR --timeout 3600
          python resources\tools\debug\loadall_sweep.py pck    --build-dir build/agentAX_wt/build/agentAXR --dirs all
          python resources\tools\debug\loadall_sweep.py report --build-dir build/agentAX_wt/build/agentAXR
regress:  python resources\tools\run_regression.py --build-dir build/agentAX_wt/build/agentAXR --no-build          -> exit 0
          python resources\tools\run_regression.py --build-dir build/agentAX_wt/build/agentAXR --no-build --only d3d9 "--game-extra-args=-noscenerender"
                                                                                                                    -> exit 1
stack:    python resources\tools\debug\dbgrun.py --build-dir build/agentAX_wt/build/agentAXR --exe-name DishonoredGame_AX.exe
              --log-name agentAX_dbg.log --ini-dir build/agentAX/config --rhi null --skip-native OnlineSubsystemPC
              --hang 300 --tag purge1 "--extra-args=-loadall=L_ArtDealer_Block -loadallpurge"
```

Scratch, not repo files: `build\agentAX` (first build + every run output), `build\agentAX_wt` (snapshot worktree,
holds no stage directory and no junction — remove it with `git worktree remove` only after checking that),
`build\agentAX_wt\build\agentAXR` (the build the numbers come from), `build\agentAX\loadall\` (per-run sweep logs),
`build\agentAX*\...\regression\` (per-stage outputs and `summary.txt`).

## 5. Hand-overs

1. **Coordinator**: `UMaterial::FinishDestroy` (§3). One line, and it is a crash at ordinary process shutdown, not
   only in the sweep. `loadall_baseline.md` lists the 29 packages to re-sweep after the fix; the expected result is
   `1 load error, 1 teardown crash`.
2. **Agent AR**: the `PF_Unknown` divide by zero in `FTexture2DResource::GetData` for `LEVEL.upk` (§3). One census
   line naming the texture and its `Format` settles it.
3. **Agent AU**: four natives fire on the walking path, not three (§2.1). `execLanded_Native` and
   `execTakeFallingDamage_Native` are in place of `Dis_Zoom`.
4. **Coordinator, at merge**: after each package merges, `run_regression.py --build-dir build/head_wt/build/wt
   --no-build` replaces the hand-run check list in `PHASE7.md`'s "Coordination and merge". When a package's census
   line lands, flip its `optional` to `false` in `regression_baseline.json` and set the value, and raise or lower the
   bound the package moved (AP's draw count, AU's native count) in the same commit — the baseline is the record of
   what the wave earned.
5. **Not covered**: the 434 `.upk` under `DLC/PCConsole/DLC05..07`. They are mounted but never loaded by the bring-up
   path; `loadall_sweep.py upk --dirs all` sweeps them when that number is wanted.
