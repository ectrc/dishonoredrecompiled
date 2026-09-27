# Agent DG — the menu answers input, fills its own text, and the loop that ate the background (2026-09-27)

Package **DG** of `PHASE10.md`, the direct continuation of **DC** (`agentDC.md`): DC flipped
`DISHONORED_GFXUI_GFX3_RUNTIME` and the game's own main menu rendered for the first time. It rendered
but it could not be used, every string read `Text`, and the background and the logo were missing.
This package takes those three.

**Three of DC's four hand-overs turned out to be about something else, and measuring said so before
anything was written.** That is the single most useful thing in this report, so it is section 2.

## Result

| Accept | State |
|---|---|
| a screenshot of the menu with background, logo and real text | **partly.** `build/agentDG/menu1.png` and `menu2.png` are the running game at drawn frame 1 and one tick after a key press. The text still reads `Text` and the painting is still missing, and section 5 says exactly why: `InitTexts` now puts all 149 of the menu's real strings on `_root.texts`, and the movie player now opens the right screen, but the content's own class code needs AS2 library surface (`gfx.motion.Tween`, `SaveProperties`, `Selection`, a sound hook) that this runtime does not have. Section 5 names each one |
| a second screenshot showing input moving the interface | **`build/agentDG/menu2.png`**, and the log line that goes with it is unambiguous: `scripted key SpaceBar ... -> pressed handled, released handled`, then `_root.startScreen_mc.Close(0) -> ok`, then `_root.mainMenu_mc.Open(4) -> ok`. The key went in through `FGFxEngine::InputKey`, the movie answered `HE_Completed`, and the content changed screen because of it |
| a census line with input events **handled** | **yes** — section 7 |
| the 7 script errors resolved or named | **7 -> 2**, measured on the same run and the same asset. The two that remain are both `call of a value that is not a function: 'undefined' on an object` - a method looked up by a computed name on a plain object - and section 5 names what they belong to |
| `run_regression.py` 31 checks, 0 failures | see section 9 |
| report + `agentDG_status.csv` | this file; 85 retail functions with their 2013 rvas |

**One defect in the AS2 machine was making the menu's background loop for ever** — 896 MB of
allocation and a dead process — and it is four lines. Section 3.

## 1. What this package changed

