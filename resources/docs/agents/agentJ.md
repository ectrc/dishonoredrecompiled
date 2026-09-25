# Agent J — Name propagation 2012 → 2013

Scope: PHASE3.md package J. Propagate the 2012 PDB function names onto the retail 2013 exe.
Databases used: `resources/docs/idb/shipping2012_agentJ.i64` (2012, image base 0) and
`resources/docs/idb/retail2013_agentJ.i64` (2013, image base 0x400000), headless only. Scratch
files (feature dumps, probes, patch scripts) are in `resources/reference/agentJ/` (gitignored).

## Deliverables

| File | What |
|---|---|
| `resources/tools/ida/match_functions.py` | `analyze` / `export` / `match` / `apply` / `report` sub-commands (docstring has the exact command lines) |
| `resources/docs/symbols/match_2012_2013.csv` | 91k rows: every 2012 function (`rva_2012, rva_2013, name, ratio, method, size_2012, size_2013, name_2013, kind`), the unmatched 2013 functions (`method=new`) and the matched named globals (`kind=global`) |
| `resources/docs/symbols/match_2012_2013.md` | totals, methods, per-module table, validation/caveats, landmarks, removed 2012 functions, new 2013 functions with sizes and DLC/curl/Steam categories |
| `resources/docs/idb/retail2013_named.i64` | the analysed 2013 db with 47,815 function names (ratio ≥ 0.9), 13,690 global names, 2,053 exec functions named from the 2013 native table, and a `2012 rva … ratio … method` comment on every matched function |
| `resources/docs/symbols/functions_2013.csv`, `natives_2013.csv`, `classes_2013.csv`, `globals_2013.csv` (+ `segments_2013.csv`, `imports_2013.csv` which the same exporters emit) | Phase 1 exporters run on the named db |
| `.gitignore` | `resources/tools/diaphora/` added (Diaphora checkout) |

## Numbers (2013 retail db unless stated)

* The shipped `Dishonored.exe.i64` was saved **before IDA's auto-analysis finished** (`auto_is_ok()`
  was false): 34,059 functions, 2.8 MB of `.text` disassembled but outside any function. `analyze`
  ran `auto_wait()` (→ 63,877 functions) and defined functions at the remaining code heads that are
  call/jump targets or follow a function (→ **65,841 functions**, vs 66,394 in 2012).
* Matching: **54,946 of 66,394 2012 functions matched (82.8 %)**, **48,051 with ratio ≥ 0.9 (72.4 %)**
  (acceptance: ≥ 60 %). 11,448 2012 functions have no 2013 counterpart, 10,895 2013 functions are new.
  Named globals matched: 14,393 (13,690 at ratio ≥ 0.9), incl. `GNatives`, `GRegisterNative`,
  `??_7UObject@@6B@`, the `_int<Class>exec<Func>` statics.
* Landmarks: `FEngineLoop::Init` 0x628d90 → 0x5e11b0 (0.978), `ULinkerLoad::CreateLoader` 0x898a0 →
  0x8e2a0 (0.990), `UClass::Serialize` 0x96b70 → 0x9a6c0 (0.752, code changed — expected, see
  `serialization_delta_core.md`), `FName::StaticInit` 0x3bdc0 → 0x3ae80 (1.000). All RVAs are
  2012 → 2013 (2013 RVA + 0x400000 = VA).
* `verify_db.py` on `retail2013_named.i64`: unnamed functions **25.9 %** (was 93.1 %), `uobject_vtable=yes`.
  Types are still absent (not in scope).
* `natives_2013.csv`: 2,160 exec functions (2012: 2,165), 170 `StaticClass` functions, and the
  DLC natives: 10 `Req_DLC05_*` plus 12 DLC06/DLC07 exec functions (`UDisDLC05MoviePlayerLeaderboard::execReq_DLC05_Leaderboards…`, …).
  Native indices: 35 numbered `GRegisterNative` call sites + 243 inlined `GNatives` stores found.

## Method (short; the .md has the per-method table)

Own matcher (Diaphora was cloned and its headless mode works, but its export ran ~22 % of the 2012
`.text` in 28 min without the decompiler; two exports + the diff would have been several hours per
iteration, so it was stopped). Per function the exporter records a fixup-masked byte hash (all
relocated dwords and rel32 targets zeroed — both exes have full `.reloc` tables: 575k / 608k
fixups), the instruction token stream, callees, string references (direct, via the first dwords of
a referenced data struct, and inherited from single-caller callees), imports, data references, and
the function-pointer runs in data (vtables, `GNatives`, CRT init table; split at code-referenced
addresses). Passes: import thunks → unique byte hash (also unique in "shape", see below) → string
anchors → iterated {global voting, vtable slot alignment by LCS, callee/caller/global neighbours
with agreement scoring, unique token streams, link-order block/window alignment} until no pass adds
a pair. Ratio = token-stream similarity (1.0 identical), method = evidence.

