# Agent EG — the modal: two movies were sharing one constant pool (2026-09-29)

The user, with two screenshots:

> `resources/reference/menu/your_menu.png` against `real_menu.png`. With a modal on screen — QUIT GAME,
> or Enter from the brightness screen — **our whole screen goes near-black**: the scene, the menu and
> the modal's own content are all gone, leaving the cursor and a faint sliver.

| | |
|---|---|
| Package | the modal: the scene behind it, the torn bar, the question and the YES/NO row |
| Starts at | HEAD `2650390` |
| Files | 10, all in `External/GFx3` and the `GFxUI` module; 2 of them are citation fixes this package's brief asked for |
| Regression | **`31 ok, 0 failed, 0 skipped, 423s`** on the clean gate build `build/agentEG_wtrel` (§7) |
| Clean build | the snapshot worktree `build/agentEG_wt` (HEAD + this package), build directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: **994 edges, 0 errors**, 0 `C4263`, 0 `C4264` |

**`build/agentEG/modal_quit.png`** is the result, and **`build/agentEG/modal_quit_before.png`** is the
same modal, reached by the same driver with the same key presses, on the untouched HEAD executable the
user played. **`build/agentEG/modal_brightness.png`** is the New Game confirmation, which is the second
box on the user's list.

## 1. Result

| Accept | State |
|---|---|
| a screenshot of the quit modal matching `real_menu.png` — scene behind a dim, bar, question, YES/NO row | **done**, `build/agentEG/modal_quit.png`, reached with real `SendInput` key presses on the running game, nothing forced visible |
| the same for the brightness-screen confirmation | **done**, `build/agentEG/modal_brightness.png` — `Creating a new game will overwrite previous autosaves. Continue?` It is the same `_level0.msgBox_mc.messageBox_mc`, so it is the same box and the same fix |
| `tweenEnd` / `tween__start` unresolved calls at 0, script errors before and after | **§6.** 0 and 0. On the user's own QUIT GAME path: **572 script errors -> 14**, and every one of the 14 is the benign `GotoLabeledFrame: no frame named 'PC'` that retail logs too. **0 unresolved calls of any name at all** |
| the menu bar fades in promptly, with the measured time | **§5.** Fully opaque **0.9 s** after the start screen is dismissed. It is **0.8 s at HEAD** — agent DM's ~30 s does not reproduce on this HEAD and the number is stale; I say so rather than claiming a fix I did not make |
| `run_regression.py` 31 checks 0 failures, own build dir, plus a clean full release build | **§7** |
| report + `agentEG_status.csv` | this file; 17 rows |

## 2. The defect, measured before it was named

Agent EA's hand-over 3 asked for exactly one measurement: at the "call of a value that is not a
function" site, whether the receiver's movie root's `Prototypes[Proto_MovieClip]` holds the name at
that instant. `-gfxuitweendiag=<n>` (§8) answers it, and adds the question EA's phrasing implies but
does not ask: whether it holds the name **by identity** (the interned-pointer compare
`GASObject::FindNode` makes) or only **by text**.

One run of the menu, first burst, `build/agentEG/run1_log.txt`:

```
NOTAFN 'tween__start' node 2A57E9A8 hash ac726d0a size 12 | strings 23F74504 | version 10 caseless 0
       | root 169A7000 def '/ package/UI_MainMenu/MainMenu'
  receiver 2435CE40 '_level0.vignette_mc._vignette_mc' gc 23F74500 strings 23F74504
  pProto+0           098E08A0 members  28  identity no   text YES node 1C557E60 hash ac726d0a
  MovieClip.proto+0  098E08A0 members  28  identity no   text YES node 1C557E60 hash ac726d0a
```

**The member is there. The name is not the same object.** Two interned `GASStringNode`s for the same
twelve characters, with the same hash, one written and a different one looked up. A `GASStringManager`
cannot hold two nodes for one string — `CreateStringNode` interns — so two managers are in play, and
`GASString::operator==` is a node-pointer compare, in retail as here. Hence a miss.

