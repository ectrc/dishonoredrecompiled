# Agent DL — the text's drop shadow, and the pass that draws it is not the one anybody expected (2026-09-28)

Package DL: the GFx filter pipeline, so the menu text carries the drop shadow the user's reference
screenshots show (`resources/reference/menu/first.jpg`, `second.jpg`). Agent DK measured the asset —
**three `DropShadow` filters, every one on an instance named `txt`** — made `GFxPlaceObject2Tag::Read`
walk the records, and handed the package back rather than rush the render-to-target pass it thought was
needed.

**It is not a render-to-target pass.** Retail has one, and the cook never reaches it: the only two
callers of `GFxDisplayContext::BeginFilters` (2013 **`0xa538f0`**) are `GFxSprite::Display`
(`0x9f1780`) and `GFxButtonCharacter::Display` (`0xa5d300`). A **text** character's own override —
`GFxEditTextCharacter::SetFilters` (**`0xa275b0`**) — folds the whole filter list into one
`GFxTextFilter` (`GFxTextFilter::LoadFilterDesc`, **`0xa89910`**), and the text engine then rasterises
each glyph a *second* time, blurred, into its own atlas slot and draws that batch underneath in the
filter's colour. That is both cheaper and sharper than blurring a subtree, and it is what the
references show. Finding that out first is what made this eleven files instead of a wave.

## Result

| Accept | State |
|---|---|
| `build/agentDL/menu_shadow.png`, plus a before/after pair from one binary differing by one switch | **done** — `menu_shadow.png` / `menu_shadow_off.png` (start screen, world time 10.0 s, the pair differs only by `-gfxuinotextshadow`), `menu_shadow_zoom.png` / `menu_shadow_off_zoom.png`, and the main-menu pair `menu_main_on.png` / `menu_main_off.png`. Section 4 |
| a census line: filters parsed, applied, passes, with applied == parsed | **`filters 3 parsed / 3 applied / 9 passes (6 deliveries)`** — and all three are `DropShadow`. Section 3 |
| a side-by-side against `first.jpg` and `second.jpg` on the text | Section 5, and it changed what "drop shadow" means here: **every one of the three has distance 0**, so it is a soft dark halo, not an offset shadow |
| `run_regression.py` 31 checks, 0 failures, own `--build-dir` | **`31 ok, 0 failed, 0 skipped, 423s`** on the clean gate build `build/agentDL_wtrel` |
| clean full Release build | **982 edges, 0 errors** on the snapshot worktree `build/agentDL_wt`, build directory deleted first (`build/agentDL/buildwt.log`, and a second run says `ninja: no work to do`) |
| report + `agentDL_status.csv` | this file; 32 rows, `rva_2013,status,note` |

## 1. What the cook actually asks for, field by field

`-gfxuicensus` now reports every descriptor the moment it reaches a character:

```
[0005.49] filter applied: DropShadow on 'txt' text 'Description'  blur 8.00 x 8.00 passes 3 angle 449 distance 0 offset 0.00,0.00 colour FF17191C strength 1.60
[0005.49] filter applied: DropShadow on 'txt' text 'PRESS'        blur 8.00 x 8.00 passes 3 angle 449 distance 0 offset 0.00,0.00 colour FF17191C strength 1.49
[0010.76] filter applied: DropShadow on 'txt' text 'W'            blur 5.00 x 5.00 passes 3 angle 449 distance 0 offset 0.00,0.00 colour FFE3F2D6 strength 1.00
```

Three things follow, and none of them was guessable from the record count DK could report:

1. **The distance is 0 on all three.** A `DropShadow` with distance 0 is a *halo*, not an offset
   shadow: the blurred copy sits exactly under the glyph. That is why the reference's text reads
   cleanly over a bright harbour and ours did not — the letters had no dark ground under them.
