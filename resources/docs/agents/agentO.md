# Agent O — Milestone 2: startup packages loaded (2026-09-27, resumed 2026-09-28)

Build dir `build\agentO` (Ninja, Debug, x86, `-DDISHONORED_REAL_LAUNCH=ON`); IDA copies
`resources/docs/idb/shipping2012_agentO.i64`, `retail2013_agentO.i64` (headless only). Decompiles and scratch
scripts under `resources/reference/decomp/agentO/` (git-ignored): `2012*/` (2012 by name), `2013_*/` (2013 by string
xref / rva through `match_2012_2013.csv`), `_xref_strings.py`, `_xref_funcs.py`, `_read_globals.py`, `_gen_table.py`
(the SystemSettings table generator), `_patch_globalshader.py`, `_pydbg.py` (ctypes debug loop + `.map` symbolizer for
crashes that leave no stack in `Launch.log`). Smoke outputs: `build\agentO\smoke_<step>.txt`,
`build\agentO\Launch_<step>.log`.

**Every number says which build it comes from**: "2013 rva" = retail `Dishonored_Latest2026` exe (the target,
`retail2013_named.i64`), "2012 rva / 2012 PDB" = the 2012 Shipping build (evidence only).

## Result

**Milestone 2 is reached for the four native packages.** With
`--skip-native GFxUI,AkAudio,OnlineSubsystemPC,OnlineSubsystemSteamworks,DishonoredGame` and the two bring-up switches
`-NoLoadStartupPackages -allowunboundnatives`, our exe (real Launch, null RHI, HEAD `a840c5a` + my files) loads
`Core.upk`, `Engine.upk`, `GameFramework.upk`, `IpDrv.upk` end to end through `ULinkerLoad`
(`CreateLoader` → LZO `SerializeCompressed` → `Link` → `CreateExport` → `Preload` → `PostLoad`) and logs the golden :68
milestone line **`Log: 24107 objects as part of root set at end of initial load.`** (golden: 51634 with all nine
packages + Startup). Acceptance run (exit 0, `build\agentO\smoke_acc7a.txt`, `Launch_acc7a.log`):

```
python resources/tools/build_and_smoke.py --build-dir build/agentO --no-build --rhi null --stage build/agentO/stage
  --milestone "objects as part of root set at end of initial load" --expect "objects as part of root set at end of initial load"
  --skip-native GFxUI,AkAudio,OnlineSubsystemPC,OnlineSubsystemSteamworks,DishonoredGame
  "--extra-args=-NoLoadStartupPackages -allowunboundnatives"
```

The plan's `--expect "Finished loading startup packages"` cannot be met with the retail ini: the line is a
`debugf(NAME_Init, …)` and retail `BaseEngine.ini:336` has `[Core.System] Suppress=Init` (the golden log has no `Init:`
line after the inis are loaded either); the string does not exist in the 2013 exe. The root-set line (`Log:` category,
golden :68) is the visible milestone and is what the runs above check.

| Run | Switches | Result |
|---|---|---|
| 7a | skip list + `-NoLoadStartupPackages -allowunboundnatives` | **exit 0**, 24107 root-set objects; then step 8 `StaticLoadClass(engine-ini:Engine.Engine.GameEngine)` = `DishonoredGame.DishonoredEngine` (retail `DefaultEngine.ini:120`) loads `DishonoredGame.upk` on demand and dies in `UClass::Bind` checkf `Unable to bind DishonoredGame.DisGFxMoviePlayerBase` (its super `GFxMoviePlayer` is in the skipped `GFxUI.upk`) |
| 7b | skip list + `-allowunboundnatives` (Startup.upk loads) | `Startup.upk` dies the same way on `DishonoredGameContent.DisDamageType_Arrow` (super in the skipped `DishonoredGame.upk`) before the root-set line |
| golden-style | skip list only (no bridge) | `Can't bind to native class Engine.AkBaseSoundObject` (`UnClass.cpp:1822`, retail behaviour: the string exists at 2013 rva 0x5d4f0) — the first of the 122 Arkane `Engine.upk`/`GameFramework.upk` native classes whose registrants only agent T's `DishonoredGame` module provides |

