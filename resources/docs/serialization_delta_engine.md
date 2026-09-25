# Serialization convergence report: Engine (wave 1, startup assets)

Agent AA, 2026-09-25. Target: the retail 2013 exe (`retail2013_agentAA.i64`, copy of `retail2013_named.i64`); the 2012
Shipping decompile (`shipping2012_agentAA.i64`) is the readable version of the same function (`match_2012_2013.csv`).
Decompiles: `resources/reference/decomp/agentAA/{2012,2013,2012x,2013x}` (git-ignored). This is the Engine companion of
`serialization_delta_core.md` (agent E).

## 1. Inventory

`Startup.upk` is the seek-free merge of `[Engine.StartupPackages]` (the 11 game packages, `EngineMaterials`, `EngineFonts`,
`EngineDebugMaterials`/`DebugMaterials`): none of them exists as a separate file in `CookedPCConsole`. Export classes
(`build/agentAA/inventory.py`, `read_package_classes.Package`): `Engine.upk` 1,007 classes (mostly class default objects;
assets: 27 Texture2D, 2 TextureCube, 1 Material + 28 expressions, 1 MIC, 1 SkeletalMesh + 9 sockets, 1 AnimTree, 4
AnimNodeSequence, 1 Font), `Startup.upk` 376 classes (428 AnimSequence, 427 Texture2D, 49 ParticleSystem with ~2,000
emitter/LOD/module exports, 99 Material + ~560 expressions, 118 MIC, 43 StaticMesh, 32 SkeletalMesh, 34 AnimSet, 145
AkEvent, 45 AkBank, 45 RB_BodySetup, 7 PhysicsAsset, 36 DishonoredAnimTree and the Arkane Dis* objects). The class set
plus super classes (1,325 classes) selects 271 native `Serialize`/`PostLoad`/`StaticConstructor`/`InitResource(s)`
functions of the 2012 PDB in module `engine` (`build/agentAA/candidates.csv`: 158 on UObject classes, 113 on F-structs,
mostly shader parameter serializers left for the renderer work); helper serializers the ported functions call were
decompiled on demand (`build/agentAA/extra201{2,3}.txt`).

Status: `verified` = compared with the 2013 decompile, behaviour identical (a `// DISHONORED(port): 2013 rva ... identical`
line marks it); `ported` = our body changed to the retail shape; `written` = missing in our tree, written from the
decompile; `port` = difference found, not ported (reason given). All rows are in `function_status.csv` (key: 2012 rva).

## 2. Format-level findings (what broke or would break loading)

