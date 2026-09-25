# Agent A report — P2.5 Core layout probe and convergence (2026-09-25)

Build dir: `build\agentA` (Ninja, Debug, `cmake\toolchain-x86.cmake`, configured with
`-DCMAKE_CXX_FLAGS=/Zp4 -DDISHONORED_LAYOUT_CHECKS=ON`, see "Must merge" below).
Logs: `build\agentA\probe_build*.log`, `core_build1.log`, `compile_new3.log`.

## Result

| Item | State |
|---|---|
| `LayoutProbe` | builds and runs post-build, writes `build\agentA\layout_probe.txt` (304 Core types, 45 skipped in `probe_skip_Core.txt`) |
| `resources/docs/types/reference_layout_delta.md` | produced; 269 / 304 types match exactly (size and every member offset) |
| Contract types | **38 probed, 0 mismatching** |
| `Core/Inc/DishonoredLayouts.h` | regenerated: 277 `static_assert`s (all contract types + every other type whose size already matches), 13 `// pending:` lines; included from the end of `Core.h` under `#if DISHONORED_LAYOUT_CHECKS`; compiles (the probe unit includes `Core.h` with it on) |
| `Core` target | 4 Src units need the follow-up patch below (`UnObj.cpp`, `UnCoreNet.cpp`, `UnClass.cpp`, `UnLinker.cpp`); everything else compiles with the new headers and the asserts on |

## The one thing the coordinator must do first: `/Zp4`

Dishonored (like every UE3 PC build) is compiled with **4-byte struct member packing**: the
reference `Development/Src/UnrealBuildTool/ToolChain/VCToolChain.cs:30` adds `/Zp4`, and the PDB
shows it everywhere (`UProperty::PropertyFlags` (QWORD) @68, `ULinkerLoad::TickStartTime` (DOUBLE)
@1628, `UField` = 60 bytes on top of an 8-aligned `UObject`, `FObjectExport` = 92, ...). Our CMake
does not pass it, so with the stock presets `UField` is 64, `FObjectExport` 96, etc., and no header
edit can fix that. I could not edit `cmake/`; `build\agentA` was configured with
`-DCMAKE_CXX_FLAGS=/Zp4` instead. **Add `/Zp4` to the shared MSVC options in
`cmake/DishonoredModule.cmake` (or `DishonoredDefines.cmake`) for every module and the tests.**
`DishonoredLayouts.h` starts with a packing probe
(`struct FDishonoredPackingProbe { BYTE A; QWORD B; }` must be 12 bytes) so a build without `/Zp4`
fails with one clear message instead of hundreds of size asserts.

## Header changes (`source/Development/Src/Core/Inc`), all with `// DISHONORED(layout)` comments

| File | Change | PDB evidence |
|---|---|---|
| `UnObjBas.h` | `UObject`: `Index` moved in front of `ObjectFlags` (order is now Index, ObjectFlags, HashNext, HashOuterNext, StateFrame, _Linker, _LinkerIndex, Outer, Name, Class, ObjectArchetype); reference `NetIndex` removed; `GetNetIndex()` now returns `HasAnyFlags(RF_DisNetIndexed) ? _LinkerIndex : INDEX_NONE`; new `RF_DisNetIndexed` = bit 63 | `UObject` 56 bytes: Index @4, ObjectFlags @8, HashNext @16 ... ObjectArchetype @52; no NetIndex. `UObject::SetNetIndex` (rva 0x14d30) sets/clears bit 63 of ObjectFlags, `SerializeNetIndex` (rva 0x20180) writes 0/−1 from that bit (decompiles in `build\agentA\ida\netindex`) |
| `UnClass.h` | `UClass`: added `DWORD m_OtherClassFlags` after `ClassFlags`, `FName m_DropdownCategory` after `ClassDefaultObject`, `TMap<const UClass*,INT>* m_pInterfaceOffsets` after `Interfaces`; reference `ClassGroupNames` removed | `UClass` 456 bytes: m_OtherClassFlags @204, m_DropdownCategory @332, m_pInterfaceOffsets @424; DependentOn @300 is directly followed by bForceScriptOrder @312. `UClass::Serialize` (rva 0x96b70) serializes m_DropdownCategory when `ArLicenseeVer >= 10` and m_OtherClassFlags when `ArVer >= 796`, never ClassGroupNames |
| `UnArc.h` | `FArchive`: added `UBOOL ArIsDisSaveLoad` after `ArIsFilterEditorOnly`, reset to FALSE in both `Reset()` bodies | `FArchive` 136 bytes, ArIsDisSaveLoad @132 |
| `UnStack.h`, `UnScript.h` | `FStateFrame`: reference `LocalVarsOwner` removed (ctor/copy-ctor/operator=/dtor); `ClearLocalVars()` iterates `Object->GetClass()` instead | `FStateFrame` 64 bytes, ends with StateStack @52; Dishonored's `FStateFrame::FStateFrame(const FStateFrame&)` (rva 0x3045f0) copies nothing after StateStack |
| `UnLinker.h` | `ULinkerLoad`: reference `bFixupExportMapDone` removed | `ULinkerLoad` 1832 bytes, TickStartTime @1628 directly followed by PatchDataAr @1636; `ULinkerLoad::FixupExportMap` does not exist in the PDB |
| `UnCorObj.h` | `UPackage`: reference `FName FileName` removed | `UPackage` 228 bytes, PackageFlags @216 directly followed by ThumbnailMap @220 |
| `UnCoreNet.h` | `FPackageInfo`: reference `FName FileName` removed (same feature, same Src line) | `FPackageInfo` 68 bytes, ForcedExportBasePackageName @48 directly followed by Extension @56 |
| `Core.h` | `#if DISHONORED_LAYOUT_CHECKS #include "DishonoredLayouts.h" #endif` before the closing `#endif` | |
| `DishonoredLayouts.h` | regenerated (see tools) | |

