# Agent FD (PHASE13 FD) — the brightness screen's behaviour: five tints, and one factor of twenty

Branched from HEAD `e02f5f8` and **rebased mid-package onto `3c7b90d` (agent FC)**, which is what the
final build, the final measurements and the sync script are against. Own worktree `build/agentFD_wt`, own build dirs
`build/agentFD_wt/build/agentFD_head` (the untouched HEAD baseline, built from this worktree **before**
any edit) and `build/agentFD_wt/build/agentFD_rel` (this package). Own IDA copy
`resources/docs/idb/retail2013_agentFD.i64`, headless only. **No FModel tool was used.** No commits,
nothing staged, nothing written into the main checkout outside `build/`.

Agents FB, FC and FE run concurrently on the same commit. **This package touches three files in
`External/GFx3`**; they are named in full in §7 and none of them is `GFxAS2Lib.cpp`, which is FC's.

## The answer in nine lines

* **The five marks are tinted, measured.** Mean luminance of the brightest 5 % of each mark's box,
  left to right: HEAD `e02f5f8` **185.8, 185.8, 234.8, 189.4, 185.7** (flat - the 234.8 is the
  BRIGHTNESS title behind the middle one); this package on `3c7b90d` **14.5, 24.1, 47.9, 65.0, 89.1**
  - five distinct levels, rising in the order `1 - m_BrightnessValues` gives. Side by side with retail
  in `build/agentFD/fcTint_c.png`. `flash.geom.ColorTransform` and `flash.geom.Transform` are
  installed as retail's own `GASColorTransformObject` / `GASTransformObject`, with retail's object
  types, storage and semantics; the brightness screen's **20 `ColorTransform` and 5 `Transform`
  `not a constructor` errors go to 0**. §2, §8.
* EY's hand-over said five of each. It is **four ColorTransforms per mark** — `_colorTrans_lockOpaque`,
  `_colorTrans_lock`, `_colorTrans_default` in the constructor and one more in `SetColor_Custom` — and
  one Transform, five marks: 20 and 5. A small correction, and it is the arithmetic that proves the
  count is per-mark.
* **The slider's mouse path is fixed, and the brief's diagnosis list was right about the shape and
  wrong about which one.** It is not a hit test naming the wrong target, not a stale mouse position,
  and not stage-versus-local. `_xmouse` **is** transformed into the clip's own space — and the stage
  point is converted to twips before being handed to an inverse that expects pixels, so the answer
  comes out exactly **20 × stage_x − origin_x**. §3.
* Measured on the untouched HEAD build, on the brightness screen's slider, whose stage origin is
  x 637: with the pointer at stage x **512** the slider read `_xmouse` = **9603**; at **704** it read
  **13443**. The right answers are −125 and +67. 9603 = 20·512 − 637 and 13443 = 20·704 − 637.
* `_common.P_Slider::GetIndexFromPosition` clamps its argument to ±(`_trackW` − `_thumbW`)/2 = **±128.95**
  and divides by the span, so **any pointer more than 39 stage pixels from the left edge of the window
  gives ratio 1** and the widget answers `_maxValue`. That is "clicking anywhere with a mouse the
  slider defaults to the maximum position", exactly.
* The fix is the deletion of one multiplication in `GFxPlayerSprite.cpp`. **`_root._xmouse` was 20×
  too large by the same route, and that is what the content hands `MovieClip.hitTest`**, whose point
  form takes stage pixels — so this is not only the slider.
* **The environmental blocker was gone when the measurements that matter were taken, and came back.**
  An `EnumWindows` sweep at the start of this package listed **no** security dialog; the HEAD runs held
  the foreground throughout and the mouse census read **4 press / 4 release, 8 handlers invoked**,
  which is what made the diagnosis possible. **Two new "Windows Security Alert" dialogs appeared
  partway through** — raised by the other packages' executables — and from then on one of them
  (`0x390d36`, `rundll32` pid 31616) took the foreground at **every** press. So the fix's *click*
  is confirmed only indirectly, by the property the handler reads; the pointer-driven half is
  measured exactly. I did not click through them. §3.1.
* **Two premises of my own brief are wrong** and are corrected with the measurement: EY's
  "`Mouse.addListener` … no `startDrag` anywhere" is about a different asset — the brightness screen's
  slider is `_common.P_Slider` in `UI_Options_SF`, and it has both an `onPress`-driven drag and a
  track click; and `_trackW` / `_thumbW`, the obvious second suspect, are **correct** (328 and 70.1).
  §3.2.
