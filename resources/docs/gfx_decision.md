# The main menu decision (Scaleform GFx) — inventory, three costed options, recommendation

Written 2026-09-26 by agent AW (Phase 3 wave 5, `PHASE7.md` package AW). **Investigation only — nothing
was implemented and no engine file was touched.** Everything below is measured from the two shipped
binaries, the 2012 PDB symbol database and the retail cook; the scripts that produced each number live in
`build/agentAW/` and are named at the point of use. IDA work was headless on my own copies
`resources/docs/idb/{shipping2012,retail2013}_agentAW.i64`; the xref pass and the decompiles ran on the
**2012** database (the PDB names every `libgfx` and `gfxui` function there and `functions.csv` attributes
each one to its library), and 2013 rvas come from `match_2012_2013.csv` or from `agentAF.md`. Where an rva is
2012 it says so.

## 0. The decision, up front

**Take option C now — keep `-newgame` / `-startmap` as the entry point — and schedule option B (our own
menu layer driving Arkane's own menu code) as a small, switch-gated package for the wave after the four
playability packages land. Option A is dead in the form that worked for PhysX, Steamworks and Bink, and
only becomes the best option if the user can obtain a Scaleform GFx 3.3.x licensee SDK, in which case it
wins outright.**

The one-line reason: **GFx is not a dependency we can bind to, it is 1.13 MiB of the retail exe's own
code, and the menu's behaviour is ActionScript 2 bytecode inside the cooked `.gfx` assets** — so there is
no runtime to reconstruct a binding *to*, and a stub backend cannot produce a menu, only a black screen
that never calls back.

## 1. The central negative: there is no GFx DLL, and a stub backend is not like Wwise's

Three of our middleware bindings were reconstructed rather than obtained. Two of them (PhysX, Steamworks,
and before them Bink) worked because **the retail DLL is the runtime**: we only had to describe the API and
build an import library (`middleware.md` 2.1, 2.2, 2.6). Wwise was different — statically linked, no DLL —
and agent AN's answer was *our own headers plus a silent backend* (`agents/agentAN.md` 1, 3).

GFx is statically linked like Wwise. Measured:

| Fact | Evidence |
|---|---|
| Not one shipped DLL contains a single GFx symbol or string | `build/agentAW/scan_gfx_dlls.py` over all 33 DLLs in `Dishonored_Latest2026\Binaries\Win32`: zero hits for `GFxLoader`, `gfxVersion`, `GRenderer`, `Scaleform`, `GFxMovie`, `GASObject`, `libgfx`. `Dishonored.exe` has 8 / 1 / 1 / 202 |
| The runtime is inside the exe's own `.text` | `libgfx` + `libgfx_ime` = **5,635 functions, 1,184,791 bytes = 1.13 MiB**, i.e. **9.7 % of `.text` (0xb9241f)**; `build/agentAW/libgfx_size.py` |
| It is the *same* library in both exes | 4,080 of 5,635 `libgfx` functions are **byte-identical** 2012↔2013 and 5,235 (92.9 %) match at ratio 1.000 (`build/agentAW/libgfx_match.py`), so the 2012 PDB's `libgfx` names describe the retail 2013 runtime exactly |
| Version | GFx **3.3.89**, `Dishonored.exe` rva `0xdf7374` (`middleware.md` 1) |

**Route 1 of the PhysX/Steamworks method therefore does not apply at all.** There is nothing to import.

And the Wwise answer does not transfer either, because the two libraries have opposite jobs:

* Wwise's job during bring-up is to be **inaudible**. A backend that keeps books and answers queries is
  therefore *functionally adequate*: the game's audio code gets consistent answers and proceeds.
* GFx's job is to **run the menu's ActionScript and draw it**. Section 2.4 shows the main menu's own
  navigation is AS2 code inside the `.gfx` asset (`_root.startScreen_mc.Open()`, `mainMenu_mc.Open(...)`)
  and that every button arrives back in C++ through `ExternalInterface.call`. A stub GFx returns
  `undefined` from `GetVariable`, runs no bytecode, and therefore **never calls `OnNewGameConfirm`**. The
  menu map would sit there for ever. A stub buys nothing.

## 2. Entry-point inventory: what the menu actually needs

There are four distinct boundaries. All four have to work for the retail menu to work, and each is a
different amount of work.

### 2.1 Boundary 1 — UnrealScript → `GFxUI` natives (208 natives, all stubbed today)

`natives_2013.csv`, counted by `build/agentAW`-side query:

| Class | natives | what script does with them |
|---|---|---|
| `UGFxObject` | 64 | get/set AS2 members, array elements, display info, colour transform, matrices, text |
| `UGFxMoviePlayer` | 49 | `Start`, `Advance`, `PostAdvance`, `Close`, `SetPause`, `SetPriority`, `SetVariable*`/`GetVariable*` (bool/number/string/object/arrays), `CreateObject`/`CreateArray`, `SetViewport`, `SetAlignment`, `SetExternalTexture`, `SetView3D` |
| `UGFxDataStoreSubscriber` | 7 | UI data-store bindings |
| `UGFxInteraction` | 4 | `GetFocusMovie`, `CloseAllMoviePlayers`, player add/remove |
| `UGFxFSCmdHandler_Kismet` | 1 | Kismet fscommand handler |
| **GFxUI total** | **125** | |
| `UDisGFxMoviePlayerMainMenu` | 18 | **the main menu's buttons** — see 2.4 |
| `UDisGFxMoviePlayerGlobal` | 16 | message-box and storage/save-failure callbacks |
| `UDisGFxMoviePlayerJournal` | 23 | journal |
| `UDisGFxMoviePlayerBase` | 9 | `ShowMessageBox`, `HideMessageBox`, `FormatText`, focus, async loading |
| `UDisGFxMoviePlayerMenuBase` | 7 | load-game list, settings, reset options |
| `UDisGFxMoviePlayerPauseMenu` / `MissionStats` | 3 / 1 | pause menu, mission stats |
| `ADishonoredHUD` | 6 | HUD |
| **DishonoredGame total** | **83** | |

State in the tree: the 17 native `GFxUI` classes **are** registered with the retail layout
(`GFxUI/Src/GFxUIRegistrants.cpp` + `GFxUINativeStubs.cpp` are the only two units that compile;
`Sources.cmake` excludes all 29 others), so script binds and every one of these natives returns a default.
That is why nothing crashes today and why nothing appears.

### 2.2 Boundary 2 — our C++ → GFx (93 direct entry points, 1,350 call sites)

`build/agentAW/gfx_callsites.py` walked all 66,394 functions of the 2012 exe and recorded every call whose
target is a `libgfx`/`libgfx_ime` function and whose caller is not
(`build/agentAW/gfx_entrypoints.csv`, filtered to real `G*` names in `gfx_entrypoints_filtered.csv`; rvas
are 2012). Non-virtual calls only — virtual dispatch through `GFxMovieView`/`GFxMovieDef`/`GFxLoader`
does not show as an xref, and is covered in 2.3.

| Class | calls | the single fact that matters |
|---|---|---|
| `GFxValue` (incl. `GFxValue::ObjectInterface`) | **1,096 of 1,350 (81 %)** | `ObjectRelease` 669, `SetMember` 120, `Invoke` 102, `SetDisplayInfo` 48, `PushBack` 36, `GetMember` 29, `GetElement` 25, `GotoAndPlay` 24, `SetText` 5, `AttachMovie` 4, `CreateEmptyMovieClip` 1 … |
| `GMatrix3D` / `GMatrix2D` | 66 / 19 | UI transforms |
| `GRefCountImpl*` | 71 | reference counting |
| `GRenderer` | 18 | `Cxform`, `ResizeImage`, `Adjust3DMatrixForRT` — static/​inline helpers only; the renderer itself is *ours* (2.3) |
| `GString` / `GFxWStringBuffer` | 19 | strings |
| `GFxLoader` | 4 | ctor, `CreateMovie`, `GetMovieInfo`, dtor |
| `GFxFontMap` / `GFxFontLib` / `GFxTextureFont` / `GFxFontCacheManager` | 9 | font map, fontlib, texture fonts |
| `GImage` / `GImageInfo` / `GImageBase` | 12 | image info for externally loaded textures |
| `GSystem` / `GLock` / `GThread` / `GSysAlloc*` | 13 | init/shutdown, allocator |
| `GFxIMEManager(Win32)` | 4 | IME (Japanese only) |
| others (`GFxResource`, `GFxTranslator`, `GFxURLBuilder`, `GFxSprite`, `GFxMesh`, `GFxRenderConfig`, `GFxMovieView::GetVariableStringW`) | 12 | |

**Read the 81 % literally: the engine's entire relationship with the UI is "poke values into ActionScript
objects and call ActionScript methods".** Of the 1,350 sites, 986 are in `DishonoredGame` (the HUD, journal,
power wheel and menus) and 401 in `GFxUI`.

