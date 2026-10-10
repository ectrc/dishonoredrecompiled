# Wave 18 — the ActionScript library, the options list, and getting into the game

Five more faults from the user, with screenshots in `resources/reference/menu/1.png` … `5.png` — **ours
is the left window in every one, retail the right**, same window size and same assets. Plus a standing
instruction that sets this wave's shape:

> add re implementing all missing action scripts into the next plan

That is the right call and it is overdue. Three of the five faults below are almost certainly the
*same* missing AS2 surface seen from different screens.

## THE CAPTURE RULE FOR THIS WAVE — read before running anything

**The user's monitor is off.** Every screenshot must be taken **inside the game process**:

- **`-apshottime=<seconds>`** (and `-apshottime=a,b,c` for several marks, `-apshotname=<tag>` to claim
  the file) reads the frame back off the render target at the end of `UGameViewportClient::Draw`. It
  does not touch the desktop, the compositor or any window, so **a dark or absent monitor cannot
  affect it**. This is already the standard instrument — agents FC, FD and FE all used it.
- **Never use a desktop or window capture** — GDI, `BitBlt`, `PrintWindow`, PowerShell screen grabs.
  Those read the composited desktop and will return black, stale or another window's pixels with no
  display attached. The coordinator's own `build/coord_shot.py` is in that category: **do not use it.**

**Verify this before you rely on it.** The first thing each package should do is take one
`-apshottime` capture and confirm the PNG has real content. If it does not, **stop and report it** —
do not spend the package working around it. A monitor that is merely powered off usually leaves the
desktop and the D3D9 device intact, but some monitors drop the link entirely when switched off, which
removes the display from Windows and can lose the device. If that is what has happened, say so: it is
a fact about the machine, not a defect in your package, and the coordinator needs to know.

Real-input driving with `drive_input.py` still works without a monitor — `SendInput` does not need
one — but a **locked session** would block it. If keys stop arriving, check for that before blaming
the build.

## The five faults

### 1 — `1.png`: the white panel is attached to the list container, not the row area

On the AUDIO tab, retail's light panel sits behind the whole row area and every row's value widget
carries its arrow chrome, with `<` `>` on the selected row. Ours has the panel bound to what looks
like the scroll list's own container, and the rows' value widgets are missing that chrome.

### 1b — no screenshot: the list overflows and scrolls over everything above it

The user: *"the content overflows and when scrolling down the list it will just go over the other
elements above."* **This is the strongest clue in the whole wave.** A list that draws outside its own
bounds and over its neighbours is a list **whose mask is not applied** — and
**`MovieClip.setMask` is not registered on our AS2 prototype at all.** Retail's is 2012 `0x9f5320`.
Agents EZ, FC and FD each noted it and each left it, because it never blocked what they were doing.

It is very likely that 1, 1b and part of 2 are one missing method.

### 2 — `2.png`: hovering a setting removes every setting's content

Ours, after a hover: the entire right-hand column is empty — every slider, toggle and value gone, on
all six rows at once. Retail keeps them. "All of them at once, on hover" points at a shared container
being hidden, emptied or masked out, not at six independent widgets failing.

### 3 — `3.png`: the modal's text formatting is broken

Two defects in one line. Retail: `Do you want to restore settings of the current category (Audio)?`
with `Audio` in its own colour. Ours: `…current category (?CATEGORY?)?` — **the token is never
substituted**, and **the per-run colour is not applied**.

The colour half is known: `GFxStyledText::ParseHtmlImpl` (2013 `0xa9d870`, 6,268 bytes) is unported.
Agent FB ported only the `<font face>` read out of it; `size`, `color`, `<b>`, `<i>`, `<a>`, `<img>`
and all per-run formatting are still dropped. The `?CATEGORY?` half is a separate question — find out
whether retail substitutes it in C++ or in ActionScript before assuming either.

### 4 — `4.png`: the difficulty portrait is still in the wrong place

Already root-caused by agent FC and **not** the `attachMovie` fault: `_corvo_mc` and `newGame_mc` are
timeline clips, never attached, and their numbers are identical to the last digit before and after
that fix. On EASY the portrait is where retail's is; on VERY HARD it is about 140 px right, 255 px down
and smaller, while **the rest of the clip, the difficulty rows and the title do not move at all**
(`build/agentFC/ng_frames.png`). The one thing the difficulty changes is
`portrait_mc.gotoAndStop(idx+1)` on a four-frame clip. It is a **sprite-timeline fault** —
`GotoFrame` / `IncrementFrameAndCheckForLoop` / `AddDisplayObject`'s same-character early-out — one
click to reproduce, with the rest of the screen as its own control.

### 5 — `5.png`: the game starts, but nothing is in it

Ours loads the boat and then just sits there: **no cutscene, no pawns, no black bars.** Retail's half
shows the two NPCs on the boat, the letterbox bars, and the opening scene playing.

Three faults, and they may not share a cause:

- **The pawns are not placed.** Nothing is spawned into the level at all.
- **The matinee does not start.** The Kismet side exists; agent FE confirmed `bCinematicMode` reads 1
  on our menu already, so the cinematic *flag* path works.
