# Agent DE report — the settings path, so the content's own grade reaches the LUT (2026-09-27)

Package **DE** of `PHASE10.md` (wave 8), the direct continuation of agents CE (`dec3fd4`) and DA (`fdc6aec`), and
DA's hand-over 1 plus CE's hand-over 3. Written against **HEAD `844afbe`**. Build dir `build\agentDE` (Release, x86,
Ninja, every module option) from the snapshot worktree `build\agentDE_wt`
(`resources\tools\make_snapshot.py DE --list build\agentDE\snapshot_files.txt`, build command
`build\agentDE_release_build.cmd`). IDA copies `resources\docs\idb\{retail2013,shipping2012}_agentDE.i64`, headless
`decompile_funcs.py` and two one-off scripts only; decompiles in `build\agentDE\decomp13` and `decomp12`. No commits,
nothing staged. Status rows: `agentDE_status.csv` (28).

```
rem accept 1: what the content's grade actually is, and what now reaches the LUT
python build\agentDE\run_direct.py lut13 -arkppdoflut -arkppsettingsdbg -arkppdofdbg
rem accept 2: the pair (one build, the only difference is -noarkppsettings) and its noise floors
python build\agentDE\run_direct.py before_a -noarkppsettings
python build\agentDE\run_direct.py before_b -noarkppsettings
python build\agentDE\run_direct.py after_a
python build\agentDE\run_direct.py after_d
python build\agentDE\stats.py --diff build\agentDE\before_a.bmp build\agentDE\after_d.bmp build\agentDE\diff_pair_state_a.png
rem the baked LUT, and the same ramp against agent DA's neutral identity cube
python build\agentDE\bmp2png.py build\agentDE\lut_tower.bmp build\agentDE\lut_tower.png
python build\agentDE\stats.py --diff build\agentDE\lut_tower.bmp build\agentDA\lut.bmp build\agentDE\diff_lut.png
rem accept 3
python resources\tools\run_regression.py --build-dir build/agentDE --no-build
```

## Result

| Accept | State |
|---|---|
| what grade the content resolves to | **done and it is not neutral.** `L_Tower_P`'s own grade (`AWorldInfo::m_ArkDefaultPpSettings`) is shadow tones (0, 0.030, 0.040), mid tones (0, 0.100, 0.200), high tones (0,0,0), opacity 1.000, pre/post desaturation 0, **exposure +0.200**, gamma 1.000, **film grain 3.000**, brightness 0, **contrast 0.030**, focus 35000, in-focus radius 1500, **far blur 0.300**, bloom enabled at **scale 0.300**. The neutral script defaults it replaced, which is all agent DA could measure, were shad/mid/high (0,0,0), exposure 0, gamma 1, grain **1.050**, contrast 0, focus **1000**, radius **300**, far blur **0**, bloom scale 1.000. Section 1 has both dumps verbatim |
| what reaches the LUT | **done and measured**: the depth-of-field node's own `-arkppdofdbg` line, taken in `L_Tower_P` instead of the startup map, now reads `focus 35000.0 radius 1500.0 far blur 0.300 \| shad (0.000 0.030 0.040) mid (0.000 0.100 0.200) high (0.000 0.000 0.000)` / `opacity 1.000 pre desat 0.000 post desat 0.000 \| exposure 0.200 gamma 1.000 grain 3.000 brightness 0.000 contrast 0.030` — the level's grade, field for field, through the real `ApplyTo` |
| a pair from one build differing by one switch, quantified | **done**: `before_a.bmp` (`-noarkppsettings`, which leaves `m_CurrentArkPpSettings` at its script defaults, i.e. exactly what this tree delivered before) vs `after_d.bmp`, scene frame 200 of `L_Tower_P`, `-benchmark -fps=30`. **Mean 29.49 of a possible 765, 98.68 % of pixels changed by more than 2 per channel, maximum 154.** Frame luma **86.99 → 96.07**, R/G/B **82.59/88.11/90.28 → 85.84/97.89/104.47**. Noise floor, three identical `-noarkppsettings` runs: **byte-identical** (md5 `cc661bfd…`, mean 0.00, 0.00 %); two identical default runs from two different builds: **byte-identical** (md5 `d16c720f…`). Both legs inside world state A; section 4 names the states |
| the baked LUT as an image | **done**: `build\agentDE\lut_tower.png` (256x16, and `lut_tower_x8.png` magnified), the ramp baked from the level's grade. Against agent DA's neutral identity cube `build\agentDA\lut.png`: **mean 39.12 of 765, 99.15 % of the cube's entries, maximum 69**, cube means R 141.06 G 146.38 B 150.19 against 133.12 on all three channels. Row 0 goes from `1,1,1 22,1,1 144,1,1 255,1,1` to `0,5,7 18,5,7 155,5,7 255,5,7`: the black end lifts into green and blue by the shadow tones, the red ramp bends by the exposure and contrast, and row 8's green 144 becomes 162 by the mid tones |
| regression harness green | **31 ok, 0 failed, 0 skipped**, 427 s on the final binary (`build\agentDE\regression5.txt`; 422 s on the previous one, `regression4.txt`), own `--build-dir`: `layout_types 2314`, `layout_mismatching 0`, `layout_contract 0`, `coresmoke_passed 99`, `d3d9_frames 20220`, `d3d9_criticals 0`, `unported_natives 0`, `inputtest_moved 1028.6` |
| run | `Initial startup: 4.03 s`, **0 criticals**, census `FArkPp 1 nodes rendered, 4 draws (0 material tiles), 1 passes not ported` every frame — DA's was `2 draws`, and the two new ones are the far-blur downsample pair the level's grade turns on |
| build | 643 steps, **0 errors** |

