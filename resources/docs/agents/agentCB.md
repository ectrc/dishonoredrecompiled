# Agent CB — the text engine and the glyph rasteriser: outlines to pixels (2026-09-27)

Package **CB** of `PHASE9.md`: port the text stack from the 2012 symbols in dependency order the way
agents BB and BC did — the text field, the styled text, the document view, the font and glyph caches,
and the rasteriser that turns the `DefineFont3` outline definitions into glyphs — drive it from a
harness rather than from the game, and report a table of implemented against remaining.

**All three acceptance criteria are met, and the thing that was in doubt is settled: the glyphs come
out.** `"Dishonored"` rasterises legibly in both game fonts from their `DefineFont3` outlines, every
one of the 22 cooked payloads' 282 `DefineEditText` tags builds a text field that lays out and
produces glyph output, and the whole path — outline decode, quadratic flattening, anti-aliased scanline
rasterisation, atlas packing, word wrap, the four alignments — is measured rather than asserted.

## Result

| Step | State |
|---|---|
| 1. The stack, in dependency order | **done for a verified subset**: 16 files, 6,861 lines, 8 translation units. `GRasterizer` **13 of 13 functions**, `GFxFontData` 11 of 15, `GCompoundShape` 11 of 14, `GFxTextFormat` 24 of 33, `GFxTextParagraphFormat` 17 of 24, `GFxParagraphFormatter` 6 of 9, `GFxTextDocView` 31 of 97, `GFxEditTextCharacter` 14 of 125. **185 retail functions ported with their 2013 rvas, 242 cited, and all 242 are byte-identical 2012↔2013 (ratio 1.000)** |
| 2. Agent BC's layout finding, applied | **held to**: not one `offsetof` or `sizeof` assertion is added and `GFx3Layout.cpp` is untouched, because the 2012 PDB carries no layout for any of these types. Every enumeration is pinned case by case out of the setter that writes it — section 2 |
| 3. The harness | **done**: `GFx3Text` with four commands. `--fonts` reports glyph coverage, `--raster` rasterises a string and dumps the bitmaps, `--layout` lays a string out in four alignments and prints the per-line metrics, `--run` drives every text field of a cooked movie |
| Accept: both fonts rasterise, glyph coverage reported | **yes**: all **12 fonts in the six `DisFonts*` payloads** read, each consuming its tag exactly; 187 of 188 glyphs have an outline in both game fonts. Section 3 |
| Accept: the menu asset produces laid-out text fields with real glyph output | **yes**: 12 fields, 12 laid out, 12 with glyph output, 122 glyph entries, 112 rasterised, 31,899 covered pixels. Over all 22 payloads: **282 fields, 282 laid out, 282 with glyph output, 2,636 entries, 2,481 rasterised, 0 with no outline, exit 0 everywhere**. Section 4 |
| Accept: the implemented-versus-remaining table | **`GFx3Text --table`**: 48 rows, **966 retail functions in the group, 214 ported, 22.2 %**. Section 7 |
| Regression / playability | **green**: `run_regression.py --build-dir build/agentCB_rel --no-build` = **31 ok, 0 failed, 0 skipped, 430 s**, and `-newgame` runs the retail two-map-change route with **0 criticals** |
| Two real defects found by the harness | a by-value `GArray` copy corrupting the heap, and justify stretching the last line. Both fixed; section 6 |

## 0. The interface with package CD, settled first

Agent CD is porting the tag loaders in parallel and **its `GFxCharacterDefs.h` already builds on this
package's `GFxShape.h`** (`GFxShapeCharacterDef::Shape` is a `GFxConstShapeNoStyles`, and CD's own
comment says "The record walk is package CB's `GFxConstShapeNoStyles` (GFxShape.h)"). That is the right
split and it is the one this report settles:

| owner | what |
|---|---|
| **CB (this package)** | `GFxShape.{h,cpp}` — the SWF SHAPE record walk and `GCompoundShape`; the rasteriser; the font accessors, resource, handle and manager; the glyph cache; the whole text stack |
| **CD** | the loader table and the character definitions: `GFxCharacterDefs.{h,cpp}`, `GFxTagLoaders.cpp`, `GFxPlayerData.cpp` — including `GFxEditTextCharacterDef` and `GFxFontCharacterDef` |

Both packages independently declared a `GFxEditTextCharacterDef`. **That collision is removed on this
side**: this package declares no competing definition class at all. It declares

```cpp
struct GFxTextFieldDesc { ... };                            // the POD a field is built from
bool GFxTextFieldReadDesc(GFxStream*, unsigned tagType, unsigned tagEnd, GFxTextFieldDesc*);
GFxEditTextCharacter::GFxEditTextCharacter(const GFxTextFieldDesc&, parent, id, root);
GFxEditTextCharacter* GFxTextFieldCreateFromTag(GFxStream*, tagType, tagEnd, parent, id, root);
```

`GFxTextFieldDesc`'s flag bits are **the same values at the same positions** as CD's
`GFxEditTextCharacterDef::Flags`, because both are read out of the word the retail body writes at +80
(2012 0xa26190). So CD's `CreateCharacterInstance` (0xa32df0) becomes twelve lines, and the hand-over
in section 9 spells them out. Nothing else of CD's is touched and no hook was added to
`GFxPlayerData.cpp`.

## 1. What is in the tree

`source/Development/Src/External/GFx3/` gains 16 files, 6,861 lines. None of them includes an engine
header, which is what lets `build/agentCB_run.cmd` compile the whole stack with no engine at all.

| File | Lines | What it is |
|---|---|---|
| `GFxShape.h` / `.cpp` | 186 / 549 | `GFxShapeBase`, `GFxConstShapeNoStyles` (the SWF SHAPE record decoder), `GCompoundShape` (the flattener) |
| `GFxRasterizer.h` / `.cpp` | 119 / 589 | **`GRasterizer`, all 13 functions** — the anti-aliased scanline rasteriser — plus the `GImage` helpers |
| `GFxFont.h` / `.cpp` | 310 / 781 | `GFxFont`, `GFxFontData` (the `DefineFont`/`2`/`3` reader), `GFxFontResource`, `GFxFontHandle`, `GFxFontManager`, `GFxFontLoadFromPayload` |
| `GFxGlyphCache.h` / `.cpp` | 159 / 406 | `GFxGlyphParam`, `CalcGlyphParam`, `GFxGlyphRasterize` (the raster core), `GFxGlyphRasterCache` with a shelf atlas |
| `GFxText.h` | 324 | `GFxTextFormat`, `GFxTextParagraphFormat`, `GFxTextAllocator`, `GFxTextParagraph`, `GFxStyledText` |
| `GFxTextFormat.cpp` | 476 | the two formats: defaults, setters, `Merge`, `Intersection`, `operator==`, `Hash`; the interning allocator |
| `GFxStyledText.cpp` | 471 | the paragraph with its format runs, the document, the UTF-8 widener |
| `GFxTextDocView.h` / `.cpp` | 298 / 743 | `GFxTextLineBuffer` + `Line` + `GlyphEntry`, `GFxLineCursor`, **`GFxParagraphFormatter`** (the layout engine), `GFxTextDocView` |
| `GFxTextField.h` / `.cpp` | 161 / 519 | `GFxTextFieldDesc` + the `DefineEditText` record, `GFxEditTextCharacter`, the font registry |
| `Tools/GFx3Text.cpp` | 770 | the acceptance harness |

Plus a targeted edit to `cmake/GFx.cmake`: the eight units join the `gfx3` static library inside the
existing `if(DISHONORED_WITH_GFX3)` guard, and a new `GFx3Text` target (`EXCLUDE_FROM_ALL`) is added
beside `GFx3Dump` and `GFx3Run`. **Nothing else outside the directory is touched** — in particular
`GFxPlayer.h`, `GFxPlayerData.cpp`, `GFxPlayerSprite.cpp`, `GFxPlayerRoot.cpp`, the AS2 units and all
of CD's files are untouched by me.

## 2. The evidence, and the two numbers everything depends on

Method as agent BC's, and for the same reason. Measured first, before writing a line:

```
python resources/tools/pdb/dia_types.py <DishonoredGame-Shipping.pdb> \
    --udt GFxFontData GFxTextDocView GFxStyledText GFxEditTextCharacter GRasterizer GCompoundShape
  -> not one of them carries a layout
```

