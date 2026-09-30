# Agent EM (PHASE11 EM) — the HUD is on screen: one of the two gaps was real, the other was its shadow

Branched from HEAD `0cbe3fe`, own worktree `build/agentEM_wt`, own build dirs `build/agentEM_rel` (this
package), `build/agentEM_headrel` (the untouched `0cbe3fe` baseline, built from a `git archive` export in
`build/agentEM_headsrc`), `build/agentEM_wt/build/agentEM_reg` (the regression harness) and
`build/agentEM_clean` (the gate build). Own IDA copy `build/agentEM_ida/retail2013_agentEM.i64`, headless
decompiles only. **No IDA MCP tool and no FModel tool was used.** No commits, nothing staged, nothing
written into the main checkout.

Six files, five in `External/GFx3` and one in `GFxUI`. Nothing outside those two trees, `Sources.cmake`
unchanged, `gen_classes_header.py` regeneration **not required and not run**.

## The answer in seven lines

* **The HUD is drawn, in a level, reached normally, with nothing bypassed and nothing forced visible.**
  `build/agentEM/final.png`, built from exactly the source this package hands over: `L_Tower_P` with
  `-dishud`, the health and mana vials at the top left and the stance icon at the bottom left, against
  `build/agentEM/rhead.png` — the same command line, the same driver, the untouched HEAD executable —
  which has no interface at all. (`r1.png` is the same frame from the build the isolation table of
  section 3 was taken on; the two differ only by a source comment and by the pawn's health.)
* **Cause 1 was real and is fixed. It is one line plus its consequence.** Tag 1008's second `u16` is the
  atlas's **index**, and retail turns it into a resource id by adding the packed-image type bits:
  2013 `0xa2cf50` writes `ImageIndex | 0x90000`. This tree read it bare, so a sub-image whose atlas is
  index 5 asked the dictionary for **character 5**, which in `UI_HUD` is an unrelated shape. Of the
  movie's 184 sub-images, every one whose atlas index was not 1 or 2 resolved to nothing, and the ones
  that were textured the wrong picture. Section 2.
* **Cause 2 is not a second cause. The mask path was already correct, and the brief is wrong about it.**
  With the sub-image fix in and **every line of the mask path exactly as HEAD has it**, the HUD draws and
  the health vial's red column is clipped to the health value (`build/agentEM/r2.png`). With the mask
  work in and the sub-image fix out, the frame is HEAD's (`build/agentEM/r3.png`). The measurement is in
  section 3, and what agent EK actually saw is explained there.
* **The two images the HUD "loaded" at HEAD were the wrong ones.** HEAD resolves `UI_HUD.HUD_I1` and
  `HUD_I2`; those are the movie's two `-nopack` standalone bitmaps, reached only because bare atlas
  indices 1 and 2 collide with character ids 1 and 2. With the fix the run loads five **atlases**
  (`HUD_I85`, `HUD_IA4`, `HUD_I54`, `HUD_I5C`, `HUD_I67`) and neither of the two. Section 2.3.
* **Each failing fill's shape is authored at exactly its sub-rectangle's pixel size** — nine of nine,
  checked against the cooked payload's own tag bytes — which is what makes the UV map
  `(rect origin + pixel) / atlas size` and not a guess. Section 2.2.
* **A retail reference exists now, and it says something useful rather than what was wanted.** The retail
  2013 `DishonoredGame.exe`, booted on `L_Tower_P` at the same window size, shows **no interface at all**
  (`build/agentEM/retail_tower.png`, `retail_tower2.png`): the HUD is created out of the menu flow's UI
  state, not by entering a level, which is EK's hand-over 4 measured from the other side. I did **not**
  get retail past its first-run gamma screen into a real first mission; section 6 says how far I got and
  what is left.
* **Regression 37 ok, 0 failed, 0 skipped**, built inside the harness; a clean full Release build with the
  directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**; `rva_sweep`
  plus a by-hand `ida_funcs.get_func` resolution of all twelve cited addresses. Section 5.

## 1. What the package is

| retail 2013 | what | where it is used |
|---|---|---|
| `0xa2cf50` | `GFx_DefineSubImageLoader` — the base image id is `ImageIndex \| 0x90000` | `GFxTagLoaders.cpp` |
| `0xa2c790` | `GFx_DefineExternalImageLoader2` — the id it registers an atlas under, masked `& 0x9FFFF` | the other half of the same id space |
| `0x9b6410` | `GSubImageInfo::GetWidth` — forwards to the BASE image, so it is the sheet's width | `GFxCharacterDefs.cpp` |
| `0x9b6420` | `GSubImageInfo::GetHeight` — the same | `GFxCharacterDefs.cpp` |
| `0x9b6430` | `GSubImageInfo::GetTexture` — forwards to the base image's texture | `GFxCharacterDefs.cpp` |
| `0x9b6440` | `GSubImageInfo::GetRect` — the rectangle inside the sheet | `GFxCharacterDefs.cpp` |
| `0x585d00` | `GImageInfoBase::GetRect` — `(0, 0, GetWidth(), GetHeight())`, the plain-image case of the same expression | `GFxCharacterDefs.cpp` |
| `0xa53340` | `GFxDisplayContext::PushAndDrawMask` — the mask-draw depth counter at `+148` | `GFxDisplay.h` / `.cpp` |
| `0x9cbf20` | `GFxDisplayList::Display` — `if (!ClipDepth \|\| MaskDrawDepth) draw as content; else push a mask` | `GFxDisplay.cpp` |
| `0x9f1780` | `GFxSprite::Display` — retail's visibility test, and the alpha test it does **not** have | `GFxDisplay.cpp` |

`resources/docs/agents/agentEM_status.csv` carries the same list with every note.

## 2. Cause 1: an atlas sub-image could not find its atlas, and could not have mapped it if it had

### 2.1 The base id is in the packed-image id space

`GFx_DefineExternalImageLoader2` (tag 1009) already read its id the way retail does — a `u32` masked with
`0x9FFFF`, which agent BB took off retail's own body and which 2013 `0xa2c790` still shows
(`sub_E2C3F0(v2, v6 & 0x9FFFF, ...)`). In `UI_HUD.HUD` that tag appears eleven times, and the payload
bytes say what the mask is for:

```
1009 idx=0 flags=9 fmt=13 512x512  file='HUD_I85.tga'     <- an atlas, id 0x90000
1009 idx=5 flags=9 fmt=13 512x512  file='HUD_I54.tga'     <- an atlas, id 0x90005
1009 idx=7 flags=9 fmt=13 1024x256 file='HUD_I240.tga'    <- an atlas, id 0x90007
1009 idx=1 flags=0 fmt=13 557x245  export='hud_tgt_bkgd -nopack'  file='HUD_I1.tga'   <- id 1
1009 idx=2 flags=0 fmt=13 606x232  export='hud_obj_bkgdR -nopack' file='HUD_I2.tga'   <- id 2
```

The `u32` is `idx | (flags << 16)`, so an atlas lands at `0x90000 | idx` and a `-nopack` image at its bare
index. Tag 1008 carries the atlas's index **without** those bits — six `u16`s, twelve bytes, and the
second one is 0..7 across all 184 of `UI_HUD`'s sub-images. Retail bridges the two id spaces in the
loader, and the decompile of 2013 `0xa2cf50` is one line:

```c
*((_DWORD *)v33 + 2) = v12 | 0x90000;      // GFxSubImageResourceInfo's base id, v12 = the second u16
```

This tree wrote `v12`. `GFxImageCharacterDef::GetTexture` then asked `GetCharacterDefById(5)` and got
character 5 of the movie — which is why agent EK's probe reported "base id 5 -> type 133" and
"base id 0 -> def NULL": 133 is a shape's type code and character 0 does not exist.

### 2.2 The rectangle, and why the UV map is what it is

`GSubImageInfo::GetWidth` (2013 `0x9b6410`) and `GetHeight` (`0x9b6420`) both forward to `pBaseImage`, so
for a sub-image they answer the **sheet's** size, while `GetRect` (`0x9b6440`) answers the rectangle and
`GetTexture` (`0x9b6430`) the sheet's texture. The base class's `GetRect` (`0x585d00`) is
`(0, 0, GetWidth(), GetHeight())`. One expression therefore covers both cases —
`uv = (GetRect().TopLeft + pixel) / GetWidth()` — and `GFxImageCharacterDef::BuildPixelToUVMatrix` is that
expression. Both call sites use it: the shape fill in `GFxDisplayApplyFill` and the direct timeline
placement in `GFxImageCharacterDef::Display`.

The reason the fill matrix's inverse lands in *sub-image* pixel space and not sheet space is measured, not
assumed. The nine fills agent EK reported as `NOT TEXTURED` have shape bounds in twips, and every one of
them is exactly twenty times its own sub-rectangle's pixel size:

```
id    atlas idx  rect in the atlas        shape bounds
627   5          98x98   @ 56,129         98x98
629   0          22x153  @483,  0         22x153
633   0         131x281  @302,  0        131x281
636   0          46x242  @435,  0         46x242
657   5          46x22   @207,229         46x22
678   0          45x201  @148,302         45x201
681   0          46x201  @100,302         46x201
689   0          46x201  @ 52,302         46x201
691   0          27x13   @392,283         27x13
```

Nine of nine. `build/agentEM/gfxtags.py` prints the tag bytes that left column comes from; the right
column is the `GFx fill` trace line this tree already logs.

### 2.3 What it did to the frame

`L_Tower_P?Name=Corvo?Team=255 -windowed -ResX=1280 -ResY=720 -nomovie -nosteam
-skipnativepkgs=OnlineSubsystemPC -dishud -apshottime=30`, through `build/agentEM_run.py`, on the
untouched-`0cbe3fe` executable in `build/agentEM_headrel` and on this package's in `build/agentEM_rel`.

| | HEAD `0cbe3fe` | agent EM |
|---|---:|---:|
| bitmap fills in the frame census | 10 | **20** |
| untextured fills skipped | **10** | **0** |
| draws / triangles per frame | 16 / 166 | **26 / 186** |
| mask passes per frame | 6 | 6 |
| images resolved | `HUD_I1`, `HUD_I2`, `HUDFX_I23`, `HUDFX_I25` | `HUD_I85`, `HUD_IA4`, `HUD_I54`, `HUD_I5C`, `HUD_I67`, `HUDFX_I23`, `HUDFX_I25` |
| the HUD on screen | **nothing** | health vial, mana vial, stance icon |

`HUD_I1` and `HUD_I2` disappearing is the point rather than a loss: they are the two `-nopack` bitmaps,
and nothing in the frame references them. HEAD loaded them because two sub-images' bare atlas indices, 1
and 2, collided with their character ids — so the only two images HEAD ever textured were the wrong
pictures, resolved by accident.

Logs `build/agentEM/rhead_log.txt` and `r1_log.txt`, frames `rhead.png` and `r1.png`, and the top-left
crops `cmp_rhead_tl.png` and `cmp_r1_tl.png`.

## 3. Cause 2 does not exist, and here is the run that says so

The brief states, on agent EK's evidence, that "the stencil mask path clips away what survives" and that
this is an independent second cause. It is not. `build/agentEM_isolate.py` turns either fix off and on
again in the worktree, so one build directory produces all four legs:

| leg | sub-image fix | mask work | untextured fills | the frame |
|---|---|---|---:|---|
| `rhead` | — (untouched HEAD exe) | — | 10 | nothing |
| `r3` | off | on | 10 | nothing |
| `r2` | on | **off — every mask line exactly as HEAD has it** | 0 | **the HUD, gauges clipped correctly** |
| `r1` | on | on | 0 | the HUD, gauges clipped correctly |

`r2` is the decisive one. Its `BeginSubmitMask` / `EndSubmitMask` / `DisableMask`, its stencil states, its
`GFxDisplayList::Display` mask branch and its alpha early-out are HEAD's, byte for byte — and the health
vial's red column is clipped to the health value the tick pushed (`FillHealthGauge(25.0, 0, 0, 1, 25.0)`,
and the red stands at a quarter of the vial under a clean horizontal edge: `build/agentEM/vial_r2.png`).
A stencil path that "clips away what survives" cannot produce that picture. Six mask passes a frame are
submitted in every leg, including the two that draw the HUD, and `r1` and `r2` agree on every counter of
the frame census — 94 display objects, 20 bitmap fills, 26 draws, 186 triangles, 6 masks, 0 untextured
fills, 0 empty masks.

