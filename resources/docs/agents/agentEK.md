# Agent EK (PHASE11 EK) — the in-game HUD: it is built, laid out and live, and its pixels are held by two gaps in the shared GFx layer

Branched from HEAD `84b0c9d`; the worktree was moved to HEAD **`48115da`** mid-package by taking agent
EH's three `External/GFx3` files from it with `git checkout 48115da --`, so the tree measured here is
`48115da` plus this package and nothing else. Own worktree `build/agentEK_wt`, own build dirs
`build/agentEK_rel` (this package), `build/agentEK_headrel` (the untouched `48115da` baseline, built
from a `git archive` export in `build/agentEK_headsrc`), `build/agentEK_wt/build/agentEK_reg` (the
regression harness), `build/agentEK_clean` (the gate build). Own IDA copy
`build/agentEK_ida/retail2013_agentEK.i64`, headless decompiles only (31 retail functions into
`build/agentEK/dec2013/`). **No IDA MCP tool and no FModel tool was used.** No commits, nothing staged,
nothing written into the main checkout.

**This package touches no file in `External/GFx3` and no file in `GFxUI`.** The three GFx3 files in the
worktree are agent EH's, byte-identical to `48115da`, and are deliberately absent from
`build/agentEK_sync.py`.

## The answer in nine lines

* **The HUD is constructed, started and bound.** `UI_HUD.HUD` and `UI_HUDFX.HUDFX` open through the real
  path (`StartHUD` → `UDisGFxMoviePlayerBase::Start` → `UGFxMoviePlayer::Start` → `PostStart`), and
  PostStart binds **31 of 31** movie clips by path with **0 missing**, out of the two movies retail binds
  them out of. Before this package a run of `L_Tower_P` opens **no movie at all**.
* **It is laid out where retail puts it.** `PreRender_Layout` (2013 `0x796250`) is ported whole: at
  1280×720 on a 1280×720-authored movie with the tweaks' own safe-area ratio 0.900 it anchors the
  player-status block to **(64, 36)**, the stance icon to (64, 684), the objectives and the target
  notification to (1216, 36), the special interactions to (1216, 684) and everything bottom-anchored to
  720 − `blackStripes_mc._stripeH` = **650**. Those are the numbers the census line prints, and the
  arithmetic is retail's.
* **It is live.** `Tick_PlayerStatus` (2013 `0x79f6e0`) runs ~29 times a second off `PreAdvance` and
  pushes the player's real attributes into the movie the moment they change:
  `FillHealthGauge(100.0, 0, 0, 0, 100.0)` at t=3.91 s and `FillHealthGauge(32.0, 0, 0, 0, 32.0)` at
  t=5.24 s of the same run, both `-> ok` (the AS2 function resolves and is called), driven by
  `APawn::Health` going 100 → 32 from the game's own fall damage. `FillManaGauge` and the equipment
  slot are fed the same way.
* **Its pixels are held by two gaps in the shared GFx layer, and both are measured, not guessed.**
  (1) **Every bitmap fill in the HUD is an atlas sub-image and this tree cannot texture one**: nine
  distinct `def SubImage` fills report `GFx fill NOT TEXTURED` and draw nothing. (2) **The stencil mask
  path clips away what is left**: with `BeginSubmitMask`/`EndSubmitMask`/`DisableMask` turned into
  no-ops (`build/agentEK/nomask_patch.py`, a scratch diagnostic that is NOT in the package) HUD geometry
  appears on screen at exactly the position above; with them on, nothing from the movie reaches the
  frame. Section 5.
* **Three defects in the settings layer, measured against the cooked profile's own mapping table.**
  Retail 2013's `UOnlinePlayerStorage::GetProfileSettingValueId` (`0x4f36b0`) accepts mapping type **3 or
  4**; this tree accepted 3 alone, and Dishonored gives every boolean option type 4. `ArkSettingsParameters`
  had **ten** HUD show flags where retail has **eleven**, so every field from `m_CrosshairStyle` on was one
  slot early. And `ArkSettingsParameters::Read`'s HUD block used the **2012** profile ids (85..98) where
  retail 2013 uses **87..101**. Together they made the HUD's own visibility setting read `HV_Off` when the
  profile's shipped default is `HV_Always`. All three are fixed; section 4.
* **`ADishonoredHUD` is no longer inert.** `PostBeginPlay` (`0x5fa6b0`), `Tick` (`0x601e70`) and the seven
  show-flag/player-info members are ported, and the show-flag word the whole HUD is gated on reads
  **0x7FEF** every frame in a live level.
