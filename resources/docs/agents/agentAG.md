# Agent AG report — Renderer A: material shader maps from the retail cooked caches (2026-09-26)

Package **AG** of `PHASE6.md` (wave 4). Build dir `build\agentAG`, configured from the snapshot worktree `build\agentAG_wt`
(HEAD `571bd0e` + my files + agent AH's renderer files, `resources/tools/make_snapshot.py AG`, list in
`build\agentAG\snapshot_files.txt`). IDA copies `resources\docs\idb\retail2013_agentAG.i64` /
`shipping2012_agentAG.i64`, headless only; decompiles in `build\agentAG\decomp13`, `decomp12`, `decomp12b`; the retail shader
type gate dump in `build\agentAG\statictypes2013.csv` (`build\agentAG\dump_statictypes.py`, 386 `StaticType` initializers).
Every number below comes from the `build\agentAG` build. No commits, nothing staged.

## Result

| Item | State |
|---|---|
| **Accept (d3d9)** | **passes.** `material shader maps: 2580 loaded, 177 skipped, 0 undeclared types, 0 mismatches (142902 shader references: 0 undeclared, 0 not loaded, 5172 for vertex factories retail does not have)`; exit 0, milestone `Initializing Engine...`, `Initial startup: 25.50s`; **no** `Failed to find shader map for default material`; the `-agentYnomatshaders` bypass is not used (0 occurrences) |
| **Accept (null RHI)** | unchanged: same 0/0 inventory, `Initial startup: 20.97s`, then the pre-AD baseline end `Bad export index 1065353215/6389 … NavigationMeshBase` (AD's streaming fix is not in my snapshot). The `no shader map for special material` warning is gone |
| Byte-exactness | every loaded record passes `ShaderCache.cpp`'s `checkf(Ar.Tell() == SkipOffset)`, so all 142902 references consumed exactly their cooked byte ranges |
| Build | snapshot builds with 0 errors (`build\agentAG_build10.log`); the shared working tree compiles my files with 0 errors too (`build\agentAG_build1.log`; its failures are other agents' in-flight edits in DishonoredGame/UnCamera/UnPhysic) |

Exact commands:

```
rem accept, d3d9
python resources\tools\build_and_smoke.py --build-dir build\agentAG --no-build --exe-name DishonoredGame_AG.exe ^
  --log-name agentAG.log --ini-dir build\agentAG\config --rhi d3d9 --timeout 120 --milestone "Initializing Engine..." ^
  --expect "Initial startup" --expect "material shader maps:" --skip-native OnlineSubsystemPC ^
  --extra-args "-windowed -ResX=1280 -ResY=720 -nomovie"          -> exit 0 (build\agentAG\accept_d3d9.txt)

rem same on the null RHI (baseline unchanged)
python resources\tools\build_and_smoke.py --build-dir build\agentAG --no-build --exe-name DishonoredGame_AG.exe ^
  --log-name agentAG_null.log --ini-dir build\agentAG\config_null --rhi null --timeout 120 ^
  --milestone "Initializing Engine..." --expect "Initial startup" --expect "material shader maps:" ^
  --skip-native OnlineSubsystemPC                                  -> exit 0 (build\agentAG\accept_null.txt)

rem build (snapshot; make_snapshot.py wrote build\agentAG_wt_build.cmd)
python resources\tools\make_snapshot.py AG <files>   &&   build\agentAG_wt_build.cmd DishonoredGame
```

## 1. The retail gates (package item 1)

`build\agentAG\statictypes2013.csv` dumps the immediates of every `_dynamic_initializer_for_…::StaticType__` of the 2013 exe,
so the gates are read off the binary instead of guessed:

| Gate | Value | Evidence |
|---|---|---|
| material shader **map** | 786 / licensee 23 | `UShaderCache::Load` 2013 rva 0x164a60: `if (SavedVer < 786 \|\| SavedLicenseeVer < 23) Seek(SkipOffset)` |
| `TDepthOnlyVertexShader<0/1>`, `FHitProxyVertexShader`, `FHitMaskVertexShader`, `FTextureDensityVertexShader`, `FLightFunctionVertexShader`, `TShadowDepthVertexShader<*>`, `FModShadowMesh*`, `TDistortionMeshVertexShader`, `FRadialBlur*`, `TBloomPartMeshVertexShader`, `TPpMaterialVertexShader` | 786 / 23 | 0xb7f500/0xb7f540, 0xb815e0, 0xb7feb0, 0xb81db0, 0xb7ff30, 0xb81860-0xb818e0, 0xb81c80/0xb81cc0, 0xb6f990, 0xb71100/0xb71180, 0xb82850, 0xb83150 |
| every `TLightPixelShader` / `TLightVertexShader` | **792** / 23 | 0xb6f2d0 … 0xb71920 (`push 318h`) |
| `TBasePass*`, `TDepthOnlySolidPixelShader`, `TLightMapDensity*`, `TDistortionMeshPixelShader`, `TShadowDepthPixelShader*`, `TBloomPartMeshPixelShader` | 798 / 23 | 0xb7ea00 / 0xb7ea40 …, 0xb7f580, 0xb6ff50 …, 0xb6f9d0, 0xb81920/0xb81960, 0xb82890 |
| `TPpMaterialPixelShader<*>` | **801** / 23 (the highest in the exe) | 0xb83190 / 0xb831d0 |
| `TSoulPartMesh*` | 786 / **24** and 798 / 24 | 0xb8efe0 / 0xb8f020 |

`VER_MIN_MATERIALSHADERMAP` / `VER_MIN_MATERIAL_PIXELSHADER` / `VER_MIN_MATERIAL_VERTEXSHADER` are now 786 and the licensee
floors 23 (`MaterialShader.h`); the reference `VER_INVALIDATE_SHADERCACHE5` (836) is newer than the cooked content (801 / 30)
and had rejected every material shader map. The types I touched carry their exact retail gate; the rest keep the reference
`MinPackageVersion` under the new 786/23 floor, which changes no behaviour for 801/30 content (follow-up 4).

## 2. The common part (package item 2)

The layouts come from the 2012 PDB (`resources/docs/types/types.json`) and the field order from the 2013 decompiles.

| Struct | Retail | What the reference tree had | Evidence |
|---|---|---|---|
| `FMaterialShaderParameters` | **36 bytes, 6 parameters**: CameraWorldPosition, ObjectWorldPositionAndRadius, ObjectOrientation, WindDirectionAndSpeed, FoliageImpulseDirection, FoliageNormalizedRotationAxisAndAngle | 15 parameters + `FDOFShaderParameters` + the three uniform arrays | 2013 0x3de320 / 0x3deca0 serialize +0…+30 first |
| `FMaterialPixelShaderParameters` | **192 bytes**: base, 4 uniform arrays (pixel scalar/vector/2D/cube), LocalToWorld, WorldToLocal, WorldToView, InvViewProjection, ViewProjection, `FSceneTextureShaderParameters` @114, `FWorldCubeMapTextureShaderParameters` @144, TwoSidedSign, InvGamma, DecalFarPlaneDistance, ObjectPostProjectionPosition, ObjectNDCPosition, ObjectMacroUVScales, OcclusionPercentage | screen-door fade (5), AlphaSampleTexture, FluidDetailNormalTexture, DOF, TemporalAA, ActorWorldPosition, a 2-value decal near/far parameter | 2013 0x3de320: 24 serializations, ObjectMacroUVScales (+180) **before** ObjectNDCPosition (+174) |
| `FMaterialVertexShaderParameters` | **60 bytes**: base + vertex scalar and vector uniform arrays | also 2D texture expressions, DOF, the matrices | 2013 0x3deca0: 8 serializations |
| `FWorldCubeMapTextureShaderParameters` | **6 bytes** (Arkane; samples `FSceneView::SceneReflectionTexture`) | absent | 2012 0x4715a0 (`scenerendertargets.cpp:1977`) |
| `FForwardShadowingShaderParameters` | **24 bytes, 4 parameters** (no ShadowOverrideFactor) | 5 parameters | inlined in 2013 0xf43e0 @326…344 and 0x4260f0 @114…132 |
| `FUniformExpressionSet` | **80 bytes, 6 arrays** in member order (pixel vector, pixel scalar, 2D texture, cube texture, vertex vector, vertex scalar) | pixel set, cube, vertex set + two empty hull/domain dummies | 2013 0x1464d0 (6 calls) |
| `FVertexFactoryParameterRef` | 28 bytes, type + VFHash + skip offset + parameters | identical on PC | 2013 0x388e30 — verified, unchanged |
| `FSceneTextureShaderParameters` | 30 bytes / 5 parameters | 8 parameters | **agent AH's** change (2013 0x447e20), which my pixel layout depends on |

Per-type serializers ported to the retail field order: `TLightPixelShader` (no vertex factory parameters in retail — the
retail class derives from `FShader`), `TLightVertexShader`, `TBasePassPixelShaderBaseType` (no vertex factory / TemporalAA /
DeferredRendering parameters), `TBasePassVertexShader` (one template argument, no height fog / fog volume parameters),
`FShadowDepthPixelShader` (no ShadowCasterPosition / ModShadowColor), `FShadowDepthVertexShader`, `TDepthOnly*`,
`FHitProxy*`, `FHitMask*`, `FTextureDensity*`, `TLightMapDensity*`, `TDistortionMesh*`, `FRadialBlur*`,
`FLightFunctionPixelShader` (layout already matched; retail names its sixth parameter `GreyscaleFactor`).

Vertex factory parameter classes (all 11 retail ones checked against the PDB):

| Class | Retail | Fix |
|---|---|---|
| `FGPUSkinVertexFactoryShaderParameters` | 40 bytes, 6 parameters: LocalToWorld, WorldToLocal, BoneMatrices, **MaxBoneInfluences**, MeshOrigin, MeshExtension | ported (2013 0xe26f0 / 2012 0xe0a80): dropped BoneScales, BoneIndexOffsetAndScale, PreviousBoneMatrices, bUsePerBoneMotionBlur and the whole `PrevPerBoneMotionBlur` path from `Set`; added `MaxBoneInfluences` to `ShaderDataType` (retail @4) |
| `FParticleVertexFactoryShaderParameters` | 72 bytes, 11 parameters | dropped `CornerUVs` (reference PS3/OpenGL only), 2013 0x46f100 |
| `FInstancedStaticMeshVertexFactoryShaderParameters` | 36 bytes, base + 2 | dropped `InstancingFadeOutParams`, 2013 0x1111f0 |
| `FGPUSkinVertexFactoryShaderParametersApexDestructible` | 48 bytes, base + 3 | **dropped `bUsePerBoneMotionBlur`** (2013 0x527720): this one extra serialization was rejecting every cooked mesh-material record of the factory that carries Startup.upk's skeletal meshes |
| `FLocalVertexFactory*`, `FLocalDecal*`, `FSplineMesh*`, `FFluid*`, `FParticleBeamTrail*`, `FParticleInstancedMesh*`, `FGPUSkinDecal*`, `FLensFlare*`, Apex clothing | matched already | verified against the PDB sizes and the 2013 serializers |

## 3. The undeclared types (package item 3) — all 7 now declared

| Type | Retail home | Layout | Registration |
|---|---|---|---|
| `TBasePassPixelShader<Policy,bSkyLight,bPrecomputedFog>` | BasePassRendering.h | 328 bytes = base type + `PrecomputedFog` @320 | Arkane's **third template argument**; the four cooked names per policy (`…NoSkyLightFALSEFALSE`, `…FALSETRUE`, `…SkyLightTRUEFALSE`, `…TRUETRUE`) are now all registered (798/23). The third argument is the DisFog precomputed-fog variant; the drawing policy still selects `FALSE` (wave 5) |
| `TBasePassVertexShader<Policy>` | BasePassRendering.h | 196 bytes | registered as `TBasePassVertexShader<Policy>` without the fog density policy (798/23); the reference fog-density instantiations stay for the fog volume policies |
| `FModShadowMeshVertexShader` / `FModShadowMeshPixelShader` | ShadowRendering.cpp/h | 204 / 308 bytes | written from the PDB + 2013 0x453fd0; `ModShadowMeshAttenuationVS/PS`, 786/23. Pass not ported |
| `TPpMaterialVertexShader<FPpMaterialMeshPolicy>`, `TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy/FPpMaterialGammaSpaceMeshPolicy>` | arkppnodematerial.cpp | 196 / 368 bytes (8 `m_ArkPpTextureSampleParameter` + 3 vectors) | written from 2013 0x51caa0; `ArkPpMaterialVertexShader/PixelShader`, 786/23 and 801/23. The FArkPp node graph is wave-5 work |
| `TBloomPartMesh{Vertex,Pixel}Shader<FBloomPartMeshPolicy>` | arkbloompartsrendering.cpp | 196 / 300 bytes | written; `ArkBloomPartVertexShader/PixelShader`, 786/23 and 798/23. `FSceneRenderer::RenderBloomParts` (2013 0x566120) not ported |
| `TSoulPartMesh{Vertex,Pixel}Shader<FSoulPartMeshPolicy>` | DishonoredGame/Src/dispostprocesscontrollers.cpp (not ported) | 196 / 300 bytes | written next to the bloom-part types with a note to move them when that unit lands; `ArkSoulPartVertexShader/PixelShader`, 786/24 and 798/24 |

**Static-library dead-strip:** the two Arkane units were empty stubs, so nothing referenced their object files and the linker
dropped them together with their `StaticType` registrations — the types stayed "undeclared" at runtime although the source
declared them. `BasePassRendering.cpp` now takes the address of one anchor function per unit
(`GDishonoredArkMeshShaderTypeLinkAnchors`); drop it when the passes are ported and called.

## 4. Map, uniform expressions, `IsComplete` (package item 4)

* `FMaterialShaderMap::Serialize` (2013 0x40ef30) ported; the **loading** side reads the `TMap<FShaderType*,FShader*>` by hand
  (`DishonoredLoadShaderMap`, MaterialShader.h) so every cooked reference's type name is known to the inventory. Same bytes as
  the reference `TMap` serializer (key `FShaderType*` = FName, value `FShader*` = FGuid + FName, 2013 0x160310 / 0x160590).
* `FUniformExpressionSet::Serialize` (2013 0x1464d0) ported (6 arrays, no hull/domain dummies). This was the point where the
  map load previously stopped.
* `FMaterialShaderMap::IsComplete` (2013 0x3ea7d0, 116 bytes) ported: **FALSE only while the map is being compiled.** The
  reference version walks every declared shader and vertex factory type, so with this tree's reference-only types every
  cooked map looked incomplete and `FMaterial::InitShaderMap` threw it away — the direct cause of the
  `Failed to find shader map for default material LevelColorationLitMaterial` abort.
* `FStaticParameterSet::Serialize` verified identical (2013 0x11aec0, gates 631 / 714 match).
* `UShaderCache` 132 → 128 is **not** done: it needs `ShaderCachePriority` removed from `ShaderCache.h` and its 11 uses in
  `ShaderCache.cpp` (agent AH's file), plus `FCompressedShaderCodeCache` (retail 84 bytes, no `ShaderCachePriority` /
  `bIsAlwaysLoaded`). Hand-over below; it changes no serialization for 801 content (the member is only read at Ver >= 805).

## 5. Reference-only vertex factories — why "0 mismatches" is the retail behaviour

Retail registers exactly **17** vertex factory types (`statictypes2013.csv`): Local, LocalDecal, LocalApex,
InstancedStaticMesh, SplineMesh, GPUSkin, GPUSkinDecal, GPUSkinApexDestructible, GPUSkinApexClothing, Particle,
ParticleDynamicParameter, ParticleSubUV, ParticleSubUVDynamicParameter, ParticleBeamTrail,
ParticleBeamTrailDynamicParameter, ParticleInstancedMesh, LensFlare. It has **no terrain, landscape or SpeedTree vertex
factory code at all** — neither the factories nor their `FVertexFactoryShaderParameters` subclasses exist in the 2012 PDB.

The cooked caches were written by the editor, which does register them, so they contain 177 mesh shader maps (5172 shader
references) for those factories. Retail skips them because `FindVertexFactoryType` returns NULL for an unregistered name
(`operator<<(FMeshMaterialShaderMap&)` empties the map; `operator<<(FVertexFactoryParameterRef&)`, 2013 0x388e30, reports
outdated parameters and the record is dropped). This tree compiles the reference factories, so their records were being parsed
with reference parameter layouts and counted as mismatches — that is what made all 49 mesh-material types look broken.
`FindVertexFactoryType` now returns NULL for the 14 reference-only names (VertexFactory.cpp, with the evidence), and the
inventory reports those references as **skipped**, not as mismatches. Nothing is hidden: the line prints all four numbers.

## 6. Numbers

| Measure | Before (baseline HEAD) | After |
|---|---|---|
| material shader maps loaded | 0 (every map rejected at 836/0) | **2580** |
| mesh shader maps skipped (no retail vertex factory) | – | 177 (5172 references), retail behaviour |
| undeclared material shader types | 49 | **0** |
| mismatching material shader types | 49 (all declared mesh-material types) | **0** |
| shader references resolved | 0 | **142902** |
| d3d9 run | `appError: Failed to find shader map for default material LevelColorationLitMaterial` before the device InitRHI pass | `Initial startup: 25.50s`, then out of memory in the Debug allocator (below) |

## 7. What is left / follow-ups

1. **Debug allocator runs out of memory.** With the caches actually loading, the d3d9 run dies ~2 s after `Initial startup`
   in `FMallocDebug::Malloc` (`Assertion failed: Ptr`, FMallocDebug.h:68) while the main menu streams — 32-bit address space
   plus `_DEBUG` → `FMallocDebug` (LaunchEngineLoop.cpp:298), which has no binning and large per-allocation overhead. The null
   run (no RHI shader objects) gets further, to AD's nav mesh point. **Long d3d9 runs (AF's milestone 5, AH's 30 s of frames)
   should use a Release build or `FMallocBinned` in Debug** — a one-line `#elif _DEBUG && !USE_MALLOC_PROFILER` change plus a
   switch. Hand-over to AK / the coordinator; not attempted here (`LaunchEngineLoop.cpp` is AD's file this wave).
2. **Global shader types** in my run: `global shader cache: 263 shaders, 120 loaded, 138 undeclared types, 5 parameter
   mismatches`. Those are agent AH's package (my snapshot has only part of AH's tree); AH reports 127 loaded / 0 mismatches in
   its own build. The 5 that appear in package caches here are `FMLAAVertexShader`, `FFXAAVertexShader`,
   `FApplyLightShaftsPixelShader`, `FBlurLightShaftsPixelShader`, `FDownsampleLightShaftsVertexShader`.
3. **`UShaderCache` 132 → 128** and `FCompressedShaderCodeCache` 84: needs AH's `ShaderCache.cpp` (hand-over 3 below).
4. **Exact per-type gates** for the material shader types I did not otherwise touch (they keep the reference
   `MinPackageVersion` above the new 786/23 floor; no behaviour change for 801/30 content). `statictypes2013.csv` has the
   retail value for all 386 types.
5. **`FStaticTerrainLayerWeightParameter`** field order is unverified (retail has no terrain content to test with); sizes match,
   so the static parameter set stays byte-exact either way.
6. `FSceneView::SceneReflectionTexture` (2012 PDB @32) is not declared, so `FWorldCubeMapTextureShaderParameters::Set` samples
   the white cube instead of the reflection capture (hand-over 2).
7. The Arkane passes behind the newly declared types (FArkPp material node, bloom parts, soul parts, modulated shadow mesh)
   remain unported — the types only exist so the cooked maps load completely; nothing selects them.

## 7a. Agent AD's render-thread abort in `FMaterialInstanceResource::GetMaterial` (hand-over to me)

AD's stack (`build\agentAD\dbg_commit1.txt`, read only) is
`appErrorf/DebugBreak` → `FMaterialInstanceResource::GetMaterial+0x2a4` → `FStaticMesh::AddToDrawLists+0xfa` →
`FScene::AddPrimitiveSceneInfo_RenderThread+0xa8` → `FAddPrimitiveCommand::Execute`. It is an `appErrorf`, not a null
dereference, and `GetMaterial` (MaterialInstance.cpp:23) has exactly one way to reach one: a material instance with
`bHasStaticPermutationResource` whose `StaticPermutationResources[Quality]->GetShaderMap()` is NULL falls into
`GEngine->DefaultMaterial->GetRenderProxy(...)->GetMaterial()`, and on a stripped platform the default material without a
shader map is exactly the `Failed to find shader map for default material` abort this package removes (`FMaterial::InitShaderMap`,
MaterialShared.cpp). The two `checkSlow`s above it (`IsCompilationFinalized`, `CompiledSuccessfully`) cannot fire for a loaded
map: retail's `FMaterialShaderMap` constructor sets both bits (2013 rva 0x146880, `flags & ~0xF | 0xE`, CompilingId 1) and ours
sets the same.

So this abort is a **consequence of the missing material shader maps**, which now load (2580 maps, 0 mismatches). I cannot
reproduce it here: my snapshot is HEAD + my files, so the run ends in AD's nav mesh `Bad export index` before the map change
commits. **It has to be re-measured on a tree with AD's streaming fix and this package merged** (merge order in PHASE6.md puts
AD before AG). If it still fires afterwards, the next suspects are a material instance whose `Parent` render-thread pointer is
NULL (retail's `GameThread_SetParent`, 2012 rva 0x116980, has no NULL check) and the material instances of the Arkane passes
whose shader types are declared here but never selected.

## 8. Hand-overs (outside my files, not applied)

1. **Agent AH — `ShaderManager.h`:** my rewrite of `MaterialShader.cpp` removed the only user of
   `FSceneTextureShaderParameters::SceneDepthSurfaceParameter` (the mobile unbind at the old line 1002), so the member you kept
   "only so that line compiles" can go.
2. **Agent AH — `Scene.h` / `Scene.cpp`:** `FSceneView::SceneReflectionTexture` (2012 PDB @32, `const UTextureCube*`) is needed
   by `FWorldCubeMapTextureShaderParameters::Set` (2012 0x4715a0), the Arkane cube-map binding inside every material pixel
   shader. Currently stubbed to the white cube.
3. **Agent AH / coordinator — `ShaderCache.h` + `ShaderCache.cpp`:** drop `UShaderCache::ShaderCachePriority` (retail
   `UShaderCache` is 128 bytes, `native_class_sizes.csv:2690`) and the `ShaderCachePriority` / `bIsAlwaysLoaded` members of
   `FCompressedShaderCodeCache` (retail 84 bytes). 11 uses in `ShaderCache.cpp`, 6 in `ShaderCache.h`. Closes agent AB's
   pending row.
4. **Agent AK / coordinator — `LaunchEngineLoop.cpp`:** the Debug allocator choice (follow-up 1).
5. **Coordinator — shared files I edited outside my package list, all "material shader classes only":**
   `ShadowRendering.cpp` (the shadow depth material shader classes and the mod-shadow mesh types; AH's projection edits in the
   same file are untouched and its two `ManualPCF` lines in my `LightRendering.h` are kept),
   `NvApexRenderClasses.h` (one Apex vertex factory parameter class), `LightSceneInfo.cpp` (two `ShadowOverrideFactor` uses of
   the parameter struct I trimmed), `arkbloompartsrendering.cpp` / `arkppnodematerial.cpp` (empty `import_reference.py` stubs;
   the Arkane mesh-material shader types are declared there because that is their retail home).
6. **Power-cut damage (2026-09-26).** `Engine/Src/GPUSkinVertexFactory.cpp` was zero-filled in the working tree and was
   restored with `git checkout --` before my port (it is one of my files). Also zeroed, all regenerable and git-ignored:
   `resources/reference/decomp/agentAJ/r13/*.c` (8 files, agent AJ) and `resources/reference/agentC/test/b6_1.raw`,
   `b6_9.raw`. A full scan of `source/`, `resources/`, `cmake/` found nothing else damaged.

## 9. Diagnostics used (snapshot only, never in the shared tree)

`build\agentAG\diag_patch*.py` patch the snapshot copy of `ShaderCache.cpp` (agent AH's file) to print the first
serialization mismatch of a record — the cooked and our own size lists side by side plus the record's vertex factory. That is
what localised both real bugs (the Apex destructible parameter, then the reference-only vertex factories): e.g.
`mismatch at #49 of 51 (cooked 4, ours 2)` with `ours: … 2 2 >2<` against `cooked: … 2 2 >4 4`, i.e. one parameter too many
before the uniform expression arrays. The snapshot was rebuilt without the diagnostics for the accept runs.
