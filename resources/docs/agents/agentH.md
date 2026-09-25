# Agent H — Retail native class sizes (Phase 2b)

Scope: `sizeof()` of every native UE3 class in the **retail 2013 exe**, validated by running the same extraction on
the 2012 Shipping exe against the PDB. Databases: `resources/docs/idb/shipping2012_agentH.i64` (2012, image base 0)
and `resources/docs/idb/retail2013_agentH.i64` (2013, image base 0x400000), headless only. Every number below says
which exe it comes from; all addresses are RVAs unless marked VA.

## Files

| File | What |
|---|---|
| `resources/tools/ida/export_class_sizes.py` | Exporter (`run.py <script> <db> <out.csv>`) + `merge` sub-command (plain Python, no IDA) that joins the two exports, validates 2012 against `sizes.csv` and prints the markdown summary tables |
| `resources/docs/types/native_class_sizes.csv` | 2,883 rows = union of both exes' native classes. Columns `class, package_2013, size_2012, size_2013, delta, flags_2012, flags_2013, registrant_rva_2012, registrant_rva_2013` first, then `package_2012, super_2012, super_2013, within_2013, cast_flags_*, other_flags_*, config_2013, staticclass_rva_*, init_rva_*, container_rva_*, ctor_rva_*, pdb_size_2012` |
| `resources/docs/types/native_class_sizes.md` | Method, validation, totals, Core contract types, `UClass` analysis, size changes per package, flag/super changes, only-2013 (345) and only-2012 (26) lists |

Scratch (not in the repo): per-exe exports `%TEMP%\agentH\classes_2012.csv` / `classes_2013.csv`, ctor decompiles.

## Acceptance