* **The same absent `ColorTransform` was also darkening nothing on the main menu.** `MainMenuButton`
  sets its four frame lines to `SetColor_Custom(0.2, 0.2, 0.2, 1)`; with no `ColorTransform` they drew
  at full brightness. §2.4.

## 1. What was measured, and on what

Two builds of this worktree, both RelWithDebInfo, staged into the retail install under this agent's own
exe names so that they never collide with FB's, FC's or FE's concurrent runs:

| build dir | exe | what it is |
|---|---|---|
| `build/agentFD_wt/build/agentFD_head` | `DishonoredGame_FDH.exe` | the untouched HEAD `e02f5f8`, built from this worktree before any edit |
| `build/agentFD_wt/build/agentFD_rel` | `DishonoredGame_FD.exe` | this package |

Driven by `resources/tools/drive_input.py` through `build/agentFD/go.py`, `hover`/`hoverclick` only,
`--exe` bound to the image name. **The window's client area came up 1280x720 in every run** despite
`-ResX=1600 -ResY=900`, which matters: the movie is 1280x720, so the stage scale is 1.0 and a stage
pixel is a movie pixel. The drive's own output records it (`-> client 512,450 of 1280x720`) and every
number below is in that space.

**On claiming the screenshot.** `-apshottime` writes into `DishonoredGame/Screenshots/Win32Console`,
which every agent shares, and `agentFD_run.py` sweeps `apshottime*` before the launch and prints the
source file name and byte size of the one it copies. **Byte size does not discriminate here**: the
output is an uncompressed 24-bit BMP, so every 1600x900 shot is exactly 4,320,054 bytes and every
1280x720 one is exactly 2,764,854. The discriminator is the pre-launch sweep plus the md5, and both
are recorded. One capture in this package (`build/agentFD/headMouse.png`) **was** a foreign frame —
its run's staging failed with `PermissionError` before the sweep and the copy took a bitmap left by
the previous run. It is listed here rather than quietly deleted, and it is not used for anything.

## 2. The five marks: `flash.geom.ColorTransform` and `flash.geom.Transform`

### 2.1 What the asset does

`_common.SetColorTransform` (`GammaImage.as2.txt:5258`, and the identical copy in
`MainMenu.as2.txt:13599`) is the only way anything in this cook tints a clip. Its constructor is

```
this._trans                 = new flash.geom.Transform(targetMc);
this._colorTrans_lockOpaque = new flash.geom.ColorTransform(0.4, 0.4, 0.4, 1,   0,0,0,0);
this._colorTrans_lock       = new flash.geom.ColorTransform(0.4, 0.4, 0.4, 0.4, 0,0,0,0);
this._colorTrans_default    = new flash.geom.ColorTransform(1,   1,   1,   1,   0,0,0,0);
this._colorTrans_custom     = this._colorTrans_default;
```

and `SetColor_Custom(r, g, b, a)` makes a fifth object and assigns it:

```
this._colorTrans_custom  = new flash.geom.ColorTransform(r, g, b, a, 0,0,0,0);
this._trans.colorTransform = this._colorTrans_custom;
```

`GammaImage.as2.txt:4758` calls exactly that per mark, with `1 - value` on all three colour channels —
so the five `m_BrightnessValues` become five luminance multipliers and the marks get five different
tints. **Four `new ColorTransform` and one `new Transform` per mark, five marks: 20 and 5.**

### 2.2 The two classes, out of retail

Every address below was resolved by hand at VA = RVA + `0x400000` in
`resources/docs/idb/retail2013_agentFD.i64` with `ida_funcs`; all are named in the 2013 database.
`rva_sweep.py` over `source/Development/Src/External/GFx3` is **631 citations, 0 MISLABELLED-2012,
0 UNKNOWN-CLAIMED-2013**. The full table with how each was pinned is `agentFD_status.csv`.

What the bodies settle, and none of it was guessed:

* `GASColorTransformObject` keeps a **`GRenderer::Cxform` embedded at object+52** — the very class this
  tree's renderer already takes — channel-major, multiply in column 0, offset in column 1, channels
  R G B A (`SetMember` 0xa77060 writes `this+36..this+64`, which is +52..+80 from the object base).
