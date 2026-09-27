# Agent DA report — the depth-of-field node's uber pass, which is the colour treatment (2026-09-27)

Package **DA** of `PHASE10.md` (wave 8), the direct continuation of agent CE (`dec3fd4`). Written against
**HEAD `11bbf79`**. Build dir `build\agentDA` (Release, x86, Ninja, every module option), built from the snapshot
worktree `build\agentDA_wt` (`resources\tools\make_snapshot.py DA --list build\agentDA\snapshot_files.txt`, build
command `build\agentDA_release_build.cmd`). IDA copies `resources\docs\idb\shipping2012_agentDA.i64` /
`retail2013_agentDA.i64`, headless `decompile_funcs.py` and one `dump_asm.py` only; decompiles in
`build\agentDA\decomp12` plus agent CE's `build\agentCE\decomp12`. No commits, nothing staged. Status rows:
`agentDA_status.csv` (34).

```
rem accept 1: the pair (one build, the only difference is -noarkppdof), and its two noise floors
python build\agentDA\run_pair.py pair_before_b -noarkppdof
python build\agentDA\run_pair.py pair_after
python build\agentDA\stats.py --diff build\agentDA\pair_before_b.bmp build\agentDA\pair_after.bmp build\agentDA\diff_pass.png
rem accept 2: the baked LUT
python build\agentDA\run_pair.py dof_dbg -arkppdofdbg -arkppdoflut
python build\agentDA\bmp2png.py build\agentDA\lut.bmp build\agentDA\lut.png
rem the two probes that say what the pass is worth
python build\agentDA\run_pair.py lutblack  -arkppdoflutblack
python build\agentDA\run_pair.py testgrade -arkppdoftestgrade -arkppdoflut -arkppdofdbg
rem accept 3
python resources\tools\run_regression.py --build-dir build/agentDA --no-build
```

## Result

