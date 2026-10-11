# Agent FM — the gate that missed two regressions, and the HUD switch

Wave 19, PHASE15 package FM. Base `17a7c32`, rebased mid-package onto **`98e22d4`** when the coordinator
landed the network-listener change. Worktree `build/agentFM_wt`, build dir
`D:/RecompileDishonored/Recompile/build/agentFM_wt/build/fmreg`, every run through the worktree's own
copy of the harness with an absolute build dir.

> **The headline, before anything else: the Options check reproduces the user's crashing ActionScript
> sequence exactly — same traces, same `InitScrollBar (385.699066162109, …)`, same
> `mask stage bounds 170.0,214.6..1151.7,666.2 px` as the frame the process died on — and the process
> does NOT die. Six measured runs that reached the fatal tab change (fm_disc6, fm_disc7, fm_clk1,
> fm_opt8, fm_hud1, fm_hud2), every one 0 criticals and alive to its timeout. So the check does not fail on `c702a38`, and I am
> saying so rather than shipping a green check that claims to cover the crash. §4 has what the
> difference is and what it means for agent FJ.**

## 0. What landed

| | |
|---|---|
| `newgame` stage | boots the configuration the user plays — **no `-startmap`** — reaches `l_tower_p` and asserts the opening scene is populated **and that the front end's own menu was still open**, which is the part `-finewgame` alone cannot see |
| `options` stage | opens Options off that menu, switches tabs on the user's own schedule, asserts the process is alive and the AS2 error count has not risen |
| `-playdefaults` | turns on the `DISHONORED_PLAY_DEFAULTS` table in a build not configured with it, so the harness can drive the shipped command line without a second full build and without touching the five calibrated stages |
| `-fmmenu` | reports twice a second whether the menu movie is open and which screen it is on |
| `-fmdrive=<t>:<step>[;…]` | a world-clock schedule of menu steps: `start`, `fill`, `invoke=<path>(<args>)` |
| `CRITICAL_RE` | **the harness could not see a hard crash at all.** Every `*_criticals` metric matched the literal `Critical: `; an access violation reaches the log as `=== Critical error: ===` and carries no `Critical: ` line. The user's crash log has `Critical: ` **zero** times |
| the `-dishud` answer | retail has **no gate**; the shipped build should open the HUD and the harness should suppress it. Done, with a sequencing guard, because flipping the default alone crashes the menu — §5 |

**The check count goes from 37 to 47.** Five rows per new stage: the stage's own `_log_lines` guard plus
four metrics. Named in §2 so the coordinator can update every other package's expectation.

## 1. Where my brief was wrong

Five premises, all measured.

1. **`c702a38` is not a commit I had to go back to: HEAD's code *is* `c702a38`'s.**
   `git diff --name-only c702a38 17a7c32` outside `resources/docs/` and `README.md` is **empty** —
   `21db165`, `462256c` and `17a7c32` are documentation only. "Must fail on `c702a38`" therefore means
   "must fail at my own base", and no second worktree was needed.

2. **`-finewgame` would not have caught `b540be6`.** The brief says FI's switch "already drives this".
   It drives the *mission*, not the *menu*: FI's own report says it issues `ce ChangeLvl_StartNewGame`
   **from the game thread without touching the menu**. A `newgame` check built on `-finewgame` alone
   would have gone green on the very commit the package exists because of. That is why `-fmmenu` and
   `newgame_menu_open_seconds` exist: the metric is the **last world time at which the menu movie was
   still open**, and `b540be6` would read about 1.

3. **"Two `OnTabSelected` in one frame, a scroll bar initialised twice" is not the anomaly.** It is
   normal in this build. `play_live.log` has the identical pattern at **17.47**, **21.64** and **31.98**
   with no crash, and the `ClearAnimation` / `tweenEnd` / `tweenTo` / `gotoAndStop` / `gotoAndPlay`
   cascade the plan quotes as the run-up to the fault fires **word for word at 17.47 as well**, sixteen
   seconds before the process dies. What is actually distinctive about 33.46 is in §4.

4. **The user did not launch with "no arguments".** `play_live.log:7` is
   `-LOG=play_live.log -forcelogflush` plus the six switches the play-defaults table appends. That is
   good news: it is exactly reproducible by a harness stage, which is what `-playdefaults` does.

