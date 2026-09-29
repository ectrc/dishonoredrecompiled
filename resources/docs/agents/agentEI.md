# Agent EI — making YES do something: the id, the timer and the `FArkGameEvent` (2026-09-30)

Agent EG, handing the modal over:

> *"`UDisGlobalUIManager`, and now louder: the box is visible but has no way to answer. YES on the quit
> modal does not quit, and YES on the New Game confirmation does not start a mission."*

| | |
|---|---|
| Package | the message box's answer path: the id, the queue, the timer and the game event that carries the player's choice back into the movie that asked |
| Starts at | HEAD `84b0c9d`, snapshot worktree `build/agentEI_wt` |
| Files | **12**, every one of them under `source/Development/Src/DishonoredGame` (3 new). Nothing in `Engine`, `GFxUI` or `External/GFx3`, so this package cannot collide with agent EH's `GFxMovieRoot::ProcessInput` work |
| Regression | `37 ok, 0 failed, 0 skipped` on `build/agentEI_wtrel`, **built inside the harness** (§7) |
| Clean build | the worktree, build directory deleted first, every target, `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264** (§7) |

**`build/agentEI/newgame_modal.png`** is the New Game confirmation on screen and
**`build/agentEI/quit_no_back_to_menu.png`** is the main menu after NO was answered on the quit modal.
Both were reached with real `SendInput` key presses on the running game and both drive logs are read in
§6, because this wave three separate agents' measurements were corrupted by input that went somewhere
else.

## 1. Result

| Accept | State |
|---|---|
| YES on the New Game confirmation starts a mission | **the whole chain is done and measured, and the game leaves the menu and loads the Tower.** One real key press answers the box, the answer reaches `UDisGFxMoviePlayerMainMenu::OnNewGameConfirm(difficulty 1)`, which issues retail's own `ce ChangeLvl_StartNewGame`, and 0.65 s later the engine has committed the map change and constructed `l_tower_p.TheWorld:PersistentLevel.DisFog_0/4/5` and loaded `Bank_LVL_Tower_Intro_Music`; the scene census then reports **5017 primitives, 404 visible, 862 of 6506 draw lists drawn** and 0 `Critical error`. §2 and §3. **What is NOT delivered is the picture**: the player pawn is never moved into the streamed level, so the camera stays at the menu world's spawn and the frame is black. That is *identical on the untouched HEAD executable through `-newgame`* (§4, `build/agentEI/head_newgame_control.png`), it is one unported Kismet action, and it is named exactly in §8 hand-over 1 |
| YES on the quit modal quits | **done and measured**: `message box 1 answered with button 0` -> `OnQuitGameConfirm: 'exit'` -> `appRequestExit(0)`, 0.00 s apart. §3 |
| NO dismisses the modal and returns to the screen underneath | **done and measured on the quit modal**: `answered with button 1`, no `appRequestExit`, and `build/agentEI/quit_no_back_to_menu.png` is the main menu with QUIT GAME still the selection. On the New Game box the same key sequence answered 0 on three attempts; the reason is the content's own selection list and not the answer path, and it is §8 hand-over 2 |
| a before/after against the untouched HEAD executable through the same driver | §4 |
| `run_regression.py`, own build dir, built inside the harness, 37 checks | §7 |
| a clean full release build, build dir deleted first, `DISHONORED_LAYOUT_CHECKS=ON` | §7 |
| `rva_sweep.py` over every file touched | §7: **115 citations in the twelve files, 89 `ok-2013`, 26 `ok-2013-mid`, 0 mislabelled, 0 fabricated** |
| report + `agentEI_status.csv` | this file; 34 rows |

## 2. The chain, and the two halves of it that were missing

The path from "the content asks a question" to "the game does the thing" is eight links and retail
names every one of them. Two were absent.

```
AS2  _common.MessageBoxInvoke::InvokeMessageBox(callback, targetMc, msg, YES, NO)
       stores the callback on itself, then ExternalInterface.call('ShowMessageBox', msg, b0, b1, b2)
