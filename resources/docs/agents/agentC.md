# Agent C — Core additions Arkane made: bspatch/bzip2 and Pool.h

Scope: the Core source files that exist in Dishonored's build but not in the UE3 10897 reference
tree. All six stubs are now implemented, plus one new public header. IDA work was done on my own
database copy `resources/docs/idb/shipping2012_agentC.i64` (headless only); decompiles and
scratch scripts are under `resources/reference/agentC/` (gitignored).

## Files

| File | What it implements | Origin |
|---|---|---|
| `Core/Src/bspatch/bzlib.h` | libbzip2 public API (decompress side), `InMemoryFile`, and the `bzlib_private.h` subset the decompressor needs (`DState`, `BZ_X_*` states, `BZ_GET_*` macros, CRC/rand macros) appended under `#ifdef BZ_PRIVATE` | bzip2 1.0.6, altered |
| `Core/Src/bspatch/bzlib.cpp` | `BZ2_bzDecompressInit/Decompress/DecompressEnd`, `unRLE_obuf_to_output_FAST/SMALL`, `BZ2_indexIntoF`, `BZ2_bz__AssertH__fail`, `bzFile` + `BZ2_bzReadOpen/Read/ReadClose` over `InMemoryFile`, and the `BZ2_rNums`/`BZ2_crc32Table` tables (from randtable.c / crctable.c) | bzip2 1.0.6, altered |
| `Core/Src/bspatch/decompress.cpp` | `makeMaps_d`, `BZ2_decompress` | bzip2 1.0.6, verbatim |
| `Core/Src/bspatch/huffman.cpp` | `BZ2_hbCreateDecodeTables` only (the two encoder helpers are not linked by Dishonored) | bzip2 1.0.6, trimmed |
| `Core/Src/bspatch/bspatch.cpp` | `offtin`, `ArkBsPatch` = bsdiff 4.3 `bspatch.c` main() applied to memory buffers | bsdiff 4.3 (Colin Percival, BSD-2), altered |
| `Core/Inc/bspatch.h` | **new**: declaration of `ArkBsPatch` for `ULinkerLoad::CreateLoader` | — |
| `Core/Inc/Pool.h` | `TGrowablePoolPolicy`, `TGrowablePoolThreadSafePolicy`, `TGrowOnlyPoolPolicy`, `TPoolRaw<Policy>` (+ nested `Chunk`), `TPool<T, Policy>` | written from the decompile |

### Licence notes

* bzip2/libbzip2 is BSD-style (Julian Seward, 1996–2010). The full licence text is in `bzlib.h`;
  each .cpp carries the standard bzip2 file header plus an "ALTERED SOURCE VERSION" paragraph, as
  clause 3 of the licence requires. Alterations: decompress-only subset, `FILE*` → `InMemoryFile*`
  in the high-level reader, `bzlib_private.h` folded into `bzlib.h`, the two tables folded into
  `bzlib.cpp`, C++ casts on `BZALLOC`/`malloc`, `#undef small` (windows.h/rpcndr.h defines it).
* bspatch is BSD 2-clause (Colin Percival 2003–2005); header kept in `bspatch.cpp` with a note on
  the adaptation.
* Pool.h is Arkane code reconstructed from the PDB/decompile; no third-party licence.

## What the decompile says (and how the code matches it)

