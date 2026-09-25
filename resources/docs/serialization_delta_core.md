# Serialization convergence report: Core (Dishonored 2012 Shipping vs UE3 10897)

Agent E, 2026-09-25. Input: Hex-Rays pseudocode of the 2012 Shipping exe (`resources/reference/decomp/core_serialize/`,
database copy `shipping2012_agentE.i64`) diffed by hand against `../UnrealEngine3/Development/Src/Core/Src`.
Cooked packages are file version 801 / licensee 30 (`symbols/package_summary.md`). Every engine-version threshold in
Core is below 801, so for Dishonored's own packages every `Ver() >= N` branch is taken and every `Ver() < N` branch is
dead; what matters is *which fields the reference reads that Dishonored does not* (and vice versa).

## 0. Facts pulled from the exe (static initializers, `_read_globals.py`)

| global | 2012 Shipping | our `UnObjVer.cpp` | reference 10897 |
|---|---:|---:|---:|
| `GEngineVersion` | 9014 | 9411 (2013 exe) | 10897 |
| `GPackageFileVersion` | 801 | 801 | 868 |
| `GPackageFileMinVersion` | 491 | 491 | `VER_MIN_ENGINE` |
| `GPackageFileLicenseeVersion` | 30 | 30 | 0 |
| `GPackageFileCookedContentVersion` | 132 | 133 (2013 packages) | 136 |
| `GEngineMinNetVersion` / `GEngineNegotiationVersion` | 7039 / 3077 | 9188 / 3077 | |
| `GBaseCompressionMethod` | **2 = COMPRESS_LZO** | | `COMPRESS_ZLIB` on PC |
| `GSavingCompressionChunkSize` | 131072 | | 131072 |
| `GSaveDataSavingCompressionChunkSize` (Arkane) | 65536 | (missing) | (missing) |

All shipped packages have `PKG_StoreCompressed` (0x02000000) and `CompressionFlags = 2` (LZO). Package load therefore
needs an LZO1X decompressor before anything else works; `WITH_LZO=0` (STATUS.md) is a milestone-3 blocker.

## 1. Version-number map

Numbers seen in Dishonored's Core serialization code and the reference `VER_*` name (`Core/Inc/UnObjVer.h`):

| number | reference enum | used by |
|---:|---|---|
| 516 | `VER_ADDITIONAL_COOK_PACKAGE_SUMMARY` | `FPackageFileSummary` (AdditionalPackagesToCook) |
| 536 | `VER_FIXED_PREFAB_SEQUENCES` | `ULinkerLoad::RemapClasses` |
| 543 | `VER_REMOVED_COMPONENT_MAP` | `operator<<(FObjectExport&)` |
| 550 | `VER_NEW_CURVE_AUTO_TANGENTS` | `UStruct::SerializeTaggedProperties` |
| 566 | `VER_REDUCED_STATEFRAME_LATENTACTION_SIZE` | `UObject::Serialize` |
| 584 | `VER_ASSET_THUMBNAILS_IN_PACKAGES` | `FPackageFileSummary` (ThumbnailTableOffset) |
| 591 | `VER_REMOVED_DEFAULT_SKELETALMESHACTOR_COLLISION` | `UStruct::SerializeTaggedProperties` |
| 603 | `VER_DONTSORTCATEGORIES_ADDED` | `UClass::Serialize` |
| 623 | `VER_ADDED_CROSSLEVEL_REFERENCES` | `FPackageFileSummary` (ImportExportGuids*) |
| 633 | `VER_BYTEPROP_SERIALIZE_ENUM` | `FPropertyTag`, `SerializeTaggedProperties` |
| 639 | `VER_USTRUCT_SERIALIZE_ONDISK_SCRIPTSIZE` | `UStruct::Serialize` |
| 655 | `VER_SCRIPT_BIND_DLL_FUNCTIONS` | `UClass::Serialize` (dummy FName) |
| 673 | `VER_PROPERTYTAG_BOOL_OPTIMIZATION` | `FPropertyTag` |
| 691 | `VER_REDUCED_PROBEMASK_REMOVED_IGNOREMASK` | `UObject::Serialize` |
| 749 | `VER_FORCE_SCRIPT_DEFINED_ORDER_PER_CLASS` | `UClass::Serialize` |
| 756 | `VER_MOVED_SUPERFIELD_TO_USTRUCT` | `UField::Serialize`, `UStruct::Serialize` |
| **770** | Dishonored's `VER_TEXTURE_PREALLOCATION`; reference has it at 767 (reference 770 = `VER_COMPACTKDOPSTATICMESH`) | `FPackageFileSummary` (TextureAllocations) |
| **796** | **Arkane-only**: `UClass::m_OtherClassFlags` (reference 796 = `VER_FIXED_AUTO_SHADER_VERSIONING`, unrelated) | `UClass::Serialize` |
| licensee **10** | Arkane: `UClass::m_DropdownCategory` | `UClass::Serialize` |

Reference-only thresholds that Dishonored does **not** have and that would misread 801 packages: `VER_ADDED_CLASS_GROUPS`
(789, `UClass::ClassGroupNames`). Reference-only but dead for 801 packages: `VER_FIXUP_MOBILEGAME_REFS` (822),
`VER_RENAME_MOBILEGAME_TO_SIMPLEGAME` (827) in `RemapClasses`. Do not carry the 10897 `VER_*` enum verbatim: the
QA-branch numbering diverges above ~766 (767 vs 770) and Arkane reused 796. Recommended: keep the reference enum up to
766, then add `VER_DIS_TEXTURE_PREALLOCATION = 770`, `VER_DIS_OTHER_CLASS_FLAGS = 796`, `VER_LATEST_ENGINE = 801`,
`VER_DIS_DROPDOWN_CATEGORY (licensee) = 10`, `VER_LATEST_ENGINE_LICENSEE = 30`.

Other constants confirmed identical: `PACKAGE_FILE_TAG` 0x9E2A83C1, `DVD_ECC_BLOCK_SIZE` and
`LOADING_COMPRESSION_CHUNK_SIZE` 0x20000, `ExportHash[256]`, `HashNames(A,B,C) = A + 7*B + 31*C`,
`FNameEntry` header 16 bytes (Flags QWORD, Index with unicode bit 0, HashNext), name-entry pool pages 64 KB,
`NameHash` bucket count 65536, cross-level pointer encoding `(Index & 0xFF000000) == 0xF0000000`. The inlined
`IsPackageCookedForConsole` tests `GPatchingTarget & 0x28C` (`PLATFORM_Console`) and property stripping tests
`0x2CE` (`PLATFORM_Stripped`); check `UnFile.h` gives the same values after the port.

## 2. Layout deltas that change serialization (PDB `types.json` vs reference headers)

