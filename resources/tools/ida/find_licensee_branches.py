"""P2.8: inventory every archive-version comparison (licensee and engine) in serialization code.

Decompiles every function that looks like serialization (name or file based) and records each
`ArLicenseeVer` / `LicenseeVer()` / `ArVer` / `Ver()` comparison with its operator and constant.

Outputs resources/docs/symbols/licensee_branches.csv and licensee_branches.md.
Usage: python resources/tools/ida/run.py resources/tools/ida/find_licensee_branches.py <db.i64>
"""
import collections
import csv
import re

import ida_hexrays

from _common import SYMBOLS_DIR, demangle_full, function_name, iter_functions, log, rva

NAME_RE = re.compile(r"::Serialize\w*\(|operator<<\(|::Ser\(|SerializeTaggedProperties|::Load\(|::Link\(|::Preload\(|::CreateExport\(|::CreateImport\(|LoadAllObjects|::PostLoad\(", re.I)
FILE_RE = re.compile(r"unlinker|unbulkdata|savepackage|unasyncloading|unobj\.cpp|unclass\.cpp|unprop\.cpp|unarchive|scriptserialization", re.I)
CMP_RE = re.compile(r"(ArLicenseeVer|LicenseeVer\(\)|ArVer|(?<![A-Za-z_])Ver\(\))\s*(<=|>=|==|!=|<|>)\s*(0x[0-9A-Fa-f]+|\d+)")
CMP_REV_RE = re.compile(r"(0x[0-9A-Fa-f]+|\d+)\s*(<=|>=|==|!=|<|>)\s*[\w>.-]*?(ArLicenseeVer|LicenseeVer\(\)|ArVer|Ver\(\))")
FLIP = {"<": ">", ">": "<", "<=": ">=", ">=": "<=", "==": "==", "!=": "!="}


def file_of(func) -> str:
    return ""


def main() -> None:
    if not ida_hexrays.init_hexrays_plugin():
        log("Hex-Rays not available")
        return
    files = {}
    try:
        with (SYMBOLS_DIR / "functions.csv").open(newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                files[int(row["rva"], 16)] = (row["file"], row["module"])
    except FileNotFoundError:
        pass

    rows = []
    scanned = 0
    failed = 0
    for func in iter_functions():
        name = demangle_full(function_name(func))
        file, module = files.get(rva(func.start_ea), ("", ""))
        if not (NAME_RE.search(name) or FILE_RE.search(file)):
            continue
        scanned += 1
        try:
            text = str(ida_hexrays.decompile(func.start_ea))
        except ida_hexrays.DecompilationFailure:
            failed += 1
            continue
        found = [(kind, op, int(val, 0)) for kind, op, val in CMP_RE.findall(text)]
        found += [(kind, FLIP[op], int(val, 0)) for val, op, kind in CMP_REV_RE.findall(text)]
        for kind, op, val in found:
            family = "licensee" if "Licensee" in kind else "engine"
            rows.append([f"0x{rva(func.start_ea):x}", module, file.rsplit("\\", 1)[-1], name, family, kind, op, val])
        if scanned % 500 == 0:
            log(f"scanned={scanned} rows={len(rows)}")

    out = SYMBOLS_DIR / "licensee_branches.csv"
    with out.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["rva", "module", "file", "function", "family", "expr", "op", "value"])
        w.writerows(rows)

    lic = [r for r in rows if r[4] == "licensee"]
    per_module = collections.Counter(r[1] for r in lic)
    thresholds = sorted({r[7] for r in lic})
    funcs = sorted({(r[1], r[3]) for r in lic})
    with (SYMBOLS_DIR / "licensee_branches.md").open("w", encoding="utf-8") as f:
        f.write("# Archive version comparisons in serialization code (2012 Shipping)\n\n")
        f.write(f"Scanned {scanned} serialization-like functions ({failed} failed to decompile). {len(rows)} comparisons: {len(lic)} licensee, {len(rows) - len(lic)} engine version.\n\n")
        f.write("## Licensee thresholds seen\n\n" + ", ".join(str(t) for t in thresholds) + "\n\n")
        f.write("## Licensee comparisons per module\n\n| Module | Comparisons |\n|---|---:|\n")
        for m, n in per_module.most_common():
            f.write(f"| {m or '?'} | {n} |\n")
        f.write(f"\n## Functions with licensee branches ({len(funcs)}) — port verbatim before milestone 3\n\n")
        for m, fn in funcs:
            f.write(f"- `{m}` `{fn}`\n")
    log(f"scanned={scanned} failed={failed} comparisons={len(rows)} licensee={len(lic)} thresholds={thresholds}")


if __name__ == "__main__":
    main()
