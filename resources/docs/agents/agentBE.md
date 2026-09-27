# Agent BE report — the GFxUI native layer: 159 natives, the external-interface path, the menu leaves (2026-09-27)

Package "**BE**" of wave 6 (`PHASE8.md`). Status rows: `agentBE_status.csv` (172 rows). Nothing committed,
nothing staged. Own build dir `build/agentBE`, own snapshot worktree `build/agentBE_wt`, own IDA copies
`resources/docs/idb/{shipping2012,retail2013}_agentBE.i64`, headless decompiles only (529 functions into
`build/agentBE/dec2012/`), no IDA or FModel MCP tools, no junctions, nothing deleted under
`Dishonored_Latest2026`.

## The answer in four lines

* **All 132 GFxUI exec bodies are ported** — every `DECLARE_FUNCTION` the module registers, which is agent
  AW's 125 natives plus 7 the native dump does not index. `GFxUINativeStubs.cpp` now contains zero exec
  bodies. That is `UGFxObject` (64), `UGFxMoviePlayer` (49), `UGFxInteraction` (4),
  `UGFxDataStoreSubscriber` (7+1) and `UGFxFSCmdHandler_Kismet` (1), with the ~90 C++ methods behind them.
* **27 of the DishonoredGame menu natives are ported**: the 12 unported `UDisGFxMoviePlayerMainMenu`, the 7
  `UDisGFxMoviePlayerMenuBase` and the 8 `UDisGFxMoviePlayerBase` (agents AJ and AU had the other 6/0/1
  already). New Game is now exactly what agent AW measured: `m_Difficulty` then
  `ConsoleCommand(m_NewGameCommand)`, the same `ce ChangeLvl_StartNewGame` that `-newgame` issues.
* **The external-interface path runs and is tested.** `-gfxcallbacktest` drives the real
  `FGFxExternalInterface::Callback` four ways and logs
  `GFx callback census: 4 calls, 4 of 4 cases passed, external interface path OK`, including a real return
  value (`GetChangelist` -> 334700) coming back through `SetExternalInterfaceRetVal` and two GFxValues
  marshalled into one of this package's own ported natives.
* **The regression harness is unchanged by this package**: 20 ok / 8 failed / 2 skipped, and clean HEAD from
  the same worktree, same build dir, same machine fails the identical 8 with identical values. Section 6
  has the comparison.

## 1. What is in the tree