C++  UDisGFxMoviePlayerBase::execShowMessageBox            2013 0x600b40
     UDisGFxMoviePlayerBase::ShowMessageBox                     0x7a4550   <- was not there at all
       -> UDisGlobalUIManager::ShowMessageBox                   0x83dc00   <- nor this
       -> UDisGFxMoviePlayerGlobal::AddMessageBox               0x7aa0c0
       -> UDisGFxMoviePlayerGlobal::ShowMessageBox              0x7946b0   (agent EA had this body,
          in execShowMessageBox rather than on the class it belongs to)
AS2  the global movie's own root ShowMessageBox builds _level0.msgBox_mc.messageBox_mc

  ... the player answers ...

AS2  _common.MessageBox::APressed -> Close -> OnClosed
       ExternalInterface.call('OnMessageBoxConfirm', _selectedIndex)
C++  UDisGFxMoviePlayerGlobal::execOnMessageBoxConfirm      2013 0x5f7300   <- a DISHONORED_NATIVE_STUB
     UDisGFxMoviePlayerGlobal::OnMessageBoxConfirm               0x7abbd0   <- was not there
       pops the queue, and raises FArkGameEvent(31, {id, button}, this)
     UDisGFxMoviePlayerBase::OnMessageBoxResult                  0x793c70   <- was not there
       matches the id, invokes _root.MessageBoxInvoke.OnMessageBoxClosed(<button>) on the ASKING movie
AS2  MessageBoxInvoke.OnMessageBoxClosed -> the callback it stored
       NewGameMenu.OnNewGameConfirm(idx) -> Close(true) -> OnMenuClosed
       -> ExternalInterface.call('OnNewGameConfirm', _curSelectionIdx)
C++  UDisGFxMoviePlayerMainMenu::OnNewGameConfirm          2013 0x7c1ff0   (already ported, never called)
       -> m_Difficulty, m_bStartingNewGame, and the tweak string "ce ChangeLvl_StartNewGame"
