# Agent T — native registrants + class headers from the retail SDK dump (2026-09-28)

Package "T" of `resources/docs/PHASE4.md`: `gen_classes_header.py <Module> --sdk` generates, from the
CodeRed dump of the running retail exe, the class headers, `IMPLEMENT_CLASS` registrants, FName tables,
native lookup tables and native stubs of the four retail script-package modules that `UClass::Bind`
(`Core/Src/UnClass.cpp:1822`) needs before `DishonoredGame.upk`, `GFxUI.upk`, `AkAudio.upk` and
`OnlineSubsystemSteamworks.upk` can load. **Retail 2013 is the target for every number below**: sizes from
`native_class_sizes.csv` (`size_2013`), offsets from `retail_sdk_layout.json`; the 2012 PDB is named as
such wherever it is used (member names/types of native-only gaps only).

Build dir `build\agentT` (working tree, `-DDISHONORED_LAYOUT_CHECKS=OFF` because the Engine PDB asserts lag
agents Q/R) and, for the results below, the snapshot `build\agentT\snap` (HEAD + my files + agent P's
committed D3D9Drv edits, own `external/` FetchContent dir; `build/agentT/sync_snapshot.py`,
`snap_configure.cmd`, `snap_modules.cmd`, `snap_probe.cmd`). The working-tree build broke twice on things
outside my package (the `FOnlinePlayerScore` 16→28 assert of `Engine/Inc/DishonoredLayouts.h` after agent S,
and the shared `external/*-subbuild/CMakeCache.txt` rewritten by agent Q's snapshot configure — see
"Environment hazards"), hence the snapshot.

## Result

| Module | Retail natives | Generated | Registered | Compiles | xcheck (probe vs SDK) | sizes vs `native_class_sizes.csv` |
|---|---:|---|---|---|---|---|
| GFxUI | 17 | 17 classes, 9 structs (+1 Engine struct shim `FUIDataStoreBinding`), 154 functions (132 natives) | all 17 | yes | 25 types, **0 rows** | 17/17 |
| AkAudio | 18 | 18 classes (incl. the intrinsic `UAkAudioDevice`), 2 structs | all 18 | yes | 20 types, 3 rows: `USeqAct_AkLoadBank/PostEvent/SetRTPCValue` +4, base `USeqAct_Latent` 268 vs retail 264 (Engine, Q) | 15/18 (same 3) |
| OnlineSubsystemSteamworks | 2 | 2 classes, 11 structs, 161 functions (55 natives) | all 2 | yes | 12 types, 1 row: `UOnlineSubsystemSteamworks` +4, base `UOnlineSubsystemCommonImpl` 204 vs retail 200 (IpDrv) | 1/2 (same) |
| DishonoredGame | 1,823 | 1,822 classes + 120 Engine/GameFramework shim classes, 665 structs (+55 shim structs), 45 interfaces, 1,140 functions (974 natives, 105 events, 38 delegates), 575 enums, 46 consts, 73 headers (89,633 lines incl. the two .cpp) | 1,822 of 1,823 (+120 shims; `IMPLEMENT_CLASS` ×1,942) | yes (snapshot, see "Build status") | 2,113 types in the dump, **45 rows, every one a uniform shift from an unconverged Engine/GameFramework base or embedded Engine struct** (table below); 0 rows caused by the generator | probe: 1,478 of 1,823 probed (2012-known), 1,437 match, 41 differ (all base deltas); the 345 2013-only classes are covered by the `static_assert`s of `DishonoredGameLayouts.h`: 12,164 asserts pass, 327 pending on the same bases |

Link emulation (`LinkEmulator`, the `UStruct::Link`/`UProperty::Link` rules of `UnClass.cpp`/`UnProp.cpp`
replayed over the retail package member lists): **9,342 of 9,342** DishonoredGame member offsets, 82/82
GFxUI, 32/32 AkAudio, 92/92 OSS reproduce the SDK dump exactly. That validates the dump parser, the map-gap
rule (below) and the one class the dump never saw (`ADishonoredNPCPawn`, synthesized from the package list:
112 members, span 2220..3572, sizeof 3584 = retail).