## 1. The grade, measured

`-arkppsettingsdbg` reports the four feeds and the resolved result the first time and again whenever the result
changes. Two lines came out of one run, and they are the whole answer:

```
ark pp settings 1 (DishonoredGameFull_P, frame 1): camera alpha 0.000, gameplay override 0, recovering 0, opacity 0.000
  level    uber 1 (dof 1 cb 1 hdr 1) bloom 1 enable 1 scale 1.000 | shad (0.000 0.000 0.000)/1 mid (0.000 0.000 0.000)/1
           high (0.000 0.000 0.000)/1 opacity 1.000/1 desat 0.000/1 0.000/0 | exposure 0.000/1 gamma 1.000/1 grain 0.000/0
           brightness 0.000/0 contrast 0.000/0 | focus 1000.0/1 radius 300.0/1 far blur 0.000/1
ark pp settings 2 (l_tower_p, frame 10): camera alpha 0.000, gameplay override 0, recovering 0, opacity 0.000
  level    uber 1 (dof 1 cb 1 hdr 1) bloom 1 enable 1 scale 0.300 | shad (0.000 0.030 0.040)/1 mid (0.000 0.100 0.200)/1
           high (0.000 0.000 0.000)/1 opacity 1.000/1 desat 0.000/1 0.000/0 | exposure 0.200/1 gamma 1.000/1 grain 3.000/1
           brightness 0.000/0 contrast 0.030/1 | focus 35000.0/1 radius 1500.0/1 far blur 0.300/1
  current  <identical to level: no camera override and no gameplay override at this point>
```

Each value is followed by `/` and its own override bit. The startup map `DishonoredGameFull_P` genuinely is neutral —
which is why agent DA's figures were small and why DA's LUT was the identity ramp: **DA measured the startup map, not
the mission**. The mission's grade is a cool, slightly green-blue lift in the shadows and mid-tones, a fifth of a stop
of extra exposure, a touch of contrast, nearly three times the film grain of the default, a far focus at 350 m with a
15 m in-focus radius and a 0.3 far blur, and bloom at 30 % of the default scale. That is Dishonored's look.

The `/`-suffixed bits matter: the level overrides pre-desaturation but **not** post-desaturation and **not** brightness,
so `FArkUberPpParameters::ApplyTo` copies neither and both stay at the neutral value `SetDefaultOnNoOverride` left.

### The camera and the gameplay override are wired but silent here

`camera alpha 0.000` and `gameplay override 0` for the whole run: `L_Tower_P`'s intro never raises
`ACamera::CamOverridePostProcessAlpha` and nothing calls script's `LocalPlayer.OverridePostProcessSettings` in the
first 90 s. Both channels are ported and both are exercised by the same two helpers as the level feed
(`ArkPpConfigApplyTo` and `ArkUberPpApplyTo`), so a matinee camera or a Kismet push lands the moment the content makes
one.

### The `-startmapopen` caveat that hid the grade from the LUT dump