What agent EK saw is consistent with this and with nothing else. At HEAD every bitmap fill in the HUD is
an atlas sub-image and `GFxDisplayApplyFill` answers `false` for one it cannot texture, which leaves
`GFx_SM_Disabled` and makes `DrawIndexedTriList_RenderThread` return — so the *content* drew nothing
whatever the stencil said. The mask shapes are a different matter: `BeginSubmitMask` is what turns colour
writes off, so no-oping the three entry points leaves them on and the mask geometry paints. EK corrected
itself on exactly that point ("the shapes that appeared were the *mask* shapes") and then kept the
conclusion the corrected observation no longer supports. The corrected observation is the whole of it:
`nomask.png` is a picture of mask shapes, not of a mask problem.

### 3.1 What the package still changes in the mask path, and why it is not a fix

Two things, and neither changes a pixel of this content — measured, not assumed: `r1` and `r2` differ only
by the health value the pawn happened to have (40 against 25; the fall damage in the opening varies run to
run) and by nothing else in the census.

1. **`GFxDisplayContext::MaskDrawDepth`**, retail's `GFxDisplayContext+148`. `PushAndDrawMask`
   (`0xa53340`) increments it for exactly as long as the mask character's own `Display` runs, and
   `GFxDisplayList::Display` (`0x9cbf20`) reads it: `if (!ClipDepth || MaskDrawDepth)` draw the entry as
   ordinary content, `else` open a mask. A clip that carries a `ClipDepth` while a mask is being drawn is
   part of that mask's shape, not the start of a nested one. This tree opened a nested mask there.
