# Agent CC — the renderer's drawing half: geometry, fills, masks and the texture path (2026-09-27)

Package **CC** of `PHASE9.md`: port the 221 drawing functions behind the 54-slot renderer interface
agent BB declared and verified by call, plus the RHI resources the seam deliberately left out (the
updatable and mappable textures), and prove it **without the game** — a dumped image of known
geometry, then a cooked movie's first frame rendered to a target.

**Both images exist.** They are `build/agentCC/agentCC_geometry.png` (256×256) and
`build/agentCC/agentCC_movieframe.png` (1280×720), and the .bmp the game wrote is beside each.

## Result

| Accept | State |
|---|---|
| a dumped image from the renderer showing the expected geometry | **done**: `agentCC_geometry.bmp`, 256×256, drawn through a `GRenderer*` — the background quad, a solid red quad (indexed tri list, XY16i), a Gouraud quad (XY16iC32, per-vertex colour), a bitmap-filled checkerboard (the texture path and the texture matrix), a green quad **clipped to the left half by a stencil mask**, a white line strip, and four glyph quads through `DrawBitmaps` |
| a cooked movie's first frame rendered to a target | **done**: `agentCC_movieframe.bmp`, the full 1280×720 frame of `UI_Global.Global` — its own background colour (0x666666, gamma-corrected to 0xa8a8a8 by the renderer's own inverse gamma) over its own frame rect, plus **40 of its 41 cooked bitmaps** drawn through the bitmap path with real DXT textures and per-image texture matrices. What the frame does *not* have is its characters, and why is measured in section 5 |
| per-pass draw counts in a census line | **done**, two of them: geometry `8 draws (5 trilist, 1 line, 1 bitmap, 1 background, 0 filter), 20 triangles, 4 lines, 4 glyphs, 1 masks, 4 bound shader states`; movie `41 draws (40 trilist, 0 line, 0 bitmap, 1 background, 0 filter), 82 triangles, 2 bound shader states`, both with the per-slot counts after them |
| `run_regression.py --build-dir build/agentCC_rel --no-build` green | **31 ok, 0 failed, 0 skipped, 435 s** — the whole harness, nothing skipped |
| 1:1 | every body names the 2012 rva it was written from; 4 documented deviations, all in section 6 |

Three defects were found by measuring rather than by reading, and all three are the kind that make a
renderer draw nothing or crash: section 4.

## 1. What the drawing half actually is

Worth stating plainly, because it is not what a Scaleform port looks like: **Arkane draws Scaleform
through UE3's own RHI.** There is no Scaleform renderer back end in the retail exe. Every `GRenderer`
slot runs on the game thread, keeps the state GFx asked for and enqueues a render command; the
`_RenderThread` half of each pair is what touches the RHI. Geometry goes out as **user-pointer
draws** — `RHIDrawIndexedPrimitiveUP` and `RHIDrawPrimitiveUP` straight out of the element store,
under the element lock. There is not one vertex buffer in the retail renderer's 221-function list.

| Piece | Where | Shape |
|---|---|---|
| the 54 `GRenderer` slots + the render-thread half | `GFxUI/Src/gfxuirenderer.cpp` (4,456 lines, 226 of them the PDB evidence block) | every slot enqueues an `FRenderCommand`; retail's generated `ENQUEUE_UNIQUE_RENDER_COMMAND` expansions are the ring-buffer writes the decompiles show |
| the element stores | `namespace FGFxRendererImpl` | `FGFxRenderElementStoreBase` (PDB 32) + `FGFxVertexStore`/`FGFxIndexStore`/`FGFxBitmapDescStore` (36 each), refcounted across the two threads, with the main thread taking a copy when the render thread still holds one |
| the styles | `FGFxRenderer::FGFxRenderStyle` / `FGFxFillStyle` / `FGFxLineStyle` (PDB 44 / 128 / 44) | `GetEnumeratedBoundShaderState_RenderThread` picks the pixel/vertex shader and vertex declaration; `Apply_RenderThread` sets the colour, the colour transform and up to two textures |
| the shader families and their interfaces | `GFxUI/Inc/gfxuirendererimpl.h` | `FGFxPixelShaderInterface` (13 slots) and `FGFxVertexShaderInterface` (5), implemented by agent BD's three templates, which moved here because that is the file the PDB attributes their bodies to |
| the RHI resources | same header + unit | `FGFxUpdatableTexture` (PDB 92), `UGFxUpdatableTexture` / `UGFxMappableTexture` (368 each, `DECLARE_CLASS_INTRINSIC` on `UTexture2D`), `FGFxRenderTargetResource` (56), `FGFxRenderResources` (40) |
| the probe | same unit, `-gfxdrawprobe` / `-gfxmovieprobe=` | the acceptance, section 3 |

