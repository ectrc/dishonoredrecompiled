# Agent EH — the mouse on the menu: the defect did not reproduce, and the driver is why (2026-09-29)

The user, the oldest open report in the tree:

> "There is an issue with the main menu, when I try to hover and click over a button it does nothing,
> I have to use the keyboard; and then when interacting with the buttons the first time the white
> background is applied properly but any time re-activating the hover state just breaks the hover
> effect."

| | |
|---|---|
| Package | the mouse's per-frame half of `GFxMovieRoot::ProcessInput`, and the measurement that settles what was actually wrong |
| Starts at | HEAD `84b0c9d`, worktree `build/agentEH_wt` |
| Files | **3**, all in `External/GFx3`, plus this report and `agentEH_status.csv`. No new file, `Sources.cmake` unchanged, no generated file |
| Regression | §8 |
| Clean build | §8 |

**The headline is a correction, and it is the honest result: at HEAD `84b0c9d` the mouse already
hovers and clicks the main menu.** Agent EG's contrary measurement, which this brief is built on, is
an artifact of the driver. §2 reproduces EG's screenshot on this HEAD and then shows the schedule that
makes the same binary work. What was genuinely missing is a different thing, named in §4: retail
regenerates the mouse's button events **once per frame from the stored pointer position**, and this
tree only ever generated them from a queue entry. That half is now ported.

## 1. Result

| Accept | State |
|---|---|
| a screenshot of a menu entry highlighted **by the mouse alone** | **`build/agentEH/eh_final_hover_quit_3rd.png`** — QUIT GAME, white background, **the third time that entry was hovered in the run**, on the final binary (`f6`). Also `eh_final_hover_quit.png`, `eh_final_hover_missions.png`, and at 1600x900 from `f4`: `eh_hover_quit_4th.png`, `eh_hover_options.png`, `eh_hover_missions.png`, `eh_hover_quit_1st.png`. Real `SendInput` absolute moves, nothing forced visible |
| a screenshot of the screen the click reached | **`build/agentEH/eh_click_reached_options.png`** — the Options screen, from a real left click on OPTIONS. The log carries the chain: `APressed` -> `TransitionTo (OptionsScreen)` -> `fscommand (ToOptionsScreen)`. From the `f4` run; §3.3 says why the three later attempts to repeat it lost their click to a firewall dialog |
| the hover survives repetition, at least ten transitions, with the state each time | **23 selection changes over 12 hovers** in one run (§5, `build/agentEH/f4_log.txt`), `24 rollOver / 23 rollOut, 1 press / 1 release, 24 handlers invoked`. The selected entry's local scale is 1.100 at every one and its width is **256 at every one of the 376 bar-census lines in the run** |
| a before/after on the untouched HEAD executable, same driver | **§4.3.** Pointer parked on QUIT GAME while the bar animates in: HEAD **45 hit tests / 0 targets resolved / 0 rollOver**; this package **9006 / 7399 / 4 rollOver**. And §2's before/after on the *driver*, which is the one that answers the brief's premise |
| `run_regression.py`, own build dir, built inside the harness | **§8** |
| clean full release build, build dir deleted, `DISHONORED_LAYOUT_CHECKS=ON` | **§8** |
| `rva_sweep.py` over every file touched | **595 citations in `External/GFx3`, 0 MISLABELLED, 0 UNKNOWN-CLAIMED-2013** (§8) |
| report + `agentEH_status.csv` | this file; 14 rows |

## 2. The brief's premise, reproduced and then dismantled

The brief states, from agent EG: *"the pointer reaches QUIT GAME and the game draws its own cursor on
it, and the selection does not move and the click does nothing."*

**That reproduces on this HEAD.** EG's run 6 schedule is one mouse move to `0.838,0.796` and one click
there. Driven at HEAD with that schedule (`build/agentEH/base2_log.txt`, `base3_log.txt`):

