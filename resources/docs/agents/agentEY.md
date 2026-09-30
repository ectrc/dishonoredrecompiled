# Agent EY (PHASE12 EY) — the brightness screen's symbols, the option rows' values, and three faults that are one line of `attachMovie`

Branched from HEAD `e2d7e5d` and **rebased twice mid-package — onto `8a04f5f` (agent EX) and then onto
`a677a21` (agent FA)** — which is what the final build, the final measurements and the sync script are
against. Own worktree `build/agentEY_wt`,
own build dirs `build/agentEY_wt/build/agentEY_rel` (this package),
`build/agentEY_headsrc/build/agentEY_headrel` (the untouched `a677a21` baseline, built from a
`git archive` export), `build/agentEY_wt/build/agentEY_reg` (the regression harness),
`build/agentEY_wt/build/agentEY_clean` (the gate build). Own IDA copies
`build/agentEY_ida/retail2013_agentEY.i64` and `shipping2012_agentEY.i64`, headless only. **No IDA MCP
tool and no FModel tool was used.** No commits, nothing staged, nothing written into the main checkout
outside `build/`.

**This package touches no file in `External/GFx3` and no file in `GFxUI`** — agent EX owns them this
wave. Every fault below that lands there is handed over with the measurement that places it.

## The answer in twelve lines

* **The brightness screen has its five Outsider marks.** `UDisGFxMoviePlayerGamma` had no
  implementation at all. `OpenGammaImage` now constructs the player, loads `UI_Gamma_SF`, starts
  `UI_Gamma.GammaImage` as a second movie over the menu and calls `_root.gamma_mc.SetGammaImages` with
  the five `m_BrightnessValues` from `DefaultUI.ini`. Before/after/retail:
  `build/agentEY/sbs_brightness4.png`. Section 2.
* **The option rows' values were being handed to AS2 as freed memory.** `GFxValue::SetStringW` stores
  the pointer and copies nothing; the id-mapped arm of `DisCreateGFxSetting` built it from a temporary
  `FString` that died at the semicolon before `PushBack` ran. Before: `Mapping_Names=[2: '<garbage>'
  '<garbage>']`. After: `[2: 'Off' 'On']`, `[4: 'Easy' 'Normal' 'Hard' 'Very Hard']`. Section 4.
* **The value column is not empty — it is off the right-hand edge of the screen.** Measured, from
  inside the running movie: each row's widget exists, is visible, is at `_alpha = 100` and holds the
  right text — `btn0.txt_mc.txt.text = 'OFF'`, `_txt_mc.txt_mc.txt.text = 'Normal'`. The list that
  carries them is at `_x = 0` where the asset asks for `_x = -450`. Section 4.
* **And that is the same defect as the missing footer prompt bar and as the gamma marks sitting 195 stage
  pixels too high.** `MCAttachMovie` copies the init object **after** the registered class constructor
  instead of before. Three independent proofs: `_root.help._props._x = 0` where the init object says
  1184; `_options_mc._defPosX = 0` where the init object says -450; `gamma_mc._y = 360` where
  `GammaMc`'s constructor says 555. **One line in `External/GFx3/GFxAS2Lib.cpp:736` fixes three of the
  six faults in this brief.** Section 6.
* **Faults 4 and 5 of the user's list were never this package's, and both are now closed by other
  packages.** The Corvo portrait is not misplaced and the New Game background is not enlarged on
  `a677a21`: side by side against `fourth.png` the portrait is at left-centre at its authored size, the
  difficulty rows carry their own depths, and the only visible difference left on that screen is the
  missing `ACCEPT / BACK` footer (`build/agentEY/sbs_newgame4.png`). My own mid-package measurement of a
  (208, 307) pixel displacement was taken against the user's `fourth.png`, whose left half is a
  1.25x-magnified window, and from a capture of my own taken **during the opening tween**; both were
  wrong and the conclusion is withdrawn. What was real was the diagnosis — `_z` was not a display
  property and `SetPerspective3D` was an empty body — and agent FA has ported it. Section 5.
* **`flash.geom.ColorTransform` and `flash.geom.Transform` do not exist in our AS2 library** — only
  `flash.geom.Matrix` is installed. That is why the five gamma marks are drawn bright instead of tinted
  by `1 - value`: the log carries five `new: 'ColorTransform' is not a constructor` errors, one per
  mark. Section 6.
* **The slider, one half settled and the other half honestly not.** Keyboard: **works both ways**,
  measured — three LEFT take the gamma 2.2 → 2.1 → 2.0 → 1.9 and three RIGHT take it back to 2.2. Agent
  EO's "one RIGHT then nothing" was the mapping's own maximum at 2.3. Mouse: **unmeasured** — eight
  attempts, every one lost the foreground to a **"Windows Security Alert"** window (`rundll32`; **nine**
  of them open on this desktop, holding the foreground continuously). Section 3.
