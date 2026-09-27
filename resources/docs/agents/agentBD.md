# Agent BD report — the real look: the Arkane post-process shader families and the fog pass (2026-09-27)

Package **BD** of `PHASE8.md` (wave 6). Measured from **HEAD `24d8b8d`**: build dir `build\agentBD` (Release, x86, Ninja,
every module option), built from the snapshot worktree `build\agentBD_wt` (= HEAD + my files,
`resources\tools\make_snapshot.py BD --list build\agentBD\files.txt`, list in `build\agentBD\snapshot_files.txt`,
build command `build\agentBD_release_build.cmd`). IDA copies `resources\docs\idb\retail2013_agentBD.i64` /
`shipping2012_agentBD.i64`, headless `decompile_funcs.py` only; decompiles in `build\agentBD\decomp12`,
`build\agentBD\decomp12b`. No commits, nothing staged. Status rows: `agentBD_status.csv` (37).

Agents BA, BE and BF are editing the shared tree at the same time; `build\agentBD\snapshot_fixup.py` keeps their
in-flight edits to four generated DishonoredGame files out of my snapshot (see "Files I touched outside my package").

## Result

| Accept | State |
|---|---|
| undeclared shader types **136 → under 20** | **5**, all named and explained below. `DISHONORED(bringup): global shader cache: 263 shaders, 258 loaded, 5 undeclared types, 0 parameter mismatches, 0 other skips` (HEAD: 127 loaded, 136 undeclared). 131 cooked records that had no declaration now load, and every one of them loaded byte-exactly on the first run - `ShaderCache.cpp`'s `checkf(Ar.Tell() == SkipOffset)` passes for all 258 |
| before-and-after screenshot pair, same camera | **done**: `build\agentBD\pair_before_nopostprocess.png` / `pair_after_disfog.png`, both at scene frame 200 of `L_Tower_P` with `-benchmark -fps=30`, one build, the only difference being the new `-nopostprocess` switch. **Mean per-pixel difference 27.96 of a possible 765 (3 channels), 37.0 % of pixels changed by more than 2 per channel, maximum 294** (`build\agentBD\compare_pair.py`). The two runs are the same camera to the pixel: with the pass off in both they come out byte-identical (largest difference 0), which is how the pair was validated before the fog was switched on |
| per-pass draw counts in a `DISHONORED(bringup)` census line | **done**: `post-process census: DisFog 2 layers in scene, 2 drawn in 1 passes; bloom parts 0 prims, 0 draws; FArkPp 0 nodes, 0 draws` |
| regression harness green | **every check of my package passes.** The baseline moved under me while I worked, so both numbers: against the baseline **as it stands in the shared tree now** (24 checks) the final build is **21 ok, 1 failed, 2 skipped**, and the one failure is `unported_natives <= 0`, a bound **agent BF** tightened for BF's own package, which my snapshot (HEAD + my files) does not contain; against **HEAD's baseline** it is **20 ok, 1 failed, 3 skipped**, and the one failure is `touch_census`, which needs the `-distouchprobe` argument only the newer baseline passes (with it the same build reports 248 touch begins). Mid-package, against the baseline of the time, the run was **19 ok, 0 failed, 4 skipped**. Every renderer metric - frames, draws per frame, draw elements, visible primitives, criticals, texture census - passes in all of them (`build\agentBD\regression_final2.txt`, `regression_head_baseline2.txt`) |
| run | `Initial startup: 3.16s`, **2,550 scene frames in 90 s**, 0 criticals, 0 `Failed to find shader type` |

```
rem accept
python resources\tools\build_and_smoke.py --build-dir build/agentBD --no-build --exe-name DishonoredGame_BD.exe ^
  --log-name agentBD.log --ini-dir build/agentBD/config --rhi d3d9 --timeout 90 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --forbid "Critical" --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -apshot=200 -benchmark -fps=30 -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
rem the same run with the post-process passes off, for the pair
  ... "--extra-args=... -nopostprocess ..."
rem regression
python resources\tools\run_regression.py --build-dir build/agentBD --no-build
```

## 1. The 136 undeclared types (package item 1)

