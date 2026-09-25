# Symbol exports (2012 Shipping build)

These exports describe the **2012** QA build, which has a PDB. They are the helping hand, not
the target: the rebuilt exe must match the **2013 retail** `Dishonored.exe`, whose structs
differ in size and members in many places. Treat every size/offset here as provisional until
the retail check (PLAN.md Phase 2b) confirms or corrects it.

Source database: `resources/docs/idb/shipping2012_v1.i64` (pristine snapshot of
`../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.exe.i64`, PDB applied).
IDA locks a database while it is open, so keep separate copies per consumer:
`shipping2012_work.i64` for the headless scripts and `shipping2012_v1.i64` (or another copy)
for the IDA MCP session / GUI. Never open the same file from two processes.
The database is rebased to image base 0, so `va == rva` in every export; the real image base of
the exe is 0x400000. DIA output is RVA-based and joins on `rva`.

| File | Producer | Committed |
|---|---|---|
| `db_verify.txt`, `db_verify_2013.txt` | `resources/tools/ida/verify_db.py` | yes |
| `functions.csv` | `resources/tools/ida/export_functions.py` + `resources/tools/symbols/join.py` (adds file, line, module, origin) | yes |
| `segments.csv` | `resources/tools/ida/export_functions.py` | yes |
| `globals.csv`, `imports.csv` | `resources/tools/ida/export_globals.py` | yes |
| `natives.csv`, `classes.csv` | `resources/tools/ida/export_natives.py` | yes |
| `natives_xcheck.md` | `resources/tools/symbols/xcheck_natives.py` | yes |
| `hardcoded_names.csv` | `resources/tools/ida/export_names.py` | yes |
| `functions_nofile.csv`, `functions_nofile_summary.md` | `resources/tools/symbols/join.py` | yes |
| `compilands.csv`, `sourcefiles.txt` | `resources/tools/pdb/dia_dump.py` | yes |
| `lines.csv`, `pdb_functions.csv`, `compiland_of.csv` | `resources/tools/pdb/dia_dump.py` | **no** (large, regenerated in seconds) |
| `vtables.csv` | `resources/tools/ida/export_vtables.py` | **no** (57 MB) |
| `../types/sizes.csv`, `xcheck_sdk.md` | `resources/tools/ida/export_types.py`, `resources/tools/symbols/xcheck_sdk.py` | yes |
| `../types/types.json`, `all_types.h` | `resources/tools/ida/export_types.py` | **no** (111 MB / 42 MB) |

Regenerate everything:

```
python resources/tools/ida/run.py resources/tools/ida/verify_db.py        resources/docs/idb/shipping2012_v1.i64 resources/docs/symbols/db_verify.txt
python resources/tools/ida/run.py resources/tools/ida/export_functions.py resources/docs/idb/shipping2012_v1.i64
python resources/tools/ida/run.py resources/tools/ida/export_globals.py   resources/docs/idb/shipping2012_v1.i64
python resources/tools/ida/run.py resources/tools/ida/export_vtables.py   resources/docs/idb/shipping2012_v1.i64
python resources/tools/ida/run.py resources/tools/ida/export_natives.py   resources/docs/idb/shipping2012_v1.i64
python resources/tools/ida/run.py resources/tools/ida/export_names.py     resources/docs/idb/shipping2012_v1.i64
python resources/tools/ida/run.py resources/tools/ida/export_types.py     resources/docs/idb/shipping2012_v1.i64
python resources/tools/pdb/dia_dump.py ../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.pdb --functions resources/docs/symbols/functions.csv
python resources/tools/symbols/join.py
python resources/tools/symbols/module_map.py
python resources/tools/symbols/xcheck_natives.py
python resources/tools/symbols/xcheck_sdk.py
python resources/tools/symbols/verify_phase1.py
```

Caveats:

* Identical COMDAT folding: the linker merged byte-identical functions, so one address can carry
  several PDB names (74,804 DIA function symbols on 57,943 unique RVAs). IDA keeps one name per
  address; `vtables.csv` therefore sometimes shows a slot pointing at a function named after an
  unrelated class (e.g. an empty virtual folded with another empty virtual). `pdb_functions.csv`
  lists every name per RVA.
* `functions.csv` has 66,394 entries; 8,451 of them have no DIA function symbol (compiler
  generated thunks, inline constructors emitted as COMDATs, CRT). Lines and compilands for those
  are resolved by RVA range instead (`--functions` mode of `dia_dump.py`).
* The 2013 database: `resources/docs/idb/retail2013_named.i64` (agent J, `resources/tools/ida/match_functions.py`):
  65,841 functions, 82.8 % of the 2012 names propagated. Its exports live next to the 2012 ones:
  `functions_2013.csv`, `natives_2013.csv`, `classes_2013.csv`, `globals_2013.csv`, `imports_2013.csv`,
  `match_2012_2013.csv/.md`. Agents copy it (`retail2013_agent<X>.i64`) before opening it.
* Retail runtime layout: `../types/retail_sdk_layout.json` (git-ignored) from
  `python resources/tools/sdk/parse_codered_sdk.py` over the CodeRed dump; see `resources/docs/sdk_dump.md`.

| `../types/script_classes_{2012,2013}.json` | `resources/tools/pdb/read_package_classes.py` (needs `lzo1x.py`) | **no** (regenerate: `python resources/tools/pdb/read_package_classes.py` per agentI.md) |
| `../types/native_class_sizes.csv` | `resources/tools/ida/export_class_sizes.py` | yes |