```

### 2.1 The queue is a file static, not a member of the manager

The thing that made this package smaller than its brief expected: **retail keeps the message-box queue
and the id counter as file statics of `disgfxmovieplayerglobal.cpp`**, not as members of
`UDisGlobalUIManager`. `UDisGFxMoviePlayerGlobal::AddMessageBox` (0x7aa0c0),
`RemoveMessageBox` (0x7aa230), `AddMessageBoxTimer` (0x794400), `OnMessageBoxConfirm` (0x7abbd0) and
`FilterButtonInput` (0x78c750) all address `.data` rva `0x106e660` (the `TArray<FDisMsgBoxInfo>`, count
at `0x106e664`) and `0xf3aff4` (the counter) directly, with no `this`. The three
`UDisGlobalUIManager` entry points are one-line forwarders to `m_pGlobal`:

```c
// 0x83dc00, retn 8 -> TWO arguments in 2013, though the 2012 PDB name has one
INT UDisGlobalUIManager::ShowMessageBox( const FDisMsgBoxInfo& Info, UINT Priority )
{ m_pGlobal->AddMessageBox( Info, /*out*/Priority, Priority ); return Priority; }
```

The counter's initial value is **1**, read out of the retail `.data` section (`0xf3aff4` is loaded and
holds `0x00000001`), which is what lets `m_MsgBoxID == 0` mean "this movie has no box" — the test
`UDisGFxMoviePlayerMainMenu::BackToStartScreen` already made.

So `UDisGlobalUIManager` needed three small methods, not a class. **The brief's premise that
`UDisGlobalUIManager` "is not declared in this tree" is wrong** and has been wrong since agent BE:
`DishonoredGameUIClasses.h:1460` declares it with all 33 of its reflected members, `DisGetGlobalUIManager`
is ported in `dishonoredutilities_accessors.cpp`, and the class answers non-NULL on the menu path. Agent
EA's and EG's hand-overs 4 both repeat the older statement; they were describing a gap that had already
closed.

### 2.2 The one thing that makes the box answerable at all: focus

The box's own key handling is **the global movie's** `_root.inputs` — `_common.MessageBox::APressed`
and `BPressed` are members of a class in `UI_Global.Global`, and the bring-up opens that movie with
`bAllowFocus = FALSE, bAllowInput = FALSE, bCaptureInput = FALSE` so the cursor would not swallow the
menu's keys. A box drawn in a movie that cannot receive input can never be answered, whatever the C++
does, and that is why EG's box "had no way to answer" in the literal sense as well.

Retail's answer is two functions this package ports, and it is not a special case:

* **`UDisGFxMoviePlayerGlobal::RefreshMessageBoxFocus`, 2013 `0x79ec60`** — the global movie takes focus
  and input exactly while something needs it: a queued message box, or, while the save icon is up, any
  open `UDisGFxMoviePlayerBase` with `m_bLoseFocusWhileSaving`. It is called from
  **`UpdateMessageBoxAttributes` (`0x7a5040`)**, which every add, remove and confirm ends with.
* **`UDisGFxMoviePlayerBase::AllowFocus` (`0x7877b0`) and `AllowInput` (`0x79e820`)** — the bits, plus
  `FlushPlayerInput` when the movie gains what it did not have (so the key that raised the box is not
  delivered to the box as well) and `FGFxEngine::ReevaluateFocus`.

Measured, on the running game, the frame the box goes up:

```
[0017.00] message box 1 up: 'Do you want to quit the game?' [YES|NO|]
[0017.00] DishonoredGame native not ported: UDisGFxMoviePlayerBase::execOnFocusLost   <- the menu loses it
[0017.00] AS2 trace: > OnFocusGained                                                  <- the global movie has it
```

and six seconds later the key the player presses reaches the box.

### 2.3 What `OnMessageBoxConfirm` does with the id

`0x7abbd0` pops the head of the queue, clears the "a box is up" bit, and then compares the popped id
against **fourteen** module globals (`0x106754c` … `0x1067590`) — `UDisGlobalUIManager`'s own
login-change, controller-disconnected, storage-device, corrupt-save, DLC and autosave boxes — handling
each in place. Only when the id is none of those does it build `{id, button}` on its own stack and call
`FArkGameEventDispatcher::GetInstance()->ProcessEvent(FArkGameEvent(31, &params, this))`. Nothing in
this tree raises one of the fourteen, so every box that arrives takes the event arm; the fourteen are
named in the code rather than written.

The event id **31** is not inferred. It is the literal at three retail sites, and the clearest is
`UDisGFxMoviePlayerBase::BeginDestroy` (`0x7a45c0`), whose last statement is
`FArkGameEventDispatcher::GetInstance()->UnregisterToEvent(31, this, &UDisGFxMoviePlayerBase::OnMessageBoxResult)`
— which is also where the subscription's lifetime comes from, and is ported with it. The registration
is inside `ShowMessageBox` (`0x7a4550`) and is an unregister followed by a register, so a movie that
raises a second box is still on the list exactly once.

## 3. What a real key press now does, end to end

`build/agentEI/ngfinal_log.txt`, driven by `build/agentEI/drive.py` — `SendInput` key presses into the
window of **`dishonoredgame_ei.exe`** (§6), on the game's own startup (`-startmap=` suppresses the play
default so `DishonoredGameFull_P` stays the persistent world and streams the menu map in, which is
retail's own shape). Keys: `SPACE` (dismiss the start screen), `ENTER` (NEW GAME), `ENTER` (difficulty),
`ENTER` (continue from the brightness screen, which raises the box), `ENTER` (YES).

```
[0031.10] message box 1 up: 'Creating a new game will overwrite previous autosaves. Continue?' [YES|NO|]
[0041.29] message box 1 answered with button 0
[0041.29] message box 1 result (button 0) delivered to DisGFxAutoOpenMenu: _root.MessageBoxInvoke found
[0041.47] OnNewGameConfirm(difficulty 1): 'ce ChangeLvl_StartNewGame'
[0041.5x] ScriptWarning: ... DisFog l_tower_p.TheWorld:PersistentLevel.DisFog_0 / _4 / _5
[0041.82] Wwise: Loaded bank Bank_LVL_Tower_Intro_Music (id 0xfe1eb781)
[0042.12] Committed map change via DishonoredEngine        <- 0.65 s after the key
```

and the quit modal, `build/agentEI/quitfinal_log.txt`, keys `SPACE`, `RIGHT` x3 (to QUIT GAME), `ENTER`
(open the box), `ENTER` (YES):

```
[0037.77] message box 1 up: 'Do you want to quit the game?' [YES|NO|]
[0045.92] message box 1 answered with button 0
[0045.92] message box 1 result (button 0) delivered to DisGFxAutoOpenMenu: _root.MessageBoxInvoke found
[0045.92] OnQuitGameConfirm: 'exit'
[0045.92] appRequestExit(0)
[0046.55] Exit: Preparing to exit.
```

and NO, `build/agentEI/quit5_log.txt`, the same up to the box and then `RIGHT`, `ENTER`:

```
[0016.91] message box 1 up: 'Do you want to quit the game?' [YES|NO|]
[0026.02] message box 1 answered with button 1
[0026.02] message box 1 result (button 1) delivered to DisGFxAutoOpenMenu: _root.MessageBoxInvoke found
[0026.02] AS2 trace: ::Open undefined
[0026.23] AS2 trace: PlayOpenButtonAnim (0) / sel._curSelection : 3
```

No `appRequestExit`, the process ran on, and `build/agentEI/quit_no_back_to_menu.png` is the main menu
with QUIT GAME (index 3) still selected.

**One correction to how the box is answered on PC, because it decides what "NO" even means.**
`_common.MessageBox::APressed` is

```
if (_global.PlatformName == 'PC')  Close();                       // whatever sel already selected
else { _selectedIndex = 0; PlayControllerButtonAnim(); setInterval(Close, 250, this); }
```

so on PC the A key does **not** force YES: it confirms the box's own selection, which `LEFT`/`RIGHT`
move. `B` (`BPressed`, which would force index 1) is what a pad uses. Escape and Backspace were both
measured against the box and neither answers it, so NO from the keyboard is `RIGHT` then `ENTER`, which
is what §3's third block does.

## 4. Before and after, same driver, same keys

| | HEAD `84b0c9d` (`DishonoredGame_EIB.exe`) | this package (`DishonoredGame_EI.exe`) |
|---|---|---|
| the New Game confirmation appears | yes (agent EG's work) | yes |
| YES on it | **nothing at all.** `build/agentEI/base3_log.txt`: the box goes up at 23.81 s and the fifth `ENTER`, six seconds later, produces **no log line of any kind** - not even the `DISHONORED_NATIVE_STUB` of `execOnMessageBoxConfirm`, because at HEAD the key never reaches the box's own AS2 either (§2.2). The four keys before it did land, in that same process's log, which is what makes the fifth one's silence a measurement and not a lost key | `message box 1 answered with button 0` -> `OnNewGameConfirm` -> `Committed map change` into `l_tower_p` |
| YES on the quit modal | **not measured at HEAD, and I say so rather than assert it.** It is the same box in the same movie, and `UDisGFxMoviePlayerGlobal::execOnMessageBoxConfirm` was a `DISHONORED_NATIVE_STUB` at HEAD, so there was no body for the key to reach even if it had | `OnQuitGameConfirm: 'exit'` -> `appRequestExit(0)` |
| NO on the quit modal | same | `answered with button 1`, box gone, menu back |
| the frame once the mission map is committed | **black** (`build/agentEI/head_newgame_control.png`, the HEAD executable driven with `-newgame`, 180 s after the first drawn frame) | **black** (`build/agentEI/newgame_level.png`) |

The last row is the honest one: reaching the mission map is new, and the *picture* of it is not, because
the pawn is never moved into the streamed level. The control run makes that a property of the engine's
own `-newgame` path at HEAD rather than of this package — same map, same commit, same black frame,
reached without touching the interface at all.

## 5. Deviations, stated once

1. **`DisGetGlobalMoviePlayer()`** (`dishonoredutilities.h`, body in `disgfxmovieplayerglobal.cpp`) is a
   bring-up seam with no retail counterpart: retail says `DisGetGlobalUIManager()->m_pGlobal`. It is
   needed because the `-gfxuimenu` bring-up constructs the global movie player itself, outside the UI
   manager. **It tests the movie's view, not the pointer, and that was measured the hard way**: on the
   menu path `DisGetGlobalUIManager()` answers non-NULL with an `m_pGlobal` that owns no movie (it comes
   out of the game info's default sub-objects), and the class default object is a
   `UDisGFxMoviePlayerGlobal` too. A first version that trusted `m_pGlobal` printed *"the global movie
   is not open, so the box has nowhere to draw"* on every box.
2. **`UDisGFxMoviePlayerMainMenu::OnMessageBoxResult` (`0x7bc4d0`) is deliberately not overridden.** Its
   whole body is `if (id == m_MsgBoxID) Base::OnMessageBoxResult(e);`, which is the base's own first
   test, so the override is observationally identical and would cost a third generated-header include.
3. **`UDisGlobalUIManager::OnMovieAttributesChanged` (`0x84cf80`) and `UUIRoot::GetSceneClient()`'s
   vtable +336 are not called.** Both are the UI manager's movie-stack bookkeeping and neither is
   declared in this tree; the three call sites name them. What `UpdateMessageBoxAttributes` *does* write
   — `m_bBlurGameWhileActive`, `m_bDrawBlackStripesWhileActive`, `m_bShowHUDWhileActive`,
   `m_bPauseHUDWhileActive`, `bPauseGameWhileActive` — are real members and are written exactly as the
   retail bit arithmetic `(x & 0xFFFFFFE4) | 0x13` says.
4. **`UDisGFxMoviePlayerGlobal::FilterButtonInput` (`0x78c750`) is not ported.** Its whole body swallows
   button input while the save icon or the checkpoint text is up and no message box is queued; neither
   the save icon nor the checkpoint text exists in this tree, so the port would be a constant.
5. **The fourteen well-known message-box ids of `OnMessageBoxConfirm` are named, not written** (§2.3).
6. **Four one-line `debugf`s are new and permanent**: one per box raised, per box answered, per result
   delivered, and one each in `OnNewGameConfirm`/`OnQuitGameConfirm` naming the console command. They
   fire once per box, and without them this package's own measurements would have been guesses — the
   difference between "the box answered and the menu closed" and "the menu closed for some other reason"
   is exactly one of them.

## 6. The driver, and why its log is now read before anything is believed

Agent EH measured three separate ways for an agent's real input to land somewhere else this wave. Two of
them bit this package before EH's warning arrived, and the symptom was indistinguishable from a defect:
**runs `run2` and `run3` produced no response to any key, and `basectl` on the untouched HEAD executable
in between produced the full menu**, which reads exactly like "the package broke input". It did not. The
driver bound the window by its **title**, and every concurrent agent's build carries the same title.

`build/agentEI/drive.py` now binds by the process **image name** — `GetWindowThreadProcessId` ->
`QueryFullProcessImageNameW` — prints it for the window it chose and for the foreground window at every
step, and marks a step `*** NOT THE GAME WINDOW ***` when they differ. Every **after** measurement in
this report (`ngfinal`, `quitfinal`, `quit5`) was driven with it and its drive log shows
`dishonoredgame_ei.exe` on every step with no mark. The three **before** runs (`base3`, `basectl`,
`ctl_newgame`) predate it and were title-bound; each is sound for a different reason, which is that its
own process log shows the earlier keys of the same schedule landing, so the later one's silence is a
measurement of that process and not of a lost key. The driver also sends a dozen converging deltas per
mouse move instead of two identical ones, which is EH's `FlushMouseInput` finding.

A second, found while writing this report: **`DishonoredGame_EIB.exe`, the staged HEAD binary, stopped
being HEAD the moment a run was launched without `--no-stage`**, because `stage_retail.py` re-copies
whatever the build directory holds now. The run that did it (`quitbase`) reported HEAD quitting on YES,
which is impossible, and it is struck from this report; every "before" quoted here (`base3`, `basectl`,
`ctl_newgame`) was driven before that point, with the HEAD binary, and each is identifiable by the
absence of this package's own `message box ... up` line from its log. The same drive log that caught it
also flagged its last two steps `*** NOT THE GAME WINDOW ***`.

A third thing worth recording, because it wasted a build: **`appStrfind` only matches at a
non-alphanumeric boundary** (`UnMisc.cpp:1440`, the `!Alnum` guard). `-nostartmap` therefore does *not*
suppress `DISHONORED_PLAY_DEFAULTS`' `-startmap=Dishonored_MainMenu -startmapopen`, and a run meant to
test the streamed-menu configuration silently tested the opened-menu one instead. `-startmap=` does
suppress it, and that is what §3's runs pass.

## 7. Verification

* **Every build and every measurement is in the snapshot worktree `build/agentEI_wt`** (HEAD `84b0c9d`
  plus this package's 12 files and nothing else; `git -C build/agentEI_wt status --short` is exactly
  those 12). The generated `resources/docs/types` data was copied in so the worktree's own copy of the
  layout tools could run; those files are gitignored and the worktree's `git status` is unchanged by
  them.
* **Regression, built inside the harness**, from the worktree, so the six build-stage checks count:
  `python resources\tools\run_regression.py --build-dir build/agentEI_wtrel --exe-name
  DishonoredGame_EIR.exe --log-prefix EIR` -> **`37 ok, 0 failed, 0 skipped, 440s`** (`build/agentEI/regression1.log`,
  `build/agentEI_wt/build/agentEI_wtrel/regression/summary.txt`).
* **Clean full release build** of the worktree, build directory deleted first, every target, with
  `DISHONORED_LAYOUT_CHECKS=ON`: **995 edges, 0 errors, 0 `C4263`, 0 `C4264`**, all four targets linked (`DishonoredGame`, `CoreSmoke`,
  `EdgeAnimSmoke`, `LayoutProbe`). It is 995 and not agent EG's 994 because `disglobaluimanager.cpp`
  joined the build (`build/agentEI/buildclean.log`).
* **Addresses.** Every retail address in the twelve files and in `agentEI_status.csv` was resolved
  against `build/agentEI_ida/retail2013_agentEI.i64`, my own copy of `resources/docs/idb/retail2013_named.i64`,
  opened headlessly through `resources/tools/ida/run.py`. **No IDA MCP tool and no FModel tool was used.**
  `rva_sweep.py` over `source/Development/Src/DishonoredGame`, filtered to the twelve files:
  **115 citations, 89 `ok-2013`, 26 `ok-2013-mid`, 0 `MISLABELLED-2012`, 0 `UNKNOWN-CLAIMED-2013`.** All
  26 `ok-2013-mid` are pre-existing 2012 citations in comment blocks other agents wrote in
  `dishonoredutilities.h`; every address this package added is a 2013 function start. The module's one
  `MISLABELLED-2012` (`Inc/CppText/UDisAISubState.h:16`) is agent CG's and is untouched. The four `.data`
  addresses are written as rvas (`0x106e660`, `0x106e664`, `0xf3aff4`, `0x106754c`), not as the linear
  addresses the decompiler prints.
* **`rva_sweep.py` is not sufficient on its own, and this package proves it.** Every one of the 33
  addresses this package cites was additionally resolved by hand - `ida_funcs.get_func`, is the address
  the function's **start**, and what is its name (`build/agentEI/isfunc.py`). That caught two the sweep
  passes: a `(2012 0x63d2a0)` I had typed from the shape of its neighbours rather than read (the real one
  is `0x647c80`, `?execShowMessageBox@UDisGFxMoviePlayerBase@@`), and `0x7cd970` for
  `UDisSeqAct_SetPlayerTravelDestination::Activated`, which is the **2012** address and in retail lands
  64 bytes inside `UDisGFxMoviePlayerStore::~UDisGFxMoviePlayerStore`. Both are exactly the failure mode
  the sweep exists for and neither is in a bucket it reports, because both are inside *some* 2013
  function. The fix for the second is in hand-over 1; the rule is that "the sweep is clean" is a
  necessary check and not a sufficient one.
* **`gen_classes_header.py` regeneration is required, and it was run**: the two new
  `Inc/CppText/UDisGFxMoviePlayer{Base,Global}.h` are only included by the generated class bodies once the
  generator has seen them. It was run from the worktree with `SDK_SOURCE` pointed at the worktree (the
  wrapper is `build/agentEI/regen.py`), and **its entire diff against HEAD is the two
  `#include "CppText/..."` lines in `DishonoredGameUIClasses.h` and the removal of the
  `execOnMessageBoxConfirm` stub from `DishonoredGameNativeStubs.cpp`** — the latter driven by the new
  `DishonoredGameNativeStubs.ported.agentEI.txt`. The generator writes CRLF and this tree keeps LF, so the
  regenerated files were normalised afterwards; that is the only reason the diff is not literally three
  hunks.
