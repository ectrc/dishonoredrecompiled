# Agent EX (PHASE12 EX) — the stage was never wrong; the window was, and one enum value was

Branched from HEAD `e2d7e5d`, own worktree `build/agentEX_wt`, own build dirs
`build/agentEX_wt/build/agentEX_rel` (this package), `build/agentEX_wt/build/agentEX_reg` (the regression
harness), `build/agentEX_wt/build/agentEX_clean` (the clean gate build). Own IDA copies
`build/agentEX_ida/retail2013_agentEX.i64` and `shipping2012_agentEX.i64`, headless only. **No FModel tool was
used.** No commits, nothing staged, nothing written into the main checkout.

## The answer in nine lines

* **The brief's leading hypothesis is false and I can show it is false.** The stage-to-viewport mapping is
  exactly right: stage `1280x720`, `SM_ShowAll`, viewport `1280x720`, visible rect `0,0..1280,720`, `_root`
  matrix identity, viewport matrix `2/1280` and `-2/720`. Measured with a trace inside
  `GFxMovieRoot::Display` and printed on every change. Nothing in GFx magnifies anything. Section 2.
* **"All of the content at a lower resolution in the same window size" is the desktop, not the renderer.**
  This display runs at 125 % scaling (`LOGPIXELSX 96`, `HORZRES 6144`, `DESKTOPHORZRES 7680`). Our executable
  is DPI-unaware, so DWM renders the window at the size the game asks for and then **bitmap-scales it by
  1.25 with a filter**: asked for a `1280x720` client, the physical client measures `1600x900`; asked for
  `1600x900`, it measures `2000x1125`. Retail is sharp on this machine because
  `HKCU\...\AppCompatFlags\Layers` carries `HIGHDPIAWARE` for
  `...\Dishonored\Binaries\Win32\Dishonored.exe` — a per-executable override the user set once, which no
  agent's freshly-named `DishonoredGame_<XX>.exe` inherits. Sections 1 and 3.
* **That is measurable in the user's own screenshots.** Our half of `first.png` and `second.png` carries a
  clean **period-5 modulation of the gradient magnitude in x and y** — the fingerprint of a 5:4 resample —
  at 0.089 / 0.092 and 0.046 / 0.044; retail's halves are flat at 0.011 / 0.007 and 0.015 / 0.005. After the fix ours reads
  0.005 / 0.015 and 0.017 / 0.009. Section 3.
* **The fix is one block in `Launch/Src/Launch.cpp`**: declare the process DPI-aware in
  `SetupWindowsEnvironment` before any window exists. **This is the one file outside `External/GFx3` and
  `GFxUI`.** Retail's binary declares nothing, so this is a deliberate, stated deviation: it reproduces in
  the process what the compatibility layer gives retail, without depending on a registry entry per exe name.
* **The wrong colour is real, is one wrong enumerator, and retail's binary settles it.**
  `FGFxRendererImpl::GetUIVertexDecl_RenderThread` gave the GFx vertex declarations' colour element
  `VET_Color`; retail 2013 `0x5762c0` (2012 `0x5babe0`), decompiled by hand, holds element type **7** in all
  four cases, and `VET_UByte4N` is 7 while `VET_Color` is 8 in both our `EVertexElementType` and retail's own
  (`all_types.h`). The file's own comment named 7 as `VET_Color` and the code followed the name. Section 4.
* **`VET_Color` is `D3DDECLTYPE_D3DCOLOR` and exchanges red and blue** relative to `GColor`'s memory order
  for these shaders. Measured with a probe that forced every glyph vertex to `GColor(255,0,0,255)`: the
  glyphs drew **pure blue**. `PRESS ANY KEY` drew `(214,242,227)` on a colour the display list submitted as
  `(227,242,214)`, which is retail's exact pixel value. After the change ours is `(227,242,214)`, the menu
  bar labels likewise. Section 4.