Key 2013 rvas of the three GFx calls that open a movie: `GFxLoader::CreateMovie` **0x9b4030**,
`GFxLoader::GetMovieInfo` **0x9b3fe0** (both byte-identical to 2012 `0x9baf70` / `0x9baf20`).

### 2.3 Boundary 3 — GFx → us (the interfaces the glue implements, `vtables.csv` slot counts)

These are classes *we* must supply whichever runtime is used, and every one of them is in the 1,041-function
`gfxui` module that is **not ported** (only comment-only skeletons exist, `GFxUI/Src/gfxui*.cpp`):

| We implement | GFx base | slots | our class | fns |
|---|---|---|---|---|
| the renderer | `GRenderer` | **54** | `FGFxRenderer` / `FGFxRendererImpl` | 221 (`gfxuirenderer.cpp`) + 31 (`gfxuirendererimpl.h`) |
| textures / render targets | `GTexture` / `GRenderTarget` / `GImageInfoBase` | 12 / 7 / 8 | `FGFxTexture`, `FGFxUpdatableTexture`, `FGFxRenderTarget`, `FGFxImageInfo` | 41 |
| file access | `GFxFileOpener` / `GFile` | 4 / 19 | `FGFxFileOpener`, `FGFxFile` | 20 |
| image substitution | `GFxImageCreator` / `GFxImageLoader` | 2 / 2 | `FGFxImageCreator`, `FGFxImageLoader` | 4 |
| AS2 → C++ | `GFxExternalInterface` / `GFxFSCommandHandler` | 2 / 2 | `FGFxExternalInterface`, `FGFxFSCommandHandler` | 4 |
| localisation, fonts | `GFxTranslator`, `GFxFontProvider` | 2 / 3 | `GFxUITranslator`, `FGFxUFontProvider` | 9 |
| allocator | `GSysAllocBase` | 15 | `FGFxAllocator` | 18 |
| sound, URLs, CLIK | `GFxFunctionHandler`, `GFxURLBuilder` | | `FGFxSoundEventCallback`, `FGFxURLBuilder`, `FGFxCLIKObject*EventCallback` | 10 |