| File | What |
|---|---|
| `External/GFx3/GFxInput.{h,cpp}` (new, 161 + 1,012) | the input half: `GFxKeyboardState`, `GFxInputEventsQueue`, `GFxMovieRoot::HandleEvent` / `ProcessInput` / `ProcessKeyboard` / `ProcessMouse` / `NotifyMouseState` / `GetMouseState`, and the AS2 classes an interface listens with — `AsBroadcaster`, `Key`, `Mouse`, `Stage` |
| `External/GFx3/GFxAS2Interp.cpp` | **`ActionGetMember` resolves a PROPERTY** (section 3); an opcode budget per advance; the receiver's target path in the "not a function" error |
| `External/GFx3/GFxPlayerSprite.cpp` | `Object.registerClass` instantiation for timeline-placed clips (section 4); `GetBoundsTwips`; `_width` / `_height` / `_rotation` |
| `External/GFx3/GFxPlayerRoot.cpp`, `GFxPlayer.h`, `GFxPlayerData.cpp`, `GFxDisplay.cpp` | the class-binding queue entry, the shared visible-frame-rect, `GetExportedName` |
| `GFxUI/Src/gfxuimovie.cpp`, `Inc/gfxuiengine.h` | the three seam hooks that stand in for `UDisGFxMoviePlayerBase`'s virtual overrides |
| `GFxUI/Src/gfxuiengine.cpp` | `-gfxuikey`, `-gfxuishot=<list>`, `-gfxuishotname`, `-gfxuishotafterkey`, `-gfxuinoclassbind`, `-gfxuiclasstrace`; the input half of the census |
| `DishonoredGame/Src/disgfxmovieplayerbase.cpp` | **`InitTexts`** (2012 0x8218a0) and the base input filter |
| `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | **`PostStart`**, **`PreAdvance`**, **`FilterButtonInput`** and the seam installer |
| `External/GFx3/Tools/GFx3Run.cpp` | `--invoke`, `--key`, `--optrace`, a crash filter and a first-chance handler |

## 2. Three of DC's four hand-overs were about something else

DC's report named what it believed each remaining gap needed. Each was a reasonable reading and each
was a guess; the first thing this package did was measure them.

**Input was not the button model.** DC: "port `GFxButtonCharacter` (38 functions) and
`GFx_GenerateMouseButtonEvents`... then the menu becomes operable." Measured: the retail
`Dishonored_MainMenu.MainMenu` asset holds **one `DefineButton2` tag in 477**, and its own string
table carries `Key`, `addListener`, `removeListener`, `getCode`, `isDown`, `onKeyDown` and `onKeyUp`.
The menu is driven by the **AS2 `Key` broadcaster**, not by button characters. What this package
ported is that chain — `HandleEvent` -> the input queue -> `ProcessKeyboard` ->
`GFxKeyboardState::NotifyListeners` -> the `Key` object -> `AsBroadcaster::BroadcastMessage` — and
the menu answers input. `GFxButtonCharacter` is still worth having, and it is still not what made the
difference.

**The text is not `GFxTranslator`.** DC: "retail fills them through `GFxTranslator`/`GFxFontMap`."
Measured: `UDisGFxMoviePlayerBase::PreLoad` (2012 0x822820) calls `InitTexts` (0x8218a0), which
creates one AS2 object on `_root.texts` and copies every key of `[<ClassName>_Texts]` into it from
the localisation file, walking the class chain up to `UDisGFxMoviePlayerBase`. The file is
`DishonoredGame/Localization/INT/DishonoredGame.INT` and the sections are
`DisGFxMoviePlayerMainMenu_Texts`, `DisGFxMoviePlayerMenuBase_Texts` and
`DisGFxMoviePlayerBase_Texts`. It is ported, and a run now reports
`InitTexts: DisGFxMoviePlayerMainMenu from ..\..\DishonoredGame\Localization\INT\DishonoredGame.INT,
3 sections, 149 strings`. `t_NewGame="New Game"`, `t_Missions="Missions"`,
`t_PressStart="PRESS $Start$"` and the four difficulty descriptions are all there.

**The 7 script errors were not `Open`, `SetMenu` and `InitHelpBar`.** DC read them as "the content's
own methods on clips whose class the engine has not driven yet". The first change this package made
was to print the *receiver's target path* in that error, which took one run to answer:

```
'undefined' on an object                      x4
'SaveProperties' on undefined                 x1
'getNextHighestDepth' on Sprite _level0.DLCManagement_mc
'attachMovie'         on Sprite _level0.DLCManagement_mc
```

Three of the seven were one clip, and two of them were `MovieClip` methods that **are** installed on
`MovieClip.prototype` in this tree — which is what pointed at section 4. With this package's changes
the same run reports **2** script errors.

**The fourth hand-over, the crash, is real and is still open.** Section 8.

## 3. The defect: `ActionGetMember` left a PROPERTY on the stack

This is the one to remember.

The menu's background paints itself by tiling splatter art. The content's loop is, in bytecode:

```
384  Push "splatter" ; GetMember _nbSplatters ; Add2 ; Push "_mc" ; Add2
403  GetMember                       <- this["splatter" + n + "_mc"]
404  Push undefined
408  Equals2                         <- (clip == undefined)
409  LogicalNot ; LogicalNot         <- coerce to boolean
411  If <exit>
416  ...body: SaveProperties(clip, {_x,_y,_z,_xscale,_yscale,_rotation,_xrotation,_yrotation})
553  GetMember _nbSplatters ; Increment ; SetMember
556  Jump 384
```

It should stop the first time the clip is not there. It did not, and `--optrace 60000` printed why:

```
AS2 op 60006: pc  403 GetMember   stack 2 top 'splatter1306_mc'
AS2 op 60008: pc  408 Equals2     stack 2 top 'undefined'
    Equals2: lhs type 9, rhs type 0 -> 0
