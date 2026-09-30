# Agent FA — the menu in three dimensions, and the blur behind a modal

Wave 16 (`PHASE12.md`), package FA. Worktree `build/agentFA_wt`, base `e2d7e5d`.

## 1. What landed

**The menu moves.** GFx 3.3's six 3D display properties — `_z`, `_zscale`, `_xrotation`, `_yrotation`,
`_matrix3d`, `_perspfov` — the geometry record behind them, the per-character world matrix, the stage's
view and perspective, and the renderer branch that projects through them. The main menu's own
ActionScript drives all of it; nothing in the game code had to change.

**The blur chain behind a modal** is ported from the movie's own bit to the post-process manager's
uber-parameter channel. See §6 for what it does and does not yet produce.

Twenty files: eleven in `External/GFx3`, three in `GFxUI`, six in `DishonoredGame`. `Sources.cmake` is
untouched; `gen_classes_header.py` is not needed (§8).

## 2. The premise, corrected in two places

The brief said the menu's logo, button bar and background "shift a bit in 3D as the menu sits" and that
this is GFx's `_z` / `_xrotation` / `_yrotation` and the perspective that consumes them. That part is
right and is now measured. Two things in and around it were not.

### 2a. The properties are gated on `_global.gfxExtensions`, and that gate is half the feature

`GFxASCharacter::SetMember` (2013 `0x9c9270`) tests `GASGlobalContext+684` **before every one of cases
108..113** and does nothing if it is not 1. That byte is written by `GASGlobalObject::SetMember`
(`0x9d72d0`) when ActionScript assigns `_global.gfxExtensions`, and the same body publishes
`_global.gfxVersion = "3.3.89"` — which is where the wave plan's "retail 2013's GFx is 3.3.89" can be
read out of the binary rather than taken on trust. Without the gate the six properties are inert, so a
port that implements the matrices and skips the flag produces nothing. This tree had no `GASGlobalObject`
at all; `_global` was a plain `GASObject`.

Measured: `DISHONORED(bringup): GFx _global.gfxExtensions = 1 (gfxVersion 3.3.89)` at 4.99 s of every
menu run, twice (the menu movie and the global movie each have their own context).

### 2b. `_z` is **not** in pixels, and reading it as pixels magnifies the menu 2.1x

This is the correction that cost the most and is the one worth carrying forward.

`_x` and `_y` are in the parent's pixels: `GFxASCharacter::SetStandardMember` cases 0 and 1 multiply by
20 on the way into the geometry record. `_z` is not: `GFxValue_UpdateTransform` (`0x9a5b70`) writes the
value straight into `M_[3][2]` of a matrix that is composed **before** the character's own placement
matrix — that is, in the character's own twips.

The main menu tweens `_level0.mainMenu_mc._logo_mc._logo_mc._z` to **-650** and
`._menu_mc._btnContainer_mc.btn0._z` to **-715**. Against a stage focal length of 1229 pixels:

| `_z` read as | logo magnification |
|---|---|
| pixels (-650 px) | 1229 / (1229 - 650) = **2.12x** |
| twips (-32.5 px) | 1229 / (1229 - 32.5) = **1.027x** |

The first build of this package read them as pixels and drew the logo filling the whole frame
(`build/agentFA/fa_after_first_z_as_pixels.png`: "DISHONOR" running off both edges). The second reads them as
retail writes them and the logo is where it was, 2.7 % nearer. **That 2.7 % is what "shift a bit in 3D"
means.** Anyone porting `_z` elsewhere should start from the twips reading.

### 2c. A measurement hazard, not a premise: the screenshot directory is shared

`-apshottime` always writes `Screenshots/Win32Console/apshottime<N>.bmp`, and every agent's run writes
into the same directory and deletes the same files before its own run. A 1600x900 brightness screen
appeared in one of my "before" captures whose own log had never left the main menu — another package's
frame. Every measurement here runs at **1608x904** and accepts only a file of exactly that byte size
(`4360950`), and claims it the moment it appears rather than after the run. The driver's window search
also stopped reporting two candidates once the resolution was unique.

## 3. What retail does, and what this tree does instead

Everything below is a body read out of `retail2013_named.i64`; every address is resolved by hand in §9.