5. **`UDisGlobalUIManager::RefreshGlobalUIState` is not unported, and its address was already in the
   tree.** PHASE15 calls it "2012 `0x8b75e0`, unported". It is `disglobaluimanager.cpp:113`, labelled
   `2013 rva 0x847070 (2012 0x8b75e0, private)`, with the blur arm ported by agent FA and the other four
   bits of the same walk written down by name. It also **opens and closes nothing** — the comment in
   `disgfxmovieplayerhud.cpp` that said it "opens and closes them as the game moves between the menu and
   a level" was wrong, and is now corrected in place.

And one correction to a helper report that reached me: the addresses it gave were **VAs**, and this
tree's comment convention is **2013 rva** — the image base already subtracted. `0xc47070` is
`0x847070`. Writing a VA where the tree writes an rva is exactly the `0xdf0a00`-for-`0x9f0a00` mistake
STATUS records, in the other direction, and `rva_sweep.py` did not catch it because the addresses sat in
a banner comment it does not scan. Every address below is the rva, and §6 is the by-hand resolution.

## 2. The ten new rows

| metric | op | value | what it catches |
|---|---|---|---|
| `newgame_log_lines` | >= | 2000 | the run reached the front end at all |
| `newgame_menu_open_seconds` | >= | 10 | **`b540be6`**: the menu movie closing under the player |
| `newgame_on_mission` | >= | 1 | New Game's hub sequence committed the map change to `l_tower_p` |
| `newgame_pawns` | >= | 10 | the opening scene is populated (FI measured 29) |
| `newgame_criticals` | == | 0 | a crash on the menu → mission path |
| `options_log_lines` | >= | 2000 | as above |
| `options_steps` | >= | 6 | every AS2 path in the schedule still resolves |
| `options_alive` | >= | 26 | **the process is still advancing the movie after the tab switches** |
| `options_as2_errors` | <= | 41 | the error count has not risen (41 twice with the HUD open, 39 with `-nodishud`; tight on purpose) |
| `options_criticals` | == | 0 | a crash anywhere in the stage |

`options_alive` is written so that a **dead process FAILS rather than skips**. The baseline requires it,
so `measure()` scores an absent line as `-1` (`unparsable`). A row declared `optional` would have
skipped on a crash — which is the exact shape of the hole this stage exists to close, and worth saying
out loud because `optional` is the obvious way to write a new metric and it is the wrong way here.

Five of the ten are in `LOAD_SENSITIVE`, for a different reason than the throughput rows: these stages
are driven on the **world** clock and killed on the **wall** clock, and `d3d9_startup_seconds` has read
3.4 s quiet and 141.9 s under another agent's build. A machine that spends two minutes in startup does
not reach world time 22 inside the window. `options_criticals` is deliberately **not** in the set, so a
genuine crash fails the stage once and is not re-run.

## 3. The New Game check

Command line, with the isolation every harness run has removed for clarity:

```
-forcelogflush -playdefaults -finewgame=22 -fidiag -fmmenu -fmdrive=8:start
```

and `-playdefaults` appends `-nosteam -skipnativepkgs=OnlineSubsystemPC -gfxuimenu -nomovie -windowed
-ResX=1600 -ResY=900`, verified in the log's own `Init: Command line:` echo. The only difference between
this and what the user types is the `-playdefaults` token that enables the table, so a regression in the
**table itself** is caught too.

`8:start` dismisses the start screen through the menu's own `FilterButtonInput` with `SpaceBar` — the
real path, and the one the README tells the user to use, because Enter does not activate a main-menu row
(STATUS open defect 5). Without it the game sits on the start screen for ever and the menu proper never
opens, so `-finewgame` would reach the mission with the *front end* never having been exercised.

Measured: see §7.

## 4. The Options check, and why it does not catch the crash

The schedule, read step for step off the user's own log:

```
8:start
14:fill                                                     DisFillOptionsMenu(TRUE) = retail's OnOptionsClicked (2013 rva 0x7e6b80)
15:invoke=_root.mainMenu_mc.Close()                         the `>> Close : undefined` of the user's 17.26
16:invoke=_root.optionsMenu_mc.SetMenu()                    ClearIntervals, _bIsOpening, InitTabs, Open
20:invoke=_root.optionsMenu_mc._subTabs_mc._tabs_mc.OnTabSelected(1)
24:invoke=_root.optionsMenu_mc._tabs_mc.OnTabClicked(1)     the button's own handler, one level closer than OnTabSelected
```