The cooked record is the specification: `build\agentY\gsc.json` gives every record's serialization history, so the
parameter count of a type is `(history words - the 11 of FShader::Serialize) / 3`, and agent AG's
`build\agentAG\statictypes2013.csv` gives the registered **name**, source file, entry point and the two version gates
of every `StaticType` initializer in the 2013 exe. The layouts come from the constructor (one 2-byte zero store per
parameter), `Serialize` and `SetParameters` decompiles. Every type below was checked against both.

| Family | Types | Where | Gate (package / licensee) | Parameters |
|---|---:|---|---|---|
| `FGFxFilterPixelShader<FS2_*>` | 26 | `GFxUI/Src/gfxuishaders.cpp` (new) | 786 / 1 | 2 textures + 2 tex scales + 2 colour transforms + 2 shadow colours + gamma, colour matrix, blur size, shadow offset = 12, written interleaved (0x57c920) |
| `FGFxPixelShader<GFx_PS_*>` | 17 | same | 786 / 1 | 4 textures + constant colour, colour scale/bias, gamma and the 8 distance-field text constants = 16 |
| `FGFxVertexShader<GFx_VS_*>` | 7 | same | 786 / 1 | transform + 2 texture matrices = 3 |
| `TDisFogVertexShader<FDisFogPolicy<N,M>>` | 15 | `Engine/Src/FogRendering.cpp` | **797 / 23** | screen position scale/bias, screen-to-world = 2 |
| `TDisFogPixelShader<FDisFogPolicy<N,M>>` | 15 | same | **797 / 23** | scene textures (5) + 8 per-layer constants + mask + bloom parts + 4 fog LUTs + sun direction = 20 |
| `FHeightFogMask{Vertex,Pixel}Shader` | 2 | same | 786 / 15 and **797 / 19** | transform; fill value |
| `TFilterPixelShaderDepthInAlpha<1..16>` | 16 | `Engine/Src/SceneFilterRendering.h` | 786 / 1 | `TFilterPixelShader<N>` (3) + scene textures (5) = 8 |
| `TArkPpBlur{Vertex,Pixel}Shader<Policy>` | 8 | `Engine/Src/arkppnodeblur.cpp` | 786 / 1 | 3 each (resolution, viewport scale/bias, noise; source, vector field, vector-field scales) |
| `FArkPpDof*` / `TArkPpDofUberPS<a,b>` | 8 | `Engine/Src/arkppnodedof.cpp` | 786-**789** / 1 | 1, 1, 3, 7, 0, 6 |
| `TKuwa{Vertex,Pixel}Shader<3,5>` | 4 | `Engine/Src/arkppnodekuwa.cpp` | **793 / 26** | 2 and 1 |
| `TFXAAPixelShader<Luma,Quality,ForSceneColor>` | 6 | `Engine/Src/arkppnodeaa.cpp` | 786 / **24** | 11 |
| `TMLAAEdgeDetection/BlendPixelShader<0,1>` | 4 | same | 786 / 1 | 4/3 and 5/4 - the linear variant carries the inverse display gamma, the sRGB one does not |
| Arkane bloom (`FBloomCompose/DownSample`, `TBloomBlur<5>`) | 6 | `Engine/Src/arkbloompartsrendering.cpp` | 786 / 23 | 0, 6, 0, 2, 1, 1 |
| **total declared** | **134** | | | |

Two of the gates are not the 786/1 floor and would have rejected their records: DisFog is **797 / 23** and the Kuwahara
filter **793 / 26**, read off the `StaticType` pushes exactly as agent AG read the material ones, not guessed.

Three details the cooked names forced:

* retail registers a type under a name that is often **not** the C++ class name (`FDisFogPixelShader00Layer` for
  `TDisFogPixelShader<FDisFogPolicy<0,0>>`, `FArkPpDofUber_01PS` for `TArkPpDofUberPS<0,1>`,
  `FMLAABlend_SRGB_PixelShader` for `TMLAABlendPixelShader<1>`). `ShaderManager.h` gains
  **`IMPLEMENT_SHADER_TYPE_NAMED`**, which takes the name explicitly; `IMPLEMENT_SHADER_TYPE` keeps stringifying the
  class for the types whose names do match.
