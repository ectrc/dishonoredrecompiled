# Agent FC (PHASE13 FC) — one line of `attachMovie`, and what it actually moves

Branched from HEAD `e02f5f8`, worked only in `build/agentFC_wt` (detached at that commit).
Build dirs: `build/agentFC_wt/build/agentFC_rel` (this package),
`build/agentFC_headsrc/build/agentFC_headrel` (the untouched `e02f5f8` baseline, built from a
`git archive` export so the worktree could be edited while it built),
`build/agentFC_wt/build/agentFC_reg` (the regression harness),
`build/agentFC_wt/build/agentFC_clean` (the gate build).
IDA: own copy `build/agentFC_ida/shipping2012_agentFC.i64`, headless only, **no IDA MCP tool and no
FModel tool**. No commits, nothing staged, nothing written into the main checkout outside `build/`.

**Three files, all in `External/GFx3`. Two functions touched, named in §8.**

## The answer in eight lines

* **Three of the four placement faults are fixed, and the fourth is not this fault at all.** Options
  offset: fixed. The missing ACCEPT/BACK and RESTORE SETTINGS/BACK bar: fixed. The gamma marks'
  position: fixed. The difficulty portrait: **unchanged, to the decimal** — §5.
* **The fix is the one line agent EY handed over, but its justification in the brief was wrong and
  the right one is stronger.** The brief says "Flash and GFx both apply the init object first". What
  retail actually does is queue **four** action-queue entries and drain them in a fixed order, and
  the constructor is the **last** of the four. That is read out of the 2012 binary, not inferred:
  §2. The consequence is the same, and now it is measured rather than asserted.
* **And confirmed in the running retail game.** Agent FB's dismod capture prints
  `_root.help._props _x=1184 _y=651` in retail — the constructor's snapshot, the exact quantity this
  fault corrupts, with the right value in it. Two independent sources: retail's code and retail's
  live state. §4.
* `_root.optionsMenu_mc._options_mc`: `_x = 0`, `_defPosX = 0` **→ `_x = -450`, `_defPosX = -450`**,
  which is what the asset's init object asks for. The value column is back on screen and the options
  screen matches retail's layout: `build/agentFC/sbs_options.png`, `ba_options.png`.
* `_root.help` on the brightness screen: `_x=0 _y=0` **→ `_x=1184 _y=651`**, and the bar is drawn.
  `ACCEPT / BACK` is now on the brightness and New Game screens and `RESTORE SETTINGS / BACK` on
  Options, in retail's own corner: `sbs_brightness.png`, `sbs_newgame.png`.
* The five gamma marks moved from **0.487 → 0.766** of the window height, measured off the captures;
  `GammaMc`'s own `_y = 555` of a 720 stage predicts 0.771 and the init object's 360 predicts 0.500.
  They sit in retail's band in `sbs_brightness.png`, and they are still untinted — that is agent FD's
  `flash.geom.ColorTransform`, not this package.
* **The difficulty portrait is a timeline fault, not an `attachMovie` fault and not a 3D one either.**
  `_corvo_mc` is a timeline child of `_root.newGame_mc`, never an `attachMovie`, and before and after
  the fix its numbers are identical to the last digit. It is right on EASY and thrown down-right on
  VERY HARD while **nothing else on the screen moves at all** — so what is wrong is the one thing the
  difficulty changes: `portrait_mc.gotoAndStop(idx+1)`, one frame of a four-frame clip. §5, with the
  numbers, two eliminated candidates and the hand-over.
* **The depth question is independent of this fix, and retail's whole depth convention is now
  measured**: `attachMovie` and `duplicateMovieClip` place at AS depth **+ 0x4000**, `getDepth`
  answers internal **− 0x4000**, `getNextHighestDepth` answers `max(0, largest − 0x3FFF)`, and
  `removeMovieClip` tests `depth >= 0x4000`. Our runtime applies no offset anywhere except
  `GASop_DuplicateClip`, which **subtracts**. §6 — a hand-over, not a change.
* Regression **37 ok, 0 failed, 0 skipped**, built inside the harness; a clean full Release build with
  the build directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**;
  `rva_sweep` over the whole worktree: 3 suspects, **none in a file this package touches**, all three
  already at HEAD. §7.

## 1. What was measured, and with what