```
mouse: 2 hit tests / 0 targets resolved (3 shape walks, 0 button hits, 0 sprite hits),
       0 rollOver / 0 rollOut, 0 press / 0 release, 0 handlers invoked
```

and the screenshot is EG's: `build/agentEH/ehb2_t01600004.png` beside `build/agentEG/eg6_t01600004.png`
— the cursor sitting on QUIT GAME, NEW GAME still white.

The two `ProcessMouse` entries in that whole run are the click, and they arrive at **(0.0, 0.0)**:

```
ProcessMouse idx 0 at (0.0,0.0) px buttons 1 changed 0   <- the press
ProcessMouse idx 0 at (0.0,0.0) px buttons 1 changed 128 <- the release
mouse hit test at (0.0,0.0) px -> NONE
```

which is agent DQ's fault 1.1 exactly — a click delivered at the origin because the interface's mouse
position was never updated. **Not one `MouseMove` reached the movie.** The `axis probe` line, which
counts every `FGFxEngine::InputAxis` call, does not appear once in the run: `UGameViewportClient::InputAxis`
was never called, so `FGFxEngine::MousePos` never left (0, 0).

### 2.1 Why: one move step is exactly one DirectInput delta, and a single delta can be dropped

The driver every agent in this tree has inherited moves the pointer with two `SendInput`
`MOUSEEVENTF_ABSOLUTE` events **to the same point**, 0.12 s apart. The second has a delta of zero, so a
`move` step produces **exactly one** buffered DirectInput delta. Four `move` steps, HEAD,
`build/agentEH/base4_log.txt`:

| move | axis calls after it | selection |
|---|---|---|
| 1 -> NEW GAME | **0** | unchanged |
| 2 -> QUIT GAME | 1 | 0 -> 3 |
| 3 -> MISSIONS | 2 | 3 -> 1 |
| 4 -> QUIT GAME | 3 | 1 -> 3 |

`4 moves, 3 axis calls` — **the first delta is lost**. `UWindowsClient::ProcessInput`
(`WinClient.cpp:646`) calls `FlushMouseInput()` whenever the focused viewport client is NULL, and
`PollMouseInput` re-`Acquire`s the device whenever `Poll` fails, which also empties the buffer. Either
one eats whatever single delta is in flight while the window's focus settles. It is stock UE3 and it is
in retail too; a real hand produces dozens of deltas a second, so a person never sees it — **but a
driver that sends one delta per hover sees nothing at all**.

The game's own cursor is not evidence against this, and that is the trap EG's screenshot set:
`FGFxEngine` moves `_root.mouseCursor_mc` every frame from `Viewport->GetMousePos()`
(`gfxuiengine.cpp:2806`), which is the Windows cursor position and is completely independent of the
GFx mouse queue. **The cursor can sit on QUIT GAME while the interface believes the pointer is at the
origin.**

### 2.2 The same binary, driven the way a hand drives it

`build/agentEH/drive.py` gains a `hover:<fx>,<fy>` step: twelve `SendInput` moves over ~0.3 s along the
path, which is what a mouse actually produces. Thirteen of them, HEAD, no code change of any kind
(`build/agentEH/base5_log.txt`):

```
bar selection 0 -> 3 -> 2 -> 1 -> 0 -> 1 -> 2 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0 -> ...   (24 changes)
mouse: 166 hit tests / 109 targets resolved, 24 rollOver / 23 rollOut, 24 handlers invoked
```

Every hover lands, **including the first**, and the selected entry is `scale (1.100,1.100) w 256 |
bkgdOver a 100` at every one of the 332 bar-census lines, with the other three at `1.000 / a 0`. A
click in the same family of runs opens the screen it points at (`base1`: OPTIONS,
`build/agentEH/ehbase_t03000005.png`).

**So the user's first sentence and the user's second sentence were both fixed by agent DQ and are both
still fixed.** The report the brief quotes predates `bb05b9d`. I say so plainly because the brief asks
for that, and because the alternative was to write a package around a defect that is not there.

