# Agent DM — the camera waits for the player, and the game draws its own cursor (2026-09-28)

Two faults the user found by playing the menu, and one they did not: **HEAD does not compile.**

| | |
|---|---|
| Package | the start camera's loop, and the mouse cursor |
| Starts at | HEAD `9a04708` |
| Files | 13 (3 of them the DishonoredGame generator's output), listed in §9 |
| Regression | `31 ok, 0 failed, 0 skipped, 422s` on the clean snapshot build `build/agentDM_wtrel` |
| Clean build | the snapshot worktree `build/agentDM_wt` (HEAD + this package), build directory deleted first: DishonoredGame, CoreSmoke and LayoutProbe, 0 errors |

## 0. HEAD does not compile, and that matters for what was claimed

```
gfxuiengine.cpp(2288): error C2248: 'FGFxEngine::InputKey':
        cannot access private member declared in class 'FGFxEngine'
```

`9a04708` put its topmost-open-movie fallback in the free function `DishonoredGFxInputKey`, which is
not a member, and the four-argument `FGFxEngine::InputKey` is private — as it is in the reference tree
too (`Inc/ScaleformEngine.h:399`). A clean checkout of HEAD with the standard Release configuration
stops there, so **the commit's claim that a real key press now reaches the interface had not been built
when it was made.** The claim turns out to be true — this package measured it, twice, with a real
`SendInput` key press — but it was true by luck, not by test.

The fallback moves into `FGFxEngine::InputKey(INT, FName, EInputEvent)` (2013 `0x591470`), which is
where retail resolves the focused movie, and it now picks the topmost movie that **can take focus and
input** rather than the topmost movie full stop. That second half is not cosmetic: §5 opens a second
movie on top of the menu, and with the old fallback every key went to a movie with `bAllowInput` off
and the menu stopped answering entirely.

## 1. The camera: what actually holds it, with its eleven functions

The user: *"the camera moves after a set time however the camera should only move when the screen is
clicked."* Agent DK blamed a missing `UpdateTrack` on `UInterpTrackSoireeControl`; agent DL corrected
that (retail has no such function either) and listed the eleven runtime functions that do the work,
with addresses, and deliberately did not start them, on the grounds that a loop without its release is
worse than no loop. All eleven are ported here, plus three DL's list did not name, and the mechanism is
now measured end to end.

### 1.1 The shape of it

`USeqAct_Interp::StepInterp` (**`0x239700`**) in retail is not the reference's. It computes the new
position, asks every track instance whether the matinee must be held or moved, and then hands the whole
question of wrapping the clock to **`USeqAct_Interp::UpdateInterpLoop`** (**`0x2340f0`**) — which
carries *both* wraps in one function:

* `bLooping`'s rewind of the whole sequence, which is the block the reference writes inline in
  `StepInterp`; and
* a SoireeControl key's rewind to the start of **its own segment**.

The two are the same three steps — play the last instant before the wrap point, jump to the start,
then wrap the new position by the segment's length — which is why retail writes them once.
`UpdateInterpLoop` returns TRUE only for the one case that is neither: a non-looping matinee that has
run off its end, which is what makes `StepInterp` call `Stop()`.

### 1.2 The segment and its release

`UInterpTrackInstSoireeControl::SoireeShouldLoop` (**`0x500480`**): the active key defines
`[StartTime, StartTime + KeyLength]`, and once the new position is past the end the segment is played
again — `m_LoopCount` times, with **0 meaning until something breaks it**. Two things break it: a
smaller count from `GetDistractionLoopOverride(m_PinName)` (**`0x218c50`**), or the key's own
`m_lKeysStatus(i).m_bIsBroken`.

That flag is the whole answer to the user's question, and finding what sets it is the part DL's note
left open. It is set in **`UInterpTrackInstSoireeControl::NeedsSynchronizing`** (**`0x500a40`**),
which does two unrelated jobs in one virtual; the first is a loop over every key copying
`Matinee->ActivatedLinks(status.m_InputIndex)` into `status.m_bIsBroken`.

`ActivatedLinks` is a `TArray<INT>` on `USeqAct_Interp` at SDK @488 that this tree declared and
**nothing ever wrote**. It is maintained at the head of `USeqAct_Interp::UpdateOp` (**`0x239330`**):

```
if (ActivatedLinks.Num() != InputLinks.Num()) { Empty(); AddZeroed(InputLinks.Num()); }
for (i) ActivatedLinks(i) |= InputLinks(i).bHasImpulse;
```

Sticky, because the five known impulses (Play, Reverse, Stop, Pause, Change Dir) are cleared in the
same call. So the extra Kismet input pins a SoireeControl track names are recorded there for good.

And `m_InputIndex` comes from `UInterpTrackInstSoireeControl::InitTrackInst` (**`0x506b30`**), which
matches each key's pin name against the matinee's `InputLinks(i).LinkDesc`.

### 1.3 The one thing a decompile alone would have got wrong

The match is not against the pin name. Retail concatenates a file-scope `FString` first
(`0x506c83`: `mov ecx, offset <prefix>; call FString::operator+`), and that global is initialised at
`0xb825a0` from the literal **`"SCT_"`**.

The first build without it said so immediately, because the census this package added prints the pin,
the link list and the resolved index together:

```
soiree: SeqAct_Interp_83 inputs [0:'Play' 1:'Reverse' 2:'Stop' 3:'Pause' 4:'Change Dir' 5:'SCT_Loop']
soiree:   key 0 [0.50..6.50] type 0 break 1 loopcount 0 pin 'Loop' -> input -1 broken 0
```

`'Loop'` against `'SCT_Loop'` matched nothing, so `m_InputIndex` stayed −1, so the flag could never be
set, so the camera would have looped **forever** — DL's exact stated fear, arrived at from the other
direction. With the prefix: `pin 'Loop' -> input 5`.

The menu's own data, measured rather than assumed: **one** SoireeControl key on `StartCam`, segment
`[0.50 .. 6.50]`, type `ESCT_Loop`, break mode `ESBM_BreakImmediately`, **loop count 0 — forever**,
pin `Loop`, and `SeqAct_Interp_83` has a **sixth** input link called `SCT_Loop` that the other eleven
matinees in the map do not have.

### 1.4 Measured, from one binary (the clean gate build `build/agentDM_wtrel`)

**Untouched**, pointer outside the window, 30 s (`build/agentDM/gateA4.log`):

```
[0027.68] soiree census: SeqAct_Interp_83 'StartCam' pos 4.45 key 0 loop 3 broken 0
[0029.68] soiree census: SeqAct_Interp_83 'StartCam' pos 6.45 key 0 loop 3 broken 0
[0029.68] matinee census: viewtarget CameraActor_24 (CameraActor) at X=4223.3 Y=-7119.9 Z=1894.3
```

The matinee wraps 6.50 → 0.50 every six seconds, the loop count climbs, and the view target is still
`CameraActor_24` — the harbour — after thirty seconds. **0 `Critical:` lines.** Before this package the
same run cut to `CameraActor_11`, the city, at 14.4 s (agent DK's `run17.log`).

**A real key press**, `SendInput` VK_SPACE into the game's own window at 20.4 s
(`build/agentDM/gateB.log`):

```
[0019.78] soiree census: SeqAct_Interp_83 'StartCam' pos 2.42 key 0 loop 2 broken 0
[0020.39] GFx input probe: first key 'SpaceBar' ctrl 0 | ... | OpenMovies 2 | focus NULL
[0020.78] soiree census: SeqAct_Interp_83 'StartCam' pos 6.89 key 0 loop 2 broken 1
[0043.85] matinee census: viewtarget CameraActor_20 (CameraActor) at X=743.7 Y=-3005.4 Z=6415.9
```

`broken 1`, the position jumps past the segment end (6.5001, the break-immediately arm of
`NeedsSynchronizing`) and the fly-through runs. **0 `Critical:` lines.** Nothing scripted: the key is a
real Windows message, delivered by `build/agentDM/drive.py`.

A real mouse **click** on the start screen releases it the same way, through the same fscommand
(`> fscommand (ToMainMenuScreen)`); a mouse **move** does not, which is the right distinction and was
checked both ways (`gateA5.log`: 26 s with the pointer inside the window, untouched, `broken 0`).

### 1.5 Where the classes live now

`UInterpTrackSoireeControl`, `UInterpTrackSoireeControlKeyProperties` and
`UInterpTrackInstSoireeControl` are Engine-package classes that the generator had put in
`DishonoredGameEngineShims.h`, and their runtime is `USeqAct_Interp`'s, which is Engine's. They move
into `Engine/Inc/EngineInterpolationClasses.h` beside `UMatineeData` and the other Arkane matinee
classes, under the comment block that already exists for exactly this; the bodies go into
`Engine/Src/interptracksoireecontrol.cpp` and `interptrackinstsoireecontrol.cpp`, the two import stubs
this tree has been carrying since Phase 3 with the right function lists in them.

**No generated file was hand-edited.** `gen_classes_header.py` drops a shim class the tree declares
elsewhere (`sdk_select`, `cpp not in d.declared`), so
`python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake`
removes the three classes, their two structs, their two enums, their 21 layout asserts and their three
`IMPLEMENT_CLASS` lines by itself. That command was run, and its whole diff over those three files is
those 127 deletions.

One more layout item: retail's `UInterpTrack` carries `ETrackUpdatePass TrackUpdatePass` at offset 105
and this tree's hand-written `UInterpTrack` left 105..107 as padding before `TrackTitle` — so the
reflected property loaded out of `Engine.upk` has been writing a byte nothing could name. Both Soiree
walks read it (`cmp byte ptr [edx+69h], 0`). Naming it adds no storage.

## 2. The cursor: where it comes from, and why nothing was drawing it

The user: *"I cant see my cursor, it should have a custom texture too."* Measured in the order the
brief asked for.

### 2.1 The system cursor is hidden, and that is correct

`UGameViewportClient::GetCursor` (**`0x2a6340`**) returns `MC_None` in the ordinary case — retail only
returns `MC_Arrow` when windowed and paused with `bUseHardwareCursorWhenWindowed`, or over the title
bar, or during a movie. This tree already lands in the same place, because `bDisplayHardwareMouseCursor`
is FALSE, so `FWindowsViewport::UpdateMouseCursor` sets no `HCURSOR` and `UpdateMouseLock` clips the
pointer to the client rect. **Nothing here was wrong and nothing here changed.** The game is supposed to
draw its own cursor, and it was not drawing one.

`DishonoredInputGrabDisabled()` was checked and is not involved: it is FALSE without `-unattended`, and
no screenshot run in this package used `-unattended`, so the user's mouse is left alone exactly as
before.

### 2.2 It is a Scaleform character in the global movie, not a texture

`UDishonoredViewportClient::Draw` calls `UDisGFxMoviePlayerGlobal::PreRender(FViewport*)`
(**`0x79eb90`**, from `0x5ea359`), which is three functions:

```
PreRender                     0x79eb90  hide unless m_bMouseCursorAllowed; else read the mouse whole
                                        from Viewport->GetMousePos, map it through
                                        ComputeMovieSpaceInfo, show, move
ConditionalShowMouseCursor    0x78c790  _root.mouseCursor_mc.attachMovie("mouseCursor",
                                        "mouseCursorInst", mouseCursor_mc.getNextHighestDepth())
                                        and GameViewport->bDisplayingUIMouseCursor = TRUE
MoveMouseCursor               0x78c8c0  SetDisplayInfo with V_x|V_y only
ConditionalHideMouseCursor    0x7945b0  mouseCursorInst.removeMovieClip(), the flag back off
ComputeMovieSpaceInfo         0x787860  x = MovieSpaceSize.X * (MouseX / ScreenSize.X) - EmptySpace.X
```

`m_pMouseCursor` (SDK @444, typed `FPointer` because the dump only sees its first word) is a
**`GFxValue`** holding the attached clip.

So the cursor's art is the exported symbol **`mouseCursor`** of a movie, and a scan of every cooked
payload says which: `mouseCursor`, `mouseCursor_mc` and `fakeMouseCursor` appear in
**`DishonoredGame.Global.gfx` only** — `UI_Global.Global` — and in none of the other 21 movies. It is
32x32 in movie space, and it is a real display object: the probe added here reports
`clip 32x32 visible 1` the moment it is attached.

**And `UI_Global.Global` was never open.** `-gfxuimenu` opened the main menu alone, because
`UDisGlobalUIManager` — which in retail keeps the global movie up for the whole session — does not run
here. That is the same gap `-gfxuimenu` already stands in for, so it now opens the global movie too
(`-gfxuinoglobal` leaves it closed).

### 2.3 Opening a second movie found a defect of its own

Opened **after** the menu, the global movie produced **12,580** AS2 errors of the form
`call of a value that is not a function: 'tween__start' / 'tweenEnd'` and the main menu's bar never
faded in; the identical run with the global movie closed had **39** errors in total and opened the bar.
The second movie's initialisation takes the shared library's tween functions away from the first.

Opened **before** the menu — so the menu's own registrations are the last ones made — the flood is gone
(**71** errors, the ordinary count) and the bar opens. Z-order is kept by `UGFxMoviePlayer::Priority`,
which `FGFxEngine::InsertMovieIntoList` already sorts the draw list by, rather than by open order: the
global player is given `Priority` 255 so the cursor draws over everything.

That is a workaround, not a fix. The real defect — two open movies sharing one AS2 library
registration — is handed over in §8.

### 2.4 The mouse had no route into the interface at all

`UGameViewportClient::InputAxis` never called the interface, so `FGFxEngine::MousePos` was never set
and a click had no position to land at. Three retail functions cover it:

* `FGFxEngine::InitKeyMap` (**`0x59f490`**) — the key map's value is retail's
  `FGFxEngine::UGFxInput { INT Key; INT MouseButton; FGFxMovie* Owner; }`, not a bare key code. A
  keyboard key has `MouseButton` −1; the five mouse bindings have key code 0 and
  `LeftMouseButton` 0, `RightMouseButton` 1, `MiddleMouseButton` 2, `MouseScrollUp` 4,
  `MouseScrollDown` 3.
* `FGFxEngine::InputKey` (**`0x590fd0`**) — a key-code-0 binding becomes a `GFxMouseEvent`:
  `MouseDown` on `IE_Pressed`, `MouseUp` on `IE_Released`, or `MouseWheel` with
  `ScrollDelta = 6 * MouseButton - 21` (so +3 up, −3 down), at the engine's own `MousePos`. `Owner`
  remembers which movie took the press so the release reaches the same one.
* `FGFxEngine::InputAxis` (**`0x594cc0`**) — the non-gamepad arm reads the position **whole** from
  `HudViewport->GetMousePos`, subtracts the movie's own viewport origin and delivers a `MouseMove`. It
  is the position, not the axis delta, that the interface wants; the delta is the pawn's.

### 2.5 Measured

Screenshots from the clean gate build at 1280x720, real Windows input only
(`build/agentDM/drive.py`):

* `build/agentDM/dmg2200000000.png` — the start screen, the game's own arrow at (640, 144).
* `build/agentDM/dmg31100000001.png` and its crop `build/agentDM/crop_gate_newgame.png` — the main
  menu with the bar open, the cursor sitting on **NEW GAME**.
* `build/agentDM/crop_cursor.png` — the same cursor at 1008x567, against the sky, before the key press.

The probe line, once a second under `-dismousecursor`, following the pointer:

```
mouse cursor: attached 'mouseCursor' to _root.mouseCursor_mc at depth 1, movie 1280x720, screen 1008x567
mouse cursor: screen 504,113 -> movie 640.0,143.5 | clip 32x32 visible 1 | holder at 0.0,0.0 visible 1 | movies open 2
mouse cursor: screen 403,283 -> movie 511.7,359.4 | clip 32x32 visible 1 | ...
```

**Clicks reach the content.** A real left click produces `AS2 trace: OnMouseUp (0, undefined)` from
`_common.UIBase`, and on the start screen a click runs the whole chain — `TransitionTo (MainMenuScreen)`,
`fscommand (ToMainMenuScreen)`, the Kismet route, the camera. That is a click activating something.

**A click on a menu bar entry does not activate it**, and the reason is precise. The click is delivered;
what is missing is any notion of *which entry it is over*. `GFxMovieRoot::ProcessMouse` broadcasts the
`Mouse` class's four notifications and the position, and stops there. Retail's body also tracks the
topmost entity per mouse index and generates rollOver / rollOut / press / release through
**`GFx_GenerateMouseButtonEvents` (`0xa66a90`)**, which drives `GFxButtonCharacter`'s state machine —
named as out of scope by agent BC (agentBC.md 6.9) and carried since as agentDG.md deviation 4. Until
that exists, `OnMouseUp` fires with no target and the entry never learns it was clicked. Keyboard
navigation of the same entries works (agent DK).

## 3. Census

| | before | after |
|---|---|---|
| Soiree runtime functions in the tree | 0 of 11 | 11 of 11, plus `GetKeyframeLength`, `GetTimeRange`, `GetMatinee` |
| `ActivatedLinks` writers | 0 | 1 (`UpdateOp`) |
| Camera on the harbour, untouched | 14.4 s | ≥ 30 s (the whole run) |
| Key press releases the camera | no | yes, `broken 0 -> 1` in the frame after the press |
| Movies open in the menu | 1 | 2 (`UI_Global.Global` first, `UI_MainMenu.MainMenu` second) |
| Cursor display objects | 0 | 1, 32x32, at the mouse |
| Mouse events reaching a movie | 0 | move on every axis event, down/up on every button |
| AS2 errors, menu open, real key | 39 (one movie) | 71 (two movies); 12,580 with the wrong open order |
| HEAD compiles | no | yes |

## 4. Verification

* **Regression**: `python resources\tools\run_regression.py --build-dir build/agentDM_wtrel --no-build
  --exe-name DishonoredGame_DMW.exe --log-prefix DMW` -> **`31 ok, 0 failed, 0 skipped, 422s`**
  (`build/agentDM_wtrel/regression/summary.txt`).
  An earlier pass of the same command on the same binary reported `30 ok, 1 failed` with
  `inputtest_moved 629.4` — fourteen `cl.exe` of another agent's build were running at the time, and
  the same stage on an idle machine measured **1013.8** (HEAD 1029.6). That metric is a distance walked
  in a fixed wall-clock window; it measures the machine as much as the tree.
* **Clean full Release build** of the snapshot worktree `build/agentDM_wt` (HEAD `9a04708` plus this
  package's 13 files), build directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: DishonoredGame,
  CoreSmoke and LayoutProbe, **0 errors** (`build/agentDM/buildwt2.log`).
* **The acceptance runs** are `build/agentDM/gateA4.log` (30 s untouched), `gateA5.log` (26 s untouched
  with the pointer inside the window), `gateB.log` (real key at 20.4 s) and `gateD/gateE.log` (the
  cursor screenshots), all from that same clean build. **0 `Critical:` lines in every one.**
* **Generator**: `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk
  --module-header --sources-cmake` was run and its output is committed as part of this package. It
  reports `12,282 static_asserts, 0 pending` and `9,201 members checked, 0 differ`.

## 5. Deviations, stated once

1. **`StepInterp` takes retail's shape**, which drops two things the reference had. The
   `ReplicatedActor` tail goes (retail has no such block and `ReplicatedActor` is a storage-less shim
   here), and the reference's inline `bLooping` wrap goes because `UpdateInterpLoop` is that wrap.
   The zero-length guard agent AF added stays: `UpdateInterpLoop`'s wrap is the same `while` the guard
   was written for.
2. **The group-level synchronizing early-out of `0x2193e0` is not ported.** Retail skips a group whose
   `UInterpGroupInst` slot-320 virtual answers TRUE, and answers "hold at Position" when
   `m_bIsSynchronizing` is set and `UInterpGroup`'s slot-292 virtual agrees. Both virtuals ICF-fold to
   `xor eax, eax; ret` in their base classes, and nothing in this tree sets `m_bIsSynchronizing`, so
   the branch is unreachable. The per-track-instance walk, which is the part with behaviour, is whole.
3. **`0x2195a0` and `USeqAct_Interp::UpdateMatineeLOD` (`0x219320`) are not ported.** The first
   notifies each group's pawn when a track instance moved the position forward (vtable slot 1220 on
   the pawn); no SoireeControl key reaches it and the menu's matinees have no pawn. The second is
   `StepInterp`'s first line and is absent from this tree entirely.
4. **`NeedsSynchronizing`'s unreachable `NewPosition <= StartTime` arm is dropped.** The key selection
   above it already requires `NewPosition > StartTime`.
5. **The cursor is drawn from the GFxUI bring-up, not from `UDisGFxMoviePlayerGlobal`.** The three
   retail bodies are reconstructed faithfully and tagged with their addresses, but they live in
   `gfxuiengine.cpp` beside `DishonoredGFxAutoOpen` rather than on the movie player, because
   `UDisGlobalUIManager` never runs and putting them on the class would mean a CppText hook and a
   DishonoredGame regeneration in a tree two other agents are live in. `m_pMouseCursor` is this file's
   static `GFxValue`.
6. **The global movie opens before the menu** (§2.3), which retail does not need to care about.
7. **`FGFxEngine::InputAxis`'s gamepad arm is not reconstructed** — `FUIAxisEmulationData`, the
   four-axis repeat machinery this tree's `UpdateKeyEmulation` already records as absent.
8. **`DishonoredGFxInputAxis` does not consume the event.** Retail's `InputAxis` returns the capture
   flag; while nothing captures the mouse the pawn must still turn with it, so the route is offered and
   the result ignored.
9. **`_global.bIsKeyHold`** — retail's `InputKey` sets it from `IE_Repeat` before delivering a key
   event (`0x590fd0`, pUMovie vtable slot 360). Not ported; no measurement here needed it.
10. **The topmost-movie focus fallbacks** (the real path and `-gfxuikey`'s) are still bring-up, as
    `9a04708` said. They now skip movies that cannot take focus or input. Both go when the script
    `InitInputSystem` inserts a `UGFxInteraction` per local player.

## 6. Switches this package adds

All read on first use, none a file-scope initialiser.

* `-dismousecursor` — once a second, the cursor clip's own numbers: attached or not, where in screen
  and movie space, how big, visible, and the holder's position and visibility. "No cursor on screen" is
  equally true of a clip never attached, one attached empty, one off-screen and one invisible.
* `-gfxuinoglobal` — leave `UI_Global.Global` closed. The comparison in §2.3 is this switch.
* `-dismatinee` (agent DK's) gains the Soiree half: per matinee, the input links with their impulse and
  `ActivatedLinks` state; per SoireeControl track, every key with its type, break mode, pin, authored
  loop count, resolved input index and broken flag; and once a second the live key index, loop count
  and broken flag of every playing SoireeControl track.

## 7. Two things about measuring that cost runs here

1. **A driven test must key off the log, not the clock.** The first six attempts at the cursor
   screenshots scheduled input by wall clock; the same command line reached the menu at 5.6 s on an
   idle machine and at 40 s with another agent building, so the input landed before the menu existed
   or the screenshots after the process was killed. `drive.py` now waits for the line the bring-up
   prints when the movie opens — and reads only the **tail** of the log, because re-reading a growing
   multi-megabyte log four times a second slowed the run it was watching enough to push its own
   screenshots past the end of it.
2. **Windows refuses `SetForegroundWindow` to a process that does not own the foreground**, and
   without focus no key reaches the viewport. Clicking the window to activate it works — and is itself
   input: a click landing on the start screen before the start camera has begun runs the menu's Kismet
   chain out of order, which is what made one run look like a regression. `AttachThreadInput` around
   `SetForegroundWindow` gives the keyboard without generating any.

## 8. Hand-overs

1. **Two open movies share one AS2 library registration** (§2.3). Opening `UI_Global.Global` after
   `UI_MainMenu.MainMenu` costs the menu its `tweenTo`/`tweenEnd`, measured as 12,580 errors against
   71. The open order is a workaround; retail opens the global movie and every screen movie together
   and the HUD will need the same. Start from where the shared `GFxMovieDef` of `Common_assets.lib`
   runs its `DoInitAction`s and which `_global` they write into.
2. **`GFx_GenerateMouseButtonEvents` (`0xa66a90`)** and `GFxButtonCharacter`'s state machine. Without
   them a click has no notion of which entry it is over, which is the last thing between here and a
   mouse-operable menu. `GFxMovieRoot::ProcessMouse` (2012 `0xa0e900`) is where it goes; the position
   and the four broadcaster notifications are already there.
3. **`m_bMouseCursorAllowed` / `m_bAllowMouseCursor`.** Retail hides the cursor unless the current
   screen asks for it (`PreRender`'s first test, `this+440 & 4` and `this+380 & 0x200`); the bring-up
   shows it whenever the global movie is open. The bits are on `UDisGFxMoviePlayerGlobal` @440 and
   `UDisGFxMoviePlayerBase` @380 and go with `UDisGlobalUIManager`.
4. **`UDisGlobalUIManager`** itself, which is what should be opening the global movie, the main menu
   and every screen — and whose absence `-gfxuimenu` has been standing in for since agent DC.
5. **`USeqAct_Interp::UpdateMatineeLOD` (`0x219320`)** and the jumped-forward pawn notify
   (`0x2195a0`), deviations 3.
6. **`SetDistractionLoopOverride`** — 2012 `0x22fb10`, unmatched in 2013, the writer of the
   `m_OverrideDistractionLoop` array this package's `GetDistractionLoopOverride` reads.
7. Everything agents DK and DL handed over that this package did not touch: the DLC config merge and
   `MISSIONS*`, `_bUsingGamepad`, `flash.display.BitmapData` for `_common.EmbedImg`, the sprite and
   button filter pass, `CalcGlyphParam` on the text display path, and `GTessellator`.

## 9. Files

Mine (13, none new):

`Engine/Inc/{EngineInterpolationClasses.h, EngineSequenceClasses.h, UnInterpolation.h}`;
`Engine/Src/{UnInterpolation.cpp, UnSequence.cpp, UnPlayer.cpp, interptrackinstsoireecontrol.cpp,
interptracksoireecontrol.cpp}`;
`GFxUI/{Inc/gfxuiengine.h, Src/gfxuiengine.cpp}`;
and the DishonoredGame generator's output
`DishonoredGame/{Inc/DishonoredGameEngineShims.h, Inc/DishonoredGameLayouts.h,
Src/DishonoredGameRegistrants.cpp}`, whose whole diff over HEAD is the 127 deleted lines of the three
SoireeControl classes.

**The working tree is shared and two other agents are live in it.** `build/agentDM_wt` carries this
package and nothing else, and the regression and the screenshots above come from it. Everything else
dirty in `source/Development/Src/DishonoredGame` — `dishonoredgameclasses.h`,
`DishonoredGameCameraClasses.h`, `DishonoredGamePowerClasses.h`, `Sources.cmake`, the new
`Inc/CppText/*` and the player/camera/pawn sources — belongs to agents DN and DO, not to this package.
Running the DishonoredGame generator in the shared tree picks up their CppText hooks as well as this
package's change; the snapshot's copy of the shims has one such include removed
(`build/agentDM/patch20_wtshims.py`) and **the coordinator should re-run the generator after merging**,
which restores it.

Scratch, not repo tools: `build/agentDM/` — `dmpatch.py` (the CRLF-safe patch helper),
`patch1_privateinputkey.py` … `patch20_wtshims.py` (one per change, each with its measurement in its
docstring), `drive.py` / `drive_gate.py` (the real-input driver), `realkey.py`, `movemouse.py`,
`cursorshots.py`, `bmp2png.py`, `lookup.py`, `xrefs.py`, `disasm.py`, `vslot.py`, `readstr.py`,
`dec1`..`dec8` (the headless decompiles this rests on), the run logs `gateA..gateE`, `runA`..`runR`,
and the screenshots; `build/agentDM_build.cmd`, `build/agentDM_run.py`, `build/agentDM_gate_run.py`,
`build/agentDM_sync.py`, `build/agentDM_wt_build.cmd`; snapshot worktree `build/agentDM_wt`, build
directories `build/agentDM_rel` and `build/agentDM_wtrel` (the clean gate build);
`build/agentDM_save/DisMission0.sav` (a copy of agent DJ's synthesised save, so the menu bar comes up
with its entries).

IDA: **own copy only**, `build/agentDM_ida/retail2013_agentDM.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.
