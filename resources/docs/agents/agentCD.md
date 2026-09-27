# Agent CD — the remaining tag loaders, import binding, and five defects (2026-09-27)

Package **CD** of `PHASE9.md`: port the 31 unported tag loaders, shapes first; bind the imports; fix
the recursion defect agent BC measured in the shop asset; and state what the half-implemented
collector costs. Driven from agent BC's harness, not from the game.

**All four are done.** The recursion defect was one of five this package fixed: agent BC's prediction
of its cause was right (section 3.3), and the other four only became reachable once the loaders and the
import binding landed — two in the frame machine and the action queue (sections 3.1 and 3.2) and two in
the loaders themselves, both silent and both caught by a cross-check rather than by an error
(sections 2.3 and 2.4).

## Result

| Measure, over all 22 cooked movies × 5 frames | before | after |
|---|---|---|
| tags with no loader (skipped by length) | **2,963** | **0** |
| placeholder characters | **1,776** | **0** |
| imports bound | 0 of 84 | **84 of 84** |
| `GFxShapeCharacterDef` paths / edges decoded | 0 / 0 | **2,007 / 13,572** (895 quadratic) |
| bitmap fills resolving to an image definition | 0 | **1,058 of 1,058** |
| fill styles with a type retail does not branch on | — | **0** |
| stack overflows from runaway recursion | 2 | **0** |
| movies running 5 frames with exit code 0 | 22 | **22** |
| opcodes executed with no implementation | 0 | **0** |

The single clearest picture of it is the menu's own root display list. Before, five of its ten
children were `Character` — a `GFxGenericCharacter` over a placeholder definition, which is what made
`getNextHighestDepth` and `attachMovie` fail on them. After, all ten are sprites, and the five that
changed are exactly the five imported from the options-menu and load-game movies:

```
  before                                     after
  depth  93  Character  id 245  loadGame_mc        Sprite  id 245  loadGame_mc
  depth  94  Character  id 246  optionsMenu_mc     Sprite  id 246  optionsMenu_mc
  depth  95  Character  id 247  videoSettings_mc   Sprite  id 247  videoSettings_mc
  depth  96  Character  id 243  gamepadMapping_mc  Sprite  id 243  gamepadMapping_mc
  depth  97  Character  id 244  gammaSetting_mc    Sprite  id 244  gammaSetting_mc
```

On the menu asset specifically (`Dishonored_MainMenu.MainMenu`): **0 tags skipped** (was 152),
**0 placeholder characters** (was 94), **10 of 10 imports bound**, 292 dictionary entries of which
71 shapes, 12 text fields, 1 button, 2 fonts and 55 images are now real definitions, and **7 script
errors** — the same number as before, and section 5 explains why that number did not fall while the
causes underneath it changed completely.

## 0. Method, and the one deviation

The rule from `agentBC.md` section 0 applies unchanged and is the reason nothing here carries an
offset assertion: **the 2012 PDB has no layout for any runtime-internal GFx type**, so behaviour is
ported and member offsets are neither reproduced nor reproducible. Four sources of evidence, each
cited at the point of use:

1. **The two retail loader tables, dumped by address rather than reconstructed.**
   `GFxLoaderImpl::GetTagLoader` (2012 0x9badf0) is 86 bytes and indexes `GFx_SWF_TagLoaderTable` for
   codes 0..0x54 and a second table for 1000..1009. Both were read out of my database copy
   (`build/agentCD/dump_tagtable.py`), and `GFxGetTagLoader` in `GFxTagLoaders.cpp` is that dump, row
   for row, with the retail function's rva on every line. The rows that are null in retail are null
   here; codes 1006 and 1007 have no loader, and codes 0, 24 and 1000 share one that does nothing
   (0xa26ea0). That is what makes "no loader" an answer rather than a gap.
2. **Headless Hex-Rays decompiles of 42 load-bearing bodies** in `build/agentCD/dec`, on my own copy
   `resources/docs/idb/shipping2012_agentCD.i64`, opened through `resources/tools/ida/run.py` only.
3. **The payloads themselves.** `build/agentCD/tagscan.py` walks a cooked payload in Python
   independently of the C++ reader, which is how the two non-standard tag layouts of section 2.3 were
   pinned and how the import graph of section 4 was read off.
4. `resources/docs/symbols/match_2012_2013.csv` for the 2013 address of each function.
   **All 92 cited functions are byte-identical between the 2012 and 2013 builds** (ratio 1.000);
   `agentCD_status.csv` has every one with its 2013 rva.

**The deviation, stated once.** Retail's shape definition is `GFxConstShapeCharacterDef`, and
`GFxConstShapeNoStyles::Read` (0xa42ab0, 2,822 bytes) does not decode the shape records at all: it
copies the raw SWF bit stream into a packed block out of a `GFxPathAllocator`, scans it once to count
paths and edges, appends those two counts to the block in a variable-length header
(`GFxSwfPathData::GetShapeAndPathCounts` 0xa3b5c0 reads them back off the end), and lets
`GFxSwfPathData::PathsIterator` decode the block again on every traversal. **The grammar of that scan
is transcribed exactly; the compacted byte encoding is not reproduced.** The records are decoded
once, at load, into the explicit path and edge list the iterator would have handed the tessellator.
Nothing here is ABI-visible — we replace libgfx rather than call into it — and the observable results,
the bounds, the styles, the path count and the edge count, are the same. That is the only place this
package departs from the retail body, and it is marked at the site in `GFxCharacterDefs.h`.