What GFx must offer back: `GFxMovieRoot{for GFxMovie}` is a **73-slot** interface (`GFxMovieView`),
`GFxMovieDefImpl{for GFxResource}` 29, `GFxLoader` 6, `GFxStateBag` 5, `GASObjectInterface` 21, plus
`GFxValue::ObjectInterface`'s 25 methods.

Note what is **not** in libgfx: the renderer. `FGFxRenderer` (54 virtuals on the RHI) is Epic/Arkane code
and is ours to port regardless of route. Our tree's `GFxUI/Src/Render/RHI_HAL*.cpp` and `Scaleform*.cpp`
are the reference's GFx **4** glue (`Render/RHI_HAL.h`, `Kernel/SF_*.h`) and do **not** implement `GRenderer`
— they are not reusable against 3.3 (`middleware.md` 2.3 says the same; measured here: `FGFxRenderer` has
54 slots, the GFx4 HAL has none of them).

### 2.4 Boundary 4 — ActionScript → C++, and the New Game path end to end

This is the one that decides the whole question. `FGFxExternalInterface::Callback`
(2013 **0x58d510**, 2012 `0x5e4880`, `gfxuiengine.cpp:499`, decompiled in
`build/agentAW/dec2012/FGFxExternalInterface_Callback_5e4880.c`) does exactly this: take the AS2 method
name, `UObject::FindFunction` it on the movie player's `ExternalInterface` object, convert each `GFxValue`
argument with `FGFxEngine::ConvertGFxToUProp`, `ProcessEvent`, and convert the return value back.
`FGFxFSCommandHandler::Callback` (2013 **0x586450**, `gfxuiengine.cpp:444`) is the other direction of the
same idea, routed to `UGFxEvent_FSCommand` Kismet events (and, with no movie user data, to
`eventClientMessage`).

