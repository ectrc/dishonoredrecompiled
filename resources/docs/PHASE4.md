# Phase 3 wave 2 — plan and tracker (PHASE4.md)

Written 2026-09-27 after wave 1 (`PHASE3.md`) landed. Plan of record: `PLAN.md`. Read `STATUS.md` first.

## Goal of this wave

Wave 1 gave us retail truth (native sizes, package member lists, the named retail IDA db, the
CodeRed SDK dump with runtime offsets), Core/Engine contract types that match retail, and
milestone 1 (our exe logs `Init: Object subsystem initialized` on staged retail content, null RHI).
Wave 2 turns that into a **loading engine**:

1. **Milestone 2**: the retail startup packages load through our `ULinkerLoad` (O).
2. **D3D9Drv as a real module** so the exe stops stubbing the renderer entry points (P).
3. **Engine headers converged on the retail runtime layout**: the 232 `retail_sdk_delta.md` rows,
   split by base-class family (Q, R, S).
4. **The game module exists**: `DishonoredGame` (+ GFxUI / AkAudio / OnlineSubsystemSteamworks)
   registrants and class declarations generated from the SDK dump, so `UClass::Bind` can bind
   every native class the cooked packages reference (T).
5. **Middleware decisions with evidence**: exact Wwise / PhysX / Scaleform / FaceFX / Bink /
   Steamworks versions from the retail binaries and a per-library plan (U).

The **retail 2013 exe is the target**; the 2012 PDB is evidence only. Sizes come from
`native_class_sizes.csv`, reflected offsets from `retail_sdk_layout.json` (`sdk_dump.md`).

## Facts fixed while planning (2026-09-27)

| Fact | Consequence |
|---|---|
| Next crash after milestone 1: `SystemSettings.cpp:532` checkf in `FSystemSettings::LoadFromIni`. Retail ships no `*SystemSettings.ini`; `[SystemSettings]` lives in `BaseEngine.ini`/`DefaultEngine.ini` (GEngineIni). Our table has 169 keys, retail's merged section 107: 99 of ours absent in retail, 12 retail-only. Retail exe strings: no `SystemSettings.ini`, no "Couldn't find system setting" | O rewrites the `FSystemSettings` table + ini source from the 2013 decompile; the checkf goes (retail deleted it) |
| `RHIInit` (DynamicRHI.cpp) picks the null RHI only with `-nullrhi`; `bForceD3D11=TRUE` → the stubbed `D3D9CreateRHI` returns `NullCreateRHI()` with `GUsingNullRHI` FALSE | Smoke runs pass `-nullrhi` (`build_and_smoke.py --rhi null`, default); D3D9Drv becomes a target so the stub goes |
| Retail hardcodes the native script package list (no `[Engine.ScriptPackages]` in retail inis or exe). CookedPCConsole: Core, Engine, GameFramework, IpDrv, GFxUI, AkAudio, OnlineSubsystemPC, OnlineSubsystemSteamworks, DishonoredGame, Startup, Startup_LOC_INT | `appGetScriptPackageNames` gets the retail list; a dev-only `-skipnativepkgs=` filter for bring-up |
| `UClass::Bind` (`Core/Src/UnClass.cpp:1822`) `appErrorf("Can't bind to native class")` for an `RF_Native` class export without a `ClassConstructor` when `!GIsEditor` | Packages whose native classes are stubbed are **fatal**. T's registrants gate the Startup milestone; O measures with the skip list until T merges |
| Retail `BaseEngine.ini`: `[SystemSettings]` :728, `[TextureStreaming]` :401 without `MinTextureResidentMipCount`, `[Engine.StartupPackages]` :250, `bSerializeStartupPackagesFromMemory=TRUE` :251 (→ `AsyncPreloadPackageList` → the unported `LoadDataWithEvent`) | O ports `FAsyncIORequest::Event` (PDB 64 vs ours 60) + `FAsyncIOSystemBase::LoadDataWithEvent` before the first package load |
| `retail_sdk_delta.md`: 232 rows, 211 inside a `//## BEGIN PROPS` block (mechanical with `sdk_props.py`), 21 hand work. Root causes are base classes: `UInterpTrackInst` 56→64, `UInterpGroup` −12 (interface vtable `FInterpEdInputInterface` @56 + `GroupAnimSetsPawn`), `UInterpGroupInst` −32, `UUIRoot`→`UUIDataProvider` 68→84, `ULightComponent` (`UnActorComponent.h`, native header), `UAnimNode` 224→208, `UActorFactory` +12, `UMaterialExpression` +28; too-large classes with reference-only storage: `ACamera` 1328→1040, `UNavigationHandle` 336→228, `UPlayerInput` 604→444, `ULevelStreaming` 224→160, `UOnlineSubsystem` 252→180 | Split by family; converge the base first, rebuild the probe, then the children |
| Package loading (K+L ports, LZO) never ran on a whole package in the exe | Milestone 2 is the first end-to-end test of `CreateLoader` → `SerializeCompressed` → `UClass::Link` |
| Reference `External/Novodex` is NovodeX 2.1.2 (not PhysX 2.8); the GFxUI folder mixes Epic GFx-4 glue with Arkane GFx-3 PDB stubs; no Scaleform/Wwise/FaceFX/Steamworks/Bink SDK anywhere; retail imports `binkw32`, `PhysXLoader`, `steam_api`, `libcurl`, `d3d9`, `xinput1_3`, `dinput8` | U pins versions from the binaries; only the Bink import lib is mechanical now |