* **The fonts are not wrong, at least on these two screens, and that is now directly observable.** A trace in
  `GFxTextDocView::FindFont` logs every distinct (requested list → resolved font, by which route) pair. The
  start screen and the main menu produce exactly two, both through the format's own resolved handle — the
  movie's embedded fonts — and **no field takes the fallback**: `'ChaletComprime-CologneEighty'` and
  `'Emerge BF'`. The glyph outlines are pixel-for-pixel retail's. Section 5.
* **Sharpness: the glyph stem is now retail's exactly.** At 1:1 a stem reads background, one antialiased
  pixel, a flat plateau at the text colour, one antialiased pixel, background — the same shape retail's does.
  Before, the same stem was a seven-pixel bell that matched retail's convolved with a 1.25x magnification
  kernel to within 2 %. **`GTessellator` and the glyph cache were measured and are not the cause**; nothing
  in this package touches either. Section 3.
* **Regression 37 ok, 0 failed, 0 skipped**, built inside the harness; clean full Release build with the
  directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: 0 errors, 0 C4263, 0 C4264; `rva_sweep` over
  the whole tree 7302 citations with 2 known pre-existing suspects and 1 pre-existing unknown, **none of
  them in a line this package wrote**. Section 8.

## 1. What the user was actually comparing

The user ran the staged build beside retail at what looked like the same window size and reported that ours
"seems to be shown at a lower resolution even though its the same assets and the same window size". Their own
log (`Logs/Launch720b.log:7`) shows the run was `-ResX=1280 -ResY=720 ... -windowed`, and the two client
areas in `first.png` and `second.png` both measure **1600 px wide** in the screenshot.

Those two facts are consistent only if something between the game and the screen is magnifying. It is DWM.
Measured on this machine, in this session:

| | asked for | physical client `GetClientRect` reports to a DPI-aware caller |
|---|---|---|
| HEAD `e2d7e5d` | `-ResX=1280 -ResY=720` | **1600x900** |
| HEAD `e2d7e5d` | `-ResX=1600 -ResY=900` | **2000x1125** |
| agent EX | `-ResX=1280 -ResY=720` | 1280x720 |
| agent EX | `-ResX=1600 -ResY=900` | 1600x900 |

`GetDeviceCaps`: `LOGPIXELSX` 96, `HORZRES` 6144, `DESKTOPHORZRES` 7680 — 7680/6144 = 1.25. A DPI-unaware
process is shown a 6144-wide desktop and every one of its windows is composited to the real 7680-wide one
through a bilinear stretch.

**Retail is not stretched**, and the reason is not in retail's binary: a byte scan of
`Dishonored.exe`, `DishonoredGame.exe` and `DishonoredGame-Win64-Shipping.exe` finds no `dpiAware` manifest
entry, no `SetProcessDPIAware` and no `SetProcessDpiAwareness`. The reason is in the registry:

```
HKCU\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers
  C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32\Dishonored.exe  => HIGHDPIAWARE
  D:\DishonoredDebug\Binaries\Win32\Dishonored.exe                                        => HIGHDPIAWARE
  D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\DishonoredGame.exe          => HIGHDPIAWARE
  D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\DishonoredGame_X.exe        => HIGHDPIAWARE
  D:\RecompileDishonored\Dishonored_Latest2026\Binaries\Win32\DishonoredGame_CC.exe       => HIGHDPIAWARE
```

Three of our own executable names are in that list and the rest are not, which is exactly the shape of a
setting applied by hand, twice, long ago. `stage_retail.py` gives every agent a new exe name, so every
agent's build since has been measured through a 1.25x bitmap stretch that retail was not measured through.

**This is worth more than the fix.** Every screenshot comparison this project has made between our build and
retail on this machine has been between a magnified window and a native one.

## 2. The stage, measured, and why the brief's hypothesis does not survive it

`GFxMovieRoot::Display` now logs the stage-to-viewport mapping whenever it changes. One line, for the whole
menu, at every window size tried:

```
GFx stage map: movie 1280x720 scaleMode 1 align 0 viewport buf 1280x720 rect 0,0 1280x720
               visible 0,0..1280,720 root [1 0 0 / 0 1 0]
GFx stage map: movie 1280x720 scaleMode 1 align 0 viewport buf 1600x900 rect 0,0 1600x900
               visible 0,0..1280,720 root [1 0 0 / 0 1 0]
```

