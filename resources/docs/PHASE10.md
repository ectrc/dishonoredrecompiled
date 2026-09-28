# Phase 3 wave 8 — a menu you can see, and the look the game actually ships

Wave 7 finished the reconstruction of Scaleform GFx 3.3 (`PHASE9.md`): the API and engine seam (BB), the
ActionScript machine (BC), the text engine and glyph rasteriser (CB), the renderer's drawing half (CC) and
the tag loaders (CD). Every one is verified in isolation, against the game's own cooked assets. **None of it
was wired into the game.** Wave 7 also landed the Arkane post-process graph (CE), the save system (CF) and
proved three bring-up switches had been permanently off (CA).

This wave connects the interface to the game and ports the two rendering passes that the graph turned out to
depend on.

## Where this wave starts

HEAD `4bb6772`. Wave 7 commits: `c403e2f` CA, `8e61755` (fog measurement correction), `54b57d4` CB,
`d40e26c` CF, `683fa03` CD, `2da00d8` CC, `dec3fd4` CE, `42e9cbe` (the Cxform bridge), `4bb6772` CB's
follow-up. Agent CG (the AI brain root) is still live from wave 7 and merges into this wave.

## Order and why

The three packages are independent by construction, so they run at once:

1. **DC — flip the runtime and bring the menu up.** The user's standing ask ("the full fledged menu and
   graphics") and the one package whose output they can see directly. `DISHONORED_GFXUI_GFX3_RUNTIME=1` must
   flip in the same commit that lets GFxUI call BC's runtime, or 28 `LNK2005` follow (agent CC).
2. **DA — the colour treatment.** Agent CE proved the game's colour balance, exposure and gamma are the
   depth-of-field node's uber pass — `LutCreation` (2013 rva `0x522750`) bakes a LUT, `Blend` (`0x522990`)
   applies it — and *not* a material node, which is what the wave-7 brief had assumed. The LUT target is a
   plain 256x16 A8R8G8B8 2D render target, so no volume-texture RHI work is needed.
3. **DB — Arkane's bloom parts.** Costed end to end by CE: the relevance bit exists in the PDB
   (`bBloomPartRelevance`, bit 15) and `UMaterialInterface::bHasBloomPart` is already computed in this tree;
   what remains is the relevance plumbing, `FViewInfo::BloomPartPrimSet[4]`, `RenderBloomParts` (`0x5251a0`)
   with its mesh drawing policy, and `bloom::GaussianBlur`.

## Work packages

| ID | Task | Owns |
|---|---|---|
| DA | The depth-of-field node's LUT colour pass | `Engine/Src/arkppnodedof.cpp`, `Engine/Inc/arkpp.h`, the DOF node's own lines in `arkppnodes.cpp` |
| DB | Arkane's bloom parts | `Engine/Src/SceneRendering.{cpp,h}`, `Engine/Inc/Scene.h`, `ScenePostProcessing.cpp`, the bloom policy and shaders |
| DC | Flip the GFx runtime live and bring the menu up | the `GFxUI` module, `cmake/GFx.cmake`, `cmake/GFxUI.cmake`, the `DishonoredGame` movie players, the HUD post-render path, `FogRendering.cpp` (anchor removal only) |
| CG | The AI brain root (carried over from wave 7) | the `DishonoredGame` AI units, `arkcomponent*`, `arkgameeventdispatcher*`, `gen_classes_header.py` |

## Held for the wave after this one

- **The five `UObject` save virtuals** — `GameSave`, `GameLoad`, `IsSaveable`, `IsRefSaveable`,
  `PostGameLoad`. Agent CF loaded and round-tripped all 51 real retail saves but cannot read *inside* the
  level blobs without these. Retail has **105 `GameSave` and 108 `GameLoad` overrides** plus 11
  `IsSaveable`, and it cannot be done partly: `FLevelSaver::operator<<(UObject*&)` writes a `WORD` index and
  then the object's own `GameSave` **inline, with no length prefix**, so one missing override desynchronises
  every object after it. This is what restores the player's transform on load. It is a wave of its own and it
  touches every module, so it waits until CG has cleared `DishonoredGame`.
