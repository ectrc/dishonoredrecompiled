# Agent HE — wave 25 (PHASE22): cinematic HUD state, ToggleHUDElement, debug text, stance icon

Worktree `build/agentHE_wt` (detached at `9faefc0`), build dir `build/agentHE` (RelWithDebInfo), staged
`DishonoredGame_HE.exe`. Two builds: build 1 = every code change below; build 2 = the gate's CoreSmoke/LayoutProbe
(no source change). Sync: `python build/agentHE_sync.py [--apply]`.

## Result

| measure (route `-finewgame=50 -fosoiree`, 1920x1080) | control (`head_rel` / HEC1) | HE (build 1) |
|---|---|---|
| `cutscene_compare` vials, frames on, world 50–175 | 125/126 | **0/126** |
| stance | 25/126 | **0/126** |
| debugtext detector | 0/126 (detector missed it) | 0/126 |
| `MIC::SetScalarParameterValue : Modifying ...` log lines (= on-screen lines) | 3 (HEC1.log 58.76 s) | **0** |
| ride SSIM / MAD (median, world 56.2–150.5) | .523 / 29.4 | .537 / 28.7 |
| `masterHUD_mc` Open after the soirée exit | 159.88 s (HEC1) | none: level 4 = 0x7EE1 |
| `SetIcon` after the ride | sent (sneak icon in `hec1` shot 5) | none on the whole route |