**On scissoring, which the package asked for by name: the retail renderer has none.** There is no
`RHISetScissorRect` call and no reference to `GViewport::ScissorLeft/Top/Width/Height` or
`View_UseScissorRect` anywhere in the 263 decompiled functions. Clipping is the stencil mask, and
the viewport rectangle that `SetUIViewport` computes from the clipped pixel range and
`CheckRenderTarget_RenderThread` re-applies — which is why `FGFxViewportAxisInfo` carries a
*PixelClip* range at all. This port has the same two mechanisms and no third.

`FGFxRenderer` is now the **full 892-byte retail layout**: all 35 members at their PDB offsets,
including the two element stores, the bound-shader-state cache, the eight sampler states, the
render-target stack, the temp-target and temp-stencil arrays, the element lock and the blend stack.
Agent BB's 472-byte subset is gone.

## 2. The evidence, and the two traps that were already known

Everything is from the 2012 Shipping PDB (`build/agentBB/fgfx_types.json`, agent BB's dump) and from
**241 headless decompiles** of my own IDA copy (`build/agentCC/dec`, `dec2`;
`resources/docs/idb/shipping2012_agentCC.i64`). No MCP tool of any kind was used.

**Agent BE's trap 2 holds and is now load-bearing.** GFx 3.3's `GRenderer::Cxform` is `float M_[4][2]`
— channel-major, the multiply in column 0 and the add in column 1, the add in 0..255 — and
`FGFxFilterPixelShader<11>::SetParametersColorScaleAndColorBias` (2012 0x5bcd80) proves it a second
time: `ColorScale = M_[0][0], M_[1][0], M_[2][0], M_[3][0]` and
`ColorBias = M_[0][1], M_[1][1], M_[2][1], M_[3][1]` scaled by 1/255. **One thing to pass on:
`External/GFx3/GFx3Gen.h` spells the same 32 bytes `float M_[2][4]`**, which is the GFx 4 shape; the
size assertion passes either way, so nothing caught it. Every colour-transform site here therefore
goes through `FGFxCxformMul(Cx, Channel)` / `FGFxCxformAdd(Cx, Channel)`, which index the 32 bytes
the way the retail bodies do. The generated header is agent BB's and its generator would put the
wrong shape back, so it is left alone and recorded here instead — **the next agent to touch that
generator should change the dimension order.**

**Agent BB's overload-order warning applies to two runs in this package** and both are declared
highest-slot-first: `FGFxRenderTarget::InitRenderTarget_RenderThread` (slot 9 then slot 8) and, in
the interface classes, nothing else — the 13 `FGFxPixelShaderInterface` slots and the 5
`FGFxVertexShaderInterface` slots have no same-name run, so declaration order is slot order there.

## 3. Proving it without the game

Two switches on the game exe, both one-shot, both dumping a bitmap rather than a log line. They run
from `UGameViewportClient::Draw` (`Engine/Src/UnPlayer.cpp`, 10 lines behind
`DISHONORED_WITH_GFX3 && DISHONORED_WITH_GFXUI_SHADERS`) because that is the game thread with the
RHI, a viewport and the cooked global shader map all up — which is where retail drives the UI from
too (`ADishonoredHUD::PostRender`).

```
-gfxdrawprobe               known geometry into a GFx temp render target, dumped
-gfxmovieprobe=<Pkg|Path>   one cooked movie's first frame into the same kind of target, dumped
```

Every call in both probes goes through a `GRenderer*`, not an `FGFxRenderer*`, so what is exercised
is the vtable the runtime sees — the same rule agent BB's `--slots` used.

`agentCC_geometry.bmp` reads exactly as designed, and three details in it are the port working
rather than merely running:

* the solid quad is **(238, 119, 119)**, not the (220, 48, 48) it was asked for: that is
  `pow(colour, InverseGamma)` with the 1/2.2 the renderer computed from the viewport's display
  gamma, i.e. `ApplyUIColor_RenderThread` (2012 0x5bae50) doing its job.
* the geometry lands exactly where its pixel coordinates say (the red quad at 16..112, the mask at
  144..192), which is the one thing that could not be read out of a decompile: the GFx `.usf` sources
  are not in this tree, so the 2D transform's convention came from how retail writes it into an
  `FMatrix` (2012 0x5bb480 - `M[0][0]=m00, M[0][1]=m10, M[1][0]=m01, M[1][1]=m11, M[3][0]=m02,
  M[3][1]=m12`, i.e. the row-vector form) and the image is what confirms it. The checkerboard
  mapping confirms the texture matrix the same way.
* the green quad is **clipped to the left half of itself**, which is the stencil mask:
  `BeginSubmitMask(Mask_Clear)` clears stencil and draws the mask shape with colour writes off,
  `EndSubmitMask` switches the test to *equal to the stencil counter*, and `DisableMask` puts it
  back. Masks are the one part of a UI renderer that fails silently when it is wrong; this one does
  not.

`agentCC_movieframe.bmp` is `UI_Global.Global`, one of the six cooked movies the map has loaded by
the time the probe runs; `-gfxmovieprobe=` asks for `Dishonored_MainMenu` first, and that package's
`UI_MainMenu.MainMenu` is not resident at the first viewport draw, so the probe falls back to a
loaded one and says which. The frame is the container parsed with agent BB's `GFxGfxParseFile`
(GFX v10, 1280×720, 30 fps, 22 frames, 310 tags, 41 exports, 4 imports, 41 external images), the
movie's own background colour from its `SetBackgroundColor` tag, and its own 40 resolvable cooked
bitmaps through `FillStyleBitmap` + `DrawIndexedTriList`.

**A finding for the image loader while doing that**: tag 1009's `ExportName` is *empty* for all but
the handful of images the artist named, and the `Texture2D` export name is then the TGA file name
without its extension (`Global_I2.tga` → `Global_I2`); a named one can carry the exporter's own
suffix after a space (`"gl_msgBox_bkgd -nopack"`), which is not part of the object name. With the
name taken from `ExportName` alone, 1 of 41 images resolves; with this rule, 40 of 41.

## 4. Three defects, measured

### 4.1 A GFx object freed by its own creator (a crash, and the reason the first four runs died)

`GTexture` and `GRenderTarget` keep their own `GAtomicInt RefCount`, which **default-constructs to
0**. Retail sets it to 1 explicitly — `FGFxRenderer::CreateTexture` (2012 0x5c47d0) writes
`RefCount.Value = 1` right after the vtable store, and `FGFxRenderTarget::FGFxRenderTarget`
(0x5ce5b0) writes it between its two vtable stores. Without that, the first `GPtr` that takes and
drops a reference frees the object under its creator: `PushTempRenderTarget_RenderThread` created a
render target, pushed it into the temp array, dropped its own reference and deleted it; the heap
handed the block straight back out to the next allocation, and the next virtual call jumped to
whatever had overwritten the vptr. Measured with the vptr reading as a heap address and the members
as UTF-16 text, and the stack's top frame a wild `0xebfff05b`.

Both constructors now set it, with the rva in the comment. **This is a class of defect worth naming
for the other GFx packages: every `GRefCountBase` in `GTypes.h` starts at 1, but the two renderer
interfaces do not, and the PDB layout alone does not say so.**

### 4.2 `SetUIViewport` must not touch `CurrentMatrix` (a draw that lands off screen)

`FGFxRenderer::SetUIViewport`'s command (2012 0x5b7a60) does exactly four things: copies the clipped
viewport params, calls `RHISetViewport` with them, writes the new **viewport** matrix, and appends
the user matrix to it. It does **not** write `CurrentMatrix` and does not re-bind the render target.
Writing `CurrentMatrix` there is invisible in a log and obvious in an image: `BeginDisplay` sets the
current matrix to the identity on the game thread *before* the viewport command runs on the render
thread, so the background quad is transformed by the viewport matrix twice and lands off screen. The
first dumped image had a black background and one correct pixel in the corner; the second has the
background across the whole frame.

### 4.3 The GFx reconstruction has two colliding copies of one class, and it only shows at link time

`GFxUI/Inc/gfxui_gfx3.h` (agent BE) declares its own `GRefCountImplCore` with `AddRef`/`Release`
defined out of line in `gfxuigfx3absent.cpp`, and `External/GFx3/GTypes.h` (agent BB) declares the
PDB's with both inline. Any unit that uses the GFx3 refcount chain collides with the first at link
time — two `LNK2005`, measured. The two are the same 8 bytes and the same behaviour, two
reconstructions of one retail class, so GFx3's copy is renamed for the duration of the include in
`gfxuirenderer.h` (with `GFxRenderTextureMode`, `GFxTimingMode` and their seven enumerators, which
the generated `GFxUIEngineShims.h` also declares). **The real fix is agent BE's own and belongs to
the coordinator: `DISHONORED_GFXUI_GFX3_RUNTIME=1` drops `gfxuigfx3absent.cpp` and the duplicates
with it.** See the hand-over.

## 5. What the movie frame is missing, and why it is not this package

The frame has its background and its bitmaps and not its characters. Two things stop it, both
measured:

1. **`GFxMovieRoot::Display()` draws nothing** (agent BC left it empty; `agentBC.md` 6.9), and the
   characters it would walk are placeholders until package **CD** lands the 31 tag loaders — shapes
   first. There is nothing for the renderer to be handed.
2. **This module cannot link agent BC's player at all today.** Instantiating
   `GFxMovieDataDef → GFxMovieDefImpl → GFxMovieRoot` from `gfxuirenderer.cpp` pulls
   `gfx3.lib(GFxPlayerRoot.cpp.obj)` into the link, and every `GFxValue::ObjectInterface` method in
   it collides with agent BE's stand-in bodies in `gfxuigfx3absent.cpp`: **28 `LNK2005`, measured.**
   So the movie probe parses the container and renders the frame without the player.

Neither is a renderer gap, and neither needs more code here: the player call site is already written
in the movie probe behind `#if DISHONORED_GFXUI_GFX3_RUNTIME`, so when the switch flips and
`Display()` walks the list, the same probe dumps the frame with its characters and the census counts
what the runtime submitted.

## 6. The four documented deviations

1. **`DrawBlurRect_RenderThread`** (2012 0x5dc220): the shader selection is retail's bit for bit —
   `FBox2Blur` (+2 for the multiply variants), `FBox2Shadow`/`FBox2InnerShadow` (+4 knockout, +1
   highlight, +2 multiply), `FBox2Shadowonly` under `Filter_HideObject` — and the single pass draws
   with all its parameters set. Retail's internal ping-pong through temp targets for the shadow
   composite is not reproduced. `CheckFilterSupport` reports `FilterSupport_Multipass` for a kernel
   over 64 or more than one pass exactly as retail does, which is the runtime's cue to split the
   filter itself, so the multi-pass case arrives here as several single-pass calls.
2. **`CreateTextureYUV` returns NULL**: the four `GFx_PS_TextTextureYUV*` kinds (46..49) are the
   video path and have **no cooked shader in this game's cache**, so there is no type to instantiate.
   `FGFxTexture::IsYUVTexture` returning 0 is what keeps the renderer from asking.
3. **`FGFxRenderTargetResource::OwnerDepth`**: retail takes the depth surface from the scene's own
   `FSceneDepthTargetProxy`. That type lives in `Engine/Src`, which is not on this module's include
   path, and nothing in this tree hands the seam an `OwnerDepth` yet. The member is there, the one
   line that fills `DepthBuffer` from it is a warn-once.
4. **The GC reference**: retail roots the engine texture through `UGFxEngine::AddGCReferenceFor`
   (2012 0x5c0000) on the `GGFxGCManager` singleton, which `gfxuiengine.cpp` owns and this build does
   not compile. `AddToRoot`/`RemoveFromRoot` has the same effect for the renderer's purposes; the
   stat accounting is retail's and is not reproduced.

Also recorded rather than deviated from: `MakeViewAndPersp3D` and the 3D branch of
`ApplyUITransform_RenderThread` are libgfx bodies this tree does not have, and nothing reaches them
(`GFxCharacter::SetMatrix3D` returns false, `agentBC.md` 6.9).

## 7. Verification

* **Build**: `build/agentCC_release_build.cmd` builds the HEAD snapshot `build/agentCC_wt`
  (HEAD `8e61755`, i.e. **after agent CA merged**) into `build/agentCC_rel`, RELEASE: 0 errors,
  `GFxUI.lib` and `DishonoredGame.exe` produced. The snapshot is HEAD + my five files, because the
  shared tree held 105 modified files from the other packages while this ran.
* **The two probes**: `build/agentCC_probe14.txt` and the run log
  `Dishonored_Latest2026/DishonoredGame/Logs/Launch_CC.log`; the images are in `build/agentCC/`.
  Both probes run under the d3d9 RHI at `-startmap=Dishonored_MainMenu`, with **0 criticals**.
* **Agent BD's shader cache is undisturbed by the template move**: the same run logs
  `global shader cache: 263 shaders, 258 loaded, 5 undeclared types, 0 parameter mismatches, 0 other
  skips` - BD's numbers exactly. Moving the three templates into `gfxuirendererimpl.h` and giving
  them their parameter setters changes neither the cooked type names nor the serialisation.
* **The census lines** are in the result table and in the log, in the
  `DISHONORED(bringup): <thing> census: ...` shape the other censuses of this tree use.
* **Regression**: **31 ok, 0 failed, 0 skipped** — twice, 435 s and 449 s
  (`build/agentCC_regression2.txt` and `build/agentCC_regression3.txt`,
  `build/agentCC_rel/regression/summary.txt`); the second is the final binary from the final
  sources, and both probes were re-run on it. The whole harness,
  with nothing skipped: CoreSmoke 99/0, layout 2,314 types with 0 mismatches and 0 contract
  failures, nullrhi 0 criticals, d3d9 **0 criticals** with 17,370 frames, 6,506 draw elements, 1,195
  draws per frame and 458 visible primitives, inputtest the pawn walking 1,025.3 units at a 500.4
  peak with 895 PhysX actors, 218 touch begins, 88,893 sequence ops and 0 criticals, and
  `unported_natives 0`. An earlier run of the same build (`build/agentCC_regression.txt`) skipped
  CoreSmoke and the layout probe because those two targets had not been built in this directory
  yet; they were built and the whole harness re-run twice since.
* **The decompiles**: `build/agentCC/dec` (241 functions), `build/agentCC/dec2` (22), the patterns in
  `build/agentCC/dec_patterns*.txt`, the enum dump in `build/agentCC/egfx_enums.json`.

## 8. Hand-overs

**Agent CB — your question, answered: the cache owns the texture, and it needs no hook.** The
alpha-only glyph atlas is a `GTexture` the cache holds a `GPtr` to, created through the renderer it
is handed — which is exactly why the update call takes a renderer pointer. Three slots, all
implemented here:

1. `GRenderer::CreateTexture()` → an `FGFxTexture`.
2. `GTexture::InitDynamicTexture(Width, Height, GImageBase::Image_A_8, Mipmaps, GTexture::Usage_Update)`
   → a transient `UGFxUpdatableTexture` whose resource is an `FGFxUpdatableTexture` at `PF_G8`.
   Pass `Usage_Map` (0x20) instead if you want to lock the atlas and write into it directly, which
   gives you `UGFxMappableTexture` and `Map`/`Unmap`; `Usage_Update` (0x10) is the one you want for a
   dirty-rectangle upload.
3. `GTexture::Update(Level, NumRects, Rects, Image)` with `GTexture::UpdateRect { GPoint<int> dest;
   GRect<int> src; }` — it enqueues a render command that uses `RHIUpdateTexture2D` and falls back to
   a lock-and-copy.

`PF_G8` matters beyond storage: `FGFxUpdatableTexture` sets `bGreyScaleFormat`, and
`DrawBitmaps_RenderThread` tests exactly that to pick `GFx_PS_TextTexture`, the alpha-only glyph
shader, rather than the colour or sRGB ones. So an `Image_A_8` atlas selects the right shader by
itself. **One ownership rule: release the texture before the renderer**, because `FGFxTexture` holds
a `GRenderer*` and the engine texture it roots.

And the drawing contract for your glyphs, which is already in place:
`DrawBitmaps(BitmapDesc* Bitmaps, INT ListSize, INT StartIndex, INT Count, const GTexture* Texture,
const GMatrix2D& Matrix, CacheProvider* Cache)`. Each `BitmapDesc` is
`{ GRect<float> Coords; GRect<float> TextureCoords; GColor Color; }`: `Coords` is the glyph's
destination rectangle in the space `Matrix` maps to the viewport (so your line-major, glyph-major
pen walk goes straight in), `TextureCoords` is **normalised 0..1** inside the atlas, and `Color` is
the glyph colour, which the shader modulates by the atlas alpha. One `DrawBitmaps` call per atlas and
per transform is enough: the render thread batches **192 descriptors into one
`RHIDrawPrimitiveUP`**, six vertices each. Pass a `CacheProvider` if the same list is drawn every
frame and the store will be kept instead of copied.

Your `GArray` trap: **checked, and this package has no by-value copy of one.** The three `GArray`
members of `FGFxRenderer` are never copied, the two local arrays in
`ReleaseTempRenderTargets_RenderThread` are filled element by element, and `RTState` (the only struct
copied by value here) contains no array. The one place that read an element after shrinking the
array now takes a copy first.

**Package CD — two things.** The movie probe carries the whole player call site —
`GFxMovieDataDef::Read` → `GFxMovieDefImpl::CreateInstance` → `SetViewport` / `Advance` / `Display`
— behind `#if DISHONORED_GFXUI_GFX3_RUNTIME`, compiled out only because of the link collision in
section 5. The moment that switch flips and `GFxMovieRoot::Display` walks the display list,
`-gfxmovieprobe=` dumps the frame with its characters in it and the census counts the draws per
pass; nothing else has to be written to see it. And the tag-1009 export-name rule in section 3 is
what the image loader needs to resolve a bitmap to its `Texture2D`.

**Coordinator — one merge decision, and it is the last thing between this and a visible menu.**
`DISHONORED_GFXUI_GFX3_RUNTIME` has to flip to 1 in the same commit that lets `GFxUI` call agent BC's
runtime: with it at 0, `gfxuigfx3absent.cpp` is compiled and its `GFxValue::ObjectInterface` bodies
collide with `gfx3.lib`'s real ones (28 `LNK2005`) and its `GRefCountImplCore` with `GTypes.h`'s (2
more). Agent BC reports all 25 `ObjectInterface` methods are defined now, which was agent BE's stated
precondition. Until then the rename block at the top of `gfxuirenderer.h` keeps both worlds linkable;
**delete it when the switch flips.**

Also for the merge: `GFxUI/Src/gfxuishaders.cpp` moved its three shader templates into
`gfxuirendererimpl.h` (which is where the PDB attributes their bodies) and kept all 50
`IMPLEMENT_SHADER_TYPE` lines plus `GetUIPixelShaderInterface2_RenderThread`. That last function is
now what pulls the unit into the link, so **agent BD's link anchor in `FogRendering.cpp` and the
`DISHONORED_WITH_GFXUI_SHADERS` define can go** — though the probe hook in `UnPlayer.cpp` currently
uses that define to know the module is in the build, so retire them together.

## 9. What remains in this area

1. **The engine-side call site.** Nothing constructs an `FGFxRenderer` outside the probe yet. Retail
   builds one per viewport in `FGFxEngine` and hands it to the movie player; that is
   `gfxuiengine.cpp` and `gfxuimovie.cpp`, not this file.
2. **The multi-pass blur composite** (section 6.1).
3. **`FGFxRenderer::GetStats`** stays empty: the Shipping build compiles the stat bodies out
   (`GStatBag` is `sizeof 1` in the PDB).
4. **`AddEventHandler`/`RemoveEventHandler`** keep no handler list. `GRenderer`'s own bodies do, for
   device loss; nothing in this tree registers one and the D3D9 RHI recreates its resources through
   `FRenderResource` instead.
5. **The 3D display path** (`MakeViewAndPersp3D`, the 3D branch of the transform, and
   `GRenderer::Adjust3DMatrixForRT`), unreachable today.

## 10. Files

Mine (5): `GFxUI/Inc/gfxuirenderer.h`, `GFxUI/Inc/gfxuirendererimpl.h`,
`GFxUI/Src/gfxuirenderer.cpp`, `GFxUI/Src/gfxuishaders.cpp` (the template move and the filter
lookup), and 10 lines in `Engine/Src/UnPlayer.cpp` (the probe hook), plus this report and
`agentCC_status.csv`. The snapshot list is `build/agentCC/snapshot_files.txt`.

**No build-system change was needed**: agent BB's `if(DISHONORED_WITH_GFX3)` block in
`GFxUI/Sources.cmake` already takes `gfxuirenderer.cpp` out of the exclude list, `gfxuishaders.cpp`
was already compiled for agent BD, and `DISHONORED_WITH_GFX3` is already on every target. Nothing in
`cmake/` is touched by this package.

Scratch (not repo tools): `build/agentCC/dec` + `dec2` (263 headless decompiles) and their pattern
files, `build/agentCC/egfx_enums.json` (the three `EGFx*` enums from the PDB),
`build/agentCC_release_build.cmd`, `build/agentCC_build*.log`, `build/agentCC_probe*.txt`,
`build/agentCC_regression.txt`, the two acceptance images and their .bmp originals in
`build/agentCC/`, and the snapshot worktree `build/agentCC_wt` + build directory `build/agentCC_rel`.

IDA: one own copy, `resources/docs/idb/shipping2012_agentCC.i64`, opened headlessly through
`resources/tools/ida/run.py` only; no MCP tool of any kind was used, and no FModel tool. No commits,
no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.
