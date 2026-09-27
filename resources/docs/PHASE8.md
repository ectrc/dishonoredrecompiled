# Phase 3 wave 6 — the world in the right place, the real menu, the real look

Plan of record after `PHASE7.md`. Written 2026-09-27 by the coordinator on the user's instruction: the map
is disjointed and must be fixed, and the project is to have **the full menu and the full graphics**, not a
stand-in. The user is away; this wave runs to completion and the coordinator continues into wave 7 without
waiting for review.

## Where this wave starts (HEAD `0930321`)

Wave 5 merged all seven packages and the regression harness passes on the result: 19 checks, 0 failures.

| | value |
|---|---|
| d3d9, 90 s | 20,190 frames, **0 critical errors**, 1,564 draw elements per frame |
| pawn | stands, walks 1,011 units, 48 of 79 bones animating |
| world reacts | 46 trigger events, volumes entered, **122 pickups collected** in one run |
| entry | the retail New Game route works: two committed map changes, menu then mission |
| textures | 16,257 with complete mip chains, 0 read from the wrong file |
| natives unported on the walking path | 2 |

Audio stays out of scope (`PLAN.md` Phase 10) with one caveat now known: one scripted sequence in the first
mission waits on an audio end-of-event callback, so that sequence branch will not advance until audio runs.
Agent AT left the two-part fix measured and ready in `build/agentAT_patch{11,12}.py`.

## The user's three asks, and what they mean here

1. **The map is disjointed.** Reported after playing: "nothing is in the right place other than terrain and
   some buildings". So the large individually-placed geometry is correct while something else is not. This is
   package **BA** and it leads the wave.
2. **The full menu.** Agent AW established (`gfx_decision.md`) that Scaleform cannot be imported: it is
   statically linked, 5,635 functions and 1.13 MB inside the executable, with no symbol in any shipped DLL.
   A stub backend unlocks nothing, because the menu's layout and navigation are ActionScript inside the cooked
   assets. The user has chosen the full thing, so we reconstruct the runtime from the 2012 symbols the same way
   agent AL reconstructed PhysX from its PDB — 93 % of those functions are byte-identical between the two
   builds, so the symbols describe the retail runtime exactly. **This is a multi-wave project**: packages
   **BB**, **BC** and **BE** here are its first wave, each with an acceptance that stands on its own.
3. **The full graphics.** The renderer runs and the cooked shader caches match, but 136 shader types are still
   undeclared and they are the Arkane and GFx post-process families: the look of Dishonored. Package **BD**.

## Coordinator, before spawning

- [ ] Fold the wave-5 status rows into `function_status.csv`, regenerate `progress.md`.
- [ ] Record the baseline above; `run_regression.py --build-dir build/release --no-build` is the gate every
      package must still pass at merge.
- [ ] Promote the tools three agents asked for: `sym.py` (2012 and 2013 addresses for a name) and `off.py`
      (the retail member at a byte offset) from `build/agentAU_work`, `disp.py` (displacement-operand search)
      from `build/agentAT`, `crlf.py` from `build/agentAV_work`, into `resources/tools/ida/`.

## Work packages

### BA — The world in the right place (lead package, the user's defect)
Files: `Engine/Src/UnLevel.cpp`, `Engine/Src/UnLevAct.cpp`, `Engine/Src/PrimitiveComponent.cpp`,
`Engine/Src/UnActorComponent.cpp`, `Engine/Inc/UnActorComponent.h`, `Engine/Src/UnWorld.cpp` (streaming
association only; agent AT owns the Kismet path in `UnSequence.cpp`).
- [ ] 1. **Measure before touching anything.** A `DISHONORED(bringup)` placement census: per streamed level,
      the actor count, the bounding box of their locations, and for each of the first N actors its class,
      location, rotation and draw scale; then, per primitive component, its own translation, rotation and
      scale against the composed component-to-world. The question to answer first is *which* things are
      misplaced: actors of one sub-level relative to another, or components within an actor.
- [ ] 2. The strongest hypothesis, to confirm or kill first: Dishonored's cooked levels merge many meshes into
      `AStaticMeshCollectionActor` (retail 608 bytes, an array of components plus a maximum), and each of those
      components carries its own translation, rotation and scale. If the component-to-world composition ignores
      or mis-orders them, every merged prop lands somewhere wrong while individually-placed geometry, which is
      what terrain and large buildings are, stays correct. That matches the user's description exactly. Check
      `UPrimitiveComponent`'s transform composition against retail, including the order of scale, rotation and
      translation, the parent-to-world update, and what `AStaticMeshCollectionActor` does at load.
- [ ] 3. If that is not it, the next candidates in order: a per-level offset applied on association; actor
      locations lost or reset by `PostLoad`/`ConditionalUpdateComponents`; `StaticMeshActorBase`'s cached
      transform; the base and attachment chain (`Base`, `BaseSkelComponent`, `RelativeLocation`) which agent AQ
      showed retail uses differently; brush/model geometry placed separately from static meshes.