- **`UObject` has no `NetIndex`** (56 bytes: vtable, Index, ObjectFlags, HashNext, HashOuterNext, StateFrame,
  _Linker, _LinkerIndex, Outer, Name, Class, ObjectArchetype; reference inserts `INT NetIndex` before `Outer`).
  Net-relevance is ObjectFlags bit 63 (`0x8000000000000000`, the reference's `RF_CookedStartupObject` slot; Arkane
  name unknown, call it `RF_DisNetIndexed`). `SetNetIndex(i)` sets/clears the bit, `SerializeNetIndex` writes 0/-1.
- **`FArchive`** = 136 bytes: reference members in the reference order (ArVer … ArIsFilterEditorOnly, 132 bytes)
  plus Arkane `UBOOL ArIsDisSaveLoad` at 132. `FArchive::Reset` must clear it.
- **`UClass`**: `m_OtherClassFlags` (DWORD, offset 204, directly after `ClassFlags`), `m_DropdownCategory` (FName,
  offset 332, after `ClassDefaultObject`), `m_pInterfaceOffsets` (424); no `ClassGroupNames`, no `DLLBindName`.
- **`FStateFrame`** (64 bytes): FFrame(40) + StateNode, ProbeMask, LatentAction(WORD), bContinuedState(BYTE),
  StateStack. Reference layout is the same, but the reference *serializes state locals*
  (`UObject::SerializeStateLocals`) and Dishonored does not.
- **`ULinkerLoad::FPackagePrecacheInfo`** = 24 bytes: reference 3 members + Arkane
  `FThreadSafeCounter* PatchSynchronizationObject; void* PatchPackageData; INT PatchPackageDataSize` (bsdiff patch
  applied in `CreateLoader` via `ArkBsPatch`).
- **`EStructFlags`**: Arkane `STRUCT_DevLoad = 0x200` (used by `UScriptStruct::SerializeBin`).
- **`FIOSystem::LoadCompressedData`** takes a trailing `EAsyncIORequestType` (`AIORT_Bink=0, AIORT_MipMap=1,
  AIORT_Wwise=2, AIORT_Other=3`); `LoadData` does not. `FAsyncPackage` has no `PackageType`;
  `ProcessAsyncLoading`/`FlushAsyncLoading` have no `ExcludeType` parameter. `UPackage` has no `FileName`.
- `FPackageFileSummary` (164), `FObjectExport` (92), `FObjectImport` (40), `FGenerationInfo` (12),
  `FCompressedChunk` (16), `FCompressedChunkInfo` (8), `FPropertyTag` (48), `FNameEntry` (2064), `FArchiveAsync`
  (204), `FAsyncPackage` (108), `UStruct`/`UState`/`UFunction`/`UProperty`: member order matches the reference.
- `FScriptPatcher` is a 12-byte stub (`TArray<FLinkerPatchData*> PackageUpdates`); no script patches ship.

## 3. Per-function delta

Priority key: **P0** blocks package load, **P1** blocks script classes (UClass/UStruct/CDO loading), **P2** cosmetic
(dead for cooked 801 packages, editor/console-only, logging, or only affects saving). "Reference" line = file:line to
edit in `source/Development/Src/Core/Src` (same numbering as the reference tree).

### 3.1 UnLinker.cpp: package format structs

**`operator<<(FArchive&, FPackageFileSummary&)`** (rva 0x5a1a0) — status: identical (thresholds renumbered). P2.
- Field order identical: Tag, FileVersion, TotalHeaderSize, FolderName, PackageFlags (`PKG_FilterEditorOnly`
  0x80000000 → `SetFilterEditorOnly`), NameCount/Offset, ExportCount/Offset, ImportCount/Offset, DependsOffset,
  [>=623] ImportExportGuidsOffset, ImportGuidsCount, ExportGuidsCount, [>=584] ThumbnailTableOffset, Guid,
  GenerationCount + Generations (3 INTs each), EngineVersion, CookedContentVersion (0 when saving outside cooking),
  CompressionFlags, CompressedChunks, PackageSource, [>=516] AdditionalPackagesToCook, [>=**770**] TextureAllocations.
- Only delta: TextureAllocations threshold 770 vs reference 767 (both true for 801). Tag-swap handling identical.
- Reference: UnLinker.cpp:377.

**`FGenerationInfo::Serialize`** — inlined into the summary operator; identical (ExportCount, NameCount,
NetObjectCount). UnLinker.cpp:363. P2.

**`operator<<(FArchive&, FObjectExport&)`** (0x6a420) — identical. ClassIndex, SuperIndex, OuterIndex, ObjectName,
ArchetypeIndex, ObjectFlags (QWORD), SerialSize, SerialOffset, [<543 legacy component map], ExportFlags,
GenerationNetObjectCount, PackageGuid, PackageFlags. UnLinker.cpp:267. P2.

**`operator<<(FArchive&, FObjectImport&)`** (0xd010) — identical. UnLinker.cpp:316. P2.

**`operator<<(FArchive&, FCompressedChunk&)`** (inlined in the `TArray<FCompressedChunk>` operator, 0x42a90) —
identical (UncompressedOffset, UncompressedSize, CompressedOffset, CompressedSize). UnLinker.cpp:342. P2.

**`operator<<(FArchive&, FTextureAllocations&)` / `FTextureType&`** (Engine, 0x192d20 / 0x1844e0) — identical to
Engine/Src/Texture2D.cpp:228 ff. (FTextureType: SizeX, SizeY, NumMips, Format, TexCreateFlags, ExportIndices). P2.

**`operator<<(FArchive&, FNameEntry&)`** (0x3f9d0) — identical. Loading: INT length, negative = UTF-16
(`Index |= 1`), raw `Serialize` of the buffer at entry offset 16 (no `appSerializeUnicodeString` byte-swap; fine on
PC), then `Flags` QWORD (Dishonored serializes the real `Flags`, i.e. `SUPPORT_NAME_FLAGS` behaviour). UnName.cpp:983. P2.

**`operator<<(FArchive&, FPropertyTag&)`** (0x4a80), `FPropertyTag::FPropertyTag` (0x87870),
`FPropertyTag::SerializeTaggedProperty` (0x80f70) — identical, including thresholds 633 (EnumName for
`NAME_ByteProperty`) and 673 (BoolVal as BYTE). `NAME_StructProperty`=10, `NAME_BoolProperty`=3, `NAME_ByteProperty`=1
as in `UnNames.h`. Core/Inc/UnPropertyTag.h:34-119. P2.

### 3.2 UnLinker.cpp: ULinkerLoad creation