`build/agentFM/shots/fmo8_mark1.png` is what that produces, captured in process at world time 27 with
`-apshottime=27 -apshotname=fmo8`, **1600x900 read off the file**: the OPTIONS title, the four category
tabs with CONTROLS selected, the three sub-tabs with KEYBOARD MAPPING selected, the mapped-key list with
its scroll bar, and the ACCEPT/BACK bar. It is the user's screen.
`build/agentFM/shots/fmd6_mark3.png` is the same run *without* the `mainMenu_mc.Close()` step and shows
why that step is in the schedule: the main-menu row bar draws through the Options screen.

The AS2 map behind those paths, which nothing in this repository had written down:

* `_root.optionsMenu_mc` is the **`__Packages.OptionsScreen`** instance. `SetMenu()` is the entry point
  the screen transition reaches — `ClearIntervals(); _bIsOpening = true; InitTabs(); Open();`
  (`build/agentEA/as2/om_244.txt:507–527`). `Open()` alone does **nothing visible**: measured, it
  resolves and produces no tab selection, because `InitTabs` is what builds `_tabsContent` and selects
  the start tab.
* `_root.optionsMenu_mc._tabs_mc` is a **`__Packages._common.TabsHandler`**. Its
  `OnTabSelected(idx)` takes **one** argument and computes `"l"` or `"r"` itself
  (`build/agentEY/swf/OptionsMenu.as2.txt:13104–13286`), then calls `_onSelectedTabCallback(idx,
  animSide)` with `_tabs_mc` as `this` — which is `OptionsScreen.OnTabSelected(idx, animSide)`, the
  two-argument one that traces `-------------------->> OnTabSelected (1, r)` and calls `UpdateSubTabs`
  then `HideList`. **The two-argument trace is not a two-argument entry point**, and that mattered: the
  first schedule I wrote called the two-argument form directly and silently did half the work.
* `_root.optionsMenu_mc._subTabs_mc._tabs_mc` is the **`__Packages._common.SubTabsHandler`**, whose
  one-argument `OnTabSelected(idx)` is the `>> OnTabSelected : <n>` trace.
* `OnTabClicked(idx)` is `TabsHandler`'s own mouse handler, one call above `OnTabSelected`, and the
  schedule uses it rather than `OnTabSelected` for the final step precisely because it is the
  closest thing to the user's click that a native `Invoke` can reach. It still does not reproduce
  the crash (`fm_clk1.log`, two top-level tab changes, 0 criticals).
* The buffer sizes in the user's AS2 errors identify the classes exactly: 6659 = `TabsHandler`,
  2376 = `SubTabsHandler`, 2730 = `_common.ScrollBarList`, 5861 = `OptionsList`.

**What the run produces**, `fm_disc6.log`, against the user's `play_live.log`:

| | user, dies at 33.77 | this check |
|---|---|---|
| `-------------------->> OnTabSelected (0, l)  / undefined` | 17.47 | 21.56 |
| `>> UpdateSubTabs`, `_bOpened = undefined`, `_subCategoriesList.length : 2` | 17.47 | 21.56 |
| `>> OnTabSelected : 1` ×2, `InitScrollBar (384.996643066406, …)` ×2 | 31.82 / 31.98 | 25.35 / 25.51 |
| `-------------------->> OnTabSelected (1, r)  / undefined` | 33.46 | 29.36 |
| `_bOpened = true`, `_subCategoriesList.length : 3` | 33.46 | 29.36 |
| `InitScrollBar (385.699066162109, …)` ×2 | 33.61, 33.62 | 29.51 ×2 |
| `setMask … mask stage bounds 170.0,214.6..1151.7,666.2 px` | 33.62 | 29.51 |
| **what happens next** | **dead, 0.15 s later** | **nothing; 0 criticals, alive to the timeout** |

Every number matches, including the two `InitScrollBar` arguments that differ in the fourth decimal
between the first selection and the fatal one, and the mask's bottom edge moving from 665.5 to 666.2.
The display state at the fatal instant is the same display state. **The process does not die.**

So what is different? One thing, and it is in the log:

```
[0033.46] AS2 trace: OnMouseUp (0, undefined)
[0033.46] AS2 trace: -------------------->> OnTabSelected (1, r)  / undefined
```

