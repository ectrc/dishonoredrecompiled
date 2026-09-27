# Agent DC — making the interface real: the engine seam, the display half, and the menu on screen (2026-09-27)

Package **DC** of `PHASE9.md`: five packages had reconstructed a complete Scaleform GFx 3.3 runtime —
the API (BB), the ActionScript machine (BC), the text engine (CB), the renderer (CC), the loaders (CD) —
and every one of them was verified in a harness. `DISHONORED_GFXUI_GFX3_RUNTIME` was 0 and `GFxUI` talked
to placeholder types. This package flips it and makes the game's own interface come up through it.

**It is up.** `build/agentDC/mainmenu.png` is the retail `UI_MainMenu.MainMenu` asset rendered by the
running game at 1280x720 under d3d9: real glyphs rasterised from the game's own `DefineFont3` outlines,
real cooked `Texture2D` art through the bitmap fill path, real shapes tessellated from the cooked
`DefineShape` records, laid out by the real AS2 machine. What is wrong with it, and why, is section 6.

## Result

| Accept | State |
|---|---|
| a screenshot of the game's own main menu, rendered in the running game | **`build/agentDC/mainmenu.png`**, 1280x720, d3d9, `-startmap=Dishonored_MainMenu`. Section 6 says exactly what is wrong with it |
| a census line from a real run | **yes**, one line, both halves: `movies open 1 [UI_MainMenu.MainMenu], drawn 1, display objects 182 (87 sprites, 66 shapes, 28 text fields, 51 bitmap fills), 68 draws, 194 triangles, 28 glyph batches / 157 glyphs, 0 masks, atlas 44 glyphs rasterised / 45 missed; machine: 1 frames advanced, 114 sprites created, 248 display objects placed, 84 action buffers, 3854 opcodes (0 unimplemented), 7 script errors` |
| `run_regression.py` 31 checks, 0 failures | **31 ok, 0 failed, 0 skipped, 443 s** (`build/agentDC_regression2.txt`) on `build/agentDC_rel`, plus a clean full RELEASE build with the flag on |
| `-newgame` still runs | **exit 0, 0 criticals**, `Initial startup: 2.92s`, both map changes (`build/agentDC_newgame.txt`) |
| 1:1 | 101 retail functions in `agentDC_status.csv` with their 2013 rvas: 64 ported, 29 partial, 8 named as remaining. Every deviation is at its site and in section 7 |

**Five defects were found by measuring, and four of them are the kind that make the interface do nothing
at all**: a refcount that starts at the wrong number, a missing arm in a ported native, a cached render
state released instead of set, and a texture matrix read in the wrong direction. Section 5.

## 1. What the switch actually needed

`DISHONORED_GFXUI_GFX3_RUNTIME=1` is one line. Everything else in this package is what that line then
requires, and the honest summary of the gap is: **the five runtime packages built a player, and the
engine seam that drives a player did not exist.** `GFxUI/Src/gfxuiengine.cpp` was a comment block listing
62 PDB functions and nothing else.

| File | Lines | What it is |
|---|---|---|
| `GFxUI/Inc/gfxuiengine.h` + `Src/gfxuiengine.cpp` | 240 + 1,290 | **`FGFxEngine`**: the loader, the renderer, the open-movie lists, `LoadMovie`, `StartScene`, `Tick`, `RenderUI`, the focus model, the input entry points, the file opener, the image loader and creator, the url builder. Plus `FGFxMovie` at retail's 104 bytes and `FAutoGFxValueArray` |
| `GFxUI/Src/gfxuiinteraction.cpp` | 140 | **`UGFxInteraction`**: the ten slots that make it the interface's per-frame owner |
| `External/GFx3/GFxLoaderImpl.{h,cpp}` | 95 + 360 | **`GFxLoader`'s non-virtual API** — `CreateMovie` and `GetMovieInfo`, the two functions the engine opens a movie with, which agent BC named as the one thing between the runtime and the engine (`agentBC.md` 6.6) — plus the loader-side state bag, the parse cache and the synchronous import bind |
| `External/GFx3/GFxDisplay.{h,cpp}` | 150 + 1,080 | **the display half**: `GFxDisplayContext`, the display-list walk with the stencil masks, the shape mesh and its tessellation, the fill styles, the glyph submission, and `GFxMovieRoot::Display` |
| edits | — | the switch in `gfxui_gfx3.h`; the `Display` virtuals on `GFxCharacter`/`GFxCharacterDef`; the state-bag chain on `GFxMovieDefImpl`; four defect fixes; the Engine→GFxUI edge in `UnPlayer.cpp`; agent BD's link anchor out of `FogRendering.cpp`; two units into `Sources.cmake` and two into `cmake/GFx.cmake` |