The second measurement names the writer. `-gfxuitweendiag` also logs every member write with the
manager that interned its name, and the birth of every `GASGlobalContext`
(`build/agentEG/run3_log.txt`, HEAD):

```
GCBIRTH gc 23A8CA80 strings 23A8CA84 version 10 root 18040000
GCBIRTH gc 23A84500 strings 23A84504 version 10 root 16A97000
MEMBERSET 'tweenTo' ... on object 246098D0 strings 23A84504     <- gfx.motion.Tween.prototype
... 'tweenFrom' 'tweenEnd' 'tween__start' 'tween__to' 'tween__run', the class body's order
MEMBERSET 'tweenTo' ... on object 17FE08D0 strings 23A84504     <- MovieClip.prototype
... 'tweenFrom' 'tweenEnd' 'tween__run' 'tween__to' 'tween__start', the CONSTRUCTOR's order
```

Two movie roots, two contexts, two string managers — and **one manager's strings written into both**.

### 2.1 Why: the constant pool is on the tag, and the tag is shared

`gfx.motion.Tween` is a `DoInitAction` in a movie definition that **both** open movies import. In this
tree the bytecode and the interned constant pool live in one object, `GASActionBuffer`, embedded in
the tag (`GASDoActionTag::Buffer`, `GASDoInitActionTag::Buffer`) in the shared `GFxMovieDataDef`. The
`ActionConstantPool` opcode re-ran `ProcessDeclDict` on **every** execution, with whichever movie's
`GASStringContext` was running, and replaced the pool.

So:

1. movie A runs the shared Tween buffer; its pool is A's nodes; the mixin lands on A's
   `MovieClip.prototype` under A's nodes — consistent at that instant;
2. movie B runs the same buffer; the pool is replaced with B's nodes; the mixin lands on B's
   prototype;
3. any later call out of that buffer — and `tweenTo`'s body is inside it, at buffer pc 973 of 1774,
   which is the pc every failure reported — pushes **whichever movie's nodes the pool holds now** and
   looks the name up on **the receiver's own** prototype. Cross-movie, that is a miss, forever.

That is why `tweenTo` answered and `tween__start` did not, from the same prototype, one instruction
apart: `tweenTo` was resolved by `GetMember` from the *caller's* own dictionary before the pool was
replaced, and `tween__start` by the pool afterwards. It is also, from the other side, agent DM's
hand-over 1 (`agentDM.md` §8.1): opening the global movie after the menu cost the menu its tweens,
12,580 errors against 71, because the open order decides which movie writes the pool last.

Four things this rules in that were ruled out before, correctly, and are not the cause: the two movies
really do **not** share a context; the class really **is** installed in the failing movie; the
assignment order really does **not** match the failure set; and `ASSetPropFlags` really removes
nothing. All four are about the prototype, and the prototype was never the problem — **the name was**.

One more fact worth recording because it decides how loud this defect is: every movie in this cook
reports `version 10`, so `GASStringContext::IsCaseInsensitive()` is FALSE and
`GASObject::FindNode`'s by-text fallback (retail's `GASStringNode::ResolveLowercase_Impl`,
2013 `0x9c0d50`) never runs. Below SWF 7 it would have masked this defect entirely.

## 3. What retail does, and the fix

Retail does not have the problem because it does not have the object. It splits the two halves:

| retail | what it is |
|---|---|
| `GASActionBufferData` (`CreateNew` **0x9d79a0**, `Read` **0x9d5d30** / **0x9d7490**) | the bytecode. Read once per definition, refcounted, owned by the tag in the shared `GFxMovieDataDef` |
| `GASActionBuffer` (**0x9d8140**, `GASActionBuffer(GASStringContext*, GASActionBufferData*)`) | a wrapper **per string context**, holding the interned constant pool |

