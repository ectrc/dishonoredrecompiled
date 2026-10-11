# Agent FQ (PHASE16 FQ) — the unload list, the handle under `pDeclTarget`, the user's mouse path, and the hit test that was stealing the tabs

Branched from `15e8a10`, merged onto **`b759fd5`** (the scripted-input commit) mid-package when the
coordinator retired real input. Own worktree `build/agentFQ_wt`, own build dirs
`build/agentFQ_wt/build/agentFQ_head` (this package — the name is a leftover from a control build
that raced my own edits and became the package build; see §9.1) and
`build/agentFQ_wt/build/agentFQ_gate` (the harness's). The control is a `git archive b759fd5`
export in `build/agentFQ_ctlsrc`, built in `build/agentFQ_ctlsrc/build/agentFQ_ctl`, staged as
`DishonoredGame_FQC.exe`; the package is staged as `DishonoredGame_FQ.exe`. Own IDA copies
`resources/docs/idb/shipping2012_agentFQ.i64` and `retail2013_agentFQ.i64`, headless only.
**No FModel tool was used.** No commits, nothing staged, nothing written into the main checkout
outside `build/`. Every capture in this package was taken inside the game process
(`-apshottime` / `game_input.py shot`), tagged `fq<run>`.

**Ten files, all in `External/GFx3`**, named function by function in §8 and in
`build/agentFQ_sync.py`.

## The answer in twelve lines

* **Retail's movie-root unload list is ported** (§1): `GFxSprite::OnEventUnload` links the sprite
  onto `GFxMovieRoot::pUnloadListHead` with a reference on the call that raises its unloaded bit,
  and `GFxMovieRoot::ProcessUnloadQueue` (2012 `0xa01690` / 2013 `0x9f80b0`, `0x55` in both) walks
  and releases it after each of `Advance`'s three drains, exactly where retail calls it. Measured:
  on the Options path the list is **the last holder** of a clip 8 times in the first ten seconds of
  the main menu alone (`compass2_mc`, a timeline-removed clip), and FJ's "held by the queue alone"
  instrument still fires **3 per CONTROLS switch** with the list's reference accounted for.
* **FJ's premise about the `pDeclTarget` guard was wrong, and so was the brief's order.** Retail
  does not protect a raw pointer with the unload list; `GASAsFunctionObject` holds the declaring
  clip as a **`GFxCharacterHandle`** (ctor 2012 `0x9f28f0` / 2013 `0x9e90b0`, `0x101`) and resolves
  it on every invocation (`GetEnvironment` 2012 `0x9f1ab0` / 2013 `0x9e82d0`, `0x6b`), and the
  handle's `pCharacter` is **zeroed at unload**, not at destruction (`GFxASCharacter::OnEventUnload`
  2012 `0x9cf980` / 2013 `0x9c61d0`, last two lines). The guard never dereferences freed memory in
  retail, with or without the list. Both are ported (§2); the list is still right, for the drag
  state and the environment target, which really are bare.
* **The user's path is driven, through GFx's own mouse handler, and it lives** (§3): scroll bar
  dragged to the bottom (`OnLifterPressed` → `ratioScroll = 0.999964105364522`, the user's exact
  value), sub-tab toggles, then CONTROLS — `AS2 trace: OnMouseUp (0, undefined)` immediately
  followed by `OnTabSelected (1, r)`, three cycles per run, two runs, **0 criticals, clean exit**.
  Not a claim that the crash is fixed: FJ's fix removed the dangling queue target, this package
  removes the second class, and the path that killed the user's process now runs end to end.
* **The control cannot even reach that switch after a drag, and that is a defect of its own**
  (§3.2): with the list scrolled to its end, every click on the sub-tabs and on the main tabs
  landed on a **masked-out list row** — `Click (5) in SelectionHandler`, `APressed` on undefined —
  because `GFxSprite::GetTopMostMouseEntity` never asked the sprite's `setMask` mask about the
  point. Retail does, first thing after the local transform (2012 `0x9fa5b0+0x14b` / 2013
  `0x9f0e80`, `0x428`). Ported; the tabs answer again. **This is user-visible on HEAD today:**
  scroll any Options list and the tabs above it stop working until it is scrolled back.