* The constructor (0xa77c30) applies its arguments **only when there are eight**, otherwise leaving the
  identity `CreateNewObject` (0xa77d60) put there. Argument order is Flash's: three multipliers, alpha
  multiplier, then four offsets.
* `rgb` (both accessors) is the three **offsets**, one byte each, as `0xRRGGBB`; writing it zeroes the
  three colour multipliers, and a non-number leaves all three offsets at zero.
* `GASTransformObject` (0xa703d0, `SetTarget` 0xa700b0) holds the movie root and a refcounted
  `GFxCharacterHandle`, never the character, and re-resolves on every access — so a Transform whose
  clip has been removed does nothing instead of writing through a dangling pointer.
* `SetMember` (0xa70a20) has three branches. `pixelBounds` is swallowed. `colorTransform` checks the
  source's object type is **18**, calls `GFxCharacter::SetCxform` with the Cxform at source+52, **and
  then calls the character's vtable slot at byte +32 with FALSE**. `matrix` checks type **15**, reads
  the matrix, scales tx/ty by 20 and rewrites the `GeomDataType`.
* **That vtbl+32 call names nothing, so it was pinned a second way.** `GFxASCharacter::SetAcceptAnimMoves`
  (0x9c5cd0) has four data xrefs, all vtables; in the one at `0x11f6970` it sits at +32 and
  `GetAcceptAnimMoves` (0x9cb580) at +28, which is the declaration order this tree already has. So the
  call is `SetAcceptAnimMoves(false)`, and it is why a script-set tint is not put back by the clip's
  own next `PlaceObject2`.
* The object types are measured, not interpolated: **15 Matrix, 18 ColorTransform, 20 Transform**, from
  the equality guards in 0xa70a20, 0xa77c30 and 0xa70c90. `AddBuiltinClassRegistry<12, …Transform…>`
  (0x9e5d30) and `<16, …ColorTransform…>` (0x9e6030) pin the separate *builtin type* ids the prototypes
  are fetched with.

### 2.3 How they are installed here

`GFxDrawingInstall` already owned the `flash.geom` package (`Matrix` was the only class in it). The
install block is rewritten so that all three classes hang off one `geom` object and are also published
unqualified — which is what `import flash.geom.*` compiles to — and each constructor carries retail's
own `CreateNewObject` through `GASFunctionObject::pNewObjectFunc`, **the hook
`GASEnvironment::OperatorNew` already calls**. That matters: this tree's `OperatorNew` deliberately
ignores a constructor's result (agent EA's measured deviation, and it was right), so a C function that
tried to return the instance the way retail's `GlobalCtor` does would be thrown away. `CreateNewObject`
is where the class's own storage comes from, and it is where retail puts it too.

`flash.geom.Matrix` gains nothing but retail's object type tag (`GASMatrixObject`, `Object_Matrix`), so
that `Transform`'s `matrix` branch can recognise one; its `a`/`b`/`c`/`d`/`tx`/`ty` stay ordinary
members, which is the form `beginBitmapFill` in this same file already reads.

**Stated deviation:** `GASColorTransformProto::ToString` (0xa77430) is **not** installed. Nothing in
this cook calls `toString` on a ColorTransform, and installing it would put a printf-family formatter
into a file that has none. `concat` (0xa77850) is installed.

### 2.4 The measurement

On the untouched HEAD `e02f5f8`, driven to the brightness screen
(`build/agentFD/headDiag2_log.txt`, log time 47.87 s, every one of them on the same frame):

```
  20  new: 'ColorTransform' is not a constructor
   5  new: 'Transform' is not a constructor
```

With this package (`build/agentFD/relFix.txt`, same route): **0**. No new AS2 error of any kind appears;
the twelve `GotoLabeledFrame: no frame named 'PC'` errors are pre-existing and are not this package's.

`build/agentFD/headDiag2_c.png` is the before: five marks, all at the same brightness, drawn across the
BRIGHTNESS title. (Their *position* is agent FC's half of this screen, not this package's.)

**And the same absence was visible on the main menu.** `MainMenuButton` (`MainMenu.as2.txt:1515`,
`1591`, `1667`, `1743`) puts its four frame lines — `_leftLine_mc`, `_rightLine_mc`, `_upLine_mc`,
`_downLine_mc` — through `SetColor_Custom(0.2, 0.2, 0.2, 1)`. With no `ColorTransform` they drew at
full brightness. `MissionsMenu` (`MainMenu.as2.txt:16622`) is a third user, on a screen the user has
not reported.