* **`Sources.cmake` changed**: one line, `Src/disglobaluimanager.cpp` leaving the
  `DishonoredGame_EXCLUDE` list, because that unit is no longer comment-only. A
  `gen_classes_header.py --sdk --sources-cmake` run would produce the same single change.
* **Files outside my own module: none.** All twelve are under
  `source/Development/Src/DishonoredGame`. `Engine/Src/UnPlayer.cpp` carried a temporary one-shot input
  probe during the investigation (§6) and was reverted; `git status` shows it clean.
* No commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.

## 8. Hand-overs

1. **The player is never put into the level the map change loads.** This is now the single thing between
   the front end and a picture of Corvo in the Tower, and it is one function:
   **`UDisSeqAct_SetPlayerTravelDestination::Activated`, 2013 `0x78a0a0`**, which the `StartNewGame`
   sequence runs beside its `SeqAct_PrepareMapChange_1` / `SeqAct_CommitMapChange_1`. It writes two words onto
   `ADishonoredPlayerController::s_pInstance` (+1700 and +1704) and the current level's package name, and it is
   38 bytes in 2012 (`0x7cd970`) and **unmatched** in `match_2012_2013.csv`, so the retail address is the one
   read out of `??_7UDisSeqAct_SetPlayerTravelDestination@@6B@` (`0xd4ebe8`) slot **+372** - one slot past the
   2012 index, because retail's save five inserted `UObject::IsRefSaveable` at +368. Its unit
   `Src/dishonoredkismet.cpp` is still a comment-only skeleton in `DishonoredGame_EXCLUDE`. Measured:
   after the commit the persistent world is still `DishonoredGameFull_P`, the pawn is still the one
   possessed at `X=0 Y=500 Z=-200` in the menu world, the scene holds `l_tower_p`'s 5017 primitives and
   draws 862 draw lists of them, and the frame is black. **Identical on HEAD through `-newgame`**, so it
   is not this package's and it is not the interface's.