### The per-frame path, measured rather than assumed

`PHASE9.md`'s brief said retail drives the UI from `ADishonoredHUD::PostRender`. It does not.
`build/agentDC/xr.py` over the retail database gives the callers directly:

| retail function | called from |
|---|---|
| `FGFxEngine::Tick` (2013 0x57b140) | `UGFxInteraction::Tick` (0x57b790) |
| `FGFxEngine::RenderUI` (0x58dd30), `RenderTextures` (0x58e080) | `UGameViewportClient::Draw` (0x2c59a0), **twice** for RenderUI: `(bSceneColor, SDPG_Foreground)` and `(FALSE, SDPG_PostProcess)` |
| `FGFxEngine::SetRenderViewport` (0x57af00) | `UGFxInteraction::SetRenderViewport` and `::Send` on `CALLBACK_ViewportResized` |
| `FGFxEngine::InputKey/InputChar/InputAxis` | the matching `UGFxInteraction` slots |
| `FGFxEngine::GetEngine` (0x5a2b60) | `UGFxInteraction::Init`, `UGFxMoviePlayer::Load`, `UGFxMoviePlayer::SetPriority` |

So the advance is an Interaction and the draw is the viewport client; `ADishonoredHUD::PostRender` is the
**canvas** HUD and a sibling of the `RenderUI` call, not its caller. That is what this port follows.

## 2. Evidence

* **56 + 15 + 2 headless Hex-Rays decompiles** on my own database copy
  (`resources/docs/idb/shipping2012_agentDC.i64`, opened through `resources/tools/ida/run.py` only):
  `build/agentDC/dec`, `dec2`, `dec3`, `dec4`. **No MCP tool of any kind was used, and no FModel tool.**
* **`build/agentDC/xr.py`** — the caller lists above, read out of the database rather than guessed.
* **The reference engine's GFx 4 `ScaleformEngine.{h,cpp}`**, which is `FGFxEngine` in the shape Epic
  shipped: the port is the retail decompile where the two differ and the reference where retail is
  inlined away (`CollapseRelativePath`, `FlushPlayerInput`, `InsertMovieIntoList`).
* **`resources/docs/symbols/match_2012_2013.csv`** for every 2013 rva in `agentDC_status.csv`.
* **`build/agentDC/sym.py`** and a link-map resolver: the two crashes that stopped the first drawn frame
  were located by mapping the crash address back through the build's `DishonoredGame.map`. The image is
  `/DYNAMICBASE`, so a base probe (`DishonoredGFxRenderUI at %p`) is logged at startup and the delta
  applied. That is worth keeping: it turns "Address = 0x1abcb88" into
  `FD3D9DynamicRHI::SetStencilState+0x18` in one command.

## 3. The display half, which nobody owned

Agent BC left `GFxMovieRoot::Display` empty and said so (`agentBC.md` 6.9). Agent CB stopped one call
short of the renderer and wrote out the contract (`agentCB.md` 9). Agent CC built the whole 54-slot
`GRenderer` and said the characters were not its package (`agentCC.md` 5). **The loop between them did not
exist**, and without it the runtime and the renderer could not meet. It is `GFxDisplay.cpp`:

```
GFxMovieRoot::Display            0xa07aa0  render config -> visible frame rect -> BeginDisplay ->
                                           level walk -> EndDisplay
GFxDisplayList::Display          0x9d56c0  depth order; ClipDepth opens a stencil mask
GFxSprite::Display               0x9faeb0  PreDisplay (matrix x cxform) -> children -> PostDisplay
GFxGenericCharacter::Display     0x9cfdd0  -> the definition
GFxShapeCharacterDef::Display    0xa3cf40  the cached mesh, one DrawIndexedTriList per style group
GFxImageCharacterDef::Display              a bitmap placed straight on the timeline
GFxButtonCharacterDef::Display   0xa69000  the up state's records
GFxEditTextCharacter::Display    0xa2ed90  CB's line walk with the DrawBitmaps submission put back
```