`GASDoAction::Execute` (**0x9da9f0**) and `GASDoInitAction::Execute` (**0x9daae0**) build a **new**
`GASActionBuffer` for every execution, out of `GetMovieRoot() + 120` — the executing movie root's own
`GASStringContext` — queue it with `GFxSprite::AddActionBuffer` (**0x9eaf60**) and release it. And
`GASActionBuffer::ProcessDeclDict` (**0x9dad40**) fills that pool **exactly once**, guarded by
`this+28`, which is −1 until the first `ActionConstantPool` and is then the pc it was filled at:

```
v8 = *((_DWORD *)this + 7);          // this+28
if ( v8 != a3 ) { if ( v8 == -1 ) { *((_DWORD *)this + 7) = a3; ...build the dict... } }
```

The same contract in this tree's shape, without splitting the class: **`GASActionBuffer` keeps one
constant pool per `GASStringManager`**, and each pool carries retail's once-only guard. The pool is
selected by the manager's **serial number**, not its address, because a manager freed with a movie can
be allocated again at the same address and hand a later movie the wrong pool. `GetConstant` takes the
string context, which every caller already has: it is `sc` in `Execute`, two call sites.

`GASGlobalContext::GASGlobalContext(GFxMovieRoot*, GASStringManager*)` (**0x9e7290**) is worth naming
as the other half of the same picture: retail's context is **handed** a string manager rather than
owning one, and `GFxMovieRoot::GFxMovieRoot(MemoryContextImpl*)` (**0xa064c0**) passes it from the
memory context. This tree gives each context its own manager, which is legitimate and is not changed
here — once the pool is per manager, either arrangement is correct.

## 4. What the user sees now, and why one defect explained all three symptoms

The modal is the global movie's `_level0.msgBox_mc.messageBox_mc`, whose subtree is
`_vignette_mc`, `_bkgdWhite_mc`, `_bkgdBlack_mc`, `_msgBox_mc` (`boxBkgd_mc`, `boxShape_mc`, `_txt_mc`,
`btnBar_mc`) and `_particles_mc`. Every one of those clips issues one `tweenTo`, and every one failed
on `this.tween__start`, so `onEnterFrame` was never installed and **not one of them ever left its
authored state**. The authored state of the backdrop is opaque and the authored state of the bar, the
text and the buttons is alpha 0 — which is the user's screenshot exactly: an opaque black plane over
the scene and over the menu (the global movie is `Priority` 255, so it draws over everything), with
nothing on top of it.

So the dim layer's own alpha, which the brief named as the obvious second suspect, is **not** a second
defect: it is the same tween seen from the other end, and it was measured that way rather than argued.
The dim in `modal_quit.png` is the tween arriving at its authored target.

## 5. The menu bar

Agent DM reported the bar taking ~30 s of wall clock to fade in, for the same reason. **That does not
reproduce on this HEAD.** Measured with a ladder of screenshots on one real key press, the same driver
and the same schedule for both binaries, on an idle machine, scoring the luminance of the strip the bar
occupies (`build/agentEG/barfade.py`; the 99th percentile saturates at 234 when the bar is opaque and
sits at 228 while the start screen is still up):

| | start screen dismissed | +0.2..0.4 s | +0.7..0.9 s |
|---|---|---|---|
| HEAD (`DishonoredGame-Win64-Shipping.exe`, the executable the user played) | 6.42 s | p99 **104** | p99 **234** |
| this package, the clean gate build | 6.34 s | p99 **197** | p99 **234** |

So the bar is fully in **within 0.9 s** of the start screen being dismissed, and was already within
0.8 s at HEAD. Both are a normal fade and neither is 30 s. DM's figure was real when it was measured
and something between then and now fixed it — most plausibly agent DG's opcode-budget work, since a
frame that runs out of budget stops advancing the tween rather than never starting it. The honest
statement is that the bar is prompt, that this package did not change it, and that the ~30 s figure
should not be carried forward. Ladders: `build/agentEG/fade2_after_log.txt` and `fade2_before_log.txt`.

