# Agent E — Core serialization convergence (2026-09-25)

Deliverables: `resources/docs/serialization_delta_core.md`, `resources/docs/function_status_seed.csv`
(merge into `function_status.csv`), decompiles under `resources/reference/decomp/core_serialize/`.

## How it was produced

- Database: `resources/docs/idb/shipping2012_agentE.i64` only (headless idalib via `resources/tools/ida/run.py`).
- `resources/reference/decomp/core_serialize/patterns.txt` + `decompile_funcs.py` gave 219 functions. That script
  names files by sanitized short name, so every `operator<<` overload and every ctor/dtor pair collide
  (`operator_.c`, `FArchiveAsync_FArchiveAsync.c` is the destructor). `_decompile_by_rva.py` (same directory,
  file name suffixed with the rva) was used for the `operator<<` set and a few helpers; outputs in
  `core_serialize/operators/`.
- `_read_globals.py` prints the static initializers of the version/compression globals from `.data`.
- Comparison was manual: pseudocode vs `../UnrealEngine3/Development/Src/Core/Src` with the `grep -vE` comment
  filter to keep outputs readable. Name indices were resolved with `symbols/hardcoded_names.csv`, flag values with
  `types/types.json` enums and the reference headers.

## Things worth knowing

- All shipped packages are `PKG_StoreCompressed` with `COMPRESS_LZO`; `GBaseCompressionMethod` is 2. Without LZO
  nothing loads. `SerializePackageFileSummary` swaps the loader to `FArchiveAsync` for these packages, so the async
  IO system (LZO on the IO thread) is on the synchronous `LoadPackage` path as well.
- Dishonored's engine-version enum does not match 10897 above ~766 (`VER_TEXTURE_PREALLOCATION` is 770, 796 is an
  Arkane field). All thresholds are < 801 anyway.
- `UObject` has no `NetIndex`; ObjectFlags bit 63 replaces it.
- Hex-Rays does not show C++ catch blocks; `try/catch` in the reference cannot be confirmed or refuted from
  pseudocode.
- Not diffed (decompiled only): `FAsyncIOSystemBase`, `FBufferReader*`, `AsyncPreloadPackage`,
  `RemapLinkerPackageNamesForMultilanguageCooks`, `StaticLoadObject`, `DissociateImportsAndForcedExports`,
  `LookupAllOutstandingCrossLevelExports`, `FUntypedBulkData::*`, `UStruct::SerializeExpr` (opcodes already checked
  by `symbols/opcodes.md`).
