# Agent FE — the menu's camera, and the handover to the world

Wave 17 (`PHASE13.md`), package FE. Worktree `build/agentFE_wt`, base `e02f5f8`. Ten files: seven in
`DishonoredGame`, three in `Engine`. Nothing in `External/GFx3` or `GFxUI` — FC and FD own those and this
package did not need to open either (§9).

## 1. The short version

| the brief said | what is true |
|---|---|
| "the FOV on the main menu is still too high" | **true, and it is the world camera's, not the movie's.** Ours is **90.0°** horizontal, retail's is **61 ± 2°**, measured against the user's own retail screenshot (§2). Not fixed: the number our code can produce is 90 (the camera actor's `FOVAngle`) or 75 (the camera's `DefaultFOV`), and retail's is neither. `-fefov=<deg>` is shipped as the instrument that settles it in one run. |
| "there is also a brightness & maybe bloom effect that should also show" | **true and separable.** Once the framing is matched at 60° the remaining difference is visible on its own: retail's menu is brighter, hazier and lower in contrast (`fe_menu_retail_vs_fov60.png`). It is **not** a Kismet post-process — there is no `DisSeqAct_UberPostProcess` anywhere in the four levels the menu loads (§4). |
| "doesn't have the top and bottom black bars applied to the level" | **true**, and the chain is now named end to end (§5). Not ported: it is three functions, one of which needs the in-game HUD movie open, which this tree only opens behind `-dishud`. |
| "the menu movie of the surrounding vignette effect is still showing" | **true, and fixed.** `fe_vignette_before_after.png`: same driver, same world time 80 s, HEAD above with the torn vignette over the cutscene and this package below without it (§6). |

Two premises of the brief were wrong and are corrected in §3 and §4.

## 2. The FOV: which one is wrong, and by how much

### 2a. It is the world camera's, not the movie's

Agent FA's `_perspfov` is the Scaleform stage's own 3D projection, defaults to 55° and **is not used by any
cooked movie in this install** (FA measured 0 occurrences across all 22 `.gfx` payloads, `agentFA.md` §9).
It cannot be the fault. The number that frames the menu's 3D background is the player camera's POV FOV, and
`-fediag` reads it out of the running game:

```
fediag camera 10: t 18.02 map Dishonored_MainMenu, camera DishonoredPlayerCamera,
  viewTarget CameraActor_11 (CameraActor), POV.FOV 90.000, camDefaultFOV 75.000,
  GetFOVAngle 90.000, constrain 0 ratio 1.7778, style FirstPerson
fediag cameraactor 2: Dishonored_MainMenu.TheWorld:PersistentLevel.CameraActor_11
  FOVAngle 90.000 aspect 1.7778 constrain 1 camOverridePP 0 loc 53.6/-4835.0/3036.1
```

**Our menu's horizontal field of view is 90.0°.** It comes from `ACameraActor::FOVAngle`, copied verbatim by
`ACamera::UpdateViewTarget`. Eleven `CameraActor`s exist in `Dishonored_MainMenu`; six of them share the one
point `53.6/-4835.0/3036.1` and differ only in rotation — those are the menu screens. Each screen's camera is
made the view target by a matinee director group (`SeqAct_Interp_1` group `Main_View` for the main menu,
`SeqAct_Interp_83` group `StartCam` for the start screen, `SeqAct_Interp_2` group `New_Game`), and the POV
location never moves from the actor's own, so a single frame is a valid comparison.

### 2b. Retail's is 61 ± 2 degrees, measured

The user's `resources/reference/menu/3.png` is retail and our build side by side in two windows of the same
client size — the 2D overlay of the movie (the gargoyle, the torn edges) matches between the two halves at
scale 1.000 and offset (0, 12), which is what proves the two windows are the same size and fixes the crop.

With the crop aligned, the retail half is a **pure zoom about the screen centre** of our half. That is the
signature of a field-of-view difference and of nothing else: a dolly would show parallax, and a different
camera would not fit a one-parameter zoom at all. The fit, over a mask of exactly the pixels that move when
only the FOV changes (`|frame@90 - frame@60| > 12`), against our frame forced to 60°:

| scale | NCC | implied retail FOV |
|---|---|---|
| 0.976 | 0.786 | 61.2° |
| **0.980** | **0.804** | **61.0°** |
| 0.984 | 0.796 | 60.8° |
| 0.988 | 0.778 | 60.6° |