| Criterion | Result |
|---|---|
| 2012 sizes match the PDB for >= 99 % of classes | **2,538 / 2,538 (100 %), 0 mismatches**, every class found in `sizes.csv` |
| 2013 table has >= 2,500 classes | **2,857** |
| `UObject`, `UClass`, `AActor` rows with both sizes | `UObject` 56/56, `UClass` 456/**436**, `AActor` 592/592 |
| `ADisPlayer` row | no such class exists in either exe (nor in the PDB); the player classes are `ADishonoredPlayerPawn` 3856/**4688** and `ADisPlayerControllerBase` 1344/1344 |

## How the retail exe registers classes (the part the plan got wrong)

The plan assumed the 2013 exe pushes `sizeof` and the name literal at every ctor call site like 2012 does. It does not:

- **2012**: `X::GetPrivateStaticClassX(const TCHAR* Package)` pushes the 13 ctor arguments (2,538 sites of the named
  `UClass::UClass(EStaticConstructor, ...)`, rva 0x7a1a0). `Package` is a parameter that `X::StaticClassNoInline`
  pushes as a literal, so the package comes from the registrant's single caller.
- **2013**: `X::StaticClass()` does `push offset desc; call GetPrivateStaticClass` (common, rva 0x79370, 2,857 sites).
  `desc` is a 12-dword table in `.data` holding `InSize, InClassFlags, InOtherClassFlags, InClassCastFlags, InNameStr,
  InPackageName, InConfigName, InFlags(lo, hi), ctor, static ctor, static init` in argument order; the common function
  `rep movsd`s it to the stack, pushes the 12 dwords plus `EStaticConstructor`, and calls the ctor (rva 0x77060) with
  `this = desc`. So the descriptor is the first 48 bytes of the in-place `UClass` storage (2012's `s_UClassContainer`).
- Both: `InitializePrivateStaticClassX` = `call Within::StaticClass; push; mov eax, X::PrivateStaticClass; push;
  call Super::StaticClass; push; call InitializePrivateStaticClassCommon` (2013 common rva 0x24880).

The exporter finds the target generically (function with >= 1,000 code xrefs whose sampled callers classify as
either shape), decodes each site in the shape it has, and resolves Super/Within by mapping `StaticClass` addresses.
It runs unchanged on both databases; the 2012 run is the proof (2,538/2,538 sizes, and every literal-derived class
name equals the PDB symbol).

Caveats handled: 44 (2013) / 49 (2012) class-name literals have no `U`/`A` character in front of them (pooled
`L"Name"` instead of `L"UName"+1`; `AActor`, `UWorld`, `ADishonoredNPCPawn` among them). The prefix is derived from
the super chain (`AActor` subtree = `A`); on 2012 that rule gives the PDB prefix letter for all 49. Two 2013
`StaticClass` bodies are tail chunks of a neighbouring function in the bare analysis; the script therefore works on
function chunks. Zero call sites were rejected in either exe.

Independent consistency check (both exes): in-place `UClass` containers in `.data` are never closer than
`sizeof(UClass)` rounded to 8: 2012 min spacing 456 (= 456), 2013 min spacing 440 (= 436 rounded up). The retail
`UClass` size read from the descriptors agrees with how the retail linker laid out the containers.

## Findings (2013 retail vs 2012)

- 2,512 shared classes, **274 changed size** (255 DishonoredGame, 14 Engine, 1 Core, 1 GameFramework, 1 IpDrv,
  1 OnlineSubsystemSteamworks, 1 AkAudio). 345 classes only in 2013 (132 `DLC05*`, 91 `DLC06*`, 103 `DLC07*`, 19
  new bases/interfaces such as `UDisBehaviorAttentionBase`, `ADisNPCAttachment`), 26 only in 2012 (fluid surface /
  foliage feature set, `GameplayEvents` stack, four `DEPRECATED_` classes).
- **Core contract types: all unchanged except `UClass` 456 -> 436.** `UObject` 56, `UField` 60, `UStruct` 128,
  `UState` 200, `UFunction` 160, `UProperty` 108 and the whole property family, `UEnum` 72, `UScriptStruct` 156,
  `UPackage` 228, `ULinker` 356, `ULinkerLoad` 1832, `ULinkerSave` 524, `UConst` 72, `UTextBuffer` 92, `USystem` 260,
  `UMetaData` 116, `UObjectRedirector` 60: identical in both exes.
- **`UClass` -20** (from diffing the two `UClass::UClass` decompiles, `native_class_sizes.md` has the table):
  `m_DropdownCategory` (FName, 8, @332 in 2012) is gone, and one 12-byte member of the
  `ClassReps ... ClassHeaderFilename` block is gone (22 zeroed dwords instead of 25; `ClassHeaderFilename` is the
  likely one, but the ctor cannot tell — `UClass::Serialize` in the retail exe can, via the retail `UClass` vftable at
  rva 0xbbe218; L's `UClass::Serialize` port should settle it). Consequences for `Core/Inc/UnClass.h`:
  `ClassDefaultObject` 328 -> 316, `ClassConstructor` 340 -> 320, `ComponentNameToDefaultObjectMap` 352 -> 332,
  `Interfaces` 412 -> 392, `m_pInterfaceOffsets` 424 -> 404, `DefaultPropText` 428 -> 408, `bNeedsPropertiesLinked`
  440 -> 420, `ReferenceTokenStream` 444 -> 424, size 436. `ClassFlags | 0x80`, `InOtherClassFlags`, `ClassConfigName`
  at 220 and `bNeedsPropertiesLinked = 1` are unchanged.
- Engine: `UTexture2D` +4 (368 -> 372; `ULightMapTexture2D`, `UShadowMapTexture2D`, `UTextureFlipBook` follow),
  `USkeletalMeshComponent` +32 (1056 -> 1088), `AGamePawn` and `AMatineePawn` +16 (1184 -> 1200; `APawn` itself unchanged),
  `UOnlineSubsystem` +12, `UOnlineSubsystemCommonImpl` +16, `UOnlineSubsystemSteamworks` -152,
  `UDownloadableContentManager` +12, `UArkDLCManagementBridge` +16. `AActor`, `APawn`, `UWorld`, `ULevel`, `UPrimitiveComponent`,
  `UStaticMesh`, `USkeletalMesh`, `UMaterial`, `UTexture` are unchanged (useful for M's Engine probe).
- DishonoredGame: pawn hierarchy grew (`ADishonoredPawn` +192, `ADishonoredNPCPawn` +256, `ADishonoredPlayerPawn`
  +832, `ADisTallboyNPCPawn` +240, `ADisPossessablePawn` +192), `ADishonoredUsableObject` +16 (whole subtree),
  `ADishonoredGameInfo` +48, `UDisConvGlobalMan` +28, `UDisConv_*` nodes +8.
- `InFlags` = `0x0408408400004000` and `InOtherClassFlags` = 0 for every class in both builds; packages identical
  for all shared classes; 2 `ClassFlags` changes; 5 super changes (four `UDisBehavior*` re-parented under the new
  `UDisBehaviorAttentionBase`, `ADisTallboyAttachment` under the new `ADisNPCAttachment`).

## Handles for the other packages (2013 retail, VA = rva + 0x400000)

| Function / data | rva | Evidence |
|---|---|---|
| `GetPrivateStaticClass(desc)` common | 0x79370 | 2,857 call sites, copies 12 dwords, calls the ctor |
| `UClass::UClass(EStaticConstructor, ...)` | 0x77060 | decompile matches the 2012 ctor field by field |
| `UState::UState(EStaticConstructor, ...)` | 0x76f70 | first call of the ctor |
| `InitializePrivateStaticClassCommon` | 0x24880 | 3-arg callee of every init function |
| `UObject::StaticClass` / `PrivateStaticClass` | 0x26f30 / data 0x1023430 | `Within` of 2,621 classes |
| `UObject::InternalConstructor` | 0x18900 | `ctor` field of the `UObject` descriptor |
| No-op static ctor / initializer (`retn`) | 0x59d10 | 2012 equivalent 0x57ef10 (folded `UDishonoredTask_Base::OnAdded_Impl`) |
| `TSparseArray` ctor used by `ComponentNameToDefaultObjectMap` | 0x6d77f0 | called from the ctor at `this + 0x14c` |
| `UClass` vftable | 0xbbe218 | stored by the ctor |
| Per class: `StaticClass`, `InitializePrivateStaticClass`, `InternalConstructor`, descriptor / `UClass` storage | `native_class_sizes.csv` columns `staticclass_rva_2013`, `init_rva_2013`, `ctor_rva_2013`, `container_rva_2013` | J can name 4 x 2,857 functions/objects from this table without matching |

## Coordinator notes

- `gen_layout_asserts.py --sizes native_class_sizes.csv:size_2013` can run as planned; only `UClass` changes in Core.
- The 2013 DLC05–07 classes are in the CSV with sizes and supers (`size_2012` empty); the `DishonoredGameClasses.h`
  generator for wave 2 needs I's member lists for them.
- Nothing was committed; only the three files above plus this report were created.