So the four sources are the same four BC used: **the fully demangled signature of every libgfx
function** (`resources/docs/symbols/functions.csv`, `module=libgfx`; `build/agentCB/surf2.py` groups
them by owning class and prints the counts this report quotes), **`vtables.csv`** for the dispatch
shape, **126 headless Hex-Rays decompiles** of the load-bearing bodies on my own database copy
`resources/docs/idb/shipping2012_agentCB.i64`, and **`match_2012_2013.csv`** for the 2013 address.
**All 242 cited functions are byte-identical between the 2012 and 2013 builds, ratio 1.000**
(`agentCB_status.csv`), so a body read out of the 2012 decompile *is* the retail body.

Consequence, stated once: **behaviour is ported, member offsets are not reproduced and are not
reproducible.** These files carry no `GFX3_ASSERT_OFFSET` and `GFx3Layout.cpp` is untouched.

### 2.1 The unit system, which is the whole ballgame

Get this wrong and text lays out at the wrong size with the right shapes, which is the failure mode
that looks like a rendering bug for a week. Each number is read out of a body:

| fact | evidence |
|---|---|
| a glyph outline and its metrics live on an **EM square of 1024** units | `GFxFontData::GetAdvance` (0xa52c00) returns **512.0** for the no-glyph index and `GetGlyphHeight` (0xa52ca0) returns **1024.0**; `GFxLineCursor::TrackFontParams` (0xa99640) substitutes ascent **960** / descent **64** when a font declares none, and 960 + 64 = 1024 |
| `DefineFont3` stores those coordinates at **20×** | `GFxSwfPathData::PathsIterator`'s constructor (0xa3d110) sets its scale to **0.05** when the shape's flag bit 1 is set, and `GFxFontData::Read` (0xa587d0) scales every layout metric by the same 0.05 **only when the tag is 75** (1.0 for tags 10 and 48) |
| the layout engine works in **twips** (1/20 px) | `GFxTextFormat::SetFontSize` (0xa70dc0) stores `(int)(px * 20)`; `SetLetterSpacing` (0xa70da0) the same |
| so the glyph-to-layout scale is `fontSizePx * 20 / 1024` | `GFxParagraphFormatter::Format` (0xa9d310) computes literally `fontSize * 20.0f * 0.0009765625f`, and 0.0009765625 is 1/1024 |
| the glyph-to-*pixel* scale the rasteriser uses is `sizeIn16thsOfAPixel / 16384` | `GFxGlyphRasterCache::rasterizeAndPack` (0xa4f5d0)'s literal **0.00006103515625** = 1/(1024·16) |
| a per-glyph `AdvanceEntry` is **12 bytes**: `float Advance; s16 Left; s16 Top; u16 Width; u16 Height`, the four bound fields in **twips of glyph units** | `GetAdvance` reads the float at +0; `GetGlyphBounds` (0x9c54d0) reads the four 16-bit fields at +4/+6/+8/+10 and **divides each by 20**; `Read` stores them as `bound * 20.0f` |
| a text field's **view** rect is its **text** rect inset by **40 twips (2 px) a side** | `GFxTextDocView::SetViewRect` (0xa9a1e0): `ViewRect = TextRect ± 40.0` on all four edges. The auto-size grow adds 80 twips, i.e. two gutters (0xa9e3a0) |
| the curve tolerance for a glyph is **10.0** glyph units | the `MakeCompoundShape(shape, 10.0)` call in 0xa4f5d0; `GCompoundShape::SetCurveTolerance` (0xa59730) squares a quarter of it, so the flattener's threshold is `(tol/4)²` |

### 2.2 `GRasterizer` is Anti-Grain Geometry, and the constants are measured not assumed

All 13 functions are ported. It is AGG's `rasterizer_scanline_aa`, and the two shifts that define it
were read out rather than recalled:

* `MoveTo`/`LineTo` (0xab6280 / 0xab6e50) multiply by **256.0** before truncating, so
  `poly_base_shift = 8`, `poly_base_size = 256`, `poly_base_mask = 255`;
* `SweepScanline` (0xab62b0) computes `((cover << 9) - area) >> 9`, i.e. the normalisation shift is
  `poly_base_shift*2 + 1 - aa_shift = 9`, so **`aa_shift = 8`** and a coverage value is 0..255;
* the even-odd rule is `filling_rule == 1` and folds coverage with `c &= 0x1FF; if (c > 256) c = 512 - c;`
  (same body);
* `SetGamma` (0xab65c0) builds a 256-entry `pow(i/255, gamma)*255 + 0.5` table and **drops the table
  entirely when gamma == 1.0**;
* `AddShapeScaled` (0xab6ed0) emits a contour **forwards** when its left fill style matches and
  **backwards** when its right style matches, and **skips a contour whose two sides are the same
  style**. A glyph is submitted with fill style **-1**, the "any path with a style on that side" mode
  (0xa4f5d0 passes -1 literally). That sign convention is what makes a counter (the hole in an `o`)
  come out as a hole rather than as a second blob.

### 2.3 The alignment enum, pinned case by case — because SWF's order and GFx's differ

Getting this wrong silently centres left-aligned text, so it is pinned from four bodies, not guessed:

| body | what it says |
|---|---|
| `GFxTextParagraphFormat::SetAlignment` 0xa242b0 | writes `(align << 9)` into bits 9..10 and sets bit 0 ("alignment present") |
| `IsLeftAlignment` 0xaae080 | bits 9..10 == 0 |
| `IsRightAlignment` 0xa990a0 | bits 9..10 == 1 (0x200) |
| `IsCenterAlignment` 0xa990d0 | bits 9..10 == **3** (0x600) |
| `FinalizeLine` 0xa9a940 | distributes the slack — i.e. justifies — when bits 9..10 == **2** (0x400) |

So **`AlignType` is Left 0, Right 1, Justify 2, Center 3**, which is *not* the SWF `DefineEditText`
Align byte's order (0 left, 1 right, 2 centre, 3 justify). `GFxEditTextCharacter::GetInitialFormats`
(0xa27860) does the remap and its four cases are exactly `0x001 / 0x201 / 0x601 / 0x401`. Both orders
are reproduced, each at its own site.

### 2.4 The rest of the enumerations, each from its writer

* **`GFxFont::FontFlags`**, read out of `GFxFontData::Read` (0xa587d0) one bit at the site the tag's
  flag sets it: `FF_Italic 0x0001`, `FF_Bold 0x0002`, code page `0x0100` ANSI / `0x0200` ShiftJIS,
  `FF_GlyphShapesStripped 0x1000`, `FF_HasLayout 0x2000`, `FF_WideCodes 0x4000`, `FF_SmallText 0x8000`.
  **One is inferred rather than read and the header says so**: `FF_DeviceFont 0x0040`, which
  `GetInitialFormats` tests before it builds a font handle but which no body in the cook sets.
* **`GFxTextFormat`'s present bits** from its setters: `0x02` letter spacing (0xa70da0), `0x08` font
  size (0xa70dc0), `0x10` bold (0xa92400), `0x20` italic (0xa92450), `0x40` underline (0xa90be0),
  `0x80` kerning (0xa90c00), `0x100` url (0xa90b20), `0x400` alpha (0xa70cc0), `0x800` font handle;
  and the style byte at +40: bold 1, italic 2, underline 4, kerning 8.
* **The `DefineEditText` flag word** from 0xa26190, in the order the body reads it, with the
  **three inverted bits** noted at the site: the stream's `NoSelect` is stored as `Selectable`, the
  stream's `UseOutlines` as `UseDeviceFont`, and `HasFontClass`/`WasStatic` are read and dropped.
* **`GFxGlyphParam`'s fields** from `CalcGlyphParam` (0xa4bc60): a `u8` font size at +6, a flag byte
  at +7, `u8` blur x/y at +8/+9 **in 1/16 px** (the body multiplies by 0.0625 to read them back) and
  a `u8` outline at +10, with both blurs clamped at **15.75 px**.
* **`GlyphEntry`'s 1/16-px font-size quantum**: `SetFontSize` (0xa43e40) stores `(int)(px*16)` with a
  fraction flag when the size is below 256 px and has a non-zero sixteenth, and `GetFontSize`
  (0x9bdc40) reads it back through `* 0.0625`. `SetAdvance` (0xa44420) carries the sign in a flag bit.
  `Line::GetDescent` (0xa98ff0) is `Height - BaselineOffset` and `GetNonNegLeading` (0xa482b0) is
  `max(Leading, 0)`.