The same search excludes the other two numbers our code can produce: against our frame at **90°** it finds
no match at any scale in 0.40–1.60, and against our frame at **75°** the best NCC anywhere in 0.60–1.80 is
**0.070**, against 0.804 for 60°. `-fefov=60` and the
retail reference are `fe_menu_retail_vs_fov60.png`: the billboard, the four lamps under it, the lamp post on
the left and the pipes at bottom right all land in the same places at the same size.

**So: the movie's FOV is right and the world camera's is wrong, by 90 against about 61.**

### 2c. Why it is not fixed, and what the next package needs

Three numbers our code can produce, and none of them is 61:

- `ACameraActor::FOVAngle` = **90.000** on all eleven actors, and the `CameraActor` **class default object is
  also 90.000** (`fediag defaults`), so no level instance overrides it. The reference `CameraActor.uc` default
  is 95, so the 90 we read is the cooked package's own value, not a C++ fallback — the CDO is loading.
- `ADishonoredPlayerCamera::DefaultFOV` = **75.000**, from `m_fDefaultFOV=75.0` in
  `DishonoredCamera.ini`. The user's own retail install has the same line with the same value, so the FOV
  setting is not the explanation either.
- `m_fCurFOV` = **0.000** in our build, because `ADishonoredPlayerCamera::TickFOV` (2013 0x6c9a80 region,
  2012 0x709a80) is unported.

What retail does, read out of the binary rather than argued about: **`ADishonoredPlayerCamera::UpdateViewTarget`
(2013 0x6d80a0, 5,883 bytes) overrides `ACamera::UpdateViewTarget` and is what actually runs.** Its only write
of `OutVT.POV.FOV` in the whole body is `fstp dword ptr [edi+20h]` at 0x6d8126, from `[esi+53Ch]` =
`m_fCurFOV` @1340, and that write is inside the arm taken only when the view target **is** the player pawn and
`CameraStyle` is `FirstPerson`. Every other arm — including the one the menu takes, a `CameraActor` target —
leaves POV.FOV alone, and the chain ends at 0x6d9773 with a call to `ACamera::UpdateViewTarget` (verified:
the one `call ?UpdateViewTarget@ACamera@@` in the function). That base copies `CamActor->FOVAngle`.

So on the reading of the binary alone retail's menu ought to be 90 too, and the picture says it is 61.
Something between those two writes it, and this package could not find it without the instrument the wave
was supposed to provide. **Agent FB's dismod capture of the menu world camera's FOV never landed** — there is
no `build/agentFB/retail/` and no `resources/docs/dismod_harness.md` as of this report — so the number above
is photogrammetry, not a read of the running retail game. One `ProcessEvent`/`Camera` hook that prints
`PlayerCamera.CameraCache.POV.FOV` on the retail menu closes this in one run, and `-fefov` then confirms it.

Porting `ADishonoredPlayerCamera::UpdateViewTarget` whole is the retail-faithful fix and is a package of its
own: 5,883 bytes, eight camera-style arms, collision, lean, bob and the influence group, and it is the
in-game camera as well as the menu's.

### 2d. The 2.3 points agent FA left, and this

They are not the same thing. FA's residue is the **logo**, a 2D element of the movie, measured at 0.977 of
retail's; the fault here is the **3D background**, off by a factor of 1.75 in the tangent of the half angle.
The 2D overlay of our build and retail's match at scale 1.000 in the same screenshot (§2b), so whatever the
remaining 2.3 % of the logo is, the world camera is not it.

### 2e. One porting defect found on the way, not the fault

`ACamera::UpdateViewTarget` copies the camera actor's `AspectRatio` into `OutVT` but never raises
`ACamera::bConstrainAspectRatio`, so `ULocalPlayer::CalcSceneView` takes its unconstrained branch. **Retail
does the same** (0x5e6350 writes `OutVT.AspectRatio` from `CamActor+588` and touches no bit), and at 16:9 the
two branches produce the identical projection matrix, so this is neither a defect nor the fault. Recorded so
the next package does not spend an hour on it as this one did.

## 3. The brightness and bloom, with the framing taken out of it

`fe_menu_retail_vs_fov60.png` is retail on the left and this build forced to 60° on the right, at the same
window size and the same screen. With the framing matched the tonal difference stands on its own: retail's
scene is brighter, hazier and flatter, ours is darker and more contrasty. That is a real second fault and it
is now separable from the first, which it was not while the framing was wrong.

