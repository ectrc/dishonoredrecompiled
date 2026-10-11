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

## Outcome

### FJ — merged `562f047`, gated 37 ok / 0 failed / 0 skipped

Three files, all in `External/GFx3`. Fast-forwarded onto `98e22d4` mid-run.

**The crash was a dangling target in the action queue.** `GFxMovieRoot::ActionEntry::pTarget` was a
**raw** `GFxSprite*`. Retail holds the character in a `GPtr` at `entry+8` (released by
`ActionQueueType::AddToFreeList`, 2012 `0xa0cb40` / 2013 `0xa03570`) and `ActionEntry::Execute`
(2012 `0xa024d0` / 2013 `0x9f8ed0`) **refuses outright to run an entry whose character is unloaded**:
`if (pCharacter && !(pCharacter[118] & 0x10)) switch (Type)`. `QueueClassBinding` already took a
reference; `PushActionBuffer` did not. A tab switch attaches `o_OptionsList_withTabs`, `_list_mc` and
`lib_scrollBar`, queues their frame-0 buffers, and destroys them later **in the same frame**, so the
drain ran `Env.SetTarget(freed)` and then `Execute`.

The faulting function: `0x1cc148a` = **`GASActionBuffer::Execute + 0x2DA`** (map VA `0x13111b0`,
`GFxAS2Interp.cpp.obj`), at `call dword ptr [eax+0x40]` where `eax = *(target + 0x80)` —
`env->GetTarget()->ToSprite()` on a dead target.

**The unload had to reach the subtree or retail's test is dead.** `GFxCharacter::OnEventUnload` was
`{}` and `GFxDisplayList::Clear` was a bare release loop, so only the clip a script named was ever
marked — and **the first build of the fix reported zero drops across 18 tab switches.** `Clear` now
does retail's two missing passes (2012 `0x9d5e30` / 2013 `0x9cc630`, `0x97` in both: `OnEventUnload`
and `pParent = 0` per entry, then the release sweep) and `GFxSprite::OnEventUnload` (2012
`0x9fdc80`) calls it. The script dispatch is still not ported — only the flag and the propagation.

**Measured deterministically rather than by hoping the crash stays away.** An entry whose target's
`RefCount` is down to the queue's own reference is one that would otherwise have been freed.
18 main-tab selections: GENERAL 5 → **0**, **CONTROLS 5 → 15, exactly three each**, GRAPHICS 4 → 0,
AUDIO 4 → 0. `OnTabSelected (1, r)` is the selection the user's process died on, and **three** is the
number of freed character blocks chained in the dump.

**There was a minidump of the user's crash on disk that nobody had opened** —
`DishonoredGame/Logs/unreal-v9411-2026.10.11-14.36.05.dmp`, written by the same handler that printed
the two useless lines. It confirms every number predicted before it was opened (base `0xdb0000`, eip
rva `0xf1148a`, `0xc0000005` read of `0x40`, `eax = 0`) and hands over the dangling target at
`0x31286120` with its page captured. **Open the dump before rebuilding anything.** The printed stack
is useless only because there is no PDB beside the staged exe, and `CaptureStackTraceHelper` writes
`BackTrace[n] = AddrPC` even when `StackWalk64` fails — which is why one failed walk prints as two
identical frames.

**Six premises of this plan corrected.**

- **"Decode the address first … subtract the image base" cannot work.** The exe is `/DYNAMICBASE`;
  `0x1cc148a − 0x400000` lands 644 KB past the end of a `0x181d000`-byte image. FJ recovered the real
  base **without a debugger**: Windows caches an image's ASLR base per boot, so
  `CreateProcessW(CREATE_SUSPENDED)` on the same unmodified file plus `NtQueryInformationProcess` and
  `PEB->ImageBaseAddress` gave `0xdb0000`, identical on 3 of 3 launches, in the user's own boot
  session.
