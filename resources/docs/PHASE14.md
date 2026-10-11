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

### FH — merged `f72f9f5`, gated 37 ok / 0 failed / 0 skipped

Three source files, 150 added lines, 0 removed. No regeneration, no header, `Sources.cmake`
unchanged. Rebased onto FF's `63c2539` mid-run with every number retaken there.

**Fault 4 is FIXED, and the cause was none of the three functions this brief named.**
`portrait_mc` is a four-frame sprite: frame 1 places its artwork **with** a matrix of (−160, −246) px;
frames 2, 3 and 4 each carry `Move|HasCharacter` and **no matrix at all**. That pair of flags is a
**replace**, and retail's `GFxDisplayList::ReplaceDisplayObject` (2013 `0x9ccb50`, 2012 `0x9d6350`)
**inherits the outgoing instance's transform for every field the tag does not carry**. Ours left the
replacement at identity, so every difficulty but EASY drew at the clip origin.

Live display list on `63c2539`: frames 2/3/4 `(0.00, 0.00)` → `(-160.00, -246.00)`, frame 1
unchanged. In pixels against retail: ours-before **+127, +194**; ours-after **+1, +0**. Nothing else
moved — difficulty rows dx 0 dy 0, ACCEPT/BACK bar dx 0 dy 0, title dx +1 dy 0
(`build/agentFH/sbs_hard.png`).

**This reaches past the portrait.** Across all six cooked movies: **37 replace tags, 35 with no
matrix, 17 inheriting a non-identity transform** — three are the portrait, fourteen are `lib.swf`'s
controller-icon clips (which this corrects), two are in the unreachable pause menu. Sprites 51 and
125, the only other multi-state clips in the main menu, inherit identity and cannot move.

**Three premises corrected.** All three of FC's suspects are innocent — `GotoFrame` is reached and
correct, `IncrementFrameAndCheckForLoop` is only called from `AdvanceFrame` and a `gotoAndStop` never
reaches it, and `AddDisplayObject`'s early-out is in a function these tags never enter (the trace
shows every one logging `-> replace`). FC's "+140 / +255 and smaller" is really **+160 / +246 and
twelve px wider** (332x492 vs 320x492); what read as smaller is the bottom third falling off a 720-px
stage. FC's "bounds are not recomputed on a frame change" was never a defect: `_width` reads 320 on
frame 1 and 332 on frames 2–4 **on both builds**.

**Two reusable instruments.** `build/agentFH/tagdump.py` decodes `PlaceObject/2/3` and
`RemoveObject/2` per frame with flags named, character ids resolved to definitions and bounds, and
matrices in twips and pixels — cross-checked against two independent live measurements before
anything was concluded from it (it reproduces FC's `_corvo_mc (-640.65, -150.95)` and FB's dismod
capture of **retail** at `_menu_mc (-426.50, -25.95) rot −4°`). And
`-disdifficulty=<worldTime>:<idx>`, which calls the body of the asset's own `SetPortrait`, keyed to
world time so a step and an `-apshottime` mark mean the same instant on both builds.

**Two findings left unfixed and recorded.** `portrait_mc` advances one frame on its own before its
`stop` lands, on both builds, because each frame's `stop` is queued and drained after the next
`AdvanceFrame` — harmless today, a real ordering difference from retail. And the window came up
**1008x567** with `-ResX=1600 -ResY=900` on **eight of ten runs** and 1600x900 on two, same switches,
same isolated ini dir: **read a capture's size, never assume it.**

**`rva_sweep` demonstrated insufficient, not merely asserted so**: the `UnpackedData` constructor
was first written as `0xdf0a00`, its **VA**, and swept **silently**. The by-hand pass caught it.
Correct is `0x9f0a00`.

**STATUS defect 8 is worse than recorded and is now costing work, not just measurement.** All four of
this package's real clicks went into **five** `rundll32` "Windows Security Alert" windows holding the
desktop foreground. No dialog was clicked, dismissed or minimised. Third wave running.

### FI — merged `8027ab6`, gated 37 ok / 0 failed / 0 skipped

16 changed files + 1 new header, five of them outside `DishonoredGame`; nothing in `External/GFx3`
or `GFxUI`, so no overlap with FF, FG or FH.