2. **Two of the three are near-black (`FF17191C`) and the third is near-white (`FFE3F2D6`).** The
   light one is on the field whose initial text is `W`, which is the menu bar's selected label; on the
   selected item's light band it is correctly invisible, and it is there for the frames where that
   label sits over the dark bar. A port that "made the text darker" would have been wrong for one
   field in three.
3. **Blur 8 px with three passes** on ~14 px text. That is a wide, soft halo, which is why the effect
   is worth 9.54 mean over the text's own rectangle and almost nothing over the frame (section 4).

## 2. The five defects, each with its address

### 2.1 `GFx_LoadFilters` was a step-over, so every field of every record was thrown away

DK's parse read the kind byte, counted it and skipped the record by its length. The real function is
**`GFx_LoadFilters<GFxStreamContext>` (2013 `0xa89a90`)**, reached from `GFxPlaceObject3::Unpack`
(`0xa04f10`); `0xa89910` is the `GFxStream` instantiation of the same walk and the two bodies agree.
It is ported field for field, with retail's conversions:

* the blur, the angle and the distance are SWF FIXED 16.16, a `u32` times `0x1p-16`;
* the angle is stored as **tenths of a degree**, `fmod(angle * 1800 / PI, 3600)`;
* the distance is stored in **twips**, `distance * 20`;
* the strength is FIXED 8.8;
* the flag byte's low five bits are the pass count, `0x80` is InnerShadow, `0x40` Knockout and `0x20`
  CompositeSource — and retail stores the last one **inverted**, as `Filter_HideObject`.

The descriptor it writes is `GFxFilterDesc`, 156 bytes, and every offset is measured rather than
assumed. The default constructor (`0x9c5a10`) writes blur 5 at +12/+16, one pass at +20 and strength 1
at +40; `GFxDisplayContext::EndFilters` (`0xa53d90`) copies +8..+40 and the 32 bytes at +44 straight
into a `GRenderer::BlurFilterParams`, which pins `Params` at +8 — the same 68 bytes `GFx3Layout.cpp`
already asserts; `BeginFilters` tests the byte at +35, which is `Params.Color`'s alpha;
`GetShadowOffset` (`0xa536b0`) reads shorts at +2 and +4. Five `static_assert`s hold the layout.

**One real bug in the step-over it replaces**, and it is worth naming because nothing in this cook
exercises it: a ConvolutionFilter record is `4*X*Y + 13` bytes after its two matrix dimensions
(Divisor 4, Bias 4, the matrix, DefaultColor 4, the flag byte), and DK's skip was `+ 9`. A stream with
one convolution filter in it would have desynchronised every record after it.

### 2.2 The filter list had nowhere to go, and `GFxCharacter` had no sink