## 6. The census

One keyboard-driven run of the user's own QUIT GAME path each time — key, three Rights, Enter — same
driver, same schedule, same resolution. **Before** is `build/agentEG/base1_log.txt`, the untouched
HEAD playable executable. **After** is `build/agentEG/run10_log.txt`. Both report **0 `Critical:`
lines**.

| script errors on the QUIT GAME path | before | after |
|---|---:|---:|
| `call of a value that is not a function: 'tween__start'` | 547 | **0** |
| `call of a value that is not a function: 'tweenEnd'` | 4 | **0** |
| `'updateDebug'` | 3 | **0** |
| `'SetIcon'` | 2 | **0** |
| `'AddControllerButtonInstance'` | 2 | **0** |
| every other unresolved call | 0 | **0** |
| `GotoLabeledFrame: no frame named 'PC'` (benign; retail logs it too) | 14 | 14 |
| **total** | **572** | **14** |

The three names that are not tweens went with them, and that is the same defect: they are
`ExternalInterface` calls whose *name* came out of the poisoned pool.

On agent EA's New Game path (`build/agentEG/run9_log.txt`), the same run that produces
`modal_brightness.png`: **21** script errors, of which 18 are `GotoLabeledFrame` and three are
pre-existing residuals of EA's hand-over 2, all on receivers this package does not touch —
`toUpperCase` on **undefined** and `tweenTo` on **undefined** (both `_widgetSettings.Setting_Name`,
the settings tree that is never pushed into the movie) and `UpdateSettings` on
`_level0.newGame_mc._menu_mc` (the interface asking the game for something it does not answer).

## 7. Verification

* **Every build and every measurement is made in the snapshot worktree `build/agentEG_wt`** (HEAD
  `2650390` plus this package's 10 files and nothing else — `git -C build/agentEG_wt diff --stat` is
  those 10 files). The shared working tree carries agent EF's save-layer package, which this package
  neither touches nor builds with.
* **Clean full release build** of that worktree, build directory deleted first, every target and
  `DISHONORED_LAYOUT_CHECKS=ON`: **994 edges, 0 errors** — `DishonoredGame`, `CoreSmoke`,
  `EdgeAnimSmoke`, `LayoutProbe` (`build/agentEG/buildclean.log`). **0 `C4263` and 0 `C4264`.**
* **Regression** on that clean gate build: `python resources\tools\run_regression.py --build-dir
  build/agentEG_wtrel --no-build --exe-name DishonoredGame_EGW.exe --log-prefix EGW` ->
  **`31 ok, 0 failed, 0 skipped, 423s`** (`build/agentEG_wtrel/regression/summary.txt`,
  `build/agentEG/regression2.log`).
  **The first pass of the same command on the same binary reported `29 ok, 2 failed`, and I record it
  rather than only the pass.** The two were `d3d9/unported_natives 1` —
  `ADishonoredPlayerPawn::execPlayDying_Native`, the pawn died on the plain walk — and
  `inputtest/inputtest_peak_speed 0.0` on a run whose `inputtest_moved` was **4781.1** against HEAD's
  ~1030, with `peak 2D accel 3303.0`: a pawn that fell a long way rather than one that walked. Neither
  can be this package, and not by argument: **both stages open 0 movies** (`grep -c "GFx UI census"` is
  0 in `EGW_d3d9.log` and `EGW_inputtest.log`), so no `GASActionBuffer` is executed in either and every
  line this package changes is unreachable there. They are the two metrics `PHASE11.md` already lists
  as owing a load-aware threshold.
* **The acceptance runs are on that same clean gate build**: `build/agentEG/gate1.txt` -> the quit
  modal (`egg1_t01800001.png`, copied to `modal_quit.png`), 14 script errors, **0 `Critical:`**; and
  `gate2.txt` -> the New Game confirmation (`egg2_t02800000.png`, copied to `modal_brightness.png`),
  21 script errors, **0 `Critical:`**. The before/after pair and the census come from `base1.txt`
  (the untouched HEAD executable) against `run10.txt`, driven identically.