* **`this` stringifies as `undefined` — the cause and the fix** (§4): `GASValue::ToString` went
  through `valueOf`, and `Object.prototype.valueOf` on a clip answered `undefined` because
  `GFxASCharacter` has no `ToASObject`. Retail's `ToStringImpl` (2012 `0x9cbe50` / 2013 `0x9c26f0`,
  `0x391`) consults **`toString` only**, and `GASObjectProto::ToString` (2012 `0x9dab80` / 2013
  `0x9d1340`) answers a clip's **name path**. Ported, with `GetCharacterNamePath` (`0x9cab10` /
  `0x9c13b0`) and retail's `ToPrimitive` (`0x9cc880` / `0x9c3120`): the trace reads
  `OnTabSelected (1, r)  / _level0.optionsMenu_mc._tabs_mc` in every package run and `/ undefined`
  on the control.
* **The brief's claim that every "on undefined" error is untrustworthy was wrong in its mechanism
  and right in its effect.** That text never came from stringification — it is `self == 0`, the
  receiver's `ToObjectInterface` — but it did hide two receivers behind one word. The error text
  now says which (§4.2): per package run of the user's path, **8 of the 104 "on undefined" are
  "on a removed clip `_level0.optionsMenu_mc._options_mc.scrollBar`"** (the `removeMovieClip`
  FJ flagged — the content removes a scroll bar that is already gone, a no-op in retail too) and
  **2 are "on null"**; the remaining 94 are genuine undefined receivers.
* **`UpdateSettings` is not a defect** (§5): the site is `OptionsSecondaryScreen.APressed`'s
  `_targetMc.UpdateSettings(...)`; on the difficulty screen `_targetMc` is
  `newGame_mc._menu_mc`, a `NewGameMenu extends _common.GenericMenu`, and **`MainMenu.gfx`
  contains no `UpdateSettings` string at all** — retail makes the same call on the same undefined
  method. `TArray<GFxValue>`: no instance remains in the tree; FM already replaced its own site.
* **Gates** (§6): clean full Release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`,
  **0 errors, 0 C4263, 0 C4264**; `rva_sweep.py` over `External/GFx3`: **728 citations, 0
  MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013**; `run_regression.py` from the worktree's own copy with
  an absolute build dir, built inside the harness: see §6 — the one red row is `inputtest_criticals`,
  FN's, as the plan says.
* **Where the brief was wrong**, six places, §9 — including one of my own: the first "control"
  of this package was my own code for eight runs, because a `sed` missed a `cd` line.

## 1. The unload list

### 1.1 Retail, by hand

`GFxSprite::ExecuteEvent(const GFxEventId&)`, 2012 `0x9f4a10` / 2013 `0x9eb1f0`, `0xe7` in both.
The whole body is: refuse if the unloaded bit is up; AddRef; for anything but a filtered
`Event_Load`, `GFxASCharacter::ExecuteEvent` (the handler lookup), **then, if the event is
`Event_Unload` (4)**:

```asm
9f4ac3  mov eax, [esi+0xbc]        ; pMovieRoot
9f4ac9  or  word [esi+0x76], 0x10  ; the unloaded bit
9f4ad4  mov [esi+0x8c], edx        ; sprite->pNextUnloaded = root->pUnloadListHead
9f4ada  mov [eax+0x27fc], esi      ; root->pUnloadListHead = sprite
9f4ae0  inc dword [esi+4]          ; the list's reference
```

and Release. `GFxMovieRoot::ProcessUnloadQueue`, 2012 `0xa01690` / 2013 `0x9f80b0`, `0x55`:

```
for (ch = root[0x27fc]; ch; ch = next)
    next = ch[0x8c]; ch[0x8c] = 0;
    ch->vtbl[+0x88](ch)                   ; OnEventUnload, vtable slot 34
    if (ch[0x20]) ch[0x20]->vtbl[+0x90](ch)   ; the parent is told; slot 36
    ch->Release()
