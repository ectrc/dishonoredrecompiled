# Agent AH report: global shaders and the scene renderer on the retail cooked caches (Phase 3 wave 4, 2026-09-26)

Build dir `build\agentAH` (Ninja, Debug, x86, every module option). Built from the snapshot worktree `build\agentAH_wt`
(= HEAD 571bd0e + my 12 files, `resources\tools\make_snapshot.py AH`, list in `build\agentAH\snapshot_files.txt`,
`build\agentAH_wt_build.cmd`) because AD's in-flight `UnPath.h` (`OwningPylon`) and AJ's
`UDishonoredNativeStateMachine.h` (`FDisNativeStateParam`) edits stopped the shared tree from compiling. IDA copies
`resources\docs\idb\retail2013_agentAH.i64` / `shipping2012_agentAH.i64`, headless `decompile_funcs.py` only; decompiles
in `build\agentAH\decomp13` (2013, the target) and `build\agentAH\decomp12` (readable twins). Helper scripts:
`build\agentAH\syms.py` (2012 symbol -> 2013 match), `vt.py` (2012 vtable slots -> 2013), `patch1..5*.py` (the edits, as
exact-substring patches on the CRLF sources). No commits, nothing staged. Status rows:
`resources\docs\agents\agentAH_status.csv` (47 rows).

## Result

| Step | State |
|---|---|
| 1. The global types the scene renderer binds converge on the cooked cache | **done.** `global shader cache: 263 shaders, 127 loaded, 136 undeclared types, 0 parameter mismatches, 0 other skips` (wave 3: 62 loaded, 143 undeclared, **58 mismatches**). Every cooked global record whose type is not Arkane's or GFx's now loads |
| 2. The reference-only passes guarded | **done.** `global shader map: 133 types without a cooked shader` (was 144): the 11 reference-only shadow-projection / light-shaft instances are gone, and the passes binding the other 133 (uber post-process, temporal AA, SSAO, reference FXAA/MLAA, height fog) can no longer be reached (`DISHONORED(retail)` / `DISHONORED(bringup)` gates in `RenderDPGEnd`, `RenderFinish`, `RenderPostProcessEffects`) |
| 3. The null-RHI skip removed, the scene renderer runs | **done.** `RenderViewFamily_RenderThread` renders every frame. `FSceneRenderer::UpdateDownsampledDepthSurface` no longer asserts: `FDownsampleSceneDepthPixelShader` and every other type the frame binds is found, no `Failed to find shader type` anywhere in the log |
| **Accept (null RHI)** | **met:** exit 0, both `--expect` lines ok (commands and numbers under "Runs") |
| 4. D3D9 world frame | **not reachable from my side; it is AG's precondition.** Measured: the d3d9 run loads the same 127 global shaders with 0 mismatches and then aborts in `FMaterial::InitShaderMap` (`Failed to find shader map for default material LevelColorationLitMaterial`) before the device's InitRHI pass. The joint world-frame check with AG is the merge step |
| 30 s of frames | **blocked by AD, not by the renderer.** The best run rendered **>210 scene frames** (the counter logs every 30th: 0, 30 ... 210, `smoke_null3`) with no renderer assert or warning, then ended at AD's `appError called: Bad export index 1065353215/6389 ... serializing NavigationMeshBase Dishonored_MainMenu_Env.TheWorld...` - the wave-4 baseline blocker. 30 s of frames needs AD's fix, nothing from the renderer |

## What the cooked cache told me (method)

A retail global shader type is known only through its cooked record (`renderer.md` 1-3: the serialization-history word
count) plus the 2013 decompiles of its constructor, `Serialize` and `SetParameters`. The constructor is the layout key:
each `FShaderParameter` default ctor zeroes `NumBytes` at +4 and each `FShaderResourceParameter` zeroes `NumResources` at
+2, so a constructor's list of 2-byte zero stores gives the parameter kinds *and* their offsets in the object. Two shared
structs then explained most of the 58 layout differences at once:

