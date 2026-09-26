# Agent AW report — the main menu decision (Scaleform GFx) (2026-09-26)

Package "**AW**" of wave 5 (`PHASE7.md`): establish what the main menu actually needs from Scaleform GFx,
cost the alternatives, recommend one. **Investigation only. No engine file was touched, nothing was
implemented, nothing was committed or staged, and no status rows exist** (no function was ported, so
`agentAW_status.csv` was deliberately not created).

**The deliverable is `resources/docs/gfx_decision.md`.** Read that; this file is the pointer, the method and
the honest limits.

## The answer in three lines

* **Route 1 — "reconstruct the binding from the shipped DLL", the method that worked for PhysX, Steamworks
  and Bink — does not apply to Scaleform at all.** Not one of the 33 DLLs in `Binaries\Win32` contains a
  single GFx symbol or string; the runtime is 5,635 functions / **1.13 MiB = 9.7 % of `.text`** inside
  `Dishonored.exe` itself.
* **The Wwise answer does not transfer either.** A silent audio backend is functionally adequate because
  audio's job during bring-up is to be inaudible. A stub GFx backend is a **black screen that never calls
  back**: the main menu's layout, navigation and buttons are ActionScript 2 inside the cooked `.gfx`
  assets, and every button reaches C++ only through `FGFxExternalInterface::Callback`.
* **Recommendation: keep `-newgame`/`-startmap` now (option C, zero cost, blocks nothing); schedule a
  small switch-gated menu layer that calls Arkane's own handlers (option B) after AR/AS/AT/AU; and the only
  thing worth a user decision is whether a GFx 3.3.x licensee SDK can be obtained — if it can, that option
  wins outright and option B should never be written.**

## Why option B is cheap — the positive finding

The menu's *decisions* are in C++, not in ActionScript. `UDisGFxMoviePlayerMainMenu::OnNewGameConfirm`
(2013 `0x7c1ff0`, 2012 body `0x820080`) is `ADishonoredGameInfo::SetDifficulty(difficulty)` followed by one
`ADishonoredPlayerController::ConsoleCommand(m_NewGameCommand)` — and `m_NewGameCommand` is
`ce ChangeLvl_StartNewGame`, exactly what agent AF's `-newgame` already issues. The load-game list
(`FillLoadGameMenu`, `CreateGFxLoadGameList`, `FindSaveName`, `FormatSaveDate`) and the whole options tree
(`FillSettingsCategoryList`, `CreateGFxCategory`, `CreateGFxSetting`, `OnApplyVideoSettings`, `TryBindKey`)
are C++ data models over `FDisSettingsCategory` / `FDisSaveGame`; only the final `CreateGFx*` leaves push
into AS2. So a replacement layer replaces leaves, not logic, and the 18 `MainMenu` + 7 `MenuBase` +
9 `Base` natives are the complete boundary it has to drive.

It is also buildable on infrastructure that already works: agent Y presented canvas text in the cooked
`EngineFonts.SmallFont`, and agent AI fixed `UMultiFont::GetScalingFactor`, which had made every canvas
string invisible.

## Method (so the numbers can be re-derived)

Own build dir `build\agentAW`, own IDA copies `resources\docs\idb\{shipping2012,retail2013}_agentAW.i64`,
**headless only** (`resources\tools\ida\run.py`); no IDA MCP tools, no FModel tools, no junctions, nothing
deleted. The xref pass and the decompiles ran on the **2012** database, because the PDB names every `libgfx`
and `gfxui` function there and `functions.csv` attributes each one to its library; 2013 rvas in both
documents come from `match_2012_2013.csv` (or from `agentAF.md` where the match table has a gap), and the
2013 copy was taken but only used for spot checks.

1. `build/agentAW/gfx_callsites.py` (run through `run.py` on the 2012 db) attributes all 66,394 functions to
   a module from `resources/docs/symbols/functions.csv`, then walks every code xref and records every call
   whose target is in `libgfx`/`libgfx_ime` and whose caller is not → `gfx_entrypoints.csv`,
   `gfx_callers.csv`; `filter_entrypoints.py` drops the ICF-folded aliases →
   **93 entry points, 1,350 call sites, 81 % of them `GFxValue`/`GFxValue::ObjectInterface`**.