* `FHeightFogMaskPixelShader< 0 >` is registered with the spaces of the template argument in it.
* the GFx filter shader binds a highlight parameter that its `Serialize` never writes (13 members, 12 serialized).

### The 5 that are left, and why

| Type | Why |
|---|---|
| `TMeshPaintPixelShader`, `TMeshPaintVertexShader` | **retail does not declare them either.** The 2013 exe has no `StaticType` for either (`statictypes2013.csv`); they are the editor's mesh-paint tool, cooked into the cache by the editor and skipped by the game. Comparing the 263 cooked names against the exe's registered names leaves exactly these two |
| `FBinkVertexShader`, `FBinkYCrCbAToRGBAPixelShader`, `FBinkYCrCbToRGBNoPixelAlphaPixelShader` | build flag: this build dir has `DISHONORED_WITH_BINK=OFF`, so `BinkMovieTexture.cpp` is not compiled. They load in a `-DDISHONORED_WITH_BINK=ON` build (agent Y measured that) |

So against a Bink build the number is **2**, and those two are undeclared in retail as well.

`global shader map: 136 types without a cooked shader` (was 133): the three new ones are
`FFXAAPixelShader_*_ForSceneColor`, which retail registers too and for which the PC cache has no shader.

## 2. DisFog: the pass that makes the world look like Dishonored (package item 2)

Retail has no height fog and no exponential height fog - `FSceneRenderer::RenderFog` (2013 rva 0x4370a0) draws
Arkane's layered fog and nothing else. The whole path is ported:

* **`UDisFogComponent` / `UDisFogDisplayComponent` / `ADisFog` moved into Engine** (`Engine/Inc/EngineDisFogClasses.h`,
  `Engine/Src/disfogcomponent.cpp`), the same move agent AI made for the Arkane Engine classes: they are
  `native_class_sizes.csv` `package_2013 = Engine` classes that lived in `DishonoredGameEngineShims.h`. Layouts copied
  verbatim from the generated shim blocks and asserted (156 / 464 / 608 bytes). `Attach`/`UpdateTransform`/`Detach`/
  `SetParentToWorld` and the `SetEnabled` native are ported from their decompiles.
* **`FDisFogSceneInfo`** (88 bytes, `FogRendering.h`) and **`FDisPrecomputedFogSceneInfo`** (208 bytes, `FViewInfo`
  @4032) with the retail field order; `FScene::DisFogs`, `FScene::AddDisFog` / `RemoveDisFog` and the sort that puts
  the layers with a colour lookup texture first (that leading run is what picks the shader policy).
* **`FSceneRenderer::RenderFog` / `RenderFogPass` / `FlushDisFogShader`**: the scene's layers are split into an
  interior and an exterior set, a sun layer and an exclusive layer are moved to the front of their set (the pixel
  shader reads the sun from layer 0, and an exclusive exterior layer switches the interior set off), each set is
  clamped to four layers, and each is drawn as one clip-space triangle with
  `FDisFogPolicy<Layers,LayersWithLut>` - index `LayerBaseOffset[Layers] + Luts` of the 15 policies.
* **`TDisFogPixelShader::SetParameters`** (0x41c940): per-layer minimum and maximum height relative to the camera,
  near plane, 1/(far-near), no-fog plane, height density, colour and opacity, the custom-transition fade, the four
  lookup textures, the negated sun direction with the sun power in w, and the bloom-parts texture; the same values are
  written into `FViewInfo::DisPrecomputedFogs` for the exterior pass, which is what agent AG's
  `TBasePassPixelShader<Policy,bSkyLight,TRUE>` reads.
* the reference height-fog pass is renamed `RenderReferenceFog` and kept behind `-referencefog`; `RenderDPGEnd` calls
  the retail one where retail calls it, between the soft-masked base pass and the distortion pass.

Measured in `L_Tower_P`: **2 exterior layers**, opacity 0.20 and 0.15, colours (0.58 0.80 0.98) and (0.68 0.84 0.98),
near/far 2500/35000 and 80000/120000, no lookup texture, so policy 3 = `FDisFogPolicy<2,0>`; **one pass, one draw per
frame**, and the image changes as in the pair above.