Three things about it are worth stating because they were not obvious and cost measurements:

1. **The renderer only takes 16-bit integer positions.** `EGFxVertexDeclarationType` is
   `{None, Strip, Glyph, XY16iC32, XY16iCF32}` and there is no declaration for
   `GRenderer::Vertex_XY32f`, because GFx's meshes are in *twips* and a 1280x720 frame is 25600x14400
   twips, inside `int16`. The first version submitted floats with `Index_None` and a null index pointer;
   `RHIDrawIndexedPrimitiveUP` read address zero.
2. **`GFxMovieRoot::Display` owns `BeginDisplay`/`EndDisplay`, not the engine.** The retail body
   (0xa07aa0) calls the renderer's slot 15 with the background colour, the viewport and the four floats
   of the *visible frame rect* — which is the movie's own pixel range that maps to the viewport, and
   which `SM_ShowAll` widens on the long axis rather than letterboxing. `RenderUI` only binds the target
   and calls `pView->Display()` (slot 38, the `vtable + 152` in its decompile).
3. **The root matrix is twips to the movie's own pixels**, i.e. the 0.05 scale and nothing else: the
   viewport matrix that maps those pixels to the viewport is `SetUIViewport`'s, and `BeginDisplay` draws
   its background quad with `SetMatrix(identity)` in exactly that space, which is the proof.

## 4. The two harness gaps this closed

* **The fonts were never registered with the font manager.** Agent CB's hand-over 2 and agent CD's
  hand-over 3 both name it and neither package took it: `GFx_DefineFontLoader` leaves a
  `GFxFontCharacterDef` in the dictionary, and what a text format resolves is a *named*
  `GFxFontResource` in `GFxTextGetFontManager()`. `GFxLoaderImpl::registerFonts` calls CB's own
  `GFxFontLoadFromPayload` on every payload the loader reads, including every import, so the fontlib's
  two fonts are registered by the time a text field formats. Without it every text field in the game
  lays out with no font and produces no glyph.
* **The import binding needed a real file opener.** `FGFxFileOpener::OpenFile` turns
  `/ package/UI_MainMenu/MainMenu` back into a package path and hands the runtime a memory file over a
  cooked `USwfMovie`'s `RawData`, which is exactly what the retail `FGFxFile` layout says retail does.
  All 10 of the menu's imports bind through it in the game, the same 10 the harness binds from a
  directory.

## 5. Five defects, every one found by measuring

### 5.1 `GFxResource`'s refcount starts at zero (a use-after-free, and the first in-game crash)

`GFxResource` (`GFx3Gen.h`) carries `GAtomicInt<long> RefCount` and `GAtomicInt`'s default constructor
writes **0**. `FGFxEngine::LoadMovie` assigns the movie definition to a `GPtr` (AddRef → 1) and drops its
own reference (Release → 0), and the definition was deleted under its creator; `CreateInstance` then ran
on freed memory and jumped through a null vtable. 0.19 s of work and then `Address = 0x0`.

This is **the third** instance of the class of defect agent CC named (`agentCC.md` 4.1): every
`GRefCountBase` in `GTypes.h` starts at 1, and the three classes in `GFx3Gen.h` that carry their own
`GAtomicInt RefCount` — `GTexture`, `GRenderTarget`, `GFxResource` — do not. CC fixed the first two in
the `FGFx*` constructors; `GFxResource` has no engine-side constructor to fix it in, so it now has one:
`GFxResource() : pLib(0) { RefCount = 1; }`. Retail's writes 1 there too — every `FGFxRenderer::CreateTexture`
(2012 0x5c47d0) does the same thing one class down.

### 5.2 `UGFxMoviePlayer::PreLoad` had only one of its two arms (every cook movie failed to open)

Retail's `PreLoad` (2012 0x5e3a30) branches on `MovieInfo->Outer->Outer`: with a group it builds
`"<package>.<group>.<name>"`, without one `"<package>.<name>"`. The ported body had only the first and
returned `FALSE` otherwise — and **every UI movie in the retail cook has no group**: `UI_MainMenu.MainMenu`'s
outer is the package and the package's outer is null. So `Start()` answered FALSE, `pMovie` stayed null,
and the interface had nothing to draw. Found by opening the menu; fixed with the decompile's own two arms.