## Tools for this wave (coordinator, done before spawning)

| Tool | Use |
|---|---|
| `python resources/tools/sdk/sdk_show.py [--probe build/<dir>/layout_probe.txt] Class…` | retail SDK layout / 2012 PDB / ours side by side, gaps and native-only members marked. Daily tool |
| `python resources/tools/sdk/sdk_props.py [--dry-run\|--print] Engine Class…` (= `gen_layout_probe.py props --sdk`) | regenerates a `//## BEGIN PROPS` block from the retail offsets; fills native gaps from the 2012 PDB by relative position, else `BYTE UnknownDataNN[n]`; reference-only members → `DISHONORED_SHIM_STATIC`; reports interface vtables (`VfTable_*` = inherit the interface) and the native tail after the block. `--print` for classes without a PROPS block; `--survey Engine` classifies the delta rows |
| `python resources/tools/sdk/xcheck_sdk_layout.py build/<dir>/layout_probe.txt --header EngineAnimClasses.h` | per-package check: exit 1 while any type declared in that header differs from retail |
| `python resources/tools/symbols/gen_layout_probe.py compare build/<dir>/layout_probe.txt` | sizes vs retail (`native_class_sizes.csv`), offsets vs SDK then 2012 PDB; contract types must stay 0 |
| `python resources/tools/build_and_smoke.py --build-dir build/<dir> [--no-build] --rhi null --milestone "<golden line>" --expect "<our line>" --skip-native A,B` | build + stage + run + golden diff; `--expect` lines must appear; `DISHONORED(bringup)` log lines never reach the diff |

Verified on `UInterpTrackInst` (retail span 56..64): `sdk_props.py` regenerated the block, the
probe reports 64, and because 25 `UInterpTrackInst*` children inherited the fix the SDK delta went
from 232 to **209 rows** (688 of 1,132 exact) in one step — converge bases first. The row counts per
header below are the pre-fix numbers; regenerate with `xcheck_sdk_layout.py` before starting.

## Work packages (all parallel)

Letters continue wave 1 (H–N used).