* **`GFxTextLineBuffer::CalcLineSize`** (0xa442c0) is `((base + 8*glyphs + 7) & ~3) + 4*formats` with
  base 26 for a short line and 38 for a long one — which is how we know a `GlyphEntry` is 8 bytes and
  a format entry 4. It is kept as a function because `InitParagraph` (0xa9c5b0) switches from the
  inline line to a heap line above 0x400 bytes.
* **The auto-hinting reference glyphs** are retail's literals: `calcLowerUpperTop` (0xa54700) takes
  the first glyph the font has of `"HEFTUVWXZ"` and of `"zxvwy"`, and stores 0xFFFF in both when it
  finds neither (which `GetLowerCaseTop` reports as 0).

### 2.5 The tag reader, field by field

`GFxFontData::Read` (0xa587d0, 3,924 bytes) is the largest single port here and its three branches are
reproduced: `DefineFont` (10) has nothing but an offset table whose **first `u16` divided by two is
the glyph count**, and its glyphs are read as `DefineShape` records; `DefineFont2` (48) reads its
glyphs as `DefineShape2`; `DefineFont3` (75) passes its own tag code down, which is what turns on the
20× coordinate scale. Retail's own remap is `shapeTag = 22; if (tagType != 48) shapeTag = tagType;`.

Two faithfulnesses worth naming because they look like bugs:

1. **the advance table is read *unsigned*** (`*(unsigned __int16*)` in the retail body) although the
   SWF specification calls the field `SI16`. Reproduced as retail has it, with the comment saying so;
   no glyph in the cook has a negative advance, so the two readings agree on this content.
2. **a zero first glyph offset means the outlines were stripped**, and retail sets a flag and returns
   rather than reading garbage. That flag is `FF_GlyphShapesStripped`, and it is what
   `HasVectorOrRasterGlyphs` (0xa58270) answers from.

## 3. Acceptance 1: both fonts rasterise, with glyph coverage

`GFx3Text --fonts` over the **six** `DisFonts*` payloads — 12 fonts, every one read, **every one
consuming its `DefineFont3` tag exactly** and every kerning table read in full (declared count equals
read count for all 12, so nothing was truncated):

| payload | font | export | glyphs | with outline | ascent/descent/leading | kerning | contours | flattened verts |
|---|---|---|---|---|---|---|---|---|
| `DisFonts_SF.gfxfontlib` | ChaletComprime-CologneEighty | `$NormalFont` | 188 | **187** (99.5 %) | 895 / 200 / 71 | 2,163 | 344 | 8,223 |
| `DisFonts_SF.gfxfontlib` | Emerge BF | `$TitleFont` | 188 | **187** | 965 / 199 / 140 | 5,000 | 351 | 14,067 |
| `DisFonts_SF.fonts_efigs` | Emerge BF / ChaletComprime | — | 188 each | 187 each | as above | 5,000 / 2,163 | 351 / 344 | 14,067 / 8,223 |
| `DisFonts_LRUS_SF.gfxfontlib` | ChaletComprime / **Goudy Stout** | `$NormalFont` / `$TitleFont` | 188 each | 187 each | 895/200/71 · 1044/358/378 | 2,163 / 0 | 344 / 352 | 8,223 / 22,911 |
| `DisFonts_LRUS_SF.fonts_rus` | **Benguiat** / **PragmaticaCondC** | — | **269** / **288** | 256 / 210 | 870/224/70 · 849/229/54 | 0 / 0 | 402 / 328 | 18,940 / 9,588 |
| `DisFonts_LCZEHUNPOL_SF.gfxfontlib` | ChaletComprime / Emerge BF | `$NormalFont` / `$TitleFont` | 188 each | 187 each | as above | 2,163 / 5,000 | 344 / 351 | 8,223 / 14,067 |
| `DisFonts_LCZEHUNPOL_SF.fonts_czehunpol` | Emerge BF / ChaletComprime | — | **181** each | 180 each | as above | 5,000 / 0 | 319 / 315 | 12,736 / 7,540 |

One more number out of the same run, because a recursive flattener with a depth cap has to be shown
not to truncate: retail's `flattenQuadraticCurve` (0xa5a1a0) has **no** cap and stops on the
collinearity test alone, this one carries a cap of 24 as a safety net, and the **deepest subdivision
any glyph of any of the 12 fonts reaches at tolerance 10.0 is 4** (ChaletComprime 2, Emerge BF 3,
Goudy Stout 4, Benguiat 4, PragmaticaCondC 3). The cap is nowhere near being hit, and `--fonts` prints
the figure so it stays honest.

Agent BB named the two English fonts and their 188 glyphs; **the four localised faces are new here** —
Goudy Stout is the Russian title face, and the Russian `fonts_rus` pair carries 269 and 288 glyphs
(the Cyrillic range), of which 210 of PragmaticaCondC's 288 have outlines and 78 are blank. The single
blank glyph in each 188-glyph font is the space.

Then `--raster --string Dishonored --size 32 --dump`, which writes one binary PGM per glyph plus a
combined strip (`build/agentCB/glyphs/`). Both fonts, 10 glyphs each, **20 rasterised, 0 blank, 0 with
no glyph, 2,802 covered pixels**:

```
  size           32.0 px  -> glyph scale 0.031250 px per 1024-EM unit
  font 0 'ChaletComprime-CologneEighty' (export '$NormalFont')
    char   glyph bitmap    covered   origin px        advance   contours/verts
    'D'    36     12x24    144          0.44,21.86   11.53     2/39
    'i'    73      6x26    66           0.16,23.14   5.03      2/12
    's'    83     12x21    125         -0.55,17.75   9.75      1/83
    ...
    total        10 glyphs rasterised, 0 blank, 0 with no glyph; 1212 covered pixels; pen advanced 100.41 px
  font 1 'Emerge BF' (export '$TitleFont')
    'D'    36     21x26    250          0.12,23.47   19.78     2/72
    ...
    total        10 glyphs rasterised, 0 blank, 0 with no glyph; 1590 covered pixels; pen advanced 149.28 px
```

**The bitmaps are letters.** `build/agentCB/glyphs/strip_f0.pgm` and `strip_f1.pgm`, rendered as
coverage ramps:

```
              @@#            @@#                                      %@%
   @@@@@@@@.  @@#            @@#                                      %@%
   @@#   @@%                 @@#                                      %@%
   @@#   #@@  @@#   %@@@@@#  @@%@@@@*  .@@@@@@%   @@@@@@@%   .@@@@@@%   @@@@@@@% .@@@@@@%    %@@@@@@%
   @@#   #@@  @@#  *@@. -@@+ @@@. +@@= @@@  .@@#  @@@  -@@+  @@@  .@@#  @@%  =@@-@@@  .@@*  %@@: -@@%
   @@#   #@@  @@#   +@@@=    @@#  .@@+ @@#   %@%  @@#  .@@+  @@#   %@%  @@#      @@@@@@@@#  @@#   %@%
   @@#   #@@  @@#     -@@@*  @@#  .@@+ @@#   %@%  @@#  .@@+  @@#   %@%  @@#      @@#::::::  @@#   %@%
   @@@@@@@%   @@#  .@@@@@@%  @@#  .@@+ .@@@@@@%   @@#  .@@+  .@@@@@@%   @@#      .@@@@@@%   =@@@@@@@%
```

The arithmetic checks out independently. At 32 px the font's declared ascent is
`895/1024 × 32 = 27.97` px, and the tallest glyph bitmaps come out at 26 px including the 2 px of
padding — a 24 px ascender, 0.75 EM, which sits inside the declared ascent as it must; `'D'` is a
24 px bitmap, so a 22 px cap height, 0.69 EM. `'i'` is 6 px wide in the condensed face and 10 px in
the title face, and the title face's advances are 1.49× the condensed one's over the same string
(149.28 px against 100.41 px), which matches their mean advances of 0.472 and 0.318 EM.

## 4. Acceptance 2: the menu asset's text fields lay out and produce glyphs