### 5.3 The cached stencil state was released instead of set (the render thread died on the first drawn frame)

`FGFxRenderer::InitUIBlendStackAndMiscRenderState_RenderingThread` pushed
`TStaticStencilState<>::GetRHI()` to the device and then called `CurStencilState.SafeRelease()`.
`CheckRenderTarget_RenderThread` re-asserts the state with `RHISetStencilState(CurStencilState)` on every
render-target change, and `FD3D9DynamicRHI::SetStencilState` dereferences its argument with no null check
(the reference engine's own body, so retail's too). The first frame the game drew a movie through the
real path took the null: `Rendering thread exception` at `FD3D9DynamicRHI::SetStencilState+0x18`,
resolved against the link map. The cached state now tracks the one just pushed, and
`CheckRenderTarget_RenderThread` guards it — because the *first* `CheckRenderTarget` of a frame runs from
inside `InitUIBlendStack...` itself, before that function has set anything. Retail cannot reach the null
either; what keeps it out is not in the decompile of 0x5d7e10 or 0x5d7c80, and the guard says so.

### 5.4 The bitmap fill's matrix, read three ways before the numbers were looked at

51 of the menu's bitmap fills resolved to a real cooked `Texture2D` and every one painted a flat colour.
`GRenderer::FillTexture::TextureMatrix` maps a **vertex position to 0..1** (agent CC's probe builds that
shape and the checkerboard it produced is the proof). The style's own matrix maps the **image's pixels
onto the shape's twips and already carries the twips factor**, which two of the three readings get wrong
and both of the wrong ones sample inside one texel — a flat quad, not a distortion, which is why reading
was not enough. The numbers settle it (`GFx fill N:` lines, `-gfxuiloadtrace`):

```
fill 1: matrix [20 0 0 / 0 20 0]          bounds [0 0 10240 10240]       image 512x512
fill 2: matrix [20 0 -7967 / 0 20 -548]   bounds [-7967 -548 2273 412]   image 512x48
```

Fill 2's inverse maps (-7967,-548) to (0,0) and (2273,412) to (512,48) exactly. So the texture matrix is
`diag(1/w, 1/h) * inverse(style matrix)`, with **no second factor of 20**.

### 5.5 A file-scope `ParseParam` is always FALSE (avoided, and worth repeating)

`PHASE9.md`'s rule and agent CA's measurement. Every switch this package adds — `-nogfxui`,
`-gfxuimenu`, `-gfxuishot`, `-gfxuicensus`, `-gfxuiloadtrace`, the three bisect switches — is read on
first use, never as a file-scope static.

## 6. The screenshot, and exactly what is wrong with it

`build/agentDC/mainmenu.png`. Taken on the 30th frame the interface was drawn on, at
`-startmap=Dishonored_MainMenu -gfxuimenu=MainMenu -gfxuishot=30`, d3d9, windowed 1280x720.

**What is right.** Every character of text is a real glyph, rasterised by agent CB's rasteriser from the
game's own `ChaletComprime-CologneEighty` `DefineFont3` outlines, packed into the alpha-only atlas and
submitted through `DrawBitmaps` — the alignment, the kerning and the baseline are the text engine's, and
"_Adjust brightness until you can barely see the first logo" is the asset's own string laid out by the
real layout engine. The panels are real tessellated `DefineShape` geometry in the right places. The
bottom-right panel is real cooked `Texture2D` art through the bitmap fill path with the correct matrix.
The whole frame is 182 display objects the AS2 machine placed by running 3,854 opcodes with **none
unimplemented**.

**What is wrong, and why:**

1. **The text says "Text".** That is the asset's authoring placeholder content, not a defect: retail
   fills those fields from the localisation tables through `GFxTranslator`/`GFxFontMap`, which
   `FGFxEngine::InitLocalization` installs and which this package does **not** reconstruct. Until a
   translator is in the state bag every `DefineEditText` shows its `InitialText`.