- **The log shape quoted in §1 is not a crash signature.** Two `InitScrollBar`, the
  `removeMovieClip … on undefined` and the `setMask` line are **every** CONTROLS selection — twice in
  the user's own log *before* the crash (21.64 s, 31.98 s) and 17 more times across three HEAD runs
  that did not crash.
- **All four of the open candidates are in the wrong file.** The fault is in the AS2 interpreter's
  opcode preamble, reached from the action-queue drain. `MaskLevel`, stencil nesting, the
  `>= GFxASDepthBias` test and `CloneDisplayObject` are not on the path, and the crash is in
  **`Advance`, not `Display`** — no `scene rendered` line between 33.51 and the fault.
- **"Get a real stack (a debugger, or a crash handler that walks properly)"** — neither was needed.
- `removeMovieClip`'s `>= 0x4000` guard is not involved: that AS2 error is a **method-resolution**
  failure, so nothing is removed by it. The clip actually destroyed is `_options_mc`, by `SetList`.
- **The one acceptance item not met as written**, stated rather than papered over: the scripted
  reproduction could not be built. Three HEAD runs, same driver, no focus loss, 17 CONTROLS
  selections and ~51 dangling drains produced **zero deaths** — the fatality needs the freed block
  reused within a few hundred microseconds, and no page-heap tooling exists on this machine
  (`gflags` is not installed). So the before/after is on the **defect**, measured, not on the
  symptom.

**Found and left, named rather than touched.** `GASFunctionObject::pDeclTarget` is the other raw
character pointer reaching the same instruction (`GFxAS2Interp.cpp:443`); retail guards it with the
same bit in `GASAsFunctionObject::Invoke` (2012 `0x9f3540` / 2013 `0x9e9d10`), falling back to the
caller's env. **FJ deliberately did not write that guard**, because reading
`pDeclTarget->IsUnloaded()` dereferences exactly the pointer it protects against. The prerequisite is
**retail's movie-root unload list, which is unported**: `GFxSprite::ExecuteEvent` (2012 `0x9f4a10` +
`0xb9`) links every unloaded sprite onto a list at `movieroot+0x27fc` with an explicit
`inc [esi+4]`, and that is what makes any raw `GFxASCharacter*` safe for the rest of an advance in
retail — including `DragState::pCharacter`, which retail also copies as a bare dword with no
`AddRef`, exactly as we do. **Port the unload list, then the `pDeclTarget` guard, in that order.**

**And a regression in `98e22d4` that FJ found in an unrelated dump.** Making `FDebugServer`'s
construction opt-in left the shutdown path dereferencing a null pointer: the reference engine writes
`GDebugChannel->Destroy()` unconditionally and gets away with it only because the object is always
constructed. Every exit after `98e22d4` faulted at `FDebugServer::Destroy + 0x2a`, `0xc0000005` read
of `0x8`. Fixed in **`28cf7ec`** and verified: launch, close, `Exit: Exiting.`, no new minidump.
**Making something optional means auditing everything that assumed it existed.**

**STATUS open defect 8 is closed on FJ's evidence.** After `98e22d4`, `grep -c FocusLost` is **0**
across all eight of its runs and about **80 clicks all landed**. Options main tabs are at
**y = 0.216**, x = **0.259** GENERAL / **0.419** CONTROLS / **0.581** GRAPHICS / **0.741** AUDIO,
with the selected tab rising to y ≈ 0.138. One caveat: in one run `drive_input.py` reported a
**1280x720** client while `-apshottime` wrote a **1600x900** bitmap on the same command line — **a
pixel coordinate off a capture is not a pixel coordinate for the driver.**

### FL — merged `ee2313f`, gated 37 ok / 0 failed / 0 skipped

Four source files, three in `DishonoredGame` and one in `Engine`. Nothing in `UnInterpolation.cpp`,
the `UInterpTrack` family or `External/GFx3`. (Gated on the pre-FM harness, so that run could not
have seen a hard crash in a stage — see FM below.)

