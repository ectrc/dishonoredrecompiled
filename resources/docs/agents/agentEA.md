# Agent EA — the New Game flow: the bars are back, and the modal is built (2026-09-29)

The user, of the screens after the main menu:

> *"when clicking the new game mode there is no background on each of the buttons (there should be a
> transparent bar that turn white on hover) and then normal hard and very hard, the image starts to
> overlap and then when clicking on a difficulty the brightness select screen will just show
> "undefined" and the slider in unresponsive. And then when continuing on that screen it doesnt show
> the modal to start the game, it just hides the last menu."*

| | |
|---|---|
| Package | the ActionScript built-in surface the New Game flow calls, and the four faults behind it |
| Starts at | HEAD `c598a21` |
| Files | 13 (one new), in `External/GFx3`, the `GFxUI` module and the `DishonoredGame` movie players |
| Regression | **`31 ok, 0 failed, 0 skipped, 431s`** on the clean gate build `build/agentEA_wtrel` |
| Clean build | the snapshot worktree `build/agentEA_wt` (HEAD + this package), build directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: **994 edges, 0 errors** |

**`build/agentEA/gate_hoverstrip.png`** is the first fault in one picture: four consecutive hover
states of the New Game screen on the clean gate build, driven by a real mouse —
`EASY -> NORMAL -> HARD -> VERY HARD` — each with the transparent bar on all four entries, the white
bar under the pointer, and the game's own cursor sitting on it. **`build/agentEA/newgame_before.png`**
is the same screen at HEAD.

## 0. The brief's causal table was a guess, and four of its five rows are wrong

The brief mapped the user's four faults onto an error census. That census is real — this package
reproduced it to the call — but **the mapping from the errors to the faults was not measured, and it
does not survive measurement**:

| the brief said | measured |
|---|---|
| 54 `loadBitmap` = the overlapping difficulty art | `_common.EmbedImg`'s platform-key glyphs for inline `<img>` runs in help text. Nothing to do with the difficulty art. The receiver is **`undefined`**: `flash.display.BitmapData` does not exist, so the class is undefined before `loadBitmap` is even looked up |
| 12 `lineTo` = the missing bar backgrounds | `_common.HelpBar`'s bitmap strip and `_common.ItemsList::SetMouseArea`'s **invisible** 100x100 hit rectangle (`_mouseArea._visible = false`). Neither is a mode button's background |
| 2 `getTextFormat` / 2 `getTextExtent` = the `undefined` label | `_common.TitleBar` measuring its own label. The `undefined` is a different error entirely — `toUpperCase` **on undefined**, which is `_widgetSettings.Setting_Name` |
| 4 `gotoAndPlay` = the modal that never plays in | no `gotoAndPlay` error occurs on the user's path at all. The modal is `ExternalInterface.call('ShowMessageBox', …)`, which reaches a C++ stub |
| 2 `removeMovieClip` = the old screen is hidden rather than removed | no `removeMovieClip` error occurs on the user's path; `Close()` setting `_visible = false` is what the asset does |

The one thing the brief got exactly right is that the built-in surface is missing and that porting it is
one package. It is ported, and it was **not** what caused three of the four faults. The actual causes
are §2, §3, §4 and §5, and each was found by driving the user's own path with real input and reading
what the game said.

## 1. Result

| Accept | State |
|---|---|
| the mode buttons have their transparent bar, and it turns white on hover | **done, on the clean gate build, with a real mouse.** `build/agentEA/gate_hoverstrip.png` and its four `gate_hover_*.png`; the census of that run is `14 hit tests / 8 targets resolved (33 shape walks, 8 button hits), 4 rollOver / 3 rollOut, 4 handlers invoked`, and the per-row brightness is in §2 |
| the difficulty art does not overlap | **not fixed, and named exactly**: the four entries and Corvo's portrait are coplanar because `_z` is not a display property in this tree. §3, with the retail functions |
| the brightness screen shows its real label, and the slider responds | **not fixed, and named exactly**: the label is `_widgetSettings.Setting_Name` and the settings tree is never pushed into the movie; the slider's range is unfilled for the same reason. `_xmouse`/`_ymouse`, which the slider's drag loop reads and which answered `undefined`, **are** ported. §4 |
| continuing shows the modal | **the modal is built, reached and carries the right text** — `_level0.msgBox_mc.messageBox_mc` with its background, its stroke, its button bar, its text field and its particles, from `ShowMessageBox (Creating a new game will overwrite previous autosaves. Continue?, YES, NO, )`. Its fade-in does not run, for the one residual defect of §6. §5 |
| a census: script errors by kind, before and after, the eleven built-ins at 0 | **§7.** Every one of the eleven names the brief lists is at **0 unresolved calls**. What is left on the path is one defect of one kind, and it is not a built-in: the content's own `gfx.motion.Tween` mixin, §6 |
| `run_regression.py` 31 checks, 0 failures, own build dir | §8 |
| report + `agentEA_status.csv` | this file; 60 rows |

