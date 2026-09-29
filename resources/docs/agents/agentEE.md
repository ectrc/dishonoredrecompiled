# Agent EE report — the rendering passes that were named but not ported (2026-09-29)

Package **EE** of `PHASE11.md` (wave 10), the continuation of agents CE (`dec3fd4`), DA (`fdc6aec`), DB (`b66b8bd`) and
DE (`9b77df6`). Written against **HEAD `c598a21`**. Build dir `build\agentEE` (Release, x86, Ninja, every module
option) from the snapshot worktree `build\agentEE_wt` (`resources\tools\make_snapshot.py EE --list
build\agentEE\snapshot_files.txt`, build command `build\agentEE_release_build.cmd`). IDA copies
`resources\docs\idb\{retail2013,shipping2012}_agentEE.i64`, headless `decompile_funcs.py` plus three one-off scripts
(`dump_asm.py`, `xref.py`, `findimm.py`); decompiles in `build\agentEE\decomp13`, `decomp12`, `decomp12b`, `decomp12c`.
No commits, nothing staged. Status rows: `agentEE_status.csv` (28). HEAD advanced to `6723640` (agent ED) while
this package ran; `git diff --name-only c598a21 HEAD` touches **none** of this package's fourteen files, so the
merge is clean and nothing here was copied over a newer version of a shared file.

```
rem accept 1: the pairs, one binary, one switch apart, on the map two runs of which are bit-identical
build\agentEE\pairs.cmd
rem accept 1 again, warm: the first run of a batch is an outlier (section 3)
build\agentEE\pairs2.cmd
rem accept 1 continued: where the floor comes from, and the closed Gaussian deviation
build\agentEE\pairs3.cmd
rem accept 1 continued: the Gaussian pair needs a map whose far blur reaches the frame
build\agentEE\pairs4.cmd
rem accept 2: the census line
python build\agentEE\run_pair.py census --map L_Pub_Day_P --time 6 -arkppaadbg
rem accept 3
python resources\tools\run_regression.py --build-dir build/agentEE --no-build
```

## Result

| Accept | State |
|---|---|
| FXAA ported | **done**: `RenderFxaa` (2013 rva 0x5203c0) with `FFXAAVertexShader::SetParameters` (0x50e090) and `TFXAAPixelShader<luma,1,0>::SetParameters` (0x518480), through the two `FlushShader` templates (2012 0x560170 ff.) |
| MLAA ported | **done**: `RenderMlaa` (0x521720) and its three passes — edge detection into `MLAAEdgeMask` (0x520880), line length into `MLAAEdgeCount` (0x520ca0), blend into the destination (0x5211f0) — with `FMLAAVertexShader::SetParameters` (0x50e170) and the four edge/blend pixel-shader setters |
| Kuwahara ported | **done**: `RenderKuwa` (0x523350) and `FArkPpNodeKuwaProxy::Render` (0x5238c0) with both `TKuwa*Shader<3\|5>` setters (0x5155e0 / 0x50dab0). **Unmeasurable on this content and said so**: neither Kuwahara node of the shipped chain is reachable from its root, so the pass draws 0 times (section 5) |
| motion blur | **not ported, deliberately**, and the structure is decoded instead (section 6). The chain's only blur node is `m_bShowInGame 0`, so no run of this content can exercise it; porting 2,163 bytes of code that cannot be measured is how latent defects get into this tree |
| the `GaussianBlurFilterBuffer` deviation closed | **done and measured**: retail's arithmetic is `Min(KernelRadius / FilterDownsampleFactor, 16) * (SizeX * FilterDownsampleFactor / 1280)` and its sample mask is `(0,0)..(1,1)`. Agent DA passed the node's own width where retail derives `SizeX * FilterDownsampleFactor`, which made the far blur **four times too narrow**, and the no-clamping mask. Both closed; the `Min(·,16)` clamp added to `GaussianBlurFilterBuffer` itself |
| the soul-part pass | **the Engine half is in, the pass is not, and DB's hand-over was wrong**: `FViewInfo::m_VisibleSoulPrimitives` did **not** exist and nothing filled it. The member (`@1460`), the relevance bit (23) and `ProcessVisible`'s fill are ported; the pass itself is `UDisDarkVisionMeshRenderPpController::Render` (0x7fe690) in **DishonoredGame**, not Engine, and every producer of the bit is outside this package (section 5) |
| `RenderFogMaskStencil` | **not ported, and DB's defect 4 is answered instead**: the pass writes **stencil only** (`RHISetColorWriteEnable(FALSE)`), it does not fill the fog-mask render target, and the cooked `DisFog` pixel shaders **do not bind `MaskTexture` at all** — measured, `MaskTexture bound 0` on every census line. Its only input is the audio cell/portal graph, which is blocked on `UDishonoredAudioSystem::GetCellAtPoint` (0x792d10, DishonoredGame). Section 4 |
| a pair per pass, quantified | **done**: section 3 |
| a census line naming which passes ran, their draws, and how many are stubs | **done**: `FArkPp 1 nodes rendered, 7 draws (0 material tiles, dof 4, aa 3, blur 0, kuwa 0), 0 passes not ported` — `passes not ported` was 1 at HEAD and is **0** now |
| regression harness green | **31 ok, 0 failed, 0 skipped**, 423 s, own `--build-dir` (`build/agentEE/regression1.txt`): `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `coresmoke_passed 99`, `d3d9_frames 2460`, `d3d9_criticals 0`, `unported_natives 0`, `inputtest_moved 1023.2`, `inputtest_criticals 0`. The d3d9 and inputtest stages run `L_Tower_P`, so MLAA and the corrected far blur are clean on the mission map too |
| build | incremental **714 steps, 0 errors**; a clean full release build of the snapshot into a fresh directory is **971 steps, 0 errors** for all three targets |

## 1. The finding that makes the package worth more than its passes

**MLAA is what the shipped post-process chain asks for, and its two render targets were never created.**

`-arkppaadbg` reports the node's resolved configuration once:

```
dishonored aa node: type 1 (MLAA), luma source 2, lum equation (0.300 0.590 0.110),
                    mlaa threshold 12.0000, iType_AntiAlias 1, destination back buffer
