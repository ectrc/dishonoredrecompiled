# Wave 19 — the crash, the cutscene, the heads, and the gate that missed both

The user played the wave-18 build from the shipped executable with no arguments and reported four
things. Three are in this wave; the fourth is a crash they hit on the way.

> when going into a new game the pawns spawn but still there are no black bars and all of the
> animations and sequences do not happen. Also all of the pawns heads are looking at the sky

And, from the launch before it, a hard crash on the Options screen.

**Every one of these is ours, and two of them are things this project told the user were done.** That
is the shape of the wave: the previous one measured its fixes in instrumented runs and did not check
that they held for the way the game is actually launched.

## THE CAPTURE RULE — unchanged from wave 18, and it worked

Take every screenshot **inside the game process** with `-apshottime=<seconds>` and
**`-apshotname=<your tag>`**, then glob your own tag. **Never** use a desktop or window capture —
GDI, `BitBlt`, `PrintWindow`, PowerShell grabs, or the coordinator's `build/coord_shot.py`. They read
the composited desktop and will return black, stale, or another agent's window.

Three machine facts, all measured last wave, all still true:

- **The desktop resolution flaps between 1024x768 and 6144x1728**, so the same `-ResX=1600 -ResY=900`
  yields a 1008x567 client area on about eight runs in ten and 1600x900 on the rest. **Read every
  capture's real size off the file. Never assume it.**
- **A concurrent agent's game steals the foreground and kills the menu** — one `execOnFocusLost` and
  the movie never draws again. `grep -c FocusLost <log>` is a clean validity flag: **discard such a
  run, do not average it in.**
- **`git apply` run from a non-repo subdirectory inside the repo silently applies nothing and exits
  0.** It cost agent FG a twenty-minute build whose "combined" executable was plain HEAD. Use
  `patch -p1 -d <dir>`.

## The four faults

### 1 — the crash on the Options screen. **Highest priority: it is a crash, and it is new.**

Reproduced by the user, and the coordinator has the log
(`<retail install>\DishonoredGame\Logs\play_live.log`). Options → a tab switch, **33.77 s**, dies
0.15 s after the last successful line. The run before the fault, in order:

```
[0033.46] AS2 trace: ----------->> OnTabSelected (1, r)  / undefined
[0033.46] AS2 trace: >> UpdateSubTabs
[0033.46] AS2 error: ... 'ClearAnimation' ... on undefined
[0033.46] AS2 error: ... 'tweenEnd' / 'tweenTo' ... on undefined      (x5)
[0033.46] AS2 error: ... 'gotoAndStop' / 'gotoAndPlay' ... on undefined
[0033.46] AS2 trace: >> OnTabSelected : 0
[0033.61] AS2 trace: >>> InitScrollBar (385.699066162109, [object Object], LS)
[0033.62] AS2 error: ... 'removeMovieClip' at pc 1068 of a 2730-byte buffer, on undefined
[0033.62] AS2 setMask: _level0.optionsMenu_mc._options_mc._list_mc masked by
          _level0.optionsMenu_mc._options_mc.maskContent, mask stage bounds 170.0,214.6..1151.7,666.2
[0033.77] === Critical error: ===  Fatal error!  Address = 0x1cc148a (filename not found)
```

**Two tab switches in one frame, a scroll bar initialised twice, and a mask installed 0.15 s before
the process dies.** The code in those two lines is **new as of wave 18** — `setMask` and the AS depth
bias are FF's (`63c2539`), `MarkAllEntriesForRemoval` and `GotoFrame`'s argument are FG's
(`c702a38`), and `removeMovieClip`'s guard is the signed `>= 0x4000` test FF rewrote.

**The coordinator looked for the obvious cause and did not find it, so do not start from a
hypothesis.** `GFxDisplayPushScriptedMask` **does** guard `mask->GetParent() == 0`.
`GFxCharacter::SetMask` **does** `AddRef`/`Release` the mask, and the destructor clears the
back-pointer in both directions. Candidates still open, in no particular order:

- `ctx.MaskLevel` / `MaskDrawDepth` accounting across a nested or re-entrant `Display`, and what the
  D3D9 renderer does when `Mask_Increment` nests past its stencil capability.
- A mask whose owner is re-created by the tab switch while the old mask is still installed —
  `InitScrollBar` runs **twice** in the same frame here.
- The `>= GFxASDepthBias` test in `MarkAllEntriesForRemoval` against a character whose depth is read
  after it leaves the display list.
- `GFxSprite::CloneDisplayObject`, new last wave and reachable from `duplicateMovieClip`.

**Decode the address first.** `0x1cc148a` is a VA in our own executable: subtract the image base and
resolve it against the build's map file, or rebuild with one. **A stack of two identical frames means
the walker failed, not that the fault is one frame deep** — STATUS records the time the coordinator
read thirteen "frames" that were a UTF-16 string. Get a real stack (a debugger, or
`-forcelogflush` plus a crash handler that walks properly) before writing a line of fix.