- [ ] 4. Fix against retail decompiles, every change citing its 2013 rva.
- **Accept**: a screenshot pair of the same camera in the first mission map and the hub before and after, the
  placement census showing per-level bounds that agree between related sub-levels, and the regression harness
  still green. If part of the defect is outside this package, name it precisely.
- **Report**: `agentBA.md`.

### BB — GFx runtime, foundation: the API and the renderer seam
Files: new `source/Development/Src/External/GFx3/` (headers and our own implementation), new
`cmake/GFx.cmake`; `GFxUI/Src/gfxuirenderer.cpp` and the other `GFxUI` glue units.
- [ ] 1. Reconstruct the GFx 3.3 API from the 2012 symbols with `dia_types.py` exactly as agent AL did for
      PhysX: the classes the engine names (`GFxLoader`, `GFxMovieDef`, `GFxMovieView`, `GFxValue` and its
      object interface, `GRenderer`, `GTexture`, `GRenderTarget`, `GFxFileOpener`, `GFxImageLoader`,
      `GFxTranslator`, `GFxExternalInterface`, `GMemory`/allocator), each member at its PDB offset and each
      virtual at its PDB vtable slot. **Read agent AL's report first**: consecutive virtual overloads are laid
      out in reverse declaration order, which is exactly the trap this work will hit.
- [ ] 2. Implement the seam our engine must supply, which is unported today: the 54-slot renderer, the texture
      and render-target interfaces, the file opener and image loader. `gfxuirenderer.cpp` has 221 functions to
      port; the tree's existing `Scaleform*.cpp` are GFx 4 glue and are **not** a head start.
- [ ] 3. Prove the seam with the simplest possible consumer: load one cooked `.gfx` asset through our loader
      and report its header, its exported symbols and its frame count. Parse the container format for real —
      agent AW flagged that nobody has yet checked whether the assets are `gfxexport` output with bitmaps
      stripped into engine textures, and it is the one assumption its decision rested on.
- **Accept**: the API compiles as a library with its layout assertions passing against the PDB; a cooked menu
  asset loads and reports a correct header and symbol table; the renderer seam builds with every slot present
  (bodies may log and return). Nothing needs to display yet.
- **Report**: `agentBB.md`, including the asset-format answer.

### BC — GFx runtime, the ActionScript machine
Files: `source/Development/Src/External/GFx3/` (the virtual machine and the display list).
- [ ] 1. The AS2 virtual machine is 212 KB and 1,164 functions in the retail build; the player core is 433 KB
      and 1,951. Port in dependency order from the PDB: the value model and object interface first (our engine
      talks to the UI almost entirely by setting members and invoking methods, 81 % of 1,350 call sites), then
      the display list and the tag/character model, then the bytecode interpreter.
- [ ] 2. Drive it from the tests, not from the game: a harness that loads a cooked asset, instantiates its root
      movie clip, runs frame one and reports the objects created and the actions executed. Grow that until the
      main menu's own asset gets through its first frames.
- [ ] 3. Report honestly what fraction of the opcode set and the class library is implemented, and what the next
      wave must add. **This package is not expected to finish the machine.**
- **Accept**: the harness runs a cooked asset's first frame and reports created objects and executed actions,
  with a table of implemented versus remaining opcodes and library classes.
- **Report**: `agentBC.md`.

