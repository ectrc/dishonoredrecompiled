# Agent DQ — the mouse can see the menu, and the timeline stops fighting the script (2026-09-29)

The user, of the retail main menu:

> *"There is an issue with the main menu, when I try to hover and click over a button it does nothing I
> have to use the keyboard and then when interacting with the buttons the first time the white
> background is applied properly but any time re activating the hover state just breaks the hover
> effect"*

Two faults. Both reproduced with real Windows input before anything was written, both now fixed, and
both had a cause that no amount of reading would have produced.

| | |
|---|---|
| Package | the mouse's half of the GFx player, and the timeline-versus-script fight behind the hover |
| Starts at | HEAD `20acb5c` |
| Files | 12 (one new), all inside `External/GFx3`, the `GFxUI` module and `cmake/GFx.cmake` |
| Regression | **`31 ok, 0 failed, 0 skipped`** on the clean gate build `build/agentDQ_wtrel` |
| Clean build | the snapshot worktree `build/agentDQ_wt` (HEAD + this package), build directory deleted first: **992 edges, 0 errors** |

**`build/agentDQ/hoverstrip_gate.png`** is the answer to the second fault in one picture: five
consecutive hover states of the menu bar on the clean gate build, driven by a real mouse —
`NEW GAME -> QUIT GAME -> OPTIONS -> MISSIONS -> NEW GAME` — each with exactly one white background,
masked to its own label, the game's own cursor sitting on it, and the fifth identical to the first.
`hoverstrip.png` is the same thing over seven states on the working-tree build.

## Result

| Accept | State |
|---|---|
| hovering an entry highlights it, clicking it activates it, proven with real Windows input | **done.** `build/agentDQ/hoverstrip_gate.png` and its five `crop_gate_dqhover*.png`. The activation is logged: a real left click on NEW GAME produced `AS2 trace: OnMouseUp (0, undefined)` -> `Click (1) in SelectionHandler` -> `> APressed () called` -> `>> TransitionTo (NewGameScreen)` -> `> fscommand (ToNewGameScreen)`, and the New Game screen opened (`PlayOpenButtonAnim (1..3)`, `OpenDescription`). A click on **QUIT GAME** quits the game, which is how the user found §6's hand-over |
| a screenshot of a hover on an entry that is not the default | **`build/agentDQ/crop_gate_dqhover3_sel200002.png`** (OPTIONS) and `crop_gate_dqhover2_sel300001.png` (QUIT GAME). The default is NEW GAME |
| the highlight is correct on every activation, across at least four hover changes | **four mouse-driven changes on the gate build** (`0 -> 3 -> 2 -> 1 -> 0`) and **six on the working-tree build** (`0 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0`), each plus the opening state. The per-entry census at each screenshot of both runs is in §3 and it is the same four lines every time: the selected entry `frame 13/84 scale (1.100,1.100) bkgdOver a 100 txtGlow a 100 w 256`, the other three `frame 37/84 scale (1.000,1.000) bkgdOver a 0 txtGlow a 0 w 256`. Before this package the same census read `scale (2.500,2.500)` and widths creeping `256 -> 280 -> 299 -> 300` |
| a census line: mouse events delivered, targets resolved, rollover/rollout dispatched, script errors | **§3.** Gate build: `mouse: 726 hit tests / 131 targets resolved (1305 shape walks, 131 button hits, 0 sprite hits), 4 rollOver / 3 rollOut, 1 press / 1 release, 5 handlers invoked`; `82 script errors`, broken down in §2 |
| `run_regression.py` 31 checks, 0 failures, own `--build-dir` | **`31 ok, 0 failed, 0 skipped`**, §7 |
| clean full release build | **992 edges, 0 errors**, build directory deleted first, §7 |
| report + `agentDQ_status.csv` | this file; 56 rows, `rva_2013,status,note` |

**Two addresses in the brief are the 2012 build's.** `GFx_GenerateMouseButtonEvents` is **`0xa5ad10`**
in 2013, not `0xa66a90`; `GFxMovieRoot::ProcessMouse` is **`0xa05330`**, not `0xa0e900`. Both were
resolved against `retail2013_named.i64` before anything was ported, as `PHASE10.md` requires.