Not mine: `Core/Inc/pool.h` shows as modified in the working tree (+419 lines); I did not touch it.

## Contract-type results (`reference_layout_delta.md`, `/Zp4` build)

All 38 contract types match the PDB in size and in every probed member offset:

| Type | Size | Type | Size | Type | Size |
|---|---:|---|---:|---|---:|
| FName | 8 | UField | 60 | UObjectProperty | 112 |
| FNameEntry | 2064 | UStruct | 128 | UClassProperty | 116 |
| FString | 12 | UState | 200 | UNameProperty | 108 |
| FArchive | 136 | UClass | 456 | UStrProperty | 108 |
| FFrame | 40 | UFunction | 160 | UArrayProperty | 112 |
| FStateFrame | 64 | UEnum | 72 | UMapProperty | 116 |
| FPackageFileSummary | 164 | UScriptStruct | 156 | UStructProperty | 112 |
| FObjectExport | 92 | UConst | 72 | UDelegateProperty | 116 |
| FObjectImport | 40 | UProperty | 108 | UInterfaceProperty | 112 |
| FGenerationInfo | 12 | UByteProperty | 112 | UComponentProperty | 112 |
| FCompressedChunk | 16 | UIntProperty | 108 | ULinker | 356 |
| UObject | 56 | UBoolProperty | 112 | ULinkerLoad | 1832 |
| UPackage | 228 | UFloatProperty | 108 | | |

## Remaining non-contract differences: 35 types

- 13 size mismatches (listed as `// pending:` in `DishonoredLayouts.h`): FArchiveLoadCompressedProxy /
  FArchiveSaveCompressedProxy (172 vs 168), FAsyncIOHandle (8 vs 12), FAsyncIOSystemBase / Windows
  (112 vs 116), FAsyncPackage (108 vs 116), FCallbackEventObserver (832 vs 6196), FOutputDeviceFile
  (2080 vs 2084), FPackageInfo (68 vs 72, one more reference-only member after Extension),
  FQueuedThreadWin (28 vs 32), FRingBuffer (32 vs 28), FScopedGameplayStats (12 vs 16), USystem
  (260 vs 236: Arkane `ScreenShotPath`, `MobileScriptPaths`, `Paths` @128).
- 22 types whose size matches but the probe reports members as `MISSING`: members that are private
  by default (`class X {` with no access specifier, e.g. FCriticalSection, FScopeLock, FMemMark,
  FBitReference, FDuplicateData*, FArchiveProxy) which the `#define private public` trick cannot
  reach (known limitation, porting_notes "Layout probe limitations"), plus a few genuine Arkane
  members (FCallbackEventParameters::EventString/EventVector, FArchiveObjectGraph::CurrentReferencer…).
  These are Phase 3 material; none is in the P2.5 contract.

## Src follow-up for the coordinator (I may not edit `Src/`)

`resources/docs/agents/agentA_core_src.patch` (11 hunks, `git apply --check` passes) removes the
last uses of the deleted reference members. Every hunk carries a `// DISHONORED(layout)` comment.
The four patched units were compiled from copies (`build\agentA\src\new`) with Core's exact
command lines: **0 errors** (`compile_new3.log`).

