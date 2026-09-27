# Agent BB — the GFx 3.3 foundation: the API from the PDB, the renderer seam, and the cooked container format (2026-09-27)

Package **BB** of `PHASE8.md`: reconstruct the Scaleform GFx 3.3 API from the 2012 symbols the way
agent AL reconstructed PhysX 2.8.4 from `PhysXCore.pdb`, implement the seam our engine must supply
(the renderer, the texture and render-target interfaces, the file opener, the image loader), and
prove the seam by loading one cooked menu asset and reporting its header, exported symbols and frame
count — parsing the container format for real, because agent AW's whole decision rested on an
assumption about it that nobody had checked.

**All three are done, and the assumption was half wrong.** The assets *are* `gfxexport` GFX with the
bitmaps stripped into engine textures; the fonts are **not** pre-packed into textures, they are real
`DefineFont3` glyph outlines. Details and consequences in section 3.

## Result

| Step | State |
|---|---|
| 1. The GFx 3.3 API from the 2012 PDB | **done**: 166 types (21 forward-declared only) + 69 enums generated with every member at its PDB offset and every virtual at its PDB vtable slot, plus 5 hand-written headers. `GFx3Layout.cpp` carries **171 `sizeof`, 380 `offsetof` and 696 enumerator assertions against the PDB and they all pass** |
| 2. The seam (`GRenderer` 54, `GTexture` 12 + 5, `GRenderTarget` 7 + 3, `GFxFileOpener` 4, `GFile` 19, `GFxImageLoader`/`GFxImageCreator` 1 each, `GSysAllocPaged` 5) | **done**: every slot declared *and* defined, in the GFxUI units retail attributes them to. `GFx3Dump --slots` instantiates the renderer and calls every slot through a base-class pointer: **73 distinct slots recorded, 74 calls, every one landing in the right override** |
| 3. A cooked asset through our loader | **done**: **all 22 `SwfMovie` payloads in the retail cook** parse, every one exactly (`End` tag lands on the last byte), with the right header, frame count, symbol table and tag histogram. Numbers cross-checked against an independent Python extraction |
| The asset-format question | **answered definitively**: section 3 |
| MSVC's reverse-overload trap | **handled and re-proved for this toolchain**: section 2.3 |
| The one thing that was hand-guessed | **found and fixed**, with 696 enumerator assertions so it cannot recur: section 2.4a |
| Regression / playability | **green**: `run_regression.py --build-dir build/agentBB_rel --no-build` = **28 ok, 0 failed, 2 skipped** (the 2 are the optional counters the baseline also skips), and `-newgame` still runs the retail two-map-change route with 0 criticals |

## 0. Why there is a `source/Development/Src/External/GFx3/` at all

`gfx_decision.md` established the negative: GFx cannot be imported. Not one of the 33 DLLs in
`Binaries\Win32` holds a single GFx symbol; `libgfx` + `libgfx_ime` are **5,635 functions and
1.13 MiB inside `Dishonored.exe`'s own `.text`**, 9.7 % of it. What makes the reconstruction possible
is the other measurement in the same document: **5,235 of those 5,635 functions (92.9 %) are
byte-identical between the 2012 QA build, which ships a PDB, and the retail 2013 build.** So the 2012
PDB's types *are* the retail types, and `resources/tools/pdb/dia_types.py` — agent AL's tool — reads
them out directly.

Two independent checks that this is not wishful thinking, both in section 2.

## 1. What is in the tree

`source/Development/Src/External/GFx3/` - 6,058 lines, 11 files. Four are generated.