The one missing DishonoredGame class: `UDisGameCrowdPopulationManager` — its retail base
`UGameCrowdPopulationManager` (GameFramework, a `UObject` of 144 bytes in retail) collides with the tree's
`AGameCrowdPopulationManager` (the reference Actor). Needs the GameFramework owner (see follow-ups); until
then agent O keeps `DisGameCrowdPopulationManager` on the skip list.

## Generator design (`resources/tools/symbols/gen_classes_header.py --sdk`)

```
python resources/tools/symbols/gen_classes_header.py GFxUI --sdk --module-header
python resources/tools/symbols/gen_classes_header.py AkAudio --sdk --module-header --sources-cmake
python resources/tools/symbols/gen_classes_header.py OnlineSubsystemSteamworks --sdk --module-header
python resources/tools/symbols/gen_classes_header.py DishonoredGame --sdk --module-header --sources-cmake \
       --probe build/agentT/snap/build/layout_probe.txt --report build/agentT_gen_DishonoredGame.json
```
Order matters (DishonoredGame derives from the other three; their generated headers count as "declared").
`--dry-run` prints the statistics only; `--probe` marks classes whose base has not converged as `pending`
(below); `--shims`/`--no-shims` control the Engine shim unit (default: DishonoredGame only).

Inputs and what each one is trusted for:

| Input | Used for |
|---|---|
| `resources/docs/types/retail_sdk_layout.json` (`parse_codered_sdk.py`) | class set per package, super, reflected members (offset, size, SDK type, bitfield mask, static-array count), gaps, class span; script structs (size, base); function parameter blocks + `FunctionFlags`. Derived script structs (`struct X : Y` without `public`, 152 in DishonoredGame) are dropped by `parse_codered_sdk.DECL_RE`; the generator re-parses the `*_structs.hpp` files with a relaxed regex through the same `parse_types` (no edit to `parse_codered_sdk.py`, which is not in my file list — one-line fix suggested below) |
| `native_class_sizes.csv` | which classes are native (the retail descriptor set), `size_2013`, `flags_2013` → `DECLARE_[ABSTRACT_][CASTED_]CLASS[_INTRINSIC]` flags, `cast_flags_2013`, `within_2013` → `DECLARE_WITHIN`, `config_2013` → `StaticConfigName()` when it differs from the super's, `super_2013` |
| `script_classes_2013.json` (agent I) | enum values, consts (`UCONST_`), property kinds and declaration order (names the `MapProperty` gaps the dump skips, `InterfaceProperty` names), `header_filename` (retail header groups), `interfaces`, everything the link emulator needs |
| 2012 `types.json` | names/types of native-only gap members by the `sdk_props.py` relative-position rule, `TMap<K,V>` spellings of map properties, class alignment of 2012-known classes; never offsets |
| `natives_2013.csv` | `iNative` of numbered natives (none in the four packages: every `AUTOGENERATE_FUNCTION` is `-1`) |
| the tree (`Core/Engine/GameFramework/IpDrv/WinDrv` + the earlier generated modules) | declared types (incl. `BEGIN_COMMANDLET` classes and `I*` interfaces), declared enums, structs with an `(EEventParm)` ctor, pure virtuals of interfaces (`=0;`) |

Rules:

- **Layout** (`LayoutEmitter`): members in SDK offset order from `cursor = Align(span_start, 4)` (the
  `UStruct::Link` rule; MSVC verified to place derived members and secondary vtables at the same place, even
  inside a 16-aligned base's padding: `build/agentT_padtest.cpp`). Bitfield groups in mask order (missing
  bits get `UnknownBitNN` placeholders), byte runs left to the compiler's padding when the next member is
  aligned. Gaps, in order of preference: (a) retail package properties between the same neighbours that the
  dump did not type — 24 `MapProperty` gaps (60 bytes each) named from `script_classes_2013` and typed from
  the 2012 PDB (`TMap<FName,INT> m_Map` …) when the PDB has the member, else `BYTE name[60]`; a single other
  untyped property takes the whole gap; (b) 2012 PDB members between the same neighbours whose sizes add up
  (`sdk_props.pdb_size_of`); (c) `BYTE UnknownDataNN[n]`. Tail: `sizeof - Align(span_end, class alignment)`
  bytes, named from the PDB members after the last reflected one or `UnknownDataNN`; class alignment =
  max(super, members) with the `ALIGN16` set / PDB `align` ≥ 16 (AActor family). Interface properties
  (`X_Object`/`X_Interface` pointer pairs in the dump) are folded back into `TScriptInterface<class IX>` /
  `FScriptInterface`. `VfTable_X` members become secondary bases (`class C : public Super, public X`).
  Script structs get `MS_ALIGN(4)` when their members alone would give 1 (`UStruct::Link` gives every script
  struct `MinAlignment >= 4`, `NAME_Color` 4). Struct sizeof = `Align(PropertiesSize, MinAlignment)`
  (`UStructProperty::Link`, 70 structs differ from their span end).