### O — Milestone 2: startup packages loaded
Owner: agent O. Build dir `build\agentO` (`-DDISHONORED_REAL_LAUNCH=ON`). IDA copies
`shipping2012_agentO.i64`, `retail2013_agentO.i64` (copy `retail2013_named.i64`).
Files: `Engine/Src/SystemSettings.cpp`, `Engine/Inc/SystemSettings.h`, `Core/Src/UnMisc.cpp`
(`appCreateIniNames`), `Launch/Src/LaunchEngineLoop.cpp` (`appGetScriptPackageNames`,
`LoadStartupPackages`), `Launch/Src/DishonoredStubs.cpp`, `Core/Src/UnAsyncLoading.cpp`,
`Core/Inc/UnIOBase.h`, `Core/Src/UnLinker.cpp` fixes found on the way, `Engine/Src/Texture2D.cpp`
(`StartTextureAllocation`).

Steps (each ends with a smoke run; `--milestone` = golden line for the diff cut, `--expect` = our own line):
- [ ] 1. **Ini source.** `SystemSettings.cpp:700` `Initialize` loads from `GEngineIni` (retail has no
      "SystemSettings.ini" string); same for `SaveToIni`/`WriteTextureLODGroupToIni` (:596/:610-628).
      Leave `GSystemSettingsIni` in `UnMisc.cpp:5125-5141` (`D3D9HardwareSurvey` references it). Find in
      the 2013 db what emits the `TEXTUREGROUP_*` dump lines (golden :10-38) and mirror it.
- [ ] 2. **Table convergence to retail's 107 keys** (not "misses non-fatal"). In the 2013 db xref
      `"StaticDecals"` → the static `SystemSettings[]` data (rows `{type,intent,name,addr,validator,help}`),
      walk the rows for order/type/intent; rewrite the table at `SystemSettings.cpp:39`; add members for
      the 12 retail-only keys (`bAllowD3D10`, `bAllowBetterModulatedShadows`, `bAllowRatsShadow`,
      `bEnableVSMShadows`, `bUseMaxQualityMode`, `FoliageDrawRadiusMultiplier`, `iType_AntiAlias`,
      `SkeletalLODDistanceFactorMultiplier`, `SpeakerConfiguration`, `StaticLODDistanceFactorMultiplier`,
      `TextureForcedLODBias`, `UseHighQualityBloom`) in `SystemSettings.h`; leave the 99 orphaned members
      (the `FSystemSettings` 1088-byte convergence is a later PDB row). Delete the `checkf` (:531) as
      retail did; a `warnf` on `!bFound` tagged `DISHONORED(bringup)` is fine. Check: `Dump` order = golden :10-38.
- [ ] 3. **TextureStreaming key.** Decompile `FSystemSettings::Initialize` in the 2013 db; retail cannot
      `verify` a missing key — port exactly (expect `GetInt` with default `GMinTextureResidentMipCount = 7`).
- [ ] 4. **`-nullrhi`** (smoke default): `GUsingNullRHI=TRUE` (`DynamicRHI.cpp:53`). Milestone
      `Log: Shader platform (RHI): PC-D3D-SM3` (golden :39); confirm `NullRHI` reports `SP_PCD3D_SM3`.
- [ ] 5. **Global shader map** (`LaunchEngineLoop.cpp:3518`, `GlobalShaderCache-PC-D3D-SM3.bin`): under the
      null RHI `GetGlobalShaderMap` should skip verification; if it errors on missing global shader types,
      port the `GUsingNullRHI` early-out from the 2012 decompile (`ShaderCompiler.cpp`), do not stub.
- [ ] 6. **Port before the first package load**: `FAsyncIORequest::Event` (2012 PDB 64 bytes, ours 60) +
      `FAsyncIOSystemBase::LoadDataWithEvent` (2012 rva 0x50510; find the 2013 counterpart in the named db);
      `FPackagePrecacheInfo` retail check; `ULinkerLoad::StartTextureAllocation` (2012 rva 0x193010) when
      textures serialize.