| Struct | Retail (2013) | Reference here | Types affected |
|---|---|---|---|
| `FSceneTextureShaderParameters` | 5 parameters, 30 bytes: SceneColorTexture, SceneDepthTexture, SceneDepthCalc (`MinZ_MaxZRatio`), ScreenPositionScaleBias, NvStereoFixTexture (`operator<<` 0x447e20, `Set` 0x4550b0 / 0x455370) | 8 (+ SceneDepthSurface, ScreenAndTexelSize, bDecompressSceneColor) | `FDownsampleSceneDepthPixelShader`, `TShadowProjectionPixelShader<*>` (5), `TModShadowProjectionPixelShader<*,*>` (15), `FBranchingPCFProjectionPixelShader<*>` (9) + `TBranchingPCFModProjectionPixelShader<*,*>` (27), `TDownsampleLightShaftsPixelShader<*>`, `FApplyLightShaftsPixelShader`, `FDepthDependentHaloApplyPixelShader`, `FShaderComplexityApplyPixelShader` - and every `FMaterialPixelShaderParameters` (agent AG) |
| `FLightShaftPixelShaderParameters` | 10 parameters, 60 bytes; no spot direction / spot angles (`operator<<` 0x415690, `SetParameters` 0x420300) | 12 | `TDownsampleLightShaftsPixelShader<*>`, `FBlurLightShaftsPixelShader`, `FApplyLightShaftsPixelShader` |

## What changed (my files; every edit tagged `DISHONORED(...)` with the 2013 rva)