## 1. Fault 1, and the thing the asset settles

Agent DG measured that the main menu is navigated from the AS2 `Key` broadcaster and not from button
characters — one `DefineButton2` in 477 tags — and concluded that `GFxButtonCharacter` was not what made
the menu operable (agentDG.md 2). That is true, **of the keyboard**. The mouse is the other half, and the
asset says what it needs. The menu's one DefineButton2 is character 100, and it carries two records:

```
record 0  states 0x07 [up|over|down]  char 98   identity matrix
record 1  states 0x08 [hitTest]       char 99   identity matrix      <- a 62x62 filled DefineShape
trackAsMenu 0, and 0 bytes of condition actions
```

and `_common.SelectionHandler`'s element loop (`mm_init.txt`, char 257 `__Packages._common.SelectionHandler`, pc 669 and 728 of its constructor) does

```
elem.btn.onRollOver = function () { SelectionHandlerInstance.RollOver(this._parent.idx) }
elem.btn.onRelease  = function () { SelectionHandlerInstance.Click   (this._parent.idx) }
```

`elem.btn` is that button, one inside every `MainMenuButton`, at depth 19. **So every menu entry's hit
area IS that single DefineButton2's hitTest record, and every entry's handlers live on the button
instance.** The one button in 477 tags is not evidence that the button model is unnecessary; it is the
whole mouse surface of the menu, used four times over.

Four things stood between a delivered click and an activated entry, and each was found by measuring
rather than by reading.

### 1.1 The interface's mouse position never left (0, 0)

Agent DM routed the mouse: `UGameViewportClient::InputAxis` -> `DishonoredGFxInputAxis` ->
`FGFxEngine::InputAxis` (2013 `0x594cc0`), which reads the position **whole** from
`HudViewport->GetMousePos`. The bring-up fallback DM wrote for the key — "when no local player owns
focus, offer it to the topmost open movie that can take focus and input", which a menu map needs because
`PlayerStates` is empty — went **inside `FGFxEngine::InputKey` and nowhere else**. `InputAxis` and
`InputChar` still asked `GetFocusedMovieFromControllerID`. The probe this package added says what that
cost, over one run with a real mouse:

```
axis probe: 868 calls, 867 no focus, 0 filtered, 0 gamepad/no viewport, 0 delivered;
            last key 'MouseX' delta 5.00 gamepad 0, HudViewport 08F0E404
```

**867 of 868 mouse-axis events returned at the function's first line.** `FGFxEngine::MousePos` stayed
(0, 0) for the whole session, so every click the mouse arm of `InputKey` sent was sent at the top-left
corner of the movie — which is why agent DM's click "was delivered" and activated nothing, and why the
trace read `OnMouseUp (0, undefined)` with an undefined target.

The fallback is now one function, `FGFxEngine::GetInputMovieFromControllerID`, and all three entry
points use it. It is still bring-up and still goes when the script `InitInputSystem` inserts a
`UGFxInteraction` per local player (agentDM.md deviation 10). With it: `293 calls, 0 no focus, 292
delivered`.

### 1.2 There was no notion of what the pointer was over

`GFxMovieRoot::ProcessMouse` broadcast the `Mouse` class's four notifications and stopped, which is what
agentDG.md deviation 4 and agentDM.md hand-over 2 both named. Retail's body (2013 **`0xa05330`**) does
five things in order and only the fourth existed here:

```
1  GFxMouseState::UpdateState          0x9fb950   fold the queue entry into the per-mouse state
2  GFxMovieRoot::GetTopMostEntity      0x9fabf0   who is under the pointer now
3  GFxMouseState::SetTopmostEntity     0x9fc460   remember it, and what it was
4  the Mouse broadcaster's four notifications     <- all this tree had
5  GFx_GenerateMouseButtonEvents       0xa5ad10   rollOver / rollOut / press / release / drag*
```

