# Agent FH (PHASE14 FH) — the difficulty portrait, and the tag that inherits what it replaces

Branched from `18f36a3` and **rebased onto `63c2539` (agent FF) mid-run**, when FF landed; the
worktree `build/agentFH_wt` is detached at `63c2539` and every number below was retaken on it. §9
says what FF changes, why none of it touches this fault, and how the two diffs sit beside each other.
Build dirs: `build/agentFH_wt/build/agentFH_rel` (this package),
`build/agentFH_headsrc/build/agentFH_headrel` (HEAD `63c2539` from a `git archive` export, with this
package's two **instruments** and **not** its fix — §4 says exactly why),
`build/agentFH_wt/build/agentFH_reg` (the regression harness),
`build/agentFH_wt/build/agentFH_clean` (the gate build).
IDA: own copies of `shipping2012_v1.i64` and `retail2013_named.i64`, opened headless through
`resources/tools/ida/run.py` with `build/agentFH/probe.py`, **no IDA MCP tool and no FModel tool**;
the copies are deleted and every decompile they produced is kept as text in `build/agentFH/ida_*.txt`,
so `probe.py` plus a fresh copy reproduces any of it. No commits, nothing staged, nothing written
into the main checkout outside `build/`.

**Three files. One of them is the fix; the other two are instruments.** §9 names every function.

## The answer in nine lines

* **The tag-level dump exists** (`build/agentFH/portrait_frames.txt`, the tool is
  `build/agentFH/tagdump.py`) and it answers the question in four lines: `portrait_mc` is sprite 199,
  four frames, one depth. Frame 1 places its artwork with flags `0x06` **and a matrix of
  (-160, -246) px**. Frames 2, 3 and 4 each carry flags `0x03` — `Move|HasCharacter` — and
  **no matrix, no colour transform, nothing but the new character id**.
* **A `Move|HasCharacter` tag is a *replace*, and in retail a replace inherits the transform of the
  character it replaces for every field the tag does not carry.** Ours did not inherit: it left the
  fresh character at identity. So EASY (frame 1, the only frame with a matrix) was right and the
  other three drew at the clip origin.
* **The displacement is therefore exactly +160 px right and +246 px down in stage units**, not
  "about 140 and 255". Measured three ways that agree: the asset's own matrix, the live display list
  on both builds, and the pixels — §3, §4. Measured twice over, on `18f36a3` and again after the
  rebase onto FF's `63c2539`, with identical results.
* **None of the three suspects handed over is the cause.** `GotoFrame` is reached and is correct,
  `IncrementFrameAndCheckForLoop` is not reached by a `gotoAndStop`, and `AddDisplayObject`'s
  same-character early-out is not reached at all because these tags never go through
  `AddDisplayObject`. §5 corrects the brief's own hypothesis too.
* **The fix is four lines in `GFxDisplayList::ReplaceDisplayObject`**, read out of retail 2013
  `0x9ccb50` and its 2012 twin `0x9d6350`, every address resolved by hand. §2.
* **Before / after, from the live display list, through the same driver**:
  HEAD frames 2/3/4 → `(0.00, 0.00)`; with the fix → `(-160.00, -246.00)`, the same to the last digit
  as frame 1, which was already right. §4.
* **Against retail**: the portrait's mask matches retail's position to **+1, +0 px**
  (`build/agentFH/sbs_hard.png`, ours-before / ours-after / retail in one column). At HEAD the same
  template sits **+127, +194 px** away, where the display list predicts +126.0, +193.7. The four
  difficulty rows and the `ACCEPT` / `BACK` bar match between before and after at **dx 0, dy 0**. §4.
* **Nothing else moves, and that is exhaustive rather than impressionistic.** Across all six cooked
  UI movies there are **37** replace tags; 35 carry no matrix; and of those, the ones whose inherited
  transform is **not identity** are 3 in the main menu — all three are `portrait_mc` — plus 12
  controller-icon clips in `lib.swf` and 2 in the pause menu, neither reachable today. Sprites 51 and
  125, the only other multi-state clips in the main menu, inherit **identity**, so the fix cannot move
  them. `build/agentFH/replace_scan.txt`, `inherit_scan.txt`. §6.
* On `63c2539`: regression **37 ok, 0 failed, 0 skipped**, built inside the harness; a clean full
  Release with the build directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`:
  **0 errors, 0 C4263, 0 C4264** over all three targets; `rva_sweep` over the whole worktree: 3
  suspects, **all three present at `63c2539` itself, none in a file this package touches**. §8.

## 1. The dump, which is the deliverable the last package stopped for

`build/agentFH/tagdump.py` is a display-list dumper built on agent EY's reader in
`build/agentEY/swf/` (same container walk, same `DefineSprite` recursion). It decodes
`PlaceObject` / `PlaceObject2` / `PlaceObject3` and `RemoveObject` / `RemoveObject2` per frame, with
the flag bits named, the character id resolved to its definition and bounds, and the matrix printed
in both twips and pixels. It takes `--name <instance>` (find every tag that names an instance, then
dump the sprite it placed), `--sprite`, `--root`, `--all`, `--exports`, `--recurse N`.

Published dumps, both regenerable in under a second:

| file | what |
|---|---|
| `build/agentFH/portrait_frames.txt` | `portrait_mc`, `_corvo_mc` and `m_nGame` — the four frames and their two parents |
| `build/agentFH/mainmenu_tagdump.txt` | every tag of every one of the main menu's 143 sprites, 3,215 lines |
| `build/agentFH/replace_scan.txt` | every replace tag in all six cooked UI movies |
| `build/agentFH/inherit_scan.txt` | what each of those replace tags now inherits |

The four frames:

```
sprite 199 (portrait_mc), 4 frames
  frame 1: DoAction(2)   PlaceObject2  0x06 HasCharacter|HasMatrix   depth=1 char=192
                         <DefineShape 320.0 x 492.0 px>   tx=-3200 ty=-4920 twips = (-160.00, -246.00) px
  frame 2: DoAction(2)   PlaceObject2  0x03 Move|HasCharacter        depth=1 char=194
                         <DefineShape 332.0 x 492.0 px>   no matrix
  frame 3: DoAction(2)   PlaceObject2  0x03 Move|HasCharacter        depth=1 char=196
                         <DefineShape 332.0 x 492.0 px>   no matrix
  frame 4: DoAction(2)   PlaceObject2  0x03 Move|HasCharacter        depth=1 char=198
                         <DefineShape 332.0 x 492.0 px>   no matrix

sprite 200 (_corvo_mc), 1 frame
  depth 1  char 190 'bkgd_mc'      (-59.25,   9.15) px      <- the brush strokes, one frame, never moves
  depth 4  char 199 'portrait_mc'  (160.00, 246.00) px

sprite 201 'm_nGame', 66 frames
  depth 12 char 200 '_corvo_mc'    (-640.65, -150.95) px
  depth  5 char 173 '_menu_mc'     (-426.50,  -25.95) px, rotation -4 deg
```

**The dumper is cross-checked against two live measurements before anything is concluded from it**:
`_corvo_mc` at `(-640.65, -150.95)` is the number agent FC read back through AS2 on both of its
builds, and `_menu_mc` at `(-426.50, -25.95)` with a -4° rotation is the number agent FB's dismod
capture read out of the **running retail game**. Both come out of the file unchanged.

Read off the dump, the arithmetic is the whole fault. Frame 1's artwork is 320x492 at shape-local
(0,0); the matrix puts it at portrait_mc-local (-160, -246); `portrait_mc` sits at `_corvo_mc` (160,
246); so the artwork lands at `_corvo_mc` (0, 0)..(332, 492). Frames 2-4 carry no matrix, so a
runtime that does not inherit leaves the artwork at portrait_mc-local (0,0) and it lands at
`_corvo_mc` (160, 246)..(492, 738) — **+160, +246**, and its bottom 18 px past the 720-px stage.

## 2. What retail does, read out of both binaries

`GFxDisplayList::ReplaceDisplayObject` — **2013 `0x9ccb50`** (VA `0xdccb50`), resolved by hand with
`ida_funcs.get_func` to `0xdccb50..0xdcccc6`, mangled
`?ReplaceDisplayObject@GFxDisplayList@@QAEXABVGFxCharPosInfo@@PAVGFxCharacter@@@Z`; 2012 twin
**`0x9d6350`**, `0x9d6350..0x9d64c6`, same signature, and the 2013 database's own header line reads
`2012 rva 0x9d6350 ratio 1.000 bytes`. Both bodies decompiled in
`build/agentFH/ida_replace2013.txt` and `ida_replace2012.txt`. In the 2013 body, with `a2` the
record and `v22` the character being replaced:

```c
if ( (*((_BYTE *)a2 + 83) & 8) != 0 )  v11 = (char *)a2 + 12;      // the record's colour transform
else                                   v11 = (char *)v22 + 36;     // ... else the OLD character's
qmemcpy((char *)a3 + 36, v11, 0x20u);

v13 = (float *)((char *)a2 + 44);                                   // the record's matrix
if ( (*((_BYTE *)a2 + 83) & 4) == 0 )  v13 = (float *)((char *)v22 + 68);   // ... else the OLD one's
*((float *)a3 + 17) = *v13;  ...  *((float *)a3 + 22) = v13[5];

if ( *((char *)a2 + 83) >= 0 )  v14 = (*vtable[5])(v22);            // the old character's blend mode
else                            v14 = *((unsigned __int8 *)a2 + 82);
(*vtable[9])(a3, v14);

*((float *)a3 + 7)  = *((float *)a2 + 17);                          // Ratio,     unconditional
*((_WORD *)a3 + 58) = *((_WORD *)a2 + 40);                          // ClipDepth, unconditional
(*vtable[12])(a3, filters);
```

**Which of those two conditional copies is the matrix is the one thing in this that is easy to get
backwards, and this package got it backwards first.** The field offsets come from
`GFxPlaceObjectBase::UnpackedData::UnpackedData`, **2013 `0x9f0a00`** (VA `0xdf0a00`,
`0xdf0a00..0xdf0a3e`; 2012 `0x9fa130`), which constructs a `GRenderer::Cxform` at **+12** and calls
`GMatrix2D::SetIdentity` at **+44** — so `+12` is the colour transform and `+44` is the matrix, not
the other way round. `GFxPlaceObject::Unpack` (2013 `0x9f83f0`, 2012 `0xa019d0`) confirms it from the
other side: it calls `ReadMatrix` into `+44` and `ReadCxformRgb` into `+12`. With that fixed, the
gates read `& 8` = HasCxform and `& 4` = HasMatrix — **the same bit numbering our own
`GFxCharPosInfo::Flags` already uses**, which is the reassuring answer and not the one the first
reading gave.

The rest of the record, for the next package: `+68` Ratio, `+72` Depth, `+76` CharacterId (default
`0x40000`), `+80` ClipDepth (u16), `+82` blend mode (u8), `+83` the flag byte. On the character:
`+20` Depth, `+28` Ratio, `+36` Cxform, `+68` Matrix, `+116` ClipDepth. `GFxDisplayList::AddDisplayObject`
(2013 `0x9cc8f0`) and `MoveDisplayObject` (2013 `0x9cca50`) read the same record and were used to
pin every one of those.

## 3. The fix

`GFxDisplayList::ReplaceDisplayObject` takes the outgoing character's matrix and colour transform by
value before it releases it, and uses them when the record carries none; and it writes `Ratio` and
`ClipDepth` from the record unconditionally, as retail does. Four lines of behaviour.

**Deliberately not copied from retail, and why.** Retail also inherits the **blend mode** through
virtual slots 5 and 9. Our `GFxDisplayList::AddDisplayObject` does not apply `BlendMode` on the place
path either, so a replace that inherited one would be the only site in the tree that used the field;
that is a separate, consistent gap and it belongs with whoever ports blend modes, not in a diff whose
whole claim is one clip's position. Retail's `GFxDisplayList::AddDisplayObject(pos, ch, 1)` tail-call
on the miss path is likewise not reproduced: ours falls through to `InsertAt` plus the same setters,
which is what it already did.

## 4. The measurement, before and after, through the same driver

Every capture is an `-apshottime` screenshot taken **inside the process** at the end of
`UGameViewportClient::Draw`, claimed by `-apshotname=fh<tag>` so a directory four agents share cannot
hand this run somebody else's picture. Nothing here reads the desktop. `build/agentFH_run.py` stages,
launches, drives and claims; `build/agentFH/*_log.txt` is each run's own log.

**No capture in this package was ever claimed by arrival time.** `build/agentFH_run.py` records the
set of files already matching `apshottime_fh<tag>*` before the launch and claims only names the
engine wrote for this run under this run's own tag. That matters: the coordinator reports that
agent FF's runner claimed nine bitmaps by arrival time and that `apshottime_fhrelD*` — this
package's — were among them. Those are this package's files, copied into `build/agentFH/` by name by
the run that produced them, and every conclusion drawn from one was also checked against that run's
own log line. Nothing here needs rechecking for that reason.

**The first capture was verified before anything was concluded from it** and had real content
(1,714,662 bytes, a 1008x567 main menu). The window came up **1008x567** rather than the 1600x900 the
command line asks for on eight of the ten runs and 1600x900 on two, with no change to the switches,
so which resolution it takes is apparently not deterministic; §10. It does not affect anything here,
because every pair compared was captured at one size.

### The instrument

A sprite's display list is **invisible from ActionScript**: a `DefineShape` is not a `MovieClip`, so
`-disclipdiag` cannot address the thing that moves. This package added a trace at the end of
`GFxPlaceObject2Tag::Execute`, on when `DIS_GFX_PLACETRACE` is set in the environment to a
target-path substring (`portrait_mc` here; `1` traces everything, which is 60 lines a second from the
menu's 280-frame background clip and is why it is filtered).

### The numbers

Same build of the driver, same schedule, same world-time marks, one run each:

```
HEAD 63c2539
  frame 1  char 192  flags 0x06 HasCharacter HasMatrix   -> place    matrix px (-160.00, -246.00)
  frame 2  char 194  flags 0x03 Move HasCharacter        -> replace  matrix px (   0.00,    0.00)
  frame 3  char 196  flags 0x03 Move HasCharacter        -> replace  matrix px (   0.00,    0.00)
  frame 4  char 198  flags 0x03 Move HasCharacter        -> replace  matrix px (   0.00,    0.00)

HEAD 63c2539 + agent FH
  frame 1  char 192  flags 0x06 HasCharacter HasMatrix   -> place    matrix px (-160.00, -246.00)
  frame 2  char 194  flags 0x03 Move HasCharacter        -> replace  matrix px (-160.00, -246.00)
  frame 3  char 196  flags 0x03 Move HasCharacter        -> replace  matrix px (-160.00, -246.00)
  frame 4  char 198  flags 0x03 Move HasCharacter        -> replace  matrix px (-160.00, -246.00)
```

`build/agentFH/ff_rel_log.txt`, `ff_head_log.txt`. All four difficulties, EASY unchanged, the other
three now identical to EASY to the last digit, and the depth still reads `depth 1` after FF's bias
change. The same pair taken on `18f36a3` before the rebase (`rel4_log.txt`, `head4_log.txt`) is
identical line for line, so **FF's commit changes nothing about this fault in either direction**.

### The pixels

`build/agentFH/ba_difficulties.png` is the eight captures taken on `63c2539`, HEAD above and this
package below, EASY / NORMAL / HARD / VERY HARD. `build/agentFH/sbs_hard.png` is ours-before,
ours-after and **retail's own half of `resources/reference/menu/4.png`** in one column, all three
scaled to the same rectangle.

Normalised cross-correlation of an 82x102 patch cut round Corvo's mask in the fixed capture:

| searched in | peak | correlation |
|---|---|---|
| ours at HEAD | **dx +127, dy +194** | 0.587 |
| retail | **dx +1, dy +0** | 0.589 |

and the display list predicts `160 x 1008/1280 = 126.0` and `246 x 1008/1280 = 193.7`. The three
independent routes — the asset's matrix, the runtime's matrix, and the pixels — agree to one pixel.

### Two things that were wrong in the hand-over, and one in my own brief

* **"about 140 px right and 255 px down."** It is +160 and +246 stage px. The eyeball estimate was
  taken between two landmarks in two *different* drawings, and frames 2-4 are a different drawing
  from frame 1.
* **"and smaller."** It is not smaller. Frames 2-4's artwork is **332 x 492** against frame 1's
  **320 x 492** — twelve pixels *wider*. What reads as smaller is that the displaced copy runs 18 px
  past the bottom of a 720-px stage and the bottom third of the figure is simply not drawn.
* **My brief's hypothesis** — "a `PlaceObject` on a later frame being treated as a *new* placement
  rather than a *move*… or a matrix being composed twice" — is not it either, in both halves. The tag
  was already being dispatched correctly as a replace (`GFxPlaceObject2Tag::Execute`'s three-way
  split is right and unchanged), and the matrix was composed not twice but **zero** times.

And one more, measured rather than argued: **`portrait_mc`'s bounds *are* recomputed on a frame
change**, on both builds. The hand-over records "`_width` / `_height` report 320 x 492 on every frame
although the frames plainly draw different-sized artwork". `-disclipdiag` on the New Game screen
while `-disdifficulty` walks the four frames reports `_width=320` on frame 1 and **`_width=332` on
frames 2, 3 and 4** — the authored sizes the dump gives — at HEAD as well as here, on `18f36a3`
(`build/agentFH/relV_log.txt`, `headV_log.txt`) and again on `63c2539` (`ff_rel_log.txt`,
`ff_head_log.txt`). That loose end is closed and it was never a defect.

## 5. The three suspects, each eliminated

* **`GFxSprite::GotoFrame`** (2012 `0xa012a0`) is reached on every difficulty click and behaves
  correctly: a forward goto runs the intervening frames' tags without their actions and the target
  frame's with them; a backward goto marks the list for removal, replays 0..N and sweeps. The trace
  shows exactly the tags that ordering predicts, in order, on both builds.
* **`IncrementFrameAndCheckForLoop`** is only called from `AdvanceFrame`. A `gotoAndStop` does not go
  near it.
* **`AddDisplayObject`'s same-character early-out** is in `GFxSprite::AddDisplayObject`, which a
  `Move|HasCharacter` tag never enters — `GFxPlaceObject2Tag::Execute` sends it to
  `ReplaceDisplayObject` instead. The trace confirms it: every one of these tags logs `-> replace`.

The suspects were the right *neighbourhood* and the dump is what narrowed it; the function itself was
one the hand-over did not name.

## 6. What else this can touch — complete, not sampled

`build/agentFH/scan_replace.py` and `scan_inherit.py` walk **every** sprite of **every** cooked UI
movie and list every tag that takes the replace path, with what it would now inherit.

| movie | sprites | replace tags | of those, inheriting a NON-identity transform |
|---|---|---|---|
| `Dishonored_MainMenu…MainMenu.gfx` | 143 | 7 | **3 — all three are `portrait_mc`** |
| `Startup.Common_assets.lib.gfx` | 119 | 16 | 12, every one a `*_Multi` controller-icon clip |
| `Startup.UI_LoadGame.LoadGame.gfx` | 48 | 6 | 0 |
| `Startup.UI_OptionsMenu.OptionsMenu.gfx` | 122 | 3 | 0 |
| `UI_Gamma_SF…GammaImage.gfx` | 15 | 0 | 0 |
| `UI_PauseMenu_SF…PauseMenu.gfx` | 85 | 5 | 2 |
| | | **37** | **17** |

The two other multi-state clips in the main menu, sprites **51** and **125**, place their frame-1
artwork with an explicit **identity** matrix, so inheriting it is the same as not inheriting it:
**nothing on the main menu, options, brightness or New Game screens can move except the portrait.**

The twelve in `lib.swf` are `lib_RS_scroll_Multi`, `lib_LS_scroll_Multi`, the four trigger/shoulder
clips, `lib_DPAD_Multi_b`, `lib_Start_Multi`, `lib_RS_click_Multi`, `lib_LS_click_Multi`,
`lib_RS_Multi`, `lib_LS_Multi`, and they inherit small centring offsets of (-20,-20), (-25,-25) and
(-40,-40) px. This fix **corrects** them — at HEAD each would snap 20 to 40 px on a frame change —
but none of them is live today: they are the platform-switch clips that raise
`no frame named 'PC'` forty times a run, which is agent FF's item 6. The two in the pause menu
inherit (-456.80, -24.00); the pause menu is not reachable yet.

And the pixels say the same thing, measured on landmarks rather than on a whole-frame difference.
Three places on the New Game screen that are not the portrait, matched between the before and after
captures of the *same* difficulty:

| landmark | ours-after vs ours-before | correlation |
|---|---|---|
| the four difficulty rows | **dx 0, dy 0** | 0.947 |
| the `NEW GAME` title | dx +1, dy 0 | 0.926 |
| the `ACCEPT` / `BACK` bar | **dx 0, dy 0** | 0.963 |

against the portrait's own **+127, +194**. The same `ACCEPT` / `BACK` bar against retail is
**dx -1, dy +1**, beside the portrait's **+1, 0** — ours and retail agree on both to a pixel.

A whole-frame difference is *not* a good instrument here and this package tried it first: the New
Game screen's background is a **rendered 3D level**, not a flat image, and two separate processes do
not put its camera on exactly the same sub-pixel. `build/agentFH/diff_head_rel_hard.png` shows the
result — the two solid portrait copies plus a faint one-pixel edge outline over every roof in
Dunwall. An earlier pair of runs whose backgrounds happened to agree put 95.5 % of the strongly
differing pixels inside the two portrait rectangles; on this pair it is 36 %, and the difference
between those two numbers is the background, not the change. The landmarks above and the exhaustive
tag scan are what the claim rests on.

## 7. The blocker that changed how this was measured, and it is STATUS defect 8

The first run drove real clicks onto the four difficulty rows, the way agent FC did. Every one of
them was delivered to something else. `build/agentFH/relNG_drive.txt`:

```
window 0xa0814, foreground 0xa0814
  +2.7s  key 0x20 ...                      (foreground 0xa0814)   <- keyboard: the game
 +23.4s  hoverclick 0.290,0.497 ...        (foreground 0x1407f8)  <- mouse: not the game
 +37.2s  hoverclick 0.290,0.560 ...        (foreground 0x1407f8)
 +39.8s  hoverclick 0.290,0.636 ...        (foreground 0x1407f8)
 +53.0s  hoverclick 0.290,0.713 ...        (foreground 0x1407f8)
```

A window enumeration during the run names `0x1407f8`: **`rundll32` — "Windows Security Alert"**, and
there were **five** of them holding the desktop foreground, plus another agent's
`DishonoredGame_FIH` window. The log for that run carries exactly one placement trace — the screen's
first frame — and no difficulty ever changed. Keyboard input still arrives (Space and Enter opened the
New Game screen in every run), but this screen's arrow keys do not move the selection, which is STATUS
defect 5. **No dialog was clicked, dismissed, minimised or touched.**

So the schedule is driven at the AS2 end instead: `-disdifficulty=<worldTime>:<idx>;…` calls
`_root.newGame_mc._corvo_mc.portrait_mc.gotoAndStop(idx+1)`, which is literally the body of the
asset's own `SetPortrait` (`MainMenu.as2.txt:1711-1762`) and therefore the same code path a click
takes, minus the click. It is keyed to **world time**, the clock `-apshottime` tests its marks
against, so a step and a mark mean the same instant on both builds whatever the frame rate.

It is also a *better* control than a click: between two marks **nothing on the screen changes but the
portrait's frame** — not even which row is highlighted, which is why all eight captures in
`ba_difficulties.png` show EASY selected.

The dialogs remain. They have now cost measured runs in five waves and the user is the only one who
can answer them.

## 8. Gates

| gate | result |
|---|---|
| `run_regression.py`, the worktree's own copy, absolute build dir, built inside the harness, on `63c2539` | **37 ok, 0 failed, 0 skipped**, 440 s — `build/agentFH/regression.log` |
| clean full Release, build directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`, all three targets | **0 errors, 0 C4263, 0 C4264** — `build/agentFH/buildclean.log` |
| `rva_sweep.py`, whole worktree | 7,405 citations, **3 suspects** (`disbehaviorpatrol.cpp:319`, `gfxuirenderer.cpp:1679` x2) — **all three present at `63c2539`, none in a file this package touches** |
| `rva_sweep.py`, `External/GFx3` alone | 668 citations, **0 suspects** |
| every address this package cites resolved by hand with `ida_funcs.get_func` | 13 of them, `build/agentFH/ida_byhand_2013.txt`, `ida_byhand_2012.txt`, and `agentFH_status.csv` |

The five gitignored `resources/docs/types/` inputs were copied into the worktree before the harness
ran.

**An earlier full run of the harness came back 36 ok, 1 failed, 0 skipped, and the failure was not
this package.** `d3d9_frames` measured 540, then 720 on the harness's own automatic re-run, against a
bound of 1,000. That metric is the frame count of a fixed 90-second run and its baseline note says so
("20,220 measured at HEAD idle, 4,380 under load"). A process census taken while it ran found
**three other agents' game processes rendering at the same time** (`DishonoredGame_FF`,
`DishonoredGame_FI`, `DishonoredGame_FIH`, the last with 1,159 s of CPU behind it) and **eighteen
`cl` compilers**. Re-run alone on the same build with the machine quiet, the same stage measured
`d3d9_frames = 2370` and `9 ok, 0 failed`; the full 37-check run above was then taken on the quiet
machine. Both logs are kept: `build/agentFH/regression_d3d9.log` and the summary line at the end of
`regression.log`. Nothing in this package's diff is on the render path.

One address was caught by the by-hand pass and not by `rva_sweep`: this report's first draft
cited the UnpackedData constructor as `2013 0xdf0a00`, which is its **VA**. `rva_sweep` passed it
silently. The tree's convention is the RVA, `0x9f0a00`; remember `+ 0x400000`.

## 9. Merging

* **Regeneration is NOT required and was NOT run.** No `UClass`, no script class, no native function
  signature change; nothing in the generated headers.
* **`Sources.cmake` did not change.**
* **Three source files, five with the report and its CSV. Total: 5.** The diff is **150 added lines,
  0 removed**, of which 32 are in the fix's file and 20 of those are its comment.
* **`External/GFx3`, the functions touched — the list agent FF needs:**
  * `GFxPlayerSprite.cpp`: **`GFxDisplayList::ReplaceDisplayObject`, and nothing else.** Not
    `AddDisplayObject`, not `MoveDisplayObject`, not `GFxSprite::AddDisplayObject`, not `GotoFrame`.
  * `GFxPlayerData.cpp`: **`GFxPlaceObject2Tag::Execute`** (four lines appended, all inside
    `if (GFxDK_PlaceTraceOn())`), plus two new file-scope functions beside it,
    **`GFxDK_PlaceTraceFilter`** (static) and **`GFxDK_PlaceTraceOn`**.
  * **No header changed.** The trace is gated on an environment variable rather than a new
    `GFxSprite::b…` static precisely so that `GFxPlayer.h` — where agent FF's work lands — is not in
    this diff at all.
* **One file outside `External/GFx3`**:
  `source/Development/Src/DishonoredGame/Src/disgfxmovieplayermainmenu.cpp`, one new static
  **`DisDriveDifficulty`** and one call to it at the top of
  **`DisGFxMoviePlayerMainMenuPreAdvance`**, beside the existing `DisReportClipDiag` call. Agent FG
  may be in the same file for `?CATEGORY?`; the two additions do not overlap, but it is one file and
  worth sequencing.
* **Agent FF has landed as `63c2539` and this package is rebased onto it; the whole measurement was
  retaken there.** The three-way apply of this package's diff onto `63c2539` was clean on all three
  files, and in `GFxPlayerSprite.cpp` FF's nine hunks are at lines 36, 55, 499, 541, 549, 1518, 1758,
  1797 and 1853 while this package's are at 388-443 — **no overlap, and FF does not touch
  `GFxDisplayList::ReplaceDisplayObject` at all.** On FF's two warnings:
  * The **AS depth bias** is applied at the ActionScript boundaries (`GFxAS2Lib.cpp`,
    `GFxAS2Interp.cpp`, `GFxPlayerRoot.cpp`), not in the display list: there is no `0x4000` anywhere
    in `GFxPlayerSprite.cpp` after FF. Every tag in this fault is a **timeline** tag at SWF depth 1,
    which is unbiased on either convention, and the trace prints the depth — it still reads
    `depth 1` on `63c2539`.
  * **`bScriptCreated` is gone** and this package never referenced it. The same-character early-out
    that used to be the third suspect is in `GFxSprite::AddDisplayObject`, which these tags do not
    enter (§5); nothing here is keyed on that flag.
* `build/agentFH_sync.py` is the authoritative copy list, worktree → main, and **flags rather than
  copies** anything main has changed since `63c2539`. Dry run at the time of writing: 5 files to
  copy, **0 flagged, 0 missing**.
  * **A trap for the next sync script, found by this one raising a false alarm.** This tree stores
    some sources with LF and some with CRLF, and the working copy of an LF-stored file is CRLF on
    disk. A byte compare of `git show <base>:<path>` against the file in main therefore reports every
    LF-stored file as "main changed it". `GFxPlayerData.cpp` is one: 48,331 bytes in the blob, 49,657
    on disk, exactly 1,326 extra bytes for 1,326 lines, and `git status` clean. The script normalises
    line endings before hashing and still copies byte for byte. A sync script that does not will
    either flag files nobody touched or, worse, be "fixed" by dropping the check.

## 10. Hand-over

* **`portrait_mc` advances one frame on its own before its `stop` lands.** The trace shows, on both
  builds and in every run, a frame-1 place followed immediately by a frame-2 replace, before anything
  has asked for a difficulty. Each frame's `DoAction` is two bytes — `stop` — but an action buffer is
  pushed onto the movie root's queue and drained after the next `AdvanceFrame`, so the clip gets one
  free frame. It is invisible today because the screen's `Open` calls `SetPortrait` anyway, and it was
  invisible before this package because frame 2 was in the wrong place in any case. It is a real
  ordering difference from retail and it is one line of evidence away from being measured properly.
* **The window came up 1008x567 with `-ResX=1600 -ResY=900` on the command line** on eight of the ten
  runs here, across both builds, and 1600x900 on the other two, with the same switches and the same
  isolated ini directory. Not investigated. Anyone comparing captures across packages should read the
  bitmap's size rather than assume it; every pair compared in §4 was captured at one size.
* **`GFxSprite::ReplaceDisplayObject` does not name the new character, does not register it as a
  member of its parent, and does not run its frame-0 events for a sprite** the way the attach and
  place paths do; retail's four-argument overload (2013 `0x9eb0b0`) takes a `GASString& name`. No tag
  in any of the six movies replaces a **named** character, so it is latent, and widening the diff to
  cover it would have put an untested path in the same change as a measured one.
* **The tag dumper is general.** `tagdump.py --all` over any of the six movies is a complete
  display-list listing, and `--name <instance>` answers "where does this clip come from and what does
  its timeline do" in one call. The next package that needs to know what an asset actually asks for
  should start there rather than inferring it from ActionScript.