root[0x27fc] = 0
```

Called from `GFxMovieRoot::Advance` (2012 `0xa11830` / 2013 `0xa08280`, `0x7a0`) at three sites,
each one instruction after a `DoActions`: the frame-0 arm (`0xa11bba`), the catch-up loop
(`0xa11e3e`) and the fractional-frame arm (`0xa11ef0`); 2013 `0xa0860a` / `0xa0888e` /
`0xa08940`. The destructor (2012 `0xa12700` / 2013 `0xa09150`, `0x5e4`) walks the list at `+0x24c`
and releases without the second `OnEventUnload`, after `GFxSprite::ClearDisplayList`
(`0x9f4420`: `DisplayList.Clear()` and the root's `0x400` dirty bit) has unloaded every level's
subtree. `GFxMovieRoot::Restart` has the same release loop (2012 `0xa12303..0xa12325`).

What populates it is the unload itself. `GFxCharacter::OnEventUnload` (2012 `0x9ce800` / 2013
`0x9c50d0`, `0x74`) is: flag `0x100`; leave the topmost-level list if on it; **if the unloaded bit
is clear, send `Event_Unload` through the `ExecuteEvent` virtual (vtable+0x80) and raise the bit**.
`GFxASCharacter::OnEventUnload` (2012 `0x9cf980` / 2013 `0x9c61d0`, `0x53`) wraps it: flag,
`RemoveFromPlaylist`, clear the root's drag character if it is this one, the base,
`ResetFocusForChar`, and **`pHandle->pCharacter = 0`** — the handle is detached at unload.
`GFxSprite::OnEventUnload` (2012 `0x9fdc80`, `0x59`; agent FJ's) does `DisplayList.Clear()` first
and then the base, which is how the unload reaches the subtree.

### 1.2 Ours

`GFxASCharacter::OnEventUnload` is new and is retail's order minus the play list and the focus:
drag clear, `ExecuteEvent(GFxEventId(Event_Unload))` once (the `onUnload` dispatch, which FJ had
left out — this cook defines **no** `onUnload` handler in any of its six movies, so it is a
measured no-op here and a correctness item for the cooks that do), the bit,
`pHandle->ChangeCharacter(0)`. `GFxSprite::OnEventUnload` does `Clear`, the base, and the link
onto the list on the call that raised the bit. `GFxMovieRoot::AddToUnloadList` and
`ProcessUnloadQueue` are the two bodies above, with the parent notification left out (nothing per
parent in this tree needs it) and one measurement in the walk; `Advance` calls the walk after each
of its three `DrainActionSessions`; the destructor walks it before `pLevel0`'s release and sweeps
whatever the root sprite's own teardown linked after it.

`GFxASCharacter` gains `pNextUnloaded` (retail sprite+0x8c) and `bInUnloadList`, the second so
that FJ's action-queue instrument can tell the list's reference from the queue's: its test is now
`RefCount == (bInUnloadList ? 2 : 1)` and it keeps its meaning.

### 1.3 Measured

Package run `b4` (the user's path, §3), the first ten seconds of the main menu:

```
[0008.65] unload list: _level0.newGame_mc.instance9.compass2_mc held alone until the end of the advance (1 so far of 1 queued)
   ... (8 of these by 10.4 s, the cap on the line)
