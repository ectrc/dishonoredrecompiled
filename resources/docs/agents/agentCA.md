# Agent CA report — the scene-colour rebinding defect (2026-09-27)

Package **CA** of `PHASE9.md` (wave 7). Measured from **HEAD `2cdd7b3`**. Build dir `build\agentCA_wtrel` (Release,
x86, Ninja, every module option), built from the snapshot worktree `build\agentCA_wt` (= HEAD + my four files,
`resources\tools\make_snapshot.py CA`, list in `build\agentCA\snapshot_files.txt`, build command
`build\agentCA_release_build.cmd`; the shared tree stopped compiling mid-package on `disaicensus.cpp`, agent CG's
area). IDA copies `resources\docs\idb\retail2013_agentCA.i64` / `shipping2012_agentCA.i64`, headless
`decompile_funcs.py` and a small `build\agentCA\disasm_range.py` only; decompiles in `build\agentCA\decomp12`.
No commits, nothing staged. Status rows: `agentCA_status.csv`.

## Result

**The binding was never broken. The switch that was supposed to prove it was.**

Agent BD's hand-over said the resolve leaves the resolve destination bound and *nothing re-binds the scene colour
surface*, so a pass written in retail's shape draws into a surface nothing resolves again. The first half is true and
is also what retail does. The second half is not: `BeginRenderingSceneColor` re-binds the surface on this tree at every
call, measured. What was actually wrong is that **BD's `-fogresolvetargets` — and `-nopostprocess` and `-referencefog`
with it — could never be on**: all three were file-scope

```cpp
static UBOOL GDisFogResolveTargets = ParseParam(appCmdLine(),TEXT("fogresolvetargets"));
```

initialisers. A static library's dynamic initialisers run before `WinMain`, and `GCmdLine` (`UnMisc.cpp:4567`) is still
an empty string then, so every one of them was permanently `FALSE`. The pass that ran in every measurement was BD's
workaround, not retail's shape — and it is the *workaround* that changes no pixels, because it draws into the resolve
destination and the next `ResolveSceneColor` of the frame overwrites it from the surface.

| Accept | State |
|---|---|
| a pass in retail's shape (resolve, begin, draw, finish) changes pixels | **yes.** Retail's shape vs the same build with the fog off: **mean per-pixel difference 3.44 of 765, 8.4 % of pixels changed by more than 2 per channel, maximum 295** (`build\agentBD\compare_pair.py`). The change is the distance haze over the bridge, the ships and the sky of `L_Tower_P`; the near geometry is untouched, which is what two exterior layers with near planes of 2500 and 80000 do |
| `-fogresolvetargets` passing is the acceptance | The switch is **retired**, because it now has nothing to select between: BD's deviation is gone and retail's sequence is unconditional. The acceptance as written ("the same image as the current workaround") could not be met and should not be: with the switch alive, **the workaround is byte-equal to no fog at all** (mean 0.00 of 765, 0.0 % of pixels, maximum 37) while retail's shape produces the fog |
| BD's documented deviation retired | done, `FogRendering.cpp`: `ResolveSceneColor` / `BeginRenderingSceneColor(0)` / `FinishRenderingSceneColor(1, view rect)` unconditionally, which is exactly retail's `RenderFog` (2013 rva 0x4370a0) |
| harness green | see "Numbers" |
| what retail does that we did not | section 3 |

## 1. What was measured, and how (package item 1)

`-carttrace` (temporary, removed before the final build) logged, at every `FD3D9DynamicRHI::CopyToResolveTarget`, the
surface the device actually had bound (`GetRenderTarget(0)`), the source surface, the source's dedicated texture's
level-0 surface, the resolve target's, and which of the three branches the call took; `BeginRenderingSceneColor`,
`FinishRenderingSceneColor` and `ResolveSceneColor` logged alongside it.

```
carttrace switch: file-scope static 0, lazy static 1, command line '... -carttrace ...'
carttrace 3: bound rt 20C0BFC0, source surface 20C0BFC0, source texture surface 20C0BFC0,
             resolve texture surface 20C0BE00, texture2d 09584860, resolvetex 08DA2CC0, param 00000000
carttrace 3: quad blit path, dest 1280x720
```

Two things in one run:

1. **the same `ParseParam(appCmdLine(),…)` expression is 0 at file scope and 1 inside a function**, printed side by
   side with the command line it was reading. That is the defect.