| file | what |
|---|---|
| `GFxUI/Inc/gfxui_gfx3.h` | **the one header of assumptions.** The GFx 3.3.89 surface the layer calls, plus the engine-side seam (`FGFxMovie`, the `FGFxEngine` methods this layer uses, the two Callback classes, `FAutoGFxValueArray`). One switch, `DISHONORED_GFXUI_GFX3_RUNTIME`, swaps the whole GFx half for `#include "GFx3.h"`. |
| `GFxUI/Src/gfxuimovie.cpp` | `UGFxObject` + `UGFxMoviePlayer`: 113 exec bodies and their C++ methods. 3,000 lines. Retail attributes exactly these to `gfxui/src/gfxuimovie.cpp`. |
| `GFxUI/Src/gfxuinatives.cpp` | the remaining 19: `UGFxInteraction`, `UGFxDataStoreSubscriber`, `UGFxFSCmdHandler_Kismet`. Retail splits them over `gfxuiinteraction.cpp` and `gfxuidatastore.cpp`; they are in one unit here so this package and agent BB's never edit the same file. |
| `GFxUI/Src/gfxuiexternalinterface.cpp` | `FGFxExternalInterface::Callback`, `FGFxFSCommandHandler::Callback`, `FGFxEngine::ConvertUPropToGFx` / `ConvertGFxToUProp` / `ReplaceCharsInFString`, and the self-test. Retail has these in `gfxuiengine.cpp`, which is agent BB's unit — **fold them back at any time**. |
| `GFxUI/Src/gfxuigfx3absent.cpp` | the no-runtime bodies: `GFxValue`'s managed-reference protocol, the 25 `ObjectInterface` methods, `GGFxEngine = NULL`, the `FGFxEngine` seam. `#if !DISHONORED_GFXUI_GFX3_RUNTIME`, so it disappears the day a runtime exists. |
| `GFxUI/Inc/CppText/UGFx{Object,MoviePlayer,Interaction,DataStoreSubscriber,FSCmdHandler_Kismet}.h` | the C++ method declarations, through the generator's own cpptext mechanism. |
| `GFxUI/GFxUINativeStubs.ported.agentBE.txt` | 132 lines. |
| `DishonoredGame/Src/disgfxmovieplayer{base,menubase,mainmenu}.cpp` | appended to (agent AC's trivial natives are untouched): the 27 menu natives, the settings-tree and load-list AS2 leaves, `TryBindKey`, `PostStart`. |
| `DishonoredGame/DishonoredGameNativeStubs.ported.agentBE.txt` | 27 lines. |

Two shared files changed by one line each, both in this package's own area:
`GFxUI/Sources.cmake` drops `Src/gfxuimovie.cpp` from `GFxUI_EXCLUDE`, and `DishonoredGame/Sources.cmake`
drops `Src/disgfxmovieplayermenubase.cpp`. **Neither Sources.cmake is written by
`gen_classes_header.py --sdk`** (I checked: it leaves both alone), so the exclude lists are hand-maintained
and a regeneration will not re-add them.

## 2. How the exec wrappers were derived, and the two traps

The wrappers are **derived, not guessed**. `resources/docs/types/script_classes_2013.json` carries every
native function's parameter list with its property kind and `Parm` / `OptionalParm` / `OutParm` /
`ReturnParm` flags — which is exactly what UnrealHeaderTool used to emit the `DECLARE_FUNCTION` bodies.
`build/agentBE/gen_execs.py` turns that into the right `P_GET_*` sequence and the call, and prints the
reference GFx 4 GFxUI's own wrapper beside it where it has one. **98 of the 113 root wrappers matched the
reference exactly**; the differences, all real:

* the `ActionScript*` family (11 of them) is hand-written in both: the arguments are the *caller's* parameter
  list read out of `Stack.Locals`, so there is no fixed signature. Ported from the retail bodies
  (2012 `0x5e8350` .. `0x5e8830`), which are the same `ExecuteActionScript` template the reference has.
* `execSetDisplayMatrix`, `execSetDisplayMatrix3D`, `execSetElementDisplayMatrix`,
  `execActionScriptSetFunction{,On}` — hand-written for the same reason (the delegate parameter is read out
  of the caller's frame).
* optional-parameter defaults: the script dump does not carry them (the default expression lives in the
  caller's bytecode and `INIT_OPTX_EVAL` runs it), so the macro's fallback only matters when a caller skipped
  the parameter with no expression. Taken from the reference where it has one (`Depth = -1`).

**Trap 1, and it would have been silent: Dishonored's `ASType` has no `AS_Int`.** The reference GFx 4 GFxUI
inserts `AS_Int` at 3 and shifts `AS_String`/`AS_Boolean` to 4/5, and its `ASValue` carries an extra `int`
member. Retail's is `AS_Undefined, AS_Null, AS_Number, AS_String, AS_Boolean` — confirmed twice, from
`script_classes_2013.json` and from `UGFxMoviePlayer::GetVariable` (2012 `0x5c0b70`) mapping `VT_Boolean` to
4 and `VT_String` to 3. Taking the reference's would have swapped strings and booleans in every `FASValue`
that crosses the boundary. The generator already emits the right enum into `GFxUIEngineShims.h`; this layer
uses that and declares nothing.

**Trap 2: GFx 3.3's `GRenderer::Cxform` is `M_[4][2]`, GFx 4's is `M[2][4]`.** The multiply is column 0 and
the add is column 1, proved by `UGFxObject::execGetColorTransform` (2012 `0x5b91a0`) reading
`M_[0][0], M_[1][0], M_[2][0], M_[3][0]` into `Multiply.RGBA`. Copying the reference's indexing would have
scrambled every UI colour transform.

## 3. The switch, and why it is not `DISHONORED_WITH_GFX3`

Agent BB's `DISHONORED_WITH_GFX3` means "the reconstructed GFx 3.3 headers and the renderer/file/image seam
are in the build", which is already true and default ON. It does **not** mean "something implements
`GFxValue::ObjectInterface` and `GFxMovieView`" — nothing does: `External/GFx3` is headers plus layout
assertions plus `GFx3RuntimeStubs.cpp`, and that file defines not one `GFxValue::ObjectInterface` method
(package BC is writing the machine that will). So a layer that included `GFx3.h` today would compile and
then fail to link.

This package therefore has its own switch, `DISHONORED_GFXUI_GFX3_RUNTIME`, default 0, and its own bodies
for the GFx entry points. It is **not** the stub backend agent AW rejected (`gfx_decision.md` 3, option A0):
it does not pretend to be a player. `GGFxEngine` is NULL, so every wrapped `GFxValue` stays `VT_Undefined`
and every ported body takes the same early-out retail takes for a closed movie. Nothing in
`gfxuigfx3absent.cpp` is ever reached at run time; it exists so the ported code links, and it disappears
when the switch flips.

**The module builds with the switch both ways**, which is the acceptance:

| configuration | result |
|---|---|
| `DISHONORED_WITH_GFX3=0` (snapshot, HEAD has no `External/GFx3`) | `GFxUI.lib` and `DishonoredGame.exe` build and link; the game runs |
| `DISHONORED_WITH_GFX3=1` (shared tree, BB's headers + the four seam units) | `GFxUI.lib` builds (`build/agentBE/build_gfx3.log`) |

### What the switch-over needs — three things, for agents BB and BC

1. `DISHONORED_GFXUI_GFX3_RUNTIME=1` makes `gfxui_gfx3.h` include `GFx3.h` and drops
   `gfxuigfx3absent.cpp`. Do it only once the ObjectInterface and GFxMovieView bodies exist.
2. **Three defects in `External/GFx3/GFxValue.h`, all checkable against the 2012 PDB enum dump
   (`resources/docs/types/all_types.h`) and the retail decompiles in `build/agentBE/dec2012/`:**
   * `VTC_ConvertBit` is **0x80**, `VTC_ManagedBit` **0x40** and `VTC_TypeMask` **0x8F**, not 0x08 / 0x10 /
     0x0F. Every retail body masks the type with `& 0x8F` and tests the managed bit with `& 0x40` —
     `UGFxObject::GetElementFloat` (2012 `0x5ba1a0`) is the shortest example, and
     `all_types.h`'s `GFxValue::ValueTypeControl` says the same. The convertible types follow:
     `VT_ConvertNumber` is **0x83**, not 0x0B. As it stands, `IsManagedValue()` tests the wrong bit, so
     `GFxValue`'s destructor will release objects it does not own and leak the ones it does.
   * `GFxValue::ObjectInterface::ObjVisitor::Visit` takes `const GFxValue*`, not `const GFxValue&`
     (`all_types.h`, `GFxValue::ObjectInterface::ObjVisitor_vtbl`).
   * `DisplayInfo::Flags` has no `V_projMatrix3D`; the PDB's set ends at `V_viewMatrix3D = 0x2000`, and
     0x800 / 0x1000 are `V_perspFOV` / `V_perspMatrix3D`.
   Two more, for the same header's benefit: `GFxMovieView::AlignType` is the **nine**-value viewport
   alignment (`Align_Center` .. `Align_BottomRight`), not the four-value Left/Right/Center/Justify enum IDA
   prints under that name — that one is `GFxTranslator::LineFormatDesc`'s text alignment, which IDA's type
   dedup collapsed onto it; `UGFxMoviePlayer::execSetAlignment` (2012 `0x5bdde0`) casts the nine-value
   script `GFxAlign` byte straight to it. And GFx is **not** built with UE3's `/Zp4`: `DisplayInfo` puts
   `bool Visible` at 48 and the next `double` at 56 and is 232 bytes, so everything GFx needs `pack(8)`
   (BB has this right; this header does the same).
3. The engine seam (`FGFxMovie`, `FGFxEngine`, the two Callbacks, `FAutoGFxValueArray`) belongs in
   `GFxUI/Inc/gfxuiengine.h`, which is where retail declares it and which is still a comment-only skeleton.
   When BB writes it, delete the `#else` half of `gfxui_gfx3.h`'s seam block.

**`FGFxEngine` signatures worth having right**, because the obvious guesses are wrong (all from the 2012 PDB):
`StartScene(FGFxMovie*, UTextureRenderTarget2D*, UBOOL, UBOOL)` returns **void**;
`SetMovieSize(FGFxMovie*)` takes **one** argument (the viewport rectangle is set on the view, not passed);
`FlushPlayerInput(TSet<INT>*)` takes the capture-key set, and NULL means "flush everything";
`GetTopmostMovie()` takes none; `LoadMovie(const TCHAR*, UBOOL)`.

## 4. Things the retail bodies do that a reasonable guess would not

Recorded because each is a real behaviour and each would be a bug if ported from the reference or from
intuition:

* `UGFxMoviePlayer::SetPause` does **not** call `GFxMovieView::SetPause`. It sets `FGFxMovie::fUpdate`, our
  own flag — the engine stops advancing the movie and keeps drawing it. That is why a paused Dishonored menu
  is still on screen (2012 `0x5b7090`).
* `Advance` passes a frame-catch-up count of **2** and then calls `PostAdvance` **through the vtable**, so
  the Dishonored menus' override runs (2012 `0x5b9e30`).
* `SetVariable*` all use `SV_Sticky`, so a value set from C++ survives the timeline recreating the object it
  lives on (2012 `0x5c1180`).
* `SetTimingMode` rebases `FGFxMovie::LastTime` on the new clock; without that the next `Advance` is handed
  the whole gap between `GCurrentTime` and the world's time (2012 `0x5b70b0`).
* `SetViewport` reads the current `GViewport` back first and replaces only the rectangle, keeping the buffer
  size, scissor, scale and aspect ratio the renderer set, then sets `fViewportSet` so
  `FGFxEngine::ReevaluateSizes` stops overwriting it (2012 `0x5b9d80`).
* `SetPriority` records the new priority and then calls `InsertMovie(pMovie, 2)` — the **literal 2**, not the
  new priority. Left as retail has it and flagged in the source; changing it would reorder the UI draw
  (2012 `0x5e4850`).
* `Cleanup()` is nothing but "close and unload if a movie is open" (2012 `0x5b7010`); it is `FinishDestroy`'s
  whole body.
* `UGFxObject::execActionScriptArray` is nine bytes: a tail call into `execActionScriptObject`
  (2012 `0x5e8830`).
* `UGFxObject::GetObject` really is compiled as `GetObjectW`: on a UNICODE Win32 build `windows.h` rewrites
  the name, and `?GetObjectW@UGFxObject@@...` is the symbol the PDB carries (2012 `0x5e6e30`). The port keeps
  the same spelling so the declaration and every call site are rewritten together, exactly as in retail.
* an AS2 method name beginning with `_` is the **data-store protocol**, not a script call:
  `FGFxExternalInterface::Callback` routes it to `UGFxMoviePlayer::ProcessDataStoreCall` before it looks for
  a `UFunction` (2013 `0x58d510`).
* the method-name lookup is `FNAME_Find`, never `FNAME_Add` — an AS2 call for a method that does not exist
  must not grow the name table.

## 5. The C++ data models behind the menu — what is ported, and the three declarations that block the rest

Agent AW's positive finding was that these models are ours and not ActionScript, and that a layer "replaces
only the `CreateGFx*` leaves". **The leaves are ported and complete**, because they need only the GFx object
interface this package now provides. They also pin down the contract with the cooked asset, which is worth
having written down:

| retail function | 2012 rva | the AS2 members / calls it produces |
|---|---|---|
| `CreateGFxSetting` | `0x80f920` | `Setting_Id`, `Setting_Name`, `Setting_Value`, `Setting_Minimum`, `Setting_Maximum`, `Setting_Increment`, `bDropList`, `GamepadBindingMenu`, `VideoSettings`, `GammaMenu`, `DeviceSelectionMenu`, and for a drop list `ProfileSettingIDs` / `ProfileSettingValues` |
| `CreateGFxSubCategory` | `0x813db0` | `SubCategory_Name`, `KeyboardBindingMenu`, `Setting_List` |
| `CreateGFxCategory` | `0x815a50` | `Category_Name`, `SubCategories`, `Setting_List` |
| `ShowSettingsCategoryList` | `0x817250` | `_root.options_mc.FillCategories(categories)` |
| `FillLoadGameMenu` / `CreateGFxLoadGameList` | `0x815830` / `0x813930` | an array of `{ chapterName, saveDate, itemThumb }` into `_root.loadGame_mc.SetLoadGame`, with `m_LoadGameSlots` as the parallel array of real save slots |
| `TryBindKey` | `0x8103b0` | setting ids 29..62 as an array into `_root.options_mc.OnKeyAssigned` |
| `UDisGFxMoviePlayerBase::OnFocusGained` | `0x7f5ce0` | `_root.UIBase.OnFocusGained()` |
| `UDisGFxMoviePlayerMainMenu::PostStart` | `0x821e00` | `_root.startScreen_mc.Close()`, then `_root.mainMenu_mc.Open(bHasSaveGame, bHasSaveGame, true, bSaveLoadEnabled)` |
| `BackToStartScreen` | `0x820e00` | `_root.mainMenu_mc.Close()` then `_root.startScreen_mc.Open()` |

**What is above the leaves is blocked on three declarations, each small and each in a class this package does
not own.** This is the concrete handover, and it is why the load list and the settings values are empty
rather than wrong:

1. **the save system.** `FDisSaveGame` is not declared anywhere in this tree, and `UDishonoredEngine` has no
   reflected save-slot array, so there is no input to enumerate. Needed:
   `UDishonoredEngine::GetSaveGame` (2012 `0x6425f0`), `HasSaveGame` (`0x642540`), `DeleteSaveGame`
   (`0x642660`), `FindMapConfigFromFriendlyName` (`0x642900`) — four lines in
   `DishonoredGame/Inc/CppText/UDishonoredEngine.h`, plus the struct. This gates the load list, `Continue`,
   `Req_CanLoadGame`, `OnDeleteSaveConfirm` and `PostStart`'s `bHasSaveGame`. It is PLAN.md milestone 6.
2. **the profile settings.** `UArkProfileSettings` is a shim in `DishonoredGameEngineShims.h` with one
   native, so `FindBindableKey`, `GetMissionData` and the per-setting value/min/max/increment reads do not
   exist. This gates every `Setting_Value`, `OnSettingChange`, `OnResetOptions`, `TryBindKey`'s key
   resolution, the missions screen and `OpenMissionStats`. `FillSettingsCategoryList` (2012 `0x81ba20`) is
   1.7 KB of hard-coded tree over the menu-base tweaks' `m_SettingsCategory_*` strings and is pure plumbing
   once a value can be read; it is **not** ported, deliberately, because a tree that cannot read a value is
   a thousand lines of nothing.
3. **the message box.** `UDisGlobalUIManager::ShowMessageBox` (2012 `0x8aeee0`), `AddMessageBoxTimer`
   (`0x8aef00`) and `HideMessageBox` (`0x8aef20`) are not declared. `ShowMessageBox` /
   `HideMessageBox` / `AddMessageBoxTimer` therefore keep the `m_MsgBoxID` bookkeeping, which is this class's
   own and is what `OnMessageBoxResult` matches on, and name the call they cannot make.

Every such gap is one `DISHONORED(bringup)` line naming the exact retail call and its rva. There are 23 of
them across the three menu units; `grep -n "DISHONORED(bringup)"` over them is the to-do list.

## 6. Verification

**The natives.** `GFxUINativeStubs.cpp` contains 0 exec bodies after regeneration (was 132). A cross-check
script confirmed the set my units define is exactly the set the stub file defined: nothing missing, nothing
duplicated. `DishonoredGameNativeStubs.cpp` drops the 27 menu natives.

**The build.** `GFxUI.lib` and `Binaries/Win32/DishonoredGame.exe` build and link in the snapshot
(`build/agentBE/build{8,9}.log`), and `GFxUI.lib` builds in the shared tree with
`DISHONORED_WITH_GFX3=1` (`build_gfx3.log`). The shared tree's **exe** does not link right now, and the
unresolved symbols are all package BF's in-flight attributes work
(`ADishonoredPawn::execTakeFallingDamage_Native`, `execTakeDamage`,
`IDisAttributesInterface::GetAttributes`); every symbol package BE owns resolves there.

**The external-interface test.** `-gfxcallbacktest` (and `GFXCALLBACK <Object> <Function> [args]` from a
console) drives the real `FGFxExternalInterface::Callback` against a `GFxMovieView` test double whose only
job is to answer `GetUserData` and record `SetExternalInterfaceRetVal`. From
`Dishonored_Latest2026/DishonoredGame/Logs/agentBE_cb.log`:

```
GFx callback Default__DishonoredGameInfo.ThisIsNotAnExternalInterfaceMethod: resolved 0, 0 args, returned (void)
GFx callback Default__DishonoredGameInfo.GetChangelist: resolved 1, 0 args, returned 334700
GFx callback GFxMoviePlayer_0.SetVariableString: resolved 1, 2 args, returned (void)
GFx callback GFxMoviePlayer_0.SetVariableBool: resolved 1, 2 args, returned (void)
GFx callback census: 4 calls, 4 of 4 cases passed, external interface path OK
```

Case 2 proves the return value converts back and reaches `SetExternalInterfaceRetVal` (334700 is the real
changelist). Cases 3 and 4 prove two `GFxValue`s marshal into a `UFunction`'s parameters — a string and a
string, then a string and a number that has to become a UnrealScript bool — and that `ProcessEvent` reaches
one of this package's own ported natives. Case 1 proves an unknown name is dropped without creating an
`FName` and without crashing.

**The regression harness: 20 ok, 8 failed, 2 skipped — and clean HEAD fails the identical 8 with identical
values.** `coresmoke` 2/2 and `layout` 7/7 pass; the 8 failures are all in the `d3d9` and `inputtest` stages:

| metric | with BE | clean HEAD, same worktree/build dir/machine |
|---|---|---|
| `d3d9_frames` | 0 | 0 |
| `d3d9_draws_per_frame` | 0 | 0 |
| `d3d9_draw_elements` | 0 | 0 |
| `d3d9_visible_prims` | 0 | 0 |
| `inputtest_moved` | -1 | -1 |
| `inputtest_peak_speed` | -1 | -1 |
| `physics_actors` | 2 | 2 |
| `physics_static_shapes` | 0 | 0 |

Both builds exit with code 3 about 7.4 s in, having loaded `DishonoredGameFull_P` and never reached
`L_Tower_P`: `DishonoredTickStartMap` needs six `UGameEngine::Tick` calls to issue its `OPEN`, and the
process dies before that. The abort is in the **renderer, on the rendering thread** —
`FHeightFogShaderParameters::Set` -> `SetShaderValue` -> `Assertion failed: Parameter.IsInitialized()`
(`Engine/Inc/ShaderManager.h:379`, via `TBasePassVertexShader::SetParameters` drawing a skeletal mesh) —
which is an unbound base-pass shader parameter, i.e. a shader-cache/declaration mismatch. Package BE adds no
shader, no render code and nothing on that stack. The likely cause is the shared state of
`Dishonored_Latest2026`: the log also says `Skipping saving the shader cache as another instance of the game
is running`, six agents share that install, and package BD is adding 136 shader types to it right now. The
assertion is intermittent (it fired in one `inputtest` run of mine and not the next, and not at all in
HEAD's), the eight metric values are not.

**Reproduce the HEAD comparison**: in `build/agentBE_wt`, `git checkout -- .`, delete the seven untracked
files package BE adds, rebuild `DishonoredGame` into `build/agentBE`, and run
`python resources\tools\run_regression.py --build-dir build\agentBE --no-build --exe-name DishonoredGameBE.exe --log-prefix regressionBEhead --only d3d9,inputtest`.

**A harness note for the coordinator**: `run_regression.py`'s defaults (`--exe-name DishonoredGame_AX.exe`,
`--log-prefix regression`) are shared, so two agents running it at once collide on
`Dishonored_Latest2026\DishonoredGame\Logs\regression_*.log` with a `PermissionError` and on the staged exe.
Passing `--exe-name` and `--log-prefix` fixes it; consider defaulting them from `--build-dir`.

## 7. Assumptions, stated plainly

1. **The GFx 3.3 surface in `gfxui_gfx3.h` is mine, not agent BB's**, for the linkability reason in section 3.
   Every declaration carries its 2012 rva or its `all_types.h` provenance, and six `static_assert`s hold it
   to the PDB sizes (`GFxValue` 16, `GMatrix2D` 24, `GMatrix3D` 64, `Cxform` 32, `GViewport` 52,
   `DisplayInfo` 232, `GFxMovieInfo` 36). It agrees with BB's header everywhere except the five items in
   section 3.2, where the PDB and the decompiles are on this header's side.
2. **`GFxMovieView`'s 73 slots are declared in vtable order** because this layer dispatches virtually. The
   two `Invoke` overloads are declared in the reverse of their memory order, because MSVC lays consecutive
   overloads out backwards — agent AL's PhysX trap, and the reason the order is called out in the source.
3. **`FGFxEngine`'s body is agent BB's.** This layer declares only the members it calls and never defines
   one outside `gfxuigfx3absent.cpp`.
4. **`FGFxExternalInterface::Callback`, `FGFxFSCommandHandler::Callback` and the two property converters are
   this package's**, in a file of their own. Retail has them in `gfxuiengine.cpp`. Agent BB must not define
   them a second time; fold or delete, either is fine.
5. The `UGFxDataStoreSubscriber` bodies walk `Movie->DataStoreBindings` faithfully (the `+264` / `+268` /
   `+44` of the decompiles are `DataStoreBindings.Data` / `.ArrayNum` / `DataSource.ResolvedDataStore`), but
   the **values** need Engine's UI data-store subsystem (`UDataStoreClient`,
   `FUIDataStoreBinding::ResolveMarkup`, `UUIDataStore::Get/SetDataStoreValue`), which this tree has not
   ported: `FUIDataStoreBinding` is a member-only shim struct in `GFxUIEngineShims.h`. Dishonored's menus do
   not use data stores — they use `ExternalInterface` — so nothing on the menu path depends on this.
6. `DishonoredGame`'s generated files were regenerated in the shared tree, which picked up packages BD's and
   BF's in-flight ported lists as well as mine; that is what the coordinator does at merge anyway. The
   snapshot got a **BE-only** stub file, built from HEAD's minus my 27 natives, because a stub file generated
   against the mixed working tree only links against the mixed working tree
   (`build/agentBE/DishonoredGameNativeStubs.snapshot.cpp`).

## 8. Hand-overs

1. **Agent BB** — the five header defects in section 3.2, and `gfxuiengine.h` (section 3.3). Also: the four
   `FGFxEngine` signatures at the end of section 3 are the PDB's, and three of them are not what they look
   like.
2. **Agent BC** — when the AS2 machine can answer `GFxValue::ObjectInterface`, the 25 methods this package
   calls are the whole of what the engine needs from it (agent AW's 81 %), and
   `gfxuigfx3absent.cpp` lists them in one place with their rvas. The first thing worth running against it is
   `UDisGFxMoviePlayerMainMenu::PostStart`: `GetVariable("_root.mainMenu_mc")` then `Invoke("Open", 4 bools)`
   is the whole of opening the main menu.
3. **Whoever takes the save system (milestone 6)** — four declarations plus `FDisSaveGame` turn the load-game
   list, `Continue`, `Req_CanLoadGame` and `PostStart`'s `bHasSaveGame` on; the AS2 side is already written
   and named (section 5.1).
4. **Whoever takes `UArkProfileSettings`** — the settings tree and key binding are waiting on it and on
   nothing else (section 5.2). `FillSettingsCategoryList` is the one retail function of this package left
   unported, on purpose.
5. **Coordinator, at merge** — regenerate **GFxUI** as well as DishonoredGame (`gen_classes_header.py GFxUI
   --sdk`): this package adds five `Inc/CppText` headers to that module and a ported list of 132, and the
   module does not compile without the regeneration. Neither module's `Sources.cmake` is generated, and both
   already carry the one-line exclude removals this package needs. The harness note in section 6 is worth
   folding into `run_regression.py`.