| Function | Effect in our tree before | Retail |
|---|---|---|
| `UStaticMesh::Serialize` | read `VertexPositionVersionNumber` (reference gate 801 = Dishonored's package version) and `CachedStreamingTextureFactors` at 797: every static mesh 4 bytes out of phase. This is the `TArray<FLOAT>` overrun agent W's run stalled on (`UStaticMesh::Serialize+0x648`) | no version number; factors at 771 |
| `operator<<(FStaticMeshComponentLODInfo&)` | read a `TArray<FVector> VertexColorPositions` for 801 <= Ver < 823: 4 bytes per LOD of every placed static mesh component (maps) | shadow maps, shadow vertex buffers, light map, override colors only |
| `AStaticMeshCollectionActor::Serialize` | no per-component `CachedParentToWorld`: 64 bytes per component unread in every map | matrices after the actor data |
| `operator<<(FPerPolyBoneCollisionData&)` | compact kDOP tree with `RootBound` | legacy `TkDOPTree` (32-byte nodes), 36-byte element |
| `ULightComponent::Serialize` | convex volumes read into dummies (reference gate 829, taken for 801) | read into `InclusionConvexVolumes`/`ExclusionConvexVolumes` |
| `UMaterial::Serialize`, `UMaterialInstance::Serialize` | quality-mask loop (858, dead for 801) and mobile shim copies | one resource |
| `UParticleModule::PostLoad` | seeded-distribution archetype fix-up (reference gate 828) ran on every package and re-parented distributions | editor tangent fix-up only |
| `UParticleSystem::PostLoad` | empty-emitter LOD reset (818) ran on every package | absent |
| Arkane load fix-ups missing | `AActor::Serialize` (whole function), `USeqEvent_Touch::Serialize`, `USkeletalMeshComponent::Serialize` licensee branch, `UParticleModuleEventReceiverSpawn::PostLoad`, `USeqAct_Interp::PostLoad`, `USkeletalMesh::UpdateOriginTransform`, `URB_BodySetup::PostLoad`, `UPrimitiveComponent::PostLoad` translucency sort conversion, `APylon` imported-mesh offset, `ULightComponent` `m_LightProbe`, `UMaterial*` `bHasBloomPart`/`bHasDistortion`, `FLightMap2D` coefficient strip, `ASplineLoftActor` scale reset | written/ported |

Reference version gates between 767 and 801 are live for Dishonored's 801 packages although the QA branch numbering
diverges above 766 (agent E). Engine uses left after this pass: `UnLevel.cpp:336` (`VER_DYNAMICTEXTUREINSTANCES` 797) and
`:448` (`VER_COVERGUIDREFS_IN_ULEVEL` 798) in `ULevel::Serialize` (agent Z's file, not checked here), `UnSkeletalMesh.cpp`
(`VER_ADDED_SKELETAL_MESH_SORTING_LEFTRIGHT_BONE` replaced by retail's 767), shader-type versions (not serialization),
`UnLevAct.cpp:2957`/`UnPostProcess.cpp:251` (`VER_COLORGRADING2` 800 in post-process code outside this pass).

## 3. Per function

### Textures

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `UTexture2D::Serialize` | 0x187fa0 | 0x182440 | 345/345 | ported | no ATITC/ETC/Flash mip caches (reference 857/864, shims), no Android/MOBILE paths; PVRTC mips kept (>=674) |
| `UTexture2D::PostLoad` | 0x18d250 | 0x17c1c0 | 43/43 | verified | identical |
| `UTexture::PostLoad` | 0x18d120 | 0x17baa0 | 117/117 | verified | identical |
| `UTextureCube::PostLoad` | 0x18d480 | 0x17cff0 | 36/36 | verified | identical |
| `UTexture2DComposite::Serialize` | 0x18af50 | 0x185200 | 150/150 | verified | identical |
| `UTextureFlipBook::Serialize` | 0x188440 | 0x182920 | 9/9 | verified | identical |
| `UTextureFlipBook::PostLoad` | 0x18d4b0 | 0x17d050 | 162/162 | verified | identical (UTexture2D::PostLoad inlined) |
| `UTextureMovie::Serialize` | 0x16fa80 | 0x166000 | 70/70 | verified | identical |
| `UTextureMovie::PostLoad` | 0x18d560 | 0x17d100 | 464/464 | verified | identical but for a !GIsBuildMachine test (always FALSE in game) |
| `UTextureRenderTarget2D::Serialize` | 0x16fd00 | 0x166130 | 55/55 | verified | identical |
| `UTextureRenderTarget2D::PostLoad` | 0x18d760 | 0x17d330 | 147/147 | port | Arkane m_ResolutionType (@255, ETrt2dResolutionMode) resizes SizeX/SizeY from GSceneRenderTargets.BufferSizeX/Y >> {0,0,1,2}[type]; member missing from our PROPS block (AB) - not ported |
| `UTextureRenderTargetCube::PostLoad` | 0x18d850 | 0x17d420 | 44/44 | verified | identical (CONSOLE clamp compiled out) |
| `UShadowMapTexture2D::Serialize` | 0x32e9a0 | 0x30c0c0 | 27/27 | verified | identical |
| `USeqAct_StreamInTextures::PostLoad` | 0x32dfc0 | 0x301cc0 | 31/31 | ported | no Finished->Out output link rename |

### Materials and shaders

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `UMaterialInterface::Serialize` | 0x12bb90 | 0x127d10 | 43/43 | ported | mobile parameter rename (855, shims) removed |
| `UMaterial::Serialize` | 0x11a930 | 0x119a00 | 278/278 | ported | one MaterialResources[0], no quality mask (858), no scene-color expression scan |
| `UMaterialExpression::Serialize` | 0x127850 | 0x1169a0 | 125/125 | ported | EditorX/Y_DEPRECATED conversion (576, shims) removed |
| `UMaterialInstance::Serialize` | 0x11dcc0 | 0x11cbc0 | 431/438 | ported | one static permutation resource, no quality mask (858), no mobile texture parameter copy (855, shims) |
| `UMaterialInstance::InitResources` | 0x116a90 | 0x113e10 | 137/137 | ported | agent O's port re-checked |
| `UMaterialInstanceConstant::InitResources` | 0x1268a0 | 0x126f80 | 37/37 | verified | identical |
| `UMaterialInstanceTimeVarying::InitResources` | 0x14fdd0 | 0x144a00 | 37/37 | ported | no linear color parameters (LinearColorParameterValues shim) |
| `FMaterial::Serialize` | 0x14eb00 | 0x146520 | 718/763 | verified | identical; 2013 rebuilds the TextureDependencyLengthMap hash after loading (our TSet serializer does on load) |
| `UMaterial::PostLoad` | 0x128d80 | 0x11f880 | 742/742 | ported | Arkane bHasBloomPart/bHasDistortion from BloomColor/Distortion; no material-function/minimal-compilation checks; CacheResourceShaders without RebuildMaterialFunctionInfo |
| `UMaterialInstance::PostLoad` | 0x12b2b0 | 0x123580 | 427/427 | ported | Arkane bHasBloomPart/bHasDistortion via the instance's static switch values; no bHasQualitySwitch (shim) |
| `UMaterialInstanceConstant::PostLoad` | 0x12b460 | 0x123730 | 239/239 | verified | identical (WITH_MOBILE_RHI block compiled out) |
| `UMaterialInstanceTimeVarying::PostLoad` | 0x1366d0 | 0x132300 | 458/458 | verified | identical |
| `UInterpTrackFloatMaterialParam::PostLoad` | 0x22ded0 | 0x215db0 | 54/54 | ported | no Material_DEPRECATED conversion (shim) |
| `UDecalMaterial::Serialize` | 0xd7c20 | 0xda230 | 59/59 | verified | identical |
| `UDecalMaterial::PostLoad` | 0xd7c10 | 0xda220 | 5/5 | verified | identical |
| `FShader::Serialize` | 0x1695c0 | 0x1603b0 | 474/477 | verified | identical field order |
| `FMaterialShaderMap::Serialize` | 0x432680 | 0x40ef30 | 174/222 | verified | identical |

### Static meshes

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `UStaticMesh::Serialize` | 0x398bf0 | 0x377370 | 476/476 | ported | no VertexPositionVersionNumber read at 801 (4-byte shift of every static mesh), CachedStreamingTextureFactors at 771, legacy kDOP only; no 804..859 members |
| `UStaticMesh::PostLoad` | 0x3a13a0 | 0x37fe20 | 1455/1492 | ported | rebuild below 17, strip on client, InitResources; UV conversion moved to FStaticMeshRenderData::Serialize; no legacy kDOP/Simplygon/editor fixups |
| `FStaticMeshRenderData::Serialize` | 0x3979e0 | 0x376220 | 632/614 | ported | half-float UV conversion here; no adjacency buffer (841) or color-buffer probe (842); no bStripkDOPForConsole shim read |
| `FStaticMeshRenderData::InitResources` | 0x380f60 | 0x35e8b0 | 169/169 | ported | instancing only with indices, no ConsolePreallocateInstanceCount shim, no adjacency buffer |
| `UStaticMeshComponent::PostLoad` | 0x381490 | 0x35ede0 | 16/16 | ported | Super + InitResources only |
| `UStaticMesh::InitResources` | 0x381010 | 0x35e960 | 74/74 | verified | identical |
| `UStaticMeshComponent::InitResources` | 0x381310 | 0x35ec60 | 245/245 | verified | identical |
| `UInstancedStaticMeshComponent::Serialize` | 0x11cb20 | 0x11bd10 | 96/105 | verified | identical for loading (no transacting SelectedInstances) |
| `UStaticMeshComponent::Serialize` | 0x398dd0 | 0x377550 | 81/81 | ported | agent O (no VertexPositionVersionNumber dummy INT) |
| `UStaticMesh::StaticConstructor` | 0x39b5b0 | 0x37a050 | 6064/6090 | ported | agent O (token stream) |
| `operator<<(FStaticMeshComponentLODInfo)` | 0x3977c0 | 0x3760a0 | 382/382 | ported | no VertexColorPositions read for 801..822 (4 bytes per LOD of every placed static mesh component), no PaintedVertices |
| `AStaticMeshCollectionActor::Serialize` | 0x392de0 | 0x3715a0 | 546/548 | ported | per-component CachedParentToWorld matrices read/written (reference dropped them: 64 bytes per component in every map) |

### Skeletal meshes and animation

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `FStaticLODModel::Serialize` | 0x373d70 | 0x354710 | 459/459 | ported | no adjacency container (841), no TRISORT_CustomLeftRight validation; 16-bit index/WORD raw point paths as retail |
| `FStaticLODModel::InitResources` | 0x343c40 | 0x321bf0 | 167/167 | ported | no adjacency container |
| `USkeletalMesh::PostLoad` | 0x372640 | 0x353140 | 1796/1822 | ported | Arkane UpdateOriginTransform (written, 2013 rva 0x30ec00); OLD_TriangleSorting gate 767; no Simplygon/optimization/APEX/alt-influence paths |
| `USkeletalMesh::UpdateOriginTransform` | 0x331560 | 0x30ec00 | 436/436 | written | m_OriginTransform = Translation(Origin) * Rotation(RotOrigin) |
| `USkeletalMesh::InitResources` | 0x343cf0 | 0x321ca0 | 83/83 | verified | identical |
| `USkeletalMeshComponent::Serialize` | 0x336f30 | 0x314750 | 301/301 | ported | licensee < 28 clears bEnableLineCheckWithBounds; no morph/cloth memory counting |
| `ASkeletalMeshActor::PostLoad` | 0x331740 | 0x30ede0 | 50/50 | verified | identical |
| `USkeletalMesh::Serialize` | 0x375220 | 0x355260 | 1969/2044 | ported | agent O (m_UserBounds, m_EdgeSkeleton, 771 gate, no APEX/source data) |
| `operator<<(FPerPolyBoneCollisionData)` | 0x36ade0 | - | 317/- | ported | legacy TkDOPTree (no compact tree / RootBound), no rebuild |
| `UAnimTree::PostLoad` | 0x1c19d0 | 0x1b72b0 | 248/283 | ported | no preview-profile conversion into shims |
| `USkelControlBase::Serialize` | 0x32fdd0 | 0x30cf80 | 74/74 | verified | identical |
| `USkelControlBase::PostLoad` | 0x32fdb0 | 0x30cf60 | 24/24 | verified | identical |
| `UAnimSet::PostLoad` | 0x371f50 | 0x352ad0 | 274/274 | verified | identical (UnSkeletalAnim.cpp is agent W's file: not annotated) |
| `UAnimNotify_Trails::PostLoad` | 0x3430d0 | 0x320d90 | 351/357 | verified | identical (UnSkeletalAnim.cpp is agent W's file: not annotated) |

### Particles

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `UParticleModule::PostLoad` | 0x4e48e0 | 0x4c2f60 | 417/417 | ported | no seeded-distribution archetype fixup (reference 828 ran on every package) |
| `UParticleModuleRequired::PostLoad` | 0x4e56e0 | 0x4c34d0 | 71/71 | verified | identical |
| `UParticleModuleTypeDataMesh::PostLoad` | 0x4e5bb0 | 0x4c3ad0 | 84/68 | verified | identical |
| `UParticleModuleColor::PostLoad` | 0x4e5d60 | 0x4c4fa0 | 5/5 | verified | identical |
| `UParticleModuleTrailSpawn::PostLoad` | 0x51cac0 | 0x4e7870 | 103/103 | verified | identical |
| `UParticleModuleEventReceiverSpawn::PostLoad` | 0x4970e0 | 0x46e1e0 | 47/47 | written | missing in our tree: below 635 copy EventGeneratorType/EventName into the receiver base |
| `UParticleEmitter::PostLoad` | 0x4f10f0 | 0x4c8a70 | 3032/2943 | ported | no MediumDetailSpawnRateScale clamp (shim) |
| `UParticleLODLevel::PostLoad` | 0x4eb590 | 0x4cad10 | 624/624 | verified | identical |
| `UParticleSpriteEmitter::PostLoad` | 0x4f1cd0 | 0x4c95f0 | 119/119 | verified | identical |
| `UParticleSystemComponent::Serialize` | 0x4cd420 | 0x4a9130 | 148/148 | verified | identical |
| `UParticleSystemReplay::Serialize` | 0x4f2780 | 0x4cc120 | 48/48 | verified | identical |
| `UParticleSystemComponent::PostLoad` | 0x4d5fe0 | 0x4b1b60 | 41/41 | ported | no light-environment AddRef, no mobile culling |
| `UParticleSystem::PostLoad` | 0x4ec560 | 0x4c9670 | 2774/2748 | ported | no bHasPhysics collision scan (bApplyPhysics shim), no empty-emitter LOD reset (818), no max-active recalculation (813) |

### Fonts

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `UFont::Serialize` | 0x24cdc0 | 0x23be30 | 123/170 | verified | identical |
| `UFont::PostLoad` | 0x22e790 | 0x2167e0 | 115/115 | verified | identical |
| `UMultiFont::Serialize` | 0x24ce40 | 0x23bee0 | 53/53 | verified | identical |
| `UMultiFont::PostLoad` | 0x22e990 | 0x2169e0 | 21/21 | verified | identical |

### Lights and light maps

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `ULightComponent::Serialize` | 0x124600 | 0x124c10 | 51/51 | ported | convex volumes always serialized into the members (reference: dummies below 829) |
| `ULightComponent::PostLoad` | 0x11a5d0 | 0x116520 | 542/542 | ported | only bForceDynamicLight cleared for dominant lights; Arkane LightingChannels.m_LightProbe = Static||Dynamic (bits m_LightProbe/m_bOutsider added); <564 bounced color not ported (dead) |
| `UDominantDirectionalLightComponent::Serialize` | 0xefbc0 | 0xef850 | 49/49 | verified | identical |
| `UDominantSpotLightComponent::Serialize` | 0x15c0a0 | 0x1532f0 | 49/49 | verified | identical |
| `UPointLightComponent::PostLoad` | 0x139b30 | 0x133520 | 233/233 | verified | identical |
| `USpotLightComponent::PostLoad` | 0x156980 | 0x14d5e0 | 344/344 | verified | identical |
| `USkyLightComponent::PostLoad` | 0x150710 | 0x1473a0 | 5/5 | verified | identical |
| `AStaticLightCollectionActor::Serialize` | 0x270a70 | 0x2573a0 | 542/548 | verified | identical |
| `UDynamicLightEnvironmentComponent::Serialize` | 0xf1110 | 0xf0da0 | 139/139 | verified | identical |
| `UDynamicLightEnvironmentComponent::PostLoad` | 0xdff80 | 0xe1d90 | 26/26 | verified | identical |
| `FLightMap2D::Serialize` | 0x26ab00 | 0x24bab0 | 494/494 | ported | Arkane/retail strip of the unused directional or simple coefficient textures on load |
| `FLightMap::Serialize` | 0x26aa30 | 0x24b970 | 23/23 | verified | identical |
| `FLightMap1D::Serialize` | 0x26ae10 | 0x24bca0 | 372/372 | verified | identical |

### Sequences and matinee

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `USequenceObject::PostLoad` | 0x328f20 | 0x2f34b0 | 32/32 | verified | identical |
| `USequenceOp::PostLoad` | 0x32c480 | 0x2ff070 | 1446/- | verified | identical (2013 rva 0x2ff070 found as the Super call of USeqAct_Gate::PostLoad) |
| `USeqVar_External::PostLoad` | 0x32b9a0 | 0x2f8f10 | 56/56 | verified | identical |
| `USeqAct_Gate::PostLoad` | 0x32de00 | 0x301a00 | 17/17 | verified | identical |
| `USeqAct_Toggle::PostLoad` | 0x32de20 | 0x301a20 | 197/197 | verified | identical |
| `USequence::PostLoad` | 0x32cbc0 | 0x2ff7a0 | 487/437 | ported | SequenceObjects shrunk after the NULL removal |
| `USeqAct_PrepareMapChange::PostLoad` | 0x32dfe0 | 0x301ce0 | 41/41 | ported | UpdateStatus not editor-only |
| `USeqEvent_Touch::Serialize` | 0x2ea610 | 0x2cf670 | 51/51 | written | Arkane: licensee < 30 copies m_bIgnorePossessedPawn into m_bIgnorePossessingPawn |
| `USeqAct_Interp::Serialize` | 0x24e110 | 0x23c820 | 60/107 | verified | identical |
| `USeqAct_Interp::PostLoad` | 0x22fcf0 | 0x218cd0 | 441/441 | written | missing in the reference: archetype output links re-inserted |
| `UInterpData::PostLoad` | 0x22a890 | 0x212930 | 5/5 | ported | Super only (director cache / bake-prune work on shims) |
| `UInterpData::Serialize` | 0x22a850 | 0x2128f0 | 59/59 | port | Arkane: m_iMatineeDataVersion <- m_Data->m_iDataVersion (MarkPackageDirty); UMatineeData lives in DishonoredGame's Engine shims, not portable in Engine yet |
| `UInterpGroup::PostLoad` | 0x23af10 | 0x224770 | 97/97 | ported | no AnimSets conversion (shim) |
| `UInterpTrackAnimControl::PostLoad` | 0x234c30 | 0x21de00 | 195/195 | verified | identical |
| `UPrefabSequence::PostLoad` | 0x32e330 | 0x30ae00 | 592/612 | verified | identical |
| `UInterpCurveEdSetup::Serialize` | 0x22f240 | 0x217960 | 239/239 | verified | identical |
| `UInterpCurveEdSetup::PostLoad` | 0x253200 | 0x22eca0 | 377/377 | verified | identical |

### Actors, components, levels

| Function | 2012 rva | 2013 rva | size 2012/2013 | status | note |
|---|---|---|---|---|---|
| `AActor::Serialize` | 0x171920 | 0x1679b0 | 106/106 | written | missing in our tree: Arkane fixups (bPathColliding from bStatic < 782, zero-extent flag < 783, collision trace update < licensee 26) |
| `AActor::PostLoad` | 0x193910 | 0x17dfd0 | 624/650 | ported | no in-game NULL component removal; Arkane collision-trace fixup and path-colliding fixup; editor: only bHiddenEdTemporary (bHiddenEdScene/Layer are shims) |
| `ABrush::Serialize` | 0x172660 | 0x1686f0 | 9/9 | verified | identical |
| `UBrushComponent::Serialize` | 0x1c1b00 | 0x1b8130 | 38/38 | verified | identical |
| `UModelComponent::Serialize` | 0x2a0df0 | 0x2823c0 | 102/102 | verified | identical |
| `UModelComponent::PostLoad` | 0x2a7f40 | 0x28d960 | 48/48 | verified | identical |
| `AWorldInfo::Serialize` | 0x2729f0 | 0x259350 | 475/419 | ported | no LMLevelSettings/LandscapeInfoMap (shims) |
| `AWorldInfo::PostLoad` | 0x270790 | 0x257120 | 584/481 | ported | no post-process/VisibleLayers fixups (shims), no APEX |
| `URB_BodySetup::PostLoad` | 0x3ce730 | 0x3acc00 | 105/105 | ported | box extents made positive (missing in reference) |
| `URB_BodySetup::Serialize` | 0x3ecd30 | 0x3cb830 | 38/38 | verified | identical |
| `UPhysicsAssetInstance::Serialize` | 0x3ecd60 | 0x3cb860 | 35/80 | verified | identical |
| `UPhysicsAsset::PostLoad` | 0x3eced0 | 0x3cb930 | 48/48 | verified | identical |
| `ACameraActor::PostLoad` | 0x1cf2e0 | 0x1bcfc0 | 53/53 | ported | retail bit bCamOverridePostProcess instead of the _DEPRECATED shim |
| `UDistributionFloatUniform::PostLoad` | 0x1e20a0 | 0x1cb850 | 41/41 | verified | identical |
| `UDistributionVectorUniform::PostLoad` | 0x1e2d20 | 0x1cc3b0 | 70/70 | verified | identical |
| `UDistributionVectorUniformCurve::PostLoad` | 0x1e3b70 | 0x1cda30 | 76/76 | verified | identical |
| `UDistributionVectorUniformCurve::Serialize` | 0x1e3c50 | 0x1cdbe0 | 9/9 | verified | identical |
| `ALevelStreamingVolume::PostLoad` | 0x1943b0 | 0x17ecd0 | 41/41 | verified | identical |
| `APortalTeleporter::PostLoad` | 0x312930 | 0x2df870 | 95/95 | verified | identical |
| `ASceneCaptureActor::PostLoad` | 0x2e6fc0 | 0x2cbee0 | 21/21 | verified | identical |
| `ASceneCaptureReflectActor::PostLoad` | 0x2e7070 | 0x2cbfa0 | 45/45 | verified | identical |
| `APrefabInstance::PostLoad` | 0x2eaca0 | 0x2cfe60 | 49/49 | verified | identical |
| `APrefabInstance::Serialize` | 0x311f90 | 0x644720 | 51/34 | verified | identical |
| `UPrefab::PostLoad` | 0x32bf30 | 0x2fe9b0 | 282/268 | verified | identical |
| `UPrimitiveComponent::PostLoad` | 0x134dc0 | 0x130c70 | 244/244 | ported | Arkane TranslucencySortPriority -> DisTranslucencySortPriority conversion; lighting channel check on bUsePrecomputedShadows |
| `UPrimitiveComponent::Serialize` | 0x134d10 | 0x130bc0 | 174/174 | ported | agent O (ReflectionChannels below 769) |
| `APylon::Serialize` | 0x279ba0 | 0x25f780 | 209/209 | ported | Arkane m_LastNavMeshGeneratedImportedMeshOffset on load |
| `APylon::PostLoad` | 0x2a4c40 | 0x286a10 | 627/659 | ported | no bAllowRecastGenerator (shim) |
| `UNavigationHandle::Serialize` | 0x284b60 | 0x26a380 | 111/111 | verified | identical |
| `ULensFlareComponent::PostLoad` | 0x4f45a0 | 0x4cd3b0 | 5/5 | ported | no NextTraceTime (shim) |
| `ULensFlare::PostLoad` | 0x4fd0b0 | 0x4d3cd0 | 454/454 | verified | identical |
| `UDecalComponent::Serialize` | 0xf4e40 | 0xf5420 | 1107/1107 | verified | identical structure |
| `ABrush::PostLoad` | 0x194110 | 0x17ea30 | 669/669 | verified | identical |
| `ASplineActor::PostLoad` | 0x159820 | 0x150470 | 705/605 | verified | identical |
| `ASplineLoftActor::PostLoad` | 0x15a370 | 0x151090 | 292/306 | ported | Arkane DrawScale/DrawScale3D reset to 1 |
| `UUIDataStore_GameResource::Serialize` | 0x41a590 | 0x3f9b90 | 255/232 | verified | identical |
| `UUIDataStore_DynamicResource::Serialize` | 0x41b130 | 0x3fa760 | 249/226 | verified | identical |
| `USpeedTree::Serialize` | 0x53ac00 | 0x4fc2f0 | 91/91 | verified | identical (WITH_SPEEDTREE=0 branch) |
| `UForceFeedbackWaveform::Serialize` | 0x23f580 | 0x2299b0 | 87/87 | verified | identical (inline in EngineClasses.h) |
## 4. Not covered here

| Functions | Why |
|---|---|
| `UAkBank::PostLoad`, `UAkEvent::PostLoad`, `UMatineeData::Serialize`, `UInterpTrackFaceTo/LookAt/Locomotion/StretchAnimControl::PostLoad`, `UArkComponentContainer::Serialize`, `UUIDynamicFieldProvider::Serialize` | classes of `Engine.upk` that our tree defines only in agent T's `DishonoredGame` shims (`DishonoredGameEngineShims.h`), not in `Engine/Src` |
| `UInterpData::Serialize` (`port`) | retail keeps the matinee data in `UMatineeData` (`m_Data`); needs that class in Engine |
| `UTextureRenderTarget2D::PostLoad` (`port`) | Arkane `m_ResolutionType` (@255, `ETrt2dResolutionMode`) resizes the target from `GSceneRenderTargets.BufferSizeX/Y >> {0,0,1,2}[type]`; the member sits in the `SCRIPT_ALIGN` of our PROPS block (agent AB) |
| `UAnimSequence::Serialize/PostLoad`, Edge codec | agent W |
| `UFaceFX*`, `UApex*`, `USpeedTreeComponent::*` | `WITH_FACEFX=0`, `WITH_APEX=0`, `WITH_SPEEDTREE=0` |
| `USeqAct_Log/Delay`, `UAnimMetaData_SkelControl`, `ANxForceFieldRadial`, `UUIState`, `ALevelGridVolume`, `UApexComponentBase::PostLoad` | 2012 functions without a 2013 match; not located in the 2013 db |
| 113 F-struct `Serialize`/`InitResources` | shader parameter serializers (renderer, agent Y) except the ones listed above |