`GFx3Text --run Dishonored_MainMenu.MainMenu.gfx --fontlib DisFonts_SF.gfxfontlib.gfx --verbose`, from
the cmake-built binary (`build/agentCB_rel/Binaries/Win32/GFx3Text.exe`):

```
  fontlib        DisFonts_SF.gfxfontlib.gfx: 2 fonts, 2 resources
  asset          Dishonored_MainMenu.MainMenu.gfx  GFX v10  1280 x 720 px  5 frames  477 tags

  -- text fields --
  DefineEditText tags          12
  fields built                 12
  fields that resolved a font  12
  fields that laid out         12  (12 lines in all)
  fields with glyph output     12
  glyph entries laid out       122
  glyphs rasterised            112
  glyphs with no outline       0
  atlas                        1 textures, 49 cached glyphs, 48 rasterised, 1 blank
  covered pixels               31899

  id     lines  glyphs  raster  missing w twips   h twips   text
  32     1      4       4       0       607       575       'Text'
  54     1      8       7       0       1303      575       'DLC NAME'
  55     1      8       7       0       2495      575       'DLC Size'
  62     1      58      50      0       10516     575       'No downloadable content installed on Hard Disk Drive (HDD)'
  86     1      2       2       0       1808      2820      '09'
  88     1      17      17      0       5100      682       'WWWWWWWWWWWWWWWWW'
  109    1      1       1       0       308       575       'W'
  111    1      1       1       0       308       575       'W'
  126    1      4       4       0       613       617       'TEXT'
  158    1      5       5       0       6676      575       'PRESS'
  168    1      11      11      0       1688      596       'Description'
  206    1      3       3       0       478       575       'DLC'
```

That is the menu's **own** `DefineEditText` initial text, laid out at the field's own font height, with
real glyph output: 58 characters of "No downloadable content installed on Hard Disk Drive (HDD)" fill
10,516 twips = 525.8 px. The `WWWWWWWWWWWWWWWWW` and `W` fields are the author's width gauges, which is
exactly what a Flash author leaves in a menu.

### Every payload in the cook

`build/agentCB/run_all.py`, all 22 through the same harness, all exit 0:

| asset | editText | built | laidOut | withGlyphs | entries | raster | noOutline | pixels |
|---|---|---|---|---|---|---|---|---|
| `UI_HUD_SF.HUD` | 33 | 33 | 33 | 33 | 461 | 447 | 0 | 95,726 |
| `Startup.OptionsMenu` | 53 | 53 | 53 | 53 | 280 | 267 | 0 | 51,168 |
| `DishonoredGame.Note` | 41 | 41 | 41 | 41 | 554 | 488 | 0 | 88,291 |
| `UI_Journal_SF.Journal` | 31 | 31 | 31 | 31 | 255 | 238 | 0 | 45,962 |
| `UI_HUD_DLCTest_SF.HUD` | 28 | 28 | 28 | 28 | 413 | 402 | 0 | 84,268 |
| `DishonoredGame.lib` / `Startup.lib` | 14 each | 14 | 14 | 14 | 61 | 61 | 0 | 13,616 |
| `UI_Shop_SF.Shop` | 14 | 14 | 14 | 14 | 129 | 123 | 0 | 27,515 |
| `Dishonored_MainMenu.MainMenu` | 12 | 12 | 12 | 12 | 122 | 112 | 0 | 31,899 |
| `DishonoredGame.Global` | 9 | 9 | 9 | 9 | 44 | 42 | 0 | 8,407 |
| `UI_MissionStats_SF.MissionStats` / `UI_PowerWheel_SF.powerwheel` | 6 each | 6 | 6 | 6 | 34 / 41 | 30 / 38 | 0 | 8,545 / 7,414 |
| `Startup.LoadGame` | 5 | 5 | 5 | 5 | 33 | 31 | 0 | 7,294 |
| `UI_PauseMenu_SF.PauseMenu` | 4 | 4 | 4 | 4 | 22 | 21 | 0 | 9,127 |
| the six `DisFonts*` movies | 2 each | 2 | 2 | 2 | 21 | 19–21 | 0 | 3,204–4,759 |
| `DishonoredGame.HUDFX`, `UI_Gamma_SF.GammaImage` | 0 | — | — | — | — | — | — | — |
| **total** | **282** | **282** | **282** | **282** | **2,636** | **2,481** | **0** | **516,737** |

The 155 entries that are not rasterised are spaces: they have no outline, get a zero-size cache node
and advance the pen, which is what retail does with them.

## 5. The layout engine, verified on its own

`--layout` is the third acceptance and it is the one that proves the *formatter* rather than the
rasteriser. One string, one 260×140 px box, the four alignments:

```
  box            260 x 140 px = 5200 x 2800 twips, less the 40-twip gutter a side
  size           18.0 px   word wrap on
  string         'The Outsider left his mark upon my hand and the world was never the same again'

  align left     lines 2   text 5017 x 770 twips = 250.9 x 38.5 px   78 glyphs, 63 rasterised
    line  offX     offY     width    height   ascent   leading  text
    0     0        0        5017     385      314      0        'The Outsider left his mark upon my hand and the '
    1     0        385      3290     385      314      0        'world was never the same again'
  align right    lines 2   ...   line 0 offX 103, line 1 offX 1830
  align centre   lines 2   ...   line 0 offX 52,  line 1 offX 915
  align justify  lines 2   ...   line 0 width 5117 (stretched), line 1 width 3290 (last line, left)
```

Every number is checkable and checks out. The available width is 5,200 − 80 = **5,120** twips; right
alignment puts line 0 at 5,120 − 5,017 = **103**; centre at 5,120/2 − 5,017/2 = **52**; justify
stretches line 0 to 5,117 of 5,120 over its nine spaces and leaves the last line alone. The line
height is 385 twips: ascent `895/1024 × 18 = 15.73` px = 314 twips plus descent `200/1024 × 18 = 3.5`
px = 70 twips, and 314 + 70 = 384 ≈ 385 after the round-half-away-from-zero the retail body does.

## 6. Two real defects the harness found

1. **A by-value `GArray` copy corrupting the heap.** `GTypes.h`'s `GArray` has a destructor and **no
   copy constructor or assignment operator**, so `GArray<T> a = b;` shallow-copies the buffer and both
   copies free it. The word-wrap path did exactly that to carry a line's format table onto the next
   line, and `--run` on the menu asset died with `STATUS_HEAP_CORRUPTION` (0xC0000374) before printing
   a line. Fixed at all five sites with an explicit element-wise copy helper, commented at the
   definition. **This is a trap for every other package that uses `GArray`**: see the hand-over.
2. **Justify stretched the last line.** `FinalizeLine`'s justify arm distributes the slack over the
   line's whitespace; retail does that only for a line that is *not* the paragraph's last (the
   `!IsLastLine` test in 0xa9a940). The first implementation tested "does the line end in a newline",
   which is false for the last line of a paragraph that has none, so the final line came out stretched
   to the full box width. Fixed with a flag set by `Format`'s closing call, and the `--layout` output
   above is the before/after.

Both were found by the harness rather than by reading, which is the point of having one.

## 7. Acceptance 3: implemented against remaining

`GFx3Text --table` (full output `build/agentCB/table.txt`). **966 retail functions in the
text-and-fonts group, 214 ported, 22.2 %.** The counts are from `functions.csv`, `module=libgfx`,
grouped by the demangled owning class, which is what `build/agentCB/surf2.py` prints.

| group | retail | ported | note |
|---|---|---|---|
| **`GRasterizer`** | 13 | **13** | all of it |
| `GCompoundShape` | 14 | 11 | the flattener |
| `GFxFontData` | 15 | 11 | `Read`, the code table, every accessor |
| `GFxTextFormat` | 33 | 24 | defaults, every setter, `Merge`, `Intersection`, `Hash` |
| `GFxTextParagraphFormat` | 24 | 17 | alignment, margins, `Merge`, `Intersection`, `Hash` |
| `GFxFontHandle` | 4 | 3 | |
| `GFxFontResource` | 13 | 8 | plus the two hinting tops |
| `GFxParagraphFormatter` | 9 | 6 | `Format`, `InitParagraph`, `FinalizeLine`, `CheckWordWrap` |
| `GFxTextParagraph` | 17 | 12 | |
| `GFxFontManager` | 12 | 6 | |
| `GFxTextLineBuffer` | 21 | 9 | the store and the accessors; `Display` is CC's |
| `GFxGlyphRasterCache` | 19 | 8 | `CalcGlyphParam`, `Init`, `GetGlyph`, the raster core |
| `GFxTextDocView` | 97 | 31 | `Format`, `SetViewRect`, the setters, `FindFont`, the metrics |
| `GFxStyledText` | 41 | 16 | |
| `GFxShapeBase` / `GFxConstShapeNoStyles` | 42 | 12 | the record walk and the bound |
| `GFxLineCursor` | 8 | 3 | |
| `GFxTextAllocator` | 7 | 3 | the interning |
| `GFxEditTextCharacter` | 125 | 14 | construction, formats, text, layout, 13 AS2 properties |
| `GFxEditTextCharacterDef` | 7 | 3 | the record, as `GFxTextFieldReadDesc` |
| `GFxGlyphSlotQueue` | 21 | 3 | the shelf allocation only |