- [ ] 7. **Hardcoded package list** (`LaunchEngineLoop.cpp:862`): xref `"GameFramework"`/`"IpDrv"`/
      `"OnlineSubsystemSteamworks"` in the 2013 db for the literal list and order; replace the
      `GConfig->GetArray` reads with that static array (`DISHONORED(retail)`); add dev-only
      `-skipnativepkgs=A,B` (`DISHONORED(bringup)`).
      (a) `Core,Engine,GameFramework,IpDrv` with `--skip-native GFxUI,AkAudio,OnlineSubsystemPC,OnlineSubsystemSteamworks,DishonoredGame`
      and `-NoLoadStartupPackages` (stock, :1121). Pre-check: the `RF_Native` class exports of each `.upk`
      (fmodel MCP `fmodel_list_exports`, or a small export-table reader) against our registrants
      (`resources/tools/symbols/xcheck_natives.py`) — any missing class trips `UnClass.cpp:1822`.
      `--expect "Finished loading startup packages"` (:1170). First whole-package exercise of K/L's loader.
      (b) drop `-NoLoadStartupPackages` (`EngineMaterials`, `EngineFonts`, … : textures/materials
      serialization and shader-map lookups under the null RHI).
      (c) `DishonoredGame/GFxUI/AkAudio/OSS` wait for T's registrants (coordinator merge). Measurement-only
      bridge, never in the golden run: `-allowunboundnatives` in `UClass::Bind` clearing `RF_Native|CLASS_Native`
      and inheriting the super's ctor. Then `Startup.upk`; milestones `<N> objects as part of root set`
      (the number differs while packages are skipped — keep it visible), then `Initializing Engine...`.
- [ ] 8. `StaticLoadClass(GameEngine)` + `GEngine->Init()` as far as it goes; report the next blocker.
- **Accept:** `python resources/tools/build_and_smoke.py --build-dir build/agentO --rhi null --skip-native … --expect "Finished loading startup packages"`
  exit 0 with the four native packages loaded; golden diff reviewed (RHI/device lines diverge under
  `-nullrhi`: documented as expected); every ini/packaging decision cites retail evidence (db rva or ini line).
- Risks: the 2013 table may hold per-platform rows absent from the ini (verify 107 vs the data walk);
  `FAsyncIORequest` 60 vs 64 is a memory bug if not ported first; an Engine class whose C++ size differs
  from the cooked property layout shows as `UClass::Link`/`StaticAllocateObject` corruption — keep
  `DISHONORED_LAYOUT_CHECKS` on and `xcheck_sdk_layout` green for the classes in `Core.upk`/`Engine.upk`.

### P — D3D9Drv as a module + build follow-ups
Owner: agent P. Build dir `build\agentP`. IDA copies `shipping2012_agentP.i64`, `retail2013_agentP.i64`.
- [ ] `dishonored_module(D3D9Drv)` in the root `CMakeLists.txt` behind `DISHONORED_ENABLE_D3D9DRV` (ON;
      `D3D9Drv/Sources.cmake` exists), links `Dishonored::DirectX9` + `Dishonored::nvapi`; `WITH_APEX=0`
      paths compile; remove the D3D9 stubs from `Launch/Src/DishonoredStubs.cpp:30-63` (keep D3D11/OpenGL);
      Launch links the real `D3D9CreateRHI`. Smoke keeps `-nullrhi` (device creation is milestone 4).
- [ ] Layout probe for D3D9Drv (`gen_layout_probe.py generate D3D9Drv`, `MODULE_INCLUDES` entry) vs the 2012
      PDB (no SDK rows: D3D9Drv has no reflected classes); `DishonoredLayouts.h` for the module. The 269 PDB
      functions are inventoried in `function_status.csv` (status `reference`/`port`), not converged.
- [ ] Follow-ups: `UnPNG.cpp:26` `libpng15.lib` pragma guarded; `NullRHI.cpp` PIX stubs available under
      `USE_DYNAMIC_RHI` (or the real ones from `D3D9Util.cpp`); `UnitTest.cpp` compiled or the `UEngine::Exec`
      reference removed; `nvtt` delay-load removed (retail has no nvtt import: guard the nvtt users with
      `WITH_EDITOR`); `STATGROUP_TexturePool` link anchor checked on the map file.
- **Accept:** `DishonoredGame.exe` links with `D3D9Drv.lib`; the O-level smoke passes with `--rhi null`;
  `--rhi d3d9` reaches `RHIInit` and creates a D3D9 device or logs why (no crash, no assert).