2. **The alpha early-out does not reach a mask.** `GFxSprite::Display` in retail (`0x9f1780`) tests
   visibility (`(flags & 3) == 1 || (flags & 0x8000)`) and has **no alpha test at all**; the
   `GFxDisplayCxformIsTransparent` early-out is agent EA's, added because the cooked
   `GFx_PS_CxformTexture` shader does not honour the Cxform. A mask is submitted with colour writes off,
   so its alpha decides nothing, while skipping it leaves the stencil at its cleared value and
   `EndSubmitMask`'s `== counter` test then rejects every pixel the mask covers. A Flash mask layer being
   invisible is the authoring convention, so that is a live hazard even though `UI_HUD` does not trip it.

Both are retail's own shape, both are one condition each, and the package states plainly that neither
changed a measured pixel. The third change in this area *is* measurement: the frame census now reports
**empty masks**, a mask whose shape submitted no triangle at all, and names the first eight in the log —
because a mask that draws nothing does not clip nothing, it clips everything, and that is the failure the
brief guessed at. Every run above reports **0**.

## 4. What the frame looks like, and against what

`build/agentEM/vial_r1.png` is the health and mana gauges at 3x — Dishonored's health vial in red and
mana vial in blue, at the position `PreRender_Layout` puts them, with the red column filled to the bottom
40 % of the vial behind a clean horizontal edge while `APawn::Health` reads 40 of 100, and the mana vial
empty while `m_Mana` reads 0 of 0. `FillHealthGauge(40.0, 0, 0, 0, 40.0) -> ok [health 40/100]` at
t=17.86 s and the frame is taken at t=30.0 s. The stance icon is at the bottom left.
`build/agentEM/vial_final.png` is the same pair from the delivered source's own build, at 37 of 100.