## 3. The environment made three of my own measurements lie, and it will make others lie

Three hazards, all found the hard way; the first two are now handled in `build/agentEH/drive.py`:

1. **Agents EI, EJ and EK are running their own copies of this game on this desktop at the same time,
   and their windows carry the same title.** One acceptance run of mine was driven against another
   agent's 1920x1080 window and reported `0 hit tests` — a dead mouse, on a build whose mouse works.
   The driver now resolves the window through `tasklist` against **this agent's own image name**.
   Their `SendInput` still lands in whichever window holds the foreground: one of my runs picked up a
   stray `APressed` that opened the New Game confirmation box, and `f3` left the menu for the Missions
   screen at 48 s for the same reason. Any measurement in this wave that depends on real input needs
   its drive log and its AS2 trace read for interference before it is believed.
2. **An elevated Task Manager held the foreground for three consecutive runs**, and
   `AttachThreadInput` is refused across that integrity boundary, so `SetForegroundWindow` failed and
   every `SendInput` went elsewhere. `activate()` now retries with a synthetic ALT tap, which releases
   the system's foreground lock without touching the other window.
3. **A `Windows Security Alert` firewall dialog** (`rundll32`, raised by another agent's game binding
   a socket) took the foreground and **swallowed the click of three consecutive runs** — `f5`, `f6`
   and `f7` each show `foreground 0xd40f82` at the `hoverclick` step and `0 press / 0 release` in the
   census, while every hover in the same runs landed. I did not dismiss it: it is a security prompt
   and it is the user's to answer. In the same window of time another agent's game changed the
   display mode, so my own window came up **1008x567 instead of 1600x900** — the driver's fractions
   are of the live client rect so the hovers still landed, but the screenshots of those runs are that
   size.

## 4. What was actually missing: the second half of `GFxMovieRoot::ProcessInput`

Every address resolved against my own copy of the database, `build/agentEH_ida/retail2013_agentEH.i64`,
headless through `resources/tools/ida/run.py`.

`GFx_GenerateMouseButtonEvents` (**2013 `0xa5ad10`**) has **two** callers in retail, not one:

```
0xa05330  GFxMovieRoot::ProcessMouse     (at 0xa057ac)   <- this tree had it
0xa077d0  GFxMovieRoot::ProcessInput     (at 0xa07945)   <- this tree did not
```

and so does `GFxMovieRoot::GetTopMostEntity` (`0x9fabf0`). Retail's `ProcessInput` (**`0xa077d0`**),
after draining the queue:

```c
if ( (this[9316] & 0x80) != 0 && (processed & allMice) != allMice )
    for ( i = 0; i < MouseCursorCount; ++i )
        if ( !(processed & (1 << i)) && (state[i].Flags & 0x10) )   // Flag_Updated
        {
            state[i].PrevButtons = state[i].CurButtons;
            top = GetTopMostEntity((state[i].X, state[i].Y), i, false, 0);
            state[i].SetTopmostEntity(top);
            GFxMovieRoot::CheckMouseCursorType(this, i, top);
            GFx_GenerateMouseButtonEvents(i, &state[i], ...);
        }
this[9316] &= ~0x80;                                                // 0xa0797c
```

`processed` is retail's third parameter to `ProcessMouse`, whose first line is `*a4 |= 1 << entry[16]`
— the mask of mouse indices a queue entry spoke for this pass. Bit `0x80` of the movie root's flag word
at `+9316` has exactly one writer, **`GFxMovieRoot::Advance` at `0xa088d8`**, at the tail of the
frame-advance loop and *after* Advance's call to `ProcessInput` at `0xa087dd`. So the meaning is: *the
display list has advanced since the last input pass; ask again what is under each pointer.* One frame
behind the display, which is retail's own phase and not a rounding of it.