```

`m_Type 1` is MLAA, and the AA node is the last node of the chain's main line
(`SceneColor -> Dof -> AA -> destination`, agent CE section 4), so it owns the back buffer. That is the pass this
content renders through, every frame.

`FSceneRenderTargets::InitDynamicRHI` created `MLAAEdgeMask` and `MLAAEdgeCount` behind
`if (GSystemSettings.bAllowPostprocessMLAA)`. That member is a **`DISHONORED_SHIM_STATIC`**
(`SystemSettings.h:408`) — retail 2013's `FSystemSettingsData` has no such key, it has `iType_AntiAlias @128` in its
place — so it read FALSE and **neither target existed**. Retail creates both unconditionally on PC, at the full buffer
size, `EPixelFormat` 22 (`PF_R16F`) for the mask and 16 (`PF_G16R16F`) for the count, with no MLAA-specific test
anywhere near them (`build\agentEE\decomp13\FSceneRenderTargets_InitDynamicRHI_451080.c` lines 752..788, the two
`RHICreateTargetableSurface` calls named `MLAAEdgeMaskRT` / `MLAAEdgeCountRT`).

The first build of this package therefore died on its first frame with a render-thread exception, and the bisection is
in the log: `-noarkppaa` ran 110 s clean and `-arkppaafxaa` ran 110 s clean, only the MLAA branch faulted. This is the
**sixteenth** defect of the shape agents DE and DP have been logging, and the first one found in `SceneRenderTargets`:
a member retail does not have, kept as a storage-less placeholder, answering zero and switching a whole pass off.
It is **not** in the printed ranks of `resources/docs/shim_audit.txt` — rank C has 350 entries and the file prints
only the top of each rank — so a scalar `UBOOL` shim that gates a render target is exactly the kind the audit's own
output hides. Worth a line in the audit's tooling: score a shim that is the sole condition of an `RHICreate*` call.

## 2. What each pass does

### FXAA (`RenderFxaa`, 0x5203c0)

One triangle. The destination is `GSceneRenderTargets.GetBackBuffer()` when this node ends the graph and the view is
not upscaled, and the node's own surface otherwise. The pass fills `FArkPpFxAaParameters` from the node's
`m_FxAaConfig` and the *input proxy's* texture and surface size, sets the viewport to the view's rectangle scaled into
that surface, hands the vertex shader `(1/SizeX, 1/SizeY)` and the rectangle's scale/bias, and picks the pixel shader
from `m_FxAaConfig.m_Luma`: 0 computes the luminance, 1 reads the green channel, **2 reads scene colour's alpha**,
which is what this content asks for. The quality preset is always 1 and the three `_ForSceneColor` variants are never
chosen, which is why they have no cooked shader. `RHISetColorWriteMask(CW_RGBA)` comes **after** the shaders and
before the draw, as retail has it.

The pixel shader's six tuning vectors are literals in retail, not content: `fxaaQualityParams` (0.75, 0.166, 0.0833),
`fxaaConsoleParams` (8, 0.125, 0.05, 0), `fxaaConsole360ConstDir` (1, -1, 0.25, -0.25), and three frame-option vectors
of ±0.5, ±2 and 8 / -4 texels **of the scene colour buffer**, not of the node's surface. The two exposure-biased scene
colour samplers in the layout are the Xbox 360 path and are never set.

### MLAA (`RenderMlaa`, 0x521720, three passes)

1. **edge detection** (0x520880) into the full-size `MLAAEdgeMask`, `CW_RED` only, from a **point-sampled** read of the
   node's input. `TMLAAEdgeDetectionPixelShader<bSRGB>` gets `1/BufferSize`, the luminance equation with
   `1 / m_EdgeDetectionThresold` in its W, and — for the linear variant only — the inverse display gamma.
2. **line length** (0x520ca0) into `MLAAEdgeCount`, `CW_RED | CW_GREEN`, reading the mask the first pass wrote.
   `FMLAAComputeLineLengthPixelShader` has no `SetParameters` of its own: the pass writes its two parameters directly,
   which is how retail has it.
3. **blend** (0x5211f0) into the node's destination, `CW_RGB` so scene colour's alpha — which is depth on this
   renderer — survives. `TMLAABlendPixelShader<bSRGB>` gets the same three constants plus `MLAAEdgeCount`
   point-clamped.

`bSRGB` is `View.bUseLDRSceneColor != 0` in all three, which the depth-of-field node sets earlier in the same frame,
so MLAA runs in its sRGB variants on this content. The technique sets depth `<FALSE,CF_Always>`, the solid two-sided
rasterizer and the opaque blend once for all three passes and restores `CW_RGBA` at the end.

**Retail 2013 removed MLAA's fallback.** The 2012 build opens `RenderMlaa` with
`if (!GSystemSettings.RenderThreadSettings.bAllowMLAA) return RenderFxaa(...)`; retail 2013 (0x521720) goes straight to
the three passes, because 2013 dropped `bAllowMLAA` from `FSystemSettingsData` for `iType_AntiAlias @128`, which picks
the node's type instead. Ported as retail 2013 has it — which also keeps the pass off `RenderThreadSettings`, a
structure nothing in this tree ever writes (it would have read FALSE and silently turned every MLAA run into an FXAA
run: the same trap as section 1, avoided by porting the right build).

### Kuwahara (`RenderKuwa`, 0x523350)

One triangle through `TKuwa{Vertex,Pixel}Shader<3>` when the node's `m_Type` is 1 and `<5>` otherwise. Two details are
retail's own and neither is what the names suggest: the source texture comes from the **input** proxy while the
viewport and the shader's size come from the **surface target** proxy, and the pass sets no depth, rasterizer or blend
state at all — it inherits whatever the node above it left set. The vertex shader gets
`(1/SizeX, 1/SizeY, m_Strength, 0)`; the pixel shader gets the source colour and nothing else.

## 3. The measurements

One binary; the only difference between the legs of a pair is one switch. The map is **`L_Pub_Day_P`**, which agent DB
established as the one whose runs repeat (the player stands still in the pub attic); the capture is
`-apshottime=6`, never `-apshot`. The window came out **1008x567** rather than 1280x720 for every run of this
package: the desktop was at 1024x768 while another agent held it, so the requested windowed size was clamped. Every
figure below is from that one window size, so the pairs are comparable; nothing is compared across sizes.

**Two measurement lessons came out of getting these figures, and both are worth more than the figures.**

*The first run of a batch is an outlier.* Staging copies a 23 MB exe and the OS page cache is cold, so texture
streaming lags: `pair_mlaa_a` reached world time 6 having served **0** stream requests where every later run had
served about **700**, and its frame therefore has different mip levels. It is discarded; `pairs2.cmd` opens with a
throwaway warm-up run.

*The floor is not zero, and the reason is the film grain.* The depth-of-field uber pass re-draws its noise indices
every frame (agent DA), and the index the captured frame lands on depends on how many frames the render thread got
through. Two runs of one switch are usually **byte-identical**, but sometimes the grain lands one step apart and the
frame then differs by **up to 4 per channel over about a fifth of the pixels**. So every pair is also quoted at a
**4-per-channel threshold**, which the grain cannot cross, and with the share of changed pixels in each bucket of the
reference frame's local luminance gradient: **antialiasing lives in the top bucket and dither is flat across all
four**, which is what separates a pass from noise without having to trust the machine.

### the noise floors — two runs, one switch, one binary

| floor | mean of 765 | > 2/channel | > 4/channel | max |
|---|---:|---:|---:|---:|
| two `-noarkppaa` runs (three of the four pairs taken) | **0.00** | **0.00 %** | 0.00 % | **0** |
| two `-noarkppaa` runs (the one pair whose grain landed a step apart) | 3.99 | 19.31 % | **0.00 %** | 12 |
| two MLAA runs | **0.00** | **0.00 %** | **0.00 %** | **0** |
| two FXAA runs | **0.00** | **0.00 %** | **0.00 %** | **0** |

Both antialiasing passes are byte-for-byte reproducible run to run. Neither reads anything uninitialised — which is
the check worth having, because the MLAA branch's first version did exactly that (section 1) and it showed up as a
render-thread fault rather than as noise.

### the passes

| pair (one binary, one switch) | mean of 765 | > 4/channel | max | flat | low | mid | **edge** |
|---|---:|---:|---:|---:|---:|---:|---:|
| **MLAA** vs `-noarkppaa`, leg a | 4.21 | **0.81 %** | 377 | 0.1 % | 0.1 % | 1.9 % | **44.4 %** |
| **MLAA** vs `-noarkppaa`, leg b | 0.27 | **0.79 %** | 376 | 0.0 % | 0.1 % | 1.7 % | **44.1 %** |
| **FXAA** vs `-noarkppaa`, leg a | 0.79 | **2.29 %** | 347 | 0.2 % | 0.4 % | 15.4 % | **66.9 %** |
| **FXAA** vs `-noarkppaa`, leg b | 4.49 | **2.41 %** | 345 | 0.2 % | 0.4 % | 16.3 % | **67.9 %** |
| FXAA against MLAA | 4.44 | 2.32 % | 345 | 0.2 % | 1.0 % | 16.7 % | 63.2 % |

The gradient columns are the result. **FXAA changes 67 % of the frame's high-gradient pixels and 0.2 % of its flat
ones; MLAA changes 44 % of the high-gradient pixels and 0.0 % of the flat ones.** That is antialiasing and nothing
else. The `> 4/channel` and gradient columns repeat across the two legs to within a tenth of a percent; the
mean-absolute column is the one the grain moves, and it is quoted only for continuity with the earlier packages'
numbers. MLAA touches a third as many pixels as FXAA because it rewrites only what lies next to a separating line it
found, while FXAA's filter reaches further; the two techniques disagree over 2.3 % of the frame and 63 % of its edges,
so they are genuinely two different techniques and not one pass wired twice. `diff_mlaa.png`, `diff_fxaa.png` and
`diff_fxaa_vs_mlaa.png` are the difference figures, amplified eight times.

### the closed Gaussian deviation

`-arkppdofgaussold` restores the arguments the call had before this package, so the correction is one switch of one
binary. It needed a second map. On `L_Pub_Day_P` the depth-of-field node's `Downsample` runs every frame — the census
counts its two draws — and yet the pair is **byte-identical whatever the kernel radius is**, because the uber pass
weights the blurred low-resolution colour by a depth term and nothing in a pub attic is past the in-focus radius. So
the pair is taken on **`L_Tower_P`**, the map whose grade agent DE measured (focus 35000, in-focus radius 1500, far
blur 0.300) and which opens on open water with distant geometry.

| pair on `L_Tower_P` | mean of 765 | > 4/channel | max | **mean signed** | flat | low | mid | edge |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| floor, two `-arkppdofgaussold` runs | **0.00** | **0.00 %** | **0** | +0.000 | 0.0 % | 0.0 % | 0.0 % | 0.0 % |
| floor, two default runs | 0.12 | 0.15 % | 419 | — | 0.1 % | 0.1 % | 0.2 % | 0.4 % |
| **the correction, leg a** | 0.70 | **1.16 %** | 241 | **-0.376** | 1.3 % | 1.2 % | 0.7 % | 1.6 % |
| **the correction, leg b** | 0.80 | **1.34 %** | 419 | **-0.334** | 1.5 % | 1.3 % | 1.0 % | 2.1 % |

The correction changes **1.2–1.3 %** of the frame beyond 4 per channel against a worst floor of 0.15 %, and the
**signed** mean is the figure that settles it: **-0.35 of 765 in both legs**, the same sign and nearly the same size,
which run-to-run noise cannot produce (this is agent DL's rule, and `L_Tower_P` is exactly the animating scene it was
written for). The gradient columns are also the right shape: unlike the antialiasing passes, a wider Gaussian changes
every bucket by about the same amount, because it is a blur and not an edge filter. `diff_gauss_tower.png` is the
figure.

### what buffer the graph actually works in

`-arkppaadbg` also reports it, because two earlier theories about the floor turned on it:

```
dishonored aa input: 1008x568, texture is SceneColorLDR, surface is SceneColorLDR
```

The chain's scene-colour node has `m_LowRange` set, so every node of the graph reads and writes
`GSceneRenderTargets`' **LDR scene colour** (`LightAttenuation0`), never scene colour itself. That is what
`FViewInfo::bUseLDRSceneColor` refers to and why the depth-of-field node sets it; it also means the depth-of-field uber
pass, which samples `GetSceneColorTexture()` for both its colour and its depth (agent DA's defect 4), is **not**
sampling the surface it writes. There is no read/write hazard in the chain.

One caveat for the next agent, because it cost time here: **`-noarkppdof` switches off more than the depth-of-field
node.** Its early return is taken before `View.bUseLDRSceneColor |= 1`, so `FSceneRenderer::FinishRenderViewTarget`
copies the un-post-processed scene over the view's render target afterwards and *everything* the graph drew is
discarded — the antialiasing passes included. Any pair taken with that switch measures the plain scene twice — which is what `pairs3.cmd`'s four `-noarkppdof`
frames are: all four byte-identical, at a frame luma of 17.58 against the graded 23.98, whether MLAA ran or not.

### Kuwahara, and why it has no pair

`-arkppdbg` reports both Kuwahara nodes of the shipped chain as `game 1 editor 1 ... proxy no` and the blur node as
`game 0 editor 0 ... proxy no`: neither is reached when the graph is walked from its root, so the census reads
`kuwa 0` and `blur 0` on every frame of every run. The Kuwahara pass is ported and binds its four cooked shader types,
and **this package cannot show that it changes a frame on this content, so it does not claim that it does.** Whoever
gives it a reachable node gets the pair; `-noarkppkuwa` is already the switch for it.

## 4. `RenderFogMaskStencil`, and what agent DB's defect 4 really is

Three things, in the order they matter.

**1. The cooked `DisFog` pixel shaders do not bind `MaskTexture`.** `GDisCensusFogMaskTextureBound` reports
`mMaskTextureParameter.IsBound()` on every census line and it is **0** in every run of this package, on the map with
three fog layers drawn in two passes. So there is no missing producer and no stale sampler: `MaskTexture` is a dead
name in the shader source that the cook never allocated a register for, which is exactly what retail's own
`FDisFogPixelShader::SetParameters` (0x41c940) implies by not setting it. Agent DB's defect 4 is closed as **not a
defect**, and its hand-over 1 rests on a premise that does not hold.

**2. `RenderFogMaskStencil` writes stencil, not the fog-mask target.** The pass (0x433f80, 9,806 bytes) clears stencil
to `(listener cell's interior flag == Type)`, sets
`TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,FALSE,...,0xff,0xff,Ref>` with `Ref` 0 or 1 and
**`RHISetColorWriteEnable(FALSE)`**, and draws the portal quads. No colour is written anywhere in it. Its two
`RHIClear` calls clear stencil only. So interior fog covers the cell and is punched out at each portal, or the other
way round outside it — and the fog-mask render target that agent DB's bloom-part pass writes every frame has nothing
to do with it.

**3. It is blocked on the audio cell graph, not on rendering.** Retail's only input is
`GWorld->m_pAudioSystem->GetCellAtPoint(View.ViewOrigin, GetCurrentCachedCell())`, whose result is an `AInOutVolume`
whose portal list (`AGenericPortal`, `m_Corners` / `m_PlaneNormal` / `m_fHalfWidth` / `m_fHalfHeight` /
`m_fPortalMaskBias` / `m_fPortalMaskFadeDistance`, all already in `DishonoredGameEngineShims.h` with their offsets
asserted) is clipped against the view frustum with `FPoly` + `FConvexVolume::ClipPolygon` + `AVolume::Encompasses` and
drawn as `FSimpleElementVertex` triangles. `UDishonoredAudioSystem::GetCellAtPoint` (2013 rva 0x792d10, 2012 0x7f8a50)
is **not ported** — `dishonoredaudiosystem.cpp` has `Init`, `SuspendUpdate`, `ResumeUpdate` and `ApplyGameSettings`
and nothing else — and it is DishonoredGame's file, not this package's.

**Porting the pass now would change the frame for the wrong reason.** With no cell, retail keeps its default interior
flag of 1: on the *interior* fog pass (`Type == 0`) that clears stencil to 0 and the interior layers draw nowhere,
while on the exterior pass they cover everything. `L_Pub_Day_P` draws three layers in two passes, so the visible
change would be large, and all of it would be an artefact of the missing cell lookup rather than the pass. The seam in
`FSceneRenderer::RenderFogPass` (`const UBOOL bHasMask = FALSE;`) now carries all of this at the site.

## 5. The soul-part pass: what agent DB's hand-over 3 got wrong, and what is actually missing

DB's hand-over reads *"relevance bit 23, `m_VisibleSoulPrimitives @1460` already filled, shader types already declared
— only the pass is missing"*. Two of the three clauses are wrong.

* **`FViewInfo::m_VisibleSoulPrimitives` did not exist** anywhere in the tree (the name appears only in a comment in
  `arkbloompartsrendering.cpp`), and neither did `FPrimitiveViewRelevance::iSoulRenderingRelevance`, so nothing could
  have filled it. Both are ported here: the member at `@1460` as one
  `TArray<FPrimitiveSceneInfo*,SceneRenderingAllocator>`, the bit immediately after `bSoftMaskedRelevance` where
  retail has it, and `ProcessVisible`'s fill — one flat list with **no DPG split**, unlike the bloom-part sets
  (`0x45f060`, `mRaw & 0x800000`).
* **The pass is not in Engine.** The only consumer of `TSoulPartMeshDrawingPolicy<FSoulPartMeshPolicy>` in the whole
  executable is `UDisDarkVisionMeshRenderPpController::Render` (2013 rva 0x7fe690, 2012 0x85f480), in
  **DishonoredGame**, with its policy (0x7fd8d0), `SetMeshRenderState` (0x7fd940), `DrawShared` (0x7fda40),
  `DrawDynamicMesh` (0x7fdb70) and `TDynamicPrimitiveDrawer` (0x7fe600) all in the same 0x7fd..0x7fe range. Agent DE
  already established that **no node of the shipped chain carries that controller**.
* **Nothing produces the relevance bit either.** Retail sets it from the scene proxy's own soul material
  (`FStaticMeshSceneProxy::m_SoulMaterial` and the skeletal proxy's equivalent, `types/all_types.h`), fed by
  `UMeshComponent::SetSoulMaterial` (0x12b270), `USkeletalMeshComponent::SetSoulMaterial` (0x314610) and
  `UStaticMeshComponent::SetSoulMaterial` (0x35afe0), and the only caller in the cook is
  `UDisActivePowerComponent_DarkVision::AddUniqueSoul` (0x7ecb30) — DishonoredGame again.

So the set reports **0 relevant, 0 prims, 0 draws** on every census line and will keep doing so until dark vision
exists. That is said in the census, at both sites, and here; it is not claimed to work. Only the two shader types
(`ArkSoulPartVertexShader` / `ArkSoulPartPixelShader`, agent DB's) were genuinely already in place.

Only the two `TKuwa` node instances and the blur node share the Kuwahara/blur fate: `-arkppdbg` reports
`ArkPpNodeKuwa_1` and `ArkPpNodeKuwa_2` as `game 1 editor 1 ... proxy no` and `ArkPpNodeBlur_0` as `game 0 editor 0
... proxy no`, i.e. neither is reached from the chain's root when the graph is walked. The Kuwahara pass is ported and
compiles against its four cooked shader types, and it draws zero times on this content; **it is not shown to change a
frame and this report does not claim it does**.

## 6. Motion blur: decoded, not ported

`FArkPpNodeBlurProxy::RenderMotionBlur` (2013 rva 0x521990, 2012 0x5626c0, 2,163 bytes) is the one pass of this
package left out, and the reason is the rule this wave is measured by: the shipped chain's only blur node is
`m_bShowInGame 0`, so no run of this content can exercise it, and 2,163 bytes of untestable new code in the render
thread is how the last fifteen defects got in. What it does, in full, so the next agent has a spec rather than a
decompile:

* `m_VectorFieldProxy->Render(Scene, View, Config & ~1)` once, before the loop.
* then `mMotionBlur.m_PassCount` iterations of: destination = the back buffer on the **last** iteration when
  `Config.m_bForceToDestination` and `!NeedsUpscale()`, the node's own surface otherwise; source texture = the **input**
  proxy's on iteration 0 and **this** proxy's own (`GetTexture`) afterwards, bilinear clamped; vector field texture =
  `m_VectorFieldProxy->GetTexture(View)`, bilinear clamped; the vector-field scale pair is
  `(cfg@32 * IterativeScale, bit0 ? (first ? 1 : 0) : cfg@36 * (first ? 1 : 0))`, where `cfg@32` / `cfg@36` are the two
  floats of `FMotionBlurConfig` at proxy offsets 32 and 36, `bit0` is the flag at proxy offset 44, `m_PassCount` is at
  40, and `IterativeScale` starts at 1.0 and is multiplied by **0.25** after each iteration; the flag itself is also
  passed to the pixel shader; the viewport is the full buffer size on the
  back-buffer iteration and this proxy's surface size otherwise; depth `<FALSE,CF_Always>`, `CW_RGBA`, the opaque blend
  state; `TArkPpBlurVertexShader<MotionBlurPolicy>` always, and the pixel shader is a **three-way** choice —
  `<MotionBlurPolicy>` when the node's flag bit 0 is clear, else `<MotionBlur2NoOffsPolicy>` on iterations after the
  first and `<MotionBlur2Policy>` on the first; one triangle; resolve.
* the eight blur shader types are declared and load (`arkppnodeblur.cpp`), `FArkPpBlurParameters` is 176 bytes in the
  2012 PDB, and the two setters are 2012 0x554e90 / 0x554f70.

Whoever takes it should take a map that uses it, or add a bring-up switch that forces the node visible (agent CE's
`-arkppcontrollersshown` is the shape), so that it can be measured rather than merely compiled.

## 7. Numbers

| Measure | HEAD `c598a21` | With package EE |
|---|---:|---:|
| filter-node passes the shipped chain asks for and gets | **0 of 1** | **1 of 1** (MLAA) |
| `passes not ported` in the post-process census | 1 | **0** |
| FArkPp draws per frame (`L_Pub_Day_P`) | 4 | **7** (dof 4 + aa 3) |
| MLAA edge mask / edge count render targets created | **0 of 2** | **2 of 2** |
| far-blur kernel radius the depth-of-field node gets, at a 1280-wide view | 1.75 | **7.0** (retail's) |
| the same at the 1008-wide window these runs used | 1.38 | **5.51** |
| `MaskTexture` samplers bound in the cooked DisFog shaders | unknown | **0**, measured |
| `FViewInfo::m_VisibleSoulPrimitives` | absent | present, **0 prims** (no producer, section 5) |
| pixels the MLAA pass changes above the grain floor | - | **0.79–0.81 %**, **44 %** of edge pixels |
| pixels the FXAA pass changes above the grain floor | - | **2.29–2.41 %**, **67 %** of edge pixels |
| noise floor, two identical runs of either pass | - | **byte-identical** |
| `Initial startup` (d3d9, `L_Pub_Day_P` as the first map) | 3.95 s | 3.86 s |
| criticals in a 110 s run | 0 | 0 |
| regression | 31 checks | **31 ok, 0 failed, 0 skipped**, 423 s (`build/agentEE/regression1.txt`) |
| incremental build | - | 714 steps, **0 errors** |
| clean full release build of the snapshot | - | **971 steps, 0 errors**, all three targets (`build/agentEE/build_clean.log`, `build/agentEE_clean`) |

## 8. Defects found

1. **`GSystemSettings.bAllowPostprocessMLAA` is a storage-less shim that gated the only two render targets MLAA
   needs** (`SceneRenderTargets.cpp:2381`, retail 0x451080). Section 1. Fixed by porting retail's gate, which is no
   gate at all on PC. This one aborted the render thread on the first frame the moment the MLAA pass existed.
2. **Four of the five 2013 addresses in `arkppnodeaa.cpp`'s own comment block were fabricated** — `0x5210b0`,
   `0x521570`, `0x521990`, `0x521ee0` are the 2012 addresses `0x5610b0`, `0x561570`, `0x561990`, `0x561ee0` with one
   nibble changed, and `0x521990` happens to be retail's `RenderMotionBlur`, so quoting it would have sent the next
   reader to the wrong function. The real ones are `0x5203c0`, `0x520880`, `0x520ca0`, `0x5211f0`. This is the same
   failure `PHASE11.md`'s coordinator note describes one module over; it is worth a sweep of every `2013 rva` in the
   tree whose low three nibbles match a known 2012 address.
3. **Agent DB's defect 4 is not a defect** and its hand-over 1 rests on it: the cooked `DisFog` shaders never bind
   `MaskTexture` (`MaskTexture bound 0`, measured), and `RenderFogMaskStencil` writes stencil only. Section 4.
4. **Agent DB's hand-over 3 was wrong on two of three counts**: `FViewInfo::m_VisibleSoulPrimitives` did not exist and
   nothing filled it, and the pass is in DishonoredGame, not Engine. Section 5.
5. **Agent DA's `GaussianBlurFilterBuffer` deviation was a factor of `FilterDownsampleFactor` (4), not a style
   difference.** Retail derives the reference function's `ViewSizeX` as `SizeX * FilterDownsampleFactor`; DA passed
   `SizeX` alone, so the far blur ran with a kernel radius of `KernelRadius / FilterDownsampleFactor` where retail uses
   `KernelRadius` itself: 1.75 against 7.0 at a 1280-wide view, 1.38 against 5.51 at the 1008-wide window measured
   here. `FilterDownsampleFactor` is 4, and it cancels out of retail's expression exactly. The sample mask was the
   no-clamping pair where retail passes `(0,0)..(1,1)`. Both closed, and the `Min(·,16)` clamp retail applies before
   scaling is now in `GaussianBlurFilterBuffer` itself.
6. **`GSystemSettings.RenderThreadSettings` is never written anywhere in this tree**, so every one of its eleven
   members reads zero. It is not a `DISHONORED_SHIM_STATIC`, so agent DP's audit does not see it; it is a real
   structure with no writer, which is the same failure with a different shape. Porting the 2012 `RenderMlaa` would
   have read `bAllowMLAA` out of it and silently turned MLAA into FXAA. Named here rather than fixed: the retail 2013
   pass does not read it.
7. **Retail dereferences `m_InProxy` and `m_TargetProxy` with no null check** in the AA and Kuwahara `Render`s. Ours
   guard and say so at the site, as agent DA's depth-of-field port does.
8. **Retail passes an `FVector2D` through `SetVertexShaderValues<FVector4>`** in all three of the vertex shaders'
   `SetParameters` (`OORTSize`, `InvTextureSize`, `fxaaQualityRcpFrame`), so the last eight bytes of the value read by
   the call are whatever the stack held. Only the first two floats reach the shader, because the cooked parameter is
   two floats wide; ported as an `FVector2D` write, which is what retail's shader receives.

## 9. Files

Mine (14):

| File | Change |
|---|---|
| `Engine/Src/arkppnodeaa.h` | **new**: `FArkPpFxAaParameters`, `FArkPpMlaaParameters`, and the three shared AA shader types moved out of `PostProcessAA.cpp` (which is where retail declares them) |
| `Engine/Src/arkppnodeaa.cpp` | the FXAA pass, the MLAA driver and its three passes, the two `FlushShader` templates, the three `SetParameters` of the file's own pixel shader templates, the two vertex shaders' `SetParameters`, four bring-up switches and the census counter |
| `Engine/Src/arkppnodekuwa.cpp` | `FArkPpKuwaParameters`, `RenderKuwa`, retail's `Render`, both shader setters, one switch and the census counter |
| `Engine/Src/arkppnodeblur.cpp` | the census counter's definition only (the pass is section 6) |
| `Engine/Src/arkppnodedof.cpp` | the Gaussian blur call's arguments (defect 5), the `-arkppdofgaussold` pair switch and the DOF census counter |
| `Engine/Src/PostProcessAA.cpp` | the three class bodies removed and `#include "arkppnodeaa.h"` added; the three `IMPLEMENT_SHADER_TYPE` lines stay |
| `Engine/Src/SceneFilterRendering.cpp` | retail's `Min(KernelRadius / FilterDownsampleFactor, 16)` clamp, and a note on the three reference terms retail does not have |
| `Engine/Src/SceneRenderTargets.cpp` | defect 1: the MLAA targets' gate |
| `Engine/Src/SceneRendering.h` | `FViewInfo::m_VisibleSoulPrimitives` |
| `Engine/Src/SceneRendering.cpp` | `ProcessVisible`'s soul fill, the census line's four new clauses and their counters, the per-frame resets |
| `Engine/Src/ScenePrivate.h` | five census externs |
| `Engine/Src/FogRendering.cpp` | the `MaskTexture` measurement and what `RenderFogMaskStencil` really is, at the `bHasMask` seam |
| `Engine/Inc/Scene.h` | `FPrimitiveViewRelevance::iSoulRenderingRelevance` |
| `Engine/Inc/arkpp.h` | four census externs |

Nothing of agents DA's, DB's or DE's work was reverted: every edit to a shared file is an insertion or a one-line
change against `c598a21`, diffed rather than copied (`PHASE11.md`'s third rule).

Helpers in `build\agentEE\` (not repo tools): the patch scripts (`patch_aa.py`, `patch_kuwa.py`, `patch_census.py`,
`patch_census2.py`, `patch_soul_fog_gauss.py`, `patch_mlaart.py`, `patch_gauss_switch.py`,
`patch_postprocessaa.py`, `patch_aadbg2.py`, `patch_aaclear.py`, written as files because the Bash tool mangles
CRLF), `crlf_ee.py`, `run_pair.py`, `stats.py` (image statistics, the per-channel threshold, the gradient buckets and
the amplified difference figures, no PIL), `pairs.cmd` / `pairs2.cmd` / `pairs3.cmd` / `pairs4.cmd`, `agentEE_clean_build.cmd`, `resolve.py` (agent CE's,
repointed) and the three IDA one-offs `dump_asm.py`, `xref.py`, `findimm.py`.

## 10. Bring-up switches (all function-local statics, per agent CA's finding)

| Switch | What it does |
|---|---|
| `-noarkppaa` | the AA node renders its input and draws neither antialiasing pass; the switch of the pair |
| `-arkppaafxaa` | forces the FXAA branch whatever the node's `m_Type` says, so both passes can be measured against one floor |
| `-arkppaamlaa` | forces the MLAA branch |
| `-arkppaadbg` | one line with the node's resolved type, luma source, luminance equation, MLAA threshold, `iType_AntiAlias` and destination |
| `-noarkppkuwa` | the Kuwahara node renders its input and its target and draws no filter |
| `-arkppaaclearmasks` | clears both MLAA targets before the edge pass; retail clears neither, this is the pair that says whether unwritten target memory reaches the frame |
| `-arkppdofgaussold` | restores the far blur's pre-package arguments, so defect 5's correction has a pair |

All seven are bring-up only and go with the rest of the graph's lines when it is finished. `-arkppaadbg` also reports
which buffer the node's input hands the pass, which is section 3's last finding.

## 11. Hand-overs

1. **`FArkPpNodeBlurProxy::RenderMotionBlur` (0x521990)**, with the full structure in section 6. It needs a map that
   uses a blur node, or a switch that forces the chain's `ArkPpNodeBlur_0` visible, so that it can be measured.
2. **`UDishonoredAudioSystem::GetCellAtPoint` (0x792d10) is what `RenderFogMaskStencil` waits on**, not anything in
   the renderer (section 4). It belongs with whoever takes the audio cell graph; the fog seam and the portal geometry
   are both already in place, and the pass itself is 9,806 bytes of `FPoly` clipping with no other dependency.
3. **The soul-part pass is DishonoredGame's** (`UDisDarkVisionMeshRenderPpController::Render` 0x7fe690) and belongs
   with dark vision, together with `UMeshComponent::SetSoulMaterial` (0x12b270) and the two scene proxies'
   `m_SoulMaterial`, which are what set the relevance bit this package now reads. Until then the set is empty and the
   census says so (section 5).
4. **Sweep the tree for fabricated 2013 addresses** (defect 2): four in one comment block here, three in briefs
   yesterday. A `2013 rva 0xNNNNNN` whose low three nibbles match a known 2012 address and whose high nibble differs
   by one is the signature; `resources/docs/symbols/match_2012_2013.csv` makes the check mechanical.
5. **Score a shim that is the sole condition of an `RHICreate*` or `Begin/FinishRendering*` call** in
   `resources/tools/shim_audit.py` (section 1). `bAllowPostprocessMLAA` was invisible in the audit's printed ranks and
   it switched off the entire antialiasing technique the shipped chain asks for.
6. **`GSystemSettings.RenderThreadSettings` has no writer** (defect 6). Either fill it in
   `ApplySystemSettingsToRenderThread` or delete it; today any port that reads it gets eleven zeroes.