2. **The menu's own background and logo are missing.** The frame shows the panels and the chrome, not the
   painting. Those live on the `m_StartScreen` clip, which the content reveals from its own script in
   response to the `Open` call the movie player's `PostStart` makes — and `UDisGFxMoviePlayerMainMenu::PostStart`
   is not ported (it is `DisGFxMoviePlayerMainMenu`'s, not this package's). The seven script errors in
   the census are that: `Open`, `SetMenu`, `InitHelpBar` and their kin, i.e. the content's own methods on
   clips whose class the engine has not driven yet. Agent BC measured the same seven in the harness.
3. **Two text fields are drawn rotated.** `CHAPTER NAME` and `NAME` are rotated by the asset (they are
   vertical tabs), and the rotation is the composed matrix working; what is wrong is that they are
   *visible at all* on the start screen, which is item 2 again.
4. **Half the glyphs miss the atlas** (`44 rasterised / 45 missed`). The atlas is one 1024x1024 texture
   and `GFxGlyphSlotQueue`'s LRU extrusion is not ported (agent CB's deviation 4), so once it fills it
   refuses rather than evicting. At the menu's sizes it fills.
5. **No masks ran** (`0 masks`). The menu asset places no `ClipDepth` entry in its first frames; the
   stencil path is implemented and agent CC's probe proves it draws, but this frame does not exercise it.

## 7. The deviations, stated once

1. **The tessellator.** Retail tessellates with `GTessellator` (2012 0xabdbb0..0xac5930, about sixty
   functions: a sweep-line monotone decomposition with intersection events, coherent-curve triangulation
   and `GFxEdgeAAGenerator`'s antialiasing pass) and caches a `GFxMeshSet` per scale. This decomposes
   into trapezoids — bands at the union of the vertex Y values and the edge-edge intersection Y values,
   with the non-zero winding rule, which is SWF's own — and caches one mesh per definition in shape-local
   twips. Same closed region, same fill styles; not retail's triangle count, ordering, edge antialiasing
   or per-scale retessellation. **This is the largest single remaining 1:1 item in the area.**
2. **Strokes** are a quad per segment at the style's width. `GStrokerAA::Tessellate` (0xacaba0) and
   `GFxCachedStroke::Display` (0xab36e0) — joins, caps and miter — are not ported.
3. **Gradient fills** paint with the middle gradient stop's colour.
   `GFxFillStyle::GetGradientFillTexture` (0xa90600) builds a gradient texture through
   `GFxGradientParams`; that is not generated here. The menu has 0 gradient fills.
4. **`RenderUI` never renders into scene colour.** Retail's first of the two calls passes the
   post-process flag and, when it is set, the command begins rendering scene colour and hands the
   scene-colour proxy and the scene depth proxy to the render target (2013 0x586780). `FSceneRenderTargets`
   lives in `Engine/Src`, which is private to the Engine module, and this wave's ownership rules put
   `SceneRendering` out of reach. Both calls therefore pass FALSE and bind the viewport. **The consequence
   is the same image for a HUD and differs only for a post-process effect that reads the UI.**
5. **`FGFxURLBuilder::BuildURL` is a reported no-op.** `GTypes.h`'s `GString` is the PDB's four-byte
   handle with `ToCStr` and `GetSize` and **no allocator**, so a `GString` cannot be built. Nothing in
   this runtime calls the state: `GFxLoaderImpl::ResolveImportMovie` applies the same rule with its own
   buffer, because the import binding needs it before any state bag exists.
6. **`FGFxImageInfo::FileName` is an `FString`, not a `GString`** — same reason, and it is the only
   member of that class that is not the PDB's.
7. **The glyph cache is process-wide** and the whole atlas is re-uploaded when it grows.
   `GFxGlyphRasterCache::UpdateTextures` (0xa4db30) uploads the dirty rectangle; retail hangs the cache
   off a `GFxFontCacheManager` state. The configuration applied is retail's, read out of 0x590ba0:
   one 1024x1024 texture, 2-pixel padding, 48-pixel slots, a 128x256 update block, dynamic cache on,
   maximum raster scale 1.25.
8. **`-gfxuimenu` exists at all.** In retail the main menu is created by `UDisGlobalUIManager` from its
   config movie set, and the `DisUI.ini` set names the HUD, the power wheel, the journal, the note, the
   pause menu and the mission stats but **not** the main menu, which the game's own UnrealScript
   constructs. Measured: a run at `-startmap=Dishonored_MainMenu` streams the menu map in, renders
   **34,650 frames** and never constructs a `UGFxMoviePlayer`. The switch does what the script would —
   find the class by name, point `MovieInfo` at the cooked `USwfMovie`, call `Start()` — and everything
   after that is the real path with no shortcut in it. **The interface is therefore opt-in today, which
   is also what keeps the regression harness untouched.**
9. **`GFxMovieDefImpl::CreateInstance` does not run frame 1** for `bInitFirstFrame`. The movie has no
   viewport at that point (`StartScene` sets it immediately after) and the engine's own `Tick` advances
   it on the next frame; the cost is that a `PostStart` which reads an AS2 variable sees it one frame
   later.
10. **`FGFxEngine`'s layout is not reproduced.** It is 0x20C bytes in retail (`GetEngine`'s
    `appMalloc(0x20C, 8)`) and nothing serialises it, so the members are the ones the 62 bodies touch in
    the order the constructor writes them, with no offset assertions. `FGFxMovie` **is** reproduced, at
    retail's 104 bytes (`LoadMovie`'s `appMalloc(0x68, 8)`), and the offsets the decompiles read —
    `pView` at 52, `fVisible` at 68, `fUpdate` at 72, `pUMovie` at 92 — all fall out of the member order.