So the golden-run acceptance (no bring-up switches) waits for T's registrants (`DISHONORED_ENABLE_DISHONOREDGAME` +
GFxUI/AkAudio/OSS); everything before `UClass::Bind` is in place and measured.

## How the build is set up (the shared tree is mid-edit)

The shared working tree did not compile Engine when I started (Q/R/S in flight) and again after the resume (T's
in-flight `DishonoredGame/inc`). Like agent P I build a **HEAD snapshot worktree** `build\agentO\snap\src`
(`git worktree add --detach`, now at `a840c5a`) into which `build\agentO_sync.py` copies my owned files from the main
tree (list in the script); `build\agentO_configure.cmd` configures it with
`-DDISHONORED_REFERENCE_DIR=D:/RecompileDishonored/UnrealEngine3` and the main tree's FetchContent sources,
`build\agentO_build.cmd` builds, `build\agentO_smoke.py <tag> [args]` = sync + build + `build_and_smoke.py --no-build`.
Smokes run with `--stage build/agentO/stage`: agent P's runs restage `build\stage` with **their** exe underneath mine
(one of my runs measured P's binary). P's uncommitted D3D9Drv CMake wiring is untouched and not in the snapshot
(`DISHONORED_HAVE_D3D9DRV` undefined → the `DishonoredStubs.cpp` null-RHI block compiles, which `-nullrhi` wants anyway).
Note for the coordinator: the snapshot at `921742f` (S only) did not compile Engine (`DishonoredLayouts.h:419`
`FOnlinePlayerScore` 16 vs S's retail 28) — resolved by `a840c5a` (asserts regenerated).

## Step 1 — ini source

Finding that changes the plan: **the retail `FSystemSettings` is not the reference 10897 table design.** The 2012 PDB
has `FSystemSettingsData` (1048 bytes, ten `FSystemSettingsData{WorldDetail,TextureDetail,VSync,ScreenPercentage,
Resolution,MSAA,ShadowDetail,FracturedDetail,Mesh,Audio}` bases), `FSystemSettings : FExec, FSystemSettingsData,
FNoncopyable` = 11,584 bytes (`RenderThreadSettings` @1052, `bIsEditor` @1096, `CurrentSplitScreenLevel` @1100,
`Defaults[5][2]` @1104), and `FSystemSettingsData::LoadFromIni` reads three **local** tables `Switches[43]`,
`IntValues[21]`, `FloatValues[17]` (`resources/reference/decomp/agentO/types_2012_systemsettings.txt`). The 2013 build
is the same design (`sub_5806C0` = `FSystemSettingsData::LoadFromIni`, 2013 rva 0x1806c0, 2,952 bytes, found by the
`"StaticDecals"` xref; 2012 rva 0x186830). I kept the reference's table-driven `FSystemSettings` (all users of
`FSystemSetting`/`FindSystemSetting` in `UnEngine.cpp`/`OpenAutomate.cpp` stay valid) and made its **behaviour**
retail's; the 1048/11,584-byte layout convergence is a later PDB row as the plan says.

| Change | Retail evidence |
|---|---|
| `GetSectionName`: `-SystemSettings=<Name>` (prefix `SystemSettings` stripped) wins, else `SystemSettingsEditor` / `SystemSettings`; no `simmobile`, no mobile sections, no compat bucket | 2013 rva 0x16edb0 (`GetSectionName`, named), 2012 rva 0x178e20 |
| `LoadFromIni(Override)`, `Initialize`, `SaveToIni`, `WriteTextureLODGroupToIni`: `GEngineIni` instead of `GSystemSettingsIni` | no `SystemSettings.ini` string in the 2012 or 2013 exe; the 2012 PDB has `GCompatIni`/`GLightmassIni` but no `GSystemSettingsIni`; 2013 `FSystemSettingsData::SaveToIni` (rva 0x181250) writes and flushes `GEngineIni`; `SetDefaultResolutionForDevice` (2013 rva 0x5baa10) writes `[SystemSettings] ResX/ResY` to `GEngineIni` |
| `UnMisc.cpp appInit`: no `appCreateIniNames(GSystemSettingsIni, …)` / `appCheckIniForOutdatedness(GSystemSettingsIni, …)`; the global stays declared (empty) for the reference-only `D3D9HardwareSurvey.cpp`, `MaterialShared.cpp:8165`, `NvApexManager.cpp:237` readers | 2012 `appInit` rva 0xa1000 creates Engine/Compat/Lightmass/… only |
| `DumpTextureLODGroup`: `"\t%s: %s"` (tab) | golden :10-38 (`Log: \tTEXTUREGROUP_World: (…)`); `FSystemSettingsData::DumpTextureLODGroups` 2013 rva 0x17b7f0 = 26 `GetLODGroupString` calls with the `debugf` compiled out |
| `ApplyNewSettings(…, bWriteToIni)`: no `ApplyOverrides`; dumps the texture groups when writing outside the editor | 2013 rva 0x184be0 (named), 2012 rva 0x192c80 |

## Step 2 — the 107-key retail table

`FSystemSettings::SystemSettings[]` regenerated by `_gen_table.py` in the 2013 `LoadFromIni` order: 43 switches
(`GetBool`), 21 ints (`GetInt`), 17 floats (`GetFloat`) = **81 keys**, plus the 26 `TEXTUREGROUP_*` entries read by
`FTextureLODSettings::Initialize` (2013 rva 0x17bb20) = **107**, exactly the merged retail section
(`BaseEngine.ini:728` 107 keys incl. 25 texture groups + `DefaultEngine.ini:579` 24 overrides, 0 new keys). Cross-check
(`smoke_step2`): every table key exists in the ini except **`bEnableVSMShadows`** (`BaseEngine.ini:790`), which the
2013 table does **not** read (dead ini key; the other 11 "retail-only" keys are real). Each row is annotated with its
2013 `FSystemSettingsData` offset. 98 reference-only rows dropped (mobile, APEX, D3D11, MLAA/temporal AA, …; their
members stay in the header). New members (`SystemSettings.h`, each with the 2012 PDB offset and the 2013 key):
`bUseMaxQualityMode` @4, `bUseHighQualityBloom` @68, `bAllowRatsShadow` @76 (2013 only),
`SkeletalLODDistanceFactorMultiplier` @116, `StaticLODDistanceFactorMultiplier` @120, `TextureForcedLODBias` @124,
`iType_AntiAlias` @128 (2013 only; 2012 had `bAllowMLAA` @136), `bAllowD3D10` @136 (2013; @128 in 2012),
`bAllowBetterModulatedShadows` @960, `FoliageDrawRadiusMultiplier` @884, `SpeakerConfiguration` @1048 (2012 PDB
`m_SpeakerConfiguration`). Retail spelling kept where it differs (`MaxMultisamples`).

`LoadFromIni(section, file, bAllowMissing)`: the `checkf` is gone (no "Couldn't find system setting" string in either
exe; the 2013 function counts found values in a `FoundValues` array it allocates itself and never checks); a `warnf`
tagged `DISHONORED(bringup)` lists unfound keys. **Ported in addition** (2013 only, not in the 2012 build):
`HKCU\Software\Arkane\Dishonored` overrides for the switches and ints when `GIsSeekFreePCConsole` and the file is
`GEngineIni` (`REG_DWORD` values named like the keys; a `"Timestamp"` `REG_BINARY` FILETIME of the ini invalidates and
deletes all stored values when the ini changed; floats/texture groups never overridden) — 2013 rva 0x1806c0 second
half. `SystemSettings.cpp` includes `<windows.h>` through `PreWindowsApi.h`/`PostWindowsApi.h` like `UnConsoleTools.cpp`.

`Initialize(bSetupForEditor)` = 2013 rva 0x1844e0 (2012 rva 0x18a660, `systemsettings.cpp:942`): plain
`[SystemSettings]` from `GEngineIni` (never the editor section) → `LoadFromIni(NULL)` (editor-aware section,
`-vsync`/`-novsync`) → `-MAXQUALITYMODE` / `bUseMaxQualityMode` block (constants as decompiled,
`SetSceneColorBufferFormat(PF_A32B32G32R32F)`) → `ApplySystemSettingsToRenderThread`. **Not ported** (needs the
`FSystemSettingsData` copies): `Defaults[i][0]` ← `[AppCompatBucket<i+1>]` of `GCompatIni` (`DefaultCompat.ini:5-38`) /
`Defaults[i][1]` ← `[SystemSettingsSplitScreen2]`; only `SetCompatibilityLevelWindows` (2013 rva 0x5b5070, D3D9Drv) and
`Exec` read them. No `ApplyOverrides` (`-SS:`/`-LODBIAS:`/`-MAXLOD:`/`-MSAA` are reference-only).

## Step 3 — `[TextureStreaming] MinTextureResidentMipCount`

Neither exe has the string; the retail `[TextureStreaming]` (`BaseEngine.ini:401`) has no such key. The read is
removed; `RHI.cpp` initializes `GMinTextureResidentMipCount = 7` (2012 `.data` rva 0xe2d470 = 7 via `_read_globals.py`;
the 2013 global is unnamed). Smoke `step3` reaches `Log: Shader platform (RHI): PC-D3D-SM3` (golden :39).

## Step 4 — `-nullrhi`

`RHIInit` 2013 rva 0xe6b00 (named): `-nullrhi || GIsUCC || Token == "SERVER"` → `NullCreateRHI`, `GUsingNullRHI = 1`,
else `D3D9CreateRHI` — the reference `DynamicRHI.cpp:53` path, no `bForceD3D11`; nothing to change for the smoke.
`GUsingNullRHI` 2013 `.data` rva 0x1041834. `NullRHI` reports `SP_PCD3D_SM3` (log line above).

## Step 5 — global shader map and the shader compiler

The death after step 3 was the reference `FShaderCompilingThreadManager` ctor (`ShaderCompiler.cpp:452`, `verify` on
`[DevOptions.Shaders] bAllowDistributedShaderCompileForBuildPCS`). Retail has **no shader compiler at all** — no
`FShaderCompilingThreadManager` function in either symbol table, no `"DevOptions.Shaders"` /
`"bAllowMultiThreadedShaderCompile"` / `"PrecompileShadersJobThreshold"` / `"Missing global shader"` /
`"Verifying Global Shaders"` string in the 2013 exe. The retail `BaseEngine.ini:502` section carries eight of the ten
keys; the two missing reads (`bAllowDistributedShaderCompileForBuildPCS`, `PrecompileShadersJobThreshold`) are plain
`GetBool`/`GetInt` now (defaults FALSE/0); the manager is reference-only scaffolding I left in place (P's finding 1).

`VerifyGlobalShaders` = 2013 rva 0x105690 (2012 rva 0x104be0, 83 bytes): `GetGlobalShaderMap(Platform);
GGlobalShaderMap[Platform]->BeginInit();` — no missing-type check, no compile, no cache re-save; ported as the body,
the reference compile path stays under `#if 0` (`_patch_globalshader.py`). `GetGlobalShaderMap` 2013 rva 0x107220 is
the reference. `FShaderCache::Load` 2013 rva 0x164380 = the reference (version gates 538/711/786/672). `Init()` calls
`GetGlobalShaderMap()` before `LoadStartupPackages()` (2013 rva 0x5e11b0) and it now loads
`GlobalShaderCache-PC-D3D-SM3.bin` without complaint.

**Shader source hashing (found on the way).** Reference `VerifyShaderSourceFiles` (`LaunchEngineLoop.cpp:3546`) and the
`GetSourceHash` family want `Engine\Shaders\*.usf` / `Shaders\Binaries\*.bin`, which retail does not ship
(`Couldn't load shader file 'LensFlareVertexFactory'`). Retail never hashes shader sources: no `GetShaderFileHash` /
`LoadShaderSourceFile` / `GetSourceHash` / `GetShaderIncludes` / `VerifyShaderSourceFiles` function in either symbol
table, no `".usf"` / `"Couldn't load shader file"` / `"MaterialTemplate"` / `"AutoReloadChangedShaders"` string in the
2013 exe. `operator<<(FArchive&, FVertexFactoryParameterRef&)` 2013 rva 0x388e30 / 2012 rva 0x3a9f90 (identical)
serializes `VFHash` (2012 PDB `FVertexFactoryParameterRef` @8, 28 bytes; `FShader::Hash` @72) and compares it with a
**static** hash — the reference `CONSOLE` `GetSourceHash` (static zero) with the non-CONSOLE serialization.
`ShouldReloadChangedShaders` 2013 rva 0x127d40 / 2012 rva 0x12bbc0 (25 bytes) returns a static that is never set (no
ini read; `BaseEngine.ini:503 AutoReloadChangedShaders=True` would otherwise reject every cached shader). Ported:
`FVertexFactoryType::GetSourceHash`, `FShaderType::GetSourceHash` return the static zero, the
`FVertexFactoryParameterRef` ctor no longer hashes, `ShouldReloadChangedShaders` keeps FALSE, the `VerifyShaderSourceFiles`
call is `#if 0` (`VertexFactory.cpp`, `ShaderManager.cpp`, `MaterialShared.cpp`, `LaunchEngineLoop.cpp`).
`LoadShaderSourceFile`/`GetShaderFileHash` stay for the reference-only compile paths.

## Step 6 — ports before the first package load

- `FAsyncIOSystemBase::FAsyncIORequest::Event` (`UnIOBase.h`): member order `Dest, Counter, Event, CompressionFlags,
  Priority, RequestType` = 2012 PDB @36/40/44/48/52/56 (64 bytes). 2013 `QueueIORequest` (rva 0x519b0, unnamed
  `sub_4519B0`, 421 bytes vs 346 in 2012) stores them at @52/56/60/64/68 of a **76-byte** request: 2013 inserts a second
  `FString` @24 (the file name normalized by `sub_4470E0`, used as the handle-cache key by `sub_474130`) — not ported,
  layout-pass row.
- `FIOSystem::LoadDataWithEvent` (pure virtual) + `FAsyncIOSystemBase::LoadDataWithEvent` = 2013 rva 0x51e00 / 2012 rva
  0x50510 (`unasyncloading.cpp:1356`): `QueueIORequest(…, Counter NULL, Event, Priority, RequestType)`; `QueueIORequest`
  takes `FEvent* Event` after `Counter` (both builds); `Tick` triggers `IORequest.Event` after decrementing the counter
  (2013 rva 0x74290). Exercised by `bSerializeStartupPackagesFromMemory=TRUE` → `AsyncPreloadPackageList` in every run
  above (all four packages are precached through it).
- `ULinkerLoad::StartTextureAllocation` (`Texture2D.cpp`) = 2013 rva 0x17bd40 / 2012 rva 0x193010 (both 419 bytes):
  `WillTextureBeLoaded` per export, nothing pre-allocated (no `CreateResourceMem`), time-limit checks and bookkeeping as
  the reference. `FPackagePrecacheInfo`: agent L's 24 bytes kept (2013 `AsyncPreloadPackage` rva 0x751e0 = 2012 size).

## GC token streams / intrinsic properties on `DISHONORED_SHIM_STATIC` members (coordinator finding 2)

`UnObjGC.h:47` from `ULevel::StaticConstructor` (`UnLevel.cpp:266`): `STRUCT_OFFSET` on an inline-static member. All
`StaticConstructor`/`CPP_PROPERTY` uses of a shim static were found by script; three real classes, all fixed from the
decompiled token streams and checked against `build/coord/layout_probe.txt`:

| Class | 2013 / 2012 decompile | Fix |
|---|---|---|
| `ULevel` | rva 0x242c20 / 0x25cdd0 (`unlevel.cpp:209`, identical): tokens @140,144,156,616..636,664,**676**; floats @356/360 | last array token = `m_CrossLevelReferencedActors` (2012 PDB @676), not the reference-only `CoverLinkRefs` |
| `UWorld` | rva 0x380e90 / 0x3a23e0 (`unworld.cpp:81`, identical): @72,76,60,80,84,88,296,332,336,660,360,372,**688** | no `NetDriver` (@204), `SaveGameSummary_DEPRECATED`, `DemoRecDriver`, `PeerNetDriver`, `RedirectNetDriver`, `AnimTreePool` tokens; `m_pAudioSystem` (@688) added |
| `UStaticMesh` | rva 0x37a050 / 0x39b5b0: props @192..220 (8 bools), 104/108, 80/84, 224, **304** (`m_bTransparentForVisionChecks`, Arkane), LOD structs, `LODInfo` @68, `BodySetup` @136, 260/272 | dropped `bPerLODStaticLightingForInstancing`, `ConsolePreallocateInstanceCount`, `bStripComplexCollisionForConsole`, `FoliageDefaultSettings`; added `m_bTransparentForVisionChecks` |

## Serializer and PostLoad deltas hit while loading `Engine.upk` (all 2013-verified)

| Site | Symptom | Retail evidence and fix |
|---|---|---|
| `UStaticMeshComponent::Serialize` (`UnStaticMesh.cpp`) | `Engine.Default__DynamicSMActor:StaticMeshComponent0: Serial size mismatch: Got 85, Expected 81` | 2013 rva 0x377550 / 2012 rva 0x398dd0 (81 bytes, identical): `Super`, `LODData`, `Ver<600` lightmap fixup only; the reference `VertexPositionVersionNumber` dummy INT (801 <= Ver < 820) does not exist in Dishonored's 801 packages — removed |
| `UPrimitiveComponent::Serialize` (`PrimitiveComponent.cpp`) | (no crash; Arkane addition) | 2013 rva 0x130bc0 / 2012 rva 0x134d10: `Ver<769 && IsLoading` → `ReflectionChannels` (@308) from the class default object — added |
| `UMaterialInstance::InitResources` / `FMaterialInstanceResource::GameThread_SetParent` (`MaterialInstance.cpp`) | `check(SafeParent)` in the `UMaterialInstanceConstant` ctor from `CreateExport` (`EngineMaterials.DefaultMaterial` lives in `Startup.upk`, not loaded yet; no `"Failed to load '%s %s'"` string in the 2013 exe either) | 2013 rva 0x113e10 / 2012 rva 0x116a90 (`materialinstance.cpp:117`): no `checkf`, no `bHasQualitySwitch` tail; `GameThread_SetParent` 2012 rva 0x116980 has no NULL check — both removed, NULL parent stored |
| `USkeletalMesh::Serialize` (`UnSkeletalMesh.cpp`) | `Slack>=0` in `TArray<UMaterialInterface*>::Empty` (`Materials` read out of phase) | 2013 rva 0x355260 / 2012 rva 0x375220 (1969 bytes, same shape): Arkane `m_UserBounds` (`FName` @Ver>=759, `FVector` @>=761, radius) **before** `Bounds`, `m_EdgeSkeleton` (@>=775) before `RefSkeleton`, no `ClothingAssets` (APEX) block, `CachedStreamingTextureFactors` gated at **771** (reference 797), no `SourceData` (834), `StripData(PLATFORM_Console)` on client load — ported (members from the 2012 PDB layout already in `UnSkeletalMesh.h`) |
| `UMaterial::CacheResourceShaders` / `PostLoad` (`Material.cpp`), `UMaterialInstance::CacheResourceShaders` / `PostLoad` (`MaterialInstance.cpp`), `UMaterial::CompileStaticPermutation` | `verify([Engine.Engine] bKeepAllMaterialQualityLevelsLoaded)` (four sites) | no such string in either exe; 2013 rva 0x10ff70 / 0x11f880 / 0x11a510 (2012 rva 0x112cc0 / 0x128d80 / 0x11b2f0 / 0x12a970): one `MaterialResources[0]` / `StaticPermutationResources[0]`, `InitShaderMap` only (no `CacheShaders` compile, no "Failed to compile Material" warnings), no quality-level tossing in either `PostLoad` — ported; `MSQ_*` indexing stays at `MSQ_HIGH` (0) |

Bring-up instrumentation kept (tagged `DISHONORED(bringup)`, opt-in): `-logcdoinit` in `ULinkerLoad::Preload`
(`UnLinker.cpp`) traces every class default object re-initialized from a package with its `PropertiesSize` — the tool
that located the light-environment (`S`'s headers, fixed by the merge) and static-mesh-component failures.

## Step 7 — hardcoded native package list, skip filter, bind bridge

`appGetScriptPackageNames` (`LaunchEngineLoop.cpp`): the `GConfig->GetArray("Engine.ScriptPackages", …)` reads are
replaced by the retail lists — no `"Engine.ScriptPackages"` string in the 2012/2013 exe, no such section in the retail
inis. `appGetEngineScriptPackageNames` 2013 rva 0x5def10 (named): `Core, Engine, GFxUI, AkAudio, GameFramework, IpDrv`
(GFxUI/AkAudio **before** GameFramework/IpDrv). `appGetGameNativeScriptPackageNames` 2013 rva 0x5dfb50:
`DishonoredGame`, then unless `-CHECK_NATIVE_CLASS_SIZES`: cooking → per `GCookingTarget` (`0x4` Xbox360
`OnlineSubsystemLive`, `0x8` PS3 `OnlineSubsystemPSN`, `0x40` = `PLATFORM_WindowsConsole` `OnlineSubsystemPC` +
`OnlineSubsystemSteamworks`, else `appErrorf("unsupported platform %d")`), each only if `FindPackageFile` finds it
(else `appErrorf("File not found %s")`); runtime → `"OnlineSubsystem" + appGetOSSPackageName()` (= `Steamworks`) if the
file exists. The 2013 `LoadStartupPackages` (rva 0x5e0e10) is the reference (`bSerializeStartupPackagesFromMemory`,
`_LOC` insertion, `-NoLoadStartupPackages`, `AsyncPreloadPackageList`, `LoadPackageList`, `ResetLoaders`).
`-skipnativepkgs=A,B` (`DISHONORED(bringup)`, logs each skip) at the end of `appGetScriptPackageNames`;
`-allowunboundnatives` (`DISHONORED(bringup)`) in `UClass::Bind` clears `RF_Native|CLASS_Native` and inherits the
super's constructor instead of the retail `appErrorf` (string at 2013 rva 0x5d4f0). Both are measurement-only.

Pre-check of the `RF_Native` exports (from `script_classes_2013.json`) against our `IMPLEMENT_CLASS` registrants
(`build/agentO/missing_registrants.json`): Core 9/9, IpDrv 2/2, GameFramework 34 native → **9 missing** (`GameCrowd*`,
`GameDamageType`, `GameDecal*`, …), Engine 898 native → **113 missing** (the Arkane classes cooked into `Engine.upk`:
`Ak*`, `ArkPpNode*`, `Ark*`, `Dis*`, Arkane `InterpTrack*`, `UIState*`, `Spawner`, `WaterVolume`, …). All 122 are
registered only by agent T's `DishonoredGame/Src/DishonoredGameRegistrants.cpp` (`DishonoredGameEngineShims.h`,
"shim: Engine package"), option OFF and in flight — hence the bridge for the measurement runs.

## Golden diff observations (7a run, cut at golden :68)

59 golden-only / 56 ours-only lines. Besides agent N's milestone-1 differences (`DevConfig`/`DevStats`/`Init:` header
lines, `Memory total` format, `Steam Client API Disabled!`): the golden :10-38 `TEXTUREGROUP_*` dump is missing in
ours (it comes from `SetCompatibilityLevelWindows`, 2013 rva 0x5b5070 → `ApplyNewSettings(…, TRUE)` on the first-run
compat path — P's `CompatibilityEvaluator.cpp` stub + the deferred `FSystemSettingsData` defaults); golden :40-41
`Ply_Player.Skm_Player has invalid UserBounds / has no LOD` and the 26 `ImportText (m_MapConfig/m_MissionsGame)` errors
(:42-67) come from `DishonoredGame.upk` / `Startup.upk`, which the 7a run skips; our extra lines are the
`DISHONORED(bringup)` traces (dropped by `normalize_log.py`) and `LocalizationWarning`s for `Engine.Default__*`
properties whose localized values sit in `DishonoredGame`'s localization files. `Log: Shader platform (RHI): PC-D3D-SM3`
and the root-set line (count aside) are identical.

## Files touched

Mine per the plan: `Engine/Src/SystemSettings.cpp`, `Engine/Inc/SystemSettings.h`, `Core/Src/UnMisc.cpp`,
`Launch/Src/LaunchEngineLoop.cpp`, `Core/Src/UnAsyncLoading.cpp`, `Core/Inc/UnIOBase.h`, `Engine/Src/Texture2D.cpp`,
`Core/Src/UnLinker.cpp` (`-logcdoinit` trace only), `Core/Src/UnClass.cpp` (`-allowunboundnatives`).
Loader/bring-up fixes hit on the way (outside the list, every hunk tagged with its evidence): `Engine/Src/RHI.cpp`,
`GlobalShader.cpp`, `ShaderCompiler.cpp`, `UnLevel.cpp`, `UnWorld.cpp`, `UnStaticMesh.cpp`, `VertexFactory.cpp`,
`ShaderManager.cpp`, `MaterialShared.cpp`, `PrimitiveComponent.cpp`, `MaterialInstance.cpp`, `UnSkeletalMesh.cpp`,
`Material.cpp`. Not touched: `Launch/Src/DishonoredStubs.cpp` (P's block kept). `Engine/Inc/EngineClasses.h`,
`Scene.h`, `ShowFlags.h` in `git status` are **not mine** (other agents' in-flight edits). Tools:
`build/agentO_*.{cmd,py}` (scratch under `build/`).

## What is left / next blocker

1. **Golden run without the bridge** stops at `Can't bind to native class Engine.AkBaseSoundObject`: needs T's
   `DishonoredGame` (+GFxUI/AkAudio/OSS) registrants merged and `--skip-native` dropped; then `Startup.upk` (7b) and
   the 51634 root-set count, `Initializing Engine...` (golden :70).
2. **Step 8** with the bridge: `StaticLoadClass(engine-ini:Engine.Engine.GameEngine)` = `DishonoredGame.DishonoredEngine`
   loads `DishonoredGame.upk` on demand and dies in `UClass::Bind` on `DisGFxMoviePlayerBase` (super in the skipped
   `GFxUI.upk`) — same dependency.
3. `FSystemSettings` layout convergence (`FSystemSettingsData` 1048 / `FSystemSettings` 11,584, 2012 PDB) with the
   `Defaults[5][2]` compat/split-screen fill and `SetCompatibilityLevelWindows` (P's file): needed for the golden :10-38
   dump and the compat buckets.
4. `FAsyncIORequest` 2013 layout: 76 bytes with a normalized-name `FString` @24 (2013 only); layout pass.
5. `build_and_smoke.py`: `--expect "Finished loading startup packages"` in PHASE4.md must become the root-set line
   (`Suppress=Init` in the retail ini); per-agent `--stage` (or a build-dir default) avoids two agents running each
   other's exe from `build\stage`.