* **The HUD's own call site is back.** `ADishonoredPlayerController::PreRender` (`0x6a0c50`) is a native
  virtual in retail and not a script event, so it arrives through a function-pointer seam in the Engine —
  `GDisPlayerPreRenderHook`, three lines, the same shape as the `GDisEngineTickHook` already there — called
  at exactly the line `Engine/Src/UnPlayer.cpp` already marks `PreRender is not a 2013 event`.
* **Regression 37 ok, 0 failed, 0 skipped**, built inside the harness; a clean full Release build of the
  three targets with `DISHONORED_LAYOUT_CHECKS=ON` and the build directory deleted first: **0 errors, 0
  C4263, 0 C4264**; `rva_sweep` over this package's ten source files: **198 citations, 145 `ok-2013`, 53
  `ok-2013-mid` (all of them lines that say "2012" in words), 0 mislabelled, 0 unknown**.
* **`gen_classes_header.py` regeneration is NOT required and was NOT run**; `Sources.cmake` **did** change,
  by one line. Section 8.

## 1. What the package is

`resources/docs/agents/agentEK_status.csv` has every address. In order of the chain:

| retail 2013 | what | where |
|---|---|---|
| `0x5fa6b0` | `ADishonoredHUD::PostBeginPlay` — all six show-flag levels to `0x7FEF`, the player-info timer to its duration | `dishonoredhud.cpp` |
| `0x5ea0a0` / `0x5ea0c0` / `0x5ea0e0` / `0x5ea100` / `0x5ea130` | `EnableHUDElements` / `DisableHUDElements` / `IsHUDElementEnabled` / `...ByAll` / `...Once` | `dishonoredhud.cpp` |
| `0x5ea160` / `0x5ea1b0` | `RequestPlayerInfoDisplay` / `UpdatePlayerInfoDisplay` — the countdown that takes the health and mana bars away again in contextual mode | `dishonoredhud.cpp` |
| `0x601e70` | `ADishonoredHUD::Tick` — the one test that decides whether the player info is on screen | `dishonoredhud.cpp` |
| `0x78cd00` | `UDisGFxMoviePlayerHUD::StartHUD` | `disgfxmovieplayerhud.cpp` |
| `0x787e30` | `UDisGFxMoviePlayerHUD::CloseHUD` | `disgfxmovieplayerhud.cpp` |
| `0x795650` | `PostStart` (retail vtable slot 117, `+468`) — two zero advances, the 31 clip bindings, the crosshair and popup state, the two measured offsets, `ApplyCurrentSettings` | `disgfxmovieplayerhud.cpp` |
| `0x7af370` | `ApplyGameSettings` — `ArkSettingsParameters` → `FDisHUDSettings` | `disgfxmovieplayerhud.cpp` |
| `0x7b2280` | `PreAdvance` (slot 119, `+476`) — the AND of the six show-flag levels, the crosshair chase, `Tick_PlayerStatus`, `Tick_PlayerState` | `disgfxmovieplayerhud.cpp` |
| `0x79f6e0` | `Tick_PlayerStatus` — health, mana, equipment, and `masterHUD_mc.Open` / `.Close` | `disgfxmovieplayerhud.cpp` |
| `0x797d40` | `UpdateHealthGauge` — `masterHUD_mc.FillHealthGauge(pct, small, large, low, pctWithRegen)` | `disgfxmovieplayerhud.cpp` |
| `0x797f80` | `UpdateManaGauge` — `masterHUD_mc.FillManaGauge(pct, upgrade, available, NOT visible, low, pctWithRegen)` | `disgfxmovieplayerhud.cpp` |
| `0x7981f0` | `UpdateEquipmentInfo` — `SetPower` / `SetWeapon` / `SetEmptyEquipment` | `disgfxmovieplayerhud.cpp` |
| `0x795ed0` | `Tick_PlayerState` — `playerStatesIcon_mc.SetIcon(null / "sneak" / "crouch" / "slide")` | `disgfxmovieplayerhud.cpp` |
| `0x794c10` / `0x794d40` | `HideGauge` / `UpdateGauge` — the breath, interaction, grenade-cook and skip-cutscene gauges | `disgfxmovieplayerhud.cpp` |
| `0x78cd40` | `ShouldAlwaysShowPlayerInfo` | `disgfxmovieplayerhud.cpp` |
| `0x7a5840` | `PreRender` — the movie space against this frame's canvas, the triple-screen viewport, then the layout pass | `disgfxmovieplayerhud.cpp` |
| `0x796250` | `PreRender_Layout` — every element of the interface, anchored | `disgfxmovieplayerhud.cpp` |
| `0x787860` | the movie-space math of `ComputeMovieSpaceInfo`, which `PostStart` and `PreRender` both inline | `disgfxmovieplayerhud.cpp` |
| `0x83d4a0` | `UDisGlobalEnums::PowerWheelItemIsPower` (`Item > 20`) | `disgfxmovieplayerhud.cpp` |
| `0x83d290` | the `(EeDisUISelectionType, EeDisAmmoType)` → `EDisPowerWheelItem` table, **new in the 2013 build** (no 2012 body, so no PDB name) | `disgfxmovieplayerhud.cpp` |
| `0x6a0c50` | `ADishonoredPlayerController::PreRender` | `disgfxmovieplayerhud.cpp`, through the Engine seam |
| `0x4f36b0` | `UOnlinePlayerStorage::GetProfileSettingValueId` — mapping type 3 **or 4** | `UOnlinePlayerStorage.cpp` |
| `0x539730` | `ArkSettingsParameters::Read` — the HUD/crosshair block's retail 2013 ids | `arksettings.cpp` |