**First, a regression at HEAD that no brief could have named. At `18f36a3` the main menu closed
itself one second after it opened, so New Game was unreachable.** Agent FE's
`DishonoredGFxNotifyGameSessionEnded()` in `CommitMapChange` fired on the **front end's own**
streaming commit — `DishonoredGameFull_P` → `Dishonored_MainMenu` — rather than on the commit that
ends a session. Measured twice: the movie opens at 22.44 s, the commit lands at 23.24 s, and the next
line reports `0 movie(s) open`. Retail does not decide it there at all — the reference engine raises
it from `UWorld::CleanupWorld(bSessionEnded)`, and that site is load-bearing: without it a movie open
across a `LoadMap` leaks the world (`World ... not cleaned up by garbage collection`, measured). Both
halves are now in their retail places.

**This is the gate's first known blind spot.** FE's change passed 37/37 on its own commit because
every harness stage drives a map directly with `-startmap`; **no check presses New Game**, so nothing
in the harness exercises the menu → mission handover that the README advertises. Worth a check.

**Fault 5: the boat scene is populated and the bars draw.**
`build/agentFI/fi_fault5_before_after_retail.png` is the user's own half of `5.png`, this package, and
retail's half: same boat, two NPCs on it, letterbox bars top and bottom.

**Three premises of this plan corrected.**

- **"The pawns are not placed" is half wrong.** `l_tower_p` already holds **26 `ADishonoredNPCPawn`
  plus the player** at HEAD as well as here, every one with a controller and a skeletal mesh, none
  hidden, six drawn — 17,000–25,000 units away up at the Tower, which is why none was visible. What
  was missing is the two the opening scene spawns, from `DisSeqAct_StartSpawn_1`. The fault was
  `ADishonoredSpawner::OnStartSpawn` being passed a **zeroed** `FDisSpawnInfo` where retail fills the
  requester and copies `m_bSpawnEvenIfVisible` — and that bit is what made `DoSpawnNow`'s
  player-proximity gate **silently refuse every spawn in a scene the player is standing in**.
  27 → 29 pawns, 7 → 8 drawn, within two seconds of the action running.
- **"The matinee does not start" is wrong as stated — it STALLS.** The map's Kismet stopped on a
  latent `SeqAct_AkPostEvent` that can never complete, because **`AK::SoundEngine::RenderAudio()` has
  no caller in this tree**: `UGameEngine::Tick` reaches audio through `UClient::GetAudioDevice` and
  `UWindowsClient` never builds one, so no `AK_EndOfEvent` is delivered and `UpdateOp`'s `Signal`
  never reaches 0. Fixed at retail's own call site (the last instruction of
  `UDisHonoredAudioSystem::Update` `0x7b3520`, the only non-Wwise xref to `0x913670`). Identical
  command lines: HEAD 6 ops active with `SeqAct_AkPostEvent_0` latent for ever, this package 5 and
  the stall gone. On the New Game route `L_Tower_Script` runs 47 of 1,629 ops with
  `SOIREE_Empress_Regent` and `SOIREE_Emily` live.
- **FE's blocker was not a blocker, and the dismod run this plan insisted on was not needed.**
  `execSetCinematicMode_Native` reads **nine** booleans and **no optional tenth** — the "tenth" is
  `P_FINISH`'s `EX_DebugInfo` (`0x41`) tail — and **`retail_sdk_layout.json` names all nine**
  (`params_size 36`, every one `CPF_Parm`, not one `CPF_OptionalParm`).

**Still not right, and specified the way FE specified the bars.** The opening cutscene's own matinee
does not run. Seven matinees *are* live in `l_tower_p` and every one is set dressing with
**`director NONE`** — rats, planks, a whale, the whaleboat; five frames 45–110 s apart differ by a
mean 2.0/255. Three named gaps: **`UInterpGroupPlayer`/`UInterpGroupInstPlayer` entirely unported
*and* `USeqAct_Interp::InitInterp` has no branch that would build one**;
**`ADishonoredNPCPawn::MAT_*`** — the whole matinee-AI contract, ~40 functions, zero bodies; and
**`UpdateTrack` missing on every Dishonored-added `UInterpTrack`**. Plus a sweep: **twelve of
fourteen `UDisSeqCond_*::Activated` are missing** and two of the fourteen already fire on this
mission's path, all twelve 2013 addresses hand-resolved — **the skeleton comments carry 2012 rvas**,
which is a trap. That is the next wave's shape.