So **the menu's buttons are AS2 calls into the 18 `UDisGFxMoviePlayerMainMenu` natives**:

```
BackToStartScreen  OnCampaignTabSelected  OnContinueClicked  OnDLCClicked  OnDLCListClosed
OnDeviceSelectionComplete  OnLoginCancelled  OnLoginChange  OnMissionSelected  OnMissionsClicked
OnNewGameClicked  OnNewGameConfirm  OnQuitGameConfirm  OpenMissionStats  DeleteDLC
Req_DLConHDD  Req_PSStoreEnabled  UseDLC06Progression
```

plus, from `UDisGFxMoviePlayerMenuBase`: `OnLoadGameConfirm`, `OnDeleteSaveConfirm`, `OnSettingChange`,
`OnResetOptions`, `OnSaveGameListClosed`, `Req_CanLoadGame`, `Req_IsSaveLoadEnabled`; and from
`UDisGFxMoviePlayerBase`: `ShowMessageBox`, `HideMessageBox`, `OnFocusGained`, `FormatText`.

And in the other direction, `UDisGFxMoviePlayerMainMenu::PostStart` (2012 `0x821e00`,
`disgfxmovieplayermainmenu.cpp:43`, decompile in `build/agentAW/dec2012/`) does **nothing but drive AS2**:

```
GetVariable("_root.startScreen_mc") -> Invoke("Close"/"Open")
GetVariable("_root.mainMenu_mc")    -> Invoke("Open", [bHasSaveGame, bHasSaveGame, true, bSaveLoadEnabled])
UDisGlobalUIManager::LoadTexturePackageAsync(m_MapLargeImagePackage)
```

The four booleans come from `UDishonoredEngine::HasSaveGame` and `IsSaveLoadEnabled`. **The layout,
animation, focus handling and navigation of the main menu are AS2 code and timeline inside the `.gfx`
asset. None of it is in the exe.**

The New Game path, which is the only flow the game currently needs (agent AF found it; confirmed here from
the 2012 body):

```
AS2 button -> ExternalInterface.call("OnNewGameConfirm", difficulty)
  FGFxExternalInterface::Callback (2013 0x58d510)
  -> execOnNewGameConfirm (2013 0x5f7d70) -> vtable +600
  -> UDisGFxMoviePlayerMainMenu::OnNewGameConfirm (2013 0x7c1ff0, agent AF; 2012 body 0x820080)
        ADishonoredGameInfo::SetDifficulty(difficulty)
        [2013 also: GetProfileSettings/OnSettingsChanged/SaveSettings, m_bStartingNewGame |= 4]
        ADishonoredPlayerController::ConsoleCommand(UDisTweaks_GFxMoviePlayerMainMenu::m_NewGameCommand)
        m_NewGameCommand = "ce ChangeLvl_StartNewGame"
  -> Kismet console event -> PrepareMapChange / CommitMapChange   (agentAF.md)
```

`-newgame` issues that console command directly. **The entire C++ contribution of the New Game button is
two calls**: `SetDifficulty`, then one console command.

### 2.5 How a movie is started, and the content

`UGFxMoviePlayer::Start` (2013 **0x58e820**) is `PreLoad()` then `FGFxEngine::StartScene` (2013
**0x58e300**); `UDisGFxMoviePlayerBase::Start` (2013 **0x7a3f40**) wraps it with
`UDisGlobalUIManager::OnMovieStackChanged` and `PostStart`. Nothing in the C++ *opens* the main menu — no
`OpenMainMenu` exists in `UDisGlobalUIManager` (33 fns, all listed in `build/agentAW`), so the menu is
opened from UnrealScript/Kismet on the `Dishonored_MainMenu` map.

The content: the cook has **93 `*_SF.upk` packages** plus `Dishonored_MainMenu{,_Env,_FX}.upk`,
`GFxUI.upk`, `UI_HUD_SF`, `UI_Journal_SF`, `UI_PauseMenu_SF`, `UI_Gamma_SF`, `UI_Loading_SF`, 30+
`UI_Map_*_SF`, `DisFonts*_SF`. They already load: `loadall_baseline.csv` shows e.g. `Abbey_01_Keys_SF`
loading with 0 errors. `SwfMovie extends GFxRawData` holds the movie as `RawData` bytes plus
`ReferencedSwfs` and a texture list, with `bPackTextures`, `PackTextureSize`, `TextureFormat` — i.e. this is
**`gfxexport` 3.3 output: `.gfx`, not `.swf`**, with the bitmaps stripped out into UE3 `Texture2D`s that
`FGFxImageLoader::LoadImageW` / `FGFxImageCreator::CreateImage` substitute back in, and the fonts packed
through `GFxFontLib`/`GFxTextureFont`. That matters for option A2 below.