| File | Sites | Fix |
|---|---|---|
| `UnObj.cpp` | 1057–1077 `SetNetIndex`, 1570 `SerializeNetIndex`, 4079, 8042, 8210 (`NetIndex`), 7125–7129 (`UPackage::FileName`) | `SetNetIndex` = set/clear `RF_DisNetIndexed` (rva 0x14d30: no `UPackage::AddNetObject/RemoveNetObject` in the PDB); `SerializeNetIndex` writes `0` when flagged, `INDEX_NONE` otherwise (rva 0x20180); other sites use `GetNetIndex()` / `ClearFlags(RF_DisNetIndexed)`; the `Result->FileName = ...` block is dropped |
| `UnCoreNet.cpp` | 25 (`FileName` initializer), 447/453/455 (`Object->NetIndex`) | initializer removed; `GetNetIndex()` |
| `UnClass.cpp` | 2283–2286 | `Ar << ClassGroupNames` block removed |
| `UnLinker.cpp` | 5415, 5517 | `bFixupExportMapDone` dropped from the condition / assignment |

Until the patch is applied, `--target Core` fails only in those four units; `LayoutProbe` no
longer depends on `Core.lib` (see tools), so the probe keeps working regardless.

## Tool changes

`resources/tools/symbols/gen_layout_probe.py`
- The probe no longer links (or builds) `Core.lib`: `Core.lib` drags Engine-only symbols into the
  link (`GEngine`, `FlushRenderingCommands`, `UWindowsClient::StaticWndProc`, ... 56 unresolved).
  The probe objects reference only `appMalloc`/`appFree` (UnFile.h's inline `operator new/delete`),
  so `probe_main.cpp` defines malloc-backed stubs and the generated `CMakeLists.txt` takes the
  module's `INTERFACE_INCLUDE_DIRECTORIES` / `INTERFACE_COMPILE_DEFINITIONS` via `TARGET_PROPERTY`
  (`$<COMPILE_ONLY:Core>` still adds a build dependency on Core).
- Post-build runs `$<TARGET_FILE:LayoutProbe>` (the exe lives in `Binaries\Win32`, not the cwd).
- `declared_types()` drops preprocessor lines before matching, otherwise
  `class UObject #if VTABLE_AT_END_OF_CLASS : public UObjectBase #endif {` was never probed.
- Anonymous unions (`___uN` in DIA output) are skipped; they are not addressable members.
- New `show <Type>...` subcommand prints a type's PDB layout (offset, size, type, name).

`resources/tools/symbols/gen_layout_asserts.py`
- Asserts every contract type unconditionally, other types only when `--probe <layout_probe.txt>`
  shows their size already matches; the rest become `// pending: sizeof(T) == N (ours M)` lines so
  Core keeps compiling while they converge. Honors `probe_skip_<Module>.txt`. Emits the `/Zp4`
  packing probe first.
- Usage: `python resources/tools/symbols/gen_layout_asserts.py --probe build/agentA/layout_probe.txt Core`.

`resources/docs/types/probe_skip_Core.txt`: +6 script-patcher types not reachable from `Core.h`
(FEnumPatchData, FLinkerPatchData, FPatchData, FPatchReader, FScriptPatchData, FScriptPatcher).

## Must merge into `porting_notes.md`

1. Build environment decision: `/Zp4` (struct member alignment 4) for all modules — reference
   `VCToolChain.cs:30`; without it no U*/F* layout with a QWORD/DOUBLE member can match the PDB.
2. Layout probe section: the probe does not link module libraries (stubs for `appMalloc`/`appFree`,
   `TARGET_PROPERTY` include dirs); `DishonoredLayouts.h` asserts contract + already-matching types
   and lists the rest as `pending`; regenerate it with `--probe` after every probe run.
3. Reference members proven absent by the PDB and removed: `UObject::NetIndex`,
   `UClass::ClassGroupNames`, `FStateFrame::LocalVarsOwner`, `ULinkerLoad::bFixupExportMapDone`,
   `UPackage::FileName`, `FPackageInfo::FileName`. Arkane members added: `FArchive::ArIsDisSaveLoad`,
   `UClass::m_OtherClassFlags`, `UClass::m_DropdownCategory`, `UClass::m_pInterfaceOffsets`.
4. `RF_DisNetIndexed` (bit 63, the reference's `RF_CookedStartupObject` slot) replaces `NetIndex`;
   `GetNetIndex()` returning `_LinkerIndex` when flagged is inferred from `SerializeNetIndex`
   (`SetNetIndex(_LinkerIndex)` on load of non-cooked packages) — flag it as inferred, not proven.
5. For Phase 3 serialization: `UClass::Serialize` (rva 0x96b70) reads `m_DropdownCategory` at
   `ArLicenseeVer >= 10`, `m_OtherClassFlags` at `ArVer >= 796`, and a dummy FName at `ArVer >= 655`
   in place of the reference's `DLLBindName` (WITH_LIBFFI).
6. `ULinkerLoad::FixupExportMap` (ActiveClassRedirects) is not in the shipping PDB.
7. Bash-tool pitfall confirmed again: Python heredocs lose `\\` and `\t`/`\n` escapes; write patch
   scripts with the Write/Edit tools. Headers are also transiently locked by concurrent compiles
   (PermissionError on write) — retry.