The walk under step 2 is `GFxSprite::GetTopMostMouseEntity` (`0x9f0e80`) descending the display list
top-down, each level transforming the point by its own matrix's inverse, respecting the clipDepth masks
through `CalcDisplayListHitTestMaskArray` (`0x9f0d70`), and bottoming out in a point test: a shape's
winding walk (`GFxShapeBase::PointInShape` `0xa38890`), a text field's formatted view rectangle
(`0xa23d60`), or a button's hitTest records (`0xa5c310`). A sprite that `ActsAsButton` (`0x9ecc70`) —
one carrying any of the seven button handlers on itself or up its prototype chain — claims the hit for
itself; anything else passes it up.

Step 5 is the state machine. `GFx_GenerateMouseButtonEvents` (2013 **`0xa5ad10`**) is one loop over the
changed button bits and then one final rollOver / rollOut pair:

```
a bit that went DOWN             -> Press on the active entity, the pointer counts as inside
a bit that went UP, inside       -> Release
a bit that went UP, outside      -> ReleaseOutside
a bit still down, entity same    -> DragOver, ++RollOverCnt
a bit still down, entity moved   -> DragOut,  --RollOverCnt
no bit down and the entity moved -> RollOut on the old (--cnt), RollOver on the new (++cnt)
```

**The event ids are measured, not taken from the spec.** `GFxEventId::GetFunctionNameBuiltinType`
(`0x9d5e80`) is `log2(Id)` into a 0x23-entry table at `dword_13B3F38`, and reading that table out of the
image gives the seven button events `0x400..0x10000` a contiguous run of builtin names **91..97** in SWF
ClipActionRecord bit order — which is what settles `rollOver = 0x2000`, `rollOut = 0x4000`,
`press = 0x400`, `release = 0x800`, `releaseOutside = 0x1000`, and agrees with what the algorithm does
with each of them.

### 1.3 A DefineButton2 instantiated as a character that cannot be hit

`GFxButtonCharacterDef::CreateCharacterInstance` made a `GFxGenericCharacter`, and a generic character
answers **its definition's** point test (`0x9c4980`). A button definition has none — in retail the
records are walked by `GFxButtonCharacter::PointTestLocal` (`0xa5c310`), which is a property of the
*instance*, not of the definition. So every DefineButton2 in the game was invisible to the mouse, and
the one that matters is the hit area of all four menu entries.

There is a `GFxButtonCharacter` now. It is the two hit-test virtuals and a bounds override, not the
38-function state machine: this cook's button has its up, over and down states on the **same** record
and **no condition actions at all**, so what is still missing is not reachable from the main menu.

### 1.4 The button reported empty bounds, so `btn._width` was a no-op

The display tree said so and three packages had read past it:

```
dl  Button 'btn' depth 19 vis 1 a 100  at (640,478) scale (1.04,2.49)  box (0,0)-(0,0)
```

`GFxCharacterDefGetBoundsTwips` has no case for `RT_ButtonDef`, so a button answered an empty rectangle,
and the geometry setters derive their factor from the character's own bounds. `MainMenuButton::SetText`
writes `btn._width = txt._width` and `btn._x = btn._width / 2`; **both were lost**, and the hit area
stayed the authored 62x62 square instead of following the label. `GFxButtonCharacter::GetBoundsTwips` is
now the union of the records' bounds through each record's matrix.

## 2. Fault 2 — the timeline and the script were both writing the matrix

This one was unmeasured, and the candidates in the brief — the tween library, the mask, the two movies
sharing one AS2 registration, `GeomData` — were all wrong. `-gfxuibar`, which this package added, reports
each `btn<N>` under `_btnContainer_mc` with its timeline frame, its local scale, its own width, and the
alpha, frame and width of `_bkgdOver_mc` (the white background) and of the `_maskBkgdOver_mc` that clips
it. Over five real mouse hovers, before the fix:

```
bar btn0 frame 13/84 scale (1.100,1.100) w 256   <- the first activation, correct
bar btn0 frame  8/84 scale (2.500,2.500) w 256   <- and then
bar btn0 frame 11/84 scale (2.631,2.631) w 256
bar btn1 frame  0/84 scale (2.583,2.583) w 256
bar btn1 frame 28/84 scale (1.095,1.095) w 259
bar btn1 frame 32/84 scale (1.000,1.000) w 299
bar btn1 frame 37/84 scale (1.000,1.000) w 300
```