| Accept | State |
|---|---|
| the LUT uber pass is ported | **done**: `LutCreation` (2013 rva 0x522750), `Blend` (0x522990), `Downsample` (0x522210) and `Render` (0x523170) with the ramp resource (0x514bc0), the four `SetParameters` (0x50eab0, 0x50eb50, 0x514d40, 0x50ed00), the proxy constructor's override resolution (0x50e7e0) and the five `FArkUberPpParameters` / `FArkPpColorBalanceParameters` methods it needs (0x2a2980, 0x2a2a30, 0x2aca00, 0x2a2d20, 0x2acbc0). CE's pass-through stub is gone; the node now owns the destination and sets `bUseLDRSceneColor` itself |
| a pair from one build differing by one switch, quantified | **done**: `pair_before_b.bmp` (`-noarkppdof`, CE's stub) vs `pair_after.bmp` (the ported pass), scene frame 200 of `L_Tower_P`, `-benchmark -fps=30`. **Mean 1.66 of a possible 765, 0.07 % of pixels changed by more than 2 per channel, maximum 80.** Noise floor, two identical `-noarkppdof` runs: **mean 0.10, 0.10 %**; two identical default runs: **mean 2.16, 0.09 %** — *higher* with the pass on, because the pass adds the film grain and its noise indices are re-drawn every frame. Section 4 explains why the honest figure for this content is small and why that is the right answer |
| the LUT is what the frame goes through | **done and measured**: `-arkppdoflutblack` clears the ramp instead of baking it and the frame collapses to mean luma **0.23 of 255**; against the same build's default frame that is **mean 261.23 of 765, 99.51 % of pixels, maximum 764** (`lutblack.bmp`, `diff_lutblack.png`). Every pixel the game presents comes through this lookup table |
| the grade path works end to end | **done and measured**: `-arkppdoftestgrade` pushes one non-neutral grade through the real `ApplyTo`, and the frame changes by **mean 152.77 of 765, 100.00 % of pixels, maximum 744** — luma 87.30 → 45.25, R/G/B 81.9/88.4/91.7 → 56.5/40.4/38.9 (`testgrade.bmp`, `lut_testgrade.png`). Exposure, colour balance, desaturation and contrast all reach the image |
| a dump of the baked LUT | **done**: `build\agentDA\lut.png` (256x16, and `lut_x8.png` magnified) — the 16x16x16 cube unwrapped, blue per tile, red across, green down. With this content's neutral grade it is the identity ramp: index 0 → 1, 1 → 22, 8 → 144, 15 → 255. `lut_testgrade.png` is the same ramp with the test grade on it |
| regression harness green | **31 ok, 0 failed, 0 skipped** in one pass, 422 s (`build\agentDA\regression1.txt`), including `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `d3d9_frames 19860`, `d3d9_criticals 0`, `unported_natives 0` |
| run | `Initial startup: 3.7 s` (d3d9 regression stage), 0 criticals, 0 `Failed to find shader type`, census `FArkPp 1 nodes rendered, 2 draws (0 material tiles), 1 passes not ported` every frame (CE's rest state was `0 nodes rendered, 0 draws, 2 passes not ported`) |
| build | 643 steps, **0 errors** |

## 1. What the pass actually does

`FArkPpNodeDofProxy::Render` (0x523170) renders its input with `m_bForceToDestination` cleared, sets the opaque blend
state and the solid two-sided rasterizer once for the whole node, and then runs three passes:

1. **`Downsample`** (0x522210), **only when `m_FarBlurAmount > 0`**. Two full-screen triangles: the scene colour into
   `ArkDofHalf`, then `ArkDofHalf` into the engine's **FilterColor** buffer, followed by one gaussian blur of
   FilterColor. Both passes hand the vertex shader the *node's* surface size, not the size of the surface they read.
   The second destination is FilterColor and not `m_DofQuarterRT`, which is what the node's name suggests — the quarter
   target belongs to the common-target node (`EPpCt 2`). So `mLowResColor` is a blurred full-size buffer fed from a
   half-size downsample.
2. **`LutCreation`** (0x522750) — **the colour treatment**. One triangle over a **256x16 A8R8G8B8** render target
   (`FArkDofRamp<16>`, `InitDynamicRHI` 0x514bc0, `TexCreate_NoTiling | TexCreate_ResolveTargetable`), which is a
   16x16x16 colour cube unwrapped along X, so no volume-texture RHI work is needed. There is no viewport call (setting
   the render target already covers all of it) and no blend state (Render set the opaque one). The pixel shader
   (`FArkPpDofLutBlenderPS::SetParameters`, 0x50ed00) receives:

   | parameter | value |
   |---|---|
   | `CBShadowTones` | `(m_CrMgYbShadTones.xyz, m_Opacity)` — the whole grade's opacity rides in the shadow word's W |
   | `CBMidTones` | `(m_CrMgYbMidTones.xyz, 0)` |
   | `CBHighTones` | `(m_CrMgYbHighTones.xyz, 0)` |
   | `LUTParams` | `(pow(2, m_Exposure), m_GammaAdjustment / DisplayGamma, m_PreDesaturation, m_PostDesaturation)` |
   | `Overlay` | `FViewInfo::OverlayColor` |
   | `BrightnessContrast` | `(m_GimpBrightness, tan((m_GimpContrast + 1) * pi/4), 0, 0)` — the contrast dial is 1 at its neutral setting |

   The resolve is the one place in the node that passes `bKeepOriginalSurface` **FALSE**.
3. **`Blend`** (0x522990) — the uber pass. It binds the destination (the back buffer when this node ends the graph,
   its own surface otherwise), sets the view rectangle as the viewport, and draws one triangle with
   `TArkPpDofUberPS<1,1>` when there is a far blur and `<0,1>` when there is not. The pixel shader gets scene colour,
   scene depth, the low-resolution colour, the ramp, `(m_FocusDistance, 1/m_InFocusRadius, m_FarBlurAmount, 1)`,
   `MinZ_MaxZRatio` from the RHI and the film grain pair `(grain * 2/255, grain * -1/255, 0, 0)`. The vertex shader
   gets `(1/w, 1/h, 36, 42)`, the viewport scale/bias, and a pair of noise indices each re-drawn until it differs from
   the previous frame's, so the grain never stands still. `Render` then sets `View.bUseLDRSceneColor`, which is what
   stops `FSceneRenderer::FinishRenderViewTarget` copying the un-post-processed scene over the node's output.

The node's own uber parameters are resolved in the constructor (0x50e7e0): the node's `m_Parameters`, then
`SetDefaultOnNoOverride`, then every push on the config's override stack. Measured on `Test_PPG` in `L_Tower_P`:

```
FArkPp dof uber parameters: 1 overrides, groups dof 1 cb 1 hdr 1
  focus 1000.0 radius 300.0 far blur 0.000 | shad (0 0 0) mid (0 0 0) high (0 0 0)
  opacity 1.000 pre desat 0.000 post desat 0.000 | exposure 0.000 gamma 1.000 grain 1.050 brightness 0.000 contrast 0.000
```

## 2. Defects found

1. **`ULocalPlayer::UpdatePostProcessSettings` is still the reference function** (retail 2013 rva 0x2b08b0,
   `UnPlayer.cpp:3547` here). This is the one that matters and it is the reason the figures in section 4 are small.
   Retail's body is short and does one thing: it fills `m_CurrentArkPpSettings` — the `FArkPpConfig` the view hands
   the graph — from `AWorldInfo::GetPostProcessSettings(ViewLocation, FArkPpConfig&)` (0x24b0c0), then
   `APlayerController::ModifyPostProcessSettings(FArkPpConfig&)` (0x6adeb0), then the camera's
   `m_CamPostProcessSettings` through `FArkPpConfig::ApplyTo` (0x2adb50) when `CamOverridePostProcessAlpha > 0`, then
   the gameplay override `m_ArkPpSettingsOverride` with its recovery fade. **Our tree runs the reference body instead,
   which blends `FPostProcessSettings` — a structure retail's renderer never reads — and writes nothing at all to
   `m_CurrentArkPpSettings`.** That member is reflected, so it keeps its script default properties, and those are
   exactly what the log above shows: a focus distance and radius and a film-grain amount that are plainly not the
   struct defaults, and a colour balance and exposure that are completely neutral. The grade the levels, the volumes,
   the camera and Kismet carry never reaches the LUT. This is the same shape as the six wave-4-to-7 defects: a
   reference member kept as a storage-less placeholder. It is not in this package's files; hand-over 1.
2. **`FArkUberPpParameters::SetDefaultOnNoOverride` has an inverted test in retail** (0x2a2d20). Every field of the
   depth-of-field group resets to its default when its override bit is **clear** — except `m_InFocusRadius`, which
   retail resets when the bit is **set** (`if ((bits & 2) != 0) m_InFocusRadius = 500`). So a node that overrides the
   radius has its value thrown away and replaced with 500, and a node that does not override it keeps whatever its
   archetype held. Ported as retail has it and documented at the site; the practical effect is that a graded chain's
   focus radius comes from the override stack, never from the node.
3. **`FArkPpNodeDofProxy::m_LinearToGammaRsc`** (2012 PDB @12) is cleared by the constructor and **written nowhere in
   the whole executable**, and `UArkPpNodeDof::m_LinearToGammaRamp` (@108, a `UTexture2D*` the content can set) is
   **never read**. The ramp the uber pass samples is always the one `GDofRamp` bakes. The member is kept for the
   layout's sake and stays NULL, as it does in retail. This is CE's `m_GoToLR` finding again, one node over.
4. **Retail binds the scene colour texture to both `SrcColor` and `SrcDepth`.** This is not a decompiler artefact: the
   disassembly of `Blend` reads `GSceneRenderTargets.RenderTargets[SceneColor].Texture` twice
   (`build\agentDA\blend12.asm`, two separate `mov eax, ...RenderTargets.Texture.Reference+0Ch`) and
   `RenderTargets[FilterColor].Texture` once. Depth on this renderer lives in the scene colour's alpha, which is also
   why the pass takes `MinZ_MaxZRatio` from the RHI; the only difference between the two bindings is the filter, point
   for the colour and bilinear for the depth. Ported as retail has it — taking the names at face value and binding
   `GetSceneDepthTexture()` would have bound a surface the cooked shader does not expect.
5. **Retail's proxy constructor applies the override stack in a strange order**: push 0 first, then the rest from the
   newest down to 1 (0x50e7e0). With the one push `FViewInfo` makes it is moot, but it is not the plain reverse walk
   CE's stub assumed, and it is ported as retail has it.
6. **Retail calls `m_InProxy->Render` with no null check** in `Render`. A depth-of-field node whose `m_SurfaceTarget`
   is unset would take retail down mid-frame; ours guards and says so at the site.
7. **HEAD `11bbf79` does not build `DishonoredGameModule`.** `EngineArkaneClasses.h` (committed) includes
   `arkcomponentbase.h` and uses `FArkComponentBase`, but the committed `arkcomponentbase.h` is still the
   `import_reference.py` stub with no class in it; the class only exists in another agent's uncommitted working-tree
   copy. Every TU that includes the header fails with `C2143: syntax error: missing ';' before '*'`. This snapshot
   overlays the working-tree `arkcomponentbase.h`, `arkgameeventdispatcher.h`, `arkcomponentcontainer.cpp` and
   `arkgameeventdispatcher.cpp` to get a build; **those four files are not this package's work** and are listed in
   `build\agentDA\snapshot_files.txt` only as the bridge. The coordinator needs the owning package's header committed
   with its body.

## 3. What the LUT looks like

`build\agentDA\lut.png` (and `lut_x8.png`, magnified eight times) is the baked ramp with this content's grade. It is
sixteen 16x16 tiles: blue steps per tile, red across each tile, green down. The values are monotonic and very close to
the identity:

```
row  0:   1,  1,  1   22,  1,  1  144,  1,  1  255,  1,  1 | 1,1,22 | 1,1,144 | 1,1,255 | 255,1,255
row  8:   1,144,  1   22,144,  1  144,144,  1  255,144,  1
row 15:   1,255,  1   22,255,  1  144,255,  1  255,255,  1 ... 255,255,255
```

`lut_testgrade.png` is the same ramp with the test grade (exposure -1, high tones (0.15, -0.05, -0.20), shadow tones
(-0.10, 0, 0.15), post-desaturation 0.5, contrast 0.25): the whole cube darkens, desaturates and warms, and the frame
follows it exactly.

## 4. Why the pair's number is small, and why that is the right answer

`-noarkppdof` vs the ported pass changes **0.07 % of pixels by more than 2 per channel, mean 1.66 of 765**, and the
two-identical-runs floor with the pass *on* is **2.16** — larger than the pair itself. That is not the pass failing to
draw; the census says `FArkPp 1 nodes rendered, 2 draws` on every frame and the debug line says
`destination yes, back buffer same`. Three things together explain it:

* The content's grade at this point **is neutral**. `SetDefaultOnNoOverride` forces every group the content does not
  override to the neutral state, and the resolved parameters (section 1) are a neutral colour balance, exposure 0,
  gamma 1, brightness 0 and contrast 0. The only non-neutral term is the film grain, 1.05.
* Because the grade is neutral, the ported pass and the path it replaces agree: with `-noarkppdof` nothing in the graph
  draws, `bUseLDRSceneColor` stays clear, and `FSceneRenderer::FinishRenderViewTarget` puts the gamma-corrected scene
  colour on the view's render target. With the pass on, the uber shader writes the same image through the identity LUT
  and claims the target. **Two entirely different code paths landing within 0.55 of 255 per channel of each other is
  the strongest evidence this package has that the port is right.**
* The residual difference *is* the film grain: the pass's noise indices are re-drawn every frame, so two runs of the
  same frame with the pass on differ by 2.16 while two runs with it off differ by 0.10.
* Section 2 defect 1 is why the grade is neutral rather than Dishonored's own: the settings path that would fill it
  was never ported. `-arkppdoftestgrade` shows what happens the moment a real grade arrives: **100 % of pixels, mean
  152.77 of 765**.

One measurement caveat, recorded so the next agent does not chase it: the intro of `L_Tower_P` is **not** reproducible
run to run on a loaded machine. Three `-noarkppdof` runs landed in two different world states (`a` and `c` identical to
0.10; `b` differing from them by mean 61.74 over 66 % of pixels). The pair and both floors above are taken inside one
state; `diff_noise_ab.png` is the outlier for the record. Agent CE measured a 0.09 % floor on the same command when the
machine was idle.

## 5. Not ported, and exactly what each one needs

* **The settings path** — hand-over 1, and the one that carries the look (section 2 defect 1).
* **The AA passes** (`RenderFxaa` 0x5203c0, `RenderMlaa` 0x521720 and its three passes), **motion blur**
  (`RenderMotionBlur` 0x521990) and **Kuwahara** (`RenderKuwa` 0x523350). All of their plumbing now exists in this
  file's shape: the ramp is the template for a private render resource, `ArkGetCommonVertexDeclaration` +
  `ArkFullScreenTriangleFloat2Vertices` + `SetGlobalBoundShaderState` is the pass skeleton, and
  `FArkPpDofUberParameters` is the parameter-struct pattern. Hand-over 2.
* **The `LensCompose` material node**, the root of this chain, is `game 0` — retail never shows it in game, so the
  graph falls through to its surface target and the depth-of-field node ends up owning the destination. Whoever ports
  it should establish *why* retail ships the chain with it hidden before making it draw. Hand-over 3.
* **`GaussianBlurFilterBuffer`**: retail's takes an absolute kernel radius and no view; this tree's scales the radius
  by `ViewSizeX / 1280` and takes a sample-mask pair. The `Downsample` call passes the node's own width as the view
  width and the no-clamping mask, which is the only deviation in the three passes. It only runs when
  `m_FarBlurAmount > 0`, which this content leaves at 0, so it is untested on real content.
* **The `FArkUberPpParameters` methods live as free functions** in `arkpp.h` / `arkppnodedof.cpp` rather than as
  members, because `EngineClasses.h` is generated and its Arkane structs have no CppText hook. Move them onto the
  structs when the generator grows one.

## 6. Numbers

| Measure | HEAD `11bbf79` (CE's stub) | With package DA |
|---|---:|---:|
| FArkPp passes drawn per frame | 0 | **2** (LUT bake + uber blend) |
| graph nodes reaching a render | 0 | **1** of the 3 proxies at rest |
| passes still stubbed in the census | 2 | **1** (the AA node) |
| pixels changed by the pass, same state | - | **0.07 %**, mean 1.66 of 765, max 80 |
| noise floor, two `-noarkppdof` runs | - | 0.10 %, mean 0.10 of 765 |
| noise floor, two default runs (the grain) | - | 0.09 %, mean 2.16 of 765 |
| pixels the LUT carries (`-arkppdoflutblack`) | - | **99.51 %**, mean 261.23 of 765, frame luma 0.23 of 255 |
| pixels a real grade changes (`-arkppdoftestgrade`) | - | **100.00 %**, mean 152.77 of 765, luma 87.30 → 45.25 |
| `Initial startup` (d3d9) | 3.4 s | 3.7 s |
| scene frames, 90 s regression run | ~20,000 | 19,860 |
| criticals | 0 | 0 |
| regression | 31 checks | **31 ok, 0 failed, 0 skipped** |

## 7. Files

Mine (2): `Engine/Src/arkppnodedof.cpp` (rewritten from the stub: the ramp resource, the two parameter structs, the
four `SetParameters`, the five uber-parameter helpers and the four proxy methods) and `Engine/Inc/arkpp.h` (the five
helper declarations only). `Engine/Src/arkppnodes.cpp` was **not** touched: the DOF node's registration and dispatch
lines needed no change, so agent DB's edits to that file and to `SceneRendering.cpp` do not collide with this package.

The snapshot also carries four files that are **not mine** — `Engine/Inc/arkcomponentbase.h`,
`Engine/Inc/arkgameeventdispatcher.h`, `Engine/Src/arkcomponentcontainer.cpp`,
`Engine/Src/arkgameeventdispatcher.cpp` — copied from the shared working tree because HEAD does not build without them
(section 2 defect 7). Do not merge them from here.

Helpers in `build\agentDA\` (not repo tools): `write_dof.py` and `patch_probes.py` (the edits, written as files
because heredocs mangle CRLF), `run_pair.py`, `stats.py` (image statistics and the amplified diff figures, no PIL),
`bmp2png.py`, `dump_asm.py` (the one disassembly that settled defect 4).

## 8. Bring-up switches (all function-local statics, per agent CA's finding)

| Switch | What it does |
|---|---|
| `-noarkppdof` | the node falls back to CE's pass-through stub; the switch of the pair |
| `-arkppdofdbg` | one line with the resolved uber parameters at the first proxy build, and the first two blend passes |
| `-arkppdoflut` | writes the baked ramp once to `<GameDir>/Logs/ArkDofLut00000.bmp` |
| `-arkppdoflutblack` | clears the ramp instead of baking it: the measurement of how much of the frame goes through the LUT |
| `-arkppdoftestgrade` | pushes one documented non-neutral grade through the real `ApplyTo`, standing in for the settings path of defect 1 |

All five are bring-up only. `-arkppdoftestgrade` in particular must go the moment
`ULocalPlayer::UpdatePostProcessSettings` is ported.

## 9. Hand-overs

1. **Port retail's `ULocalPlayer::UpdatePostProcessSettings` (0x2b08b0) and the three functions it calls**
   (`AWorldInfo::GetPostProcessSettings` 0x24b0c0, `ADishonoredPlayerController::ModifyPostProcessSettings` 0x6adeb0,
   `FArkPpConfig::ApplyTo` 0x2adb50, plus `FArkUberPpParameters::ForceDefault` 0x2a2e00). Everything downstream of it
   is now in place and measured: the moment a real grade lands in `m_CurrentArkPpSettings` it reaches the LUT and
   changes every pixel. This is the single highest-value item left in the post-process graph and it is a contained job
   — one function of about 60 lines plus three callees, all decompiled in `build\agentDA\decomp12`. Agent CE's
   hand-over 3 (the four DishonoredGame post-process controllers) is the same family and should be taken with it.
2. **The remaining filter passes** (AA, motion blur, Kuwahara): section 5. Every shader type loads, every target
   exists, and `arkppnodedof.cpp` is now the worked example of a node pass in this tree.
3. **The `LensCompose` node** and `UPostProcessChain::CreateMutableMaterialInstanceOnNodes` (0x2d2a40): section 5.
4. **Commit `arkcomponentbase.h` with a body** (defect 7) — HEAD cannot build `DishonoredGameModule` without it.
5. **Lines to drop when the graph is finished**: the five switches of section 8 and the parameter dump.