Two switches were added, both read on first use: `-gfxuishotat=<sec>[,…]` and
`-gfxuidumpdlat=<sec>[,…]`, §9.

## 2. Fault 1 — the bars, and the one line that hid them

The bars are **authored art**, not drawn at run time. `_common.GenericMenu::SetMenu`
(`Dishonored_MainMenu.MainMenu` char 281, pc 806) attaches one `m_nGame_btn` per entry, and when
`_bMultiBkgdTypes` is set — `NewGameMenu` sets it — cycles the background clip's frame:

```
btn._bkgd_mc.gotoAndStop(reg5);  if (reg5 < btn._bkgd_mc._totalframes) ++reg5; else reg5 = 1;
```

The display tree of `_level0.newGame_mc._menu_mc._btnContainer_mc`, dumped in the running game at the
moment the screen was on screen, says the AS2 did its job:

```
btn0  _bkgd_mc frame 0/3  Shape 'instance1'  box (195,336)-(575,384)
btn1  _bkgd_mc frame 1/3  Shape ''           box (196,389)-(579,437)
btn2  _bkgd_mc frame 2/3  Shape ''           box (196,443)-(579,491)
btn3  _bkgd_mc frame 0/3  Shape 'instance1'  box (220,496)-(600,544)
```

Four clips, each on its own frame of a three-frame cycle, each with one shape of the right size. So
the bars were not missing — **they had no texture**. The census said so and named nothing: `25,541
untextured fills skipped` over one run. A diagnostic that names the picture instead of counting it
(§9) gave eleven distinct images, among them:

```
image id 4   'm_nGame_bkgdMenu'  the four New Game mode-button bars
image id 1,2,3 'n_menuLine0..2'  the main menu bar's own lines
image id 39,44,45 'PC_Shift', 'PC_Esc', 'PC_Enter'   the help bar's key glyphs
             'lib_help_bkgdExt'  the help bar's strip
```

and printing what the package actually holds, the moment a resolve failed, settled it:

```
GFx image 'UI_MainMenu.n_menuLine1' resolved to no texture; the textures of package 'UI_MainMenu' are:
  UI_MainMenu.n_menuLine2 -nopack
  UI_MainMenu.n_menuLine1 -nopack
  UI_MainMenu.n_menuLine0 -nopack
  UI_MainMenu.m_nGame_bkgdMenu -nopack
  UI_MainMenu.MainMenu_IEF
  ...
```

**The cooked texture is called `m_nGame_bkgdMenu -nopack`, space and suffix and all**, and
`GFxImageCharacterDef::GetResolveName` cut the name at its first space — "the exporter's suffix, then
the extension". Only the extension is the exporter's.

The second half of the same measurement is that the name to resolve is the **file** name, not the
export name. The two strings a `GFxDefineExternalImage` tag carries disagree, in both directions:

```
image id 4   export 'm_nGame_bkgdMenu -nopack'   file 'm_nGame_bkgdMenu -nopack.tga'
image id 39  export 'PC_Shift -nopack'           file 'lib_I27.tga'
```

and the package holds `UI_MainMenu.m_nGame_bkgdMenu -nopack` and `Common_assets.lib_I27`. The
exporter renames a movie's own packed images to `<Movie>_I<hex>` and leaves the artist's name on one
marked `-nopack`; the export name is the ExportAssets symbol `attachMovie` and `BitmapData.loadBitmap`
look a symbol up by, and it is not a texture. Preferring the export name lost the library's glyphs;
cutting at the space lost the movie's own `-nopack` images. Two lines, both measured both ways round,
and `untextured fills skipped` goes **25,541 -> 2,609** (§7).

**Measured, on the clean gate build, with a real mouse.** Four pointer positions over the four
entries, mean brightness of each bar's row in the captured frame (the bar spans x 150..440 of a
1008-wide frame), the pointer returning to a neutral corner between each so every move is an
unambiguous motion:

| pointer on | EASY | NORMAL | HARD | VERY HARD |
|---|---:|---:|---:|---:|
| EASY | **151.3** | 47.3 | 39.5 | 46.5 |
| NORMAL | 39.4 | **175.8** | 38.9 | 27.7 |
| HARD | 39.6 | 44.4 | **144.4** | 30.7 |
| VERY HARD | 39.9 | 44.7 | 39.5 | **110.0** |

Exactly one bar white at a time, following the pointer, the transparent bar on the other three, and
each row rising by a factor of three to four when the pointer reaches it. Before this package all four
rows were the background and the whole strip read 27..40.