### 2.6 What the runtime would cost to write (by function group)

`build/agentAW/libgfx_size.py`, 153 objects:

| group | bytes | fns |
|---|---|---|
| player core (sprites, buttons, timeline, states, sound, IME, misc) | 433 KiB | 1,951 |
| text engine + fonts (`GFxTextField` 294, `GFxStyledText` 134, `GFxTextDocView` 121, glyph/font caches) | 270 KiB | 1,140 |
| AS2 virtual machine (`GAS*`: `GASGlobalContext` 63, `GASEnvironment` 50, `GASActionBuffer`, the whole AS2 class library) | 212 KiB | 1,164 |
| geometry / tessellation / render feed (`GTessellator` 77, `GStrokerAA`, `GFxShape`, `GFxMesh`) | 101 KiB | 368 |
| loader / SWF+GFX parsing / resources | 74 KiB | 471 |
| kernel (heap, string, file, threads, math) | 68 KiB | 541 |
| **total** | **1.13 MiB** | **5,635** |

Largest single objects: `GFxAction` 69 KiB/285 fns, `GFxPlayerImpl` 68 KiB/278, `GFxTextField` 64 KiB/294,
`GFxSprite` 53 KiB/254, `GFxShape` 38 KiB/163, `GTessellator` 31 KiB/81. `GFxMovieRoot` alone is 205
functions.

## 3. The three options, costed

### Option A — reconstructed GFx bindings (with a stub or a real renderer)

**A0, the literal form of the package brief — headers plus a stub backend: unlocks nothing.** The headers
are cheap (the 2012 PDB names every `libgfx` function and `dia_types.py` dumps the layouts, exactly as
agent AL did for PhysX and AN for Wwise; call it a few days for the ~30 types and 8 interfaces the glue
touches). But behind them there is no player. With a stub: `GFxLoader::CreateMovie` returns an empty
`GFxMovieDef`, `GetVariable("_root.mainMenu_mc")` returns `undefined`, `Invoke("Open")` is a no-op, no AS2
runs, no `ExternalInterface.call` ever arrives, `GFxMovieRoot::Advance` draws nothing. Net effect: the menu
map renders black and no button exists. **Everything that is currently broken stays broken, and 1,041
functions of `gfxui` glue would have to be ported first to reach even that.** This is the option that must
be rejected explicitly, because it is the one the Wwise precedent superficially suggests.

**A1 — a real GFx 3.3.x SDK (licensee source/lib drop).** *If it existed*, this is the best outcome of any
option: zero of the 5,635 functions to write, exact behaviour, the cooked `.gfx` and fontlib load as-is,
and the only work is porting `gfxui`'s 1,041 functions from the 2012 decompile (a real but ordinary
package: comparable to agent AN's 111-row Wwise port, bigger because of the 221-function renderer) plus
`cmake/GFx.cmake` and the MSVC 9 → MSVC 2022 x86 CRT question that Bink/Wwise already have a pattern for.
Cost if the SDK appears: roughly 2–3 waves for `gfxui` + the DishonoredGame UI classes, and the reward is
the **whole** UI: menu, HUD, journal, power wheel, notes, loading screens, subtitles.
**Availability: none we can create.** Scaleform was bought by Autodesk in 2011 and discontinued in 2018;
3.3 was a licensee-only source drop, never public (`middleware.md` 2.3). This is a blocker for the user,
not a task for an agent, and it is the only thing that flips the recommendation.

**A2 — an open AS2 player behind a `GFxUI` adapter (the "Ruffle" idea).** Measured obstacles, each concrete:
* the content is **`gfxexport` `.gfx`, not `.swf`** (2.5): GFx-specific tags, bitmaps replaced by external
  UE3 `Texture2D` references, fonts pre-packed through `GFxFontLib`/`GFxTextureFont`. A stock Flash player
  cannot open these files; a converter back to `.swf` would have to re-inject 30+ texture packages;