## Findings worth knowing

1. **Sibling swaps from layout shifts.** 2013 inserted virtuals/members, so 2012 `execSetRotation`
   (`call [eax+0x228]`) is byte-identical to 2013 `execSetTranslation` (now slot 0x228). Validated
   against the 2013 native table, 27 % of the exec names from a naive unique-byte-hash rule were
   sibling swaps. Fix: uniqueness must also hold for the displacement-abstracted shape; siblings are
   paired by link order only when no other same-shape candidate is in the window
   (`order-sibling`, ratio capped at 0.89, not applied). After the fix: 25 of 1,141 propagated exec
   names disagreed with the table (2.2 %), and the table overrides them anyway.
2. **2013 native registration table.** The retail exe has a static table of
   `{"<Class>exec<Func>", &Class::execFunc}` records (2,560 records, 2,053 distinct functions; up to
   6 names per address from COMDAT folding — e.g. all stubbed `UDishonoredCheatManager` natives fold
   into one function). `apply` names every exec function from it (912 newly named, incl. all
   DLC05–07 natives) and puts the other folded names in the function comment. 2012 instead had a
   dynamic initializer per native (`_dynamic_initializer_for__<Class>exec<Func>` → `GRegisterNative`),
   which is why ~1,000 of those 2012 initializers have no 2013 counterpart.
3. **Per-class registration structs.** 2013 has no per-class `GetPrivateStaticClass<Class>` (2,538 in
   2012, 52 instructions each, pushing `L"<Name>"`/`L"Engine"`); instead each class has a static
   struct in `.data` (`size, 0, 0, 0, name*, package*, within*, flags, cast flags, constructor*,
   ...`) and `StaticClassNoInline` calls one generic function with it (2013 `sub_9F8840` pattern).
   Agent H's size extraction should read `size` from these structs (first dword) rather than from
   `push <imm>` before the `UClass` constructor call.
4. **New 2013 code** (10,895 functions, 1.28 MB): 346 functions reference DLC05/06/07 strings
   (largest: the DLC leaderboard/challenge GFx movie players), 3 reference curl, the rest is mostly
   boilerplate of new classes and restructured code; the list with sizes is in the .md.
5. **Removed 2012 code** (11,448): `AutoInitializeRegistrantsDishonoredGame` (24.7 kB; 2013 registers
   through the table walk), `physWalking`, fluid-surface, FPS-chart HTML dumps, `libgfx_ime` (187 of
   192 unmatched — IME support dropped), `onlinesubsystemsteamworks` (133 of 250 unmatched, reworked),
   and the identical-body leftovers that could not be disambiguated.

## Rerun

```
python resources/tools/ida/run.py resources/tools/ida/match_functions.py resources/docs/idb/retail2013_agentJ.i64 --save analyze     # once
python resources/tools/ida/run.py resources/tools/ida/match_functions.py resources/docs/idb/shipping2012_agentJ.i64 export resources/reference/agentJ/features_2012.pkl
python resources/tools/ida/run.py resources/tools/ida/match_functions.py resources/docs/idb/retail2013_agentJ.i64 export resources/reference/agentJ/features_2013.pkl
python resources/tools/ida/match_functions.py match resources/reference/agentJ/features_2012.pkl resources/reference/agentJ/features_2013.pkl resources/docs/symbols/match_2012_2013.csv
copy resources\docs\idb\retail2013_agentJ.i64 resources\docs\idb\retail2013_named.i64
python resources/tools/ida/run.py resources/tools/ida/match_functions.py resources/docs/idb/retail2013_named.i64 --save apply resources/docs/symbols/match_2012_2013.csv
python resources/tools/ida/run.py resources/tools/ida/export_functions.py resources/docs/idb/retail2013_named.i64 _2013   # same for export_natives.py, export_globals.py
python resources/tools/ida/match_functions.py report resources/docs/symbols/match_2012_2013.csv resources/reference/agentJ/features_2012.pkl resources/reference/agentJ/features_2013.pkl resources/docs/symbols/match_2012_2013.md "<apply log line>"
```

## Open points

* Ratio < 0.9 pairs (6,895) are listed in the CSV with their evidence; a reviewer can promote them
  individually. The `order-sibling` rows (1,345) are the sibling-ambiguous ones.
* No types on the 2013 db yet; the 2012 PDB types could be applied per matched function prototype
  once the 2013 layouts (H, I) are known.
* `db_verify_2013.txt` was not regenerated (not in the package's file list); the numbers above come
  from running `verify_db.py` without an output file.