- The AS2 garbage collector (~147 functions).
- The remaining DOF blur passes and the LensCompose node, depending on what DA hands over.
- **The DishonoredGame regeneration is one command, not two halves.**
  `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`.
  Merging CG I ran it without `--sources-cmake`, so the 14 units that come off the skeleton-exclude set were
  compiled out and HEAD linked with **113 unresolved externals**. Wave 6 cost two broken HEADs by committing a
  *subset of the generator's output*; this was a subset of the *generator*. Same rule, and the same check
  catches both: build a clean checkout of the commit.

- Audio, which stays where the user put it: `PLAN.md` Phase 10, polish.

## Wave result (coordinator, 2026-09-27)

Five packages merged, each gated by building a clean checkout of its own commit and running the harness
there: CG `f13ad82`, DA `fdc6aec`, DB `b66b8bd`, DE `9b77df6`, DC `00e66eb`, plus `a42f4a3` (a measurement
correction). **31 checks, 0 failures at HEAD.** Agent DF is still running and merges into wave 9.

**The menu renders in the game.** That was the user's standing ask, and `build/agentDC/mainmenu.png` is the
game's own gamma-calibration screen in Dishonored's own typeface, beside the chapter panel's cooked
artwork - glyphs rasterised from the game's `DefineFont3` outlines, shapes tessellated from cooked
`DefineShape` records, 182 display objects placed by the reconstructed ActionScript machine running 3,854
opcodes with none unimplemented, drawn through the reconstructed renderer from `UGameViewportClient::Draw`.

**An NPC exists and thinks**: 0 NPC pawns became 26, each with a controller, an initialised brain and a
running sub-state machine, because the first mission map turns out to hold 41 spawners and no placed pawns.

**The look is fed for the first time.** The colour treatment is the depth-of-field node's LUT pass, not a
material node; the level's own grade reaches it now and moves 98.68 % of the frame; and the bloom parts
draw.

### What this wave taught, beyond its packages

1. **Measure the map you mean.** Agent DA reported the content's grade as neutral and it was - of
   `DishonoredGameFull_P`, the startup map, which one-shot probes latch onto. DE passed the map as the
   command line's first token and found a real grade in the mission. Two careful agents, one blind spot.
2. **`-apshot` captured a frame nobody chose.** The request is raised on the render thread and consumed on
   the game thread, so the captured frame depends on frame rate and a costly pass moves it by itself - worth
   73 mean over 71 % of pixels of pure camera ghosting. It had corrupted the wave-6 fog figure and the
   wave-7 graph figure. `-apshottime` replaces it and its floor is byte-identical runs.
3. **The placeholder pattern reached nine, and the ninth was the widest.**
   `GSystemSettings.MaxFilterBlurSampleCount` was a storage-less shim reading 0, so *every Gaussian blur in
   the tree* aborted the render thread - DA's downsample, DB's bloom blur and the reference chains alike.
   Nothing had ever reached it until content did.
4. **Two agents, one file, one commit.** `UnPlayer.cpp` was shared by DC and DE, and neither snapshot built
   with the other's half. Merged by committing each package's block separately and building the union.

### Coordinator errors this wave, both caught by the clean-checkout gate

- Merging CE I staged `EngineArkaneClasses.h`, whose whole diff belonged to still-running CG; HEAD did not
  compile (`aedbdaa`).
- Merging CG I ran the generator without `--sources-cmake`, so 14 units were compiled out and HEAD linked
  with 113 unresolved externals. Wave 6 was a subset of the generator's *output*; this was a subset of the
  *generator*.
- I also used `build/head_wt` as the gate worktree while agent DF had adopted it, checking commits out
  underneath it. `build/gate_wt` is the coordinator's, and agents create their own.

## Rules for agents

As `PHASE9.md`. The additions this wave:

- **Decompile the caller to find out which retail function you are actually porting.** Agent CB's follow-up
  deleted two helpers that were a faithful-looking port of the wrong functions: both packages had been
  comparing against `GFxFillStyle::Read` when the code in play was `GFx_ReadFillStyles` (`0xa429c0`). A
  cross-package audit produced two real bugs and one false one, and only the caller settled all three.
- **A cross-check beats an assertion.** CB ran both record walks over the identical byte range of every
  glyph in the cook — 2,423 glyphs, 2,423 agreeing — rather than asserting the two agreed.
- A file-scope `static UBOOL G... = ParseParam(appCmdLine(), ...)` in a static library is **always FALSE**;
  static initialisers run before `WinMain` sets `GCmdLine`. This silently disabled three bring-up switches
  and corrupted one published measurement (agent CA, `c403e2f`). Read switches on first use.
- Generated headers inherit their producer's spelling: DIA reports array extents innermost-first, so
  `GRenderer::Cxform` reached the tree transposed and the machine and the renderer disagreed about all four
  colour channels in the same 32 bytes (`42e9cbe`). When two packages read the same bytes differently, check
  the declaration against a second source before trusting either.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| D0 | coordinator | This plan, the wave-7 fold, the baseline | doing | 2026-09-27 | |
| DA | DA | The DOF node's LUT colour pass | done | 2026-09-27 | commit fdc6aec: the whole node, not just the LUT. The baked ramp is a clean identity cube because this content's grade is neutral - and that is the evidence, since the ported pass and the gamma copy it replaces land on the same image from two different code paths. Blacking the bake out turns the frame black (99.51 %), a real grade moves every pixel |
| DE | DE | Make the content's own grade reach the renderer | done | 2026-09-27 | commit 9b77df6: L_Tower_P's grade is not neutral after all - a cool green-blue lift, +0.2 EV, three times the film grain - and it moves 98.68 % of pixels, mean 29.49, against a byte-identical floor. DA had been measuring the startup map. Found the ninth placeholder too, and it was the widest yet: MaxFilterBlurSampleCount read 0, so **every Gaussian blur in the tree** aborted the render thread |
| DB | DB | Arkane's bloom parts | done | 2026-09-27 | commit b66b8bd: the pub's attic windows and floor lantern bloom and nothing else, 3.7 % of pixels against a floor of exactly zero. Also found that -apshot captures a frame nobody chose, which had corrupted two published measurements |
| DC | DC | Flip the GFx runtime live and bring the menu up | done | 2026-09-27 | commit 00e66eb: **the game's own menu renders in the game** - 182 display objects, 68 draws, 157 glyphs, 3,854 opcodes with 0 unimplemented, through the real renderer from UGameViewportClient::Draw. build/agentDC/mainmenu.png |
| DG | DG | Make the menu operable and complete | done | 2026-09-28 | commit e24a4ca: **the menu answers input** - a key dismisses the start screen and opens the main menu. Three of DC's four hand-overs named the wrong cause and measuring said so each time: the menu is driven by the AS2 Key broadcaster, not the button model (one DefineButton2 in 477 tags); the text is InitTexts out of the game's own .INT, not GFxTranslator (149 strings); the 7 script errors were three other things. Found the loop that ate the background - ActionGetMember left a getter/setter pair on the stack, so `while (this["splatter"+n+"_mc"] != undefined)` never terminated and reached 896 MB. Four lines: 1,000,000 opcodes -> 5,573, 15,322 script errors -> 40. Also: Object.registerClass never instantiated a timeline-placed clip, so all ten menu screens were live at once - which is what DC's screenshot was. Still reads "Text" and has no painting; that is now a five-name AS2 library list (tweenTo/tweenEnd, SaveProperties, PlaySound, one anonymous call) |
| CG | CG | The AI brain root | done | 2026-09-27 | commit f13ad82: 0 NPC pawns -> 26, each with a controller, an initialized brain, a behaviour and a running sub-state machine. The map has 41 spawners and no placed NPCs, which nobody had measured - that is why the AI root looked bottomless. Gate on a clean checkout: 31 ok, 0 failed, and the AI shows in the numbers (PhysX actors 895 -> 1,369, sequence ops 35,812 -> 87,380, probe_natives 7 -> 6) |
| DF | DF | The 17 AI sub-state classes and their _Param structs | done | 2026-09-28 | commit 0e97595: the idle loop runs end to end - sub-state transitions 0 -> 26, one open locomotion desire each, stable over 100,412 brain ticks. UDisBehaviorIdle::OnBehaviorResume had been hand-written with an argument the base does not take, so it was an overload, not an override, and the whole transition path was dead code in a green build - MSVC's C4263/C4264 had said so and were lost in the noise. Also: a vtable slot read against the wrong base had landed on an ICF-folded `ret`, which reads as proof a function does nothing; the real one rings the alarm bell |
| DH | DH | The menu crash | done | 2026-09-28 | commit bf6e5e6: **the menu stays up** - 400 s of drawing across four runs where the same command line died at 8.06 s. Two faults, one per thread, and both earlier packages had blamed one and diagnosed it wrongly. Game thread: the display list's revive branch released the caller's only reference and the caller used it for twenty more lines. Render thread, and far wider than this package: **GAtomicInt was not atomic** - operator++/-- were a plain read-modify-write on a volatile long, so the glyph atlas texture, AddRef'd on the game thread and Released on the render thread, could be deleted under its owner. Now interlocked. Fourth defect in the refcount family here, and the first to be about the update rather than the initial value |
| DI | DI | The NPCs have no heads | done | 2026-09-28 | commit 9dc346b: head mesh set 0 -> 26, attached 0 -> 26, and heads drawn per second equal to bodies drawn. **A whole virtual pass was absent**: nothing overrode ADishonoredPawn::PostBeginPlay, so APawn's ran and all six per-aspect passes were skipped - a feature fully authored in the content (every NPC's tweaks name a head mesh) doing nothing. The tenth instance of the placeholder/absent pattern, one level up from the nine before it |
| DJ | DJ | The menu matches the user's reference screenshots | done | 2026-09-28 | commit 24f3fe0: both screens render and the key press moves between them. Thirteen defects, three fatal alone - **SetMatrix and SetCxform were game-thread stores** where retail copies them into a render command, so every draw of a frame used the last matrix of that frame, and *that* was the "weird texture issues" the three previous packages had blamed on the tessellator, the atlas and the shaders; the builtin at string-context +336 is `__resolve`, not `__constructor__`, so every read of an absent member ran the object's constructor; and `new` never asked the constructor to make the object, so `new Array()` had no length and the menu bar had no entries |
| DK | DK | The camera, the DLC button, and making the menu operable | done | 2026-09-28 | commit d3fe89d: screen two is the reference's city view, reached by the key press, and the highlight moves CONTINUE -> NEW GAME -> MISSIONS with Enter activating. Three stacked defects held the camera: `UGFxEvent_FSCommand::RegisterEvent` absent (38 nodes matched, 0 routed); every `GFXUI_*` FName was `None` because name generation sat inside `#if WITH_GFx` while the registrants beside it did not; and **the whole matinee runtime read shims** - 12 matinees playing over 0 groups of 0 length, which was never menu-only, `L_Tower_P`'s matinee was equally dead. Also: the menu had looked uninteractable because the *harness* pressed and released a key in one tick, which `Key.isDown` cannot see |
| DL | DL | The text drop shadow | done | 2026-09-28 | commit 416fd71: shadows render, signal +7.648 signed mean against a -0.299 floor. **Retail does not put a text field's shadow through the render-to-target filter pass** - that pass has only sprite and button callers; a text character folds its filters into a `GFxTextFilter` and the text engine rasterises each glyph a second time, blurred, into its own atlas slot. Reading the caller first turned a wave-sized package into eleven files. The asset's three filters are distance 0 (a halo, not an offset) and one of the three is near-**white**, so "make the text darker" would have been wrong. Twelfth instance of the standing pattern: `GFxGlyphParam` already carried the blur fields and `GetGlyph` already compared them, but the rasteriser never looked at them, so two glyphs differing only in blur got identical unblurred pixels |