* the engine's use of GFx is not "play a movie" but the 1,096 `GFxValue::ObjectInterface` calls of 2.2 — a
  foreign player would have to expose an equivalent live object bridge (`SetMember`, `Invoke`, `PushBack`,
  `SetDisplayInfo`, `AttachMovie`, `CreateEmptyMovieClip`) *and* a `GFxExternalInterface` equivalent with
  UE3 `UFunction` marshalling;
* the renderer seam is `GRenderer` (54 virtuals) feeding UE3's RHI. Ruffle renders through its own
  wgpu backend; there is no `GRenderer`-shaped seam to hook, so either the adapter re-implements GFx's
  mesh/tessellation contract on top of Ruffle's display list, or the whole UI renders to an offscreen
  surface and loses the RHI integration (`SetExternalTexture`, render-target movies, `FGFxRenderTarget`);
* plus CLIK widgets, `GFxTranslator` localisation, distance-field text, `GFxIMEManagerWin32`.
Honest verdict: a project of its own, a second foreign runtime (Rust) in the build, and *still* not
Arkane's UI — it would be an approximation whose divergences would be discovered one screen at a time.

**A3 — rewrite the 1.13 MiB from the decompile.** 5,635 functions with PDB names but no source lines,
including an AS2 VM, a text engine with a glyph cache, and a tessellator. `middleware.md` already calls
this "the biggest single item of the whole project". Confirmed; not a candidate.

**A4 — considered and rejected: call retail's own `libgfx` in place.** `Dishonored.exe` does have a
`.reloc` section (0x17ca0c bytes), so it could in principle be mapped and called. But `libgfx`'s statics
are initialised by retail's own CRT startup and its allocator is `GSysAlloc`/`GMemoryHeap` state built at
retail's `main`; making those live means running retail's initialisers, i.e. booting the retail exe. That
is not a functional recompilation, it makes the retail binary a runtime dependency, and it would poison
every other subsystem's ownership of memory. Recorded here only so it is not re-proposed.

**What A unlocks / costs / risks / leaves broken**

* unlocks: A1 unlocks everything; A0 nothing; A2 a partial, divergent UI; A3 everything, eventually.
* costs: A0 ≈ 1 wave wasted; A1 2–3 waves *after* an SDK appears; A2 several waves plus a Rust dependency;
  A3 the rest of the project.
* risks: A0 the risk of looking like progress; A1 the licence/availability risk and the MSVC 9 CRT risk;
  A2 open-ended divergence; A3 schedule annihilation.
* leaves broken: A0 everything; A2 whatever the adapter approximates badly, discovered late.

### Option B — a minimal replacement menu layer driving the same script events

**The shape.** Do not touch the flow. Keep every Arkane class, keep the 208 GFx natives stubbed, and add a
small presentation layer, behind a switch (`-dismenu` / `DISHONORED_WITH_SIMPLE_MENU`), that:
1. draws a list on `FCanvas` — this already works: agent Y presented canvas text in the cooked
   `EngineFonts.SmallFont` (`agentY.md` 3) and agent AI fixed `UMultiFont::GetScalingFactor`, which had
   made every canvas string invisible (`agentAI.md`);
2. routes player input to the selection (the same `FilterButtonInput` shape the menu classes already use);
3. on activation, calls **Arkane's own handlers** — exactly the functions AS2 would have called:
   `OnNewGameConfirm(difficulty)`, `OnContinueClicked()`, `OnLoadGameConfirm(slot)` +
   `Req_SaveSlotInfos(...)` / `Req_CanLoadGame`, `OnQuitGameConfirm()`, `OnSettingChange(id, value)` +
   `OnResetOptions(...)`, `BackToStartScreen()`.

**Why it is cheap.** The decision logic is in C++, not in AS2. `OnNewGameConfirm` is `SetDifficulty` plus
one console command (2.4). The load-game list is built in C++ (`UDisGFxMoviePlayerMenuBase::FillLoadGameMenu`,
`CreateGFxLoadGameList`, `FindSaveName`, `FormatSaveDate`, `FindSaveImagePath`) and only the last step
pushes it into AS2 — the data model is ours already. Same for options:
`FillSettingsCategoryList` / `ShowSettingsCategoryList` / `CreateGFxCategory` / `CreateGFxSetting` /
`OnApplyVideoSettings` / `TryBindKey` are all C++ over `FDisSettingsCategory`. A replacement layer replaces
only the `CreateGFx*` leaves.