2. **NO on the New Game confirmation.** `RIGHT` then `ENTER` answers 1 on the quit box and 0 on the New
   Game box, three attempts, with the `RIGHT` between 0.3 s and 8 s ahead of the `ENTER`. The C++ passes
   whatever index the content sends, so what is unproven is `_common.MessageBox`'s own selection list on
   that screen — most likely its `sel._handlerEnabledState`, which `NewGameMenu` touches and the main
   menu does not. One `-gfxuiwatch=onSelectionUpdated` run should settle it.
3. **Agent EH's Options screen** (`> BPressed () called` with no `TransitionTo (MainMenuScreen)`) is
   **not** this package's: it is the same `B` that does not reach `_common.MessageBox::BPressed` from the
   keyboard (§3), i.e. the PC key that CLIK's input handler maps to `B` is not `Escape` and not
   `BackSpace`. Finding that one key fixes both, and it is a single measurement in
   `_common.InputsHandler` (Global movie), not a package.
4. **`UDisGlobalUIManager` is declared and is instantiated**, and three of its methods now exist. The
   rest of it — `Init` (2013 `0x8b8d10`), the movie set from `DisUI.ini`, `OnMovieAttributesChanged`
   (`0x84cf80`), the texture-package cache, `m_EquipmentIcons` — is still absent, and with it the whole
   idea that the UI manager *owns* the movie players. The bring-up's `-gfxuimenu` construction is what
   stands in; `DisGetGlobalMoviePlayer()` is the one seam between them and is the only line to delete
   when the manager opens its own movies.