### Q — Engine convergence: Interpolation / Sequence / Camera / Decal / LensFlare / Physics
Owner: agent Q. Build dir `build\agentQ`. IDA copies as above (2012 for member names, 2013 for checks).
Rows: `EngineInterpolationClasses.h` 35 (base `UInterpTrackInst` done by the coordinator as the tool
check; `UInterpGroup` −12: inherit `FInterpEdInputInterface` + `GroupAnimSetsPawn`; `UInterpGroupInst` −32),
`EngineSequenceClasses.h` 18, `EngineCameraClasses.h` 2 (`ACamera` too large), `EngineDecalClasses.h` 4,
`EngineLensFlareClasses.h` 2, `EnginePhysicsClasses.h` 5.
- [ ] Per class: `sdk_show.py` → `sdk_props.py` (base first, rebuild the probe, then children) → hand edits
      for native-only members with `// DISHONORED(layout): retail SDK @off` / `2012 PDB @off`; `.cpp` uses of
      removed reference members become `DISHONORED_SHIM_STATIC` or are ported.
- [ ] `UnknownDataNN` placeholders left by the tool are resolved from the 2012 PDB (`sdk_show.py` lists the
      candidates) or kept with the comment when the PDB has nothing.
- **Accept:** `xcheck_sdk_layout.py build/agentQ/layout_probe.txt --header <each header>` exit 0; sizes =
  `native_class_sizes.csv`; Engine + Launch build; `build_and_smoke.py --no-build` at milestone 1 unchanged.

### R — Engine convergence: Anim / Material / Particle / Light / Mesh / Texture / SpeedTree
Owner: agent R. Build dir `build\agentR`. Rows: `EngineAnimClasses.h` 33 (base `UAnimNode` 224→208;
structs `FAnimBlendChild`/`FAnimBlendInfo`/`FAnimInfo`/`FAnimNotifyEvent` too small),
`EngineMaterialClasses.h` 18 (`UMaterialExpression` +28), `EngineParticleClasses.h` 8,
`EngineLightClasses.h` 10 (root `ULightComponent` in `UnActorComponent.h:508`, hand-written: `sdk_props.py --print`),
`EngineMeshClasses.h` 2, `EngineTextureClasses.h` 2, `EngineSpeedTreeClasses.h` 2. Same method and acceptance as Q.

### S — Engine convergence: EngineClasses.h core / UI / AI / GameEngine / Controller / Client
Owner: agent S. Build dir `build\agentS`. Rows: `EngineClasses.h` 38 (`UActorFactory` family +12,
`AGameInfo`, `ANavigationPoint`, `APylon`, `UOnlineSubsystem` 252→180, `ULevelStreaming` 224→160,
`UGameViewportClient`, `USettings`, `UPostProcessChain`, `AActor` bitfield-only rows),
`EngineUIPrivateClasses.h` 28 (`UUIRoot`→`UUIDataProvider` 68→84), `EngineUserInterfaceClasses.h` 7
(`UPlayerInput` 604→444), `EngineAIClasses.h` 7 (`UNavigationHandle` 336→228),
`EngineGameEngineClasses.h` 4, `EngineControllerClasses.h` 2, `EnginePawnClasses.h` 2 (`AScout`),
hand: `UnClient.h` `UClient`, `UnLevel.h` `ULineBatchComponent`, `Core/Inc/UnCorObj.h` `USystem`,
`UnActorComponent.h` `UActorComponent`/`UPrimitiveComponent`/`UMeshComponent` bitfield rows.
Same method and acceptance as Q. S owns `Core/Inc/UnCorObj.h` for `USystem` only.