Deliver: the reproduction as a scripted run, the decoded faulting function, the fix, and a
before/after on the same schedule.

### 2 — no black bars, and the reason is a command-line switch

**Confirmed by the coordinator before this plan was written.** `Tick_Cinematic` turns the cinematic
mask bit into `_root.blackStripes_mc`, but that clip lives in the **HUD movie**, and the HUD movie
only opens behind **`-dishud`** (`disgfxmovieplayerhud.cpp:1329`:
`State = ParseParam(appCmdLine(), TEXT("dishud")) ? 0 : -1;`). **`-dishud` is not in
`DISHONORED_PLAY_DEFAULTS`** (`Core/Src/UnMisc.cpp:5092` — `nosteam`, `skipnativepkgs`, `gfxuimenu`,
`nomovie`, `windowed`, `ResX` and nothing else).

The user's log proves the consequence: **`blackStripes` 0 mentions, `cinematic` 0, `matinee` 0** in a
no-argument run that reached the mission.

So the question is not "why don't the bars draw" but **"why is the HUD behind a switch at all, and
what does retail gate it on?"** The switch exists because the regression harness's thresholds were
calibrated without a HUD movie (`disgfxmovieplayerhud.cpp:1233`). That was a reasonable bring-up
decision and it is now hiding a finished feature from the person playing the game. Find retail's own
condition for opening the HUD movie — `UDisGlobalUIManager::RefreshGlobalUIState` (2012 `0x8b75e0`,
unported, named by agent FI) is the obvious place to read it — and make the shipped build open the
HUD the way retail does, without breaking the harness. **Coordinate with FM**, which owns the
harness side.

### 3 — the animations and sequences do not happen

**Not a regression: a hole agent FI scoped and left, and the coordinator relayed to the user as "the
next wave's shape".** Three named gaps, all from FI's report:

- **`UInterpGroupPlayer` and `UInterpGroupInstPlayer` are entirely unported, *and*
  `USeqAct_Interp::InitInterp` has no branch that would build one.** A matinee therefore has no
  player group, which is why **all seven matinees live in `l_tower_p` have `director NONE`** and five
  frames 45–110 s apart differ by a mean of 2.0/255.
- **`ADishonoredNPCPawn::MAT_*` — the whole matinee-AI contract, about forty functions, zero
  bodies.** Nothing can hand a pawn to a cutscene.
- **`UpdateTrack` is missing on every Dishonored-added `UInterpTrack`.**

Alongside it, FI measured **twelve of fourteen `UDisSeqCond_*::Activated` missing**, two of which
already fire on this mission's path, with all twelve 2013 addresses hand-resolved. **The skeleton
comments in those files carry 2012 rvas — that is a trap, and FI said so.** An unported
`USequenceCondition` kills every Kismet chain it sits on **silently** (STATUS, agent EL), so these
are not cosmetic.

Take it in that order: the group player first, because without it nothing else can be observed; then
`UpdateTrack`; then the `MAT_*` contract; then the conditions. **If the wave runs out before the
`MAT_*` contract, say so and specify what is left** — a specification is worth more here than a
rushed half.

### 4 — every pawn's head is looking at the sky

Two findings, and the second was not expected.

**`FArkComponentLookat` has fifty-one functions attributed to it and not one of them is written.**
`Engine/Src/arkcomponentlookat.cpp` is a comment-only skeleton; so are `arkanimnodelookat.cpp` and
`arkcomponentlookatconfig.cpp`. The absent bodies include `GetLookAtNeutralDirection`, `GetHeadAt`,
`GetCurrentLookAtDirection`, both `LookAt` overloads and `AimAt`. **Our own log says it outright**
(`play_live2.log:3168`):

```
Warning: the AI issued a desire but FArkComponentLookat::StartProceduralLookAt is not ported;
         the request is resolved and counted, not executed
```

`USkelControlLookAt` itself **is** implemented (`Engine/Src/UnSkelControl.cpp`), so the bone
controller runs every frame with no one setting its target.

**And separately: 396 `Found bad AnimNotify` errors in one run, 143 of them
`DisNotify_NPCDisableLookAt`.** The pattern is an off-by-one in ownership — the notify listed on
`AnimSequence_39` is owned by `AnimSequence_38`, `_9`'s by `_7`, `_76`'s by `_71`. Something in our
loader is attaching notifies to the **wrong `AnimSequence`**, and `DisNotify_NPCDisableLookAt` is
precisely the notify that would switch the look-at **off** during a scripted animation.

**Do not assume these are one fault, and do not assume either is the cause of the sky-gaze.** Measure
the head bone's actual world rotation and what writes it, then decide. The honest outcomes here are
"the look-at component is absent and that is why", "the notifies are mis-linked and that is why",
"both contribute, in these proportions", or "neither — it is a third thing, which is this". Any of
the four is a good result. **A fix that moves the heads without naming what was writing them is
not.**