**Also measured, and not this package**: the menu's **Enter key does not activate a main-menu row**.
SPACE works and runs `TransitionTo(MainMenuScreen)`; the four Enters produce **no AS2 trace at all**.
STATUS open defect 5 — and it is the keyboard path the README tells the user to use.

**`Sources.cmake` takes one unit off the exclude list and it needed a link anchor**, measured the
hard way: it compiled, the exe linked green, and the object was not in the executable, because
`IMPLEMENT_CLASS`'s vtable is a folded COMDAT rather than a reference into the unit. It looked
exactly like a virtual that would not dispatch.

**Two edits sit in generated headers that a regeneration would drop**: one `CppText` include (the
mechanism the generator preserves) and three inline virtuals on `IDisSpawnRequesterInterface`.

**One caveat stated plainly rather than papered over**: HEAD cannot reach the mission from the menu
at all, so the level-content A/B on an identical command line is `-startmap=l_tower_p -startmapopen`,
a valid control for the audio stall but one that reaches the opening scene on neither executable
(both read 46/1670). The scene itself is measured on the New Game route, which only this package can
run.

### FG — merged `c702a38`, gated 37 ok / 0 failed / 0 skipped

Seven files, plus the two documents. Rebased twice while running (`18f36a3` → `63c2539` → `f72f9f5`)
with every acceptance number retaken on the last. No regeneration, `Sources.cmake` unchanged.

**Fault 3 is fixed and its two halves were ONE bug, in a place neither half of this brief named.**
The token is **`§CATEGORY§`** (U+00A7), not `?CATEGORY?`, and it is substituted **in ActionScript**,
not C++ — `_common.ConvertVariableTags.Convert`, whose same loop substitutes the token **and** wraps
it in `_blueColorTag = "<font color=\"#B1C0D7\">"`. It failed at `indexOf`, on an encoding mismatch:

| | bytes for `§` |
|---|---|
| the SWF constant pool | `C2 A7` — UTF-8 |
| the localisation → `GFxValue::SetStringW` | `U+00A7` |
| what our `GFxValue2ASValue` made of it | **`A7`** — `buf[n] = (char)w[n]` |

Retail encodes UTF-8 at that boundary, resolved by hand through `GFxMovieRoot::CreateStringW`
(`0x9a9030`) → `GASStringManager::CreateStringNode` (`0x9c0800`) → `GString::operator=(const
wchar_t*)` (`0x9b5390`) → `GUTF8Util::GetEncodeStringSize`/`EncodeString` (`0x9e7e80`/`0x9e7f40`).
Three truncating sites now encode UTF-8. The colour then needed the parser, so
**`GFxStyledText::ParseHtmlImpl<wchar_t>` (2013 `0xa9d870`, 6,268 bytes) is ported** and
`SetTextValue` calls it instead of stripping tags; the tag set comes from a census of the six cooked
movies plus the localisation, and the new unknown-markup census line **never fired on any run**.

Against retail at 1600x900: retail's coloured run is RGB **(177,192,215)** — the asset's own
`#B1C0D7` — B−R **+38.0** against −12.8 on the rest of the line; ours is **(171.0,185.6,207.7)**,
B−R **+36.7** against −12.1, starting **within 2 px** of retail's x. Untouched HEAD has **no coloured
run at all**. `build/agentFG/sbs_modal.png`.

**Fault 2 is root-caused and fixed, and it is not what this brief said.** Not a shared container
hidden, not six at once: **one row at a time, the row the pointer just left**, accumulating until the
column looks empty. The value widget is `attachMovie`'d; the row (`o_options_line`, 32 frames, `out`
at 16) sits at `_currentframe 22` after a hover, so the next `gotoAndPlay("over")` is a **backward**
goto. Retail's `GFxDisplayList::MarkAllEntriesForRemoval` (2013 `0x9cbed0`, decompiled whole) marks
an entry only when `depth <= 0x3FFF && CreateFrame > fromFrame`; **ours marked every entry**, so the
rewind's sweep unloaded the attached widget. The argument is also a **frame, not an index** —
`GFxSprite::GotoFrame` (`0x9f7cc0`) passes `target-1` where ours passed 0. Both corrected, with the
depth test written as retail's `>= GFxASDepthBias`, **which only does any work because FF's depth
bias landed first**.