`m_pMovieClips` is indexed by **retail's own `EDisHUDMovieClip`**, which is already in this tree
(`DishonoredGameEngineShims.h`). That is not decoration: the enum's 32 members line up one for one with
the 32 `GetVariable` calls of `PostStart`, and `DHMC_PlayerStatus` = 3 = `_root.masterHUD_mc`,
`DHMC_Cinematic` = 28 = `_root.blackStripes_mc`, `DHMC_CrosshairAspect` = 1 = the one slot retail sets to
null and never binds. The independent agreement between the enum and the decompile is what makes the
clip table certain rather than plausible.

## 2. How it is reached, and why that is not a fallback

Retail constructs the HUD in `UDisGlobalUIManager::RefreshGlobalUIState` (2012 `0x8b75e0`) out of the
config movie set, and `UDisGlobalUIManager` is agent EI's package, not this one. What this package does
is the HUD's share of that function and nothing else, behind **`-dishud`**:

1. find or construct `ADishonoredGameInfo::m_pGlobalUIManager` — the real config object, so its
   `m_DefaultUI.m_HUDMoviePath` is `DisUI.ini`'s own `"UI_HUD.HUD"` and `DisGetGFxHUD()` (2013
   `0x7bf730`, already in this tree) answers for everyone;
2. load the two cooked movies the way `UDisGFxMoviePlayerBase::LoadMoviePackage` (2013 `0x7a3fd0`) loads
   them — the package file name gets the seek-free `_SF` suffix, the object path does not;