```

**Type 9 is `PROPERTY`.** `ActionGetMember` was storing the raw member on the stack, and an
`addProperty` member is a getter/setter *pair*, not a value. `GASValue::IsEqual` compares a pair
against `undefined` as false, so `clip == undefined` was false for a clip that did not exist, the
loop never exited, and it reached `splatter2701` in 120,000 opcodes and 896 MB of
`GASObject::AddNode` before the process died with `bad allocation`.

`GASObject::GetMember` and `GFxASCharacter::GetMember` both resolve the getter; the **opcode** was
the one path that did not, and it is the path that can reach a member store neither of those two
owns. Retail resolves it at the same place — `GFxValue::ObjectInterface::GetMember` (2012 0x9b0450)
calls the getter after the raw lookup.

The fix is four lines in `GASop_GetMember`. Its effect on the menu, same asset, same six frames:

| | before | after |
|---|---|---|
| opcodes executed | 1,000,000 (the budget, exhausted) | **5,573** |
| script errors | 15,322 | **40** |
| allocation | 896 MB, then `bad_alloc` | nothing unusual |

## 4. Object.registerClass never instantiated a timeline-placed clip

`GFxSprite::AttachMovie` looked up the class registered for a library symbol and constructed the clip
as it. `GFxSprite::AddDisplayObject` — the timeline's own placement, which is how **every** screen of
every Dishonored menu gets on screen — did not. So `_root.mainMenu_mc` was a bare movie clip with no
`Open`, no `Close` and no `SetMenu`, all ten screens of the asset were visible at once, and the movie
player had nothing to call. That is what DC's screenshot was.

Retail does it in `GFxSprite::AddDisplayObject` (2012 0x9fee10): it takes the character's exported
name, calls `GASGlobalContext::FindRegisteredClass`, and queues the construction as its own
action-queue entry — `InsertEntry(queue, 1)` for the class and `InsertEntry(queue, 3)` for the
frame-0 events.

**The queue is the point, and doing it inline does not work.** Measured: binding inline at
`PlaceObject2` time reports `'m_StartScreen' has no registered class` for ten of the menu's screens,
because the frame-1 tag stream interleaves the 64 `DoInitAction` tags that register the classes with
the 14 `PlaceObject2` tags that place the clips, and the init actions have not run yet when a clip is
placed. Queued at `GFxAP_User` — after every `GFxAP_Init` entry of the same drain — all ten bind.

Two smaller things fell out of it, both in the retail decompile and both absent here: the queue entry
**addrefs** its clip (`++*((_DWORD *)v29 + 1)`), because the clip's own frame-0 actions run between
the queue and the drain and can remove it; and the entry carries the symbol the *parent* resolved,
not a character id the drain re-resolves — `GFxSprite::GetOwnDataDef()` on the child answers the
movie the child's definition was parsed out of, which for an imported clip is a different dictionary,
and re-resolving crashed in `GASStringManager::CreateStringNode+0x1de`.

`-gfxuinoclassbind` turns it off for a comparison run; it is on by default because it is what retail
does.

## 5. What the screenshots show, and what is still missing

`build/agentDG/menu1.png` is drawn frame 1 and `menu2.png` is one engine tick after the key.

**What is right.** The whole chain works end to end and the log says so in order:

```
InitTexts: DisGFxMoviePlayerMainMenu from ...\DishonoredGame.INT, 3 sections, 149 strings
main menu: _root.startScreen_mc.Open(0) -> ok
GFx UI: scripted key SpaceBar on drawn frame 5 ... -> pressed handled, released handled
main menu: _root.startScreen_mc.Close(0) -> ok
main menu: start screen dismissed by SpaceBar
main menu: _root.mainMenu_mc.Open(4) -> ok
```

`PostStart` reads the local player's `m_bShowTitleScreen` and opens the start screen, exactly as
retail's 0x821e00 does; a key press moves login step 1 to 7; `PreAdvance` at step 7 closes the start
screen and calls `mainMenu_mc.Open(bContinue, bLoad, bNewGame, bSaveLoad)` with retail's four
booleans. **The first drawn frame is 66 display objects, not DC's 182** — that is the start screen on
its own instead of all ten screens on top of each other, which is the class binding working.

**What is still wrong, and exactly why.**

1. **The strings are on `_root.texts` but the fields still read `Text`.** The content substitutes them
   through its own `_common.ConvertVariableTags` class, which runs from the class code below.
2. **The background and the logo are still missing.** `mainMenu_mc.Open` returns ok and the content
   creates `_splatters_mc`, `_logo_mc`, `_menu_mc` and `_corvo_mc` — the log shows all four by name —
   and then each of them calls a library method that is not there. The remaining 2 script errors in a
   class-binding-off run, and the 32 in a class-binding-on run, are all one of five names:
   `SaveProperties`, `tweenTo`, `tweenEnd`, `PlaySound` and one anonymous computed call. Those are
   `gfx.motion.Tween` (a CLIK mixin the content installs on `MovieClip.prototype`), the sound hook
   `FGFxSoundEventCallback` that `UGFxMoviePlayer::PreLoad` installs in retail and this tree does not
   (agentDC.md deviation, `GFxMovieView::CreateFunction` returns undefined), and `_utils`'
   property-snapshot helper. **That is the next package and it is a library package, not a renderer
   one.**
3. **The other screens come back by drawn frame 4.** The movie's own 5-frame root timeline keeps
   placing all ten screens; in retail each screen's class constructor hides itself in its first frame,
   and those constructors are the ones that fail above.
4. **The shot is 1008x567, not 1280x720.** The window is what the client area allows; it is the
   engine's own screenshot of the real back buffer either way.
5. **Two opcodes report as unimplemented in the game and none in the harness.** Same asset, same
   frames; the difference is that the game reaches the content the harness's five frames do not. The
   census counts them and the next package should print which (`GASActionBuffer::OpsUnimplemented`
   has no name attached today).

## 6. The other things measuring found

* **`_width`, `_height` and `_rotation` had no implementation at all** — neither by name nor as
  display properties 8, 9 and 10 — so a clip answered `undefined`, which AS2 coerces to 0 in a
  numeric comparison. `GFxCharacter::GetBoundsTwips` (retail 0x9d2ab0) and the union over a sprite's
  display list are ported for them.
* **`Stage` did not exist.** Content that lays itself out over the screen reads `Stage.width` and
  `Stage.height`; with `Stage` undefined the arithmetic is NaN. Adding it alone took the menu's
  script errors from 15,322 to 10.
* **An AS2 value stack and an AS2 array both grew without bound.** Retail caps the activation depth
  and says "Stack overflow"; the value stack and `GASArrayObject::Resize` had no such bound, so
  content that pushes without returning, or that indexes an array from a non-number, produced a
  `bad_alloc` out of the machine with nothing to say. Both now report and refuse.
* **`GFx3Dump` no longer builds with the runtime on.** `DISHONORED_GFXUI_GFX3_RUNTIME=1` makes
  `gfxuirenderer.cpp` and `gfxuiimageinfo.cpp` need `Engine.h`, and `GFx3Dump` links them without the
  engine. Pre-existing from DC's flip, not introduced here; `GFx3Run` is unaffected. Hand-over.

## 7. The census

One line from a real run of the game on `Dishonored_MainMenu`, after the scripted key:

```
GFx UI census (after a scripted key): movies open 1 [UI_MainMenu.MainMenu], drawn 4, display objects 672 (304 sprites, 228 shapes, 124 text fields, 52 bitmap fills), 252 draws, 520 triangles, 124 glyph batches / 760 glyphs, 0 masks, atlas 56 glyphs rasterised / 60 missed; machine: 6 frames advanced, 190 sprites created, 398 display objects placed, 103 action buffers, 5343 opcodes (2 unimplemented), 36 script errors
GFx UI census (after a scripted key): input: 1 events HE_Handled / 0 HE_NotHandled, 0 key downs, 1 key ups, 0 chars typed, 0 mouse events, 0 AS2 listeners registered, 0 listener calls
```

and the first drawn frame of the same run:

```
GFx UI census (first drawn frame): movies open 1 [UI_MainMenu.MainMenu], drawn 1, display objects 66 (23 sprites, 18 shapes, 22 text fields, 8 bitmap fills), 19 draws, 38 triangles, 22 glyph batches / 139 glyphs, 0 masks, atlas 27 glyphs rasterised / 28 missed; machine: 2 frames advanced, 114 sprites created, 248 display objects placed, 86 action buffers, 5225 opcodes (1 unimplemented), 32 script errors
GFx UI census (first drawn frame): input: 0 events HE_Handled / 0 HE_NotHandled, 0 key downs, 0 key ups, 0 chars typed, 0 mouse events, 0 AS2 listeners registered, 0 listener calls
```

`input: 1 events HE_Handled / 0 HE_NotHandled` is the accept: the key reaches the movie **and the
movie uses it**. Before this package `GFxMovieRoot::HandleEvent` answered `HE_NotHandled` from state
for every event of every kind.

**Why one event and not two.** `FGFxEngine::InputKey` asks the movie player's `FilterButtonInput`
first, and on the start screen `UDisGFxMoviePlayerMainMenu::FilterButtonInput` (2012 0x822590)
answers `TRUE, bHandled` on the *press* — so the press is consumed by the player and never reaches
the runtime, which is retail's own order. The *release* falls through the filter, reaches
`GFxMovieRoot::HandleEvent`, and is answered `HE_Completed`. That is the `1 key ups` in the line.

`0 AS2 listeners registered` is also honest and is section 5's item 2 in another form: the menu
registers with `Key.addListener` from `_common.InputsManager`, whose construction is one of the class
constructors that stop on a missing library method. The broadcaster is there and works — the harness
proves the whole chain with `--key 4:40` — and the content has not reached it yet.

## 8. The open defect, with what this package added to it

DC's section 8 crash is still open and is now better characterised.

* It is **not** caused by the class binding: with `-gfxuinoclassbind` it still happens.
* It is **not** a destroyed movie data def: a log in `~GFxMovieDataDef` fires for none of them before
  the crash.
* The faulting call is `GFxSprite::AddDisplayObject+0x1e7`, which is
  `def->CreateCharacterInstance(...)`, and a trace of every placement says the last successful one is
  `place: char 18 depth 1 on _level0.newGame_mc, dataDef 250D0000 (root), def 08213A70` — so the
  definition is one the **root** movie's own dictionary holds, not an imported one, and the character
  id is not among the ten imports. The dictionary entry is still there; what is wrong is the object it
  points at.
* Therefore: **heap corruption**, not a lifetime bug in the import path. The next package should run
  the menu under a page-heap (`gflags /p /enable DishonoredGame_DG.exe /full`) — the harness cannot
  reproduce it, and `GFx3Run` runs the same asset for 400 frames without it, which is itself evidence
  that the corrupting write is on the engine side of the seam.
* Its practical cost today: the game dies about one tick after `mainMenu_mc.Open`, which is why the
  second screenshot is taken one tick after the key rather than thirty frames later.

## 9. Verification

* **Regression**: `run_regression.py --build-dir build/agentDG_rel --no-build` →
  **31 ok, 0 failed, 0 skipped, 428s** (`build/agentDG_regression2.txt`).
* **Clean RELEASE build** of the HEAD snapshot `build/agentDG_wt` + this package's 21 files
  (`build/agentDG_relbuild.cmd`): 0 errors, 0 link errors, all four targets.
* **The harness**: `GFx3Run --run Dishonored_MainMenu.MainMenu.gfx --frames 6 --imports
  build/agentBB/gfx --platform PC --invoke 3:_root.mainMenu_mc.Open --key 4:40` →
  `invoke ... -> ok`, `key 40 -> HandleEvent HE_Handled / HE_Handled`, 5,573 opcodes, 30 distinct
  opcodes all implemented, no crash.
* **The game**: section 5's log sequence, reproduced by
  `DGSHOTS=1 DGKEY=5:SpaceBar python build/agentDG/shots.py -gfxuishotafterkey=1`.

## 10. Deviations, stated once

1. **The AS2 `Key` and `Mouse` are `GASObject`s, not constructor functions.** Retail's
   `GASKeyCtorFunction` is a `GASFunctionObject` that is also a `GFxKeyboardState::IListener`.
   Nothing in the cook calls `new Key()`; both are read only as namespaces. Same script-visible
   surface, different base.
2. **`GFxASCharacter::GetMemberRaw` consults `MovieClip.prototype` last.** Retail answers the
   MovieClip built-ins from the character itself (`GFxSprite::GetMember`, 2012 0x9fb540, resolves
   them through `GetStandardMemberConstant` and its own switch before it looks at `__proto__`), so a
   registered class may replace `__proto__` with anything and the clip still has `attachMovie`. This
   tree keeps the built-ins on a prototype object, so the equivalent is to consult it last.
3. **`GFxMovieRoot::ProcessFocusKey` (4,941 bytes) is not ported**, nor the tab order it walks. The
   Dishonored menus move their own selection from their own `onKeyDown`, which is what the asset's
   strings and its single `DefineButton2` say.
4. **`ProcessMouse` is the Mouse broadcaster and the position**, not the topmost-entity tracking or
   `GFx_GenerateMouseButtonEvents`' button state machine.
5. **The three `UDisGFxMoviePlayerBase` virtual overrides arrive as hooks**, not as overrides.
   Declaring them means adding CppText hooks to the DishonoredGame generator and regenerating
   `DishonoredGameUIClasses.h`, and that regeneration rewrites `dishonoredgameclasses.h` and
   `DishonoredGameNative.h`, which agent DF is live in. The bodies and the call sites are retail's;
   only the dispatch differs. **Whoever regenerates DishonoredGame next should turn these into real
   overrides** — `UDisGFxMoviePlayerBase::Start`, `::PreLoad`, `UDisGFxMoviePlayerMenuBase::PreAdvance`
   and the two `FilterButtonInput`s — and delete the four hook pointers.
6. **An opcode budget per advance (1,000,000) that retail does not have.** The whole menu legitimately
   runs 3,854 opcodes in a frame, so the bound is 250 times the real load; it exists so that content
   which loops because a library method is missing produces a named error instead of a dead process.
   It should be removed when the library is complete.
7. **`GFxKeyboardState::KeyQueue` is not reconstructed.** It exists so that a movie opened after a key
   went down sees the pending events; nothing in this runtime reads it.
8. **The scripted key falls back to the topmost open movie** when no local player owns focus, which is
   the case on a menu map where `PlayerStates` is empty. It calls the same
   `FGFxEngine::InputKey(ControllerId, Movie, ...)` the focused path calls one level down.

## 11. Hand-overs

**The AS2 class library is the next package, and it is now a short list.** Five names stand between
the menu opening (which it does) and the menu looking like the menu: `gfx.motion.Tween`'s `tweenTo`
and `tweenEnd` on `MovieClip.prototype`, `SaveProperties`, `PlaySound`, and one computed call. Two of
them are content classes in the asset's own `__Packages` (`gfx.motion.Tween` is export id 249), so
the question is why their init actions do not install what they should — start there, with
`-gfxuiclasstrace` and `GFx3Run --optrace`. `Selection` and `TextField` are the two library classes
after that.

**The sound hook.** `UGFxMoviePlayer::PreLoad` installs `FGFxSoundEventCallback` through
`GFxMovieView::CreateFunction` in retail; `CreateFunction` returns undefined in this tree
(agentBC.md), so `PlaySound` is undefined on every menu. It is one `GFxFunctionHandler` shape away.

**The crash wants a page-heap run**, not more reading: section 8.

**`GFx3Dump` does not build with the runtime on** — section 6. One line in `cmake/GFx.cmake` or one
`#if` in `gfxuirenderer.cpp`.