**bzip2 version.** Stock 1.0.6: `BZ2_decompress` contains the 1.0.6-only `if (N >= 2*1024*1024)`
guard, the `nInUse == 0`, `origPtr >= nblock`, `unzftab`/`cftab` range checks and the
`nSelectors < 1` check; `BZ2_bz__AssertH__fail`'s "internal error number %d" text is inlined into
it. The PDB local names in `unRLE_obuf_to_output_FAST` (`c_tt`, `cs_next_out`, `avail_out_INIT`,
`ro_blockSize100k`, `s_save_nblockPP`) are the stock ones. `DState` from `types.json` matches the
stock struct field for field (64116 bytes), `bz_stream` is 48 bytes, `bzFile` is 0x13CC with
`handle` at 0, `bufN` at 5004, `writing` at 5008, `strm` at 5012, `lastErr` at 5060,
`initialisedOk` at 5064 — stock layout with the `FILE*` replaced by an `InMemoryFile*`.
Public entry points are `extern "C" __stdcall` (`_BZ2_bzRead@16`, ...), the private ones C++
(`?BZ2_decompress@@YAHPAUDState@@@Z`), exactly what stock `bzlib.h`/`bzlib_private.h` produce
when compiled as C++ with `BZ_EXPORT`. `BZ2_bzDecompressInit` and `BZ2_bzReadOpen` are not in the
PDB because they were inlined into `ArkBsPatch` (the inlined body — `lastErr = 0`, `handle = &cp`,
`bufN = 0`, `bzalloc/bzfree/opaque = NULL`, `BZALLOC(64116)`, `state = BZ_X_MAGIC_1`, ... — is
stock). The two data tables were read back from the exe (.data 0xe2b230 / 0xe2b630) and are the
stock tables.

**InMemoryFile** (`bzlib.h:187` in the PDB, 12 bytes: `const BYTE* m_pBuffer; const UINT
m_nBufferSize; UINT m_nCurrentPosition`). Only `fread` survived as a symbol; `BZ2_bzRead`'s
decompile shows `myfeof` as `if (pos < size) { pos++; pos--; }` i.e. `fgetc`/`ungetc` inlined, and
`ferror` folded to 0. The header provides the four as static functions with the CRT signatures.

**ArkBsPatch** (`bspatch.cpp:76`). Header at `PatchData[0..31]`: `memcmp("BSDIFF40")` is
evaluated with the result discarded (a `verify()` in Shipping), `bzctrllen`/`bzdatalen`/`newsize`
via `offtin`, three `InMemoryFile`s at offsets 32, 32+X, 32+X+Y, `appMalloc(newsize + 1, 8)`,
**`OutBufferLen = newsize + 1`** (the caller receives the allocation size, not `newsize`), then
the stock control/diff/extra loop with `oldpos += ctrl[0] + ctrl[2]`. All of bspatch's `errx`
sanity checks vanished in Shipping, so they are `check`/`checkf` here. `offtin` reads only bytes
0..3 and the sign bit of byte 7 in the exe: that is stock `offtin` with a 32-bit `off_t`, which is
what the port does with `INT`.

**Pool.h.** PDB lines: ctor 262, dtor 283, `AllocateNewChunk` 296, `AllocateRawBuffer` 340,
`DestroyRawBuffer` 469. Layout (56 bytes, identical for all three policies): `m_pFirstFreeElement,
m_pFirstChunkWithFreeElement, m_pLastChunkWithFreeElement, m_iElementAlignedSize,
m_iGrowNbrOfElements, m_iInitialNbrOfElements, m_pFirstChunk, FCriticalSection
m_RequestCriticalSection`. `Chunk` (32 bytes): `m_pNext, m_pNextChunkWithFreeElements,
m_pPrevChunkWithFreeElements, m_pRawElements, m_pRawEndElements, m_iNbrFreeElements,
m_iNbrElements, m_pFirstFreeElementInChunk`; elements start at `Align(chunk+1, 8)` and are
`appMalloc(sizeof(Chunk) + n * size, 8)`. Behaviour per policy, from the decompiles:

* Growable: chunks with free elements form a doubly linked list kept in ascending free-count
  order (allocation takes from the head and swaps it one step down if needed; destruction bubbles
  the chunk down); a chunk whose free count reaches `m_iNbrElements` is `appFree`d; a chunk going
  from 0 to 1 free goes to the head. Grow logic: `AllocateNewChunk(m_pFirstChunk == NULL &&
  m_iInitialNbrOfElements ? m_iInitialNbrOfElements : m_iGrowNbrOfElements)` in a loop until a
  chunk with a free element exists.
* ThreadSafe: same code with `FCriticalSection::Lock()` (TryEnter then Enter, which is UE3's
  `Lock`) at the top of each loop iteration and `Unlock()` after growing / on return.