* **No generated file was touched.** Nothing this package changed is produced by
  `gen_classes_header.py`, so no regeneration is needed.
* **Addresses.** Every address in this package and in `agentEG_status.csv` was resolved against
  `build/agentEG_ida/retail2013_agentEG.i64`, my own copy, headless, through
  `resources/tools/ida/run.py`. `rva_sweep.py --path .../External/GFx3` is **0 MISLABELLED** after
  §9's two fixes (589 citations). `--path .../GFxUI` reports two `MISLABELLED-2012` on
  `gfxuirenderer.cpp:1669`; that line labels its addresses `2012 0x...` in words and the tool's
  "labelled" bucket only covers the first token on the line, so those two are the tool, not the file.
* **The driver is real Windows input**: `SendInput`, the schedule keyed to a log line rather than to
  the clock, `AttachThreadInput` around `SetForegroundWindow`. It is agent EA's driver with the log
  name changed, and EA's `SendInput`-not-`SetCursorPos` finding is why.

## 8. The switch this package adds

One, read on first use, at the head of `DishonoredGFxAutoOpen` rather than in the draw path — because
a movie's initialisation actions run **before** the first drawn frame, and read in the draw path the
switch armed itself after the thing it was meant to watch. That cost one build.

* **`-gfxuitweendiag[=<n>]`** — three reports, all off unless asked for:
  * `NOTAFN`: the first n unresolved method calls in full — the failing name with its interned node,
    its hash and its string manager; the movie the receiver belongs to, its SWF version and whether
    that version makes lookups case-insensitive; and then the receiver's whole resolution chain
    (`pASObject` and its `__proto__` walk, `pProto` and its walk, and the movie's own
    `Prototypes[Proto_MovieClip]`), each object with whether it holds the name **by identity** and
    **by text**. Identity-no / text-yes is this package's defect in one line.
  * `MEMBERSET`: every member write whose name matches `GFxAS2WatchMatches`, with the object written
    to and the string manager that interned the name. `-gfxuitweendiag` seeds that pattern with
    `tween*` when no `-gfxuiwatch` was given, so it is not tween-shaped in the code.
  * `GCBIRTH`: one line per `GASGlobalContext`, which is what maps a manager to a movie.

## 9. Deviations and other work, stated once

1. **The pool is kept per string manager on one `GASActionBuffer`, not in a per-context
   `GASActionBuffer` over a shared `GASActionBufferData`.** Retail's split is the cleaner shape and it
   is what a later package should do if `GASActionBuffer` is ever touched again; it reaches
   `GASDoActionTag`, `GASDoInitActionTag`, `GFxSwfEvent`, `GFxButtonAction`, `GASFunctionObject`'s
   `pBuffer` and the action queue, which is a refactor rather than a fix. The observable contract is
   the same: one pool per executing context, filled once, never shared. It costs one pointer and two
   words per buffer per movie that runs it, against one allocation per execution in retail.
2. **A function body executed under a context that never ran that buffer's `ActionConstantPool` reads
   an empty pool** rather than building one on demand. Retail is the same — its buffer is bound to one
   context for its whole life — and nothing in this cook can reach it, because an AS2 object graph does
   not cross movie roots.
3. **`GASStringManager` gains a serial number.** Retail has no such field. Keying a cached pool by the
   manager's *address* is wrong across a movie close, and a wrong pool is silent; the serial is
   monotonic for the process and four bytes.
4. **Two mislabelled citations fixed**, as the brief asked, both pre-existing and in other agents'
   files: `GFxTextDocView.h` cited 2012 `0xa9f2c0` for `GetViewRect` — retail's is **`0xa95270`**, and
   2012's `0xa9f2c0` lands inside `GFxTextIMEStyle::Unite` in retail; `GFxTextField.cpp` cited 2012
   `0xa27200` for the `DefineEditText` record's constructor — retail's is **`0xa1d900`**
   (`GFxEditTextCharacterDef::GFxEditTextCharacterDef`), and `0xa27200` is not a function start in
   retail at all.