**What it unlocks.** A real main menu (New Game with a difficulty choice, Continue, Load, Quit, and
plausibly Options), the pause menu the same way, and — the part that matters beyond presentation — a
**save/load UI**, which is the difference between a demo and something you can put down and come back to.
It also makes the game demonstrable without a command line.

**What it costs.** Small in code, real in care: new files only (no engine file needs editing if the layer
lives in DishonoredGame + a canvas hook), one switch, input routing, and the menu screens' state machine.
Estimate one package, not one wave, *provided* the prerequisite below is met.

**What it risks.**
* **It is not Arkane's menu.** The wave's whole discipline is "port what retail does". This deliberately
  writes something retail does not have. It must be marked `DISHONORED(written)` everywhere, kept behind a
  switch, and deleted the day option A1 becomes possible — otherwise it silently becomes the answer.
* It competes for the same classes a future real GFx path will use. Mitigation: the layer only *calls* the
  Arkane handlers; it never modifies them.
* **Prerequisite, and it is not ours:** the menu map today cannot be entered and left. Agent AF's **B1**
  (`Dishonored_MainMenu`'s `SeqAct_Interp_14` has `InterpLength == 0`, so the world tick never returns) and
  **B2** (the streamed-menu teardown deadlocks between the game and rendering threads in the CRT heap) are
  agent AT's package this wave. Until B1/B2 are fixed, a replacement menu has nowhere to live.

**What it leaves broken.** Everything GFx that is not the menu, i.e. most of the UI:
`UDisGFxMoviePlayerHUD` 117 functions, `Journal` 82, `PowerWheel` 45, `Global` 33, `PauseMenu` 31,
`Store` 25, `Note` 20, `MissionStats` 17, `Gamma` 11, `HUDFX` 7 — plus loading screens, subtitles, the
in-game maps (30+ `UI_Map_*_SF`) and every CLIK widget. **The player would have a menu and no HUD.**

### Option C — leave `-newgame` / `-startmap` as the entry point

**Cost: zero.** It already works: `resources\play.cmd [map]`, `-startmap=`, `-startmapopen`, `-newgame`
(agentAF.md). **What it unlocks: nothing new — but it blocks nothing either.** The menu is not on the
critical path to a playable game: the four packages that are (AR textures, AS touch, AT Kismet, AU
gameplay natives) do not need a single GFx function, and the game already renders its world and lets the
player walk. **What it leaves broken:** no menu, no pause, no options, no save/load UI, no HUD — i.e. the
same as today. **Risk:** only that "revisit later" becomes "never"; countered by this document and by the
tracker row.

A one-line-each cheap extension of C, if presentation pressure arrives before option B: expose the
handlers of 2.4 as console commands (`ce ChangeLvl_StartNewGame` already is one), so difficulty, load,
save and quit are reachable without any UI at all.

## 4. Recommendation

**C now; B as a contained, switch-gated package in the wave after AR/AS/AT/AU land; A only via A1, and
only if the user obtains a GFx 3.3.x SDK; never A0, A2, A3 or A4.**

Reasoning, in the order the evidence forces it:

1. **A0 is excluded by measurement, not by taste.** No shipped DLL holds GFx (section 1), so there is no
   binding to reconstruct; and a stub backend cannot run the AS2 that *is* the menu (2.4). This is the
   clear negative the package asked for: **the method that worked for PhysX, Steamworks and Bink does not
   apply to Scaleform at all, and the Wwise answer does not transfer either, because silence is an
   acceptable audio backend and a black screen is not an acceptable UI backend.**
2. **C costs nothing and blocks nothing.** The user's priority is a playable game; the menu is flow and
   presentation; the four packages on the critical path need no GFx.
3. **B is the only route that buys a menu at a cost of days.** And the reason it is cheap is a positive
   finding, not a hope: the menu's decisions live in C++ (`OnNewGameConfirm` = `SetDifficulty` + one
   console command; the load list and the settings tree are C++ data models), and the canvas + font path it
   needs already works in this tree.
4. **B waits for AT.** B1/B2 make the menu map unusable today; building a menu into a map that deadlocks on
   teardown would produce an unverifiable package.
5. **A1 is the only thing worth a decision from the user**, and it is an availability question, not an
   engineering one.

### What I would have to be wrong about for a different option to win