| piece | retail | here |
|---|---|---|
| the property table | `GFxASCharacter::MemberTable` at data `0x13b38c8`, 115 entries of `{const char*, id, caseflag}`; ids **108 `_z`, 109 `_zscale`, 110 `_xrotation`, 111 `_yrotation`, 112 `_matrix3d`, 113 `_perspfov`** | the names are matched in `GFxASCharacter::Set/GetMemberRaw`; this tree has no member-id table |
| the geometry record | `GeomDataType` 88 bytes, 3D tail at +56 / +64 / +72 / +80 | the same four doubles appended to `GeomDataType` |
| local 3D matrix | `GFxValue_UpdateTransform` `0x9a5b70`: `Scale(1,1,_zscale/100) * RotX * RotY * Translate(0,0,_z)` | `GFxASCharacter::UpdateMatrix3D`, term for term |
| the stage | `GFxMovieRoot::Setup3DDisplay` `0x9f93b0`, once per frame from `Display` `0x9fe550`; default FOV **55°** from the constructor `0xa064c0` | the same, in `GFxDisplay.cpp` |
| view + perspective | `GRenderer::MakeViewAndPersp3D` `0x9b08c0`: eye one focal length in front of the frame rect's middle, `ViewRH` + `PerspectiveFocalLengthRH` | the same, in `GFx3Support.cpp`; the generated stub in `GFx3RuntimeStubs.cpp` is removed |
| the walk | `GFxDisplayContext::PreDisplay` `0xa53400` / `PostDisplay` `0xa53630` | the same, with a `SavedTransform` value struct instead of retail's five pointers |
| the renderer | `FGFxRenderer::ApplyUITransform_RenderThread` `0x576630`: `obj2D * World * (View * Proj)`, **no viewport matrix** | the same |

### The one deviation, and why it is forced

`GMatrix3D::SetFrom2DWithDepth` scales the depth axis by the 2D matrix's own uniform scale;
retail's `GMatrix3D(const GMatrix2D&)` (`0x9ac010`) leaves `M_[2][2]` at 1.