`GFxCharPosInfo` gains `pFilters` / `FilterCount`. The array is owned by the tag that parsed it
(`GFxPlaceObject2Tag::pFilterList`, allocated only for the three tags that carry filters and freed in
the tag's destructor) and never by the record, because a `GFxCharPosInfo` is built on the stack in half
a dozen places; the character copies what it needs inside `SetFilters` and keeps no pointer.
`GFxCharacter::SetFilters` is retail's (`0x9c6940`) — a free and nothing else — and
`GFxDisplayList::AddDisplayObject` / `MoveDisplayObject` / `ReplaceDisplayObject` pass the list on
where they already pass the matrix and the colour transform.

### 2.3 `GFxTextFilter` did not exist

36 bytes, ported whole: the constructor (`0xa24420`, which is where the 45-degree / four-pixel defaults
come from), `FloatToFixed44` (`0xa24480`), `UpdateShadowOffset` (`0xa22f30`) and `LoadFilterDesc`
(`0xa89910`). `LoadFilterDesc`'s branch structure is retail's and it matters: a **Blur** record fills
the text's own blur, a **DropShadow** or a **Glow** fills the shadow block — but only the *first* one
does. Once a colour and a distance are set, a following Glow contributes its colour and its size and
nothing else. That is how retail keeps three filters on one field from fighting, and it is why this
cook's three filters produce exactly one shadow each.

### 2.4 The glyph rasteriser keyed its cache on the blur and then ignored it

This is the package's own instance of the project's defining pattern, and it was hiding in a field that
was already there. `GFxGlyphParam` carried `BlurX`, `BlurY` and `GPF_Blur`/`GPF_Shadow`/`GPF_Strengthen`,
and `GFxGlyphRasterCache::GetGlyph` already compared all of them — so two glyphs that differed only in
their blur got two atlas slots holding **identical, unblurred pixels**. `rasterizeAndPack` never looked
at them. Nothing had ever asked for a blurred glyph until this package did.

Retail's `rasterizeAndPack` (**`0xa45a70`**) grows the glyph box by the blur radius — rounded to whole
pixels, and a blur that rounds to zero but is not zero still gets one pixel — and then branches on
`GPF_FineBlur`: clear calls **`stackBlur` (`0xa43a50`)**, set calls `recursiveBlur` (`0xa43f30`).
`stackBlur` is Anti-Grain Geometry's `stack_blur_gray8`, and its multiply/shift tables are **read out of
the retail image** rather than remembered: `word_11FDFD8` and `byte_11FE1D8` at VAs `0x11FDFD8` /
`0x11FE1D8` are

```
mul 512 512 456 512 328 456 335 512 405 328 271 456 388 335 292 512 ...
shr   9  11  12  13  13  14  14  15  15  15  15  16  16  16  16  17 ...
```

which is `g_stack_blur8_mul` / `g_stack_blur8_shr` term for term (`mul[r] / 2^shr[r]` is `1/(r+1)^2`).
`strengthenImage` (`0xa420c0`) follows it, with retail's own bias of 2 when a blur ran and the strength
is above 1.

### 2.5 The text field drew one batch and retail draws two

`GFxEditTextCharacter::Display` now walks its glyphs three times: fill the atlas (both the plain and the
blurred variant of every glyph, or the shadow's slot is missing the frame it is first asked for), submit
the shadow batch at the filter's twips offset in the filter's colour, then submit the text. The shadow
batch is closed before the first glyph of the text, so the two can never interleave — which is the
guarantee retail gets by submitting a line's shadow before that line's glyphs
(`GFxTextLineBuffer::Display`, `0xa3c0e0`).

## 3. Census

The last frame of the acceptance run (`-gfxuicensus`):

```
GFx UI census (frame): filters 3 parsed / 3 applied / 9 passes (6 deliveries)
GFx UI census (frame):   DropShadow 3 parsed / 3 applied
GFx UI census (frame): movies open 1 [UI_MainMenu.MainMenu], drawn 1, display objects 178
  (96 sprites, 61 shapes, 6 text fields [0 unbound], 45 bitmap fills), 60 draws, 174 triangles,
  7 glyph batches / 64 glyphs / 7 shadow glyphs, 4 masks, atlas 47 packed / 4 blank / 0 failed
```

**Applied equals parsed, and both are 3.** The two numbers mean different things and the report says
which: *applied* counts descriptors, identified by their address in the tag that owns them, so a filter
delivered twice because the timeline re-placed its field is one descriptor; *deliveries* counts the
calls, and it is 6 because each of the three place tags executes twice. The first measurement of this
package read `3 parsed / 6 applied` and both numbers were right — the counter was answering the wrong
question, and it was changed rather than explained away.

On the start screen, where the shadow is the one the reference shows, the same census is

```
GFx UI census (frame): filters 3 parsed / 2 applied / 6 passes (2 deliveries)
GFx UI census (frame): ... 2 glyph batches / 11 glyphs / 11 shadow glyphs, atlas 16 packed
```

— 11 of 11 glyphs shadowed, and 2 of 3 filters applied because the third field is not placed until the
main menu opens. With `-gfxuinotextshadow` it is `1 glyph batch / 11 glyphs / 0 shadow glyphs, atlas 8
packed`: the extra 8 atlas slots are the blurred glyphs, and on the main menu the atlas goes 33 -> 47
for the same reason.

## 4. Before and after, measured

Both frames come from **one binary**, one command line apart, and both are keyed on **world time**
(`-apshottime=10.0`), not on a drawn frame — agent DB's standing lesson, and it matters here because
the menu map's camera is mid-fly-through at that moment and a frame index would photograph two
different instants.

The floor is not zero and saying so is part of the measurement: the scene behind the interface is
animated — water, reeds, particles and the colour grade's film grain, which is re-seeded every frame —
so two runs that ask for the same world time do not produce the same bitmap. Over the `PRESS ANY KEY`
rectangle (530,530)-(760,585) of the gate build:

| Pair | pixels differing | mean absolute | **signed mean** | darker / lighter by more than 1 |
|---|---|---|---|---|
| floor: two identical runs, shadow on | 89.75 % | 7.18 | **-0.299** | 4,176 / 4,430 |
| `-gfxuinotextshadow` vs shadow on | 93.92 % | 13.63 | **+7.648** | 6,905 / 3,144 |

The **signed** column is the one that settles it. A drop shadow can only darken, and scene noise is
symmetric: the floor's signed mean is -0.3 with as many pixels going up as down, and the shadow's is
+7.6 with more than twice as many going down as up. That is 25 times the floor's magnitude, and it is a
statistic the animated background cannot fake.

`menu_shadow_zoom.png` and `menu_shadow_off_zoom.png` are the same crop at 5x, and they are the honest
version of the same fact: with the shadow off, `PRESS ANY KEY` all but dissolves into the bright water
behind it; with it on, the letters sit on their own dark ground and read exactly as the reference's do.
On the dev build, whose t = 10 s frame happened to land on a quieter moment, the same rectangle gave a
floor of 1.66 mean against a signal of 9.54.

The main-menu pair (`menu_main_on.png` / `menu_main_off.png`, `-apshottime=25.0` after a scripted key)
is included for completeness and it is deliberately undramatic: the only filtered field visible on that
screen is the selected label, whose filter is the **light** one, drawn over the light selection band.
It is correctly almost invisible there, and saying so is worth more than cropping until it looks like
something.

## 5. Against the references, on the text specifically

**`first.jpg` vs `menu_shadow.png`.** This is the pair that carries the package. In the reference,
`PRESS ANY KEY` sits over a flat grey-green water surface and the pixels immediately around the letters
are visibly darker than the field a few pixels further out — a soft dark aura with no directional
offset. That is exactly what a `DropShadow` with distance 0, blur 8 and colour `FF17191C` produces, and
it is what our frame now has. Before this package our letters had a hard edge straight onto the
background and, where the water is bright, the stroke ends disappeared into it.

**`second.jpg` vs `menu_main_on.png`.** The bar labels — `NEW GAME`, `MISSIONS`, `OPTIONS`,
`QUIT GAME` — carry **no** filter in the cook, in the reference or in ours, and they look the same in
both. The reference's extra legibility there comes from the black bar behind them, not from a shadow.
The three differences DK named against this screenshot are unchanged by this package and remain open:
`MISSIONS` has no asterisk (the DLC config merge), the DLC row's glyph is the unlit PC one, and the
camera is a little further back than the reference's.

## 6. Deviations, stated once

1. **The blurred glyph's padding is `max(radiusX, radiusY)` on both axes**, where retail grows the box
   by `radiusX` horizontally and `radiusY` vertically (`0xa45a70`). This tree's raster core
   (`GFxGlyphRasterize`, agent CB's) takes one padding for both. Every filter in this cook has
   `BlurX == BlurY`, so the two are the same number here; an anisotropic blur would cost a few wasted
   atlas pixels and nothing else.
2. **`GFxGlyphParam` gains `Strength` as a trailing byte.** Retail keeps it in the byte after `BlurY`
   and reads it as `param[10] != 16`; this reconstruction already has `Outline` in that byte. The
   struct is a cache key and nothing serialises it, so the order costs nothing — but it is not retail's.
3. **`GFxTextFilter` has no refcounted base.** Retail derives it from `GRefCountBaseNTS` and the
   character holds a pointer; this tree holds one by value, because nothing else shares it.
4. **`GFxTextFieldParam::LoadFromTextFilter` (`0xa809c0`) is not ported.** Retail copies the filter into
   the per-format parameter block on its way to the line buffer; here the character's `Display` reads
   the `GFxTextFilter` directly. Same values, one hop fewer.
5. **The glyph parameters are built directly rather than through `CalcGlyphParam` (`0xa42150`)**, which
   is how this tree's text display already worked before this package: the blur is therefore used at its
   authored size rather than scaled by the text's own scale and clamped to the slot budget. At this
   cook's sizes the clamp never binds (a 14 px glyph with an 8 px blur is 30 px in a 44 px budget).
6. **`recursiveBlur`, `makeKnockOutCopy` and `knockOut` are not ported.** `GPF_FineBlur` is never set by
   anything in this tree and no filter in this cook sets the Knockout bit.
7. **The filter parameters reach `GRenderer::BlurFilterParams` but nothing calls `DrawBlurRect`.**
   Agent CC's renderer half is complete and correct and is still not driven by anything, because the
   path that would drive it is the *sprite* and *button* filter pass, and this cook has no filter on
   either. Hand-over 2.

## 7. The two secondary items

### 7.1 The start camera's fly-through — DK's diagnosis was one class too high, and the correction is the point

DK wrote that `UInterpTrackSoireeControl` "is a declaration-only shim with no `UpdateTrack`" and is
therefore the twelfth instance of this project's pattern. **Retail has no `UpdateTrack` on it either.**
There is no such function in the 2012 PDB or the 2013 image; every `UInterpTrackSoireeControl` entry is
editor-side (`AddKeyframe`, `DuplicateKeyframe`, `GetClosestSnapPosition`). The runtime is somewhere
else entirely, and all of it is absent from this tree:

```
USeqAct_Interp::UpdateInterpLoop                    0x2340f0   492 b   the whole loop-and-pause step
USeqAct_Interp::SoireeShouldLoop                    0x218f80   263 b
USeqAct_Interp::SoireeLoopShouldBackupTransforms    0x219090   248 b
USeqAct_Interp::SoireeLoopBackupActorTransforms     0x219190   178 b
USeqAct_Interp::SoireeLoopRestoreActorTransforms    0x219250   202 b
USeqAct_Interp::GetDistractionLoopOverride          0x218c50   122 b
USeqAct_Interp::SetDistractionLoopOverride     2012 0x22fb10   153 b   (unmatched in 2013)
UInterpTrackInstSoireeControl::InitTrackInst        0x506b30   559 b
UInterpTrackInstSoireeControl::SoireeShouldLoop     0x500480   351 b
UInterpTrackInstSoireeControl::SoireeStartLoop      0x5005e0   152 b
UInterpTrackInstSoireeControl::NeedsSynchronizing   0x500a40   795 b
```

The mechanism, out of `UInterpTrackInstSoireeControl::SoireeShouldLoop`: a SoireeControl key
defines a segment `[KeyTime, KeyTime + Length]`, and when the matinee's time passes the segment's end it
is **rewound to the segment's start**, up to `m_LoopCount` times — **with 0 meaning forever**. The loop
ends when the per-key status flag at `m_lKeysStatus[i] + 4` bit 0 is set, or when
`GetDistractionLoopOverride(m_PinName)` returns a smaller count. So the start camera does not "run on
the map's own clock": retail holds it on a looping section of `StartCam` until something sets that
break flag, and the key press is what sets it.

**Not attempted**, and deliberately: the two classes already have their real storage and their layout
asserts, but the eleven functions above plus the actor-transform backup and restore are a package, and a
half-port that started the loop without the release would leave the camera stuck instead of early —
which is worse than what is there now.

### 7.2 The GC assertion — DK's prime suspect is ruled out by the code

`check(!Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading))` in `UObject::StaticAllocateObject`'s
replace-an-existing-object branch (`UnObj.cpp:7958`). DK saw it once in nine runs and named the new
matinee `UInterpGroupInst` churn as the prime suspect. It cannot be that:

* the replace branch is reached only when `Obj != NULL`, and `Obj` is only ever assigned from
  `StaticFindObjectFastInternal` in the `else` of `if (InName == NAME_None)` (`UnObj.cpp:7880-7889`);
* **every** matinee construction passes `NAME_None` — `UInterpGroupInst`, `UInterpGroupInstDirector`,
  `UInterpGroupInstAI`, `UInterpGroupInstCamera` (`UnInterpolation.cpp:1973, 1985, 1994, 2016, 2023,
  2029, 2143`) and `UInterpTrackInst` (`:3299`) — so they take the `MakeUniqueObjectName` path and
  never reach the find;
* and `MakeUniqueObjectName` (`UnObj.cpp:7570`) does carry its existence loop, so it cannot hand back a
  colliding name either.

So the branch needs an **explicit `FName`**, and the search should be for a named
`StaticConstructObject` on the menu path. Every `ConstructObject`/`StaticConstructObject` in `GFxUI` and
in the `DishonoredGame` movie players uses the default name, so it is not there either.

**It did not reproduce here**: **16 runs** of the menu across this package, 30 to 100 s each, about
twelve minutes of menu time in total, **0 `Critical:` lines** in every one (eight of those logs are
kept in `build/agentDL/`), plus the 31-check regression, whose `d3d9_criticals`, `inputtest_criticals`
and `nullrhi_criticals` are all 0.

## 8. Verification

* **The acceptance screenshots** come from the clean gate build `build/agentDL_wtrel` at
  `-startmap=Dishonored_MainMenu -startmapopen -gfxuimenu -windowed -ResX=1280 -ResY=720 -nomovie`,
  with `-apshottime=10.0` for the start-screen pair and `-gfxuikey=600:SpaceBar -gfxuikeyhold=30
  -apshottime=25.0` for the main-menu pair. The two members of each pair differ by
  `-gfxuinotextshadow` and nothing else. **0 `Critical:` lines.**
* **Regression**: `python resources\tools\run_regression.py --build-dir build/agentDL_wtrel --no-build
  --exe-name DishonoredGame_DLW.exe --log-prefix DLW` -> **`31 ok, 0 failed, 0 skipped, 423s`**
  (`build/agentDL_wtrel/regression/summary.txt`).
* **Clean full Release build** of the snapshot worktree `build/agentDL_wt` (HEAD `d3fe89d` plus this
  package's 11 files), build directory deleted first: **982 edges, 0 errors**
  (`build/agentDL/buildwt.log`; a second invocation reports `ninja: no work to do`).
* **No generator output was touched.** Nothing in this package is produced by
  `gen_classes_header.py`, so no regeneration is needed.

## 9. Files

Mine (11, none new):
`External/GFx3/{GFxDisplay.cpp, GFxDisplay.h, GFxGlyphCache.cpp, GFxGlyphCache.h, GFxPlayer.h,
GFxPlayerData.cpp, GFxPlayerSprite.cpp, GFxTextField.cpp, GFxTextField.h}`;
`GFxUI/{Inc/gfxuiengine.h, Src/gfxuiengine.cpp}`; plus this report and `agentDL_status.csv`.

One switch, read on first use in `FGFxEngine`'s constructor beside the other five, never as a file-scope
static: **`-gfxuinotextshadow`** — drop the shadow batch and change nothing else, so a before/after pair
comes out of one binary.

Scratch, not repo tools: `build/agentDL/` — `dlpatch.py` (the CRLF-safe patch helper),
`patch1_filterdesc.py` … `patch7_applytrace.py` (one per defect, each with its measurement in its
docstring), `bmp2png.py`, `crop.py`, `diffshot.py`, `signeddiff.py`, `xrefs_to.py`, `dump_tables.py` (the
stack-blur table read), `dec1`..`dec6` (the headless decompiles this rests on), the run logs and the
screenshots;
`signeddiff.py`; `build/agentDL_build.cmd`, `build/agentDL_run.py`, `build/agentDL_sync.py`; snapshot worktree
`build/agentDL_wt`, build directories `build/agentDL_rel` and `build/agentDL_wtrel` (the clean gate
build).

IDA: **own copy only**, `build/agentDL_ida/retail2013_agentDL.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.

## 10. Hand-overs

1. **The Soiree loop** (section 7.1): eleven functions, all addressed, and the mechanism named. It is
   what gates the start camera on the key press, and it is not a missing `UpdateTrack`.
2. **The sprite and button filter pass.** `GFxDisplayContext::BeginFilters` (`0xa538f0`), `EndFilters`
   (`0xa53d90`) and `DisplayFilterPrePass` (`0xa54410`), plus `GFxASCharacter::SetFilters` (`0x9cacd0`)
   and its `CharFilterDesc` (`0x9c9ab0`). The renderer half is already there and correct — agent CC's
   `CheckFilterSupport` (`0x5733d0`), `DrawBlurRect` (`0x59e2e0`) and `DrawBlurRect_RenderThread`
   (`0x59c260`) — so what is missing is the runtime that would call it. The algorithm is read out and
   recorded here so the next package does not have to re-read it: `BeginFilters` sums, per filter, a
   growth of `ceil(Passes * BlurX) * 20` twips (doubled for a bevel, plus `ceil(|Offset| * 20)` for a
   shadow), grows the character's own bounds by it, allocates a temp target of
   `ceil(width_twips * XScale)` by `ceil(height_twips * YScale)`, resets the colour transform to
   identity and pushes blend mode 13; `EndFilters` pops the target, then per filter sets the matrix to
   identity, allocates the next temp target for every pass but the last, and calls `DrawBlurRect` with
   the temp target's pixel rect as the source rect and the grown bounds as the destination, after
   `BlurFilterParams::Scale(20 * XScale, 20 * YScale)` (`0xa53170`). On the **last** pass the
   descriptor's own cxform is replaced by the character's saved one, which is how the character's alpha
   is applied. **Nested and masked characters follow from the same code**: the context's filter state is
   saved and restored through `GFxDisplayContextFilters::CopyFilterStateFrom` (`0xa53730`) and the
   already-in-a-filter flag shifts the last-pass index by one, and `GFxSprite::Display` pushes the mask
   before `BeginFilters`, so a masked character's filter runs inside its own mask.
3. **`GFxStaticTextCharacter::SetFilters` (`0xa81ef0`)** is the same four lines as the edit-text one and
   is not ported, because no `DefineText` survives this cook. If one ever does, it needs
   `GFxStaticTextCharacter::Display` (`0xa81060`) to draw the same two batches.
4. **`CalcGlyphParam` (`0xa42150`) is still not on the text display's path** (deviation 5). It is what
   scales a filter's blur with the field's own scale, so a text field that animates its `_xscale` with a
   shadow on it will keep the shadow at its authored size.
5. **The GC assertion** (section 7.2): the matinee is eliminated; what remains is a named
   `StaticConstructObject` somewhere on the menu path, and it did not reproduce in 16 runs.
6. Everything DK handed over that this package did not touch: the DLC config merge and `MISSIONS*`,
   mouse input for the interface, `_bUsingGamepad`, `flash.display.BitmapData` for `_common.EmbedImg`,
   and `GTessellator`.