```

That clip is removed by its own timeline (`UnloadMarkedObjects`) every loop; before this package
it was freed on the spot, and anything that still named it — the mouse state's weak proxy resolves,
but a `GASEnvironment` target, a drag, or a queued buffer would not — had a dangling pointer for
the rest of the advance. And the Options switch the user died on: three entries per CONTROLS
selection still read "held by the queue alone", with the list's reference subtracted, so FJ's
count stands and the two references are independent.

## 2. `pDeclTarget`: a handle, then the guard

**FJ left the guard unwritten because `pDeclTarget` was a raw `GFxASCharacter*` here and reading
its unloaded bit would dereference freed memory.** Retail's is not raw. `GASAsFunctionObject`'s
constructor (2012 `0x9f28f0`, `+0xa6..0xf5`) takes `env->Target->GetCharacterHandle()` (creating
one), AddRefs it into `this+56`, and `GetEnvironment` (2012 `0x9f1ab0`) resolves it with
`GFxCharacterHandle::ResolveCharacter` at every call — a handle whose `pCharacter` the unload has
already zeroed re-finds the clip at that path or nothing. `Invoke` (2012 `0x9f3540` / 2013
`0x9e9d10`, `0x11f`) then applies the test FJ read: `if (env->Target->Flags76 & 0x10) env =
call.pEnv`.

Ported as such: `GASFunctionObject::pDeclTarget` is a `GFxCharacterHandle*` with
`SetDeclTarget` / `ResolveDeclTarget`, released in the destructor and in `PrepareForCollection`;
`GFxAS2InvokeScriptFunction` resolves it, sets the target only when it resolves to a clip that is
not unloaded, and otherwise leaves the caller's target in place — this tree has one environment
per movie, so "the caller's environment" is the target the caller left. The fallback is counted
(`Census::DeclTargetFallbacks`, logged for the first eight). **Measured on the user's path: 0
fallbacks in two full runs** — no function in this cook is invoked after its declaring clip has
gone. The guard is retail's and it is now safe; it just has nothing to catch on this screen.

## 3. The user's path, through GFx's own mouse handler

### 3.1 How

Real input is forbidden since `b759fd5`; `game_input.py` injects through
`UGameViewportClient::InputKey/InputAxis`, which is the same entry `UWindowsClient::ProcessInput`
hands DirectInput to, so `FGFxEngine::InputKey` → `GFxMovieRoot::HandleEvent` → the input queue →
`ProcessMouse` → `GFx_GenerateMouseButtonEvents` is the user's own path from the GFx side. The
schedule is `build/agentFQ/user_path.txt`, read step for step off `play_live.log`: dismiss the
start screen, `Right`, `Right`, `Enter` (Options, GENERAL/GAMEPLAY), two toggles of the first row's
widget (the user toggled Auto Use Mana Elixir four times), USER INTERFACE, **the lifter dragged
from 0.433 to 0.90 of the viewport height in four moves**, GAMEPLAY, USER INTERFACE, **CONTROLS**;
then the same twice more and GRAPHICS / AUDIO / GENERAL, then `quit` so the movie root's teardown
runs. Coordinates are viewport fractions read off 1600x900 in-process captures.

### 3.2 What the control showed — the tabs are unreachable after a scroll

Control run `c9` (`b759fd5`, untouched): the drag works to the user's exact ratio, and then
**every click on a sub-tab or a main tab is taken by a list row that the mask hides**:

```
[0039.62] AS2 trace:  Click (5) in SelectionHandler ; _root.inputs.InputsEnabled : true
[0039.62] AS2 error: call of a value that is not a function: 'APressed' ... on undefined
```

six times, one per tab click, and not one `OnTabSelected` after the drag. `-dismouse`'s hit-test
trace on the same schedule (`build/agentFQ/b2_stdout.txt`, the package before the fix): at the
GAMEPLAY sub-tab's point `(523.2,214.4) px` the entity is `_options_mc._list_mc.widget5.btn`
(139 tests) / `widget4.btn` (173); at CONTROLS `(536.0,155.2)` it is `widget2.btn` 130 times out
of 287. The rows scrolled up under `_list_mc`'s `setMask` mask are still hit-testable.

Retail's `GFxSprite::GetTopMostMouseEntity` (2012 `0x9fa5b0` / 2013 `0x9f0e80`, `0x428`) does, at
`+0x14b`, right after the local-point transform and before the display list is walked:
`GetMask()`; if it is in use as a mask and not unloaded, carry the point by `inverse(mask world) *
this world` into the mask's space and `PointTestLocal(pt, 1)` on the mask; a miss answers no entity
at all. Its first test (`+0x4b..0x63`) also refuses a sprite that is itself serving as a mask. Agent FF
ported exactly that arm into `PointTestLocal` (MovieClip.hitTest's path) and not into the mouse's.
Ported here; afterwards (`b3`) the same two points answer `_subTabs_mc._tabs_mc._tabs_mc.tab0.btn`
(236/236) and `_tabs_mc._tabs_mc.tab1.btn` (228/228).

This is not only a driver problem. On HEAD, with a real mouse, scrolling any Options list to the
end and then clicking the tabs above it presses invisible rows instead. The user's own log shows
sub-tab switches working after the drag (29.80 s), so the user's clicks must have fallen between
the hidden rows' buttons — a 40-pixel-high target behind a 30-pixel gap.

### 3.3 What the package showed

Runs `b4` and `b5`, same schedule, package build:

| | `b4` | `b5` |
|---|---|---|
| `OnLifterPressed` → `ratioScroll` 0.999964105364522 → `OnLifterReleased` | 3 | 3 |
| `OnMouseUp (0, undefined)` immediately followed by `OnTabSelected (1, r)` | 3 | 3 |
| other main-tab switches through the mouse (`(0, l)`, `(2, r)`, `(3, r)`, `(0, r)`) | 6 | 6 |
| "held by the queue alone" (FJ's instrument, list-adjusted) | 16 | 16 |
| `DeclTargetFallbacks` | 0 | 0 |
| criticals / `Exit: Exiting.` | 0 / yes | 0 / yes |

The fatal line of the user's log — `OnMouseUp (0, undefined)` then `OnTabSelected (1, r)` from
inside the mouse dispatch, with the list at its end — is reproduced six times and the process lives
through all of them and through its own teardown. The scripted events arrive from
`UGameViewportClient::Tick`, not from inside the Win32 message pump; if the user's death depended on
re-entrancy with the pump rather than with GFx's advance, this does not test it — but FJ's decode
puts the fault in the action-queue drain, which is advance-side, and this path drives exactly that
drain re-entrantly from the mouse handler (`ProcessInput` → `DrainActionSessions` inside
`Advance`). `ctx.MaskLevel` across a re-entrant `Display`, PHASE15's first candidate, stays
unmeasured: nothing on this path reached a `Display` re-entrantly, and FJ's decode already placed
the fault in `Advance`.

## 4. `undefined`

### 4.1 The cause

`GASValue::ToString`'s OBJECT/CHARACTER arm called `ToPrimitiveInto`, which tried `valueOf`
first. `Object.prototype.valueOf` here was `fn.pThis->ToASObject()` → `SetAsObject`, else
`SetUndefined` — and `GFxASCharacter` has no `ToASObject`, so a clip's `valueOf` answered
`undefined`, `ToPrimitiveInto` took that as a primitive, and `ToString` printed it. That is why
the user's `OnTabSelected (1, r)  / undefined` printed `undefined` for a `this` that the next
instruction read `_parent` off without complaint.

Retail (`GASValue::ToStringImpl` 2012 `0x9cbe50` / 2013 `0x9c26f0`, `0x391`, cases 6 and 7):
`GetMember(toString)` on the object interface — only `toString`; `valueOf` is `ToPrimitive`'s
(`0x9cc880` / `0x9c3120`, `0x2c7`) and is never consulted here — invoke it when it is a function
and stringify whatever it returned under the 255-deep guard; otherwise a boxed primitive's text,
else a character's `GetCharacterNamePath` (`0x9cab10` / `0x9c13b0`: the handle's interned path
when it still resolves, the empty builtin when not), else `[object Object]`.
`GASObjectProto::ToString` (`0x9dab80` / `0x9d1340`, `0xcc`): `[type Function]` for type 23, the
name path for the character types 2..5, `[object Object]` otherwise. `GASObjectProto::ValueOf`
(`0x9dac50` / `0x9d1410`, `0x11`): the receiver itself. All four are ported as read; `ToPrimitiveInto`
is retail's `ToPrimitive` now (valueOf only, the result returned whatever its type — which is why
`clip + 1` is NaN in retail and was a string here).

Package runs print `-------------------->> OnTabSelected (1, r)  / _level0.optionsMenu_mc._tabs_mc`
at every switch; the control prints `/ undefined`.

### 4.2 The error text, and how many errors change meaning

The brief said every "on undefined" is untrustworthy until stringification is fixed. The text is
not produced by stringification: `GFxAS2CallFunctionValue` printed `undefined` whenever the
receiver's `ToObjectInterface` was null, which covers a value that **is** undefined and a
**CHARACTER value whose handle no longer resolves** (the clip was removed — and now that the handle
is detached at unload, as retail's is, that case is exact). The function now takes the receiver
value and names it: `undefined`, `null`, `a removed clip <the handle's path>`, `an object`, or the
type and path as before.