What is already running on our menu, from the same run's census: `DisFog 2 layers in scene, 2 drawn in 1
passes`, `bloom parts 24 relevant, 24 prims, 27 draws`, `FArkPp 1 nodes rendered, 5 draws (dof 2, aa 3)`,
`0 passes not ported`. So fog and bloom both draw; the difference is in their parameters or in a term that
never reaches them, not in a missing pass.

## 4. The premise this package corrected: the menu has no Kismet post-process

The brief and `PHASE13.md` put the menu's brightness on "the active post-process chain on the menu", and the
obvious candidate in this tree was the Kismet uber channel, which was unported end to end. It is now ported
(§7) — and it is **not** the menu's brightness:

```
fediag kismet 2: 4 levels, 1858 sequence objects, 118 of interest
```

and **no object of any class whose name contains `PostProcess` exists in any of the four levels**
(`DishonoredGameFull_P`, `Dishonored_MainMenu`, `Dishonored_MainMenu_Env`, `Dishonored_MainMenu_FX`). The
census prints every one it finds and printed none. `DisSeqAct_UberPostProcess::Activated` therefore never
runs on the menu, and the "Kismet uber post-process ON" line the port emits never appears in a menu run.

The port stands on its own merits — it is a real retail body the campaign needs and it was never called by
anything — but the menu's brightness is somewhere else. The remaining candidates, in the order the evidence
supports: the fog layers' own parameters (`DisFog layer 0: ... opacity 0.200 colour (0.59 0.62 0.65)`), the
bloom node's thresholds, and `AWorldInfo`'s default post-process settings for the streamed level.

## 5. The letterbox bars: the chain, named end to end

Not ported. What it is, so the next package does not have to find it again:

1. `Dishonored_MainMenu.TheWorld:PersistentLevel.Main_Sequence.SeqAct_ToggleCinematicMode_1` and
   `DishonoredGameFull_P...Fade_In.SeqAct_ToggleCinematicMode_0` raise `PlayerController.SetCinematicMode`.
   Both exist and both fire: `fediag hud` reads `bCinematicMode 1` on our menu already.
2. `ADishonoredPlayerController::execSetCinematicMode_Native` (2013 0x5eea00) and
   `execPreSetCinematicMode_Native` (0x5ee960) are **`DISHONORED_NATIVE_STUB`s**, so the script's native half
   does nothing. The virtuals behind them are 2013 0x6af150 (252 bytes) and 0x6a33a0 (49 bytes).
3. `SetCinematicMode_Native` draws nothing itself. Everything it does is to `ADishonoredHUD`'s six show-flag
   masks: `DisableHUDElements(eDisHUDMaskLevel_Cinematic, 32687)` then
   `EnableHUDElements(eDisHUDMaskLevel_Cinematic, 24592)` on entry, and the reverse on exit. The one bit in
   24592 (0x6010) that is **not** in 32687 (0x7FAF) is **0x10**, and that is the bars.
   Measured now: `fediag hud 3: showFlags 7fef/7fef/7fef/7fe1/7fef/7fef` — every mask still full, because the
   native is a stub.
4. `UDisGFxMoviePlayerHUD::Tick_Cinematic` (2013 0x7960e0) is what turns that bit into
   `_root.blackStripes_mc`. It is one of eight Tick_* bodies `disgfxmovieplayerhud.cpp:1178` names as
   unported. The clip is already bound as `DHMC_Cinematic` and the layout pass already reads its `_stripeH`
   and positions `_stripeUp_mc` / `_stripeDown_mc` (`disgfxmovieplayerhud.cpp:731`, `:839`).
5. And the HUD movie has to be open in the level at all, which in this tree happens only behind `-dishud`.

Four unported bodies and a bring-up switch. `execSetCinematicMode_Native` parses **nine** booleans and an
optional tenth off the script stack (read at 0x5eea00), not the six of stock UE3's `SetCinematicMode`, and
the parameter names are not in any truth source this tree holds; a wrong count corrupts the script stack, so
this package did not guess at it under time pressure. That is the one thing to get right first.

## 6. The vignette: fixed

`_root.vignette_mc` is one of the menu movie's ten root children (agent EY). It survives into the level
because **nothing in this tree ever closed the menu movie.** The mechanism to close it was already there and
had no caller:

- `FGFxEngine::NotifyGameSessionEnded()` (`GFxUI/Src/gfxuiengine.cpp:1190`) calls
  `CloseAllMovies(TRUE)`, which closes every movie whose `bCloseOnLevelChange` is set;
- `GFxUI/Src/gfxuiengine.cpp:2979` already exports `DishonoredGFxNotifyGameSessionEnded()` for the
  Engine → GFxUI edge, in the same family as the five calls `UnPlayer.cpp:79` declares;
- **`grep` finds no caller of either.** Measured: `fediag movie 2: DisGFxMoviePlayerMainMenu open 1
  bCloseOnLevelChange 1` — the flag is already set on both open movies, so the only missing piece was the
  call.

This package calls it from `UGameEngine::CommitMapChange`, immediately after the
`eventPreCommitMapChange` block that is already there under the comment "tell the game we are about to switch
levels", guarded by the same `#if DISHONORED_WITH_GFX3 && DISHONORED_WITH_GFXUI_SHADERS` the other five use.