5. **`execOnFocusLost` is a `DISHONORED_NATIVE_STUB`** and fires every time the box takes focus.
   Retail's `UDisGFxMoviePlayerBase::OnFocusLost` is 2013 `0x78c3d0` and is the mirror of
   `OnFocusGained` (already ported): `_root.UIBase.OnFocusLost()`. It is three lines and it is why the
   screen under the box behaves oddly (`>> TransitionTo (undefined)` right after a box goes up).

## 9. Files

Mine (12, three new), all under `source/Development/Src/DishonoredGame`:

* **new**: `Inc/CppText/UDisGFxMoviePlayerBase.h`, `Inc/CppText/UDisGFxMoviePlayerGlobal.h`,
  `DishonoredGameNativeStubs.ported.agentEI.txt`
* **edited**: `Inc/CppText/UDisGlobalUIManager.h`, `Inc/dishonoredutilities.h`,
  `Src/disgfxmovieplayerbase.cpp`, `Src/disgfxmovieplayerglobal.cpp`, `Src/disglobaluimanager.cpp`,
  `Src/disgfxmovieplayermainmenu.cpp`, `Sources.cmake`
* **regenerated**: `Inc/DishonoredGameUIClasses.h`, `Src/DishonoredGameNativeStubs.cpp`

`build/agentEI_sync.py` is the authoritative copy list and `--check` prints what differs.

Scratch, not repo tools: `build/agentEI/` — `drive.py` (the real-input driver, bound by image name),
`go.py`, `dec.py`, `disasm.py`, `xrefs.py`, `names.py`, `vt.py`, `gval.py`, `regen.py`, `bmp2png.py`,
the run logs `base1`..`base6`, `basectl`, `run1`..`run10`, `quit1`..`quit5`, `quitfinal`,
`ng_no`..`ng_no3`, `ngfinal`, `ctl_newgame`, the build logs and the screenshots; `build/agentEI_build.cmd`, `build/agentEI_run.py`,
`build/agentEI_sync.py`; snapshot worktree `build/agentEI_wt`; build directories
`build/agentEI_baserel` (the play build the runs are driven on), `build/agentEI_wt/build/agentEI_wtrel`
(the harness's own) and `build/agentEI_clean` (the clean gate build). IDA: **own copy only**, `build/agentEI_ida/retail2013_agentEI.i64`.