### T — Native registrants + headers from the SDK dump
Owner: agent T. Build dir `build\agentT`. Files: `resources/tools/symbols/gen_classes_header.py`
(`--sdk`), `source/Development/Src/DishonoredGame/{Inc,Src}` (+ `Sources.cmake`), root `CMakeLists.txt`
(`DISHONORED_ENABLE_DISHONOREDGAME`, default OFF until it compiles), then the same for `GFxUI` (18 classes),
`AkAudio` (19), `OnlineSubsystemSteamworks` (2) as registrant-only units behind their own options.
Priority: whatever makes `UClass::Bind`/`Link` succeed for every `RF_Native` class of the cooked packages
comes before function-wrapper polish.
- [ ] `gen_classes_header.py <Module> --sdk`: classes/structs/enums in retail order with SDK offsets, sizes
      from `native_class_sizes.csv`, enum values + member kinds from `script_classes_2013.json`, gap members
      named from 2012 `types.json` where a member of the same name/size exists, else `UnknownDataNN`;
      `VERIFY_CLASS_SIZE`/`OFFSET` from the SDK; `DECLARE_CLASS`, `DECLARE_FUNCTION(exec…)` and `event…`
      wrappers with parameter structs from the dump's `functions` (`*_parameters.hpp`) and `FunctionFlags`.
- [ ] `DishonoredGameClasses.h` (split per group like Engine if > 30k lines), `DishonoredGame.h`,
      `AutoInitializeRegistrantsDishonoredGame` / `AutoGenerateNamesDishonoredGame` /
      `AutoCheckNativeClassSizesDishonoredGame` generated; natives as `DECLARE_FUNCTION` stubs that `appErrorf`
      with the function name; `IMPLEMENT_CLASS` for all 1,870 (1,823 in `native_class_sizes.csv` + the
      script-only ones the dump lists).
- [ ] Layout probe for the module (`generate DishonoredGame`) checked with `xcheck_sdk_layout.py`.
- [ ] Hand-over to O: `DISHONORED_HAVE_DISHONOREDGAME` (etc.) replaces the registrant stubs in
      `Launch/Src/DishonoredStubs.cpp`.
- **Accept:** each module compiles behind its option; `xcheck_sdk_layout.py` 0 rows for the module's reflected
  members; class count and sizes match `native_class_sizes.csv` (DLC05–07 included); DLC-only classes listed.

### U — Middleware versions and Phase 4 memo
Owner: agent U. Build dir `build\agentU`. IDA copies `retail2013_agentU.i64`, `shipping2012_agentU.i64`.
- [ ] Exact versions from exe strings / DLL version resources: Wwise (`aksoundengine` build), PhysX 2.8.x
      (`PhysXLoader`, `NxCharacter`), APEX 1.x, Scaleform GFx 3.x build, FaceFX, Bink (`binkw32.dll`),
      Steamworks (`steam_api.dll` interface strings `SteamUser0xx`…), libcurl (2013 only). Table with evidence.
- [ ] Bink: import library from `binkw32.dll` exports (`dumpbin /exports` → `.def` → `lib /def`), header
      reconstructed from the reference `Engine/Bink` glue and the 20 imported functions; `USE_BINK_CODEC`
      behind `DISHONORED_WITH_BINK` (link check only).
- [ ] Steamworks: SDK version the interface strings need; offline path (`-nosteam`) design for Phase 8; the SDK
      is obtained by the user, never committed.
- [ ] PhysX 2.8.4 / APEX: SDK availability, what `UnNovodexSupport.h` needs, driving the shipped DLLs via
      reconstructed headers; blast radius (110 Engine files under `WITH_NOVODEX`).
- [ ] Scaleform + Wwise: recommendation with reasoning (matching SDK vs rewrite from decompile vs subset) and
      the minimum milestone 4 needs (main menu = Scaleform; audio may stay silent).
- [ ] Output `resources/docs/middleware.md` (versions, decisions, blockers the user must resolve); `cmake/`
      changes for Bink only.
- **Accept:** every version claim cites an exe/DLL string or resource; the Bink import lib builds and links
  into Launch behind `DISHONORED_WITH_BINK`.

## Coordinator