- **Type map**: `sdk_props.map_type` (reused, not duplicated) plus the generator built-ins it lacks
  (`FDouble`→`DOUBLE`, `FQWord`→`QWORD`, `uint32_t`→`INT`, `bool`→`UBOOL`); enums → `BYTE`;
  `*_Mirror` types → the 2012 PDB declaration when the member exists with the same size, else
  `BYTE name[size]` (4 in DishonoredGame: `FBitArray_Mirror`/`FSparseArray_Mirror`/`FSet_Mirror` in the
  three `TSet`-typed members — the PDB names differ). Parameter blocks use by-value spellings
  (`FString`, `TArray<T>`, `UBOOL`).
- **Functions**: `FUNC_Native` → `DECLARE_FUNCTION(execFoo)` + `MAP_NATIVE` table + `AUTOGENERATE_FUNCTION`
  + a body in `<Module>NativeStubs.cpp` that `appErrorf(TEXT("<Module> native not ported: %s"), "Class::execFoo")`;
  `FUNC_Event` → `Class_eventFoo_Parms` + `eventFoo(...)` wrapper (out params copied back, optional params
  defaulted, `ReturnValue` initialised like the reference); `FUNC_Delegate` with a `__Foo__Delegate` member →
  `delegateFoo(...)` through `ProcessDelegate`; interface classes get no wrappers. Locals the dump lists in
  parameter blocks (no `CPF_Parm`) are skipped. `<Module>NativeStubs.ported.txt` (`Class::execFoo` lines)
  removes stubs once a native is ported.
- **Interfaces**: `CLASS_Interface` classes emit `UFoo` (`DECLARE_ABSTRACT_CLASS(...,CLASS_Interface,...)`)
  and `class IFoo [: public IParent] { virtual ~IFoo(); typedef UFoo UClassType; }`. Tree interfaces with pure
  virtuals inherited by a generated class (`IUIDataStorePublisher`, `IInterface_NavigationHandle`,
  `IArkHealthInterface`, ...) get `appErrorf` overrides so the class stays constructible
  (`GetUObjectInterface*` → `return this`), unless the class's own event wrapper already overrides the name.
  Native interfaces nothing declares (`FDisDialogSelNotify`) get a polymorphic placeholder.
- **Headers**: one per retail group (`header_filename`), `DishonoredGame<Group>Classes.h`, in the group-DAG
  order (SCCs merged), each including the groups it depends on under `#ifndef NAMES_ONLY`; enums, consts and
  the forward declarations of every class in the first header; the reference skeleton otherwise
  (`NAMES_ONLY`/`NATIVES_ONLY`/`STATIC_LINKING_MOJO`/`VERIFY_CLASS_SIZES` blocks, `AUTO_INITIALIZE_REGISTRANTS_*`,
  `G<Module><Class>Natives[]`). `<Module>.h`, `<Module>Names.h`, `<Module>Layouts.h`, `Src/<Module>Registrants.cpp`
  (`AutoInitializeRegistrants<M>`, `AutoGenerateNames<M>`, `AutoCheckNativeClassSizes<M>` behind
  `CHECK_NATIVE_CLASS_SIZES`, `IMPLEMENT_CLASS` for all), `Src/<Module>NativeStubs.cpp`, `Sources.cmake`
  (`--sources-cmake`: the 968 comment-only skeleton units of `import_reference.py` excluded).