## 8. One defect that is open, with its stack

**The machine crashes after about eighty movie frames in the game, and does not in the harness.**
`GFx3Run --run Dishonored_MainMenu.MainMenu.gfx --frames 200 --imports … --platform PC` exits 0. The same
asset in the game dies at ~2.5 s of movie time. The stack, resolved against the link map:

```
0x70003                                   <- the wild jump
GFxSprite::AddDisplayObject      +0x1da   (GFxPlayerSprite.cpp)   def->CreateCharacterInstance
GASExecuteTag::ExecuteWithPriority +0x9
GFxSprite::ExecuteFrameTags      +0x6b
GFxSprite::AdvanceFrame          +0xac    (a child)
GFxSprite::AdvanceFrame          +0xf8    (its parent)
GFxMovieRoot::Advance            +0x88
UGFxMoviePlayer::Advance         +0x32
FGFxEngine::Tick                 +0xfb
```

So a character definition the dictionary still points at has been freed, or a dictionary entry has been
overwritten, on a frame the timeline loops. It is reachable only past the five frames every harness run
uses, and `0x70003` has the shape of a `GFxResourceId` rather than of a heap address, which is a lead.
**It does not affect the regression harness** (no movie is open without `-gfxuimenu`, so `RenderUI`
early-outs) and it does not affect the screenshot, which is taken on drawn frame 30. It is the first
thing the next package in this area should take, and `GFx3Run` with a `--frames 400` loop plus an
interleaved `Advance(0)` is very likely to reproduce it in the fast harness.

## 9. Verification

* **The whole GFx3 directory still compiles with no engine at all** (`build/agentDC_run.cmd`, VS 2022
  x86, `/Zp4`): 0 errors. The two new units include no engine header, and the image binding goes through
  a function pointer the host installs for exactly that reason.
* **Isolated full RELEASE build**: `make_snapshot.py DC` (HEAD `a42f4a3` + my 30 files) built with
  `build/agentDC_relbuild.cmd` into `build/agentDC_rel`: **0 errors, 0 link errors**, all four targets.
  The snapshot is used rather than the shared tree because package DB is adding 400 lines of Arkane
  post-process configuration to `UGameViewportClient::Draw` that depend on `EngineClasses.h` changes it
  has not landed, so the shared `UnPlayer.cpp` does not compile; this package's four edits to that file
  are applied to the snapshot's HEAD copy by `build/agentDC/patch_snapshot_unplayer.py` and are in the
  shared tree as well.
* **Regression**: `run_regression.py --build-dir build/agentDC_rel --no-build` →
  **31 ok, 0 failed, 0 skipped, 443 s** (`build/agentDC_regression2.txt`). CoreSmoke 99/0, layout 2,314
  types with 0 mismatches and 0 contract failures, nullrhi 0 criticals, d3d9 **20,190 frames / 0
  criticals** / 6,506 draw elements / 462 visible primitives / `unported_natives 0`, inputtest the pawn
  walking **1,029.8** units at a 500.4 peak with 1,369 PhysX actors, 308 touch begins, 87,171 sequence
  ops and 0 criticals. The first full run (`build/agentDC_regression.txt`) reported
  `d3d9/unported_natives 1` — `ADishonoredPlayerController::execDis_Lean_Toggle`, a player-controller
  input native nothing in this package touches — and a re-run of that stage alone gave
  **9 ok, 0 failed** with `unported_natives 0` (`build/agentDC_regression_d3d9.txt`), so it is a stray
  key event in a 90-second unattended run rather than a regression.