## 3. Fault 2 — `_z`, and why the art overlaps

With the bars drawn, what the user described is visible in one picture
(`build/agentEA/newgame_hover_hard.png`): **VERY HARD is behind Corvo's portrait.** It is not the
timeline and it is not the display list — `_corvo_mc.portrait_mc` sits at `frame 3/4` with exactly one
shape on it, which is the right frame for the fourth difficulty and no accumulation at all. Every
`gotoAndStop` the screen makes lands where it should.

What is missing is **`_z`**. The New Game screen is authored in perspective and says so in four places:

```
NewGameScreen           _corvo_mc.props._z = -450        _bkgdTexture_mc.props._z = 150
NewGameMenu             _btnOverProps._z = -350          _btnOutProps._z = -20
NewGameMenu.OpenBkgd    _bkgd_mc._z = 550
NewGameMenu.onSelectionUpdated  _z = -10 - 10.25 * idx,  _xrotation, _yrotation per entry
```

`_z` and the two 3D rotations are display properties this tree does not have: agent DJ's deviation 11
records that `GeomData` reproduces five of retail's fields and not the 3D tail (Z, ZScale, XRotation,
YRotation), and agent DQ's deviation 3 records that every hit-test body is the 2D arm because
`GFxCharacter::Is3D`, `GetPerspective3D`, `GetView3D` and `GScreenToWorld` have no equivalent here. So
the portrait at `_z = -450` and the entries at `_z = -350` are drawn in the same plane, in authored
order, and the portrait — a later depth — covers the fourth entry. Writing `_z` without the projection
would move nothing; the projection is the package. Hand-over 1.

## 4. Fault 3 — `undefined` is a string the game never sent

The brightness screen (`build/agentEA/gamma_undefined.png`) reproduces exactly as the user described
it, and the error that produces the word is not a built-in:

```
AS2 error: call of a value that is not a function: 'toUpperCase' on undefined
```

`GammaSetting::FillGammaSetting` (`Startup.OptionsMenu` char 240, pc 686) is

```
_title_txt.text = _widgetSettings.Setting_Name.toUpperCase();
_desc_txt.text  = _root.texts.t_GammaHint;          // this one renders, and is correct
InitWidget(widgetSettings) -> _widget_mc.SetSlider(OnWidgetChange, this,
                                Setting_Increment, Setting_Maximum, Setting_Minimum, Setting_Value);
```

`_widgetSettings` comes from `OptionsScreen::DisplayGammaSetting`, which indexes
`_categoriesList[_gammaCategoryIdx].Setting_List[_gammaWidgetIdx]`. `_categoriesList` is what the game
pushes into the movie, `_gammaCategoryIdx` defaults to **-1** in the constructor and is only set by
`SetGammaIndex`, which `SetMenu` calls. **Nothing ever calls `SetMenu`**: agent BE ported the whole AS2
tree builder (`DisShowSettingsCategoryList`, `DisCreateGFxCategory`, `DisCreateGFxSubCategory`,
`DisCreateGFxSetting`, 2012 `0x817250` / `0x815a50` / `0x813db0` / `0x80f920`), and on the menu path
nothing drives it and `m_SettingsCategoryList` is empty. So the index stays -1, the widget is
`undefined`, and `undefined.toUpperCase()` produces the literal string the user saw.

The same measurement disposes of the slider's range: `DisCreateGFxSetting`'s own comment says
`Setting_Value`, `Setting_Minimum`, `Setting_Maximum` and `Setting_Increment` are **not** filled,
because they come from `UArkProfileSettings` and that class has no accessors in this tree. So even
with the tree pushed, `SetSlider` would be given four `undefined`s.

What this package did fix is the one part of the slider that is the ActionScript built-in surface.
`_common.P_Slider::StartDrag` (`Startup.OptionsMenu` char 231, pc 1307) does **not** call `startDrag`;
it sets `this.onEnterFrame` to a function whose whole body is

```
idx = this._parent.GetIndexFromPosition(this._parent._xmouse);
if (idx != this._parent._selectedValue) { this._parent._selectedValue = idx;
                                          this._parent.UpdateValue(false); }
```

and display properties **20 `_xmouse` and 21 `_ymouse` had no case at all** in
`GFxAS2GetDisplayProperty` — they fell through to `SetUndefined`, which AS2 coerces to 0, so the drag
loop computed the same index every frame however far the pointer moved. They are ported: the movie
root's pointer, in stage pixels, through the inverse of the character's world matrix, snapped to a
twip. `GFxCharacterWorldMatrix` is new beside the two matrix operations the display half already
exports, because the display walk composes the transform as it descends and the hit-test walk inverts
it level by level, so nothing needed the whole product in one place until this did.