- **Engine shims** (`DishonoredGameEngineShims.h`, DishonoredGame only): the 111 Engine + 9 GameFramework
  retail natives the tree does not declare (Arkane additions: `UArkPpNode*`, `UArkAnimNode*`, `UAkBank/UAkEvent`
  (Engine package in retail), `AGenericPortal`, `ASpawner`, `UDisEngineTweaksBase/Interface`, `UInterpTrack*KeyProperties`,
  the old `UUI*` data providers, ...) — 401 DishonoredGame classes derive from `UDisEngineTweaksBase` alone.
  Declared with their retail package (`DECLARE_CLASS(X,Super,flags,Engine)`), registered from
  `AutoInitializeRegistrantsDishonoredGame`, so `Engine.upk`/`GameFramework.upk` bind them too. The set is
  recomputed at every generation from what the tree declares: when Q/R/S/O add one of them to Engine, rerun
  the generator and it disappears from the shim unit.
- **Layout checks** (`<Module>Layouts.h`, `DISHONORED_SDK_LAYOUT_CHECKS`, root option default ON, per target):
  `static_assert(sizeof(X) == retail)` for every class/struct and `static_assert(offsetof(X, m) == retail)` for
  every reflected member (12,164 + 327 pending in DishonoredGame). `pending` comes from `--probe` (uniform
  shift of every member, or a class without members with the wrong sizeof, or an embedded engine struct whose
  probed sizeof differs from the retail element size; propagated to descendants and embedders) and from
  `<Module>Layouts.pending.txt` (4 hand entries, each with the Engine type that causes it). Independent of
  `DISHONORED_LAYOUT_CHECKS` so the Engine convergence cannot block these modules.
- **Naming**: `CLASS_Deprecated` classes get the `DEPRECATED_` prefix `IMPLEMENT_CLASS` strips
  (`UDEPRECATED_DisContactType_Env_Foliage`, `UDEPRECATED_DisDLC06SeqAct_EndGame`); C++ keywords / `Result` /
  `Stack` member names get `_`.

## xcheck rows (DishonoredGame, all explained by unconverged bases)

`python resources/tools/sdk/xcheck_sdk_layout.py build/agentT/snap/build/layout_probe.txt --header "DishonoredGame*Classes.h" --header DishonoredGameEngineShims.h`
→ 2,113 types, 45 differ. The generator's `--probe` pass classifies all of them (59 classes pending, the
extra 14 are 2013-only descendants the probe cannot see):

| Base / struct not converged (our tree vs retail) | Shift | Generated classes hit |
|---|---:|---:|
| `AKActor` (via `ADynamicSMActor` 656 vs 640, PHASE4 S) | +16 | 20 (`ADishonoredKActor`, `ADishonoredMovable`, `ADishonoredBreakable`, `ADisPickup_Base` family, `ADisWhaleOilBattery`, …) +5 descendants |
| `USeqAct_Latent` 268 vs 264 (Q) | +4 | 15 `UDisSeqAct_*` +3; also AkAudio's 3 rows |
| `UDecalComponent` 800 vs 752 (Q/R) | +52 | `UDishonoredDecalComponent`, `UGameDecal` (shim) |
| `ADecalManager` (Q) | −12 | `AGameDecalManager` (shim), `ADishonoredDecalManager`-family |
| `ACamera` 1328 vs 1040 (Q) | +276/+288 | `ADishonoredPlayerCamera`, `ADisDebugNPCCamera` |
| `AGameCrowdAgentSkeletal` (GameFramework) | +228 | `ADisGameCrowdAgentSkeletalRat` |
| `AGamePawn` (GameFramework) | +12 | 1 |
| `AGamePlayerController` (+8), `UGameViewportClient` (+8, S), `AGameCrowdDestination` (+16), `UInterpGroupInst` (−32, Q), `UInterpGroup` (−12, Q) | | 1 each (+2 descendants) |
| `FTViewTarget` 44 vs 40, `FPolyReference` 28 vs 24 (embedded structs, S/Q) | | `FDishonoredViewTarget`, `ADisDoor` (hand pending) |
| `USeqAct_Interp` 472 vs 520, `USeqAct_MultiLevelStreaming` 288 vs 284 | | `UDisSeqAct_DLC05_Interp`, `UDisSeqAct_DLC05_StreamLevels` (2013-only, hand pending) |

Nothing else differs: once those Engine/GameFramework/IpDrv types converge, regenerate with `--probe`
and the pending lines and rows go away without touching the generator.

## Counts

