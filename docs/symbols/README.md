# Symbol exports (2012 Shipping build)

Source database: `docs/idb/shipping2012_v1.i64` (pristine snapshot of
`../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.exe.i64`, PDB applied).
IDA locks a database while it is open, so keep separate copies per consumer:
`shipping2012_work.i64` for the headless scripts and `shipping2012_v1.i64` (or another copy)
for the IDA MCP session / GUI. Never open the same file from two processes.
The database is rebased to image base 0, so `va == rva` in every export; the real image base of
the exe is 0x400000. DIA output is RVA-based and joins on `rva`.

| File | Producer | Committed |
|---|---|---|
| `db_verify.txt`, `db_verify_2013.txt` | `tools/ida/verify_db.py` | yes |
| `functions.csv` | `tools/ida/export_functions.py` + `tools/symbols/join.py` (adds file, line, module, origin) | yes |
| `segments.csv` | `tools/ida/export_functions.py` | yes |
| `globals.csv`, `imports.csv` | `tools/ida/export_globals.py` | yes |
| `natives.csv`, `classes.csv` | `tools/ida/export_natives.py` | yes |
| `natives_xcheck.md` | `tools/symbols/xcheck_natives.py` | yes |
| `hardcoded_names.csv` | `tools/ida/export_names.py` | yes |
| `functions_nofile.csv`, `functions_nofile_summary.md` | `tools/symbols/join.py` | yes |
| `compilands.csv`, `sourcefiles.txt` | `tools/pdb/dia_dump.py` | yes |
| `lines.csv`, `pdb_functions.csv`, `compiland_of.csv` | `tools/pdb/dia_dump.py` | **no** (large, regenerated in seconds) |
| `vtables.csv` | `tools/ida/export_vtables.py` | **no** (57 MB) |
| `../types/sizes.csv`, `xcheck_sdk.md` | `tools/ida/export_types.py`, `tools/symbols/xcheck_sdk.py` | yes |
| `../types/types.json`, `all_types.h` | `tools/ida/export_types.py` | **no** (111 MB / 42 MB) |

Regenerate everything:

```
python tools/ida/run.py tools/ida/verify_db.py        docs/idb/shipping2012_v1.i64 docs/symbols/db_verify.txt
python tools/ida/run.py tools/ida/export_functions.py docs/idb/shipping2012_v1.i64
python tools/ida/run.py tools/ida/export_globals.py   docs/idb/shipping2012_v1.i64
python tools/ida/run.py tools/ida/export_vtables.py   docs/idb/shipping2012_v1.i64
python tools/ida/run.py tools/ida/export_natives.py   docs/idb/shipping2012_v1.i64
python tools/ida/run.py tools/ida/export_names.py     docs/idb/shipping2012_v1.i64
python tools/ida/run.py tools/ida/export_types.py     docs/idb/shipping2012_v1.i64
python tools/pdb/dia_dump.py ../Dishonored_Debug2012/Binaries/Win32/DishonoredGame-Shipping.pdb --functions docs/symbols/functions.csv
python tools/symbols/join.py
python tools/symbols/module_map.py
python tools/symbols/xcheck_natives.py
python tools/symbols/xcheck_sdk.py
python tools/symbols/verify_phase1.py
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
* The 2013 database (`Dishonored.exe.i64`) is a plain auto-analysis: 34,059 functions, 93 %
  unnamed, 18 local types. Phase 7 populates it from the 2012 names.