So the slider needs neither `startDrag`/`stopDrag` nor the focus model nor `setMask`/`hitArea` — the
four things the brief named. It needs its numbers. Hand-over 2.

## 5. Fault 4 — the modal, and what it is made of

The content asks for the modal, and it asks correctly. From the AS2 trace on the user's path:

```
AS2 trace: InvokeMessageBox ([type Function], undefined,
           Creating a new game will overwrite previous autosaves. Continue?, YES, NO, ...)
```

`_common.MessageBoxInvoke::InvokeMessageBox` ends in
`flash.external.ExternalInterface.call('ShowMessageBox', msg, button0, button1, button2)`, which
reaches `UDisGFxMoviePlayerBase::execShowMessageBox` — and that body was
`m_MsgBoxID = 0;` and nothing else, because retail's `DisGetGlobalUIManager()->ShowMessageBox(Info)`
needs `UDisGlobalUIManager`, which is not declared in this tree. So the screen closed and no box
appeared: *"it just hides the last menu"*, exactly.

Retail's own body is one level further down and it is small.
**`UDisGFxMoviePlayerGlobal::ShowMessageBox(const FDisMsgBoxInfo&)`, 2013 `0x7946b0`** (2012
`0x7fa880`): unless the "a box is up" bit at `this+440 & 8` is set, build four `GFxValue` strings —
the message and the three button captions, each empty when the array is short — call
`pView->Invoke("ShowMessageBox", &result, args, 4)` on the **global** movie's view, then
`Invoke("SetMessageBoxTimer", …)` when the info carries a non-zero duration, and set the bit.

The asset agrees: `DishonoredGame.Global.gfx` carries a root-level
`ShowMessageBox(msg, button0, button1, button2)`, `__Packages._common.MessageBox`, and the box's art —
`gl_msgBox`, `gl_msgBox_bkgd -nopack`, `gl_msgBox_circleBtn`, `gl_msgBox_timer` — and no other movie
in the cook does.

Routed there, the box is built. Same run, after:

```
AS2 trace: > ShowMessageBox (Creating a new game will overwrite previous autosaves. Continue?, YES, NO, )
_level0.msgBox_mc.messageBox_mc          _vignette_mc  _bkgdWhite_mc  _bkgdBlack_mc
                  _msgBox_mc             boxBkgd_mc  boxShape_mc (strokeUp_mc, strokeDown_mc)
                                         _txt_mc  btnBar_mc
                  _particles_mc          particle1_mc  blade0_mc  blade1_mc  blade2_mc
```

and the screen dims to the box's own vignette (`build/agentEA/ea20_t05800004.png`). What does not
happen is the fade-in: every one of those fourteen clips issues one `tweenTo`, and every one of them
fails on `this.tween__start` **in that single burst**, so the box's contents never leave alpha 0. That
is §6 and it is the last thing between here and the modal being visible.

## 6. The one residual defect, measured to the call

`gfx.motion.Tween` is a CLIK mixin: the class's constructor copies six functions from
`Tween.prototype` onto `MovieClip.prototype` (`Dishonored_MainMenu.MainMenu` char 249, pc 446..186),
and `Tween._instance = new gfx.motion.Tween()` at pc 1699 is what runs it.

`-gfxuiwatch=tween__start` says precisely what goes wrong, and it is not what it looks like:

```
[0005.68] watch CALL 'tween__start' receiver type 7, interface 254BCEA8 (pc 973 of 1774)
[0005.68] AS2 error: call of a value that is not a function: 'tween__start' ... on Sprite _level0.vignette_mc._vignette_mc
[0005.68] watch RET  'tween__start' -> type 0
[0005.85] watch CALL 'tween__start' receiver type 7, interface 254BCEA8 (pc 973 of 1774)
[0005.85] watch INVOKE 'tween__start' buffer 2502B244 pc 1299 len 104 args 4 depth 2 op 77   <- resolves
[0005.85] watch RET  'tween__start' -> type 0
```

**The same clip. The first call has no INVOKE between its CALL and its RET; every later call has one.**
`tweenTo` resolves in the same call, from the same prototype, one instruction earlier, which is what
makes this worth a package of its own rather than a guess: the two names live on the same object and
one of them answers.

It has two shapes, and the second is the one that matters:

* **on the menu's own movie it is a one-frame burst.** Five at 5.68 s — `_level0.vignette_mc`,
  `vignette_mc._vignette_mc`, `startScreen_mc`, `startScreen_mc._txt_mc`, `startScreen_mc._logo_mc` —
  and then every call of the whole run resolves. Cosmetic: the start screen's first fade is lost.
