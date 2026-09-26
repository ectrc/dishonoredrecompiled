# Agent AK report — build hygiene, debug tooling, Edge in-engine check and whole-tree decision memo (wave 4)

Build dirs `build\agentAK`, `build\agentAK2`, `build\agentAK3` (three full configures + builds of the shared working tree at
HEAD 4035b87, run concurrently, see §1). Snapshot worktree `build\agentAK_wt` (made by the new `make_snapshot.py`, §4).
IDA copy `resources\docs\idb\retail2013_agentAK.i64` (used once, to test `decompile_funcs.py`). Nothing committed, nothing
`git add`ed; every change is in the shared working tree.

## What changed

| File | Change | Evidence / test |
|---|---|---|
| `resources/tools/build_and_smoke.py` | `-seekfreeloadingpcconsole` dropped from `GAME_ARGS` (retail default since X's `FEngineLoop::PreInit` port, 2013 rva 0x5e1910); new `--expect-count "<substring>=N"` (at least N lines) and `--forbid "<substring>"` (first offending line printed with its line number); `--expect/--expect-count/--forbid` results in `<build>/smoke/expect.txt`; malformed `--expect-count` fails before the build; `prepare_run()` factored out (log/ini cleanup + isolation switches) so `debug/dbgrun.py` shares it. All existing switches unchanged | §2: baseline exit 0 without the switch; `--forbid "Bad export index"` exit 1 on the baseline; unit run of `check_lines` on a fake log (A true, count 3 false, forbid false, missing log false) |
| `resources/tools/ida/decompile_funcs.py` | output name `<sanitized name>_<rva hex>.c` (AA follow-up 5) | `rva:0x2909e0 rva:0x2780f0` → `UNavigationMeshBase_Serialize_2909e0.c`, `operator__2780f0.c` (the `operator<<` overloads no longer collide) |
| `resources/tools/symbols/gen_layout_probe.py` | `compare <probe> [--write]`: the shared `reference_layout_delta.md` is written only with `--write` (AA follow-up 6); the summary line says so | `compare build/game/layout_probe.txt` → `probed=2341 exact=2316 contract_mismatches=0 -> reference_layout_delta.md not written (pass --write)`, `git status` of the doc clean |
| `resources/tools/debug/dbg.py` (new) | X's `agentX_dbg.py` + Z's `zdbg.py` merged: debug loop, stop on the first real exception (AV, `appErrorf` → `DebugBreak` 0x80000003 inside the image), registers + EBP-chain stack through the `.map`, `--hang=N` all-thread dump, `--attach=PID <map>` one-shot dump of a running exe (resumes and detaches), Z's WOW64 breakpoint filtering, `EXIT_THREAD` bookkeeping | §3 |
| `resources/tools/debug/stack_sample.py` (new) | W's sampler promoted and made non-invasive: `--pid`, `--name DishonoredGame_X.exe`, `--after S [--kill] <exe> args…`; suspends, dumps every thread, **resumes** (W's killed the process and never resumed); shares `MapSymbols`/`dump_stack` with `dbg.py` | §3 |
| `resources/tools/debug/dbgrun.py` (new) | stage (`stage_retail.py`) + run under `dbg.py` with the smoke tool's switches (`--build-dir --exe-name --log-name --ini-dir --rhi --skip-native --extra-args`, plus `--hang`, `--tag`); `-forcelogflush`; output `<build>/dbg/<tag>.txt` + `<tag>.log` | §3 |
| `resources/tools/debug/README.md` (new) | what each tool stops on, how to read a `.map`-symbolized EBP chain, the debug-heap fill values, the isolation rules | — |
| `resources/tools/make_snapshot.py` (new) | `make_snapshot.py <X> [files…] [--list f]`: `git worktree add --detach build/agent<X>_wt HEAD` (reused if registered) + overlay of the named files (identical files skipped, deleted sources deleted in the snapshot), list remembered in `build/agent<X>/snapshot_files.txt` (`--sync` re-copies), writes `build/agent<X>_wt_build.cmd` (`cmake -S build/agent<X>_wt -B build/agent<X>` with every module option). **Refuses** a target containing a reparse point (walk that never descends into links, same as `unlink_junctions.py`) or a stage dir (`build/stage` / `stage`), refuses sources outside the repo or under `build/`/`external/`, refuses reparse points among sources/destinations. `--remove` refuses while any link is inside and names the `unlink_junctions.py --apply` command; only with 0 links does it call `git worktree remove` | §4 |
| `resources/docs/edgeanim.md` §7 (new) | decision memo: whole-tree Edge job vs Plan B — stage-by-stage comparison table, cost, the pending in-engine procedure, recommendation (keep Plan B for wave 5, triggers that flip it) | §5 |
| `resources/docs/toolchain.md` | full-exe build line; the FModel rule (never `mcp__fmodel__*`, UE4-only, forbidden by the user — coordinator's mid-wave rule, HEAD 7b3e82a); a "repository build and debug tooling" table (per-build `_deps`, snapshot tool, unlink tool, smoke switches, debug tools, decompile naming, `compare --write`) | — |

The generated `build/agent<X>_wt_build.cmd` carries `-DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/UnrealEngine3` like
`resources\build-game.cmd` since 7b3e82a (coordinator's note); `build-game.cmd` itself was not edited by me.

`cmake/Dependencies.cmake` and the root `CMakeLists.txt` needed no change beyond the coordinator's pre-wave edit: the
three-build test below is the verification of that edit.

## 1. Three concurrent full builds (step 1, accept line part 1)

Started within the same minute from Git Bash (`BUILD_DIR='build\agentAKn' cmd //c 'resources\build-game.cmd'`, i.e. a
fresh configure + `DishonoredGame` build of every module with `GFXUI/AKAUDIO/OSS/DISHONOREDGAME=ON`), while other agents'
builds were also running (cl.exe count on the machine > 60 at times):

| Build dir | Configure start | Exe written | Ninja steps | `C1083` | `: error` | Exe |
|---|---|---|---|---|---|---|
| `build\agentAK` | 00:35:01 | 00:50:49 | 769/770 (`-k 0`, the 770th is the never-built stub target) | 0 | 0 | 63,968,256 bytes |
| `build\agentAK2` | 00:35 | 00:50:43 | 769/770 | 0 | 0 | 63,968,256 bytes |
| `build\agentAK3` | 00:35 | 00:50:46 | 769/770 | 0 | 0 | 63,968,256 bytes |

Logs: `build\agentAK_build0.log`, `agentAK2_build0.log`, `agentAK3_build0.log` (each ends with `exit 0`). Each build dir has
its own `_deps\{zlib,libpng,lzokay}-build` (`nvapi-build` too where FetchContent created it; nvapi has no CMakeLists), the shared
`external\<name>-src` trees were not modified (mtimes 13:24–13:26 of the original fetch) and no `external\<name>-subbuild`
ran (the `FETCHCONTENT_SOURCE_DIR_<NAME>` path). ~16 minutes wall for three full builds in parallel. The stale pre-wave
`external\<name>-build` / `<name>-subbuild` directories are unused now and can be removed by the coordinator (plain
directories, no links; I did not delete them).

## 2. Smoke tool changes and their tests (step 2, accept line part 2)

Commands (exe from `build\agentAK`, staged as `DishonoredGame_AK.exe`, log `agentAK.log`, ini `build\agentAK\config`):

```
A  python resources/tools/build_and_smoke.py --build-dir build/agentAK --no-build --exe-name DishonoredGame_AK.exe --log-name agentAK.log
     --ini-dir build/agentAK/config --rhi null --skip-native OnlineSubsystemPC --milestone "Initializing Engine..."
     --expect "Finished loading level" --expect "Initial startup"
   -> run exit code 3, Launch.log 9309 lines, milestone reached, expect ok x2, exit 0                    (build/agentAK/smoke/expect_A.txt)
B  same + --expect-count "Committed map change via DishonoredEngine=1" --forbid "Bad export index" --forbid "Serial size mismatch"
   -> expect MISSING: Committed map change via DishonoredEngine x0 (wanted >= 1)
      expect FORBIDDEN: Bad export index at Launch.log:9784: [0027.44] Critical: appError called: Bad export index 1065353215/6389
                        [package ..\..\DishonoredGame\CookedPCConsole\Dishonored_MainMenu_Env.upk, serializing NavigationMeshBase ...]
      expect ok: no Serial size mismatch
      exit 1                                                                                            (build/agentAK/smoke/expect_B.txt)
```

The run command line shows the switch set now used: `-log -nosteam -unattended -LOG=… -ENGINEINI=… -GAMEINI=… -INPUTINI=…
-UIINI=… -nullrhi -skipnativepkgs=OnlineSubsystemPC` — no `-seekfreeloadingpcconsole`, and the baseline behaves exactly as with
it (`Initial startup` reached, then the AD blocker). Both runs were made with the same tool file the other agents import
(`prepare_run` keeps `run_game`'s behaviour; the argument list is a superset of the wave-3 one).

## 3. Debug tools (step 4)

Tests on the baseline exe (`build\agentAK`, `DishonoredGame_AK.exe`), outputs in `build\agentAK\dbg\`:

| Run | Command | Result |
|---|---|---|
| `dbgrun.py --tag baseline` (`--hang 90`) | stage + `dbg.py --hang=90 … -nullrhi -skipnativepkgs=OnlineSubsystemPC` | exit 3 = hang dump after 90 s: 28 threads, the main thread in `appOutputDebugString ← FOutputDeviceDebug::Serialize ← … FMaterial::InitShaderMap ← UMaterialInstance::PostLoad ← UObject::EndLoad ← LoadStartupPackages ← FEngineLoop::Init`, i.e. still loading — a debugged run is 5–7× slower (every log line is an `OutputDebugString` debug event); `--hang` default raised to 400 s and the README says how to read this |
| `dbgrun.py --tag baseline2` (`--hang 400`) | same | the game reached its `appErrorf` (`[0183.70] Critical: appError called: Bad export index …`) but `dbg.py` let it exit (exit 0): the `int 3` of `appDebugBreak()` in 32-bit code arrives at a 64-bit debugger as `0x4000001F` (`STATUS_WX86_BREAKPOINT`), the same code as the WOW64 loader's initial breakpoint, which Z's loop (and mine, first version) skipped |
| `dbgrun.py --tag baseline3` (fixed `dbg.py`) | same | **exit 1, `appErrorf/DebugBreak 0x4000001f`** with the full EBP chain: `ULinkerLoad::operator<<(UObject*&)+0xf2 ← operator<<(AActor*&) ← FActorReference ← FCoverReference ← TArray<FCoverReference> ← operator<<(FNavMeshPolyBase&)+0x81 ← TArray<FNavMeshPolyBase> ← UNavigationMeshBase::Serialize+0x4b7 ← ULinkerLoad::Preload ← FAsyncPackage::CreateExports ← FAsyncPackage::Tick ← UObject::ProcessAsyncLoading ← FlushAsyncLoading ← UObject::CollectGarbage ← UGameEngine::CommitMapChange+0xb2f ← ConditionalCommitMapChange ← UGameEngine::Tick ← FEngineLoop::Tick` (`build\agentAK\dbg\baseline3.txt`, 27 frames) |
| `stack_sample.py <map> --after 8 --kill <exe> …` | launch, sample at 8 s, kill | 26 threads dumped, main thread in `FOutputDeviceConsoleWindows::Serialize ← … FMaterial::InitShaderMap ← UMaterial::PostLoad ← UMaterialInstanceConstant::PostLoad ← UParticleSpriteEmitter::PostLoad ← UParticleSystem::PostLoad ← … LoadStartupPackages`; the process was resumed and then killed as asked |

`dbg.py`'s breakpoint rule now: a breakpoint (0x80000003 or 0x4000001F) whose EBP chain has no frame inside the exe image is
a loader/system breakpoint (continue); one with a frame of ours below it is the game's `appDebugBreak()` (stop, dump). That is
the "appErrorf stop" of step 4. Hand-over value for AD: the baseline3 stack says the bad index is read by
`FCoverReference` inside `operator<<(FArchive&, FNavMeshPolyBase&)` — the reference `FNavMeshPolyBase` serializer reads a
`TArray<FCoverReference>` the 2013 layout (0x2780f0, licensee-27 branch) does not have at that point.

## 4. `make_snapshot.py` (step 1, accept line part 3)

Refusal test (`build\agentAKtest_wt\sub\link` = junction to `build\agentAK\junk_target`, a scratch dir inside `build`, never the
retail or reference tree):

```
python resources/tools/make_snapshot.py AKtest resources/tools/make_snapshot.py
  REFUSED: D:\...\build\agentAKtest_wt contains 1 reparse point(s), e.g. ...\sub\link -> \\?\D:\...\build\agentAK\junk_target.
  Remove them with `python resources/tools/unlink_junctions.py ... --apply` (links only) and rerun.          exit 1
python resources/tools/make_snapshot.py AKtest --remove
  link: ...\sub\link -> ...   REFUSED: 1 reparse point(s) inside ... Run unlink_junctions.py ... first        exit 1
python resources/tools/make_snapshot.py AKtest2        (build\agentAKtest2_wt\build\stage exists)
  REFUSED: ...\build\agentAKtest2_wt\build\stage exists: a stage directory ... is never part of a snapshot     exit 1
```

Cleanup went through `unlink_junctions.py build/agentAKtest_wt --apply` (1 link removed, `junk_target` intact afterwards) and
`rmdir` of the empty directories. Positive test: `make_snapshot.py AK <9 tool/doc files>` created `build\agentAK_wt` (detached
4035b87, 4,462 files), copied 9 files, wrote `build\agentAK\snapshot_files.txt` and `build\agentAK_wt_build.cmd`; `--sync`
reported `0 copied, 0 removed, 9 unchanged`. `git worktree list` shows the new worktree.

## 5. Edge in-engine check and memo (step 5)

`edgeanim.md` §7 holds the memo. The in-engine check needs an NPC in a loaded map, i.e. AD's serializers and AF's
`-startmap`; at the end of my run neither is in the shared tree (`git status` shows only my files; AD/AF/AJ work in their
`build\agent*_wt` snapshots) and the baseline still ends at the `Bad export index` in `Dishonored_MainMenu_Env.upk`. The
check is **pending** with its procedure written down (§7.3: same NPC default vs `-edgerefpose`, poses finite, `w` sign
against `RefSkeleton`). The memo's recommendation for wave 5 is **keep Plan B**, with the three triggers that would flip it
(§7.4) and the ≤ 1-day items that stay under Plan B (additive order check against `_edgeAnimBlendAdditive`,
the finite-pose probe, `ExtractRootMotionCurve` on the Edge locomotion delta).

## 6. Optional items

- **Root set 63,718 vs golden 51,634.** Both logs presize for the same 58,555 GC-exempt objects, so the delta is in what the
  initial load pulls in, not in a counter. Ours reaches the line at 2.71 s (golden 1.92 s). What our log shows before the line
  and the golden does not: ~500 `Missing cached shader map for material … compiling` / `Can't compile … on console, will
  attempt to use default material` pairs (AG's material caches; these create `FMaterialResource`s, not `UObject`s) and
  `LocalizationWarning`s for `Default__DisDLC05/06…` class defaults (the 2013 `DishonoredGame.upk` classes, loaded by both
  exes). The golden shows `Ply_Player.Skm_Player has invalid UserBounds` (that mesh is in its initial load; ours does not log
  it). Not explained; the cheap check is a `DISHONORED(bringup)` line at the root-set message listing object counts per
  outermost package (UnGame.cpp / UnObj.cpp, not my files — coordinator or AI).
- **`jmarshall` leftovers** (reference-tree edits by the previous porter, all `// jmarshall … // jmarshall end` blocks):
  `Core/Inc/UnMath.h:905–923` (`FVector::GetXAxisVector/GetYAxisVector/GetZAxisVector` additions — harmless helpers, not in
  retail), `Engine/Src/SplashScreen.cpp:396–399` (`appSetSplashText` returns early for `StartupProgress` "too noisy" — a
  behaviour change vs retail, which draws the splash progress), `Engine/Src/UnAnimPlay.cpp:225–233` (the
  `bIsIssuingNotifies && AnimSeqName != InSequenceName` guard of `UAnimNodeSequence::SetAnim` commented out — a behaviour
  change; retail 2013 `SetAnim` should be checked before keeping it). None edited (not my files).

## What is left

1. Edge in-engine check (§5) once AD + AF land: run the §7.3 procedure from an AK snapshot (`make_snapshot.py AK --list …` with
   their files), then fill §7.3 with the result and revisit §7.4 if a trigger fires.
2. The `-loadall` / `--forbid` combination for AD's accept line is ready: `--forbid "Bad export index" --forbid "Serial size
   mismatch" --forbid "native not ported"` with `--expect-count "Committed map change via DishonoredEngine=2"` for AF.
3. Coordinator, at merge: the AkAudio/DishonoredGame/DisJobs index-case fix is already in HEAD (PHASE6.md's tracker note about
   AK's commit is outdated); the stale `external\*-build` / `*-subbuild` directories can go.
4. Follow-ups outside my files: root-set per-package count line (above); the `jmarshall` behaviour changes in `SplashScreen.cpp`
   and `UnAnimPlay.cpp` checked against 2013 (AI / AE).