- DishonoredGame: 1,822 native classes (+120 shims), 665 script structs (+55 shim structs, 16 of them
  Engine structs the tree lacks, e.g. `FArkCpntLocoFootProp`, `FGameCrowdSpawnerSettings`), 45 script
  interfaces, 575 enums, 46 consts, 1,140 functions (974 native stubs, 105 events, 38 delegates, 870 FNames),
  73 headers + shims header, `DishonoredGameRegistrants.cpp` 2,373 lines, `DishonoredGameNativeStubs.cpp` 3,901 lines.
- Gaps: 24 map properties named from the package (all typed from the 2012 PDB), 4 other package-named
  gaps, 42 members typed from the 2012 PDB by the relative rule, **1 `UnknownData` placeholder**
  (`UDisDLC07Tweaks_WhaleBoneCharmCrackedList` @140, 24 bytes, 2013-only), 0 native tails after the
  alignment rule (the 9 apparent tails were 16-byte alignment padding of AActor-family classes), 4 unresolved
  member types (`TSet` mirrors, `BYTE[n]`).
- DLC / 2013-only DishonoredGame classes: **343** (no `size_2012`; `build/agentT_dlc_only.txt` — 132 `DLC05*`,
  91 `DLC06*`, 103 `DLC07*` per `native_class_sizes.md`, the remaining 17 are 2013 base-game additions such as
  `ADishonoredNPCPawn`-family renames); they are generated like the rest (layout from the dump, sizes asserted).
- Skipped: `UGameCrowdPopulationManager` (shim; tree declares `AGameCrowdPopulationManager`),
  `UDisGameCrowdPopulationManager` (needs it).

## Build status

Snapshot `build\agentT\snap\build` (`snap_modules.cmd`, log `build/agentT/snap_build.log`): `GFxUI.lib`,
`AkAudio.lib`, `OnlineSubsystemSteamworks.lib` and `DishonoredGameModule.lib` (48.8 MB) compile with the layout
static_asserts on (`DISHONORED_SDK_LAYOUT_CHECKS=1`): **0 errors**; DishonoredGame 12,137 asserts pass, 354 pending
(59 classes on unconverged bases via `--probe` + the 4 hand entries and their descendants), AkAudio 36/10,
OSS 48/84 (all `UOnlineSubsystemSteamworks` members, base +4), GFxUI 133/0. Link of the exe with all four
modules: see "Exe link" below. Working tree
`build\agentT` (`agentT_configure.cmd`, `agentT_build.cmd`): GFxUI and AkAudio built before the shared
`external/` caches were clobbered; the configure needs `-DDISHONORED_LAYOUT_CHECKS=OFF` while
`Engine/Inc/DishonoredLayouts.h` asserts the pre-S `FOnlinePlayerScore` size.

CMake: root `CMakeLists.txt` options `DISHONORED_ENABLE_GFXUI` / `_AKAUDIO` / `_OSS` /
`_DISHONOREDGAME` (all default OFF; the last needs the other three + GameFramework + IpDrv),
`DISHONORED_SDK_LAYOUT_CHECKS` (ON). The DishonoredGame library target is **`DishonoredGameModule`**
(the exe owns the name `DishonoredGame`; the `dishonored_module()` recipe is inlined for it — follow-up:
give `dishonored_module()` a TARGET argument). `Launch/CMakeLists.txt` links the four when present and defines
`DISHONORED_HAVE_GFXUI/AKAUDIO/OSS/DISHONOREDGAME`; `DishonoredStubs.cpp` keeps the empty hooks behind
`#if !DISHONORED_HAVE_*`. Agent P's D3D9Drv blocks are untouched. `GFxUI/Sources.cmake` and
`OnlineSubsystemSteamworks/Sources.cmake` exclude the Epic GFx-4 / Steamworks glue (`WITH_GFx=0`,
`WITH_STEAMWORKS=0`); `AkAudio/Sources.cmake` and `DishonoredGame/Sources.cmake` are generated (skeleton units excluded).
The module folders were renamed `inc/`→`Inc/`, `src/`→`Src/` (case only) so `Sources.cmake`'s `Src/x.cpp`
entries match the glob.

## Exe link