The user's tab switch was entered **from inside GFx's own mouse dispatch** — `GFxMovieView::HandleEvent`
→ the button model → `OnTabClicked` → `OnTabSelected` — in the same frame as `OnMouseUp`, re-entrantly
with respect to the runtime's advance and display state. An external `GFxMovieView::Invoke` enters
ActionScript at a clean point between advances. **That is the first candidate on PHASE15's own list**
("`ctx.MaskLevel` / `MaskDrawDepth` accounting across a nested or re-entrant `Display`"), and this is a
measurement in its favour: the same AS2 work, the same display list, the same mask, crashes from one
entry point and not from the other.

For agent FJ, three concrete things:

1. The AS2 work is **not** sufficient. Do not chase a hypothesis that only involves the tab switch.
2. The user's run also **dragged the scroll bar to the bottom** between the sub-tab toggles —
   `>> OnLifterPressed` at 23.80, `ratioScroll` walking 0.623 → 0.999964, `>> OnLifterReleased` at
   25.11. I could not drive that: `_root.optionsMenu_mc._options_mc.MoveTo_End()` resolves **NOT FOUND**,
   so the `ScrollBarList` instance is not `_options_mc` under that name. Worth settling, because the
   fatal tab switch re-attaches `_options_mc` while the list is scrolled to its extreme.
3. What the check *is* good for: it is the control. It builds the whole Options screen through the
   screen's own entry point, exercises every AS2 path, and reports the error count — so a fix can be
   measured against it even though it does not reproduce the fault.

I have **not** renamed it to pretend otherwise. `options_alive` is in the baseline at a value it
currently passes, and the metric's note says what it does and does not cover.

## 5. `-dishud`: retail has no gate, and flipping the default alone crashes the menu

**Retail's condition is that there is no condition.** `ADishonoredGameInfo::PostBeginPlay`
(2013 rva `0x6155d0`) → `InitGlobalManagers` (`0x5e9bd0`) → `UDisGlobalUIManager::Init` (`0x85fdb0`,
1428 bytes, 2012 `0x8c45d0`) calls `UDisGFxMoviePlayerHUD::StartHUD` (`0x78cd00`) unconditionally, on
every map including the menu map — the retail ini names `DefaultGame=DishonoredGame.DishonoredGameInfo`
for everything and has no separate menu `GameInfo` — and `UDisGFxMoviePlayerBase::Start` has exactly one
early-out, `if( bMovieIsOpen ) return TRUE`. No `GIsGame`, no state enum, no menu or cinematic flag, no
player check. The HUD movie is **always open**; what varies is `pMovie->fVisible`, which
`RefreshGlobalUIState` (`0x847070`, 901 bytes) recomputes as

```
HUD visible == !PersistentWorldInfo.m_bNeverDisplayHUD && AND-over-every-open-player( m_bShowHUDWhileActive )
```

— note that the accumulator is **AND**-ed across the open movies, not OR-ed.

**So the shipped default opens the HUD and the harness suppresses it, which is the way round I have
implemented it**: `DisHUDBringUp` now reads `-nodishud` to suppress instead of `-dishud` to enable, and
`run_regression.py` passes `NO_HUD` in `nullrhi`, `d3d9` and `inputtest`. `-dishud` is still accepted and
now means nothing, so no existing command line in the tree or in any agent's notes breaks. `-nodishud`
cannot be matched by `ParseParam` looking for `dishud`, because `appStrfind` only matches at a
non-alphanumeric boundary and the character before `dishud` in `-nodishud` is `o`.

Why the harness keeps suppressing it rather than re-measuring:

* the three stages' bounds are cliff detectors measured with no movie open — `d3d9_frames` against 1000,
  `d3d9_draws_per_frame`, `texture_census`. Opening a movie moves all three.
* nothing is lost, because the HUD-open path is now covered by `newgame`, which passes `-playdefaults`
  and no `-nodishud`. The configuration the user plays is tested in the stage that is the user.

**And the flip does not work on its own.** With the default on and nothing else changed, two of three
measured runs died at the menu:

```
Assertion failed: !Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading)
    [File: Core/Src/UnObj.cpp] [Line: 7979]
```