2. **`bound rt == source surface == source texture surface` at every one of the 60 traced resolves** (0 of them had a
   null `Texture2D`). The device really does have the scene colour *surface* bound whenever scene colour is resolved,
   so every `BeginRenderingSceneColor` between two resolves did bind it. Scene colour is a `TargetSurfCreate_Dedicated`
   surface (`SceneRenderTargets.cpp:1715`), so the resolve is the full-screen quad blit, and that blit leaves the
   destination bound — as it does in retail.

The tree's own behaviour confirms it from the other side: with the switches alive, `-nopostprocess` and the fog
workaround give the **same image to within the comparison's threshold** (`mean 0.00`), because the workaround draws
into the resolve texture and the next `FinishRenderingSceneColor` of the frame blits the surface over it.

## 2. Retail's resolve and its scene-colour begin/finish, term for term (package item 1)

Decompiled from the 2012 Shipping exe (named) and checked against the 2013 retail exe by disassembly.

| Function | 2013 rva | 2012 rva |
|---|---|---|
| `FD3D9DynamicRHI::CopyToResolveTarget` | 0x5c1f70 | 0x609380 |
| `FSceneRenderTargets::BeginRenderingSceneColor` | 0x448ff0 | 0x46c550 |
| `FSceneRenderTargets::FinishRenderingSceneColor` | 0x4490b0 | 0x46c610 |
| `FSceneRenderTargets::ResolveSceneColor` | 0x449160 | 0x46c6c0 |
| `FSceneRenderer::RenderFog` | 0x4370a0 | 0x4599e0 |

**`BeginRenderingSceneColor(DWORD)`** — retail takes one argument and is three statements: `CopyFromResolveTarget` when
`usage & RTUsage_RestoreSurface`, then `SetRenderTarget(RenderTargets[1].Surface, RenderTargets[6].Surface)`. Ours is
the same plus the `SP_PCD3D_SM5` MRT block, which never runs on d3d9. **No difference that matters.**

**`ResolveSceneColor(const FResolveParams&)`** — retail is `CopyToResolveTarget(SceneColor.Surface, TRUE,
ResolveParams)` then `bSceneColorTextureIsRaw = 0`. Ours matches (our extra `bKeepOriginalSurface` parameter defaults
to the constant retail passes). **No difference.**

**`FinishRenderingSceneColor(UINT, const FResolveParams&)`** — retail hands *its* resolve params straight to
`ResolveSceneColor`. Ours called `ResolveSceneColor()` with no argument and **dropped the rect**, so every finish
resolved the whole buffer. Fixed.

**`CopyToResolveTarget`** — four differences, two of them real:

| Retail | This tree before | Now |
|---|---|---|
| entry test `Texture2D && (Texture2D != ResolveTarget2D \|\| ResolveTargetTextureCube)` | no `Texture2D &&` term | retail's test, with the reference's no-dedicated-texture case kept as an explicitly marked fallback (below) |
| no `DestinationSurface == *SourceSurface` early-out, no `GetDesc(Source)`, **no StretchRect branch at all** | both present | kept (below) |
| `Direct3DDevice->SetDepthStencilSurface(NULL)` between `SetRenderTarget` and `SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE)` | **missing** | **ported** |
| `RHISetColorWriteMask(CW_RGBA)` after the four state setters, immediately before the two `AddTriangle` calls | **missing** | **ported** |