**Fault 4's answer is the "neither — it is a third thing" outcome the brief asked for.** The cause is
**`UArkAnimNodeLookAt`**, not `FArkComponentLookat` and not the notifies. Two missing functions:
**`InitAnim`** (2013 rva `0x5138d0`), whose first act in retail is to zero all eighteen pose weights
so the cook's default (entry 0, `ADD_Empty_AimHeadIdle_UpLeft`, weight 1.00) never survives into
play; and **`GetBoneAtoms`** (indexed `0x51b420`, four-arg `0x51bc80`), which fills every atom with
`FBoneAtom::Identity` — the **additive** identity — when a set's total weight is zero.

**The pairing is the finding worth carrying forward: zeroing `InitAnim` alone would have made it
worse.** Stock `UAnimNodeSequenceBlendBase::GetBoneAtoms` answers weights that do not sum to 1 with
`check(FALSE)` + `FillWithRefPose`, and **a reference pose applied as an additive delta is not the
identity.** The two land together or not at all.

Measured with a new `-flheaddiag`, because `-dislookat` could not answer the question: the four NPC
pawns **nearest the camera** (not the first by iteration), with **two** deltas per bone —
`LocalAtoms` vs the bind pose (the tree's output) and `SpaceBases` vs the reference pose composed
down the same chain (what the renderer uses). **That distinction is the whole diagnosis.** Before, on
the two pawns the user sees: `neck_jnt l34/c32; head_jnt l60/c84; head_end_jnt l0/c84`, spine inside
2°, `nonzero: 4:ADD_Empty_AimHeadIdle_Center=1.00`. After: `neck_jnt l0/c1; head_jnt l1/c0`,
`nonzero: all zero`. `FArkComponentLookat` is **still 100 % absent** and the heads are level.

**Six premises of this plan corrected.**

- "51 functions and not one is written" is true as a statement, **false as a cause**.
- **The 396 `Found bad AnimNotify` errors are not ours.** 5,637 notifies load, 396 (7.0 %) arrive
  outered to another `AnimSequence` exactly as the cooked file's own `FObjectExport::OuterIndex` says,
  and **396 of 396 are repaired by stock UE3's `Rename` in `UAnimSequence::PostLoad`**, byte-identical
  to the reference engine. The objects are **shared** — export 16799 is listed by both
  `AnimSequence_4` and `_15`, and a shared notify can only be outered to one of them. Asset artefact,
  retail shares it, already repaired, and a notify fires out of the `Notifies` array regardless of
  its Outer.
- **There is no look-at skel control on these pawns at all**: `NPC_Human_at`'s five lists are
  `SpineBender`, two `SkelControlSingleBone` on the eyes, and two `FootPlacement`.
- Agent DO's note that `OnComposeSkeleton` overrides `head_jnt` is wrong (it is the **eyes**, clamped
  against `MIN/MAX_EYE_YAW`/`PITCH`), and DO's conclusion that the grid centre is retail's rest state
  is wrong — measured as a pair, `UpLeft` at 1.00 gives head l62/c84 and the centre at 1.00 gives
  l60/c84. **The centre is a pose, not a rest state.** Both corrected in the tree.
- "A standing guard may be a cheaper control" would have measured **the wrong pawn** — `-dislookat`
  already reported a guard as healthy at 2° from bind. The affected pawns are the two the opening
  scene spawns, last by iteration order.

Eight functions ported, all hand-resolved in both databases. **One deliberate divergence, named in
the source**: retail's indexed `GetBoneAtoms` is virtual; here it is a private non-virtual
`GetBoneAtomsForSet`, because a second virtual overload overriding no base virtual is **C4263**, an
error in this tree. Plus three `IsValidIndex` guards where retail would let `Array.h`'s assert fire,
which for a player is a crash.