User's path, one package run (`b4`, 121 errors; `b5` identical):

| receiver text | count |
|---|---|
| on undefined | 94 |
| **on a removed clip `_level0.optionsMenu_mc._options_mc.scrollBar`** | **8** |
| **on null** | **2** |
| on Sprite / Button / an object | 17 |

**Ten of 104 former "on undefined" change meaning.** The eight are the `removeMovieClip` at pc 1068
of `_common.ScrollBarList` that FJ singled out: `InitScrollBar` removes the previous scroll bar,
whose clip `SetList` has already unloaded, so its handle resolves to nothing — retail makes the same
call on the same detached handle and it is a no-op there too. The other 94 are genuine: the
`tweenTo` / `gotoAndStop` / `ClearAnimation` family on children that do not exist, agent FF's finding.

## 5. `UpdateSettings`, and `TArray<GFxValue>`

The only `UpdateSettings` error in any log of this tree is
`'UpdateSettings' at pc 1921 of a 2243-byte buffer, on Sprite _level0.newGame_mc._menu_mc`, on the
New Game difficulty screen's validation. The buffer is `OptionsSecondaryScreen` and the site is its
`APressed`: `this._targetMc.UpdateSettings(this._widgetSettings.Setting_Value,
this._targetMc._options_mc._curWidget_idx)` (`OptionsMenu.as2.txt:1849–1922`). On the difficulty
screen `_targetMc` is `newGame_mc._menu_mc`, a timeline instance of `m_nGame_menu`, registered to
`NewGameMenu`, which `extends _common.GenericMenu` (`MainMenu.as2.txt:19812`, `:1796 extends`).
`VideoSettings` and `OptionsScreen` define `UpdateSettings`; `NewGameMenu` and `GenericMenu` do not,
and **the string `UpdateSettings` does not occur in `MainMenu.gfx`'s constant pools at all**. The
call is undefined in retail as well; retail does not log it. Nothing to port, and FG's "not seen on
any screen my driver reaches" is because FG never validated the difficulty screen.