`Flag_Updated` is `state+32 & 0x10`, set unconditionally by `GFxMouseState::UpdateState` (`0x9fb950`);
the neighbouring `0x08` is the position-moved bit. This tree's `GFxMouseState` enum already carried
both values correctly — agent DQ read them right — it simply had nothing that read `Flag_Updated`.

### 4.1 What this tree did instead, and what it cost

Button events were generated **only** from a queue entry, and a queue entry only ever reports the
pointer *moving*. So a display list that changed under a stationary pointer produced nothing: a button
that slides under the cursor, a bar that fades in under it, a clip that is attached under it. The whole
of the menu's opening animation happens under a pointer the player has not moved.

### 4.2 The three files

* `GFxInput.cpp` — `ProcessInput` gains retail's second arm and the processed-mouse mask; `ProcessMouse`
  gains retail's third parameter and its first line.
* `GFxPlayer.h` — `GFxMouseState::IsUpdated()` and `CarryButtons()`, the two things retail reads and
  writes inline on a struct whose fields are public there and private here; `ProcessMouse`'s signature;
  and `GFxMovieRoot::bMouseStateDirty`, retail's bit `0x80`.
* `GFxPlayerRoot.cpp` — `Advance` sets that flag at its tail, and the constructor initialises it.

### 4.3 The before/after, same driver, same schedule, two binaries

The pointer is put on QUIT GAME **while the start screen is still up**, jiggled four more times so the
position is certainly registered, and then the start screen is dismissed with a real key press and the
mouse is **not touched again**. `build/agentEH/pk2_before_log.txt` is the untouched HEAD executable
(`build/agentEH_baserel`, built from this worktree with the three files reverted to HEAD);
`pk2_after_log.txt` is this package (`build/agentEH_playrel`). Same driver, same steps, each bound to
its own process.

| with the pointer parked on the entry, over the whole run | HEAD | this package |
|---|---:|---:|
| mouse hit tests | 45 | **9006** |
| targets resolved | **0** | **7399** |
| button hits | 0 | 7399 |
| rollOver dispatched | **0** | **4** |
| handlers invoked | 0 | 4 |

HEAD asked what was under the pointer **forty-five times, all of them before the menu bar existed**,
and then never again. This package asks once a frame, finds the entry, and dispatches the `onRollOver`
retail dispatches as each entry sweeps under the cursor during the open animation.

**What it does not do, and I am saying so rather than letting the table imply it:** the *visible*
selection at the end of that run is still NEW GAME in both. `_common.SelectionHandler::RollOver(idx)`
is guarded by `_handlerEnabledState && _mouseEnabled && inputs.InputsEnabled` (char 257 of the menu's
init actions, the `RollOver` body at pc 3933), and during the open animation those are not all true, so
the four correct events land on a handler that is deliberately ignoring them. The defect this package
fixes is that the events were **not generated at all**; whether the content acts on them is the
content's business and it is the same in retail. An earlier run of the same test
(`build/agentEH/f_park2_log.txt`) did land it — `bar selection 0 -> 3`, the parked pointer taking QUIT
GAME — so the outcome is timing-dependent on this asset.

### 4.4 The cost

