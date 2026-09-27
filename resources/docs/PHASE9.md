# Phase 3 wave 7 — a visible interface and the real look

Plan of record after `PHASE8.md`. Written 2026-09-27 by the coordinator. The user is away and has asked for
this wave to run **fully automated, with no prompts**, and for every choice to **assume the best case for a
1:1 recreation of retail**: reconstruct from the binaries and symbols rather than stub, port the real
function rather than approximate, and keep a documented stub only where the evidence genuinely is not there.
The coordinator merges, verifies and continues into wave 8 without pausing.

## Where this wave starts (HEAD `c196974`)

Wave 6 merged six packages; the regression harness passes 31 checks with 0 failures. The game renders its
world correctly, the player walks, touches, and picks things up, the Arkane fog draws, and the reconstructed
interface runtime runs the real main menu asset.

| | value |
|---|---|
| d3d9, 90 s | ~20,000 frames, **0 criticals**; pub interior and first mission both correct |
| shaders | 263 cooked, 258 loaded, **5 undeclared** (2 in a Bink build), 0 parameter mismatches |
| interface runtime | all 22 cooked movies parse; the menu asset runs 5 frames: 85 sprites, 170 display objects, 3,783 opcodes, **none unimplemented** |
| natives unported on the walking path | **0** (7 more are reached only by the touch probe: the `probe_natives` ratchet) |

**Nothing of the interface is visible yet**, and that is one cause with three consequences: the fonts are
glyph outlines rather than textures, the renderer's drawing half is unwritten, and a third of every movie's
tags are unhandled. Packages **CB**, **CC** and **CD** are that, together.

## Order and why

1. **CA first, alone if need be.** Agent BD measured that a pass can bind, draw, carry correct data and
   change **zero pixels**, because the resolve leaves the resolve destination bound and nothing re-binds the
   scene colour surface. It is tree-wide. Every post-process and interface pass this wave adds would
   otherwise appear to work and do nothing, and the cause would be hunted twice.
2. **CB, CC, CD** make the interface visible. None is sufficient alone.
3. **CE** is the look: the Arkane post-process graph, whose input chain agent BD filled from 0 to 42 nodes.
4. **CF** is the save system: milestone 6, and what the menu's Continue and Load entries need.
5. **CG** is the AI brain root: 275 stubs and the difference between an empty world and an inhabited one.

## Work packages

Standing rule for every package this wave, from the user: **assume the best case for 1:1**. Where a choice
exists between a faithful port and a shortcut, take the faithful port and say what it cost.