The entry balloons to two and a half times its size and its width creeps 256 -> 280 -> 299 -> 300 px.
The white background is applied correctly every time — the `bkgdOver a` column is right throughout — and
what "breaks the hover effect" is the geometry compounding around it. **The first activation is correct
because it is the first: there is nothing yet to compound on.**

The mechanism is two retail bodies.

**`GFxASCharacter::SetStandardMember` (2013 `0x9c9bf0`)** calls the slot-32 virtual with FALSE in nine of
its cases — 0 `_x`, 1 `_y`, 2 `_xscale`, 3 `_yscale`, 6 `_alpha`, 8 `_width`, 9 `_height`, 10
`_rotation` and 25 `filters` — and that virtual is `GFxASCharacter::SetAcceptAnimMoves` (`0x9c5cd0`),
whose first line is `EnsureGeomDataCreated`.

**`GFxDisplayList::MoveDisplayObject` (2013 `0x9cca50`)** clears the entry's marked-for-removal bit and
then:

```c
if ( !entry->vt28() )                             // GetContinueAnimation
{
    if ( !entry->GetAcceptAnimMoves() ) return;   // <-- and it touches NOTHING
    entry->SetAcceptAnimMoves(true);
}
if ( flags & 8 ) copy the colour transform
if ( flags & 4 ) copy the matrix
```

So in retail **a clip whose transform script has written is no longer moved by its own timeline.** That
is Flash's documented rule, and it is the whole reason a `GeomData` record can be a cache at all:
`GetGeomData` (`0x9c53e0`) returns the stored record verbatim, with no check that the matrix still
matches it, because nothing else is allowed to change the matrix any more.

This tree wrote the matrix and the colour transform from every move tag unconditionally. Each of the
menu button's 84 roll-over animation frames re-applied the authored matrix under the script's feet, and
the next `_xscale` or `_width` write re-derived its factor from a `GeomData` record that no longer
described the live matrix. Agent DJ predicted the symptom — "compounds on repeated writes if `GeomData`
is missing" — and `GeomData` was not missing. The detach was.

Both halves are ported. Same run, after:

```
bar btn0 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 ... | txtGlow a 100
bar btn1 frame 37/84 scale (1.000,1.000) w 256 | bkgdOver a   0 ... | txtGlow a   0
bar btn2 frame 37/84 scale (1.000,1.000) w 256 | bkgdOver a   0 ... | txtGlow a   0
bar btn3 frame 37/84 scale (1.000,1.000) w 256 | bkgdOver a   0 ... | txtGlow a   0
```

**None of the brief's four candidates was it**, and each was checked rather than dismissed.

* **The tween library is there.** Agent DJ's `ActionExtends` fix restored the `gfx.motion.Tween` mixin
  and `tweenTo` resolves: the opcode watch catches it reading and writing `_xscale` at pc 1509/1519 of a
  1774-byte buffer, which is `__Packages.gfx.motion.Tween` (char 249, len 1774). What is still missing is
  **`tweenEnd`**, called 4 times in a 115 s run — a residual, not the cause; agent DM's "~30 s to fade
  in" and agent DG's five-name list predate DJ.