Every capture is an `-apshottime` screenshot, taken inside the process on the game thread, driven by
`resources/tools/drive_input.py` with `--exe DishonoredGame_FC.exe` (`_FCH.exe` for the untouched HEAD
baseline), real `SendInput`, from the start screen, windowed at 1600x900.
`build/agentFC/go.py` launches the run and the driver together; `build/agentFC_run.py` stages and runs.
The AS2 side is read with agent EY's `-disclipdiag=<path>[;<path>…]` and `DisReportHelpBar`, which are
already in the tree and which this package did not modify.

**One thing had to be built before anything could be believed: a screenshot claim that survives four
agents sharing one directory.** The engine names every capture `apshottime<N>.bmp` into the retail
install's shared screenshot folder, and the other agents' runners **delete every `apshottime*` at
launch**, so the index is reused and a file that is "new since the run started" can be somebody else's
picture under a name that was already there. Two of this package's first runs claimed a bitmap that was
not theirs — one was another agent's main menu, one was an in-game street — and both were caught only
by looking at them. `build/agentFC_run.py` now watches the directory every 0.15 s keyed on
`(name, size, mtime)`, copies each bitmap the moment it appears, watches **this run's own log** for the
`-apshottime: screenshot of the frame at world time` line, and claims the first bitmap **named
`apshottime<digits>.bmp`** that arrives at or after that line. Every run's report prints the name, the
byte size and the arrival offset of every candidate it saw, claimed or not.

Reference: the user's own side-by-side captures `resources/reference/menu/{1,2,4}.png`, retail on the
left half and this build on the right, both windowed at the same size (retail's half is 1602x936
including the window frame, ours is a 1600x900 client area). `build/agentFC/retail_*.png` are their
left halves, cut out so the comparison is against retail rather than against a description of it.
For the footer bar and the depth convention there is something better still: **agent FB's dismod
capture of the running retail game**, `build/agentFB/retail/displaylist.log`, which is used in §4 and
§6 in preference to anything inferred here.

## 2. What retail does, read out of the 2012 binary

The AS2 `MovieClip.attachMovie` handler is **2012 `0x9ff990`** — found from the AS2 MovieClip method
table, whose `"attachMovie"` name pointer sits next to it, not from any CSV. Resolved by hand with
`ida_funcs.get_func`: `0x009ff990..0x009ffca7`. It:

* reads argument 3 with `GASValue::ToObjectInterface` when `GetNumArgs() == 4`, and
* calls `GFxSprite::AddDisplayObject` (**2012 `0x9fee10`**, resolved by hand to
  `0x009fee10..0x009ff98d`, signature
  `(const GFxCharPosInfo&, const GASString&, const GArrayLH<GFxSwfEvent*>*, const GASObjectInterface*, unsigned, unsigned long, GFxCharacterCreateInfo*, GFxASCharacter*)`)
  passing that object as the **fourth** parameter.

So retail does not copy the init object at the call site at all: it hands it to the placement, and the
placement decides when it lands. In the arm that finds a registered class
(`GASGlobalContext::FindRegisteredClass` at **2012 `0x9ff3ce`**, inside that function), it makes four
action-queue entries:

| order | priority | entry | what it does |
|---|---|---|---|
| 1 | 1 | type 4, handler **2012 `0x9fb170`** | `GFxASCharacter::SetProtoToPrototypeOf` — `__proto__` only |
| 2 | 3 | type 2, event id `0x40000` | the `onConstruct` event |
| 3 | 3 | type 4, handler **2012 `0x9f61b0`** | the init object's members — its visitor's vftable is `GFx_InitObjectMembers::InitVisitor` |
| 4 | 3 | type **3**, the class function itself | **the constructor** |

and the drain order is not a guess either:
`GFxMovieRoot::ActionQueueIterator::getNext` (**2012 `0xa0cbe0`**) starts at priority index 0 and walks
**upward** to 5, and `ActionQueueType::InsertEntry` (**2012 `0xa09440`**) appends after a per-priority
insert cursor, so entries at one priority run **FIFO**. Entries 2, 3 and 4 are all at priority 3 and
were inserted in that order.

