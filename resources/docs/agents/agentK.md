# Agent K — LZO in the engine (PHASE3 package K)

Date: 2026-09-25. Build dir `build\agentK` (+ isolated worktree, see "Verification"). No IDA.

## Result

`WITH_LZO=1`. `appCompressMemoryLZO` / `appUncompressMemoryLZO` are implemented on **lzokay** (MIT,
LZO1X-1, stream-compatible with the LZO Professional `lzopro_lzo1x_*` the retail exe links).
`GBaseCompressionMethod` now defaults to `COMPRESS_LZO` (= 2, the retail value from
`serialization_delta_core.md`). CoreSmoke decompresses the real `Core.upk` chunk of the 2013 retail
package through `appUncompressMemory` and `FArchive::SerializeCompressed` and walks its name table:
**99/99 green in Debug and RelWithDebInfo** (67 existing + 32 new checks).

## Files

| File | Change |
|---|---|
| `cmake/Dependencies.cmake` | `dishonored_fetch(lzokay https://github.com/jackoalan/lzokay.git db2df1fcbebc2ed06c10f727f72567d40f06a2be)` (master as of 2026-09; the project has no tags). `Dishonored::lzokay` INTERFACE target = `lzokay` static lib + its source dir on the include path. lzokay's own `lzokaytest` exe and `lzokay-c` wrapper are `EXCLUDE_FROM_ALL`. The commented `nemequ/lzo 2.10` example is gone. |
| `cmake/DishonoredDefines.cmake` | `WITH_LZO=1`, comment updated (lzokay, and that `WITH_LZO` selects `COMPRESS_DefaultPC = COMPRESS_LZO`). |
| `CMakeLists.txt` | Core links `Dishonored::lzokay` (one line). |
| `Core/Src/UnMisc.cpp` | The `#if WITH_LZO` block (reference lines 6273–6486: lzopro includes, `LZOMalloc/LZOFree/GLZOCallbacks`, `LZO_*` macros, `LZOCriticalSection`, `InitializeLZO`, the two functions) is replaced by a lzokay implementation, `// DISHONORED(port):` comments. |
| `source/Tests/CoreSmoke/CoreSmoke.cpp` | New `TestCompressedChunk()` (32 checks), called after `TestPackageSummary()`. |
| `source/Tests/CoreSmoke/stubs.cpp` | Two harness fixes needed by the new test (below). |

External checkout: `external/lzokay-src` (+ `-build`, `-subbuild`), like the other FetchContent deps.

## UnMisc.cpp port details

Signatures, return conventions and `check()` semantics are the reference's:

- `appCompressMemoryLZO(Flags, CompressedBuffer, CompressedSize&, UncompressedBuffer, UncompressedSize)`:
  `check(UncompressedSize <= MaxUncompressedSize)`; compresses into a scratch buffer of
  `lzokay::compress_worst_size(UncompressedSize)` (= n + n/16 + 67; the reference used
  `UncompressedSize + LZO_WORK_MEM_SIZE`), `check(Result == Success)`, copies into `CompressedBuffer`
  only if it fits, otherwise returns FALSE; `CompressedSize` is always set to the real compressed size
  so the caller can retry with a bigger buffer (reference behaviour, verified by a test).
  `COMPRESS_BiasSpeed` / `COMPRESS_BiasMemory` selected lzopro's `1_08` / `99` / `1_14` compressors;
  lzokay has one LZO1X-1 compressor, so the flags no longer change the output (compression is
  cook/save-time only; retail packages are never re-written by the game).
  The reference's 64 KB pointer alignment, zeroed over-read padding and the shared work memory behind
  `LZOCriticalSection` are gone: lzokay bounds-checks input and output and the match dictionary
  (`lzokay::Dict<>`, ~350 KB, heap) is per call, so the function is thread-safe without the lock.
- `appUncompressMemoryLZO(UncompressedBuffer, UncompressedSize, CompressedBuffer, CompressedSize)`:
  `lzokay::decompress` with `UncompressedSize` as the destination capacity, the same bounds check
  `lzopro_lzo1x_decompress_safe` does; any result other than `Success` (input/output overrun,
  lookbehind overrun, unconsumed input) returns FALSE. No size-equality check, like the reference
  (the retail `appUncompressMemoryLZO` 0x14a80 has "identical semantics" per
  `serialization_delta_core.md`).
- `InitializeLZO()` / `lzo_init()` has no lzokay equivalent and is dropped. The `_MSC_VER == 1400`
  optimisation pragmas are kept.
- No other `lzo*` call exists in Core (`UnBulkData.cpp` / `UnArchive.cpp` only use the
  `COMPRESS_LZO` flag through `appCompressMemory` / `appUncompressMemory`).

`GBaseCompressionMethod` stays `= COMPRESS_Default` (UnMisc.cpp 6124): `UnFile.h` defines
`COMPRESS_DefaultPC` as `COMPRESS_LZO` under `WITH_LZO`, so the retail value follows from the switch
without editing the definition (the test asserts `GBaseCompressionMethod == COMPRESS_LZO`).

## CoreSmoke: `TestCompressedChunk`

Data (2013 retail `CookedPCConsole\Core.upk`, 45,534 bytes), everything read by the test itself:

- Summary (157 bytes stored) has one `FCompressedChunk`: `UncompressedOffset 141 = NameOffset`,
  `UncompressedSize 193,512`, `CompressedOffset 157`, `CompressedSize 45,377` (= to end of file).
  141 = the summary without its chunk table (157 − 16), i.e. the chunk starts exactly at the name table.
- Chunk header as `FArchive::SerializeCompressed` reads it: `{PACKAGE_FILE_TAG, 131072}`,
  `{45,345, 193,512}`, then 2 `FCompressedChunkInfo` blocks `{29,113 → 131,072}`, `{16,232 → 62,440}`;
  the sums and `16 + 2·8 + 45,345 = 45,377` are checked.
- Before decompressing, the test does what `appInit` does for the stats system:
  `GSynchronizeFactory = &FSynchronizeFactoryWin`, `GConfig = new FConfigCacheIni` with file
  operations disabled, `GStatManager.Init()` (appUncompressMemory books `STAT_UncompressorTime`
  through `GStatManager.Increment`, which asserts on the uninitialised sync object otherwise).
- Block 0 through `appUncompressMemory(COMPRESS_LZO)`; the truncated-input and too-small-destination
  variants must return FALSE.
- Whole chunk through `FMemoryReader::SerializeCompressed(Dest, 193512, COMPRESS_LZO)` (the
  `ULinkerLoad` / `UnLinker.cpp:1237` path); archive position ends at 157 + 45,377; block 0 equals
  the first 131,072 bytes of the result.
- Name table: 720 × (`FString`, `QWORD` flags) as `operator<<(FNameEntry&)` reads it; all lengths
  plausible, ends exactly at `ImportOffset` (17,942), contains `None`, `Core`, `Object`, is sorted
  (`appStricmp`, the cooker sorts names). **The first name is `"!"`, the last `"~="`** (the
  UnrealScript operator names of `Object`) — not `"None"` as the package description assumed; the
  count 720 comes from the summary and is verified by the walk ending at `ImportOffset`.
- Round trip: `appCompressMemory(COMPRESS_LZO)` on block 0 shrinks it, decompresses back to the
  same bytes, and reports the needed size (returning FALSE) when given a 16-byte buffer.

### Harness fixes in `stubs.cpp`

- `FName::SafeString()` returns an `FString`; the two `printf("%ls", FName::SafeString(Event), Text)`
  calls passed the object itself, so the second `%ls` read `ArrayNum` as a pointer. Every formatted
  `GLog->Logf(...)` crashed in the harness before (nothing had logged with arguments); now `*FName::SafeString(Event)`.
- `DECLARE_STATS_GROUP(TEXT("StreamingDetails"), STATGROUP_StreamingDetails)`: Core's
  `UnAsyncLoading.cpp` registers `STAT_AsyncLoadingTime` in that group, whose factory lives in
  `Engine/Src/UnContentStreaming.cpp`; `GStatManager::CreateCanonicalStats` asserts without it in a
  Core-only link. Remove it if CoreSmoke ever links Engine.

## Verification

- While this ran, the shared tree did not compile Core: agent L's serialization port is in flight
  (`UnObj.cpp` / `UnAsyncLoading.cpp` / `UnClass.cpp` vs. edited `UnObjBas.h` / `UnIOBase.h` /
  `UnLinker.h`), later also `EngineClasses.h` edits. `build\agentK` therefore only proves that
  `UnMisc.cpp.obj` and `CoreSmoke.cpp.obj` compile against the shared headers.
- Full proof in a throw-away git worktree at `HEAD` (`b649b2b`) plus exactly the six files above,
  externals pointed at `Recompile/external/*-src` via `FETCHCONTENT_SOURCE_DIR_<NAME>`:
  - Debug: Core + CoreSmoke build, `CoreSmoke.exe` → `99 passed, 0 failed, 0 skipped`
  - RelWithDebInfo: same, `99 passed, 0 failed, 0 skipped`
  - logs: `build\agentK_wt_debug.log`, `build\agentK_wt_rel.log` (worktree removed afterwards).
- lzokay compiles with 3 × C4267 (size_t → uint8_t) under the global `/W4`; third-party, left as is.
  `UnMisc.cpp`, `CoreSmoke.cpp`, `stubs.cpp` add no warnings.

## Notes for the coordinator

- Retail evidence used: `serialization_delta_core.md` (`GBaseCompressionMethod = 2`, `CompressionFlags = 2`,
  `lzopro_lzo1x_decompress_safe`), the 2013 `Core.upk` bytes. The 2012 exe was not consulted.
- `LOADING_COMPRESSION_CHUNK_SIZE` (131072) and the chunk header layout match the 2013 package exactly;
  nothing Arkane-specific in the load path was needed (consistent with "loading identical" in the delta doc).
- Package I (python LZO1X) can cross-check against `TestCompressedChunk`: block 0 of `Core.upk`
  decompresses to 131,072 bytes whose first name entry is `02 00 00 00 21 00` + 8 flag bytes
  (`FString` "!" including the NUL, then the `QWORD` flags).
- Milestone 2 (load `Core.upk` … `Startup.upk`) can now go through `ULinkerLoad::CreateLoader` →
  `SerializeCompressed` once L's port lands.