* **`CONTINUE` and `LOAD` are correct behaviour for an empty profile, settled by experiment.** Our save
  directory is empty, so `HasSaveGame(0)` is FALSE. Point `-savedir=` at the retail install's own save
  folder and the bar becomes `CONTINUE | NEW GAME | MISSIONS | LOAD | OPTIONS | QUIT GAME` — `second.png`
  exactly. `build/agentEY/relSave_c.png`. Section 7.
* **Regression 37 ok, 0 failed, 0 skipped** on `a677a21` + this package, built inside the harness; a clean full
  Release build with the build directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors,
  0 C4263, 0 C4264**; `rva_sweep` over the whole worktree: 3 suspects, **none in a file this package
  touched**. Section 9.
* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` IS required and WAS
  run**; `Sources.cmake` did **not** change. **No file outside `DishonoredGame`.** Section 10.
* **Two extraction tools are the reason most of the above could be measured at all**: the cooked UI
  packages are LZO-compressed, and the whole options screen and the whole footer bar live in movies
  the menu only *imports*. Section 1.

## 1. What was measured, and with what

Every capture is an `-apshottime` screenshot — taken inside the process on the game thread from the
viewport's own backbuffer, so neither an overlapping window nor a DWM magnification can enter it — driven
by `resources/tools/drive_input.py` with `--exe DishonoredGame_EY.exe` (or `_EYH.exe` for the baseline),
real `SendInput` keys, from the start screen, at 1600x900, with nothing forced visible.
`build/agentEY/go.py` launches the run and the driver together and keeps the run log, the drive log and
the bitmap under one tag.

Three things this package needed that did not exist, all scratch, none of them part of the merge:

* `build/agentEY/swf/unpack.py` — the cooked UI packages are **LZO-compressed** (`COMPRESS_LZO`,
  compression flags 2); this decompresses one and carves the `GFX` payload out of every `SwfMovie`
  export.
* `build/agentEY/swf/as2dis.py` — an ActionScript 2 disassembler for those payloads. Zero unknown
  opcodes across six files. The dumps are `build/agentEY/swf/*.as2.txt` and every asset-side claim below
  cites a line number in them.
* `build/agentEY/drag.py` — a mouse-drag step built on the shared driver's own primitives (it imports
  `drive_input` rather than copying it, so the three defects that module documents are not
  re-introduced).

**The single most useful thing the extraction found: `MainMenu.gfx` is not self-contained.** It
`ImportAssets2`-es from three sibling movies that live in **`Startup.upk`** —
`Startup.Common_assets.lib`, `Startup.UI_OptionsMenu.OptionsMenu` and `Startup.UI_LoadGame.LoadGame`.
The **whole options screen** is the imported symbol `o_OptionsScreen` out of `OptionsMenu.gfx`, and the
**whole footer prompt bar** is `_common.HelpBar` / `lib_dynamicHelpBar` out of `lib.gfx`. Four of this
brief's six faults are in code that is not in the movie the menu appears to be.

Three censuses were added to `DishonoredGame` to read the AS2 side of the interface back, because every
one of these faults is invisible from C++ alone:

* `-disoptionsdiag=<n>` — read the first *n* option rows back off the AS2 objects one call after they
  were written, through the same `ObjectInterface` the asset reads them with.
* `-disclipdiag=<path>[;<path>…]` — one clip's display properties, its `props` object and every member
  it carries with its value. This is what found the portrait's transform, the help bar's position, the
  options list's `_defPosX` and the value widgets' text.
* `DisReportHelpBar` — the footer bar's clip, reported when the options tree is filled and when the
  brightness screen opens.

## 2. The brightness screen's five symbols

Agent EO's hand-over 2 was right about the shape and wrong about nothing: `UDisGFxMoviePlayerGamma` had
no implementation file, `OpenGammaImage` and `CloseGammaImage` were `DISHONORED_NATIVE_STUB`,
`m_pGammaMenu` was never constructed, `UI_Gamma_SF` was never loaded and `m_BrightnessValues` was never
filled.

| retail | what | here |
|---|---|---|
| 2012 `0x63d7a0` exec / `0x822980` body, 2013 `0x7c2580` | `OpenGammaImage` — construct `m_pGammaMenu`, then load | ported |
| 2012 `0x81daf0`, 2013 `0x7c2d30` | `OpenGammaMenu` — Start, Advance, `_root.gamma_mc.SetGammaImages([values])` | ported |
| 2012 `0x81dcf0`, 2013 `0x7c2f30` | `OnCompleteMoviePackageLoading` — the async completion | **gap**, unreachable: the load is synchronous |
| 2012 `0x801340`, 2013 `0x7c2d20` | `CloseGammaImage` → `CloseGammaMenu` | ported |
| 2012 `0x7f72c0`, 2013 `0x7bcb90` | `CloseGammaMenu` — `_root.gamma_mc.Close()` | ported |
| 2012 `0x7f73d0` | `OnGammaImageClosed` — `Close(TRUE)` and `Outer->m_pGammaMenu = NULL` | ported |
| 2012 `0x81d320`, 2013 `0x7a4270` | `LoadMoviePackageAsync` | **gap**, stated deviation |

Two deviations, both stated in the source rather than hidden:

1. **The package load is synchronous.** Retail's shipping arm queues `UObject::LoadPackageAsync` with
   `UDisGFxMoviePlayerBase::StaticOnCompleteMoviePackageLoading` as the callback; neither that function
   nor `OnCompleteMoviePackageLoading` exists in this tree's `UDisGFxMoviePlayerBase`, and retail's own
   `GIsEditor` arm of `LoadMoviePackageAsync` is a synchronous `LoadPackage` followed by the same
   completion body. That arm is what is ported.
2. **The movie path is measured, not read off a tweak object.** Retail takes it from the player's
   `UDisTweaks_GFxMoviePlayerBase` (`m_MoviePackageName` at +144, `m_MovieName` at +156 — confirmed
   against the decompile's `v19 + 12` / `v19 + 13` with `FString` at 12 bytes), and **no tweak object in
   this build names the gamma movie**. The path used is what the cook actually wrote: `UI_Gamma_SF.upk`'s
   export table has exactly one `SwfMovie`, `UI_Gamma_SF.UI_Gamma.GammaImage`, beside its two
   `Texture2D`s `GammaImage_I2` and `GammaImage_I5`.

`m_BrightnessValues` is a `config` property of a `CLASS_Config` class and nothing in this tree runs
`UObject::LoadConfig` over these generated classes, so the five floats are read out of
`[DishonoredGame.DisGFxMoviePlayerGamma]` of `DefaultUI.ini` by the same key spelling the ini uses:
`0.998, 0.99, 0.95, 0.90, 0.80`.

**What the asset does with them**, `GammaImage.as2.txt:4706-4801`: `CreateElements` makes N `gamma_logo`
clips in one row, centres the container on its own size, and for each builds
`new _common.SetColorTransform(img).SetColor_Custom(1-v, 1-v, 1-v, 1)` — each value is a **luminance
multiplier complement**, larger value → darker swatch — then attaches a `gamma_logo_indic` outline at
`_alpha = 12.5` and fades the whole thing in. The movie's own debug harness (`GammaImage.as2.txt:5661`)
passes `[0.8, 0.9, 0.99, 0.995, 0.999]`, the same five numbers in the other order: the ini's values are
right and the argument shape is right.

The gamma player takes no input (`bAllowFocus`, `bAllowInput`, `bCaptureInput` all FALSE) for the reason
retail's does: the topmost open movie that can receive input is what `FGFxEngine` falls back on while no
local player owns focus, so a focusable second movie over the menu would swallow the arrow keys the
slider is driven with. `Priority = 128` puts it over the menu and under the cursor's Global movie (255).

**Measured, from the menu, nothing forced** (`build/agentEY/fa_relC_log.txt`):

```
GFx movie loaded: UI_Gamma.GammaImage  1280x720  30.0 fps  2 frames
DisOpenGammaMenu: UI_Gamma.GammaImage open, 5 brightness values -> _root.gamma_mc.SetGammaImages ok
GFx UI census: movies open 3 [UI_MainMenu.MainMenu, UI_Gamma.GammaImage, UI_Global.Global], drawn 3
```

`build/agentEY/sbs_brightness4.png` is the untouched `a677a21` baseline, this package, and retail's half
of `fitfth.png`, side by side at the same window size and the same moment: no marks, five marks, five
marks. **Two differences remain and both are section 6's**: ours are about 230 screen pixels too high
and are untinted.

## 3. The slider: which path works

**The keyboard path works, in both directions.** `build/agentEY/relKey_log.txt`, three LEFT then three
RIGHT on the brightness screen, real `SendInput` keys, the game holding the foreground throughout:

```
[0027.27] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 2.1000 -> changed 1, gamma now 2.2000
[0030.00] ...                                                                 value 2.0000 -> changed 1, gamma now 2.1000
[0033.00] ...                                                                 value 1.9000 -> changed 1, gamma now 2.0000
[0037.00] ...                                                                 value 2.0000 -> changed 1, gamma now 1.9000
[0040.29] ...                                                                 value 2.1000 -> changed 1, gamma now 2.0000
[0043.00] ...                                                                 value 2.2000 -> changed 1, gamma now 2.1000
```

(`gamma now` is the value *before* the reread, which is why it lags one step.) **Agent EO's second
measurement was not a fault**: one RIGHT from 2.2 gives one change to 2.3 and then nothing because 2.3 is
the mapping's own maximum. Starting below the maximum, RIGHT walks up exactly as LEFT walks down.

**The mouse path is not settled, and I did not guess it.** Eight runs. What they establish:

* Mouse **motion** reaches the interface: 8316 hit tests, 8281 targets resolved, 6 rollOver / 5 rollOut
  on the brightness screen (`relMouse8`).
* A mouse **click** does reach the movie when the game holds the foreground: **1 press / 1 release**,
  2618 button hits, on the main menu bar (`relMouse7`).
* Every attempt to land a click or a drag **on the brightness slider** read `0 press / 0 release`, and in
  every one the foreground at the moment of the press belonged to another window.

That window is identified rather than guessed: handle `0x240756` is `rundll32`, title **"Windows Security
Alert"**, and `Get-Process rundll32` lists **nine** of them. Sampled six times at 0.7 s intervals it held
the foreground continuously. This is `STATUS.md`'s open defect 4, the standing rule is not to click
through a Windows Defender Firewall prompt, and I did not. **The mouse path on the slider is
unmeasured.** It needs one run on a desktop with those prompts answered or cancelled; the line to read is
the census's `press / release` on the brightness screen, and anything above zero settles it.

What can be said statically: the slider widget is `Slider_widget` / `_common.P_Slider` in
`OptionsMenu.gfx` and it drives itself from `Mouse.addListener` with `onMouseDown` / `onMouseMove` plus
`onPress` (`OptionsMenu.as2.txt:6828`, `6915`, `14671`) — there is **no `startDrag` anywhere in the
asset**, so CLIK drag handling as such is not the gap; delivering the press is.

## 4. The option rows' values

**Two separate faults, one fixed here and one measured and handed over.**

### 4.1 The labels were freed memory (fixed)

Agent EO's `DisCreateGFxSetting` is member for member what `0x7db5f0` does. But `GFxValue::SetStringW`
stores the pointer and copies nothing (`GFxValue.h:275`), and the id-mapped arm was written as

```cpp
GFxValue Label;
Label.SetStringW( *DisSettingValueLabel( Mapping.Name ) );   // temporary dies at the semicolon
MappingNames.PushBack( Label );                              // reads freed memory
```

`Setting_Name` survived because `DisSetGFxString` takes a `const FString&` and the whole `SetMember` sits
inside the caller's full expression; the drop-list labels did not, because `PushBack` is a separate
statement. Read back through `-disoptionsdiag`, **before**:

```
row Setting_Id=104 Setting_Name='Auto Use Mana Elixir'  Mapping_Type=4 Setting_Value=1 Mapping_Names=[2: '<garbage>' '<garbage>']
row Setting_Id=106 Setting_Name='Difficulty'            Mapping_Type=3 Setting_Value=1 Mapping_Names=[4: '<garbage>' '' '<garbage>' '<garbage>']
```

**after**:

```
row Setting_Id=104 Setting_Name='Auto Use Mana Elixir'  Mapping_Type=4 Setting_Value=1 Mapping_Names=[2: 'Off' 'On']
row Setting_Id=105 Setting_Name='Kill Cam Mode'         Mapping_Type=3 Setting_Value=1 Mapping_Names=[3: 'Off' 'Normal' 'Frequent']
row Setting_Id=106 Setting_Name='Difficulty'            Mapping_Type=3 Setting_Value=1 Mapping_Names=[4: 'Easy' 'Normal' 'Hard' 'Very Hard']
row Setting_Id=108 Setting_Name='Head Bob Amount'       Mapping_Type=2 Setting_Value=1 Setting_Minimum=0 Setting_Maximum=1 Setting_Increment=0.1
```

### 4.2 The column is not empty — it is off the screen

This is the part the brief, and my own first two hypotheses, got wrong. Walked down from the movie root
with `-disclipdiag`:

```
_root.optionsMenu_mc._options_mc._list_mc                          8 members: widget0..widget5 ghostItem0 ghostItem1
  .widget1              _x=0 _y=72 _alpha=100 _visible=true        members: _txt_mc _widget_mc _stepper_mc _bkgd_mc …
  .widget1._widget_mc   _x=680 _y=0 _alpha=100 _visible=true       _optionsList=[3: 'Off' 'Normal' 'Frequent'] _selectedIdx=1
  .widget1._widget_mc._txt_mc.txt_mc.txt   text='Normal'  htmlText='Normal'  textWidth=134.55  embedFonts=true
  .widget0._widget_mc.btn0.txt_mc.txt      text='OFF'     htmlText='OFF'     textWidth=125.75  embedFonts=true
```

**Every row's value widget is attached, is visible, is at full alpha and holds the right text.** The
dispatch in `_common.OptionsList.SetList` (`OptionsMenu.as2.txt:11207-11418`) runs correctly and
`SetOptionStepper` / `SetOptionStepperB` are reached. The fault is one level up:

```
_root.optionsMenu_mc._options_mc   _x=0  _y=-80  _rotation=-4  _defPosX=0
```

and the asset attaches that clip as (`OptionsMenu.as2.txt:22476-22484`)

```
this._options_mc = this.attachMovie("o_OptionsList_withTabs", "_options_mc",
                                    this.getNextHighestDepth(),
                                    {_x:-450, _y:-80, _rotation:this._tabs_mc._rotation});
```

`_y` and `_rotation` took; **`_x` is 0 where the asset asks for -450**, and `_defPosX` is **0**.
`_common.OptionsList`'s constructor is `this._defPosX = this._x` (`OptionsMenu.as2.txt:6297`, between
`_mcCreator` and `_curSelection = 0`) and its own slide writes `_x = _defPosX - _offsetPosX` back
(`OptionsMenu.as2.txt:7300`, `7338`). `_y` and `_rotation` survive because nothing ever rewrites them.
That is 450 stage pixels = **562 screen pixels at 1600x900**: the row labels land at screen x ≈ 870
instead of ≈ 300, and the value widgets at `_x = 680` inside the row land past the right edge of a
1600-wide window. `build/agentEY/sbs_options3.png`, this package against `third.png`, shows exactly
that offset; it is unchanged by either of the two rebases, as it must be.

**Why `_defPosX` is 0 is section 6**: the constructor that snapshots it runs before the init object is
applied. Fixing `MCAttachMovie`'s order puts the whole list back where the asset puts it and the value
column with it — and with §4.1 already merged the values that appear there will be the right ones.

## 5. The portrait and the New Game background — neither was this package's, and a premise of my own that did not survive either

**Faults 4 and 5 of the user's list are closed on `a677a21`, by agents EX and FA, and neither of them was
ever a `DishonoredGame` fault.** Side by side against `fourth.png` at the same window size, on a settled
frame: `build/agentEY/sbs_newgame4.png`. The portrait is at left-centre at its authored size, the
difficulty rows sit at their own depths, the background framing is within a few per cent of retail's, and
the only difference left on that screen is the missing `ACCEPT / BACK` footer, which is §6. (Ours shows
the unmasked portrait because `SetPortrait` is `this._corvo_mc.portrait_mc.gotoAndStop(idx)` — one frame
per difficulty — and the capture sits on EASY where retail's sits on VERY HARD.)

**I got this wrong once myself and it is worth recording how.** Mid-package I measured, by template match
at eleven scales, that the portrait was the same size as retail's and translated by (208, 307) screen
pixels, and I wrote that up as a finding. Two things were wrong with it: the left half of
`reference/menu/fourth.png` is our window as it was *before* agent EX's DPI fix, a 1.25x DWM magnification
of a smaller render; and my own first capture of that screen was taken at world time 18, **during the
opening tween** that slides the portrait in. A capture of the same screen at world time 30 under load was
still mid-tween; only at 45 was it settled. **A reference screenshot of an animated menu is not a
measurement of a resting state**, and neither is a capture taken on a schedule that happens to land inside
a tween.

**What was real was the diagnosis, and agent FA has since ported it.** The asset places both clips by
reading our own getters back into a plain object and tweening to it, with a Scaleform-3D depth added
(`MainMenu.as2.txt:22119-22153`, `22156-22190`):

```
_corvo_mc.props       = {_x:…, _y:…, _z:-450,  _xscale:…, _yscale:…, _rotation:…, _xrotation:…, _yrotation:…, _alpha:…}
_bkgdTexture_mc.props = {_x:…, _y:…, _z: +150, …}
```

and one level up `NewGameMenu.OpenBkgd` sets `_bkgd_mc._z = 550` and **never tweens it back**
(`MainMenu.as2.txt:20151-20180`: the tween's prop list carries `_alpha`, `_xscale`, `_yscale`, `_rotation`
and no `_z`), while `OpenDescription` sets `_description_mc._z = -150` and `_description_mc.txt._z = -750`
the same way. Measured live with `-disclipdiag` on `8a04f5f`, six identical consecutive samples
(`build/agentEY/relNG_log.txt`):

```
live  _x=-640.65 _y=-150.95 _xscale=100 _yscale=100 _rotation=0 _alpha=100 _visible=true
props _x=-640.65 _y=-150.95 _z=-450 _xscale=100 _yscale=100 _rotation=0 _xrotation=undefined _yrotation=undefined _alpha=100
```

Everything that existed round-tripped exactly; **`_z` did not appear in the live list at all** and
`_xrotation` / `_yrotation` came back `undefined`, `GFxAS2Runtime.cpp:619`'s builtin display-property
table ended at `_rotation`, and `GFxMovieRoot::SetPerspective3D` was an empty function body. That is
what `a677a21` fixes, and it is why the screen now matches.

One corollary worth keeping, because "enlarged and blurry" reads like an autofit: **neither clip is an
image load.** `SetPortrait` is a `gotoAndStop` (`MainMenu.as2.txt:22359`) and the only `_common.ImgLoader`
in the whole movie is the missions screen's thumbnail (`MainMenu.as2.txt:16885`). `resizeImg` is never
reached from either clip. Anyone re-opening this screen should not go looking for a resize.

## 6. One line of `attachMovie`, three faults

### The footer prompt bar

The bar is entirely asset-side. `_common.UIBase.InitHelpBar` (`MainMenu.as2.txt:1331-1476`) builds a
constant list of `{btn, txt}` pairs from `_root.texts.t_Exit / t_Close / t_Back / t_Validate`, and
`MainMenuBase.InitHelpBar` (`MainMenu.as2.txt:5755-5925`) appends the `Options_main` entry —
`X / t_RestoreSettings` + `B / t_Back`, retail's `RESTORE SETTINGS / BACK` exactly. The New Game and
brightness screens use `SecondaryScreen`, `A / t_Validate` + `B / t_Back` = `ACCEPT / BACK`. All four keys
are in `DishonoredGame.INT` and our own `InitTexts` already reads all three sections. **No C++ call
supplies the bar's content.**

`DisReportHelpBar` reads the clip back off the menu movie. Before it opens, and after:

```
DisReportHelpBar(options screen):    _root.help _x=1184 _y=651 _alpha=0   _width=25.4  _height=2   _helpList array of 10, _helpContent_mc absent
DisReportHelpBar(brightness screen): _root.help _x=0    _y=0   _alpha=100 _width=213.2 _height=55  _helpList array of 10, _helpContent_mc present
```

The bar **is** attached, **is** populated with its ten entries, **is** opaque and **does** have content —
it is drawn at **(0, 0)** instead of **(1184, 651)**, in the top-left corner of the stage, which is "we
draw nothing there".

`_root.help = _root.attachMovie("lib_dynamicHelpBar", "help", depth, {_x:1184, _y:651})`
(`MainMenu.as2.txt:1460-1475`); `_common.HelpBar`'s constructor does
`this._props = {_x:this._x, _y:this._y, _alpha:100, …}` (`lib.as2.txt:9599`); `OpenHelp` does
`_x = _props._x + 20; _alpha = 0; tweenTo(0.3, _props, …)` (`lib.as2.txt:9648-9708`). The bar ends up
wherever `this._x` was **when the constructor ran**.

### The options list

`_options_mc._defPosX = 0` where its init object says `_x: -450` (§4.2). Same snapshot, same cause.

### The gamma marks

`GammaImageBase`'s constructor does `_root.attachMovie("GammaMc", "gamma_mc", depth, {_x:640, _y:360})`
and `GammaMc`'s own constructor sets `_y = 555` (`GammaImage.as2.txt:4573`, `4686`). Retail: init object
360, then the constructor's 555 wins — `555/720 = 0.771` of the stage, which is where retail's marks are.
Ours: constructor 555, then the init object's 360 wins — `360/720 = 0.5`, which is where ours are, about
230 screen pixels too high at 1600x900.

### The cause

`MCAttachMovie` (`GFxAS2Lib.cpp:736-764`) calls `s->AttachMovie(...)`, which creates the child and runs
its registered class constructor, and **then** copies the init object onto it. Flash and Scaleform apply
the init object **first**. Its own comment already says what it should do — *"The init object's members
are copied onto the new clip before its first frame runs"* — and it does not.

### And the tint

`SetGammaImages` tints each mark through `new _common.SetColorTransform(img).SetColor_Custom(…)`, and our
AS2 library installs only `flash.geom.Matrix` — `GFxDrawing.cpp:744-749` is the whole `flash.geom`
package. The run log carries **five** `new: 'ColorTransform' is not a constructor` and five
`new: 'Transform' is not a constructor`, one per mark. Ours are drawn untinted and identical; retail's
are graded.

### One more for EX, cheap and visible

Every button glyph does `gotoAndStop(_global.PlatformName)` and the log carries a dozen
`GotoLabeledFrame: no frame named 'PC' on … (3 labels)`. `_common.ControllerButtonIcon.SetIcon`
(`lib.as2.txt:6355`) branches to a separate `SetPCIcon()` when `_global.PlatformName === "PC"` and only
falls through to `gotoAndStop` otherwise, so either the strict comparison against `"PC"` is failing or
`SetPCIcon` is. That is the pad glyph over the PC menu in `third.png`.

## 7. `CONTINUE` and `LOAD`: correct behaviour, settled by experiment

The asset asks C++ for these by name — `req_CanContinueGame` and `req_CanLoadGame` are in `MainMenu.gfx`'s
`ExternalInterface.call` inventory — rather than believing the four booleans `mainMenu_mc.Open` was opened
with. (Worth noting for whoever owns it: `DisMainMenuOpenMenu` hard-codes both of those booleans to TRUE
with a bring-up comment, and it makes no difference for exactly this reason.)

A census line added to `Req_CanLoadGame`:

```
Req_CanLoadGame: engine yes, save/load enabled 1, saves 0  -> 0   (save dir …\DishonoredGame\SaveData\)
Req_CanLoadGame: engine yes, save/load enabled 1, saves 51 -> 1   (save dir …\Binaries\Win32\LocalFIles\)
```

and with the second, the bar reads **`CONTINUE | NEW GAME | MISSIONS | LOAD | OPTIONS | QUIT GAME`** —
`second.png` exactly, down to the `SHIFT Purchase Downloadable Content` prompt
(`build/agentEY/relSave_c.png`).

**So it is correct behaviour for an empty profile and not a defect of the menu.** There is a real finding
underneath it, and it belongs to the save system: `DisGetSaveGameDir()`
(`dishonoredengine.cpp:589`, marked `DISHONORED(written)`, not ported from retail) answers
`appGameDir() + SaveData\`, and the 51 saves this machine's retail install actually has are in
`Binaries\Win32\LocalFIles\`. The two builds do not read the same folder. Nothing here changes it.

## 8. Where each item of this brief ended up

| brief item | verdict |
|---|---|
| 1. portrait misplaced | **not a fault, and not this package's.** Premise withdrawn, including my own first measurement. On `a677a21`, on a settled frame, it is at left-centre at its authored size |
| 2. New Game background enlarged and blurry | the "blurry" is agent EX's DPI magnification, now fixed; the residue is `_bkgd_mc._z = 550` going unapplied — **agent FA's**. Neither EX nor EY fixed it |
| 3. no symbol icons on the brightness screen | **fixed**. Five marks drawn. Position and tint have two named causes in `External/GFx3`, handed to EX |
| 4. the slider | keyboard **works both ways, measured**; mouse **unmeasured**, blocked by nine Windows Defender Firewall prompts holding the foreground |
| 5. empty value column | **one real defect found and fixed** (freed strings); **the column itself is not empty** — it is 562 screen pixels off the right edge because `_options_mc._x` is 0 instead of -450. `External/GFx3`, handed to EX |
| 6. missing footer prompt bars | **cause found and proven**: built, populated, and drawn at (0,0). Same `attachMovie` ordering. Handed to EX |
| 7. `CONTINUE` / `LOAD` | **correct behaviour for an empty profile**, proved by running with `-savedir=` at a folder that has saves |

## 9. Gates

* **Regression:** `python resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEY_wt/build/agentEY_reg` — the worktree's own copy, an
  absolute build dir, built inside the harness (no `--no-build`), after copying the five gitignored layout
  inputs into the worktree. **37 ok, 0 failed, 0 skipped** on `a677a21` + this package,
  `build/agentEY/reg3.log`. It was run once per base, and all three are recorded rather than only the
  last: `e2d7e5d` **37/37** (`reg1.log`), `8a04f5f` **37/37** (`reg2.log`), `a677a21` **37/37**
  (`reg3.log`). One run in between failed a single check and is kept as `reg2_firstattempt.log`:
  `d3d9 unported_natives 1`, one `ADishonoredPlayerPawn::execPlayDying_Native` at 24.26 s in the middle of
  the texture-streaming burst. That is `STATUS.md`'s documented artefact — `L_Tower_P` streams eight
  levels and the pawn falls out of the world when they arrive late — it is on the walking path this
  package does not touch, and the immediate re-run was clean. **`unported_natives` is not in the harness's
  load-sensitive set, so it did not re-run the stage itself; it belongs there**, and that is a small
  concrete suggestion for whoever owns `run_regression.py`.
* **Clean gate build:** the build directory deleted first, full Release, `DISHONORED_LAYOUT_CHECKS=ON`:
  **0 errors, 0 C4263, 0 C4264.** `build/agentEY/buildclean.log`.
* **`rva_sweep.py`** over the whole worktree: **7349 citations, 4256 `ok-2013`, 3068 `ok-2013-mid`, 22
  `ok-2012-labelled`, 0 unknown-2012, 2 `MISLABELLED-2012`, 1 `UNKNOWN-CLAIMED-2013`.** All three suspects
  are pre-existing and in files this package does not touch — two in `GFxUI/Src/gfxuirenderer.cpp:1679`
  (agent EO reported the same two as a tool artefact: the 40-character lookback does not reach the front
  of a seven-address list) and one in `DishonoredGame/Src/disbehaviorpatrol.cpp:319`. **104 citations in
  the two files this package edits, all `ok-2013` or `ok-2013-mid`, none flagged.**
* **By hand, not by sweep:** every retail address cited was resolved with `ida_funcs.get_func` in its own
  build's database — 13 in the 2012 database (`build/agentEY/resolve_2012.txt`) and 10 in the 2013 one
  (`build/agentEY/resolve_2013.txt`). **All 23 land on a function START.** The five with no name in the
  2013 database (`0x7c2580`, `0x7c2d20`, `0x7a4270`, `0x7db5f0`, `0x7e50c0`) were each pinned a second way
  by their own bodies rather than by `match_2012_2013.csv` alone — `0x7c2580` by its `[this+0x1F0]`
  (`m_pGammaMenu`'s asserted retail offset 496) and its `StaticConstructObject` of
  `UDisGFxMoviePlayerGamma`, `0x7c2d20` by its tail call into the *named* `CloseGammaMenu`, and so on;
  `agentEY_status.csv` states the pin for each.

## 10. Merging

* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is REQUIRED and WAS
  run.** It removes the three ported stubs from `DishonoredGameNativeStubs.cpp`. Note for the next agent:
  the generator does **not** discover a ported native from the presence of its definition — it reads
  `DishonoredGameNativeStubs.ported.*.txt`, and the first run with no such file left all three stubs in
  place.
* **`Sources.cmake` did NOT change.** No source file was added or removed: `UDisGFxMoviePlayerGamma`'s
  bodies go in `disgfxmovieplayermenubase.cpp`, where the 2012 PDB attributes all six of them.
* **No file outside `DishonoredGame`.** In particular nothing in `Engine/` — this package does not touch
  `Engine/Inc/arksettings.h`, `Engine/Src/arksettings.cpp` or `Engine/Src/UOnlinePlayerStorage.cpp`, the
  three that agents EK, EO and EQ have all been through. Nothing collides with agent EX's `8a04f5f` (`External/GFx3/GFxDisplay.cpp`,
  `External/GFx3/GFxTextDocView.cpp`, `GFxUI/Src/gfxuirenderer.cpp`, `Launch/Src/Launch.cpp`) or with
  agent FA's `a677a21` (eleven in `External/GFx3`, three in `GFxUI`, and in `DishonoredGame`
  `dispostprocessmanager.cpp`, `disglobaluimanager.cpp`, `disgfxmovieplayerglobal.cpp`,
  `dishonoredplayercontroller.cpp` and two `Inc/CppText` headers). **The two files this package edits,
  `disgfxmovieplayermenubase.cpp` and `disgfxmovieplayermainmenu.cpp`, are in neither set.** The
  generated `DishonoredGameNativeStubs.cpp` is regenerated on top of `a677a21` from the merged
  `DishonoredGameNativeStubs.ported.*.txt` set, so it already carries FA's entries as well as this
  package's three.
* **Total: six paths — four source, two documents.**

  | file | what |
  |---|---|
  | `DishonoredGame/Src/disgfxmovieplayermenubase.cpp` | the gamma player, the `Mapping_Names` lifetime fix, the row and help-bar censuses, the `Req_CanLoadGame` census |
  | `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | `-disclipdiag` |
  | `DishonoredGame/DishonoredGameNativeStubs.ported.agentEY.txt` | **new**, three natives |
  | `DishonoredGame/Src/DishonoredGameNativeStubs.cpp` | generated: three stubs removed |
  | `resources/docs/agents/agentEY.md` | **new**, this report |
  | `resources/docs/agents/agentEY_status.csv` | **new**, the address table |

  `build/agentEY_sync.py` is the authoritative copy list and runs worktree → main; it **flags rather than
  copies** any file that changed in main since `a677a21`, and `--check` reports without copying. Dry-run:
  6 to copy, 0 flagged. Nothing is committed and the worktree is left dirty.

## 11. Hand-over

1. **Agent EX, `External/GFx3/GFxAS2Lib.cpp:736`:** `MCAttachMovie` copies the init object after the
   registered class constructor instead of before. **Three faults of this brief fall out of it**: the
   footer prompt bar's position, the options list's position (and therefore the whole value column), and
   the gamma marks' position. Each is proved separately in §6.
2. **Agent EX, `External/GFx3`:** `flash.geom.ColorTransform` and `flash.geom.Transform` are not
   installed; only `Matrix` is. Five failed constructions per brightness screen (§6).
3. **Agent EX, `External/GFx3`:** `_common.ControllerButtonIcon.SetIcon`'s `"PC"` branch is not being
   taken; a dozen `no frame named 'PC'` per screen (§6).
4. **Agent FA:** `_z`, `_xrotation`, `_yrotation` and `GFxMovieRoot::SetPerspective3D` (§5). Four clips on
   the New Game screen alone carry a `_z` the asset never tweens back.
5. **The save directory** (§7): `DisGetSaveGameDir()` is agent-written and points at
   `DishonoredGame\SaveData\`; this machine's retail saves are in `Binaries\Win32\LocalFIles\`.
6. **The nine "Windows Security Alert" prompts** (§3). They hold the foreground continuously and no
   real-input mouse measurement is possible until they are answered or cancelled. This is `STATUS.md`
   open defect 4; it now has a window handle, a process and a count.
7. **`UDisGFxMoviePlayerBase::LoadMoviePackageAsync` and `OnCompleteMoviePackageLoading`** are still
   absent (§2). Five other movie players in this tree open their packages through them.
8. **`build/agentEY/swf/`** is worth keeping: the LZO package unpacker, the AS2 disassembler and six
   disassembled movies including `OptionsMenu`, `lib` and `LoadGame`, which no previous wave had.

## 12. Scratch

`build/agentEY/` — the decompiles (`dec2012/`, `dec2013/`), the IDA scripts (`dec.py`, `dis.py`,
`xref.py`, `resolve.py`), `resolve_2012.txt` and `resolve_2013.txt`, the run logs, the drive logs, the
screenshots and the side-by-side composites, and `swf/`. `build/agentEY_run.py` is the runner,
`build/agentEY/go.py` the launcher, `build/agentEY/drag.py` the mouse-drag step, `build/agentEY_sync.py`
the copy list, `build/agentEY_ida/` the IDA copies, `build/agentEY_headsrc/` the `git archive` export of
`a677a21` and its build. None of it is part of the package.