scaleMode 1 is `SM_ShowAll`, the movie's authored stage is 1280x720, the viewport is the whole window, the
visible frame rect is the whole stage, and `_root`'s matrix is the identity. `FGFxRenderer::BeginDisplay`
then builds a viewport matrix of `2/1280` and `-2/720` — logged and checked — which maps stage pixel 0 to
NDC -1 and stage pixel 1280 to +1.

The five candidates the brief listed, and what each measured:

| candidate | measured | verdict |
|---|---|---|
| authored stage vs viewport, and the scale mode | stage 1280x720, viewport = the window, `SM_ShowAll`, visible rect = the whole stage, viewport matrix exactly 2/W and -2/H | **correct; not the cause** |
| rendered to a smaller offscreen target and upscaled | the GFx layer draws into `HudRenderTarget`, whose resource is the game viewport; `scene rendered ... 1280x720` and `presented frame (1280x720, windowed)` agree with the requested size, and a per-parity test on the frame shows no 2x structure | **not the cause** |
| edge antialiasing / the unported `GTessellator` | the window was measured 1.25x larger than the back buffer, so every pixel in it was magnified whatever drew it; and the softening was the same on the logo **bitmap** as on the **text**, which no tessellator can cause | **not the cause; still unported, still not needed for this** |
| the glyph cache rasterising small and scaling up | the glyph batch's own screen rectangle, logged at submission, matched the pixels 1:1 once the window stopped being stretched; the stem profile became retail's exactly with no change to the cache | **not the cause** |
| the UI texture filter and mip bias | `GetSamplerState` already selects `SF_Trilinear` / `MIPBIAS_HigherResolution_13` per retail; changing nothing here, sharpness matched | **not the cause** |

The change rests on **none of them**. It rests on the window being 1.25x larger than the back buffer.

## 3. The measurement that found it

A bilinear resample by a rational factor p/q leaves a period-q modulation in the mean absolute gradient as a
function of column (and row) index. Computing the coefficient of variation of that mean over x mod k:

| image | k=2 | k=3 | k=4 | **k=5** | k=6 | k=10 |
|---|---|---|---|---|---|---|
| ours, `first.png` left half | .003 | .009 | .005 | **.089** | .016 | .090 |
| retail, `first.png` right half | .000 | .003 | .006 | **.011** | .003 | .024 |
| ours, `second.png` left half | .000 | .010 | .011 | **.046** | .024 | .056 |
| retail, `second.png` right half | .007 | .007 | .008 | **.015** | .014 | .020 |
| HEAD, my run, native 1280x720 | .000 | .009 | .015 | **.083** | .009 | .085 |
| HEAD, my run, native 1600x900 | .002 | .004 | .007 | **.086** | .006 | .087 |
| **agent EX, native 1280x720** | .016 | .002 | .018 | **.019** | .028 | .028 |
| **agent EX, native 1600x900** | — | — | — | **.005** | — | — |

Period 5 at *both* 1280 and 1600 rules out a fixed pixel count and names a ratio of 5:4 — which is 125 %.

The same thing on one glyph stem, the vertical of the P in `PRESS ANY KEY`, as coverage:

```
retail (reference)        0.00  0.48  1.00  1.00  1.00  0.50  0.00
ours before               0.08  0.52  0.875 1.00  0.86  0.49  0.11
retail convolved [1 2 1]  0.12  0.49  0.87  1.00  0.875 0.50  0.125
ours after (native 1:1)   0.00  0.56  1.00  1.00  0.55  0.00        (2-px plateau at 1:1, 3 at 1.25x)
```

Before, our stem was retail's convolved with a magnification kernel, every sample within 0.04. After, it is
retail's shape.

Before/after through the same driver, both asked for a 1600x900 client, both captured with a DPI-aware
`BitBlt` of the window's own client rect:

| | physical client | 5-phase resample x / y | `NEW GAME` row label colour |
|---|---|---|---|
| BEFORE, HEAD `e2d7e5d` | **2000x1125** | .051 / .046 | (222, 239, 211) |
| AFTER, agent EX | **1600x900** | .017 / .009 | **(227, 242, 214)** |
| RETAIL, `second.png` | 1600x900 | .015 / .005 | **(227, 242, 214)** |

Images: `build/agentEX/beforeafter_menu.png`, `build/agentEX/sbs_first.png`, `build/agentEX/sbs_second.png`.

## 4. The colour

The colour was **not** in the field's `TextColor`, not in an HTML or CSS tag, not in a `Cxform` and not in
the translator. A trace at the point the glyph batch leaves the game thread printed the colour the display
list had already resolved:

```
GFx glyph batch: 11 glyphs, screen (549.4,537.4)-(733.7,577.8), colour 23,25,28,255     <- the drop shadow
GFx glyph batch: 11 glyphs, screen (557.4,545.4)-(725.7,569.8), colour 227,242,214,255  <- PRESS ANY KEY
```

`(227,242,214)` is retail's pixel exactly. The framebuffer held `(214,242,227)`. A probe that replaced every
glyph vertex colour with `GColor(255,0,0,255)` drew **pure blue** `(0,0,255)`.

`FGFxVertex_Glyph::Color` is a `GColor`, and retail's own `GColor::Rgb32` is `{Blue, Green, Red, Alpha}`
(`resources/docs/types/all_types.h`) — our struct matches retail's byte for byte. The only remaining free
variable was the declared element type, and retail settles it. `FGFxRendererImpl::GetUIVertexDecl_RenderThread`,
**2013 `0x9762c0` (RVA `0x5762c0`), 2012 `0x5babe0`**, decompiled from `retail2013_agentEX.i64`:

```
case 2:  (0, 0, 2, 0)   (0, 8, 2, 1)   (0, 16, 7, 7)            stride 20     GFx_VD_Glyph
case 3:  (0, 0, 9, 0)   (0, 4, 7, 7)                            stride 8      GFx_VD_XY16iC32
case 4:  (0, 0, 9, 0)   (0, 4, 7, 7)   (0, 8, 7, 7, index 1)    stride 12     GFx_VD_XY16iCF32
case 1:  (0, 0, 9, 0)                                           stride 4      GFx_VD_Strip
```
(each tuple is Stream, Offset, Type, Usage, and UsageIndex where it is not zero.)

Every colour element is **type 7**. `EVertexElementType` is `VET_UByte4N = 7`, `VET_Color = 8`, in our
`RHI.h` and in retail's own enum. Our code had `VET_Color`, and the comment immediately above it said
"element types 9 / 2 / 7 are VET_Short2 / VET_Float2 / **VET_Color**" — the number was right and the name was
wrong, and the code followed the name. `VET_Color` is `D3DDECLTYPE_D3DCOLOR`, which expands an ARGB DWORD to
(R,G,B,A); `VET_UByte4N` is `D3DDECLTYPE_UBYTE4N`, which expands the four bytes in memory order. The cooked
GFx pixel shaders this build loads out of retail's `GlobalShaderCache-PC-D3D-SM3.bin` expect the latter.

After the change, `PRESS ANY KEY` is `(227,242,214)` over 292 pixels of plateau and the main-menu labels are
`(227,242,214)` over 979 — retail's values, not near them.

Note for the record, because the brief asked: `GRenderer::Cxform` was **checked and is not transposed now**.
`FGFxCxformToScaleAndBias` reads multiply from column 0 and add from column 1 per channel, which is GFx 3.3's
channel-major `[4][2]`, and the identity it is handed for these fields reaches the shader as an identity.

## 5. The fonts

`GFxTextDocView::FindFont` now logs each distinct `'<requested list>' -> '<resolved>' (<route>)` once, with
the route being the format's resolved handle, the font manager, `FALLBACK` (font index 0, which is what a
field gets when nothing can supply what it asked for) or `NONE`. The whole start screen and main menu:

```
GFx font resolved: 'ChaletComprime-CologneEighty' -> 'ChaletComprime-CologneEighty' (handle)
GFx font resolved: 'Emerge BF' -> 'Emerge BF' (handle)
```

Two fonts, both resolved from the movie's own embedded fonts through the format's handle, **no field on
either screen falls back**, and `DisFonts.gfxfontlib` and `DisFonts.fonts_efigs` are both resident. The
letterforms in `PRESS ANY KEY` and in `MISSIONS / OPTIONS / QUIT GAME` are the same face, the same size and
the same advance widths as retail's, checked glyph by glyph at 4x.

**So the "wrong fonts" half of the user's report is, on these two screens, wrong — and what they were seeing
is the 1.25x stretch plus the red/blue exchange.** A pale green-white instead of a warm cream reads as a
different typeface at a glance far more than it reads as a different colour. The two faults had one
appearance.

What I have **not** measured is the Options screen and the New Game screen (`third.png`, `fourth.png`), whose
right-hand value column and footer do not render at all yet (EY and EZ). One attempt to drive to Options was
made and abandoned: four agents were driving real input at the same time and `drive_input` could not hold the
foreground, which is the hazard STATUS.md already records. **Nothing further is needed to answer the question
there**: the trace prints `FALLBACK` for exactly the case the brief describes, so a single run on any later
build will name a wrong font on those screens without any more work.

## 6. What this package did not do, and what is left

* The 3D perspective drift of the menu bar and the logo (retail's bar is visibly tilted in `second.png` and
  ours is flat) is **FA's**, and is untouched here.
* Retail's frame is hazier and slightly warmer than ours across the whole image, in the 3D scene as much as
  over it. That is the post-process / tone difference, not the interface; after this package our mean
  gradient is 4.55/4.13 against retail's 3.92/4.03 — ours is now *more* contrasty, which is the same
  difference seen from the other side. **FA / seventh.png.**
* `CONTINUE` and `LOAD` in retail's main menu and not in ours is **EY's** question about saves; my runs use a
  private ini directory and no profile, so mine could not show them either way.
* `GTessellator` is still unported. It was measured and is not what wave 16's blur was, and **no missing
  edge is visible on either screen after this change** — the menu bar's fills, its separators and the
  logo's rule all land where retail's do with retail's antialiasing — so its cost was not estimated.
  If an edge deficiency turns up on a screen this package did not reach, that estimate is still owed.

## 7. Where the brief and the plan were wrong

Three places, each with the measurement:

1. **"A single wrong stage-to-viewport mapping would magnify every movie."** It would, and there is no such
   mapping. Section 2. The magnification was real and it was outside the process.
2. **`PHASE12.md`'s table says of `first.png` that ours has the "logo larger" and the "scene framed
   closer", retail "logo crisp, scene framed wider".** Measured by correlating the logo band of the two
   halves over scale and offset: `first.png` best scale **1.002**, `PRESS ANY KEY` **1.000** — the logo is
   the same size in both. In `second.png` the best scale is **0.956**, i.e. ours is 4.4 % *smaller*, the
   opposite of the claim, and that residue is the missing 3D perspective (FA). And retail's scene is framed
   *closer*, not wider, which is the title camera at a different moment. The only thing wrong with the logo
   was that it was soft and cool instead of crisp and warm.
3. **"Which font each text field resolves to ... a field falling back to a default is the most likely
   reading of 'the wrong fonts'."** It is the most likely reading and it is not what happened: zero
   fallbacks on both screens. Section 5.

## 8. Acceptance