| File | Lines | What it is |
|---|---|---|
| `GTypes.h` | 645 | **hand-written** kernel: the scalar typedefs, `GColor`/`GPoint`/`GPoint3`/`GSize`/`GRect`/`GMatrix2D`/`GMatrix3D`/`GViewport`, the whole `GRefCountImplCore → GRefCountImpl → GRefCountBaseStatImpl → GRefCountBase` chain, `GPtr`, `GList`/`GListNode`, `GAtomicInt`, `GArray`, `GString` (+`DataDesc`), `GStringBuffer`, `GLock`, the stat tag classes, `GFileConstants`, `GFxFileConstants`. Every layout is the PDB's and every enum value is the PDB's |
| `GFx3Enums.h` | 140 | **generated**: the 10 file-scope `G*` enums (`GHeapId`, `GStatGroup`, `GStatRenderer`, `GFxStatMovieView`, `GFxTimingMode`, …). The 59 nested enums are emitted inside their class in `GFx3Gen.h` |
| `GFx3Gen.h` | 2,518 | **generated**: 166 classes, 416 virtuals each at its PDB slot with the slot in a trailing comment. The loader, movie definition and view, the renderer and its nested parameter structs, textures and render targets, the file opener, image loader and creator, the translator, the external interface, the FS-command handler, the font provider, the allocator hierarchy and `GMemoryHeap` |
| `GFxValue.h` | 417 | **hand-written**: `GFxValue` (16 bytes), `GFxValue::ObjectInterface` with all 25 methods at their 2012 rvas, `GFxValue::DisplayInfo` (232 bytes). Hand-written because the accessors are header-inline in the SDK and because **81 % of the engine's 1,350 calls into libgfx go through these two types** (`gfx_decision.md` 2.2) |
| `GFxGfxFile.h` / `.cpp` | 213 / 343 | the cooked container parser — section 3 |
| `GFx3Layout.cpp` | 1,264 | **generated**: 171 `sizeof` + 380 `offsetof` + 696 enumerator assertions. This is the library's assertion translation unit: if it compiles, the reconstruction agrees with the PDB byte for byte |
| `GFx3RuntimeStubs.cpp` | 72 | **generated**: a bringup body for each of the **57 non-pure virtuals** libgfx implements and we do not. The inventory of what package BC still has to write at the interface level; deleting an entry as the real body lands is the intended workflow |
| `GFx3Support.cpp` | 63 | `GArray`'s growth allocator, `GLock` over a Win32 critical section, the one non-pure virtual of `GFxLogBase` |
| `GFx3.h` | 27 | the umbrella |
| `Tools/GFx3Dump.cpp` | 347 | the acceptance harness: `--parse` a cooked payload, `--slots` call every seam slot |