* **The mask is right.** `mask clip 5 w 80 vis 1` in every line of every census, before and after.
* **The two open movies cost script errors, not geometry.** The 82 errors of the acceptance run break
  down as **54 `loadBitmap`** (`_common.EmbedImg`, agent DJ's hand-over 3), **14 `GotoLabeledFrame: no
  frame named 'PC'`** (the gamepad glyph, agent DK's hand-over 6), 4 `tweenEnd`, and 9 of the AS2
  drawing API (`lineTo`, `moveTo`, `beginBitmapFill`, `endFill`, `new Matrix`, `getTextFormat`,
  `getTextExtent`). None of them is in the hover path. The count is higher than agent DM's 71 because
  the menu now runs for two minutes and reaches screens it never used to.
* **`GeomData` was ported and correct.** It is a cache whose invalidation retail does not do, because
  retail does not need to.

## 3. The census, at each of the seven hover states

Two runs, real mouse only, `-gfxuibar -gfxuicensus -gfxuishotonhover=200`, pointer moves 10 s apart and
then a click. Each screenshot carries the four lines the bar was in when it was taken. The working-tree
run's seven states, selected line of each, and it is the same line seven times:

```
dqhover1  bar btn0 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover2  bar btn1 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover3  bar btn2 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover4  bar btn3 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover5  bar btn2 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover6  bar btn1 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
dqhover7  bar btn0 frame 13/84 scale (1.100,1.100) w 256 | bkgdOver a 100 frame 1 vis 1 w 256 | mask clip 5 w 80 vis 1 | txtGlow a 100
```

with the three unselected entries at `frame 37/84 scale (1.000,1.000) w 256 | bkgdOver a 0 | txtGlow a 0`
in every one, and the bar's own number agreeing:

```
bar selection 0 -> 1 -> 2 -> 3 -> 2 -> 1 -> 0
```

which is `_root.mainMenu_mc._menu_mc.sel._curSelection`, the value `_common.SelectionHandler` decides
with, not a guess from which background is brightest. The clean gate build's run is the same picture
over five states, `bar selection 0 -> 3 -> 2 -> 1 -> 0`, with the identical selected line at each
(`build/agentDQ/hoverstrip_gate.png`). The working-tree run's census:

```
GFx UI census (frame): mouse: 314 hit tests / 312 targets resolved (632 shape walks, 312 button hits,
  0 sprite hits), 6 rollOver / 5 rollOut, 1 press / 1 release, 7 handlers invoked
GFx UI census (frame): input: 315 events HE_Handled / 0 HE_NotHandled, 0 key downs, 1 key ups,
  0 chars typed, 314 mouse events, 5 AS2 listeners registered, 2 listener calls
GFx UI census (frame): machine: 51455 frames advanced, 1230 sprites created, 2586 display objects
  placed, 2626 action buffers, 41930466 opcodes (0 unimplemented), 82 script errors
GFx UI census (frame): movies open 2 [UI_MainMenu.MainMenu, UI_Global.Global] ... 0 masks,
  atlas 125 packed / 8 blank / 0 failed
```

**0 `Critical:` lines** in the run. Against HEAD, whose numbers are agent DM's: mouse events delivered
but `0` of anything else — no hit test, no target, no rollOver, no press, and a click that reached
`OnMouseUp` with nothing under it.

| | HEAD | after |
|---|---|---|
| mouse-axis events that reach the interface | 1 of 868 | 292 of 293 |
| hit tests / targets resolved | 0 / 0 | 314 / 312 |
| rollOver / rollOut dispatched | 0 / 0 | 6 / 5 |
| press / release dispatched | 0 / 0 | 1 / 1 |
| clicking a menu entry | `OnMouseUp (0, undefined)`, nothing | `Click (1)` -> `APressed` -> `TransitionTo` |
| the selected entry's local scale over 5 hovers | 1.100, **2.500, 2.631, 2.583**, 1.095 | 1.100 every time |
| the selected entry's width over 5 hovers | 256, 256, 259, **299, 300** | 256 every time |
| script errors | 71-82 | 82 |

## 4. Three defects of my own, all found by measuring or by re-reading the retail body

1. **`GFxCharacter`'s new members were never initialised.** `pWeakProxy`, `RollOverCnt` and the last-hit
   cache were added to the class without touching its constructor, so every character started with a
   garbage weak-proxy pointer, and `~GFxCharacter` never told the proxy its object was gone — which is
   the whole reason the indirection exists. The symptom was one rollOver in a run that should have had
   five. Fifth defect in this tree's refcount/lifetime family (agentDH.md), and this one was mine.
2. **The first `-gfxuibar` reported the entry with the brightest background, and during a crossfade two
   are over half**, so one pointer move produced four "selection changes" in 0.6 s. It reads the
   content's own `_curSelection` now. A census that measures a proxy for the thing rather than the thing
   is worth exactly as much as the proxy.
3. **The sprite walk's button arm had a test retail does not have**, and it passed the point in the
   parent's space to a function whose contract is the child's. Retail's condition is `v18 != 0` alone,
   and it is sufficient because a plain child answers the walk by returning the nearest ancestor that
   acts as a button — which is `this`. Found by reading the retail body again beside the written one
   rather than by a failing measurement, which is the only way this class of thing is ever found. The
   same pass gave `GFx_GenerateMouseButtonEvents` the AddRef retail holds on both entities for the
   length of the body, since every `ExecuteEvent` in it runs content that can remove the clip it is
   called on.

## 5. Deviations, stated once

1. **`GFxMovieRoot::ProcessMouse` drops four arms of retail's body**, each because nothing in this cook
   reaches it: the IME manager's `OnMouseDown` (no IME state is set), the focus move a press makes
   (`GFxMovieRoot::QueueSetFocusTo` — this tree has no focus model, agentDG.md deviation 3), the
   per-level `vtable+160` event (one level), and `ChangeMouseCursorType` (`0xa04490`), which is the hand
   cursor; Dishonored's cursor is a movie clip the global movie attaches (agentDM.md 2.2).
2. **`GFxMovieRoot::GetTopMostEntity` walks `_level0` only**, not retail's movie-level array, because
   this tree has one level; and it does not fill the normalised point the 3D path reads, nor walk the
   "topmost level characters" array at movieroot+9320, which nothing in this cook registers into.
3. **Every hit-test body is the 2D arm.** `GFxCharacter::Is3D` / `GetPerspective3D` / `GetView3D` /
   `GScreenToWorld` have no equivalent here and no character in the menu is 3D.
4. **`PointInShape`'s stroker half is not reproduced.** Retail builds a `GFxRenderGenStroker`, generates
   the stroked outline of each line-styled path and tests that with `GCompoundShape::PointInShape`
   (`0xa50410`). It only matters for a shape whose hit area is its outline rather than its fill; the
   menu's hit shape (char 99) is a filled rectangle.
5. **`GFxASCharacter::ExecuteEvent(GFxEventId)` runs the member handler and not the ClipActions
   handlers.** This tree does not store a PlaceObject2 `ClipActions` list, so a clip event authored on
   the timeline rather than assigned from script is not reached — measured, the main menu authors none.
   Retail also invokes with no arguments unless the global context's extended-clip-event flag is set
   (`*(pGC + 684) == 1`), which this cook does not set, and which is also what makes
   `GFx_GenerateMouseButtonEvents`' button count 1 rather than 16.
6. **`GASMovieClipObject::ActsAsButton` walks the member store by name.** Retail keeps a per-object
   bitmask of which handler names have ever been assigned; the walk gives the same answer for the same
   reason and costs one hash lookup per name per level.
7. **`GFxSprite::GetMask` (`0x9ea2c0`) and `GetHitArea` (`0x9ec030`) are not reproduced**, because this
   tree has neither `MovieClip.setMask` nor `MovieClip.hitArea`. The clipDepth masks, which is how every
   mask in this cook is authored, are respected through `CalcDisplayListHitTestMaskArray`.
8. **`GFxDisplayList::MoveDisplayObject`'s guard is unconditional.** Retail guards it with the slot-28
   virtual (`GetContinueAnimation`), which a re-placed character sets so the timeline may adopt it
   again. Nothing in this tree sets it, so the conservative arm — always respect `AcceptAnimMoves` — is
   the one reproduced, and it is named here because it is the one place this package changes behaviour
   for every clip in the game rather than for the menu.
9. **`GFxButtonCharacter` is the two hit-test virtuals, the bounds and nothing else.** The up / over /
   down state machine, `GFxButtonCharacter::SetStandardMember` (`0xa5d770`) and the DefineButton2
   condition actions are still the rest of the 38-function package. This cook's button has one record
   for all three visible states and zero condition actions.
10. **The mouse-index-1 event ids** (`0x80000`..`0x800000`, which retail gives five builtin names of
    their own) map to the same handler names here. Nothing in this cook has a second mouse.
11. **`GFxEventId::GetFunctionName` inlines the table** rather than reading it through the string
    manager's builtin array, because this tree's builtin enum is its own. The table was read out of the
    retail image rather than assumed (§1.2).
12. **`GetInputMovieFromControllerID` is bring-up**, not a retail function; see §1.1.

## 6. Hand-overs

1. **The interface's teardown faults, and it is not this package's - but this package is what made it
   visible.** The user found it by clicking QUIT GAME: the whole chain runs correctly —
   `appRequestExit(1)`, `Exit: Game engine shut down`, `Exit: Windows client shut down` — and then a
   `Critical error` with no `appError` message and a 13-frame stack in the exe, **after** both shutdown
   lines, so the game has stopped. Three runs settle whose it is, each closing the window with
   `WM_CLOSE`, which is the same `appRequestExit(FALSE)` that QUIT GAME reaches:

   | binary | switches | menu touched | `Critical error` |
   |---|---|---|---|
   | this package | `-gfxuimenu` | no | **1** |
   | this package | no `-gfxuimenu` | n/a | **0** — `Exit: Object subsystem successfully closed. Exit: Exiting.` |
   | **HEAD `20acb5c`** | `-gfxuimenu` | no | **1** |

   So it is the **GFx runtime's teardown**, it is present at HEAD, and none of this package's code needs
   to have run for it to happen. What this package changed is that a player can now reach it, because
   until now nothing could click QUIT GAME. Agent DJ's deviation 3 is the first place to look:
   `GASStringManager`'s destructor leaks a node something still holds, because a movie definition's
   action buffers keep their constant pool as `GASString`s and the loader outlives the movie root. Note
   also that `FEngineLoop::Exit`'s `delete GGFxEngine` is inside `#if WITH_GFx`, which is 0 in this tree
   (the same `#if` agent DK found `AutoGenerateNamesGFxUI` behind), so the engine is never torn down at
   all — whatever faults, faults without it. A symbols build and one `WM_CLOSE` answers it.
2. **The hover has no sound.** `SelectionHandler::RollOver` ends with
   `SoundHandler.PlaySound('Move')`, which is the `FGFxSoundEventCallback` that
   `UGFxMoviePlayer::PreLoad` installs in retail and that `GFxMovieView::CreateFunction` cannot return
   here (agentDG.md hand-over). Audio is Phase 10 polish, so this is recorded rather than done.
3. **`GFxMovieRoot::ChangeMouseCursorType` (`0xa04490`) and the three `GetCursorType` overrides**
   (`0x9ed080`, `0xa5c250`, `0xa23d10`). They are `useHandCursor`, and the Dishonored cursor is a movie
   clip, so the only thing they would buy is the content's ability to ask for a different cursor art.
4. **`GFxMovieRoot::QueueSetFocusTo`** and the focus model: retail moves the keyboard focus to the
   entity a press lands on. Without it a click does not take focus away from whatever had it, which on
   this menu is nothing.
5. **The rest of `GFxButtonCharacter`** (deviation 9) — the state machine and the condition actions —
   for any asset in the cook whose buttons are not this one.
6. **Everything the earlier packages handed over that this one did not touch**: the DLC config merge and
   `MISSIONS*`, `_bUsingGamepad`, `flash.display.BitmapData` for `_common.EmbedImg` (most of the 82
   script errors), `UDisGlobalUIManager`, the two open movies sharing one AS2 library registration,
   `GTessellator`, and the scene's depth-stencil surface for the UI target.

## 7. Verification

* **The acceptance run is on the clean gate build** `build/agentDQ_wtrel`: one 120 s run at
  `-startmap=Dishonored_MainMenu -startmapopen -gfxuimenu -windowed -ResX=1600 -ResY=900 -nomovie` with
  `-savedir=build/agentDQ_save -gfxuibar -gfxuicensus -gfxuishotonhover=200`, driven by
  `build/agentDQ/drive.py` — real `SetCursorPos` / `mouse_event` / `keybd_event` input into the game's
  own window, scheduled off the log rather than off the clock (agentDM.md 7). Log `DQW.log`, stdout
  `build/agentDQ/run17.txt`. **0 `Critical:` lines.** Its numbers:

  ```
  bar selection 0 -> 3 -> 2 -> 1 -> 0
  mouse: 726 hit tests / 131 targets resolved (1305 shape walks, 131 button hits, 0 sprite hits),
         4 rollOver / 3 rollOut, 1 press / 1 release, 5 handlers invoked
  AS2 trace: >> TransitionTo (NewGameScreen)          <- the click on NEW GAME
  ```

  and the per-shot bar census is the same four lines at each of its five states, the fifth identical to
  the first. `build/agentDQ/hoverstrip_gate.png` is the picture of it; `hoverstrip.png` and
  `dqhover1..7_*.png` are the earlier seven-state run (`run12.txt`) on the working-tree build.
* **Regression** on that same clean gate build: `python resources\tools\run_regression.py --build-dir
  build/agentDQ_wtrel --no-build --exe-name DishonoredGame_DQW.exe --log-prefix DQW` ->
  **`31 ok, 0 failed, 0 skipped, 423s`** (`build/agentDQ_wtrel/regression/summary.txt`).
* **Clean full Release build** of the snapshot worktree `build/agentDQ_wt` (HEAD `20acb5c` plus this
  package's 12 files), build directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`: **992 edges, 0
  errors**, `DishonoredGame`, `CoreSmoke`, `EdgeAnimSmoke`, `LayoutProbe` and the rest
  (`build/agentDQ/buildwt3.log`).
* **No generated file was touched.** Nothing this package changed is produced by
  `gen_classes_header.py`, so no regeneration is needed.

## 8. Switches this package adds

All read on first use, none a file-scope initialiser (agentCA.md).

* `-dismouse` — narrate the mouse: every `ProcessMouse` with its point and buttons, every level the
  topmost-entity walk descends into with its own local point, the resolved target's path or `NONE`,
  every dispatched event with its handler name and target, and the axis probe of §1.1 once a second.
* `-gfxuibar` — the menu bar's own state once a second and on every selection change: per entry the
  timeline frame, the local scale, the width, and the alpha / frame / width of `_bkgdOver_mc`, the clip
  depth / width / visibility of `_maskBkgdOver_mc` and the alpha of `_txtGlow_mc`.
* `-gfxuishotonhover[=<N>]` — a screenshot N drawn frames after the bar's `_curSelection` changes, with
  the bar census logged beside it, so a run photographs every hover state rather than a frame nobody
  chose (agentDB.md 2).
* The census line gains a `mouse:` row: hit tests, targets resolved, shape walks, button hits, sprite
  hits, rollOver, rollOut, press, release, handlers invoked. A delivered event, a resolved target and an
  invoked handler are three different things and nothing could tell them apart before.

## 9. Files

Mine (12, one new):

`External/GFx3/GFxHitTest.cpp` (**new**, 780 lines); edits to
`External/GFx3/{GFxAS2Lib.cpp, GFxCharacterDefs.cpp, GFxCharacterDefs.h, GFxInput.cpp, GFxPlayer.h,
GFxPlayerSprite.cpp, GFxTextField.h, GTypes.h}`; `GFxUI/{Inc/gfxuiengine.h, Src/gfxuiengine.cpp}`;
`cmake/GFx.cmake` (one entry).

Scratch, not repo tools: `build/agentDQ/` — `dqpatch.py` (the CRLF-safe patch helper),
`patch1_matrix.py` … `patch18_curselection.py` (one per change, each with its measurement in its
docstring), `drive.py` (the real-input driver), `btndump.py` (the DefineButton2 record walker),
`evtable.py` (the event-id name table read out of the image), `vtdump.py`, `lookup.py`, `disasm.py`,
`xrefs.py`, `bmp2png.py`, `swfscan.py` (agent DJ's), `dec1`..`dec9` (the headless decompiles this rests
on), the run logs `run1`..`run12` and the screenshots; `build/agentDQ_build.cmd`, `build/agentDQ_run.py`,
`build/agentDQ_sync.py`; snapshot worktree `build/agentDQ_wt`, build directories `build/agentDQ_rel` and
`build/agentDQ_wtrel` (the clean gate build); `build/agentDQ_save/DisMission0.sav` (a copy of agent DJ's
synthesised save).

IDA: **own copy only**, `build/agentDQ_ida/retail2013_agentDQ.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.
