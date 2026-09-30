# Agent EO (PHASE13 EO) — the options screen: the values were never put in it, and the key that closes it was never answered

Branched from HEAD `0cbe3fe`, own worktree `build/agentEO_wt`, own build dirs `build/agentEO_wt/build/agentEO_rel`
(this package), `build/agentEO_headsrc/build/agentEO_headrel` (the untouched HEAD baseline, built from a
`git archive` export), `build/agentEO_wt/build/agentEO_reg` (the regression harness),
`build/agentEO_wt/build/agentEO_clean` (the gate build). Own IDA copy
`build/agentEO_ida/retail2013_agentEO.i64`, headless only. **No IDA MCP tool and no FModel tool was used.** No
commits, nothing staged, nothing written into the main checkout.

**This package touches no file in `External/GFx3` and no file in `GFxUI`.**

## The answer in eight lines

* **Both halves of the brief are wrong about the cause, and both faults are now closed.**
* **The brightness screen said `undefined` because the row it reads has no value in it, not because the gamma
  could not be read.** `DisCreateGFxSetting` set `Setting_Id`, `Setting_Name` and the screen flags and stopped
  before `Setting_Value`, `Setting_Minimum`, `Setting_Maximum`, `Setting_Increment` and `Mapping_Names` — its own
  comment said so. AS2 reads an absent member as `undefined` and `String(undefined)` is the literal the user saw.
  Retail's `CreateGFxSetting` (2013 `0x7db5f0`) is ported whole. Section 3.
* **Two more things had to be true before that row existed at all.** `_root.options_mc` — the clip
  `ShowSettingsCategoryList` and `TryBindKey` invoked on — is a name that appears in **no retail function and no
  cooked movie**; retail's own string is `_root.optionsMenu_mc` (measured: `FillCategories NOT FOUND` → `ok`).
  And `FillSettingsCategoryList` / `FillOptionsMenu` were never ported, so `m_SettingsCategoryList` was written
  by nothing and read by three places. Both are ported; the tree is **4 categories, 87 rows**.
* **`B` on the Options screen was never a missing PC key binding.** Agent EI's judgement is falsified by agent
  EH's own log, one line below the line EH quoted: `> BPressed () called` is followed by
  `DishonoredGame native not ported: UDisGFxMoviePlayerMenuBase::execOnLeaveOptions`. Escape *is* bound, it *did*
  reach the content, the content *did* call back — into a stub. `OnLeaveOptions` (2013 `0x7bcad0`) and the
  `CloseOptions` (`0x7bc9d0`) it needs are ported and the screen closes. Section 5.
* **The rest of `Read` was not "two off": it was on the wrong id list, the wrong accessor family, and two
  members short.** 2013's `EProfileSettingID` has 155 values where 2012 has 133 and renumbers 80 of them; every
  slider and volume is a `PVMT_Ranged` mapping, which the plain `GetProfileSettingValueInt/Float` reject
  outright; and retail's `ArkSettingsParameters` is **220 bytes / 55 members**, not 212 / 53. Section 2.
* **`SetProfileSettingValueId` had agent EK's bug too** — it took mapping type 3 alone where retail takes 3 or 4,
  so every write-back of a boolean option failed. So did `IsProfileSettingIdMapped`.
* **Measured, by real input, from the menu, nothing forced:** the brightness screen reads **BRIGHTNESS** with its
  knob at the profile's value, and five LEFT presses walk the gamma **2.2 → 2.1 → 2.0 → 1.9 → 1.8 → 1.7** with
  the knob and the picture following. The same schedule on the untouched HEAD executable reads `undefined` and
  the knob does not move. Section 6.
* **Regression 37 ok, 0 failed, 0 skipped**, built inside the harness; a clean full Release build with the build
  directory deleted first and `DISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**; `rva_sweep` over the
  whole tree: 6989 citations, 0 unknown, 0 mislabelled in any file this package touched. Section 8.
* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` IS required and WAS run**;
  `Sources.cmake` did **not** change. Nine files outside `resources/docs`, three of them in `Engine`. Section 9.

