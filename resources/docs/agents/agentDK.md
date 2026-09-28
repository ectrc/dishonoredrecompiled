# Agent DK — the menu's camera moves, and the menu is operable (2026-09-28)

Package DK: finish agent DJ's package. The three differences DJ named against the user's reference
screenshots (`resources/reference/menu/first.jpg`, `second.jpg`), plus the two the user added while
this package was running: **the menu must be navigable and activatable**, and **the text must carry
its drop shadow**.

`build/agentDK/menu_screen1.png` is the start screen and `build/agentDK/menu_screen2.png` the main
menu, reached by a key press, **with the 3D view changed between them**: the harbour on screen one and
the city street with "THE BOLDEST MEASURES ARE THE SAFEST", the Outsider's shrine and the tram rails
on screen two — which is `second.jpg`'s scene. `build/agentDK/navstrip2.png` is three menu bars from
one run with the highlight on **CONTINUE**, then **NEW GAME**, then **MISSIONS**, moved by two
scripted Right presses; `build/agentDK/menu_newgame.png` is the New Game screen that Enter opened on
NEW GAME, with its four difficulties and its own camera.

Ten defects, every one measured against the retail binary or against the cook, never guessed. Three of
them are the reason the camera stood still, and each is one level deeper than the last: the Kismet
node had no handler, the matinee had no data, and the camera crashed the moment it was finally asked
to be a camera.

## Result