That horizontal edge is the mask, and `build/agentEM/vial_r2.png` is the same gauge with **HEAD's mask
code**: `FillHealthGauge(25.0, 0, 0, 1, 25.0)`, and the red column stands at a quarter of the vial with
the low-health glow the fourth argument turns on. Two runs, two health values, two correctly clipped
columns, one of them with every line of the mask path untouched. The movie is reading at least the
percentage and the low-health flag out of the five arguments, and the stencil is doing its job.

**The reference.** `resources/reference/` has four menu images and nothing of the in-game interface (EK's
hand-over 6), so I went to the retail game itself. `build/agentEM_retailshot.py` launches retail 2013's own
`DishonoredGame.exe` on the same map at the same window size and takes the game's **own** screenshot — F10
is bound to `screenshot` in `DishonoredGame/Config/DefaultInput.ini:266`, so the capture is inside the
process and an overlapping window cannot corrupt it, which mattered (see section 6). The result, twice:
retail on a direct `-map L_Tower_P` boot shows **no interface at all**, before and after the player is
given control and moves (`retail_tower.png`, `retail_tower2.png`, the second after six keypresses that
visibly move the camera). That is a real retail measurement and it confirms EK's hand-over 4 from the
other side: retail's HUD comes out of `UDisGlobalUIManager::RefreshGlobalUIState` and the menu flow's UI
state, not out of entering a level. It is **not** a picture of retail's HUD, and this package does not have
one; section 6 says exactly how far the attempt got.