## 1. Where the two faults actually were

`resources/docs/agents/agentEO_status.csv` has every address with its verdict. The chain the user walked:

| retail 2013 | what | verdict |
|---|---|---|
| `0x7e6970` | the `UDisGFxMoviePlayerMenuBase` virtual that fills the options tree when the **main menu movie** comes up | was not ported — so the tree was empty when the new-game brightness screen asked for the gamma row |
| `0x7e65f0` | `FillOptionsMenu` | not ported |
| `0x7e50c0` | `FillSettingsCategoryList` — the 87 rows | not ported |
| `0x7e3920` | `ShowSettingsCategoryList` | ported, but onto `_root.options_mc`, a clip that does not exist |
| `0x7db5f0` | `CreateGFxSetting` — the row object | ported down to the four value members, which were left out |
| `0x539730` | `ArkSettingsParameters::Read` | on the 2012 id list, on the plain accessors, two members short |
| `0x7cb870` | `OnSettingChange` — the slider's write-back | `m_bOptionsChanged = TRUE;` and nothing else |
| `0x7bcad0` | `OnLeaveOptions` — what `B` calls | `DISHONORED_NATIVE_STUB` |
| `0x7bc9d0` | `CloseOptions` — what closes the screen | not ported |

Five of those nine are enough on their own to produce `undefined`; all nine are now closed or carried as a named
gap.

## 2. `ArkSettingsParameters` and `Read`, corrected against retail 2013

The brief said "the rest of `Read` is still on the 2012 profile ids — mouse sensitivity, gamma, volumes,
subtitles are all two off". Three things in that sentence are wrong, and the third is why the gamma could not be
read at all.

**The offset is not two, and it is not constant.** 2013's `Engine.OnlineProfileSettings.EProfileSettingID` has
**155** values against the 2012 enum's **133**, and `script_delta_2012_2013.md:1634` records exactly what it
adds: `PSI_GBA_QuickSave`, `PSI_GBA_QuickLoad` (+2 from `PSI_Mouse_Sensitivity` on),
`PSI_HUD_bShowObjectiveMarkers` (+1 more from `PSI_HUD_CrosshairStyle` on), `PSI_GraphicsPC_LightShaftEnable` in
place of the **removed** `PSI_GraphicsPC_PostProcessQuality`, `PSI_bProfileSaved`, the eleven `PSI_DLC05_*`, and
`PSI_DLC06_MasterAssassinUnlocked` / `PSI_DLC06_Difficulty` / `PSI_DLC06_bWelcomeMsgboxShown`. So 2012's 65
(mouse sensitivity) is 2013's **67**, 2012's 85 is 87, 2012's 96 is **99**, and 2012's 109 (gamma) is **112**.
The old body read id 109, which in 2013 is `PSI_Gameplay_CameraRelativeClimbing`, an id-mapped boolean, with
`GetProfileSettingValueFloat`, which wants `PVMT_RawValue`: `GetProfileSettingValueFloat did not find a valid
MappingType. ProfileSettingId: 109` is in `build/agentEK/r3_log.txt`, and `Error: Invalid DisplayGamma! Resetting
to the default of 2.2` is four lines away from it.

**Retail does not use the accessors this tree used.** Read off the decompile and resolved by hand out of the
2013 `UArkProfileSettings` vtable at `0xca3d58`:

| retail call | vtable | what it is |
|---|---|---|
| `+332` | `0x4f36b0` | `GetProfileSettingValueId(int, int&, int*)` |
| `+348` | `0x4f3ac0` | `SetProfileSettingValueId` |
| `+376` | `0x4f75d0` | **`SetRangedProfileSettingValueInt`** |
| `+384` | `0x4f77e0` | **`GetRangedProfileSettingValueFloat`** |
| `+388` | `0x4f7860` | **`GetRangedProfileSettingValueInt`** |