`AWorldInfo::GetPostProcessSettings` (0x24b0c0) reads the grade off the **persistent level's** `WorldInfo`. With
`-startmap` (the streaming route) the persistent level stays `DishonoredGameFull_P`, so a streamed `L_Tower_P` gets the
startup map's neutral grade; only `-startmapopen` makes it the persistent level. And even then, the startup map renders
a handful of frames first, which is enough to latch DA's one-shot `-arkppdoflut` dump on the neutral ramp. This package
added `build\agentDE\run_direct.py`, which passes the map as the command line's **first token**: `UGameEngine::Init`
takes a non-switch first token as the travel URL, so `L_Tower_P` becomes the very first `LoadMap` and every one-shot
probe latches on the mission. That is how `lut_tower.png` and the `-arkppdofdbg` line above were taken.

## 2. What retail's settings path is, and what the reference body was doing instead

`ULocalPlayer::UpdatePostProcessSettings` (2013 rva 0x2b08b0) is 377 bytes and fills exactly one member,
`m_CurrentArkPpSettings`, from four feeds:

| feed | retail |
|---|---|
| LEVEL | `AWorldInfo::GetPostProcessSettings(const FVector&,FArkPpConfig&)` (0x24b0c0) — `m_ArkDefaultPpSettings`, 132 bytes, copied whole |
| ACTOR | `APlayerController::ModifyPostProcessSettings(FArkPpConfig&)`, vtable slot 304, empty in the base; `ADishonoredPlayerController`'s override is 0x6adeb0 |
| CAMERA OVERRIDE | `ACamera::m_CamPostProcessSettings` through `FArkPpConfig::ApplyTo` (0x2adb50) when `CamOverridePostProcessAlpha > 0` |
| GAMEPLAY OVERRIDE | `m_ArkPpSettingsOverride` blended at an opacity that fades over `OverridePPRecoveryTime` when the override is dropped |

`CalcSceneView` then hands the member to the view (`FSceneView::m_ArkPpConfig`, UnPlayer.cpp:4245), `FViewInfo`'s
constructor pushes its `m_UberPpParameters` onto the proxy config's override stack at weight 1
(`SceneRendering.cpp:285`), and the depth-of-field node's proxy resolves that against the node's own parameters and
bakes the LUT. **Every link in that chain already existed; only the first one was missing.**

The reference body that stood in `UnPlayer.cpp:3547` blended `FPostProcessSettings` through a post-process volume
lookup, an interpolation state (`LevelPPInfo`, `CurrentPPInfo`) and an override array (`ActivePPOverrides`). Retail's
licensee branch has **no `FPostProcessSettings` script struct at all** — the name does not appear once in the retail
SDK dump (`retail_sdk_layout.json`: 1,118 structs, zero matches), and `CurrentPPInfo`, `LevelPPInfo` and
`ActivePPOverrides` are `DISHONORED_SHIM_STATIC` shims in this tree because retail's `ULocalPlayer` has no such
members. So the whole body computed a structure nothing downstream reads and left the member the renderer does read at
its script defaults. This is the eighth instance of the pattern, and the largest one by consequence: the level's, the
camera's and Kismet's colour grade never reached the LUT.

Retail also ignores the `ViewLocation` it is handed: there is **no post-process volume lookup anywhere** in retail's
settings path. The Arkane grade is per level, per camera and per gameplay push — never per volume. The only other
things in the cook that carry an `FArkPpConfig` are `CameraActor::m_CamOverridePostProcess`,
`DishonoredPlayerCamera::m_PpSettings`, `DishonoredPlayerPawn::m_ArkPpSettings`,
`DishonoredWaterVolumeInfo::m_PpOverride` and three tweak classes, and all of them arrive through the ACTOR or CAMERA
channel.

## 3. Defects found