### One measured deviation from retail, with the tree defect it exposes

Retail's pass is `ResolveSceneColor` → `BeginRenderingSceneColor` → draw → `FinishRenderingSceneColor`. Written that
way the pass draws, the shaders bind (logged), the layer data is right - and **not one pixel changes**. I measured it
with an `RHIClear` to red at four points of the pass: a clear *before* `BeginRenderingSceneColor` covers the screen, a
clear *after* it is never seen, with or without `RTUsage_RestoreSurface`, with or without the `Finish`. So on this tree
the scene colour **surface** is not where the visible frame is: `FD3D9DynamicRHI::CopyToResolveTarget`
(`D3D9Drv/Src/D3D9RenderTarget.cpp:24`) leaves the *resolve destination* bound as the render target and nothing
re-binds the scene colour surface afterwards, so a pass that binds it again writes to a surface nothing resolves.

The pass therefore keeps retail's resolve (the fog pixel shader samples scene colour through
`FSceneTextureShaderParameters`, so the resolve has to happen) and **skips the surface switch**, drawing into the
target the caller has bound - which after that resolve is the texture the frame is read from. `-fogresolvetargets`
runs the retail sequence, so the next agent can re-measure in one run once the resolve handling is fixed. This is a
tree-wide defect, not a fog one: any pass that re-binds scene colour mid-frame has the same problem.

## 3. The post-process chain reaches the player again (package item 3)

`FSceneView::m_ArkPpConfig` (2012 PDB @28) did not exist in this tree, so `FBloomDownSamplePixelShader::SetParameters`
(0x50e5a0) and the FArkPp depth-of-field node had nothing to read. It is declared now and
`ULocalPlayer::CalcSceneView` fills it from `m_CurrentArkPpSettings`, which is where the camera, the level default and
Kismet/volume pushes end up (`ADishonoredPlayerCamera::SetPostProcessTarget` 0x70a010 and
`UDisPostProcessManager::ApplyKismetPostProcessSettings` 0x850ef0 are DishonoredGame and still unported, so it holds
the class defaults today: bloom enabled, scale 1.0, threshold 0.0).

On the volumes agent AS enumerated: **`ACullDistanceVolume`** is declared in this tree and feeds primitive draw
distances, not the post-process chain. **`AColorScaleVolume` has no reflected member at all in the retail SDK dump**
(`retail_sdk_layout.json`: `AVolume`, empty span) - Dishonored's colour treatment does not come from a colour-scale
volume but from `FArkPpConfig`, through the path above. So the answer to "do they reach the chain" is: the cull
distance volumes do their own job; the colour path is now wired end to end except for the two DishonoredGame writers.

### And a real defect found on the way: the player's chain was being emptied

`ULocalPlayer::RebuildPlayerPostProcessChain` was the reference implementation: it builds a **new, empty**
`UPostProcessChain` and copies the source chain's `Effects` array into it. Retail's (2013 rva 0x2b9ac0) duplicates the
inserted chain **whole**. Dishonored's chain is Arkane's node graph (`m_GraphRoot`, `m_AllNodes`), and `Effects` is one
of this tree's storage-less shims, so the rebuild threw the graph away on every level load. Ported:

```
post-process chain: Transient.PostProcessChain_3 ..., graph root none, 0 nodes in chain      (before)
post-process chain: Transient.PostProcessChain_3 ..., graph root set, 42 nodes in chain      (after)
```

The content's chain is `AltScreen_Effects.PostProcessChain.Test_PPG` (the engine default from
`DefaultPostProcessName`; `WorldInfo.WorldPostProcessChain` is unset in this map), 210 `ArkPpNode` objects are loaded,
and the player's chain now has all 42 nodes. **That was the missing input for the FArkPp graph** - it is why the graph
port below is now a contained piece of work rather than a search.

## 4. What is not ported, and exactly what it needs