The colour write mask is the one that would have bitten the rest of this wave. The resolve inherits the mask of the
pass that has just finished. The DisFog pass sets `CW_RGB` (`FogRendering.cpp:1409`, retail 0x436d10) and only restores
`CW_RGBA` *after* `FinishRenderingSceneColor`, so its own resolve was dropping alpha; a pass that had turned colour
writes off entirely would have resolved nothing at all and looked exactly like the defect BD described. Retail sets the
mask inside the resolve so the caller's state cannot reach it. Verified in both exes: 2012 `609ae1: mov eax,[edx+168h];
push 0Fh; call eax`, 2013 `5c26d7: mov eax,[edx+170h]; push 0Fh; call eax`, in both cases the last call before
`FBatchedElements::AddTriangle`.

**`RenderFog`** (0x4370a0 / 0x4599e0) confirms BD's port and my retirement of the switch:

```
if (!m_BloomNeedBlit)  ResolveSceneColor(full rect)
BeginRenderingSceneColor(0)
RenderFogPass(interior) / RenderFogPass(exterior)
FinishRenderingSceneColor(1, view rect)
SetColorWriteEnable(TRUE); SetColorWriteMask(0x0F); ... SetScissorRect(0,0,0,0,0)
```

Both the begin and the finish are unconditional in retail, and the argument to the begin is 0 — no
`RTUsage_RestoreSurface`, which is right, because the pass overwrites the whole view.

## 3. What retail does that we did not

* **`CopyToResolveTarget` has no MSAA and no StretchRect in retail.** Retail's `CreateTargetableSurface` (0x604730 in
  2012) has no `TargetSurfCreate_Multisample` handling, no `CheckDeviceMultiSampleType` and no `CreateRenderTarget`
  for a resolve-targetable surface: it either creates a dedicated `IDirect3DTexture9` or returns the resolve texture's
  own surface. So in retail a resolve-targetable surface always owns a `Texture2D`, and the `Texture2D &&` guard costs
  retail nothing. This tree keeps the reference engine's MSAA path, which can produce a surface with
  `Texture2D == NULL` that only `StretchRect` can resolve, and keeps the reference's cube-resolve case (a surface with
  a cube resolve target and no 2D one, which `L_Tower_P` uses 12 times at load for its reflection captures and which
  retail resolves with the quad). Both are marked at the site as reference-only; retail's guard is otherwise in place.
  Measured: in a full run, `Texture2D` was null at **0** of 60 resolves, so the guard changes nothing on this build.
* **`BeginRenderingSceneColor`'s SM5 MRT block** is reference code with no retail counterpart. Left alone: it is
  `GRHIShaderPlatform == SP_PCD3D_SM5` and d3d9 never enters it.
* **`RHICopyFromResolveTarget` is a stub in this tree's d3d9 RHI** (`D3D9RenderTarget.cpp`, three functions that only
  reference `FScreenVertexShader`/`FScreenPixelShader` so the types survive the linker). Retail's
  `BeginRenderingSceneColor` calls it when `usage & RTUsage_RestoreSurface`. Nothing on the walking path passes that
  flag today — every traced call had usage 0 — so no pass depends on it yet, but a post-process node that wants the
  previous surface contents back will. Left as it is, named here; it is not this package's file to fill.

## 4. The fix

Four files.

1. **`D3D9Drv/Src/D3D9RenderTarget.cpp`** — `CopyToResolveTarget`: retail's entry guard, `SetDepthStencilSurface(NULL)`
   before the blit, `RHISetColorWriteMask(CW_RGBA)` before the draw.
2. **`Engine/Src/SceneRenderTargets.cpp`** — `FinishRenderingSceneColor` forwards its resolve rect.
3. **`Engine/Src/SceneRendering.cpp`** — `-referencefog` and `-nopostprocess` become function-local statics
   (`DishonoredRenderReferenceFog()`, `DishonoredNoPostProcess()`), which is the reference engine's own idiom for these
   (`UnAnimPlay.cpp:583`, `UnChan.cpp:2212`). Only the two switch declarations and their two call sites changed, so
   package CE rebases cleanly.
4. **`Engine/Src/FogRendering.cpp`** — `-fogresolvetargets` and BD's deviation deleted; retail's pair is unconditional.

## 5. Numbers

| Measure | HEAD `2cdd7b3` | With package CA |
|---|---:|---:|
| pixels the DisFog pass changes, same camera, same build, one switch (`-nopostprocess`) | **0.0 %**, mean 0.00 of 765 | **8.4 %**, mean **3.44** of 765, maximum 295 |
| DisFog layers drawn in the census (fog on / `-nopostprocess`) | 2 / **2** (the switch was dead) | 2 / **0** |
| `-fogresolvetargets`, `-nopostprocess`, `-referencefog` reaching their code | **never** | on when passed |
| resolves whose bound render target was the scene colour surface | 60 of 60 (measured; the binding was never the defect) | unchanged |
| resolves with a null `Texture2D` (what retail's entry guard excludes) | 0 of 60 | 0 of 60 |
| image with retail's sequence vs. the switch-only build | - | **identical**, mean 0.00 of 765, maximum 31: the two ported resolve terms cost nothing visible today (nothing yet reads scene colour alpha), and they are what stops the next pass that masks colour writes from losing its resolve |
| regression harness | 31 checks | **31 ok, 0 failed, 0 skipped** (`build\agentCA_wtrel\regression\summary.txt`, 424 s) |
| run | - | `Initial startup 3.4 s`, 19,890-20,100 scene frames in 90 s, 0 criticals |

```
rem accept: the pair, one build, one switch
python resources\tools\build_and_smoke.py --build-dir build/agentCA_wtrel --no-build --exe-name DishonoredGame_CA.exe ^
  --log-name agentCA_final_fog.log --ini-dir build/agentCA/config --rhi d3d9 --timeout 60 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --forbid "Critical" --skip-native OnlineSubsystemPC ^
  "--extra-args=-startmap=L_Tower_P -startmapopen -apshot=200 -benchmark -fps=30 -forcelogflush -windowed -ResX=1280 -ResY=720 -nomovie"