Merge order: P (module target) → O (bring-up with skip list) → Q/R/S (headers; rebuild O's smoke after
each) → T (new modules, options ON in `build/coord`, skip list dropped → `objects as part of root set`)
→ U (docs + Bink option). After each merge: `gen_layout_probe.py compare`, `xcheck_sdk_layout.py`,
CoreSmoke, `build_and_smoke.py` at the O milestone line; one commit per agent.

End of wave:
- `build_and_smoke.py --build-dir build/coord --milestone "objects as part of root set"` exit 0.
- `xcheck_sdk_layout.py build/coord/layout_probe.txt`: contract 0; Engine rows ≤ 20 (leftovers listed by
  owner); DishonoredGame rows 0 for reflected members.
- `gen_layout_probe.py compare`: 0 contract mismatches; `DishonoredLayouts.h` regenerated for
  Core/Engine/DishonoredGame/D3D9Drv.
- CoreSmoke 99/99; Engine, D3D9Drv, DishonoredGame (option ON) compile.
- Docs: this tracker, `STATUS.md`, `PLAN.md` Phase 3/4/6 boxes, `middleware.md`, memory.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| C4 | coordinator | Wave-2 tooling (`sdk_props.py`, `sdk_show.py`, `xcheck --header`, smoke `--rhi/--expect/--skip-native`) | done | 2026-09-27 | verified on `UInterpTrackInst` (56 → 64, delta row gone) |
| C5 | coordinator | Merge of all packages, verification on a clean HEAD worktree | done | 2026-09-25 | HEAD 0c9bd4d with D3D9Drv + GFxUI + AkAudio + OSS + DishonoredGame ON and layout checks ON: 677 units, 0 errors; SDK delta 2,314 types / 1,677 exact / 4 rows (ADisDoor, ADisGameCrowdAgentSkeletalRat, ADishonoredPlayerController, UOnlineSubsystemSteamworks: bases in GameFramework/IpDrv still to converge); compare 0 contract mismatches; CoreSmoke 99/99 before the incident. Milestone-2 smoke of the combined exe could not be rerun after the retail content loss (see Incident); O's acceptance run stands |
| O | done | Milestone 2: startup packages loaded | done (4 native packages) | 2026-09-25 | FSystemSettings on retail design (GEngineIni, 107 keys, no checkf), retail RHIInit/-nullrhi, no shader compiler per retail, FAsyncIORequest::Event + LoadDataWithEvent, retail native package lists (2013 0x5def10/0x5dfb50) + `-skipnativepkgs=`/`-allowunboundnatives` bring-up switches, GC token streams fixed, five loader Serialize deltas ported. Core/Engine/GameFramework/IpDrv load end to end: "24107 objects as part of root set". Next: DishonoredGame/GFxUI/AkAudio/OSS packages with T's registrants (122 Arkane classes cooked into Engine.upk/GameFramework.upk need them), then Startup.upk |
| P | done | D3D9Drv module + build follow-ups | done (part 1 committed) | 2026-09-28 | D3D9Drv compiles (4 mechanical fixes), links into the exe; `--rhi d3d9` creates the D3D9 RHI and reaches `Shader platform (RHI)`; nvtt users WITH_EDITOR-only, libpng15 pragma guarded, USE_UNIT_TESTS=0; probe 14 types / 9 exact, D3D9Drv/Inc/DishonoredLayouts.h. CMake/Launch/stubs wiring lands with T (shared files). Found: `[DevOptions.Shaders]` verifies and a ULevel GC-offset assert (handed to O) |
| Q | done | Engine convergence: Interp/Sequence/Camera/Decal/LensFlare/Physics | done | 2026-09-28 | 43 PROPS blocks regenerated, 45 classes/structs; 40 rows -> 0 after the coordinator shimmed `FPhysEffectInfo::Sound`; ACamera 1040, USeqAct_Interp 520, URB_BodyInstance 224; shim clusters listed in agentQ.md |
| R | done | Engine convergence: Anim/Material/Particle/Light/Mesh/Texture/SpeedTree | done | 2026-09-28 | 91 PROPS blocks regenerated + ULightComponent by hand; 79 rows -> 0, 95 size mismatches -> 0; nine *_DEPRECATED members renamed to retail names; 132 shims (120 used in Src) are porting TODOs |
| S | done | Engine convergence: EngineClasses/UI/AI/GameEngine/Controller/Client | done | 2026-09-28 | 82 PROPS blocks regenerated; 0 rows in every header except `UGameViewportClient` (ShowFlags is a QWORD in retail/2012, TStaticBitArray<128> in the reference: engine-wide follow-up); new UI provider interfaces, USystem/UClient/ULineBatchComponent retail members |
| T | done | Registrants + headers from the SDK dump (DishonoredGame, GFxUI, AkAudio, OSS) | done | 2026-09-28 | `gen_classes_header.py --sdk`: 1,822/1,823 DishonoredGame native classes, 665 structs, 1,140 functions (stubs/events/delegates), 575 enums, 73 group headers; GFxUI 17 / AkAudio 18 / OSS 2 classes; options `DISHONORED_ENABLE_{GFXUI,AKAUDIO,OSS,DISHONOREDGAME}` (OFF) + `DISHONORED_SDK_LAYOUT_CHECKS`; exe links with all four (64 MB), milestone 1 with ~1,980 more native classes registered. Coordinator check on the merged HEAD with all four ON: SDK delta 2,314 types / 1,676 exact / 5 rows (bases: AGameCrowdAgentSkeletal, UGameViewportClient ShowFlags, UOnlineSubsystemCommonImpl, ADisDoor, ADishonoredPlayerController), contract 0, CoreSmoke 99/99, smoke exit 0 |
| U | done | Middleware versions + Phase 4 memo + Bink import lib | done | 2026-09-28 | `middleware.md`: GFx 3.3.89, Wwise 2012.1 (bank v65), FaceFX 1.7.3.1, PhysX 2.8.4 (APEX shipped, never linked), Bink 1.9p, steam_api 1.30.50.46 (SDK 1.18/1.19), libcurl 7.77.0; Bink header/import lib behind DISHONORED_WITH_BINK, link check passed |
| V | done | EShowFlags QWORD (ShowFlags follow-up from S) | done | 2026-09-25 | `typedef QWORD EShowFlags`, 62 bits with 2012/2013 witness functions, 22 flags -> 0, UGameViewportClient 284, FSceneViewFamily 76; last Engine SDK delta row closed. Follow-ups: FSceneViewFamily::CurrentBendTime @0, SHOW_DefaultGame += Selection|Portals |

