# Phase 2b + Phase 3 wave 1 — plan and tracker

Written 2026-09-25 after Phase 2 (`PHASE2.md`, `verify_phase2.py` 18/18). Plan of record: `PLAN.md`.
Read `STATUS.md` first.

## Goal of this wave

Turn the provisional 2012-based Core into a **retail-verified** Core, and start Engine
convergence and the exe bring-up, in parallel:

1. **Retail truth (Phase 2b).** Establish the 2013 layouts and tables from the retail exe and the
   retail cooked packages; reconcile Core's headers with them.
2. **Package loading blockers.** LZO decompression and the 24 Core serialization functions that
   differ from the reference (`serialization_delta_core.md`).
3. **Name propagation 2012 → 2013 (Phase 7 start).** Without it every 2013 check is by pattern
   only; with it the 2013 exe becomes as readable as the 2012 one.
4. **Engine layout probe** (against 2012 first, re-checked against retail as soon as 2b delivers).
5. **Milestone 1**: the real `Launch` module linking Core + Engine reaching
   `Init: Object subsystem initialized`.

Facts fixed while planning (2026-09-25):

| Fact | Consequence |
|---|---|
| `UProperty::Serialize` writes ArrayDim/PropertyFlags/Category/ArraySizeEnum, **not `Offset`**; `UStruct::Serialize` does not write `PropertiesSize`. Offsets are recomputed by `UStruct::Link` at load | Cooked packages give the exact **member list, order and types** of every script class (2012 and 2013), not offsets. Offsets follow deterministically from that list plus the native base size and `/Zp4` alignment rules |
| `UClass::UClass(ENativeConstructor, DWORD InSize, DWORD InClassFlags, ..., const TCHAR* InNameStr, const TCHAR* InPackageName, ...)` is called from every `InitializePrivateStaticClass<X>` (2,540 in the 2012 exe) with `sizeof(X)` as an immediate and the class name as a wide-string literal | The **2013 `sizeof` of every native class** can be read from the retail exe by following the xrefs of the class-name strings to the constructor call, without any symbols. Validating the method on the 2012 exe against `sizes.csv` proves it |
| 2013 IDA db is bare (34,059 functions, 93 % unnamed, 18 types); Diaphora / BinDiff not installed; no Python LZO module | Name propagation needs Diaphora (clone into `resources/tools/diaphora`, run headless via `idat`) or an own matcher; LZO needs a pure-Python LZO1X decoder for the tools and lzokay (MIT) via FetchContent for the engine |
| All cooked packages are `COMPRESS_LZO` (`CompressionFlags=2`) | Both the package parser and the engine need LZO1X before any name/export table is readable |
| Per-agent isolation that worked in Phase 2: own build dir `build\agent<X>`, own IDA copy `resources/docs/idb/shipping2012_agent<X>.i64`, report in `resources/docs/agents/agent<X>.md`, coordinator commits | Same rules below. Agents on the 2013 exe get their own copy of `../Dishonored_Latest2026/Binaries/Win32/Dishonored.exe.i64` as `resources/docs/idb/retail2013_agent<X>.i64` |

## Work packages (wave 1, all parallel)

Letters continue Phase 2's sequence (A–G used).

### H — Retail native class sizes (Phase 2b)
Owner: agent H. Own IDA copies: `shipping2012_agentH.i64`, `retail2013_agentH.i64` (create from the two `.i64` files; never open another agent's copy; no IDA MCP tools).

- [ ] `resources/tools/ida/export_class_sizes.py`: for every wide-string literal that is a class
      name (strings whose xref lands inside a small function calling a common target with the
      pattern `push <imm size>` … `push offset L"<Name>"` … `push offset L"<Package>"` → call), record
      `class, package, size, flags, cast_flags, super_ptr, registrant_rva`. On the 2012 db the
      target is `??0UClass@@QAE@W4EStaticConstructor@@...` (named); on the 2013 db find the
      target by locating the 2012 pattern's equivalent (the function called from >2,000 sites
      each preceded by two string pushes), then apply the same walk.
- [ ] Validate on 2012: `size` must equal `sizes.csv` for every class (report exceptions).
- [ ] Output `resources/docs/types/native_class_sizes.csv` (`class, size_2012, size_2013, delta,
      package_2013`) and `native_class_sizes.md` (summary: classes only in 2013, only in 2012,
      changed sizes grouped by module).
