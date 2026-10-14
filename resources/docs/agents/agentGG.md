# Agent GG — the spawn order: possess after the floor exists

Wave 24 (`PHASE20.md`, package GG). Worktree `build/agentGG_wt`, base `4b889ff`, with main `b90cebf` (GH, GK,
docs) merged by `git merge` before hand-back - fast-forwards, no conflict, no source overlap. Working build `build/agentGG_rel`
(`build/agentGG/gg_build.cmd` → `build_queued.py`, `gg_build0.log`: one invocation, 0 errors / 0 C4263 / 0 C4264), the
gate's in-harness build `build/agentGG_reg` (`gg_gate.cmd`, `gg_gate.log`). Control `build/head_rel` (rebuilt at `4b889ff`,
exe of 15:04) staged as `DishonoredGame_GGctl.exe`; this build staged as `DishonoredGame_GG.exe`. Every launch through
`build/agentGG/gg_run.py` → `game_input.py launch` (`-inputcmd`, `-unattended`, `-windowed`, the build directory's private
inis) or the harness (`run_regression.py --only ...`); every capture in-process and viewed. No retail `Dishonored.exe`
launch; no real input; no FModel. IDA: private copies `build/agentGG/idb/GG_retail2013.i64` / `GG_shipping2012.i64`; the
2013 one read through the idalib MCP, the 2012 one headlessly (`ge_decomp.py` / `ge_xrefs.py`, the MCP worker for it was
evicted by the other agents' sessions), decompiles in `build/agentGG/decomp2012/`.

## 0. The answer in six lines

1. **Retail never ticks the new map before its streaming levels are visible.** `UDishonoredEngine::LoadMap` (2012
   `0x64cf10`, 2013 `0xa15210`) is `UGameEngine::LoadMap` followed by `this->PostCommitMapChange()` (2012 `0x65d6b0`, 2013
   `0xa152f0`), whose first act is **`UWorld::FlushLevelStreaming( NULL, FALSE )`**: every streaming level the map wants
   is loaded and made visible, AddToWorld's time slice suspended, before LoadMap returns. This tree had neither override:
   the levels came in through `UGameEngine::Tick`'s per-frame `UpdateLevelStreaming` at 5 ms / 50 components a frame,
   so the possessed pawn fell for ~1.4 s through a floor that was still being attached (GE §4).
2. Ported (§2): `UDishonoredEngine::LoadMap`, `::PostCommitMapChange` (the streaming half; the save-state half is named
   and left to agent ED's `RestoreLoadedLevels`), `::CancelAllPendingAutosaves`; `UWorld::AddToWorld`'s three budgets
   read `AWorldInfo::m_fAddWorldTimeLimitPerFrame` / `m_NumComponentsToUpdatePerFrame` / `m_NumActorsToInitPerFrame` as
   retail does (config: 0.005 s / 50 / 50 — the same numbers the reference hard-codes, so no behaviour change there).
3. Measured (§3): GG1 `ggdiag PostCommitMapChange l_tower_p: FlushLevelStreaming took 0.082 s, 7 of 8 streaming levels
   loaded, 7 visible, 8 levels in the world, PhysX static shapes 1312`; the first physics tick sees `895 actors, 1312
   static shapes` (control GGC1, same minute: `6 actors, 4 static shapes`); **0 `gedeathdiag` damage lines**, HUD health
   `100/100` on every census to the quit at +13 s. Control GGC1 (a 37 s startup under the other agents' builds): KillZ at
   2.36 s, `1000 KillZDamageType`, health 0 — the capture `ggc1b` is the view from under the world.
4. **The New Game route never fell** (premise corrected, §1): `-finewgame` travels by `SeqAct_PrepareMapChange` +
   `CommitMapChange` (STREAMMAP), not `LoadMap`, and the reference `UGameEngine::CommitMapChange` already makes every level
   visible before the player enters. Control and this build both read health `100/100` and 0 damage lines over 300 s.
5. The d3d9 loop (§5): **20 of 20 clean on the gate exe** - 0 `DmgType_Fell`, 0 deaths, health 100/100, 1312 static shapes at the first physics tick in every log, three of them slow loads (35-44 s) that would have been the control's KillZ case. Control loop: **20 of 20 take 57-79 `DmgType_Fell`** (mean 66.6, health 21-43) at 1.3-1.5 s, 4 static shapes at the first tick.
6. Gate: §6. Builds: 0 errors, 0 C4263, 0 C4264 in `gg_build0.log` and the gate's log.

## 1. Where the brief was wrong, with the measurement

| the brief said | what is measured |
|---|---|
| "find where retail's load order puts `InitWorldRBPhys` / the static actors relative to `SpawnPlayActor`/`Possess`" | The order inside `UGameEngine::LoadMap` and `UWorld::BeginPlay` is the same in retail and in this tree (2012 `0x250ff0` / `0x3b37a0`: `UpdateComponents`, `InitWorldRBPhys`, `InitGame`, `RouteBeginPlay`, then `SpawnPlayActor`, then `LayoutPlayers` + `FViewport::Draw`). The persistent level has almost no geometry (the GE logs: `0 static shapes` and `0 triangle meshes` at the first tick); it lives in the eight streaming levels of `L_Tower_P`, and the difference is what happens *after* LoadMap: retail flushes them synchronously in `UDishonoredEngine::PostCommitMapChange`; this tree let them trickle in. |
| "`AGameInfo::Login` / `RestartPlayer` / the Dishonored overrides" | Not on the path: possession happens inside `SpawnPlayActor` on both (the cooked script's `bDelayedStart` is FALSE, `ADishonoredGameInfo::CanStartMatch` 2012 `0x62fb70` returns 1 unconditionally). |
| "check the New Game route (`-finewgame=50`) the same way" | It never fell: no `LoadMap:` for `l_tower_p` on that route (STREAMMAP `CommitMapChange`), 0 damage lines and health `100/100` on control and on this build. Only the `-startmap` route (the gate's d3d9 stage, every agent's `ge_run`/`gg_run` launch) had the fall. |
| "possess after `InitWorldRBPhys` has the static actors, or hold the pawn's physics" (GE §4) | Neither is what retail does; retail possesses first and then blocks on level streaming before the first tick. |
| GE: "the spawn order in this tree's `UGameEngine::LoadMap` / `UWorld::Tick`" | `UWorld::AddToWorld`'s time slice (reference 5 ms / 50 per frame; retail's config members hold the same values) is where the floor was being attached frame by frame — it is retail's mechanism too, retail just does not tick the world until the flush is done. |
| `ge_run.py`'s `newgame` route (`-finewgame=50 -fosoiree` without `-playdefaults -fmmenu -fmdrive`) | never leaves the menu (GG2: 103 s on `DishonoredGameFull_P`); the harness's own newgame stage (`run_regression.py --only newgame`) is the route that works, and is what §3 measures. |

## 2. The port, resolved by hand in both databases

| function | 2012 | 2013 | what |
|---|---|---|---|
| `UDishonoredEngine::LoadMap` | `0x64cf10` (218 B) | `0xa15210` (0xd8) | cancel pending autosaves + the mission-start save, zero the 21 disable-save counts (2012: one count + a debug list), the disallow timer and the two ignore-autosave counts, drop the local player's `ActorVisibilityHistory`, `UAkAudioDevice::StopAllSounds`, `UGameEngine::LoadMap`, `PostCommitMapChange` (vtable slot: 2012 `??_7UDishonoredEngine@@6BUObject@@@` `0xce6818` +384, 2013 `0xcdb390` +404). Not ported: the 2013-only `SLC_WaitValidation` arm (release of `FGameState`'s save archive, `ProcessSaveComplete(0, TRUE)`). |
| `UDishonoredEngine::PostCommitMapChange` | `0x65d6b0` (432 B) | `0xa152f0` (0x2d9) | drop the visibility history, **`FlushLevelStreaming( NULL, FALSE )`**, `PrecomputedVisibilityHandler.UpdateScene` on the persistent level (2013 `GWorld+708 → +692`). Named, not ported: the two engine bits at +1812, `FGameState`'s level-state reset and `LoadLevel` on the current level unless `IsLoadingGame` (ED's `RestoreLoadedLevels` is this tree's `SLC_PostLoad` arm), `UDisNPCTravelManager::PrepareForArrivals` (2012 `0x721070`, unported), the HUB chapter reset (2013), the autosave queued on entering a non-menu map (`TryDoAutosave` unported), the DLC07 crosshair bit (2013). |
| `UDishonoredEngine::CancelAllPendingAutosaves` | `0x64ccf0` | `0xa070b0` | deny every pending autosave action, clear the slot |
| `UWorld::AddToWorld` + its static `IsTimeLimitExceeded` | `0x3bcfd0` / `0x3a24d0` | `0x79b900` / `0x780f80` | `m_fAddWorldTimeLimitPerFrame` (2013 WorldInfo +1532), `m_NumComponentsToUpdatePerFrame` (+1536), `m_NumActorsToInitPerFrame` (+1540) in place of the reference's 5 ms / 50 / 50 (Engine `UnWorld.cpp`, hand-edited, said so in the file) |

`UDishonoredEngine::PreCommitMapChange` (2012 `0x65d5d0`, slot +380 / 2013 +400) is reached only from `UGameEngine::CommitMapChange`
through `ADishonoredGameInfo::PreCommitMapChange` (2012 `0x661260`), neither on this package's path; not ported. Retail's
`UWorld::FlushLevelStreaming` (2013 `0x79cc10`) is byte-for-byte the reference's (ratio 1.000). The diag: one
`DISHONORED(bringup): ggdiag PostCommitMapChange <map>: ...` line per map (flush time, levels loaded/visible, static shapes),
unconditional like the other bring-up lines.

## 3. The runs

| run | exe | route / switches | result | captures |
|---|---|---|---|---|
| GG1 | this build | startmap, `gg1_steps.txt`, `-gedeathdiag` | `ggdiag ... 7 of 8 streaming levels loaded, 7 visible, static shapes 1312`; first physics tick `895 actors, 1312 static shapes`; 0 damage lines; health 100/100 at every census (+0.9 s .. +13.4 s) | `gg1a` (+3 s, on the boat, health vial full), `gg1b` (+7 s) |
| GGC1 | control | same | `6 actors, 4 static shapes` at the first tick (2.9 s after possession, startup 37.3 s); `gedeathdiag t=2.36 Died ... KillZDamageType`, health 0 | `ggc1a`, `ggc1b` (the view from under the world) |
| GG2 | this build | `gg_run.py --route newgame` (GE's route) | never left the menu — the route lacks the harness's `-playdefaults -fmmenu -fmdrive`; superseded by the next row | — |
| GG newgame | this build | `run_regression.py --build-dir agentGG_rel --no-build --only newgame --no-load-retry --game-extra-args=-gedeathdiag` | 5 ok; `l_tower_p` at t 50.65 by STREAMMAP (no `LoadMap:` line), 29 pawns, 0 damage lines, health 100/100 to 299 s | `build/agentGG/GG_newgame.log` |
| GGC newgame | control (`--build-dir head_rel`) | same | 5 ok; the same: 0 damage lines, health 100/100 | `build/agentGG/GGC_newgame.log` |
| loop | gate exe | `gg_deathloop.py --build-dir agentGG_reg --runs 20` | §5 | `build/agentGG/deathloop/d3d9_<n>.log` |
| control loop | `head_rel` | `gg_deathloop.py --build-dir head_rel --runs 20 --out deathloop_ctl` | §5 | `build/agentGG/deathloop_ctl/` |

Logs `build/agentGG/GG1.log`, `GGC1.log`, `GG2.log`; captures (BMP + PNG) `build/agentGG/captures/<run>/`. Every "Initial
startup" of this afternoon read 32–37 s (GE's loop: 4.75 s) — four other agents were building and running at the same time;
the fix does not depend on it (the flush is synchronous), the control's death does (a slow load is the KillZ case).

## 4. Files

`build/agentGG_sync.py`'s header names every file and function. Engine (reference file, hand-edited, said so):
`UnWorld.cpp`. DishonoredGame: `Inc/CppText/UDishonoredEngine.h` (three declarations appended), `Src/dishonoredengine.cpp`
(three bodies + one extern). No reflected member added or moved; no generated file touched; no exclude-list change.

## 5. The loop

`gg_deathloop.py` (GE's, with `--out` and counting all N runs instead of stopping at the first death): each run is
`run_regression.py --build-dir <dir> --no-build --only d3d9 --no-load-retry --game-extra-args=-gedeathdiag` from the worktree's
copy, the stage log copied aside. The first attempt ran under three other agents' builds (31 `cl` processes) and every run
stalled at `Initializing Engine...` until the 90 s stage timeout (358 lines, no map) - discarded, re-run once no compiler was
running (the other agents' game runs stayed).

| loop | runs | reached the map | `DmgType_Fell` | deaths | health after spawn | first physics tick | startups |
|---|---|---|---|---|---|---|---|
| gate exe `agentGG_reg` (`build/agentGG/deathloop/d3d9_1..20.log`) | 20 | 20 (`ggdiag ... 7 of 8 streaming levels loaded, 7 visible` in every log) | **0** (0 damage lines of any type) | **0** | 100/100 | `895 actors, 1312 static shapes` in all 20 | 4.3-6.9 s (17 runs), 35.4 / 41.8 / 43.9 s (runs 10-12, under a build - the control's KillZ profile, clean here) |
| control `head_rel` (`build/agentGG/deathloop_ctl/d3d9_1..20.log`) | 20 | 20 | **20 of 20**: 57-79 points (mean 66.6) at t 1.30-1.50, landing speed 2,625-2,765 u/s on 52-1,097 static shapes | 0 in the loop (the KillZ death is GGC1 §3 and GE's run 3: a 37-42 s load) | 21-43 | `6 actors, 4 static shapes` | 3.9-4.4 s (14 runs), 22.7-40.1 s (6 runs) |

Every one of the 40 runs' d3d9 stage read `ok, 0 failed` (the control's fall is not a gate check). The one streaming level of
eight that stays unloaded on every run is the same on control and here (the flush loads what `ShouldBeLoaded` answers;
its name is not in the diag line - a follow-up if anyone needs it).

## 6. Acceptance

| check | result |
|---|---|
| gate `run_regression.py --build-dir D:/RecompileDishonored/Recompile/build/agentGG_reg --newgame-at 50 --game-extra-args=-windowed` from the worktree's copy, built in-harness (the package's second build) | **47 ok, 0 failed, 0 skipped**, 2280 s; `build/agentGG/gg_gate.log`, `build/agentGG_reg/regression/summary.txt` |
| the gate's build | `build_DishonoredGame_errors 0`, 0 `error C` / `error LNK`, **0 C4263, 0 C4264** (`build/agentGG_reg/regression/build_DishonoredGame.log`, through `build_queued.py`); `coresmoke` 99/0, `layout_types` 2314, `verify_phase2_total 3` |
| the rows | `unported_natives 0 (<= 0)`, `nullrhi` / `d3d9` / `inputtest` / `newgame` / `options` `_criticals 0`, `d3d9_startup_seconds 5.2`, **`d3d9_frames 3180`** (>= 1000, no re-run needed), `inputtest_moved 1007.4`, `newgame_pawns 29` |
| `-gedeathdiag` on the startmap route | GG1: 0 `DmgType_Fell`, 0 damage lines of any type, health 100/100 at +5 s (census at 38.15 s = +3.8 s after possession and at every later second); control GGC1: KillZ death at 2.36 s |
| the New Game route | 0 damage lines, health 100/100 on this build and on the control (it never fell; §1) |
| the sync | `python build/agentGG_sync.py`: 5 files, **0 flagged, 0 missing** against main `b90cebf` (merged into the worktree by `git merge` - fast-forwards at `99eaf6b`, `9c44722`, `e3bc866`, `b90cebf`, no conflict); `git diff --name-only 4b889ff` + `ls-files --others` minus main's own files = the sync list (the script checks both ways) |
| the rules | every launch `gg_run.py` / the harness → `game_input.py launch` (`-inputcmd -unattended -windowed`); no real input; no retail `Dishonored.exe`; no FModel; two builds through `build_queued.py` (working + gate); every address resolved by hand in both databases |
