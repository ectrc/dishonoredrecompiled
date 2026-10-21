# Agent JB (wave 27, PHASE24 JB): main-menu UI sounds

The main menu now plays its UI sounds. The chain is the AS2 `SoundHandler.PlaySound(event, theme)` call to
`_global.gfxProcessSound(null, theme, event)`, then `FGFxSoundEventCallback`, then every `SoundThemes` entry of the movie
under that theme, then `UDisUISoundTheme::ProcessSoundEvent` (script), then `WorldInfo.PostAkEvent(SoundToPlay)` and
`UAkAudioDevice::PostEvent`. Four links were broken, not two:

1. `UGFxMoviePlayer::PreLoad` installed no callbacks (the brief's link).
2. **`GFxMovieRoot::CreateFunction` was a stub that returned `undefined`.** The brief says "CreateFunction works and is
   used at :1211". It was used there, but nothing it installed was callable. The same stub had also silently disabled
   every `UGFxObject::SetFunction` / `ActionScriptSetFunction*` delegate in the game.
3. `UDisGFxMoviePlayerBase::ApplyTweakChanges_Derived` was a comment (the brief's link). Neither `SetTweaks` nor
   `ApplyTweakChanges` was ever reached for a movie player, because `UDisGFxMoviePlayerBase` did not implement the
   `IDisTweaksInterface` slots.
4. **`UGFxAction_OpenMovie::Activated` constructed the player inside the action's outer (the sequence).** Retail
   constructs it inside the action itself. Because of this, the main menu never found its action's
   `m_pTweaks = Twk_UI_MainMenu.Twk_GFxMoviePlayerMainMenu`. This was measured: after fixing links 1-3, the run had 0
   sound themes on `DisGFxMoviePlayerMainMenu_0`, and both `gfxProcessSound` events answered "no theme of 0".

Setup: worktree `build/agentJB_wt`, made at `96cdf21` and moved to `810f13a` (JA) by stash / checkout / stash pop, which
merged cleanly. Build dir `build/agentJB` (RelWithDebInfo), staged as `DishonoredGame_JB.exe`, private IDA copies in
`build/agentJB/idb/` (2012 imagebase 0, 2013 VA = rva + 0x400000). Builds:

- Build 1 stopped on one compile error in `gfxuimovie.cpp` (`pMovie` is an `FPointer`) after compiling the other 1,321
  steps. It was resumed after the move to 810f13a (391 steps, because of JA's headers) and linked. It is counted as
  build 1.
- Build 2 added the action-outer fix.

No fmodel, no real input, no retail exe. Nothing committed. Sync script: `build/agentJB_sync.py` (base `810f13a`;
clean against main `c754fc6`).

## What was ported

| file | what | retail |
|---|---|---|
| `GFxUI/Src/gfxuimovie.cpp` | `PreLoad` tail: three `CreateFunction`s; `SetVariable("_global.gfxProcessSound" / "_global.CLIK_loadCallback" / "_global.CLIK_unloadCallback", SV_Sticky)` | 2012 0x5e3a30 / 2013 0x5a3af0 (AVM1 only; no AVM2 arm in retail) |
| same | `FGFxSoundEventCallback` (holds the `FGFxMovie*`; theme = pArgs[1], event = pArgs[2]; every binding with that name and a theme gets `eventProcessSoundEvent(event, eventGetPC())`) | Call 2013 0x588570 / 2012 0x5cb160 |
| same | `FGFxCLIKObjectEventCallback::Call` (path split from the right through `WidgetPathBindings`, then `SubWidgetBindings` or `WidgetBindings`, then `CreateValueAddRef` in the bound class) + OnLoad (sets `bWidgetsInitializedThisFrame`, `WidgetInitialized`) / OnUnload (`WidgetUnloaded`) | 2012 0x5e6800 / 0x5cf480 / 0x5cf550; 2013 0x595250 / 0x58e6c0 / 0x58e7c0 |
| `External/GFx3/GFxPlayerRoot.cpp` | `GFxMovieRoot::CreateFunction`: a `GASUserDefinedFunctionObject` (Function prototype, the handler AddRef'd) wrapped in a value and converted to a `GFxValue`. The object's `Invoke`: `this` plus the arguments as GFxValues (`pArgsWithThisRef[0]` = this, `pArgs` = the rest or null), the handler's `Call`, and a non-undefined return value written to the call's result. `Finalize_GC` releases the handler | 2012 0x9af5c0, ctor 0x9af0d0, Invoke 0x9b0ce0, InvokeImpl 0x9b0a60, Finalize_GC 0x9af180 (2013 0xda85e0 / 0xda8100 / 0xda9d50 / 0xda9ad0 / 0xda8190) |
| `External/GFx3/GFxAS2Object.h` | `GASFunctionObject::Invoke` made virtual (retail's is a virtual of the function-object hierarchy) | |
| `DishonoredGame/Src/disgfxmovieplayerbase.cpp`, `Inc/CppText/UDisGFxMoviePlayerBase.h` | `ApplyTweakChanges_Derived`: `SoundThemes.Empty()`, then for each `m_SoundThemes` binding add (name, Theme) and (name, ExternalTheme) when non-null; `GetUObjectInterfaceDisTweaksInterface` (this), `GetTweaks_Derived` / `SetTweaks_Derived` (`m_pTweaks`) | 2012 0x808a20, 0x81c4b0, vtable 0xd528c8 (folded getter), 0x7f6030 |
| `DishonoredGame/Src/disgfxmovieplayermainmenu.cpp` | the PreLoad prologue: `SetTweaks(Action->m_pTweaks)` when the player has none and the action has some, otherwise `ApplyTweakChanges()` | `UDisGFxMoviePlayerBase::PreLoad` 2012 0x822820 / 2013 0x7b9590 |
| `GFxUI/Src/gfxuikismet.cpp` | `UGFxAction_OpenMovie::Activated`: `StaticConstructObject(MoviePlayerClass, this)` | 2012 0x5e51a0 |
| `AkAudio/Src/akaudiodevice.cpp` | `-jbdiag` line in `PostEvent(UAkEvent*)` | diagnostic |

The script half needed no port. A `-jbdiag` dump of the bytecode references of
`DishonoredGame.DisUISoundTheme:ProcessSoundEvent` (398 bytes) names:

- `AkSoundEventBindings`, `FilteredEvents`, `LastSoundEventIndex`
- `LastSoundEventTime` / `WorldInfo.RealTimeSeconds` (the repeat filter)
- `WorldInfo.GetWorldInfo`
- one call: **`Engine.Actor:PostAkEvent`** on the WorldInfo

That call is `AActor::execPostAkEvent` -> `AActor::PostAkEvent` (2013 0x2cd730), which is bodied (IH). The banks:
`UI_M_OpenMainMenu`, `UI_Validation` and `UI_ChangeMainTab` live in `Snd_UI_Ingame`. Their voices have media and
output (the voice-out lines below), so IQ's bank media flag fix covers them.

## Diagnostics

- `-jbdiag` prints:
  - every `ApplyTweakChanges_Derived` with the bound themes;
  - every PreLoad install and the three function value types (6 = VT_Object; it would be 0 with the old stub);
  - every `gfxProcessSound` with its movie, theme and event, plus the theme it reaches or "no theme of N";
  - the one-time bytecode-reference dump of the theme's `ProcessSoundEvent`;
  - every CLIK callback;
  - every `PostEvent(UAkEvent*)` with its playing id.
- `-jbfirstframe0`: retail's `Load(FullName, 0)`. This tree has loaded with `bInitFirstFrame` TRUE since agent BE; see
  the deviations section.
- `build/agentJB/tools/ui_sound_table.py <log> <wav>` builds the event table. It reads the -akwav header by hand,
  because a run that is quit keeps zero RIFF sizes. One buffer tick is 1024 frames from the start of the capture,
  checked against the music: posted at tick 265, first audible sample at tick 342.8, and the voice-out line shows the
  music's stream starting at -62.9 dBFS.

## Measurements

**Options stage (run_regression --only options, -fmdrive), build 2, run JB2 with `-jbdiag -akdiag -akwav`:**
- 4 `gfxProcessSound` calls (M_OpenMainMenu at log 16.62 s, ChangeMainTab at log 32.60 s; each reaches both "Default" themes,
  `Twk_GFxMoviePlayerMainMenu:DisUISoundTheme_0` and `UISoundTheme_Common`, as retail binds Theme and ExternalTheme).
- 2 non-zero playing ids: UI_M_OpenMainMenu id 4 and UI_ChangeMainTab id 5.
- `options_as2_errors` 39 (bound 41), criticals 0, 6/6 steps.

The options script only drives four AS2 entry points by `invoke`, so it cannot produce 5 sound events. **The brief's
"≥ 5 on the options stage" is not reachable with that schedule.** The ≥ 5 is shown on a key-driven menu run instead.

**Menu run JB4** (`game_input.py launch`, `-windowed -jbdiag -akdiag -akwav`): keys Enter, Enter, Escape, Enter,
Escape, Enter, Escape, Down, Up. 7 UI AkEvents were posted with non-zero playing ids, and all 7 are audible in the
capture:

| # | event | AkEvent | playing id | tick | before 1 s dBFS | after 250 ms dBFS | Δ dB | Δ high-passed dB |
|---|---|---|---|---|---|---|---|---|
| 1 | M_OpenMainMenu | Snd_UI_Ingame.UI_M_OpenMainMenu | 4 | 715 | -28.8 | -28.9 | -0.0 | +13.9 |
| 2 | validation | Snd_UI_Ingame.UI_Validation | 5 | 860 | -32.9 | -22.7 | +10.3 | +22.2 |
| 3 | M_OpenMainMenu | UI_M_OpenMainMenu | 7 | 982 | -35.8 | -32.8 | +3.0 | +19.3 |
| 4 | validation | UI_Validation | 8 | 1106 | -34.3 | -23.0 | +11.3 | +28.8 |
| 5 | M_OpenMainMenu | UI_M_OpenMainMenu | 10 | 1229 | -39.8 | -33.2 | +6.6 | +20.8 |
| 6 | validation | UI_Validation | 11 | 1352 | -33.1 | -23.0 | +10.1 | +22.7 |
| 7 | M_OpenMainMenu | UI_M_OpenMainMenu | 13 | 1475 | -37.8 | -33.3 | +4.5 | +20.6 |

"High-passed" is the RMS of the first difference, which tilts the measure towards the clicks and away from the
low-heavy menu music. Broadband, UI_M_OpenMainMenu's swell starts about 0.45 s after its tick. In JB2 it reaches
-19.8 dBFS against a -33.9 music floor; the voice-out line gives -21.5 dBFS out. UI_ChangeMainTab (JB2) is a 312 ms
click at -42.9 dBFS out, under the music broadband (-1.7 dB) and +17.2 dB high-passed.

- Down / Up / mouse hover produced no `Move` sound. The content has 9 `PlaySound('Move')` sites.
- Exits were clean in JB3, JB4 and JB5 (`Exiting.`, 0 criticals).

**CLIK callbacks:** installed and never called, by default or with `-jbfirstframe0` (JB5: 0 callbacks, 7 posts, 28 AS2
errors against JB4's 30). The main menu, options and loading movies never reference `CLIK_loadCallback` (agentDJ's
`mm_init/om_init/lg_init` dumps: 0 occurrences). So the bInitFirstFrame difference has no effect on these movies.

**Gate on build 2** (`JBg_`, d3d9, inputtest, newgame, options):
- d3d9, inputtest and newgame: 24/24 ok, with `unported_natives` 0, `probe_natives` 3, `inputtest_moved` 1018.8,
  `newgame_pawns` 29, criticals 0.
- options: **failed once**. A render-thread `Assertion failed: VertexBuffer` (`D3D9Commands.cpp:59`) in
  `FVertexFactory::SetVertexShadowMap` during `RenderLights` of the menu map, 1.7 s after it loaded and before any
  menu step; 5 criticals.
- Options passed in the other four runs of the same exe: JB1 (build 1), JB2, JBo1 and JBo2. Each had 0 criticals, 39
  AS2 errors and 6/6 steps.
- The stack contains no GFx, tweaks or Wwise frame. The same exe passed 4 of 5 runs. No other agent's log in
  `Logs/` contains the assert. It is reported as a flake to watch, not as fixed.

## Named deviations

- `-jbfirstframe0`: retail loads with bInitFirstFrame 0. The default stays TRUE (BE's choice), because these movies
  never call the CLIK callbacks, and I had no third build to flip the default and re-gate. Switching to retail's 0 is
  one line.
- `GASUserDefinedFunctionObject::Invoke`: retail calls through a `super` object and resets the super's alternate
  prototype afterwards. This tree's `GASSuperObject` keeps the real `this`, and that is what the handler receives.
- The callbacks read their string arguments as wide strings, as retail does, or as UTF-8 when our `ASValue2GFxValue`
  hands over UTF-8 (`GFxArgString`).
- PreLoad keeps three separate `GFxValue`s instead of retail's one reused `func`. Our `ASValue2GFxValue` does not
  release a managed value it overwrites, so reuse would leak a function object.
- `RefCount = 1` is set explicitly in the callbacks' constructors, as retail's inlined `GRefCountImplCore` constructor
  does. It is redundant with the runtime header and needed with the absent one.

## Where the brief was wrong (with the measurement)

1. **"CreateFunction works and is used at :1211"**: `GFxMovieRoot::CreateFunction` was
   `v->SetUndefined()` ("Reported to agent BB rather than guessed"). PreLoad's function values now read type 6
   (VT_Object). With the stub they would be undefined, and the content guards on
   `if (!_global.gfxProcessSound) return`. Side effect: every `UGFxObject::SetFunction` / `ActionScriptSetFunction*`
   delegate is now live for the first time. The gate shows no regression, but any script that set a delegate now
   runs it.
2. **"ApplyTweakChanges_Derived filling SoundThemes" was not enough.** The main menu's player was constructed in the
   sequence, not in the action (`gfxuikismet.cpp` against 2012 0x5e51a0). JB1 measured 0 themes on the main menu and
   2 on pGlobal and pHUD. After the fix, JB2 has 2 themes on the main menu.
3. **"≥ 5 gfxProcessSound events on the options stage"**: the stage's -fmdrive schedule reaches 2 sound-producing AS2
   paths (4 theme dispatches, 2 posts). The ≥ 5 was measured on a key-driven menu run (JB4: 14 dispatches, 7 posts).
4. **"UI bank: Bank_UI or wherever"**: the events are `Snd_UI_Ingame.*` AkEvents, posted on `WorldInfo_0`. Their media
   play (voice-out lines: id 4 -21.5 dBFS out, id 5 -42.9 dBFS out).
5. The CLIK load/unload callbacks have no effect on the menu: the content never calls them.

## Open

- Retail also plays `Move` on focus changes. Down / Up / hover produced none here. Either our key injection does not
  move the menu's focus, or the focus change does not reach `SoundHandler`. Not traced in this package.
- The menu-map render-thread `VertexBuffer` assert (1 in 5 options runs).
- Flip `-jbfirstframe0` to the default, after a gate.