`TArray<GFxValue>`: `grep -rn "TArray<GFxValue>" source/Development/Src` is empty. FM's own
`DisFMInvokeStep` already uses `GFxValue Args[4]`. Nothing left to do.

## 6. Gates

| gate | result |
|---|---|
| clean full Release, `build/agentFQ_wt/build/agentFQ_clean` deleted first, `DISHONORED_LAYOUT_CHECKS=ON`, targets `DishonoredGame CoreSmoke LayoutProbe` | **0 errors, 0 C4263, 0 C4264**, 969/969 + 5/5 + 29/29 steps, all three executables linked (`build/agentFQ/build_clean.log`) |
| `rva_sweep.py --path source/Development/Src/External/GFx3` | **728 citations, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013** (277 ok-2013, 446 ok-2013-mid, 5 ok-2012-labelled) |
| `run_regression.py`, worktree's own copy, absolute build dir `build/agentFQ_wt/build/agentFQ_gate`, built inside the harness | **46 ok, 1 failed, 0 skipped**, 3676 s; the one failure is `inputtest_criticals 1 (wanted == 0)`, the known FN row; the five `options_*` rows pass (`build/agentFQ/regression.log`) |

## 7. Found and left

* **The hidden-row click stealing is fixed for the mouse; `MovieClip.hitTest` had it already.**
  What is still not retail's: `CalcDisplayListHitTestMaskArray` and the ClipDepth masks are FF's,
  untouched; the hit-area (`GetHitArea`) arm of retail's `GetTopMostMouseEntity` has no slot here.
* **`ProcessUnloadQueue`'s parent notification** (vtable+0x90 on `[ch+0x20]`) is not reproduced;
  it needs the per-parent state retail keeps and this tree does not.
* **The `removeMovieClip` on a removed scroll bar** is now named in the log and is a content no-op;
  if anyone wants the Options error count down, that is 8 per three switches with no behaviour
  behind it.
* FJ's §7 note about `DragState::pCharacter` stands: it is still bare, as retail's is, and the
  list is what makes it safe now.

## 8. Every function touched, per file