**Said plainly rather than claimed: the node is NOT proven to turn a head by the right amount.** The
head and torso grids are deltas from a shared base that `ComputeHeadAndTorsoAim` drives together, and
`-flaimx/-flaimy` drives the head grid alone, which retail never does — so those runs prove the two
branches agree and nothing about magnitude. **Nothing can until `FArkComponentLookat` exists**, whose
51 functions are inventoried with 2013 rvas in the report, with the trap recorded: when `m_pConfig`
becomes non-NULL, `UpdateNodeWeights` fed a raw (0,0) puts **1.00 on the grid centre**, worth 60° of
head.

**`rva_sweep.py` passed eight VAs silently** when written beside their own RVAs (`0x9138d0`
classified `ok-2013`) — the same failure recorded for FH's `0xdf0a00`, in the other direction.

Left for someone else: **`UDisSkelControl_SpineBender`** (9 functions, 0 written,
`CalculateNewBoneTransforms` 2012 `0x6daf30`) is live on every NPC at `spine_0_jnt` with
`ControlStrength` 1.00 and is a **silent no-op** today.

### FM — merged `cd5fadd`, gated 46 ok / 1 failed / 0 skipped, and the one red row is real

Six source and tool files plus the baseline and 25 evidence files.

**The headline is not the new checks. The gate could not see a crash at all.** The three
`*_criticals` metrics matched the literal **`Critical: `** — what `appError` logs through
`NAME_Critical`. An access violation **never produces that line**; it reaches the log through the
structured-exception handler as **`=== Critical error: ===`**. The user's crash log contains
`Critical: ` **zero** times. So for every wave this harness has run, **a hard crash inside a stage
was invisible to it**, and every "37 ok, 0 failed" was worth less than it was reported to be. The
coordinator confirmed this independently in its own `gate_rel` inputtest log: `=== Critical error:`
once, `Critical: ` zero. `CRITICAL_RE` now matches both forms.

**With the pattern widened, a real crash surfaced on the first run and it is still failing.** At
**45.85 s of a 150 s budget**, `-distouchprobe` walks the pawn onto `DisTrigger_9`, which fires
`SOIREE_Guards.SeqEvent_Touch_0`, which runs `DisSeqAct_StartSpawn_0`, and **the spawn kills the
process**. Every metric that stage measures is emitted at 45.67 s, **which is exactly why it could
pass**. Not FM's: that stage passes `-nodishud` and never sees `-playdefaults`, and the identical
line is in agent FJ's gate run, agent FI's run and the coordinator's own gate run of HEAD.
Intermittent — FG's and FH's logs have none. Nine of the ten stack frames resolve thousands of bytes
past a public symbol and should not be believed; the one credible frame is
**`UArkComponentContainer::AddNewComponent_Internal+0x2d`**.

**The bound stays at `== 0`.** FM left it there and said relaxing it to `<= 1` was the coordinator's
call; the coordinator declined, because that one line would make the board green and hide the next
crash too. **The gate is red until this is triaged, and triaging it is wave 20's first item.**

**The new total is 47**, ten new rows, five per stage: `newgame_log_lines`,
`newgame_menu_open_seconds`, `newgame_on_mission`, `newgame_pawns`, `newgame_criticals`,
`options_log_lines`, `options_steps`, `options_alive`, `options_as2_errors`, `options_criticals`.
**Every package's expectation becomes 47.**

**New Game stage**: `-playdefaults` turns the `DISHONORED_PLAY_DEFAULTS` table on in a build not
configured with it (one `#if` moved in `appInit`), so the stage's command line is the user's plus the
usual isolation with **no `-startmap`**, and the other five stages never see it. Measured: menu movie
open to world 21.6 at `screen 3 loginStep 8`, New Game fires, 136 `l_tower_p` census lines, **29
pawns**, 0 criticals.