* **`-newgame`**: exit 0, `Initial startup: 2.92s`, both map changes, **0 criticals**
  (`build/agentDC_newgame.txt`).
* **The interface itself**: `build/agentDC/shot.py` runs the menu map with the movie open and converts
  the engine's own screenshot; the run log is `agentDC_shot.log` in the retail log directory. The census
  line of the result table is from it.
* **The harness is undisturbed**: `GFx3Run --run MainMenu --frames 5 --imports build/agentBB/gfx
  --platform PC` still reports agent CD's numbers exactly — 292 dictionary entries, 0 placeholders, 10 of
  10 imports bound, 12 text fields with a font, 3,914 opcodes with none unimplemented, 7 script errors.

## 10. Hand-overs

**Input — this is the next package and it is small.** Everything up to the movie's door is here and
measured: `UGFxInteraction::InputKey/InputChar/InputAxis` (2013 0x591f10 / 0x591fd0 / 0x595140) route to
`FGFxEngine::InputKey` (0x591470), which consults the focused movie, asks the movie player's script
filter (`FilterButtonInput`, which `UDisGFxMoviePlayerBase` overrides), maps the Unreal key name to a
`GFxKey::Code` and delivers a `GFxKeyEvent` to `pView->HandleEvent`. **`GFxMovieRoot::HandleEvent` answers
`HE_NotHandled` from state**, because the button and focus model is not ported: `GFxButtonCharacter`
(38 retail functions) plus `GFx_GenerateMouseButtonEvents` (2012 0xa66a90), which agent BC named as not in
its wave. Port those two and the menu becomes operable, because the keys already arrive. Two smaller
pieces go with it: `GFxDisplayList::SwapDepths` (0x9d5c10) and the `Key`/`Mouse` AS2 classes (21 retail
functions, `agentBC.md` 4).

**The in-game HUD.** `UI_HUD_SF.HUD` is resident on the first mission map and opens through exactly the
same path: `-gfxuimenu=HUD` on a mission map will load and draw it today. Three things it needs that the
menu did not: the **atlas sub-image** path (483 of the cook's 1,015 images are `BaseImageId` + `SubRect`
rectangles of a packed atlas, and `GFxImageCharacterDef::GetTexture` resolves the base image but the UV
offset into the atlas is **not** applied — the menu has 0 sub-images so nothing forced it); the
**render-texture** movies (`FGFxEngine::RenderTextures`, 2013 0x58e080, which walks the list and reports
rather than drawing); and `UDisGFxMoviePlayerHUD`'s own natives. The HUD is also where the glyph atlas's
missing LRU (`agentCB.md` deviation 4) will start to bite, because a HUD cycles through sizes.

**Whoever regenerates `External/GFx3/GFx3Gen.h`.** Two things in it are now hand-edited and a
regeneration deletes them silently: `GFxLoader`'s six non-virtual declarations (the engine's entry points
— there is a comment saying so at the site) and `GFxResource`'s constructor (defect 5.1). The generator
`build/agentBB_gen_gfx3.py` must learn both before it is re-run.

**Agent CC — two corrections to your hand-over, both measured.** (1) "Delete the rename block in
`gfxuirenderer.h` when the switch flips" is half right: the `GRefCountImplCore` rename retires with the
switch and is gone, but the two **enum** renames (`GFxRenderTextureMode`, `GFxTimingMode`) do not. They
solve a collision between two *generators* — `GFxUIEngineShims.h` from `gen_classes_header.py --sdk` and
`GFx3Enums.h` from the 2012 PDB — and neither has stopped emitting them, so deleting them breaks the
build with or without the runtime. `gfxui_gfx3.h` now reaches the reconstruction **through**
`gfxuirenderer.h` so the rename is applied exactly once per translation unit. (2) `DISHONORED_WITH_GFXUI_SHADERS`
is **kept**: agent BD's link anchor in `FogRendering.cpp` is retired as you asked, but that define is the
only signal Engine has that the GFxUI module is in the build, and it now guards the three-function
Engine→GFxUI edge instead.