**`ULinkerLoad::Tick`** (0xa25d0) — differs. **P0** (compile/port) but behaviourally harmless.
- Step order: CreateLoader, SerializePackageFileSummary, SerializeNameMap, SerializeImportMap, SerializeExportMap,
  StartTextureAllocation, IntegrateScriptPatches, FixupImportMap, RemapClasses,
  RemapLinkerPackageNamesForMultilanguageCooks, SerializeDependsMap, SerializeGuidMaps, CreateExportHash,
  FindExistingExports, FinalizeCreation.
- Dishonored has **no `FixupExportMap()`** step (reference 10897 addition, UnLinker.cpp:5411, `!FINAL_RELEASE`
  redirect tables) and no `RemapLinkerPackageNames()` (`SUPPORTS_SCRIPTPATCH_CREATION`). Drop both.
- Reference: UnLinker.cpp:951.

**`ULinkerLoad::ULinkerLoad(UPackage*, const TCHAR*, DWORD)`** (0x7b030) — identical (member init list incl.
`_LOC_<lang>.` seek-free check). UnLinker.cpp:1099. P2.

**`ULinkerLoad::CreateLoader`** (0x898a0) — differs. **P0** for layout, logic optional.
- No `CreateActiveRedirectsMap` (reference `!FINAL_RELEASE`).
- Precached-package path (`PackagePrecacheMap.Find(*Filename)`): after waiting on `SynchronizationObject`, if
  `PatchPackageDataSize > 0` it waits on `PatchSynchronizationObject`, calls
  `ArkBsPatch(PackageData, PackageDataSize, PatchPackageData, PatchPackageDataSize, &OutBuffer, &OutLen)`,
  frees both inputs and replaces `PackageData/PackageDataSize` with the patched buffer (Arkane bsdiff package
  patching; only reachable through `AsyncPreloadPackage`). Then `new FBufferReaderWithSHA(...)` and
  `PackagePrecacheMap.Remove` as in the reference.
- Remaining branches identical: seek-free fully-compressed error, `.uncompressed_size` manifest path
  (`SerializeCompressed(Buffer, Size, GBaseCompressionMethod)`), `LOAD_MemoryReader`/non-seek-free file reader with
  SHA preload, `FArchiveAsync` for seek-free, `LinkerExists` check, `ArVer/ArLicenseeVer/ArIsLoading/ArIsPersistent/
  ArForEdit/ArForClient/ArForServer`, `GWarn->UpdateProgress(1,6)`, precache `Min(0x20000, TotalSize)`.
- Load-flag values identical to `ELoadFlags` (SeekFree 1, NoWarn 2, Throw 8, Verify 0x10, AllowDll 0x20,
  NoVerify 0x80, Quiet 0x2000, FindIfFail 0x4000, MemoryReader 0x8000, RemappedPackage 0x10000).
- Reference: UnLinker.cpp:1164. Port: extend `FPackagePrecacheInfo`, stub `ArkBsPatch` (no bsdiff library needed
  until patch files are supported).

**`ULinkerLoad::SerializePackageFileSummary`** (0x8a330) — identical logic. P2.
- Same: `PKG_Cooked` → `ThisContainsCookedData` on linker and loader; editor clears
  `PKG_RequireImportsAlreadyLoaded` (+`PKG_Cooked` for make); `Loader->ArVer = FileVersion & 0xffff`,
  `ArLicenseeVer = FileVersion >> 16`; `PKG_StoreCompressed` → `SetCompressionMap`, on failure delete loader and
  create `FArchiveAsync` (seek back, propagate byte swapping, set map again); `LinkerRoot->PackageFlags =
  Summary.PackageFlags & ~PKG_Trash`; `FolderName`; `EngineVersion > GEngineVersion` → `PKG_SavedWithNewerVersion`
  (reference also `debugf`s outside the editor); `ArAllowLazyLoading`; `__Trashcan`; tag / `GPackageFileMinVersion`
  / `GPackageFileVersion`+`GPackageFileLicenseeVersion` throws (Dishonored omits the `warnf` and passes fewer format
  args); `ImportMap/ExportMap/NameMap.Empty(count)`; `UpdateProgress(2,6)`.
- Reference: UnLinker.cpp:1351.

**`ULinkerLoad::SerializeNameMap`** (0x5a4a0) — identical (precache `NameOffset..TotalHeaderSize`, 100-granularity
time limit, `FName(ENAME_LinkerConstructor, ...)` = `Init(name, 0, FNAME_Add, bSplitName=FALSE)`). UnLinker.cpp:1522. P2.

**`ULinkerLoad::SerializeImportMap`** (0x30860) / **`SerializeExportMap`** (0x6a6a0) — identical; the export map
version lacks the `!WITH_FACEFX` editor FaceFX scan (editor-only). UnLinker.cpp:1572 / 1792. P2.

**`ULinkerLoad::FixupImportMap`** (0x30a00) — identical to the `FINAL_RELEASE` path: `SoundCueLocalized`(904) class
import with Engine(21) outer → `SoundCue`(905); `ClassName==SoundCueLocalized && ClassPackage==Engine` → SoundCue;
`SequenceObjects`(842) package → `Engine`; `ClassPackage==SequenceObjects` → Engine. No `ObjectNameRedirects`.
UnLinker.cpp:1593. P2 (build with the `!FINAL_RELEASE` block off).

**`ULinkerLoad::RemapClasses`** (0x7b200) — differs, dead for 801. P2. Only the `< 536 VER_FIXED_PREFAB_SEQUENCES`
prefab-sequence fixup (identical body, no `warnf`); the reference's `< 827` MobileGame/CastleGame → SimpleGame/UDKBase
remap does not exist. UnLinker.cpp:1839.

**`ULinkerLoad::IntegrateScriptPatches`** (0x8b250) / **`GetScriptPatcher`** (0x48db0) — effectively identical: the
patcher is a stub, `GetLinkerPatch` never succeeds, all six `bHasIntegrated*` flags are set and
`UpdateProgress(4,6)` fires. Port `GetScriptPatcher` as `if (!ScriptPatcher) ScriptPatcher = new FScriptPatcher;`
with the 12-byte stub type. UnLinker.cpp:2027 / 836. P2.

**`ULinkerLoad::SerializeDependsMap`** (0x48e00) — identical (skipped when `GUseSeekFreeLoading` or not
editor/commandlet). UnLinker.cpp:2236. P2.

**`ULinkerLoad::SerializeGuidMaps`** (0x5a810) — identical (`LinkerRoot->ImportGuids` FLevelGuids{FName,
TArray<FGuid>}, `ExportGuidsAwaitingLookup` TMap<FGuid,INT>). Reference name for the task's
"SerializeImportExportGuids". UnLinker.cpp:2269. P2.

**`ULinkerLoad::SerializeThumbnails`** — not present in the exe (editor only). Keep the reference under `WITH_EDITOR`.