| Accept | State |
|---|---|
| screen one and screen two, screen two by a key press, **camera changed between them** | **done** — `build/agentDK/menu_screen1.png`, `menu_screen2.png`. The view target goes `DishonoredPlayerPawn` -> `CameraActor_24` (the map's `StartCam` group) -> `CameraActor_11` (`Main_View`), driven by the menu map's own Kismet. Section 1 |
| the DLC platform button lit, or why not | **half**: the button now registers itself — the two `AddControllerButtonInstance` errors are gone and `UIBase.UpdateButtonsInstance` drives it — but its glyph is still the unlit PC one. Section 5 names what is left, with the reason the reference shows a gamepad glyph |
| `MISSIONS*`, or the reason | **reason, with the address**: 2013 rva **0x7d28b0** appends the `*` to `Localize("DisGFxMoviePlayerMainMenu_Texts","t_Missions")` when any of the ten `m_DLCCommands` slots is an installed DLC, and writes `_root.texts.t_Missions`. The slots come from the DLC packs' own `DishonoredUI.ini`, which this tree does not merge. Section 6 |
| **the menu is interactable** | **done for the keyboard**: Right/Left move the highlight, Enter activates. The defect was the *test harness* — section 4 |
| **the text drop shadow** | **measured, not ported**: the cook asks for **3 `DropShadow` filters, all on the instance named `txt`**, through PlaceObject3's filter list — which this tree did not parse at all. It parses and reports them now; applying them is a package. Section 7 |
| census: script errors below 41, 0 unbound text fields, 0 atlas failures | **39 script errors** (41 before), `8 text fields [0 unbound]`, `atlas 36 packed / 2 blank / 0 failed`. Section 9 |
| `run_regression.py` 31 checks, 0 failures | **`31 ok, 0 failed, 0 skipped, 422s`** on the clean gate build `build/agentDK_wtrel` |
| clean full release build | **982/982 targets, 0 errors** on the snapshot worktree `build/agentDK_wt`, build directory deleted first. Section 10 |
| report + `agentDK_status.csv` | this file; 19 rows, `rva_2013,status,note` |

## 1. Why the camera never moved: three defects, one under the other

### 1.1 Every Kismet fscommand node in the game had a NULL handler

`-gfxuifscmd` (new) logs the routing. Before anything was changed:

```
fscommand 'OpenMainMenuScreen' arg '' movie 17C50000
fscommand: node ...Main_Sequence.GFxEvent_FSCommand_44 wants movie UI_MainMenu.MainMenu, this movie is UI_MainMenu.MainMenu, handler NONE
fscommand 'OpenMainMenuScreen': 38 nodes in the sequence, 2 by name, 0 routed
```

So the fscommand reached `FGFxFSCommandHandler::Callback` (2013 `0x586450`), the menu map's 38
`UGFxEvent_FSCommand` nodes were all there, two matched on **both** the movie and the name — and
`Event->Handler` was NULL on every one of them, so nothing was routed.

`UGFxEvent_FSCommand::RegisterEvent` (2013 **`0x58e5f0`**, 96 bytes, matched byte-for-byte to the 2012
build) is an override of `USequenceEvent::RegisterEvent` that calls the base and then

```c
*((_DWORD *)this + 72) = UObject::StaticConstructObject(UGFxFSCmdHandler_Kismet::PrivateStaticClass,
                                                        UObject::GetTransientPackage(), 0, ...);
```

`this + 288` is `Handler`. **The whole override was absent from this tree** — the eleventh instance of
this project's defining pattern, and the same shape as agent DI's missing `PostBeginPlay`. It is now
in `Src/gfxuinatives.cpp` with `FinishDestroy` (`0x5724c0`) beside it, declared through
`Inc/CppText/UGFxEvent_FSCommand.h`.

### 1.2 Every `GFXUI_*` FName in the module was `None`

With the handler built, the first routed command died:

```
Critical: appError called: Failed to find function None in GFxFSCmdHandler_Kismet Transient.GFxFSCmdHandler_Kismet_8
```

`AutoGenerateNamesGFxUI()` was inside `#if WITH_GFx` in `LaunchEngineLoop.cpp`, and `WITH_GFx` is 0 in
this tree, while `AutoInitializeRegistrantsGFxUI()` three hundred lines above it is already called
unconditionally. So all 139 `GFXUI_*` name globals stayed `NAME_None` and the first
`ProcessEvent(FindFunctionChecked(GFXUI_X))` through any of them called `appErrorf`. Retail builds with
`WITH_GFx=1`; the call is now unconditional, as the registrant call is.

After that: `fscommand 'OpenMainMenuScreen': 38 nodes in the sequence, 2 by name, 2 routed`, 8
`GFxEvent_FSCommand` fired, and the menu map's sequence went from **7 of 118 ops ever activated to 20**.

### 1.3 The matinee runtime read shims — Arkane moved the data one object down

The camera still did not move. `-dismatinee` (new) says why:

```
matinee inventory: SeqAct_Interp_14 data InterpData_14 (0 groups, 0 groupinst, playing 1, position 0.00, length 0.00, looping 1)
```

Twelve `SeqAct_Interp` in the menu map, every one with authored data behind it, every one playing over
**0 groups of 0 length**. Arkane replaced the reference `UInterpData`'s payload with a single pointer,
`UMatineeData* m_Data` (2012 PDB @148, retail @148), and the groups, the length and the path-build time
live in `UMatineeData::m_RunData` — `FRuntimeMatineeData` at `UMatineeData+56`: `InterpGroups` @56,
`InterpLength` @68, `PathBuildTime` @72. The reference members are `DISHONORED_SHIM_STATIC` on
`UInterpData` — storage-less and permanently empty — and **all 48 runtime uses in
`UnInterpolation.cpp` read them**. A comment at `UnInterpolation.cpp:1639` already knew ("leaves
`UInterpData::InterpLength` at 0 until that is ported", agent AF's B1); nothing had followed it up.

The three accessors, decompiled from the retail exe:

```
UInterpData::GetInterpLength    0x212940   *(float*)(m_Data + 68)
UInterpData::GetNbInterpGroups  0x215000   m_Data ? m_Data->m_RunData.InterpGroups.Num() : 0
UMatineeData::GetInterpGroup    0x217f60   m_RunData.InterpGroups(Index)
```

With them wired through, three more defects fell out in a row, each named by the crash it caused:

1. **`Actor.InterpolationStarted` does not exist in the retail script.**
   `Failed to find function InterpolationStarted in InterpActor ...InterpActor_16`. Two sources agree:
   `script_classes_2013.json`'s `Engine.Actor` has no `Interpolation*` in its 208 `func_map` entries or
   its 421 children, and retail's own `USeqAct_Interp::InitInterp` (**`0x2342e0`**) builds the group
   instances — director, player, AI, plain — and notifies no actor at all. The reference's three
   `eventInterpolation*` calls are Epic's; Arkane dropped the script events.
2. **A group's `Outer` is the `UMatineeData`, not the `UInterpData`.**
   `Cast of MatineeData Dishonored_MainMenu...Main_Sequence.InterpData_14.MatineeData_3 to InterpData failed`
   — the cooked path says it: `<InterpData>.<MatineeData>.<Group>`. Nine
   `CastChecked<UInterpData>(Group->GetOuter())` sites now go through `UInterpGroup::GetInterpData()`.
3. **`ACamera::ApplyCameraModifiers` carried Epic's camera-anim block**, and the first frame in which
   the director group made a `CameraActor` the view target it died on `AnimCameraActor->Location`
   (`UnCamera.cpp:357`) — a temporary actor nothing in this tree spawns. Retail's body
   (**`0x1e5b40`**, 273 bytes) is the `ModifierList` loop and nothing else; `ActiveAnims`,
   `AnimCameraActor`, `InitTempCameraActor` and `ApplyAnimToCamera` are not in it.

### 1.4 What the camera does now

```
[0005.3] viewtarget DishonoredPlayerPawn ... POV 4030,-6990,1945 rot 0 -10240 0
[0006.6] viewtarget CameraActor_24 (CameraActor) at 4223.3,-7119.9,1894.3   <- StartCam, the harbour
[0012.4] viewtarget CameraActor_24 at 2750.2,-7132.0,2833.5                 <- the fly-through
[0013.4] viewtarget CameraActor_24 at 350.3,-5255.0,3078.6
[0014.4] viewtarget CameraActor_11 (CameraActor) at 53.6,-4834.9,3036.0     <- Main_View, the city
```

and the matinee census names the groups it is running:

```
SeqAct_Interp_83 group 'DirGroup' (InterpGroupDirector, DIRECTOR): actors [DishonoredPlayerController], tracks [InterpTrackDirector]
SeqAct_Interp_83 group 'StartCam' (InterpGroup): actors [CameraActor_24 (5 trackinst)], tracks [InterpTrackMove InterpTrackSoireeControl InterpTrackEvent InterpTrackEvent InterpTrackAkEvent]
SeqAct_Interp_1  group 'Main_View' (InterpGroup): actors [CameraActor_11 (2 trackinst)], tracks [InterpTrackMove InterpTrackEvent]
SeqAct_Interp_12 groups Wagon / Wagon_Top / FX_Car_01..04 / FX_Dust_01 ...
```

This is the whole matinee runtime coming alive, not only the menu's camera: `L_Tower_P`'s single
`SeqAct_Interp` and every matinee in the game was reading the same empty shims.

**Named difference, measured:** the fly-through from `StartCam` to `Main_View` runs on the map's own
clock — it happens at 12.4 s with or without the key press (`build/agentDK/run17.log`, no
`-gfxuikey` at all, and the camera still cuts to `CameraActor_11` at 14.4 s). In retail the start
screen holds until the player presses a key. The only track on `StartCam` that could hold it is
**`UInterpTrackSoireeControl`**, which exists in this tree as a declaration only — a shim class in
`DishonoredGame/Inc/DishonoredGameEngineShims.h:13319` with no `UpdateTrack` — so it is the first place
to look. The two acceptance screenshots are taken inside the windows the reference shows (frame 700 on
`StartCam`, frame 3000 on `Main_View`).

## 2. Two defects in the GFx action queue, and only one of them was mine to find

Moving the queued class binding to `GFxAP_Lowest` (section 3) made the game crash inside
`GFxDisplayList::GetCharacterByName` on a destroyed character. It was not caused by the move; the move
only changed the timing enough to expose it.

`-gfxuidlcheck` (new) keeps a live set of `GFxASCharacter` and reports a display-list entry whose
character has been destroyed:

```
dlcheck: destroying 2604D200 name 'instance19' depth 19 parent 2604DDE0, parent list 18 of 19 STILL LISTED
dlcheck: entry 18 of 19 in a display list is a DESTROYED character 2604D200 (looking for 'onEnterFrame')
```

and no `RemoveAt`, `AddDisplayObject` or `ReplaceDisplayObject` ever logged a release of it — the
display list was not what dropped the reference. An extra `AddRef` per entry did not help either,
which is what said the pointer was stale rather than the count wrong.

**`GFxMovieRoot::DoActions` executed the same queue entry twice.** It walked `Actions` by index while
the actions it ran queued more entries, and `PushActionBuffer` / `QueueClassBinding` insert **by
priority**, not at the end: a higher-priority insert shifts every later entry up by one, so an entry
already executed reappears at the loop's next index. For an action buffer that is a duplicated script
run; for a class binding it is a second `e.pTarget->Release()` against one `AddRef`, and the sprite is
destroyed while its parent's display list still points at it. Retail cannot hit this:
`GFxMovieRoot::ActionQueueType::DoActionsForSession` (2012 `0xa0d9e0`) walks an
`ActionQueueSessionIterator` over a linked list and detaches each entry before running it. The session
is now taken out of the queue before it runs.

Two smaller robustness fixes went in beside it, both of the same family and both kept:

* `GFxDisplayList::RemoveAt`, `AddDisplayObject` and `ReplaceDisplayObject` **detach the entry before
  they release it**. A character's destructor destroys its own children and each of those runs script
  through `OnEventUnload`, which walks display lists by name; for the length of that `Release` the list
  published a pointer to an object being destroyed.
* `UnloadMarkedObjects` and `UnloadAll` re-clamp their index every step and hold a reference across
  `OnEventUnload`, instead of caching the size before calling a handler that can shrink the list.
* `GFxSprite::AdvanceFrame` advances its children over a refcounted snapshot.

## 3. The DLC button: `_root.UIBase` did not exist yet

`-gfxuiwatch=UIBase`, 50 ms apart:

```
[0005.47] AS2 error: call of a value that is not a function: 'AddControllerButtonInstance' at pc 681 of a 1805-byte buffer, on undefined
[0005.47] AS2 error: call of a value that is not a function: 'AddControllerButtonInstance' at pc 359 of a 731-byte buffer, on undefined
[0005.52] watch 'UIBase' on _level0 -> type 6 '[object Object]' (pc 353 of 731)
```

The identical constructor reads `_root.UIBase` successfully one drain later.
`_common.ClickableButton`'s constructor is two statements — `super()`, then
`_root.UIBase.AddControllerButtonInstance(this)` — so an instance built before `_root.UIBase` exists is
never registered and its `_glow_mc` never animates.

`_root.UIBase` is built by `new MainMenuBase()` in the root's **frame-1 DoAction**
(`build/agentDJ/mm_rootaction.txt` offset 654), which is `GFxAP_Frame` (4). Agent DG's
`QueueClassBinding` queued the binding — and therefore the bound class's constructor — at
`GFxAP_User` (2), which drains first. `GFxAP_Lowest` (5) keeps it after the `__Packages` registration
at `GFxAP_Init` (1), which is what DG needed, and behind the frame actions of the same session, which
is what the constructors need. That is also the real GFx ordering: a registered class's constructor
runs on the load event, and the load priority is below the frame priority.

Measured: the two `AddControllerButtonInstance` errors are gone, and the row's button is registered —
`UIBase.UpdateButtonsInstance` now drives it, which is why seven *new*
`GotoLabeledFrame: no frame named 'PC' on ..._glow_mc (3 labels)` lines appear (they are the
registration working; the clip being asked is the glow, which has `default/loop/stop`, not the
platform clip).

## 4. The menu is operable — and the defect was the test harness

Input reaches the content: `FGFxEngine::InputKey` -> `UDisGFxMoviePlayerMainMenu::FilterButtonInput`
-> the runtime -> the AS2 `Key` broadcaster -> `_common.UIBase.InputManager`, which compares
`Key.getCode()` against 38/40/37/39/13 and calls `this.InputDelegate('right', Key.isDown(39))`. All of
that was already running: the census showed the events handled and the listener called. Nothing moved.

`_common.UIBase.InputDelegate(keyID, keyHold)` (`build/agentDJ/mm_init.txt`, outer pc 6272) begins

```
  120 Push reg5, true
  127 Equals2
  128 LogicalNot
  129 If -> 935          <- the end of the function
```

so its **entire** body — the `getKeyStatus` / `targetMc` / `InputsEnabled` chain and the call into the
screen's handler — is skipped unless `keyHold` is true, and `keyHold` is the `Key.isDown(code)` that
`InputManager` passed. `-gfxuiwatch=targetMc` reported **0 reads** across two Right presses: the loop
body never ran.

`-gfxuikey` pressed and released the key in the same tick. The queue only reaches the AS2 listeners on
the next `GFxMovieRoot::ProcessInput`, so by then the release had been applied and `Key.isDown` read
FALSE inside `onKeyDown`. Retail's `GFxKeyboardState::SetKeyDown` (**`0xa52f30`**) sets the bit *and*
queues the event, so a listener sees the state at notification time in retail too — the tree's order is
right and the test's was not. `-gfxuikey` now holds the key `-gfxuikeyhold` drawn frames (default 30)
before releasing it.

With that: `build/agentDK/navstrip2.png` — **CONTINUE -> NEW GAME -> MISSIONS** across two Right
presses, the highlight and its mask following the label each time. And with Enter on NEW GAME:

```
AS2 trace: > APressed () called
AS2 trace: >> Close : undefined
AS2 trace: >> TransitionTo (NewGameScreen)
AS2 trace: > fscommand (ToNewGameScreen)
AS2 trace: PlayOpenButtonAnim (0..3)
AS2 trace: OpenDescription
```

`build/agentDK/menu_newgame.png` is the result: the NEW GAME screen with EASY / NORMAL / HARD / VERY
HARD, the difficulty description text, and the camera moved again by the map's Kismet to the rooftop
view — a third camera, driven by the same fscommand path as section 1.

**Mouse**: the content does listen for it — `_common.UIBase.AddMouseListener` sets
`onMouseUp = this.OnMouseUp` and `onMouseWheel = this.MouseWheelManager` and calls
`Mouse.addListener`. The census reports `0 mouse events` because nothing in this harness generates
them; there is no `-gfxuimouse` equivalent of `-gfxuikey`. That is the next step for the mouse half,
not a missing runtime.

## 5. What is still not right in the DLC row

The row draws, its band and its label are right, and its button is registered. Two things remain, and
only the first is ours:

1. The glyph is the **PC** one. The reference shows the blue gamepad `X`, which is what
   `lib_X_Multi` shows when `_root.UIBase._bUsingGamepad` is true — the reference was taken with a pad
   connected. `UIBase.UpdateButtonsInstance` reads `_bUsingGamepad` and calls
   `gotoAndStop(PlatformName)`; nothing in this tree ever sets it, because gamepad input does not reach
   the interface yet.
2. The seven `no frame named 'PC' on ..._glow_mc` errors say the goto is reaching the glow rather than
   the platform clip. `AddControllerButtonInstance(mc)` (mm_init.txt outer pc 2223) pushes `mc` into
   `_buttonsInstance` and `UpdateControllerButtonsInstances` walks it; which `mc` the constructor passes
   is the thing to read next.

## 6. `MISSIONS*`, precisely

2013 rva **`0x7d28b0`**, a method of `UDisGFxMoviePlayerMainMenu` (guarded by
`GetClass() == UDisGFxMoviePlayerMainMenu::StaticClass()` and a bit at `this+196`):

```c
Localize(&s, L"DisGFxMoviePlayerMainMenu_Texts", L"t_Missions", L"DishonoredGame", 0, 0);
for (i = 0; i < 10; ++i) if (IsDLCSlotAvailable(i)) { FString::operator+=(&s, L"*"); break; }
movie->SetVariable("_root.texts.t_Missions", s);
```

`IsDLCSlotAvailable` is `0x7c2070`: `UDisGlobalDLCManager::GetDLCStatus(GameInfo->m_pDLCManager,
m_DLCCommands[i].m_DLC) == 2` **and** `m_DLCCommands[i].m_Command` non-empty **and**
`m_DLCCommands[i].m_CheckedClass` non-null. `FDisDLCMissionCommand` is 24 bytes at
`UDisGFxMoviePlayerMainMenu+504`, ten of them, and they are **config**:

```
DLC/PCConsole/DLC05/DishonoredUI.ini: m_DLCCommands[0]=(m_DLC=eDisDLC_05,m_Command="start L_DLC05_MainMenu_P",m_CheckedClass=DishonoredGame.DisDLC05GameInfo)
DLC/PCConsole/DLC06/DishonoredUI.ini: m_DLCCommands[1]=(...)
DLC/PCConsole/DLC07/DishonoredUI.ini: m_DLCCommands[2]=(...)
```

The base `DefaultUI.ini`'s `[DishonoredGame.DisGFxMoviePlayerMainMenu]` section has only
`m_bEnablePSStoreAccess=FALSE`, and the merged config this tree produces
(`build/agentDK_config/DishonoredUI.ini`) has no `m_DLCCommands` line at all: the DLC packs' `.ini`
patching is not ported (`UDownloadableContentManager`'s DLC natives are on the stub list in
`STATUS.md`; the engine does find the three DLC directories — `DevDlc: Found DLC dir DLC05/06/07` — and
stops there). **So retail shows the asterisk because it has three installed DLC packs whose config it
merged, and we do not because we merge none.** `DishonoredGame.int`'s `t_Missions` is `"Missions"` with
no asterisk in either build; the string that proves the mechanism is
`NewDLCInstalled="Go to the MISSIONS* menu to find your new Downloadable Content Pack."`.
`UDisGlobalDLCManager::GetDLCStatus` (`0x8465e0`) and `UArkDLCManagementBridge` are both unported
stub files. Nothing was written for this: a port that can only ever answer "no DLC" would be dead code.

## 7. The text drop shadow: the cook asks for it and the tag reader threw it away

`GFxPlaceObject2Tag::Read` had, for PlaceObject3 (tag 70):

```cpp
if (flags3 & 0x01) { /* filter list */ }
```

The filter list was never parsed. It is parsed now, by the same record walk `GFxButtonRecord::Read`
already used for `GFx_LoadFilters` (2012 `0xa93b60`), and reported:

```
PlaceObject3 filter: DropShadow on 'txt'
PlaceObject3 filter: DropShadow on 'txt'
PlaceObject3 filter: DropShadow on 'txt'
```

**Three drop-shadow filters, every one on an instance named `txt`** — the text fields. So the reference
screenshots' shadow is a PlaceObject3 drop-shadow filter on the text field, not a duplicated layer in
the asset and not a text-format property; and this tree neither parsed it, nor stored it, nor rendered
it. `gfxuishaders.cpp`'s `FGFxFilterPixelShader` types are declared but nothing requests a filter, so
"requested and dropped" is not the shape here: it was never requested.

Applying them is a package: the filter parameters have to reach the character
(`GRenderer::BlurFilterParams`, 68 bytes, `Mode/BlurX/BlurY/Passes/Offset/Color/Color2/Strength/cxform`
— the layout is already asserted in `GFx3Layout.cpp:381`), and `GFxDisplayList::Display` has to render
the subtree to a target, blur it and composite, which is `FGFxRenderer::PushFilters` /
`PopFilters` and the shader family `gfxuishaders.cpp` declares. A cheap intermediate that would look
right at this text size — draw the subtree once offset and tinted, then normally — is a deviation, not
a port, and it is the coordinator's call. **Not attempted here**, and said so rather than rushed.

## 8. Against the references, honestly

**Screen one vs `first.jpg`**: same camera, same scene — the rowboat with the bloody sword, the reeds,
the mooring posts, the pier and the harbour behind — same logo, same `PRESS ANY KEY` in the same face
and place. The reference is framed lower and closer, with the whaling ship's hull filling the right
half; ours looks slightly down and out over the water. The camera actor's own `FOVAngle` is 90 with
`bConstrainAspectRatio` and `AspectRatio` 1.778, and the view renders at exactly 90, so the field of
view is retail's; what differs is where along `StartCam`'s move track the shot lands, and the harness
cannot hold a point on that track (hand-over 1).

**Screen two vs `second.jpg`**: the city street, the "THE BOLDEST MEASURES ARE THE SAFEST" sign, the
banners, the Outsider's shrine at the top right, the tram rails, the logo, the bar
`CONTINUE | NEW GAME | MISSIONS | LOAD | OPTIONS | QUIT GAME` with CONTINUE selected and masked to its
label, and the DLC row. Three differences remain and all three are named above: **the text has no drop
shadow** (section 7), **MISSIONS has no asterisk** (section 6), and **the DLC row's glyph is the unlit
PC one** (section 5). The camera is again a little further back than the reference's.

## 9. Census

Last frame of the 90-second acceptance run (`-gfxuicensus`, `build/agentDK/run45.log`):

```
GFx UI census (frame): movies open 1 [UI_MainMenu.MainMenu], drawn 1, display objects 208
  (114 sprites, 69 shapes, 8 text fields [0 unbound], 45 bitmap fills), 68 draws, 190 triangles,
  8 glyph batches / 77 glyphs, 6 masks, atlas 36 packed / 2 blank / 0 failed
GFx UI census (frame): machine: 20206 frames advanced, 938 sprites created, 1967 display objects
  placed, 2361 action buffers, 24787722 opcodes (0 unimplemented), 39 script errors
GFx UI census (frame): input: 1 events HE_Handled / 0 HE_NotHandled, 0 key downs, 1 key ups,
  0 chars typed, 0 mouse events, 2 AS2 listeners registered, 1 listener calls
```

**39 script errors**, down from DJ's 41, of two kinds: 27 `flash.display.BitmapData.loadBitmap`
(unchanged, `_common.EmbedImg`) and 12 `GotoLabeledFrame: no frame named 'PC'`. The composition of the
12 changed: DJ's two `AddControllerButtonInstance` errors are gone and seven `_glow_mc` gotos appeared
in their place, which is the DLC button being driven for the first time (section 5).

## 10. Verification

* **The two acceptance screenshots** come from one 90-second run of the **clean gate build**
  `build/agentDK_wtrel` at `-startmap=Dishonored_MainMenu -startmapopen -gfxuimenu -windowed
  -ResX=1280 -ResY=720 -nomovie` with `-savedir=build\agentDK_save -gfxuikey=1200:SpaceBar
  -gfxuishot=1100,3000 -gfxuicensus`. **0 `Critical:` lines.** Frame 1100 is the start screen while
  `CameraActor_24` is the view target; frame 3000 is the main menu on `CameraActor_11`.
* **Regression**: `python resources\tools\run_regression.py --build-dir build/agentDK_wtrel --no-build
  --exe-name DishonoredGame_DKW.exe --log-prefix DKW` -> **`31 ok, 0 failed, 0 skipped, 422s`**
  (`build/agentDK_wtrel/regression/summary.txt`). `d3d9_criticals 0`, `inputtest_criticals 0` and
  `nullrhi_criticals 0` are the ones the matinee change could have moved, and it did not: the mission
  map's own `SeqAct_Interp` now has data too.
* **Clean full Release build** of the snapshot worktree `build/agentDK_wt` (HEAD `24f3fe0` plus this
  package's 17 files), build directory deleted first: **982/982 targets, 0 errors** —
  `DishonoredGame`, `CoreSmoke`, `LayoutProbe`, `GFx3Run` and the rest
  (`build/agentDK/buildwt.log`).
* **Known intermittent**, and it is named rather than hidden: one run in nine
  (`build/agentDK/run43.log`, 22.2 s) hit
  `Assertion failed: !Obj->HasAnyFlags(RF_Unreachable|RF_AsyncLoading)` in
  `UObject::StaticAllocateObject`'s replace-an-existing-object branch — a named object being
  constructed on top of one the collector has already marked. Three symbol-build runs of the same
  command line did not reproduce it. The matinee runtime now creates and destroys `UInterpGroupInst`
  objects on every loop of `SeqAct_Interp_1` (5 s), which is new GC traffic that did not exist before
  this package, so that is where to look first.

## 11. Deviations, stated once

1. **`UInterpData` gains four accessors** (`GetInterpLength`, `GetNbInterpGroups`, `GetInterpGroup`,
   `GetPathBuildTime`) and `UInterpGroup` gains `GetInterpData()`. The first two are retail's own
   (`0x212940`, `0x215000`); the other two are written, because the reference reads `InterpGroups(i)`
   and `Group->GetOuter()` directly at 48 and 9 sites and Arkane's object graph has one more level.
2. **`UInterpData::GetInterpLength` guards `m_Data`**; retail dereferences it unguarded.
3. **`UInterpGroupDirector` lookup always scans.** The reference caches the director group in
   `CachedDirectorGroup` and only rescans outside the game; `FRuntimeMatineeData` has no such field.
4. **The three `eventInterpolation*` calls in `USeqAct_Interp` are removed**; the identical calls in
   `UnActor.cpp`'s `AMatineeActor::PostNetReceive` are left, because that is a net-client path this
   tree cannot reach (`ReplicatedActor` / `ReplicatedActorClass` are reference-only shims). If it ever
   becomes reachable it will `appError` the same way.
5. **The queued class binding drains at `GFxAP_Lowest`**, not `GFxAP_User` (section 3).
6. **`GFxMovieRoot::DoActions` copies the session out of the queue** rather than walking a linked list
   with a session iterator; same guarantee, different container.
7. **PlaceObject3's filter list is parsed and counted but not stored or applied** (section 7).
8. **`-gfxuikey` now holds the key for 30 drawn frames.** It is a bring-up switch, not a port, but the
   old behaviour was not a real key press and it made an operable menu look inoperable.

## 12. Files

Mine (17, one new):
`Engine/{Inc/EngineInterpolationClasses.h, Inc/EngineSequenceClasses.h, Src/UnCamera.cpp,
Src/UnEngine.cpp, Src/UnInterpolation.cpp, Src/UnInterpolationDraw.cpp, Src/UnSequence.cpp}`;
`External/GFx3/{GFxPlayer.h, GFxPlayerData.cpp, GFxPlayerRoot.cpp, GFxPlayerSprite.cpp}`;
`GFxUI/{Inc/CppText/UGFxEvent_FSCommand.h (new), Inc/GFxUIUISequenceClasses.h, Src/gfxuiengine.cpp,
Src/gfxuiexternalinterface.cpp, Src/gfxuinatives.cpp}`; `Launch/Src/LaunchEngineLoop.cpp`;
plus this report and `agentDK_status.csv`.

**Generator output was hand-edited and NOT regenerated.**
`source/Development/Src/GFxUI/Inc/GFxUIUISequenceClasses.h` is produced by
`python resources/tools/symbols/gen_classes_header.py GFxUI --sdk --module-header --sources-cmake`.
I added exactly one line to it — `#include "CppText/UGFxEvent_FSCommand.h"` inside
`UGFxEvent_FSCommand`'s body — and created `Inc/CppText/UGFxEvent_FSCommand.h`. **Re-running the
generator reproduces that line by itself**, because the generator emits the include for any class with
an `Inc/CppText/<Class>.h`, which is how `UGFxFSCmdHandler_Kismet.h` already gets its own. Nothing the
DishonoredGame generator produces was touched, so
`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` was not needed.

Switches this package adds, all read on first use, none a file-scope initialiser:
`-gfxuifscmd` (the fscommand routing census), `-dismatinee` (the matinee and camera census: every
`SeqAct_Interp` with its data, groups, bound actors and track classes, plus the view target, the POV
and the field of view once a second), `-gfxuidlcheck` (the display-list liveness check),
`-gfxuikeyhold=<N>`.

Scratch, not repo tools: `build/agentDK/` — `dkpatch.py` (the CRLF-safe patch helper),
`patch_*.py` (one per defect, each with its measurement in its docstring), `bmp2png.py`,
`xrefs_to.py`, `dec1`..`dec8` (the headless decompiles this rests on), `run1`..`run43` logs, the
screenshots; `build/agentDK_build.cmd`, `build/agentDK_run.py`, `build/agentDK_sync.py`,
`build/agentDK_save/DisMission0.sav` (a copy of agent DJ's synthesised save, so the bar comes up with
all six entries); snapshot worktree `build/agentDK_wt`, build directories `build/agentDK_rel`,
`build/agentDK_sym` (symbols, for the crash stacks) and `build/agentDK_wtrel` (the clean gate build).

IDA: **own copy only**, `build/agentDK_ida/retail2013_agentDK.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions into the retail
or reference trees, nothing deleted under `Dishonored_Latest2026`.

## 13. Hand-overs

1. **The start camera's fly-through is not gated on the key press** (section 1.4).
   `UInterpTrackSoireeControl` is a declaration-only shim with no `UpdateTrack`
   (`DishonoredGameEngineShims.h:13319`) and it is the only track on `StartCam` that could hold the
   matinee. First place to look.
2. **The text drop shadow** (section 7): three `DropShadow` filters on `txt`, parsed and counted,
   not stored or applied. `GRenderer::BlurFilterParams` and `FGFxFilterPixelShader` are the two ends of
   it.
3. **The GC assertion** (section 9), one run in nine, in the replace branch of
   `UObject::StaticAllocateObject`. New matinee object churn is the prime suspect.
4. **The DLC config merge** (section 6) — the one thing between here and `MISSIONS*` and a working
   DLC row.
5. **Mouse input for the interface**: the content registers `Mouse.addListener` with `onMouseUp` and
   `onMouseWheel`; there is no harness path that generates a mouse event.
6. **`_bUsingGamepad`** is never set, so every `*_Multi` platform clip shows its PC face; the reference
   screenshots were taken with a pad.
7. Everything DJ handed over that this package did not touch: `flash.display.BitmapData` for
   `_common.EmbedImg` (27 of the 39 errors), the scene's depth-stencil surface for the UI target,
   `GFx3Dump` with the runtime on, and the AS2 action-buffer ownership.
8. `GTessellator` was **not** ported and, against the reference at 1:1, is still not what is wrong:
   the logo, the splatter frame and the bar chrome are in the right place at the right size in both
   screenshots. What it would buy remains edge antialiasing on the logo's diagonals.