- **Accept:** 2012 sizes match the PDB for ≥ 99 % of classes; 2013 table has ≥ 2,500 classes;
      `UObject`, `UClass`, `AActor`, `ADisPlayer` rows present with both sizes.

### I — Package class/member tables (Phase 2b)
Owner: agent I. No IDA needed.

- [ ] `resources/tools/pdb/lzo1x.py`: pure-Python LZO1X-1 decompressor (spec is small; verify on
      the first compressed chunk of `Core.upk` by checking the decompressed name table starts
      with plausible `FString`s).
- [ ] `resources/tools/pdb/read_package_classes.py`: parse a cooked package fully — summary
      (reuse `read_package_summary.py`), compressed chunks → decompress, name table, import
      table, export table; then deserialize the exports that are `UClass`, `UScriptStruct`,
      `UFunction`, `UEnum`, `UConst`, `U*Property` following the reference `UnClass.cpp`
      (`UField::Serialize`, `UStruct::Serialize`, `UState::Serialize`, `UClass::Serialize` with
      the Arkane deltas from `serialization_delta_core.md`, `UProperty::Serialize` and each
      subclass, `UEnum::Serialize`, `UFunction::Serialize` incl. `iNative`/`FunctionFlags`).
      Version-gate with `Ver()`=801 / `LicenseeVer()`=30 constants from the summary.
- [ ] Run on 2013 `Core.upk`, `Engine.upk`, `GameFramework.upk`, `IpDrv.upk`, `GFxUI.upk`,
      `OnlineSubsystemSteamworks.upk`, `AkAudio.upk`, `DishonoredGame.upk`, DLC05/06/07 script
      packages, and on the 2012 equivalents → `resources/docs/types/script_classes_2013.json`,
      `script_classes_2012.json` (per class: super, flags, children in order with type/arraydim/
      flags/struct-or-enum ref, enums with values, functions with flags and native index).
- [ ] `resources/docs/types/script_delta_2012_2013.md`: classes added/removed, members
      added/removed/reordered, enum value changes, native index changes. This is the exact
      2012→2013 script-side delta.
- **Accept:** 2013 `DishonoredGame.upk` yields ≥ 1,400 classes; `UObject` from `Core.upk` has
      the expected children; the DFSDK `.uc` property order for 20 sampled classes matches the
      2013 JSON exactly; 2012 JSON matches `types.json` member order for 20 sampled script classes.

### J — Name propagation 2012 → 2013 (Phase 7 start)
Owner: agent J. Own IDA copies: `shipping2012_agentJ.i64`, `retail2013_agentJ.i64`.

- [ ] Install Diaphora (`git clone https://github.com/joxeankoret/diaphora resources/tools/diaphora`,
      gitignored) and run it headless (`idat.exe` batch with `DIAPHORA_AUTO=1`,
      `DIAPHORA_EXPORT_FILE`, then the diff step) between the two databases; if headless Diaphora
      is unworkable, fall back to an own matcher: exact byte-hash of functions, then
      call-graph + string-reference matching, then relaxed opcode-hash — all in
      `resources/tools/ida/match_functions.py`.
- [ ] Output `resources/docs/symbols/match_2012_2013.csv` (`rva_2012, rva_2013, name, ratio,
      method`) and apply the names with ratio ≥ 0.9 to `retail2013_agentJ.i64` (save as
      `resources/docs/idb/retail2013_named.i64`); export `functions_2013.csv`, `natives_2013.csv`,
      `classes_2013.csv` with the Phase 1 scripts (`--suffix _2013` / suffix argument).
- [ ] `resources/docs/symbols/match_2012_2013.md`: matched / unmatched counts per module,
      list of 2012 functions with no 2013 counterpart (removed), sizes of the 2013 functions
      with no 2012 counterpart (new code: DLC, curl, etc.).
- **Accept:** ≥ 60 % of 2012 functions matched with ratio ≥ 0.9; `FEngineLoop::Init`,
      `ULinkerLoad::CreateLoader`, `UClass::Serialize`, `FName::StaticInit` matched; 2013
      natives export shows the `Req_DLC05_*` functions.

### K — LZO in the engine (milestone-3 blocker)
Owner: agent K. Build dir `build\agentK`. No IDA.