## 3. The slider's mouse path

### 3.1 The blocker was gone, then came back — and what that did and did not cost

My brief says nine `rundll32` "Windows Security Alert" dialogs were open on this desktop and one held
the foreground continuously, which is why agent EY read `0 press / 0 release` on eight attempts.

**At the start of this package there were none.** An `EnumWindows` sweep listed eleven visible
top-level windows, not one of them a security dialog, and the one `rundll32` process alive had no
window at all. That is why every HEAD measurement in this section exists. The drive's own output
records the game holding the foreground through every step, and the census on the untouched HEAD build
reads

```
mouse: 5413 hit tests / 589 targets resolved (3493 shape walks, 18 button hits, 571 sprite hits),
       7 rollOver / 6 rollOut, 4 press / 4 release, 8 handlers invoked
```

**4 press, 4 release, 8 handlers invoked.** The clicks reach the movie and the movie runs its handlers.

**Then two new ones appeared.** A second sweep, taken after the fix was built, lists
`723956 | 31616 | #32770 | Windows Security Alert` and `2231326 | 26152 | #32770 | Windows Security
Alert`, and four `rundll32` processes where there had been one. They were raised while this wave ran,
by the other packages' executables. From that point every `hoverclick` in this package's runs reports
`foreground 0x390d36` — that dialog — in the instant the driver re-takes the foreground before the
button goes down, while every `hover` in the same run reports the game. Four consecutive clicks in the
last run, all stolen; `_selectedValue` stayed at 2.2 for all 40 samples.

**What that costs, precisely.** The *pointer* half is unaffected: `hover` delivers its deltas, the game
takes them, and `_xmouse` is read off the live clip — which is the quantity the whole diagnosis turns
on, and it is measured exactly, before and after, in §3.3 and §5. What could not be re-confirmed after
the fix is the last link, the click producing a new gamma. It is confirmed *before* the fix (a click
gives `value 5.0000`; the identical keyboard-only run with no clicks at all gives **zero**
`DisOnSettingChange` lines, so the click is what causes it), and the code between `_xmouse` and that
line is the asset's, unchanged. **I did not click through the dialogs.** The user has been told about
them before and they are back.

Two hazards are worth passing on, because both cost runs here. A run whose `stage_retail.py` fails with
`PermissionError` (the previous run's process still holding the exe) **still copies a screenshot**, and
it will be the previous run's — `agentFD_run.py` sweeps the shared directory only after staging
succeeds. And with `-gfxuicensus` the log reaches hundreds of megabytes, `drive_input.py` polls that
log's tail to time its steps, and the steps drift: in one run the step scheduled at +2.0 s fired at
+12.9 s and the flow never reached the brightness screen at all. Both are visible in the driver's own
output, which is exactly where the tool's own docstring says to look.

### 3.2 Two premises corrected

**(a) The asset.** My brief, from agent EY: "the asset uses `Mouse.addListener` with
`onMouseDown`/`onMouseMove`/`onPress` and has no `startDrag` anywhere, so CLIK drag handling is not the
gap." That is a different asset. The brightness screen is `_root.optionsMenu_mc._gammaSetting_mc`, an
**imported** `o_Options_gammaSetting` (`MainMenu.as2.txt:1157`, `import id 244`), and its slider is
`_common.P_Slider` in `UI_Options_SF` (`OptionsMenu.as2.txt:16049`). `P_Slider` has **both** paths:
`_thumb_mc.onPress = StartDrag`, which installs an `onEnterFrame` that reads `this._parent._xmouse`
every frame, and `_bkgd_mc.onPress = OnTrackClick`, which reads the same property once. There is a
`stopDrag` in it as well. The conclusion — that CLIK drag handling is not the gap — still holds, but
for a different reason: the asset does its own dragging out of `_xmouse`.

**(b) `_trackW` / `_thumbW`.** The obvious second suspect was `P_Slider`'s constructor snapshotting
`this._bkgd_mc._width` before the children exist — the same shape as the `MCAttachMovie` fault agent FC
is fixing, and it would produce NaN and hence ratio 1, the right symptom for the wrong reason. It is
**not** what happens. Measured on the untouched HEAD:

```
_root.optionsMenu_mc._gammaSetting_mc._widget_mc._widget_mc:
  _trackW=328  _thumbW=70.1  _bkgd_mc._width=328  _thumb_mc._width=70.1
  _minValue=0.5  _maxValue=5  _incrementValue=0.1  _selectedValue=2.2  _startValue=2.2
  _bWidgetEnabled=true  _lastIdx=5  _minIdx=0.5
```

Every one of them is right. The widget is correctly built.

### 3.3 What `GetIndexFromPosition` does, and what it was fed

`_common.P_Slider::GetIndexFromPosition(x)` (`OptionsMenu.as2.txt:1780`) is

```
minX  = -0.5 * _trackW + 0.5 * _thumbW          = -128.95
maxX  =  0.5 * _trackW - 0.5 * _thumbW          = +128.95
r     = (clamp(x, minX, maxX) + 0.5*_trackW - 0.5*_thumbW) / (_trackW - _thumbW)
ratio = r < 1 ? round(r * n) / n : 1            with n = (_maxValue - _minValue) / _incrementValue = 45
return  _minValue + ratio * (_maxValue - _minValue)
```

so `ratio` is forced to **1** whenever the clamped position reaches the right-hand end — and the return
is `_maxValue`, which for this widget is **5**.

The slider's stage origin is x **637** (`_gammaSetting_mc._x` 640 + `_widget_mc._x` -3 + the slider's
own 0), and with a 1280x720 client over a 1280x720 movie a stage pixel is a movie pixel. So the answers
are `stage_x - 637`.

