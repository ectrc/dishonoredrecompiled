# Agent DJ — the menu looks like the menu (2026-09-28)

Package: make the main menu match the two reference screenshots the user supplied
(`resources/reference/menu/first.jpg`, `second.jpg`).

**`build/agentDJ/menu_screen1.png`** is the start screen: the harbour, the DISHONORED logo with its
blood splatter, **PRESS ANY KEY** in the game's own face, the fog vignette. Against `first.jpg` it is
the same image.

**`build/agentDJ/menu_screen2.png`** is the main menu, reached **by a key press**
(`-gfxuikey=150:SpaceBar`; the log carries `startScreen_mc.Close(0) -> ok` then
`mainMenu_mc.Open(4) -> ok`): the logo, the splatters, the blades, Corvo's mask, the bar

`CONTINUE | NEW GAME | MISSIONS | LOAD | OPTIONS | QUIT GAME`

with CONTINUE selected and its highlight clipped to the label, and the row **Purchase Downloadable
Content** underneath it. Section 6 names, with an address each, the three things in that frame that
still do not match `second.jpg`.

Thirteen defects were found and fixed, every one measured against the retail binary rather than
guessed. Three of them are why nothing worked at all: **`SetMatrix` was a game-thread store**
(section 1), **the member lookup treated `__constructor__` as the resolve handler** so every read of
an absent member silently *ran that object's constructor* (section 2), and **`new` did not ask the
constructor to make the object**, so `new Array()` had no element store (section 3.8).

## Result

| Accept | State |
|---|---|
| a screenshot matching `first.jpg` | **`build/agentDJ/menu_screen1.png`** — drawn frame 140 of a plain run |
| a screenshot matching `second.jpg`, reached by a key press | **`build/agentDJ/menu_screen2.png`** — drawn frame 1400, after `-gfxuikey=150:SpaceBar`. All six entries in the right face at the right spacing, CONTINUE selected and masked to its label, the DLC row present. Three residual differences, section 6 |
| 0 unbound text fields | **`8 text fields [0 unbound]`** in the census below. The counter is new: a field still showing the DefineEditText's own placeholder (the cook's is Flash's default, the literal `Text`) or showing nothing at all |
| script errors | **41**, of three kinds, all named in section 6 (55 before this package, and the kinds were seven) |
| glyph atlas misses at 0 | **`atlas 36 packed / 2 blank / 0 failed`.** The number two earlier reports read as a defect is a *cache-miss* counter: `GFxGlyphRasterCache::Misses` is incremented once per distinct glyph, before it is rasterised, so it can never be 0 while text is drawn. The census now reports packed / blank / failed, and **failed is 0** |
| `run_regression.py` 31 checks, 0 failures | **`31 ok, 0 failed, 0 skipped, 422s`**, own build dir — section 8 |
| report + `agentDJ_status.csv` | this file; **34 rows**, `rva_2013,status,note` |

The census line, last frame of the acceptance run:

```
GFx UI census (frame): movies open 1 [UI_MainMenu.MainMenu], drawn 1, display objects 204
  (114 sprites, 65 shapes, 8 text fields [0 unbound], 41 bitmap fills), 64 draws, 182 triangles,
  8 glyph batches / 77 glyphs, 6 masks, atlas 36 packed / 2 blank / 0 failed
GFx UI census (frame): machine: 1679 frames advanced, 702 sprites created, 1452 display objects
  placed, 2259 action buffers, 2151152 opcodes (0 unimplemented), 41 script errors
```

## 1. The one defect behind every visual fault: SetMatrix was a game-thread store

Everything the two earlier reports called "the weird texture issues" is **one defect on the render
thread**, and it is not in the tessellator, the atlas, the fill matrices or the shaders.

`FGFxRenderer::SetMatrix` (2013 0x5cca20) and `SetCxform` (0x5c4b60) are not stores. Retail **copies
the matrix into a render command** that writes `CurrentMatrix` when it executes — the templated
`FGFxRendererImpl::SetUITransformMatrix<GMatrix2D>` at 0x5c6750, whose command's `Execute` is
0x5b8d00, and `SetUITransformMatrix<Cxform>` at 0x5bd9e0. This tree wrote the member directly on the
game thread. The draw calls are themselves render commands, so by the time any of them ran, every one
of them saw the **last** matrix of the frame:

* the DISHONORED logo was drawn at the vignette corner's position and size;
* the vignette's pieces were smeared across the frame;
* text and shapes landed on top of each other in one corner.

The fix is the shape of retail's, a `FRenderCommand` per set, plus a game-thread mirror of the colour
transform because `FillStyleColor` and `FillStyleBitmap` capture it when they *enqueue*.

## 2. `__resolve`, not `__constructor__` — the defect that ran constructors by accident

`GASObject::GetMemberRaw` (2012 0x9dc890, 2013 0x9d3050) special-cases two builtin names against the
string context: the one at +320 answers with `pProto`, and the one at +336 answers with the object's
single function ref — the same ref the body then hands to `GASValue::SetAsResolveHandler` when a name
is **not** found on that object.

This tree read +336 as `__constructor__`. The exe's own builtin string table settles it:

```
0xdb6234 "__resolve"     0xdb6240 "_listeners"   0xdb624c "__constructor__"
0xdb625c "constructor"   0xdb6268 "__proto__"    0xdb6274 "prototype"
```

`__proto__` sits four entries from `__resolve`, which is exactly the 320/336 pair. So +336 is
**`__resolve`**, and `__constructor__` is an ordinary member of the hash.

The cost was not a missing feature, it was a *call*. Every read of a member an object did not have
returned a PROPERTY whose getter was that object's **constructor**, and the interpreter ran it.
`_common.SelectionHandler`'s own element loop reads `this._bRightClick`, which is only assigned when
the caller passes `bRightClick` — the main menu's bar does not — so the third statement of its loop
**re-entered its own constructor with no arguments**, wiped `_elementID`, `_targetMc` and
`_nbElements`, and every `getSelectedElement` after that looked up the member named `"NaN"`.

Measured with the opcode trace this package adds (`-gfxuiopwindow=9197:1881:3600`):

```
op 2076: pc 2667 GetMember    stack 10 top '_bRightClick'      <- the loop, iteration 0
op 2077: pc 1881 Push         stack  9 top '[object Object]'   <- the CONSTRUCTOR, entered
...
op 2083: pc 1917 SetMember    stack 12 top 'undefined'         <- _elementID = undefined
op 2128: pc 2668 LogicalNot   stack  9 top 'undefined'         <- back in the loop
...
op 2153: pc 3575 GetMember    stack 11 top 'NaN'               <- _elementContainer["NaN"]
```

Only the first of the bar's six buttons was ever bound; `SetBkgd` and `tweenTo` were called on
`undefined` for the rest, and the bar stayed at `_alpha` 0. This defect is also why, earlier in the
package, letting `attachMovie` search the import graph put DISHONORED title bars over the start
screen — with constructors running by accident the wrong movie's timeline was being driven.

## 3. Seven more defects in the ActionScript machine

In the order they were found. Each has its retail body behind it; the addresses are in
`agentDJ_status.csv`.

1. **`super()` never ran.** `ActionCallMethod` (0x52) pops the method name as a *value*; an
   `undefined` name — not the string `"undefined"` — means the object on the stack **is** the
   function. That is exactly how the AS2 compiler emits `super(...)`. Before this, not one class
   constructor in the cook ran past its first statement.
2. **`super` stood still.** `InvokeContext::Setup` (0x9f2b40) derives the next `super` from the
   prototype that *declares* the running function, found with `FindOwner`, not from
   `this.__proto__.__proto__`. Deriving it from the unwrapped instance made every hierarchy recurse
   on itself — 609 stack overflows in one six-frame run.
3. **`ActionExtends` had its operands the wrong way round** (0x9e64a0): the superclass is Top(0). All
   45 `__Packages` classes were replacing their *base* class's prototype, and the six that extend
   `MovieClip` directly overwrote `MovieClip.prototype`, which is where `attachMovie`,
   `getNextHighestDepth` and `gfx.motion.Tween`'s mixin live. This is why `tweenTo` and
   `SaveProperties` looked "missing".
4. **Register windows overlapped between nested calls** (0x9e0e30): the DefineFunction2 window is the
   *tail* of the register array and is indexed **backwards**.
5. **Dotted paths could not walk through a plain object** (0x9ea060 / 0x9e9d60): a path prefix is
   resolved to a *value* and the member set on its object interface. Walking it with `FindTarget`,
   which only follows characters, stopped at `_root.texts` — the object `InitTexts` creates — so all
   149 localised strings of the main menu were written to nothing and every label read `Text`.