**Coordinator.** (1) `DISHONORED_GFXUI_GFX3_RUNTIME` follows `DISHONORED_WITH_GFX3` in
`gfxui_gfx3.h` rather than coming from cmake, deliberately: three `DishonoredGame` units include that
header too, and a per-target define could have given them a different `FGFxEngine` from `GFxUI`'s — an
ODR violation that links. (2) `gfxuiengine.cpp` and `gfxuiinteraction.cpp` come out of `Sources.cmake`'s
exclude list, and `GFxLoaderImpl.cpp` and `GFxDisplay.cpp` join the `gfx3` library in `cmake/GFx.cmake`;
both files already carry CB's and CD's blocks and all of them have to survive the merge. (3) `middleware.md`
2.3 should gain the line that the reconstruction now draws the game's own interface in the game, with the
census of the result table. (4) The open defect of section 8 wants a package; so does `GTessellator`.

## 11. Files

Mine (34): the four new `External/GFx3` units (`GFxLoaderImpl.{h,cpp}`, `GFxDisplay.{h,cpp}`); edits to
`GFx3Gen.h`, `GFx3Support.cpp`, `GTypes.h`, `GFxValue.h`, `GFxPlayer.h`, `GFxPlayerData.cpp`,
`GFxPlayerRoot.cpp`, `GFxCharacterDefs.{h,cpp}`, `GFxTextField.h`; `GFxUI/Inc/{gfxui_gfx3.h,
gfxuiengine.h, gfxuirenderer.h, gfxuiimageinfo.h, CppText/UGFxInteraction.h}`;
`GFxUI/Src/{gfxuiengine.cpp, gfxuiinteraction.cpp, gfxuirenderer.cpp, gfxuimovie.cpp,
gfxuiexternalinterface.cpp, gfxuiimageinfo.cpp, gfxuifile.cpp}`; `GFxUI/Sources.cmake`;
`DishonoredGame/Src/{disgfxmovieplayerbase.cpp, disgfxmovieplayermenubase.cpp}`;
`Engine/Src/{UnPlayer.cpp, FogRendering.cpp}`; `cmake/GFx.cmake`; plus this report and
`agentDC_status.csv`. The snapshot list is `build/agentDC_files.txt` (`UnPlayer.cpp` is deliberately not
in it — section 9).

Scratch (not repo tools): `build/agentDC/dec{,2,3,4}` (73 headless decompiles) and their list files,
`build/agentDC/xr.py` (the caller lists), `build/agentDC/sym.py` (dbghelp address resolution),
`build/agentDC/patch_*.py` (the CRLF-safe patch scripts), `build/agentDC/mkstatus.py`,
`build/agentDC/crlf.py`, `build/agentDC/shot.py` (run the menu and convert the screenshot),
`build/agentDC/mainmenu.png` + `mainmenu_first.png` (the accept image and the first drawn frame, before
the fill matrix was corrected), `build/agentDC_run.cmd` (the no-engine build),
`build/agentDC_relbuild.cmd`, `build/agentDC_regression{,2,_d3d9}.txt`, `build/agentDC_newgame.txt`,
`build/agentDC/run*.txt`, snapshot `build/agentDC_wt` and build directory `build/agentDC_rel`.

Switches this package adds, all read on first use: `-gfxuimenu[=<Movie>]`, `-gfxuimenuclass=<Class>`,
`-gfxuishot=<N>` (the Nth **drawn** frame), `-gfxuicensus`, `-gfxuiloadtrace`, `-nogfxui`, and the four
bisect switches `-gfxuinoshapes`, `-gfxuinotext`, `-gfxuinoimages`, `-gfxuinodisplay` (plus
`-gfxuinobind` and `-gfxuifitfill`), which are how both render-thread crashes and the fill matrix were
localised and which are worth keeping for the next one.

IDA: one own copy, `resources/docs/idb/shipping2012_agentDC.i64`, opened headlessly through
`resources/tools/ida/run.py` only; **no MCP tool of any kind was used, and no FModel tool.** No commits,
no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.