**Measured on the untouched HEAD, with the pointer parked and `_xmouse` read off the live clip**
(`build/agentFD/relSlider3`, the same instrument, and the pre-fix binary differs only in this one
expression):

| pointer, stage px | `_xmouse` read | should be | `20 * stage_x - 637` |
|---|---|---|---|
| (never moved, 0) | **-637** | -637 | -637 |
| 512 | **9603** | -125 | 9603 |
| 704 | **13443** | +67 | 13443 |

Both are exact. And with `_xmouse` = 20·stage_x − 637, the clamp is reached for every stage_x above
**39 pixels**: the whole window except a 39-pixel strip down its left edge. A click anywhere gives
ratio 1 and the widget answers 5. On HEAD that is exactly what the log shows, once, because the second
and third clicks find the value already there and change nothing:

```
DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 5.0000 -> rounded 5, changed 1
```

**"Clicking anywhere with a mouse the slider defaults to the maximum position" is that line.**

### 3.4 The cause, and the fix

`GFxAS2GetDisplayProperty` case 20/21 in `GFxPlayerSprite.cpp` inverts the character's world matrix to
put the pointer into the clip's own space, which is right, and then feeds it the stage point converted
to twips, which is wrong. The world matrix is the product of every `GetMatrix()` up to and including
the root sprite's, and **the root's carries the viewport's twips-to-pixels scale**: the product maps
local *twips* to stage *pixels*, so its inverse maps stage *pixels* to local *twips*. Converting the
stage point to twips first makes the inverse apply the factor of 20 a second time, and the trailing
`* GFxTwipsToPixels` then only divides it back once.

The fix is the removal of one multiplication:

```
-               const float sx = mx * GFxPixelsToTwips;
-               const float sy = my * GFxPixelsToTwips;
-               const float lx = inverse.M_[0][0] * sx + inverse.M_[0][1] * sy + inverse.M_[0][2];
-               const float ly = inverse.M_[1][0] * sx + inverse.M_[1][1] * sy + inverse.M_[1][2];
+               const float lx = inverse.M_[0][0] * mx + inverse.M_[0][1] * my + inverse.M_[0][2];
+               const float ly = inverse.M_[1][0] * mx + inverse.M_[1][1] * my + inverse.M_[1][2];
```

**This is wider than the slider.** `_root._xmouse` is what the content hands `MovieClip.hitTest`, whose
point form takes stage pixels — this tree's own comment at `GFxHitTest.cpp:967` says so — and on HEAD
`_root._xmouse` is 20x the stage position, so every such hit test was being asked about a point far off
the stage. `_common.ItemsList`'s mouse area, `_common.SelectionHandler::RightClick` and
`_common.SlideMenu::UpdateMouseScroll` all read `_xmouse` the same way.

## 4. Where this package's half ends and agent FC's begins

