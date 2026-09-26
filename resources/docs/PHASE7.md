# Phase 3 wave 5 — from a walkable world to a reacting one

Plan of record for the wave after `PHASE6.md`. Written 2026-09-26 by the coordinator, on the user's
instruction to take the two defects found by running the game (corrupt textures, inert triggers and
volumes) and build the wave around them.

## Where this wave starts (HEAD `90571b6`)

The game boots, loads the first mission map with its sub-levels, renders it, and the player pawn stands
on the world and walks. Measured on the release build, d3d9 windowed 1280x720:

| | value |
|---|---|
| startup | 3.2 s |
| frames, 90 s run | 20,370, **0 critical errors** |
| static mesh elements drawn per frame | 979 of 6,381 (was 0 of 6,381) |
| pawn | stands on the intro boat, `PHYS_Walking`, `-inputtest moved` **1030.1** (was 0.0) |
| physics scene | 895 actors, 1,312 static + 59 dynamic shapes, 413 convex meshes (was 2 actors, 1 shape) |
| natives without a body on the path | 3 (`HandleHeldButtons`, `Dis_Zoom`, `GameEnding`) |

Launch it with `resources\play.cmd [map] [resx] [resy]`. Audio is **deliberately out of scope** for this
wave and every wave until the game plays (`PLAN.md` Phase 10, the user's call).

## Facts fixed while planning

| Fact | Consequence for the packages |
|---|---|
| The two defects the user saw are one rendering bug and one gameplay bug, and they are independent | AR and AS run in parallel and share no file |
| Textures are corrupt on large surfaces while sky, untextured surfaces and silhouettes are clean, with **no load or shader warning in the run** | Not materials, not geometry, not the shader caches (which are byte-verified against the cook). It is the RHI upload or the streamed-mip path: `build\agentAP\shot_pub.png` is the exhibit and `-apshot=N` the repro |
| Arkane replaced `BlockZeroExtent`/`BlockNonZeroExtent` with the 7-bit `FDisPrimTraceMask m_CollisionTraceTypes` (@320); those booleans are FALSE in every cooked package | Agent AQ converted the octree and encroachment gates, which is why the pawn stands. **The overlap and touch gates were not converted**: `UnActor.cpp:2783/2793`, `2885/2899` (`IsOverlapping`, the box-overlap helper) and `2431` (`FindTouchingActors`) still read the dead booleans, so nothing ever touches anything |
| `SetDefaultCollisionType`/`SetCollisionType` (`UnActor.cpp:2059..2217`) run retail's mapping backwards | Retail's `SetDefaultCollisionType` (2013 rva **0x179a00**) derives `CollisionType` @263 *from* the mask; ours derives the mask from `CollisionType`. Fix with the touch gates, same package |
| `UPrimitiveComponent::execSetTraceBlocking` (`PrimitiveComponent.cpp:1288`) writes the dead booleans | Retail (**0x12a230**) reduces to `m_CollisionTraceTypes.SetAllMovementTrace(NewBlockNonZeroExtent); SetAllGameplayTrace(NewBlockZeroExtent);`. Any script call is silently a no-op until then |
| The retail scripts declare **36 volume, 16 trigger and 19 pickup classes**, plus `SeqEvent_Touch` | This is what AS unlocks: stealth volumes and line-of-sight triggers, possession/tether/suppression volumes, water and physics volumes, every pickup, hideouts, colour-scale and cull-distance volumes |
| `L_Tower_Water` is a `LevelStreamingKismet` sitting unloaded in the user's run, and the log carries `Attach to Event SeqAct_AttachToEvent_3 has no targets!` | Parts of levels never appear because Kismet is not driving them. That is AT, and it needs AS's touch events first |
| Agent AF's B2 is still open: after the menu map change the game thread stops in `USequence::ExecuteActiveOps -> UObject::IsA` (agent AL) / inside the allocator (agent AO) | Same subsystem as AT (Kismet), so AT owns it. It is what blocks the retail New Game route |
| 275 DishonoredGame natives remain stubbed, gated behind `UDishonoredAIBrain`, `UDisAISubProcess/SubState`, `FArkComponentLocomotion`, `UDisItemContext` | AU. Triage of all 295 rows is in `agents/agentAJ.md`; pickups also need their Dis* natives, so AU and AS meet at the pickup path |
| The pawn's Arkane anim tree asserts on an unported node, so the tweak anim tree is gated behind `-distweakanimtree` | AV. Characters are posed but not animating as Arkane intended |
| Occlusion behaved correctly once geometry existed (4,348 of 4,578 occluded inside a closed room) | The renderer's culling is sound; AR must not "fix" culling |

## Coordinator, before spawning (½ day)

- [ ] Fold the per-agent status rows (`agents/agent{AP,AQ}_status.csv`) into `function_status.csv`, regenerate
      `progress.md` and `module_map.md` (`module_map.py`).
- [ ] Update `STATUS.md` and `PLAN.md` Phase 6: milestone 5 **done**, milestone 4 done bar the menu.
- [ ] Record the baseline numbers above in this file so every package can diff against them.
- [ ] Commit this plan, then spawn AR–AX in one message.

## Work packages

Each package: own build dir `build\agent<X>`, own IDA copies `retail2013_agent<X>.i64` /
`shipping2012_agent<X>.i64`, report to `resources/docs/agents/agent<X>.md`, status rows to
`agent<X>_status.csv` (`rva,status,note`). Rules at the end.

### AR — Textures: the world looks right (lead package, the user's first defect)
Files: `D3D9Drv/Src/{D3D9Texture,D3D9Device,D3D9Viewport,D3D9Resource}.cpp` and their headers,
`Engine/Src/{Texture2D,Texture,UnTex,TextureStreaming,UnContentStreaming}.cpp`, `Engine/Inc/UnTex.h`.
- [ ] 1. Reproduce and characterise before changing anything: `-apshot=N` on `L_Pub_Day_P` (interior, worst
      case) and `L_Tower_P`. Classify which surfaces corrupt: by format (DXT1/DXT3/DXT5/A8R8G8B8/normal maps),
      by size, by mip count, by whether the texture is streamed or fully resident, by cooked vs engine content.
      A `DISHONORED(bringup)` census line per world (textures created, by format, by mip range, streamed vs not)
      is worth more than any guess, exactly as agent AP's scene census was.
- [ ] 2. The most likely causes, in order, each to be confirmed or eliminated **with evidence**: the mip
      offsets or pitch used when locking and filling a texture (a wrong pitch gives precisely this blocky
      diagonal noise); the cooked `FTexture2DMipMap` bulk-data offsets for seek-free packages; DXT block size
      and row stride for compressed formats; `TEXTUREGROUP` LOD bias dropping to a mip that was never uploaded;
      the streamed-mip request path uploading into the wrong mip level.
- [ ] 3. Port the retail counterparts of whatever is wrong (decompile `UTexture2D::Serialize`, the D3D9
      texture creation and lock paths, and `FTexture2DResource::InitRHI` / the streaming request handler).
- [ ] 4. Check the same code with the null RHI unaffected, and confirm no regression in AP's scene census.
- **Accept**: `-apshot` screenshots of `L_Pub_Day_P` and `L_Tower_P` in which wall, floor and building
  textures read as their intended material (stone, wood, brick, water) with no colour noise; the texture
  census shows every created texture with a complete mip chain; 0 critical errors over a 60 s run; and a
  before/after screenshot pair in the report.
- **Report**: `agentAR.md`, with the census table and the screenshot pair.

### AS — Touch, triggers and volumes: the world reacts (lead package, the user's second defect)
Files: `Engine/Src/UnActor.cpp` (the overlap, touch and collision-type paths), `Engine/Src/PrimitiveComponent.cpp`
(`execSetTraceBlocking` only), `Engine/Inc/UnActorComponent.h` and `Engine/Inc/UnLevel.h` only if the mask needs
more accessors (agent AQ added `MatchesTraceFlags`, `SetAllMovementTrace`, `SetAllGameplayTrace` and the six
`TRACE_Dis*` flags — **reuse them, do not redefine**).
- [ ] 1. `execSetTraceBlocking` to retail (**0x12a230**), as above.
- [ ] 2. Convert the remaining dead-boolean readers to the mask, each against its retail decompile:
      `AActor::IsOverlapping` (`UnActor.cpp:2783/2793`), the box-overlap helper (`2885/2899`),
      `AActor::FindTouchingActors` (`2431`). Same substitution agent AQ used for the octree gates
      (`MatchesTraceFlags`), so read `agentAQ.md` first and match its style.
- [ ] 3. `SetDefaultCollisionType` / `SetCollisionType` (`2059..2217`) the right way round: retail
      **0x179a00** derives `CollisionType` @263 *from* the mask. The decompile is in `build/agentAQ_decomp`.
- [ ] 4. Instrument, then measure: a `DISHONORED(bringup)` line counting touch begin/end notifications,
      volume entries and `PhysicsVolume` changes per world. Walk the pawn with `-inputtest` and prove
      touches fire.
- [ ] 5. The volume path end to end for at least: `DefaultPhysicsVolume`/`DynamicPhysicsVolume`
      (`eventActorEnteredVolume`, `PhysicsVolume` assignment), `DishonoredWaterVolume`, `DisStealthVolume`,
      `TriggerVolume`/`DisTrigger`, and one pickup class. Anything whose script side needs a DishonoredGame
      native that is still stubbed is a hand-over to AU, named in the report.
- **Accept**: `-inputtest` with a new `-distouch` census showing non-zero touch notifications and at least one
  volume entry; walking into a trigger volume fires `SeqEvent_Touch` (log it); a pickup can be picked up, or
  the exact native that blocks it is named; 0 critical errors; and the collision numbers from `agentAQ.md`
  (895 actors, 1,312 static shapes) unchanged.
- **Report**: `agentAS.md`.

### AT — Kismet: mission scripting runs, and the menu teardown stops crashing
Files: `Engine/Src/UnSequence.cpp`, `Engine/Inc/EngineSequenceClasses.h` (native declarations only),
`Engine/Src/UnLevel.cpp` only for the Kismet-streaming path (coordinate with AR/AS, neither owns it).
- [ ] 1. Agent AF's **B2**: after the menu map change the game thread stops in `USequence::ExecuteActiveOps ->
      UObject::IsA` (agent AL's stack) and inside `FMallocBinned::GetAllocationInfo` (agent AO's). Both
      readings fit a stale sequence op of the torn-down menu level. Fix against retail's own
      `ExecuteActiveOps` / level teardown, and prove the retail New Game route (`-newgame`, agent AF's switch,
      which issues the menu's own `ce ChangeLvl_StartNewGame`) reaches the mission map.
- [ ] 2. `LevelStreamingKismet`: `L_Tower_Water` never loads because nothing drives it. Make Kismet-driven
      streaming work, and account for `Attach to Event SeqAct_AttachToEvent_3 has no targets!`.
- [ ] 3. The sequence ops the first mission's Kismet actually executes: port what runs, list what is stubbed.
- **Accept**: `-newgame` reaches the mission map with 0 critical errors; `L_Tower_Water` becomes visible when
  its Kismet trigger fires; a census line counts sequence ops executed per tick and events fired.
- **Report**: `agentAT.md`. Depends on AS for touch events: work from a snapshot until AS merges.

### AU — DishonoredGame gameplay: the AI brain foundation and the pickups
Files: the DishonoredGame units for the AI brain, sub-process and sub-state, item context and inventory, and
the pickup classes; `DishonoredGameNativeStubs.ported.agentAU.txt` (regenerate the module **in your snapshot
only**; the coordinator regenerates the shared tree at merge).
- [ ] 1. From `agentAJ.md`'s triage of all 295 remaining stubs, port the dependency roots first:
      `UDishonoredAIBrain`, `UDisAISubProcess` / `UDisAISubState`, `UDisItemContext`, `FArkComponentLocomotion`.
      Everything else is gated behind them.
- [ ] 2. Then the 19 pickup classes' natives, so AS's touch path can actually pick something up.
- [ ] 3. The three natives still firing on the path: `ADishonoredPlayerController::execHandleHeldButtons`,
      `execDis_Zoom`, `ADishonoredGameInfo::execGameEnding`.
- **Accept**: at least 120 natives ported; `-strictnatives` reaches the mission map without aborting; no
  DishonoredGame warn-once line on the walking path; a pickup picked up end to end with AS.
- **Report**: `agentAU.md`.

### AV — Arkane animation nodes: characters animate
Files: `Engine/Src/UnAnimTree.cpp`, `Engine/Src/UnAnimPlay.cpp` and the DishonoredGame anim-node units.
- [ ] 1. `DishonoredAnimNodeStatePicker` and the other Arkane nodes that make the tweak anim tree assert
      (`UnAnimTree.cpp:1583`, child weights all zero). Agent AJ gated the tweak anim-tree assignment behind
      `-distweakanimtree`; the goal is to **delete that gate**.
- [ ] 2. With the gate gone, check the player and one NPC animate: the Edge evaluator (wave 3, bit-exact) and
      the retail blend path already exist, so this is the Arkane node set, not the codec.
- **Accept**: the game runs without `-distweakanimtree` and with the tweak anim tree applied, 0 critical
  errors over 60 s, and the report states which nodes are ported and which still warn.
- **Report**: `agentAV.md`.

### AW — The main menu decision (Scaleform GFx) — investigation first, then a recommendation
Files: `resources/docs/middleware.md` (its GFx section), a new `resources/docs/gfx_decision.md`; no engine
files without saying so.
- [ ] 1. Establish what the menu actually needs: which GFx entry points the retail exe calls, whether the same
      "reconstruct the API from the binary" method that worked for PhysX, Steamworks and Wwise applies (GFx is
      **statically linked**, like Wwise, so there is no DLL to import — say so plainly if that kills route 1).
- [ ] 2. Cost the alternatives: reconstructed bindings with a stub renderer, a minimal replacement menu layer
      that drives the same script events, or leaving `-startmap`/`-newgame` as the entry point for now.
- [ ] 3. Recommend one, with the evidence. Do not implement without the coordinator's go-ahead.
- **Accept**: `gfx_decision.md` with the entry-point inventory, three costed options and a recommendation.
- **Report**: `agentAW.md`.

### AX — Verification: the load-all sweep and the regression harness
Files: `resources/tools/build_and_smoke.py`, `resources/tools/debug/`, a new
`resources/tools/run_regression.py`; `Engine/Src/DishonoredLoadAll.cpp` (agent AD's, extend only).
- [ ] 1. The milestone 3 exit check, now cheap: `-loadall` over **all 471 `.upk` and every `.pck`**, object
      counts and errors per package, as a committed baseline file.
- [ ] 2. A one-command regression run for the coordinator: build, CoreSmoke, LayoutProbe + `xcheck` +
      `compare` + `verify_phase2`, the null-RHI milestone run, the d3d9 render run with frame and census
      counts, and the `-inputtest` moved distance — each with its expected value, exit non-zero on regression.
- [ ] 3. Wire the census lines the other packages add (textures, touches, sequence ops) into it as counters,
      so a future wave cannot silently undo them.
- **Accept**: `run_regression.py` passes on HEAD and fails when a counter is deliberately broken; the
  load-all baseline is committed with its error list triaged (each error assigned to a package or explained).
- **Report**: `agentAX.md`.

## Coordination and merge

- **File ownership, never edited by two packages**: `D3D9Drv/*` and the texture units → AR;
  `UnActor.cpp` and `PrimitiveComponent.cpp` → AS; `UnSequence.cpp` → AT; DishonoredGame gameplay units → AU;
  the anim units → AV; the tools → AX. `UnLevel.cpp` is shared between AR (streaming mips) and AT (Kismet
  streaming): AT owns it, AR hands over.
- **Dependencies**: AT needs AS (touch events), AU meets AS at the pickup path, AV is independent, AR is
  independent, AW and AX are independent. AT, AU work from a snapshot until AS merges.
- **Merge order**: AX (tools) → AR (textures) → AS (touch) → AT (Kismet) → AU (gameplay) → AV (animation) →
  AW (decision doc). After each merge, on a clean `build/head_wt` worktree with layout checks on: CoreSmoke,
  `xcheck_sdk_layout.py`, `gen_layout_probe.py compare`, `verify_phase2.py retail`, the baseline smoke and
  the merged package's accept command. One commit per agent.
- **End of wave**: this tracker, `STATUS.md`, `PLAN.md` Phase 6, `progress.md`, memory.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| C8 | coordinator | Fold status rows, refresh STATUS/PLAN, baseline, this plan | todo | | |
| AR | | Textures: the world looks right | todo | | user-reported defect 1 |
| AS | | Touch, triggers and volumes: the world reacts | todo | | user-reported defect 2 |
| AT | | Kismet: mission scripting runs, menu teardown fixed | todo | | needs AS |
| AU | | DishonoredGame gameplay: AI brain foundation, pickups | todo | | |
| AV | | Arkane animation nodes | todo | | |
| AW | | Main menu decision (GFx) | todo | | investigation only |
| AX | | Load-all sweep and regression harness | todo | | merge first |

## Rules for agents

- Edit only the files your package names. If another package's in-flight edits break the shared tree,
  snapshot with `python resources/tools/make_snapshot.py <X> <files>` and build there.
- Own build dir `build\agent<X>`; `cmd /c resources\build-game.cmd` (Debug, asserts and layout checks) or
  `resources\build-release.cmd` with `BUILD_DIR` pointing at your own dir for a fast run (startup ~3 s).
  **Never edit `resources\play.cmd` or `resources\build-release.cmd`.**
- Run only through `build_and_smoke.py --build-dir build/agent<X> --no-build --exe-name DishonoredGame_<X>.exe
  --log-name agent<X>.log --ini-dir build/agent<X>/config ...`. Use the `=` form for extra args
  (`"--extra-args=-forcelogflush -startmap=L_Tower_P"`), always pass `-forcelogflush` or the log truncates
  mid-line, and use `--forbid`/`--expect-count`. `resources/tools/debug/dbgrun.py` gives full crash stacks.
- Own IDA copies, headless only (`resources\tools\ida\run.py ... decompile_funcs.py`); **never the IDA MCP
  tools and never the FModel MCP tools** (FModel is UE4-only and the user has forbidden it).
- Never create junctions or symlinks into the retail or reference trees, never recursively delete a directory
  that may contain one, never delete anything under `D:\RecompileDishonored\Dishonored_Latest2026`.
- No commits, no `git add`. Every edit tagged `// DISHONORED(port|written|layout|retail|bringup): <2013 rva>`.
  Sources are CRLF; write patch scripts with the Write tool. The Engine link is near its size limit, so no
  inline-static members in widely included headers, and `/Zc:alignedNew-` must stay in `CMakeLists.txt`.
- **Measure, do not assume.** Every package in waves 4 and 5 that found its bug did so by instrumenting first
  (agent AO's watchpoint, AP's scene census, AQ's reflected-offset dump). A census line is cheap and is what
  turns "it looks wrong" into a number the next agent can diff.
- **Audio is out of scope** (`PLAN.md` Phase 10).