**Options stage**: it reproduces the user's AS2 sequence **exactly** — same traces, `InitScrollBar
(385.699066162109, …)` twice, `mask stage bounds 170.0,214.6..1151.7,666.2 px`, the frame the process
died on — **and the process lives. Six runs, 0 criticals.** So it does **not** fail on `c702a38`, and
FM said so rather than shipping a check that looks like it tests the crash.

**`-dishud` is flipped and the user's fault 2 is closed.** Retail has **no gate**: `PostBeginPlay`
(2013 rva `0x6155d0`) → `InitGlobalManagers` (`0x5e9bd0`, **zero conditional branches**) →
`UDisGlobalUIManager::Init` (`0x85fdb0`) → `StartHUD` (`0x78cd00`), unconditional end to end, with
Init's only real branch a `Cast<>` picking challenge-mode movies whose arms converge three
instructions before the `StartHUD` call. The hard anchor is the **named** `OnMovieStackChanged` at
`0x84cf70`, five bytes `E9 FB A0 FF FF` = `jmp 0x847070`. The shipped build now opens the HUD,
`-nodishud` suppresses it, and the harness passes `-nodishud` in nullrhi/d3d9/inputtest — **no
calibrated threshold moved**, confirmed by the full run.

**The flip alone crashed the menu**: 2 of 3 runs died with
`Assertion failed: !Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading)` 0.02 s after opening
`UI_Global.Global`, because **retail has one owner for every movie player and this tree has two
racing bring-ups**. `DisHUDBringUp` now waits, **latched**, for `DisGFxAutoOpenMenu` to be open —
latched because an unlatched wait holds the HUD shut for the whole game after the map change, which
was measured. With the guard, 0 of 3 crashed, and a play-defaults New Game logs
**`HUD: SetBlackStripes(1) ok`**. **The real fix is to port `UDisGlobalUIManager::Init` and delete
both bring-ups.** Note: every run without `-nodishud` now has the HUD movie open.

**Five premises of this plan corrected.**

- **`c702a38`'s code IS HEAD's code** — the diff outside `resources/docs/` and `README.md` is empty,
  so no second worktree was needed.
- **`-finewgame` alone would NOT have caught `b540be6`**, the regression this package exists to stop
  recurring: it issues the console command from the game thread **without touching the menu**. Hence
  `-fmmenu` and `newgame_menu_open_seconds` — the last world time the menu movie was still open,
  which reads ~1 on `b540be6` against a bound of 10. **That is the row that actually closes the blind
  spot.**
- "Two `OnTabSelected` in one frame, `InitScrollBar` twice" is **not** the anomaly — normal in this
  build at 17.47, 21.64 and 31.98 with no crash. Agent FJ reached the same conclusion independently.
- **The user did not launch with no arguments**: `play_live.log:7` is `-LOG= -forcelogflush` plus the
  six appended defaults. The coordinator wrote otherwise in this plan. The crash is exactly
  reproducible.
- **`RefreshGlobalUIState` is not unported** — it is `disglobaluimanager.cpp:113` with the blur arm
  ported by agent FA, and **it opens and closes nothing**.

**For FJ, with a measurement**: the difference between a reproduction that lives and the user's crash
is **the entry point**, and it is in the log. `AS2 trace: OnMouseUp (0, undefined)` is the line
immediately before the fatal `OnTabSelected`, so the user's switch was dispatched **from inside GFx's
own mouse handling**, re-entrant with the runtime's advance/display state, where `GFxMovieView::Invoke`
enters at a clean point. Invoking `TabsHandler::OnTabClicked` is still not enough. **That is this
plan's own first candidate — `ctx.MaskLevel` across a re-entrant `Display` — with evidence in its
favour.** The user also **dragged the scroll bar to the bottom** before the fatal switch
(`OnLifterPressed` → `ratioScroll` 0.623 → 0.999964), which FM could not drive because
`_options_mc.MoveTo_End()` resolves NOT FOUND.

Also recorded: **`rva_sweep.py` does not scan banner `/* … */` comment blocks**, so six of FM's seven
addresses were invisible to a clean sweep; `TArray<GFxValue>` is unsafe (destructor plus
uninitialised `TArray::Add`); the letterbox exists in **two** movies with different `SetBlackStripes`
arities and the `UI_Global` one (`0x7aa610`) is **absent**; and `m_bShowHUDWhileActive`'s default
lives in cooked script defaults, which would make an open HUD invisible.

### FK — merged `af98872`, gated 46 ok / 1 failed / 0 skipped (the known row)

Twenty files. Two units off `Sources.cmake` with external-linkage anchors verified in the map and in a
run. No header regeneration. Rebased onto `ee2313f` then `cd5fadd`; FL's 703 lines and FK's six
coexist in `dishonoredplayercontroller.cpp`.

**A Dishonored cutscene matinee with a director group and a player group now initialises and plays,
with the player's pawn bound to its player group and blended to its stage mark.** Before/after on one
binary with `-fknoplayergroup` as the control: `SOIREE_Empress_Regent.SeqAct_Interp_26` forced, the
`PlayerGroup` instances as **`InterpGroupInstPlayer` on `DishonoredPlayerPawn`** where the control gives
a bare `InterpGroupInst` with no actor; the pawn moves **595.8 units** to its mark, `blendTime 0.20`.

**What was actually stopping every Dishonored cutscene was none of this plan's three named gaps.**
`UInterpGroupInst::InitGroupInst` raised the **script** event `BeginAnimControl`. `eventBeginAnimControl`
is `FindFunctionChecked`, Dishonored's script `Actor`/`Pawn` do not declare it, and **retail does not
use the script path at all** — retail's `InitGroupInst` (2012 `0x231210`, read in full) calls the native
`AActor::MAT_BeginAnimControl`. So **the first Dishonored cutscene to reach `InitGroupInst` killed the
process, every time**, on the first group with an anim-control track — every AI group and every player
group in the game. Three successive `appError`s, one per measured run: `BeginAnimControl` →
`RestoreAnimSetsToDefault` → `BuildScriptAnimSetList` → none. Four edits close it.

**Five premises of this plan corrected.**

- **"All seven matinees are set dressing with `director NONE`"** was true of the seven that *run*, and
  it hid the level: `l_tower_p` holds **91 `SeqAct_Interp`, 396 groups, 694 tracks, 4 director
  groups, 6 player groups, 99 AI groups.** The opening cutscene is
  `L_Tower_Script…SOIREE_Empress_Regent.SeqAct_Interp_12` — 38 groups, eight `InterpGroupAI`, a
  `PlayerGroup`, a `DirGroup` — and its **activation count is 0 across a whole new-game route.**
- **"The player group first"** was the wrong order and cost most of the budget. The cutscene is never
  activated, and when forced it died on the anim-control handshake before any group finished
  instancing. The player group is third.
- **"`UpdateTrack` is missing on every Dishonored `UInterpTrack`"** is wrong for six of nine. Only
  `AIControl`, `Attachment`, `DisAllowPlayerLook` use `UpdateTrack`. `FaceTo`, `Locomotion`,
  `StretchAnimControl`, `Dialog` use **`UInterpTrackInst*::UpdateTrackInst(FLOAT, UBOOL)` — a virtual
  that exists nowhere in this tree** — and `LookAt` a static. The gap is a missing **seam**, and
  `StretchAnimControl` is **267 of 694 tracks** and on all six player groups.
- **"`MAT_*`, about forty"** is 22 on `NPCPawn`, 8 on `ADishonoredPawn`, 5 on `ADishonoredPlayerPawn`,
  6 on preview classes. The player group needs the player five, and `MAT_BeginPlayerGroup` requests a
  state change into `UStatePlayerMasterSoiree`, an **excluded unit**. `m_pMatineeBlender` and
  `UInterpGroup::HasMoveTrack` do not exist here at all.
- **"Twelve of fourteen conditions missing, not cosmetic"**: count right, consequence wrong here. Two
  have no `.cpp`; **eight of the remaining ten are blocked on a helper in another excluded unit**. And
  `DisSeqCond_CompareTweaks`, the one FI said fires on this mission, fires 6× and **the link it takes
  has `0 connections` every time** — `ops ever activated` stayed at FI's exact 65/1670 and 47/1629.
  The missing body was killing nothing here.

Also: `UInterpGroupInst::UpdateGroupInst` and `UInterpGroup::NeedWaitToStart` do not exist in this
tree, so the blend is ported onto `UInterpGroupPlayer::UpdateGroup`. And **`-finewgame=<secs>` is on
the world clock, which starts before the first map loads** — under load `LoadMap` began at 27.6 s, so
`-finewgame=22` fired into an async load and asserted in `StaticAllocateObject`. The harness's
`--newgame-at` should go up (→ FN).

**Ported**: `UInterpGroupPlayer::{NeedWaitToStart, UpdateGroup}`,
`UInterpGroupInstPlayer::{InitGroupInst, TermGroupInst, GetStageMarkPosition, BlendPlayerToMark}` —
all nine retail functions **identical in size in both executables**, three deliberately not ported
(two are `jmp`s to the base, `HasActor` is byte-identical to the base's inline). The `InitInterp`
player-group branch via `GDisInterpPlayerGroupHook`. `UDisSeqCond_CompareTweaks::Activated` +
`ExtractTweak`, `UDisSeqCond_CompareBoolExtended::Activated`. `-fkdiag` / `-fkdiagat` /
`-fknoplayergroup`.

**The red row, with FK's control.** Three measurements on the `inputtest` stage: this package 1, this
package with `-fknoplayergroup` 1, **and a clean rebuild of `cd5fadd` with none of FK's changes also
1** — same world time, same preceding lines. The trigger is a pawn from `DisSeqAct_StartSpawn_0` one
frame earlier, with **`an Actor may only be connected to one Group! [DishonoredSpawner_11]`** on the
line before — the same spawner-as-group-actor problem as hand-over item 2. → FN.

**Not reached, specified in `agentFK.md` §9**, in order: what activates `SeqAct_Interp_12` (**now the
only known blocker to the user-visible cutscene**); `UInterpGroupInstAI::InitGroupInst` with a
spawner group actor; the `UpdateTrackInst` seam plus nine track bodies; `AActor::MAT_SetPhysics` /
`BackupCollisionType`, `MAT_BeginAnimControl` as an `AActor` virtual; the `MAT_*` contract (blocked
on the player FSM); `NeedWaitToStart`'s caller, the eight blocked conditions and their ten leaf
helpers, two DLC conditions, `InitInterp`'s three retail differences; and an **intermittent assertion
on the front end's own `PrepareMapChange`**, 2 of 10 runs, undiagnosed.

Three condition rvas are **flagged as not independently pinned** (match-table neighbours, differing
sizes). Four `#include "CppText/*.h"` lines in generated bodies and the `GDishonoredFKLinkAnchors`
block are flagged as what a regeneration would drop.

## Wave result

**Four of four merged**: FJ `562f047` (37/37), FL `ee2313f` (37/37, pre-FM harness), FM `cd5fadd`
(46/47), FK `af98872` (46/47). Plus the coordinator's `98e22d4` (the listeners) and `28cf7ec` (its own
regression from that).

**Of the user's four reports**: the Options crash is fixed at the defect (FJ) though the user's exact
re-entrant path is not reproduced (→ FQ); the **black bars draw from the shipped command line** (FM);
the **heads are level** (FL), magnitude unproven until `FArkComponentLookat` exists; and the cutscene
**initialises and plays when forced** (FK) but **is never activated** on the real route (→ FO).

**What the wave found that nobody briefed**: the gate could not see a crash for nineteen waves; there
were minidumps on disk for every one of them; the firewall prompts that cost five waves were three of
our own listeners; the exit faulted after the listeners were removed; the bars were ported and
unreachable for a whole wave; and the thing killing every cutscene was a script event retail never
raises. **Every one of the four packages found its cause somewhere the plan had not pointed.** FJ
corrected six premises, FL six, FK five, FM five.

The gate is **red on purpose** at 46/47 — see `STATUS.md` — and wave 20 (`PHASE16.md`) starts there.