5. **The menu bar's entries still do not answer a mouse click**, so the quit modal is reached here by
   the keyboard — three Rights and Enter, real `SendInput` presses into the game's own window, which is
   how the bar is navigated. Measured, with a real mouse: the pointer reaches the entry and the game
   draws its own cursor on it (`build/agentEG/eg6_t01600004.png` — the cursor is sitting on QUIT GAME),
   and the selection does not move and the click does nothing. That is agent DM's hand-over 2,
   `GFx_GenerateMouseButtonEvents` (2012 `0xa66a90`) and `GFxButtonCharacter`'s state machine, which is
   not this package. Stating it rather than letting a screenshot imply the mouse works.

## 10. Hand-overs

1. **`GFx_GenerateMouseButtonEvents` and the menu bar's hit testing** (deviation 5). Agent EA made the
   New Game screen's entries hover and click; the main menu bar's do not. A user who reaches for the
   mouse on the first screen of the game finds it dead, which is worth a package on its own, and it is
   agent DM's hand-over 2 unchanged.
2. **Retail blurs the scene behind a modal and this tree does not.** Compare `real_menu.png` with
   `build/agentEG/modal_quit.png`: the dim, the vignette, the bar, the text and the buttons all match,
   and retail's background is also out of focus. Nothing in the box's own subtree does that, so it is
   the game's post-process, not the interface — the same family as package EE's remaining filter
   passes. Not measured here; named so it is not mistaken for a tween.
3. **The `GASActionBufferData` / `GASActionBuffer` split** (deviation 1), if `GASActionBuffer` is ever
   opened again. The addresses are in `agentEG_status.csv`.
4. **`UDisGlobalUIManager`, still**, and now with the box visible the missing half is louder: the box
   has no way to answer. `_common.MessageBoxInvoke::OnMessageBoxClosed` needs the id, the timer and the
   `FArkGameEvent` that carries the player's answer back, which is agent EA's hand-over 4. YES on the
   quit modal does not quit and YES on the New Game confirmation does not start a mission.
5. **The three residual unresolved calls on the New Game path** are all agent EA's hand-over 2, the
   options settings tree, unchanged by this package (§6).

## 11. Files

Mine (10, none new):

`External/GFx3/{GFxAS2.h, GFxAS2Interp.cpp, GFxAS2Lib.cpp, GFxAS2Object.cpp, GFxAS2Object.h,
GFxAS2Runtime.h, GFxAS2Value.cpp}` — the defect and the diagnostic;
`External/GFx3/{GFxTextDocView.h, GFxTextField.cpp}` — the two citation fixes (deviation 4);
`GFxUI/Src/gfxuiengine.cpp` — the switch.

Scratch, not repo tools: `build/agentEG/` — `egpatch.py` (the CRLF-safe patch helper),
`patch01_diag.py` … `patch05_tidy.py` (one per change, each with its measurement in its docstring),
`drive.py` (agent EA's real-input driver, log name changed), `go.py` (run and drive in one command),
`barfade.py`, `bmp2png.py`, `lookup.py`, `xrefs.py`, `disasm.py`, `dec.py`, `name_at.py`, the run logs
`run1`..`run10`, `base1`, `fade_before`, `fade_after` and the screenshots;
`build/agentEG_build.cmd`, `build/agentEG_run.py`, `build/agentEG_sync.py`; snapshot worktree
`build/agentEG_wt`; build directories `build/agentEG_wtrel` (the clean gate build, no play defaults)
and `build/agentEG_playrel` (the same sources with `DISHONORED_PLAY_DEFAULTS=ON`, which is what the
screenshots were driven on).

IDA: **own copy only**, `build/agentEG_ida/retail2013_agentEG.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.