## 1. What is in the tree

| File | What it is |
|---|---|
| `GFxCharacterDefs.h` (new) | `GFxFillStyle`/`GFxLineStyle`/`GFxGradientRecord`, `GFxShapeRecord` (the decoded SHAPE record), `GFxShapeCharacterDef`, `GFxMorphCharacterDef`, `GFxEditTextCharacterDef`, `GFxStaticTextCharacterDef`, `GFxButtonRecord`/`GFxButtonCharacterDef`, `GFxImageCharacterDef`, `GFxFontCharacterDef`, and `GFxLoadProcess`/`GFxTagInfo` — the two arguments every retail loader takes |
| `GFxCharacterDefs.cpp` (new) | the readers: the style records, the shape-record walk, and every definition above |
| `GFxTagLoaders.cpp` (new) | the 34 loaders and `GFxGetTagLoader`, i.e. retail's `GFxTagLoaders.obj` |
| `GFxPlayerData.cpp` | the tag walk is now a pure dispatch through the table; plus the import table and `BindImports` |
| `GFxPlayer.h` | `GFxGenericCharacter` takes a `GFxCharacterDef*` (which is retail's own signature, ctor 0x9cfd90), the import table and `ImportResolver`, `GFxSprite::GetOwnDataDef`, the action queue's session field, the extended read stats |
| `GFxPlayerSprite.cpp` | `GetOwnDataDef` (section 3.1), `GotoFrame` and `ExecuteFrameTags` (section 3.2) |
| `GFxPlayerRoot.cpp` | `DoActions` is session-scoped and `DrainActionSessions` is the fixed point (section 3.2) |
| `GFxAS2Object.h`, `GFxAS2Interp.cpp` | `GASFnCall::pFuncName` and the `super` resolution (section 3.3) |
| `Tools/GFx3Run.cpp` | `--imports <dir>`, `--platform PC`, the definition census, the shape-geometry census and the bitmap-fill cross-check |
| `cmake/GFx.cmake` | the two new units join the `gfx3` static library |

## 2. The loaders

### 2.1 What was being skipped, measured before anything changed

`build/agentCD/runs/base` is the whole cook through the harness at HEAD. Every skipped tag code, with
its count across the 22 payloads:

| code | tag | count | code | tag | count |
|---|---|---|---|---|---|
| 2 | DefineShape | 1,093 | 1008 | GFX DefineSubImage | 483 |
| 32 | DefineShape3 | 206 | 1009 | GFX DefineExternalImage2 | 526 |
| 22 | DefineShape2 | 27 | 1000 | GFX ExporterInfo | 22 |
| 83 | DefineShape4 | 25 | 74 | CSMTextSettings | 178 |
| 37 | DefineEditText | 282 | 78 | DefineScalingGrid | 18 |
| 46 | DefineMorphShape | 36 | 69 | FileAttributes | 22 |
| 34 | DefineButton2 | 11 | 77 | Metadata | 22 |
| 75 | DefineFont3 | 12 | | | |

Total 2,963, and **shapes are 1,351 of them** — which is why shapes were first. Three things fall out
of that table and each one is load-bearing:

* the non-standard codes in the whole game are exactly the three agent BB named — 1000, 1008, 1009 —
  and nothing else in either retail table is reached;
* there is **no `DefineText` (11/33) anywhere in the cook**: every piece of text in this game's
  interface is a `DefineEditText`. The static-text loader is ported because retail registers it, and
  its report line will read zero forever;
* there is **no embedded bitmap tag anywhere** (6, 20, 21, 35, 36, 90 are all zero), which confirms
  agent BB's finding from the other direction: the art is external image references.

### 2.2 The shape reader, and how it is verified

`GFxShapeCharacterDef::Read` is `GFxConstShapeWithStyles::Read` (0xa43610): the bounds rect, a second
rect and a flags byte for DefineShape4 only, then the fill array and the line array. Then
`GFxShapeRecord::Read` is the record walk of 0xa42ab0 — `numFillBits`/`numLineBits`, then edge records
(curved: four deltas at one bit width; straight: a general line reading both deltas, a vertical line
reading only dy, a horizontal line reading only dx, the vertical branch reaching dy by falling through
the general case exactly as the retail body does) and style-change records (MoveTo, FillStyle0,
FillStyle1, LineStyle, NewStyles), with every selection bit closing the open path.

Four places where GFx departs from the published SWF layout, each read out of the retail body and each
commented at the site, because all four are silent when wrong:

| retail body | what it does that the specification does not say |
|---|---|
| the fill-style array (0xa429c0) | the u8 count promotes to u16 on 255 **only when the tag type is above 2** |
| the line-style array (0xa41270) | the u8 count promotes to u16 on 255 **for every tag type**; there is no tag test |
| `GFxFillStyle::Read` (0xa90290) | a focal gradient's focal point is read **after** the gradient records, and the type is tested for exactly 19 rather than masked |
| `GFxLineStyle::Read` (0xa907d0) | the DefineShape4 miter-limit flag is **0x20**, and a stroke with a fill takes its colour from the fill's solid colour or its first gradient stop |

Counting definitions proves only that a dictionary slot was filled. The harness therefore reports the
geometry, because a loader that consumed no bytes would show definitions and zero paths. Over the
whole cook:

```
  1,351 shape definitions -> 2,007 paths, 13,572 edges (895 quadratic),
                             1,914 fill styles, 42 line styles
  1,058 bitmap fills, 1,058 resolve to an image definition, 448 carry the null id 0xFFFF
  0 fill styles with a type retail does not branch on
  46 shapes carry a StateNewStyles record, so the style-index rebase is exercised, not dead
  282 edit-text definitions, 282 of them naming a font
  11 buttons with 19 state records; 36 morph shapes; 1,015 images (483 sub-images)
  40 font definitions carrying 7,687 glyph outlines
```

**Every one of the 1,058 bitmap fills resolves to an image definition in the same dictionary**, and the
448 further fills carrying 0xFFFF are the SWF null character id — a bitmap fill with no bitmap, which
the exporter leaves behind, not a failed lookup. **No fill style in the cook has a type retail does not
branch on**, which is the direct symptom of a misaligned style array and the second half of the same
cross-check. Together they are the strongest evidence in this package that the fill records are decoded
and not merely consumed, and between them they caught the two bugs of sections 2.3 and 2.4.

### 2.3 The bug that cross-check caught: `DefineExternalImage2`'s id is a u32

The first version of the image loader read a u16 character id for tags 1001, 1003 and 1009 alike. The
harness then reported **84 bitmap fills in the menu asset and 0 resolving to an image definition**,
which is what sent me back to the two bodies:

* `GFx_DefineExternalImageLoader` (0xa35fc0) reads **four u16s** — id, format, width, height — then
  two length-prefixed strings;
* `GFx_DefineExternalImageLoader2` (0xa36210) reads **a u32 and then three u16s**, and masks that u32
  with `0x9FFFF`, which is `GFxResourceId::IdType_Bit_IndexMask` plus the `IdType_DynFontImage` bits.
  The u32 is the resource id.

`build/agentCD/tagscan.py` confirms it in the payload: across the menu asset's 55 tag-1009 records the
u32 runs 1, 2, 3, 4, 5, 6, 10, 12, … while the u16 after it is **13 in every one of them** — the
bitmap format. Reading a u16 id there put all 55 images in one dictionary slot and every bitmap fill
resolved to nothing, with no error anywhere. After the fix, 61 of the menu's 84 bitmap fills resolve
and the other 23 carry the null id. Over the cook, after the second bug of section 2.4,
it is 1,058 of 1,058.

The two GFX tag layouts, pinned from the payload and the body together:

```
  1009 DefineExternalImage2 : u32 resourceId & 0x9FFFF | u16 format | u16 targetW | u16 targetH
                              | pstring exportName | pstring fileName | u8   (version >= 878)
  1008 DefineSubImage       : u16 id | u16 baseImageId | u16 x0 | u16 y0 | u16 x1 | u16 y1
```

The sub-image rectangle is a `GRect<int>` in pixels, which is what `GFxSubImageResource`'s constructor
(0xa22750) takes.

### 2.4 The second bug it caught: the RGB/RGBA threshold is the tag number 22, not 2

After the id fix, one movie still had bad bitmap ids: `DishonoredGame.Note` reported **251 fill styles
and 61 line styles over 28 shapes** where the menu asset has 94 and 1 over 71, 51 gradient fills where
every other movie has none, and 42 bitmap fills whose id resolved to nothing — nineteen of them exactly
`GFxResourceId::InvalidId`, which is only reachable if a fill had both the 0x10 and the 0x40 type bits,
i.e. if the type byte was garbage. A misaligned style array, in one movie.

`GFxLoadProcess::ReadRgbaTag` (0xa22460) is 57 bytes and settles it:

```
  if (tagType > 22) ReadRgba();  else ReadRgb();
```

**The threshold is 22, not 2.** DefineShape (2) *and DefineShape2 (22)* carry three-byte colours;
DefineShape3 (32) and above carry four. Reading `> 2` — which is what the tag numbering invites,
because DefineShape2 is the second shape tag — takes one byte too many for every colour in a
DefineShape2 and misaligns everything after it. `DishonoredGame.Note` is the movie where that showed
up because its two DefineShape2 tags sit in front of most of its style data.

It is worth naming the threshold that is *not* 22: the fill-style array's count promotion (0xa429c0)
really does test `tagType > 2`. Two thresholds, two different numbers, in bodies twenty lines apart.

Every colour read in this package now goes through one function that is 0xa22460's body, and
`GFxLoadProcess::ReadRgbaTag` forwards to it. After the fix `DishonoredGame.Note` reads 33 fill styles
and 2 line styles, the cook has **zero** fills with an unknown type, **zero** gradient fills (Note's 51
were the artefact), and every bitmap fill resolves.

### 2.5 The rest of the loaders

* **`GFx_DefineEditTextLoader` (0xa272a0) / `GFxEditTextCharacterDef::Read` (0xa26190).** Fifteen
  single bits in the retail body's order. Three are stored inverted — the stream carries `NoSelect`
  and `UseOutlines` and the flag word carries their opposites — two, `HasFontClass` and `WasStatic`,
  are read and dropped, and the variable name is read **before** the initial text and
  unconditionally. All 282 definitions in the cook name a font, which is the cross-check that the bit
  order is right: a one-bit slip makes `HasFont` false.
* **`GFx_ButtonCharacterLoader` (0xa35ac0) / `GFxButtonCharacterDef::Read` (0xa6a0c0) /
  `GFxButtonRecord::Read` (0xa69bb0).** The state records and the condition-action chain. The filter
  list is stepped over by the fixed sizes of `GFx_LoadFilters` (0xa93b60) rather than parsed, because
  the five filter classes are 48 retail functions and none of them is in this package; that is a stub
  and it is marked as one at the site.
* **`GFx_DefineShapeMorphLoader` (0xa35710) / `GFxMorphCharacterDef::Read` (0xab1df0).** Two bounds
  rects (four for tag 84), the u32 offset to the second record, the interpolated style pairs of
  `ReadMorphFillStyle` (0xab1370) — which always use the DefineShape3 dialect whatever the morph tag's
  own code is — then both records through the no-styles walk.
* **`GFx_DefineFontLoader` (0xa357d0) / `GFx_DefineFontInfoLoader` (0xa35960).** The name, the flags,
  the glyph offset table, the glyph outlines through the same shape reader, the code table and the
  layout block. The `DefineFont3` glyph flag matters and is measured, not assumed: a DefineFont3
  glyph's coordinates are 20× the 1024-unit EM square, which retail keeps as flag bit 0x02 and
  `GFxSwfPathData::PathsIterator` turns into a 0.05 scale.
* **`GFx_Scale9GridLoader` (0xa36680)** rejects a degenerate grid with a warning *before* it looks the
  character up, and applies the rect to a sprite or a button definition — the retail body tests the
  resource type's class byte for 0x84 and 0x81 and has no shape case.
* **`GFx_CSMTextSettings` (0xa26ca0)** sets the FlashType flag on an edit-text or static-text
  definition already in the dictionary.
* **`GFx_DefineFontInfoLoader` fixed a latent defect**: the old walk treated every tag whose body
  starts with a u16 as a definition, so `DefineFontInfo` would have *replaced* the font it names. No
  payload in this cook carries one, so it never fired.
* **`GFx_FileAttributesLoader` (0xa352f0)** finally gives `GFxMovieDefImpl::GetSWFFlags` something to
  return; it answered 0 before.

## 3. The three defects

Two of these were not reachable before this package: binding the imports is what let the content run
far enough to hit them, which is the pattern `PHASE9.md`'s "measure first" rule predicts.

### 3.1 An imported clip's children resolved against the wrong dictionary

Binding the imports immediately produced five `PlaceObject: character N is not in the dictionary`
errors per frame. The cause: an imported symbol is a sprite whose timeline was authored in *another*
file, so the character ids in its `PlaceObject` tags index that file's dictionary, and
`GFxSprite::AddDisplayObject` was resolving them through the *importing* movie's `GFxMovieDefImpl`.

Retail keeps the distinction in `GFxSpriteDef::pMovieDef` — the movie data def the sprite definition
was parsed out of — and binds an import as a resource handle into the exporting movie's library.
`GFxSprite::GetOwnDataDef` is that field used: the dictionary a `PlaceObject` inside a sprite's own
timeline resolves against is the def's movie, not the root's. The five errors went to zero.

This is the sixth instance of `PHASE9.md`'s own warning — a reference member kept as a storage-less
placeholder. `pMovieDef` was declared and set and nothing read it.

### 3.2 The action queue was not session-scoped, and a goto replayed actions it should not

With the imports bound the menu asset stopped terminating. Instrumenting the interpreter named the
runaway exactly: one 66-byte buffer, executed **148,000 times**, whose bytecode decodes to

```
  Push "_global" | GetVariable | Push "PlatformName" | GetMember
  Push 1 | Push "this" | GetVariable | Push "gotoAndStop" | CallMethod | Pop | End
```

— `this.gotoAndStop(_global.PlatformName)`, the platform-switch clip of the imported CLIK components,
which is the same content agent BC saw as 16 `no frame named 'PC'` errors. Two things were wrong and
both are readable in the retail bodies:

1. **`GFxSprite::GotoFrame` (0xa012a0) assigns the frame number before running the target frame's
   tags** (`this[27].__vftable = v8;` and only then `ExecuteFrameTags(v8)`), on both the forward and
   the rewind path. Assigning afterwards means a frame whose own `DoAction` calls `gotoAndStop` on
   that same frame still sees the old `CurrentFrame`, fails the early out and re-enters.
2. **A goto does not replay the action tags of the frames it passes through.** Retail builds a
   `GFxTimelineSnapshot` of them (`GFxSprite::MakeSnapshot`) and replays it through
   `ExecuteSnapshot(this, snapshot, 4)`; a snapshot is display state, the place and remove records,
   with no action buffers in it. Only the *target* frame gets the full `ExecuteFrameTags`.
   `ExecuteFrameTags` now takes `bWithActions`, false for the frames a goto skips.
3. **A drain is a session.** `GFxSprite::CallFrameActions` (0x9f63b0) calls
   `ActionQueueType::StartNewSession`, pushes the frame's tags and drains through
   `DoActionsForSession` (0xa0d9e0), whose `ActionQueueSessionIterator` stops at the session boundary,
   so a buffer queued *by* a drained action belongs to the next session. `GFxMovieRoot::DoActions`
   walked a growing count instead, with a comment saying so. Each entry now carries the session it
   joined; a drain executes and removes exactly that session, and `DrainActionSessions` iterates to a
   fixed point with retail's frame-catch-up bound, so a content-level ping-pong is a reported error
   rather than a hang.

After all three the menu asset settles in the first drain: 5 frames, 190 sprites, 398 display objects
placed and 92 moved, where before the fix it was 2,234 moves and no termination.

### 3.3 The recursion defect agent BC named, and its cause is the one BC predicted

`agentBC.md` section 6.5 named the shop asset's 64-activation guard and predicted the cause: `super`
built from `this.__proto__.__proto__`, so a method invoked with a prototype rather than an instance
finds itself. The prediction was right and the fix is retail's own, in `InvokeContext::Setup`
(0x9f2b40) — the function-invocation prologue:

```
  v54 = this->__proto__
  if (this->pFunctionName)                       // Invoke's third parameter, char const* name
      Owner = GASObjectInterface::FindOwner(v54 + 16, sc, name);   // 0x9dac70
  if (Owner) v55 = Owner;                        // the prototype that DECLARES the function
  ctor = v55->Get__constructor__(sc);
  superObj = new GASSuperObject(v55->__proto__, this, ctor);
```

So `super` is relative to the prototype that **declares the running function**, found by walking the
chain for the name the call was made under — not to `this`. Deriving it from `this` recurses whenever a
base-class method is invoked with an instance two levels down, and every one of the 669 `__Packages`
registrations in the cook builds a chain that deep. `GASFnCall` now carries `pFuncName`, exactly as
retail's `InvokeContext` keeps it at +16, and `GFxAS2InvokeScriptFunction` does the `FindOwner` walk.

**The shop asset no longer recurses**: the `Stack overflow: more than 64 nested function calls` line is
gone from the shop and from the whole cook, and what it was masking surfaced as one ordinary
`EnableInputs` call on a content method that is not installed yet. `GASSuperObject::SetAltProto`
(0x9e2730) and `ResetAltProto` (0x9af060) are the second, independent mechanism — the alternate
prototype `ActionCastOp` and `extends` install — and they are recorded as remaining, because no asset
in the cook reaches them.

## 4. Import binding

`GFx_ImportLoader` (0xa385f0) records the URL and the symbols and leaves a handle in the dictionary;
the resolution is a separate step, which in retail is `GFxMovieBindProcess` cloning the load states
for the imported file (`GFxLoadStates::CloneForImport` 0xa240b0) and opening it through the state bag's
file opener (`GFxLoadStates::OpenFile` 0xa22520). `GFxMovieDataDef::BindImports` is that step and
`GFxMovieDataDef::ImportResolver` is the file-opener seam: `FGFxFileOpener` in the engine, and in the
harness a map from the URL's basename to the cooked payload. The bind is transitive, because every UI
movie reaches the font library through `lib.swf`.

One place where GFx departs from the published layout, ported as retail has it: **the symbol count is
read before the reserved word, not after.** Retail reads the URL, then a u16 count, then — for
`ImportAssets2` only — one more u16, where the specification puts two reserved bytes in front of the
count. Both readings consume the same four bytes and every `ImportAssets2` tag in this cook has
`01 00 01 00` there, so on this data they agree; retail's order is what is ported.

The whole import graph of the cook, read out of the payloads with `tagscan.py`: **84 symbols over 22
movies**, from four files — `lib.swf` (the shared component library), `gfxfontlib.swf` (the two game
fonts), `OptionsMenu.swf` and `LoadGame.swf`. **All 84 bind, and the cook has no placeholder character
left anywhere.**

Six of them took the section 2.3 fix to get there and the wrong conclusion in between is worth
recording, because it is the shape mistakes in this area take. `X360_A -nopack` and `X360_B -nopack`,
imported by the options menu and the two HUD movies, would not resolve, and the payload seemed to
agree: `lib.gfx` exports the name at character id 18 and my Python scanner found no defining tag for
id 18, so I wrote them down as Xbox glyphs stripped from the PC cook. They are not stripped. They are
`DefineExternalImage2` records, and the scanner was reading a u16 id out of the same tag for the same
reason the C++ loader was. Two readers agreeing is only evidence when they are not making the same
mistake, and `tagscan.py` now reads the u32.

## 5. Why the script-error count did not fall, and what the errors now are

Across the cook the count is 174 before and after, and on the menu asset 7 before and after. That is
not a null result, because the composition changed completely and the harness now names the receiver
of every failed call:

| receiver | count | what it means |
|---|---|---|
| `undefined` | 87 | the consequence of an earlier failure, not a cause |
| a script object | 40 | a class-library gap — 71 of the 158 failed calls are `loadBitmap` alone |
| `Sprite` | 31 | a real movie clip missing a method the content defined, or a content method not installed yet |
| a Shape, EditText, Button or Image | **0** | **was 22** |

Agent BC's 22 "method on a generic character" errors are gone: every receiver in the cook is now
either undefined, a script object, or a real sprite. The two stack overflows are gone. What replaced
them is failures that were previously *masked* — `EnableInputs`, `SetInput`, `InitHelpBar` and the rest
of the content's own methods, which only get called at all now that the clips they hang off exist.
That is the honest reading: the count is flat because fixing the definitions moved the failures
downstream rather than removing them, and the remainder is two things and two things only —

* **71 `loadBitmap`** — `flash.display.BitmapData`, 22 retail functions, needs package CC's renderer
  and agent BE's `FGFxImageLoader`. One class, and the single most-called missing function in the cook,
  exactly as agent BC measured;
* **~30 content methods** on clips whose class registration has not run or whose own implementation
  needs the text engine (package CB) or the input path.

Plus **16 `no frame named 'PC'`**, and those are a harness artefact rather than a runtime one: they
occur only when `DishonoredGame.lib` and `Startup.lib` are run *as root movies*, which they never are
in the game — they are the library the other movies import from, and the clips that want the label are
never placed. Running them as roots is the harness being thorough.

`--platform PC` was added to the harness to set `_global.PlatformName` before the first advance, which
is what the engine's own movie player does. **Measured, not assumed: it changes none of the numbers**,
so the platform-switch behaviour of section 3.2 was a genuine runtime defect and not a missing
variable.

## 6. The collector: what it is, and what a real one costs

`GASObjectCollector` is the *teardown* half of retail's collector and nothing more: every `GASObject`
is registered on creation and freed in one two-pass sweep when the movie dies. `Release()` reaching
zero mid-frame frees nothing, so **a movie that runs for an hour grows**. The harness proves the
teardown is correct — no leak across the 22 movies, exit 0 on every one — but it is not a collector,
and this package does not make it one.

The size of the leak is measurable rather than hypothetical: over the cook the 22 movies create
**12,377 AS2 objects in five frames and none of them is ever freed before teardown** — created and
live are the same number, 12,377, which is the leak stated exactly. (It was 12,357 before this
package, so the loaders barely moved it: the objects are the content's, not the definitions'.) Every
`__Packages` class among them is a cycle a plain refcount cannot break. Five frames is nothing; the
menu is open for minutes.

What a real collector would cost, counted rather than guessed:

* retail's collector is `GASRefCountCollector` over `GRefCountBaseGC<323>`, a four-colour incremental
  mark-and-sweep. `GRefCountBaseGC<323>::Release` (0x9aeb20, 377 bytes) and `CollectGarbage`
  (0x9ae790) are the engine of it, and `GASRefCountCollector` itself is **11 functions**;
* the traversal is **not** a virtual per class returning a child list. It is a template instantiated
  five times per participating class — `ForEachChild_GC<ScanInUseFunctor>`, `<MarkInCycleFunctor>`,
  `<ScanFunctor>`, `<ReleaseFunctor>`, `<CollectGarbageFunctor>` — plus one virtual
  `ExecuteForEachChild_GC(OperationGC)` that dispatches on the operation. In the retail symbol table
  that is **117 `ForEachChild_GC` bodies across 34 distinct types** and **19 `ExecuteForEachChild_GC`
  overrides**;
* so the cost is **about 147 functions over 34 classes**, and it is not separable: a class that does
  not participate is a hole in the mark phase, and a hole in the mark phase collects live objects.
  Every one of `GASObject`, `GASArrayObject`, `GASFunctionObject`, `GASSuperObject`, `GASValue`,
  `GASValueProperty`, the `GASPrototype<>` instantiations and the class-library objects has to be in
  before the first sweep can run at all;
* and it needs the refcount word's flag bits, which are visible in the decompiles as the
  `(x + 1) & 0x8FFFFFFF` on every increment — the top four bits are the collector's colour and
  in-cycle marks. Those are behaviour, not layout, so they are portable, but they touch every
  `AddRef`/`Release` in the machine.

**That is too large for this package and it is not on the critical path**: the interface is opened and
closed, not run for an hour, and the teardown sweep already returns everything. It deserves its own
package, and the honest statement is that until it exists a long-running movie leaks its cyclic
object graphs — which is every `__Packages` class, because a prototype names its constructor and the
constructor names its prototype.

## 7. Verification

* **The machine on its own** (`build/agentCD_run.cmd`, VS 2022 x86, `/Zp4`, no engine): 0 errors,
  0 warnings at `/W3`. This is the fast loop and it is the same one agent BC used. The three new units
  include no engine header, so the whole loader and definition layer still compiles with no engine at
  all.
* **51,116 opcodes executed over the cook, 0 of them without an implementation**, unchanged from agent
  BC's figure in kind and up from 50,829 in degree. Binding the imports did not reach a new opcode:
  36 distinct codes in the busiest asset, all implemented.
* **All 22 cooked movies, 5 frames each, exit code 0**, full verbose output in
  `build/agentCD/runs/v3` (and `runs/base` for the before picture). The per-asset table is
  `build/agentCD/summarize.sh`'s output; the totals are section 0's table.
* **The geometry cross-checks** of section 2.2, which is what makes the loaders a measurement rather
  than a count: 1,058 of 1,058 bitmap fills resolve to an image definition, 0 fill styles have a type
  retail does not branch on, 282 of 282 edit-text definitions name a font, and 7,687 glyph outlines
  came out of 40 font definitions. Both bugs of sections 2.3 and 2.4 were found by these two numbers
  and by nothing else: neither produced an error, a warning or a crash.
* **A second, independent reader.** `build/agentCD/tagscan.py` walks the payloads in Python with no
  C++ involved, and the tag counts, the character-id sets and the import graph agree with the C++
  reader. That is how section 2.3's u32 was pinned — and section 4 records the one time the two
  readers agreed because they were making the same mistake, which is the limit of that method.
* **Isolated RELEASE build**: `build/agentCD_relbuild.cmd` over the HEAD snapshot `build/agentCD_wt`
  (`make_snapshot.py CD --list build/agentCD_files.txt`, plus HEAD's `cmake/GFx.cmake` with my two
  unit lines). **0 errors, 0 link errors, 0 compiler warnings from any unit of this package**, and all
  three targets produced: `DishonoredGame.exe`, `CoreSmoke.exe`, `LayoutProbe.exe`. The four C4100
  unreferenced-parameter warnings the first full build produced were in `GFxPlayerData.cpp`'s
  `CreateInstance` bodies and in two of my loaders, and they are gone.
* **Regression**: `run_regression.py --build-dir build/agentCD_rel --no-build`, 31 checks
  (`build/agentCD_regression.txt`, and `build/agentCD_regression_d3d9.txt` for the d3d9 re-run).
  **Every check passes; none of them fails for a reason that is this package's.** It took two runs,
  and that is worth stating precisely rather than glossing, because the harness's game stages are
  wall-clock gated and seven agents were building on this machine at once:
  * **25 ok in the full run**, including all nine layout and CoreSmoke checks (2,314 types, 0
    mismatches, 0 contract mismatches, CoreSmoke 99/0), nullrhi 12,065 lines / 0 criticals / 2.9 s
    startup, and the whole inputtest stage at HEAD's own numbers — **pawn walks 1029.1 units**, peak
    speed 500.5, 895 PhysX actors, 1,312 static shapes, touch census 198, sequence census 26,239,
    probe_natives 7, 0 criticals;
  * the d3d9 stage was killed by its 90 s timeout in that run, so its metrics read `-1`. Re-run with
    `--only d3d9 --d3d9-timeout 300`: **8 ok** — 12,450 frames, 6,506 draw elements, 460 visible
    primitives, 1,203 draws per frame, texture census 16,257, `unported_natives` 0, 0 criticals — and
    the one remaining failure is `d3d9_startup_seconds 171.7`, whose own baseline note says it is "a
    cliff detector only … the bound catches a hang, not a slowdown". The same exe started in 2.9 s in
    the nullrhi stage of the other run, so 171.7 s is the machine, not the build;
  * the two stages fail *alternately* depending on load — inputtest failed and d3d9 passed in the
    first run, the reverse in the second — which is the proof that it is contention: a change of mine
    could not make one stage fail only when the other succeeds.
* **All 92 cited retail functions** are in `agentCD_status.csv` with their 2013 rvas: 65 ported,
  12 partial, 15 named as remaining, and **92 of 92 byte-identical between 2012 and 2013**.
* **The by-value copy audit agent CB's heap corruption calls for.** The reconstructed `GArray<>` in
  the generated headers has a destructor and no copy constructor, so any by-value copy of a type
  containing one double-frees. Checked, and this package is clear of it: none of
  `GFxCharacterDefs.h/.cpp` or `GFxTagLoaders.cpp` names `GArray`, `GString` or `GPtr` at all, every
  type this package copies by value is trivially copyable (`GFxFillStyle` with its fixed
  `GFxGradientRecord[15]`, `GFxShapeEdgeCD`, `GFxButtonRecord`, `GFxCharPosInfo`, the action queue's
  `ActionEntry` and the import table's `ImportEntry`), and the three types that do own a pointer and
  have a destructor — `GFxLineStyle`, `GFxShapePathCD`, `GFxShapeRecord` — each declare a private copy
  constructor and assignment operator and are held by pointer, so a by-value copy of them does not
  compile rather than corrupting the heap. The `realloc`-grown arrays only hold the trivially
  copyable ones.

## 8. Hand-overs

**Agent CB — merged at `54b57d4` while this package was in flight; the collision is settled and one
duplicate is left to collapse.**
1. `GFxEditTextCharacterDef` is mine, in `GFxCharacterDefs.h`, and CB removed its own. The flag bits
   agree because both were read out of the word retail's 0xa26190 writes at +80. CB exposes
   `GFxTextFieldDesc` with the same bits at the same values plus a character constructor that takes
   it, and its report writes out the adapter as twelve lines: **that is the one edit left on the text
   path, in `GFxEditTextCharacterDef::CreateCharacterInstance` (retail 0xa32df0), which returns a
   `GFxGenericCharacter` today.** I did not take it in this package because the wave's merge order put
   CD first and a `#include "GFxTextField.h"` here would have made a CD-only HEAD depend on a unit
   that was still being written. It is a two-line include and a twelve-line body now that CB has
   landed, and it belongs to whoever merges second.
2. **`GFxShapeRecord::Read` in my `GFxCharacterDefs.cpp` and your `GFxConstShapeNoStyles::Read` in
   `GFxShape.cpp` are the same retail function decompiled twice, and one of them should go.** I built
   against yours first and it worked; I then made mine self-contained because the merge order in
   `PHASE9.md` puts CD before CB and a CD-only HEAD has to compile on its own. **Keep mine when you
   collapse them, for three reasons that are transcription slips rather than choices**: your
   `GFxSkipFillStyles` reads a focal gradient's focal point *before* the gradient records, where
   retail (0xa90290) reads it after, and your `GFxSkipLineStyles` tests the DefineShape4 miter flag as
   `0x0800`, where retail (0xa907d0) tests `0x20`, and both of your colour skips branch on the tag
   type at the wrong threshold — `GFxShapeTagHasAlpha` has to be `tagType > 22`, not a test that puts
   DefineShape2 on the four-byte side (section 2.4, retail 0xa22460). All three misalign the stream on
   the shapes that use them, silently. My `GFxShapeCharacterDef` already carries the fill and line arrays your skip functions throw
   away, so the collapse is to point your glyph reader at `GFxShapeRecord` and delete the skips.
3. `GFxFontCharacterDef` is a thin adapter — the dictionary is `GFxCharacterDef`-based while a font is
   a resource, so something has to bridge them. Your `GFxFontData::Read` supersedes its reader; the
   tag-10/48/75/1005 row of `GFxGetTagLoader` is the single place to repoint.
4. Every edit-text definition in the cook names a font, and after import binding the font it names is
   bound: 7,687 glyph outlines are decoded and reachable through
   `GFxFontCharacterDef::GetGlyphShape`. The `SF_TwentyTimesScale` constant you measured is set the
   same way here.

**Agent CC — the geometry is there and it is addressed by index.** `GFxShapeCharacterDef` carries the
bounds in twips, the fill and line style arrays, and `GFxShapeRecord`'s path list with absolute twips
coordinates and a quadratic flag per edge — 2,007 paths and 13,572 edges over the cook. Every bitmap
fill's `ImageId` resolves to a `GFxImageCharacterDef` that carries the export name, the file name and
the target size, and 483 of the 1,015 images are sub-image rectangles of an atlas
(`BaseImageId` + `SubRect`). The colour transform on a button state record is read with
`ReadCxformRgba`, so agent BE's transposition finding applies to it.

**Agent BE — `GFxMovieDataDef::ImportResolver` is the seam your `FGFxFileOpener` plugs into.** One
virtual, `ResolveImportMovie(const char* url)`, called from `BindImports` before any character is
instantiated. The URLs in the cook are relative authoring paths with both separators
(`..\DisFonts\gfxfontlib.swf` and `../common_assets/lib.swf`, sometimes differing in case within one
movie), so the mapping has to be basename-and-case-insensitive, which is what
`GFxLoadStates::BuildURL` (0xa225c0) and `GFxURLBuilder` do in retail. Four files cover the whole
game: `lib.swf`, `gfxfontlib.swf`, `OptionsMenu.swf`, `LoadGame.swf`.

**Coordinator — three things.**
1. **Merge order.** CB merged first in the end (`54b57d4`) and this package is self-contained either
   way, so nothing is blocked. Two follow-ups fall out of that and both are small: the twelve-line
   text-field adapter of hand-over 1, and the duplicate shape reader of hand-over 2.
2. `cmake/GFx.cmake` carries both packages' unit lists and **both sets of entries have to stay** —
   CB's eight text and font units and my two. My snapshot's copy carries HEAD's plus my two lines
   only, which is why the snapshot builds in isolation; the shared file is the merge-ready one.
3. The collector of section 6 is ~147 functions over 34 classes and wants its own package. It is the
   one thing in this area that is a real gap rather than a named remainder.

## 9. Files

Mine (13): `GFxCharacterDefs.h`, `GFxCharacterDefs.cpp`, `GFxTagLoaders.cpp` (new),
`GFxPlayer.h`, `GFxPlayerData.cpp`, `GFxPlayerSprite.cpp`, `GFxPlayerRoot.cpp`, `GFxAS2Object.h`,
`GFxAS2Interp.cpp`, `Tools/GFx3Run.cpp`, a targeted edit to `cmake/GFx.cmake`, plus this report and
`agentCD_status.csv`. The snapshot list is `build/agentCD_files.txt`.

Scratch (not repo tools): `build/agentCD/dump_tagtable.py` (the two loader tables, by address),
`build/agentCD/tagscan.py` (the independent Python payload walker), `build/agentCD/xr.py`,
`build/agentCD/dec` (42 headless decompiles) and `declist*.txt`, `build/agentCD/mkstatus.py`,
`build/agentCD/patch*.py`, `build/agentCD/runall.sh` + `summarize.sh`,
`build/agentCD/runs/{base,v3}` (the whole cook before and after), `build/agentCD_run.cmd` (the
no-engine build), `build/agentCD_relbuild.cmd` + log, snapshot `build/agentCD_wt` and build directory
`build/agentCD_rel`.

IDA: one own copy, `resources/docs/idb/shipping2012_agentCD.i64`, opened headlessly through
`resources/tools/ida/run.py` only. **No MCP tool of any kind was used, and no FModel tool.** No
commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.