`snap_modules.cmd` also links `Binaries/Win32/DishonoredGame.exe` (64.6 MB) with `GFxUI.lib`, `AkAudio.lib`,
`OnlineSubsystemSteamworks.lib` and `DishonoredGameModule.lib` and `DISHONORED_HAVE_*=1`: 0 errors, 0 unresolved
symbols (every `AutoInitializeRegistrants*`/`AutoGenerateNames*`/`AutoCheckNativeClassSizes*` the reference
`LaunchEngineLoop.cpp` calls is now the generated one). Smoke (`build_and_smoke.py --build-dir build/agentT/snap/build
--no-build --rhi null --skip-native OnlineSubsystemPC`, log `build/agentT/snap_smoke.log`): **exit 0, milestone 1
reached** — `Init: Object subsystem initialized` at 0.71 s with the 1,942 DishonoredGame/shim + 37 GFxUI/AkAudio/OSS
native classes registered (`Presizing for 58555 objects not considered by GC`), then the known `SystemSettings.cpp:532`
checkf (agent O's step 2; the snapshot is HEAD without O's in-flight work). No registrant, name or size-check hook aborted.

## Hand-over for agent O

1. Configure with `-DDISHONORED_ENABLE_GFXUI=ON -DDISHONORED_ENABLE_AKAUDIO=ON -DDISHONORED_ENABLE_OSS=ON
   -DDISHONORED_ENABLE_DISHONOREDGAME=ON`; Launch then links the real `AutoInitializeRegistrants*` /
   `AutoGenerateNames*` / `AutoCheckNativeClassSizes*` of all four (the `DishonoredStubs.cpp` hooks compile out).