2. `libgfx_size.py` — the 1.13 MiB by object and by group (player core 433 KiB, text+fonts 270 KiB, AS2 VM
   212 KiB, tessellation 101 KiB, loader 74 KiB, kernel 68 KiB).
3. `libgfx_match.py` — 4,080 of 5,635 `libgfx` functions byte-identical 2012↔2013 (92.9 % at ratio 1.000),
   so the 2012 PDB names describe the retail runtime exactly.
4. `scan_gfx_dlls.py` — the "no GFx in any shipped DLL" scan.
5. `vt.py` on `vtables.csv` — the interface sizes both directions: `GRenderer` **54** slots,
   `GFxMovieRoot{for GFxMovie}` **73**, `GFxMovieDefImpl{for GFxResource}` 29, `GFxLoader` 6,
   `GTexture` 12, `GRenderTarget` 7, `GFxFileOpener` 4, `GASObjectInterface` 21.
6. `dump_ui_surface.py` → `ui_cpp_surface.txt`, `ui_natives.txt` — the C++ surfaces and all **208** GFx/UI
   natives with 2013 rvas.
7. `map2013.py` — 2012→2013 rva lookup through `match_2012_2013.csv`.
8. 15 headless decompiles in `build/agentAW/dec2012/`, the load-bearing ones being
   `FGFxExternalInterface::Callback` (2013 `0x58d510`), `FGFxFSCommandHandler::Callback` (2013 `0x586450`),
   `UDisGFxMoviePlayerMainMenu::PostStart` (2012 `0x821e00`) and `OnNewGameConfirm`.

Nothing was built and nothing was run: the package needed no build, and the tree was in flux from six other
packages.

## Limits of this investigation, stated plainly

* The 93-entry-point inventory is **direct calls only**. Virtual dispatch through `GFxMovieView` (73 slots),
  `GFxMovieDef` and `GFxLoader` does not appear as an xref; section 2.3 of the decision document covers that
  side from the vtable slot counts instead.
* I did **not** parse a cooked `.gfx` header. That the assets are `gfxexport` output with external textures
  is inferred from `SwfMovie extends GFxRawData` (`RawData`, `ReferencedSwfs`, `bPackTextures`,
  `PackTextureSize`, `TextureFormat`) plus the existence of `FGFxImageLoader::LoadImageW` /
  `FGFxImageCreator::CreateImage` and the 93 `*_SF.upk` texture packages. It is the first thing to verify if
  anyone argues for the open-AS2-player route, and the decision document says so.
* I did not attempt to estimate option B in days beyond "one package, not one wave", because its
  prerequisite (agent AF's B1/B2, agent AT's package this wave) is not resolved and the estimate would be
  fiction until it is.

## Hand-overs

1. **Coordinator, at merge:** fold section 0 and the recommendation of `gfx_decision.md` into
   `middleware.md` 2.3 and item 4 of its blocker list. I deliberately did not edit `middleware.md` — the
   PhysX and Steamworks packages of this wave were editing it, and agent AN left the same note for the same
   reason.
2. **User decision, the only one that changes the answer:** can a Scaleform GFx **3.3.x** licensee SDK
   (headers + `libgfx.lib`) be obtained? If yes, that route wins outright and option B should not be
   written. If no, options C then B stand.
3. **Agent AT:** option B cannot be built until B1 (`Dishonored_MainMenu`'s `SeqAct_Interp_14` with
   `InterpLength == 0` spins the world tick for ever) and B2 (the streamed-menu teardown deadlock) are
   fixed. Both are in AT's package already; this is only a note that a second consumer is waiting on them.
4. **Whoever ports `GFxUI` one day:** the reference glue in the tree is GFx **4**
   (`Src/Scaleform*.cpp`, `Src/Render/RHI_HAL*.cpp`, `Kernel/SF_*.h`) and implements none of `GRenderer`'s
   54 slots. It is not a head start against 3.3. The Arkane GFx-3 skeletons (`Src/gfxui*.cpp`, 1,041
   functions, currently comment-only) are, and `gfxuirenderer.cpp` alone is 221 of them.
