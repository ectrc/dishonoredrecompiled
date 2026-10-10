# Agent FF (PHASE14 FF) — the ActionScript class library: `setMask`, the depth convention, and three premises of the brief that were wrong

Branched from HEAD `18f36a3`. Own worktree `build/agentFF_wt`, own build dirs
`build/agentFF_headsrc/build/agentFF_head` (the untouched HEAD baseline, built from a `git archive`
export of `18f36a3` **before any edit**, so the worktree could be edited while it built) and
`build/agentFF_wt/build/agentFF_rel` (this package), plus `build/agentFF_wt/build/agentFF_clean`
(the clean gate build) and `build/agentFF_wt/build/agentFF_gate` (the regression harness's own).
Own IDA copies `resources/docs/idb/shipping2012_agentFF.i64` and `retail2013_agentFF.i64`, headless
only. **No FModel tool was used.** No commits, nothing staged, nothing written into the main
checkout outside `build/`.

Agents FG, FH and FI run concurrently on the same commit. **This package touches nine files, every
one of them in `External/GFx3`.** They are named in full, function by function, in §11 and in
`build/agentFF_sync.py`.

## The answer in twelve lines

* **`MovieClip.setMask` is ported, end to end — the AS2 thunk, the sprite-side storage, the render
  pass and the hit test — and it fixes fault 1b.** Measured, same driver, same schedule, both
  builds: on the Options → CONTROLS → KEYBOARD MAPPING list, HEAD draws **"Move Forward",
  "Move Backward" and "Strafe Left" over the GENERAL tab and the sub-tab bar, and "Power/Gadget"
  below the panel — four rows outside the list's own mask. This package draws **none**. The band is
  in `build/agentFF/mask_band.png` (HEAD above, this package below) and the full frames are
  `build/agentFF/headCtl_2.png` and `relCtl_2.png`. §2, §3.
* The mask is named in the log now: `_level0.optionsMenu_mc._options_mc._list_mc` masked by
  `_level0.optionsMenu_mc._options_mc.maskContent`, stage bounds **170.0, 214.6 .. 1151.7, 665.5 px**
  of a 1280x720 stage. `setMask … is not a function` **2 → 0**; the GFx seam census goes from
  **0 masks to 1** on that screen.
* **Fault 1 is not fixed and half of it is not a fault.** The white panel's extent is identical in
  HEAD and in this package. And the brief's second half — "the rows' value widgets are missing that
  chrome" — is **wrong**: the `<` `>` arrows *are* drawn on both, `build/agentFF/headOpt_0_crop.png`
  and `relOpt_0_crop.png`. §3.1.
* **Fault 2 did not reproduce, on either build, in four matched runs.** The value column was never
  emptied wholesale. One package run lost the single hovered row's widget where HEAD kept it; a
  repeat of the identical schedule showed all six present. I can claim neither a fix nor a
  regression and I am not going to. §3.2.
* **The `'PC'` errors are not a defect. The brief's item 6 is wrong in every part.** The `"PC"`
  branch *is* taken and it is the right branch; it succeeds on the icon clips, which all carry
  `default, Xenon, PC, PS3, PS3_Jap`. The ~40 errors come from a **different clip** — the icons'
  `_glow_mc` children and the DPAD's four direction children — each of which carries its **own**
  frame-0 script `stop(); this.gotoAndStop(_global.PlatformName);` while having only
  `default, loop, stop`. I decoded that bytecode out of `Startup.Common_assets.lib.gfx` byte by
  byte. It is an **asset bug and retail logs the same thing**. There is nothing to fix. §7.
* **Two of the three "missing methods" in brief item 2 are not methods.** `tweenTo` is installed by
  the asset itself (`gfx.motion.Tween`'s constructor writes it onto `MovieClip.prototype`) and every
  failure is `on undefined`; `UpdateSettings` is movie-defined on `VideoSettings.prototype` and
  `OptionsScreen.prototype` and its failure is `on Sprite`. Neither is a class-library gap. §5.
* **`stopDrag` is the one that was real, and it is fixed: 1 → 0** on a matched brightness-screen run
  (`headGam2` / `relGam2`), total AS2 errors 45 → 44. `startDrag` is ported with it because retail
  has it, and is **measured unused** by this cook. §5.
* **The depth convention is retail's now**, at every ActionScript boundary and at the two
  `GFxValue::ObjectInterface` entry points the engine's mouse cursor needs. `GASop_DuplicateClip`
  was subtracting the bias *and* making an empty clip instead of a clone; both are fixed. Agent EZ's
  `bScriptCreated` flag **is gone** — declaration, initialiser and both writes. §6.
* **`GFxCharacterHandle::ResolveCharacter` is real**: `pCharacter ? pCharacter : FindTarget(NamePath)`,
  with the absolute path interned at construction and rewritten by `SetName`, exactly as retail's
  `0x9cdd60` and its constructor do. **Measured effect in my runs: none** — nothing in the menu run
  resolves a dead handle. Honest: it is a gap closed, not a symptom cured. §8.
* **`BitmapData` and `TextFormat` now set `__constructor__`, and the measured effect is zero.** The
  survey shows `BitmapData` is only ever the *static* `BitmapData.loadBitmap(name)` (281 calls) and
  `new TextFormat` appears nowhere in any of the six movies. It is a real latent defect and it has
  no current symptom, which is not what the brief implies. §4.
* **The survey is done and is evidence-based**, not a checklist: of the fifteen classes the brief
  listed, **thirteen are referenced nowhere in any cooked movie**. The only two absences the content
  actually touches are `Selection.disableFocusKeys` (one property, six sites) and `MovieClipLoader`
  (four instantiations, reachable from Missions, gamepad mapping and the save thumbnails). I
  installed **neither**, on purpose. §9.
* **Gates**: clean full Release build with the directory deleted first and
  `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**. `rva_sweep.py` over `External/GFx3`:
  **668 citations, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013**. `run_regression.py` from the
  worktree's own copy with an absolute build dir, built inside the harness: **37 ok, 0 failed,
  0 skipped**. §10.

## 1. What was measured, and on what

| build dir | exe | what it is |
|---|---|---|
| `build/agentFF_headsrc/build/agentFF_head` | `DishonoredGame_FFH.exe` | the untouched HEAD `18f36a3`, from a `git archive` export |
| `build/agentFF_wt/build/agentFF_rel` | `DishonoredGame_FF.exe` | this package |

Driven by `resources/tools/drive_input.py` through `build/agentFF/go.py` (FD's runner, renamed),
`hover` / `hoverclick` only, `--exe` bound to the image name, every run with its own `--log-name`
and `--ini-dir`.

### The capture rule, and the one thing it cost

**Every screenshot in this package was taken inside the game process with `-apshottime`.** No
desktop or window capture was used anywhere, and `build/coord_shot.py` was not run.

It was verified first, before anything else: `build/agentFF/capchk.png`, `-apshottime=20` on the
HEAD build, is the main menu with the four buttons legible. The monitor being off has not cost the
device.

**The window's client area came up 1008x567 in every run** despite `-ResX=1600 -ResY=900`, so a
stage pixel is 0.7875 window pixels and every pixel coordinate below is in that 1008x567 space.

**One correction to the shared instrument, worth having in writing**: `agentFD_run.py` claims a shot
by "arrived at or after my launch", and that is **not sufficient any more** — my first Options run
claimed **nine** bitmaps belonging to agents FH (`apshottime_fhrelD*`) and FI (`apshottime_fi5*`),
because all three of us were running at once. `-apshotname=<tag>` is the only reliable claim.
`build/agentFF_run.py` now passes `-apshotname=ff` and both its stale sweep and its claim glob are
`apshottime_ff*`. Anyone reusing FD's runner should do the same.

## 2. `MovieClip.setMask`

### 2.1 What retail does, resolved by hand

Every address below was resolved in my own copies of the 2012 symbolized and 2013 retail databases
with `ida_funcs.get_func`, and each is confirmed to be a **function start** with its size. The
`setMask` identification is pinned a **second way, without `vtables.csv` and without
`match_2012_2013.csv`**: the MovieClip prototype method table lives in `.rdata` at 2012 `0xdb35xx`
as an array of `{const char* name, void(*)(GASFnCall&)}` pairs, and the pair at `0xdb3660` carries
the string `"setMask"` and the handler `0x9f5320`. The same table pins `attachMovie` `0x9ff990`
(`0xdb35f8`), `duplicateMovieClip` `0x9f5210` (`0xdb3600`), `removeMovieClip` `0x9f6c00`
(`0xdb3608`), `getDepth` `0x9cde60` (`0xdb3620`), `getNextHighestDepth` `0x9f5510` (`0xdb3630`),
`startDrag` `0x9f9a70` (`0xdb3650`) and `stopDrag` `0x9f9c50` (`0xdb3658`). **All six addresses the
brief gave are correct** — the first wave in three where the brief's addresses were not the thing
that was wrong.

The mechanism, end to end:

* **The thunk, `0x9f5320`** (size 0x87, 2013 `0x9ebb00`, bodies byte-identical). Result set to
  `undefined` first and never changed. No argument at all → the mask is left alone. `null`,
  `undefined`, a plain object, a `TextField`, or a character handle that no longer resolves → the
  mask is **cleared**, with no warning. Only a live `MovieClip` reaches `SetMask`.
* **`GFxSprite::SetMask`, `0x9f4c20`** (size 0x123). Retail keeps **one** pointer, `GFxSprite+0x1CC`,
  for two roles, discriminated by `GFxASCharacter+0xA0` bit `0x8000` (`IsUsedAsMask`): "my mask" when
  the bit is clear, "the sprite I am a mask for" when it is set. A clip is therefore **either masked
  or a mask, never both**, and most of the body is unlinking whichever role conflicts, recursively.
  The reference down is strong; the back-pointer up is not. Setting a scripted mask also **zeroes
  the masked clip's SWF `ClipDepth`**. The only invalidation is `pMovieRoot->Flags |= 0x400`, the
  optimized-advance-list dirty bit — there is **no** render-side invalidation, the mask is consulted
  fresh every frame.
* **The render pass.** `GFxSprite::Display` (`0x9faeb0`) pushes the mask around the **whole** sprite,
  its own drawing and its children, through `PreDrawMask` (`0xa5f2f0`) and `PushAndDrawMask`
  (`0xa5f510`), and pops it with `PopMask` (`0xa5f5a0`) after `PostDisplay`. `PreDrawMask` sets the
  mask up with **its own parent's** world matrix, not the masked clip's — that is the one fact that
  makes a scripted mask different from a `ClipDepth` mask, because `setMask` takes a clip from
  anywhere in the tree.
* **The hit test.** `GFxSprite::PointTestLocal` (`0x9fba50`) honours it:
  `pt_mask = inverse(mask world) * this world * pt`, then `mask->PointTestLocal(pt_mask, flags)`, and
  a miss ends the test. `GetTopMostMouseEntity` (`0x9fa5b0`) does **not** reference the mask slot at
  all; it reaches scripted masks only through each child's `PointTestLocal`. I left it alone.

### 2.2 What is ported, and the one thing I had invented and deleted

`GFxCharacter` gains `pMaskCharacter` / `pMaskOwner` and `SetMask` / `GetMask` / `GetMaskOwner`
(`GFxPlayer.h`, body in `GFxPlayerSprite.cpp`). Two pointers rather than retail's one dual-purpose
slot, with the mutual exclusion and the recursive unlink spelled out, because a slot that means two
different things by a flag is a defect waiting to be read wrongly in a reconstruction. `ClipDepth`
is zeroed on the masked clip exactly as retail does.

`GFxDisplayContext` gains `StageMatrix` — the twips-to-pixels transform `GFxMovieRoot::Display`
installs, kept so a mask can be drawn with its own chain of parents. `GFxDisplayPushScriptedMask`
(new, `GFxDisplay.cpp`) composes `StageMatrix * GFxCharacterWorldMatrix(mask->GetParent())`, resets
the colour transform, submits, and restores; `GFxSprite::Display` pushes it around its drawing and
its display list and pops after.

**The thing I had wrong and the IDA read corrected**: my first version skipped a clip from ordinary
rendering while it served as a mask — Flash's documented behaviour, and an **invention** here.
Retail's own early-out is `if ((Flags & 3) != 1 && (Flags & 0x8000) == 0) return`, i.e. invisible
**and** not a mask: GFx does **not** hide a mask, it draws a *visible* mask as content as well, and
relies on the content setting `_visible = false`. I deleted my version and wrote retail's. This is
the ninth or tenth wave in which the measured answer and the "obviously right" answer differed.

The invisible-mask half is real and necessary: a mask is drawn even when `_visible` is false,
because the mask pass has colour writes off and submitting nothing leaves the stencil empty, which
`EndSubmitMask`'s `== counter` test then turns into the whole masked subtree missing.

### 2.3 What the cook actually does with it

Six call sites, in `_common.ItemsList` (the options list) and `_common.P_DropDownMenu`:
`MainMenu.as2.txt:6256`, `OptionsMenu.as2.txt:6400` and `:11133`, `LoadGame.as2.txt:8528`,
`PauseMenu.as2.txt:14309` and `:16744`. All of the form `this._list_mc.setMask(maskMc)`, one
argument, and every logged failure was "on Sprite" — a real clip asking for a real built-in.

## 3. Faults 1, 1b and 2, re-measured against it

All three measured with the **same schedule on both builds**, `build/agentFF/{head,rel}Ctl_*` and
`{head,rel}Aud*`, shots at identical marks.

### 3.0 Fault 1b — **fixed**, and it is the biggest visible change in the package

Options → CONTROLS → KEYBOARD MAPPING, eleven `Down` presses, shot at t=72 s.

| | HEAD `18f36a3` | this package |
|---|---|---|
| label rows drawn | 12 | 8 |
| rows **outside** the list's mask band (window y 169..524) | **4** — "Move Forward" (y 123), "Move Backward" (156), "Strafe Left" (198) above, "Power/Gadget" (535) below | **0** |
| `--` value cells drawn over the tab bar | yes, from y≈93 | none |
| GFx seam census, masks | 0 | 1 |
| `setMask … is not a function` | 2 | 0 |

`build/agentFF/mask_band.png` is the y 80..180 band of both frames, HEAD above and this package
below: the top strip has two list rows and two value cells printed over "GENERAL", "KEYBOARD
MAPPING" and the AUDIO/GRAPHICS tabs; the bottom strip is clean and the tabs are legible. That is
the user's *"the content overflows and when scrolling down the list it will just go over the other
elements above"*, reproduced and then removed.

### 3.1 Fault 1 — **not fixed**, and the brief's second half is itself wrong

`headOpt_0.png` vs `relOpt_0.png`, Options as opened: the white panel's extent is identical to the
pixel in the two builds. `setMask` does not touch it. That half stands and is FG's.

The other half — *"the rows' value widgets are missing that chrome"* — does **not** stand.
`headOpt_0_crop.png` and `relOpt_0_crop.png` are the same 380x70 window region of the two builds and
both show the `<` and `>` arrow glyphs at the ends of the selected row's value widget, as retail's
`1.png` does. The chrome is drawn. Whatever is different about `1.png` is not a missing arrow.

### 3.2 Fault 2 — **not reproduced**, on either build

Four matched runs on the AUDIO tab (`headAud`/`relAud`, hovers at window-fraction 0.50; `headAud2`/
`relAud2` and `relAud4`, hovers at 0.76 and 0.80, i.e. over the value column where the user's
pointer is in `2.png`). The value column was **never** emptied wholesale on either build: every shot
has 100 / 90 / 90 / 100 / OFF / Auto.

The one asymmetry: in `relAud2_1.png` the single hovered row's widget is absent where
`headAud2_1.png` has it. A repeat of that exact schedule (`relAud4_1.png`, `relAud4_2.png`) has all
six present, and at t=62 s this package additionally **clips the stray widget that used to sit above
"Global Volume"** — i.e. the steady state is closer to retail than HEAD's. So the single asymmetry
is a transient and not a regression, and I am claiming neither a fix nor a break for fault 2.

**For agent FG**: fault 1b is closed by `setMask` — do not spend a package on it. Fault 1's panel is
open; fault 1's "missing chrome" is not a fault. Fault 2 I could not reproduce; if you can, say what
the pointer was doing, because the four schedules I tried do not do it.

## 4. `BitmapData` and `TextFormat`: `__constructor__`, and why it changes nothing today

Both prototypes now carry `__constructor__`, the way `GeomInstall` and `GFxAS2Lib.cpp`'s eight
built-ins do. It is a real latent defect: `GASEnvironment::OperatorNew` (2012 `0x9e89c0`) makes the
instance with the constructor the **prototype** names, so a prototype that names none falls through
to `Object.prototype` and is allocated by `Object`.

**And it has no symptom in this cook, measured.** The survey of all six movies: `BitmapData` appears
281 times and **every one is the static `BitmapData.loadBitmap(name)`** — there is no
`new BitmapData` anywhere, no `draw`, no `getPixel`, no `width`/`height` read. `new TextFormat`
appears **nowhere**; the only `TextFormat` object is the one `TextField.getTextFormat()` returns,
which C++ makes. Neither class has a `pNewObjectFunc`, so `Object`'s allocator and their own produce
the same plain `GASObject` today. The brief's framing — "the same latent defect that made `Transform`
silently allocate as a plain `GASObject`" — is right about the mechanism and wrong about the
consequence: `Transform` had a real allocator to lose and these two do not. Fixed anyway, because the
next class that gains storage would inherit the bug.

## 5. The three "missing methods"

### `stopDrag` — real, fixed, measured

`_common.P_Slider.prototype.StopDrag` does `this.stopDrag()` with no arguments
(`OptionsMenu.as2.txt:16330`), and it is the **only** call site in the entire cook. Retail's handler
`0x9f9c50` (size 0x4b) clears exactly one field — `DragState.pCharacter` at `GFxMovieRoot+0x23D4` —
leaves the lock flag, the bounds and the centre delta stale, and does not check that the clip it was
called on is the one being dragged.

Ported with the state it operates on, because a `stopDrag` that clears a drag nothing can start is a
shim: `GFxMovieRoot::DragState` (nine fields at retail's meanings), `MCStartDrag` from `0x9f9a70`,
`GFxMovieRoot::InitDragCenterDelta` from `0xa037c0`, and `GFxASCharacter::DoMouseDrag` from
`0x9cfa00`, called once per `Advance` instead of through retail's optimized play list.

Matched measurement, brightness screen, pointer clicked on the slider thumb at window fraction
(0.46, 0.62): **HEAD 1 `'stopDrag' is not a function`, this package 0**; total AS2 errors 45 → 44.
`build/agentFF/headGam2_log.txt`, `relGam2_log.txt`.

**`startDrag` is measured unused**: `P_Slider::StartDrag` installs its own `onEnterFrame` reading
`_parent._xmouse` and never calls the built-in. It is ported because retail has it at a confirmed
address and because half a pair is worse than none, and it is exercised by nothing in this cook.
Said plainly so the next wave does not read its presence as evidence.

### `tweenTo` — **not a built-in**

`gfx.motion.Tween` is compiled into all six movies (`MainMenu.as2.txt:1682`, a 1774-byte
`DoInitAction`) and **its constructor assigns `MovieClip.prototype.tweenTo = this.tweenTo`**
(`:1761`), along with `tweenFrom`, `tweenEnd` and three private helpers. There is nothing to port.
Every logged failure is **`on undefined`**, never "on Sprite", and the expression is always a named
child: `this._LShoulder_mc.tweenTo(0.25, {...}, mx.transitions.easing.Back.easeOut)` in
`_common.TabsHandler` (a 6659-byte buffer) and the same shape in `_common.SubTabsHandler` (2376
bytes) and `OptionsSecondaryScreen` (2243 bytes). The same classes raise `gotoAndStop`,
`gotoAndPlay`, `ClearAnimation` and `removeMovieClip` "on undefined" from the *same four*
expressions. One root cause, five method names, and the cause is that `this` in those handlers is
not the clip that owns `_LShoulder_mc`.

### `UpdateSettings` — **not a built-in either**

Capital U, defined by the movie on exactly two prototypes, both in `OptionsMenu.gfx`:
`VideoSettings.prototype.UpdateSettings(widgetIdx, widgetValue)` (`OptionsMenu.as2.txt:19753`) and
`OptionsScreen.prototype.UpdateSettings` (`:22842`), registered on `o_videoSettings` and
`o_OptionsScreen`. The failure is **`on Sprite`** — the clip is live, the method is not on its
prototype chain, i.e. its registered class is not bound. Same family as the 127
`SetWidgetFocus` / `RemoveWidgetFocus` "on undefined" from `__Packages.OptionsLine` and
`__Packages.OptionsList`, which is the single largest AS2 fault left in the menu.

**Brief item 2 is therefore one method, not three**, and the other two are a `registerClass` /
prototype-binding problem in the options list. **That is agent FG's**, and it is worth more than
anything else left in the AS2 surface.

## 6. The depth convention

Retail's rule, confirmed from three independent directions:

1. The disassembly. `attachMovie` applies `add eax, 4000h` at `0x9ffb3e`, to `(int)ToNumber(arg2)`,
   **before** the `GFxCharPosInfo` constructor, and the validity test at `0x9ffb55` is on the
   **already-biased** value and **unsigned**, `> 0x7EFFFFFD`, with the error naming the biased
   number. `duplicateMovieClip` applies `+ 0x4000` inline at its `CloneDisplayObject` call and has
   **no** range check and **no** warning. `getDepth` is `add edi, 0FFFFC000h` with **no clamp**.
   `getNextHighestDepth` is `sub eax, 3FFFh; sets cl; dec ecx; and ecx, eax` = `max(0, largest -
   0x3FFF)` — the constant is `0x3FFF`, not `0x4000`, and an empty display list answers `-1` so the
   result is `0`. `removeMovieClip` is a **signed** `Depth >= 0x4000`, with the else arm logging the
   handle's short name.
2. Agent FB's live capture of the running retail game, `build/agentFB/retail/displaylist.log`:
   `_root` at `depth=-16385`, `mainMenu_mc` −16383, `optionsMenu_mc` −16290, `mouseCursor_mc` −16381,
   `blackStripes_mc` −16379 — all timeline, all internal − 16384 — beside `help` at `0` and
   `vignette_mc` at `+1`, both script-created. Negative for the timeline, non-negative for script,
   no clamp anywhere. Exactly `0x9cde60`.
3. Our own build, live. `-gfxuidlcheck` prints every character's internal depth as it is destroyed;
   on the package build (`build/agentFF/relDepth_log.txt`) the census is 944 at depth 1, 243 at 5,
   66 at 2, 41 at 3 **and 10 at depth 16384** — the timeline clips low, and the script-created ones
   sitting at `GFxASDepthBias + 0`, which is where `attachMovie(…, 0)` now puts them. Separately:
   with `removeMovieClip` now testing `Depth >= 0x4000` instead of agent EZ's flag, a build in which
   the bias were **not** applied would log `"…removeMovieClip() failed - depth must be >= 0"` for
   every one of the cook's 59 `removeMovieClip` calls. The count in the measured runs is **0**, and
   no clip went missing from any screen.

Applied at every ActionScript boundary and nowhere else: `MCAttachMovie` (+ retail's unsigned
guard and its message), `MCCreateEmptyMovieClip`, the new `MCDuplicateMovieClip`, `MCGetDepth`
(− bias, unclamped), `MCGetNextHighestDepth`, `MCRemoveMovieClip`, `GASop_DuplicateClip`,
`GASop_RemoveClip`, and **both `GFxValue::ObjectInterface` entry points** — which matters, because
`FGfxEngine`'s mouse cursor reads a depth from the **AS2** `getNextHighestDepth` and hands it
straight to the **C++** `AttachMovie`, so the two have to be in the same space. The negative
sentinel on the C++ side ("put it on top") stays a display-list depth.

`GASop_DuplicateClip` was wrong twice over: it **subtracted** 16384, putting a cloned clip 16384
*below* its parent's timeline children where the display walk draws it first and anything with a
`ClipDepth` masks it out, and it made an **empty** clip instead of a clone. It now biases up and
calls a new `GFxSprite::CloneDisplayObject`, which instantiates the source's own definition beside
it carrying its transform, colour transform, ratio, clip depth and visibility. **Measured unused**:
the cook never calls `duplicateMovieClip` and never emits the `cloneSprite` opcode. So is
`getDepth`. Said so rather than implied.

**Agent EZ's `bScriptCreated` is gone** — the declaration in `GFxPlayer.h`, the constructor
initialiser, and both writes in `GFxPlayerSprite.cpp`. The depth is the test again, as it is in
retail.

One cosmetic difference left, not worth a change: retail reports `_root` at AS depth −16385
(internal −1) and ours would say −16384 (internal 0). Nothing reads it.

## 7. The `'PC'` errors: there is no defect here

Brief item 6: *"The `"PC"` branch of `_common.ControllerButtonIcon.SetIcon` is never taken … about
forty `no frame named 'PC'` errors per run — by far the largest source of AS2 noise."*

**Every part of that is wrong, and here is the measurement.**

1. **The branch is taken, and it is the right branch.** `SetIcon` reads `_global.PlatformName` off
   the preloaded-`_global` register — not `System.capabilities`, not a `_level0` variable, not a
   `Key`/`Mouse` probe, and there is no `_global.gfxPlatform` anywhere in the cook. The error text
   itself *says* `'PC'`, which proves `_global.PlatformName === "PC"` and `_global.bUsingGamepad` is
   falsy at that moment: the correct PC keyboard-and-mouse configuration.
2. **It succeeds on the clips it is supposed to run on.** All seventeen `ControllerButtonIcon`
   symbols in `Startup.Common_assets.lib.gfx` are 39-frame sprites carrying
   `default@1, Xenon@9, PC@16, PS3@23, PS3_Jap@30`. I read the `FrameLabel` tags straight out of the
   raw tag stream.
3. **The receiver is a different clip, and this package's new diagnostic names it.**
   `GFxSprite::GotoLabeledFrame`'s error now lists the labels the clip *does* have, which is the
   difference between "a label is missing" and "this is the wrong clip":
   `no frame named 'PC' on …_RShoulder_mc._glow_mc (3 labels: default,loop,stop)` and
   `…dir_Up (5 labels: default,on,stop,out,red)`.
4. **Those clips ask for the frame themselves.** `lib_RShoulder_Multi` is sprite 100 and places
   char 93 as `_glow_mc` on its first frame; the glow sprites (lib chars 79, 84, 93, 98, 107) and the
   DPAD direction sprite (char 193) each carry a 66-byte frame-0 `DoAction` which decodes to
   `stop(); this.gotoAndStop(_global.PlatformName);`:
   `07` Stop · `96 09 00 | 00 "_global" 00` Push · `1c` GetVariable · `96 0e 00 | 00 "PlatformName" 00`
   Push · `4e` GetMember · `96 0b 00 | 07 01000000 | 00 "this" 00` Push · `1c` GetVariable ·
   `96 0d 00 | 00 "gotoAndStop" 00` Push · `52` CallMethod.
   They have `default, loop, stop` and no platform labels.
5. The remaining one, `_level0.gammaSetting_mc (1 labels: default)`, is the same shape in
   `o_Options_gammaSetting`'s own frame-0 script.
6. **The class binding is correct**, which I checked rather than assumed: `-gfxuiclasstrace` on the
   HEAD build reports `'lib_RShoulder_Multi' -> class … reaches MovieClip.prototype` and
   `'lib_FrontBtnGlow_Multi' has no registered class`, which is right in both cases.

So this is an **asset bug**: a shared glow clip authored with a platform switch it has no frames for.
Retail's GFx logs the same warning from the same place and the behaviour is identical — the clip
stays on the frame it is on. **There is nothing to port and nothing to fix.** I did not suppress the
lines; I made them say which labels the clip has, so nobody spends another package on them.

A fair restatement of the brief's underlying observation: the `'PC'` lines are the largest source of
AS2 **log volume**, and they are not a defect. The largest source of AS2 **failure** is the
`SetWidgetFocus` / `RemoveWidgetFocus` / `tweenTo` "on undefined" family in the options list, §5.

## 8. `GFxCharacterHandle::ResolveCharacter`

`0x9cdd60`, 0x17 bytes, 2013 `0x9c4600`, bodies identical:
`pCharacter ? pCharacter : GFxMovieRoot::FindTarget(root, NamePath)`. `FindTarget` (`0xa014c0`)
resolves against **`_level0`'s own AS environment** — not the level the handle was made under and
not its original parent — and nothing is cached, in either direction. The handle's constructor
(`0x9d17b0`) interns `NamePath` as `parent->GetAbsolutePath() + "." + Name`, and `ChangeName`
(`0x9d18a0`) rewrites it with the name.

All of that is now in `GFxCharacterHandle::NamePath`, `GFxASCharacter::CreateCharacterHandle` and
`GFxASCharacter::SetName`. It matters because `GASValue::ToASCharacter` (`0x9caae0`) goes through it,
so **every** AS2 thunk taking a MovieClip argument — `setMask` among them — re-resolves a stale
handle rather than reading `undefined`.

**Measured effect in this package's runs: none.** No handle in the menu runs lost its live pointer,
so the fallback never fired. Agent EZ implemented this, measured no change, and reverted it; I have
kept it, and I am recording the same measurement rather than claiming otherwise. The reason to keep
it is that it is a real retail body at a confirmed address replacing a stub that returns its
argument, and the stub is the kind of thing that makes a later fault inexplicable.

## 9. The survey: what the cooked movies actually reference

Read out of the six disassemblies in `build/agentEY/swf/` (`MainMenu`, `OptionsMenu`, `LoadGame`,
`PauseMenu`, `GammaImage`, `lib`) by tokenising every `push`/opcode line and reconstructing global
reads, dotted member chains, `newObject` / `newMethod` receivers and `callMethod` names. Counts are
totals across all six; each movie embeds its own copy of the shared `__Packages.*`, so "6" usually
means once per movie. Backing files in `build/agentFF/`: `survey.txt`, `strings.txt`,
`framelabels_all.txt`, `lib_placements.txt`.

### Of the fifteen classes the brief listed, thirteen are referenced nowhere

**Zero occurrences in any of the six movies**, checked by exact-token and by substring:
`Date`, `Sound`, `System`, `System.capabilities`, `SharedObject`, `LoadVars`, `XML`, `XMLNode`,
`Color`, `flash.geom.Point`, `flash.geom.Rectangle`, `ExternalInterface` *(as a bare name — see
below)*, `ContextMenu`, `StyleSheet`, `TextSnapshot`. Also absent: every `flash.filters.*`,
`Accessibility`, `NetConnection`, `LocalConnection`, `Video`, `Camera`, `Microphone`, `PrintJob`,
`mx.transitions.Tween`, `mx.transitions.TweenManager`, and `Error` as a constructor. The near-hits
are all false positives: the literal `"Object.clone won't work on MovieClip or XML"`; `saveDate`;
`SoundHandler` / `PlaySound` / `gfxProcessSound` (movie-defined and a GFx extension — there is no
`Sound` class); `textColor`, `ConvertColorTags`, `SetColorTransform` (movie-defined — there is no
`Color` class).

**Installing any of those thirteen would be exactly agent ES's mistake**: 1,053 of this tree's
placeholders were features retail does not have.

### What is used and absent — the two that are worth the next wave's time

| gap | sites | what it is for |
|---|---|---|
| **`Selection.disableFocusKeys`** | 6, one per movie, `_common.UIBase`'s constructor | a single property write, `Selection.disableFocusKeys = true`. Currently a `setMember` on `undefined`. **I did not install it**: a one-property `Selection` is a shim, retail's class is real and large, and nothing in this runtime reads the property. Evidence recorded so the decision is someone's to make rather than mine by default |
| **`MovieClipLoader`** | 4 `new`, in `_common.ImgLoader` | external image loading, and **reachable**: `MissionsMenu`, Options → gamepad mapping, `_common.ItemListDetails`, `LoadGameList`, `_common.EquipmentIcon`. Needs `new MovieClipLoader()`, `.addListener(obj)`, `.loadClip(targetMc, url)` and the two callbacks `onLoadComplete` / `onLoadInit`. Nothing else — no `unloadClip`, no progress, no error callbacks |
| **`TextField.setImageSubstitutions`** | 6, `_common.EmbedImg::ConvertButtonTags` | not a class but the other missing method: it takes an array of `{subString, image:BitmapData, baseLineY, height, width, _offset}` and is what turns the menus' `$A$`-style button tags into inline glyphs. A GFx TextField extension. Pairs with agent FG's `ParseHtmlImpl` |

### What is used and present, with what is actually touched

* **`flash.display.BitmapData`** — 281 calls, **static `loadBitmap(name)` only**. The object is
  stored and handed to `setImageSubstitutions` and `beginBitmapFill`. On the PC path
  `_common.EmbedImg`'s constructor **falls through** from the PC block into the Xenon block (no jump
  at `MainMenu.as2.txt` offset 2401), so a PC run loads 7 `PC_*` **and** 19 `X360_*` bitmaps — which
  is where the 19 `BitmapData.loadBitmap: no exported image 'X360_* -nopack'` errors in a
  brightness-screen run come from. Those images are exports of **lib.gfx**, not of the importing
  movie, so image exports are not being resolved across the import; that is a separate, real defect
  and it is not mine.
* **`flash.external.ExternalInterface`** — 74 calls, `.call(name, …)` only. The host-call inventory
  is in `agentFF_status.csv`'s neighbour `survey.txt`; `FormatText`, `ShowMessageBox`,
  `OnSettingChange`, `OnResetOptions` and the two dozen `On*Clicked` are the live ones.
* **`flash.geom.ColorTransform`** — 11 `new`, always with 8 arguments, no property reads afterwards.
  **`flash.geom.Transform`** — 2 `new`, ctor plus the `colorTransform` setter, 6 write sites, no
  getter reads. **`flash.geom.Matrix`** — 1 `new`, zero arguments, no methods. All three landed last
  wave (agent FD) and all three are exactly enough.
* **`TextFormat`** — never constructed. The only use is
  `textField.getTextFormat().getTextExtent(str).textFieldWidth`, twice, and `getTextExtent` is a
  Scaleform extension rather than stock AS2. Already installed.
* **`gfx.motion.Tween`** and **`mx.transitions.easing.{Strong,Back,None}`** — **movie-defined**,
  compiled into all six. 374 `mx` reads, all easing functions. Nothing to port.
* `Key` — `isDown` 114, `getCode` 42, `addListener`/`removeListener` 6 each, `_listeners.length` 6;
  none of the `Key.UP`-style constants. `Mouse` — `addListener` 10, `removeListener` 6 only.
  `Stage` — **`Stage.width` only**, two sites; no `height`, `scaleMode`, `align` or `onResize`.
  `Function` — `call` 72 sites, `apply` never. `Math` — `round, floor, ceil, abs, min, max, random`
  only. `Object.registerClass` — 79 calls.
* GFx extensions the content relies on: `_global.gfxExtensions`, `_global.gfxProcessSound` (guarded,
  so harmless when absent), the 3D properties `_z` (117 writes), `_yrotation` (114), `_xrotation`
  (78), and `fscommand` through `getURL "FSCommand:…"` (4 sites).

### MovieClip methods: the complete call census

Used and present: `gotoAndPlay` 113, `attachMovie` 99, `gotoAndStop` 84, `getNextHighestDepth` 130,
`removeMovieClip` 59, `createEmptyMovieClip` 31, `lineTo` 24, `endFill` 6, `moveTo` 6, `beginFill` 5,
`hitTest` 4, `swapDepths` 2, `beginBitmapFill` 1, plus the `play`/`stop` opcodes (173 / 351).

Used and missing before this package: **`setMask` 6** (now ported), **`stopDrag` 1** (now ported),
**`setImageSubstitutions` 6** (still missing, above).

**Not used at all** — do not port for this cook: `startDrag`, `duplicateMovieClip`, `getDepth`,
`localToGlobal`, `globalToLocal`, `getBounds`, `getRect`, `curveTo`, `lineStyle`, `clear`,
`beginGradientFill`, `attachBitmap`, `loadMovie`, `unloadMovie`, `loadVariables`, `nextFrame`,
`prevFrame`, `createTextField`, `getInstanceAtDepth`, `attachAudio`, `getSWFVersion`,
`setTextFormat`, `setNewTextFormat`, `removeTextField`, `replaceSel`, `replaceText`.
Three of those (`startDrag`, `duplicateMovieClip`, `getDepth`) **are** in this package, each because
it is the other half of something that is used; that is stated here rather than buried.

Opcodes absent from all six movies, for the interpreter's benefit: `getProperty`, `setProperty`,
`with`, `try`/`throw`, `castOp`, `implementsOp`, the `startDrag`/`endDrag` **opcodes**,
`cloneSprite`, `removeSprite`, `setTarget`, `targetPath`, `stopSounds`.

## 10. Gates

| gate | result |
|---|---|
| clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`, all three targets | **0 errors, 0 C4263, 0 C4264** (`build/agentFF/clean_build.log`) |
| `rva_sweep.py --path source/Development/Src/External/GFx3` | 668 citations: 244 ok-2013, 419 ok-2013-mid, 5 ok-2012-labelled, **0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013, 0 unknown** |
| every cited address resolved by hand | 24 addresses, each confirmed a function start with its size in both databases, `agentFF_status.csv`. `setMask` and its six depth siblings pinned a **second** way through the `.rdata` prototype method table at 2012 `0xdb35xx`, without `vtables.csv` and without `match_2012_2013.csv` |
| `run_regression.py`, worktree's own copy, absolute build dir, built inside the harness | **37 ok, 0 failed, 0 skipped** (`build/agentFF/regression.log`) |

The five gitignored `resources/docs/types/` inputs were copied into the worktree before the layout
stage, so no metric recorded `-1`.

## 11. Merging

* **Regeneration required: no.** `gen_classes_header.py` was not run and does not need to be — no
  source file was added or removed, no native stub was ported, no `DECLARE_FUNCTION` was touched.
* **`Sources.cmake` changed: no.**
* **Files outside `External/GFx3`: none.**
* **Total: nine source files**, plus this report and `agentFF_status.csv`.
* `build/agentFF_sync.py` is the authoritative copy list, worktree → main, and it **flags rather than
  copies** anything main has changed since `18f36a3`. Its docstring names, per file, every function
  this package touched, because agents FG and FH are in `External/GFx3` too.

Every function touched, by file:

| file | functions |
|---|---|
| `GFxPlayer.h` | new `GFxASDepthBias`; `GFxCharacter` +`SetMask` +`GetMask` +`GetMaskOwner` +`pMaskCharacter` +`pMaskOwner`, **−`bScriptCreated`**; `GFxCharacterHandle` +`NamePath`, `ResolveCharacter` no longer inline; `GFxSprite` +`CloneDisplayObject`; `GFxASCharacter` +`DoMouseDrag`; `GFxMovieRoot` +`DragState` +`GetDragState` +`InitDragCenterDelta` +`Drag`; `GFxTimelineDef` +`GetLabelName` |
| `GFxPlayerSprite.cpp` | `GFxCharacter::GFxCharacter`, `GFxCharacter::~GFxCharacter`, **`GFxCharacter::SetMask`** (new), **`GFxMovieRoot::InitDragCenterDelta`** (new), **`GFxASCharacter::DoMouseDrag`** (new), `GFxASCharacter::~GFxASCharacter`, `GFxASCharacter::CreateCharacterHandle`, `GFxASCharacter::SetName`, **`GFxCharacterHandle::ResolveCharacter`** (new), `GFxSprite::GotoLabeledFrame` (error text only), `GFxSprite::CreateEmptyMovieClip` and `GFxSprite::AttachMovie` (one deleted line each), **`GFxSprite::CloneDisplayObject`** (new). **`GotoFrame`, `IncrementFrameAndCheckForLoop` and `AddDisplayObject` are NOT touched** — those are agent FH's |
| `GFxPlayerRoot.cpp` | `GFxMovieRoot::GFxMovieRoot`, `GFxMovieRoot::Advance` (one added call), `GFxValue::ObjectInterface::CreateEmptyMovieClip`, `GFxValue::ObjectInterface::AttachMovie` |
| `GFxDisplay.h` | `GFxDisplayContext` +`StageMatrix` |
| `GFxDisplay.cpp` | `GFxDisplayContext::GFxDisplayContext`, **`GFxDisplayPushScriptedMask`** (new static), `GFxSprite::Display`, `GFxMovieRoot::Display` (one added line). `GFxDisplayList::Display` untouched |
| `GFxHitTest.cpp` | `GFxSprite::PointTestLocal` only, plus one `#include` |
| `GFxAS2Lib.cpp` | `MCCreateEmptyMovieClip`, `MCAttachMovie`, `MCRemoveMovieClip`, `MCGetNextHighestDepth`, `MCGetDepth`, **`MCSetMask`**, **`MCStartDrag`**, **`MCStopDrag`**, **`MCDuplicateMovieClip`** (four new), and the `MovieClip.prototype` registration block |
| `GFxAS2Interp.cpp` | the `GASop_DuplicateClip` and `GASop_RemoveClip` arms only |
| `GFxDrawing.cpp` | `GFxDrawingInstall` only — one `__constructor__` write in the `BitmapData` block and one in the `TextFormat` block. Agent FG is in `GFxStyledText` / `GFxTextField` / `GFxTextDocView`; none of those is this file |

## 12. Where this brief was wrong

In waves 16 and 17 every package corrected a premise of its own brief. This one corrects four.

1. **Item 6 is wrong in whole.** The `"PC"` branch is taken, it is the right branch, it succeeds on
   the clips it is for, and the ~40 errors are an asset bug in a shared glow clip that retail logs
   identically. Nothing to fix. §7.
2. **Item 2 is one method, not three.** `tweenTo` is installed by the asset; `UpdateSettings` is
   movie-defined. Both failures are a class-binding problem in the options list, not a library gap.
   §5.
3. **Item 3 is a real defect with no symptom.** Neither `BitmapData` nor `TextFormat` is ever
   constructed by this cook, so the missing `__constructor__` costs nothing today. §4.
4. **Fault 1's "the rows' value widgets are missing that chrome" is not a fault** — the `<` `>`
   arrows are drawn. §3.1.

And one correction to my own first attempt, caught by reading retail rather than trusting Flash's
documented behaviour: GFx does **not** hide a clip that is serving as a mask, so the early-out I
wrote for it was an invention and was deleted. §2.2.