### CA — The scene-colour rebinding defect (first; small, measured, blocks the rest)
Files: `Engine/Src/SceneRenderTargets.cpp/.h`, `D3D9Drv/Src/D3D9RenderTarget.cpp`, `Engine/Src/SceneRendering.cpp`
(only the binding sites), `Engine/Src/FogRendering.cpp` (to retire agent BD's workaround).
- [ ] 1. Reproduce with agent BD's evidence: a clear to red before `BeginRenderingSceneColor` covers the
      screen, a clear after it is never seen; `FD3D9DynamicRHI::CopyToResolveTarget`
      (`D3D9RenderTarget.cpp:24`) leaves the resolve destination bound. Decompile retail's resolve and its
      scene-colour begin/finish and compare term for term.
- [ ] 2. Fix so a pass written in retail's shape (resolve, begin rendering scene colour, draw, finish)
      changes pixels. Agent BD left `-fogresolvetargets` to re-run retail's exact sequence in one run: that
      switch passing **is** the acceptance.
- [ ] 3. Retire BD's documented deviation in the fog pass once it holds.
- **Accept**: `-fogresolvetargets` produces the same image as the current workaround (compare with
  `build/agentBD/compare_pair.py`), the harness stays green, and the report states what retail does that we
  did not.
- **Report**: `agentCA.md`.

### CB — The text engine and glyph rasteriser (the large one)
Files: new units under `source/Development/Src/External/GFx3/` (text and font), coordinating with CC on the
renderer interface it draws through.
- [ ] 1. Port the text stack from the 2012 symbols in dependency order, as agents BB and BC did:
      `GFxTextField` (294 functions), `GFxStyledText` (134), `GFxTextDocView` (121), the font and glyph
      caches, and the rasteriser that turns `DefineFont3` outlines into glyphs. Agent BB proved the game's
      fonts are outlines with no font-texture tag anywhere, and named the two fonts and their glyph counts.
- [ ] 2. Note agent BC's finding: the runtime-internal types have **no layout in the symbols**, so offsets
      are not reproducible; build from signatures, vtable shape and decompiles, and pin enumerations case by
      case from the setters. Do not invent `offsetof` assertions for these.
- [ ] 3. Drive it from a harness like agent BC's, not from the game: rasterise a known string in both game
      fonts and dump the glyph bitmaps and metrics, then run the menu asset and report how many text fields
      resolve, lay out and produce glyphs.
- **Accept**: the harness rasterises both fonts and reports glyph coverage; running the menu asset produces
  laid-out text fields with real glyph output; a table of implemented versus remaining text functions.
- **Report**: `agentCB.md`.

### CC — The renderer's drawing half
Files: `GFxUI/Src/gfxuirenderer.cpp` and its header (agent BB's seam), plus the RHI resources it needs
(`FGFxUpdatableTexture`, `UGFxMappableTexture`), `D3D9Drv` only if a gap is proven.
- [ ] 1. Port the 221 drawing functions behind the 54-slot renderer interface agent BB declared and verified
      by call: vertex and index submission, the fill and blend modes, the transform and colour-transform
      handling (note agent BE's finding that this version's colour transform is transposed relative to the
      newer one), scissor and mask handling, and the texture path.
- [ ] 2. The shader types are already loading: agent BD declared the 50 interface shader types and they load
      byte-exactly from the cooked cache, so this is binding and submission, not shader work.
- [ ] 3. Prove it without the game first: draw a known geometry through the renderer into a render target and
      dump it, then draw one cooked movie's first frame.
- **Accept**: a dumped image from the renderer showing the expected geometry, then a cooked movie's first
  frame rendered to a target; per-pass draw counts in a census; harness green.
- **Report**: `agentCC.md`.

### CD — The remaining tag loaders and import binding
Files: `External/GFx3/GFxPlayerData.cpp` and the loader units (agent BC's area — BC is finished, so these are
free).
- [ ] 1. The 31 unported tag loaders, **shapes first**: every movie currently skips about 152 tags and ends
      with 94 placeholder characters, which is 22 of the script errors on its own.
- [ ] 2. Import binding: the import symbols are placeholders today, which accounts for the missing frame
      labels and much of the undefined-call count.
- [ ] 3. Agent BC also named a real recursion defect in the shop asset with its likely cause (an owner
      prototype field declared and unused) and the fact that the collector here is only the teardown half of
      retail's mark and sweep, so a long-running movie grows. Fix the first; state the second's cost.
- **Accept**: placeholder characters near zero on the menu asset, script errors down from 7 with the
  remainder named, the shop asset no longer recursing, and every cooked movie still running 5 frames.
- **Report**: `agentCD.md`.

### CE — The Arkane post-process graph
Files: the ark post-process units, `Engine/Src/ScenePostProcessing.cpp`, `SceneRendering.cpp` (the graph call
site), and the node classes that must move from the game module's shim header into the engine.
- [ ] 1. Agent BD mapped it: the node classes move to the engine as the fog classes did, the proxies are
      small (12 to 48 bytes), they are built in the view constructor (2013 rva 0x493e40) and rendered from
      the post-process pass. **The material node carries the colour treatment and its shaders already load.**
- [ ] 2. `RenderBloomParts` needs the view's bloom primitive set and the relevance that fills it; the shaders,
      targets and flag are already in place.
- [ ] 3. Depends on CA: without it the passes will draw and change nothing.
- **Accept**: a before-and-after pair from one build with one switch, quantified as agent BD did, showing the
  colour treatment active; per-node draw counts in the census; harness green.
- **Report**: `agentCE.md`.

### CF — The save system (milestone 6)
Files: the DishonoredGame save units, `UDishonoredEngine`'s save list, and the CppText declarations agent BE
named.
- [ ] 1. Agent BE named the exact blockers: the save-game struct is declared nowhere and the engine has no
      reflected save list; four declarations turn on the load list, Continue, the can-load query and the
      menu's has-save flag, with their addresses given.
- [ ] 2. Port save and load for real: retail's format, so a **retail save file loads in our build and ours
      loads in retail**. That is the 1:1 bar and it is the acceptance.
- [ ] 3. `PLAN.md` milestone 6 is "retail savegames load; save/load round-trip".
- **Accept**: a retail save file loads and places the player correctly; our own save round-trips; the menu's
  load list populates from real files; harness green.
- **Report**: `agentCF.md`.

### CG — The AI brain root
Files: the DishonoredGame AI units, `DishonoredGameNativeStubs.ported.agentCG.txt`.
- [ ] 1. Agent AJ's triage of all 295 stubs and agent AU's costing say the same thing: one root unlocks
      twenty to a hundred and fifty natives. Port `UDishonoredAIBrain`, `UDisAISubProcess` and
      `UDisAISubState` first, then the behaviours that hang off them.
- [ ] 2. Agent AU's rule, which this package inherits: do not port an exec whose subsystem does not exist —
      that trades an honest warning for silent nothing. Port the chain, not the wrapper.
- [ ] 3. The navigation mesh runtime is retail's poly-based path finding and is **not** in this package;
      name what the AI needs from it.
- **Accept**: at least 120 natives ported; an NPC exists, thinks and moves in the first mission or the hub;
  no new warn-once line on the walking path; harness green.
- **Report**: `agentCG.md`.

## Coordination and merge

- **Ownership**: `SceneRenderTargets`/`D3D9RenderTarget` → CA. The text units → CB. `gfxuirenderer.cpp` and
  the RHI resources → CC. The loader units → CD. The ark post-process units and `ScenePostProcessing` → CE.
  The save units → CF. The AI units → CG. `SceneRendering.cpp` is shared by CA (binding sites) and CE (the
  graph call site): CA merges first and CE rebases onto it.
- **Merge order**: CA → CE → CD → CC → CB → CF → CG. After each: `run_regression.py`, then the package's
  accept command. One commit per agent. **The coordinator regenerates any module whose units changed and
  commits every file the generator touches** — that rule cost this project two broken HEADs in wave 6.
- **End of wave**: tracker, `STATUS.md`, `PLAN.md` (milestone 6 if CF lands), `progress.md`, memory, then
  **straight into wave 8**.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| C10 | coordinator | Fold wave-6 rows, this plan, baseline | done | 2026-09-27 | plus the Cxform bridge from CC (42e9cbe): the machine and the renderer disagreed about all four colour channels |
| CA | CA | The scene-colour rebinding defect | done | 2026-09-27 | commit c403e2f: the binding was never broken. Three bring-up switches were permanently off - a file-scope static read of ParseParam runs before GCmdLine exists - and the wave-6 fog figure came from one of them. Real value 8.4 % of pixels, mean 3.44, corrected in 8e61755 |
| CB | CB | Text engine and glyph rasteriser | done | 2026-09-27 | commits 54b57d4 and 4bb6772: real glyphs come out of the game's own fonts, and the follow-up found that its two style helpers were a port of functions retail does not call on this path - deleted, then cross-checked glyph by glyph against CD's walk, 2,423 of 2,423 agreeing |
| CC | CC | The renderer's drawing half | done | 2026-09-27 | commit 2da00d8: the full 892-byte FGFxRenderer, 54 slots with their render-thread halves. 1280x720 first frame of UI_Global.Global with 40 of its 41 cooked bitmaps. Found the refcounts starting at 0 and SetUIViewport writing CurrentMatrix |
| CD | CD | Remaining tag loaders and import binding | done | 2026-09-27 | commit 683fa03: every tag loads, every import binds, and five silent defects in the interface loader |
| CE | CE | The Arkane post-process graph | done | 2026-09-27 | commit dec3fd4: 99.2 % of pixels changed against a 0.09 % noise floor. Twelve node classes moved into Engine. The colour treatment turns out to be the DOF node's LUT uber pass (0x522750 / 0x522990), not a material node - that pass is the next package |
| CF | CF | The save system | done | 2026-09-27 | commit d40e26c: all 51 real retail saves load and round-trip. BUILT_FROM_CHANGELIST was 334700, so every retail save was rejected by construction; it is 1274963 now |
| CG | | The AI brain root | todo | | |

## Wave result (coordinator, 2026-09-27)

Seven packages merged: CA `c403e2f`, CB `54b57d4` and `4bb6772`, CF `d40e26c`, CD `683fa03`, CC `2da00d8`,
CE `dec3fd4`, plus `8e61755` (a wave-6 measurement correction) and `42e9cbe` (the Cxform bridge).
**Verified on a clean checkout of HEAD, not on the working tree: 642 of 642 targets build and link, and the
regression harness gives 31 ok, 0 failed** (22 in the play stages, 9 in the verification stages) - 20,130
d3d9 frames, 1,196 draws per frame, 6,506 draw elements, 457 visible primitives, the pawn walking 1,024.6 at
peak speed 500.5, 895 PhysX actors, 2,314 layout types with 0 mismatching, CoreSmoke 99 of 99, and 0 critical
errors in all three play stages.

What this wave delivered:

| | |
|---|---|
| the interface runtime | **complete and verified, and it draws the game's own art.** CC renders a 1280x720 first frame of `UI_Global.Global` with 40 of its 41 cooked bitmaps; CB rasterises 2,481 glyphs from the game's own fonts; CD makes every tag load and every import bind; all 22 cooked movies parse with 0 placeholders. It is deliberately not wired in: `DISHONORED_GFXUI_GFX3_RUNTIME` only flips in the commit that lets GFxUI call the runtime, which is wave 8 |
| the real look | CE's post-process graph changes **99.2 % of pixels** against a 0.09 % noise floor - and found that the game's colour treatment is the depth-of-field node's LUT uber pass, not a material node, which redirects the next package |
| milestone 6 | all 51 real retail saves load and round-trip. One number did it: `BUILT_FROM_CHANGELIST` was 334700 against retail's 1274963 |

**Three findings that reach past their own packages.**

1. **A file-scope `static UBOOL G... = ParseParam(appCmdLine(), ...)` in a static library is always FALSE**,
   because static initialisers run before `WinMain` sets `GCmdLine`. Three bring-up switches had been dead
   since they were written, and one of them had corrupted a published measurement: the wave-6 fog figure was
   **8.4 % of pixels, not 37 %**. Agent CA found it by checking the switch rather than the renderer.
2. **A generated header inherits its producer's spelling.** DIA reports array extents innermost-first, so
   `GRenderer::Cxform` reached the tree transposed, and the ActionScript machine and the renderer disagreed
   about all four colour channels inside the same 32 bytes - a movie's `_alpha` landed in green's add term.
   Size and offset assertions cannot see this; only a second source for the declaration can.
3. **Decompile the caller to find out which retail function you are actually porting.** CB's follow-up was
   sent to fix three defects CD had reported in its code. Two were real, one was not - and both packages had
   been auditing against `GFxFillStyle::Read` when the function in play was `GFx_ReadFillStyles` (`0xa429c0`).
   The helpers were not a port with errors in them; they were an invention of a wire format retail does not
   read on that path. Deleting them was then **checked rather than argued**: both record walks run over the
   identical byte range of every glyph in the cook, 2,423 compared, 2,423 agreeing.

**One process failure of mine, and it is the wave-6 failure wearing new clothes.** Merging CE, I staged
`EngineArkaneClasses.h` because it sat in the same module - but every line of its diff belonged to agent CG,
which was still running, and the two headers it began including are still stubs at HEAD. HEAD did not
compile. In wave 6 I committed a *subset* of a generator's output; here I committed a *superset* of a
package's files. The first merge-gate run did not catch it, because it measured the working tree and so
blamed CG's own unfinished files. Backed out in `aedbdaa`. **The rule that catches both shapes: verify on a
clean checkout of the commit, never on the working tree.** That is how this wave's result was measured.

## Rules for agents

As `PHASE8.md`, and one addition from the user: **assume the best case for a 1:1 recreation**. Prefer the
faithful port; where you must stub, say so at the site and in the report.

- Edit only your package's files; snapshot with `make_snapshot.py <X> <files>` when the shared tree breaks.
- Own build dir; `resources\build-release.cmd` with `BUILD_DIR` for fast runs; **never** edit `play.cmd` or
  `build-release.cmd`. `run_regression.py` derives its run names from `--build-dir`, so concurrent runs no
  longer collide; it must stay green.
- Own IDA copies, headless decompiles only; **never the IDA MCP tools and never the FModel MCP tools**.
- No junctions into the retail or reference trees; never delete anything under `Dishonored_Latest2026`.
- No commits, no `git add`. Every edit tagged `// DISHONORED(port|written|layout|retail|bringup): <2013 rva>`.
  Sources are CRLF; `resources/tools/ida/crlf.py` repairs the silent conversion `read_text` performs.
- **Measure first.** Every defect of waves 4 to 6 was found by instrumenting before changing. Six were a
  reference member or method kept as a storage-less placeholder or simply absent: the draw-distance scale,
  the animation arrays, the merged-prop transform, the post-process chain, the attribute refresh, the
  collision mask. If something is inexplicably dead, empty or shared, look there first.
- Audio is out of scope (`PLAN.md` Phase 10).