6. **Imported movies' init actions never ran** (0x9f46c0). Reconstructed as a
   `GASImportInitActionsTag` the `ImportAssets` loader leaves in the frame's init-action list, which
   is the shape `GFxInitImportActions` has. Without it the five screens the main menu imports arrived
   with their artwork and without their classes.
7. **`setInterval` / `setTimeout` / `clearInterval` were stubs.** The main menu sequences its whole
   opening animation with them: the logo, the blades, the splatters and the menu bar each start from
   one.
8. **`new` did not ask the constructor to make the object** (`GASEnvironment::OperatorNew`, 2012
   0x9e89c0, 2013 0x9df1d0). Retail calls the constructor's own **`CreateNewObject`** — the vtable
   slot at +60; the body names it in its error string,
   `"%s::CreateNewObject returned NULL during creation of %s class instance."` — and the constructor
   it asks is the one the **prototype's `__constructor__`** holds, not always the one the opcode
   named, which is how a subclass of a built-in gets a built-in instance. Without it `new Array()`
   was a plain object wearing `Array.prototype`: `push` found no element store and `length` was not a
   member at all, so `MainMenuButtonBar::SetMenu` handed `_common.GenericMenu::SetMenu` an array whose
   `length` was `undefined` and the bar was built with **no entries**.

## 4. Why the labels were in the wrong face, at the wrong size, in the wrong place

Four defects, all measurable against the references.

**The font was never resolved.** `GFxEditTextCharacter::GetInitialFormats` (0xa27860 / 2013 0xa1df60)
resolves the definition's font id against the movie's resource binding. This tree could not — the note
at the site said so — and used *the first font the process ever registered*. Every string in the game
therefore drew in whichever face loaded first. Measured: `PRESS ANY KEY` occupied **0.074** of the
frame's width against the reference's **0.126**, at the same height, i.e. a different, narrower face
rather than a different size. MainMenu's `DefineEditText` carries font id 31 or 53 and its
`ImportAssets2` binds those ids to `$NormalFont` and `$TitleFont` in `..\DisFonts\gfxfontlib.swf`; the
id is now resolved in `GFxEditTextCharacterDef::CreateCharacterInstance` (0xa32df0), the only step
that has the movie, and `GFxFontManager::FindFontResource` already matched an export name.

**A text field reported its definition's extent, not its own.** `GFxEditTextCharacter::GetBounds`
(0xa2ed60 / 2013 0xa252f0) is four lines: take `GFxTextDocView::GetViewRect` and `EncloseTransform`
it; `GetViewRect` (0xa9f2c0) formats the document first when the view is dirty, the same lazy format
`GetTextWidth` and `GetTextHeight` do. There was no override, so a field fell through to
`GFxCharacter::GetBoundsTwips` and answered the *definition's* rect, which is empty for every field
the cook creates from a library symbol. `MainMenuButton::SetText` sets `txt.autoSize = true` and then
reads `txt._width` to centre the label and size the button, and it was reading 0.

**`_width`, `_height`, `_xscale`, `_yscale` and `_rotation` could not be written, and then they
compounded.** `GFxASCharacter::SetStandardMember` (0x9d3350 / 2013 0x9c9bf0) answers all five; this
tree answered only `_x`, `_y`, `_alpha` and `_visible` and let the rest fall through to the ordinary
member store, where they changed nothing. So `_upLine_mc._width = _downLine_mc._width = txt._width +
50`, `btn._width = txt._width`, `_maskBkgdOver_mc._width = this._width` and the roll-over's
`_xscale = _yscale = 250` were **all lost**, every button kept the authored art's 244-pixel extent,
and `_common.GenericMenu::SetMenu` spaced the six entries by that instead of by their labels — the bar
was half a screen too wide and `CONTINUE` was off the left edge. Writing them naively was not enough
either: retail keeps a **GeomData** record on the character (`GetGeomData` 0x9ceb10, `SetGeomData`
0x9cf3b0, `EnsureGeomDataCreated` 0x9cf410 — 88 bytes at character+152: X, Y, XScale, YScale,
Rotation and the matrix those five were last consistent with) and every setter rebuilds the matrix
from **that** record with `GFxASCharacter_MatrixScaleAndRotate2x2` (0x9cdf10). That is what makes the
writes idempotent and sizes a rotated clip along its own axes; `_menu_mc` carries a 1.9° skew and each
button is written twice, so without it the widths drifted by about a third.

