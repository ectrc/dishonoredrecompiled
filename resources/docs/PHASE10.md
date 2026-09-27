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
- Audio, which stays where the user put it: `PLAN.md` Phase 10, polish.

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
| DE | DE | Make the content's own grade reach the renderer | doing | 2026-09-27 | DA's hand-over 1: ULocalPlayer::UpdatePostProcessSettings (0x2b08b0) writes nothing to m_CurrentArkPpSettings, so the level's, the volumes', the camera's and Kismet's grade never reaches the LUT. The eighth storage-less placeholder |
| DB | DB | Arkane's bloom parts | doing | 2026-09-27 | costed by CE |
| DC | DC | Flip the GFx runtime live and bring the menu up | doing | 2026-09-27 | the user's standing ask |
| CG | CG | The AI brain root | doing | 2026-09-27 | carried over from wave 7 |
