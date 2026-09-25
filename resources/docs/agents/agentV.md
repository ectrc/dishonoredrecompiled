# Agent V report — EShowFlags is a QWORD (2026-09-25)

Build dir `build\agentV` (tools, logs, decompiles) + detached worktree `build\agentV_wt` (HEAD `add99ea` + my three
headers, configured into `build\agentV_wt\build\wt`: Ninja, Debug, `cmake\toolchain-x86.cmake`, `-DDISHONORED_REAL_LAUNCH=ON`,
layout checks at their default). IDA copies `resources/docs/idb/shipping2012_agentV.i64` (2012 Shipping, PDB names) and
`retail2013_agentV.i64` (2013 retail, matched names). **The retail 2013 exe is the target**; every bit below was read in the
2012 exe (typed decompile: `View->Family->ShowFlags & 0x...`) and re-read in the 2013 exe (untyped decompile:
`*(_DWORD*)(*(_DWORD*)View + 24/28) & 0x...`, i.e. `FSceneViewFamily::ShowFlags` low/high dword) — "2012 rva" / "2013 rva"
say which build a number comes from.

## Result

| Check | State |
|---|---|
| `EShowFlags` | `typedef QWORD EShowFlags;` (`Engine/Inc/ShowFlags.h`), 8 bytes; the reference `TStaticBitArray<128>` (16) and the console `FShippingShowFlags` wrapper are gone |
| `SHOW_*` | 90 constants in `Engine/Inc/Scene.h`: 62 real bits (each with its 2012/2013 witness), 8 Dishonored-era flags the exes never test (0, `DISHONORED(port)`), 14 reference-only flags (0, `DISHONORED(port)`), `SHOW_RESERVED_FLAG` 0, +1 Dishonored-only flag `SHOW_Foliage` (bit 44) |
| `UGameViewportClient` | 292 -> 284 = retail (`ShowFlags` @96, 8 bytes; `LoadingMessage` @104); `FShowFlags_Mirror` (EngineClasses.h) one QWORD so its `checkAtCompileTime` holds |
| `FSceneViewFamily` / `FSceneViewFamilyContext` | 84 -> 80 = 2012 PDB (`ShowFlags` @24) |
| Engine build (worktree) | **0 errors** on the first build, no Src edit needed (`build\agentV\engine_build2.log`, 620 units) |
| probe / xcheck / compare / CoreSmoke / smoke | see "Checks" below |

## Evidence: Dishonored's show flags are the pre-2011 UE3 64-bit set