- [ ] `cmake/Dependencies.cmake`: `dishonored_fetch(lzokay https://github.com/jackoalan/lzokay.git <pinned commit>)`
      (MIT, C++, LZO1X-1 compress + decompress), target `Dishonored::lzokay`, Core links it.
- [ ] `Core/Src/UnMisc.cpp`: implement `appCompressMemoryLZO` / `appUncompressMemoryLZO` on
      lzokay (the reference calls `lzopro_*`/`lzo1x_*`; keep the UE3 function signatures and
      the `check` semantics); set `WITH_LZO=1` in `DishonoredDefines.cmake`; make
      `GBaseCompressionMethod` default match the retail exe (`COMPRESS_LZO`, see
      `serialization_delta_core.md`).
- [ ] CoreSmoke: new test that reads the first `FCompressedChunk` of the 2013 `Core.upk` and
      decompresses it through `appUncompressMemory`, then checks the name table (`FString`
      entries, first name plausible, count == 720).
- **Accept:** Core builds; CoreSmoke green with the new test; Debug and Release presets.

### L — Core serialization port
Owner: agent L. Own IDA copy `shipping2012_agentL.i64`. Build dir `build\agentL`.

- [ ] Port the 24 `port` functions of `resources/docs/function_status_seed.csv` in the order
      given in `serialization_delta_core.md` ("port before milestone 3"): `UClass::Serialize`
      (no `ClassGroupNames`; `m_OtherClassFlags` at Ver≥796; `m_DropdownCategory` at
      LicenseeVer≥10), `UScriptStruct::SerializeBin` (Arkane override, `STRUCT_DevLoad`,
      `ArIsDisSaveLoad`), `UObject::SerializeNetIndex`/`SetNetIndex` (verify against decompile),
      `ULinkerLoad::CreateLoader` (`ArkBsPatch` on precached packages; `FPackagePrecacheInfo`
      +3 members), `ULinkerLoad::AsyncPreloadPackage(const TCHAR*)` (reads `Patches\<name>.bs`),
      `FIOSystem::LoadCompressedData` extra `EAsyncIORequestType` argument, `UnObjVer.h` version
      enum divergence above 766 (add Arkane's `VER_*` values used by the thresholds), the
      `LicenseeVer()` branches listed in `licensee_branches.md` that are in Core.
- [ ] Each ported function gets `// DISHONORED(port): <rva> <what differs>`; update
      `resources/docs/function_status.csv` (create from the seed; `port` → `ported`).
- **Accept:** Core builds in both presets; CoreSmoke green; `function_status.csv` shows 24
      `ported`; `module_map.py` regenerates `progress.md` with the new counts.

### M — Engine layout probe
Owner: agent M. Build dir `build\agentM`. Own IDA copy `shipping2012_agentM.i64` only if needed.

- [ ] `gen_layout_probe.py generate Core Engine` (add the Engine include/definition wiring the
      generator needs, and the Engine `probe_skip_Engine.txt` loop); `gen_layout_asserts.py
      --probe … Engine` → `Engine/Inc/DishonoredLayouts.h` included from `Engine.h` under
      `DISHONORED_LAYOUT_CHECKS`.
- [ ] Engine contract list (add to `gen_layout_probe.py CONTRACT`): `AActor`, `APawn`,
      `AController`, `APlayerController`, `AWorldInfo`, `UWorld`, `ULevel`, `UActorComponent`,
      `UPrimitiveComponent`, `UMeshComponent`, `UStaticMeshComponent`, `USkeletalMeshComponent`,
      `UStaticMesh`, `USkeletalMesh`, `UTexture`, `UTexture2D`, `UMaterial`,
      `UMaterialInstance`, `UMaterialInstanceConstant`, `UAnimSequence`, `UAnimSet`,
      `UPhysicsAsset`, `UEngine`, `UGameEngine`, `UPlayer`, `ULocalPlayer`, `UNetDriver`,
      `UNetConnection`, `FPackageInfo`, `USequence`, `USequenceOp`, `UInterpData`.
- [ ] Converge these on the **2012** PDB first (same rules as P2.5: `// DISHONORED(layout)` with
      evidence), then mark every changed type `provisional-2012` in the report so the
      coordinator re-checks them against H's 2013 sizes and I's member lists.
- **Accept:** `LayoutProbe` builds with Core+Engine; `reference_layout_delta.md` gains an
      Engine section with the contract list at 0 mismatches vs 2012; Engine still compiles.

### N — Milestone 1: real Launch
Owner: agent N. Build dir `build\agentN`. Starts immediately; the last steps depend on K/L only
if package loading is reached, which milestone 1 does not require.

- [ ] Compile the reference `Launch/Src/Launch.cpp`, `LaunchEngineLoop.cpp`, `LaunchMisc.cpp`
      instead of `DishonoredLaunchStub.cpp` (option `DISHONORED_REAL_LAUNCH`, default OFF until
      it links); `GAMENAME=DISHONOREDGAME` branch in `LaunchEngineLoop.cpp` for the
      `AutoInitializeRegistrants*` / `AutoGenerateNames*` calls of the modules we have (Core,
      Engine, GameFramework?, IpDrv?, WinDrv, D3D9Drv?) and stubs for the ones we do not
      (`DishonoredGame`, `AkAudio`, `OnlineSubsystemSteamworks`, `GFxUI`).
- [ ] Bring the modules milestone 1 needs to compile as `dishonored_module` targets:
      `WinDrv`, `IpDrv`, `GameFramework` (already imported); `D3D9Drv` only if Launch cannot be
      linked without it (it can: `USE_NULL_RHI` path, or a stub RHI).
- [ ] Link: collect unresolved externals, resolve by adding the module that owns them or by
      stubs in `source/Development/Src/Launch/Src/DishonoredStubs.cpp` (each stub commented with
      owner module and reason). Run with `-log -nosteam` from a staging copy of
      `Dishonored_Latest2026` (junction the content folders: `tools/stage_retail.py` creates
      `build\stage\` with `Binaries\Win32` from our build and `DishonoredGame`/`Engine`
      junctioned).
- [ ] Goal: `Launch.log` contains `Init: Object subsystem initialized` and the log lines before
      it match the golden 2012 ArkProfile log after `normalize_log.py` (allowing the known
      differences).
- **Accept:** `build\stage\Binaries\Win32\DishonoredGame.exe -log -nosteam` writes the milestone
      line; `tools/build_and_smoke.py` (new) automates build + run + normalized diff.

## Coordinator (after H and I land)

- [ ] Retail reconciliation: for every Core contract type compare `native_class_sizes.csv`
      (2013) with the current header size; for every script class compare the 2013 member list
      with the 2012 one. Fix Core headers where 2013 differs (`// DISHONORED(layout): retail …`),
      regenerate `DishonoredLayouts.h` from the 2013 sizes (`gen_layout_asserts.py --sizes
      native_class_sizes.csv:size_2013`), rebuild, re-run CoreSmoke and the probe.
- [ ] Merge M's Engine results against the 2013 numbers the same way.
- [ ] `verify_phase2.py` gains a `retail` section; `STATUS.md`, `PLAN.md` updated; commit per
      agent as in Phase 2.

## Wave 2 (after wave 1)

- Module compiles: `GameFramework`, `IpDrv`, `WinDrv`, `D3D9Drv` as targets (N may have done
  some); layout probe for each.
- Milestone 2: load `Core.upk` … `Startup.upk` (needs K + L + the 2013-verified layouts).
- DishonoredGame: generate `DishonoredGameClasses.h` from I's 2013 JSON + H's sizes with
  `gen_classes_header.py` (switch its inputs from the 2012 PDB to the retail JSON), start the
  native side from J's named 2013 decompiles.
- Phase 4 decisions with sizes in hand: Scaleform, Wwise, PhysX 2.8.4, FaceFX.

## Tracker

| ID | Agent | Task | Status | Date | Notes |
|---|---|---|---|---|---|
| H | done | Retail native class sizes | done | 2026-09-26 | 2,857 retail classes, method validated 2,538/2,538 on 2012. Core contract types unchanged in retail except `UClass` 456→436 (m_DropdownCategory + one 12-byte member gone). 274 shared classes changed size (255 DishonoredGame, 14 Engine: UTexture2D +4, USkeletalMeshComponent +32, AGamePawn +16). 345 DLC/new classes. Retail uses a 12-dword `.data` descriptor per class → `GetPrivateStaticClass(desc)` (rva 0x79370), not ctor call sites |
| I | done | Package class/member tables + LZO1X (python) | done | 2026-09-26 | `read_package_classes.py` deserializes every field export (0 SerialSize mismatches, ~60k per tree). 2013: 3,043 classes / 236 files; 2012: 2,740. Delta: 401 classes added, 98 removed, 258 changed; 453 members added, 160 removed, 7 reorders, 35 enum changes, 0 native-index changes. 2013 cooker strips/zeroes many exports; DLC classes live as forced exports in DishonoredGame.upk / level packages |
| J | done | Name propagation 2012→2013 | done | 2026-09-26 | Own matcher (`match_functions.py`, Diaphora too slow): 82.8% of 2012 functions matched, `retail2013_named.i64` (65,841 functions, 25.9% unnamed), `functions_2013.csv` / `natives_2013.csv` / `classes_2013.csv` / `globals_2013.csv`, `match_2012_2013.csv/.md` |
| K | done | LZO in the engine | done | 2026-09-26 | lzokay (MIT) via FetchContent, `WITH_LZO=1`; `appUncompressMemoryLZO` bounds-checked; CoreSmoke 99/99 incl. decompressing the retail Core.upk name-table chunk (first name "!", last "~=", 720 entries, sorted) |
| L | done | Core serialization port (24 functions) | done | 2026-09-26 | 26 ported from the 2012 decompile (UClass::Serialize, UScriptStruct::SerializeBin, CreateLoader+ArkBsPatch, AsyncPreloadPackage .bs patches, FIOSystem request types, version enums); `function_status.csv` created. Engine call sites fixed by coordinator (FlushAsyncLoading(), AIORT_MipMap) |
| M | done | Engine layout probe | done | 2026-09-26 | 1,899 types probed (Core 304 + Engine 1,595), 1,374 exact; all 29 probeable Engine contract types exact vs 2012 (provisional). `Engine/Inc/DishonoredLayouts.h` 669 asserts + 284 pending. Reference-only members kept as storage-less shims (568). `WITH_EDITORONLY_DATA=1`. Retail check: only USkeletalMeshComponent (+32) and UTexture2D (+4) differ in 2013 (C2). Remaining: FPackageInfo 72 vs 68 (Core), 524 non-contract Engine rows |
| N | done | Milestone 1: real Launch | done | 2026-09-26 | **Milestone 1 reached**: `DISHONORED_REAL_LAUNCH=ON` builds Launch.cpp/LaunchEngineLoop.cpp with GAMENAME=DISHONOREDGAME branches + stubs; GameFramework/IpDrv/WinDrv targets on; null RHI; `build_and_smoke.py` exits 0 (`Init: Object subsystem initialized`, log prefix matches golden). Next blocker: `SystemSettings.cpp:532` assert on retail `DishonoredSystemSettings.ini` (milestone 2) |
| C1 | coordinator | Retail reconciliation of Core | done | 2026-09-27 | `types/retail_reconciliation.md`: only UClass differs (456→436). Retail `UClass::Link` decompile shows `NetFields` removed and `ClassReps` kept; `m_DropdownCategory` removed (still deserialized into a discarded local). Headers/ctors/Link/`GetClassNetCache` adapted, assert 436, CoreSmoke 99/99 |
| C2 | coordinator | Retail reconciliation of Engine | done | 2026-09-27 | UTexture2D +`MinResidentMipCount` (372; subclasses 376/376/436), USkeletalMeshComponent +`RawExtractedRootMotionDelta`, −2 `bRootMotion*Notify` bits (1088). `gen_layout_probe.py compare` / `gen_layout_asserts.py` now take retail sizes from `native_class_sizes.csv`: 0 contract mismatches vs retail; 9 non-contract Engine classes still differ (table in `retail_reconciliation.md`) |

## Rules for agents (unchanged from Phase 2)

- Own build dir `build\agent<X>`; own IDA copies; never open another copy; no IDA MCP tools.
- Edit only the files your package names; no commits; report to `resources/docs/agents/agent<X>.md`.
- Every reference-source edit carries `// DISHONORED…` with evidence; no behavior changes beyond
  the stated port.
- The **retail 2013 build is the target**; 2012 is evidence, not truth. Say which one every
  number in your report comes from.