The **zero** rows, with why each one is zero — this is the part the next wave plans from:

| group | retail | why not |
|---|---|---|
| `GTessellator` | 60 | triangulation: the renderer's, **package CC** |
| `GFxFontCacheManager(Impl)` | 23 | the batch package submits through `GRenderer`: **package CC** |
| `GFxTextEditorKit` | 31 | the caret, selection and key handling: **needs the input path** |
| `GFxFontDataCompactedSwf/Gfx` + `GFxFontCompactor` | 51 | GFx's own compacted glyph stream; **the cook is plain `DefineFont3`, so unreachable** |
| `GFxEditTextCharacter`'s AS2 methods | 26 | `replaceSel`, `getTextFormat`, `getLineMetrics`, the clipboard, the image substitution |
| `GASTextFieldObject/Proto`, `GASTextFormatObject/Proto`, `GASTextSnapshot*`, `GASStyleSheet*` | 71 | the AS2 class objects: **package BC's class library** |
| `GFxTextDocView`'s filter properties | 22 | the shadow and blur go through the glyph cache's filter path |
| `GFxStyledText::ParseHtml*` | 4 | 6,268 bytes of HTML parser; **the markup is stripped instead and the site says so** |
| `GFxTextHighlighter`, `GFxTextCompositionString`, `GFxTextClipboard/KeyMap`, `GFxTextStyleManager` | 46 | selection, IME, editing, CSS |
| `GFxGlyphFitter` | 9 | auto-hinting |
| `GFxFontGlyphPacker`, `GFxTextureGlyph(Data)`, `GFxTextureFont` | 28 | pre-baked font textures. **`ExportFlags` is 0 in every cooked asset and there is no 1002/1005 tag anywhere in the cook (agentBB.md 3.4), so this whole family is unreachable in this game** |
| `GFxSwfPathData` iterators, `GFxPathPacker/Allocator` | 28 | the lazy decode and the packed path form: section 8 |
| `GFxFontMap` | 4 | the font-name substitution table |
| `GFxStaticTextCharacter` | 15 | `DefineText`; CD measured that no asset in the cook carries one |

## 8. What is deliberately not 1:1, stated at the site and here

Four, and no more:

1. **`GFxConstShapeNoStyles` decodes eagerly.** Retail keeps the raw SWF shape bytes and walks them
   lazily through `GFxSwfPathData::PathsIterator::ReadNext` (0xa3a7f0) on every traversal, with the
   shape and path counts appended to the end of the blob (0xa3b5c0 reads them off the end). This
   decodes the same records once, at load, into `GFxShapePath`/`GFxShapeEdge`. **The decoded semantics
   are retail's** — read out of `ReadNext` and `GetEdge` — only the moment of decoding differs, and
   nothing here is ABI-visible. Cost: a glyph's storage is the decoded paths rather than the tag bytes
   (8,223 vertices for a whole 188-glyph font, so this is not a memory problem).
2. **`Line` and `GlyphEntry` are plain structs.** Retail's are hand-packed variable-length
   allocations with fields at two different offset sets depending on a high bit. The *semantics* are
   reproduced exactly — the 1/16-px size quantum, the signed advance, `descent = height - baseline`,
   `leading = max(leading, 0)` — and `CalcLineSize` is kept because `InitParagraph` branches on it.
3. **The font registry is process-wide, not per movie definition.** Retail hangs a `GFxFontManager`
   off each `GFxMovieDefImpl` and a UI movie reaches the fontlib's fonts through import binding
   (`GFx_ImportLoader` 0xa385f0), which this tree does not have yet — package CD is adding it and
   `ImportAssets2` symbols are placeholders today (agentBC.md 6.2). Until then one shared registry is a
   close approximation of what `gfxfontlib` *is*: one movie whose two exported fonts every UI movie
   imports. **When CD's import binding lands this should become per-movie-def**; the accessor
   (`GFxTextGetFontManager`) can stay as the fallback.
4. **The glyph atlas fills and then refuses instead of evicting.** `GFxGlyphSlotQueue`'s shelf
   allocation is ported; its LRU extrusion (`extrudeOldSlot` 0xa4f100) and neighbour merging
   (`mergeSlotWithNeighbor` 0xa4e1d0) are not, so a long-running movie that cycles through many sizes
   would eventually get null nodes rather than evicting old glyphs. The harness reports a miss when
   that happens; it has not happened on any asset in the cook (one 1024×1024 texture holds the menu's
   49 glyphs).

Two smaller ones, both commented at the site: the font **size ramp** (`snapFontSizeToRamp` 0xa4bdd0)
has retail's *shape* (exact below 16 px, then coarser) but **its exact table is not measured**, because
the table lives in the font-cache manager's state bag; and `SetTextValue`'s **HTML branch strips the
markup** rather than parsing it, decoding only the five entities the cook uses.

## 9. Hand-overs

**Agent CD — the interface, and the twelve lines.** `GFxShape.h` is stable and yours to depend on:
`GFxConstShapeNoStyles::Read(GFxStream*, unsigned tagType, unsigned endPos)` is the record walk, and
`GFxShapeBase::MakeCompoundShape(GCompoundShape*, float tolerance)` is what the rasteriser and the
tessellator both consume. Three things:

1. **I removed the `GFxEditTextCharacterDef` collision from my side**, so your definition class is the
   only one. To make a text field from it, `CreateCharacterInstance` (0xa32df0) becomes:

   ```cpp
   GFxTextFieldDesc d;
   d.TextRectTwips   = TextRect;
   d.FontId          = FontId;
   d.FontHeightTwips = FontHeight;
   d.TextColor       = TextColor;
   d.MaxLength       = MaxLength;
   d.Align           = Alignment;                 // the raw SWF byte; the remap is mine
   d.LeftMarginTwips = LeftMargin;   d.RightMarginTwips = RightMargin;
   d.IndentTwips     = Indent;       d.LeadingTwips     = Leading;
   d.Flags           = Flags;                     // same bits, same values, both from 0xa26190
   memcpy(d.VariableName, VariableName, sizeof(d.VariableName));
   memcpy(d.InitialText,  InitialText,  sizeof(d.InitialText));
   return new GFxEditTextCharacter(d, parent, id, defImpl ? parent->GetMovieRoot() : 0);
   ```

   Your `Flags` enum and mine agree bit for bit by construction, because we read the same body. The
   one thing to keep: your `Flag_UseFlashType 0x400` (set by `GFx_CSMTextSettings` 0xa26ca0) exists in
   my descriptor too and is carried through unused.
2. **`GFx_DefineFontLoader` (0xa357d0) should register with the font manager**, not only leave a
   dictionary entry. `GFxFontData::Read(GFxStream*, tagType, tagEnd)` in `GFxFont.h` is the reader;
   wrap the result in a `GFxFontResource`, call `SetExportName` with the `ExportAssets` name if the
   movie exports the character, and `GFxTextGetFontManager()->AddFont(res)`. `GFxFontLoadFromPayload`
   does exactly that over a whole payload and is what the harness uses — you can call it or copy the
   twenty lines. Once your **import binding** lands, the registry should move to the movie def
   (section 8 item 3) and I would rather you make that change than me, since it is your resource
   binding.
3. Your `GFxFontCharacterDef` keeps its own `GFxConstShapeNoStyles**` glyph array. That is fine and
   duplicates nothing of mine, but note that `GFxFontData` already holds the glyphs *and* the code
   table, the advances, the bounds in twips-of-glyph-units and the kerning hash — so if the two end up
   side by side, `GFxFontCharacterDef` can hold a `GPtr<GFxFontData>` and drop its four arrays.