* **The FArkPp node graph** (0 nodes, 0 draws in the census). Everything it needs is now measured:
  1. the node classes `UArkPpNode{,AA,Blur,CommonTarget,Controller,Dof,Kuwa,Material,SceneColor,Switch,Target}` and
     `UArkPpSettings` are declared and registered by **DishonoredGame's shim header**; they have to move to Engine
     (`EngineArkPpClasses.h`) the same way the DisFog classes did in this package, because the proxies are Engine's;
  2. the proxy family is small: `FArkPpNodeProxy` is 12 bytes (`FRefCountedObject` + `m_bDone`) with
     `Render`/`GetSurface`/`GetTexture`/`GetSurfaceSizeX,Y`; `FArkPpNodeSceneColorProxy` 16,
     `FArkPpNodeTargetProxy` 20, `FArkPpNodeCommonTargetProxy` 40, `FArkPpNodeMaterialProxy` 48 (2012 PDB); the switch
     node has no proxy, it returns the chosen input's;
  3. the graph is built in **`FViewInfo::FViewInfo(const FSceneView*)`** (2012 rva 0x493e40): reuse the view's proxy if
     it has one, else `m_GraphRoot->IsValid(FArkPpIsValidData)`, then `FArkPpCreateProxyConfig`,
     `PushUberOverride(m_ArkPpConfig->m_UberPpParameters, 1.0)`, `m_GraphRoot->CreateSceneProxy(config)` and
     `FViewInfo::AddPostProcessProxy`;
  4. it renders from `RenderPostProcessEffects(SDPG_PostProcess)` (0x448990), where agent AH left the gate;
  5. the material node draws with agent AG's `TPpMaterialVertexShader`/`TPpMaterialPixelShader` types, which load
     today, through `TPpMaterialDrawingPolicy` (0x55b010 / 0x560890 / 0x55a640) - that node is where Dishonored's
     colour treatment lives, so it is the one to port first.
  The render targets it needs (`ArkDofHalf`, `ArkDofQuarter`) are already allocated by this package.
* **`FSceneRenderer::RenderBloomParts`** (0x5251a0): the shaders, the two quarter-size targets (`ArkBloom`,
  `ArkBloom2`), `BeginRenderingBloom`/`FinishRenderingBloom` and `m_BloomNeedBlit` are in place; what is missing is
  `FViewInfo::BloomPartPrimSet[4]` and the per-primitive relevance that fills it (`FArkBloomPartPrimSet` is 12 bytes,
  `AddScenePrimitive` 0x54e6c0, `DrawBloomPrims` 0x565e80), plus the Gaussian blur helper. The fog pixel shader
  already binds the bloom-parts texture when `m_BloomNeedBlit` is set, so bloom will light up the fog for free.
* **`FSceneRenderer::RenderFogMaskStencil`** (0x433f80, 9.9 KB): the fog mask meshes that stop interior fog at a
  portal. Without it every layer covers the whole view, which is what a level with no mask meshes does anyway; the
  `FHeightFogMask*` shaders it needs are declared.
* **`FSceneView::SceneReflectionTexture`** (agent AG hand-over 2) and `UPostProcessChain::CreateMutableMaterialInstanceOnNodes`
  (0x2d2a40) still wait for the node classes.

## 5. Files

Mine (`build\agentBD\files.txt`, 28):
`Engine/Inc/ShaderManager.h` (the named-type macro), `Engine/Inc/FogRendering.h`, `Engine/Src/FogRendering.cpp`,
`Engine/Src/SceneFilterRendering.{h,cpp}`, `Engine/Src/SceneRenderTargets.{h,cpp}` (the five Arkane render targets and
the bloom begin/finish), `Engine/Src/arkppnode{blur,dof,kuwa,aa}.cpp`, `Engine/Src/arkbloompartsrendering.cpp`,
`Engine/Src/arkcommonvertexdeclaration.cpp` + new `Engine/Inc/arkcommonvertexdeclaration.h`,
new `Engine/Inc/EngineDisFogClasses.h`, `Engine/Src/disfogcomponent.cpp`, `Engine/Inc/Scene.h`, `Engine/Src/Scene.cpp`,
`Engine/Src/SceneCore.{h,cpp}`, `Engine/Src/ScenePrivate.h`, `Engine/Src/SceneRendering.{h,cpp}`,
`Engine/Inc/Engine.h`, `Engine/Src/UnEngine.cpp`, `Engine/Src/UnPlayer.cpp`,
`GFxUI/Src/gfxuishaders.cpp`, `GFxUI/Sources.cmake`, `CMakeLists.txt`.