**`ULinkerLoad::CreateExportHash`** (0x65d60) / **`FindExistingExports`** (0x7b980) / **`FinalizeCreation`**
(0xa22e0) — identical (hash = `(ObjectName + 7*ClassName + 31*ClassPackage) & 0xFF`; `GObjLoaders.AddItem`,
`InitNetInfo`, patched-export net counts, `PackageSource != appStrCrcCaps(basename)` → `GWasUserCreatedContentLoaded`
else `PKG_NoExportAllowed`, `Verify()` unless `LOAD_NoVerify`, `RF_Public`). UnLinker.cpp:2425 / 2457 / 2495. P2.

**`ULinkerLoad::StartTextureAllocation`** (Engine, Texture2D.cpp:307, rva 0x193010) — differs, P2: Dishonored only
walks `TextureTypes[NumTextureTypesConsidered..]` calling `WillTextureBeLoaded` and bumping
`NumExportIndicesProcessed`; it never calls `UTexture2D::CreateResourceMem` / touches `PendingAllocationCount`
(PC has no resource-memory preallocation). Engine pass.

### 3.3 UnLinker.cpp: object creation

**`ULinkerLoad::Serialize(void*, INT)`** (0xd150) — identical (`Loader->Serialize`). UnLinker.cpp:4998. P2.

**`ULinkerLoad::operator<<(FName&)`** (0x30d30) — identical (`Bad name index %i/%i` appErrorf, NAME_None short
circuit still consumes the Number INT). UnLinker.cpp:~4960. P2.

**`ULinkerLoad::operator<<(UObject*&)`** (0x9bca0) — identical (cross-level `0xF0000000` decode via
`PotentialCrossLevelOwner/Property`, `ResolveCrossLevelReference`, else `IndexToObject`). UnLinker.cpp:4784. P2.

**`ULinkerLoad::Preload`** (0x94670) — identical: `RF_NeedLoad` (bit 41) guard, super-struct preload, script-patch
loader swap (`EF_ScriptPatcherExport` 2 → `PatchDataAr`, else `OriginalLoader` unless `GIsScriptPatcherActive`),
`Seek/Precache(SerialOffset, SerialSize)`, clear `RF_NeedLoad`, CDO → `InitClassDefaultObject(Class, 0, 0)` +
`SerializeDefaultObject`, else `GSerializedObject` + `Serialize`, `SerialSize` mismatch appErrorf, restore pos,
CDO && !make → `LoadConfig(); LoadLocalized(NULL, TRUE)`, else class with defaults → `Preload(CDO)`. No perf
trackers. UnLinker.cpp:3662. P2.

**`ULinkerLoad::CreateExport`** (0x9e460) — identical (all branches verified: editor forced-export path, class
lookup, `RF_Native` (bit 58) sanity, intrinsic/`ConditionalLink`, deprecated/transient editor errors, forced-export
`CreatePackage`+`InitNetInfo`, outer/archetype circularity, `VerifyImport` of archetype for non-cooked, CDO template
(`NAME_Object`=151 → own CDO), find-in-memory condition incl. `bShouldFindExportsInMemoryFirst`,
`bNeedsPropertiesLinked`, `RF_Load` mask `0x067F0125_00080700`, `HACK_VerifyObjectReferencesOnly` 0x80,
`RF_NeedLoad|RF_NeedPostLoad|RF_NeedPostLoadSubobjects`, `GIsInitialLoad` → `RF_RootSet|RF_DisregardForGC`,
`StaticConstructObject(Class, Outer, Name, Flags, Template, GError, NULL, NULL)`, `SetLinker`, `GObjLoaded`,
`EF_MemberFieldPatchPending` 4 → `RF_PendingFieldPatches` (bit 21), `SuperStruct`, `Bind()`). Only `warnf`/`debugf`
text is stripped. UnLinker.cpp:3891. P2.

**`ULinkerLoad::CreateImport`** (0x9b690) — differs, dead code. P2. Dishonored's in-memory lookup runs when
`(!GIsEditor && !GIsUCC) || (Import.ClassName == NAME_Class && IS_IMPORT_INDEX(OuterIndex) &&
OuterImport.ObjectName == "GearGameContentWeapons")`, and when the class lookup fails for
`FindClass == UClass && FindOuter->GetFName() == "GearGameContentWeapons"` it retries in package `GearGame` (Epic
Gears-of-War hack that 10897 removed). Everything else (`VerifyImport` when `SourceLinker == NULL`,
`SourceLinker->CreateExport(SourceIndex)`, `GImportCount`) identical. Keep the reference body. UnLinker.cpp:4307.

**`ULinkerLoad::VerifyImport`** (0xa0710) / **`VerifyImportInner`** (0x9d8c0) — identical (redirector follow with
`NAME_ObjectRedirector`=165/`NAME_Core`=20, `LOAD_Throw|(LoadFlags & 0x12092)`, `LOAD_FindIfFail` for make,
`bIsGatheringDependencies` → `LOAD_NoVerify`, private-import `SafeReplace` scan, `FailedImportPrivate` throw,
native-transient `RF_Public|RF_Native|RF_Transient` acceptance). The `try/catch(LOAD_FindIfFail)` in the reference is
not visible in pseudocode (catch funclets are not decompiled); keep it. UnLinker.cpp:2960 / 3118. P2.

**`ULinkerLoad::LoadAllObjects`** (0xa0bc0) / **`IndexToObject`** (0x9bb50) — identical (`LOAD_SeekFree` forces
preload; `IsTemplate` = outer chain with `RF_ClassDefaultObject|RF_ArchetypeObject`; `MarkAsFullyLoaded`).
UnLinker.cpp:3396 / 4425. P2.

### 3.4 UnObj.cpp / UnClass.cpp / UnProp.cpp: object serializers

**`UObject::Serialize`** (0x949c0) — identical: `RF_DebugSerialize`, `Preload(Class)` + `ConditionalLink` on load
+ `Preload(CDO)` when `!RF_ClassDefaultObject && PropertiesSize > 0`; GC/transacting name-outer-class-linker-archetype
paths; `RF_HasStack` (bit 57) → `StateFrame` (64-byte `new FStateFrame(this)`), `Node`, `StateNode`, ProbeMask
(`>=691` DWORD else QWORD+recompute), LatentAction (`>=566` WORD else INT), `StateStack`, code offset with
`%s: Offset mismatch: %i %i`; `IsAComponent()` → `UComponent::PreSerialize`; `SerializeNetIndex`;
`SerializeScriptProperties` unless the object is a `UClass`; transacting `RF_UndoRedoMask` (`0x20000000` in the
high DWORD); `CountBytes(Align(PropertiesSize, MinAlignment))`. UnObj.cpp:1596. P2.