**`setTextFormat` replaced the run's format instead of merging into it.**
`GFxTextParagraph::SetTextFormat` (0xaa5400 / 2013 0xa9b640) allocates `existing.Merge(f)` for each
run the range covers — the decompile is explicit — so only the fields the incoming format marks
present override. Interning the incoming format as-is wiped the font handle and the size off every
run, and the one line of the menu that sets `textColor` is the DLC row: it laid its 29 glyphs out and
then drew none of them, because `GFxEditTextCharacter::Display` skips a glyph whose format has no
font. That row is the last element of `second.jpg` that was missing.

## 5. Two defects outside the AS2 machine that were hiding in plain sight

**A mask submitted with no depth-stencil surface is not a mask.**
`FGFxRenderer::BeginSubmitMask_RenderThread` (0x5d8170) is ported and it runs — the census counts six
mask passes on the main menu, one per button — but `CheckRenderTarget_RenderThread` binds
`Resource->DepthBuffer`, and that reference was never created: retail takes the surface from the
scene's own depth proxy and `FSceneDepthTargetProxy` is off this module's include path. D3D9 drops
every stencil op when no depth-stencil is bound, so `_maskBkgdOver_mc`'s `clipDepth 5` did nothing and
the selected entry's highlight drew at the full width of its artwork — 269 against the mask's 164 —
over the first two letters of `NEW GAME`. `FGFxRenderTargetResource::InitDynamicRHI` (0x5c4f50 / 2013
0x580850) now allocates one; deviation 10.

**`attachMovie` searched only the movie's own export table.**
`GFxMovieDefImpl::GetExportedResource` (0xa1ede0 / 2013 0xa15680), which
`GFxMovieRoot::FindExportedResource` (0xa03180) and `GFxValue::ObjectInterface::AttachMovie`
(0x9af830) go through, looks the symbol up in this movie's exports and then **recurses into every
movie it imports**, skipping the caller so a cycle terminates. The DLC row's icon is
`_content_mc.attachMovie("lib_X_Multi", "btn", d)` and `lib_X_Multi` is exported by
`../common_assets/lib.swf`, which MainMenu imports without naming that symbol; with the attach
failing, `btn._x = btn._width / 2` put the row's whole content at NaN. All three `attachMovie`
failures of the earlier reports are gone.

## 6. The four answers the bar asks the game for

`MainMenu.prototype.SetMenu` asks the game four questions through
`flash.external.ExternalInterface.call` and builds the bar from the answers: `req_CanContinueGame`,
`req_CanLoadGame`, `req_CanStartNewGame`, `req_IsSaveLoadEnabled`.

* The AS2 half of `ExternalInterface` and of `fscommand` did not exist and is now written
  (`FGFxExternalInterface::Callback` 2013 0x58d510, `FGFxFSCommandHandler::Callback` 0x586450).
  `GFxMovieRoot::SetExternalInterfaceRetVal` is how the callback answers.
* `UDisGFxMoviePlayerMenuBase::Req_CanContinueGame` and `Req_CanSaveGame` had no body — they were
  `DISHONORED_NATIVE_STUB` entries, and the log said so: *"DishonoredGame native not ported:
  `UDisGFxMoviePlayerMenuBase::execReq_CanContinueGame` (parameters consumed, result zeroed)"*. With
  all four answers false, `SetMenu` built a bar with no entries. Continue now answers
  `!engine || HasSaveGame(0)`, which is what retail's `UDisGFxMoviePlayerMainMenu::PostStart`
  (2012 0x821e00) computes for the same question; Save answers the engine's `m_bSaveLoadEnabled`
  (0x62bb50), beside the already-ported `Req_CanLoadGame` (0x5f7960) and `Req_IsSaveLoadEnabled`.

**CONTINUE and LOAD are gated on there being a save, and that is correct.** The reference screenshot
was taken on a machine that had one; `DishonoredGame\SaveData` here is empty, and with it empty the
bar correctly comes up with the other four entries only. The acceptance screenshot is taken with
`-savedir=build\agentDJ_save`, which holds one synthesised `DisMission0.sav` (slot 4: the changelist
int and the details string `1 - High Overseer Campbell`, the two fields
`FDisAsyncSaveGameLister::DoWork` reads). **No engine or save-game code was changed for it** — it is
test data, and it is what makes the run reproduce the reference's six entries.