## Packages

### FJ — the Options crash

Fault 1, alone. It is a crash in code three days old, it is reproducible from the shipped executable,
and everything else in this wave is cosmetic by comparison. **This package owns
`External/GFx3`'s mask and display-list code.** Do not grow it: if you find a second defect in GFx3,
name it and leave it.

Start by getting a real stack. Then reproduce it in a scripted run so the fix has a control.

### FK — the matinee

Fault 3. The largest package in the wave and the one most likely to run out of room, so **sequence it
and report what you did not reach.** `UInterpGroupPlayer` / `UInterpGroupInstPlayer` /
`InitInterp`'s missing branch first.

### FL — the heads, and the notifies

Fault 4, both halves, because they may be one fault and the same measurement distinguishes them.
Measure before you port. `FArkComponentLookat` is fifty-one functions; **port what the opening scene
and a standing guard actually reach, and inventory the rest** — the 1,053-placeholder mistake is in
this file's history too.

### FM — the gate that missed two regressions, and the HUD switch

**Wave 18 merged a commit that closed the front end's own menu one second after it opened, and it
passed 37/37.** Wave 19 is being written because the user found a crash on the Options screen that
37/37 also did not see. Both have the same cause: **every harness stage drives a map directly with
`-startmap`, so nothing presses New Game and nothing opens Options.**

Deliver:

1. **A New Game check**: boot with no arguments as the user does, reach the mission, assert the pawn
   exists and the map is `l_tower_p`. Agent FI's `-finewgame[=<secs>]` already drives this and exists
   in the tree.
2. **An Options check**: open Options, switch tabs the way the crash log shows, and assert the
   process is still alive and the AS2 error count has not risen. **This check must fail on
   `c702a38`** — if it passes, it is not testing the crash, and you should say so rather than ship a
   green check.
3. The `-dishud` question from fault 2, with FJ's and FK's needs in mind: the harness's thresholds
   were calibrated without a HUD movie, so decide whether the shipped default opens the HUD and the
   harness suppresses it, or the reverse, and write down why.

Any new check must be **load-sensitive-aware** — `run_regression.py` already has a `LOAD_SENSITIVE`
retry for the frame-rate metrics, and four agents building at once is this machine's normal state.
Report the new total; it will not be 37.

## Rules for agents

- **Capture in-process only, with your own `-apshotname` tag.** See the capture rule above.
- Work **only** in your own worktree. Your sync script runs **worktree → main** and **flags rather
  than copies** anything changed in main since your base. Name **every** function you touch in a
  shared file, per file — wave 18's one semantic collision (`bScriptCreated`, deleted by one package
  and used by another) **would have survived a three-way textual merge** and was caught only by a
  build.
- **Prefer a measurement in the real game over an argument about it.** `dismod_harness.md` hooks the
  **retail** game; use it when you are about to decide what retail does.
- **Resolve every retail address yourself.** `rva_sweep.py` is necessary but **not sufficient** — it
  passed `0xdf0a00` for `0x9f0a00` silently last wave, and the by-hand pass caught it. `vtables.csv`
  is the **2012** table; `match_2012_2013.csv` has propagated an entirely wrong vtable; the 2013
  database mislabels functions; **skeleton comments carry 2012 rvas**. Remember **`+ 0x400000`**.
- **A unit taken off `Sources.cmake` can compile, link green, and not be in the executable** —
  `IMPLEMENT_CLASS`'s vtable is a folded COMDAT, not a reference into the unit. It needs an
  external-linkage anchor, and the symptom is a virtual that will not dispatch.
- **Never use the FModel MCP tools.**
- `run_regression.py` from **the worktree's own copy** with an **absolute** build dir, built inside
  the harness; the set is **37** unless FM has landed. Copy the five gitignored
  `resources/docs/types/` inputs in first.
- C4263 and C4264 are errors. `DISHONORED_SHIM_STATIC` is `inline static`, shared process-wide.
- **Assume this plan is wrong somewhere and say where, with the measurement.** In wave 18 all four
  packages corrected at least three premises of their own brief, two packages corrected each other,
  and two things this repository had recorded as fact were overturned. That is the expected outcome,
  not an exception.

## Acceptance, common to all four

1. For anything claimed fixed: a before/after against the untouched HEAD executable through the same
   driver, and a side-by-side against the retail game where the fault is visual.
2. `run_regression.py` 37 ok, 0 failed, 0 skipped (or FM's new total).
3. A clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: 0 errors,
   0 C4263, 0 C4264.
4. `rva_sweep.py` plus a by-hand resolution of every address cited, with its function start and size
   in both databases.
5. `resources/docs/agents/agent<XX>.md` and `agent<XX>_status.csv`, written **inside the worktree**.
6. A `build/agent<XX>_sync.py` naming every file and, for shared files, every function.