One `GetTopMostEntity` per movie per frame. Measured on the same twelve-hover schedule, drawn frames
between the first and the last selection change: **224.6 fps at HEAD, 203.5 fps with this package**, a
9% frame cost on the menu, for 17,148 hit tests over 100 s of which 1,551 reach a shape's winding walk
(a button's `hitTest` record answers before them). It is what retail pays.

## 5. The repetition acceptance, in full

One run, twelve `hover` steps, the four entries in a rotation that returns to each of them three or
four times, and then a click (`build/agentEH/f4_log.txt`, drive log `f4_drive.txt`):

```
bar selection 0 -> 2 -> 3 -> 2 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0 -> 1 -> 2 -> 3 ->
               2 -> 1 -> 0 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0 -> 1 -> 2            (23 changes)
mouse: 17148 hit tests / 16931 targets resolved (1551 shape walks, 16931 button hits, 0 sprite hits),
       24 rollOver / 23 rollOut, 1 press / 1 release, 24 handlers invoked
AS2 trace: > APressed () called -> >> TransitionTo (OptionsScreen) -> > fscommand (ToOptionsScreen)
```

and over all 376 bar-census lines of that run:

| | value |
|---|---|
| local scale while the white background is at alpha 100 | `1.097 .. 1.110` (authored 1.100; the tween's ease-out overshoots to 1.110) |
| local scale while it is not | `1.000 .. 1.002`, and `0.801 .. 0.958` during the opening animation before the bar settles |
| **every `_width` seen, selected or not, entry or background** | **256** |

Agent DQ's broken run for comparison read `scale (2.500,2.500)` and widths creeping `256 -> 280 -> 299
-> 300`. There is no compounding at the tenth transition, the twentieth or the twenty-third.

Thirteen screenshots were taken automatically, one per selection change, with `-gfxuishotonhover=200`;
four are copied into `build/agentEH/eh_hover_*.png` and the click's screen into
`eh_click_reached_options.png`.

`f4` was driven against a binary that predates this package's last comment edit (§8.2). **`f6` is the
same schedule on the final binary** — `build/agentEH/f6_log.txt`, 13 selection changes over 6 hovers,
`13 rollOver / 12 rollOut, 13 handlers invoked`, nine automatic screenshots of which three are copied
to `build/agentEH/eh_final_hover_*.png` — and `f5` is a twelve-hover run on the same binary with
**24** selection changes and `25 rollOver / 24 rollOut`. Neither of those two could complete its click
(§3.3), which is why the click screenshot is `f4`'s.

## 6. A real fault that is NOT a defect, measured and then argued down

`build/agentEH/base6_log.txt`, HEAD: the pointer hovers QUIT GAME (selection 3), the **keyboard** then
moves the selection to 1, and three further real mouse motions *inside QUIT GAME* follow.

```
692 hit tests / 666 targets resolved, 666 button hits ... 6 rollOver
```

The hit test resolves that entry's own button six hundred and sixty-six times and **not one rollOver is
generated**, so the entry cannot take the selection back until the pointer leaves it and returns. Read
literally, that is the user's second sentence, and it is the first thing I tried to fix.

**It is retail's behaviour.** `GFx_GenerateMouseButtonEvents` (`0xa5ad10`) ends with
`if ( !CurButtons && topmost != active )`, and agent DQ's port is that condition exactly; when the
pointer has not left the entry, `topmost == active` and there is nothing to dispatch. Adding §4's
per-frame arm does not change it and was not expected to — `build/agentEH/f1_log.txt` is the same run on
the patched build and the desync is still there, which is the right answer. The only thing in retail
that could resynchronise is the trackAsMenu arm (the character flag at `+160` bit `0x2000`, on the
release and drag paths), and this cook's one `DefineButton2` has `trackAsMenu 0`.

So: a Flash menu whose selection the keyboard moved does not follow a mouse that never moved, in retail
or here. Recorded, not "fixed". If the user wants it to, that is a content-level change and it should be
a decision, not a silent deviation.

## 7. Deviations, stated once

1. **`GFxMovieRoot::CheckMouseCursorType` is not called from the new arm.** Retail calls it between
   `SetTopmostEntity` and `GFx_GenerateMouseButtonEvents`. It is the hand cursor, and Dishonored's
   cursor is a movie clip the global movie attaches — agent DQ's deviation 1 and hand-over 3, unchanged.
2. **`buttonCount` is 1**, as in `ProcessMouse`: retail's `(*(pGC+684) - 1) != 0 ? 1 : 16`, and this cook
   does not set the extended-clip-event flag (agent DQ deviation 5).
3. **The loop bound is `min(MouseCursorCount, MaxMice)`.** Retail's is `MouseCursorCount` with no clamp
   because its state array is allocated to that count; this tree's is a fixed array of four.
4. **`GFxMouseState::IsUpdated()` and `CarryButtons()` are accessors retail does not have** as
   functions — it reads `state+32 & 0x10` and writes `state+16 = state+12` inline, because the fields
   are public there. Same two operations, same place.
5. **The arm drains the action queue after each mouse index**, as the queue loop above it already does,
   for the reason that loop states. Retail's action model differs and it drains in `Advance`.

## 8. Verification

* **Every build and every measurement is in the snapshot worktree `build/agentEH_wt`** (HEAD `84b0c9d`
  plus this package's three files and nothing else). `git -C build/agentEH_wt diff --stat` is exactly
  those three, and `agentEH.md` / `agentEH_status.csv` are the only untracked additions.
* **`rva_sweep.py`**, run from the worktree's own copy over `External/GFx3` (which is every file this
  package touches): **595 citations, 200 `ok-2013`, 390 `ok-2013-mid`, 5 `ok-2012-labelled`,
  0 `MISLABELLED-2012`, 0 `UNKNOWN-CLAIMED-2013`.** Agent EG's sweep of the same directory reported 589;
  the six new ones are this package's.
* **The driver is real Windows input**: `SendInput` with `MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE |
  MOUSEEVENTF_VIRTUALDESK` for the pointer and `SendInput` keyboard events for the keys, into the game's
  own window, with the schedule keyed to a log line rather than to the clock. `SetCursorPos` is not used
  anywhere: `UWindowsClient::ProcessInput` reads the mouse from DirectInput8 and `SetCursorPos` puts
  nothing in that buffer (agent EA's finding, and §2.1 is why it matters).
* **No generated file was touched.** Nothing this package changes is produced by `gen_classes_header.py`,
  so **no regeneration is required and none was run**. `Sources.cmake` is unchanged: no file added or
  removed.
* **IDA: own copy only**, `build/agentEH_ida/retail2013_agentEH.i64`, a copy of
  `resources/docs/idb/retail2013_named.i64`, opened headlessly through `resources/tools/ida/run.py`.
  **No FModel tool was used.**

### 8.1 Clean full release build

`build/agentEH_layoutrel`, **deleted first**, configured from the worktree with
`-DDISHONORED_LAYOUT_CHECKS=ON` and built for every target — `DishonoredGame`, `CoreSmoke`,
`EdgeAnimSmoke`, `LayoutProbe`. Log `build/agentEH/buildclean.log`.

**940 edges for `DishonoredGame` and the three smaller targets on top of it, 0 errors, 0 `C4263`,
0 `C4264`**, and all four executables produced (`DishonoredGame.exe`, `CoreSmoke.exe`,
`EdgeAnimSmoke.exe`, `LayoutProbe.exe`). The only warnings in the log are the pre-existing `C4316`
alignment notes the tree already carries.

### 8.2 Regression

`python resources\tools\run_regression.py --build-dir build/agentEH_wtrel --exe-name
DishonoredGame_EHW.exe --log-prefix EHW`, run from **the worktree's own copy** with the build
directory deleted first and **no `--no-build`**, so the six build-stage checks are included:

```
37 ok, 0 failed, 0 skipped, 1812s
```

(`build/agentEH_wt/build/agentEH_wtrel/regression/summary.txt`, copied to
`build/agentEH/regression1_summary.txt`; full log `build/agentEH/regression1.log`). Every stage ran:
build 6, coresmoke 2, layout 7, nullrhi 4, d3d9 9, inputtest 9. **No check was skipped and none
failed on the first attempt**, which is worth saying because two agents before me had to record a
first pass that did not.

One exactness note, because it is checkable and I would rather say it than have it found: the
`DishonoredGame.exe` the three game stages ran was linked at 21:50:31, and the last edit to this
package is a **comment block** in `GFxInput.cpp` written at 21:47:42 whose object was rebuilt at
21:53:06 — i.e. into `GFx3.lib` but not into that exe. So the measured binary differs from the merged
source by that comment and by nothing else; no statement, declaration or initialiser changed. §8.1's
clean build is of the final source and is what says the final source compiles with 0 errors.

Running the harness from the worktree needs five **generated** files the worktree does not carry
(`resources/docs/types/{all_types.h, retail_sdk_layout.json, script_classes_2012.json,
script_classes_2013.json, types.json}`); they were copied in from the main checkout unmodified, they
are gitignored, and `build/agentEH_sync.py` does not copy them back. Without them the layout stage
cannot run and the run is 30 checks, not 37.

## 9. Hand-overs

1. **The brief's premise should be retired.** Agent DQ's package `bb05b9d` fixed the user's report and
   it is still fixed at `84b0c9d`; agent EG's deviation 5 and hand-over 1, and this wave's brief, are all
   the same one-move driver reading a working build as a dead one. §2 is the measurement.
2. **Every driver in this tree should use a multi-delta move.** `build/agentEH/drive.py`'s `hover` and
   `hoverclick` steps are the whole change; a `move` step is a single DirectInput delta and a single
   delta is droppable. This is worth promoting into `resources/tools` rather than being copied between
   agent scratch directories a ninth time.
3. **Concurrent agents share one desktop, and real-input measurements collide** (§3). Either the waves
   that need real input are serialised, or the driver gets a mutex; a stray `APressed` from another
   agent's schedule opened a modal in the middle of one of my runs.
4. **The keyboard/mouse desync** (§6) is retail-faithful and is therefore a product decision, not a
   defect. If it is to change, the place is `_common.SelectionHandler` or a deliberate deviation in
   `GFx_GenerateMouseButtonEvents`, and it should be written down as one.
5. **The Options screen does not close on B.** `build/agentEH/base7_log.txt`: a real click opens it,
   `> BPressed () called` arrives, and no `TransitionTo (MainMenuScreen)` follows, so the player is
   stuck on it. Not this package and not measured further.
6. **The rest of `GFxButtonCharacter`** — the up/over/down state machine and the `DefineButton2`
   condition actions — is still agent DQ's deviation 9. The brief named it as a suspect; it is not
   reachable from this menu (one record for all three states, zero condition actions) and nothing in
   this package needed it.

## 10. Files

Mine, three, none new:
`External/GFx3/{GFxInput.cpp, GFxPlayer.h, GFxPlayerRoot.cpp}`.

Plus `resources/docs/agents/agentEH.md` (this file) and `resources/docs/agents/agentEH_status.csv`.
Nothing outside `External/GFx3` and `resources/docs/agents`. **Total: 5 files.**

Scratch, not repo tools: `build/agentEH/` — `drive.py` (the real-input driver with the `hover` step,
the process binding and the foreground retry), `go.py`, `bmp2png.py`, `dec.py`, `lookup.py`, `xrefs.py`,
`disasmrange.py`, `findbit.py`, `patch01_perframe.py`, `patch02_processinput.py`, the run logs
(`base1`..`base7`, `f1`..`f4`, `pk2_before`, `pk2_after`, `f_park2`, `park_*`) and the screenshots;
`build/agentEH_build.cmd`, `build/agentEH_run.py`, **`build/agentEH_sync.py` — the authoritative list
of what the coordinator has to copy**; build directories
`build/agentEH_playrel` (this package, play defaults), `build/agentEH_baserel` (the untouched HEAD
executable, play defaults), `build/agentEH_wtrel` (the regression's own build) and
`build/agentEH_layoutrel` (the clean layout-checked build); IDA copy `build/agentEH_ida/`.

The five generated files under `resources/docs/types` (`all_types.h`, `retail_sdk_layout.json`,
`script_classes_2012.json`, `script_classes_2013.json`, `types.json`) were copied **into** the worktree
from the main checkout so the worktree's own `run_regression.py` could run its layout stage. They are
gitignored, they are unmodified, and `build/agentEH_sync.py` does not copy them back.