So the claims about how the HUD should look rest on three things that are retail's and not mine: the
sub-rectangles in retail's own cooked atlas (section 2.2, nine of nine), `PreRender_Layout`'s arithmetic
(agent EK ported it whole from `0x796250`), and the live values the tick pushes.

## 5. Verification

* **Regression**: `python resources/tools/run_regression.py --build-dir
  D:\RecompileDishonored\Recompile\build\agentEM_wt\build\agentEM_reg --exe-name DishonoredGame_EMreg.exe
  --log-prefix EMreg`, run from `build/agentEM_wt` with no `--no-build`, so the harness builds this
  package's sources. The five gitignored layout inputs were copied into the worktree first, so every
  layout metric is a real number. Result **37 ok, 0 failed, 0 skipped, 543 s**
  (`build/agentEM/regression8.log`), on exactly the source this package hands over. The harness never
  passes `-dishud`, so the HUD is measured by hand above.

  It took four full runs to get one, and the three that failed are worth naming because none of the
  failures is this package's and the next agent will meet them. Three other agents were running the game
  or building throughout. Every failure is a load metric:

  | run | result | what failed |
  |---|---|---|
  | `regression1` | 35 ok, 2 failed | `d3d9_frames` 510 (bound 1000); `inputtest_peak_speed` 0.0 |
  | `regression5` | 36 ok, 1 failed | `inputtest_moved` 617.9 (bound 800) |
  | `regression6` | **37 ok, 0 failed** | — |
  | `regression7` | 35 ok, 2 failed | `startup_seconds` **141.9** (bound 60, 2.7 at HEAD); `d3d9_frames` 990 |
  | `regression8` | **37 ok, 0 failed** | — |

  `startup_seconds` at 141.9 s against 2.7 s is the machine and nothing else. The two `inputtest` metrics
  have a mechanism worth recording: `L_Tower_P` streams eight levels, and when they arrive late the pawn
  falls out of the world before it ever has a floor. The failing run's own trace says so —
  `inputtest waiting for ground: 8.1s ... Z=-6220.261, floor none`, seven more such lines, then a walk
  that never happened (`peak 2D speed 0.0`) with `-distouchprobe`'s teleports counted as 4781 units of
  movement. The passing runs find ground at 2.3 s. Across seven runs of this package's exe the distance
  read 4781.1 / 3049.1 / 381.3 / 1031.5 / 617.9 / 1031.3 / 1038.9, and the untouched HEAD exe through the
  same harness read 1047.0 (`build/agentEM/regression_head_inputtest.log`) — which is what rules this
  package out as the cause, together with the fact that the harness opens no movie and so runs none of
  the changed code.