**`UObject::SerializeNetIndex`** (0x20180) + **`UObject::SetNetIndex`** (0x14d30) — differs. **P0** (layout).
- Skipped when `ArPortFlags & PPF_Duplicate` (0x1000) as in the reference.
- Writes `INT InNetIndex = (ObjectFlags & (1ull<<63)) ? 0 : INDEX_NONE` (there is no `NetIndex` member).
- On load: if `_Linker && _Linker->LinkerRoot && !(LinkerRoot->PackageFlags & PKG_Cooked)` →
  `if (_LinkerIndex != INDEX_NONE) SetNetIndex(_LinkerIndex)`; else `SetNetIndex(InNetIndex)`. (Reference tests the
  cooked/NULL case first with `SetNetIndex(InNetIndex)`; same result.)
- `SetNetIndex(i)`: `i == INDEX_NONE` clears bit 63, otherwise sets it; no `UPackage::Add/RemoveNetObject`.
- Reference: UnObj.cpp:1565 and 1058. Remove `NetIndex` from `UObject` (UnObjBas.h:1133) and `GetNetIndex()`.

**`UObject::SerializeScriptProperties`** (0x8a8f0) — differs. **P1**.
- Identical up to the bin/tagged dispatch (`DiffObject = ObjectArchetype`, CDO → `Class->GetSuperClass()` as
  defaults struct, `ArPortFlags != 0` → `SerializeBinEx` with `DiffCount = DiffObject->Class->PropertiesSize`, else
  `SerializeBin`).
- **No `SerializeStateLocals` call** (reference `if (HasAnyFlags(RF_HasStack) && StateFrame->Locals) ...`,
  UnObj.cpp:1834-1837). Remove it and `UObject::SerializeStateLocals` (UnObj.cpp:1848).
- Reference: UnObj.cpp:1797.

**`UObject::SerializeScriptPropertiesBin(FArchive&, UClass* ExcludingBaseClass)`** (0x6de00) — Arkane addition, no
reference. Walks `Class->PropertyLink` calling `UStruct::SerializeBinProperty` until the property's owner class equals
`ExcludingBaseClass`. Used by DisSaveLoad; write when Engine/DishonoredGame needs it. P2.

**`UStruct::SerializeTaggedProperties`** (0x879b0) — identical for loading and saving (bCollideActors redirect
`<591`, InterpCurve `InterpMethod` fixup `<550` for names 1112-1116, `InitChild2StartBone`(902) →
`BranchStartBoneName`, Str→Name and Byte→Int conversions, enum gain/loss `>=633`, `Components`(694)/`Actor`(164)
editor skip, RawDistribution `Initialize` on save, size back-patching). One drop: the editor-only skip test has no
`GUglyHackFlags & HACK_ForceLoadEditorOnly` term. UnClass.cpp:599. P2.

**`UStruct::SerializeBin`** (0x4c30) / **`SerializeBinEx`** (0x23630) / **`SerializeBinProperty`** (0x4bc0) —
identical. UnClass.cpp:544 / 569 / 526. P2.

**`UScriptStruct::SerializeBin`** (0x4d70) — Arkane override, no reference. P2 (needed for DisSaveLoad, not for
package load): `UStruct::SerializeBin(Ar, Data, Max); if ((StructFlags & STRUCT_DevLoad) && Ar.ArIsDisSaveLoad &&
Ar.IsLoading()) DisStructDevLoad(this, Data);` (`DisStructDevLoad` lives in DishonoredGame, rva 0x793c70).

**`UField::Serialize`** (0x96240) — identical (`<756` legacy SuperField, `Next`). UnClass.cpp:80. P2.

**`UStruct::Serialize`** (0x962b0) — identical: `>=756` SuperStruct; ScriptText/CppText/Line/TextPos unless cooked
for console; Children; bytecode size + `>=639` storage size; script-patch reader path; seek-free `TempScript`
+ `FMemoryReader` swap of `Loader` and `UpdateScriptSHAKey`; saving `EX_EndOfScript` (0x53) append for make and
storage-size back-patch; `ScriptObjectReferences` collector unless `IsDisregardedForGC`; `Link(Ar, TRUE)`.
UnClass.cpp:1026. P2.

**`UStruct::SerializeExpr`** (0x2d120, 12.5 KB) — not diffed line by line; no version tests inside; opcode table
matches the reference per `symbols/opcodes.md`. Treat as reference (ScriptSerialization.h). P2.

**`UScriptStruct::Serialize`** (0x968d0) — identical (StructFlags; defaults via `SerializeBin` when
`WantBinaryPropertySerialization`, else `SerializeTaggedProperties(..., SuperStruct, SuperDefaults)`).
UnClass.cpp:1443. P2.

**`UState::Serialize`** (0x96a50) — identical (ProbeMask DWORD, LabelTableOffset WORD, StateFlags, FuncMap; script
patcher `LabelTableOffset` keep and `RF_PendingFieldPatches` rebuild). UnClass.cpp:1556. P2.

**`UClass::Serialize`** (0x96b70) — differs. **P1** (licensee branch; the reference misreads every class).
- After `UState::Serialize`: ClassFlags, ClassWithin, ClassConfigName, ComponentNameToDefaultObjectMap, Interfaces
  (identical).
- Editor block, skipped when cooked for console/PC-server or when cooking for them (same test as the reference):
  `>=603` DontSortCategories; HideCategories, AutoExpandCategories, AutoCollapseCategories; `>=749` bForceScriptOrder
  (else 0); **`if (Ar.LicenseeVer() >= 10) Ar << m_DropdownCategory;` (FName)**; ClassHeaderFilename.
  **No `ClassGroupNames` at 789** — delete the reference's `VER_ADDED_CLASS_GROUPS` read (UnClass.cpp:2278-2281)
  and the `DEDICATED_SERVER` mirror.
- `>=655`: dummy `FName` (no `WITH_LIBFFI`/`DLLBindName`).
- **`if (Ar.Ver() >= 796) Ar << m_OtherClassFlags; else if (Ar.IsLoading()) m_OtherClassFlags = 0;`** (DWORD) —
  new, sits before `StartSerializingDefaults`.
- CDO part identical (`ClassDefaultObject`; make → inline `InitClassDefaultObject`; `ClassUnique = 0`;
  `IsIgnoringArchetypeRef` → `ClassDefaultObject->Serialize`).
- Reference: UnClass.cpp:2221.

**`UClass::SerializeDefaultObject`** (0x88910) — identical (NetIndex first, defaults patch reader, tagged vs
`SerializeBinEx(…, SuperClass->PropertiesSize)` vs `SerializeBin`). UnClass.cpp:2359. P2.

**`UFunction::Serialize`** (0x96ef0) — identical (iNative WORD, OperPrecedence BYTE, FunctionFlags, RepOffset if
`FUNC_Net` 0x40, parm precomputation incl. `FUNC_HasDefaults` 0x800000 / `FirstStructWithDefaults`, FriendlyName unless
cooked for console). UnClass.cpp:2636. P2.