The brightness screen has two faults in it and they are not the same fault.

| | owner | state |
|---|---|---|
| the five marks are all the same brightness | **this package** | fixed: `flash.geom.ColorTransform` / `Transform` installed, 25 constructor errors -> 0 |
| the five marks sit at the centre instead of the bottom | agent FC | `MCAttachMovie`'s init-object ordering, `GFxAS2Lib.cpp`; not touched here |
| the slider does nothing sensible with the mouse | **this package** | fixed: `_xmouse` was 20x |
| there is no ACCEPT / BACK bar | agent FC | `_common.HelpBar` snapshots `this._x` in its constructor; same one line |

`External/GFx3` is shared with FC. **This package changes no function in `GFxAS2Lib.cpp`** and FC's
package is that file. The three files touched here and every function in them are listed in §7.

## 5. Before and after, through the same driver

Same worktree, same driver, same schedule, same window size (client 1280x720), the only difference
being which of the two exes is staged.

### The `_xmouse` the slider reads

| pointer, stage px | HEAD `e02f5f8` | this package | correct answer |
|---|---|---|---|
| never moved (0) | -637 | -637 | -637 |
| 512 | **9603** | **-125** | -125 |
| 704 | **13443** | **+67** | +67 |

Sixteen consecutive samples at -125 and seven at +67, one sample per second off the live clip, with
`_ymouse` -1.35 throughout (pointer y 450 against a slider at stage y 451.35). Both answers are exact.

### The gamma the click produces

| | HEAD `e02f5f8` | this package |
|---|---|---|
| `ColorTransform` / `Transform` `not a constructor`, per brightness screen | **20 / 5** | **0 / 0** |
| a mouse click on the slider | `value 5.0000` -- `_maxValue`, from any pointer position | **not re-measurable**: a Windows Security Alert took the foreground at all four presses (§3.1) |
| the same route with no clicks at all | **zero** `DisOnSettingChange` lines | zero |
| keyboard LEFT / RIGHT | works both ways (agent EY) | unchanged |

### The pictures

| file | what |
|---|---|
| `build/agentFD/headDiag2_c.png` | HEAD `e02f5f8`, the brightness screen: five marks, all one brightness, across the title |
| `build/agentFD/fcTint_c.png` | this package on `3c7b90d`: five marks at the bottom in five levels, with ACCEPT / BACK |
| `build/agentEY/sbs_brightness4.png` | agent EY's three-way against retail, for the target |

Mean luminance of the brightest 5 % of each mark's box:

| | mark 0 | 1 | 2 | 3 | 4 |
|---|---|---|---|---|---|
| HEAD `e02f5f8` | 185.8 | 185.8 | 234.8 | 189.4 | 185.7 |
| this package on `3c7b90d` | **14.5** | **24.1** | **47.9** | **65.0** | **89.1** |
| `1 - m_BrightnessValues` (`DefaultUI.ini`) | 0.002 | 0.01 | 0.05 | 0.10 | 0.20 |

The order and the separation are the asset's; the absolute levels are not a linear read of the
multiplier because each box also holds the `gamma_logo_indic` outline, which is attached at
`_alpha = 12.5` and is *not* tinted.

Both were taken with `-apshottime` into the shared screenshot directory, swept before the launch, and
each is recorded with the `apshottime*.bmp` it came from and its md5 in `build/agentFD/shots.md5`.

## 6. Where my brief was wrong

Wave 16 had all four packages correct a premise of their own brief. Four here.

1. **"The environmental blocker is real and may still be there."** It is not. No `rundll32` security
   dialog exists on this desktop; the game held the foreground through every step of every run; the
   mouse path was measured end to end. Every measurement agent EY could not take is in §3.
2. **"The asset uses `Mouse.addListener` … and has no `startDrag` anywhere."** That is a different
   asset. The brightness screen's slider is `_common.P_Slider` in `UI_Options_SF`, reached through the
   imported `o_Options_gammaSetting`, and it has an `onPress`-installed `onEnterFrame` drag, an
   `onPress` track click, and a `stopDrag`. §3.2(a).
3. **"…or a mouse position that is stale or zero at the moment the handler reads it."** The mouse
   position is neither stale nor zero and the hit test names the right target: the handler is reached,
   on the right clip, with a live pointer. The number it is given is 20x too large. §3.3.