**Evidence**: `fe_vignette_before_after.png`, the opening cutscene at world time 80.0 s, both runs driven by
the identical `drive_input.py` schedule at 1600x900. Above, HEAD: the torn dark vignette across the top and
into all four corners. Below, this package: clean. Nothing else in the frame moves.

Deviation, stated: the exact retail call site of `UEngine::PreCommitMapChange` (2013 0x1e3330, vtable slot
100) was not pinned. It is called through a vtable and the six register forms of `call [reg+190h]` find no
match, so the caller loads the slot into a register first. `UDishonoredEngine::PreCommitMapChange`
(2013 0x616150) is pinned a second way — its first instruction calls 0x5e3330, which is
`UEngine::PreCommitMapChange`'s own body, the nine `TArray::Shrink` calls our port already has — but its body
is `Client`-side teardown (`GEngine+1148` is `UEngine::Client`) and has nothing to do with the movies, so the
close does not belong in it.

## 7. What landed

| | |
|---|---|
| `UDisPostProcessManager::SetKismetPPParams` | ported, 2013 rva 0x7e7e90 (2012 0x8496b0) |
| `UDisPostProcessManager::ApplyKismetPostProcessSettings` | ported, 2013 rva 0x7ef9b0 (2012 0x850ef0), 746 bytes |
| its call site in `ADishonoredPlayerController::ModifyPostProcessSettings` | ported, before the UI channel as retail has it |
| `UDisSeqAct_UberPostProcess::Activated` | ported, 2013 rva 0x791570 (2012 0x7d4140), 263 bytes |
| `Src/disseqact_bendtime.cpp` off the `Sources.cmake` exclude list | one unit; §9 |
| the menu movie's teardown on a map change | `UnGame.cpp`, §6 |
| `-fediag` | the camera, the camera actors, the matinees and their tracks, the post-process effect states, the open movies and the HUD masks, twice a second |
| `-fefov=<degrees>` | forces a `CameraActor` view target's FOV; the bisect of §2 |
| `-apshottime=a,b,c` and `-apshotname=<tag>` | several marks in one run, and a screenshot file a run can claim by name |

The four post-process bodies are the twin of the UI channel agent FA ported, on `Epp_UberKismet` (18) instead
of `Epp_UberUI` (19), pinned by their own offsets rather than by the match table: the state byte is
`this+462` = `m_EffectStates[18]` (the array starts at 444), the request is `this+432` =
`m_RequiredEffects[18]` (starts at 360), and 532/536/540/544/548 are `m_KismetStateDuration`,
`m_KismetPPFadeOutTime`, `m_KismetPPFadeInTime`, `m_KismetPPWeight`, `m_KismetPPParams` — every one of them
already asserted at that offset in `DishonoredGameLayouts.h`. **One difference from the UI channel that is
easy to lose**: `ArkUberPpApplyTo`'s last argument is `FALSE` here and `TRUE` there.

## 8. Addresses, resolved by hand

`rva_sweep.py` passes the tree. Every address this report cites was also resolved by name or by body in
`retail2013_named.i64` (copied to `build/agentFE/idb/FE_retail2013.i64`; image base 0x400000, so the VAs below
are the RVA plus that).