### BE — The GFxUI native layer: 208 natives and the movie players
Files: `GFxUI/Src/*` (the module's natives and movie-player classes), the DishonoredGame GFx movie-player
units, `DishonoredGameNativeStubs.ported.agentBE.txt`.
- [ ] 1. Port the 208 stubbed GFxUI natives against their retail bodies, roots first: `UGFxObject` (64),
      `UGFxMoviePlayer` (49), then the menu-facing ones — `UDisGFxMoviePlayerMainMenu` (18),
      `MenuBase` (7), `Base` (9).
- [ ] 2. The C++ data models behind the menu, which agent AW found are ours and not ActionScript: the load-game
      list, the settings tree and key binding (`FillLoadGameMenu`, `FillSettingsCategoryList`,
      `CreateGFxCategory`/`CreateGFxSetting`, `TryBindKey`), plus the external-interface callback path that
      turns a button into a script call.
- [ ] 3. Keep everything behind the existing module switch so a build without the GFx runtime still runs: the
      game must remain playable through `-newgame` and `-startmap` throughout this wave.
- **Accept**: the natives are ported and the module builds both with and without the runtime; the external
  interface path is exercised by a test that invokes a callback by name and sees the script side run; the
  regression harness stays green.
- **Report**: `agentBE.md`.

### BD — The real look: the Arkane post-process graph and the missing shader families
Files: `Engine/Src/ScenePostProcessing.cpp`, `PostProcessAA.cpp`, the ark post-process units
(`arkppnodematerial.cpp`, `arkbloompartsrendering.cpp`), `Engine/Src/LightShaftRendering.cpp`,
`Engine/Inc/SceneRenderTargets.h`, and the DishonoredGame post-process classes.
- [ ] 1. The 136 undeclared shader types are the Arkane and GFx post-process families. Declare and port them
      with their cooked records as the specification, the way agents AG and AH did: the `FArkPp` node graph,
      `DisFog`, Arkane bloom including its parts pass (2013 rva 0x5251a0), the depth-in-alpha filter family.
- [ ] 2. Wire the graph into the scene renderer where retail wires it, and confirm each pass runs by counting
      its draws, not by eye.
- [ ] 3. The colour-scale and cull-distance volumes agent AS enumerated feed this; check they reach it.
- **Accept**: undeclared shader types down from 136 to under 20 with the remainder named and explained; a
  before-and-after screenshot pair of the same camera in the first mission showing the post-process chain
  active; per-pass draw counts in the census; regression harness green.
- **Report**: `agentBD.md`.

### BF — The attributes system, the fifth root
Files: the DishonoredGame attributes units, `DishonoredGameNativeStubs.ported.agentBF.txt`.
- [ ] 1. Agent AU found this root and its trap: `IDisAttributesInterface::GetAttributeValue` gates fall damage,
      stat-pickup bonuses, item values and elixir caps, and the whole read and write core is about 1.9 KB of
      map work with its addresses tabulated in `agentAU.md`. **But** the attributes tweaks class has no
      reflected properties at all in the retail dump, so its refresh reads native members the generated class
      does not have: fill those from the 2012 PDB type first. Porting the reads without the writes divides by
      zero and kills the pawn on every landing.
- [ ] 2. Then the last two natives on the walking path, which depend on it: `execLanded_Native` and
      `execTakeFallingDamage_Native`.
- **Accept**: the harness metric for unported natives on the walking path reaches 0; the pawn survives a fall
  with a sensible damage value; regression green.
- **Report**: `agentBF.md`.

## Coordination and merge

- **Ownership**: `UnLevel.cpp`, `PrimitiveComponent.cpp`, `UnActorComponent.cpp` → BA. `External/GFx3` → BB
  and BC (BB owns the API headers and the renderer seam, BC owns the virtual machine; they share the directory
  and must not edit each other's files). `GFxUI/*` → BE. The post-process units → BD. The attributes units → BF.
- **Dependencies**: BE can port natives against BB's headers as soon as they exist, and works from a snapshot
  until BB merges. BC depends on BB's asset loading. BA, BD and BF are independent of all of it.
- **Merge order**: BA (the user's defect) → BF → BD → BB → BE → BC. After each: the regression harness, plus
  the merged package's accept command. One commit per agent. The DishonoredGame module is regenerated by the
  coordinator at the merge of any package that adds units.
- **End of wave**: tracker, `STATUS.md`, `PLAN.md`, `progress.md`, memory, then **continue into wave 7
  without pausing**, per the user's instruction. Wave 7's likely content: finishing the ActionScript machine
  and the menu, the AI brain root, the save system (milestone 6), and whatever BA leaves behind.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| C9 | coordinator | Fold status, promote the three tools, baseline, this plan | todo | | |
| BA | | The world in the right place | todo | | user-reported defect |
| BB | | GFx runtime foundation: API and renderer seam | todo | | multi-wave project, wave 1 |
| BC | | GFx runtime: the ActionScript machine | todo | | not expected to finish |
| BD | | The real look: Arkane post-process and the missing shader families | todo | | |
| BE | | GFxUI native layer: 208 natives and the movie players | todo | | |
| BF | | The attributes system root | todo | | unblocks the last 2 path natives |

## Rules for agents

Unchanged from `PHASE7.md`, and they matter more this wave because six packages run at once:

- Edit only your package's files; snapshot with `make_snapshot.py <X> <files>` when the shared tree breaks.
- Own build dir; `resources\build-release.cmd` with `BUILD_DIR` for fast runs; **never** edit `play.cmd` or
  `build-release.cmd`. Run through `build_and_smoke.py` with the `=` form for extra args and always
  `-forcelogflush`. `run_regression.py` must stay green.
- Own IDA copies, headless decompiles only; **never the IDA MCP tools and never the FModel MCP tools**.
- No junctions into the retail or reference trees; never delete anything under `Dishonored_Latest2026`.
- No commits, no `git add`. Every edit tagged `// DISHONORED(port|written|layout|retail|bringup): <2013 rva>`.
  Sources are CRLF, and `pathlib.read_text` silently converts them: use `crlf.py` or read bytes.
- **Measure first.** Every defect in waves 4 and 5 was found by instrumenting before changing: a watchpoint, a
  scene census, a reflected-offset dump, a texture census, an inventory dump. Four of them turned out to be a
  reference member kept as a storage-less shim — if something is inexplicably dead or shared, look there first.
- Audio is out of scope.