rem the same run with -nopostprocess for the other half of the pair
python build\agentBD\compare_pair.py build\agentCA\final_nofog.bmp build\agentCA\final_fog.bmp
rem regression
python resources\tools\run_regression.py --build-dir build/agentCA_wtrel --no-build
```

Images: `build\agentCA\final_fog.bmp` / `final_nofog.bmp` (and `.png` through `build\agentCA\bmp2png.py`, written
because this environment has no PIL), plus the three shots of the intermediate build that isolate the switch:
`workaround.bmp` (BD's deviation), `retailseq.bmp` (retail's sequence behind the repaired `-fogresolvetargets`),
`nopp.bmp`.

**On agent BD's pair.** BD reported 37.0 % of pixels and a mean of 27.96 for the same pass. With the switches actually
working, the DisFog pass on this tree changes 8.4 % and a mean of 3.44, and the two halves of my pair differ by exactly
the census line (`DisFog 2 drawn in 1 passes` against `0 drawn in 0 passes`), so it really is one build and one switch.
BD's pair cannot have been: `-nopostprocess` could not be on in either half of it.

## 6. Hand-overs

1. **Every agent of this wave, and the coordinator**: a switch written as a file-scope
   `static UBOOL G… = ParseParam(appCmdLine(), …);` in a static library is **always FALSE**. Put it in a function:
   `static UBOOL Foo() { static UBOOL bOn = ParseParam(appCmdLine(), TEXT("foo")); return bOn; }`. All three that
   existed were bring-up switches introduced in wave 6 and all three were dead; the tree has none left. Any
   before-and-after pair taken with one of them before today is worth re-taking.
2. **Package CE (the Arkane post-process graph) and CC (the interface renderer)**: the pattern your passes need is
   retail's and it works — `ResolveSceneColor()` so the pass can sample scene colour, `BeginRenderingSceneColor(0)`,
   draw, `FinishRenderingSceneColor(TRUE, FResolveRect(view rect))`. `RenderFog` (`FogRendering.cpp`) is the worked
   example. Do **not** draw into whatever the caller left bound: after a resolve that is the resolve texture, and the
   next `FinishRenderingSceneColor` of the frame blits the surface straight over it. That is what made BD's workaround
   invisible, and it is the failure mode to expect if a new pass draws and nothing appears.
3. **Package CE**: `RHICopyFromResolveTarget` is a stub in this tree's d3d9 RHI (section 3). A node that wants the
   previously resolved contents back in the surface — `BeginRenderingSceneColor(RTUsage_RestoreSurface)` — will get
   nothing. Nothing passes that flag today (every traced call had usage 0), so it is a gap, not a live defect.
4. **Whoever owns `D3D9RenderTarget.cpp` next**: retail's `CopyToResolveTarget` has no `StretchRect` branch, no
   `DestinationSurface == *SourceSurface` early-out and no MSAA anywhere in `CreateTargetableSurface`. Those three
   reference terms are still here, marked, because this tree's `CreateTargetableSurface` is still the reference's and
   can produce surfaces retail cannot. Porting `CreateTargetableSurface` to retail's shape is what would let them go.
5. **Bring-up lines to drop when the graph lands**: `-nopostprocess` and `-referencefog` (now that they work), as BD's
   hand-over 5 already lists.

## 7. Files

Mine (`build\agentCA\snapshot_files.txt`, 4):
`D3D9Drv/Src/D3D9RenderTarget.cpp`, `Engine/Src/SceneRenderTargets.cpp`, `Engine/Src/FogRendering.cpp`,
`Engine/Src/SceneRendering.cpp` (two switch declarations and their two call sites only, so package CE rebases cleanly).

Nothing outside the package list was touched. The measurement trace (`-carttrace`, in `D3D9RenderTarget.cpp` and
`SceneRenderTargets.cpp`) was removed before the final build.