| File | Change | Evidence |
|---|---|---|
| `Engine/Inc/ShaderManager.h`, `Engine/Src/SceneRenderTargets.cpp` | `FSceneTextureShaderParameters` to the retail 5-parameter layout: `Bind`, `SetCustom` (scene color, depth texture when supported, stereo fix texture, `RHISetViewPixelParameters` without ScreenAndTexelSize), `operator<<`. `SceneDepthSurfaceParameter` stays declared (never bound, never serialized) only so AG's `MaterialShader.cpp:1002` keeps compiling | 2013 rva 0x447e20, 0x4550b0, 0x455370 (2012 0x46b260 / 0x479900, identical); the ctor zero-store lists of every type in the table above |
| `Engine/Src/SceneRendering.cpp` | the wave-3 null-RHI skip in `RenderViewFamily_RenderThread` removed, with `DISHONORED(bringup): scene rendered (N so far, rhi, views, WxH)` at the first render and every 30th. `RenderDPGEnd`: the reference image-reflection, subsurface-scattering, lighting-only post-process and velocity passes removed; the reference height-fog pass gated off until DisFog lands (`-referencefog` re-enables it). `RenderFinish`: no `RenderTemporalAA`, no reference `FPostProcessAA` pass. `RenderPostProcessEffects`: runs only at `SDPG_PostProcess`, and reference effect proxies are skipped with one warning (retail renders the FArkPp graph there). The `bScreenCaptureRenderTarget` shim read replaced by FALSE | `FSceneRenderer::Render` 0x46c060 (2012 0x494b00), `RenderDPGEnd` 0x464290 (2012 0x48cf90: lights, mod shadows, decals, soft-masked base, occlusion, **RenderBloomParts**, fog, distortion, resolve, translucency, radial blur, light shafts, post-process), `RenderFinish` 0x45e5e0 (2012 0x4872d0), `RenderPostProcessEffects` 0x448990 (2012 0x46bef0: `m_PostProcessProxy->Render` at DPG 4 only), `RenderDPGBegin` 0x46bf00 |
| `Engine/Src/ScenePostProcessing.cpp` | the `bScreenCaptureRenderTarget` term of the final-copy test removed | retail `FSceneViewFamily` is 80 bytes without it (`Scene.h` layout note, agent V/AB) |
| `Engine/Src/ShadowRendering.h` | `FShadowProjectionVertexShader` / `FModShadowProjectionVertexShader`: no parameter at all. New retail policies `F4SampleManualPCF`, `F16SampleManualPCF` (the reference `*PerPixel` / `*PerFragment` stay only for AG's light shader types). `TShadowProjectionPixelShader<*>` rebuilt to the retail layout (SampleOffsets, scene textures, ScreenToShadowMatrix, ShadowDepthTexture, SampleOffsets, ShadowBufferSize, ShadowFadeFraction) with `SetParameters` and `Serialize` ported; the reference `FShadowProjectionShaderParameters` (deferred G-buffer parameters, HomShadowStartPos, ShadowTexelSize) and the lighting-channel mask are gone. `TModShadowProjectionPixelShader<*,*>`: two more parameters after ScreenToWorld. `GetModProjPixelShaderRef` uses the retail policy names | cooked: the vertex shaders carry 0 parameter words, `TShadowProjectionPixelShader` 30. 2013 ctor 0xe34b0, `Serialize` 0xe3610 / 0xe3800, `SetParameters` 0xed400 (2012 0xedb30); mod `Serialize` 0xe7de0 / 0x134440 / 0x162950; mod `SetParameters` 2012 0xf17e0 (= BPCF 2013 0xf2010); `GetModProjPixelShaderRef<FPointLightPolicy>` 0x1451e0 |
| `Engine/Src/ShadowRendering.cpp` | the vertex shader ctor / `SetParameters` / `Serialize` without the parameter; the five retail `TShadowProjectionPixelShader<*>` instances (`F4SampleHwPCF`, `F4SampleManualPCF`, `F16SampleHwPCF`, `F16SampleFetch4PCF`, `F16SampleManualPCF`); `GetProjPixelShaderRef(BYTE)` ported without the per-fragment argument (the SM5 per-fragment block, unreachable on D3D9, calls it too) | 2013 rva 0x45cc20 (2012 0x485ab0) |
| `Engine/Src/BranchingPCFShadowRendering.h` | `FBranchingPCFModProjectionPixelShader<*>`: two more parameters (retail has four own parameters at +252..+270, the light parameters at +276). `FBranchingPCFProjectionPixelShader<*>` itself was already retail-shaped (13 parameters, same order) once the scene-texture struct is | ctor 0xb39f0, `Serialize` 0xb3db0, mod `Serialize` 0x1383c0 / 0x162fd0 / 0xed7e0, `SetParameters` 0xf2010 |
| `Engine/Src/LightShaftRendering.cpp` | `FLightShaftPixelShaderParameters`: the 10 retail parameters (SpotAngles / WorldSpaceSpotDirection and the `SetSpotLightShaftParameters` call removed). `FDownsampleLightShaftsVertexShader`: ScreenToWorld only (the 2012 multi-viewport `ScreenToViewport` addition removed). `TDownsampleLightShaftsPixelShader<UBOOL bPointLightShafts>` with the retail instances `<TRUE>` / `<FALSE>`; `RenderLightShafts` picks `<FALSE>` for directional lights and `<TRUE>` for the rest | `operator<<` 0x415690, `SetParameters` 0x420300, VS `SetParameters` 0x415460, PS ctor 0x416c70 / `Serialize` 0x416d00 / `SetParameters` 0x421930, `RenderLightShafts` 0x437480 (2012 0x459dc0: `LightType == 2 \|\| 3` -> `<0>`), blur / apply ctors 0x415620 / 0x4158e0 |
| `Engine/Src/PostProcessAA.cpp` | `FMLAAVertexShader` and `FFXAAVertexShader` carry the two Arkane parameters (the cooked types are Arkane's from `arkppnodeaa.cpp`: (1/w, 1/h) at +108 and the source-rectangle scale/bias at +114); the names are guesses, tagged `DISHONORED(layout)` | `SetParameters` 0x50e170 / 0x50e090, `Serialize` 0x411fe0 (folded with `TDisFogVertexShader<FDisFogPolicy<3,3>>::Serialize`) |
| `Engine/Src/LightRendering.h` (AG's file, 2 lines - hand-over 1) | the two `IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE` lines name the retail policies `F4SampleManualPCF` / `F16SampleManualPCF`, which declares the six cooked `TModShadowProjectionPixelShader<*,*ManualPCF>` types and drops six reference-only ones | 0x1451e0, 0x45cc20; without it the link fails with 6 unresolved `StaticType` symbols |
| `Engine/Src/RHI.cpp` | `GMobileTiledRenderer = FALSE` on PC | agent Y follow-up 6 (an extra `ClearAll` per frame); retail has no such flag |
| `Engine/Src/DynamicRHI.cpp` | `RHIInit` creates the D3D9 RHI directly; the reference block (`bForceD3D11 = TRUE`, a UDK leftover, `IsDirect3D11Supported`, the "Command line -d3d11 set" warning and the OpenGL selection) removed | retail has one PC RHI; device creation 2013 rva 0x5bc1e0 |

Already retail once the two shared structs are, so left alone (verified against the decompiles, rows in the status CSV):
`FDownsampleSceneDepthPixelShader` (0x448ad0 / 0x448b20), `FDepthDependentHaloApplyPixelShader` (0xe01a0 / 0xe01f0 /
0xe02a0), `FShaderComplexityApplyPixelShader` (0x162010 / 0x154440), `FBlurLightShaftsPixelShader` /
`FApplyLightShaftsPixelShader` (0x415710 / 0x415970), `FApplyLightShaftsVertexShader` (0x415750), the nine
`FBranchingPCFProjectionPixelShader<*>` quality types (`LowQualityShaderName` ...), and the three
`ModShadowPixelParamsType` classes (directional 0, point 2, spot 4 parameters: 0x1571a0 / 0x14d3f0 / 0x133230).

## Runs

Build (snapshot, HEAD 571bd0e + my 12 files): `build\agentAH_wt_build.cmd` -> `build\agentAH_build4.log`, `_build5.log`
and `_build6.log` (the last a full 567-unit Engine rebuild after `ShaderManager.h` / `SceneRendering.cpp` were normalized
back to CRLF), all exit 0, 65 MB exe. The accept was then re-run on those exact bytes:
`build\agentAH\smoke_null3.txt`, `SMOKE_EXIT=0`, both expects ok.

Accept (null RHI), `build\agentAH\smoke_null2.txt`:

```
python resources/tools/build_and_smoke.py --build-dir build/agentAH --no-build --exe-name DishonoredGame_AH.exe ^
  --log-name agentAH.log --ini-dir build/agentAH/config --rhi null --timeout 120 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --expect "DISHONORED(bringup): scene rendered" --skip-native OnlineSubsystemPC
-> expect ok: Initial startup
   expect ok: DISHONORED(bringup): scene rendered
   EXIT=0
```

From that run's log (`Dishonored_Latest2026\DishonoredGame\Logs\agentAH.log`):

```
[0004.75] DISHONORED(bringup): global shader cache: 263 shaders, 127 loaded, 136 undeclared types, 0 parameter mismatches, 0 other skips
[0004.76] DISHONORED(bringup): global shader map: 133 types without a cooked shader
[0011.42] DISHONORED(bringup): scene rendered (0 so far, null RHI, 1 views, 1920x1080)
[0014.91] DISHONORED(bringup): scene rendered (30 so far, null RHI, 1 views, 1920x1080)
...       (every 30th; the final run of the same build reached "210 so far" by 5.04 s)
[0015.xx] Critical: appError called: Bad export index 1065353215/6389 ... NavigationMeshBase   <- agent AD's blocker
```

How the inventory moved against agent Y's wave-3 numbers (`renderer.md` 2):

| Line | Wave 3 (agent Y) | Now | Why |
|---|---:|---:|---|
| cooked global records | 263 | 263 | unchanged |
| loaded | 62 | **127** | +58 layout mismatches fixed, +10 cooked types newly declared, -3 Bink types (this build dir has `DISHONORED_WITH_BINK=OFF`; they load in a Bink build, agent Y used `-DDISHONORED_WITH_BINK=ON`) |
| undeclared types | 143 | **136** | -10 newly declared (6 `TModShadowProjectionPixelShader<*,*ManualPCF>`, 2 `TShadowProjectionPixelShader<*ManualPCF>`, 2 `TDownsampleLightShaftsPixelShader<TRUE/FALSE>`), +3 Bink (build flag only) |
| parameter mismatches | 58 | **0** | the layout work of this package |
| declared types with no cooked shader | 144 | **133** | the 11 reference-only projection / light-shaft instances removed |

The 136 still-undeclared records are exactly the Arkane and GFx families that wave 5 owns (`renderer.md` 4): FGFx 50,
FDisFog 30, `TFilterPixelShaderDepthInAlpha<1..16>` 16, FArkPp 16, FMLAA 4, FKuwa 4, FBloom 4, FFXAA 3, TMeshPaint 2,
TBloom 2, FHeightFog 2 - plus the 3 FBink ones this build dir switches off. **No cooked record whose type this build
declares is rejected any more.**

d3d9 (`build\agentAH\smoke_d3d9.txt`, log `agentAH_d3d9.log`): run and measured, stops where agent Y left it.

```
python resources/tools/build_and_smoke.py --build-dir build/agentAH --no-build --exe-name DishonoredGame_AH.exe ^
  --log-name agentAH_d3d9.log --ini-dir build/agentAH/config --rhi d3d9 --timeout 120 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --skip-native OnlineSubsystemPC --extra-args "-windowed -ResX=1280 -ResY=720 -nomovie"
-> Log: Shader platform (RHI): PC-D3D-SM3
   Log: DISHONORED(bringup): global shader cache: 263 shaders, 127 loaded, 136 undeclared types, 0 parameter mismatches, 0 other skips
   Critical: Failed to find shader map for default material LevelColorationLitMaterial!  Please make sure cooking was successful.
   EXIT=1 (expect MISSING: Initial startup)
```

The global cache is identical to the null-RHI run (127 loaded, **0 mismatches**), then `FMaterial::InitShaderMap` aborts
on the first default material without a shader map, before the device's InitRHI pass - exactly agent AG's package
(`PHASE6.md` AG; retail aborts there too). Hence no `-dumpframes` shots from me: the world frame needs AG's material
shader maps, and the joint check is the merge step. Everything the scene renderer itself needs is in place - the same
frame runs to completion, repeatedly, under the null RHI.

## Hand-overs

1. **Agent AG, `Engine/Src/LightRendering.h` (2 lines + a comment, applied in my tree).** The two
   `IMPLEMENT_LIGHT_UNIFORMPCF_SHADER_TYPE` lines of `IMPLEMENT_LIGHT_SHADER_TYPE` now name the retail uniform-PCF
   policies `F4SampleManualPCF` / `F16SampleManualPCF` instead of the reference `*PerPixel` ones. The light policies are
   local classes of `DirectionalLightComponent.cpp` / `PointLightComponent.cpp` / `SpotLightComponent.cpp`, so that macro
   is the only place the six cooked `TModShadowProjectionPixelShader<*,*ManualPCF>` types can be instantiated; without it
   the link fails with 6 unresolved `StaticType` symbols. The rest of that header is untouched. Tagged
   `DISHONORED(retail)` with 2013 rva 0x1451e0 / 0x45cc20.
2. **Agent AG, `Engine/Src/MaterialShader.cpp:1002`** unbinds `FSceneTextureShaderParameters::SceneDepthSurfaceParameter`.
   Retail has no such parameter; I kept the member declared (never bound, never serialized) only to keep that line
   compiling. Drop the line and the member goes with it.
3. **Agent AG (information).** `FSceneTextureShaderParameters` is now the retail 5-parameter struct (15 history words)
   and it is embedded in every `FMaterialPixelShaderParameters` - one reason all 49 declared mesh-material types
   mismatched. Expect the material-side history counts to drop by 9 words per embedded instance.
4. **Coordinator / wave 5 (`renderer.md` 5 updated).** Still undeclared and un-ported, with their passes gated off by a
   named `DISHONORED(bringup)` gate rather than reached: the FArkPp post-process node graph (`FArkPp*` 16, `FKuwa*` 4,
   Arkane `FBloom*`/`TBloom*` 6, Arkane `FFXAA*` 3 / `FMLAA*` 4), DisFog (`FDisFog*` 30 + `FHeightFogMask*` 2, with
   `FViewInfo::DisPrecomputedFogs`), `TFilterPixelShaderDepthInAlpha<1..16>`, `TMeshPaint*` 2 and the 50 GFx shaders
   (Scaleform decision). `FSceneRenderer::RenderBloomParts` (2012 rva 0x566120 / 2013 0x5251a0) sits between the
   soft-masked base pass and the fog pass in retail's `RenderDPGEnd` and is not ported either.
5. **Agent AD (blocker).** Every run of mine ends at `Bad export index 1065353215/6389` in `Dishonored_MainMenu_Env.upk`
   `NavigationMeshBase`, ~3.5 s after the first scene render. Until that lands nobody can show 30 s of frames.
6. **Mine, left open.** `FSceneViewFamily::CurrentBendTime` is still the first member with no caller passing it (agent V's
   layout note); I only removed the two `bScreenCaptureRenderTarget` shim reads. The two extra retail parameters of
   `TModShadowProjectionPixelShader` / `FBranchingPCFModProjectionPixelShader` (at +212/+218 and +264/+270) are in the
   layout and in `Serialize`, but their names are guesses (`EmissiveAlphaMaskScaleParam` / `UseEmissiveMaskParam`, tagged
   `DISHONORED(layout)`); retail's `SetParameters` never writes them, so nothing depends on the names. The
   `FMLAAVertexShader` / `FFXAAVertexShader` parameter names are guesses for the same reason.
7. The bring-up lines (`scene rendered`, the cache inventory, the post-process warning) are aids: drop them when the
   FArkPp graph lands.