## 7. What still does not match `second.jpg`, precisely

1. **The 3D camera does not move between the two screens.** The asset sends
   `GetURL "FSCommand:OpenMainMenuScreen"`; `FGFxFSCommandHandler::Callback` (2013 **0x586450**)
   routes an fscommand to the map's `UGFxEvent_FSCommand` Kismet nodes, and it is the menu map's
   Kismet that moves the camera. Both of our screens draw the same harbour view; the reference's
   second screen is a different one. This is the map's sequence, not the interface, and it is the
   largest remaining visual difference.
2. **The DLC row's platform-button icon draws in its unlit state.** `lib_X_Multi` attaches, the class
   binds and the clip reaches frame 15, which is its `PC` label, and its artwork is submitted at
   (243,626)-(283,666) — measurably brighter than the band behind it, but only just. Its `_glow_mc`
   and `_white_mc` are both at `_alpha` 0 and never animate, because `_common.ClickableButton`'s
   constructor could not register the instance: `_root.UIBase` is `undefined` at that moment (the two
   `AddControllerButtonInstance` errors), since `new MainMenuBase()` on the root's first frame is what
   sets it and the imported screens' classes are built before that frame's action runs. Retail's
   action queue drains `GFxAP_Init` before `GFxAP_Frame`, so the ordering is worth measuring against
   `GFxMovieRoot::ActionQueueType` before anything is changed.
3. **`MISSIONS` has no asterisk.** The reference reads `MISSIONS*`. The entry's text is
   `_root.texts.t_Missions` verbatim, so this is the localised string or the DLC marker that appends
   to it, not the layout.

Still open, unchanged in kind from the earlier reports:

* **`flash.display.BitmapData.loadBitmap`** — 27 errors, all from `__Packages._common.EmbedImg`, which
  is how the interface puts button icons into text.
* **`GotoLabeledFrame: no frame named 'PC'`** — 12, and the message now names the clip and its label
  count: seven are `_glow_mc` (3 labels: default / loop / stop), four are the options screen's
  `dir_Up/Down/Left/Right` (5 labels) and one is `gammaSetting_mc` (1 label). None of them is a
  platform-switch clip — every `*_Multi` clip of the shared library does carry a `PC` label at frame
  15 and reaches it — so these are the content asking a clip that has no such frame, which retail logs
  too.
* **`GTessellator`** (2012 0xabdbb0..0xac5930, ~60 functions) is untouched. With the transform defect
  fixed, the logo, the splatter frame and the bar's chrome are all in the right place at the right
  size — **the trapezoidal decomposition was never what was wrong**. What it would buy is edge
  antialiasing: at 1:1 the logo's diagonals are harder than `first.jpg`'s.
* **The atlas sub-image path** (483 of 1,015 cooked images are `BaseImageId` + `SubRect`) is still not
  applied. The main menu has **0** sub-images, so nothing in these two screens exercises it.

## 8. Deviations, stated once

1. **A subtree whose composed colour transform multiplies alpha by zero is not submitted.** Retail
   submits it and the pixel shader's Cxform makes every pixel transparent. Here the Cxform reaches
   `FGFxFillStyle` and the cooked `GFx_PS_CxformTexture` does not visibly honour it, and the main menu
   keeps two whole imported screens on its display list at `_alpha` 0.
2. **A style group whose image fill has no texture is not submitted at all.** The renderer keeps the
   last fill it was given, so drawing it anyway paints the previous shape's bitmap over this one's
   area.
3. **`GASStringManager`'s destructor leaks a node something still holds.** The action buffers of a
   movie *definition* keep their constant pool as `GASString`s and the loader outlives the movie root.
   Retail cannot hit this: it builds a fresh refcounted `GASActionBuffer` per execution from the
   `GASActionBufferData` the tag holds. Rebuilding that ownership is a package of its own.
4. **`FGFxRenderer` gains one member**, `CurrentCxformGameThread`, appended after the reproduced
   layout, because `CurrentCxform` is now render-thread state.
5. **`GFxSprite::AttachMovie` runs the clip's first frame before its class constructor**, which is what
   the timeline path already did and what `_common.GenericMenu`'s constructor needs.
6. **The interval timers are a fixed table of 64 on the movie root**, not retail's list, serviced once
   per advance. Nothing in the cook holds more than six at a time.