**`UConst::Serialize`** (0x97080), **`UEnum::Serialize`** (0x97120, Names + enum patch + make map),
**`UTextBuffer::Serialize`** (0x970b0), **`UMetaData::Serialize`** (0x970f0), **`UObjectRedirector::Serialize`**
(0x95b70) — identical. UnClass.cpp:2764, UnCoreNative.cpp:438 / 133 / 145, UnObjectRedirector.cpp:42. P2.

**`UPackage::Serialize`** (0x971e0) — identical except the reference's trailing `if (Ar.IsCountingMemory())
NetObjects.CountBytes(Ar)` block is absent. UnCoreNative.cpp:959. P2.

**`UProperty::Serialize`** (0x98180) — identical (ArrayDim, PropertyFlags QWORD, Category + ArraySizeEnum unless
cooked for console, RepOffset if `CPF_Net` 0x20, load → `Offset = 0; ConstructorLinkNext = NULL`). UnProp.cpp:145. P2.

**`UByteProperty::Serialize`** (0x98240, Enum + Preload), **`UBoolProperty`** (0x98280, BitMask only when
neither loading nor saving), **`UClassProperty`** (0x982b0, PropertyClass then MetaClass), **`UInterfaceProperty`**
(0x98320, InterfaceClass), **`UMapProperty`** (0x982f0, Key, Value), **`UStrProperty`** (0x982e0) — identical.
`UObjectProperty/UStructProperty/UArrayProperty::Serialize` (one object) and `UDelegateProperty::Serialize`
(Function, SourceDelegate) are COMDAT-folded into `UClassProperty`/`UMapProperty` and are not separate symbols;
`UIntProperty/UFloatProperty/UNameProperty/UComponentProperty` have no override. UnProp.cpp:662, 1370, 2279, 2997,
3916, 3148, 1926, 4286, 3369, 1297. P2.

### 3.5 Names, compression, package loading entry points

**`FName::Init(const ANSICHAR*, INT, EFindName)`** (0x49aa0) / **`AllocateNameEntry`** (0x20060) — identical
(65536-bucket `appStrihash`, `FNAME_Replace` in-place copy, 64 KB pool pages, `Index = 2*i | bUnicode`).
UnName.cpp:646 / 1119. P2. `FName::Serialize` does not exist in either tree; `FArchive::operator<<(FName&)` is the
virtual no-op (rva 0x1580) overridden by the linkers.

**`FArchive::SerializeCompressed`** (0x22f60) — loading identical, saving differs. **P0** because of LZO.
- Loading: `FCompressedChunkInfo` tag {CompressedSize, UncompressedSize} (byte-swap detection on
  `PACKAGE_FILE_TAG_SWAPPED`), summary chunk, chunk size `0x20000` when the tag was stored, per-chunk table,
  one `appMalloc(MaxCompressedSize)` buffer, `appUncompressMemory` inlined: `COMPRESS_ZLIB` → zlib `uncompress`,
  `COMPRESS_LZO` → `lzopro_lzo1x_decompress_safe` after `__lzopro_lzo_init_v2`. No PS3 padding.
- Saving: chunk size = `Ar.ArIsSaveGame ? GSaveDataSavingCompressionChunkSize (65536) : GSavingCompressionChunkSize
  (131072)`; single-threaded `appCompressMemory` loop (no `FAsyncCompressionChunk` worker pool, no `MTCHILD`).
- Reference: UnArchive.cpp:146 (+ `operator<<(FCompressedChunkInfo&)` :74, identical).

**`appUncompressMemory`** (0x14ae0) / **`appUncompressMemoryLZO`** (0x14a80) — semantically identical for
ZLIB/LZO (no LZX, no stats); the LZO implementation is LZO Professional (`lzopro_*`), format-compatible with LZO1X.
Port with the reference `WITH_LZO` path (`lzo1x_decompress_safe`, minilzo or LZO 2.x). UnMisc.cpp:6572 / ~6540.
**P0: all shipped packages are LZO-compressed (`CompressionFlags = 2`, `GBaseCompressionMethod = 2`).**

**`UObject::LoadPackage`** (0xa7300) — differs. P2 (compiles against the Dishonored layout only after edits).
- No `Result->FileName = FName(*FileToLoad)` (`UPackage::FileName` does not exist), no `FFilename FileToLoad`
  fallback to `InOuter->GetName()`, `EndLoad()` without context, `appOnFailSHAVerification` reduced to nothing, no
  `LOAD_NoSeekFreeLinkerDetatch` test before `ResetLoaders`. Otherwise identical (`LoadFlags | LOAD_Throw`,
  SHA script key, `LoadAllObjects` unless `LOAD_Verify`, `CancelRemainingAllocations(TRUE)`,
  `LookupAllOutstandingCrossLevelExports`, `SetLoadTime`, `HintDoneWithFile`, script patcher free).
- Reference: UnObj.cpp:7099.

**`UObject::GetPackageLinker`** (0x9bff0) — differs. P2. No `PackageNameToFileMapping` lookup (uses
`InOuter->GetName()` directly); everything else identical (`FindExistingLinkerForPackage` loop, `LOAD_AllowDll` +
`IsBound`, path stripping, `CreatePackage(..., LOAD_RemappedPackage)`, `ResetLoaders` on package mismatch,
`Sandbox->SupportsPackage`, `CreateLinker`, `CompatibleGuid` check). UnObj.cpp:6480.

**`UObject::BeginLoad`** (0xa6990) — identical (`FlushAsyncLoading()` with no exclude type). UnObj.cpp:7274. P2.

**`UObject::EndLoad`** (0x728f0) — body identical (sort by linker/serial offset, `Preload` for `RF_NeedLoad`,
editor `LoadedLinkers` set, `GIsRoutingPostLoad` + `ConditionalPostLoad`, `GIsWatchingEndLoad` callback,
`MarkAsFullyLoaded` sweep, `DissociateImportsAndForcedExports`); **signature is `EndLoad()`** (no `LoadContext`,
no `WITH_EDITOR` progress). UnObj.cpp:7326. P2.

**`UObject::ResetLoaders`** (0x40300) — identical (`Detach(TRUE)`). UnObj.cpp:7512. P2.

### 3.6 UnAsyncLoading.cpp

**`FAsyncPackage::Tick`** (0xa2700) — identical behaviour; `BeginAsyncLoad/EndAsyncLoad`, `FinishLinker`,
`FinishTextureAllocations`, `FinishExportGuids` and `GiveUpTimeSlice` are inlined (`GIsRequestingExit` break,
`GObjBeginLoadCount++`/`GIsAsyncLoading`, linker `Tick`, `PendingAllocationCount` wait/cancel, cross-level lookup,
loop `while (!bTimeLimitExceeded && !bExecuteNextStep)`). UnAsyncLoading.cpp:137. P2.

**`FAsyncPackage::CreateLinker`** (0x9b450) — differs. P2 (streaming, not milestone 3). No
`ULinkerLoad::FindExistingLinkerForPackage(Package)` and no `GetPackageNameToFileMapping()`; always
`GPackageFileCache->FindPackageFile(*PackageName, Guid.IsValid() ? &Guid : NULL, ...)` then `CreateLinkerAsync(Package,
*File, (GIsGame && !GIsEditor) ? LOAD_SeekFree|LOAD_NoVerify : LOAD_None)`. UnAsyncLoading.cpp:260.

**`FAsyncPackage::CreateImports`** (0x9d3e0), **`CreateExports`** (0x9f9b0, `EF_ScriptPatcherExport` skip,
`Precache(SerialOffset, SerialSize)`, `LoadPercentage`), **`PreLoadObjects`** (0x2cb30), **`PostLoadObjects`**
(0x74d60), **`FinishObjects`** (0x67ae0, clears `RF_AsyncLoading` = bit 42, `HintDoneWithFile`, script patcher
free, `CancelRemainingAllocations(TRUE)`), **`FAsyncPackage(const FString&, const FGuid*)`** (0x995d0, no
`PackageType`) — identical. UnAsyncLoading.cpp:332-568, Inc/UnAsyncLoading.h:77. P2.

**`UObject::ProcessAsyncLoading(UBOOL, FLOAT)`** (0xa28e0) — differs. P2. Older Epic form: `while
(GObjAsyncPackages.Num()) { if (!GObjAsyncPackages(0).Tick(bUseTimeLimit, TimeLimit)) break; if
(GUseSeekFreeLoading) ResetLoader(); GObjAsyncPackages.Remove(0); }` — only the head package is ticked and there is
no `ExcludeType`. **`FlushAsyncLoading()`** (0xa29d0) likewise has no parameter (`SetMinPriority(AIOP_Normal)`,
`ProcessAsyncLoading(FALSE, 0)`, `SetMinPriority(AIOP_MIN)`). UnAsyncLoading.cpp:761 / 668.

**`FArchiveAsync`**: ctor (0x38500), `SetCompressionMap` (0x2cd30), `FindCompressedChunkIndex` (0x2cdc0),
`Precache` (0x2cfb0), `Serialize` (0x1b9e0), `BufferSwitcheroo` (0x46c0), `FlushCache` (0x1b940), `Seek`
(0x4730), `Close` (0x46a0) — identical (`Precache` non-compressed path `Max(RequestSize, 0x20000)`, `LoadData(...,
AIOP_Normal)`). **`PrecacheCompressedChunk`** (0x2ce70) — differs: `IO->LoadCompressedData(FileName,
CompressedOffset, CompressedSize, UncompressedSize, Buffer, CompressionFlags, &Status, AIOP_Normal, AIORT_Other)`
(extra `EAsyncIORequestType`). UnAsyncLoading.cpp:1807-2240. **P0**: `SerializePackageFileSummary` replaces the
file/buffer reader with `FArchiveAsync` for every `PKG_StoreCompressed` package, so the async-IO path
(`FAsyncIOSystemBase::LoadCompressedData` → `appUncompressMemory` on the IO thread) is on the synchronous
`LoadPackage` route too. `FAsyncIOSystemBase`/`FAsyncIOSystemWindows` were not diffed here (follow-up).

## 4. Summary

| function | rva | status | priority |
|---|---|---|---|
| `operator<<(FPackageFileSummary&)` | 0x5a1a0 | identical (770 vs 767) | P2 |
| `FGenerationInfo::Serialize` | inlined | identical | P2 |
| `operator<<(FObjectExport&)` / `(FObjectImport&)` / `(FCompressedChunk&)` | 0x6a420 / 0xd010 / 0x42a90 | identical | P2 |
| `operator<<(FTextureAllocations&)` / `(FTextureType&)` (Engine) | 0x192d20 / 0x1844e0 | identical | P2 |
| `operator<<(FNameEntry&)` | 0x3f9d0 | identical | P2 |
| `FPropertyTag` operator / ctor / `SerializeTaggedProperty` | 0x4a80 / 0x87870 / 0x80f70 | identical | P2 |
| `ULinkerLoad::Tick` | 0xa25d0 | differs: no `FixupExportMap` | P0 |
| `ULinkerLoad::ULinkerLoad` | 0x7b030 | identical | P2 |
| `ULinkerLoad::CreateLoader` | 0x898a0 | differs: `ArkBsPatch` precache, `FPackagePrecacheInfo` +3 | P0 (layout) |
| `SerializePackageFileSummary` / `SerializeNameMap` / `SerializeImportMap` / `SerializeExportMap` | 0x8a330 / 0x5a4a0 / 0x30860 / 0x6a6a0 | identical | P2 |
| `FixupImportMap` / `IntegrateScriptPatches` / `GetScriptPatcher` | 0x30a00 / 0x8b250 / 0x48db0 | identical / stub patcher | P2 |
| `RemapClasses` | 0x7b200 | differs: no MobileGame remap (dead) | P2 |
| `SerializeDependsMap` / `SerializeGuidMaps` / `CreateExportHash` / `FindExistingExports` / `FinalizeCreation` | 0x48e00 / 0x5a810 / 0x65d60 / 0x7b980 / 0xa22e0 | identical | P2 |
| `StartTextureAllocation` (Engine) | 0x193010 | differs: no `CreateResourceMem` | P2 |
| `ULinkerLoad::Serialize` / `operator<<(FName&)` / `operator<<(UObject*&)` | 0xd150 / 0x30d30 / 0x9bca0 | identical | P2 |
| `Preload` / `CreateExport` / `VerifyImport` / `VerifyImportInner` / `LoadAllObjects` / `IndexToObject` | 0x94670 / 0x9e460 / 0xa0710 / 0x9d8c0 / 0xa0bc0 / 0x9bb50 | identical | P2 |
| `CreateImport` | 0x9b690 | differs: dead GearGame hack | P2 |
| `UObject::Serialize` | 0x949c0 | identical | P2 |
| `UObject::SerializeNetIndex` / `SetNetIndex` | 0x20180 / 0x14d30 | differs: no `NetIndex`, flag bit 63 | P0 |
| `UObject::SerializeScriptProperties` | 0x8a8f0 | differs: no `SerializeStateLocals` | P1 |
| `UObject::SerializeScriptPropertiesBin` | 0x6de00 | Arkane addition | P2 |
| `UStruct::SerializeTaggedProperties` / `SerializeBin` / `SerializeBinEx` / `SerializeBinProperty` | 0x879b0 / 0x4c30 / 0x23630 / 0x4bc0 | identical | P2 |
| `UScriptStruct::SerializeBin` | 0x4d70 | Arkane override (`STRUCT_DevLoad`, `ArIsDisSaveLoad`) | P2 |
| `UField` / `UStruct` / `UScriptStruct` / `UState` / `UFunction` / `UConst` / `UEnum` `::Serialize` | 0x96240 / 0x962b0 / 0x968d0 / 0x96a50 / 0x96ef0 / 0x97080 / 0x97120 | identical | P2 |
| `UStruct::SerializeExpr` | 0x2d120 | not diffed; opcodes match | P2 |
| `UClass::Serialize` | 0x96b70 | differs: `m_DropdownCategory`@lic10, `m_OtherClassFlags`@796, no `ClassGroupNames` | P1 |
| `UClass::SerializeDefaultObject` | 0x88910 | identical | P2 |
| `UPackage` / `UTextBuffer` / `UMetaData` / `UObjectRedirector` `::Serialize` | 0x971e0 / 0x970b0 / 0x970f0 / 0x95b70 | identical (UPackage: no CountBytes block) | P2 |
| `UProperty` and all `UXxxProperty::Serialize` | 0x98180, 0x98240, 0x98280, 0x982b0, 0x982e0, 0x982f0, 0x98320 | identical | P2 |
| `FName::Init(ANSI)` / `AllocateNameEntry` | 0x49aa0 / 0x20060 | identical | P2 |
| `FArchive::SerializeCompressed` | 0x22f60 | load identical (LZO), save differs (`ArIsSaveGame` chunk size, single-threaded) | P0 |
| `appUncompressMemory` / `appUncompressMemoryLZO` | 0x14ae0 / 0x14a80 | identical semantics; needs LZO1X | P0 |
| `UObject::LoadPackage` / `GetPackageLinker` / `EndLoad` | 0xa7300 / 0x9bff0 / 0x728f0 | differs: Epic-drift removals, signatures | P2 |
| `UObject::BeginLoad` / `ResetLoaders` | 0xa6990 / 0x40300 | identical | P2 |
| `FAsyncPackage::Tick/CreateImports/CreateExports/PreLoadObjects/PostLoadObjects/FinishObjects/ctor` | 0xa2700 … 0x995d0 | identical | P2 |
| `FAsyncPackage::CreateLinker` | 0x9b450 | differs: no existing-linker / name mapping lookup | P2 |
| `UObject::ProcessAsyncLoading` / `FlushAsyncLoading` | 0xa28e0 / 0xa29d0 | differs: no `ExcludeType`, head-only | P2 |
| `FArchiveAsync::*` | 0x38500 … | identical except `PrecacheCompressedChunk` (extra `AIORT_Other`) | P0 |

### Ordered port list before milestone 3 (package load)

1. LZO1X decompression (`WITH_LZO` / minilzo) + `appUncompressMemory` + `FArchive::SerializeCompressed` load path.
2. `FIOSystem::LoadCompressedData` signature (+`EAsyncIORequestType`), `FArchiveAsync::PrecacheCompressedChunk`;
   then diff `FAsyncIOSystemBase`/`FAsyncIOSystemWindows` (not in this set).
3. `UObject` layout (drop `NetIndex`), `UObject::SerializeNetIndex`, `UObject::SetNetIndex`.
4. `UClass::Serialize` (+`m_OtherClassFlags`, `m_DropdownCategory`, remove `ClassGroupNames`) and the `UClass` members.
5. `UObject::SerializeScriptProperties` (remove `SerializeStateLocals`).
6. `ULinkerLoad::Tick` (remove `FixupExportMap`), `ULinkerLoad::FPackagePrecacheInfo` + `CreateLoader`
   (`ArkBsPatch` stub), `GetScriptPatcher` stub.
7. `UnObjVer.h` enum as described in section 1; `FArchive::ArIsDisSaveLoad`; `STRUCT_DevLoad`.
8. `UObject::LoadPackage` / `GetPackageLinker` / `EndLoad()` / `BeginLoad` / `ProcessAsyncLoading` /
   `FlushAsyncLoading` signature and drift edits.
9. Later (streaming / DisSaveLoad): `FAsyncPackage::CreateLinker`, `UScriptStruct::SerializeBin`,
   `UObject::SerializeScriptPropertiesBin`, saving path of `SerializeCompressed`.

## 5. Licensee-version branches outside Core (`symbols/licensee_branches.md`) — port in the Engine/DishonoredGame passes

| module | function | rva | comparison |
|---|---|---|---|
| engine | `FMaterial::Serialize` | 0x14eb00 | `LicenseeVer() >= 0` |
| engine | `AActor::Serialize` | 0x171920 | `< 26` |
| engine | `ULevel::Serialize` | 0x2737b0 | `< 27` |
| engine | `operator<<(FArchive&, FNavMeshPolyBase&)` | 0x2928c0 | `< 27` (plus engine `>= 586`, `< 588`) |
| engine | `USeqEvent_Touch::Serialize` | 0x2ea610 | `< 30` |
| engine | `USkeletalMeshComponent::Serialize` | 0x336f30 | `< 28` |
| engine | `UWorld::Serialize` | 0x3bba70 | `< 27` |
| engine | `UDecalMaterial::Serialize` | 0xd7c20 | `< 0` |
| dishonoredgame | `UDisTweaks_StaticBreakable::Serialize` | 0x646c00 | `< 24` |
| dishonoredgame | `UDisTweaks_SkeletalBreakable::Serialize` | 0x669cd0 | `< 24` |
| dishonoredgame | `UDisTweaks_UsableObject::Serialize` | 0x671590 | `< 24`, `< 29` |
| dishonoredgame | `UDisTweaks_Attributes::Serialize` | 0x8f0d30 | `< 25` |
| core | `UClass::Serialize` | 0x96b70 | `>= 10` (section 3.4) |

## 6. Not covered / follow-ups

- `FAsyncIOSystemBase` (queue, `FulfillCompressedRead`), `FBufferReader(WithSHA)`, `FSHA1::GetFileSHAHash`,
  `ULinkerLoad::AsyncPreloadPackage` (uses the patched `FPackagePrecacheInfo`), `RemapLinkerPackageNamesForMultilanguageCooks`,
  `UObject::StaticLoadObject`, `DissociateImportsAndForcedExports`, `UPackage::LookupAllOutstandingCrossLevelExports`
  were decompiled (files in `core_serialize/`) but not diffed.
- `FUntypedBulkData::Serialize/SerializeBulkData` (Engine content, not milestone 3) decompiled, not diffed.
- Catch handlers (`LOAD_FindIfFail`, `LoadPackage`'s `SafeLoadError`) are invisible in pseudocode; assume the
  reference `try/catch` structure.