**The constructor is the last of the four, after the init object's members.** Which is why
`_common.HelpBar`'s constructor — `this._props = {_x:this._x, _y:this._y, _alpha:100, _xrotation:…,
_yrotation:…}`, `lib.as2.txt:1465-1525` — reads 1184 in retail and read 0 here.

**Two things in the brief this corrects.** First, its reason ("Flash and GFx both apply the init object
first") is not what the binary shows; what the binary shows is a queue whose last entry is the
constructor, which is a stronger claim and a different one. Second, `sub_9FB170` at priority 1 is *not*
the constructor — it only sets `__proto__` — and event `0x40000` is `onConstruct`, not the class
constructor either. Reading either of those as the constructor gives exactly the opposite answer, and
this package believed it for about twenty minutes.

`GFx_FindClassAndInitializeClassInstance` (2012 `0x9fcdf0`), the other arm of the same function and the
one `GFxButtonCharacter::RecreateCharacters` uses, executes the same entries **inline** in the same
order: `__proto__`, `onConstruct`, then the class function.

## 3. The fix

`GFxSprite::AttachMovie` takes the init object as a defaulted fourth parameter, the way retail's
`AddDisplayObject` does, and applies it after `ExecuteFrame0Events()` and **before**
`BindRegisteredClass()`. `MCAttachMovie` reads argument 3 and passes it down instead of copying it
after the call.

The choice of *where* between frame 0 and the constructor is deliberate and conservative: retail's
queue proves only **init before constructor** (its frame-0 tags are not in that queue at all), so this
moves exactly the one ordering the evidence names and leaves agent DG's frame-0-before-constructor
ordering — which was itself an empirical finding, `GFxPlayerSprite.cpp:1798-1804` — untouched. An
earlier build of this package put the copy before `ExecuteFrame0Events()` as well; it was discarded
before measurement because it changes two orderings at once and the binary only justifies one.

## 4. The three it fixes

All numbers from `-disclipdiag` / `DisReportHelpBar`, HEAD `e02f5f8` against HEAD + this package,
same switches, same window size, reached the same way.

### The options list

```
HEAD  _root.optionsMenu_mc._options_mc  live _x=0     _y=-80  _xscale=99.7564 _rotation=-4   _defPosX=0
FC    _root.optionsMenu_mc._options_mc  live _x=-450  _y=-80  _xscale=99.7564 _rotation=-4   _defPosX=-450
asset attachMovie("o_OptionsList_withTabs", "_options_mc", d, {_x:-450, _y:-80, _rotation:…})   OptionsMenu.as2.txt:22476-22484
      _common.ItemsList constructor: this._defPosX = this._x                                    OptionsMenu.as2.txt:6297
```

`_y` was already right at HEAD and `_x` was not, which is the whole shape of the fault: the init
object *was* being applied, after the constructor, and then the list's own open path put `_x` back to
its stale `_defPosX`. Screens: `build/agentFC/ba_options.png` (HEAD | FC),
`sbs_options.png` (retail | FC).

### The footer prompt bar

```
HEAD  DisReportHelpBar(brightness screen): _root.help _x=0     _y=0    _alpha=100 _width=213.2 _height=55
FC    DisReportHelpBar(brightness screen): _root.help _x=1184  _y=651  _alpha=100 _width=213.2 _height=55
asset _root.help = _root.attachMovie("lib_dynamicHelpBar", "help", _root.getNextHighestDepth(),
                                     {_x:1184, _y:651})                                        MainMenu.as2.txt:1841-1922
```

**And this one is confirmed against the running retail game, not against the asset.** Agent FB's dismod
capture, `build/agentFB/retail/displaylist.log`, reads out of retail at the main menu:

```
_root.help         _x=1184  _y=651  _alpha=0  _visible=true  depth=0
_root.help._props  _x=1184  _y=651  _alpha=100
```

`_props` — the constructor's snapshot, the exact quantity this package repairs — is **1184 / 651 in
retail**. At HEAD ours is effectively `{_x:0,_y:0}` (the bar tweens to (0, 0)); with the fix the bar
lands on (1184, 651). That is the whole claim, measured at both ends.

Before the bar opens it is at (1184, 651) with `_alpha = 0` on both builds; what the fault changed was
where `OpenHelp` **tweened it to**, because it tweens to `_props`, and `_props` is the constructor's
snapshot. `ACCEPT / BACK` is now drawn on the brightness and New Game screens and
`RESTORE SETTINGS / BACK` on Options, in retail's corner: `sbs_options.png`, `sbs_brightness.png`,
`sbs_newgame.png`.

### The gamma marks

The brightness screen is a **second movie** (`UI_Gamma.GammaImage`), so `-disclipdiag`, which reads the
main-menu view, cannot address `_root.gamma_mc` — it reports nothing for that path on either build, and
this package did not add an instrument for it rather than touch a file outside its package. The
measurement is the picture, against the asset's own numbers:

```
asset GammaImageBase ctor: _root.attachMovie("GammaMc", "gamma_mc", d, {_x:640, _y:360})   GammaImage.as2.txt:181-237
      GammaMc ctor:        this._y = 555                                                   GammaImage.as2.txt:4686
