# Wave 17 — the interface, to the original

The user played the wave-16 build and listed twelve faults. **The only goal of this wave is to make the
UI as close to the shipped game as it can be**, and the instruction that matters most is this one:

> Dont just treat the symptoms try to find the root cause to all these problems. Some (like the fonts
> and position) may be linked.

They are almost certainly linked, and wave 16 already proved that instinct right twice: one wrong enum
caused every wrong colour *and* every "wrong font" on two screens, and one line in `MCAttachMovie`
accounts for at least three separate placement faults. Look for the shared cause before porting.

## The new instrument: read the answer out of the real game

The user has given us `D:\Christmas\github\dismod\` — their own mod DLL for the **real retail
Dishonored**, built on MinHook, libhat pattern scanning and a CodeRed UE3 `sdk.hpp`, with
`ProcessEvent`, `StaticConstructObject`, `StaticLoadObject`, `LoadPackage` and a D3D9 `EndScene` hook
already working (`src/hooks/`, `include/hook.h`, `src/engine/engine.h`).

The loop, in their words:

1. edit the source in `D:\Christmas\github\dismod\`
2. build into `D:\Christmas\github\dismod\build\debug\`
3. copy the DLL to `C:\Program Files (x86)\Steam\steamapps\common\Dishonored\Binaries\Win32\dinput8.dll`
4. run `Dishonored.exe` — a console opens; redirect it, or use the log file

**`include/logger.h` hardcodes the log path to `C:\Users\User\Desktop\asd\dismod.log`, which does not
exist on this machine.** Point it somewhere real first.

**Why this changes how this wave works.** Almost every wrong answer this project has had came from
inferring retail's behaviour — from a disassembly, or from a truth source that was itself wrong
(`vtables.csv` is the 2012 table; `match_2012_2013.csv` propagated an entire wrong vtable; the 2013
database mislabels functions; `retail_sdk_layout.json` is missing a class outright). dismod reads the
answer out of the running retail game. **When a package in this wave is about to decide what retail
does, it should measure it in retail rather than argue about it.**

## The twelve faults, grouped by suspected shared cause

### A. The fonts — and the user's own clue

> "On the brightness menu, it IS using the wrong font. The `BRIGHTNESS` header text should be using the
> same font as the `NEW GAME` and `OPTIONS` and `QUIT` (I will refer to these as the **Menu Action
> Buttons**)."
> "On the difficulty screen the `NEW GAME` text is also the same wrong font."
> "On the options menu, `OPTIONS`, `GENERAL`, `CONTROLS`, `GRAPHICS`, `AUDIO`, `GAMEPLAY`,
> `USER INTERFACE` all use the wrong font and should be the same as the menu action buttons."

**The main menu bar is right and the headers are wrong.** That is the whole shape of the fault: this is
not a global font failure, it is something that distinguishes a header field from a bar entry. Agent EX
measured **zero fallbacks** on the start screen and main menu — the two screens whose fonts are
correct — and never traced the others. Its `GFxTextDocView::FindFont` trace names a `FALLBACK`
explicitly and is in the tree; one run over these screens settles what each field resolves to and by
which route.

Candidates worth eliminating: a font the headers name that we do not have registered, so they fall
back; a shared font movie that retail loads and we do not; an embedded-font lookup that fails on a
style (bold/italic) rather than a family; a text field using device text where retail uses embedded.
**And the ground truth is one dismod hook away** — log the resolved font for the same field in retail.

Also open from wave 16, same area: text clarity is better but still not perfect. `GTessellator` is
still unported (`STATUS.md`), and EX ruled it out only for the magnification fault, not for edge
quality.

### B. Everything that is in the wrong place

> "the options are still shifted to the right"
> "The brightness indicators … is at the center of the screen instead of offsetted at the bottom and
> there is no accept or back button"
> "the image that should change is still in the wrong offset when on normal, hard or very hard"

**Agent EY root-caused the first three and handed over the exact line.** `MCAttachMovie`
(`External/GFx3/GFxAS2Lib.cpp:736`) copies the init object **after** the registered class constructor
instead of before, so every constructor that snapshots its own position reads the wrong one. Three
independent proofs, all measured:

- `_root.help._x` is 1184 before it opens and **0** after, because `_common.HelpBar`'s constructor
  snapshots `_props` from `this._x` — **that is the missing ACCEPT/BACK bar; it is built, populated and
  drawn at (0,0)**;
- `_options_mc._defPosX` is 0 where the init object says **-450** — 562 screen px at 1600x900, exactly
  the offset the user sees;
- `gamma_mc._y` is 360 where `GammaMc`'s constructor says **555**.

EY did not fix it because `External/GFx3` belonged to another package. **It is one line and it is the
single highest-value fix in this wave.** The difficulty-screen portrait moving with the difficulty is
the fourth candidate for the same cause and should be tested against it before anything else is tried.

### C. The brightness screen's behaviour

> "The slider only works when using keyboard left and right and then when clicking anywhere with a
> mouse the slider defaults to the maximum position."
> "The brightness indicators are all the same brightness level."

**The slider.** Agent EY established that the asset uses `Mouse.addListener` with
`onMouseDown`/`onMouseMove`/`onPress` and has **no `startDrag` anywhere**, so CLIK drag handling is not
the gap. "Any click anywhere jumps it to maximum" is the signature of a handler computing its ratio
from the wrong coordinate space or the wrong origin — stage coordinates where the asset expects local,
or a hit test naming the wrong target. EY could not measure the mouse path at all because nine
"Windows Security Alert" dialogs were holding the foreground; **that is still true and the user has
been told.** If it is still true, say so rather than reporting a guess.

**The indicators being identical** is almost certainly EY's hand-over 2: `flash.geom.ColorTransform`
and `flash.geom.Transform` are **not installed** on the AS2 prototype (only `Matrix`), which raises
five `not a constructor` errors per brightness screen. Five marks that should be tinted to five
different levels and are all the same is exactly what an absent `ColorTransform` produces.

### D. The menu world: camera and post-process

> "The FOV on the main menu is still too high and there is also a brightness & maybe bloom effect that
> should also show."

Two separate readings of the same scene. Note that agent FA ported `_perspfov` for the **movie's** 3D
projection last wave — that is not the same thing as the **world camera's** FOV behind the menu, and
conflating them would be the obvious mistake here. Measure both, and measure retail's.

The post-process half is a question dismod answers directly: hook retail's menu and read the active
post-process chain and its parameters.

### E. Leaving the menu

> "after clicking the new game with a selected difficulty, it does load the new map however it starts a
> cutscene and doesnt have the top and bottom black bars applied to the level. Also there is an issue
> where the menu movie of the surrounding vignette effect is still showing."

Two faults in the same transition. The letterbox bars are the cinematic mode retail applies for a
matinee; the vignette is `_root.vignette_mc`, which agent EY observed in the menu movie's display list
and which is evidently not torn down when the menu closes. Both are about what happens when the menu
hands over to the world, which is why they are one package.

## Packages

### FB — the ground-truth harness, and the fonts

**Build the dismod instrument first and publish it**, because three other packages want it.

1. Get the loop working end to end: fix the hardcoded log path, build, copy to `dinput8.dll`, run, and
   capture output. Write down exactly what you did in `resources/docs/dismod_harness.md` so the others
   can repeat it without rediscovering it.
2. Add the hooks this wave needs and dump, from the **real game**: the resolved font for every text
   field on the main menu, the brightness screen, the difficulty screen and the options screen; the
   display-list position, depth and scale of the clips named in group B; the menu world camera's FOV;
   and the active post-process chain on the menu.
3. **Then fix the fonts** (group A), using that capture as the target rather than a guess.

Publish the captures under `build/agentFB/retail/` with a README naming what each file is, and say in
your report which of them the other packages should read.

### FC — one line, and everything it moves

Group B. Start by applying EY's `MCAttachMovie` fix and **re-measuring all four placement faults
against it** before writing anything else — the options offset, the gamma marks, the ACCEPT/BACK bar,
and the difficulty portrait. Report which of the four it fixes and which it does not, with the numbers.
Then fix what is left.

This package owns `External/GFx3/GFxAS2Lib.cpp`. It is small, it is already diagnosed, and it is the
most user-visible fix available, so do not let it grow.

### FD — the brightness screen

Group C: the slider's mouse path and the five marks' tinting. `flash.geom.ColorTransform` and
`flash.geom.Transform` are yours to install. Coordinate with FC: the marks' *position* is FC's and
their *tint* is yours, and you will both be looking at the same screen.

If the foreground is still being held by security dialogs, say so and deliver the tint half with the
slider's diagnosis rather than a fix — an honest "could not measure" has been worth more than a guess
every wave so far.

### FE — the menu's camera, and the handover to the world

Groups D and E: the menu world's FOV and its missing brightness/bloom; then the cutscene's letterbox
bars and the vignette that survives into the level. Use FB's retail captures for the FOV and the
post-process chain rather than inferring them.

## Rules for agents

The standing set, each earned:

- Work **only** in your own worktree. A sync script runs **worktree → main** and **flags rather than
  copies** anything changed in main since your base.
- **Prefer a measurement in the real game over an argument about it.** That is what dismod is for.
- **Resolve every retail address yourself.** `rva_sweep.py` is necessary but not sufficient. `vtables.csv`
  is the 2012 table; `match_2012_2013.csv` has propagated an entirely wrong vtable; the 2013 database
  mislabels functions. **Pin a vtable a second way before reading a body out of it.** And remember the
  image base: an address resolved without `+ 0x400000` lands mid-function in an unrelated class, which
  is a convincing-looking wrong answer rather than an obvious failure.
- **Never use the FModel MCP tools.**
- `resources/tools/drive_input.py` with `--exe <image name>`; `hover`/`hoverclick`, not `move`/`click`.
  Prefer `-apshottime=`, and claim its output by exact byte size — it writes into a directory every
  agent shares. **Do not click through a Windows Defender Firewall or Security Alert dialog.**
- Run the harness from **the worktree's own copy** with an **absolute** build dir, built inside it; the
  full set is **37**. Copy the five gitignored `resources/docs/types/` inputs in first.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** Every wave so far at
  least one package has corrected a premise of its own brief; in wave 16 all four did, and one of them
  found the fault it was chasing had been caused by the coordinator's own earlier change.

## Acceptance, common to all four

1. A side-by-side against the retail game for each fault claimed fixed, at the same window size.
2. A before/after against the untouched HEAD executable through the same driver.
3. `run_regression.py` 37 ok, 0 failed, 0 skipped.
4. A clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: 0 errors,
   0 C4263, 0 C4264.
5. `rva_sweep.py` plus a by-hand resolution of every cited address.
6. `resources/docs/agents/agent<XX>.md` and `agent<XX>_status.csv`, written **inside the worktree**.