## Rules for agents (unchanged)

- Own build dir `build\agent<X>`; own IDA copies (`resources/docs/idb/shipping2012_agent<X>.i64`,
  `retail2013_agent<X>.i64`, copied from `shipping2012_v1.i64` / `retail2013_named.i64`); never open another
  agent's copy; no IDA MCP tools (`python resources/tools/ida/run.py <script> <db>` only).
- Edit only the files your package names; no commits; report to `resources/docs/agents/agent<X>.md`
  (what changed, evidence per change, what is left, follow-ups outside your files).
- Every reference-source edit carries `// DISHONORED(layout|port|retail|bringup)` with evidence.
- The **retail 2013 build is the target**; 2012 is evidence, not truth. Say which build every number comes from.
- Tool hygiene: write patch scripts with the Write tool (Bash heredocs mangle backslashes); headers are
  CRLF; never `git add -A`.
- **Never create junctions/symlinks into the retail or reference trees, and never recursively delete a
  directory that may contain one** (`rm -rf`, `Remove-Item -Recurse`, `git worktree remove`): on
  2026-09-25 (wave-2 merge) such a delete went through the old `build\stage` junctions and wiped the
  retail `Engine/`, `CookedPCConsole`, `DLC`, `Localization`, `Movies`. `stage_retail.py` now copies our
  exe into the retail `Binaries\Win32` instead; links are removed only with `os.rmdir`/`rmdir`
  (`build/unlink_junctions.py`).