HEAD  the five marks' row spans rows 380..497 of 900, centre 0.487 of the window   (360/720 = 0.500)
FC    the same row spans 638..740, centre 0.766                                     (555/720 = 0.771)
```

measured off the captures themselves by the bright-pixel profile of the column through the first mark
(`build/agentFC/headGam2.png`, `relGam2.png`, both 1600x900 client area, the stage letterboxed 1:1 at
16:9 so a stage fraction is a window fraction). The predicted 0.500 → 0.771 and the measured
0.487 → 0.766 agree to within the glyph's own offset from its clip origin. **Retail's own figure is not
quoted as a number**: in `reference/menu/1.png` the marks are graded dark, so a brightness profile finds
the slider above them rather than the marks, and the comparison there is the picture —
`build/agentFC/sbs_brightness.png`, where our five marks and retail's sit in the same band under the
slider and the `ACCEPT / BACK` bars line up.

`build/agentFC/ba_brightness.png` and `sbs_brightness.png`. At HEAD the marks cut straight through the
`BRIGHTNESS` header; they are now in a row under the slider where retail's are. They are still all the
same brightness — that is `flash.geom.ColorTransform`, agent FD's half of this screen.

## 5. The one it does not fix: the difficulty portrait

**Measured, before and after, identical to the last digit:**

```
HEAD  _root.newGame_mc._corvo_mc              live _x=-640.65 _y=-150.95 _z=0 _xscale=100 _yscale=100 _width=774.7 _height=637.6
FC    _root.newGame_mc._corvo_mc              live _x=-640.65 _y=-150.95 _z=0 _xscale=100 _yscale=100 _width=774.7 _height=637.6
HEAD  _root.newGame_mc._corvo_mc.portrait_mc  live _x=160 _y=246 _xscale=100 _yscale=100 _width=320 _height=492
FC    _root.newGame_mc._corvo_mc.portrait_mc  live _x=160 _y=246 _xscale=100 _yscale=100 _width=320 _height=492
```

and it could not have been otherwise: **`_corvo_mc` is a timeline child of `_root.newGame_mc`, and
`newGame_mc` is a timeline child of the root.** Neither is ever an `attachMovie` — the only
appearances of `"newGame_mc"` in `MainMenu.as2.txt` are the constant pool entry and one `getMember`
(`MainMenu.as2.txt:23547`, `24768`), and every `_corvo_mc` is a `getMember`. An init object is never
applied to either, so the ordering cannot reach them. **The brief's fourth candidate is not this
fault**, and that is the premise this package corrects.

What it is instead, measured on the fixed build:

* On **EASY** the portrait is where retail's is, upper left, at its authored size
  (`build/agentFC/relNG3_view.png`).
* On **VERY HARD**, reached by clicking the row, the portrait is about **+140 px right and +255 px
  down** of retail's at 1600x900 and visibly smaller, a third of it off the bottom of the window
  (`relNG4_view.png`, measured off `sbs_newgame.png` between the two mask centres) — the user's own
  capture, `reference/menu/2.png` right half, exactly.
* **And on the same screen at the same moment, everything that is not the portrait *artwork* is in
  exactly the same place on both difficulties.** `build/agentFC/ng_frames.png` is our own EASY beside
  our own VERY HARD, cropped to the same rectangle: the black brush-stroke shapes that are the rest of
  `_corvo_mc`, the four difficulty rows and the `NEW GAME` title are where they were, and only the
  picture of Corvo has moved. Against retail (`sbs_newgame.png`, both on VERY HARD) the rows, the
  description and the title match too. **The portrait is the only thing on that screen that is wrong.**
* **So it is not the screen's 3D tween either**, and that is worth saying because it is the obvious
  suspect and this package believed it for a while. `onSelectionUpdated(idx)`
  (`MainMenu.as2.txt:2934-3141`) does tween a clip to
  `_rotation:0, _xrotation:-5-1.25*idx, _yrotation:-5-2*idx, _z:-10-10.25*idx`, but the clip it tweens
  is `this` — the difficulty list, the screen's `_menu_mc`, which agent FB's retail capture shows as
  `_root.newGame_mc._menu_mc` at `_x=-426.5 _y=-25.95 _rotation=-4.00034` — and `_corvo_mc` is its
  **sibling**, not its child. A transform on `_menu_mc` cannot move `_corvo_mc`, and the picture
  confirms it does not: the rest of `_corvo_mc` never moves.
* **What does change between the two is one call.** `SetPortrait(idx+1)` is
  `this._corvo_mc.portrait_mc.gotoAndStop(idx+1)` (`MainMenu.as2.txt:1711-1762`) — one frame of
  `portrait_mc` per difficulty. Frame 1's artwork lands where retail's does; frames 2, 3 and 4 land
  about 140 right and 255 down of it and smaller, while `portrait_mc` itself keeps reporting
  `_x=160 _y=246 _width=320 _height=492` on every frame, so nothing about the clip changed — only what
  its timeline drew. **The fault is a timeline one: what the display list does with a `gotoAndStop`
  onto a later frame of a multi-frame clip** (`GFxSprite::GotoFrame`,
  `IncrementFrameAndCheckForLoop`, and `AddDisplayObject`'s "same character at this depth" early-out).

**One candidate eliminated on the way, so the next package does not spend a day on it.** The 3D
projection's focal length looks wrong at a glance: `GRenderer::MakeViewAndPersp3D` builds it from the
stage rectangle's **width** against a *vertical* field of view, which reads like a width/height mix-up
that would scale every `_z` displacement by the aspect ratio. It is not a mix-up. Retail's own body
(2012 `0x9b7790`, named in the database and resolved by hand) computes
`|right-left| * 0.5 / tan(fov/2)` and passes `|width|` then `|height|` to
`GMatrix3D::PerspectiveFocalLengthRH` — the same as ours, term for term. Agent FA's port is faithful.

**Hand-over, to whoever owns the sprite timeline.** It is one click to reproduce — New Game, then
click `VERY HARD` — and `portrait_mc` is a four-frame clip whose four frames are the whole test case,
with everything else on the screen held fixed as a control. Two loose ends found beside it, both
cheap to check first: `portrait_mc._width` / `_height` report `320 x 492` on every frame although the
frames plainly draw different-sized artwork, so the clip's bounds are not being recomputed on a frame
change; and `_z` still reads back as **0** on `_root.newGame_mc` and on `_corvo_mc` on both builds
although the asset tweens both, so `_z` has an effect but is not a readable display property and every
`tweenTo` on it starts from 0.

## 6. The depth convention — independent of this fix, and now measured

The hand-over from agents EY/EZ asked whether this fix makes the `+16384` depth question reachable.
**It does not: it is independent.** The three clips in §4 are all attached at
`getNextHighestDepth()`, which is above every timeline depth on either convention, so none of them is
affected by the offset. Nothing here needed it and nothing here changes it.

What is new is that retail's convention is no longer inferred. From the 2012 binary, every address
resolved by hand:

| retail | what it does | ours |
|---|---|---|
| `attachMovie` 2012 `0x9ff990` | places at AS depth **+ 0x4000**, and refuses `depth > 0x7EFFFFFD` with *"depth (%d) must be >= 0"* | no offset |
| `duplicateMovieClip` 2012 `0x9f5210` | `CloneDisplayObject` at AS depth **+ 0x4000** | `GASop_DuplicateClip` **subtracts** 16384 |
| `getDepth` 2012 `0x9cde60` (`GFxASCharacter::CharacterGetDepth`) | returns internal depth **− 0x4000** | returns it raw |
| `getNextHighestDepth` 2012 `0x9f5510` | returns `max(0, largestDepthInUse − 0x3FFF)` | returns `largest + 1` raw |
| `removeMovieClip` 2012 `0x9f6c00` | refuses unless `depth >= 0x4000`, warning *"%s.removeMovieClip() failed - depth must be >= 0"* | agent EZ's `bScriptCreated` flag |

**And the running retail game says the same thing.** Agent FB's `displaylist.log` prints
`getDepth()` for every clip at retail's main menu: `_root.mainMenu_mc` **-16383**, `_root.optionsMenu_mc`
**-16290**, `_root.mouseCursor_mc` **-16381**, `_root` itself **-16385** — every timeline clip is its
SWF depth minus 0x4000 — while `_root.help`, an `attachMovie` at `getNextHighestDepth()`, is **0**, and
`_root.vignette_mc`, an `attachMovie` at AS depth 1, is **1** and draws above all of them. Two
independent sources, the binary and the live game, agree.

So our runtime is self-consistent in having **no** offset anywhere — except `GASop_DuplicateClip`,
which subtracts one and is therefore wrong on its own terms as well as retail's. Agent EZ's
`bScriptCreated` stands in for retail's `>= 0x4000` test and is equivalent while nothing else creates
clips at script depth. Moving the tree to retail's convention is a five-function change that would
also make `swapDepths` and `getInstanceAtDepth` agree; it is not this package's and is left with the
addresses above so the next one does not have to find them again.

## 7. Gates

| gate | result |
|---|---|
| `run_regression.py`, worktree's own copy, absolute build dir, built inside the harness | **37 ok, 0 failed, 0 skipped** — `build/agentFC/regression.log` |
| clean full Release, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`, all three targets | **0 errors, 0 C4263, 0 C4264** — `build/agentFC/buildclean.log` |
| `rva_sweep.py`, whole worktree | 7,356 citations, **3 suspects, none in a file this package touches** (`disbehaviorpatrol.cpp:319`, `gfxuirenderer.cpp:1679` ×2 — all present at HEAD) |
| `rva_sweep.py`, `External/GFx3` alone | 630 citations, **0 suspects** |
| every address cited by this package resolved by hand with `ida_funcs.get_func` | `0x9ff990`, `0x9fee10`, `0x9ff3ce`, `0x9fb170`, `0x9f61b0`, `0xa0cbe0`, `0xa09440`, and §6's five — `build/agentFC/ida_*.txt` |