The three ranged ones are what every slider and every volume is read with, and they are what makes them readable
at all: their mappings are `PVMT_Ranged` (type 2 in the cooked profile's own table), and the plain
`GetProfileSettingValueInt`/`Float` return FALSE for anything that is not `PVMT_RawValue`. The comment block this
unit carried named `+380`/`+384` as the plain float and int getters; in 2012's own vtable those offsets are
`GetRangedProfileSettingValueFloat` and `GetRangedProfileSettingValueInt`. The mis-identification is the whole
reason the mouse sensitivity, the four volumes, the FOV, the crosshair opacity, the head bob and the gamma were
all unreadable, and it survived because `vtables.csv` is the **2012** table and retail 2013's is shifted by four.

**Retail's struct is 220 bytes, not 212.** `Read` writes `this[54]`. The three members 2013 adds over the 2012
PDB type are `m_bShowObjectiveMarkers` (agent EK found this one), and, between `m_KillCamMode` and
`m_bAutoSaveInMenu`, **`m_Difficulty` (id 106, `PSI_Gameplay_Difficulty`, four value mappings, falling back to
the Live-standard `PSI_GameDifficulty` = 12)** and **`m_DifficultyDLC06` (id 152, `PSI_DLC06_Difficulty`, falling
back to the campaign's own clamped to `EDifficulty_Hard`)**. `Settings.int` confirms both by name:
`PSI_Gameplay_Difficulty=Difficulty` and, under `; Other`, `PSI_DLC06_Difficulty=Difficulty`, with the four
`EDifficulty_*_DLC06` labels *Novice / Veteran / Elite / Master Assassin* beside them — and retail's
`CreateGFxSetting` hides the fourth unless profile setting 151 (`PSI_DLC06_MasterAssassinUnlocked`) is set,
which is what `0x5359b0` reads.

**One member is renamed, not added.** Retail reads id 121 as `this[46] = (value == 1)`, a boolean. Id 121 in 2013
is `PSI_GraphicsPC_LightShaftEnable`; `PSI_GraphicsPC_PostProcessQuality` and the `EPostProcessQuality` enum were
both removed in 2013, and `Settings.int` has `PSI_GraphicsPC_LightShaftEnable=Light Shafts`. The 2012 PDB calls
that slot `INT m_PostProcessQuality`; this tree now calls it `UBOOL m_bLightShaftEnable`. That naming is a
judgement, not a measurement — the evidence is the removed enum value, the new one at the same id, the localised
label, and retail narrowing it to a boolean. Nothing in this tree reads the member either way.

**And the two setters had agent EK's bug.** `SetProfileSettingValueId` (`0x4f3ac0`) and
`IsProfileSettingIdMapped` (`0x4f31a0`) both test `type == 3 || type == 4` in retail and tested 3 alone here, so
every write-back of a boolean option silently failed — including the four `Read` itself makes for the pad's
invert-Y and auto-aim and for the fullscreen and vsync overrides.

The whole of `Read` is rewritten against `0x539730`, id for id, accessor for accessor, including the four
fallback ladders (ids 13, 2, 16, 12), the `Min(m_Difficulty, EDifficulty_Hard)` default for 152, and the
RUS/CZE/HUN/POL subtitle default.

## 3. The row, and what `undefined` was

`DisCreateGFxSetting` now does what `0x7db5f0` does:

* `Setting_Id`, and `Setting_Name` from `m_SettingNameOverride` when it has one, otherwise
  `Localize("ProfileSettingIDs", DisEnumTypeToString(id, "Engine.OnlineProfileSettings.EProfileSettingID"),
  "Settings")` — which is how `PSI_Graphics_Gamma` becomes `Brightness` and the asset's `toUpperCase` becomes
  `BRIGHTNESS`;
* `Mapping_Type` from the setting's own `FSettingsPropertyPropertyMetaData`;
* **`PVMT_RawValue`**: `GetProfileSettingValueInt`, and for the 36 ids in the bindable-action span 29..64 the
  localised key name instead of the number (`GetKeyName` `0x533560` → the `[Keys]` section), else
  `GetProfileSettingValueFloat`;
* **`PVMT_Ranged`**: `GetRangedProfileSettingValueInt` else `...Float`, plus `GetProfileSettingRange` into
  `Setting_Minimum` / `Setting_Maximum` / `Setting_Increment` — **this is the slider**;
* **`PVMT_IdMapped` and type 4**: the value mappings become `Mapping_Names` through
  `Localize("ProfileSettingValues", …, "Settings")` and the current value becomes the index into that list, with
  the id-152 / Master Assassin exclusion;
* exactly **one** of the five screen flags, set to `true`, in retail's else-if order — the old code set all five
  unconditionally.

`DisEnumTypeToString` (2012 `0x82cd60`) is added to `dishonoredutilities.cpp`, where the PDB attributes it.

## 4. The tree, and where it is filled

`FillSettingsCategoryList` (`0x7e50c0`) is ported whole. The category and sub-category names come off
`m_pMenuBaseTweaks`, and the offsets retail uses line up member for member with this tree's names:
`+224 m_SettingsCategory_General`, `+236 …GameplaySettings`, `+248 …HUDSettings`, `+260 …Controls`,
`+272 …KeyboardMapping`, `+284 …MouseSettings`, `+296 …GamepadSettings`, `+308 …Graphics`,
`+320 m_SettingsSubMenu_VideoSettings`, `+356 m_SettingsCategory_Audio`. The id runs are 104..109 (with 106/107
dropped in the Dunwall City Trials and 106 replaced by 152 in the assassin campaigns), 87..101, 29..64 with
`m_bKeyboardBindingMenu`, 67..73, 76..84 with `m_bGamepadBindingMenu` on 76 alone, then the video sub-screen row
(id −1), the **gamma row (id 112, `m_bGammaMenu`)**, 118..123, 126..130, and 133. Ids 119 and 133 are offered
only when `bAllowRestartSettings`, which is `TRUE` from the main menu and `FALSE` from the pause menu — they are
the two settings that need a restart.

**Where it is filled matters and the brief did not know about it.** Retail does not fill the tree only when
Options is clicked: the `UDisGFxMoviePlayerMenuBase` virtual at `0x7e6970` opens with
`if (GetClass() == UDisGFxMoviePlayerMainMenu::StaticClass()) FillOptionsMenu(TRUE);`. That is why retail's
new-game brightness screen — which is reached without ever opening Options — already has a value. The virtual is
new in 2013 (its other half is the DLC06 return transition) so it has no 2012 name; this tree's
`UDisGFxMoviePlayerBase` has no `PostFirstAdvance` at all. Measured here: `_root.optionsMenu_mc` does not exist
at `PostStart` and appears about two frames later (`build/agentEO/afterB_log.txt`, `PostStart` at 7.66 s, the
clip's first AS2 error at 7.73 s), so `DisFillOptionsMenuOnce` fills on the first advance on which the clip
resolves, which happens once because the list is only empty until then.

## 5. `B`, settled: agent EI was wrong, and the measurement that settles it is agent EH's own log

Agent EI's judgement was that agent EH's stuck Options screen and EI's own unanswerable message box were "the
same missing PC key binding in `_common.InputsHandler`", and that finding one key would fix both. It is
falsified twice over:

1. **Escape is bound and always was.** `FGFxEngine::InitKeyMap` maps `Escape → GFxKey::Escape` (27) — and maps
   `XboxTypeS_B` to the *same* code, because this tree's `GFxKey::Code` has no `GAMEPAD_*` values, so on PC the
   pad's B is faked through the keyboard's Escape rather than the other way round.
2. **The key reached the content and the content acted on it.** `> BPressed () called` is the cooked SWF's own
   `trace()`, printed by `GFxAS2Interp.cpp`'s `GASop_Trace`. Its appearance is proof the AS2 handler ran.

What follows it in `build/agentEH/base7_log.txt:21640-21641`, and what nobody read, is:

```
[0024.21] AS2 trace: > BPressed () called
[0024.22] DishonoredGame native not ported: UDisGFxMoviePlayerMenuBase::execOnLeaveOptions (parameters consumed, result zeroed)
```

The content called back into C++ by name and hit a `DISHONORED_NATIVE_STUB`. `OnLeaveOptions` (`0x7bcad0`) is
what drives the screen back, through `CloseOptions` (`0x7bc9d0`) → `_root.optionsMenu_mc.Close()`. Both are now
ported, and the pause menu's own declaration of the native forwards to the base body, as retail's does.

**Measured before and after, same schedule, same driver, same machine** (Space, →, →, Enter to reach OPTIONS,
then Escape):

| | untouched HEAD `0cbe3fe` (`build/agentEO/beforeB_log.txt`) | this package (`build/agentEO/optC_log.txt`) |
|---|---|---|
| `TransitionTo (OptionsScreen)` | 40.06 | 47.79 |
| `> BPressed () called` | 49.88 | 57.62 |
| next | `native not ported: …execOnLeaveOptions`, then **nothing** — a second Escape at +34 s does nothing either | `fscommand (FromOptionsScreen)` → `BackToMainMenu` → `PlayOpenAnimation` → the main menu bar reopens with `sel._curSelection : 2` |

EH also expected `TransitionTo (MainMenuScreen)` to follow. It does not, on either build: the asset leaves the
Options screen through `BackToMainMenu`, not through `TransitionTo`. That expectation was a guess and it is
worth recording, because its absence is what made the screen look unreachable rather than unanswered.

**The message box is a different question and this package did not close it.** It is in the *Global* movie, not
the menu movie, and EI's own quote of `_common.MessageBox::APressed` — `if (_global.PlatformName == 'PC')
Close();` — makes a symmetrical PC guard in `BPressed` the likeliest explanation, in which case RIGHT+ENTER is
the intended PC "NO" and nothing is broken. Settling that needs one AS2-side measurement with a box up, not a
code change. It is handed over, not guessed at.

## 6. What the user sees now

All four captures are `-apshottime` screenshots — taken inside the process, so no overlapping window can corrupt
them — driven by `resources/tools/drive_input.py` with `--exe DishonoredGame_EO.exe`, real `SendInput` keys,
from the start screen, with nothing forced visible.

| | file | what it shows |
|---|---|---|
| before | `build/agentEO/beforeA_t40_head.png` | HEAD, after the same five LEFT presses: the brightness screen reads **`undefined`**, the knob is still at its authored centre and the picture is unchanged. It cannot be otherwise: HEAD's `execOnSettingChange` is `m_bOptionsChanged = TRUE;` and nothing else (`disgfxmovieplayermenubase.cpp:371`), and with `Setting_Minimum` / `Setting_Maximum` absent the asset has no range to move the knob along |
| after | `build/agentEO/afterC_t22.png` | the same screen reads **`BRIGHTNESS`** and the knob is at the profile's own value |
| after | `build/agentEO/afterF_t40_slidermoved.png` | after five LEFT presses the knob has moved left and the picture is visibly darker |
| after | `build/agentEO/optD_options.png` | the Options screen itself: GENERAL / GRAPHICS / AUDIO, the GAMEPLAY and USER INTERFACE sub-tabs, and the six gameplay rows with their localised names — *Auto-Use Mana Elixir, Kill Cam Mode, Difficulty, Auto-Save In Journal, Head Bob Amount, Chains Climbing Relative to Camera* |

The slider is not inferred from the picture. `build/agentEO/afterF_log.txt`:

```
[0056.31] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 2.1000 -> rounded 2, changed 1, gamma now 2.2000
[0059.32] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 2.0000 -> rounded 2, changed 1, gamma now 2.1000
[0062.32] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 1.9000 -> rounded 2, changed 1, gamma now 2.0000
[0065.37] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 1.8000 -> rounded 2, changed 1, gamma now 1.9000
[0068.30] DisOnSettingChange: id 112 (PSI_Graphics_Gamma) mapping 2 data 5 value 1.7000 -> rounded 1, changed 1, gamma now 1.8000
```

mapping 2 is `PVMT_Ranged` and data 5 is `SDT_Float`, so the row went down the ranged-float arm; `gamma now` is
the value *before* the reread, which is why it lags by one step. A RIGHT press from 2.2 produces exactly one
change, to 2.3, and then nothing — 2.3 is the mapping's own maximum, and the asset stops there.

The tree census, `build/agentEO/afterC_log.txt`:

```
DisShowSettingsCategoryList: 4 categories -> _root.optionsMenu_mc.FillCategories ok
DisFillOptionsMenu: 4 categories, 87 rows, 22 listeners, profile Transient.ArkProfileSettings_1, initial objective markers 1
```

Against `_root.options_mc` the same line reads `NOT FOUND` (`build/agentEO/afterB_log.txt`).

## 7. One thing this package broke, and how it is held

Porting `OnSettingChange` made the first slider move call `ArkSettings::OnSettingsChanged`, which republishes to
every listener `FindListeners` collects. **The game aborted on the first press**:
`build/agentEO/afterE_log.txt`, `Critical: appError called: DishonoredGame native not ported:
UDisPostProcessManager::ApplyGameSettings`. Nine DishonoredGame listeners still carry the generated `appErrorf`
`ApplyGameSettings` — `ADishonoredPlayerCamera`, `ADishonoredGameInfo`, `ADishonoredPlayerController`,
`ADishonoredPlayerPawn`, `UDisPostProcessManager`, `UDishonoredPlayerInput`, `UDisItemContext_AimAssistAttack`,
`UDisGFxMoviePlayerHUD`, `UDisGlobalUIManager`.

`UnUIDataStores.cpp` already meets this by gating its own `OnSettingsChanged` behind `-arksettings`.
`DisRepublishSettings` does the same: with `-arksettings` it runs retail's loop, and without it it rereads the
parameters exactly as retail rereads them and hands them to `UEngine` alone — the one listener whose body is
ported, and the one that carries the gamma to the client and the subtitle mode to the engine. Those nine
`ApplyGameSettings` bodies are the single thing standing between this and an options screen that applies
everything live, and they are the largest hand-over in this package.

## 8. Gates

* **Regression:** `python resources/tools/run_regression.py --build-dir
  D:/RecompileDishonored/Recompile/build/agentEO_wt/build/agentEO_reg` — the worktree's own copy, an absolute
  build dir, built inside the harness (no `--no-build`), after copying the five gitignored layout inputs
  (`all_types.h`, `retail_sdk_layout.json`, `script_classes_2012.json`, `script_classes_2013.json`, `types.json`)
  into the worktree. **37 ok, 0 failed, 0 skipped.** `build/agentEO/reg1.log`.
* **Clean gate build:** the build directory deleted first, full Release, `DISHONORED_LAYOUT_CHECKS=ON`, all three
  targets: **0 errors, 0 C4263, 0 C4264.** `build/agentEO/buildclean.log`.
* **`rva_sweep.py`** over the whole worktree: **6989 citations, 4005 `ok-2013`, 2960 `ok-2013-mid`, 22
  `ok-2012-labelled`, 0 unknown, 2 `MISLABELLED-2012`**. Both of the two are pre-existing, in
  `GFxUI/Src/gfxuirenderer.cpp:1669`, in a line that *does* say "2012" — the tool's 40-character lookback does not
  reach the front of a seven-address list — and `GFxUI` is another agent's this wave. Nothing in any file this
  package touched is flagged.
* **By hand, not by sweep:** every retail address this package cites was resolved in the 2013 database with
  `ida_funcs.get_func`, start-or-mid and name. All 31 land on a function **START**:
  `build/agentEO/resolve_2013.txt`. `0x539730`, `0x7db5f0`, `0x7e50c0`, `0x7e65f0`, `0x7bcad0`, `0x7cb870`,
  `0x7e6970`, `0x7c2580`, `0x5359b0`, `0x793dc0` and `0x7af370` have no name in the 2013 database; each was
  identified by its callers or its callees rather than by a symbol, and the identification is stated in
  `agentEO_status.csv`.

## 9. Merging

* **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is REQUIRED and WAS run.** It is
  what removes the four ported stubs from `DishonoredGameNativeStubs.cpp`. Its only content-bearing output is
  that one file: `Sources.cmake`, `DishonoredGame.h`, `DishonoredGameRegistrants.cpp` and the seventy-odd
  generated class headers came back byte-identical apart from line endings.
* **`Sources.cmake` did NOT change.** No source file was added or removed.
* **Files outside this package's own module: three, all in `Engine`, all three of them agent EK's own files from
  the last wave** — `Engine/Inc/arksettings.h`, `Engine/Src/arksettings.cpp`, `Engine/Src/UOnlinePlayerStorage.cpp`.
  Expect to merge them together with EK's.
* **Total: nine source files plus this report and its status table.** `build/agentEO_sync.py` is the
  authoritative copy list and runs worktree → main; `--check` reports without copying. Nothing is committed and
  the worktree is left dirty.

## 10. Hand-over

1. **The nine `appErrorf` `ApplyGameSettings` bodies** (§7). Until they exist, the options screen applies only
   what `UEngine::ApplyGameSettings` applies, and `-arksettings` aborts the game on the first slider move.
2. **`UDisGFxMoviePlayerGamma` has no implementation file at all.** `OpenGammaImage` (`0x7c2580`) and
   `CloseGammaImage` are stubs, `m_pGammaMenu` is never constructed, `UI_Gamma_SF` is never opened, and
   `m_BrightnessValues` is never filled — so the brightness screen's reference symbols are missing and the
   asset's `tweenTo` still lands on `undefined`. The label, the value and the slider do not depend on any of it.
3. **`ArkSettings::SaveSettings` (`0x5334e0`)** — nothing persists the profile, so a setting the player changes
   lives only for the session.
4. **`ArkSettings::GetSettingProvider` (`0x5396a0`) and `PCResolutionSettingProvider`.** The resolution row
   (id 115) is the one row with neither a value nor a value list.
5. **The objective-marker republish (`0x6cbf50`)** that `OnLeaveOptions` makes when setting 95 changed.
6. **The message box's `B`** (§5) — one AS2-side measurement with a box up, not a code change.
7. **For whoever owns `GFxUI`:** `FGFxEngine::InitKeyMap` ignores `[GFxUI.KeyMap]` and `FullKeyboard=1` in
   `DishonoredInput.ini`, and `External/GFx3/GFx3Gen.h`'s `GFxKey::Code` carries no `GAMEPAD_*` codes, so
   `XboxTypeS_B → GFxKey::Escape` is a stand-in for retail's `GAMEPAD_B`. It works for the menus and will
   misbehave for any content that distinguishes the pad's B from Escape. Not touched here.
8. **Two pre-existing mislabelled citations** in `GFxUI/Src/gfxuirenderer.cpp:1669` (§8) — a tool artefact, but
   worth a one-line fix by whoever owns that file.

## 11. Scratch

`build/agentEO/` — the decompiles (`dec2013/`), the IDA scripts (`resolve.py`, `callers.py`, `vtab.py`,
`imm.py`, `strs.py`, `slot.py`, `hunt.py`), the build logs, the drive logs, the run logs, the screenshots, the
sweep CSV and `resolve_2013.txt`. `build/agentEO_run.py` is the runner, `build/agentEO_sync.py` the copy list,
`build/agentEO_ida/` the IDA copy, `build/agentEO_headsrc/` the `git archive` export of HEAD and its build.
None of it is part of the package.