2. Drop `GFxUI`, `AkAudio`, `OnlineSubsystemSteamworks`, `DishonoredGame` from `--skip-native`
   (`OnlineSubsystemPC` stays: not generated, its 1 class is not in the four packages' business).
3. Engine.upk / GameFramework.upk: the 120 shim classes make their RF_Native exports bind; the remaining
   unbound ones are `UGameCrowdPopulationManager` (collides with the reference `AGameCrowdPopulationManager`:
   the tree registers "GameCrowdPopulationManager" as an Actor while retail has a 144-byte UObject — the
   cooked GameFramework.upk property list is the retail one) and, in DishonoredGame.upk,
   `DisGameCrowdPopulationManager`. Keep those two on the bridge/skip list.
4. Native functions: every `exec*` of the four packages is registered (`GNativeLookupFuncs`) and aborts with
   `"<Module> native not ported: Class::execFoo"` when script calls it — the abort names the first native to port.
5. Layout: the 45 xcheck rows / 327 pending asserts are all base deltas (table above); any `UClass::Link` size
   complaint for a DishonoredGame class while loading points at the same Engine bases, not at the generated
   layout.
6. `-DDISHONORED_LAYOUT_CHECKS=OFF` is not needed once `Engine/Inc/DishonoredLayouts.h` is regenerated for
   agent S's `FOnlinePlayerScore` (28); the generated modules' checks are separate (`DISHONORED_SDK_LAYOUT_CHECKS`).

## Follow-ups outside my files

- `resources/tools/sdk/parse_codered_sdk.py`: `DECL_RE` should accept `struct X : Y` (no `public`) — 171
  derived script structs (152 DishonoredGame, 19 Engine/GFxUI/OSS) are missing from `retail_sdk_layout.json`
  for everyone else (`xcheck` cannot check them). One-line fix: `(?:\s*:\s*(?:public\s+)?([A-Za-z_]\w*))?`.
- Engine (Q/R/S) types the generated modules wait on (table above): `ADynamicSMActor`/`AKActor`,
  `USeqAct_Latent`, `UDecalComponent`, `ADecalManager`, `ACamera`, `UInterpGroup(Inst)`, `USeqAct_Interp`,
  `USeqAct_MultiLevelStreaming`, `UGameViewportClient`, `FTViewTarget`, `FPolyReference`; IpDrv
  `UOnlineSubsystemCommonImpl` (204 vs 200); GameFramework `AGamePawn`, `AGamePlayerController`,
  `AGameCrowdAgentSkeletal`, `AGameCrowdDestination`, and the `AGameCrowdPopulationManager` → retail
  `UGameCrowdPopulationManager` (UObject, 144) change.
- The 120 shim classes (Engine/GameFramework package) belong in Engine/GameFramework eventually; the
  generated declarations (`DishonoredGameEngineShims.h`) can be pasted per class; regenerating removes them
  from the shim unit automatically once the tree declares them.
- `GFxUIClasses.h`, `GFxUIUIPrivateClasses.h`, `GFxUIUISequenceClasses.h`, `GFxUINames.h`, `GFxUI.h`,
  `OnlineSubsystemSteamworksClasses.h`, `OnlineSubsystemSteamworksNames.h`, `OnlineSubsystemSteamworks.h` are
  now generated (retail layout) and replace the reference (10897) versions; `Engine/Src/UnPlayer.cpp`,
  `UnEngine.cpp`, `UnGame.cpp` and `Core/Src/UnVcWin32.cpp` include `GFxUIClasses.h` and compile against the
  generated one (the retail `UGFxInteraction` etc. — only used under `WITH_GFx`). The Epic Scaleform/Steamworks
  glue sources need the retail-layout classes reworked when Phase 4 brings the SDKs (`middleware.md`).
- Event wrappers of tree interfaces implemented directly by generated classes (`eventNotifyPathChanged` of
  `IInterface_NavigationHandle` in 4 classes) are `appErrorf` stubs, not `ProcessEvent` calls: port with the class.
- `dishonored_module()` needs a TARGET-name parameter (DishonoredGame inline block in the root CMakeLists).
- Probe skip files created: `resources/docs/types/probe_skip_GFxUI.txt`, `probe_skip_OnlineSubsystemSteamworks.txt`
  (native-only types of the excluded Epic glue), `probe_skip_DishonoredGame.txt` (empty header).
- `gen_layout_probe.py generate` names the include-dir target after the module (`DishonoredGame` = the exe);
  it works because every module exports every `*/Inc` dir, but `MODULE_INCLUDES` could map to `DishonoredGameModule`.

## Environment hazards met (for the coordinator)

- `cmake/Dependencies.cmake` forces `FETCHCONTENT_BASE_DIR` to `<repo>/external`; a snapshot build whose
  source dir differs (agent Q's `build/agentQ/snap/src`) rewrote `external/*-subbuild/CMakeCache.txt` with its
  own paths and every later configure of the main tree failed ("CMakeCache.txt directory ... is different").
  I deleted `external/nvapi-subbuild/CMakeCache.txt` once (regenerated), then moved to a snapshot with its own
  `external/`. Snapshots should not share that directory.
- Case-only renames: the generated headers were written through the existing lowercase skeleton names
  (NTFS kept `dishonoredgameclasses.h`); `build/agentT_fixcase.py` renamed them and the `inc/`/`src/` folders.
  git tracks the old lowercase names — expect renames in the commit.
- The user's mid-run message "Can you respawn and send off all agents again the limit has been increased"
  reached this agent; it is for the coordinator, no action taken here.

## Files

- `resources/tools/symbols/gen_classes_header.py` (`--sdk` section, ~1,400 lines added), `gen_layout_probe.py`
  (`MODULE_INCLUDES` for the four modules).
- `source/Development/Src/DishonoredGame/{Inc/*.h (73 group headers, EngineShims, Names, Layouts, DishonoredGame.h),
  Src/DishonoredGameRegistrants.cpp, Src/DishonoredGameNativeStubs.cpp, Sources.cmake, DishonoredGameLayouts.pending.txt}`;
  same shape under `GFxUI`, `AkAudio`, `OnlineSubsystemSteamworks`.
- Root `CMakeLists.txt` (four module blocks + `DISHONORED_SDK_LAYOUT_CHECKS`), `Launch/CMakeLists.txt`
  (module links, `DISHONORED_HAVE_*`), `Launch/Src/DishonoredStubs.cpp` (lines 12-28 → `#if !DISHONORED_HAVE_*`).
- `resources/docs/types/probe_skip_{DishonoredGame,GFxUI,OnlineSubsystemSteamworks}.txt`.
- Scratch: `build/agentT/*.cmd`, `build/agentT/sync_snapshot.py`, `build/agentT_gen_*.json` (generation reports),
  `build/agentT_dlc_only.txt`, `build/agentT_padtest.cpp` (MSVC base-padding evidence), `build/agentT_debug_emu.py`.