- **The bars do not draw.** FE named this end to end: `execSetCinematicMode_Native` (2013 `0x5eea00`)
  and `execPreSetCinematicMode_Native` (`0x5ee960`) are stubs; `SetCinematicMode_Native` (`0x6af150`)
  draws nothing itself but swaps `ADishonoredHUD`'s mask level between 32687 and 24592, and **the one
  bit exclusive to the cinematic set is `0x10`**; `UDisGFxMoviePlayerHUD::Tick_Cinematic` (`0x7960e0`)
  turns that bit into `_root.blackStripes_mc` and is unported; and the HUD movie only opens behind
  `-dishud`.

FE stopped at one specific thing and it is the first thing to settle: **`execSetCinematicMode_Native`
reads nine booleans and an optional tenth off the script stack, where stock UE3's `SetCinematicMode`
reads six**, and no truth source in this tree names them. A wrong count corrupts the stack.
**Read them out of the running retail game with dismod** rather than guessing — that is exactly what
the harness is for (`resources/docs/dismod_harness.md`).

## Packages

### FF — the ActionScript library

The user's instruction, and the backlog behind three of the faults above. Our AS2 **interpreter** is
complete — all 98 opcodes in the enum are handled. What is thin is the **class library**: 18 classes
are installed (`Object Array String Number Boolean Function Math MovieClip TextFormat Key Mouse Stage
Error AsBroadcaster BitmapData Matrix ColorTransform Transform`), the last three only landed last wave.

Deliver, in this order:

1. **`MovieClip.setMask`** (retail 2012 `0x9f5320`) — the likeliest single cause of fault 1b and a
   contributor to 1 and 2. Do this first and **re-measure those three faults against it** before
   anything else, the way agent FC did with `attachMovie`.