1. **`GSystemSettings.MaxFilterBlurSampleCount` is a storage-less shim, so every Gaussian blur in the tree aborted
   the render thread** (`SceneFilterRendering.cpp:619`, 2013 rva 0x45c3d0). It is a `DISHONORED_SHIM_STATIC INT` —
   retail's `FSystemSettings` has no such member — so it reads 0, `Compute1DGaussianFilterKernel` computes
   `ClampValue = Min(16, 0) - 1 = -1`, its loop never runs, it returns 0 samples, and `SetFilterShaders` calls
   `appErrorf("Invalid number of samples: 0")`. The first content that ever reached the line was the level's own far
   blur (0.300) the moment this package delivered it: the very first run with the settings path on died at 4.56 s with
   a render-thread exception. Retail passes no maximum at all — it clamps the kernel radius instead
   (`Min(KernelRadius / FilterDownsampleFactor, 16) * (SizeX * FilterDownsampleFactor / 1280)`, verified in the
   disassembly, `build\agentDE\gauss12.asm`) — so the `MAX_FILTER_SAMPLES` default is what retail gets. Fixed at the
   call site. **This blocked every blur pass in the tree, not just this one**, which is why it matters past this
   package: agent DA's `Downsample` was never reached, the reference bloom and DOF chains would have died the same
   way, and agent DB's bloom Gaussian would have too.
2. **Retail's `UArkPpNodeController::IsShown` returns TRUE, not FALSE.** Slot 74 of the base's vtable is 2013 rva
   0x5ea9d0, whose entire body is `mov eax, 1; retn 4` (identical-code-folded with `UObject::IsRefSaveable`, which is
   the name the vtable dump prints). Agent CE had to answer FALSE because no subclass existed and the one node that
   would then have drawn carries a closed eyelid; with both overriding subclasses ported the base can answer retail's
   TRUE. The frame is **byte-identical** either way on this content (section 4), because the two controller-driven
   nodes that matter are hidden by their own ported `IsShown` at rest and the other two nodes are `m_bShowInGame 0`.
3. **Retail's gameplay override cannot change a bloom field, only mark the group overridden.** Both
   `UpdatePostProcessSettings` and `FArkPpConfig::ApplyTo` raise the destination's `m_bOverrideBloomPpParameters` bit
   and copy no bloom value with it (`*(_DWORD *)(a2 + 100) |= 1u` and nothing else). Ported as retail has it and
   documented at both sites; it means `m_PpBloomParameters` reaches the renderer from the level only.
4. **`FArkUberPpParameters::ForceDefault` clears only three of the HDR group's five override bits** (`&= 0xFFFFFFF8`,
   0x2a2e00), leaving `m_bOverrideGimpBrightness` and `m_bOverrideGimpContrast` set if they were;
   `SetDefaultOnNoOverride`'s else branch has the same three-bit mask. Ported as retail has it, because those two bits
   decide what the next `ApplyTo` copies.
5. **`UDisOpacityParameterPpController`'s rise and its visibility disagree about `m_bDoNotDisablePp`.** `Tick`
   (0x7eaba0) raises `m_CurrentTime` only while `m_bIsOn` is set, but `IsShown` (0x7e7ca0) also accepts
   `m_bDoNotDisablePp`, so a controller with that flag and `m_bIsOn` clear keeps its node drawn all the way down the
   fade-out. Consistent with the flag's name; ported as retail has it.
6. **`UDisDarkVisionPpController::Update` gives the power material the *closed*-lid colour.** Both branches set the
   same two parameters, `Alpha` and `EyeLidColor`, and both take `EyeLidColor` from `m_GlobalParameters.m_ClosedLidColor`
   (0x7ee630). Ported as retail has it.
7. **Nothing in this tree calls `UArkPpNodeController::Tick`.** In retail the tick sits behind
   `ADishonoredPlayerController::ModifyPostProcessSettings` (0x6adeb0), which finds the graph's nodes by name and
   drives the water, health, dark-vision and possession effects. Until that is ported, `m_CurrentTime`, `m_EyeLidTime`
   and `m_PowerTime` stay 0 and both `IsShown` bodies answer their retail rest state, which is why defect 2's fix is
   invisible on this content. Said at the site and in hand-over 1.
8. **`UnPlayer.cpp` is shared with agent DC and HEAD does not build with DC's half of it.** DC's in-flight edits to
   `UGameViewportClient::SetViewport`/`Draw` call four `DishonoredGFx*` functions that exist only in DC's uncommitted
   `GFxUI/Src/gfxuiengine.cpp`, so overlaying the working-tree `UnPlayer.cpp` into a HEAD snapshot fails the link with
   four `LNK2019`. `build\agentDE\snapshot_unplayer.py` builds the snapshot's copy as HEAD plus this package's block
   only; the shared working tree keeps both agents' edits, which is what the coordinator merges. This is DA's defect 7
   one file over, and the coordinator should merge DC's `gfxuiengine.cpp` in the same commit as DC's `UnPlayer.cpp`.