3. construct `UDisGFxMoviePlayerHUD` and `UDisGFxMoviePlayerHUDFX`, set `m_pTweaks` from the tweaks class
   default object (which is `LoadMoviePackage`'s own fallback when `GetTweaks_Derived` answers null), set
   `m_pHUD` on the manager, and call `StartHUD()`.

Everything from `StartHUD` on is the real path: `UDisGFxMoviePlayerBase::Start` → `UGFxMoviePlayer::Start`
→ `LoadMovie` → `CreateMovie` → `StartScene` → `PostStart`, and then `Advance` → `PreAdvance` every frame
and `UGameViewportClient::Draw` → `PreRender` every frame. It is a switch for the same reason `-gfxuimenu`
is one: `run_regression.py`'s thresholds are calibrated on command lines with no movie open, and the
harness does not pass `-dishud`, so the regression measures the same thing it did before (§7 proves it: 0
HUD lines and 0 movies in the harness's own d3d9 log).

`PostStart` and `PreAdvance` are virtual in retail. Declaring them would mean adding them to
`CppText/UDisGFxMoviePlayerHUD.h` and regenerating `DishonoredGameUIClasses.h`, which is agent DG's
deviation 5 word for word (`GFxUI/Inc/gfxuiengine.h`), so they arrive as free functions through the
`GGFxMoviePlayerStartedHook` / `GGFxMoviePlayerPreAdvanceHook` seam DG already built — the same order, the
same arguments, the same call sites. The dispatch is two `else if( Cast<UDisGFxMoviePlayerHUD> )` branches
in `disgfxmovieplayermainmenu.cpp`, which is where that seam lives. `ApplyGameSettings` is the same case
and is called at its one retail call site, the tail of `PostStart`.

## 3. The measurement

`build/agentEK_rel`, `L_Tower_P?Name=Corvo?Team=255 -windowed -ResX=1280 -ResY=720 -nomovie -nosteam
-skipnativepkgs=OnlineSubsystemPC -dishud`, through `build/agentEK_run.py`. The same driver and the same
command line on the untouched-`48115da` executable in `build/agentEK_headrel` is the before leg.

| | HEAD `48115da` | agent EK |
|---|---:|---:|
| movies opened in `L_Tower_P` | **0** | **2** (`UI_HUDFX.HUDFX`, `UI_HUD.HUD`) |
| movies drawn | 0 | 2 |
| HUD census lines in the log | 0 | 63 |
| movie clips bound / missing | — | **31 / 0** |
| display objects in the GFx frame census | 0 | 94 (31 sprites, 26 shapes, 3 text fields, 10 bitmap fills) |
| draws, triangles, mask passes per frame | 0 | 16, 166, 6 |
| `Tick_PlayerStatus` ticks | 0 | ~29 per second |
| `masterHUD_mc.Open` | — | **ok** (the AS2 function resolves and is called) |

Logs: `build/agentEK/head_before_log.txt` and `build/agentEK/ek_after_log.txt`; bitmaps
`head_before.png` and `ek_after.png`.

### 3.1 The census lines this package adds

```
DISHONORED(bringup): HUD census: 31/31 clips bound, 0 missing, movie 1280x720, screen 1280x720,
    movie space 1280x720, empty 0.0/0.0, scale 1.0000, visibility 2
DISHONORED(bringup): HUD: opened UI_HUD.HUD through DisGFxMoviePlayerHUD (fx UI_HUDFX.HUDFX),
    tweaks Twk_GFxMoviePlayerHUD safe area 0.900, low health 30 low mana 19
DISHONORED(bringup): HUD viewport census: canvas 1008x567 -> buffer 1008x567, rect 0,0 1008x567,
    scissor 0,0 1008x567, scale 1.000 aspect 1.000 flags 0x1000
DISHONORED(bringup): HUD frame rect 0.0,0.0 .. 1280.0,720.0
DISHONORED(bringup): HUD layout census: canvas 1008x567, safe 0.900, left 64.0 top 36.0 right 1216.0
    bottom 684.0, stripeH 70.0, movie 1280x720, space 1280x720, empty 0.0/0.0
DISHONORED(bringup): HUD gauge census: FillHealthGauge(100.0, 0, 0, 0, 100.0) -> ok [health 100/100 regen 0]
DISHONORED(bringup): HUD gauge census: FillManaGauge(0.0, 0, 1, 0, 1, 0.0) -> ok [mana 0/0 regen 0]
DISHONORED(bringup): HUD gauge census: equipment item 0, ammo 0/0, power 0/0
DISHONORED(bringup): HUD masterHUD_mc.Open -> ok
DISHONORED(bringup): HUD status census: 38 ticks, show flags 0x7FEF, health 31/100 (+0), mana 0/0 (+0),
    item 0 ammo 0/0, player status open, gauges 1/1
```

The canvas is 1008×567 and not the 1280×720 asked for because this desktop scales a DPI-unaware window
by 78.75 %; the same happens at HEAD and to the regression harness, and the layout is computed from the
canvas either way.

### 3.2 Health and mana change with the underlying values — measured

One run of `L_Tower_P`, no forcing of any kind, `build/agentEK/gauges_log.txt`:

```
[0003.91] HUD gauge census: FillHealthGauge(100.0, 0, 0, 0, 100.0) -> ok [health 100/100 regen 0]
[0005.24] HUD gauge census: FillHealthGauge(32.0, 0, 0, 0, 32.0) -> ok [health 32/100 regen 0]
```

`Tick_PlayerStatus` rebuilds `FDisPlayerStatus_Health` from `APawn::Health`, `APawn::HealthMax` and
`ADishonoredPawn::m_HealthRegenAmount` every tick and calls `UpdateHealthGauge` **only when the struct
changed**, which is retail's own test — so two calls in ~700 ticks is the correct behaviour, and the
second one carries the value the pawn actually has after the game's own fall damage. The invoke returns
`ok`, so `masterHUD_mc.FillHealthGauge` exists in the movie and ran.

**Mana is fed the same way and does not change in this level, and that is retail's state, not a gap**:
in the first mission Corvo has no powers, so `ADishonoredPlayerPawn::m_Mana` and `m_ManaMax` are both 0
and the gauge is invoked once with `FillManaGauge(0.0, 0, 1, 0, 1, 0.0)`. I could not produce a non-zero
mana in any level this tree can reach: `-disrestoreslot=16` and `=17` report `0 level state(s) in the
save` on both `L_Tower_P` and `DishonoredGameFull_P` in my runs and leave the session at health 100 /
mana 0, so agent EF's restored 70 / 100 was not reproducible here. **I did not measure a changing mana
number.** What is measured is that the mana gauge reads `m_Mana`, `m_ManaMax` and `m_ManaRegenAmount`
off the live pawn, is re-invoked whenever any of them changes, and that its three bits come from
`ADishonoredHUD::IsHUDElementEnabled(4, 4)`, the possession test and the bone-charm attribute modifier.

The equipment slot is invoked once with item 0 → `SetEmptyEquipment`, which is also retail's first-mission
state (Corvo starts unarmed); `ADishonoredPawn::m_pInventory->GetEquippedItem(EDisEquipUsage_Secondary)`
answers null because `SpawnInventoryLoadout` is only half ported (agent EF).

## 4. Three defects in the settings layer, and how the profile was read

`UDisGFxMoviePlayerHUD::ApplyGameSettings` writes `FDisHUDSettings` out of `ArkSettingsParameters`, and
`ADishonoredHUD::Tick` and `Tick_PlayerStatus` are both gated on `m_Settings.m_HUDVisibility`. It read
`HV_Off` and the HUD stayed closed. The profile itself was dumped, per setting id, with its mapping type
and its raw value (`build/agentEK/r3_log.txt`):

```
DISHONORED(diag): profile class Engine.ArkProfileSettings, ProfileSettings 114, ProfileMappings 124
DISHONORED(diag): mapping id 87 type 3 values 3 raw 2      <- HUD visibility = HV_Always
DISHONORED(diag): mapping id 88..98 type 4 values 2 raw 1  <- ELEVEN booleans, all true
DISHONORED(diag): mapping id 99 type 3 values 3 raw 2      <- crosshair style = CS_Normal
DISHONORED(diag): mapping id 100 type 4 values 2 raw 1     <- crosshair movement
DISHONORED(diag): mapping id 101 type 2 values 0 raw 100   <- crosshair opacity
```

Three separate things were wrong, and each is fixed by what retail itself does:

1. **`UOnlinePlayerStorage::GetProfileSettingValueId` accepted only `PVMT_IdMapped`.** Retail's body
   (2013 `0x4f36b0`) is `if ( v12 != 3 && v12 != 4 ) return 0;` — mapping type **3 or 4**. Dishonored's
   `ArkProfileSettings` gives every boolean option a mapping type of 4, one past stock UE3's `PVMT_MAX`,
   and all eleven HUD show flags are such options, so with the stock test none of them was readable.
   One line in `Engine/Src/UOnlinePlayerStorage.cpp`.
2. **`ArkSettingsParameters` had ten HUD show flags where retail 2013 has eleven.** `ApplyGameSettings`
   copies members 17..27 into `FDisHUDSettings`' eleven bits and then reads member **28** as
   `m_CrosshairStyle`; `Read` fills 17..27 from ids 88..98 and 28..30 from 99..101. Without
   `m_bShowObjectiveMarkers` the struct is 208 bytes where retail's is 212 and every field from the
   crosshair style on is one slot early. Added at retail's position in `Engine/Inc/arksettings.h`.
3. **`ArkSettingsParameters::Read`'s HUD block used the 2012 profile ids.** The two id lists are offset by
   two: this tree read id 85 for the HUD visibility where retail 2013 reads 87, and id 96 for the
   crosshair style where retail reads 99. The block now uses retail 2013's own run, 87..101, taken off
   `ArkSettingsParameters::Read` (2013 `0x539730`).

After the three, `HUDVisibility 2, crosshair 2/1/0, popups 1 tuto 1 inter 1 hl 1 log 1 icons 1 stance 1
objm 1 gren 1 aware 1 heart 1` — the shipped defaults, and `visibility 2` in the HUD census.

**The ids below that block are still the 2012 list** (65..84 and 101..), which is why id 101 is now read
twice: once as the crosshair opacity, which is what retail reads it as, and once as
`m_bAutoUseManaElixir`, which is what the 2012 list calls it. Moving the rest is the options package's
call, not this one's — and `m_CrosshairOpacity` still reads 0 because id 101 is mapping type 2
(`PVMT_Ranged`) and `GetProfileSettingValueInt` does not answer for it. Hand-over 3.

## 5. Why nothing is on screen yet, and how that was established

Everything above is true and the frame still contains no HUD. Two gaps in the shared GFx layer, both
measured:

1. **Every bitmap fill in the HUD movie is an atlas sub-image, and a sub-image fill draws nothing.**
   `-gfxuidrawtrace` (`build/agentEK/r11_log.txt`) reports nine distinct fills as
   `GFx fill NOT TEXTURED: image id <n> ... type 0x41 def SubImage`. The base atlases themselves load
   (`GFx image UI_HUD.HUD_I1: 560x248`, `HUD_I2: 608x232`), and `GFxImageCharacterDef::GetTexture` does
   have a sub-image branch — but a probe of it showed the `BaseImageId` it resolves is 0 or 5 for the
   failing fills (`base id 0 -> def NULL`, `base id 5 -> type 133`) and only 1 and 2 resolve to an
   `RT_Image` def. So the sub-image tag's base id, or the id space it is resolved in, is wrong.
2. **The stencil mask path clips away what is left.** The HUD submits **6 mask passes** and 166 triangles
   every frame with a valid depth-stencil surface bound (`BeginSubmitMask mode 0 ref 1 counter 1 test 7
   op 2 rt 1 depth 1`), and none of it reaches the frame. With `BeginSubmitMask` / `EndSubmitMask` /
   `DisableMask` turned into no-ops, HUD geometry **does** appear — at the top left, which is where
   `PreRender_Layout` put it (`build/agentEK/nomask.png` against `build/agentEK/on1.png`, the same run
   with and without the switch). The control that says this is not a render-path problem in general is
   `build/agentEK/menuinlevel.png`: the same map with `-gfxuimenu`, where `UI_Global.Global` — a movie
   with **0 masks** — draws its cursor over the world correctly.

Neither is this package's, and neither is small. `build/agentEK/nomask_patch.py` re-applies the mask
diagnostic to the worktree and reverts it; **the tree handed over has neither change**, and
`git status` in `build/agentEK_wt` shows no file in `External/GFx3` or `GFxUI` modified by this package.

**So: acceptance 1 is not met.** There is no screenshot of the HUD drawn normally, because with this
tree's GFx runtime there cannot be one. `build/agentEK/nomask.png` is a diagnostic and is labelled as
one; it is not the HUD working.

## 6. What is NOT ported, and named rather than left silent

Of `UDisGFxMoviePlayerHUD`'s 157 PDB functions this package ports 18 and `ADishonoredHUD`'s 10. Named in
the source at their retail addresses, in `PreAdvance`'s and `PreRender`'s own comments:

* `Tick_Stealth` (`0x796050`), `Tick_Interaction` (`0x78d3b0`), `Tick_Tutorials` (`0x7afa60`),
  `Tick_ObjectivePopup` (`0x7afe70`), `Tick_Markers` (`0x7b00c0`), `Tick_Subtitles` (`0x79fe20`),
  `Tick_Cinematic` (`0x7960e0`) and the interaction probe's five checks — the rest of `PreAdvance`;
* `PreRender_Crosshair` (`0x79ff30`), `PreRender_Markers` (`0x7a0890`), `PreRender_Damage` (`0x7970f0`),
  `PreRender_Keyhole` (`0x797b20`) — the four passes that project world positions through the canvas;
* `PreClose` (slot 118, `0x7af820`), which frees `m_pMovieClips` and resets ~30 members;
* `ADishonoredHUD::TickHUD_Powers` (`0x5fd880`), the power post-process intensity ramp (package DO's path);
* the equipment slot's re-equip lookup (`GetReequipItem` `0x8055e0`, `FindItemByClass` `0x80b4a0`), the
  two power bits (`UDishonoredActivePowerComponent` vtable `+352` and `+396`), the infinite-ammo answer
  (`ADishonoredGameInfo` vtable `+1060`), `ADishonoredPawn::ArePowersInhibited` (`0x749020`, whose
  `m_fPowersInhibitedUntil` is not declared in this tree), `IsPossessing` (`0x74a550`) and `IsSliding` —
  each one a named accessor this tree does not declare, each one marked at its use;
* the five take-downs `ApplyGameSettings` runs when a setting turns something off (`0x78cf30`,
  `0x7ad010`, `0x79f0d0`, `0x79f170`).

The **power wheel's resting state** is a separate movie player (`UDisGFxMoviePlayerPowerWheel`,
`UI_PowerWheel.powerwheel`) and is not opened by this package; at rest it is closed, and
`UDisGFxMoviePlayerPowerWheel::IsActive` (`0x7b9ee0`) — which `ADishonoredHUD::Tick` reads — is
`m_bWheelIsOpen && (Mask & m_Mode)`, both zero while it is closed, so the branch is inert and correct.
What retail shows at rest for the wheel is the equipped-item slot **inside** `masterHUD_mc`, which is
`UpdateEquipmentInfo` and is ported.

## 7. Verification

* **Regression**: `python resources/tools/run_regression.py --build-dir build/agentEK_reg --exe-name
  DishonoredGame_EKreg.exe --log-prefix EKreg`, run from `build/agentEK_wt` so the harness builds **this
  package's** sources (`build-release.cmd` cds to its own tree). The worktree needs the five generated
  files `resources/docs/types/{all_types.h, retail_sdk_layout.json, script_classes_2012.json,
  script_classes_2013.json, types.json}` copied in first — they are gitignored, so a worktree has none and
  the layout stage cannot run without them. Result **37 ok, 0 failed, 0 skipped, 494 s** (`build/agentEK/regression3.log`).
  A first attempt read `d3d9_frames 450` against a bound of 1000; the same stage re-run alone read **2340**,
  and its startup time was 39.4 s against 3.5 s - four agents were building on this machine at the time.
  The harness's own note calls that bound a cliff detector, and the run above is the clean one.
  The harness never passes `-dishud`: its own d3d9 log has **0** `DISHONORED(bringup): HUD` lines and
  **0** `GFx scene started`, so this package is inert in every regression stage.
* **Clean build**: `build/agentEK_clean` deleted first, `LAYOUT=1`, all three targets. **0 errors, 0
  C4263, 0 C4264.**
* **rva_sweep**: `python resources/tools/rva_sweep.py --path
  D:/RecompileDishonored/Recompile/build/agentEK_wt/source/Development/Src`. Over this package's ten
  source files: **198 citations, 145 `ok-2013`, 53 `ok-2013-mid`, 0 MISLABELLED-2012, 0
  UNKNOWN-CLAIMED-2013**. Every `ok-2013-mid` is a line whose own words say "2012 0x…", which is what the
  tool classifies as mid when that 2012 value lands inside a 2013 function. The whole-tree sweep reports
  3 suspects, all pre-existing and all in files this package does not touch
  (`DishonoredGame/Inc/CppText/UDisAISubState.h:16`, `GFxUI/Src/gfxuirenderer.cpp:1669` twice).

## 8. Merging

* **`gen_classes_header.py` regeneration: NOT required, and NOT run.** No new class, no new reflected
  property, no new interface override was declared. The two CppText headers gain only ordinary member and
  friend declarations, which the generator does not read.
* **`Sources.cmake` changed**, by one line: `Src/disgfxmovieplayerhud.cpp` leaves the
  `DishonoredGame_EXCLUDE` list, because it is no longer a comment-only skeleton. A run of
  `python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header
  --sources-cmake` produces exactly that edit, so regenerating later is safe and idempotent.
* **Files outside the DishonoredGame module (5, all in Engine):**
  `Engine/Inc/UnWorld.h` (one `extern` beside `GDisEngineTickHook`), `Engine/Src/UnPlayer.cpp` (its
  definition and the one call at the line already marked `PreRender is not a 2013 event`),
  `Engine/Inc/arksettings.h` (`m_bShowObjectiveMarkers`), `Engine/Src/arksettings.cpp` (the HUD block's
  retail 2013 profile ids), `Engine/Src/UOnlinePlayerStorage.cpp` (mapping type 3 or 4).
  **Nothing in `External/GFx3`. Nothing in `GFxUI`.**
* **Inside the module (6):** `Inc/CppText/ADishonoredHUD.h`, `Inc/CppText/UDisGFxMoviePlayerHUD.h`,
  `Src/dishonoredhud.cpp`, `Src/disgfxmovieplayerhud.cpp`, `Src/disgfxmovieplayermainmenu.cpp` (the two
  seam branches), `Sources.cmake`.
* **Total: 11 source files**, plus this document and `agentEK_status.csv`.
  `build/agentEK_sync.py` is the authoritative list.
* **Sequencing note for agent EI**: `disgfxmovieplayermainmenu.cpp` is the file the GFxUI seam lives in;
  my change there is two `else if( Cast<UDisGFxMoviePlayerHUD>( Player ) )` branches in
  `DisGFxMoviePlayerStarted` and `DisGFxMoviePlayerPreAdvance`, plus two forward declarations. When
  `UDisGlobalUIManager` is ported, `DisHUDBringUp` in `disgfxmovieplayerhud.cpp` is the function to
  delete: it is the manager's job and it is marked as such.

## 9. Deviations

1. **The bring-up opener** (§2). `RefreshGlobalUIState` is not ported; its HUD share is reproduced behind
   `-dishud`, and it constructs `m_pGlobalUIManager` if nothing else has.
2. **`PostStart`, `PreAdvance` and `ApplyGameSettings` are free functions, not virtual overrides**, for
   agent DG's reason (`GFxUI/Inc/gfxuiengine.h`): declaring them means regenerating
   `DishonoredGameUIClasses.h`.
3. **`ADishonoredPlayerController::PreRender` arrives through an Engine function-pointer seam**, because
   Engine cannot link DishonoredGame. Same shape as `GDisEngineTickHook`.
4. **`Tick_PlayerState` reads the crouch bit alone.** Retail gates it on `IsPossessing`, the pawn's vtable
   slot `+1660` and `IsSliding`; none of the three is declared here, so state 3 (slide) is unreachable.
5. **`FDisPlayerStatus_Mana::m_bAvailable` tests possession only**, not
   `ADishonoredPawn::ArePowersInhibited`, whose `m_fPowersInhibitedUntil` this tree does not declare.
6. **The tweaks come from the class default object**, not from the `Twk_UI` SF tweaks package
   `m_SFTweaksPackageName` names — which is `LoadMoviePackage`'s own fallback, and gives safe area 0.900,
   low health 30, low mana 19.
7. **Three Engine settings files changed** (§4). They are outside the HUD but the HUD cannot be correct
   without them, and each change is retail's own instruction.

## 10. Hand-overs

1. **Atlas sub-image fills draw nothing** (§5.1). Nine distinct `def SubImage` fills in `UI_HUD.HUD` and
   `UI_HUDFX.HUDFX` report `GFx fill NOT TEXTURED`. `GFxImageCharacterDef::GetTexture` resolves
   `BaseImageId` through `GFxMovieDataDef::GetCharacterDefById` and gets NULL for base id 0 and a
   non-`RT_Image` def for base id 5, while 1 and 2 resolve. Either `GFx_DefineSubImageLoader`
   (`GFxTagLoaders.cpp:161`, 2012 `0xa369d0`) reads the wrong field for the base id, or the base ids live
   in the imported movie's id space. **This is what stands between this package and a visible HUD**, and
   it is `External/GFx3`.
2. **The stencil mask path clips every masked draw away** (§5.2). Six mask passes a frame with a valid
   depth-stencil bound and nothing survives them; the geometry appears the moment the three mask entry
   points are no-ops. The code in `GFxUI/Src/gfxuirenderer.cpp` looks right at the state-machine level
   (`Mask_Clear` → clear stencil, ref 1, `SO_Replace`; `EndSubmitMask` → `CF_Equal` ref
   `StencilCounter`), so the next step is at the RHI level, not the logic level. `build/agentEK/nomask_patch.py`
   reproduces the experiment in one command.
3. **The rest of `ArkSettingsParameters::Read` is the 2012 profile id list** (§4). Every id below 87 and
   above 101 is two off retail 2013's, so the mouse sensitivity, the gamma, the volumes and the subtitles
   mode are all reading the wrong setting or nothing. The dump that resolves them is
   `build/agentEK/r3_log.txt`; the retail body is `0x539730`. `m_CrosshairOpacity` in particular needs a
   reader for mapping type 2 (`PVMT_Ranged`).
4. **`UDisGlobalUIManager`** (agent EI). When `Init` (2012 `0x8c45d0`) and `RefreshGlobalUIState`
   (`0x8b75e0`) land, `DisHUDBringUp` goes away and `-dishud` with it; the HUD should then open when the
   game enters a level and close when it leaves, which is what `CloseHUD` and `PreClose` are for.
5. **`SpawnInventoryLoadout`'s other half** (agent EF). Until the player spawns with the sword, the
   equipment slot is correctly but uninterestingly `SetEmptyEquipment`, and `SetWeapon`'s ammo path has
   never run.
6. **No retail HUD reference exists.** `resources/reference/` has four menu images and nothing of the
   in-game interface, so every placement claim in this document is against retail's own
   `PreRender_Layout` arithmetic and not against a picture. A screenshot of the retail game's first
   mission would make the next HUD package far cheaper to check.

## 11. Files

Mine (11 source, 2 documents):

`DishonoredGame/{Inc/CppText/ADishonoredHUD.h, Inc/CppText/UDisGFxMoviePlayerHUD.h, Src/dishonoredhud.cpp,
Src/disgfxmovieplayerhud.cpp, Src/disgfxmovieplayermainmenu.cpp, Sources.cmake}`;
`Engine/{Inc/UnWorld.h, Inc/arksettings.h, Src/UnPlayer.cpp, Src/arksettings.cpp,
Src/UOnlinePlayerStorage.cpp}`; `resources/docs/agents/{agentEK.md, agentEK_status.csv}`.

Scratch, not repo tools: `build/agentEK/` — `dec.py`, `vt.py`, `xrefs.py` (the headless IDA helpers),
`dec2013/` (31 retail decompiles), `bmp2png.py`, `sweepcount.py`, `nomask_patch.py` (the mask
diagnostic, applied and reverted), the run logs `before_*`, `r1`..`r12`, `off1`, `on1`, `nomask`,
`menuinlevel`, `gauges`, `slot16*`, `head_before`, `ek_after`, the build logs `b1`..`b19`,
`regression*.log`, and the screenshots; `build/agentEK_build.cmd`, `build/agentEK_playbuild.cmd`,
`build/agentEK_headbuild.cmd`, `build/agentEK_run.py`, `build/agentEK_sync.py`; the snapshot worktree
`build/agentEK_wt`; the pristine HEAD export `build/agentEK_headsrc` and its build
`build/agentEK_headrel`; build dirs `build/agentEK_rel`, `build/agentEK_playrel`, `build/agentEK_clean`,
`build/agentEK_wt/build/agentEK_reg`.

IDA: **own copy only**, `build/agentEK_ida/retail2013_agentEK.i64` (a copy of
`resources/docs/idb/retail2013_named.i64`), opened headlessly through `resources/tools/ida/run.py`.
**No IDA MCP tool and no FModel tool was used.** No commits, no `git add`, no junctions, nothing deleted
under `Dishonored_Latest2026`, nothing written into the main checkout.