4. **"`flash.geom.ColorTransform` … Five `not a constructor` errors per brightness screen."** Twenty
   ColorTransform and five Transform. The arithmetic is four ColorTransforms per `SetColorTransform`
   instance and one Transform, five instances, and it is what shows the errors are per-mark. §2.1.

And one correction to a number quoted in `PHASE13.md` from an earlier wave: the brightness widget's
range is **`_minValue` 0.5, `_maxValue` 5, `_incrementValue` 0.1** — not a maximum of 2.3. The 2.3 an
earlier package saw is what the *keyboard* reached from the starting value, not the widget's bound.

## 7. Merging

**Regeneration: not required, and not run.** No source file was added or removed, no native stub was
ported, no `Sources.cmake` entry changed. `Sources.cmake` **did not change**.

**Four files, three of them source.**

| file | what changed |
|---|---|
| `source/Development/Src/External/GFx3/GFxDrawing.cpp` | **new**: `GASMatrixObject`, `GASColorTransformObject` (+ `GetMember`, `SetMember`, `FindChannel`), `GASTransformObject` (+ `GetMember`, `SetMember`, `SetTarget`, `ResolveTarget`), `GeomPublish`, `MatrixNewObject`, `ColorTransformNewObject`, `TransformNewObject`, `ColorTransformCtor`, `TransformCtor`, `ColorTransformConcat`, and the two file-scope pointers `GMatrixProto` / `GColorTransformProto`. **changed**: `GFxDrawingInstall` only, and only its `flash.geom` block. No other function in the file is touched. |
| `source/Development/Src/External/GFx3/GFxAS2Object.h` | three values added to `enum GASObjectType`: `Object_Matrix = 15`, `Object_ColorTransform = 18`, `Object_Transform = 20`. Purely additive; no existing value changed and no class touched. |
| `source/Development/Src/External/GFx3/GFxPlayerSprite.cpp` | `GFxAS2GetDisplayProperty`, **case 20/21 only** (`_xmouse` / `_ymouse`): two lines removed, two changed, plus the comment. No other function in the file is touched. |
| `source/Development/Src/DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | agent EY's `-disclipdiag` census: `_xmouse` / `_ymouse` added to its property list and its per-path sampling budget raised from 2 to 40. Measurement only; `DisReportOneClip` and `DisReportClipDiag`, nothing else. |

**Total: 4 files.** One is outside this package's own area in the sense that matters — it is agent EY's
census in `DishonoredGame` — and three are in `External/GFx3`, which agent FC also has open.

### For agent FC specifically

FC's package is `External/GFx3/GFxAS2Lib.cpp` (`MCAttachMovie`). **This package does not touch that
file.** The three GFx3 files above and every function in them are named in the table. The one place the
two packages could collide is `GFxAS2Object.h`, and only if FC also adds a `GASObjectType` value: the
three added here are 15, 18 and 20, all measured, and they are inserted between `Object_TextField = 13`
and `Object_Key = 22`.

`build/agentFD_sync.py` is the authoritative copy list, worktree -> main, and it **flags rather than
copies** anything whose copy in main differs from `e02f5f8`. It compares with `git diff` against the
base rather than a byte compare against `git show`, because the blobs are stored LF and the working
tree is CRLF and a byte compare flags every file in the tree.

**Nothing is committed and nothing is staged.**

## 8. A second defect the tint hunt turned up, in the class library itself

Installing the two classes was not enough and the reason is general enough to be worth its own
section, because it will bite the next package that adds a built-in class.

With `ColorTransform` and `Transform` installed, the 25 `not a constructor` errors went to **zero** and
the marks were **still untinted, with no error logged anywhere**. The instrument that settled it was a
temporary line in `TransformCtor` printing the object it had been handed:

```
FDNEW self=09003DE0 type=6 args=1      (five of them, one per mark)
```

**Object type 6 is `Object_Object`** — a plain `GASObject`, not the `GASTransformObject` (type 20) that
`pNewObjectFunc` was there to make. `GASEnvironment::OperatorNew` allocates with the constructor the
**prototype** names, not the one the opcode named:

```
GASFunctionObject* maker = ctor;
GASFunctionObject* protoCtor = proto->Get__constructor__(GetSC());
if (protoCtor != 0) maker = protoCtor;
GASObject* obj = maker->CreateNewObject(GetSC(), proto);
```

That is correct and deliberate — it is what makes a subclass of Array an array. But
`Get__constructor__` is a plain `GetMemberRaw`, so it **walks the prototype chain**: a prototype that
does not name its own constructor falls through its `__proto__` to `Object.prototype`, finds
`Object`'s constructor there, and every instance of the class comes out a plain object. Both
constructors then failed their object-type guard and returned silently, which is exactly the
"no error, no effect" the screenshots showed.

`GFxAS2Lib.cpp` sets `prototype.__constructor__` for all eight of its built-ins, which is why `Array`
works. The three `flash.geom` classes now do the same, in `GeomInstall`. **`flash.display.BitmapData`
and `TextFormat` in this same file still do not** — it costs them nothing today because neither needs a
storage subclass, but it is the same latent defect and it is recorded here rather than changed, since
neither is this package's.

## 9. Acceptance

| | state |
|---|---|
| 1. five marks tinted to five levels, side by side with retail | **done**: 14.5 / 24.1 / 47.9 / 65.0 / 89.1 against HEAD's flat 185.8 / 185.8 / 234.8 / 189.4 / 185.7. `build/agentFD/fcTint_c.png`, taken on `3c7b90d` so the marks are also in the right place and the two changes are not conflated |
| 2. the slider's mouse path fixed and measured | **fixed and measured**: `_xmouse` 9603 -> -125 and 13443 -> +67, exact, before and after (§3.3, §5). The last link - the click producing the new gamma - could not be re-measured after the fix because a Windows Security Alert took the foreground at every press (§3.1) |
| 3. before/after against the untouched HEAD through the same driver | done, `build/agentFD_wt/build/agentFD_head` vs `.../agentFD_rel`, same driver, same schedule (§5) |
| 4. `run_regression.py` 37/0/0, worktree's own copy, absolute build dir, built inside the harness | **done: 37 ok, 0 failed, 0 skipped, 1662 s**, `build/agentFD_reg.log`. `inputtest_peak_speed` read 0.0 on its first pass with four other agents' games on the machine and the harness's own LOAD_SENSITIVE retry took it; the final line is 37/0/0 |
| 5. clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | **done: 0 errors, 0 C4263, 0 C4264**, 963/963, `build/agentFD_clean.log`, into `build/agentFD_wt/build/agentFD_clean` after `rm -rf` |
| 6. `rva_sweep.py` plus a by-hand resolution of every cited address | **done**: 631 citations over `External/GFx3`, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013. Every address resolved by hand at VA = RVA + 0x400000 in this package's own IDA copy; `agentFD_status.csv` records how each was pinned, including the vtable-slot pin for `SetAcceptAnimMoves` and two addresses that resolve into an unrelated function without the image base |
| 7. `agentFD.md` and `agentFD_status.csv` inside the worktree | this file and its neighbour |

## 10. Rebase onto agent FC, and what was checked

FC merged at `3c7b90d` while this package was running. Its three files are all in `External/GFx3` and
**one of them is also mine**: `GFxPlayerSprite.cpp`. Checked rather than assumed —
`git diff e02f5f8 3c7b90d` puts FC's two hunks in that file at lines **1761 and 1804**
(`CreateEmptyMovieClip` and `AttachMovie`); this package's single hunk is in
`GFxAS2GetDisplayProperty`, case 20/21, around line 1260. **Disjoint, by ~500 lines and by function.**
Neither touches `MCAttachMovie`, and this package touches no function in `GFxAS2Lib.cpp` at all.

The worktree was rebased rather than left on `e02f5f8`: `GFxAS2Lib.cpp` and `GFxPlayer.h` were taken
whole from `3c7b90d`, `GFxPlayerSprite.cpp` was taken from `3c7b90d` and the one-function change
re-applied, and everything was rebuilt (`build/agentFD_rel_build5.log`, 0 errors). The sync script's
base is now `3c7b90d`. **The tint measurement in §5 is on that build**, so the marks are in FC's
corrected position and the before/after does not conflate the two changes.

FC's screenshot-claiming rule is adopted rather than rediscovered: `agentFD_run.py` now records the
launch time, prints every `apshottime<digits>.bmp` it finds with size and mtime, and claims only one
whose mtime is at or after its own launch. This package had already lost one capture to the shared
directory (§1) by the older rule.

`setMask`, which FC names as still missing from the AS2 MovieClip table, did not block the tint and is
not taken here.
