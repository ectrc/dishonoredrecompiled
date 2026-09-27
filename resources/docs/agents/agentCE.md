# Agent CE report — the Arkane post-process graph (2026-09-27)

Package **CE** of `PHASE9.md` (wave 7). Written against **HEAD `8e61755`** (agent CA merged, `c403e2f`) and
**re-verified on HEAD `683fa03`**, which has CB, CF and CD in it: none of those three touched any of this
package's eighteen files, the snapshot rebuilt clean on it (863 units, 0 errors), the pair measured the same to
the hundredth and the harness is green there. Build dir
`build\agentCE` (Release, x86, Ninja, every module option), built from the snapshot worktree `build\agentCE_wt`
(= HEAD + my files, `resources\tools\make_snapshot.py CE --list build\agentCE\files.txt`, list in
`build\agentCE\snapshot_files.txt`, build command `build\agentCE_release_build.cmd`). IDA copies
`resources\docs\idb\shipping2012_agentCE.i64` / `retail2013_agentCE.i64`, headless `decompile_funcs.py` only;
306 decompiles in `build\agentCE\decomp12`, `decomp12b`, `decomp13`. No commits, nothing staged. Status rows:
`agentCE_status.csv` (29).

Six packages were editing the shared tree at the same time, so every build and every run is the snapshot's.
`build\agentCE\resolve.py` resolves a Launch.log callstack against the build's `.map`; `build\agentCE\run_pair.py`
takes one 90 s run and copies its screenshot out (the retail screenshot index is shared by every agent's runs);
`build\agentCE\rva2013.py` rewrote the package's `2012 rva` citations as `2013 rva ... (2012 ...)` from
`match_2012_2013.csv`.

## Result

| Accept | State |
|---|---|
| the node classes move into the engine | **done**: twelve classes, six nested structs and four enums moved out of DishonoredGame's generated shim header into `Engine/Inc/enginearkppclasses.h`, sizes and offsets asserted, registered from `UnEngine.cpp` exactly as agent BD's DisFog move. The generator drops a shim class or struct the tree declares, so regenerating DishonoredGame produces the same result |
| the graph is built in the view constructor and rendered from the post-process pass | **done**: `FViewInfo::FViewInfo` (2013 rva 0x46b3a0) turns the chain's `m_GraphRoot` into a proxy tree and `FSceneRenderer::RenderPostProcessEffects` (0x448990) renders it at `SDPG_PostProcess`; `FArkPp graph: root Transient.PostProcessChain_3:ArkPpNodeMaterial_15, 42 nodes (20 material, 1 target, 3 common target, 1 scene colour, 12 switch, 1 blur, 1 dof, 2 kuwa, 1 aa), proxy built, 3 proxies built` |
| the material node draws | **done and measured**: the pass binds the surface the frame is presented from (the red-clear measurement, section 5.3), draws the content's own material over it at the right size with both of its node inputs bound and their sampler states, and the census counts the tile. What it draws is the node the content puts first in this chain: `TEST_PPG_EyeLid`. **The colour grade of this content is not a material node at all - it is the depth-of-field node's uber pass, which this package does not port (section 4).** That is the one place where this package does not deliver what the plan expected, and section 4 is the evidence |
| before-and-after pair, one build, one switch, quantified | **done**: `build\agentCE\pair_before_a.bmp` / `pair_after.bmp`, scene frame 200 of `L_Tower_P`, `-benchmark -fps=30`, one build, the only difference being `-noarkpp` (`-arkppcontrollersshown` is on in both, so the eyelid node is the one retail draws while its effect runs). **Mean per-pixel difference 261.83 of a possible 765, 99.2 % of pixels changed by more than 2 per channel, maximum 765** - the eyelid material over the whole frame, which is what that node draws. The camera is the same: two runs with `-noarkpp` in both differ in **832 of 921,600 pixels (0.09 %), mean 0.01 of 765** (`pair_before_a.bmp` vs `pair_before_b.bmp`), which is this map's own animation, not the camera |
| per-node draw counts in the census | **done**: `post-process census: DisFog 2 layers in scene, 2 drawn in 1 passes; bloom parts 0 prims, 0 draws; FArkPp 1 nodes rendered, 1 draws (1 material tiles), 2 passes not ported` for the pair's after run and `FArkPp 0 nodes rendered, 0 draws (0 material tiles), 2 passes not ported` at rest, plus a one-shot line per graph node and per chain switch under `-arkppdbg` (section 4) |
| regression harness green | **31 ok, 0 failed**: `22 ok, 0 failed, 2 skipped` for the five game stages (`build\agentCE\regression1.txt`, the two skips being CoreSmoke and LayoutProbe, which that run did not have built yet) and `9 ok, 0 failed, 0 skipped` for those two once built (`regression2.txt`). The layout stage is worth naming: `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341` - the probe checks the moved classes' offsets and sizes against the retail SDK dump, so the class move is verified and not just asserted |
| re-verified on the newer HEAD | **done**: `683fa03` (CB, CF, CD merged) - rebuilt from scratch, pair identical (mean 261.83, 99.2 %), `regression_head.txt` 25 ok, 5 failed, 1 skipped in one pass and 9 ok, 0 failed when the d3d9 stage is re-run on its own: that stage's first attempt never created the D3D9 device (431 log lines, no critical, it stops after 'Initializing Engine...'), which is the machine being saturated by six agents' runs - the re-run measured 20,130 frames, 1,203 draws per frame, 6,506 draw elements, 461 visible primitives, 0 criticals, and the inputtest stage, which also renders with d3d9, passed in the same pass (1,024.7 units walked, 895 physics actors) |
| run | `Initial startup: 3.33 s`, 321 scene frames in the 90 s pair run (4,830 in the regression's own d3d9 run, which does not stop for a screenshot), 0 criticals, 0 `Failed to find shader type` |
| the graph at rest | `FArkPp 0 nodes rendered, 0 draws (0 material tiles), 2 passes not ported` and **3 proxies of 42 nodes** (scene colour, depth of field, AA): with no effect running, this content's graph asks for exactly the two passes this package stubs, and the frame is then the `-noarkpp` frame. Section 4 |

```
rem accept: the pair (one build, the only difference is -noarkpp)
python build\agentCE\run_pair.py pair_before_a -noarkpp -arkppcontrollersshown
python build\agentCE\run_pair.py pair_after            -arkppcontrollersshown
python build\agentBD\compare_pair.py build\agentCE\pair_before_a.bmp build\agentCE\pair_after.bmp
rem the census and the node dump at rest
python build\agentCE\run_pair.py after_graph2 -arkppdbg
rem regression
python resources\tools\run_regression.py --build-dir build/agentCE --no-build
```

## 1. The node classes (package item 1)

`UArkPpNode`, `UArkPpNodeController`, `UArkPpNode{SceneColor,Target,CommonTarget,Switch,Material,Blur,Dof,Kuwa,AA}`
and `UArkPpSettings` were `package_2013 = Engine` classes living in DishonoredGame's shim header. They are now
`Engine/Inc/enginearkppclasses.h`, with the six structs nested in them (`FAnInput`, `FBoxBlurConfig`,
`FMotionBlurConfig`, `FRadialBlurConfig`, `FFxAaConfig`, `FMlAaConfig`) and the four enums
(`EPpNodeRenderStage`, `EPpNodeCommonTarget`, `EPpNodeBlurType`, `EPpNodeAAType`) they use. The blocks are copied
verbatim, every size and the load-bearing offsets are asserted, `IMPLEMENT_CLASS` moved to
`Engine/Src/arkppnodes.cpp` and `AUTO_INITIALIZE_REGISTRANTS_ENGINE_ARKPP` into `UnEngine.cpp` next to DisFog's.
The four enum *names* matter: `EPpNodeCommonTarget` reads `EPpCt_AttenuationBuffer, EPpCt_FogMask, EPpRs_SceneColor,
EPpRs_SceneColorLdr, EPpRs_LowResParticles` in the content, and those names do not say what they select —
`FArkPpNodeCommonTargetProxy::GetSurface` (2013 rva 0x50b840) maps 0 and 3 to the LDR scene colour, 1 to the fog
mask, 2 to `ArkDofQuarter` (buffer >> 2) and 4 to `ArkDofHalf` (buffer >> 1). Taking the names at face value would
have pointed three of the five at the wrong surface.

The base class's own virtuals are not guesses either: the eight graph methods are COMDAT-folded in both shipping
builds, and `vtables.csv` slots 72..80 of `UArkPpNode` fold onto bodies that return 0, 0, NULL, 1, 0, 0, 0, NULL —
so `NumInputs` is 0, `LinkInput` returns TRUE, `IsValid` returns FALSE and `CreateSceneProxy` returns NULL in the
base, and `UArkPpNodeSceneColor` / `CommonTarget` override `IsValid` to TRUE while `SceneColor`, `Target` and
`CommonTarget` override `OutputIsSurface` to TRUE.

## 2. The graph (package item 1, continued)

New `Engine/Inc/arkpp.h`: `FArkPpRenderConfig` (one bit, `m_bForceToDestination`), `FArkPpIsValidData` (the memo of
`IsValid`), `FArkPpNodeProxy` (12 bytes: `FRefCountedObject` + `m_bDone`, with `Render`, `GetSurface`, `GetTexture`,
`GetSurfaceSizeX/Y`) and `FArkPpCreateProxyConfig` (2013 rva 0x46ab60: `mbOnlyInEditor`, `mbRedirectToBackBuffer`
starting TRUE, the node cache and the uber-override stack, `PushUberOverride` 0x44ca10).

* **built** in `FViewInfo::FViewInfo` (0x46b3a0), gated on `SHOW_PostProcess` where the reference chain is gated:
  reuse `InView->m_PostProcessProxy` when a scene capture handed one down, else (in the editor only) validate the
  graph with `IsValid`, then `FArkPpCreateProxyConfig`, `PushUberOverride(m_ArkPpConfig->m_UberPpParameters, 1.0)`,
  `mbOnlyInEditor = ShowFlags & SHOW_Editor`, `m_GraphRoot->CreateSceneProxy(config)` and `AddPostProcessProxy`.
  `FSceneView::m_PostProcessProxy` is the 2012 PDB's member at @36 (`m_ArkPpConfig` @28 is agent BD's,
  `SceneReflectionTexture` @32 is still agent AG's open hand-over); the view destructor releases it.
* **rendered** from `FSceneRenderer::RenderPostProcessEffects` (0x448990): `Views(0).m_PostProcessProxy->Render` at
  `SDPG_PostProcess` when a back buffer exists, between `RHISetShaderRegisterAllocation(24,104)` and `(64,64)`, with
  `FArkPpRenderConfig(TRUE)`. The reference effect proxies are still reported once and skipped.
* each node's proxy renders its inputs with `m_bForceToDestination` **cleared** and draws into its own surface; only
  the node that still holds the bit draws into the back buffer. `m_bDone` makes a shared node draw once; the whole
  tree is rebuilt with the `FViewInfo` of every frame, which is why nothing resets it.
* the switch node has no proxy of its own (0x509970): it returns the selected branch's, and a node that is not shown
  returns its surface target's, so hiding a node splices it out of the graph instead of breaking the chain.

## 3. The material node (package item 1: "the one to port first")

`FArkPpNodeMaterialProxy` (48 bytes, ctor 0x518d00, `Render` 0x51e720) with `TPpMaterialDrawingPolicy`
(0x51a690 / `DrawShared` 0x51ad50 / `SetMeshRenderState` 0x519cf0 / `CreateBoundShaderState` 0x519c20),
`TPpMaterialDrawingPolicyFactory::DrawDynamicMesh` (0x51cd70) and `TPpMaterialPixelShader::SetParameters`
(0x519e10). The shader types are agent BD's and load byte-exactly; what is new is everything that binds them:

* the colour write mask is `CW_RGB` when the node preserves alpha and `CW_RGBA` otherwise (retail computes it as
  `~(8 * m_bPreserveAlphaChannel) & 8 | 7`), depth is off, the blend state is opaque.
* the eight node textures are bound with **nine** static sampler states indexed `m_TileU + 3 * m_TileV`
  (wrap/clamp/mirror in each axis); retail binds `SF_Bilinear` whatever the input's `m_FilterType` says.
* the pixel shader gets the material's parameters, the view's `ScreenPositionScaleBias`, and the surface and screen
  resolutions as `(size, 1/size)`.
* the tile is drawn through **`FTileRenderer::DrawTile<FactoryType>(View, MaterialRenderProxy, Context)`**
  (0x51e510, `PrepareShaders` 0x51d9c0) — the template retail added to `tilerendering.h` beside the reference
  DrawTile, hard-wired as that one is to the base-pass and translucency factories. It is the reference full-view
  path with the identity view-projection detour, and it neither reads the material's blend mode (retail fetches it
  and drops the result) nor sets a blend state, because a node pass owns its own.
* one deviation, documented at the site: retail looks the three shaders up with `FMaterial::GetShader`, which
  `appErrorf`s when a material's cooked map has none. A post-process material cooked without the pp-material types
  would take the game down mid-frame, so they are looked up without the assert and the pass reports and skips.

## 4. What the content's graph actually does — and where the colour treatment really is

This is the finding that matters most for the next package, and it is not what the plan assumed.

`DefaultPostProcessName=AltScreen_Effects.PostProcessChain.Test_PPG` in `DishonoredEngine.ini` is retail's own
default and `L_Tower_P` does not override it, so this graph is what retail runs here. `-arkppdbg` dumps it
(42 nodes, `build\agentCE\nodedump.txt`); the shape of it is:

```
node  2 ArkPpNodeSceneColor  ArkPpNodeSceneColor_0: game 1, controller none, proxy yes
node  1 ArkPpNodeDof         ArkPpNodeDof_1:        game 1, controller none, proxy yes
node  6 ArkPpNodeAA          ArkPpNodeAA_0:         game 1, controller none, proxy yes
node 38 ArkPpNodeMaterial    ArkPpNodeMaterial_15:  game 0, controller none, material PPG_LensCompose   <- the root
node  3 ArkPpNodeMaterial    ArkPpNodeMaterial_9:   game 1, controller DisDarkVisionPpController, material TEST_PPG_EyeLid
node  0 ArkPpNodeMaterial    ArkPpNodeMaterial_3:   game 0, controller DisBlindedPpController,   material TEST_PPG_Blinded_INST
node  4 ArkPpNodeMaterial    ArkPpNodeMaterial_5:   game 0, controller DisOpacityParameterPpController, material TEST_PPG_Lens
...  PPG_BlinkVectors_*, PPG_AdrenalineVectors_INST, PPG_PossessionVectors, PPG_KOVectors, POSSESSION_IN/OUT_INST
```

* **the main line is `SceneColor -> Dof -> AA -> (LensCompose, not shown in game) -> destination`.** The twenty
  material nodes are the *effect* overlays — blink and bend-time vector fields, adrenaline, possession, knock-out,
  the lens, the eyelid, blinded — every one of them either not shown in game (`game 0`), behind a switch whose
  selection is 0, or driven by a controller. Three proxies are built at rest: scene colour, depth of field, AA.
* **so the colour treatment of this content is the depth-of-field node's uber pass, not a material node.**
  `FArkPpNodeDofProxy::Render` (2013 rva 0x523170, 2012 0x563f40) runs `LutCreation` (0x522750, 2012 0x563560) and `Blend`
  (0x522990, 2012 0x5637a0) on every frame and `Downsample` (0x522210, 2012 0x563050) only when `m_DOFParameters.m_FarBlurAmount > 0`;
  `LutCreation` draws one triangle into a **256x16 A8R8G8B8 2D render target** (`FArkDofRamp<16>`,
  `InitDynamicRHI` 0x514bc0, 2012 0x555220 — a 16x16x16 colour lookup table unwrapped, *not* a volume texture, which is what
  makes it portable to our D3D9 RHI) with `FArkPpDofLutBlenderPS::SetParameters` (0x50ed00, 2012 0x54f440) writing
  `m_CBParameters`' three tone vectors, the overlay colour, the brightness/contrast pair, the exposure
  (`pow(2, m_Exposure)`), the gamma adjustment over the display gamma and the pre/post desaturation. `Blend` then
  draws the uber pass with `TArkPpDofUberPS<a,b>` sampling scene colour, scene depth, the low-resolution colour and
  that ramp. **That is Dishonored's colour balance, exposure and gamma, and it is the one pass of the graph's main
  line that this package does not port** (section 6, hand-over 1).
* the chain carries two named switches (`bUnderWater = 0`, `bBendTime = 0`) while all twelve switch *nodes* have an
  empty `m_Switch` name and `m_bFromPostProcessChain = 0`, so each node's own `m_Selection` decides - which is what
  `UArkPpNodeSwitch::CreateSceneProxy` does, and there is no other switch code in the exe to port (the only
  `UPostProcessChain` method that is not a class registrant is `CreateMutableMaterialInstanceOnNodes`).
  `buildgentCE
odedump.txt` is the whole dump.

### The controllers decide whether a node draws at all

The first build of the graph drew `TEST_PPG_EyeLid` over the whole frame — a closed eyelid, i.e. a black screen
(mean 261.83 of 765, 99.2 % of pixels). That was not a binding or a shader defect: the pass was working. It was the
base `UArkPpNodeController::IsShown` answering TRUE. The four controller classes are DishonoredGame's and unported,
and the retail bodies say what they answer:

```
UDisOpacityParameterPpController::IsShown  = state bits set || m_CurrentTime > 0                   (2013 rva 0x7e7ca0)
UDisDarkVisionPpController::IsShown        = state bits set || m_EyeLidTime > 0 || m_PowerTime > 0  (2013 rva 0x7e7c30)
```

A controller at rest hides its node, and the graph then falls through to that node's surface target. The unported
base therefore answers what every one of them answers at rest — FALSE — and `-arkppcontrollersshown` answers what
they answer while the effect runs, which is how the material pass is measured on the content's own nodes
(the pair in the result table). Drawing a controller-driven node with its default material is worse than not
drawing it: the eyelid material's default is closed.

## 5. Three things found on the way

1. **Retail never initialises `FArkPpNodeMaterialProxy::m_GoToLR`** (offset 16). Both constructors store every
   other member and leave that one as `appMalloc` handed it over (2013 0x518d00 `this[1..3,5..11]`, 2012 0x559670
   the same), and `Render` does `View.bUseLDRSceneColor |= m_GoToLR`. The bit matters:
   `FSceneRenderer::FinishRenderViewTarget` (0x45c500) copies scene colour over the view's render target **unless**
   it is set, and that render target is the very back buffer the last node of the graph draws into — so with the bit
   clear the graph's output is overwritten by the un-post-processed scene. The depth-of-field node sets it
   unconditionally (`View->bUseLDRSceneColor |= 1` at the end of its Render), which is how retail's real chains end
   up correct; the material node's copy of the idea reads garbage. Ours sets it from the surface the node actually
   drew into, which is what retail gets in practice, and says so at the site.
2. **`TArray::Add` does not construct**, and `FArkPpNodeMaterialProxy::FAnEntry` holds a `TRefCountPtr`. The first
   build of this package crashed in the graph build with a garbage callstack; assigning to a `TRefCountPtr` over
   uninitialised memory releases a garbage pointer. `AddZeroed` is the fix, and the crash is the reason the
   `.map`-based `resolve.py` exists.
3. Agent CA is right that the scene-colour binding is not broken. A red `RHIClear` immediately before the material
   tile survives only where the tile does not cover it (`build\agentCE\dbg_clear.bmp`, mean red 0.6 of 255 over the
   frame): the pass binds the surface the frame is presented from, exactly as retail's sequence says it should.
   `-arkppclear` is that measurement, kept for the next agent.

## 6. Not ported, and exactly what each one needs

* **the depth-of-field uber pass** — hand-over 1, and the one that carries the look (section 4). Needs
  `FArkDofRamp<16>` (256x16 `PF_A8R8G8B8`, `RHICreateTexture2D` + `RHICreateTargetableSurface`),
  `FArkPpDofLutBlenderPS::SetParameters` (0x50ed00), `FArkPpDofUberVS::SetParameters` (0x50eb50:
  `(1/SizeX, 1/SizeY)`, the viewport scale/bias as `(w/SizeX, h/SizeY, MinX/SizeX, MinY/SizeY)` and a noise pair of
  two random 0..127 indices that never repeat the previous frame's), `TArkPpDofUberPS::SetParameters` (0x514d40, 2012 0x5553a0) and
  `FArkPpDofDownsample{VS,PS}::SetParameters` (0x50eab0, 2012 0x54f1f0). Every shader type loads today and both half/quarter
  targets exist. The node's proxy, its uber-parameter resolution against the override stack and its place in the
  graph are ported; only the four passes are missing, and the proxy currently renders its input with the config
  unchanged so that the image still reaches the destination.
* **the AA passes** (`FArkPpNodeAAProxy::RenderFxaa` 2013 rva 0x5203c0, `RenderMlaa` 0x521720 and its three
  passes 2012 0x561570 / 0x561990 / 0x561ee0): ten shader types load; the MLAA edge mask and edge count targets exist in the
  tree's render-target enum. Same shape of stub as the depth-of-field node.
* **the blur and Kuwahara passes** (`RenderMotionBlur` 2013 rva 0x521990, `RenderKuwa` 0x523350) with
  `FArkPpBlurParameters` (176 bytes) and `FArkPpKuwaParameters` (112).
* **`FSceneRenderer::RenderBloomParts`** (0x5251a0, package item 2): unchanged from agent BD's hand-over except that
  the relevance is now costed. It needs `FPrimitiveViewRelevance::bBloomPartRelevance` (2012 PDB bit 15 of the
  relevance word) fed from `FMaterialViewRelevance::bBloomPart` (bit 4), which comes from
  `UMaterialInterface::bHasBloomPart` — **already computed in this tree** (`Material.cpp:1325`,
  `MaterialInstance.cpp:1520`); then `FViewInfo::BloomPartPrimSet[4]` (`FArkBloomPartPrimSet` is 12 bytes,
  `AddScenePrimitive` 2012 rva 0x54e6c0 - the match table has no 2013 address for it - and `DrawBloomPrims` 2013 rva 0x524f00) filled where `DistortionPrimSet` is filled, and
  `RenderBloomParts` itself (694 lines of decompile) with `bloom::GaussianBlur` (2013 rva 0x51fe50). The shaders, the two
  quarter-size targets, `BeginRenderingBloom`/`FinishRenderingBloom` and `m_BloomNeedBlit` are all in place from
  package BD. This package did not get to it: the graph and the material pass were the whole of the day.
* **`UPostProcessChain::CreateMutableMaterialInstanceOnNodes`** (2013 0x2d2a40) and the two DishonoredGame writers
  of `FArkPpConfig` (`ADishonoredPlayerCamera::SetPostProcessTarget`, `UDisPostProcessManager::ApplyKismetPostProcessSettings`)
  are still unported, so the uber parameters the graph reads are the class defaults.

## 7. Numbers

| Measure | HEAD `8e61755` | With package CE |
|---|---:|---:|
| ArkPp node classes declared by the engine | 0 (12 in DishonoredGame's shim) | **12** |
| graph nodes reaching a proxy | 0 | **3 of 42** at rest (scene colour, depth of field, AA), **5** with the controller-driven nodes shown |
| FArkPp passes per frame | 0 | **1 material tile** with the controllers shown, 0 at rest |
| pixels changed by the graph (same camera) | - | **99.2 %**, mean 261.83 of 765 |
| camera noise floor (two identical runs) | - | 0.09 % of pixels, mean 0.01 of 765 |
| scene frames in the 90 s run | 321 | 321 |
| `Initial startup` | 3.3 s | 3.33 s |
| regression | 31 checks | **31 ok, 0 failed** |

## 8. Files

Mine (`build\agentCE\files.txt`, 18): `Engine/Inc/arkpp.h` (new content), `Engine/Inc/enginearkppclasses.h` (new
content), `Engine/Inc/Engine.h` (one include), `Engine/Inc/Scene.h` (`FSceneView::m_PostProcessProxy`),
`Engine/Inc/TileRendering.h` (the templated DrawTile/PrepareShaders), `Engine/Src/arkppnodes.cpp`,
`Engine/Src/arkppnodematerial.cpp`, `Engine/Src/arkppnode{blur,dof,kuwa,aa}.cpp`, `Engine/Src/Scene.cpp` (the member
initialiser), `Engine/Src/SceneRendering.{h,cpp}` (the graph build, the render call site, the census),
`Engine/Src/UnEngine.cpp` (the registrant), and the three generated DishonoredGame files the move empties:
`DishonoredGameEngineShims.h`, `DishonoredGameLayouts.h`, `DishonoredGameRegistrants.cpp`.

`SceneRendering.cpp` was shared with agent CA. CA merged first (`c403e2f`), and after that merge this file's diff
against HEAD is only the graph work above: CA's own changes to it, including the two switches it made function-local,
are already committed, so nothing of CA's rides along with this package after all.

The generated files lose 12 classes, 6 structs, 4 enums, 73 layout asserts, 12 `IMPLEMENT_CLASS` lines, 12
registrant lines and the forward declarations. `gen_classes_header.py` skips a shim class, struct or enum the tree
declares (`sdk_select`: `if s in items or s in d.declared: continue`), so regenerating DishonoredGame produces the
same file; as with agent BD's move the shared tree does not compile until the coordinator regenerates or keeps
these three edits.

## 9. Bring-up switches (all function-local statics, per agent CA's finding)

| Switch | What it does |
|---|---|
| `-noarkpp` | leaves the graph out of `RenderPostProcessEffects`; the switch of the pair |
| `-arkppdbg` | one line per graph node and per chain switch at the first build, and the first eight material passes |
| `-arkppclear` | clears the destination to red just before the material tile (the binding measurement) |
| `-arkppcontrollersshown` | the unported controller base answers what the real controllers answer while their effect runs |

## 10. Hand-overs

1. **The depth-of-field uber pass is the look** (section 4 and 6). Whoever takes it gets a contained job: the node,
   its proxy, its parameters and its place in the graph are ported, the eight shader types load, the LUT target is a
   plain 256x16 2D render target, and the three `SetParameters` are decompiled in `build\agentCE\decomp12`.
2. **Bloom parts** (section 6): the relevance path is now costed end to end and `bHasBloomPart` already exists.
3. **The four post-process controllers** (`UDisBlindedPpController`, `UDisOpacityParameterPpController`,
   `UDisDarkVisionPpController`, `UDisDarkVisionMeshRenderPpController`, DishonoredGame): until they exist, every
   node they drive is hidden, which is what they report at rest. `UDisBlindedPpController::Update` (2013 rva 0x7ea620)
   builds a material instance at runtime, so `UPostProcessChain::CreateMutableMaterialInstanceOnNodes` belongs with
   them.
4. **Bring-up lines to drop when the graph is finished**: the four switches above, the per-node dump, and the
   material-pass report.