The five gitignored `resources/docs/types/` inputs were copied into the worktree before the harness ran.

## 8. Merging

* **Regeneration is NOT required and was NOT run.** No `UClass`, no script class, no native function
  signature changes; nothing in `DishonoredGame` or `GFxUI` at all.
* **`Sources.cmake` did not change.**
* **No file outside `External/GFx3`** except this report and its CSV.
* **Total: 3 source files, 5 with the report.**
* **Functions touched in `GFxAS2Lib.cpp`: `MCAttachMovie`, and nothing else.** Agent FD is installing
  `flash.geom.ColorTransform` / `Transform` in the same file; that work lands in the `flash.geom`
  registration, which this package does not go near, so the two diffs do not overlap.
* Functions touched elsewhere: `GFxSprite::AttachMovie` (declaration in `GFxPlayer.h`, definition in
  `GFxPlayerSprite.cpp`). Nothing else in either file.
* `build/agentFC_sync.py` is the authoritative copy list, worktree → main, flagging rather than copying
  anything changed in main since `e02f5f8`.
* **The fourth fault was deliberately not fixed here.** §5 places it in the sprite timeline, a
  different subsystem from this package's one line, and going further needs a tag-level dump of
  `portrait_mc`'s four frames that does not exist yet. Landing three fixes cleanly was worth more than
  putting a fourth, riskier change in the same diff, and the diagnosis is handed over with the
  reproduction and two eliminated candidates instead.

## 9. What is left on these screens, and whose it is

* The five gamma marks are in the right place and still **untinted** — agent FD.
* `OPTIONS`, `GENERAL`, `BRIGHTNESS`, `NEW GAME` still use the wrong font — agent FB.
* The difficulty portrait's per-difficulty displacement — §5, the sprite timeline's `gotoAndStop`.
* `GFxValue::ObjectInterface::AttachMovie` (`GFxPlayerRoot.cpp:1409`) still ignores its `initArgs`
  parameter. Nothing in the menu passes one — `gfxuiengine.cpp:2848`'s mouse cursor passes `NULL` —
  so it is a latent gap rather than a live defect, and wiring it needs a `GFxValue` → `GASValue`
  conversion this package had no caller to test.
* `setMask` is not in our AS2 MovieClip table (retail's is 2012 `0x9f5320`) and the options list
  raises *"call of a value that is not a function: 'setMask'"* twice per open. It does not block the
  options screen — the screen and its footer bar are correct with the fix — but it is a real missing
  builtin.