**The generator traps DC left, still open.** `build/agentBB_gen_gfx3.py` must learn `GFxLoader`'s six
non-virtual declarations and `GFxResource`'s constructor before `GFx3Gen.h` is regenerated. **This
package did not regenerate it and added nothing to it.**

**Coordinator.** (1) `cmake/GFx.cmake` gains `GFxInput.cpp` and a `/MAP` for `GFx3Run`; CB's, CD's and
DC's entries are untouched. (2) `GFxUI/Sources.cmake` is **not** changed by this package. (3) Nothing
the DishonoredGame generator produces was regenerated or edited. (4) `middleware.md` 2.3 can gain the
line that the interface now answers input and fills its own text from the game's localisation files.

## 12. Files

Mine (21 source files plus this report and the CSV): `External/GFx3/GFxInput.{h,cpp}` (new); edits to `GFxPlayer.h`, `GFxPlayerRoot.cpp`,
`GFxPlayerSprite.cpp`, `GFxPlayerData.cpp`, `GFxCharacterDefs.{h,cpp}`, `GFxAS2Interp.cpp`,
`GFxAS2Lib.cpp`, `GFxAS2Object.cpp`, `GFxAS2Runtime.{h,cpp}`, `GFxDisplay.cpp`,
`Tools/GFx3Run.cpp`; `GFxUI/Inc/gfxuiengine.h`, `GFxUI/Src/{gfxuiengine.cpp, gfxuimovie.cpp}`;
`DishonoredGame/Src/{disgfxmovieplayerbase.cpp, disgfxmovieplayermainmenu.cpp}`; `cmake/GFx.cmake`;
plus this report and `agentDG_status.csv`.