**Agent CC — the seam between us is one loop, and here it is.** I deliberately did not write any
`GRenderer` call. `GFxEditTextCharacter::ProduceGlyphs` is retail's
`GFxTextLineBuffer::Display` (0xa45bf0, 8,522 bytes) traversal with the submission left out, and the
body of its inner loop is where your mesh submission goes. What you need to know:

* the traversal is **line-major, then glyph-major**, and each `GlyphEntry` carries a `FormatIndex`
  into the line's `Formats` array, which holds the `GFxFontResource*`, the `GColor`, the font size in
  pixels and the underline flag. A format run is exactly a state change;
* the per-glyph transform retail builds is
  `translate(line.OffsetX + pen, line.OffsetY + line.BaselineOffset) * scale(sizePx / 1024)`, in
  **twips**, and the pen advances by each entry's `Advance` (also twips, and it may be negative);
* the glyph's pixels come from `GFxGlyphRasterCache::GetGlyph(param)`, which returns a `GFxGlyphNode`
  with the atlas texture index, the rectangle `X/Y/Width/Height` inside it, and `OriginX/OriginY` —
  **the bitmap's top-left relative to the pen, with y measured upwards**. A node with a zero-size
  rectangle is a space;
* the atlas is a `GImage` of format `Image_A_8` (`GFxGlyphRasterCache::GetTexture(i)`), which is what
  retail's glyph textures are. **It needs a `GTexture`**: today nothing uploads it, and
  `GFxGlyphRasterCache::UpdateTextures` (0xa4db30) is the retail function that does — it takes a
  `GRenderer*`, so it is on your side of the line. Give me a hook or take the cache; either is fine,
  but say which in your report;
* `GFxTextLineBuffer::DrawUnderline` (0xa44c50) and `GFxTextDocView::HighlightDesc::DrawBackground`
  (0xaa03c0) are also yours, and the flags they need (`EF_Underline`, the colour) are already on the
  entries.

**Agent BB — one trap in `GTypes.h` that cost me an afternoon and will cost the next package the
same.** `GArray<T>` has a destructor (`~GArray { Clear(); }`) and **no copy constructor and no
assignment operator**, so the implicit ones shallow-copy `Data` and both objects free it. That is a
heap corruption, not a leak, and it is silent until it is not (section 6 item 1). Either delete the
two implicit members so a by-value copy fails to compile, or define them; the first is cheaper and
catches every caller at build time. I did not edit `GTypes.h` — it is yours and it is assertion-guarded
— but every package that stores a `GArray` in a struct it copies is exposed.

**Agent BC — two answers.** (1) Your section 6 item 3 said "nothing the UI does will show a character
of text until this lands". It has landed to the extent this report measures: 2,481 glyphs rasterised
across the cook. What is still missing between here and a visible menu is the *drawing*, which is
CC's. (2) Your `GFxValue`/`GASValue` machine is untouched: `GFx3Run --run MainMenu --frames 5` on my
own build still reports **85 sprites, 170 display objects, 3,783 opcodes, 7 script errors** — byte for
byte your numbers — so nothing I added changed the AS2 path.

**Coordinator — four things.**
1. `gfx_decision.md` 2.6's "270 KiB / 1,140-function text-and-fonts group" can be replaced with a
   measurement: the group is **966 functions** as `functions.csv` groups it, of which **214 are
   ported**. Section 7 has the per-class table.
2. `middleware.md` 2.3 should gain the line that the reconstruction now rasterises the game's own
   `DefineFont3` outlines and lays text out, with the numbers of sections 3 to 5.
3. **The one edit outside my directory is `cmake/GFx.cmake`**: eight units appended to the `gfx3`
   source list inside the existing `if(DISHONORED_WITH_GFX3)` guard, and a `GFx3Text` target appended
   before the closing `message(STATUS ...)`. **Agent CD has since added its own two units to the same
   list and the two edits coexist in the shared tree as it stands** — mine after `GFxPlayerRoot.cpp`,
   CD's after mine — so the merge-ready state is already there and needs nothing from either of us. My
   *snapshot* deliberately uses a different copy: `build/agentCB/patch_snapshot_cmake.py` regenerates
   `cmake/GFx.cmake` from `HEAD` plus my block only, because CD's two sources are not in a snapshot
   that holds my files alone and cmake fails on a missing source. That is why `cmake/GFx.cmake` is not
   in `build/agentCB_files.txt`; the shared-tree version is the one to commit.
4. **`GFxPlayer.h` is being edited by CD while I worked** (it is modified in the shared tree). My units
   include it for `GFxStream`, `GFxCharacterDef`, `GFxASCharacter` and `GFxMovieRoot::CreateString`
   only; if CD moves `GFxStream` out of it, my four `#include "GFxPlayer.h"` lines need to follow it.

## 10. Verification

* **The stack on its own** (`build/agentCB_run.cmd`, VS 2022 x86, `/Zp4`, no engine): 0 errors, 0
  warnings at `/W3`, `GFx3Text.exe` produced. This is the fast loop and it proves the same thing BB's
  seam script and BC's run script prove: the text engine needs no engine.
* **The snapshot, not the shared tree.** `python resources/tools/make_snapshot.py CB --list
  build/agentCB_files.txt` → `build/agentCB_wt` (detached at HEAD `2cdd7b3`) plus my 17 files.
  Deliberate: the shared tree held 98 modified files from six other packages while I worked, including
  three of BC's GFx3 files and three new files of CD's.
* **But also against the shared tree, for the merge.** All nine of my translation units compile clean
  against the shared tree's *current* `GFxPlayer.h` — i.e. against CD's in-flight edits to it — with
  **0 errors and 0 warnings at `/W3`** (`build/agentCB_cc.cmd`). So the merge risk on my side is
  `cmake/GFx.cmake` and nothing else.
* **Isolated full RELEASE build** (`build/agentCB_relbuild.cmd`): **863 units, 0 errors, 0 link
  errors**, all six targets produced — `DishonoredGame.exe`, `CoreSmoke.exe`, `LayoutProbe.exe`,
  `GFx3Dump.exe`, `GFx3Run.exe`, `GFx3Text.exe` — with `-- GFx: DISHONORED_WITH_GFX3=1` in the log
  (`build/agentCB_relbuild.log`). **The cmake-built `GFx3Text.exe` reproduces every number in sections
  3 to 5 exactly**, which is what proves the build wiring rather than only the hand-rolled script.
* **Regression**: `python resources/tools/run_regression.py --build-dir build/agentCB_rel --no-build`
  → **31 ok, 0 failed, 0 skipped, 430 s** (`build/agentCB_regression.txt`). CoreSmoke 99/0, layout
  2,314 types with 0 mismatches and 0 contract mismatches, nullrhi 0 criticals, d3d9 20,220 frames /
  0 criticals / 6,506 draw elements / 16,257 textures, inputtest **pawn walks 1,014.6 units** with
  0 criticals, 895 PhysX actors, 1,312 static shapes, **unported natives 0**, probe natives 7.
* **`-newgame` still works** on the same exe (`build_and_smoke.py … --exe-name DishonoredGame_CB.exe
  --rhi null "--extra-args=-newgame -forcelogflush"`, exit 0): `Initial startup: 2.80s`, then two
  `Committed map change via DishonoredEngine` lines — the retail New Game route end to end — with
  **0 criticals** in 936 normalised log lines (`build/agentCB_newgame_out.txt`).
* **With the switch off**: the same snapshot configures cleanly with `-DDISHONORED_WITH_GFX3=OFF`
  (`build/agentCB_off`, log `build/agentCB_offconfig.log`): **0 references to any of my units in
  `build.ninja`** and no `GFx3Text` target. So the wiring can be turned off without editing a file.
* **Nothing instantiates the text engine in a normal run.** The eight units are in the `gfx3` static
  library `dishonored_apply_defines()` already linked into every target for BB; they include no engine
  header, declare no `UObject`, and no engine or `GFxUI` translation unit names `GFxEditTextCharacter`,
  `GFxTextDocView`, `GRasterizer` or `GFxFontData`. The regression numbers above are the evidence.