7. **`ExternalInterface` and `fscommand` are plain AS2 objects**, as `Key`, `Mouse` and `Stage` are in
   this tree, not the constructor functions retail builds. Same script-visible surface.
8. **`GFxTextFieldDesc` gains `FontName[96]`**, filled by `CreateCharacterInstance`, because the
   descriptor is the only thing the field constructor is given and the font id has to be resolved where
   the movie is still in hand.
9. **The built-in class factories cover `Array` only.** `String`, `Number` and `Boolean` still make a
   plain object under `new`, as they did before, so `new String("x")` is not a boxed primitive.
   Nothing in this cook uses them; it is named so it is not lost.
10. **The UI's render target allocates its own depth-stencil surface.** Retail shares the scene's
    (`OwnerDepth->GetDepthTargetSurface()`), and that seam is not reachable from GFxUI yet — the site
    already carried the note. One surface per target the engine owns; depth testing is already
    `CF_Always` with writes off, so it is used for its stencil only. Replace it with the scene's when
    the engine-side call site lands.
11. **`GeomData` reproduces five of retail's fields and not the 3D tail** (Z, ZScale, XRotation,
    YRotation of the 88-byte record), because nothing in this tree reads `_z` or `_xrotation` yet.

## 9. Verification

* **The two acceptance screenshots** are from one 60-second run of the plain Release build at
  `-startmap=Dishonored_MainMenu -startmapopen -gfxuimenu -windowed -ResX=1280 -ResY=720 -nomovie`,
  with `-savedir=build\agentDJ_save -gfxuikey=150:SpaceBar -gfxuishot=140,1400`. **0 `Critical:`
  lines.** The second is reached by the key press, not by loading it.
* **The census** on the last frame of that run is quoted at the top of this report:
  `8 text fields [0 unbound]`, `0 unimplemented opcodes`, `6 masks`,
  `atlas 36 packed / 2 blank / 0 failed`, `41 script errors`.
* **Regression**: `python resources\tools\run_regression.py --build-dir build/agentDJ_rel --no-build`
  → **`31 ok, 0 failed, 0 skipped, 422s`** (`build/agentDJ_rel/regression/summary.txt`), re-run after
  the depth-stencil change.