| RVA (2013) | what, and how it was pinned a second time |
|---|---|
| 0x791570 | `UDisSeqAct_UberPostProcess::Activated` — named in `functions_2013.csv`; body reads `this+248` (`m_Parameters`), `this[86..88]` (weight, fade in, fade out), which is this class's own 356-byte layout |
| 0x7e7e90 | `SetKismetPPParams` — unnamed in 2013; the body `qmemcpy(this+548, src, 0x60); this[136]=..; this[135]=..; this[134]=..` is `m_KismetPPParams/Weight/FadeIn/FadeOut` at 548/544/540/536 exactly |
| 0x7e7dc0 / 0x7e7df0 | `StartEffect` / `StopEffect` — `this[a2+90]` is `m_RequiredEffects[a2]` at 360+4a2 |
| 0x7ef9b0 | `ApplyKismetPostProcessSettings` — the same four-state fade as 0x7efca0 on the effect one index below |
| 0x7efca0 | `ApplyUIPostProcessSettings` — agent FA's, used only to compare the last argument of `ArkUberPpApplyTo` |
| 0x7e7e30 | `UDisPostProcessManager::ApplyGameSettings` — already ported by agent EQ; used to pin the 0x7e7dc0..0x7e7ee0 run against the 2012 order 0x8495f0/0x849620/0x849660/0x8496b0/0x849700 |
| 0x6d80a0 | `ADishonoredPlayerCamera::UpdateViewTarget` — `match_2012_2013.csv` matched it by string at 0.981; confirmed by body (its tail calls `ACamera::UpdateViewTarget` at 0x6d9773 and it is the only function that writes `m_fCurFOV` into a POV) |
| 0x6af150 | `ADishonoredPlayerController::SetCinematicMode_Native` — the match table's 0.630 "neighbours" match, **which is exactly the kind this project has been burned by**; confirmed by body instead: it calls `ADishonoredHUD::DisableHUDElements` / `EnableHUDElements`, `DisPushDisableSave`, `DisPopDisableSave` and `DisGetGlobalProjectileManager`, which no other function does |
| 0x6a33a0 | `PreSetCinematicMode_Native` — named in `functions_2013.csv`, 49 bytes, body is two `AActor::SetHidden` calls on `ADishonoredPlayerPawn::s_pInstance` |
| 0x5eea00 / 0x5ee960 | the two exec wrappers — named; 0x5eea00 parses nine booleans and dispatches through vtable +1412 |
| 0x5e6350 | `ACamera::UpdateViewTarget` — named; body reads `CamActor+592` into POV.FOV and `CamActor+588` into `OutVT.AspectRatio`, which `retail_sdk_layout.json` gives as `FOVAngle` and `AspectRatio` |
| 0x1e3330 | `UEngine::PreCommitMapChange` — named in our own port comment; confirmed as the first call of 0x616150 |
| 0x616150 | `UDishonoredEngine::PreCommitMapChange` — the match table's 0.536 vtable match, confirmed by that first call |
| 0x7960e0 | `UDisGFxMoviePlayerHUD::Tick_Cinematic` — named at `disgfxmovieplayerhud.cpp:1179`; not opened by this package |

## 9. Merging

- **`gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is not required and was not
  run.** No reflected member was added or moved and no class was added; the one new declaration is a
  `virtual void Activated();` inside a `#include "CppText/UDisSeqAct_UberPostProcess.h"` in the generated
  class body, which is the mechanism the generator preserves (agent EL used it for
  `UDisSeqAct_SetStoryFlag`).
- **`Sources.cmake` changed by one unit**: `Src/disseqact_bendtime.cpp` comes off the exclude list. No
  separate registrant is needed and none was added: `IMPLEMENT_CLASS(UDisSeqAct_UberPostProcess)` in
  `DishonoredGameRegistrants.cpp` emits the class's vtable, the vtable names `Activated`, and that undefined
  symbol is what pulls the object in — the same reason agent EL's `disseqact_setstoryflag.cpp` needs none.
  Verified: `disseqact_bendtime.cpp.obj` is in the build and the exe links.
- **Files outside `DishonoredGame`: three, all in `Engine`, none in `External/GFx3` or `GFxUI`.**
  `Engine/Src/UnGame.cpp` (the movie teardown call and its `extern`), `Engine/Src/UnCamera.cpp` (the
  `-fefov` override inside the `CameraActor` arm, inert without the switch), `Engine/Src/SceneRendering.cpp`
  (`-apshottime` list form and `-apshotname`, inside the one existing static function). FC and FD are in
  `External/GFx3`; this package opened neither that directory nor `GFxUI`.