Report: `build/cmp/HE1/report.md` (`python resources/tools/cutscene_compare.py run --exe ...DishonoredGame_HE.exe --tag HE1
--baseline build/cmp/head_rel/scores.csv -- -phuddiag -diskismet`, log `CC_HE1.log`). Control shots
`build/agentHE/shots/hec1/` (vials + the cyan engine line at world 55, vials + stance icon at 165); HE walk shots
`build/agentHE/shots/he1/` (GB's gb2 route + 10 s hold, `HE1.log`): nothing drawn.

## What was wrong, and the fix (function by function)

1. **`DisGFxMoviePlayerHUDPostStart` (2013 `0x795650`)** — the HUD movie's own start state is *open* (masterHUD_mc
   visible). 2012 (`0x7fb450`) invoked `masterHUD_mc.Close` here; **2013 replaced it with `SetDisplayInfo` on
   `m_pMovieClips + 48` (= `masterHUD_mc`) with `VarsSet 0x40, Visible 0`** (Visible is at +48 in `GFxValue::DisplayInfo`,
   `v51`). Our port hid `_dot_mc` (index 0) instead; nothing ever hid the vials, and `Tick_PlayerStatus` sends Open/Close
   only on a change of the cached `m_bPlayerStatusVisible`, which PostStart sets to 0 — so a re-started movie kept them
   drawn through the fade and the ride. Now masterHUD_mc is hidden; the dot stays hidden as a marked bring-up (its owner
   `PreRender_Crosshair` 2013 `0x79ff30` is unported). `PlayerGauges.Open` (AS2, `UI_HUD_SF.HUD.gfx`, disassembled
   with a patched copy of agent FI's as2dis) sets `_visible = true` before tweening, so the next Open shows them again.
   Also the 2013 bit write `this+508 & 0xFFFFF2FF | 0x500`: `m_bGameMessagesVisible` + `m_bSystemicTutorialsEnabled` set,
   `m_bTutorialWindowSet` cleared; our `m_bShowHighlightNextFrame = TRUE` is not retail's and is gone (nothing reads it).
2. **`UDisGFxMoviePlayerHUD::PreClose` (new; 2013 `0x7af820`, 574 B; 2012 `0x811e40`)** — found as the vtable slot after
   PostStart (2013 vtable `0x115cf64` PostStart, `0x115cf68` PreClose, `0x115cf6c` PreAdvance `0x7b2280`; 2012 `0xd53330/34/38`).
   Ported whole except `ClearMarkerInstances` (unported, marked): clip array freed and NULLed, `m_ShowFlagsOnLastTick`
   0, health/mana caches -1 with their bits cleared, equipment NULL / `DPWI_MAX` (36) / 0 / 0, `m_PlayerState` 0, the
   interaction name/groups/probe, the active popup and queue, damage info, keyhole rotation, subtitle parts,
   `m_bStealthShroudVisible`, `m_bPlayerBusted*`, `m_bCinematicMode`, `m_bChoiceSelection`. **Retail keeps
   `m_bTickedOnce` and `m_bPlayerStatusVisible` here.** Reached through the GFxUI PreClose seam
   (`DisGFxMoviePlayerPreClose`, `disgfxmovieplayermainmenu.cpp`); declared in `Inc/CppText/UDisGFxMoviePlayerHUD.h`.
3. **`Tick_PlayerStatus` (2013 `0x79f6e0`)** — the `bBecameValid && m_HUDVisibility == 1` forced open is retail's
   (`if (v70 && *(this+444) == 1) { v54 = 1; RequestPlayerInfoDisplay(1) }`; 2012 differs: any change forces it). Fixed:
   retail **assigns** the flag in each of the three blocks (the last changed block decides); ours OR-ed them.
   **No unconditional Close on the first tick**: the first tick invokes `Close` only when the flags want it open and
   `m_bTickedOnce` is 0 (the menu: Close then Open at 4.18/4.20 s, as before). Not ported (HF's area): the 2013 head's
   low-health tutorial block (`m_bPlayerLowOnHealth` → this+508 bit 0x8, calls `0xbe4280` / `0xbd4fc0`).
4. **`UDisSeqAct_ToggleHUDElement::Activated` (2013 `0x791940`, 2012 `0x7f75b0`)** — off the exclude list through the
   generator. Input 0 enables, input 1 disables, level 4 only, through retail's table crosshair 0x1 / health 0x2 / mana
   0x4 / equipment 0x8 / stealth gem 0x100; the shroud (element 5) is `m_bDisableStealthShroud` (PC+1552 bit 0x8000).
   Measured at load (`CC_HE1.log` 2838–2842, world 50.62): `_5` health **disable**, `_2` mana disable, `_3` equipment
   disable, `_4` stealth gem disable, `_6` shroud disable → level 4 `0x7EE1`. **Bits 0x2/0x4/0x8 hide the vials**
   (`Tick_PlayerStatus` shows on `flags & 0xE`). `_0` and `_1` never fire on the route (they are the later enables).
5. **`Tick_PlayerState` (2013 `0x795ed0`)** — retail's gates: `!IsPossessing` (2013 `0x74a550` = vtable +1188 =
   `ADishonoredPlayerPawn::DisIsPossessing`, 2012 `0x6fb7d0`: Possess or PrePossess state) **and `IsSneaking`** (vtable
   +1660) and `flags & 0xE00` and the stance setting; slide (`IsSliding`, 2012 `0x6fec70`) over crouch (`APawn+640 & 0xC`
   = `bWantsToCrouch | bIsCrouched`, offsets from `retail_sdk_layout.json`) over sneak. The two pawn functions are new
   members (`Inc/CppText/ADishonoredPlayerPawn.h`, bodies in `dishonoredplayerpawn_inventory.cpp` beside `IsSneaking`).
   **The icon was the missing `IsSneaking` gate, not the 0xE00 bits**: no ToggleHUDElement touches 0xE00 (level 4 reads
   0x7EE1, which keeps it). 0 `SetIcon` from the soirée exit to the end of the run.
6. **`SetCinematicMode_Native`** — body kept; `DisPushDisableSave(DDST_CinematicMode)` on entry and
   `DisPopDisableSave(DDST_CinematicMode, 1.f)` on exit wired (already ported by FZ). Still out, marked: the projectile
   manager's destroy-all (2013 `0x8410b0`) and `ADishonoredPlayerPawn::CancelAllActivePowers` (2013 `0x6aa540`, 2012 `0x7048c0`).
7. **`ADishonoredHUD::UpdatePlayerInfoDisplay` (2013 `0x5ea1b0`)** — was inverted: retail's loop walks the six levels
   while bit 0x10 is clear and counts down only when all six are clear (i.e. outside a letterboxed cinematic).
8. **Debug text** — `UMaterialInstance::CheckSafeToModifyInGame` body under `DISHONORED_RETAIL_SHIPPING_UI` (default 1,
   `Engine/Src/MaterialInstance.cpp`): 2012 has it at `0x118b00`, called first thing from
   `UMaterialInstanceConstant::SetScalarParameterValue` `0x125a30`; **the 2013 executable has 0 strings
   `MIC::SetScalarParameterValue`, `MITV::Set`, `during gameplay`** (2012: 1 each), so every call is compiled out there.
   `GAreScreenMessagesEnabled` is now off unless `-screenmessages` (`FEngineLoop::PreInit`, `LaunchEngineLoop.cpp`).
   Grep of `resources/tools/` and the harness: nothing reads screen messages.

`-phuddiag`: every change of the six levels with the ANDed flags and `m_bPlayerStatusVisible`, each
`masterHUD_mc` Open/Close (the existing bring-up line), each ToggleHUDElement (path, element, enable/disable, level-4
result), each `SetIcon` state, and each PreClose.

## Where the plan was wrong (measured)

- **2013 PostStart sends no Close at all** — 2012 did; 2013 hides masterHUD_mc by DisplayInfo. The fault was the
  clip index of that hide, not a missing cache reset in PostStart (the caches are PreClose's).
- **`DisPushDisableSave` / `DisPopDisableSave` are 2013 `0x7ec820` / `0x7ec850`**, not `0x7ec770` / `0x7ec7a0`: those land
  inside `DisSavePhysicsAssetInstanceBodies` (`0x7ec6f0`) and `DisLoadPhysicsAssetInstanceBodies` (`0x7ec790`); the calls
  at `0xaaf183` / `0xaaf1d8` resolve them. And both were already ported (FZ).
- "After 1–2 the 0xE00 bits decide the icon": no — ToggleHUDElement never maps to 0xE00; the gate is `IsSneaking`.
- The 2013 db labels the ported functions `sub_*` (0x795650 PostStart, 0x79f6e0 Tick_PlayerStatus, 0x7af820 PreClose);
  the given 2013 addresses are rvas and the 2013 db holds VAs (rva + 0x400000).
- The user's settings run `m_HUDVisibility = 2` (log `HUD census ... visibility 2`), so the contextual forced-open never
  applies on their machine.

## Open items

- Showing the vials coming back needs the route to reach `DisSeqAct_ToggleHUDElement_0/_1`; GB's walk (to world ~204)
  does not, which matches retail showing no vials through video t 110. The Open path itself is exercised on the menu
  (Close/Open at 4.18/4.20) and not by the gate (see Gate).
- `ClearMarkerInstances`, the projectile destroy-all, `CancelAllActivePowers`, `PreRender_Crosshair`, the low-health
  tutorial head of `Tick_PlayerStatus`, `IsSneaking`'s choke half.

## Files / functions changed (HF shares `disgfxmovieplayerhud.cpp`)

`disgfxmovieplayerhud.cpp`: `Tick_PlayerState`, `Tick_PlayerStatus`, `DisGFxMoviePlayerHUDPostStart`, new
`UDisGFxMoviePlayerHUD::PreClose`, new `UDisGFxMoviePlayerHUD::IsDiagEnabled` — nothing else (not PreAdvance).
Others: see `build/agentHE_sync.py`. Generated (regenerated, byte-identical into a scratch `--out-dir`: 0 of 79
files differ): `dishonoredgamekismetclasses.h`, `Sources.cmake`, `DishonoredGameRegistrants.cpp`. `bitfield_audit`
mismatches 0.

## Gate

`python resources/tools/run_regression.py --build-dir build/agentHE --no-build --exe-name DishonoredGame_HE.exe
--log-prefix HEg_ --only newgame,options`: 10 ok, 0 failed; `options_as2_errors` 39; criticals 0.
Full gate, same exe (`run_regression.py --build-dir build/agentHE --no-build --exe-name DishonoredGame_HE.exe --log-prefix HEg_`):
**41 ok, 0 failed, 0 skipped** (1132 s; `build/agentHE/regression/summary.txt`), every `*_criticals` 0, `options_as2_errors` 39.
The `-startmap` stages run without the HUD movie (0 masterHUD_mc calls in `HEg__d3d9/inputtest/nullrhi.log`), so the gate
does not exercise a later Open; the menu's first-tick Close/Open (`HEg__newgame.log`, `HEg__options.log`: 1 each) is the
only Open in it. The options stage's HUD setting path (`ApplyGameSettings`) is untouched.