* GrowOnly: allocation pops `m_pFirstFreeElement`; destruction pushes onto it (seen inlined in
  `FDisJobQueue::clearMap`); chunks are never released. `AllocateNewChunk` is shared (it resets
  all three list heads to the new chunk — safe because it is only called when nothing is free).
* Ctor stores `m_iElementAlignedSize` in the body (after the critical section) and calls
  `AllocateNewChunk(iInitial)` when `iInitial > 0`; dtor frees the `m_pNext` chain.
* `TPool<T,Policy>(initial, grow)` passes `Align(sizeof(T), 8)`; the static pools' initial/grow
  values were read from .data: `g_PrimitiveSceneInfoPool` 4000/100 (208 B),
  `s_LightPrimitiveInteractionPool` 5000/500 (56 B), `s_StaticMeshPool` (thread safe) 2000/500
  (256 B), `g_DisJobQueueAsyncJobPool` (grow only) 100/20 (16 B). Call sites do
  `new (pool.AllocateRawBuffer()) T(...)` and `p->~T(); pool.DestroyRawBuffer(p)`; `TPool` adds
  `Allocate()`/`Destroy()` for that.
* One deliberate deviation, marked `// DISHONORED:` in `AllocateRawBuffer`: when the head chunk is
  swapped with its successor the original never updated the back link of the chunk that now
  follows it; the port sets it. (Functional rebuild, not byte matching.)

## Verification

1. **Standalone harness** (`resources/reference/agentC/test/`, stand-in `CorePrivate.h`,
   MSVC x86 /O2): `bstest.exe` applies patches produced by Python `bsdiff4` (same BSDIFF40 format)
   and decodes streams produced by Python `bz2`. 5/5 bspatch cases pass (200 KB random with
   insert/replace/delete, 1.8 MB repetitive multi-block input, empty old, empty new, identical) with
   `OutBufferLen == newsize + 1` and byte-exact output; 28/28 bzip2 cases pass (empty, 1 byte,
   200 KB random, 1.8 MB text, 2.5 MB random, 3 MB zeros, 1.4 MB "ab", each at levels 1 and 9,
   `small` 0 and 1).
2. **Pool behaviour** (`test/pool_fake/pooltest.exe`): 40 rounds of random allocate/destroy per
   policy (initial/grow of the real pools plus tiny and lazy configurations) with duplicate and
   alignment checks, full drains to force chunk release/reuse, destructor/constructor checks, plus
   direct `TPoolRaw` use as `AllocateObjectFromPool` does. All pass.
3. **Core flags**: the four .cpp files and a scratch TU including `CorePrivate.h`, `bzlib.h`
   (`BZ_PRIVATE`), `Pool.h`, `bspatch.h` compile with the exact `cl` line from `build\agentC`
   (`/W4 /permissive- -std:c++17 /Zp4`, all Core defines) with **0 errors, 0 warnings**, and
   `static_assert`s for `sizeof(TPoolRaw<>) == 56`, `sizeof(InMemoryFile) == 12`,
   `sizeof(bz_stream) == 48`, `sizeof(DState) == 64116` hold under /Zp4.
4. `cmake --build build\agentC --target Core` itself currently stops in the first TU
   (`Color.cpp`) on the **uncommitted** `Core/Inc/DishonoredLayouts.h` static_asserts another
   agent is adding through `Core.h` ("add /Zp4 ...", `FArchive` 136, `UClass` 456, ...); the
   cmake change that pairs with it is also uncommitted, so no Core TU builds in the shared tree
   right now. That is outside my files; the compile in (3) used `-DDISHONORED_LAYOUT_CHECKS=0`
   to get past it.

## Callers — where Dishonored patches