| | |
|---|---|
| side-by-side vs `first.png` / `second.png`, same window size, reached normally | `build/agentEX/sbs_first.png`, `build/agentEX/sbs_second.png`. Both at a 1600x900 **physical** client, reached by launching and pressing Space; nothing forced, no console, no debug draw |
| which candidate causes were measured, and which the fix rests on | section 2, table |
| before/after against the untouched HEAD executable through the same driver | `build/agentEX/beforeafter_menu.png` and the table in section 3. HEAD staged once as `DishonoredGame_EXH.exe` before any edit and never re-staged |
| `run_regression.py` | **37 ok, 0 failed, 0 skipped**, 437 s, this worktree's own copy, absolute build dir `build/agentEX_wt/build/agentEX_reg`, built inside the harness (no `--no-build`); the five gitignored `resources/docs/types` inputs and the four gitignored `resources/docs/symbols` tables copied into the worktree first. **An earlier run of the same binary read 35 ok / 2 failed and is reported here in full**: `d3d9/unported_natives 1` (`ADishonoredPlayerPawn::execPlayDying_Native` - the pawn died) and `inputtest/inputtest_peak_speed 0.0` with `inputtest_moved 4781.1`, which is the known load failure mode in STATUS.md (`L_Tower_P` streams eight levels and the pawn falls out of the world when they arrive late, and the walk becomes `-distouchprobe`'s teleports). Three other agents were running games on the machine: `d3d9_startup_seconds` read **46.1 s** in that run and **4.6 s** in this one, `d3d9_frames` 1050 against 16620, and `inputtest_peak_speed` 0.0 against 500.3. Nothing in this package can move a pawn |
| clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | `build/agentEX_wt/build/agentEX_clean` deleted and re-configured from scratch, `CMAKE_BUILD_TYPE=Release`, `-DDISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 fatal errors, 0 C4263, 0 C4264**, 963/963 targets, `Binaries/Win32/DishonoredGame.exe` linked (`build/agentEX_clean.log`) |
| `rva_sweep.py` plus a by-hand resolution of every cited address | whole tree 7302 citations, 0 unknown-2012, 0 UNKNOWN-CLAIMED-2013 outside one pre-existing line in `disbehaviorpatrol.cpp` (agent EP's), 2 MISLABELLED-2012 at `gfxuirenderer.cpp:1679` — **the two pre-existing suspects the brief names**, on a line whose own words say "2012", which this package did not edit (the line number moved from 1669 only because the comment above it grew). The one address this package cites, 2013 `0x9762c0` / RVA `0x5762c0`, was resolved by hand with `lookup_funcs` and read with `decompile`; its name and size (`0x1ca`) match `functions_2013.csv`, and its body is quoted in section 4 |

## 9. Merging

* `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is **not required** and was
  **not run**. This package adds no UnrealScript-visible member, no native, no class and no compile unit.
* **`Sources.cmake` does not change. 0 units.**
* **Files outside `External/GFx3` and `GFxUI`: one.** `source/Development/Src/Launch/Src/Launch.cpp`, the
  DPI-awareness block in `SetupWindowsEnvironment`. It is self-contained, it runs once before any window
  exists, and it is the whole of fault 1. It is a stated deviation from retail's binary, which declares
  nothing and is made sharp on this machine by a per-executable compatibility layer instead.
* **Total: 6 files** — four source, two documents:

| file | new? |
|---|---|
| `source/Development/Src/External/GFx3/GFxDisplay.cpp` | no |
| `source/Development/Src/External/GFx3/GFxTextDocView.cpp` | no |
| `source/Development/Src/GFxUI/Src/gfxuirenderer.cpp` | no |
| `source/Development/Src/Launch/Src/Launch.cpp` | no |
| `resources/docs/agents/agentEX.md` | yes |
| `resources/docs/agents/agentEX_status.csv` | yes |

`build/agentEX_sync.py` is the authoritative copy list, runs worktree → main, and **flags rather than
copies** any file that changed in main since `e2d7e5d`. Nothing is committed.

## 10. A note for the next wave, and for the coordinator

Set `HIGHDPIAWARE` on nothing; the executable now declares it. But **every screenshot comparison made on this
machine before this commit was between a 1.25x-magnified window and a native one**, including the seven that
are wave 16's brief. Anything in `PHASE12.md` that reads "blurry", "soft", "lower resolution" or "enlarged"
should be re-read against a build that carries this change before a package is written to chase it — in
particular EY's *"the background of the new game screen is way too enlarged and blurry"*, of which the
"blurry" is this and the "enlarged" may or may not be.