* **on the global movie it never resolves at all.** Over one 140 s run that opens the modal,
  **971** `tween__start` failures, from 6.83 s to 137.87 s, on `_level0.msgBox_mc.messageBox_mc`'s own
  subtree — the box's background, its stroke, its button bar, its text field, and 34 retries each on
  nine `boxBkgd_mc.particles_mc.particleN`. The box's contents therefore never leave alpha 0.

What is established, and what is not:

* **the two movies do not share a context.** `GFxMovieRoot::GFxMovieRoot` does
  `pGC = new GASGlobalContext(this, dataDef->GetVersion())`, so `_global`, `MovieClip.prototype` and the
  registered-class table are per movie root. The obvious theory — agent DM's hand-over 1, that opening
  the global movie after the menu costs the menu its `tweenTo`/`tweenEnd` because they share one AS2
  library registration — does not explain this, and I checked it rather than quoting it.
* **the class IS installed in the failing movie's own context.** The failing pc is inside the
  1774-byte `__Packages.gfx.motion.Tween` buffer, which means `mc.tweenTo(...)` resolved and ran. Both
  names were copied onto the same `MovieClip.prototype` by the same constructor, three assignments
  apart (`tweenTo`, `tweenFrom`, `tweenEnd`, `tween__run`, `tween__to`, `tween__start`), and the
  assignment order does not match the failure set either: `tweenEnd` is copied *before* `tween__start`
  and both fail while `tweenTo` and `tweenFrom` do not.
* **`ASSetPropFlags` is not it.** The class's last statement is
  `ASSetPropFlags(MovieClip.prototype, "tween__start,tween__to,tween__run", 1)`; this tree's
  `GlobalASSetPropFlags` sets flags on the named members and removes nothing, `PropFlag_DontEnum` is 1
  as it should be, and `tweenEnd` is not in that list anyway.

So the next step is one measurement and not a theory: a one-shot log at the "call of a value that is
not a function" site saying whether the receiver's movie root's `Prototypes[Proto_MovieClip]` holds
the name at that instant, and which movie the receiver belongs to. That is ten lines in
`GASop_CallMethod` and one 60 s run; the first burst is at the first drawn frame and costs nothing to
reproduce.

Three adjacent facts, so the next package does not have to find them again: agent DJ's deviation 5
says `GFxSprite::AttachMovie` runs the clip's first frame **before** its class constructor; agent DK's
deviation 5 says the class binding is queued at `GFxAP_Lowest`, after the frame actions; and agent DJ's
defect 6 is that an imported movie's init actions reach the frame as a `GASImportInitActionsTag`. Every
one of the three touches the order in which a newly created subtree's first action buffer sees
`MovieClip.prototype`, which is the object both failing names live on. Hand-over 3.

## 7. The census, before and after

One keyboard-driven run of the user's own path each time (main menu -> NEW GAME -> a difficulty ->
brightness), same command line, same driver:

**Before** is `build/agentEA/run8.txt` (log `EA_run8.log`), the baseline build — HEAD plus the
screenshot switch and nothing else. **After** is `build/agentEA/run20.txt` (log `EA_run20.log`), the
finished package on the same path with one Down press fewer. Both drove the same screens with the same
real-input driver, and both report **0 `Critical:` lines** and **0 unimplemented opcodes**.

| | before | after |
|---|---:|---:|
| `loadBitmap` unresolved | 54 | **0** |
| `lineTo` | 8 | **0** |
| `moveTo` | 2 | **0** |
| `endFill` | 2 | **0** |
| `beginBitmapFill` | 2 | **0** |
| `new 'Matrix' is not a constructor` | 2 | **0** |
| `getTextFormat` | 1 | **0** |
| `getTextExtent` | 1 | **0** |
| `gotoAndPlay`, `gotoAndStop`, `removeMovieClip`, `ClearAnimation`, `beginFill`, `curveTo`, `lineStyle`, `clear` | 0 | **0** (never failed on this path — §0) |
| `toUpperCase` on undefined (the gamma label, §4) | 1 | 1 |
| `tweenTo` on undefined | 1 | 1 |
| `tween__start` / `tweenEnd` (`gfx.motion.Tween`, §6) | 5 | **975** |
| the interface asking the game for something it does not answer (`UpdateSettings`, `SetIcon`, `updateDebug`, `AddControllerButtonInstance`) | 0 | 8 |
| untextured image fills skipped | 17,126 | **331** |
| distinct images that resolve to no texture | 11 | **1** |
| `GotoLabeledFrame: no frame named 'PC'` (benign; retail logs it too) | 16 | 18 |