Retail's whole display chain and its perspective are in one unit. This tree's is not: the root matrix
`GFxMovieRoot::Display` installs is the twips-to-pixels scale (`GFxDisplay.cpp`'s "Coordinate
convention" note, agent DC), and the perspective is built from the visible frame rect in **pixels**. A
depth left unscaled through that chain is twenty times too deep, and a five-degree `_yrotation` on a
clip 1494 pixels from its pivot throws it 2580 pixels into the scene instead of 129. Measured: with the
literal body the menu drew at about 2.5x and skewed
(`build/agentFA/fa_after_first_z_as_pixels.png`, which is that build.

The retail-faithful alternative is to move this tree's twips-to-pixels scale out of the character
matrices and into the renderer's viewport matrix, which is what retail does. That is a change to every
2D path in `GFxDisplay.cpp` and belongs to whoever owns the stage question (EX), not here.

## 4. Measurement 1 — the menu moves, over time

`-gfxui3ddiag=<n>` reports the first n writes of a 3D display property with the clip's target path and
the value; the per-frame census line reports how many characters the walk drew through the projected
path.

**The transform over time**, `_level0.mainMenu_mc._yrotation`, one sample per half second out of
`LaunchFA_A.log` (2,770 samples of the two rotations in the first 15 s):

```
  6.78   0.000     (menu opens)
  7.28   1.131
  7.79   2.218
  8.30   2.728
  8.80   2.927
  9.30   2.988
  9.80   2.999
 10.30   3.000     (settles)
 10.80   2.608
 11.30   2.258
 11.80   2.091
 12.30   2.025
 12.80   2.004
 13.30   2.000     (settles again)
 14.30   2.389     (moves again)
```

`_xrotation` over the same run ranges **-5.000 … +2.000**. Those are degrees on the whole
`mainMenu_mc`, so the logo, the button bar, Corvo, the compasses and the blades all move with it. On top
of that:

* `_logo_mc._logo_mc._z` tweens to **-650**, `_menu_mc._lineDown_mc._z` from **118 to 250**,
  `_menu_mc._bkgd_mc._z` to **200**, `_DLCButton_mc._z` to **-250**;
* `_blades_mc._yrotation` is **-5**, which puts `blade0_mc` **129 px** behind the stage plane and
  `blade1_mc` **105 px** in front (composed world matrices, `GFx 3D world` lines);
* the **selected button** carries its own depth: `btn0._z` runs to **-715** and back to 65, which is why
  `QUIT GAME` in `build/agentFA/fa_menu_quit_selected.png` is visibly nearer than its neighbours.

**What reaches the renderer**: `GFx 3D census: 97 characters projected` per census window, against 0 on
the HEAD build (the counter does not exist there; the property writes land in the script object and
nothing reads them).

**Property writes**: 246,850 in an 80 s run. That is the ActionScript's own traffic, identical on both
builds; what is new is that each one now rebuilds a 4x4. Cost, measured on the same schedule: 4,950
frames in 30.7 s (161 fps) on HEAD against 2,610 in 19.7 s (133 fps) here, about **17 % of the menu's
frame rate**. Retail pays the same per-write cost in `GFxValue_UpdateTransform` and the same per-character
`SetWorld3D` render command, so this is the shape of the feature rather than a defect; it is worth
saying out loud because the menu is not frame-limited here.

## 5. Measurement 2 — the projection is right, proved by bisection

One frame of the menu cannot tell a correct projection from a wrong one, because most of the menu's 3D
matrices are nearly the identity. `-gfxui3dflat` takes the projected path with **every character's own
3D matrix forced to the identity**: the picture must then be the one the 2D path drew.

`build/agentFA/fa_flat_vs_before_8a04f5f.png` — HEAD above, `-gfxui3dflat` below. The logo, the bar, the four
button labels and the DLC prompt are in the same pixels; the mean absolute channel difference over the
whole frame is **12.8/255**, and all of it is the animated 3D scene behind the movie and a sub-pixel
offset (the 2D path carries `GPixelCenterOffset`, the 3D path does not — retail's does not either).

And the numbers behind it, from `-gfxui3ddiag`'s matrix line: for `_level0.mainMenu_mc` the composed
`obj2D * World * View * Proj` gives an x coefficient of **0.000078082** per twip against the 2D path's
`ViewportMatrix.M_[0][0] * 0.05 = 0.000078125` — agreement to 0.05 %, which is float noise.

And a third form of the same proof, in pixels. The logo's own bright glyphs, measured as the horizontal
extent of pixels above 215/255 in the band y 380..600 of a 1608x904 capture at world time 25 s, same
schedule, same moment, **all three on agent EX's merge `8a04f5f`** so the frames are DPI-aware and 1:1:

| build | logo extent | ratio to HEAD |
|---|---|---|
| HEAD `8a04f5f` | x 196..1312, **1116 px** | 1.0000 |
| FA, `-gfxui3dflat` (identity local matrices) | x 197..1313, **1116 px** | **1.0000** |
| FA, the feature live | x 191..1332, **1141 px** | **1.0224** |

The flat row is the plumbing: taking the projected path changes nothing, to the pixel. The live row is
the feature: **+2.24 %**, which is the `_z = -650` twips of `_logo_mc._logo_mc` against a 1229-pixel
focal length, and is the size of the thing the fault report calls "a bit". (The same three numbers
measured 1116 / 1116 / 1143 on the pre-rebase `e2d7e5d` build, i.e. the 1.25x DWM resample EX removed
did not affect the ratio, only the sharpness.)

## 6. Measurement 3 — the blur behind a modal

The chain, read out of retail end to end:

```
UDisGFxMoviePlayerGlobal::UpdateMessageBoxAttributes  (0x7a5040, already ported)
    m_bBlurGameWhileActive = <a box is up>
  -> UDisGlobalUIManager::OnMovieAttributesChanged     (0x84cf80)   NEW
  -> UDisGlobalUIManager::RefreshGlobalUIState         (0x847070, 2012 0x8b75e0)   NEW, blur half only
       ORs m_bBlurGameWhileActive across every open movie
       -> UDisPostProcessManager::SetUIPPParams        (0x7e7ee0)   NEW
       -> UDisPostProcessManager::StartEffect(Epp_UberUI=19, TRUE)  (already present)
  -> UDisPostProcessManager::ApplyUIPostProcessSettings (0x7efca0)  NEW
       from ADishonoredPlayerController::ModifyPostProcessSettings  NEW call site
       blends m_UIPPParams into the frame's FArkPpConfig::m_UberPpParameters
```

So the blur is **not a Scaleform filter and not a separate pass**: it is the uber post-process's own
depth of field (`FArkPpDofParameters`: focus distance, in-focus radius, far blur amount) blended in at a
weight the four-state fade walks. The parameters come from `UDisGlobalUIManager::m_pBlurTweaks`, a
`UDisTweaks_PostProcess` at manager+708 whose `m_Parameters` / `m_fWeight` / `m_fFadeInTime` /
`m_fFadeOutTime` are exactly the four arguments `SetUIPPParams` takes.

`Epp_UberUI = 19` is confirmed twice: `m_RequiredEffects` is at manager+360 and retail reads
`this+436` (`360 + 4*19`), `m_EffectStates` at +444 and retail reads `this+463`.

**Measured, with a real box on screen.** `LaunchFA_am2.log`, driven to the main menu, LEFT to
`QUIT GAME`, ENTER:

```
[0058.85] DISHONORED(bringup): message box 1 up: 'Do you want to quit the game?' [YES|NO|]
[0058.85] DISHONORED(bringup): UI blur census: started, weight 1.000 fade 0.200/0.200,
                               DOF focus 0.0 radius 0.0 far 0.900
```

So `m_pBlurTweaks` **is** resolved (it comes from the script package's class defaults, not from any
`.ini` — none in the retail install names it), the chain fires on the same frame the box comes up, and
the parameters are what the fault report describes: focus distance 0, in-focus radius 0, far blur 0.9,
at weight 1 over a 0.2 s fade. That is "everything past the focus point is out of focus, and the focus
point is the camera" — the whole scene.

**And on screen.** `build/agentFA/fa_modal_before_after_8a04f5f.png` — HEAD above, this package below,
same schedule, same world time (60 s), both on `8a04f5f`, both 1608x904. Above, the Dunwall skyline
behind the box is sharp. Below, it is heavily out of focus, while the torn bar, the question and the
YES/NO row with YES lit stay sharp. That is the same difference `sixth.png` shows between our half and
retail's, in the same direction.

Three earlier capture attempts missed because `-apshottime` is keyed to world time and the box lands
anywhere between 19 s and 87 s of it depending on how loaded the machine is. The one that worked waited
for the file on the host itself rather than in my own session — see the hand-over.

## 7. What is not ported

* `_matrix3d` (member id 112), the ActionScript-array-to-`GMatrix3D` arm of `0x9c9270` case 112.
  **Zero occurrences across all 22 cooked `.gfx` payloads** in this install, so nothing reaches it.
  `_perspfov` (113) and `_zscale` (109) are ported and are also unused by the cook — the same scan.
* The four non-blur terms of `RefreshGlobalUIState` (black stripes, HUD visibility, HUD pause,
  controller input mask / mouse cursor) and the manager's own movie-set seed term. Named at the site.
* `UDisPostProcessManager::ApplyKismetPostProcessSettings` (`0x7ef9b0`), which retail calls immediately
  before the UI one. Named at the site.
* `GFxMovieRoot::HitTest3D` and `GFxCharacter::GetProjectedBounds` — the mouse's 3D arm. The menu's
  clips are within a few per cent of the stage plane, so the 2D hit test still answers; a screen with a
  large `_z` would need it.

## 8. Merging

* `gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake` is **not required** and
  was **not run**. Nothing was added to a `//## BEGIN PROPS` block; the two new declarations are in
  `Inc/CppText/UDisPostProcessManager.h` and `Inc/CppText/UDisGlobalUIManager.h`, which are included
  into the generated class bodies by hand-written text that the generator does not rewrite.
* `Sources.cmake` **did not change**, in any module. No file was added or removed.
* **Files in `External/GFx3` (11)** — agent EX owns this directory this wave, so each is named with what
  it carries:
  * `GTypes.h` — the whole `GMatrix3D` class body (was a 12-line stub)
  * `GFxPlayer.h` — `GFxCharacter`'s four 3D members and eleven accessors, `GeomDataType`'s four-double
    tail, `GFxMovieRoot`'s `pPerspective3D` / `pView3D` / `PerspectiveFOV` and five methods
  * `GFxPlayerSprite.cpp` — the `GFxCharacter` 3D bodies, `UpdateMatrix3D`, and the six names in
    `SetMemberRaw` / `GetMemberRaw`
  * `GFxPlayerRoot.cpp` — `SetPerspective3D` / `SetView3D` / `SetPerspectiveFOV` / `GetRenderer` (three
    were empty stubs), and two lines in the constructor and destructor
  * `GFxDisplay.h` — four fields and `SavedTransform` on `GFxDisplayContext`, four diagnostic externs
  * `GFxDisplay.cpp` — `PreDisplay` / `PostDisplay` rewritten, `Setup3DDisplay`, one call in
    `GFxMovieRoot::Display`, the three call sites of Pre/PostDisplay, the diagnostics
  * `GFxAS2Runtime.h` / `GFxAS2Runtime.cpp` — `GASGlobalContext`'s `bGFxExtensions`
  * `GFxAS2Lib.cpp` — `GASGlobalObject` and the one line that makes `_global` one
  * `GFx3Support.cpp` — `GRenderer::MakeViewAndPersp3D` appended
  * `GFx3RuntimeStubs.cpp` — one generated stub line replaced by a comment (the generator's own
    documented workflow)
* **Files in `GFxUI` (3)**, also EX's:
  * `Inc/gfxuirenderer.h` — one declaration, `Adjust3DMatrixForRT`
  * `Src/gfxuirenderer.cpp` — `ApplyUITransform_RenderThread`'s 3D branch, `MakeViewAndPersp3D` now
    forwarding, `Adjust3DMatrixForRT`'s body
  * `Src/gfxuiengine.cpp` — two switch parses and one census line
* **Files outside my own modules**: all of the above are outside `DishonoredGame`; within
  `DishonoredGame` the six are `Inc/CppText/UDisPostProcessManager.h`,
  `Inc/CppText/UDisGlobalUIManager.h`, `Src/dispostprocessmanager.cpp`, `Src/disglobaluimanager.cpp`,
  `Src/disgfxmovieplayerglobal.cpp`, `Src/dishonoredplayercontroller.cpp`.
* **Total: 20 files.**
* **Overlap with agent EX, named**: EX's six files include `External/GFx3/GFxDisplay.cpp` and
  `GFxUI/Src/gfxuirenderer.cpp`. **Both are in my list and both need a three-way merge, not an
  overwrite.** My changes in them are:
  * `GFxDisplay.cpp` — `GFxDisplayContext::PreDisplay` / `PostDisplay` (rewritten signature and body),
    `GFxMovieRoot::Setup3DDisplay` (new function), one added line in `GFxMovieRoot::Display`, the three
    `PreDisplay` / `PostDisplay` call sites in `GFxSprite::Display`, `GFxGenericCharacter::Display` and
    `GFxEditTextCharacter::Display` (the argument list changed), and four file-scope diagnostic globals
    at the top.
  * `gfxuirenderer.cpp` — the 3D branch of `ApplyUITransform_RenderThread`, the body of
    `MakeViewAndPersp3D` (now a forward), and `Adjust3DMatrixForRT` (new function appended after it).
  EX's third shared file, `External/GFx3/GFxTextDocView.cpp`, I do not touch.
  **Both shared files in this worktree already contain EX's changes**, merged by hand: in
  `GFxDisplay.cpp` its `<stdio.h>` include and its `GFx stage map` logging block, which sits
  immediately above the `BeginDisplay` call my `Setup3DDisplay(ctx)` sits immediately below; in
  `gfxuirenderer.cpp` its rewritten comment on `GetUIVertexDecl_RenderThread` and its four
  `VET_Color` to `VET_UByte4N` element changes, none of which is in a function this package touches.
  So the two files can be copied whole from this worktree, and `build/agentFA_sync.py` will still
  refuse to if main has moved again since `8a04f5f`.
* **Rebased onto `8a04f5f`.** The worktree's HEAD is still `e2d7e5d` (a `git checkout` of the new
  commit would have discarded the working tree), but EX's six files are merged into it: four applied
  cleanly with `git apply`, and the two we share — `GFxDisplay.cpp` and `gfxuirenderer.cpp` — were
  merged **by hand, hunk by hand**, because `git apply -3` refuses a file the working tree has already
  changed and applying a whole file would have reverted EX's work. `git diff --stat 8a04f5f` over
  `source/` now lists exactly this package's twenty files and nothing else, which is the check that the
  merge is complete. `build/agentFA_sync.py`'s `BASE` is `8a04f5f`.
* **Every capture in this report after the rebase is DPI-aware**, i.e. the 1608x904 client is 1608x904
  physical pixels rather than 2010x1130 filtered down. The three-row logo table in §5 was taken on
  `8a04f5f` on all three rows; the pre-rebase numbers are quoted beside it and agree, which is itself
  worth knowing: the resample changed the sharpness of the frame and not the geometry of the logo.
* `build/agentFA_sync.py` is the authoritative copy list, worktree to main. It **flags rather than
  copies** any file whose content in main differs from its content at `e2d7e5d`, and refuses to copy
  anything at all while one is flagged. Dry run at the time of writing: 20 to copy, 0 flagged.
* `resources/docs/agents/agentEX.md` and `agentEX_status.csv` are untracked in this worktree only
  because its HEAD is still `e2d7e5d`; they are agent EX's, came in with its patch, and are **not**
  part of this package's copy list.
* Nothing is committed.
* `build/agentFA_base_wt` is a second detached worktree at `8a04f5f`, created only to build the
  untouched HEAD executable for the before/after pair. It holds no junction into the retail or
  reference trees, so `git worktree remove` on it is safe; I left it in place rather than removing it.

## 8a. The two acceptance runs, honestly

**Clean full release build, directory deleted first, `DISHONORED_LAYOUT_CHECKS=ON`** (the default):
done twice. `build/agentFA_wt/build/clean` on `e2d7e5d` + this package, and
`build/agentFA_wt/build/clean2` on the **rebased** tree, both 963 targets, both **0 errors, 0 C4263,
0 C4264**, both linking the executable. `build/agentFA/fa_clean_build.log` is the first;
`build/agentFA/fa_clean_build2.log` the second.

**`run_regression.py`: 37 ok, 0 failed, 0 skipped, 520 s** — on the **rebased** tree, from the
worktree's own copy, with an absolute build dir and built inside the harness (no `--no-build`).
`build/agentFA/fa_regression2.log`. The five gitignored `resources/docs/types` inputs were copied into
the worktree first, which is why `layout_types` reads 2314 and not -1.

Two things a reviewer should know about how that number was obtained.

* An **earlier** run of the same harness, on `e2d7e5d` + this package, reached 28 ok / 0 failed through
  `build`, `coresmoke`, `layout`, `nullrhi` and `d3d9` and I stopped it during `inputtest` when the
  instruction to rebase arrived — the remaining nine checks would have measured a tree that was about
  to change. `build/agentFA/fa_regression.log`.
* During that earlier run I killed the `nullrhi` and `d3d9` game processes believing they had hung past
  their timeouts. They had not: the host's clock advances only while a command of mine is actually
  running on it, so a process that looked stalled for half an hour of my time had in fact moved on by
  seconds of its own. Both stages had already written the lines their metrics are read from and both
  passed (`d3d9_frames` 15,990 against a bound of 1,000), but the kills were mine and not the
  harness's. The run quoted above had no such interference; it was waited out with a loop running on
  the host itself, which is the thing to do here and is in the hand-over.

## 9. Addresses

`rva_sweep.py` over the three directories touched:

```
External/GFx3       602 citations, 0 MISLABELLED-2012, 0 UNKNOWN-CLAIMED-2013
GFxUI               564 citations, 2 MISLABELLED-2012, both pre-existing at
                    Src/gfxuirenderer.cpp:1679 (1669 before EX's merge moved it), a line whose own
                    words say 2012 - the brief names these two as the tool misreading it
DishonoredGame     3913 citations, 1 UNKNOWN-CLAIMED-2013, pre-existing at
                    Src/disbehaviorpatrol.cpp:319
```

Every address this package cites, resolved by hand with `ida_funcs.get_func` against
`retail2013_agentFA.i64` (VA = rva + 0x400000). All 34 are function **starts** and every name is the one
claimed:

| rva | name |
|---|---|
| `0x9ac010` | `GMatrix3D::GMatrix3D(GMatrix2D const&)` |
| `0x9ac060` | `GMatrix3D::IsValid` |
| `0x9ac280` | `GMatrix3D::MultiplyMatrix` |
| `0x9ac6a0` / `0x9ac720` | `GMatrix3D::RotateX` / `RotateY` |
| `0x9ac800` / `0x9ac7a0` | `GMatrix3D::PerspectiveFocalLengthLH` / `RH` |
| `0x9aca90` / `0x9ac860` | `GMatrix3D::ViewLH` / `ViewRH` |
| `0x9abf40` / `0x9abdb0` | `GMatrix3D::MatrixInverse` / `Cofactor` |
| `0x9c4300` | `GFxCharacter::Is3D` |
| `0x9c4c40` | `GFxCharacter::CreateMatrix3D` |
| `0x9a5970` / `0x9a5990` / `0x9a59b0` | `GFxCharacter::SetMatrix3D` / `SetPerspective3D` / `SetView3D` |
| `0x9c4410` / `0x9c4460` | `GFxCharacter::GetPerspective3D` / `GetView3D` |
| `0x9c4190` / `0x9c5a50` | `GFxCharacter::GetPerspectiveFOV` / `SetPerspectiveFOV` |
| `0x9c4330` / `0x9c4390` | `GFxCharacter::GetLocalMatrix3D` / `GetWorldMatrix3D` |
| `0x9c9270` | `GFxASCharacter::SetMember` |
| `0x9c9bf0` / `0x9c6fd0` | `GFxASCharacter::SetStandardMember` / `GetStandardMember` |
| `0x9c8aa0` / `0x9c9b20` | `GFxASCharacter::GetStandardMemberConstant` / `InitStandardMembers` |
| `0x9a5b70` | `GFxValue_UpdateTransform` |
| `0xa06df0` / `0xa06e60` / `0xa06c40` | `GFxMovieRoot::SetPerspective3D` / `SetView3D` / `SetPerspectiveFOV` |
| `0xa064c0` | `GFxMovieRoot::GFxMovieRoot` (the 55° default FOV, its last instruction) |
| `0x9f93b0` / `0x9fe550` / `0x9f1780` | `GFxMovieRoot::Setup3DDisplay` / `Display`, `GFxSprite::Display` |
| `0xa53400` / `0xa53630` | `GFxDisplayContext::PreDisplay` / `PostDisplay` |
| `0x9b08c0` / `0x9b0a70` | `GRenderer::MakeViewAndPersp3D` / `Adjust3DMatrixForRT` |
| `0x576630` | `FGFxRenderer::ApplyUITransform_RenderThread` |
| `0x9d72d0` | `GASGlobalObject::SetMember` |
| `0x84cf80` | `UDisGlobalUIManager::OnMovieAttributesChanged` |

Six addresses are `sub_*` in the 2013 database and are pinned through the 2012 PDB instead (their sizes
and call graphs agree, and `functions.csv` names them):

| 2013 rva | 2012 rva | 2012 PDB name |
|---|---|---|
| `0x7e7da0` | `0x8495d0` | `UDisPostProcessManager::IsEffectRequired` |
| `0x7e7dc0` | `0x8495f0` | `UDisPostProcessManager::StartEffect` |
| `0x7e7df0` | `0x849620` | `UDisPostProcessManager::StopEffect` |
| `0x7e7ee0` | `0x849700` | `UDisPostProcessManager::SetUIPPParams` |
| `0x7efca0` | `0x8511a0` | `UDisPostProcessManager::ApplyUIPostProcessSettings` |
| `0x847070` | `0x8b75e0` | `UDisGlobalUIManager::RefreshGlobalUIState` (private) |

The data address `0x13b38c8` is `GFxASCharacter::MemberTable`, read as 115 twelve-byte rows; the six 3D
ids in §3 come from it and from `InitStandardMembers`'s own walk, not from a guess.

## 9a. EX's 0.956, and how much of it this accounts for

Agent EX measured our main-menu logo at **0.956** of retail's after correcting for the display scaling,
and attributed the residue to the missing 3D perspective. This package moves the logo by **+2.24 %**
(the table in §5), so `0.956 x 1.0224 = 0.977`: the perspective accounts for **about half** of EX's
4.4-point gap and leaves about **2.3 points** unexplained. Worth saying because "the missing
perspective" as a whole explanation would now be wrong, and the 2.1 points are somebody's.

Two consistency checks on that: EX found the *start screen's* logo already matches at 1.002, and the
start screen's logo clip carries no `_z` at all in the cook, which is exactly what a perspective-only
residue predicts. And the `-gfxui3dflat` row of §5's table is 1.000 against HEAD, so none of the 2.4
points is plumbing.

## 6a. The difficulty screen, the second case

`PHASE11.md` has carried "perspective support for the difficulty screen (`_z` is not a display property
here)" as a queued item since agent EO's wave. It is the same feature and it is now live:
`build/agentFA/fa_difficulty_before_after_8a04f5f.png`, HEAD above and this package below.

Above, `EASY / NORMAL / HARD / VERY HARD` are four flat bars at one size. Below they recede: each row
sits at its own depth and its own size, the selected one nearest. That is not a guess about what the
screen wants — the cook's own ActionScript names it. The constant pool of
`Dishonored_MainMenu.MainMenu.gfx` around that screen's `onSelectionUpdated` reads

```
onSelectionUpdated inputs SetInput _inputsList xRot idx yRot rot posZ mc this _xrotation _yrotation
_parent SetPortrait difficultyDesc _root.texts.t_difficultyDesc htmlText
```

— an x rotation, a y rotation and a z position per row index, written onto `_xrotation` / `_yrotation`
of each row's clip. Before this package those three writes landed in the clip's script object and were
read by nothing.

## 9b. Where the evidence is

`build/agentFA/`:

| file | what it is |
|---|---|
| `fa_before_menu_8a04f5f.png` | HEAD, main menu at world time 25 s, 1608x904 |
| `fa_flat_menu_8a04f5f.png` | the same with `-gfxui3dflat`: the 3D path, identity local matrices |
| `fa_after_menu_8a04f5f.png` | the same with the feature live |
| `fa_menu_before_after_8a04f5f.png` | the first and third stacked |
| `fa_flat_vs_before_8a04f5f.png` | the first and second stacked - the plumbing proof |
| `fa_after_first_z_as_pixels.png` | the first build of this package, with `_z` read as pixels |
| `fa_menu_quit_selected.png` | the main menu with `QUIT GAME` selected, showing the per-button depth |
| `fa_modal_before_8a04f5f.png`, `fa_modal_after_8a04f5f.png`, `fa_modal_before_after_8a04f5f.png` | the quit box, sharp scene and blurred scene, and the two stacked |
| `fa_difficulty_before_8a04f5f.png`, `fa_difficulty_after_8a04f5f.png`, `fa_difficulty_before_after_8a04f5f.png` | the difficulty screen, flat rows and rows with depth |
| `fa_clean_build.log`, `fa_regression.log`, `fa_regression2.log` | the two acceptance runs of 8a |

## 10. Hand-over

1. **`_z` is in the character's own twips.** §2b. If any other package touches the 3D properties, start
   from that table.
2. **`-gfxui3dflat` is the bisect that settles a projection.** It takes the 3D path with identity local
   matrices, so the picture must be the 2D one. It found the units question in one run after two days'
   worth of reasoning had not.
3. **The screenshot directory is shared and `apshottime` is not a unique name.** §2c. Any measured run
   should use a resolution no other agent is using and check the file's size, or it will eventually read
   another package's frame — mine did.
4. **Wait on the host, not in your own session.** The Windows side only advances while a command of
   yours is actually running on it: a `sleep` in the agent's own shell leaves the game paused, and a
   process that looks stalled for half an hour has in fact moved on by seconds. Every long wait here
   that worked was a PowerShell loop polling for the file or the metric. That single fact is what
   turned three failed modal captures and a "stuck" regression into a screenshot and 37 ok.
5. **The 17 % frame cost of the 3D path** is real and is retail's shape, not a defect; if the menu ever
   needs to be fast, the place to look is `UpdateMatrix3D` being called 246,850 times in 80 seconds by
   the ActionScript's own tweens, not the renderer.