— the *replace* branch of `StaticAllocateObject` — **0.02 s after**
`-gfxuimenu: opened UI_Global.Global through DisGFxMoviePlayerGlobal`, with the main-menu movie still
mid-open and `-fmmenu` never having reported a single line. `fm_disc2.log` (crash at 22.27, HUD opened
22.25), `fm_disc3.log` (crash at 15.98, HUD opened 14.23), `fm_disc1.log` (HUD opened 3.66, clean to
world time 68). The cause is structural: **retail has one owner for every movie player and this tree has
two independent bring-up state machines racing** — `-gfxuimenu`'s in `gfxuiengine.cpp` and this one in
`disgfxmovieplayerhud.cpp`. `UDisGlobalUIManager::Init` walks the config movie set and constructs them
in its order; nothing here imposes an order.

So `DisHUDBringUp` now waits for the front end's own bring-up before opening the HUD: if `-gfxuimenu`
was asked for, the HUD holds until `Transient.DisGFxAutoOpenMenu` exists and `bMovieIsOpen`. **With the
guard: two of two runs clean** — `fm_hud1.log` (HUD opened at 6.30, 0 criticals, all five Options steps
`ok`, alive to world time 131.62) and `fm_hud2.log` (0 criticals, alive to 118.09) — against two of
three crashing without it.

**And the wait has to be latched, which I got wrong first and the measurement caught.** The map change
into the mission closes the menu movie (`CommitMapChange`, `bCloseOnLevelChange`), so an unlatched test
holds the HUD shut for the rest of the game: `fm_bars1.log` reached `l_tower_p` with
`bCinematicMode 1` and the letterbox element `0x10` set in the HUD mask (`masks 6050/7fef/…`,
"letterbox bit on at some level") and invoked `SetBlackStripes` **zero** times, because the HUD had
opened once on the menu map and could not reopen. The guard is now a one-time sequencing wait.

