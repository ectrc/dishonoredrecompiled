# Wave 16 — the menu, against the shipped game

The user ran the staged build beside retail and captured seven side-by-side comparisons. **Ours is the
left window in every one; retail is the right.** Both are the same window size, the same cooked assets
and the same map.

`resources/reference/menu/first.png` … `seventh.png`.

This wave is the whole of wave 16 and the user has asked for it **before anything else**. Wave 15 (ET
perception, EU the patrol's blockers, EV the last 82 KB of the save, EW the five layout defects) is
**paused, not cancelled** — its four worktrees are intact at `build/agent{ET,EU,EV,EW}_wt` and its plan
is in `PHASE11.md`.

## What the seven comparisons show

| | ours | retail |
|---|---|---|
| **first** — start screen | logo larger and softer; scene framed closer | logo crisp, scene framed wider |
| **second** — main menu | logo much larger, bar lower and larger; `NEW GAME \| MISSIONS \| OPTIONS \| QUIT GAME` | logo smaller and sharper; `CONTINUE \| NEW GAME \| MISSIONS* \| LOAD \| OPTIONS \| QUIT GAME` |
| **third** — Options | tabs and rows present; **the whole right-hand value column is empty**; no `RESTORE SETTINGS / BACK` footer; labels soft | every row carries its value (`OFF/ON`, `Easy`); footer present; labels sharp |
| **fourth** — New Game | Corvo portrait **bottom-left, oversized, half off-screen**; background enlarged and soft; no `ACCEPT / BACK` footer | portrait left-centre at its authored size; background sharp; footer present |
| **fitfth** — brightness | **no symbol icons**; slider bar only; no footer | five Outsider-mark icons under the slider; footer present |
| **sixth** — modal | background **not blurred** | background heavily blurred |
| **seventh** — in-world camera | sharper, brighter, no haze | hazier, dimmer, a HUD element bottom-right |

The user's own list, in their words, with what it maps to:

1. *"all of the content seems to be shown at a lower resolution even though its the same assets and the
   same window size"* — **EX**
2. *"lots of the texts are the wrong fonts and also the wrong colours"* — **EX**
3. *"clicking options, going back and then clicking options again … just shows an empty screen"* — **EZ**
4. *"the character images are still out of place on the new game screen"* — **EY**
5. *"the background of the new game screen is way too enlarged and blurry"* — **EY** (likely the same
   cause as 1; see below)
6. *"the icons to detect the brightness doesnt show up and the slider doesnt work either"* — **EY**
7. *"The modal doesnt have a blurred background"* — **FA**
8. *"When accepting the new game, nothing loads or starts"* — **EZ**
9. *"elements are supposed to shift a bit in 3d however on ours they just stay static"* — **FA**

## A warning about the first fault, for whoever takes EX

"Lower resolution" and "too enlarged and blurry" and "the portrait is out of place" may be **one cause
or four**. A single wrong stage-to-viewport mapping would magnify every movie, soften every bitmap,
move every anchored element and enlarge every background — which would explain 1, 4 and 5 at once, and
part of 2.

**That is a hypothesis, not a finding.** The last three waves have each had a package spend hours on a
brief's confident cause that turned out not to exist — most recently the HUD's "two causes", of which
the second did not exist and the coordinator had repeated a conclusion its own author had already
withdrawn. **Measure before porting.** The candidates worth eliminating, in rough order of cheapness:

- The movie's **authored stage size** against the **viewport** we hand `GFxMovieView::SetViewport`, and
  which scale mode is in force (`SM_NoScale` / `SM_ShowAll` / `SM_ExactFit`). Retail's choice is in the
  binary, not in our comment.
- Whether the movie is rendered into an **offscreen target at a smaller size and then upscaled**, rather
  than drawn at viewport resolution.
- **Edge anti-aliasing and tessellation.** `STATUS.md` has recorded since wave 8 that retail's
  `GTessellator` is not ported. Vector shapes drawn without it, or with a coarse tolerance, are exactly
  what "the same asset, softer" looks like.
- The **glyph cache**: GFx can rasterise text into a cache texture at one size and scale it, or render
  it as vector. Blurry text at a magnified stage is the signature of the former.
- The **texture filter and mip bias** the UI draws with.

Do not stop at the first candidate that produces an improvement — say which of them was measured, what
each measured, and which one the change rests on.

## Packages

### EX — the stage, the text and the colour

Faults 1 and 2, and whatever of 4 and 5 shares their cause.

Deliver: **the menu drawn at the same effective resolution and sharpness as retail's, in the same
fonts and the same colours**, with a side-by-side against `first.png` and `second.png` at the same
window size.

Specific things to settle, each with a measurement:

- The stage/viewport/scale question above.
- **Which font each text field resolves to**, and where from — `[GFxUI.FontConfig]`, the movie's own
  embedded fonts, `GFxFontLib`, or a fallback. A field falling back to a default font is the most
  likely reading of "the wrong fonts", and it is directly observable.
- **Where the colours come from** for the fields that are wrong: the field's own `TextColor`, an HTML
  or CSS tag in the string, a `GRenderer::Cxform`, or the translator. Note that `GRenderer::Cxform`
  once reached this tree **transposed** from the generator, so the ActionScript machine and the
  renderer disagreed about all four channels in the same 32 bytes (wave 7). Check it rather than
  assuming it is right now.
- Whether retail's `GTessellator` is needed for the edges we are missing, and if so what it costs.

This package owns `External/GFx3` and `GFxUI` for the wave. Say so in your report if you need a file
outside them.

### EY — the New Game screen, the brightness screen, and the missing value columns

Faults 4, 5 and 6, plus two things the comparisons show that the user did not list:

1. **The Corvo portrait is bottom-left, oversized and half off-screen** (`fourth.png`). Retail has it
   left-centre at its authored size.
2. **The New Game background is enlarged and soft.** If EX settles the stage question this may fall out
   for free — coordinate, and say which package fixed it rather than both claiming it.
3. **The brightness screen has no symbol icons.** Agent EO's hand-over 2 already names the cause and it
   has never been opened: *"`UDisGFxMoviePlayerGamma` has no implementation file at all.
   `OpenGammaImage`/`CloseGammaImage` are still stubs, `UI_Gamma_SF` is never opened, so the brightness
   screen's reference symbols are missing."*
4. **The slider.** Agent EO measured five LEFT presses taking the gamma 2.2 → 1.7 with the picture
   darkening, and one RIGHT from 2.2 giving exactly one change to 2.3, the mapping's own maximum. The
   user reports it does not work. **Both can be true**: test the keyboard path and the mouse path
   separately and say which works. CLIK drag handling is a likely gap.
5. **Every option row's value column is empty** (`third.png`). Agent EO listed "whether the options
   screen's value column renders correctly for drop-list rows" as explicitly not measured. It does not.
6. **The footer prompt bars are missing** on New Game, brightness and Options — retail draws
   `ACCEPT / BACK` and `RESTORE SETTINGS / BACK`. Ours draws nothing there.

Also worth one measurement while you are here, and **do not assume it is a defect**: retail's main menu
shows `CONTINUE` and `LOAD` where ours does not (`second.png`). Retail had saves; our profile may not.
Settle it and say which.

### EZ — the two functional bugs

**1. New Game does not start the mission on the shipped build.** The user's own run:

```
[0120.99] message box 1 answered with button 0
[0121.15] OnNewGameConfirm(difficulty 3): 'ce ChangeLvl_StartNewGame'
          <- nothing follows
```

`Logs/Launch720b.log:3770`. No `SetPlayerTravelDestination`, no `Committed map change`. Agent EL
measured the identical path **completing** at difficulty 1 in its own worktree — `SetPlayerTravelDestination
'PlayerStart_NewGameEmpress'`, the map change committed 0.65 s after the key, Corvo on the boat landing
(`build/agentEL/el_tower_t08000003.png`). So this is a regression since `41cf1c0`, or a condition
nobody has isolated. Candidates: the difficulty differs (3 vs 1); the console-event dispatch differs in
the play-defaults configuration; a story-flag or `USequenceCondition` gate that EL's run satisfied and
this one does not. **Note that an unported `USequenceCondition` kills its chain with no log line**, which
is exactly this shape — EL found seven functions behind what looked like one.

Note also the AS2 error 200 ms later in the same log: `call of a value that is not a function:
'removeMovieClip'`.

**2. Options, back, Options again gives an empty screen.** Reproduce it, then fix it. Agent EO ported
`OnLeaveOptions` (`0x7bcad0`) and `CloseOptions` (`0x7bc9d0`) last wave and measured `B` leaving Options
once; nobody measured the second entry. The likely shapes are a movie or clip removed on close and not
recreated, or a state flag left set — but measure it.

**3. ~~The exit teardown fault~~ - wrong, and agent EZ settled it while declining the item.** There is
no teardown fault. The critical error in the user's log is `Ran out of virtual memory` at **217.73 s
with the game still rendering the menu**, one frame after a normal render census: a 32-bit process
exhausting its 2 GB address space after three and a half minutes on the main menu. It is also *why*
the user crashed - New Game did nothing, so they waited. The thirteen "unsymbolised frames" decode as
UTF-16 to the error message itself, so the stack walker was handed a text buffer and anyone starting
from those addresses is chasing text. **This should be opened as "what allocates per frame at the
menu", as its own package.**

### FA — the menu in three dimensions, and the blur behind a modal

**1. The menu elements do not move.** Retail shifts the logo, the button bar and the background
slightly in 3D as the menu lives; ours are static. This is GFx's 3D display properties — `_z`,
`_xrotation`, `_yrotation` and the perspective projection — which `PHASE11.md` has carried as a queued
item since the difficulty screen needed it (*"`_z` is not a display property here"*). It is one feature
serving at least two screens.

**2. Retail blurs the scene behind a modal and we do not** (`sixth.png`). Agent EG established that
everything else about the modal matches — dim, vignette, bar, text and the YES/NO row — and named this
as the remaining difference, in the post-process family rather than the interface.

These two are one package because they are both "the menu is not flat", and because doing them together
keeps one agent in the perspective and post-process code rather than two.

## Rules for agents

The standing set, each earned:

- Work **only** in your own worktree. Two agents have left untracked files in the main checkout and one
  set got swept into another package's commit.
- Your sync script runs **worktree → main**, the merge direction, and **flags rather than copies** any
  file that changed in main since your base. That is what stopped one package destroying another's new
  header last wave.
- **Resolve every retail address yourself.** `rva_sweep.py` is necessary but not sufficient — it passes
  any address landing inside *some* 2013 function. Six mislabels were found in one wave only by a
  by-name audit with `ida_funcs.get_func`.
- **`vtables.csv` is the 2012 table**; 2013's is shifted for some classes. `match_2012_2013.csv` has
  propagated an entirely wrong vtable through a 27-byte constructor match. **Pin a vtable a second way
  before reading a body out of it.**
- **Never use the FModel MCP tools.**
- Use the shared `resources/tools/drive_input.py` with `--exe <image name>`; `hover`/`hoverclick`, not
  `move`/`click`. Prefer `-apshottime=` for screenshots. Read the drive log before believing a
  measurement. **Do not click through a Windows Defender Firewall prompt.**
- Run the harness from **the worktree's own copy** with an **absolute** build dir, built inside the
  harness (no `--no-build`); the full set is **37**. Copy the five gitignored
  `resources/docs/types/*.json|h` inputs into the worktree first or every layout metric records `-1`.
- A d3d9 or inputtest failure is usually load. The harness now re-runs a stage whose failures are all
  throughput metrics and prints both numbers.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Every wave so far, at
  least one package has corrected a premise of its own brief, and most waves one has corrected a truth
  source. That has been the most valuable part of every report.

## Acceptance, common to all four

1. A side-by-side against the named reference image, same window size, reached normally, nothing forced
   visible.
2. A before/after against the untouched HEAD executable through the same driver.
3. `run_regression.py` 37 ok, 0 failed, 0 skipped.
4. A clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: 0 errors,
   0 C4263, 0 C4264.
5. `rva_sweep.py` plus a by-hand resolution of every cited address.
6. `resources/docs/agents/agent<XX>.md` and `agent<XX>_status.csv`, written **inside the worktree**.