`source/Development/Src/External/GFx3/GFxPlayer.h`
  * `GFxASCharacter`: `OnEventUnload()` declared virtual (new override); members `pNextUnloaded`,
    `bInUnloadList` after `pGeomData`.
  * `GFxSprite::OnEventUnload`: comment only.
  * `GFxMovieRoot`: `AddToUnloadList`, `ProcessUnloadQueue` declared; private `pUnloadListHead`
    after `Drag`; `Census` gains `UnloadsQueued`, `UnloadsHeldAlone`, `DeclTargetFallbacks`.
`source/Development/Src/External/GFx3/GFxPlayerSprite.cpp`
  * `GFxASCharacter::GFxASCharacter` (two initialisers), `GFxASCharacter::OnEventUnload` (new),
    `GFxSprite::OnEventUnload`.
`source/Development/Src/External/GFx3/GFxPlayerRoot.cpp`
  * `GFxMovieRoot::GFxMovieRoot` (one initialiser), `~GFxMovieRoot`, `AddToUnloadList` (new),
    `ProcessUnloadQueue` (new), `DoActions` (the instrument's RefCount test only), `Advance`.
`source/Development/Src/External/GFx3/GFxAS2Object.h` / `.cpp`
  * `GASFunctionObject::pDeclTarget` (type), `SetDeclTarget` (new), `ResolveDeclTarget` (new),
    `~GASFunctionObject`, `PrepareForCollection`.
`source/Development/Src/External/GFx3/GFxAS2Interp.cpp`
  * `GFxAS2Function1OpCode`, `GFxAS2Function2OpCode`, `GFxAS2InvokeScriptFunction`,
    `GFxAS2CallFunctionValue` (one more parameter), and its two callers in
    `GASActionBuffer::Execute` (`GASop_CallFunction`, `GASop_CallMethod`).
`source/Development/Src/External/GFx3/GFxAS2.h` / `GFxAS2Value.cpp`
  * `GASValue::ToString`, `GASValue::GetCharacterNamePath` (new), `GASValue::GetCharacterHandleNamePath`
    (new), `GASValue::ToPrimitiveInto`.
`source/Development/Src/External/GFx3/GFxAS2Lib.cpp`
  * `ObjectToString`, `ObjectValueOf`.
`source/Development/Src/External/GFx3/GFxHitTest.cpp`
  * `GFxSprite::GetTopMostMouseEntity` (the mask arm and the mask-serving refusal).

Nothing in `resources/tools` (the `drag:` step I had added to `drive_input.py` went with the
driver's retirement; the merge of `b759fd5` replaced the file).

## 9. Where the brief was wrong

1. **"Port the unload list, then the `pDeclTarget` guard, in that order — without the list the
   guard would dereference the pointer it protects."** Retail's guard does not depend on the list:
   the declaring clip is a `GFxCharacterHandle` whose `pCharacter` the unload zeroes. The order is
   immaterial and the guard was always portable; what was missing was the handle. §2.
2. **"Drive the tab switch through a real mouse event."** Real input is now forbidden, and it turned
   out not to be the useful half anyway: the first eight runs of this package sent 40 s of
   keyboard and mouse into a window that had been launched with `-unattended`, which
   `UWindowsViewport` reads as "input grab disabled" — the driver reported every key and click as
   landing (foreground HWND ours) and the game logged none of them. Only the log line said why.
3. **"Every 'on undefined' in our AS2 error text is untrustworthy until stringification is
   fixed."** The text and the stringification are unrelated code paths; what the word hid was a
   removed clip's handle, 10 of 104 on the user's path. §4.2.
4. **"The user's path could not be driven because `_options_mc.MoveTo_End()` resolves NOT
   FOUND."** It can be driven — but on HEAD it cannot be *completed*, because after the scroll the
   tabs are unreachable to any click, scripted or real, through a hit test that ignores the mask.
   The brief's path was blocked by a defect nobody had named. §3.2.
5. **`UpdateSettings` "the one genuine missing prototype method."** It is missing in retail's cook
   too; the receiver on that screen is a `GenericMenu`. §5.
6. **My own**: `build/agentFQ/build_ctl.cmd`, a `sed` of the package build script, missed the
   `cd /d` line and configured the "control" from the worktree. Runs `c1`–`c8` measured the
   package, which is why a "control" printed the name path for `this` in §4 before the true control
   (`c9`) printed `undefined`. Every number in this report that says "control" is from `c9`, built
   from the clean export with the hand-written script.