`Engine/Inc/enginedisfogclasses.h` is tracked lowercase (the `import_reference.py` stub) and is now
`EngineDisFogClasses.h` on disk, to match `EngineArkaneClasses.h` and the include in `Engine.h`; git sees a
case-only rename, so the merge wants a `git mv`.

### Files I touched outside my package list

* **`GFxUI/Sources.cmake`** (one line): `Src/gfxuishaders.cpp` is back in the build, because that is where the 50 GFx
  shader types live. **Agent BE rewrote this file while I was working and my line was lost once**; I re-applied it on
  top of BE's version. Whoever merges must keep both changes.
* **`CMakeLists.txt`** (four lines): `DISHONORED_WITH_GFXUI_SHADERS=1` on the Engine target when GFxUI is on, so
  `FogRendering.cpp`'s link anchor can reference `DishonoredLinkGFxShaderTypes`. Engine is a static library and the
  Arkane post-process units have no caller yet, so one symbol per unit is referenced from `FogRendering.cpp` - agent
  AG's trick, and it is needed for the same reason: an object file nothing references is dropped with its shader type
  registrations.
* **four generated DishonoredGame files** - `DishonoredGameEngineShims.h`, `DishonoredGameLayouts.h`,
  `DishonoredGameRegistrants.cpp`, `DishonoredGameNativeStubs.cpp`: the three DisFog classes, their layout asserts,
  their `IMPLEMENT_CLASS`, their native table and the `execSetEnabled` stub are removed, because Engine declares them
  now. `sdk_select` drops a shim class the tree declares, so **regenerating DishonoredGame produces the same result**;
  as with agent AI's move, the shared tree does not compile until the coordinator regenerates (or keeps these edits).
  My snapshot never takes these four files from the shared tree - agents BE and BF are editing them at the same time -
  so `build\agentBD\snapshot_fixup.py` restores HEAD's copies and re-applies only the DisFog removals after every
  `make_snapshot.py BD --sync`.

## 6. Numbers

| Measure | HEAD `24d8b8d` | With package BD |
|---|---:|---:|
| cooked global shader records loaded | 127 | **258** of 263 |
| undeclared types | 136 | **5** (2 in a Bink build) |
| parameter mismatches | 0 | 0 |
| DisFog layers rendered per frame | 0 | **2, in 1 pass, 1 draw** |
| post-process graph nodes reaching the player's chain | 0 | **42** |
| pixels changed by the post-process chain (same camera) | - | **37.0 %**, mean 27.96 of 765 |
| scene frames in the 90 s run | 2,520 | 2,550 |
| `Initial startup` | 3.2 s | 3.16 s |
| regression (the shared baseline of the day, 24 checks) | - | **21 ok / 1 failed**: agent BF's `unported_natives` bound, which needs BF's package |

## 7. Hand-overs

1. **Coordinator / whoever owns `SceneRenderTargets` and the D3D9 RHI**: `CopyToResolveTarget` leaves the resolve
   destination bound (section 2). Every pass that re-binds the scene colour surface mid-frame draws into a surface
   nothing resolves again. The fog pass has `-fogresolvetargets` to re-measure the retail sequence in one run.
2. **Wave 7 / the FArkPp graph**: section 4, items 1-5. The chain now has its 42 nodes, which was the blocker.
3. **Agent BE / the coordinator**: `GFxUI/Sources.cmake` and the four generated DishonoredGame files (section 5).
4. **Agent AG's hand-over 2** (`FSceneView::SceneReflectionTexture`) is still open; `m_ArkPpConfig` and
   `m_PostProcessProxy` are the other two PDB members of `FSceneView` in that block - `m_ArkPpConfig` is declared now,
   `m_PostProcessProxy` comes with the graph.
5. **Bring-up lines to drop when the graph lands**: the post-process census, the one-shot chain line in
   `CalcSceneView`, `-nopostprocess`, `-fogresolvetargets`, and the link-anchor array in `FogRendering.cpp`.
