# Renderer convergence work list (renderer.md)

Written by agent Y (Phase 3 wave 3, 2026-09-25). Two inventories: (1) the cooked global shader cache
`DishonoredGame\CookedPCConsole\GlobalShaderCache-PC-D3D-SM3.bin` (419,617 bytes) against the global shader types this
build declares, and (2) the Arkane members of `FSceneViewFamily` / `FSceneView` / `FViewInfo` / `FSceneRenderer` (2012 PDB)
against ours. Retail has **no shader compiler** (agentO.md step 5): every shader the game draws with comes from the cooked
caches, so a type whose C++ declaration or parameter layout differs from Arkane's simply has no shader here.

## 1. How the inventory is made

* **File format** (`build\agentY\parse_gsc.py`, output `build\agentY\gsc.json`): tag `GSMB`, package version 801, licensee
  30, platform 0 (`SP_PCD3D_SM3`), an empty compressed-code map, then 263 records. Each record = type name (FName as
  FString), shader Id (FGuid), source hash (SHA, 20), skip offset, the **serialization history** (`TArray<WORD>`, one entry
  per `FArchive::Serialize` call of the type's `Serialize`) and the shader data. After the records the global shader map
  (263 entries: type name, Id, type name). One shader per type.
* **FShader::Serialize** is the reference field order (2013 rva 0x1603b0): history `1,1,4,<code>,4,4,4,4,4,20,4` = platform,
  frequency, code count, code bytes, ParameterMapCRC, Id (4 dwords), hash, NumInstructions. Everything else in a history is the
  type's own parameters: 3 words per `FShaderParameter` (BaseIndex, NumBytes, BufferIndex) and per
  `FShaderResourceParameter` (BaseIndex, NumResources, SamplerIndex). Some Arkane types write their parameters *before*
  `FShader::Serialize` (the Bink pixel shaders).
* **Runtime report** (agent Y's build, `DISHONORED(bringup)` lines, no switch needed): `ShaderCache.cpp` names every cooked
  record whose type is not declared and every declared type whose cooked layout differs (the `FShaderLoadArchive` history
  check), then prints `global shader cache: N shaders, L loaded, U undeclared types, M parameter mismatches`;
  `GlobalShader.cpp` lists every declared global type the loaded map has no shader for; after device creation
  `D3D9Device.cpp` prints `device InitRHI pass: N shader objects created, M rejected` (per-shader failures are named by
  `FShader::InitRHI` with type, frequency, bytecode size and the HRESULT from `D3D9Shaders.cpp`).
* **Retail version gates** (fixed this wave in `ShaderManager.h`): every retail global `FShaderType` is constructed with
  `MinPackageVersion 786`, `MinLicenseePackageVersion 1` (2013 rva 0xb6fac0 `FOneColorVertexShader`, 0xb722a0
  `FBinkVertexShader`, 0xb7f2c0 `FSimpleElementPixelShader`: `push 312h` / `push 1`). The reference `VER_MIN_SHADER`
  (`VER_INVALIDATE_SHADERCACHE5` = 836) is newer than the cooked caches and had rejected all 263 records.
* **Material shader caches** (`RefShaderCache-PC-D3D-SM3.upk`, `UShaderCache` in packages): retail skips a material shader map
  below version 786 / licensee 23 (2013 rva 0x164a60) and constructs every material shader type with 798 / 23 (2013 rva
  0xb7ea00 `TBasePassVertexShader<FNoLightMapPolicy>`, 0xb7ea40 `TBasePassPixelShader<FNoLightMapPolicy,0,0>`: `push 31Eh` /
  `push 17h`). **Not applied yet**: with those gates the material shader maps are parsed, every declared mesh-material
  type's layout differs from Arkane's and `FMaterialShaderMap::Serialize` then stops in `FUniformExpressionSet::Serialize`
  (section 5, item 2). The reference gates (836) keep skipping every cooked material shader map, so materials fall back to the
  default material as before, and a D3D9 run aborts in `FMaterial::InitShaderMap` on the first default material without a
  map (retail does the same; the agent Y test runs pass a local `-agentYnomatshaders` bypass, never committed).
* **Shader frequency** (fixed this wave in `ShaderCompiler.h`): Dishonored's `EShaderFrequency` is `SF_Vertex = 0,
  SF_Pixel = 1` (2012 PDB; no D3D11 stages). The reference numbering (`SF_Hull = 1`, `SF_Pixel = 3`) made the D3D9 device's
  InitRHI pass create every cooked pixel shader as a hull shader (`D3D9 Render path does not support Hull shaders!`).
* **Robust skip** (`ShaderCache.cpp`, `FShaderLoadArchive`): a record whose layout differs before its first sized field
  (it reads a name or an object first) is detected by validating the name/object index against the linker tables and is
  skipped like any other mismatch, instead of aborting on a garbage index.

## 2. Families of the cooked global shader cache

| Family | What | Records | Loads | Layout differs | Undeclared | Work |
|---|---|---:|---:|---:|---:|---|
| FGFx | Scaleform GFx 3.3 render shaders (GFxUI, `WITH_GFx=0` here) | 50 | 0 | 0 | 50 | Scaleform decision (middleware.md); declared with the GFxUI port |
| FDisFog | Arkane layered height fog (`FDisFogPolicy<N,M>`, 0..4 layers) | 30 | 0 | 0 | 30 | port with `FDisFogSceneInfo` / `FDisPrecomputedFogSceneInfo` (FViewInfo) and the DisFog render pass |
| FArkPp | Arkane post-process graph (DOF uber/LUT/downsample, motion blur, radial and box blur) | 16 | 0 | 0 | 16 | port with `FArkPpNode*Proxy` (UPostProcessChain m_GraphRoot/m_AllNodes, FSceneView::m_PostProcessProxy) |
| FKuwa | Kuwahara painterly filter (FArkPpNodeKuwaProxy) | 4 | 0 | 0 | 4 | with the FArkPp graph |
| FBloom | Arkane bloom (down-sample/compose) | 4 | 0 | 0 | 4 | with the FArkPp graph / FArkBloomPartPrimSet |
| TBloom | Arkane bloom blur taps | 2 | 0 | 0 | 2 | with FBloom |
| FFXAA | Arkane FXAA 3 variants (FArkPpNodeAAProxy::RenderFxaa) | 4 | 0 | 1 | 3 | Arkane FXAA types; the reference FFXAABlendPixelShader0..5 are reference-only |
| FMLAA | Arkane MLAA (FArkPpNodeAAProxy::RenderMlaa*) | 6 | 1 | 1 | 4 | Arkane MLAA types; the reference FSRGBMLAA* are reference-only |
| TFilterPixelShaderDepthInAlpha | filter shaders writing depth to alpha (1..16 samples) | 16 | 0 | 0 | 16 | declare next to TFilterPixelShader (same parameters + Arkane depth path) |
| TFilter | reference separable filter | 32 | 32 | 0 | 0 | loads |
| TMeshPaint | mesh paint (editor tool, cooked anyway) | 2 | 0 | 0 | 2 | declare as reference MeshPaint shaders (editor-only use) |
| FHeightFog | height fog mask (Arkane) | 2 | 0 | 0 | 2 | with DisFog |
| TDownsampleLightShafts | light shafts, retail uses <TRUE>/<FALSE> template argument | 2 | 0 | 0 | 2 | reference uses LS_Directional/LS_Spot/LS_Point: re-template |
| TModShadowProjection | modulated shadow projection | 15 | 0 | 9 | 6 | Arkane parameter set (mismatch) + retail-only ManualPCF variants |
| TBranchingPCF | branching PCF mod-shadow projection | 27 | 0 | 27 | 0 | Arkane parameter set (mismatch) |
| TShadowProjection | shadow projection | 5 | 0 | 3 | 2 | Arkane parameter set (mismatch) + ManualPCF variants |
| FSimpleElement | canvas / batched elements | 6 | 6 | 0 | 0 | loads (first frame) |
| FBink | Bink YCrCb to RGB | 3 | 3 | 0 | 0 | loads (FBinkYCrCbAToRGBAPixelShader::Serialize fixed to retail) |
| FFluid | fluid surface simulation | 4 | 4 | 0 | 0 | loads |
| other | reference-named global shaders (OneColor, Screen, Restore*, light shafts, halo, depth down-sample, ...) | 33 | 16 | 17 | 0 | per type: see section 3 |
| **total** | | **263** | **62** | **58** | **143** | |

## 3. Every cooked global shader

`Params` = serialization-history words of the type's own parameters (each `FShaderParameter` and each
`FShaderResourceParameter` is 3 words: BaseIndex, NumBytes/NumResources, BufferIndex/SamplerIndex), `order` = whether the
cooked Serialize writes them before or after `FShader::Serialize`. `Code` = D3D9 bytecode bytes.

| Type | Family | Code | Params (words, order) | This build |
|---|---|---:|---|---|
| `FArkPpBlurBoxBlurPixelShader` | FArkPp | 136 | 9, after | undeclared |
| `FArkPpBlurBoxBlurVertexShader` | FArkPp | 356 | 9, after | undeclared |
| `FArkPpDofDownsamplePS` | FArkPp | 236 | 3, after | undeclared |
| `FArkPpDofDownsampleVS` | FArkPp | 332 | 3, after | undeclared |
| `FArkPpDofLutBlenderPS` | FArkPp | 1688 | 18, after | undeclared |
| `FArkPpDofLutBlenderVS` | FArkPp | 200 | 0, - | undeclared |
| `FArkPpDofUberVS` | FArkPp | 580 | 9, after | undeclared |
| `FArkPpDofUber_01PS` | FArkPp | 840 | 21, after | undeclared |
| `FArkPpDofUber_10PS` | FArkPp | 452 | 21, after | undeclared |
| `FArkPpDofUber_11PS` | FArkPp | 1032 | 21, after | undeclared |
| `FArkPpMotionBlur2NoOffsPixelShader` | FArkPp | 520 | 9, after | undeclared |
| `FArkPpMotionBlur2PixelShader` | FArkPp | 540 | 9, after | undeclared |
| `FArkPpMotionBlurPixelShader` | FArkPp | 560 | 9, after | undeclared |
| `FArkPpMotionBlurVertexShader` | FArkPp | 356 | 9, after | undeclared |
| `FArkPpRadialBlurPixelShader` | FArkPp | 400 | 9, after | undeclared |
| `FArkPpRadialBlurVertexShader` | FArkPp | 608 | 9, after | undeclared |
| `FBinkVertexShader` | FBink | 172 | 0, - | loads |
| `FBinkYCrCbAToRGBAPixelShader` | FBink | 616 | 3, before | loads |
| `FBinkYCrCbToRGBNoPixelAlphaPixelShader` | FBink | 540 | 21, before | loads |
| `FBloomComposePixelShader` | FBloom | 328 | 18, after | undeclared |
| `FBloomComposeVertexShader` | FBloom | 224 | 0, - | undeclared |
| `FBloomDownSamplePixelShader` | FBloom | 1208 | 6, after | undeclared |
| `FBloomDownSampleVertexShader` | FBloom | 300 | 0, - | undeclared |
| `FDisFogPixelShader00Layer` | FDisFog | 324 | 60, after | undeclared |
| `FDisFogPixelShader10Layer` | FDisFog | 1132 | 60, after | undeclared |
| `FDisFogPixelShader11Layer` | FDisFog | 1220 | 60, after | undeclared |
| `FDisFogPixelShader20Layer` | FDisFog | 1216 | 60, after | undeclared |
| `FDisFogPixelShader21Layer` | FDisFog | 1316 | 60, after | undeclared |
| `FDisFogPixelShader22Layer` | FDisFog | 1392 | 60, after | undeclared |
| `FDisFogPixelShader30Layer` | FDisFog | 1268 | 60, after | undeclared |
| `FDisFogPixelShader31Layer` | FDisFog | 1380 | 60, after | undeclared |
| `FDisFogPixelShader32Layer` | FDisFog | 1468 | 60, after | undeclared |
| `FDisFogPixelShader33Layer` | FDisFog | 1544 | 60, after | undeclared |
| `FDisFogPixelShader40Layer` | FDisFog | 1288 | 60, after | undeclared |
| `FDisFogPixelShader41Layer` | FDisFog | 1408 | 60, after | undeclared |
| `FDisFogPixelShader42Layer` | FDisFog | 1496 | 60, after | undeclared |
| `FDisFogPixelShader43Layer` | FDisFog | 1604 | 60, after | undeclared |
| `FDisFogPixelShader44Layer` | FDisFog | 1680 | 60, after | undeclared |
| `FDisFogVertexShader00Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader10Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader11Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader20Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader21Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader22Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader30Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader31Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader32Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader33Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader40Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader41Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader42Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader43Layer` | FDisFog | 376 | 6, after | undeclared |
| `FDisFogVertexShader44Layer` | FDisFog | 376 | 6, after | undeclared |
| `FFXAAPixelShader_ComputeLuma_SRGBColor` | FFXAA | 1492 | 33, after | undeclared |
| `FFXAAPixelShader_LumaAsGreen_SRGBColor` | FFXAA | 1356 | 33, after | undeclared |
| `FFXAAPixelShader_LumaInAlpha_SRGBColor` | FFXAA | 1356 | 33, after | undeclared |
| `FFXAAVertexShader` | FFXAA | 412 | 6, after | layout differs |
| `FFluidApplyPixelShader` | FFluid | 192 | 6, after | loads |
| `FFluidNormalPixelShader` | FFluid | 864 | 12, after | loads |
| `FFluidSimulatePixelShader` | FFluid | 808 | 21, after | loads |
| `FFluidVertexShader` | FFluid | 172 | 0, - | loads |
| `FGFxFilterPixelShader<FS2_FBox1Blur>` | FGFx | 716 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox1BlurMul>` | FGFx | 776 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2Blur>` | FGFx | 792 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2BlurMul>` | FGFx | 852 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadow>` | FGFx | 1020 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowHighlight>` | FGFx | 1240 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowHighlightKnockout>` | FGFx | 1240 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowKnockout>` | FGFx | 1032 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowMul>` | FGFx | 1080 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowMulHighlight>` | FGFx | 1260 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowMulHighlightKnockout>` | FGFx | 1260 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2InnerShadowMulKnockout>` | FGFx | 1092 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2Shadow>` | FGFx | 1024 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowHighlight>` | FGFx | 1136 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowHighlightKnockout>` | FGFx | 1152 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowKnockout>` | FGFx | 1040 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowMul>` | FGFx | 1084 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowMulHighlight>` | FGFx | 1196 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowMulHighlightKnockout>` | FGFx | 1212 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowMulKnockout>` | FGFx | 1100 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2Shadowonly>` | FGFx | 868 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowonlyHighlight>` | FGFx | 868 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowonlyMul>` | FGFx | 928 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FBox2ShadowonlyMulHighlight>` | FGFx | 928 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FCMatrix>` | FGFx | 572 | 36, after | undeclared |
| `FGFxFilterPixelShader<FS2_FCMatrixMul>` | FGFx | 608 | 36, after | undeclared |
| `FGFxPixelShader<GFx_PS_Cxform2Texture>` | FGFx | 628 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraud>` | FGFx | 440 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraudMultiply>` | FGFx | 488 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraudMultiplyNoAddAlpha>` | FGFx | 460 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraudMultiplyTexture>` | FGFx | 628 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraudNoAddAlpha>` | FGFx | 424 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformGouraudTexture>` | FGFx | 580 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformMultiply2Texture>` | FGFx | 664 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformTexture>` | FGFx | 504 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_CxformTextureMultiply>` | FGFx | 540 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_SolidColor>` | FGFx | 164 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTexture>` | FGFx | 544 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTextureColor>` | FGFx | 532 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTextureColorMultiply>` | FGFx | 540 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTextureDFA>` | FGFx | 1132 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTextureSRGB>` | FGFx | 480 | 48, after | undeclared |
| `FGFxPixelShader<GFx_PS_TextTextureSRGBMultiply>` | FGFx | 488 | 48, after | undeclared |
| `FGFxVertexShader<GFx_VS_Glyph>` | FGFx | 320 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_Strip>` | FGFx | 372 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_XY16iC32>` | FGFx | 408 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_XY16iCF32>` | FGFx | 444 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_XY16iCF32_NoTex>` | FGFx | 320 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_XY16iCF32_NoTexNoAlpha>` | FGFx | 284 | 9, after | undeclared |
| `FGFxVertexShader<GFx_VS_XY16iCF32_T2>` | FGFx | 568 | 9, after | undeclared |
| `FHeightFogMaskPixelShader< 0 >` | FHeightFog | 164 | 3, after | undeclared |
| `FHeightFogMaskVertexShader` | FHeightFog | 360 | 3, after | undeclared |
| `FKuwaPixelShader3` | FKuwa | 1172 | 3, after | undeclared |
| `FKuwaPixelShader5` | FKuwa | 1504 | 3, after | undeclared |
| `FKuwaVertexShader3` | FKuwa | 480 | 6, after | undeclared |
| `FKuwaVertexShader5` | FKuwa | 600 | 6, after | undeclared |
| `FMLAABlend_Linear_PixelShader` | FMLAA | 6468 | 15, after | undeclared |
| `FMLAABlend_SRGB_PixelShader` | FMLAA | 5748 | 12, after | undeclared |
| `FMLAAComputeLineLengthPixelShader` | FMLAA | 6028 | 6, after | loads |
| `FMLAAEdgeDetection_Linear_PixelShader` | FMLAA | 800 | 12, after | undeclared |
| `FMLAAEdgeDetection_SRGB_PixelShader` | FMLAA | 568 | 9, after | undeclared |
| `FMLAAVertexShader` | FMLAA | 340 | 6, after | layout differs |
| `FSimpleElementDistanceFieldGammaPixelShader` | FSimpleElement | 1776 | 42, after | loads |
| `FSimpleElementGammaPixelShader` | FSimpleElement | 544 | 12, after | loads |
| `FSimpleElementHitProxyPixelShader` | FSimpleElement | 224 | 3, after | loads |
| `FSimpleElementMaskedGammaPixelShader` | FSimpleElement | 612 | 15, after | loads |
| `FSimpleElementPixelShader` | FSimpleElement | 300 | 9, after | loads |
| `FSimpleElementVertexShader` | FSimpleElement | 356 | 3, after | loads |
| `TBloomBlurPixelShader<5>` | TBloom | 408 | 3, after | undeclared |
| `TBloomBlurVertexShader<5>` | TBloom | 436 | 3, after | undeclared |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFHighQualityFetch4PCF` | TBranchingPCF | 3828 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFHighQualityHwPCF` | TBranchingPCF | 2428 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFHighQualityManualPCF` | TBranchingPCF | 3988 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFLowQualityFetch4PCF` | TBranchingPCF | 2292 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFLowQualityHwPCF` | TBranchingPCF | 1716 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFLowQualityManualPCF` | TBranchingPCF | 2348 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFMediumQualityFetch4PCF` | TBranchingPCF | 2828 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFMediumQualityHwPCF` | TBranchingPCF | 1956 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFDirectionalLightPolicyFMediumQualityManualPCF` | TBranchingPCF | 3444 | 51, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFHighQualityFetch4PCF` | TBranchingPCF | 4260 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFHighQualityHwPCF` | TBranchingPCF | 2860 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFHighQualityManualPCF` | TBranchingPCF | 4400 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFLowQualityFetch4PCF` | TBranchingPCF | 2724 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFLowQualityHwPCF` | TBranchingPCF | 2172 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFLowQualityManualPCF` | TBranchingPCF | 2780 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFMediumQualityFetch4PCF` | TBranchingPCF | 3260 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFMediumQualityHwPCF` | TBranchingPCF | 2388 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFPointLightPolicyFMediumQualityManualPCF` | TBranchingPCF | 3856 | 57, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFHighQualityFetch4PCF` | TBranchingPCF | 4440 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFHighQualityHwPCF` | TBranchingPCF | 3040 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFHighQualityManualPCF` | TBranchingPCF | 4580 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFLowQualityFetch4PCF` | TBranchingPCF | 2904 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFLowQualityHwPCF` | TBranchingPCF | 2352 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFLowQualityManualPCF` | TBranchingPCF | 2960 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFMediumQualityFetch4PCF` | TBranchingPCF | 3440 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFMediumQualityHwPCF` | TBranchingPCF | 2568 | 63, after | layout differs |
| `TBranchingPCFModProjectionPixelShaderFSpotLightPolicyFMediumQualityManualPCF` | TBranchingPCF | 4036 | 63, after | layout differs |
| `TDownsampleLightShaftsPixelShader<FALSE>` | TDownsampleLightShafts | 1412 | 51, after | undeclared |
| `TDownsampleLightShaftsPixelShader<TRUE>` | TDownsampleLightShafts | 1712 | 51, after | undeclared |
| `TFilterPixelShader<10>` | TFilter | 1308 | 9, after | loads |
| `TFilterPixelShader<11>` | TFilter | 1460 | 9, after | loads |
| `TFilterPixelShader<12>` | TFilter | 1512 | 9, after | loads |
| `TFilterPixelShader<13>` | TFilter | 1664 | 9, after | loads |
| `TFilterPixelShader<14>` | TFilter | 1716 | 9, after | loads |
| `TFilterPixelShader<15>` | TFilter | 1868 | 9, after | loads |
| `TFilterPixelShader<16>` | TFilter | 1920 | 9, after | loads |
| `TFilterPixelShader<1>` | TFilter | 440 | 9, after | loads |
| `TFilterPixelShader<2>` | TFilter | 492 | 9, after | loads |
| `TFilterPixelShader<3>` | TFilter | 644 | 9, after | loads |
| `TFilterPixelShader<4>` | TFilter | 696 | 9, after | loads |
| `TFilterPixelShader<5>` | TFilter | 848 | 9, after | loads |
| `TFilterPixelShader<6>` | TFilter | 900 | 9, after | loads |
| `TFilterPixelShader<7>` | TFilter | 1052 | 9, after | loads |
| `TFilterPixelShader<8>` | TFilter | 1104 | 9, after | loads |
| `TFilterPixelShader<9>` | TFilter | 1256 | 9, after | loads |
| `TFilterVertexShader<10>` | TFilter | 340 | 3, after | loads |
| `TFilterVertexShader<11>` | TFilter | 368 | 3, after | loads |
| `TFilterVertexShader<12>` | TFilter | 368 | 3, after | loads |
| `TFilterVertexShader<13>` | TFilter | 396 | 3, after | loads |
| `TFilterVertexShader<14>` | TFilter | 396 | 3, after | loads |
| `TFilterVertexShader<15>` | TFilter | 424 | 3, after | loads |
| `TFilterVertexShader<16>` | TFilter | 424 | 3, after | loads |
| `TFilterVertexShader<1>` | TFilter | 228 | 3, after | loads |
| `TFilterVertexShader<2>` | TFilter | 228 | 3, after | loads |
| `TFilterVertexShader<3>` | TFilter | 256 | 3, after | loads |
| `TFilterVertexShader<4>` | TFilter | 256 | 3, after | loads |
| `TFilterVertexShader<5>` | TFilter | 284 | 3, after | loads |
| `TFilterVertexShader<6>` | TFilter | 284 | 3, after | loads |
| `TFilterVertexShader<7>` | TFilter | 312 | 3, after | loads |
| `TFilterVertexShader<8>` | TFilter | 312 | 3, after | loads |
| `TFilterVertexShader<9>` | TFilter | 340 | 3, after | loads |
| `TFilterPixelShaderDepthInAlpha<10>` | TFilterPixelShaderDepthInAlpha | 668 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<11>` | TFilterPixelShaderDepthInAlpha | 716 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<12>` | TFilterPixelShaderDepthInAlpha | 752 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<13>` | TFilterPixelShaderDepthInAlpha | 800 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<14>` | TFilterPixelShaderDepthInAlpha | 836 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<15>` | TFilterPixelShaderDepthInAlpha | 884 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<16>` | TFilterPixelShaderDepthInAlpha | 920 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<1>` | TFilterPixelShaderDepthInAlpha | 296 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<2>` | TFilterPixelShaderDepthInAlpha | 332 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<3>` | TFilterPixelShaderDepthInAlpha | 380 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<4>` | TFilterPixelShaderDepthInAlpha | 756 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<5>` | TFilterPixelShaderDepthInAlpha | 464 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<6>` | TFilterPixelShaderDepthInAlpha | 500 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<7>` | TFilterPixelShaderDepthInAlpha | 548 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<8>` | TFilterPixelShaderDepthInAlpha | 584 | 24, after | undeclared |
| `TFilterPixelShaderDepthInAlpha<9>` | TFilterPixelShaderDepthInAlpha | 632 | 24, after | undeclared |
| `TMeshPaintPixelShader` | TMeshPaint | 1108 | 18, after | undeclared |
| `TMeshPaintVertexShader` | TMeshPaint | 320 | 3, after | undeclared |
| `TModShadowProjectionPixelShaderFDirectionalLightPolicyF16SampleFetch4PCF` | TModShadowProjection | 2780 | 42, after | layout differs |
| `TModShadowProjectionPixelShaderFDirectionalLightPolicyF16SampleHwPCF` | TModShadowProjection | 1660 | 42, after | layout differs |
| `TModShadowProjectionPixelShaderFDirectionalLightPolicyF16SampleManualPCF` | TModShadowProjection | 2408 | 42, after | undeclared |
| `TModShadowProjectionPixelShaderFDirectionalLightPolicyF4SampleHwPCF` | TModShadowProjection | 1012 | 42, after | layout differs |
| `TModShadowProjectionPixelShaderFDirectionalLightPolicyF4SampleManualPCF` | TModShadowProjection | 1692 | 42, after | undeclared |
| `TModShadowProjectionPixelShaderFPointLightPolicyF16SampleFetch4PCF` | TModShadowProjection | 3212 | 48, after | layout differs |
| `TModShadowProjectionPixelShaderFPointLightPolicyF16SampleHwPCF` | TModShadowProjection | 2092 | 48, after | layout differs |
| `TModShadowProjectionPixelShaderFPointLightPolicyF16SampleManualPCF` | TModShadowProjection | 2840 | 48, after | undeclared |
| `TModShadowProjectionPixelShaderFPointLightPolicyF4SampleHwPCF` | TModShadowProjection | 1444 | 48, after | layout differs |
| `TModShadowProjectionPixelShaderFPointLightPolicyF4SampleManualPCF` | TModShadowProjection | 2124 | 48, after | undeclared |
| `TModShadowProjectionPixelShaderFSpotLightPolicyF16SampleFetch4PCF` | TModShadowProjection | 3376 | 54, after | layout differs |
| `TModShadowProjectionPixelShaderFSpotLightPolicyF16SampleHwPCF` | TModShadowProjection | 2272 | 54, after | layout differs |
| `TModShadowProjectionPixelShaderFSpotLightPolicyF16SampleManualPCF` | TModShadowProjection | 3004 | 54, after | undeclared |
| `TModShadowProjectionPixelShaderFSpotLightPolicyF4SampleHwPCF` | TModShadowProjection | 1624 | 54, after | layout differs |
| `TModShadowProjectionPixelShaderFSpotLightPolicyF4SampleManualPCF` | TModShadowProjection | 2288 | 54, after | undeclared |
| `TShadowProjectionPixelShader<F16SampleFetch4PCF>` | TShadowProjection | 2784 | 30, after | layout differs |
| `TShadowProjectionPixelShader<F16SampleHwPCF>` | TShadowProjection | 1664 | 30, after | layout differs |
| `TShadowProjectionPixelShader<F16SampleManualPCF>` | TShadowProjection | 2412 | 30, after | undeclared |
| `TShadowProjectionPixelShader<F4SampleHwPCF>` | TShadowProjection | 1016 | 30, after | layout differs |
| `TShadowProjectionPixelShader<F4SampleManualPCF>` | TShadowProjection | 1696 | 30, after | undeclared |
| `FApplyForcePixelShader` | other | 496 | 12, after | loads |
| `FApplyLightShaftsPixelShader` | other | 968 | 48, after | layout differs |
| `FApplyLightShaftsVertexShader` | other | 332 | 6, after | loads |
| `FBlurLightShaftsPixelShader` | other | 980 | 33, after | layout differs |
| `FDepthDependentHaloApplyPixelShader` | other | 832 | 24, after | layout differs |
| `FDistortionApplyScreenPixelShader` | other | 584 | 9, after | loads |
| `FDistortionApplyScreenVertexShader` | other | 284 | 3, after | loads |
| `FDownsampleLightShaftsVertexShader` | other | 320 | 3, after | layout differs |
| `FDownsampleSceneDepthPixelShader` | other | 632 | 24, after | layout differs |
| `FGammaCorrectionPixelShader` | other | 556 | 12, after | loads |
| `FGammaCorrectionVertexShader` | other | 172 | 0, - | loads |
| `FLDRExtractVertexShader` | other | 172 | 0, - | loads |
| `FModShadowProjectionVertexShader` | other | 296 | 0, - | layout differs |
| `FNULLPixelShader` | other | 136 | 0, - | loads |
| `FOcclusionQueryVertexShader<0>` | other | 260 | 0, - | loads |
| `FOcclusionQueryVertexShader<NUM_CUBE_VERTICES>` | other | 260 | 0, - | loads |
| `FOneColorPixelShader` | other | 176 | 3, after | loads |
| `FOneColorVertexShader` | other | 136 | 0, - | loads |
| `FScreenPixelShader` | other | 188 | 3, after | loads |
| `FScreenVertexShader` | other | 204 | 0, - | loads |
| `FShaderComplexityAccumulatePixelShader` | other | 212 | 3, after | loads |
| `FShaderComplexityApplyPixelShader` | other | 1016 | 18, after | layout differs |
| `FShadowProjectionVertexShader` | other | 296 | 0, - | layout differs |
| `Fetch4PCFHighQualityShaderName` | other | 3848 | 39, after | layout differs |
| `Fetch4PCFLowQualityShaderName` | other | 2312 | 39, after | layout differs |
| `Fetch4PCFMediumQualityShaderName` | other | 2848 | 39, after | layout differs |
| `HighQualityShaderName` | other | 4008 | 39, after | layout differs |
| `HwPCFHighQualityShaderName` | other | 2448 | 39, after | layout differs |
| `HwPCFLowQualityShaderName` | other | 1736 | 39, after | layout differs |
| `HwPCFMediumQualityShaderName` | other | 1976 | 39, after | layout differs |
| `LowQualityShaderName` | other | 2368 | 39, after | layout differs |
| `MediumQualityShaderName` | other | 3464 | 39, after | layout differs |
| `VisualizeTexturePixelShader` | other | 620 | 6, after | loads |

## 4. Reference-only global shader types (144: declared here, no cooked record in retail)

Retail has no shader for these; the renderer paths that bind them must be guarded or replaced by the Arkane path:

`FUberPostProcessBlendPixelShader*` (72), `FUberHalfResPixelShader*` (9), `TAmbientOcclusion*` (8), `FFXAABlendPixelShader*` (6), `TModShadowProjectionPixelShader<*>` (6), `FLUTBlenderPixelShader*` (5), `TDownsampleLightShafts*` (3), `TDOF*` (2), `THeightFog*` (2), `TShadowProjectionPixelShader*` (2), `FAmbientOcclusionVertexShader`, `FBloomGatherPixelShaderNumFPFilterSamplesFALSE`, `FBloomGatherPixelShaderNumFPFilterSamplesTRUE`, `FDOFAndBloomBlendPixelShader`, `FDOFAndBloomBlendVertexShader`, `FDownsampleDepthVertexShader`, `FDownsampleScene`, `FEdgePreservingFilterVertexShader`, `FFourLayerFogPixelShader`, `FHistoryUpdateVertexShader`, `FLUTBlenderVertexShader`, `FOneLayerFogPixelShader`, `FSRGBMLAABlendPixelShader`, `FSRGBMLAAEdgeDetectionPixelShader`, `FSimpleElementColorChannelMaskPixelShader`, `FSimpleF32PixelShader`, `FSimpleF32VertexShader`, `FStaticHistoryUpdatePixelShader`, `FTemporalAAMaskExpandPixelShader`, `FTemporalAAMaskSetupPixelShader`, `FTemporalAAPixelShader`, `FTemporalAAVertexShader`, `FUberPostProcessVertexShader`, `TAOApplyPixelShader<AOApply_Normal>`, `TAOApplyPixelShader<AOApply_ReadFromAOHistory>`, `TDownsampleDepthPixelShader`, `TEdgePreservingFilterPixelShader`, `TExponentialHeightFogPixelShader<MSAASF_NoMSAA>`, `TMotionBlurGatherPixelShader<NumFPFilterSamples>`

## 5. Work list, in order

**Wave-4 update (agent AH, 2026-09-26).** Items 3 and 5 are **done**; item 2 is agent AG's; item 4 is wave 5. The cooked
global cache now reports `263 shaders, 127 loaded, 136 undeclared types, 0 parameter mismatches, 0 other skips` and
`global shader map: 133 types without a cooked shader` (was 62 / 143 / 58 / 144). The scene renderer runs every frame on
the null RHI (the wave-3 skip in `RenderViewFamily_RenderThread` is gone); a D3D9 world frame still waits on item 2.
Details and the per-type rvas: `agents/agentAH.md`, `agents/agentAH_status.csv`.


1. **First frame** (this wave): nothing more needed from the cache: `FSimpleElement*` (canvas, 6 types), `FOneColor*`
   (clears through `DrawClearQuad`), the Bink shaders and `TFilterPixelShader`/`TFilterVertexShader` load.
2. **Material shaders** (blocks every D3D9 run: `FMaterial::InitShaderMap` aborts on a default material without a shader map,
   2013 rva 0x13db20 does the same on stripped platforms). Measured with the retail gates applied locally
   (`build\agentY\logs\run_material_gates_uniformexpr.log`, first material cache of Startup.upk, each type reported once):
   * **every declared mesh-material shader type's layout differs** (49 types: `TLightPixelShader`/`TLightVertexShader<*,*>`,
     `TDepthOnly*`, `TLightMapDensity*`, `FHitProxy*`, `FHitMask*`, `FTextureDensity*`, `TDistortionMesh*`, `FLightFunctionPixelShader`,
     `FRadialBlur*`, `FTranslucencyPostRenderDepthPixelShader`, ...), so the difference is in the common part
     (`FMaterialShader`/`FMeshMaterialShader` parameters, `FVertexFactoryParameterRef`): converge that first. The first type
     read before any sized field diverged (`TLightPixelShader<FSpotLightPolicy,FNoStaticShadowingPolicy>` reads an FName), which
     is why `FShaderLoadArchive` now validates name/object indices against the linker tables and skips the record;
   * **49 types undeclared**, mostly `TBasePassPixelShader<Policy, SkyLight, X>`: retail names end in `FALSEFALSE` /
     `FALSETRUE` / `TRUEFALSE` / `TRUETRUE` (a third template argument Arkane added, 2013 rva 0xb7ea40
     `TBasePassPixelShaderFNoLightMapPolicyNoSkyLightFALSEFALSE`) and `TBasePassVertexShader<Policy>`, `FModShadowMesh*`;
   * then `FMaterialShaderMap::Serialize` (2013 rva 0x40ef30) stops in `FUniformExpressionSet::Serialize` (garbage name index):
     Arkane's uniform-expression set layout is the next item.
   Retail `FMaterialShaderMap::IsComplete` (2013 rva 0x3ea7d0, 116 bytes) returns FALSE only while the map compiles (the
   per-vertex-factory lookups are there but unused); the reference checks every type and vertex factory. Full list:
   `build\agentY\material_types.txt`.
3. **Global types whose layout differs (58)**: **done (agent AH, wave 4; 0 mismatches left)**. Most of the 58 fell to two
   shared structs rather than to per-type work: `FSceneTextureShaderParameters` is 5 parameters / 30 bytes in retail
   (SceneColorTexture, SceneDepthTexture, SceneDepthCalc, ScreenPositionScaleBias, NvStereoFixTexture; `operator<<`
   2013 rva 0x447e20, `Set` 0x4550b0) against 8 here, and `FLightShaftPixelShaderParameters` is 10 parameters / 60 bytes
   (no spot direction / spot angles; 0x415690, `SetParameters` 0x420300) against 12. Per type on top of that:
   the projection vertex shaders (`FShadowProjectionVertexShader`, `FModShadowProjectionVertexShader`) carry **no**
   parameter (the screen-to-shadow matrix is a pixel shader constant); `TShadowProjectionPixelShader<*>` is scene textures
   + ScreenToShadowMatrix + ShadowDepthTexture + SampleOffsets + ShadowBufferSize + ShadowFadeFraction (ctor 0xe34b0,
   `Serialize` 0xe3610, `SetParameters` 0xed400) with no deferred G-buffer parameters and no lighting-channel mask;
   `TModShadowProjectionPixelShader<*,*>` and `TBranchingPCFModProjectionPixelShader<*,*>` add **four** own parameters
   before the light policy's (0xe7de0 / 0x134440 / 0x162950, 0x1383c0 / 0x162fd0), of which retail's `SetParameters` writes
   only ShadowModulateColor and ScreenToWorld; `FDownsampleLightShaftsVertexShader` has ScreenToWorld only (0x415460);
   `FMLAAVertexShader` / `FFXAAVertexShader` are Arkane's with two parameters each (0x50e170 / 0x50e090). The retail
   uniform-PCF policy set is `F4SampleHwPCF`, `F4SampleManualPCF`, `F16SampleHwPCF`, `F16SampleFetch4PCF`,
   `F16SampleManualPCF` (0x45cc20 `GetProjPixelShaderRef`, 0x1451e0 `GetModProjPixelShaderRef`) - no per-fragment variant -
   and the light shaft pixel shader is templated on a `UBOOL` (`<FALSE>` directional, `<TRUE>` everything else, 0x437480).
4. **Undeclared Arkane types (143 -> 136, wave 5)**: the 10 that were only misnamed are declared and loading since wave 4
   (`TShadowProjectionPixelShader<*ManualPCF>` 2, `TModShadowProjectionPixelShader<*,*ManualPCF>` 6,
   `TDownsampleLightShaftsPixelShader<TRUE/FALSE>` 2). What is left needs its Arkane pass: `FDisFog*` 30
   (+`FHeightFogMask*` 2), `FArkPp*` 16, `FKuwa*` 4, `FBloom*`/`TBloom*` 6, `FFXAAPixelShader_*` 3, `FMLAA*` 4,
   `TFilterPixelShaderDepthInAlpha<1..16>` 16, `TMeshPaint*` 2, and the 50 GFx shaders (Scaleform, gated by the GFxUI
   decision). `FSceneRenderer::RenderBloomParts` (2012 rva 0x566120 / 2013 0x5251a0, between the soft-masked base pass and
   the fog pass in retail's `RenderDPGEnd`) is the entry point for the Arkane bloom set. The PDB has **no UDT** for most Arkane shader classes (local
   classes of their .cpp files); their parameter layouts come from their `Serialize` / `SetParameters` / constructor
   decompiles (2012 names in `functions.csv`, e.g. `FArkPpDofLutBlenderPS::Serialize`, `FBloomComposePixelShader::Serialize`,
   `FFluidSimulatePixelShader::Serialize`) checked against the history word counts of section 3. They are useless without
   their render passes (the FArkPp node graph, DisFog, Arkane bloom), so each is declared together with its pass.
5. **Reference-only types (section 4)**: **guarded (agent AH, wave 4)**; 144 -> 133 declared types without a cooked
   shader, and no pass that binds one can be reached any more: `FSceneRenderer::RenderDPGEnd` (2013 rva 0x464290) lost the
   image-reflection, subsurface-scattering, lighting-only post-process and velocity passes and gates the reference
   height-fog pass off until DisFog lands (`-referencefog` re-enables it for experiments), `RenderFinish` (0x45e5e0) lost
   temporal AA and the reference MLAA/FXAA pass, and `RenderPostProcessEffects` (0x448990) runs only at `SDPG_PostProcess`
   and skips the reference effect proxies with one `DISHONORED(bringup)` warning (retail renders the FArkPp graph there).
   The 11 reference-only shadow-projection / light-shaft instances were removed outright. Retail has none of these: the
   uber post-process chain (`FUberPostProcess*`,
   `FUberHalfRes*`, `FDOFAndBloom*`, `FBloomGather*`, `FLUTBlender*`), temporal AA, SSAO (`TAmbientOcclusion*`,
   `TAOApply*`, `FAmbientOcclusionVertexShader`, `FStaticHistoryUpdate*`, `FHistoryUpdate*`, `TEdgePreserving*`), the reference
   FXAA/MLAA (`FFXAABlendPixelShader0..5`, `FSRGBMLAA*`), `FOneLayerFogPixelShader`/`FFourLayerFogPixelShader`/
   `TExponentialHeightFog*`/`THeightFog*` (retail fog is DisFog), `FSimpleElementColorChannelMaskPixelShader`,
   `FSimpleF32*`, the per-pixel ManualPCF shadow variants. Guard their passes (they must never be reached in game) or remove
   the types when the Arkane replacement lands; `ScenePostProcessing`/`UberPostProcessEffect` are replaced by the FArkPp graph
   (`UPostProcessChain::m_GraphRoot`, agentS.md).

## 6. Scene view / renderer members (2012 PDB vs this tree)

Retail truth for these structs is the 2012 PDB only (no reflection, no retail SDK rows). `FSceneViewFamily` is 80 bytes in
the PDB (`CurrentBendTime` @0 is the missing member, agentV.md), `FSceneView` 1280 (ours larger: reference-only post-process
settings, temporal AA, RealD), `FViewInfo` 4240, `FSceneRenderer` 4528. Arkane members (PDB-only) are the convergence list:

* `FSceneView`: `m_ArkPpConfig` (@28, `FArkPpConfig`), `SceneReflectionTexture` (@32), `m_PostProcessProxy` (@36,
  `FArkPpNodeProxy*`: the post-process graph instance), `InvDeviceZToWorldZTransform_Raw` (@1120),
  `m_iCurrentActiveReflectCaptureSceneBit` (@1232), `EditorViewBitflag` (@1236, QWORD). Reference-only here:
  `PostProcessSettings`, `PostProcessSceneProxies`, `DepthOfFieldParams`, `TemporalAAParameters`, `bRenderTemporalAA`,
  `ScreenDoorRandomOffset`, `ClipX/ClipY`, `InvViewMatrix`, `bCameraCut`, `bForceLowestMassiveLOD`, `bForceClear`,
  `RenderingOverrides`, `SpriteCategoryVisibility`, `RealDCoefficients`, `DBATempVariables`.
* `FViewInfo`: `PrimitiveParentProcessedMap`, `PrimitiveViewRelevanceMap` (a TArray, not a bit array), `m_VisibleSoulPrimitives`
  (dark vision), `BloomPartPrimSet[4]` (`FArkBloomPartPrimSet`), the per-DPG bits `bHasTranslucentViewMeshElements`/
  `bHasDistortionViewMeshElements`/`bHasBloomPartViewMeshElements`, rain (`m_fWrapHeight`, `m_fRainSpeed`, `m_fRainIntensity`,
  `m_fRainMinLuminance`, `m_fRadToTexCoordA/B`, `m_fRadiusA/B`, `m_pRainTexture`, `m_RainRotation`), `m_NumPossiblePixels`,
  `m_InvNumPossiblePixels`, `MotionBlurParameters`, `NumVisibleDynamicPrimitives`, `DisPrecomputedFogs`
  (`FDisPrecomputedFogSceneInfo`, 208 bytes). Reference-only: `StaticMeshVelocityMap`, `HeightFogParams`, `FogMaxOpacity`,
  `bRenderExponentialFog`, `bRenderedToDoFBlurBuffer`, `PrevTranslatedViewProjectionMatrix`, `MotionBlurParams`,
  `OneOverNumPossiblePixels`.
* `FSceneRenderer`: `mLightInFrustumIndices`, `mLightWithShaft`, `ProjectedShadowToTerminate`,
  `MinViewDistanceSquaredOverride`, `m_BloomNeedBlit`, `bPerformMinDistanceChecks`. Reference-only: `PlanarReflectionShadows`,
  `FrameNumber`, `MobileProjectedShadows`.

Order: `FSceneViewFamily::CurrentBendTime` (layout-only, agent V's follow-up), then the `FSceneView` post-process members with
the FArkPp graph, then `FViewInfo` DisFog/rain/bloom-part with their passes. `build\agentY\cmp_members.py` regenerates the
raw diff below (heuristic parse of our class bodies; template-argument noise such as `FPrimitiveSceneInfo` in the ours-only
column is not a member).

```
## FSceneViewFamily: PDB 17 members, ours 18
  PDB-only (Arkane or missing here):
  ours-only (reference members retail lacks): bScreenCaptureRenderTarget
## FSceneView: PDB 53 members, ours 63
  PDB-only (Arkane or missing here):
    28     4  const FArkPpConfig *  m_ArkPpConfig
    32     4  const UTextureCube *  SceneReflectionTexture
    36     4  FArkPpNodeProxy *  m_PostProcessProxy
    1120    16  FVector4  InvDeviceZToWorldZTransform_Raw
    1232     4  int  m_iCurrentActiveReflectCaptureSceneBit
    1236     8  unsigned __int64  EditorViewBitflag
  ours-only (reference members retail lacks): PostProcessSettings, PostProcessSceneProxies, ClipX, ClipY, ScreenDoorRandomOffset, DepthOfFieldParams, TemporalAAParameters, InvViewMatrix, bCameraCut, bForceLowestMassiveLOD, bRenderTemporalAA, bForceClear, RenderingOverrides, SpriteCategoryVisibility, RealDCoefficients, DBATempVariables
## FViewInfo: PDB 56 members, ours 47
  PDB-only (Arkane or missing here):
    1296    28  TBitArray<SceneRenderingBitArrayAllocator>  PrimitiveParentProcessedMap
    1324    12  TArray<FPrimitiveViewRelevance,SceneRenderingAllocator>  PrimitiveViewRelevanceMap
    1460    12  TArray<FPrimitiveSceneInfo const *,SceneRenderingAllocator>  m_VisibleSoulPrimitives
    1856    48  FArkBloomPartPrimSet[4]  BloomPartPrimSet
    3568     0  unsigned int  bHasTranslucentViewMeshElements :4@0
    3568     0  unsigned int  bHasDistortionViewMeshElements :4@4
    3569     0  unsigned int  bHasBloomPartViewMeshElements :4@0
    3584     4  float  m_fWrapHeight
    3588     4  float  m_fRainSpeed
    3592     4  float  m_fRainIntensity
    3596     4  float  m_fRainMinLuminance
    3600     4  float  m_fRadToTexCoordA
    3604     4  float  m_fRadToTexCoordB
    3608     4  float  m_fRadiusA
    3612     4  float  m_fRadiusB
    3616     4  UTexture2D *  m_pRainTexture
    3632    64  FMatrix  m_RainRotation
    3696     4  float  m_NumPossiblePixels
    3700     4  float  m_InvNumPossiblePixels
    3900    20  FMotionBlurParameters  MotionBlurParameters
    3936     4  int  NumVisibleDynamicPrimitives
    4032   208  FDisPrecomputedFogSceneInfo  DisPrecomputedFogs
  ours-only (reference members retail lacks): StaticMeshVelocityMap, FPrimitiveSceneInfo, FPrimitiveSceneInfo, FPrimitiveSceneInfo, FVisibleLightViewInfo, SDPG_MAX_SceneRender, HeightFogParams, FogMaxOpacity, bRenderExponentialFog, bRenderedToDoFBlurBuffer, PrevTranslatedViewProjectionMatrix, MotionBlurParams, OneOverNumPossiblePixels
## FSceneRenderer: PDB 21 members, ours 22
  PDB-only (Arkane or missing here):
    4352    12  TArray<int,SceneRenderingAllocator>  mLightInFrustumIndices
    4364    12  TArray<int,SceneRenderingAllocator>  mLightWithShaft
    4388    12  TArray<FProjectedShadowInfo *,SceneRenderingAllocator>  ProjectedShadowToTerminate
    4480     4  float  MinViewDistanceSquaredOverride
    4516     4  unsigned int  m_BloomNeedBlit
    4520     4  unsigned int  bPerformMinDistanceChecks
  ours-only (reference members retail lacks): FVisibleLightInfo, FReflectionPlanarShadowInfo, PlanarReflectionShadows, FrameNumber, FLightSceneInfo, FProjectedShadowInfo, MobileProjectedShadows
```