9. **`dispostprocesscontrollers.cpp` was in `DishonoredGame/Sources.cmake`'s exclude list**, along with the other 862
   comment-only skeleton units, so a body written into it linked to nothing. One line removed; whoever fills another
   skeleton unit has to do the same.

## 4. The pair, and the world states

One build, one switch, `-startmap` replaced by the direct URL so there is one `LoadMap` and no world change:

| pair | mean of 765 | % of pixels > 2 | max | state |
|---|---:|---:|---:|---|
| `before_a` (`-noarkppsettings`) vs `after_d` | **29.49** | **98.68 %** | 154 | A / A |
| `before_a` vs `before_b` vs `before_c` (three identical runs) | 0.00 | 0.00 % | 0 | A, byte-identical |
| `after_a` vs `after_d` (two identical runs, **two different builds**) | 0.00 | 0.00 % | 0 | A, byte-identical |
| `after_c` vs `after_d` (two identical runs of one build) | 67.94 | 67.73 % | 752 | **B / A** |
| `before_d` vs `after_f`, machine loaded by another agent's runs | 29.10 | 98.82 % | 144 | two further states |

Frame statistics inside state A: luma **86.99 → 96.07**, R **82.59 → 85.84**, G **88.11 → 97.89**, B
**90.28 → 104.47**, median 70 → 78. The frame gets brighter by a fifth of a stop and distinctly cooler and greener,
which is what shadow tones (0, 0.03, 0.04) and mid tones (0, 0.10, 0.20) over an exposure of +0.2 do.

Agent DA's caveat holds and this package reproduces both halves of it. On a **quiet** machine the direct-URL route is
**bit-exact**: three `-noarkppsettings` runs and two default runs across two different builds each produced one md5.
Under load it is not: one run in three landed in a second world state (`after_c`), and once other agents began running
the game the `-noarkppsettings` leg stopped repeating too (`before_d`). The pair above is taken inside state A and
cross-checked by the loaded-machine pair, which agrees to 1.3 % on the mean and 0.14 % on the pixel count. The single
`-benchmark -fps=30` frame step is fixed, so the divergence is not the time step; it is the intro's own state at
scene frame 200.

`after_a` (built before the controller work) and `after_d` (built after it) being **byte-identical** is the
measurement of defect 2: retiring agent CE's FALSE stand-in for retail's TRUE, and porting three controllers' worth of
`IsShown`, `Tick` and `Update`, changes this content's frame by exactly zero bytes — which is what it must do, since
every controller is at rest.

## 5. Not ported, and exactly what each one needs

* **`ADishonoredPlayerController::ModifyPostProcessSettings` (0x6adeb0)** — the ACTOR feed and hand-over 1. 1,498
  bytes: it looks up about thirty graph nodes by name (`DisGetArkPpNode`, `DisGetArkPpNodeMaterial`) and caches them
  with a copy of two nodes' 96-byte uber parameters, then calls `ADishonoredPlayerCamera::ApplyCameraPostProcess`,
  `ApplyWaterPostProcessSettings`, `ADishonoredPlayerPawn::ApplyHealthEffectsPost`,
  `ApplyDarkVisionPostProcessSettings` and two free functions on the config. The seam exists: retail's base
  `APlayerController::ModifyPostProcessSettings(FArkPpConfig&)` is declared and empty, and this package's port calls
  it. It is also where the controllers get ticked (defect 7).
* **`UDisBlindedPpController::Update` (0x7ea620)** — hand-over 3's remainder: a runtime material driven from a 1D
  Perlin noise over `GBlindedT` (`SmoothIntNoise` 2012 0x8493f0, `PerlineNoise1D` 0x84b980). Its node in `Test_PPG` is
  `m_bShowInGame 0`, so nothing reaches it in game.
* **`UDisDarkVisionMeshRenderPpController::Render` (0x7fe690)** — a mesh pass of its own,
  `TSoulPartMeshDrawingPolicy<FSoulPartMeshPolicy>` with its own vertex and pixel shader types, not a post-process
  pass. No node in the shipped chain carries this controller.