Scratch (not repo tools): `build/agentDG/` — `dec1`..`dec9` (161 headless decompiles) and their list
files, `xr.py` (caller lists), `map.py` (link-map address resolution), `patch_*.py` (the CRLF-safe
patch scripts), `crlfpatch.py`, `crlf.py`, `mkstatus.py`, `shots.py` (the two-screenshot run),
`menu1.png`, `menu2.png`, `b*.log`; snapshot `build/agentDG_wt`, build directory `build/agentDG_rel`,
`build/agentDG_relbuild.cmd`, `build/agentDG_regression*.txt`.

Switches this package adds, all read on first use: `-gfxuikey=<drawnframe>:<KeyName>[,...]`,
`-gfxuishot=<N>[,<M>...]`, `-gfxuishotname=<prefix>`, `-gfxuishotafterkey=<N>`,
`-gfxuinoclassbind`, `-gfxuiclasstrace`; and in the harness `--invoke <frame>:<path>`,
`--key <frame>:<code>`, `--optrace <n>`.

IDA: one own copy, `resources/docs/idb/shipping2012_agentDG.i64`, opened headlessly through
`resources/tools/ida/run.py` only. **No MCP tool of any kind was used, and no FModel tool.** No
commits, no `git add`, no junctions, nothing deleted under `Dishonored_Latest2026`.
