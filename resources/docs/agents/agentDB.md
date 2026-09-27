# Agent DB report — Arkane's bloom parts (2026-09-27)

Package **DB** of wave 7/8: `FSceneRenderer::RenderBloomParts` (2013 rva 0x5251a0) and everything that feeds it.
Written against **HEAD `668a4ec`** ("Wave 7 result: verified on a clean checkout, 31 checks green"). Build dir
`build\agentDB` (Release, x86, Ninja, every module option), built from the snapshot worktree `build\agentDB_wt`
(= HEAD + my ten files, `resources\tools\make_snapshot.py DB --list build\agentDB\files.txt`, build command
`build\agentDB_release_build.cmd`). IDA copies `resources\docs\idb\shipping2012_agentDB.i64` /
`retail2013_agentDB.i64`, headless `decompile_funcs.py` only; the decompiles are in `build\agentDB\decomp12`,
`decomp12b`, `decomp12c`, `decomp13`. No commits, nothing staged. Status rows: `agentDB_status.csv` (28).

## Result

| Accept | State |
|---|---|
| the relevance plumbing | **done**: `FMaterialViewRelevance::bBloomPart` (retail bit 4) from `UMaterialInterface::bHasBloomPart`, `FPrimitiveViewRelevance::bBloomPartRelevance` (retail bit 15) through `SetPrimitiveViewRelevance`, and `ProcessVisible` filling `FViewInfo::BloomPartPrimSet[4]` from it - outside the translucency branch the distortion set sits in, which is the point: a bloom part is usually an opaque mesh element |
| `FViewInfo::BloomPartPrimSet[4]` | **done**: `@1856` in retail, immediately after `DistortionPrimSet @1808`, 12 bytes each (one `TArray<FPrimitiveSceneInfo*>`), plus `bHasBloomPartViewMeshElements` (the second half of the pass's gate) |
| `RenderBloomParts` with its mesh drawing policy | **done**: the whole 694-line function - the fog-mask scratch pass, the threshold/tint downsample, `bloom::GaussianBlur` and the compose - with `TBloomPartMeshDrawingPolicy` / `TBloomPartMeshDrawingPolicyFactory` and `bloom::FArkGaussianVertexDeclaration` |
| `bloom::GaussianBlur` | **done** (0x51fe50): two five-tap passes ping-ponging the two quarter-size targets, direction (1,0,0,0) then (0,1,0,0) |
| census in a real level | **done**: `L_Pub_Day_P` **7 bloom-part primitives, sets 0/7/0/0, 16 draws** on 45 of the run's 51 census lines (8 and 17 on four more); `L_Tower_P` **5 relevant, sets 0/5/0/0, 5 prims, 8 draws**, the same on all 656 census lines of a 19,680-frame run. 235 primitives carry the bit on the first frame after the level comes up, 7 survive culling |
| a pair from one build differing by one switch, quantified | **done**: `build\agentDB\day_a.bmp` (`-nobloomparts`) vs `day_after.bmp`, `L_Pub_Day_P` at world time 6.0 - **mean 3.95 of 765, 3.7 % of pixels changed by more than 2 per channel, largest 366**, against a noise floor of **0.00 mean, 0.0 %, largest 0** (two runs of the same switch are bit-identical). The change is the three windows and the floor lantern blooming and nothing else (`figure_pair.png`, `figure_window.png`) |
| the compose half measured too | **done**: `-bloomcompose` makes the pass compose the bloom itself where the fog pass would have (this map has fog): **mean 3.98, 3.8 %** against the same floor, and the two routes agree with each other to **0.31 mean / 1.8 %** |
| regression harness green | **31 ok, 0 failed, 0 skipped** (`build\agentDB\regression1.txt`, 424 s, `--build-dir build/agentDB`) |
| run | `Initial startup: 3.4 s`, 20,130 d3d9 frames in the regression's 90 s run, 0 criticals, no new "no cooked shader" line |

```
rem the pair (one build, the only difference is -nobloomparts)
set MAP=L_Pub_Day_P
set APSHOTTIME=6
python build\agentDB\run_pair.py day_a -nobloomparts
python build\agentDB\run_pair.py day_b -nobloomparts
python build\agentDB\run_pair.py day_after
python build\agentBD\compare_pair.py build\agentDB\day_a.bmp build\agentDB\day_b.bmp      rem the floor
python build\agentBD\compare_pair.py build\agentDB\day_a.bmp build\agentDB\day_after.bmp  rem the pair
rem regression
python resources\tools\run_regression.py --build-dir build/agentDB --no-build
```

## 1. What the pass does

Dishonored's bloom is not UE3's. A material has a `BloomColor` input; the primitives whose material uses it are
collected per DPG into `FViewInfo::BloomPartPrimSet[4]` and drawn in a pass of their own, at `SDPG_World` only,
between the soft-masked base pass and the fog pass (`FSceneRenderer::RenderDPGEnd`, 2013 rva 0x464290). In order
(2013 rva 0x5251a0, 2012 0x566120):

1. **gate** - for each view: the view has an `FArkPpConfig` and its `m_PpBloomParameters.m_bEnable` is set, and
   either the set for this DPG is non-empty or `bHasBloomPartViewMeshElements` has this DPG's bit. `m_BloomNeedBlit`
   is cleared on entry and set here.
2. **draw** - resolve scene colour, bind the **full-size fog-mask target with the scene depth buffer**
   (`BeginRenderingFogMask`, 0x448f00), set the view's viewport and view parameters, clear to
   `FLinearColor(128,128,0,0)`, set `TStaticDepthState<FALSE,CF_LessEqual>` and call
   `BloomPartPrimSet[SDPG_World].DrawBloomPrims` (0x524f00), which draws the view's own bloom-part mesh elements and
   then, per primitive of the set, its dynamic elements through a `TDynamicPrimitiveDrawer` and its visible static
   meshes through the factory. The material's own `ArkBloomPartVertexShader` / `ArkBloomPartPixelShader` pair draws
   the geometry, depth-tested against the scene so a bloom part behind geometry does not bloom, colour mask `CW_RGBA`.
3. **reduce and blur** - `BeginRenderingBloom`, viewport and scissor at a quarter of the buffer, one full-target
   triangle with the source texel size in `zw`, `FBloomDownSamplePixelShader` scaling by
   `m_PpBloomParameters.m_Tint.rgb * m_Scale` with `-m_Threshold * m_Scale` in alpha, then `bloom::GaussianBlur`
   (0x51fe50): two five-tap passes between `m_BloomRT` and `m_BloomRT2`, each resolving the rect it drew.
4. **compose** - unless the fog pass will do it. `if (!bDirty || (ShowFlags & SHOW_Fog) && (Scene->DisFogs.Num() ||
   Scene->m_Rains.Num())) return FALSE;` leaves `m_BloomNeedBlit` set, which is exactly what `FogRendering.cpp` reads
   to bind `GetBloomPartsTexture()` as its `BloomParts` parameter instead of `GBlackTexture` - and what makes it skip
   its own `ResolveSceneColor`, because this pass already did one. Otherwise the pass composes it itself:
   `BeginRenderingSceneColor`, `FBloomComposePixelShader` with the scene textures and the blurred target, colour mask
   `CW_RGB` (the compose keeps scene colour's alpha; the downsample does not).

Both routes were measured (section 3). On the two maps used here the fog route is the live one.

## 2. Evidence for the parts that are not obvious

* **the relevance bits are retail's, not the reference's.** `types.json` gives the 2012 `FPrimitiveViewRelevance`
  (4 bytes, a union with `mRaw`) bit for bit: `bDistortionRelevance` 14, **`bBloomPartRelevance` 15**,
  `iSoulRenderingRelevance` 23, and no `bStaticButFadingRelevance`, `bInheritDominantShadowsRelevance`,
  `bSceneTextureRenderBehindTranslucency`, `bTranslucencyDoFRelevance`, `bSeparateTranslucencyRelevance` or
  `bInitializedThisFrame` at all. `FMaterialViewRelevance` has **`bBloomPart` at bit 4**, between `bDistortion` and
  `bOneLayerDistortionRelevance`. `FMaterialViewRelevance::SetPrimitiveViewRelevance` (2013 rva 0x26ffa0) is eleven
  bit copies and one of them is `<<11 & 0x8000`, i.e. material bit 4 -> primitive bit 15: that is the whole route
  from the material to the pass. `UMaterialInterface::GetViewRelevance` (0x12d8b0) sets it from `@84` bit 0
  (`bHasBloomPart`) with no regard for the blend mode - an opaque material blooms.
  Ours keeps the reference members the tree's other passes use, so `bBloomPartRelevance` cannot land on bit 15 as
  well; it is placed where retail has it, immediately after `bDistortionRelevance`, and nothing in the tree reads the
  word as a word.
* **the material flag is a virtual, slot 12.** `vtables.csv` for `FMaterialResource`: 11 `IsDistorted`,
  **12 `HasBloomPartForCompilation`**, 13 `IsSpecialEngineMaterial`. Its body (2013 rva 0x128320) is
  `Material->BloomColor.Expression != NULL`, `UMaterial::BloomColor` being the `FColorMaterialInput` at @364. That is
  what the factory tests per mesh element, and it is also what decided whether the two shaders were cooked.
* **the pass draws into the fog mask.** `BeginRenderingFogMask` (2012 0x46c460) binds `m_FogMaskRT.Surface` with
  `RenderTargets[6].Surface`, the scene depth buffer - so the bloom-part draw is depth-tested against the scene, and
  the mask is a full-size scratch surface the pass reduces into the bloom target. `RenderFogMaskStencil` (0x433f80),
  which is not ported, is that target's other user.
* **`bHasBloomPartViewMeshElements` is four bits, not one.** The 2012 PDB lists it at `@3569 bit 0`, i.e. bits 8..11
  of the word at 3568 that also holds `bHasTranslucentViewMeshElements` (0..3) and `bHasDistortionViewMeshElements`
  (4..7) - all three are `: SDPG_MAX_SceneRender`. The gate reads `(word >> 8) & 2`, which is that field indexed by
  `DPGIndex == SDPG_World`, not bit 9 of anything.
* **the blur shader's parameter is a direction, not a table of offsets.** `TBloomBlurVertexShader<5>` has one
  `FShaderParameter` at offset 108 and `bloom::GaussianBlur` writes one 16-byte value into it per pass: (1,0,0,0)
  then (0,1,0,0). The five taps come from the source texel size the vertex carries in `zw` and the weights are baked
  into `MainBlur`. Agent BD's declaration set `NumSamples` `FVector2D`s into it, which is the reference
  `TFilterVertexShader` shape; the cooked record's parameter is 16 bytes. Section 4, defect 3.
* **`iKernelSpread` and the trailing float are dead.** The exported symbol takes nine parameters; the only call site
  passes 5 and 1.0f (2013 rva 0x5259a6) and the retail body reads neither.

## 3. The measurement

`L_Tower_P` is the map every earlier package measured on, and it cannot carry this measurement: it opens on a boat
under way, and two runs of it do not hold the same world state. The pair is therefore measured on **`L_Pub_Day_P`**,
whose player stands still in the attic of the pub, and where two runs are bit-identical.

| | mean of 765 | pixels > 2/channel | largest |
|---|---:|---:|---:|
| noise floor (`-nobloomparts` twice) | **0.00** | **0.0 %** | **0** |
| the pair (`-nobloomparts` vs the pass) | **3.95** | **3.7 %** | **366** |
| the pass composing it itself (`-bloomcompose`) | 3.98 | 3.8 % | 363 |
| the two compose routes against each other | 0.31 | 1.8 % | 42 |

What changed is the three attic windows and the lantern on the floor, and nothing else: `figure_window.png` is the
before, the after and the difference of the region the difference's bounding box covers (428,230)..(1280,452). The
quarter-resolution steps are visible in the halo, which is the size the blur runs at.

**Getting two frames that can be compared at all took four attempts, and the finding is worth more than the pair**
(section 4, defect 1): `-apshot=<frame>`, the switch agents BD and CE measured with, sets `GScreenShotRequest` from
the **render thread**, while the flag is consumed at the end of `UGameViewportClient::Draw` on the game thread. The
game thread runs ahead by however many frames the queue holds, and that lead depends on how fast the frames are - so
a pass that costs GPU time moves the captured frame by itself, and the pair then compares two different moments of
the world. On `L_Tower_P` that showed up as 73 mean / 71 % of pixels of pure ghosting, more than twenty times the
real effect. `-apshottime=<seconds>` is the same request keyed to `ViewFamily.CurrentWorldTime` and tested in
`BeginRenderingViewFamily`, on the game thread, before the frame is submitted; with it the same-switch noise floor on
`L_Pub_Day_P` is zero.

## 4. Defects found

1. **`-apshot` captures a frame nobody chose** (`SceneRendering.cpp`, the census block; the consumer is
   `UnPlayer.cpp:1858`). Above. The switch is left as it was - agents BD and CE's numbers were taken with it and
   their effects were large enough to survive it - and `-apshottime` is the one to use for a quantified pair.
2. **The fog-mask render target was `PF_G8`** (`SceneRenderTargets.cpp`, agent BD's allocation, no evidence cited).
   Retail's `FSceneRenderTargets::InitDynamicRHI` (2013 rva 0x451080) creates "FogMask" at the full buffer size with
   `EPixelFormat` **2 = `PF_A8R8G8B8`**, and the two quarter-size "BloomBuffer" targets with **10 = `PF_FloatRGBA`**
   (agent BD had `PF_FloatRGB`, which maps to the same D3D format here). A single-channel target cannot hold the
   bloom parts' colour, and `D3DFMT_L8` is not a render-target format this RHI is given at all: the pass would have
   drawn into a surface that is not there. Corrected with the addresses at the site.
3. **`TBloomBlurVertexShader<5>::SetParameters` wrote the wrong shape** into the blur's one parameter (section 2).
   With the reference shape the blur direction is whatever the first two floats of a `FVector2D[5]` happened to be.
4. **`FDisFogPixelShader` binds `MaskTexture` and never sets it** (`FogRendering.cpp:553/574/700`, agent BD's file,
   **not fixed here**). Retail's own `SetParameters` (2013 rva 0x41c940) does not set it either - it has exactly two
   sampler binds, the per-layer `FogLUT`s and `BloomParts` - so retail fills that target elsewhere, in
   `RenderFogMaskStencil` (0x433f80), which this tree does not have. It matters now that something writes the
   fog-mask target every frame: with `-bloomclearblack`, which changes **only the clear colour of that target**, the
   whole frame turns magenta on both maps (mean 464 of 765 on `L_Pub_Day_P`, R and B saturated, G untouched). Retail's
   `FLinearColor(128,128,0,0)` quantizes to (255,255,0,0) - `FLinearColor::Quantize` multiplies by 255 and clamps in
   both builds, checked against 2013 rva 0x19cf0 - so the odd-looking clear is a **white** mask, and a white mask is
   what the fog wants. Copy it exactly; do not "fix" it to black. `-bloomclearblack` is kept as the measurement.
5. **`FScene` has no `m_Rains`** (@10160 in retail, the rain scene info). The compose gate is
   `DisFogs.Num() || m_Rains.Num()`; only the fog half can be tested here. A level with rain and no fog would compose
   twice once the rain pass exists. Documented at the site.

## 5. Not ported, and what each one needs

* **`FSceneRenderer::RenderFogMaskStencil`** (2013 rva 0x433f80, 9,806 bytes): the fog mask's real producer, and
  defect 4's fix. Until it exists the fog samples whatever is resident in that sampler slot.
* **the bloom parameters are still the class defaults** - `enable 1, scale 1.000, threshold 0.000` in every run here.
  The two writers of `FArkPpConfig` (`ADishonoredPlayerCamera::SetPostProcessTarget`,
  `UDisPostProcessManager::ApplyKismetPostProcessSettings`) are agent CE's hand-over 6 and are still unported, so no
  level's own bloom tint, scale or threshold reaches the pass. With a real threshold the halos would be tighter than
  the ones measured here; with a real tint they would be coloured.
* **the soul-part pass** (`TSoulPartMeshVertexShader` / `PixelShader`, `FViewInfo::m_VisibleSoulPrimitives` @1460,
  relevance bit 23 `iSoulRenderingRelevance`): the same shape as this package, one set and one mesh pass. The shader
  types are declared and load; nothing fills the set or draws it. `ProcessVisible` fills `m_VisibleSoulPrimitives`
  already (it is the reference `VisibleDynamicPrimitives` block's neighbour) - the pass itself is what is missing.
* **`bHasBloomPartViewMeshElements` is never set**, exactly as the reference `bHasTranslucentViewMeshElements` beside
  it is never set: `ViewMeshElements` is an editor path. The gate degrades to "the set is non-empty", which is what
  the game exercises.

## 6. Numbers

| Measure | HEAD `668a4ec` | With package DB |
|---|---:|---:|
| primitives carrying the bloom-part relevance bit (`L_Pub_Day_P`, settled) | 0 | **7** (235 on the first frame after the level is up) |
| `BloomPartPrimSet[0..3]` | absent | **0 / 7 / 0 / 0** |
| draws the pass issues per frame (`L_Pub_Day_P`) | 0 | **16** (13 mesh + 1 downsample + 2 blur) |
| draws the pass issues per frame (`L_Tower_P`) | 0 | **8** (5 mesh + 1 downsample + 2 blur) |
| pixels the pass changes (same world frame) | - | **3.7 %**, mean 3.95 of 765 |
| noise floor (two runs, same switch) | - | **0.0 %**, mean 0.00 of 765 |
| d3d9 frames in the regression's 90 s run | 20,220 | 20,130 |
| `Initial startup` | 3.4 s | 3.4 s |
| regression | 31 checks | **31 ok, 0 failed** |

## 7. Files

Mine (5): `Engine/Src/arkbloompartsrendering.h` (new), `Engine/Src/arkbloompartsrendering.cpp` (the pass, the policy,
the factory, the set, the blur, the vertex declaration, and the two mesh shaders' `SetParameters`/`SetMesh`),
`Engine/Src/SceneRendering.h` (`BloomPartPrimSet[4]`, `bHasBloomPartViewMeshElements`, the `RenderBloomParts`
declaration), `Engine/Src/SceneRendering.cpp` (the relevance plumbing in `ProcessVisible`, the call site in
`RenderDPGEnd`, the census, `-apshottime`), `Engine/Inc/Scene.h` (the two relevance bits).

Outside my list, small and deliberate (5), because the package cannot work without them:
`Engine/Src/ScenePrivate.h` (one include - `FViewInfo` carries the set - and four census externs),
`Engine/Inc/MaterialShared.h` + `Engine/Src/MaterialShared.cpp` (`HasBloomPartForCompilation`, three lines, and the
one line of `GetViewRelevance` that fills `bBloomPart`), `Engine/Src/SceneRenderTargets.h` +
`Engine/Src/SceneRenderTargets.cpp` (`BeginRenderingFogMask` / `FinishRenderingFogMask`, and defect 2's two formats).
`ScenePostProcessing.cpp` was not needed. Nothing of agent DA's (`arkppnodedof.cpp`, `arkpp.h`, `arkppnodes.cpp`) was
touched.

## 8. Bring-up switches (all function-local statics, per agent CA's finding)

| Switch | What it does |
|---|---|
| `-nobloomparts` | leaves the pass out; the switch of the pair |
| `-bloomcompose` | composes the bloom in this pass even where the fog pass would have, so the compose half can be measured on a map with fog |
| `-bloomclearblack` | clears the fog mask to black instead of retail's (128,128,0,0); the measurement behind defect 4 |
| `-apshottime=<s>` | one bitmap of the frame whose world time has reached `<s>`, requested on the game thread (section 3); named `apshottime*.bmp` so concurrent agents' runs do not collide |

## 9. Hand-overs

1. **`RenderFogMaskStencil` (0x433f80) is the next thing in this corner**, and defect 4 says why: the fog's
   `MaskTexture` has no producer, and this pass now writes that target every frame.
2. **The bloom parameters** (section 5): until `FArkPpConfig` has a writer, every level blooms with
   `threshold 0, scale 1, tint white`. That is agent CE's hand-over 6 and it is now measurable - the halos in
   `figure_window.png` are what the defaults look like.
3. **The soul-part pass** is the same shape as this one and its set is already filled.
4. **Bring-up lines to drop when the pass is finished**: the four switches above, the bloom counters in the
   post-process census line, and the `ReportMissingBloomPartShaders` warn-once.
5. **`-apshot` should be retired in favour of `-apshottime`** for anything quantified (defect 1).