* `ArkBsPatch` has exactly one caller: `ULinkerLoad::CreateLoader` (rva 0x898a0). Dishonored's
  `ULinkerLoad::FPackagePrecacheInfo` grew from 12 to 24 bytes: `SynchronizationObject,
  PackageData, PackageDataSize` + `PatchSynchronizationObject, PatchPackageData,
  PatchPackageDataSize`. CreateLoader waits for both async reads, and if `PatchPackageDataSize >
  0` calls `ArkBsPatch(PackageData, PackageDataSize, PatchPackageData, PatchPackageDataSize,
  Buffer, OutBufferLen)`, frees both inputs, stores `Buffer/OutBufferLen` as the new
  `PackageData/PackageDataSize` and continues with `FBufferReaderWithSHA` on the patched image.
* The patch is queued by the Arkane-only `ULinkerLoad::AsyncPreloadPackage(const TCHAR*)`
  (rva 0x68810, static): after issuing the package read it builds
  `..\..\DishonoredGame\Patches\<CleanFilename>.bs`, and if `GFileManager->FileSize()` is >= 0
  allocates the buffer and `FIOSystem::LoadData`s it under a fresh `FThreadSafeCounter`. So
  package patching is a file-level bsdiff of a cooked package applied in memory at load time
  (DLC/title-update mechanism; the game folders on this machine have no `Patches` directory, so
  retail 2013 probably shipped no .bs files).
* `BZ2_bzRead`/`BZ2_bzReadClose` are called only from `ArkBsPatch`; `BZ2_bzDecompress` only from
  `BZ2_bzRead`; `BZ2_bzDecompressEnd` only from `BZ2_bzReadClose`; `InMemoryFile::fread` only from
  `BZ2_bzRead`. No other bzip2 use in the exe (package compression stays zlib/LZO).
* `TPoolRaw` users: Core `AllocateObjectFromPool(UClass*)` / `DestroyObjectFromPool(UObject*)`
  (unobj.cpp:81/121; `TMap<INT, TPoolRaw<TGrowablePoolPolicy>*> s_ObjectPools` keyed by
  `Align(PropertiesSize, 8)`, parameters from `TMap<INT, ObjectPoolParam{m_iInitSize,
  m_iGrowSize}> s_ObjectPoolParams`, only for classes with `MinAlignment <= 8`); Engine
  `g_PrimitiveSceneInfoPool` (FScene::AddPrimitive / FPrimitiveSceneInfo::FinishCleanup),
  `s_LightPrimitiveInteractionPool` (FLightPrimitiveInteraction::Create, FLightSceneInfo::Detach,
  FPrimitiveSceneInfo::RemoveFromScene), every `TStaticMeshDrawList<>::s_ElementHandlePool`
  (FElement ctor / FElementHandle::Release), `s_StaticMeshPool` (FBatchingSPDI::DrawMesh,
  FDecalInteraction, ~FPrimitiveSceneInfo), `g_DisJobQueueAsyncJobPool` (FDisJobQueue::AddJob /
  clearMap).

## Open questions

* Dishonored's tree almost certainly had `randtable.cpp` (the unity object is
  `Unity_randtableEtAl.cpp.obj`) and probably `crctable.cpp` and `bzlib_private.h`; they hold no
  line info so the PDB does not list them. I kept the tables in `bzlib.cpp` and the private
  declarations in `bzlib.h` to stay within the stub file set — split them out if file parity with
  Arkane's tree matters.
* `OutBufferLen = newsize + 1` is faithful to the exe; the linker then serializes from a buffer
  one byte longer than the package. Harmless, but whoever ports `CreateLoader` should not
  "fix" it to `newsize` without checking `FBufferReaderWithSHA` size assumptions.
* `TPool<T>` element size: the four static pools all have `sizeof(T) % 8 == 0`, so whether
  Arkane passed `sizeof(T)` or `Align(sizeof(T), 8)` is not observable; the port aligns (as
  `AllocateObjectFromPool` does explicitly).
* `AllocateObjectFromPool`/`DestroyObjectFromPool` and `AsyncPreloadPackage` are UnObj.cpp /
  UnLinker.cpp work for whoever owns those files; `bspatch.h` is ready for the latter.
* The whole Core target is blocked by the in-progress `DishonoredLayouts.h` (+ missing `/Zp4`)
  change in the shared tree; re-run `cmake --build build\agentC --target Core` once that lands.