- **Ten files in all**, plus one new header.
- `build/agentFE_sync.py` is the authoritative copy list, worktree → main, and flags rather than copies
  anything changed in main since `e02f5f8`.

## 10. Hand-over

1. **The FOV number is 61 ± 2 and our code cannot produce it.** §2c. The first thing to do is read
   `PlayerCamera.CameraCache.POV.FOV` out of the real game on the menu; `-fefov` then confirms the answer in
   one run against `resources/reference/menu/3.png`.
2. **`ADishonoredPlayerCamera::UpdateViewTarget` is unported and it is the camera.** 5,883 bytes, and the
   only function in retail that writes `m_fCurFOV` into a POV. It is a package.
3. **The menu has no Kismet post-process.** §4. Do not spend a wave on it; the fog layers and the bloom node
   are where the brightness is.
4. **The bars are four bodies and a switch**, §5, and the one to be careful with is
   `execSetCinematicMode_Native`'s nine-boolean stack read.
5. **`-apshotname=<tag>` makes a screenshot claimable.** The shared directory ate two of this package's
   captures before it existed and another package's frame was read as ours once. Use it.
6. **`-unattended` implies `-noinputgrab`**, so a driven run needs `-inputgrab` as well or every key press is
   dropped silently (`WinViewport.cpp:761`). Two runs of this package were lost to that, and a third to a
   previous run of the same exe still holding the window the driver picked — kill the old process first, and
   read the driver's "candidate" lines: two candidates means two games are up.

## 11. Acceptance, and where the evidence is

| | |
|---|---|
| `run_regression.py` | **37 ok, 0 failed, 0 skipped**, 987 s. The worktree's own copy, absolute build dir `build/agentFE_reg`, built inside the harness (no `--no-build`); the five gitignored `resources/docs/types/` inputs copied in first, so every layout metric measured rather than recording -1. `build/agentFE/fe_regression.log` |
| clean full release build | directory deleted first, `-DDISHONORED_LAYOUT_CHECKS=ON`: **0 errors, 0 C4263, 0 C4264**, `DishonoredGame.exe` linked at 23,974,400 bytes. The only `C42xx` warnings in the whole log are eleven pre-existing `C4244`s inside `external/zlib-src`. `build/agentFE/fe_clean_build.log` |
| `rva_sweep.py` | 7,360 citations, 3 suspects, **none of them in a file this package touched** (`disbehaviorpatrol.cpp:319`, `gfxuirenderer.cpp:1679` twice — all three pre-existing). Every address this package cites is also resolved by hand in §8 |
| `build/agentFE_sync.py` | 9 files to copy, 3 new, **0 flagged** |

`build/agentFE/`:

| file | what it is |
|---|---|
| `fe_menu_retail_vs_fov60.png` | **the headline.** Retail left, this build with `-fefov=60` right, main menu, 1600x900 each. The framing matches; the tonal difference of §3 is what is left |
| `fe_menu_retail_vs_head_fov90.png` | the same pair with HEAD's own 90°, which is the fault |
| `fe_vignette_before_after.png` | **the fix.** The opening cutscene at world time 80.0 s, HEAD above, this package below, same driver schedule |
| `fe_cutscene_retail_vs_fe.png` | retail's cutscene beside this package's: no vignette in either, and retail's letterbox bars which we still do not draw (§5) |
| `fe_head_mainmenu_t20.png`, `fe_fov60_menu.png`, `fe_fov75_menu.png` | our menu at 90°, 60° and 75°, the three frames the fit of §2b is measured on |
| `fe_head_level_t80.png`, `fe_fix_level_t80.png` | the two halves of the before/after |
| `ref{1..5}_retail.png`, `ref{1..5}_ours.png` | the user's five reference screenshots split into their two windows, 1600x900 each |
| `ref3_retail_aligned.png` | the retail half re-cropped 12 px down so the movie's 2D overlay aligns; this is the one the fit uses |
| `fe_regression.log`, `fe_clean_build.log` | the two acceptance runs |
| `idb/FE_retail2013.i64` | this package's copy of the retail database (never open one from two processes) |