* **All 242 cited retail functions** are in `agentCB_status.csv` with their 2013 rvas: **185 ported,
  57 cited as evidence or named as remaining, and 242 of 242 byte-identical 2012↔2013** (ratio 1.000,
  no exceptions).

## 11. What the next wave should do here, in order

1. **The drawing** — package CC. Everything above is upstream of one loop (section 9).
2. **`GFxTextLineBuffer::Display`'s own half**: the mask (`DrawMask` 0xa44ae0), the underline
   (`DrawUnderline` 0xa44c50) and the scroll clipping. They belong with CC's submission.
3. **The AS2 `TextField` and `TextFormat` class objects** (71 functions with `TextSnapshot` and
   `StyleSheet`). The cook's own CLIK components set `.text`, `.autoSize` and `.textColor` through the
   property surface that is here; they call `setTextFormat`, `getTextFormat` and `getLineMetrics`
   through the class object that is not.
4. **`GFxTextEditorKit`** (31) plus `Selection` and `Key` — the moment the input path lands, this is
   what makes a text field editable, and the options menu's name entry needs it.
5. **The HTML parser** (`ParseHtmlImpl` 0xaa7630, 6,268 bytes) and `GFxTextStyleManager`'s CSS. The
   journal and the note assets set `htmlText`; today their markup is stripped, so their bold and
   coloured runs come out flat.
6. **The glyph filters**: `stackBlur` (0xa4d5b0), `recursiveBlur` (0xa4da90), `strengthenImage`
   (0xa4bbd0), `knockOut` (0xa4c230) and the `GFxTextDocView` shadow properties (22 functions). The
   HUD's drop shadows are these, and `CalcGlyphParam` already computes the parameters they need.
7. **The atlas's LRU** (section 8 item 4) before anything animates a text field's scale.
8. **`GFxGlyphFitter`** (9) — auto-hinting — last: it changes small-size quality, not correctness, and
   `GFxFontResource`'s two reference heights are already measured and cached for it.

## 12. Files

Mine (19): the 16 files of `source/Development/Src/External/GFx3/` and `Tools/` listed in section 1,
a targeted edit to `cmake/GFx.cmake`, plus this report and `agentCB_status.csv`. The snapshot list is
`build/agentCB_files.txt` (my 16 sources; `cmake/GFx.cmake` is deliberately not in it, section 9
item 3).

Scratch (not repo tools): `build/agentCB/patch_snapshot_cmake.py` (the snapshot's own `GFx.cmake`,
section 9 item 3), `build/agentCB/surf.py` + `surf2.py` (the libgfx surface from the demangled
names, grouped by owning class), `build/agentCB/dec{,2,3,4}/` (126 headless decompiles) and
`dec{1..6}.txt` (their function lists), `build/agentCB/make_status.py` (the status CSV generator),
`build/agentCB/run_all.py` + `run_all.txt` (all 22 payloads), `build/agentCB/runs/*.txt` (the four
acceptance runs), `build/agentCB/table.txt`, `build/agentCB/glyphs/*.pgm` (the dumped glyph bitmaps and
the two strips), `build/agentCB_cc.cmd` (the per-unit compile check), `build/agentCB_run.cmd` (the
no-engine build), `build/agentCB_relbuild.cmd` + `build/agentCB_relbuild*.log`,
`build/agentCB_regression.txt`, `build/agentCB_newgame_out.txt`, `build/agentCB_offconfig.log`,
snapshot `build/agentCB_wt` (detached at `2cdd7b3`) and the two build directories `build/agentCB_rel`
(GFx3 on) and `build/agentCB_off` (configure-only, GFx3 off).

IDA: one own copy, `resources/docs/idb/shipping2012_agentCB.i64`, opened headlessly through
`resources/tools/ida/run.py` only; **no MCP tool of any kind was used, and no FModel tool**. No
commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.

---

# Addendum — agent CD's two follow-ups (2026-09-27, after the merges at `54b57d4` and `683fa03`)

Package CB merged at `54b57d4`, package CD at `683fa03`. CD's report left this package two follow-ups
(`agentCD.md` section 8, hand-over 1 and 2). Both are taken. One of the three defects CD reported is
**not** a defect, and chasing that down turned the second follow-up into a better outcome than either
report proposed: **two functions this package had invented are deleted rather than corrected.**

| follow-up | outcome |
|---|---|
| 1. The twelve-line adapter in `GFxEditTextCharacterDef::CreateCharacterInstance` (2012 0xa32df0) | **done**: **282 of 282** edit-text definitions in the cook now become live `GFxEditTextCharacter` instances instead of `GFxGenericCharacter`, all 282 lay out and all 282 produce glyph output |
| 2. Three claimed transcription slips in my style skippers | **two confirmed, one refuted, and both functions deleted**: they were an invention, not a port — retail has no such walk. Section A.2 |
| Verification | **green**: full RELEASE build at `683fa03` + my 6 files, **31 ok / 0 failed / 0 skipped**, `-newgame` 0 criticals, and every text and glyph number in this report unchanged |

## A.1 The adapter, and the proof that is not an assertion

`GFxEditTextCharacterDef::CreateCharacterInstance` returned a `GFxGenericCharacter`; it now fills a
`GFxTextFieldDesc` field for field and returns a `GFxEditTextCharacter`. Thirteen lines and a
`#include`, exactly as section 9's hand-over specified, in CD's `GFxCharacterDefs.cpp`.

The interesting part is proving it. `--run` builds a field straight from a `DefineEditText` body, which
exercises my reader but not the wiring, so the harness gained **`--defs`**: it builds the movie's
dictionary through **CD's** tag loaders, then asks every edit-text definition in it for a character
instance and checks what comes back. On the main menu:

```
  asset          Dishonored_MainMenu.MainMenu.gfx  292 dictionary entries
    id 32    EditText  lines 1  glyphs 4   raster 4   'Text'
    id 62    EditText  lines 1  glyphs 58  raster 50  'No downloadable content installed on Hard Disk Drive (HDD)'
    id 158   EditText  lines 1  glyphs 5   raster 5   'PRESS'
    ...
  edit-text definitions in the dictionary   12
  character instances created               12
  came back an EditText (the adapter)       12
  came back something else                  0
  of those, laid out                        12
  of those, produced glyph output           12
  glyph entries 122, rasterised 112
```

Over all 22 payloads (`build/agentCB/defs_all.txt`):

| | edit-text defs | instances | **came back EditText** | came back something else | laid out | with glyph output | entries | rasterised |
|---|---|---|---|---|---|---|---|---|
| **total** | **282** | **282** | **282** | **0** | **282** | **282** | **2,636** | **2,481** |

`Startup.OptionsMenu` 53, `DishonoredGame.Note` 41, `UI_HUD_SF.HUD` 33, `UI_Journal_SF.Journal` 31,
`UI_HUD_DLCTest_SF.HUD` 28 — and **the 2,636 / 2,481 totals are identical to the ones `--run` reports
by building the fields directly from the tag bodies**, which is the cross-validation: two independent
routes to the same 282 fields agree to the glyph.

## A.2 The three claimed slips: two real, one not, and why both functions are now gone

CD cited `GFxFillStyle::Read` (0xa90290), `GFxLineStyle::Read` (0xa907d0) and
`GFxLoadProcess::ReadRgbaTag` (0xa22460). I decompiled all three from my own database rather than take
either report on trust, and then found the function CD and I had both been comparing against the wrong
thing.

| claim | verdict against retail |
|---|---|
| my `GFxSkipFillStyles` read a focal gradient's focal point **before** the gradient records | **CONFIRMED.** 0xa90290 reads the matrix, then the info byte, then loops `info & 0xF` times through `GFxGradientRecord::Read`, and **only then** `if (*this == 19) ReadU16() * 0.00390625` — the focal point is last, and it is an 8.8 fixed value |
| my `GFxSkipLineStyles` tested the DefineShape4 miter flag as `0x0800` | **CONFIRMED.** 0xa907d0: `if (a3 == 83) { flags = ReadU16(); if ((flags & 0x20) != 0) ReadU16() * 0.00390625; }`. The flag is **0x20** and the value is an 8.8 fixed miter limit. CD's own `GFxLineStyle::Flag2_HasMiterLimit = 0x20` is right |
| my colour skips "branch on the tag type at the wrong threshold … a test that puts DefineShape2 on the four-byte side" | **NOT A DEFECT.** 0xa22460 is `if (tagType > 22) ReadRgba else ReadRgb`, i.e. DefineShape2 is on the **three**-byte side — which is exactly where my `GFxShapeTagHasAlpha` already put it. Mine was `tagType == 32 \|\| tagType == 83`, and for all four DefineShape tags {2, 22, 32, 83} that is the same answer retail gives. CD's fix to *its own* reader is right; the transcription of it into a claim about mine is not |