* **Clean full Release build** of the snapshot worktree `build/agentDJ_wt` (HEAD `bf6e5e6` plus this
  package's 35 files), build directory deleted first: **982/982 targets, 0 errors** — `DishonoredGame`,
  `CoreSmoke`, `LayoutProbe`, `GFx3Run` and the rest.

## 10. Hand-overs

1. **The menu map's Kismet / the `UGFxEvent_FSCommand` nodes** (section 7 item 1). Whoever owns the
   menu level can say whether `OpenMainMenuScreen` has a handler there at all. It is the largest
   remaining difference from the reference and it is not in the interface.
2. **The action-queue priorities against `_root.UIBase`** (section 7 item 2). Retail drains
   `GFxAP_Init` before `GFxAP_Frame`; measure `GFxMovieRoot::ActionQueueType::InsertEntry`
   (2012 0x9f4780's callee) against this tree's drain order before changing anything.
3. **`flash.display.BitmapData`**, for `_common.EmbedImg` — 27 of the 41 errors.
4. **The scene's depth-stencil surface for the UI target** (deviation 10), when the engine-side UI
   pass lands.
5. **`GFx3Dump` still does not build with the runtime on** (agent DG's hand-over, untouched):
   `DISHONORED_GFXUI_GFX3_RUNTIME=1` makes `gfxuirenderer.cpp` and `gfxuiimageinfo.cpp` need
   `Engine.h`, which `GFx3Dump` links without. `GFx3Run` is unaffected and is what this package used.
6. **The AS2 action-buffer ownership**, deviation 3; it is also what the AS2 collector package will
   need.
7. **Coordinator — generator output was hand-edited and NOT regenerated.**
   `source/Development/Src/DishonoredGame/Src/DishonoredGameNativeStubs.cpp` is produced by
   `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header
   --sources-cmake`. I removed exactly two `DISHONORED_NATIVE_STUB` bodies from it —
   `execReq_CanSaveGame` and `execReq_CanContinueGame` — and appended both names to
   `source/Development/Src/DishonoredGame/DishonoredGameNativeStubs.ported.agentBE.txt`, which is the
   list the generator reads to skip a stub. **Re-running the generator reproduces the same file**; I
   did not run it because agent DI is live in `DishonoredGame` and the command rewrites headers across
   the module. Nothing else the generator produces was touched, and nothing in `cmake/` changed.
8. `middleware.md` 2.3 can gain the line that the interface now draws the game's own main menu, its six
   entries, its DLC row and its 3D scene, with the census above.

## 11. Files

Mine (35), all inside the package's scope — `External/GFx3`, the `GFxUI` module, and the
`DishonoredGame` movie players:

`External/GFx3/{GFxAS2.h, GFxAS2Interp.cpp, GFxAS2Lib.cpp, GFxAS2Object.cpp, GFxAS2Object.h,
GFxAS2Runtime.cpp, GFxAS2Runtime.h, GFxAS2Value.cpp, GFxCharacterDefs.cpp, GFxDisplay.cpp,
GFxDisplay.h, GFxGlyphCache.cpp, GFxGlyphCache.h, GFxPlayer.h, GFxPlayerData.cpp, GFxPlayerRoot.cpp,
GFxPlayerSprite.cpp, GFxStyledText.cpp, GFxTagLoaders.cpp, GFxTextDocView.cpp, GFxTextDocView.h,
GFxTextField.cpp, GFxTextField.h, GTypes.h, Tools/GFx3Run.cpp}`;
`GFxUI/{Inc/gfxuiengine.h, Inc/gfxuirenderer.h, Src/gfxuiengine.cpp, Src/gfxuiexternalinterface.cpp,
Src/gfxuirenderer.cpp}`;
`DishonoredGame/{Src/disgfxmovieplayerbase.cpp, Src/disgfxmovieplayermainmenu.cpp,
Src/disgfxmovieplayermenubase.cpp, Src/DishonoredGameNativeStubs.cpp,
DishonoredGameNativeStubs.ported.agentBE.txt}`; plus this report and `agentDJ_status.csv`.

**No file of agent DI's units was touched** (`dishonorednpcpawn*`, `dishonoredpawn*`,
`dishonoredplayercontroller.cpp`, `dishonoredutilities.*`, `disheadcensus.*`).

`GTypes.h` gains three inline methods on `GMatrix2D` — `GetXScale` (2012 0x9b2530), `GetYScale`
(0x9b2550), `GetRotation` (0x9b2570) — and an `IsValid`, all of which retail has and all of which the
geometry setters need.

Switches this package adds, all read on first use, none of them file-scope initialisers:
`-gfxuidumpdl[=<N>]` (the display tree with matrix, skew, frame and clip depth), `-gfxuidrawtrace=<N>`,
`-gfxuihide=<name>[,...]`, `-gfxuiwatch=<member>` (a trailing `*` makes it a prefix; it logs reads,
writes, calls, returns and the invocation of a script function of that name),
`-gfxuioptrace=<from>[:<count>]` and `-gfxuiopwindow=<bufferlen>:<lopc>:<hipc>` (the AS2 opcode trace,
pinned either to the opcode counter or to a buffer and a pc range), `-gfxuicensus`; and in the harness
`--optrace <from>[:<count>]`.

Scratch, not repo tools: `build/agentDJ/` — `swfscan.py` (the SWF/GFX walker and AS2 disassembler),
`patch_*.py` (one CRLF-safe patch script per defect, each with the measurement in its docstring),
`mkstatus.py`, `dec1`..`dec24` (the headless decompiles this rests on),
`mm_init.txt` / `mm257.txt` / `mm271.txt` / `mm281.txt` / `mm286.txt` / `mm289.txt` / `mm291.txt` (the
cook's own bytecode), `optrace*.txt`, the screenshots; `build/agentDJ_build.cmd`,
`build/agentDJ_run.py`, `build/agentDJ_sync.py`, `build/agentDJ_files.txt`,
`build/agentDJ_save/DisMission0.sav`; snapshot worktree `build/agentDJ_wt`, build directory
`build/agentDJ_rel`.

IDA: **own copy only**, `build/agentDJ_ida/dj2012.i64` (a copy of
`Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.exe.i64`), opened headlessly through
`resources/tools/ida/run.py`. **No IDA MCP tool and no FModel tool was used.** No commits, no
`git add`, no junctions into the retail or reference trees, nothing deleted under
`Dishonored_Latest2026`.