Identical schedule (six hovers down the label column, then park away), runs with `FocusLost != 0`
discarded: on `63c2539` untouched 1 of 1 valid run reproduced it, 3 of 6 widgets gone; with FG 0 of
2. On `f72f9f5` untouched 1 of 2 reproduced it, **4 of 6 gone**; with FG 0 of 2.
**Why FF could not reproduce it: the pointer must *leave* each row.** A hover that stays put keeps
the row focused, so it never rewinds. Both packages were right about what each measured.

**Fault 1's premise does not reproduce, and FG claims nothing.** At a matched 1600x900 our pale panel
and retail's sit in the same place and read the same strength; nine scanline measurements give steps
of −106..+55 (ours) and −122..+67 (retail) with **no consistent sign**, and the two background scenes
differ by far more than the quantity in dispute — the comparison **cannot** resolve it. What is real:
`maskContent` (sprite 22's only timeline child, one `DefineShape3`, `#E3F2D6` at **alpha 24/255**,
scaled to 957×385) was being **rendered** before FF's `setMask` took it out of the content pass.
On the arrows, FG's claim and FF's are different claims rather than a contradiction — FF compared our
HEAD to its own build, FG compared ours to **retail**, where the focused row carries a bold pair
against our faint default one. Handed to FH's family, not claimed. **1b: no scrolled capture, none
claimed** — it moved to FF with the fix.

**FF's 127 `SetWidgetFocus` hand-over, corrected downward.** Both `RemoveWidgetFocus` sites in
`__Packages.OptionsList` are `this._curWidget_mc.RemoveWidgetFocus()` with `_curWidget_mc`
**undefined on the first selection** — a normal AS2 no-op that this runtime logs as an error — and
`-gfxuiclasstrace` reports **every** options symbol healthy, each reaching `MovieClip.prototype`.
**`UpdateSettings` "on Sprite" is the one genuine missing prototype method**, and it did not occur on
any screen FG's driver reaches. **The next package starts there, not from the 127.**

**One simplification stated plainly**: the `CreateFrame` half of retail's condition is not
reproduced, because `GFxCharacter` in this tree carries no create frame. It costs nothing here —
both callers replay the frames up to the target, every character those frames place is re-placed, and
`AddDisplayObject` clears the mark.

**The one semantic collision in this wave was real and a build caught it.** FF deleted
`GFxCharacter::bScriptCreated`, which FG's first `MarkAllEntriesForRemoval` used. **A three-way
textual merge would have applied clean and been wrong.** It is now retail's own depth test.

**Three machine facts.** The **desktop resolution flaps between 1024×768 and 6144×1728**, so the same
`-ResX=1600 -ResY=900` yields 1008×567 or 1600×900 — read every capture's size off the file. **A
concurrent agent's game steals the foreground and kills the menu**: one `execOnFocusLost` and the
movie never draws again, which makes `grep -c FocusLost` a clean validity flag (2 of 9 hover runs
discarded rather than averaged). And **`git apply` run from a non-repo subdirectory inside the repo
silently applies nothing and exits 0** — it cost one 20-minute build whose "FG+FF combination" was
plain HEAD. Use `patch -p1 -d <dir>`.

## Wave result

**Four of four merged and gated, each 37 ok / 0 failed / 0 skipped on a clean checkout of its own
commit**: FF `63c2539`, FH `f72f9f5`, FI `8027ab6`, FG `c702a38`.

Of the user's five faults: **1b, 2, 3, 4 and 5 are fixed with side-by-side measurements against
retail**; **fault 1's panel does not reproduce** and is withdrawn rather than claimed either way, with
its one real component (`maskContent` rendering) fixed by FF. The arrows and fault 1's chrome were
**not** a fault in the terms the brief used.

**Every package corrected at least three premises of its own brief**, and between them they overturned
two things this repository had recorded as fact: that the "PC" frame errors were ours to fix (they are
an asset bug retail shares), and that `execSetCinematicMode_Native`'s boolean count was unknowable
without dismod (`retail_sdk_layout.json` names all nine). Two packages also corrected each other —
FF could not reproduce fault 2 and FG showed why, and FG's reading of the 127 `SetWidgetFocus` errors
replaced FF's.