So far that is one report correcting another. The finding that matters came from asking which retail
function my helpers were a port *of*. Not the two style readers: the record walk
(`GFxConstShapeNoStyles::Read`, 0xa42ab0) **contains no style-array reading at all**, and its
`StateNewStyles` arm instead calls two helpers I had not identified —

```
      v29 = sub_A429C0(a3);          // GFx_ReadFillStyles, 2012 0xa429c0
      ...
      v31 = sub_A41270(a3);          // GFx_ReadLineStyles,  2012 0xa41270
      ... memmove ... SetPosition(v33);
      UInt = ReadUInt(&v93, 4u);     // fillBits again
      v86  = ReadUInt(&v93, 4u);     // lineBits again
```

— whose names come from their own error strings. Decompiled, they say something neither report did:

1. `GFx_ReadFillStyles` reads a `u8` count, promoted by a `0xFF` sentinel **only when `tagType > 2`**;
   `GFx_ReadLineStyles` reads its count and promotes it **unconditionally**. The two thresholds differ,
   and neither is the `>= 22` my helpers used or the `> 22` CD described.
2. **When there is no style owner — which is precisely the no-style shape this class is — neither
   helper walks anything. It logs an error and returns**:
   `"Error: GFx_ReadFillStyles, trying to read %d fillstyles into no-style shape"`. Only with an owner
   does it call `GFxFillStyle::Read` / `GFxLineStyle::Read` per style, and the record walk then
   `memmove`s those bytes *out* of the blob it is building — which is why
   `GFxSwfPathData::PathsIterator::ReadNext` (0xa3a7f0) only ever accumulates style *bases*.

**So my two helpers were not a port with three errors in it; they were an invention of a wire-format
walk retail does not perform.** Correcting the two real slips would have left a function that is still
not retail's. Both are therefore deleted, and the `StateNewStyles` arm now does what 0xa429c0 and
0xa41270 do with a null style owner: consume the two counts (with the two different sentinel rules),
count the refusal, and let the record walk re-read `fillBits`/`lineBits`. That is smaller, faithful,
and it makes CD's `GFxFillStyle::Read` and `GFxLineStyle::Read` **the tree's only implementation of
0xa90290 and 0xa907d0** — the collapse CD asked for, by removal rather than by correction.

The refusal is counted rather than assumed, in `GFxShapeBase::StyleRecordsRefused`, so the harness can
show the number.

## A.3 The cross-check, which is the real verification

Neither record walk can replace the other — CD's `GFxFontCharacterDef` stores **my**
`GFxConstShapeNoStyles` and my rasteriser consumes it, while CD's `GFxShapeRecord` carries the style
arrays my class by definition has none of. So rather than assert the two agree, `--xcheck` runs **both
over the identical byte range of every glyph** (`GFxFontData` now records each glyph's stream range for
exactly this) and compares path and edge counts glyph by glyph:

| payload | glyphs compared | agree | differ | style arrays a no-style shape had to refuse |
|---|---|---|---|---|
| `DisFonts_SF.gfxfontlib` | 376 | 376 | 0 | 0 |
| `DisFonts_SF.fonts_efigs` | 376 | 376 | 0 | 0 |
| `DisFonts_LRUS_SF.gfxfontlib` | 376 | 376 | 0 | 0 |
| `DisFonts_LRUS_SF.fonts_rus` | 557 | 557 | 0 | 0 |
| `DisFonts_LCZEHUNPOL_SF.gfxfontlib` | 376 | 376 | 0 | 0 |
| `DisFonts_LCZEHUNPOL_SF.fonts_czehunpol` | 362 | 362 | 0 | 0 |
| **total** | **2,423** | **2,423** | **0** | **0** |

Two things fall out of that last column. **No glyph record in the entire cook carries a
`StateNewStyles` record with style arrays**, so the deleted helpers were unreachable code on the only
path that ever called them — which is why my harness never caught the two real slips, and why deleting
them changes no output. And the two independent walks agree on 2,423 glyphs, 4,145 paths and 64,060
edges, which is a far stronger statement about either one than either could make alone.

The "changes no output" claim is checked rather than argued: every figure in sections 3 to 5 of this
report is byte-identical after the deletion — 187 of 188 glyphs with outlines, 2,163 and 5,000 kerning
pairs, 20 glyphs and 2,802 covered pixels for `"Dishonored"` at 32 px, the four alignment offsets
103 / 52 / 5,117, and the whole-cook 282 / 2,636 / 2,481 / 516,737.

## A.4 Verification of the addendum

* **Base**: the snapshot was rebuilt at the **new** HEAD `683fa03` (both merges in) plus my six changed
  files — `GFxShape.{h,cpp}`, `GFxFont.{h,cpp}`, `Tools/GFx3Text.cpp` and CD's
  `GFxCharacterDefs.cpp` (the adapter). `cmake/GFx.cmake` needs nothing: HEAD's copy already carries
  both packages' blocks and the `GFx3Text` target, so `build/agentCB/patch_snapshot_cmake.py` is
  retired.
* **Full RELEASE build**: 0 errors, 0 link errors, all six targets; the no-engine build
  (`build/agentCB_run.cmd`, now linking CD's two units as well) is 0 errors and **0 warnings at `/W3`**.
* **Regression**: `run_regression.py --build-dir build/agentCB_rel --no-build` → **31 ok, 0 failed,
  0 skipped, 422 s** (`build/agentCB_regression2.txt`).
* **`-newgame`**: exit 0, `Initial startup: 2.75s`, both map changes, **0 criticals**
  (`build/agentCB_newgame2_out.txt`). Both the regression and this were re-run on the *final*
  relinked `DishonoredGame.exe`, not on an earlier one.
* **Agent BC's harness still agrees with itself** on the fuller HEAD: `GFx3Run --run MainMenu
  --frames 5 --imports build/agentBB/gfx` reports 292 dictionary entries with **0 placeholders**,
  **12 text fields all with a font**, 376 glyphs, and 3,914 opcodes with none unimplemented — CD's
  loader and this package's text stack in one run.
* New harness commands: `--defs` (the adapter, section A.1) and `--xcheck` (the two walks, A.3), both
  wired into `GFx3Text` and both used by `build/agentCB/defs_all.py`.

## A.5 What this changes in the tables above

Section 7's row for `GFxShapeBase` / `GFxConstShapeNoStyles` is unchanged in count but better in kind:
the two deleted helpers were never retail functions, so they were never in the 966, and the
`StateNewStyles` arm now cites `GFx_ReadFillStyles` (0xa429c0) and `GFx_ReadLineStyles` (0xa41270) for
their no-style-owner behaviour. `GFxEditTextCharacterDef`'s row gains the adapter: retail's 0xa32df0
is now ported rather than named as remaining, so `GFxEditTextCharacter` goes from 14 to 15 of 125.
`agentCB_status.csv` goes from 242 rows to **248** (188 ported, 60 cited, and all 248 still ratio
1.000). Diffed against the merged version, exactly six rows are new — `GFx_ReadLineStyles` (0xa41270)
and `GFx_ReadFillStyles` (0xa429c0) as **ported**, and `GFxLoadProcess::ReadRgbaTag` (0xa22460),
`GFxConstShapeWithStyles::Read` (0xa43610), `GFxFillStyle::Read` (0xa90290) and `GFxLineStyle::Read`
(0xa907d0) as **cited**, the last two because they are CD's ports and this package only cites them as
evidence — and exactly one row changes state: 0xa32df0, cited → **ported**, which is the adapter.

**One correction to section 8's deviation list.** Deviation 1 said retail keeps the raw SWF bytes and
decodes lazily while this decodes once at load. That is still true of *this* class, but it is now only
half the picture: package CD's `GFxShapeRecord` also decodes eagerly, from the same retail function, so
the tree has one convention rather than a deviation in one corner of it.