* **The menu, which is the one thing this package could have broken.** `-gfxuimenu` opens
  `UI_MainMenu.MainMenu` and `UI_Global.Global`, and both go through the changed fill path. This
  package's exe and the untouched HEAD exe give the same census to the counter — 33 display objects,
  9 bitmap fills, 9 draws, 18 triangles, 0 masks, 0 untextured fills — and the same frame:
  `build/agentEM/menu_head.png` and `menu_em.png` differ only in the animated 3D background's phase, with
  the logo, the reeds and `PRESS ANY KEY` where they were. The menu's own payloads carry no tag 1008 at
  all (`Dishonored_MainMenu.MainMenu.gfx` and `DishonoredGame.Global.gfx`: 477 and 310 tags, 0 of them
  1008) and the frame reports 0 masks, so this is a "nothing moved where nothing could move" result and
  it is stated as one. Nothing in this wave exercises the mask changes of section 3.1.
* **Clean build**: `build/agentEM_clean` deleted first (`DISHONORED_LAYOUT_CHECKS:BOOL=ON` in its
  cache), all three targets built from the worktree: **0 errors, 0 C4263, 0 C4264**
  (`build/agentEM/clean1.log`, `clean2.log`, `clean3.log`).
* **rva_sweep**: over `External/GFx3`, where five of the six changed files live - **602 citations, 206
  `ok-2013`, 391 `ok-2013-mid`, 5 `ok-2012-labelled`, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013**. Over
  the whole tree: **2** suspects, both pre-existing, both the `GFxUI/Src/gfxuirenderer.cpp:1669` line
  whose own words say "2012" and which this package does not touch.
* **By-hand resolution**: every cited address through `ida_funcs.get_func` in
  `build/agentEM/resolve.txt` — the ten of section 1 plus the two of hand-over 4 (`0xa19650`,
  `0xa18ff0`). All twelve are function **starts** with the name the citation claims.

## 6. What I did not get, named rather than left silent

* **A picture of retail's own HUD.** `build/agentEM_retailplay.py` starts retail with no map, walks the
  menu with Enter/Space and takes the game's own screenshot every twenty seconds for five minutes
  (`build/agentEM/retail_play/`, `retail_contact.png`). It gets past the first-run gamma calibration and
  no further: the blind Enter/Space presses move the gamma slider instead of taking New Game, and the
  fourteen frames are the main menu with the brightness pushed to maximum. Finishing it needs
  `drive_input.py`'s `hoverclick` against `--exe DishonoredGame.exe` and the menu item's position, which
  is half an hour someone should spend once — `agentEM_retailplay.py` is the other 90 % of it.
* **Two Windows Defender Firewall prompts appeared during that work and I clicked neither.** One is mine:
  my first retail attempt ran `Dishonored.exe` (the launcher — it never opens a usable window, which is why
  the script uses `DishonoredGame.exe`) and Windows raised a prompt for it. The other names
  `dishonoredgame_ei.exe`, which is agent EI's executable and not mine to answer. Both sat over the game
  window and corrupted one outside-the-process capture, which is what moved the script onto the game's own
  F10 screenshot. `build/agentEM/retail_tower.png` was re-taken after that change and is clean.
* **The desktop is 1024x768 and DPI-scaled**, so a 1280x720 window has a 1008x567 client area and there is
  nowhere on screen to move a window clear of a modal dialog. Every frame in this package is 1008x567 for
  that reason, HEAD's and retail's alike, so the comparison is like for like.
* **`GFxDisplayFitFill`** — the diagnostic branch in the fill path that fits the texture to the shape's
  bounding box — is untouched and still `false`. It bypasses the fill matrix entirely and would hide any
  future error in it.
* **Three of `UI_HUD`'s eight atlases are never drawn in these runs**: atlas index 3 (`HUD_IC8`), 6
  (`HUD_I46`) and 7 (`HUD_I240`, 1024x256 and the only non-square sheet). The opening frame reaches the
  other five. Nothing says these three are wrong; nothing has exercised them either.

## 7. Merging

* **`gen_classes_header.py` regeneration: NOT required, and NOT run.** No class, reflected property or
  interface override is declared; the two changed headers gain one member and one member function.
* **`Sources.cmake`: unchanged.** No file is added or removed from any module.
* **Files outside `External/GFx3` and `GFxUI`: none.** The one `GFxUI` file is
  `GFxUI/Src/gfxuiengine.cpp`, and the change is two format specifiers and one argument on the existing
  `GFx UI census (…): machine:` line.