| If this is false | then |
|---|---|
| "No GFx 3.3.x SDK can be obtained" | **A1 wins outright**, immediately, and B should never be written: with a real runtime, porting `gfxui`'s 1,041 functions gives the *whole* UI faithfully, and B's replacement menu becomes dead weight that has to be deleted. This is the single fact to check before spending anything on B. |
| "The menu is presentation, not playability" | If the **HUD** turns out to gate play (objectives, subtitles, the power wheel needed to use powers, the journal needed to know what to do), then the question is no longer the menu and B's scope explodes from 25 handlers to ~400 functions of HUD/journal/power-wheel — at which point A2's "a real AS2 player, somehow" starts to look less unreasonable than 400 hand-written screens, and the decision should be re-taken with that measured. |
| "The cooked assets are `gfxexport` `.gfx` with external textures" | If they were plain AS2 `.swf` with embedded bitmaps, A2's first and largest obstacle disappears and an open-player adapter moves from "a project of its own" to "a hard package". I measured the `SwfMovie`/`GFxRawData` shape (2.5) but did not parse a `.gfx` header; that check is one afternoon and is the thing to do before anyone argues for A2. |
| "B can be confined to new files behind a switch" | If a replacement menu turns out to need edits inside `UDisGFxMoviePlayer*` (e.g. because `Start`/`PostStart` cannot be bypassed cleanly), B stops being contained, starts competing with the real port, and C should simply be kept. |
| "B1/B2 are AT's and will be fixed" | If the menu-map teardown deadlock proves to be a deep heap bug that outlives this wave (agent AF suspects B2 and B4 are one heap corruption), then B has no host map and C is the only option for longer than planned. |

## 5. Evidence index (all in `build/agentAW/`, nothing committed)

| file | what it is |
|---|---|
| `gfx_callsites.py` | the IDA pass: attributes all 66,394 functions by module from `functions.csv`, then records every call into `libgfx` from outside it |
| `gfx_entrypoints.csv` / `gfx_callers.csv` | raw output: 103 targets (incl. 10 ICF-folded aliases), and every call site with its caller |
| `filter_entrypoints.py` / `gfx_entrypoints_filtered.csv` | the 93 real `G*` entry points, 1,350 call sites, by class |
| `libgfx_size.py` | the 1.13 MiB / 5,635-function breakdown by object and group |
| `libgfx_match.py` | 2012↔2013 identity of `libgfx` (4,080 byte-identical, 92.9 % at ratio 1.000) and of `gfxui` |
| `scan_gfx_dlls.py` | the "no GFx in any shipped DLL" scan, all 33 DLLs + every exe in `Binaries\Win32` |
| `map2013.py`, `vt.py`, `status_join.py` | 2012→2013 rva lookup, vtable slot counts, status-CSV joins |
| `dump_ui_surface.py` → `ui_cpp_surface.txt`, `ui_natives.txt` | the C++ surfaces quoted in section 2 (`UDisGlobalUIManager` 33, `MainMenu`, `MenuBase`, `Base`, `PauseMenu`, `HUD`, `FGFxEngine`, `FGFxRenderer`, `UGFxMoviePlayer`) and all 208 GFx/UI natives with their 2013 rvas |
| `dec2012/` | 15 headless decompiles: `FGFxExternalInterface::Callback`, `FGFxFSCommandHandler::Callback`, `UGFxMoviePlayer::Start`/`Advance`, `FGFxEngine::StartScene`/`RenderUI`/`LoadMovie`/`LoadMovieDef`/`Tick`/ctor/`InitGFxLoaderCommon`, `UDisGFxMoviePlayerBase::Start`/`Advance`, `UDisGFxMoviePlayerMainMenu::PostStart`/`OnNewGameConfirm`/`FilterButtonInput` |

`middleware.md` 2.3 stays correct in its conclusion (SDK or an open runtime, never a rewrite) and is
superseded in one respect: it left the choice between "a licensee SDK" and "Ruffle behind an adapter".
This document adds the measurements that make the second one a project rather than an option, and adds the
third path Arkane's own code makes cheap — drive the menu handlers directly. **The coordinator should fold
section 0 and the recommendation into `middleware.md` 2.3 and item 4 of its blocker list at merge** (not
edited here: `middleware.md` was being edited by other packages of this wave, the same reason agent AN gave).