- 2012 PDB (`types.json`): `UGameViewportClient::ShowFlags` unsigned __int64 @96, `FSceneViewFamily::ShowFlags` unsigned
  __int64 @24 (sizeof 80), `USceneCaptureComponent::GetSceneShowFlags` returns unsigned __int64 (2012 rva 0x2e65d0, 2013 rva
  0x2cb520). Retail SDK dump: `UGameViewportClient::ShowFlags` `FQWord` @96, sizeof 284. No `EShowFlags`/`SHOW_*` enum or
  global in either PDB/exe: the flags are compile-time QWORD constants (the reference's `appInitShowFlags` globals do not exist).
- The high dword is exactly the old `enum EShowFlags` still present as a fossil in our tree (`Engine/Inc/UnScene.h:315`,
  `SHOW_Editor = 1 ... SHOW_CamFrustums = 0x01000000`) shifted by 32 (17 of its 25 bits witnessed below), continued by
  NavigationNodes / Particles / LightInfluences / BuilderBrush (57-60, witnessed) and TerrainPatches / Cover / ActorTags (61-63,
  inferred from the reference's tail order). The low dword holds the flags Epic added between 2005 and 2010.
- **Neither exe has a `show <flag>` name table**: `UGameViewportClient::Exec` is 97 bytes in both builds (2012 rva 0x2c1560,
  2013 rva 0x2a66c0: `UScriptViewportClient::Exec` -> console -> `GEngine->Exec`), `SetShowFlags`/`VIEWMODE` are compiled out and
  none of the reference table's 52 names exists as a string (`build\agentV\str2012.txt`, `str2013.txt`: only `SystemSettings`
  keys and unrelated hits). The mapping therefore comes from the 2012 rendering code that tests the flags (1,747 + 140 + 60
  decompiled functions, `build\agentV\decomp2012b/c/d`, `decomp2013`), matched to the reference functions of the same name.
- `SHOW_DefaultGame`: the retail `UGameViewportClient` constructor stores `ShowFlags = 0x04062BD2_17403362` (2013 rva 0x2c53c0,
  `mov [esi+60h],17403362h / mov [esi+64h],4062BD2h`; 2012 rva 0x2deee0: `0x04063BD2_17403362`, the extra bit 44 is
  `SHOW_Foliage`, dropped from the 2013 default) = reference `SHOW_ViewMode_Lit | (SHOW_DefaultGame & ~SHOW_ViewMode_Mask)`
  (UnPlayer.cpp:312). `GetSceneShowFlags` starts from `0x0406xBD2_17403262` = the same minus bit 8 (`& ~SHOW_SceneCaptureUpdates`),
  clears bit 43 when `!bEnableFog` and bit 6 when `!bEnablePostProcess` (`and edx,0FFFFF7FFh` / `and eax,0FFFFFFBFh`), and its
  view-mode cases are Unlit = `-Lighting`, Wire = `+Wireframe -Lighting -Materials -PostProcess`, LitNoShadows = `-DynamicShadows`.
  Retail `SHOW_DefaultGame` = Decals, DynamicShadows, PostProcess, SceneCaptureUpdates, Sprites, Lighting, Materials,
  InstancedStaticMeshes, bit 24, LensFlares, LOD, bit 28, Game, Selection, StaticMeshes, Terrain, BSP, SkeletalMeshes, Fog,
  (Foliage 2012 only), BSPTriangles, UnlitTranslucency, Portals, Particles.
- `SHOW_ViewMode_Lit` is used as one constant `0x00002000_00403040` (BSPTriangles | InstancedStaticMeshes | Materials | Lighting |
  PostProcess) in `FBrushSceneProxy::DrawDynamicElements` (2012 rva 0x1b8db0) and `DrawRichMesh` (2012 rva 0x302290) = the
  reference's `Scene.cpp` definition. `IsRichView` (2012 rva 0x2e62e0, 2013 rva 0x2cb1d0): `NonRichShowFlags` = `0x04003000`
  (Materials | LOD | Lighting), `RichShowFlags` = `0x08008000_88A84C80` (Wireframe, LevelColoration, BSPSplit, LightComplexity,
  ShaderComplexity, PropertyColoration, MeshEdges, LightInfluences, TextureDensity, LightMapDensity, VertexColors) = the
  reference `UnRenderUtils.cpp:1236-1258` lists. `IsCollisionView` = `& 0x70000` = `SHOW_Collision_Any`.

## Flag table

Reference bit = the `MAKE_SHOW_FLAG*(n)` number of the reference `Scene.h` (bit n-1 of the 128-bit array). "2012" / "2013"
name the witnessing function (rvas: `build\agentV\decomp2012b\*.c` line 2, `decomp2013\*.c` line 2; the main ones:
IsShown 2012 0x13c680 / 2013 0x136650, SetRelevanceForShowBounds 0x12dfc0 / 0x129ba0, FStaticMeshSceneProxy::GetViewRelevance
0x3abde0 / 0x38ae50, DrawDynamicElements 0x3af970 / 0x38fdd0, ShouldDrawCollision 0x3a4ca0 / 0x3838b0, DrawRichMesh
0x302290 / 0x2ec780, RenderDPGBegin 0x4949a0 / 0x46bf00, RenderDPGEnd 0x48cf90 / 0x464290, FSceneRenderer::Render 0x494b00 /
0x46c060, RenderLightShafts 0x459dc0, FModelSceneProxy::GetViewRelevance 0x28b9c0 / 0x271360, FBrushSceneProxy::GetViewRelevance
0x1b8b60 / 0x1aba20, FSpriteSceneProxy::GetViewRelevance 0x5ab500 / 0x17b530, FParticleSystemSceneProxy::GetViewRelevance
0x514bf0 / 0x4e5140, FSkeletalMeshSceneProxy::GetViewRelevance 0x33e870 / 0x31c0a0, FLensFlareSceneProxy::GetViewRelevance
0x4f4620 / 0x4cd430, FDecalSceneProxy::GetViewRelevance 0x10de70 / 0xf4af0, FDrawFrustumSceneProxy::GetViewRelevance 0xe4c80 /
0xe68e0, IsRichView 0x2e62e0 / 0x2cb1d0).

| Flag | ref bit | Dishonored bit | Evidence |
|---|---:|---:|---|
| SHOW_RESERVED_FLAG | 1 | 0 (no-op) | shipping-wrapper artefact; only `UnPlayer.cpp:1996` compares against it |
| SHOW_Lighting | 2 | 12 | RenderDPGEnd `& 0x1000` -> lights; RenderLightShafts; base-pass shader `SetParameters`; IsRichView NonRich |
| SHOW_SceneCaptureUpdates | 3 | 8 | ctor default 0x...3362 vs GetSceneShowFlags 0x...3262 (`SHOW_DefaultGame & ~SHOW_SceneCaptureUpdates`) |
| SHOW_DynamicShadows | 4 | 5 | RenderDominantLightShadowsForBasePass `& 0x20 && bAllowDynamicShadows`; RenderLights; RenderDPGEnd (modulated shadows); GetSceneShowFlags LitNoShadows |
| SHOW_Fog | 5 | 43 | RenderDPGEnd `HIDWORD & 0x800` -> RenderFog; GetSceneShowFlags `and edx,~0x800` when `!bEnableFog`; FTranslucencyDrawingPolicyFactory::DrawDynamicMesh |
| SHOW_PostProcess | 6 | 6 | GetSceneShowFlags `and eax,~0x40` when `!bEnablePostProcess`; FViewInfo ctor; RenderDPGBegin |
| SHOW_Sprites | 7 | 9 | FSpriteSceneProxy::GetViewRelevance `& 0x200`; FArrowSceneProxy (`bTreatAsASprite && !(& 0x200)`) |
| SHOW_LightShafts | 8 | 0 (no-op) | 2012 RenderLightShafts gates on `Lighting && DynamicShadows && !ShaderComplexity` (no own flag) |
| SHOW_Decals | 32 | 1 | HasLitDecals / HasRelevantStaticDecals / HasRelevantDynamicDecals `& 2`; FDecalSceneProxy; RenderDPGEnd decal pass |
| SHOW_InstancedStaticMeshes | 33 | 22 | FInstancedStaticMeshSceneProxy::GetViewRelevance `& 0x400000`; ViewMode_Lit mask |
| SHOW_StaticMeshes | 34 | 38 | FStaticMeshSceneProxy::GetViewRelevance / DrawDynamicElements `HIDWORD & 0x40` |
| SHOW_Terrain | 35 | 39 | in the retail default (bit 39); enum order; no terrain proxy tests it |
| SHOW_BSPTriangles | 36 | 45 | FModelSceneProxy::GetViewRelevance `HIDWORD & 0x2000 && HIDWORD & 0x100` (BSPTriangles && BSP, reference order); ViewMode_Lit mask |
| SHOW_SkeletalMeshes | 37 | 41 | FSkeletalMeshSceneProxy::GetViewRelevance / DrawDynamicElements `HIDWORD & 0x200` |
| SHOW_SpeedTrees | 38 | 0 (no-op) | no `FSpeedTreeSceneProxy` in either exe; not in the retail default; bit not recoverable |
| SHOW_LensFlares | 39 | 25 | FLensFlareSceneProxy::GetViewRelevance / DrawDynamicElements `& 0x2000000` |
| SHOW_LOD | 40 | 26 | DrawRichMesh `!(& 0x4000000) && bSelected && ReplacementPrimitiveMapKey`; IsRichView NonRich `0x4003000` |
| SHOW_Game | 41 | 33 | SetRelevanceForShowBounds / RenderBounds (`SDPG_World` when set); GetOcclusionPercentage; occlusion tracker |
| SHOW_CameraInterpolation | 42 | 0 (no-op) | reference-only |
| SHOW_Particles | 43 | 58 | FParticleSystemSceneProxy::GetViewRelevance / DrawDynamicElements `HIDWORD & 0x4000000` |
| SHOW_BSP | 44 | 40 | FModelSceneProxy (second operand); FBrushSceneProxy |
| SHOW_Materials | 45 | 13 | FStaticMeshSceneProxy::GetViewRelevance `!(& 0x2000)`; particles/lens flares `!Wireframe && Materials`; IsRichView; DrawRichMesh |
| SHOW_MotionBlur | 46 | 24 (inferred) | in the retail default (bit 24), never tested; pre-2011 header order |
| SHOW_ImageGrain | 47 | 0 (no-op) | never tested; not in the retail default; bit not recoverable |
| SHOW_DepthOfField | 48 | 28 (inferred) | in the retail default (bit 28), never tested; pre-2011 header order |
| SHOW_ImageReflections | 49 | 0 (no-op) | reference-only |
| SHOW_SubsurfaceScattering | 50 | 0 (no-op) | reference-only |
| SHOW_LightFunctions | 51 | 0 (no-op) | reference-only |
| SHOW_Tessellation | 52 | 0 (no-op) | reference-only |
| SHOW_UnlitTranslucency | 53 | 49 | RenderDPGEnd `HIDWORD & 0x20000` gates RenderDistortion and RenderTranslucency (reference `bRenderUnlitTranslucency`) |
| SHOW_TranslucencyDoF | 54 | 0 (no-op) | reference-only |
| SHOW_SSAO | 55 | 0 (no-op) | reference-only |
| SHOW_DecalInfo | 64 | 2 | FDecalSceneProxy::RequiresOcclusion `& 4`; GetViewRelevance `& 4 && (GIsGame \|\| bSelected)` |
| SHOW_LightRadius | 65 | 3 | FDrawConeSceneProxy::GetViewRelevance `& 8` (reference PrimitiveComponent.cpp:2967) |
| SHOW_AudioRadius | 66 | 0 (no-op) | no sound-radius proxy in either exe; bit not recoverable |
| SHOW_Wireframe | 67 | 11 | FSceneRenderer::Render `bIsWireframe = & 0x800`; FinishRenderViewTarget; DrawRichMesh; every particle renderer |
| SHOW_LightComplexity | 68 | 14 | DrawRichMesh `& 0x4000` branch; IsRichView Rich |
| SHOW_Brushes | 69 | 0 (no-op) | never tested (2012 FBrushSceneProxy::GetViewRelevance has no `bBSPVisible && bBrushesVisible` term); bit not recoverable |
| SHOW_LevelColoration | 71 | 10 | FStaticMeshSceneProxy::DrawDynamicElements, FSpriteSceneProxy::DrawDynamicElements, FBrushSceneProxy::DrawDynamicElements, DrawRichMesh `& 0x400` |
| SHOW_BSPSplit | 72 | 7 | DrawRichMesh `& 0x80 && PrimitiveInfo->Component`; IsRichView Rich |
| SHOW_CollisionNonZeroExtent | 73 | 16 | FStaticMeshSceneProxy::ShouldDrawCollision / ShouldDrawSimpleCollision, FBrushSceneProxy::ShouldDrawCollision `& 0x10000`; IsCollisionView `& 0x70000` |
| SHOW_CollisionZeroExtent | 74 | 17 | same, `& 0x20000` |
| SHOW_CollisionRigidBody | 75 | 18 | same, `& 0x40000` |
| SHOW_PropertyColoration | 76 | 19 | same four functions as LevelColoration, `& 0x80000` |
| SHOW_StreamingBounds | 77 | 0 (no-op) | never tested; bit not recoverable |
| SHOW_TextureDensity | 78 | 21 | RenderDPGBegin `& 0x200000 && AllowDebugViewmodes` -> RenderTextureDensities; IsRichView |
| SHOW_ShaderComplexity | 79 | 23 | RenderDistortion, FDistortionPrimSet::DrawScreenDistort, RenderLightShafts, FinishRenderViewTarget `& 0x800000` |
| SHOW_LightMapDensity | 80 | 27 | RenderDPGBegin `& 0x8000000 && !(& 0x40)` -> RenderLightMapDensities; DrawRichMesh; IsRichView |
| SHOW_SentinelStats | 81 | 0 (no-op) | never tested; bit not recoverable |
| SHOW_Splines | 82 | 30 | FSplineSceneProxy::GetViewRelevance `& 0x40000000` |
| SHOW_VertexColors | 83 | 31 | FStaticMeshSceneProxy::DrawDynamicElements `SLODWORD(ShowFlags) >= 0 \|\| !AllowDebugViewmodes`; IsRichView |
| SHOW_Editor | 84 | 32 | IsShown / IsShadowCast `HIDWORD & 1` (else branch = game hiding rules); FViewInfo ctor |
| SHOW_Collision | 85 | 34 | FStaticMeshSceneProxy::GetViewRelevance `HIDWORD & 0x24` (Bounds \| Collision); DebugDrawPhysicsAsset; FBrushSceneProxy |
| SHOW_Grid | 86 | 35 (inferred) | enum order; editor only, never tested |
| SHOW_Selection | 87 | 36 | FModelSceneProxy::DrawDynamicElements, particle / lens-flare renderers `GIsEditor && HIDWORD & 0x10`; in the retail default |
| SHOW_Bounds | 88 | 37 | SetRelevanceForShowBounds / RenderBounds `HIDWORD & 0x20`; FDecalSceneProxy; draw-shape proxies |
| SHOW_Constraints | 89 | 42 | FConstraintDrawSceneProxy::GetViewRelevance, DebugDrawPhysicsAsset `HIDWORD & 0x400` |
| SHOW_Paths | 90 | 46 | FPathRenderingSceneProxy / FNavMeshRenderingSceneProxy / FRouteRenderingSceneProxy::GetViewRelevance `HIDWORD & 0x4000`; UWorld::FixupCrossLevelRefs |
| SHOW_MeshEdges | 91 | 47 | DrawRichMesh `HIDWORD & 0x8000`; IsRichView Rich |
| SHOW_LargeVertices | 92 | 48 (inferred) | enum order; editor only |
| SHOW_HitProxies | 93 | 51 | InitViews (no occlusion queries), FFluidSurfaceSceneProxy::DrawDynamicElements, every `TDynamicPrimitiveDrawer<>` factory `HIDWORD & 0x80000` |
| SHOW_ShadowFrustums | 94 | 52 | InitProjectedShadowVisibility `HIDWORD & 0x100000` -> RenderFrustumWireframe (the only frustum branch in 2012) |
| SHOW_ModeWidgets | 95 | 53 (inferred) | enum order; editor only |
| SHOW_KismetRefs | 96 | 54 (inferred) | enum order; editor only |
| SHOW_Volumes | 97 | 55 | FLevelGridVolumeRenderingSceneProxy::GetViewRelevance; FBrushSceneProxy::GetViewRelevance `Volumes \| Game` = `0x0080_0002_0000_0000` |
| SHOW_CamFrustums | 98 | 56 | FDrawFrustumSceneProxy::GetViewRelevance 2013 `HIDWORD & 0x1000000` (2012: same instruction, IDA renders the immediate as `offset s_UClassContainer...`) |
| SHOW_NavigationNodes | 99 | 57 | IsShown / IsShadowCast `bIsNavigationPoint && !(HIDWORD & 0x2000000)` |
| SHOW_LightInfluences | 100 | 59 | IsRichView Rich `HIDWORD 0x8008000` |
| SHOW_BuilderBrush | 101 | 60 | FBrushSceneProxy::GetViewRelevance `bBuilder && HIDWORD & 0x10000000` |
| SHOW_TerrainPatches | 102 | 61 (inferred) | tail order; editor only |
| SHOW_Cover | 103 | 62 (inferred) | tail order; no cover proxy in either exe |
| SHOW_ActorTags | 104 | 63 (inferred) | tail order; editor only |
| SHOW_VisualizeDOFLayers | 105 | 0 (no-op) | reference-only |
| SHOW_PreShadowFrustums | 106 | 0 (no-op) | reference-only (2012 has one frustum branch) |
| SHOW_TemporalAA | 107 | 0 (no-op) | reference-only |
| SHOW_PreShadowCasters | 108 | 0 (no-op) | reference-only |
| SHOW_VisualizeSSAO | 109 | 0 (no-op) | reference-only |
| SHOW_PostProcessAA | 110 | 0 (no-op) | reference-only |
| SHOW_Foliage | — | 44 (Dishonored-only) | FFoliageSceneProxy::GetViewRelevance / DrawDynamicElements `HIDWORD & 0x1000` (2012 rva 0x5156c0); in the 2012 default, not the 2013 one |
| SHOW_Portals | — (legacy) | 50 | in the retail default (bit 50); enum order (`UnLevelVisibility.cpp:352` is legacy code) |

Free low-dword bits with no test in either exe: 0, 4, 15, 20, 29 — the six unrecoverable flags (Brushes, AudioRadius,
StreamingBounds, SpeedTrees, ImageGrain, SentinelStats) live there in Epic's old header, but no instruction in the 2012 or 2013
exe distinguishes them, so they are 0 rather than guessed. The two inferred low bits (24, 28) are set in the retail default;
the runtime effect of the choice is nil (nothing in the exe tests them; our reference code sees both "on" by default, which
is what Dishonored's renderer does anyway).

## What changed

| File | Change |
|---|---|
| `Engine/Inc/ShowFlags.h` | `typedef QWORD EShowFlags;` with the evidence comment; the `TStaticBitArray<128>` typedef and the `CONSOLE && FINAL_RELEASE` `FShippingShowFlags` wrapper deleted (`DISHONORED(layout)`) |
| `Engine/Inc/Scene.h` | the `MAKE_SHOW_FLAG*` macros and the 88 `SHOW_*` definitions replaced by the table above as `ULL` literals (every line carries its witness; no-ops list their reference uses); the `extern EShowFlags SHOW_ViewMode_*` / `SHOW_DefaultGame` / `SHOW_Collision_Any` / `SHOW_EditorOnly_Mask` globals and `Scene.cpp`'s `appInitShowFlags` are untouched (they still compile and OR the no-ops harmlessly) |
| `Engine/Inc/EngineClasses.h` | `FShowFlags_Mirror` one QWORD instead of two (`DISHONORED(layout)`): the retail `.uc` declares `ShowFlags` as a `qword` (SDK `FQWord` @96), and `UGameViewportClient`'s `checkAtCompileTime(sizeof(FShowFlags_Mirror) == sizeof(EShowFlags))` holds again |
| Engine/Src | **nothing**: with a QWORD every `&`, `\|`, `~`, `^`, `==`, `!=`, `&=`, `\|=` and bool use of the reference compiles unchanged (only `E_ForceInit` constructions existed, all inside the replaced macros). The three `UBOOL x = (ShowFlags & SHOW_X)` sites are all followed by `&& ...` (bool), so no high-dword truncation |

Not done on purpose (outside the package, one-liners for the coordinator):
- `Scene.cpp` `appInitShowFlags`: the reference `SHOW_DefaultGame` lacks `SHOW_Selection | SHOW_Portals` that the retail default
  has (both in the ctor constant), and includes `SHOW_SpeedTrees | SHOW_ImageGrain` + the reference-only flags (0 now). To make
  `UGameViewportClient::ShowFlags` bit-identical to retail add `| SHOW_Selection | SHOW_Portals` to `SHOW_DefaultGame_Part1`
  (`DISHONORED(retail): 2013 rva 0x2c53c0`). `UnPlayer.cpp:312` then matches the ctor constant exactly.
- `UnScene.h` (fossil enum) is included by nothing; it is the ordering witness for bits 32-56 and can stay.

## Checks

Worktree: `git worktree add --detach build/agentV_wt HEAD`, `build\agentV\sync_worktree.py` copies the three headers over it,
`build\agentV_build.cmd <log> <target>` configures `build\agentV_wt\build\wt` and builds. HEAD's committed
`source/Tests/LayoutProbe` is T's DishonoredGame/GFxUI/AkAudio/OSS probe (its CMakeLists needs those targets, so a default
configure fails); I regenerated `gen_layout_probe.py generate Core Engine` in the main tree, moved the files into the worktree
and restored the main tree (`git checkout -- source/Tests/LayoutProbe`, 0 rows in `git status`). `xcheck` / `compare` rewrite
`resources/docs/types/retail_sdk_delta.md` / `reference_layout_delta.md`; both restored too (copies:
`build\agentV\retail_sdk_delta_agentV.md`, `reference_layout_delta_agentV.md`).

| Check | Result |
|---|---|
| `cmake --build ... --target Engine` | **0 errors** (`engine_build2.log`, 620 units) — no Src edit needed |
| `--target DishonoredGame` (Launch, real loop) / `LayoutProbe` / `CoreSmoke` | 0 errors each (`game_build1.log`, `probe_build1.log`, `coresmoke_build1.log`) |
| probe (`build\agentV\layout_probe.txt`) | `UGameViewportClient` **284** (was 292): `ViewportConsole` @92, `ShowFlags` @96, `LoadingMessage` @104, `SavingMessage` @116; `FSceneViewFamily` / `FSceneViewFamilyContext` 76 (was 84; 2012 PDB 80, see follow-ups); `FSceneCaptureProbe.ShowFlags` @8 (8 bytes) |
| `xcheck_sdk_layout.py build/agentV/layout_probe.txt` | **types=1161 exact=910 mismatching=0 contract_mismatches=0**, exit 0 (HEAD: 1 row, `UGameViewportClient`); `--header EngineClasses.h`: 243 types, 0 differ |
| `gen_layout_probe.py compare build/agentV/layout_probe.txt` | probed=1907 exact=1649 **contract_mismatches=0** (`compare1.log`) |
| `CoreSmoke.exe` (from the repo root) | **99 passed, 0 failed** (`coresmoke_run2.log`; run from elsewhere the `hardcoded_names.csv` test fails on cwd, `coresmoke_run1.log`) |
| `build_and_smoke.py --build-dir build/agentV_wt/build/wt --no-build --retail D:/RecompileDishonored/Dishonored_Latest2026 --stage build/agentV_wt/build/stage` | **exit 0**, `Init: Object subsystem initialized` reached, 81-line Launch.log, diff = the known milestone-1 noise (memory / WinSock / Steam lines) (`smoke1.log`) |

Main-tree footprint: `Engine/Inc/ShowFlags.h`, `Engine/Inc/Scene.h`, `Engine/Inc/EngineClasses.h` (3 lines), this report. The
worktree `build\agentV_wt` is still registered (`git worktree remove build/agentV_wt` when done). No commits, no `git add`.

## Follow-ups

- Porting: reference code gated on the no-op flags is now permanently off (`RenderLightShafts` via `SHOW_LightShafts`,
  light functions, SSAO, temporal AA, image reflections, subsurface scattering, translucency DoF, post-process AA, DOF layer /
  SSAO visualisation, pre-shadow frustums). Where the 2012 exe has the feature without a flag (light shafts: `Lighting &&
  DynamicShadows && !ShaderComplexity`, `GSystemSettings.bAllowLightShafts`), the port must drop the flag test rather than
  invent a bit — each such site is a `DISHONORED(port)` decision when the renderer is ported.
- `USceneCaptureComponent::GetSceneShowFlags` in Dishonored has a fourth view mode (case 0 clears Sprites and Particles;
  1 = Unlit, 2 = LitNoShadows, 3 = Wire) — the retail `ESceneCaptureViewMode` order differs from the reference; for whoever
  ports `UnSceneCapture.cpp`.
- The no-op flags make `SHOW_ViewMode_BrushWireframe == SHOW_ViewMode_Wireframe` and `SHOW_EditorOnly_Mask` lose
  `SHOW_AudioRadius`; editor-only, no game effect.
- `FSceneViewFamily` is 76 now vs 80 in the 2012 PDB: the 4 missing bytes are Arkane's `FLOAT CurrentBendTime` @0 (PDB:
  `CurrentBendTime` @0, `Views` @4, `RenderTarget` @16, `Scene` @20, `ShowFlags` @24; ours starts at `Views`). One member in
  `Scene.h` `class FSceneViewFamily` (outside this package's block) plus its constructor initialisers; `FSceneViewFamilyContext`
  follows. `DishonoredLayouts.h` pending rows `sizeof(UGameViewportClient) == 284` becomes a real assert on the next
  regeneration; the `FSceneViewFamily(Context) == 80` rows need that member first. `FSceneView` (1280 vs ours 1392) is not a
  show-flag problem (it has no `ShowFlags` member in the 2012 PDB) — someone's later row.