* **Total: 6 source files**, plus this document and `agentEM_status.csv`. `build/agentEM_sync.py` is the
  authoritative list.
  * `External/GFx3/GFxTagLoaders.cpp` — the base id gets `| 0x90000`
  * `External/GFx3/GFxCharacterDefs.h` / `.cpp` — `BuildPixelToUVMatrix`
  * `External/GFx3/GFxDisplay.h` / `.cpp` — both call sites of it, `MaskDrawDepth`, the empty-mask counter
  * `GFxUI/Src/gfxuiengine.cpp` — the empty-mask count on the census line
* **Sequencing**: nothing in this package touches a file any other package in this wave owns. Agent EK's
  HUD is at HEAD already and this package is what makes it visible; the two compose with no edit to either.

## 8. Hand-overs

1. **The brief's cause 2 is closed, and the way it was reached is worth keeping.** A bypass screenshot
   says what a frame looks like without a subsystem; it does not say the subsystem was wrong. The cheap
   test that settles it is the *converse* — leave the suspected subsystem exactly as it is and fix the
   other thing — and it cost one incremental relink here.
2. **Three of the eight atlases are unexercised** (section 6). A frame that draws from `HUD_I240`
   (1024x256, the only non-square sheet) is the one that would catch a width/height transposition in
   `BuildPixelToUVMatrix`, and none of this wave's frames does.
3. **A retail HUD screenshot is still missing and is now nearly free** (section 6). `agentEM_retailplay.py`
   plus one `hoverclick` position finishes it, and it would turn every placement claim in agent EK's
   document and this one from arithmetic into a comparison.
4. **`GFxImageCharacterDef` still resolves lazily and by name.** Retail binds at load time through
   `GFxSubImageResourceCreator::CreateResource` (2013 `0xa19650`), which builds a real `GFxSubImageResource`
   over a `GSubImageInfo` (`0xa18ff0`). The reconstruction keeps the rectangle on the character def
   instead. That is agent DC's stated deviation and this package does not change it — but everything the
   resource path would carry is now present on the def, so replacing it is a contained job.
5. **`resources/reference/` should gain in-game frames.** Four menu images is what the whole wave has to
   check interface work against.

## 9. Files

Mine (6 source, 2 documents): `External/GFx3/{GFxTagLoaders.cpp, GFxCharacterDefs.h, GFxCharacterDefs.cpp,
GFxDisplay.h, GFxDisplay.cpp}`; `GFxUI/Src/gfxuiengine.cpp`;
`resources/docs/agents/{agentEM.md, agentEM_status.csv}`.

Scratch, not repo tools: `build/agentEM_build.cmd`, `agentEM_run.py`, `agentEM_sync.py`,
`agentEM_isolate.py` (the four-leg switch of section 3), `agentEM_retailshot.py` and
`agentEM_retailplay.py` (the retail capture); `build/agentEM/` — `gfxtags.py` (the tag-1008/1009 payload
reader), `dec.py` and `resolve.py` (the headless IDA helpers), `dec2013/` (the retail decompiles),
`bmp2png.py`, the run logs and frames `rhead`, `r1`, `r2`, `r3`, `menu_head`, `menu_em`, `vial_r1`,
`vial_r2`, `cmp_*_tl`, `retail_tower`, `retail_tower2`, `retail_play/`, `retail_contact`, `final`, `vial_final`, the build logs
`b1`..`b4`, `bhead`, `clean1`..`clean3`, and `regression1`..`regression8` plus
`regression_head_inputtest`. Build dirs `build/agentEM_rel`, `agentEM_headrel` (from the `git archive`
export `agentEM_headsrc`), `agentEM_clean`, `agentEM_wt/build/agentEM_reg`. IDA: own copy only,
`build/agentEM_ida/retail2013_agentEM.i64`, opened headlessly through `resources/tools/ida/run.py`.

No commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`, nothing written
into the main checkout. The five gitignored layout inputs were copied into the worktree so the harness's
layout stage could run; they are generated files and are not part of the package.