Every one of the eleven names the brief listed is at **0 unresolved calls**: `loadBitmap`, `lineTo`,
`moveTo`, `endFill`, `beginBitmapFill`, `getTextFormat`, `getTextExtent`, `gotoAndPlay`,
`removeMovieClip`, `ClearAnimation` and `tweenTo` — the last four because they never failed on this
path in the first place (§0). `tweenEnd` is the twelfth and it is §6.

The `tween__start` count **rises**, and that is the honest reading of it: before this package the
message box did not exist, so its fourteen clips and its nine particles never asked for a tween. The
eight `UpdateSettings` / `SetIcon` / `updateDebug` / `AddControllerButtonInstance` calls appear for the
same reason — the run now reaches screens it used to stop short of.

The one image that still resolves to nothing is a **sub-image**: `id 257, def SubImage, export '' file
''`, a tag-1008 rectangle of a packed atlas whose base image is resolved against `dataDef` and is not
found there. That is agent DC's hand-over — 483 of the cook's 1,015 images are `BaseImageId` +
`SubRect` and the UV offset is not applied either — and it is package EC's.

## 8. Verification

* **Every build and every measurement of this package is made in the snapshot worktree
  `build/agentEA_wt` (HEAD `c598a21` plus this package's 13 files), not in the shared working tree.**
  The shared tree does not link: package ED declares `AActor::GameSave` in `EngineClasses.h` and has
  not defined it, so every module's link fails with `LNK2001` on it, in Engine, GameFramework, IpDrv,
  AkAudio and DishonoredGameModule alike.
* **Clean full Release build** of that worktree, build directory deleted first, every target and
  `DISHONORED_LAYOUT_CHECKS=ON`: **994 edges, 0 errors** — `DishonoredGame`, `CoreSmoke`,
  `EdgeAnimSmoke`, `LayoutProbe` and the rest (`build/agentEA/buildclean.log`). **0 `C4263` and 0
  `C4264`.**
* **Regression** on that same clean gate build: `python resources\tools\run_regression.py --build-dir
  build/agentEA_wtrel --no-build --exe-name DishonoredGame_EAW.exe --log-prefix EAW` ->
  **`31 ok, 0 failed, 0 skipped, 431s`** (`build/agentEA_wtrel/regression/summary.txt`). An earlier
  pass over a build made with `--target DishonoredGame` alone reported `22 ok, 0 failed, 2 skipped`:
  the `coresmoke` and `layout` stages skip when `CoreSmoke.exe` and `LayoutProbe.exe` are not in the
  build directory, and a skipped stage is not a passed one.
* **The acceptance runs on the clean gate build** are `build/agentEA/gate1.txt` (the whole path:
  the menu, the New Game screen, four mouse hovers, a difficulty, the brightness screen, continue) and
  `gate2.txt` (the four hover states with the pointer returning to a neutral corner between each).
  Both report **0 `Critical:` lines** and **0 unimplemented opcodes**; `gate1` reaches
  `> ShowMessageBox (Creating a new game will overwrite previous autosaves. Continue?, YES, NO, )`.
  The earlier working runs on the same sources are `run18.txt` (the built-in census and the fills),
  `run20.txt` (the modal), `run22.txt` (the modal's display tree) and `run23.txt`; each has its own log
  `EA_<name>.log` and its own screenshots.
* **No generated file was touched.** Nothing this package changed is produced by
  `gen_classes_header.py`, so no regeneration is needed.
* **The driver is real Windows input** and it is not agent DQ's. Movement is `SendInput` with
  `MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE`, not `SetCursorPos`: `UWindowsClient::ProcessInput` reads
  the mouse from **DirectInput8** (`WinClient.cpp:652`, `DirectInput8Mouse->GetDeviceData`) and
  `SetCursorPos` puts nothing in that buffer. Measured: a 110 s run driven with `SetCursorPos` reported
  `axis probe: 0 calls` and `0 mouse events` however many times the pointer was moved; the same run
  driven with `SendInput` reported `3 hit tests / 3 targets resolved`. That is the fallback rule of
  `PHASE11.md` applied to the driver itself.

## 9. Switches this package adds

Both read on first use, neither a file-scope initialiser.

* **`-gfxuishotat=<sec>[,<sec>…]`** — a screenshot at N seconds after the interface's first drawn
  frame. Agent DJ's `-gfxuishot` takes a drawn-frame index, and a drawn-frame index is not the same
  instant twice: measured here, the same command line reached drawn frame 1392 at 28.0 s in one run
  and at 12.0 s in the next, so a dump asked for by frame number photographed the start screen instead
  of the New Game screen. Seconds since the first drawn frame is the clock the driver's schedule is
  keyed to, so the two agree.
* **`-gfxuidumpdlat=<sec>[,<sec>…]`** — the display tree of every open movie, on the same clock.
* The `NOT TEXTURED` line: one per distinct image id, at most 32, naming the image, its export name,
  its file name, its fill type and its bounds. A counter says a fill drew nothing; a name says which
  picture. §2 is that line.
* The texture listing: when an image url resolves to no texture, the textures the package **does**
  hold, once per run. That is what read the `-nopack` suffix out of the running game.

## 10. Deviations, stated once

1. **`GFxDrawingContext` is built on this tree's `GFxShapeCharacterDef`, not on a `GFxPathPacker`.**
   Retail packs the contour into a `GFxShapeWithStyles` through `GFxPathPacker`; this builds the same
   definition the tag loader builds from a `DefineShape` record, so the existing tessellator, fill
   styles, mask pass, `Display` and `DefPointTestLocal` draw and hit-test it unchanged. The moment of
   decoding is agent CD's existing deviation (`GFxCharacterDefs.h`'s own header comment), not a new
   one; every entry point's semantics are retail's, read out of the bodies in `agentEA_status.csv`.
2. **`GFxFillStyle` gains `pDirectImage`.** A SWF fill names its image by dictionary id and the display
   walk resolves that id against the movie the shape came from; an AS2 `beginBitmapFill` is given a
   `BitmapData` that `loadBitmap` resolved out of an **imported** movie's export table, so there is no
   id in this dictionary to name it by. Retail has the same shape from the other side: its
   `GFxDrawingContext::SetBitmapFill` (`0xa83be0`) is handed the `GFxImageResource` itself.
3. **`BitmapData`, `Matrix` and `TextFormat` are plain AS2 objects**, as `Key`, `Mouse`, `Stage`,
   `ExternalInterface` and `fscommand` already are in this tree (agentDG.md deviation 1, agentDJ.md
   deviation 7). Same script-visible surface; nothing in this cook calls `new BitmapData()`.
   `BitmapData.width` and `.height` are ordinary members filled once, because a cooked image never
   changes size, where retail answers them from a `GetMember` override (`0xa7a990`).
4. **`TextFormat.getTextExtent` measures through the field the format was taken from**, by formatting
   the string in that field's own document and putting the field's text back. Retail's format carries a
   font handle and lays the string out with it; this carries the field, which has the same font, the
   same size and the same paragraph format, and is the field whose extent both callers
   (`_common.TitleBar`, `_common.HelpBar`) are asking about. `ascent` is reported as the height and
   `descent` as 0; nothing in this cook reads either.
5. **`GFxEditTextCharacter::GetMember` consults a TextField prototype directly**, where retail resolves
   it through `GASTextFieldObject`'s own prototype chain (`GASTextFieldProto` `0xa21120`). A text field
   in this tree has no AS2 object of its own, so the prototype is consulted in the place the chain
   would have been.
6. **An export symbol resolves case-insensitively after an exact match fails.** Measured:
   `_common.EmbedImg` asks `BitmapData` for `X360_DPad_Up -nopack` and the shared library exports
   `X360_Dpad_Up -nopack` — the content disagrees with itself about one letter, in its own asset. GFx
   resolves a symbol case-insensitively for a movie authored below SWF 7, which every movie in this cook
   is, and `GASStringNode` carries a pre-resolved lower-case node for exactly that rule. The exact match
   is still tried first, so nothing that resolved before changes.
7. **`UDisGFxMoviePlayerBase::execShowMessageBox` finds the global movie by the name of its cooked
   `USwfMovie`**, where retail holds a `UDisGFxMoviePlayerGlobal` pointer on `UDisGlobalUIManager`. Same
   movie either way; the class is not declared in this tree. The id bookkeeping above it —
   `HideMessageBox`, `AddMessageBoxTimer`, and the `FArkGameEvent` the box answers with — is still
   `UDisGlobalUIManager`'s and is still absent.
8. **`beginGradientFill`, `lineGradientStyle` and `attachBitmap` are not ported** (`0x9ee7f0`,
   `0x9ee850`, `0x9f66d0`): no caller on this path. `startDrag` / `stopDrag` (`0x9f0270` / `0x9f0450`)
   are not ported either, and §4 is why that is a finding rather than an omission.

## 11. Hand-overs

1. **`_z` and the 3D display properties** (§3). This is fault 2 and it is a package: `GeomData`'s 3D
   tail (Z, ZScale, XRotation, YRotation — agentDJ.md deviation 11), and the projection that makes them
   mean anything: `GFxCharacter::Is3D`, `GetPerspective3D`, `GetView3D` and `GScreenToWorld`
   (agentDQ.md deviation 3). Four screens of the New Game flow are authored in perspective and the
   values are in §3.
2. **The options settings tree** (§4). This is fault 3. Agent BE ported the AS2 builder
   (`ShowSettingsCategoryList` 2012 `0x817250`, `CreateGFxCategory` `0x815a50`, `CreateGFxSubCategory`
   `0x813db0`, `CreateGFxSetting` `0x80f920`); what is missing is (a) something calling it on the menu
   path — retail's is `UDisGFxMoviePlayerGamma::OpenGammaMenu` **2013 `0x81daf0`** and
   `UDisGFxMoviePlayerMenuBase::FillOptionsMenu` **`0x81da70`** — (b) `m_SettingsCategoryList`, which is
   config, and (c) `Setting_Value` / `Setting_Minimum` / `Setting_Maximum` / `Setting_Increment`, which
   come from `UArkProfileSettings` and are the slider's range. `_xmouse`/`_ymouse` are done, so the
   slider's drag loop has a position the moment it has numbers.
3. **`gfx.motion.Tween`'s mixin and the two open movies** (§6). Five errors in one frame on the menu's
   own movie, and **971** on the global movie's, which never resolve — the message box's whole subtree.
   It is the last thing between the modal being built and the modal being visible, and agent DM's
   hand-over 1 is the same defect seen from the other side.
4. **`UDisGlobalUIManager`**, still, and now with one more caller: the message box's id, its timer and
   the game event that carries the player's answer back to
   `_common.MessageBoxInvoke::OnMessageBoxClosed`. Without it the box has no way to answer, so
   `OnNewGameConfirm` cannot be reached from it. `UDisGlobalUIManager::ShowMessageBox` is 2012
   `0x8aeee0`, `HideMessageBox` `0x8aef20`, `AddMessageBoxTimer` `0x8aef00`; none of the three has a
   2013 match in `retail2013_named.i64`, so they were folded or inlined and the 2013 body to work from
   is `UDisGFxMoviePlayerGlobal::ShowMessageBox` `0x7946b0`, which this package ported.
5. **The atlas sub-image** (§7): one image id in the whole New Game flow still resolves to nothing and
   it is a tag-1008 sub-image whose base is looked up in the wrong dictionary. Agent DC's hand-over,
   package EC's, and this is a second instance of it.
6. **`_common.EmbedImg`'s glyphs are loaded and not yet drawn.** `loadBitmap` now answers with a real
   `BitmapData` and `width`/`height`, which is what `EmbedImg` builds its `_buttonsArray` of
   `{subString, image, baseLineY}` from; putting that image into a text run is an `<img>` tag in the
   HTML text path, which `GFxStyledText` does not parse. 54 errors became 0 and the glyphs are still
   not in the help bar's text.
7. **`GFxDrawingContext`'s stroke is the tessellator's quad-per-segment** (agentDC.md deviation 2), so
   `lineStyle`'s caps, joints and miter limit are read and dropped. The one `lineStyle` caller in this
   flow draws no visible line.

## 12. Files

Mine (13, one new):

`External/GFx3/GFxDrawing.cpp` (**new**, 730 lines); edits to
`External/GFx3/{GFxAS2Lib.cpp, GFxCharacterDefs.cpp, GFxCharacterDefs.h, GFxDisplay.cpp, GFxDisplay.h,
GFxHitTest.cpp, GFxPlayer.h, GFxPlayerData.cpp, GFxPlayerSprite.cpp, GFxTextField.cpp}`;
`GFxUI/Src/gfxuiengine.cpp`; `DishonoredGame/Src/disgfxmovieplayerbase.cpp`; `cmake/GFx.cmake` (one
entry).

**No file of agent EE's is touched** (`ScenePostProcessing.cpp`, `SceneRendering.{cpp,h}`, the ark
post-process units, `FogRendering.cpp`, the filter and bloom passes).

Scratch, not repo tools: `build/agentEA/` — `eapatch.py` (the CRLF-safe patch helper),
`patch01_shotat.py` … `patch17_msgbox.py` (one per change, each with its measurement in its
docstring), `drive.py` (the real-input driver), `go.py` (run and drive in one command),
`scan_names.py`, `findstr.py`, `dumptab.py`, `lookup.py`, `xrefs.py`, `disasm.py`, `bmp2png.py`,
`as2/` (the cook's own bytecode for the eleven classes this rests on), `dec1`..`dec4` (the headless
decompiles), the run logs `run1`..`run23` and the screenshots; `build/agentEA_build.cmd`,
`build/agentEA_run.py`, `build/agentEA_sync.py`; snapshot worktree `build/agentEA_wt`, build directory
`build/agentEA_wtrel`; `build/agentEA_save/DisMission0.sav` (a copy of agent DJ's synthesised save).

IDA: **own copy only**, `build/agentEA_ida/retail2013_agentEA.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.