**With both changes, the user's fault 2 is closed.** `fm_bars2.log`, launched the way the user launches
and driven only by `-fmdrive=8:start -finewgame=22`: `HUD: opened UI_HUD.HUD` **twice** (once on the menu
map, once for `l_tower_p` after the map change), `bCinematicMode 1`, and
**`HUD: SetBlackStripes(1) ok`** — the letterbox bars, drawn, with no command line. 0 criticals. The
control is the user's own `play_live.log`, where `blackStripes` has **0** mentions, and `fm_bars1.log`
with the unlatched guard, which also has 0. `build/agentFM/shots/fmbar_mark1.png` is the picture:
`-apshottime=38 -apshotname=fmbar`, in process, **1600x900 read off the file** — the opening scene on
the whaleboat with both pawns on it and **the letterbox bars drawn, top and bottom**. (It also shows
agent FL's fault 4 plainly: both heads are pointed at the sky.)
At the menu the HUD comes up with `show flags 0x6040` and
`player status closed`, which is the right shape for a map with no pawn on screen; element `0x10` is
raised only by the mission.

**Everyone else's ad-hoc runs are affected, and this is the one thing to tell the other packages.**
Any run of the game that does not pass `-nodishud` now has the in-game HUD movie open, where before
it had to ask for one with `-dishud`. If a measurement of yours suddenly has a `DISHONORED(bringup):
HUD` line in it and numbers that moved, that is why — pass `-nodishud` and you have the old
behaviour exactly. Agent EK predicted this change in its own report (`agentEK.md:391`): "when
`RefreshGlobalUIState` lands, `DisHUDBringUp` goes away and `-dishud` with it".

**The guard is sequencing, not a fix, and I want that on the record.** It makes two racing bring-ups
take turns; it does not make them one thing. **The fix is to port `UDisGlobalUIManager::Init`
(`0x85fdb0`) and delete both of them**, and that is now the single piece of work standing between this
tree and a HUD that behaves like retail's.

Two more things read out of retail that this tree does not do, neither in this package:

* **the letterbox bars exist in two movies.** `UI_HUD`'s are `Tick_Cinematic`'s, off HUD element `0x10`,
  with a **two-argument** `SetBlackStripes` — that is the one this tree has, from wave 18 FI. `UI_Global`
  has its own, driven by `m_bDrawBlackStripesWhileActive` accumulated across the open movies, with a
  **one-argument** `SetBlackStripes`, out of `UDisGFxMoviePlayerGlobal::PreAdvance` (2013 rva
  `0x7aa610`). That path is absent, and the only writer of `m_bDrawBlackStripesWhileActive` in this tree
  is `disgfxmovieplayerglobal.cpp:263`, the message-box arm.
* **`m_bShowHUDWhileActive`'s default** on the HUD player comes from cooked script defaults, not from any
  ini. If this tree's default is `FALSE`, the movie opens and draws nothing — which looks exactly like a
  HUD that never appeared. Check that before concluding anything else is wrong.

## 6. Addresses, by hand

`rva_sweep.py --path source/Development/Src/DishonoredGame/Src`: 3211 citations, **1 suspect**, and that
one is pre-existing (`disbehaviorpatrol.cpp:319`, `0x7398a0`, not mine). The sweep is not sufficient
here for a specific reason worth recording: **it does not scan a banner `/* … */` comment block**, so six
of my seven addresses were invisible to it, which is how the VA-for-rva error above survived a clean
sweep.

All seven were therefore resolved by hand in the 2013 named database (a private copy; the shared
`.i64` is never opened twice). **Every one is an exact function start, not an address that merely lands
inside something** — which is the distinction `rva_sweep.py` cannot make:

| 2013 rva | size | name in the 2013 db |
|---|---|---|
| `0x847070` `RefreshGlobalUIState` | 901 | unnamed `sub_C47070` |
| `0x84cf70` `OnMovieStackChanged` | 5 | **named**, the only one of the seven that is |
| `0x85fdb0` `Init` | 1428 | unnamed |
| `0x78cd00` `StartHUD` | 61 | unnamed |
| `0x5e9bd0` `InitGlobalManagers` | 148 | unnamed |
| `0x6155d0` `PostBeginPlay` | 348 | unnamed |
| `0x7aa610` (`PreAdvance`) | 1241 | unnamed, **name not provable** — see §5 |

Since six carry no symbol, each is pinned structurally, and the pins are in the comment block in
`disgfxmovieplayerhud.cpp`. The hard one is `0x84cf70`: its five bytes are `E9 FB A0 FF FF`, a rel32
`jmp` whose target computes to exactly `0x847070`, and the function is **named**
`UDisGlobalUIManager::OnMovieStackChanged` — so the thing it forwards to is `RefreshGlobalUIState`,
with no inference. Three more are pinned inside this repository, independently of any database read:

* `0x847070` `RefreshGlobalUIState` — `disglobaluimanager.cpp:113` already carries it, with 2012
  `0x8b75e0`, and that file's PDB-attributed skeleton list names 2012 `0x8b75e0` as
  `private: void UDisGlobalUIManager::RefreshGlobalUIState(void)`.
* `0x84cf70` `OnMovieStackChanged` — `disglobaluimanager.cpp:103` carries `0x84cf80` for
  `OnMovieAttributesChanged`, and the 2012 pair is `0x8bc570` / `0x8bc580`, sixteen bytes apart in both
  builds.
* `0x8c45d0` `Init` (2012) — the same skeleton list, line 23.

**One caveat I am not glossing over**: the three 2012 rvas quoted anywhere in this package
(`0x8b75e0`, `0x8bc570`, `0x8c45d0`) come from this tree's own PDB-attributed skeleton lists and from
`disglobaluimanager.cpp`. They were **not** re-read out of the 2012 database, because the only open 2012
sessions on this machine belong to other packages and nothing in this package depends on them.

## 6a. What the widened crash pattern found on its first run

**The `inputtest` stage's process has been dying mid-run, and the gate has been reporting 37 ok /
0 failed over it.** The first full run with `CRITICAL_RE` in place failed exactly one row:

```
  FAIL inputtest_criticals      1 (wanted == 0)
```

It is **not** this package's doing and not a teardown crash. The stage passes `-nodishud` and never
sees `-playdefaults`, and the identical line is in three other logs in the shared retail install that
predate every change here: agent FJ's gate run, agent FI's run, and **the coordinator's own
`gate_rel` run of HEAD**. (Agents FG's and FH's inputtest logs have 0, so it is intermittent.)

What it is, from `regression_…_fmclean_inputtest.log:3410–3512`, at **45.85 s of a 150 s budget**:

```
[0045.67] distouch probe 25/48 -> DisTrigger_9 (DisTrigger) at X=10405.3 Y=24681.5 Z=2525.3: moved 1, hit 1
[0045.67] distouch SeqEvent_Touch …Main_Sequence.SOIREE_Guards.SeqEvent_Touch_0 fired on DisTrigger_9
[0045.67] StartSpawn DisSeqAct_StartSpawn_0: 1 spawner(s) asked, spawnEvenIfVisible 1, latent actors 1
[0045.68] Warning: Skipping instancing group - an Actor may only be connected to one Group! [DishonoredSpawner_11]
[0045.85] === Critical error: ===  Fatal error!
```

`-distouchprobe` walks the pawn onto `DisTrigger_9`, that fires `SOIREE_Guards.SeqEvent_Touch_0`,
which runs `DisSeqAct_StartSpawn_0`, and **the spawn kills the process**. Every metric this stage
measures — `inputtest_moved`, the touch, kismet and physics censuses — is emitted at 45.67 or earlier,
which is precisely why a green row was possible: the run dies after the numbers and before the
timeout.

On the stack, a warning rather than a claim. Of the ten executable frames, resolved against
`build/fmclean/Binaries/Win32/DishonoredGame.map` (303,449 public symbols, nearest-public, VA − image
base 0x400000), **nine land thousands of bytes past a public symbol and should not be believed** —
`ATerrain::ClearComponents+0x2d5`, `FContentComparisonHelper::CompareClasses+0x1695` and the rest are
COMDAT-folded neighbours, and this project has read a UTF-16 string as thirteen frames before. The one
frame with a small, credible offset is `UArkComponentContainer::AddNewComponent_Internal+0x2d`
(`0xd49a7d`). Treat that as the only usable hint and get a real stack before acting on it.

**The gate is therefore red, and I have left it red.** Setting `inputtest_criticals` to `<= 1` would
make it green in one line and would also make a *second* crash invisible again, which is the exact
mistake this package exists to undo. The decision belongs to the coordinator; what is not defensible
is going back to a pattern that could not see it.

## 7. Numbers

The `newgame` stage, `fm_ng1.log` (startup 39.44 s, a moderately loaded machine):

| | |
|---|---|
| menu movie open to | world time **21.89**, `screen 3 loginStep 8` — the menu proper, not the start screen |
| `-finewgame` fires | world time 22.00, `ce ChangeLvl_StartNewGame` on `Dishonored_MainMenu` |
| first `l_tower_p` census | wall 65.6, world 22.60, 1 pawn (the level is still streaming) |
| populated | wall 68.6, world 24.61, **27 pawns**, and 29/29/29 by world 170 |
| criticals | 0 |
| log lines | 4182 |

The `options` stage, `fm_opt8.log` and `fm_disc6.log`: six steps, all `ok`, 0 criticals, the heartbeat
alive to world time 125+ in every run. The trace comparison against the user's crash log is the table in
§4 and every line of it matches.

Reproducibility of the HUD race, before and after the sequencing guard: **2 of 3 runs crashed** without
it (`fm_disc2`, `fm_disc3` crashed; `fm_disc1` clean), **0 of 3** with it (`fm_hud1`, `fm_hud2`,
`fm_bars2`).

`grep -c FocusLost` is **0** on every run quoted anywhere in this report. No run was discarded for a
stolen foreground, and none was averaged in without the check.

**The full harness.** This worktree's own copy, absolute build dir
`D:/RecompileDishonored/Recompile/build/agentFM_wt/build/fmclean`, **the directory deleted first and
all three targets built inside the harness**, `DISHONORED_LAYOUT_CHECKS` on (its cmake default, with
`DISHONORED_SHIPPING` off). 1549 s. `build/agentFM/fm_regression.log`.

```
45 ok, 2 failed, 0 skipped, 1549s
  REGRESSION inputtest/inputtest_criticals: 1 (wanted == 0)
  REGRESSION options/options_as2_errors:   41 (wanted <= 0)
```

**47 rows, which is the new total.** Of the two failures:

* `options_as2_errors` is **my own bookkeeping, not a defect.** I measured 41 and wrote it into the
  baseline while this run was already in flight, and `run_regression.py` loads the baseline once at
  startup, so the run was still comparing against the placeholder `0`. The committed baseline says 41;
  `build/agentFM/fm_regression_options.log` is the `--only options --no-build` re-run against it:
  **5 ok, 0 failed, 0 skipped, 253 s** — `options_as2_errors 41 (wanted <= 41)`. So the committed
  state of the gate is **46 ok, 1 failed, 0 skipped** out of 47, and the one failure is the next item.
* `inputtest_criticals` is real, pre-existing, and §6a.

Everything else on the clean build: `build_{DishonoredGame,CoreSmoke,LayoutProbe}_errors` **0**, exits
0, **0 C4263 and 0 C4264 across all three build logs**, `coresmoke 99 passed / 0 failed`,
`layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `layout_probed 2341`,
`verify_phase2 2/2`. The five pre-existing game stages are untouched by this package:
`d3d9_frames 15750`, `d3d9_draw_elements 6506`, `texture_census 16257`, `inputtest_moved 1010.7`,
`physics_actors 1350`, `unported_natives 0`, `probe_natives 4` — the `-nodishud` token did exactly what
it was added to do and **no calibrated threshold moved**.

The ten new rows as measured: `newgame_log_lines 10322`, `newgame_menu_open_seconds 21.6`,
`newgame_on_mission 136`, `newgame_pawns 29`, `newgame_criticals 0`, `options_log_lines 6841`,
`options_steps 6/6`, `options_alive 234.4`, `options_as2_errors 41`, `options_criticals 0`.

## 8. Files, and every function touched

See `build/agentFM_sync.py`, whose docstring is the per-function list. In summary:

| file | functions |
|---|---|
| `Core/Src/UnMisc.cpp` | `appInit`, the `DISHONORED_PLAY_DEFAULTS` block only. The table's six entries are unchanged |
| `DishonoredGame/Src/disgfxmovieplayerhud.cpp` | `DisHUDBringUp` (two changes: the switch sense, and the sequencing guard) and the banner comment above `DisHUDLoadMovie`. Nothing in `Tick_Cinematic`, `StartHUD` or the letterbox path |
| `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | new: `DisFMReportMenu`, `DisFMSetArg`, `DisFMInvokeStep`, `DisFMRunStep`, `DisFMDriveMenu`; `DisGFxMoviePlayerMainMenuPreAdvance` gains **two call lines**; a forward declaration of `DisGFxMoviePlayerMainMenuFilterButtonInput` |
| `resources/tools/run_regression.py` | `CRITICAL_RE`, `NO_HUD`, `OPTIONS_SCRIPT`, `STAGES`, `LOAD_SENSITIVE`, `LOG_METRICS`, `stage_newgame`, `stage_options`, `main`; one token added to each of `stage_nullrhi` / `stage_d3d9` / `stage_inputtest` |
| `resources/docs/regression_baseline.json` | ten new rows, two new `_extra_args` keys, one `_about` paragraph |
| `README.md` | one table cell, 37 → 47 |

`resources/docs/STATUS.md` also says 37 in three places and is the coordinator's.

## 9. One note on the machine

`GFxValue` has a destructor that releases a managed value, and UE3's `TArray::Add` hands back raw
uninitialised memory. `TArray<GFxValue>` is therefore wrong — the first `ChangeType` releases garbage —
and `DisFMInvokeStep` uses a fixed `GFxValue Args[4]`. `SetStringW` stores the pointer rather than a
copy, so the parsed argument strings have to outlive the `Invoke`; they do, in the same scope. Both of
these are easy to get wrong and neither fails at compile time.

Every capture in this package was taken in-process with `-apshottime` and claimed by its own
`-apshotname` tag (`fmd5`, `fmd6`, `fmo8`, `fmbar`), and every size in this report was read off the
bitmap header rather than assumed. `grep -c FocusLost` is **0** on every run quoted above; no run was
discarded for a stolen foreground and none was averaged in without the check.

Two machine notes for whoever runs this next. **A killed game can keep its staged image locked for
longer than the next run waits**, and `stage_retail.py`'s `shutil.copy2` then raises `PermissionError`
and the whole probe is lost before it starts — that cost three runs here before
`build/agentFM/probe.py` grew a wait on the file. And **a timed-out run can leave an orphan**: one
`DishonoredGame_fm.exe` outlived its 280 s budget by more than ten minutes and had to be killed by
hand. `run_regression.py` has not shown either, because `build_and_smoke.py` reaps its child before
returning and the stages are strictly sequential, but an agent running two probes back to back will.