2. **The methods the live log is missing**, measured on a real menu run: `stopDrag` (raised on the
   gamma slider's thumb), `tweenTo`, `UpdateSettings`.
3. **`BitmapData` and `TextFormat` set no `__constructor__` on their prototypes** — the same latent
   defect that made `Transform` silently allocate as a plain `GASObject` until agent FD caught it, and
   which produces *no error at all*.
4. **The depth convention.** Retail places a script-created clip at AS depth **+0x4000**
   (`attachMovie` 2012 `0x9ff990`, `duplicateMovieClip` `0x9f5210`), `getDepth` returns internal
   **−0x4000** (`0x9cde60`), `getNextHighestDepth` returns `max(0, largest − 0x3FFF)` (`0x9f5510`) and
   `removeMovieClip` tests `>= 0x4000` (`0x9f6c00`). Ours applies no offset anywhere except
   `GASop_DuplicateClip`, which **subtracts**. Agent FB's retail capture confirms the convention live.
   Agent EZ worked around it with a flag; that flag should go when this lands.
5. **`GFxCharacterHandle::ResolveCharacter`** is a stub returning its argument.
6. **The `"PC"` branch of `_common.ControllerButtonIcon.SetIcon` is never taken**, which is about
   forty `no frame named 'PC'` errors per run — the single largest source of AS2 noise in the log.
7. **Then survey what else is absent and say so.** An AS2-5 runtime also has `Date`, `Sound`,
   `Selection`, `System`/`Capabilities`, `SharedObject`, `LoadVars`, `XML`/`XMLNode`, `Color`,
   `Point`, `Rectangle`, `ExternalInterface`, `ContextMenu`, `MovieClipLoader`, `StyleSheet`,
   `TextSnapshot`. **Do not install all of them blindly.** Find which ones the cooked movies actually
   reference — the disassembler in `build/agentEY/swf/` reads them — and report the list with what
   each is used for, so the next wave ports from evidence.

This package owns `External/GFx3`.

### FG — the options list, and the HTML text

Faults 1, 1b, 2 and 3.

**Sequence with FF**: `setMask` is FF's and lands first. Re-measure 1, 1b and 2 against it, report what
it fixes, and fix what it does not. If FF has not landed when you need it, say so.

Fault 3 is yours in both halves: port **`GFxStyledText::ParseHtmlImpl`** (2013 `0xa9d870`) so per-run
formatting works, and separately find out where `?CATEGORY?` should be substituted — C++ or
ActionScript — and make it happen. Those are different mechanisms and should be reported separately.

### FH — the difficulty portrait

Fault 4, and it is bounded: one clip, four frames, one click to reproduce, the rest of the screen as a
control. Agent FC named the three suspects — `GotoFrame`, `IncrementFrameAndCheckForLoop`, and
`AddDisplayObject`'s same-character early-out — and said it needs a tag-level dump of those four
frames, which does not exist. Produce that dump, then fix it.

### FI — into the game

Fault 5: the pawns, the matinee, and the bars. Take them in that order — **a cutscene with nothing in
the level to film is the less useful half.**

For the bars specifically: **use dismod to read the nine booleans off retail's live script stack**
rather than guessing the count. `resources/docs/dismod_harness.md` has the loop; agent FB's
`build/agentFB/retail/` has the captures and the probe's command channel.

## Rules for agents

- **Capture in-process only. See the capture rule at the top of this file.**
- Work **only** in your own worktree. Your sync script runs **worktree → main** and **flags rather
  than copies** anything changed in main since your base.
- **Prefer a measurement in the real game over an argument about it.** Two waves running, dismod has
  either confirmed a fix or overturned a wrong conclusion.
- **Resolve every retail address yourself.** `rva_sweep.py` is necessary but not sufficient.
  `vtables.csv` is the **2012** table; `match_2012_2013.csv` has propagated an entirely wrong vtable.
  Pin a vtable a second way. Remember **`+ 0x400000`** — an address resolved without the image base
  lands mid-function in an unrelated class, which is a convincing-looking wrong answer.
- **Never use the FModel MCP tools.**
- `run_regression.py` from **the worktree's own copy** with an **absolute** build dir, built inside the
  harness; the full set is **37**. Copy the five gitignored `resources/docs/types/` inputs in first.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** In waves 16 and 17
  every single package corrected a premise of its own brief, and two overturned a conclusion the
  coordinator had already written down.

## Outcome

### FF — merged `63c2539`, gated 37 ok / 0 failed / 0 skipped

Nine files, all in `External/GFx3`. No regeneration, `Sources.cmake` unchanged.

**Landed.** `MovieClip.setMask` end to end — the AS2 thunk (2012 `0x9f5320`), storage
(`GFxSprite::SetMask` `0x9f4c20`), the render pass (`PreDrawMask` `0xa5f2f0` / `PushAndDrawMask`
`0xa5f510` / `PopMask` `0xa5f5a0`, pushed with the **mask's own parent's** world matrix) and the hit
test (`PointTestLocal` `0x9fba50`). The AS depth bias at every AS boundary and both
`GFxValue::ObjectInterface` entry points, confirmed three ways (disassembly, FB's live retail
capture, a census on our own build); `GASop_DuplicateClip` was wrong twice and now clones;
**agent EZ's `bScriptCreated` is gone**. `stopDrag` with the drag state it needs.
`GFxCharacterHandle::ResolveCharacter`'s real body. `__constructor__` on `BitmapData`/`TextFormat`.

**Fault 1b — FIXED and measured.** Options → CONTROLS → KEYBOARD MAPPING, same schedule both
builds: HEAD draws 4 of 12 list rows outside the mask, over the GENERAL tab and the sub-tab bar;
this build draws 8 and none outside (`build/agentFF/mask_band.png`).

**Four premises of the brief above are wrong**, each corrected with a measurement:

- **Item 6 is wrong in whole.** The `"PC"` branch *is* taken and is the right branch — the error
  text naming `'PC'` is itself proof that `_global.PlatformName == "PC"` — and it succeeds on all
  seventeen `ControllerButtonIcon` symbols. The receiver is a different clip: six lib sprites carry
  a 66-byte frame-0 `DoAction` decoding to `stop(); this.gotoAndStop(_global.PlatformName);` on
  clips with no platform labels. **An asset bug retail shares. Nothing to fix.** The lines now name
  the labels so no one spends another package here.
- **Item 2 is one method, not three.** `tweenTo` is assigned by `gfx.motion.Tween`'s own
  constructor; `UpdateSettings` is movie-defined on `VideoSettings`/`OptionsScreen`. Both fail the
  way the **127 `SetWidgetFocus`/`RemoveWidgetFocus` on undefined** do — a `registerClass` /
  prototype-binding fault in the options list. **FG's, and the largest AS2 failure left.**
- **Item 3 is latent with zero symptom**: `BitmapData` is only the static `loadBitmap` (281 calls,
  no `new`), and `new TextFormat` appears nowhere.
- **Fault 1's "missing arrow chrome" is not a fault** — the `<` `>` arrows draw on both builds. The
  panel's extent is identical on both and stays open. **Fault 2 did not reproduce on either build**
  in four matched runs, including over the column the user's pointer is on.

**The survey, as evidence.** Thirteen of the fifteen classes listed above are referenced **nowhere**
in any of the six movies; installing them would repeat ES's 1,053-placeholder mistake. Used and
absent, for the next wave: `MovieClipLoader` (4 `new` — Missions, gamepad mapping, save thumbnails,
`EquipmentIcon`), `TextField.setImageSubstitutions` (6 sites, pairs with FG's `ParseHtmlImpl`),
`Selection.disableFocusKeys` (6 sites, one property write, deliberately not installed). Separately:
the 19 `X360_*` `loadBitmap` failures are `_common.EmbedImg`'s PC branch falling through into the
Xenon block, and those images are exports of `lib.gfx` — **image exports are not resolving across
the import**. Real, separate.

**Instrument defect, everyone's problem.** FD's runner claims a capture by "arrived after my
launch". That is no longer sufficient: FF's first Options run claimed **nine** bitmaps belonging to
FH and FI. **`-apshotname=<tag>` is the only reliable claim.** FG, FH and FI were told, and told to
re-check anything already concluded from a shot claimed by arrival time.