* **`UPostProcessChain::CreateMutableMaterialInstanceOnNodes` (0x2d2a40)** — each material node's own mutable
  instance. The three controllers' `Update` do the same job per controller and are ported, so this is only needed by
  nodes without a controller.
* **The rim-light loop in `CalcSceneView`** reads `CurrentPPInfo.LastSettings.RimShader_Color`, and
  `CurrentPPInfo` is now a pure shim: retail's `CalcSceneView` (2012 rva 0x2df1c0) has no such loop and retail has no
  `FPostProcessSettings`, so the loop writes the struct's default colour into `APawn::MIC_PawnMat` and
  `MIC_PawnHair` — neither of which anything in this tree ever assigns. Left untouched and named here so it is
  retired with the rest of the reference post-process state, not silently.
* **`FArkPpConfig::ApplyTo`'s `bDOFOnlyBlendAmount` parameter** is threaded through and always FALSE from both call
  sites, as in retail.

## 6. Numbers

| Measure | HEAD `844afbe` (DA merged) | With package DE |
|---|---:|---:|
| grade reaching the LUT in `L_Tower_P` | the script defaults (neutral) | **the level's own** (section 1) |
| baked LUT vs DA's neutral identity cube | — | **99.15 %** of entries, mean 39.12 of 765, max 69 |
| pixels the settings path changes, one world state | — | **98.68 %**, mean 29.49 of 765, max 154 |
| noise floor, three `-noarkppsettings` runs | — | **byte-identical** (0.00, 0.00 %) |
| noise floor, two default runs, two builds | — | **byte-identical** (0.00, 0.00 %) |
| frame luma | 86.99 | **96.07** |
| FArkPp draws per frame | 2 | **4** (the far blur's downsample pair) |
| passes still stubbed in the census | 1 | 1 (the AA node) |
| Gaussian blur passes that can run at all | **0** (every one aborted the render thread) | all of them |
| `Initial startup` (d3d9, direct URL) | 3.4 s | 4.03 s |
| scene frames, 90 s regression run | ~20,000 | 20,220 |
| criticals | 0 | 0 |
| regression | 31 checks | **31 ok, 0 failed, 0 skipped** |
| build | — | 643 steps, 0 errors |

`probe_natives` reads 8 against the harness's note of 7 at HEAD (bound 12). The eight are
`ADishonoredPawn::execOnTeleport_Native`, `ADishonoredPlayerController::exec{CalcPlayerSwimAccelRate,DisToggleSprint,
OnTeleport_Native,PreSetCinematicMode_Native,SetCinematicMode_Native}`,
`ADishonoredPlayerPawn::execPlayDying_Native` and `ADishonoredSpawner::execOnStartSpawn` — all reached only by
`-distouchprobe` walking the pawn to every volume, trigger and pickup, and none of them on this package's path.

## 7. Files

Mine (14):

| File | Change |
|---|---|
| `Engine/Src/UnPlayer.cpp` | retail's `UpdatePostProcessSettings`, `AWorldInfo::GetPostProcessSettings`, `ArkUberPpForceDefault`, `ArkPpConfigApplyTo`, the census and the two switches |
| `Engine/Inc/arkpp.h` | the two helper declarations |
| `Engine/Inc/EngineGameEngineClasses.h` | retail's `GetPostProcessSettings` overload, one line |
| `Engine/Inc/EngineControllerClasses.h` | retail's `ModifyPostProcessSettings(FArkPpConfig&)`, empty base |
| `Engine/Src/SceneFilterRendering.cpp` | defect 1, the sample-count shim out of the kernel computation |
| `Engine/Inc/enginearkppclasses.h` | defect 2, the base `IsShown` answers retail's TRUE |
| `Engine/Src/arkppnodes.cpp` | comment only, at `IsShownInConfig` |
| `DishonoredGame/Src/dispostprocesscontrollers.cpp` | the three controllers' bodies (was a comment-only skeleton) |
| `DishonoredGame/Inc/CppText/UDis{OpacityParameter,Blinded,DarkVision}PpController.h` | the virtual declarations (new, 3 files) |
| `DishonoredGame/Inc/DishonoredGamePostProcessClasses.h`, `DishonoredGamePowerClasses.h` | the three `#include "CppText/…"` lines the generator will emit by itself now |
| `DishonoredGame/Sources.cmake` | defect 9, one line removed from the exclude list |

`Engine/Src/arkppnodedof.cpp` was **not** touched (agent DA's), nor `SceneRendering.{cpp,h}`, `Scene.h`, the `GFxUI`
module or any AI unit. `Engine/Src/UnPlayer.cpp` in the working tree holds **both** this package's block and agent DC's
GFxUI seam edits; defect 8 explains how the snapshot separates them and what the coordinator has to merge together.

Helpers in `build\agentDE\` (not repo tools): `patch_settings.py`, `patch_describe.py`, `patch_filter.py`,
`patch_controllers.py`, `patch_rvas.py` (the edits, written as files because heredocs mangle CRLF),
`snapshot_unplayer.py` (defect 8), `run_direct.py` (the map as the command line's first token), `run_pair.py`,
`stats.py`, `bmp2png.py`, `dump_asm.py`, `xref.py`.

## 8. Bring-up switches (both function-local statics, per agent CA's finding)

| Switch | What it does |
|---|---|
| `-noarkppsettings` | `UpdatePostProcessSettings` returns after the world time, leaving `m_CurrentArkPpSettings` at its script defaults: the switch of the pair, and an exact reproduction of what this tree delivered before the port |
| `-arkppsettingsdbg` | the four feeds and the resolved result, the first time and whenever the result changes, capped at 40 lines |

`-arkppdoftestgrade` (agent DA's stand-in for this package) can now go: a real grade arrives on its own.
`-arkppcontrollersshown` (agent CE's) is now a probe rather than a stand-in and should go with the rest of the
graph's bring-up lines.

## 9. Hand-overs

1. **`ADishonoredPlayerController::ModifyPostProcessSettings` (0x6adeb0), the ACTOR feed.** It is the last unported
   channel of the settings path and it is what makes the powers visible: it drives the water, health, dark-vision and
   possession effects, and it is where retail ticks the post-process controllers (defect 7), so the three controllers
   ported here are dormant until it lands. Its callees — `ADishonoredPlayerCamera::ApplyCameraPostProcess`,
   `ApplyWaterPostProcessSettings`, `ADishonoredPlayerPawn::ApplyHealthEffectsPost`,
   `ApplyDarkVisionPostProcessSettings` — are `ADishonoredPlayerController`/`ADishonoredPlayerPawn` work and belong
   with whoever takes the player's powers. The seam is already in place and already called.
2. **The far blur is live and untested beyond frame 200.** `m_FarBlurAmount` is 0.300 in this level, so
   `FArkPpNodeDofProxy::Downsample` (0x522210) now runs every frame on real content for the first time. Agent DA's one
   documented deviation is in it: retail's `GaussianBlurFilterBuffer` takes an absolute radius while this tree's
   scales by `ViewSizeX / 1280`, and DA passes the node's own width as the view width. Retail's arithmetic is now
   known exactly — `Min(KernelRadius / FilterDownsampleFactor, 16) * (SizeX * FilterDownsampleFactor / 1280)`, two
   passes, no early-out (`build\agentDE\gauss12.asm`) — so the deviation can be closed against it. That file is agent
   DA's, so it is a hand-over and not a change here.
3. **Hand-over 3's remainder**: `UDisBlindedPpController::Update` (0x7ea620),
   `UDisDarkVisionMeshRenderPpController::Render` (0x7fe690) and
   `UPostProcessChain::CreateMutableMaterialInstanceOnNodes` (0x2d2a40) — section 5.
4. **The reference post-process state can now be retired**: `FPostProcessSettings`, `APostProcessVolume`,
   `FPostProcessSettingsOverride`, `FCurrentPostProcessVolumeInfo`, `ULocalPlayer::{CurrentPPInfo,LevelPPInfo,
   ActivePPOverrides}`, `UpdatePPSetting`, `OverridePostProcessSettings*` and `ClearPostProcessSettingsOverride` as
   natives (retail's are script functions taking an `ArkPpConfig`), and the rim-light loop in `CalcSceneView`. None of
   it exists in retail and nothing in retail's renderer reads any of it. It is a tidy-up, not a port, but it is what
   stops the next agent mistaking one of them for the real thing — which is how this defect survived seven waves.
5. **`arkcomponentbase.h` is committed with a body now** (agent DA's defect 7 is closed: HEAD `844afbe` builds
   `DishonoredGameModule` with no overlay). Defect 8 is the same shape one file over and is still open.