The seam, in the GFxUI units the PDB attributes it to (1,632 lines added, none of them removed —
every file's original `// PDB functions attributed to this file (N):` evidence block is intact):

| File | Lines | What it is |
|---|---|---|
| `GFxUI/Inc/gfxuirenderer.h` + `Src/gfxuirenderer.cpp` | 269 + 821 | `FGFxRenderer` (all 54 `GRenderer` slots), `FGFxTexture` (12 + 5), `FGFxRenderTarget` (7 + 3), and the seam census |
| `GFxUI/Inc/gfxuirendererimpl.h` | 123 | `FGFxRendererImpl`'s element stores at their PDB layouts + `ConvertFromUI` |
| `GFxUI/Inc/gfxuifile.h` + `Src/gfxuifile.cpp` | 67 + 146 | `FGFxFileOpener` (4) and `FGFxFile` (19) |
| `GFxUI/Inc/gfxuiimageinfo.h` + `Src/gfxuiimageinfo.cpp` | 50 + 77 | `FGFxImageInfo`, `FGFxImageLoader`, `FGFxImageCreator` |
| `GFxUI/Inc/gfxuiallocator.h` + `Src/gfxuiallocator.cpp` | 34 + 45 | `FGFxAllocator` (5 of `GSysAllocPaged`'s 15) |

Plus `cmake/GFx.cmake` (new), one `include()` line in `cmake/Dependencies.cmake`, one block in
`dishonored_apply_defines()` (`cmake/DishonoredDefines.cmake`), and a 12-line `if()` at the end of
`GFxUI/Sources.cmake`. All four shared-file edits are targeted; nothing was rewritten.

## 2. Method, and the three traps

### 2.1 The dump

```
python resources/tools/pdb/dia_types.py \
    ../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.pdb \
    --udt-re "^G" --enum-re "^G" --json build/agentBB/g_types.json      # 694 UDTs, 158 enums
python resources/tools/pdb/dia_types.py <same pdb> --udt-re "^FGFx" --json build/agentBB/fgfx_types.json
python build/agentBB_gen_gfx3.py
```

The second dump matters as much as the first: **our own glue classes are in the PDB too.**
`FGFxRenderer` is 892 bytes with 35 members and 43 introduced virtuals, `FGFxTexture` 24,
`FGFxRenderTarget` 36, `FGFxFile` 88, `FGFxImageInfo` 40, `FGFxAllocator` 16 — so the seam is not
guesswork either. The ones the seam reproduces come out at exactly those sizes (section 5).

### 2.2 DIA's vtable slots are only trustworthy for *introducing* virtuals

`IDiaSymbol::virtualBaseOffset` is 0 for an override, so every class reports its inherited virtuals
at "slot 0". The generator therefore uses only `intro` virtuals and composes the vtable along the
base chain, and it must use the **primary** base (the one at offset 0): under multiple inheritance a
second base at a non-zero offset carries a vtable of its own. That is not academic —
`GImageInfoBase` derives from `GRefCountBaseNTS` at @0 (1 slot) and `GTexture::ChangeHandler` at @8
(3 slots), and its own virtuals start at **1**, not 3. Getting that wrong was the generator's first
crash and it is the reason `base_vtable_size()` looks at `offset == 0` only.

### 2.3 MSVC reverses consecutive virtual overloads — the trap agent AL flagged

`agentAL.md` 4: a run of consecutive virtual functions with the same name is laid out in the vtable
in **reverse declaration order**. A UI runtime is full of overloads and this hits repeatedly:

| class | the run | PDB slots |
|---|---|---|
| `GFxMovie::Invoke` | `(…, const char*, ...)` / `(…, const GFxValue*, unsigned)` | 22 / 23 |
| `GFxMovieView::TranslateToScreen` | `GRect` / `GPoint` | 63 / 64 |
| `GFxMovieDef::CreateInstance` | `MemoryContext*` / `MemoryParams&` | 23 / 24 |
| `GMemoryHeap::Alloc` | `(size, align, dbg)` / `(size, dbg)` | 9 / 10 |
| `GMemoryHeap::AllocAutoHeap` | 4-arg / 3-arg | 13 / 14 |
| `FGFxTexture::InitTexture` | `GImageBase*` / `UTexture*` | 1 / 12 (not a run) |
| `FGFxRenderTarget::InitRenderTarget_RenderThread` | `(GTexture*, …)` / `(NativeRenderTarget&)` | 9 / 8 |

The generator emits each run **highest-slot-first**, then re-applies MSVC's reversal to its own
emitted order and asserts the result is `0..n-1` for every class. Two independent confirmations:

1. **A probe in this toolchain.** `build/agentBB/vt/vtprobe.cpp` declares a class with the same
   shape and `cl /FAsc` prints the vtable (`build/agentBB/vt/vtprobe.cod`): the `CreateInstance`
   declared *second* lands in the *lower* slot, and so does the `Alloc` declared second. Exactly
   AL's finding, re-measured rather than assumed.
2. **The retail vtables themselves.** `resources/docs/symbols/vtables.csv` (IDA's own export, no DIA
   involved) gives `GFxMovieRoot{for GFxMovie}` slot 22 = `Invoke(char const*, GFxValue*, char
   const*, ...)` and slot 23 = `Invoke(char const*, GFxValue*, GFxValue const*, unsigned int)`;
   `GFxMovieDefImpl{for GFxResource}` 23/24 = `CreateInstance(MemoryContext*)`/`(MemoryParams&)`;
   `GMemoryHeapPT` 9/10 and 13/14 as above. Every one agrees with the PDB, and with what the
   generator emitted.

### 2.4 GFx was not compiled with UE3's `/Zp4`

`GFxValue::DisplayInfo` settles it: `bool Visible` at @48 and the next `double Z` at **@56**, i.e.
doubles are 8-aligned and the struct is 232 bytes. With `/Zp4` that `double` would sit at @52. Every
header in the directory therefore pushes `#pragma pack(8)`, so the layouts are right whatever the
including translation unit's packing is — which is how the same headers assert clean while being
compiled with the engine's `/Zp4` on the command line.

### 2.4a The one thing that was hand-guessed, and how it got caught

Everything generated is the PDB's by construction. `GFxValue.h` is hand-written, and three of its
enums were hand-guessed from the SDK's usual convention rather than read out of the dump — and two of
the three were **wrong**:

| what | guessed | the PDB |
|---|---|---|
| `GFxValue::ValueTypeControl` | `VTC_ConvertBit 0x08`, `VTC_ManagedBit 0x10`, `VTC_TypeMask 0x0F` | **`0x80`, `0x40`, `0x8F`** |
| the convertible types | `VT_ConvertNumber 0x0B` | **`0x83`** (the plain type with 0x80 set) |
| `DisplayInfo::Flags` | a `V_projMatrix3D 0x4000`, `V_FOV 0x800`, `V_matrix3D 0x1000` | the set **ends at `V_viewMatrix3D 0x2000`**; 0x800 and 0x1000 are `V_perspFOV` and `V_perspMatrix3D` |

Agent BE found the first two independently against the retail decompiles while porting the natives,
and the dump confirms it: every retail body masks the type with `0x8F` and tests the managed bit with
`0x40`. With the wrong masks `IsManagedValue()` would have been false on every live object, so
`GFxValue`'s copy constructor would have leaked every AS2 reference it touched.

All three are fixed, and the generator now emits **696 enumerator assertions** covering every enum in
the directory, hand-written ones included, so a guessed value cannot survive a compile again.

For the record, two of agent BE's other notes do **not** hold against the PDB and its own
`gfxui_gfx3.h` should be corrected, not this directory:

* `GFxValue::DisplayInfo` is **232** bytes, not 120. `dia_types.py` on
  `DishonoredGame-Shipping.pdb` reports 232 with 15 members (two `GMatrix3D`s at @96 and @160), and
  the committed `resources/docs/types/sizes.csv` — produced by IDA, a different tool on a different
  path — says `GFxValue::DisplayInfo,232,8` as well. `GFX3_ASSERT_SIZE(GFxValue::DisplayInfo, 232)`
  compiles.
* `ObjVisitor::Visit`'s second parameter is spelled `const GFxValue&` in the PDB, not
  `const GFxValue*`. It makes no ABI difference, so code written against either links; the
  declaration here follows the PDB.

### 2.5 What is deliberately opaque, and why

Four members out of 380 are kept as correctly-sized byte arrays with the PDB type in a comment
(`build/agentBB/gen_notes.txt`): `GFxResourceLib::PinSet` and `GFxResourceWeakLib::Resources` (GFx's
own hash sets, 4 bytes each — a pointer to a table), `GFxResourceLib::ResourceSlot::ResolveComplete`
(a `GEvent`, 44 bytes), and `GFxResourceWeakLib::ResourceNode::pResolver`. All four are internal to
the resource library, which is the runtime's business, not the seam's. Nothing else is approximated.

`FGFxRenderer`'s full 892-byte layout is **not** reproduced, and that is a decision rather than an
omission: 14 of its 35 members are RHI resource references, `TArray`/`TMap` instances and the
shader-state cache, it is a runtime object that nothing serializes, and the code that needs those
members is the drawing code this package deliberately does not write. The 21 members that are
matrices, viewports, stats and flags are all there at their PDB offsets, and the header lists every
omitted one. `sizeof(FGFxRenderer)` is 472 here against retail's 892; nothing reads it.

## 3. The asset-format answer — agent AW's open assumption, settled

`gfx_decision.md` section 4 lists this as one of the five things it could be wrong about: *"The
cooked assets are `gfxexport` `.gfx` with external textures … I measured the `SwfMovie`/`GFxRawData`
shape but did not parse a `.gfx` header; that check is one afternoon."* It has now been done twice —
once in Python over the cooked packages, once in C++ through our own parser — over **every**
`SwfMovie` payload in the retail cook, 22 of them.

**The answer: yes on the container and the bitmaps, no on the fonts.**

1. **`gfxexport` GFX, uncompressed, never Flash SWF.** All 22 payloads begin with the ASCII bytes
   `GFX` (`47 46 58`), never `CFX`/`FWS`/`CWS`/`ZWS`, and the `u32` at +4 is *exactly* the payload
   length in every one — which is what "uncompressed" means in this format. Version byte 8 or 10
   (the authoring Flash version). The header is the SWF header: signature, version, length, a
   bit-packed `RECT` frame rect in twips, an 8.8 fixed frame rate, a `u16` frame count; the first tag
   is at offset **21** in all 22 files.
2. **Every file carries Scaleform's own tags, and no standard SWF tag above 91 exists in the cook.**
   The non-standard codes over the whole game are exactly `{1000: 22, 1008: 483, 1009: 526}` — no
   code between 92 and 999, none above 1009. Tag **1000** (`GFx_ExporterInfo`) is always the first
   tag: exporter version **878**, `ExportFlags` **0** in every single file, `FileFormatType` **13** =
   `GFxFileConstants::File_TGA`. That is the decisive proof of `gfxexport` output, and the parser
   decodes all three tag bodies (layouts in `GFxGfxFile.h`).
3. **Bitmaps are stripped into engine textures.** There is **not one** `DefineBits*` tag in the whole
   game (codes 6, 8, 20, 21, 35, 36, 90 — zero occurrences). Instead one tag **1009**
   (`GFx_DefineExternalImage2`) per bitmap, naming a `.tga`, and the pixels live in a `Texture2D`
   export in the same package: **the 1009 count equals the package's `Texture2D` export count
   exactly** in every single-movie package (UI_HUD 11/11, UI_PauseMenu 25/25, UI_Journal 91/91,
   UI_PowerWheel 55/55, UI_Shop 46/46, UI_MissionStats 33/33, UI_Gamma 2/2). Where `bPackTextures`
   is on, tag **1008** (`GFx_DefineSubImage`, always 12 bytes: character id, image index, x0, y0, x1,
   y1) gives each character's rectangle inside the atlas. Cooked UI textures are `PF_DXT5`,
   `TEXTUREGROUP_UI`, `NeverStream`; `TextureRescale = FlashTextureScale_Mult4`, so the cooked size
   is the 1009 `SrcWidth`/`SrcHeight` rounded up to a multiple of 4 and **is not always equal to it**
   (HUD_I2: tag 606×232, texture 608×232). A consumer must not assume they match.
4. **Fonts are NOT pre-packed into textures — `gfx_decision.md` 2.5 guessed wrong here.** The
   `DisFonts*_SF` packages carry no `Texture2D` and no UE3 `Font` at all: they carry two `SwfMovie`s
   each, and each of those has exactly two `DefineFont3` (tag 75) with full glyph tables —
   `gfxfontlib` exports `$NormalFont` (`ChaletComprime-CologneEighty`, 188 glyphs, a 34,820-byte tag)
   and `$TitleFont` (`Emerge BF`, 188 glyphs, 68,729 bytes). There is **no GFx font-texture tag
   (1002/1005) anywhere in the cook**, consistent with `ExportFlags == 0`
   (`EXF_GlyphTexturesExported` clear). Every UI movie with `bUsesFontlib` pulls the two symbols in
   with `ImportAssets2` (tag 71). **Consequence: the glyph rasteriser and the glyph cache are on the
   critical path for any text in the UI** — the 270 KiB / 1,140-function text-and-fonts group of
   `gfx_decision.md` 2.6 cannot be skipped by loading pre-baked font textures. That is a real
   addition to the multi-wave plan and the single most important thing in this section.
5. **The content is ActionScript 2.** 669 `DoInitAction` tags (code 59) carrying `__Packages.*` class
   registrations, and **zero** `DoABC` (82) and **zero** `SymbolClass` (76) in the whole cook. So
   package BC is building an AS2 machine, not an AS3 one, which is what was assumed — now measured.
6. **The import URLs are authoring paths.** They still say `.swf`
   (`..\DisFonts\gfxfontlib.swf`) and mix `\` and `/` separators, sometimes inside one file. Any
   `GFxFileOpener` implementation has to normalise both.
7. One correction to `gfx_decision.md` 2.4's naming: the main menu's exported symbols are
   `m_StartScreen` (id 165), `m_MainMenu` (242) and `m_nGame` (201). `startScreen_mc` and
   `mainMenu_mc` are *instance* names on the timeline, not entries in the symbol table.

## 4. Step 3's acceptance, measured

`GFx3Dump --parse` reads the payload through the seam's own `FGFxFile` (a memory file — which is what
the retail `FGFxFile` layout says retail does: `BYTE* Buffer` @8, `INT Length` @12, `INT Position`
@16, so the opener hands the runtime a pointer into a package's bytes, never a stream) and then
through `GFxGfxParseFile`.

```
build\agentBB\dump\GFx3Dump.exe --parse build\agentBB\gfx\Dishonored_MainMenu.MainMenu.gfx
  signature      GFX  version 10  gfxexport GFX
  declared len   209523   payload 209523   (equal: uncompressed)
  frame rect     twips [0 25600 0 14400] = 1280 x 720 px
  frame rate     30.0 fps   frames 5
  first tag at   21   tags 477   distinct codes 22   non-standard 56
  embedded bitmap tags 0   glyph tags 0   consumed exactly yes
  exporter       version 878  flags 0x00000000  fileFormat 13  prefix ""  swf "MainMenu"
  exports 75   imports 10   external images 55   sub-images 0
```

75 exported symbols, read back with their character ids: `1 n_menuLine2 -nopack`, …,
`165 m_StartScreen`, `201 m_nGame`, `242 m_MainMenu`, then the 45 `__Packages.*` AS2 class
registrations (`__Packages.MainMenu`, `__Packages.StartScreen`, `__Packages.NewGameMenu`,
`__Packages._common.InputsManager`, `__Packages.gfx.motion.Tween`, …). `UI_HUD_SF.HUD`: 313,254
bytes, v8, 30 fps, 9 frames, 1,089 tags, 116 exports, 11 external images, **184 sub-images** in the
512×512 atlas `bPackTextures`/`PackTextureSize=512` produced. `DisFonts_SF.gfxfontlib`: 105,314
bytes, 12 fps, 1 frame, 14 tags, 2 exports (`$NormalFont`, `$TitleFont`), 2 glyph tags.

**All 22 payloads: parsed, `End` tag on the last byte, `ExportAssets` read, zero failures, zero
truncations, zero length mismatches** (`build/agentBB/parse_all.txt`). Every number the C++ parser
prints matches the independent Python extraction (`build/agentBB/movies.json`) — tag histograms
included, down to the per-code counts.

## 5. Step 2's acceptance, measured

`GFx3Dump --slots` instantiates `FGFxRenderer`, takes a `GRenderer*` to it and calls **all 53
non-destructor slots in order**, then every `GTexture` and `GRenderTarget` slot through their base
pointers, then the file opener, the image loader and creator, and the allocator:

```
DISHONORED(bringup): GFx renderer seam: 73 distinct slots recorded, 74 calls
sizeof: FGFxRenderer 472  FGFxTexture 24  FGFxRenderTarget 36  FGFxFile 88
        FGFxImageInfo 40  FGFxAllocator 16
```

Every call lands in the `FGFx*` override, named in the census, in the order the slots were called —
so the vtable is not merely complete, it is in the right order. `FGFxTexture` 24, `FGFxRenderTarget`
36, `FGFxFile` 88, `FGFxImageInfo` 40 and `FGFxAllocator` 16 are the PDB's sizes exactly.

Not every body is a stub. Real ports, with their 2013 rvas in `agentBB_status.csv`:
`FGFxRendererImpl::ConvertFromUI` (0x572c30), `FGFxRenderer::GetRenderCaps` (0x572e00, the caps the
player branches on before it calls any drawing slot), the viewport-matrix arithmetic of
`BeginDisplay` (0x59dd90) which the player reads back through `SetMatrix`, `GetRenderStats`,
**all 19 `GFile` slots** (0x587340 / 0x572210 / 0x5722a0 …: a memory file needs no engine),
`FGFxFileOpener::GetFileModifyTime` (0x58d9a0), `FGFxImageInfo`'s constructor and `Recreate`
(0x585c00 / 0x575070), and `FGFxAllocator::Alloc`/`Free` (0x574d90 / 0x574dc0). The drawing slots
count their primitives into `GRenderer::Stats` so a future run can prove they were reached.

## 6. Build wiring

`cmake/GFx.cmake` follows `cmake/PhysX.cmake`, with the one difference that there is no import
library to build because there is no DLL to import from:

* `DISHONORED_WITH_GFX3`, default **ON** when `External/GFx3/GFx3.h` exists. It builds the static
  library `gfx3` from the four translation units (`GFx3Layout.cpp`, `GFx3Support.cpp`,
  `GFx3RuntimeStubs.cpp`, `GFxGfxFile.cpp`) and exports `Dishonored::gfx3` carrying the header
  directory.
* `dishonored_apply_defines()` sets `DISHONORED_WITH_GFX3=1` on every target and links
  `Dishonored::gfx3`, for the same reason Bink and PhysX go on every target: `GFxUI` and
  `DishonoredGame` both name these types. It changes no engine layout.
* `GFxUI/Sources.cmake` drops `gfxuirenderer.cpp`, `gfxuifile.cpp` and `gfxuiimageinfo.cpp` from the
  exclude list when the option is on (and excludes the new `gfxuiallocator.cpp` when it is off).
* `GFx3Dump` is `EXCLUDE_FROM_ALL`: `cmake --build <dir> --target GFx3Dump`.

**Turning this on cannot change how the game runs.** The seam units include no engine header at all
(that is deliberate, and it is what lets `build/agentBB_seam.cmd` compile them with no engine to
check that every slot is present), nothing in the engine instantiates any of it yet, and the headers
are only visible to translation units that include them.

## 7. Verification

* **The API and the seam, on their own** (`build/agentBB_seam.cmd`, VS 2022 x86, `/Zp4`): 0 errors,
  0 link errors, `gfx3seam.lib` produced — i.e. every one of the 54 + 12 + 5 + 7 + 3 + 4 + 19 + 2 + 5
  slots is not just declared but *defined*. The 171 + 380 + 696 PDB assertions are in that compile.
* **The harness** (`build/agentBB_dump.cmd` → `build/agentBB/dump/GFx3Dump.exe`): `--slots` output
  above; `--parse` over all 22 payloads in `build/agentBB/parse_all.txt`, exit code 0.
* **Isolated full build**: `python resources/tools/make_snapshot.py BB --list
  build/agentBB_files.txt` (HEAD `28eb3cf` + only my 24 files) built RELEASE into `build/agentBB_rel`
  with `build/agentBB_relbuild.cmd` and `build/agentBB_relbuild_all.cmd`: **833 units, 0 errors, 0
  link errors**, all four targets produced (`DishonoredGame.exe`, `CoreSmoke.exe`, `LayoutProbe.exe`,
  `GFx3Dump.exe`). `-- GFx: DISHONORED_WITH_GFX3=1` and `Linking CXX static library gfx3.lib` are in
  the log (`build/agentBB_relbuild.log`, `..._all.log`). Using the snapshot rather than the shared
  tree is deliberate: five other packages were editing it at the same time.
* **Regression**: `python resources/tools/run_regression.py --build-dir build/agentBB_rel --no-build`
  → **28 ok, 0 failed, 2 skipped, 489 s** (`build/agentBB_regression.txt`,
  `build/agentBB_rel/regression/summary.txt`). The two skips are the optional `touch_census` and
  `sequence_census` counters, which do not exist in HEAD and are skipped in the baseline too.
  Highlights: CoreSmoke 99/0, layout 2,314 types with 0 mismatches, nullrhi 0 criticals, d3d9 1,740
  frames / 0 criticals / 6,506 draw elements / 16,257 textures, inputtest **pawn walks 1,024.3 units**
  with 0 criticals, 895 PhysX actors, 2 unported natives on the walking path. Nothing in this package
  touches a code path the harness measures: no engine file was edited, and the only shared files
  changed are three cmake files and one `Sources.cmake`, all with targeted edits.
  After the `GFxValue` enum fix of section 2.4a relinked the exe, the three load-bearing stages were
  re-run on it (`--only coresmoke,nullrhi,inputtest`, `build/agentBB_regression2.txt`): **13 ok,
  0 failed, 2 skipped**, pawn walks 1,013.4 units, 0 criticals in either run.
* **`-newgame` still works** on the same exe (`build_and_smoke.py … --rhi null "--extra-args=-newgame
  -forcelogflush"`, exit 0, log `agentBB_newgame.log`): `Initial startup: 5.77s`, then
  `Committed map change via DishonoredEngine`, then
  `DISHONORED(bringup): startmap: 'ce ChangeLvl_StartNewGame' after the Dishonored_MainMenu commit`,
  then the second commit — the retail New Game route, end to end, with no critical error. `-startmap`
  is what the regression's d3d9 and inputtest stages already run.
* **With the switch off**: the same snapshot configures cleanly with `-DDISHONORED_WITH_GFX3=OFF`
  (`build/agentBB_off`, log `build/agentBB_offconfig.log`): no `gfx3` target, none of the four seam
  units in `build.ninja`, and `DISHONORED_WITH_GFX3=0` on every compile line. So the wiring can be
  turned off without editing a file, which is what agent BE's "builds with and without the
  runtime" acceptance needs.
* **The overload-order probe**: `build/agentBB/vt/` (`vtprobe.cpp`, `vtprobe.cod`).

## 8. Hand-overs

**Agent BC — the API is stable. Build on it.** Specifically:

1. `GFx3Gen.h`, `GTypes.h`, `GFxValue.h`, `GFx3Enums.h` and `GFxGfxFile.h` are final for this wave.
   Do not edit them; they are generated (`build/agentBB_gen_gfx3.py`) or hand-written against the
   PDB with assertions, and an edit that drifts from the PDB will fail `GFx3Layout.cpp`. If you need
   a type the dump has but the generator excluded (the `EXCLUDE_RE` list: the IME family, the text
   and font internals, the threading primitives), **tell me rather than adding it by hand** — one
   line in the generator's exclude list brings it in with its assertions.
2. **`GFx3RuntimeStubs.cpp` is your work list at the interface level**: 57 non-pure virtuals that
   libgfx implements and we do not. Delete each entry as the real body lands; the link will tell you
   if you removed one that is still needed.
3. **`GFxValue::ObjectInterface`'s 25 methods are declared and not defined** — deliberately. They are
   the single most important thing you can implement, because 1,096 of the engine's 1,350 calls into
   libgfx are those methods. Their exact signatures and 2012 rvas are in `GFxValue.h`; the two
   `GotoAndPlay` and two `SetText` overloads are non-virtual, so no slot ordering to worry about.
4. **The content is AS2 with 669 `DoInitAction` tags of `__Packages.*` class registrations, and the
   fonts are glyph outlines, not textures** (section 3). Plan the text engine in.
5. `GFxGfxParseFile` already gives you the tag stream boundaries, the symbol table, the imports and
   the external-image table for any payload; the tag *bodies* other than 56/71/1000/1008/1009 are
   skipped by length, which is where your character/tag model plugs in.

**Agent BE — thank you for the `GFxValue` catch, and three replies.** (1) The `VTC_*` bits, the
convertible-type values and `DisplayInfo::Flags` are fixed here exactly as you measured them, and 696
enumerator assertions now guard every enum in the directory. (2) `GFxValue::DisplayInfo` is **232**
bytes, not 120, and `ObjVisitor::Visit` takes `const GFxValue&` — evidence in section 2.4a; please fix
those two in `gfxui_gfx3.h`. (3) `DISHONORED_WITH_GFX3` means exactly what your header says it means,
so your separate `DISHONORED_GFXUI_GFX3_RUNTIME` switch is the right shape and nothing in
`cmake/GFx.cmake` fights it: `GFx3RuntimeStubs.cpp` provides the 57 non-pure interface virtuals and
nothing else, and I checked: **none of those 57 names appears in your `gfxuigfx3absent.cpp`**, so the
two files cannot define the same symbol. One merge note for the coordinator rather than for you:
we both edited `GFxUI/Sources.cmake` and the edits are disjoint (you took `gfxuimovie.cpp` out of
the exclude list, agent BD took `gfxuishaders.cpp`, I appended an `if(DISHONORED_WITH_GFX3)` block
at the end that removes `gfxuirenderer.cpp`, `gfxuifile.cpp` and `gfxuiimageinfo.cpp`), so both
sets have to survive the merge.

**Agent BE — the seam is yours to call, not to edit.** `FGFxFileOpener::OpenFile`,
`FGFxImageLoader::LoadImageW` and `FGFxImageCreator::CreateImage` are the three bodies that need
`UObject` and are therefore yours: the url or export name has to be resolved against the package the
movie came from, and `FGFxTexture::InitTexture(UTexture*)` (2013 0x580810) is where the resulting
`Texture2D` lands. Everything else in `gfxuirenderer.{h,cpp}`, `gfxuirendererimpl.h`,
`gfxuifile.{h,cpp}`, `gfxuiimageinfo.{h,cpp}` and `gfxuiallocator.{h,cpp}` is mine for this wave —
please do not edit those six headers and four units; `gfxuiengine.cpp`, `gfxuimovie.cpp`,
`gfxuiinteraction.cpp`, `gfxuidatastore.cpp`, `gfxuilocalization.cpp`, `gfxuifont.cpp`,
`gfxuishaders.cpp`, `GFxUINativeStubs.cpp` and `GFxUIRegistrants.cpp` are untouched by me. If you
add units to `GFxUI/Sources.cmake`, note that I appended a 12-line `if(DISHONORED_WITH_GFX3)` block
at the end of it.

**Agent BD** — `FGFxTexture::Bind` and `FGFxRenderer`'s `CheckFilterSupport`, `DrawBlurRect` and
`DrawColorMatrixRect` are the four seam slots that need the GFx pixel shaders. The 31 PDB functions
attributed to `gfxuirendererimpl.h` are almost all `FGFxPixelShader<N>` / `FGFxFilterPixelShader<N>`
/ `FGFxVertexShader<N>` instantiations, which is your shader-family work, not the seam's.

**Coordinator** — `gfx_decision.md` section 4 row 3 and section 2.5 should be updated at merge: the
container assumption is confirmed, the **font** assumption is not (section 3.4), and that changes the
scope of the runtime port. `middleware.md` 2.3 should gain the line that the reconstruction route is
now open and has a foundation.

## 9. What remains in this package's area

1. **The runtime.** 5,635 functions; this package wrote the interfaces and the seam, not the player.
2. **`FGFxRenderer`'s RHI half**: 14 members and the 221 functions of `gfxuirenderer.cpp`'s PDB list
   that actually draw. Every slot is in place for them to be dropped into one at a time.
3. **`FGFxUpdatableTexture`** (PDB sizeof 92, base `FTextureResource`) and
   `UGFxMappableTexture`/`UGFxUpdatableTexture` are not declared here: they are RHI resources, not
   part of the GFx-facing contract, and they need `FTextureResource`, which would have forced the
   seam to include Engine and given up the no-engine compile check.
4. **The four opaque members** of section 2.5, if the resource library is ever driven from our side.
5. **`GFxGfxFile.h`'s three GFx tag layouts are measured, their names are inferred.** `1000`, `1008`
   and `1009` are decoded byte for byte and every length adds up, but the symbolic names
   (`GFx_ExporterInfo`, `GFx_DefineSubImage`, `GFx_DefineExternalImage2`) are Scaleform's own and the
   `GFxTagType` enum is not in this tree's type dump. Likewise the meaning of tag 1009's
   `u16 Flags` (9 on the packed-atlas entries, 0 otherwise) is a correlation with `bPackTextures`,
   not a proven bit layout.
6. **A compressed `CFX` payload** would need a zlib pass before the tag walk; the parser says so
   rather than guessing. Nothing in the retail cook is compressed, so this is unreachable today.

## 10. Files

Mine (26): the 11 files of `source/Development/Src/External/GFx3/`, `cmake/GFx.cmake`, targeted edits
in `cmake/Dependencies.cmake` and `cmake/DishonoredDefines.cmake`, `GFxUI/Sources.cmake`, the five
`GFxUI/Inc/gfxui{renderer,rendererimpl,file,imageinfo,allocator}.h`, the four
`GFxUI/Src/gfxui{renderer,file,imageinfo,allocator}.cpp`, plus this report and
`agentBB_status.csv`. The file list used for the snapshot is `build/agentBB_files.txt`.

Scratch (not repo tools): `build/agentBB_gen_gfx3.py` (the header generator — re-run it after any
PDB dump change), `build/agentBB_seam_headers.py` and `build/agentBB_seam_bodies.py` (the seam
writers), `build/agentBB/g_types.json` + `fgfx_types.json` (the two PDB dumps),
`build/agentBB/gen_notes.txt`, `build/agentBB/show.py` and `show2.py` (dump readers),
`build/agentBB/extract_gfx.py` + `movies.json` + `scan_all.txt` + `gfx/*.gfx` (the 22 extracted
payloads and the independent Python measurement), `build/agentBB/parse_all.txt` and
`parse_mainmenu.txt`, `build/agentBB/vt/` (the overload-order probe),
`build/agentBB_{cc,seam,dump,relbuild,relbuild_all}.cmd`, `build/agentBB_relbuild*.log`,
`build/agentBB_regression{,2}.txt`, `build/agentBB_offconfig.log`, snapshot `build/agentBB_wt` +
the two build directories `build/agentBB_rel` (GFx3 on) and `build/agentBB_off` (configure-only,
GFx3 off).

No IDA database was opened: everything came from the 2012 PDB through DIA and from the committed
`resources/docs/symbols/*.csv` and `resources/docs/types/sizes.csv`. No commits, no `git add`, no junctions, nothing deleted under
`Dishonored_Latest2026`.
